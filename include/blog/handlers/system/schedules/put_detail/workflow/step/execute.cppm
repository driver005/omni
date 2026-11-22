export module blog_handlers:schedules_put_detail_step;
import :domain_all;
import :posts_support;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::schedules::put_detail::workflow {

/**
 * @brief Ghost `schedules.publish` step.
 *
 * Strict-parity mapping (docs/parity/behavior/schedules.md):
 *   validation  -> preflight() (id required, resource in [posts,pages], global id format)
 *   permissions -> handler calls blog::handlers::posts::support::authorize(resource,"publish")
 *   query()     -> read scheduled post/page, tolerance/force guard, edit to published
 *   side effects-> core outbox events (post/published, post/edited, cache.invalidate), drained
 *   serializer  -> posts::support::clean_rows + local wrap (no registered :mapping partition)
 *
 * No shared file is edited. Helpers come from already registered partitions
 * (`:posts_support`, `:outbox_shim`, `core_serde`, `core_workflow`).
 */
class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>; // keys: status, body, invalidate
    static constexpr std::string_view name{"schedules_put_detail_execute"};

    struct Preflight {
        bool ok{true};
        std::string status{"200"};
        std::string body{"{}"};
        std::string resource;
        std::string id;
    };

    /** @brief Ghost input validation, run before auth and before the mutation. */
    [[nodiscard]] static Preflight preflight(std::string_view path)
    {
        Preflight out;
        const auto cut = path.find('?');
        const auto route = cut == std::string_view::npos ? path : path.substr(0, cut);
        std::vector<std::string> segs;
        for (auto s : std::views::split(route, '/')) {
            if (!s.empty()) segs.emplace_back(std::ranges::begin(s), std::ranges::end(s));
        }
        if (segs.size() < 3) {
            out.ok = false;
            out.status = "422";
            out.body = blog::api::ApiError{}.validation(
                "Validation (FieldIsRequired) failed for id", "Invalid options", "id");
            return out;
        }
        out.resource = segs[segs.size() - 2];
        out.id = segs.back();
        if (out.resource != "posts" && out.resource != "pages") {
            out.ok = false;
            out.status = "422";
            out.body = blog::api::ApiError{}.validation(
                "Validation (AllowedValues) failed for resource", "Invalid options", "resource");
            return out;
        }
        if (!valid_ghost_id(out.id)) {
            out.ok = false;
            out.status = "422";
            out.body = blog::api::ApiError{}.validation(
                "Validation (matches) failed for id", "Invalid options", "id");
            return out;
        }
        return out;
    }

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto pf = preflight(in.get_path());
        if (!pf.ok) {
            return core::workflow::StepResponse<Output>{{{"status", pf.status}, {"body", pf.body}}};
        }

        // data:['force'] — Ghost checks `force === true` (strict boolean).
        const auto fields = blog::api::JsonBody{}.parse(in.body_as_string());
        const bool force = fields.contains("force") && fields.at("force") == "true";

        const auto noop = [&pf]() {
            return core::workflow::StepResponse<Output>{
                {{"status", "200"}, {"body", "{\"" + pf.resource + "\":[]}"}}};
        };
        const auto error = [](std::string_view body) {
            return core::workflow::StepResponse<Output>{{{"status", "404"}, {"body", std::string{body}}}};
        };

        // Ghost postScheduling.publish() in-flight Set, expressed as context-local
        // ResourceStore state (no new process-global state).
        const std::string inflight_resource =
            "schedules_inflight:" + pf.resource + ":" + pf.id;
        if (ctx.store().resources().count(inflight_resource) != 0) {
            return noop();
        }
        ctx.store().resources().create(inflight_resource, "{}");
        struct InFlightGuard {
            blog::BlogContext* ctx;
            std::string resource;
            ~InFlightGuard() { clear_inflight(*ctx, resource); }
        } guard{&ctx, inflight_resource};

        // api[resource].read({id}, {status:'scheduled', context:{internal:true}}).
        const auto stored = ctx.store().find_by_resource(pf.resource, pf.id);
        if (!stored) return noop(); // NotFound swallowed -> NO_OP
        const auto pre = blog::api::JsonBody{}.parse(*stored);
        const auto sit = pre.find("status");
        if (sit == pre.end() || sit->second != "scheduled") return noop();

        // Tolerance window: Ghost default 5 minutes (shared/config/overrides.json).
        // The config service is outside this folder; the default is used verbatim and
        // recorded as a residual gap in the parity docs.
        const auto minutes = iso_offset_minutes(value_of(pre, "published_at"));
        if (minutes.has_value()) {
            const auto delta = *minutes - now_minutes();
            if (delta > kToleranceMinutes) return noop(); // fired too early
            if (delta < -kToleranceMinutes && !force) {
                return error(blog::api::ApiError{}.validation(
                    "Use the force flag to publish a post in the past.", "", "", "NotFoundError"));
            }
        }

        // api[resource].edit({status:'published', updated_at:now}, ...) — keep every field.
        auto doc = core_serde::JsonCodec{}.decode<serde::Value>(*stored);
        if (!doc) return noop();
        auto* object = std::get_if<serde::Value::Object>(&doc->get());
        if (!object) return noop();
        (*object)["status"] = serde::Value{std::string{"published"}};
        (*object)["updated_at"] = serde::Value{std::string{
            std::format("{:%Y-%m-%dT%H:%M:%S}+00:00",
                        std::chrono::floor<std::chrono::seconds>(
                            std::chrono::system_clock::now()))}};
        const std::string edit_body = core_serde::JsonCodec{}.encode(*doc);

        const auto result = ctx.store().edit(pf.resource, pf.id, edit_body);
        if (!result.ok()) return noop(); // 404 -> no-op, matching NotFound swallow

        // Output serializer schedules.all -> frame.response = {<resource>: [resource]}.
        // clean_rows() is the registered posts/page Ghost allow-list mapper.
        const auto wrapped = blog::api::JsonBody{}.parse_raw(result.body());
        const auto rows_it = wrapped.find(pf.resource);
        const std::string singular = pf.resource == "posts" ? "post" : "page";
        const std::string rows = rows_it == wrapped.end()
            ? std::string{"[]"}
            : blog::handlers::posts::support::clean_rows(rows_it->second, singular);
        const std::string body = "{\"" + pf.resource + "\":" + rows + "}";

        // Ghost model edit side effects: emitChange('published') + emitChange('edited'),
        // plus the cache-invalidation event that drives X-Cache-Invalidate / emit-events.
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(blog::handlers::outbox::Event{
            .type = singular + ".published", .payload = body, .aggregate_id = pf.id});
        outbox.emit(blog::handlers::outbox::Event{
            .type = singular + ".edited", .payload = body, .aggregate_id = pf.id});
        const std::string invalidate = handle_cache_invalidation(
            "published", "scheduled", value_of(pre, "uuid"));
        if (!invalidate.empty()) {
            outbox.emit(blog::handlers::outbox::Event{
                .type = "cache.invalidate", .payload = invalidate, .aggregate_id = pf.id});
        }
        [[maybe_unused]] const auto drained = outbox.drain();

        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body},
            {"invalidate", invalidate}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;

private:
    static constexpr std::int64_t kToleranceMinutes{5};

    [[nodiscard]] static std::string value_of(
        const std::map<std::string, std::string>& fields, std::string_view key)
    {
        const auto it = fields.find(std::string{key});
        return it == fields.end() ? std::string{} : it->second;
    }

    [[nodiscard]] static bool valid_ghost_id(std::string_view id)
    {
        if (id.size() == 2) {
            return std::tolower(static_cast<unsigned char>(id[0])) == 'm' &&
                   std::tolower(static_cast<unsigned char>(id[1])) == 'e';
        }
        if (id.size() != 24) return false;
        for (const char c : id) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        }
        return true;
    }

    [[nodiscard]] static std::string handle_cache_invalidation(
        std::string_view next, std::string_view prev, std::string_view uuid)
    {
        if ((next == "published" && prev != "published") ||
            (next == "draft" && prev == "published")) {
            return "/*";
        }
        if ((next == "draft" && prev != "published") ||
            (next == "scheduled" && prev != "scheduled")) {
            return "/p/" + std::string{uuid} + "/";
        }
        return {};
    }

    static void clear_inflight(blog::BlogContext& ctx, std::string_view resource)
    {
        const std::string rows = ctx.store().resources().list(resource);
        std::size_t pos = 0;
        while ((pos = rows.find("\"id\":\"", pos)) != std::string::npos) {
            const auto start = pos + 6;
            const auto end = rows.find('"', start);
            if (end == std::string::npos) break;
            ctx.store().resources().destroy(resource, rows.substr(start, end - start));
            pos = end + 1;
        }
    }

    [[nodiscard]] static bool take_number(std::string_view s, std::size_t& pos,
                                          int& out, std::size_t digits)
    {
        if (pos + digits > s.size()) return false;
        int value = 0;
        for (std::size_t i = 0; i < digits; ++i) {
            const char c = s[pos + i];
            if (c < '0' || c > '9') return false;
            value = value * 10 + (c - '0');
        }
        out = value;
        pos += digits;
        return true;
    }

    [[nodiscard]] static std::int64_t days_from_civil(int year, unsigned month, unsigned day)
    {
        year -= month <= 2;
        const std::int64_t era = (year >= 0 ? year : year - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(year - era * 400);
        const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
    }

    /** @brief RFC3339 -> epoch minutes, matching moment(published_at).diff(now,'minutes'). */
    [[nodiscard]] static std::optional<std::int64_t> iso_offset_minutes(std::string_view text)
    {
        std::size_t p = 0;
        int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
        if (!take_number(text, p, year, 4) || p >= text.size() || text[p++] != '-') return std::nullopt;
        if (!take_number(text, p, month, 2) || p >= text.size() || text[p++] != '-') return std::nullopt;
        if (!take_number(text, p, day, 2) || p >= text.size() ||
            (text[p] != 'T' && text[p] != 't')) return std::nullopt;
        ++p;
        if (!take_number(text, p, hour, 2) || p >= text.size() || text[p++] != ':') return std::nullopt;
        if (!take_number(text, p, minute, 2) || p >= text.size() || text[p++] != ':') return std::nullopt;
        if (!take_number(text, p, second, 2)) return std::nullopt;
        if (p < text.size() && text[p] == '.') {
            ++p;
            while (p < text.size() && std::isdigit(static_cast<unsigned char>(text[p]))) ++p;
        }
        std::int64_t offset = 0;
        if (p < text.size() && (text[p] == 'Z' || text[p] == 'z')) {
            ++p;
        } else if (p < text.size() && (text[p] == '+' || text[p] == '-')) {
            const int sign = text[p] == '-' ? -1 : 1;
            ++p;
            int offset_hour = 0, offset_minute = 0;
            if (!take_number(text, p, offset_hour, 2)) return std::nullopt;
            if (p < text.size() && text[p] == ':') ++p;
            if (p < text.size() && std::isdigit(static_cast<unsigned char>(text[p]))) {
                if (!take_number(text, p, offset_minute, 2)) return std::nullopt;
            }
            offset = sign * (offset_hour * 3600 + offset_minute * 60);
        } else {
            return std::nullopt; // Ghost moment() would use server TZ; no shared TZ service here.
        }
        const std::int64_t epoch = days_from_civil(year, static_cast<unsigned>(month),
                                                   static_cast<unsigned>(day)) * 86400
                                 + hour * 3600 + minute * 60 + second - offset;
        return epoch / 60;
    }

    [[nodiscard]] static std::int64_t now_minutes()
    {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) / 60;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::schedules::put_detail::workflow
