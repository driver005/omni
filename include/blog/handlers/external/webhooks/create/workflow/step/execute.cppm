export module blog_handlers:webhooks_create_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;

/**
 * @file execute.cppm
 * @brief Strict Ghost `webhooks.add` step.
 *
 * Shared abstractions used here:
 *  - `blog_handlers:outbox_shim` (registered partition of `blog_handlers`) for the
 *    `webhook.added` model change event — Ghost `Webhook.onCreated` -> `emitChange`.
 *  - `core_workflow` for the step driver, `interfaces` for the request, and the
 *    context-local `ApiDataStore` resources namespace for state.
 *
 * `blog_handlers:mapping` (`clean`/`wrap`) is the intended shared output layer but is
 * not a registered partition yet; `webhook_model` below is the local equivalent of
 * `local_mapping::clean(obj, "webhook")` until that registration lands.
 */
export namespace blog::handlers::webhooks::support {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

inline constexpr std::string_view kIntegrationHeader{"x-blog-integration-id"};
inline constexpr std::string_view kDefaultApiVersion{"v5.0"};

[[nodiscard]] inline std::string quote(std::string_view text)
{
    return blog::api::JsonBody{}.quote(text);
}

/** @brief Plain scalar from a raw JSON value; JSON `null` becomes the empty string. */
[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    if (raw == "null") return {};
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
        return std::string{raw.substr(1, raw.size() - 2)};
    }
    return std::string{raw};
}

[[nodiscard]] inline std::string lower(std::string_view text)
{
    std::string out{text};
    std::ranges::transform(out, out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

/** @brief UTC ISO-8601 with milliseconds (Ghost `new Date()` wire shape). */
[[nodiscard]] inline std::string now_iso()
{
    using namespace std::chrono;
    const auto tick = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    const long long seconds = tick / 1000;
    const long long millis = tick % 1000;
    long long z = seconds / 86400 + 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long year = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp < 10 ? mp + 3 : mp - 9;
    year += (month <= 2) ? 1 : 0;
    const unsigned sod = static_cast<unsigned>(seconds % 86400);
    return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
                       year, month, day, sod / 3600, (sod % 3600) / 60, sod % 60, millis);
}

/** @brief Input-pipeline unwrap: `data.webhooks[0]` / `webhooks[0]`, else the raw object. */
[[nodiscard]] inline blog::api::RecordDto unwrap(std::string_view raw)
{
    blog::api::JsonBody parser;
    blog::api::RecordDto fields = parser.parse_raw(raw);
    if (const auto it = fields.find("webhooks");
        it != fields.end() && it->second.starts_with('[')) {
        const auto rows = blog::api::parse_array(it->second);
        if (!rows.empty()) return rows.front();
    }
    if (const auto it = fields.find("data");
        it != fields.end() && it->second.starts_with('{')) {
        const auto data = parser.parse_raw(it->second);
        if (const auto w = data.find("webhooks");
            w != data.end() && w->second.starts_with('[')) {
            const auto rows = blog::api::parse_array(w->second);
            if (!rows.empty()) return rows.front();
        }
    }
    return fields;
}

[[nodiscard]] inline std::string scalar(const blog::api::RecordDto& fields, std::string_view key)
{
    const auto it = fields.find(std::string{key});
    return it == fields.end() ? std::string{} : unquote(it->second);
}

/** @brief Ghost `options:['id']` path extraction; query string ignored. */
[[nodiscard]] inline std::string id_tail(std::string_view path)
{
    const auto cut = path.find('?');
    const auto base = cut == std::string_view::npos ? path : path.substr(0, cut);
    const auto slash = base.find_last_of('/');
    const std::string tail{slash == std::string_view::npos ? base : base.substr(slash + 1)};
    return tail == "webhooks" ? std::string{} : tail;
}

/**
 * @brief The full model `toJSON` field set from the webhooks e2e snapshot:
 * `id,name,event,target_url,secret,api_version,integration_id,created_at,updated_at,
 * last_triggered_at,last_triggered_status,last_triggered_error`. Ghost's snapshot has
 * no `status` field, so none is emitted. `secret` is deliberately not masked.
 */
[[nodiscard]] inline std::string webhook_model(const blog::api::RecordDto& stored,
                                               const blog::api::RecordDto& input,
                                               std::string_view id,
                                               bool touch_updated)
{
    const auto raw = [&](const char* key) -> std::string {
        if (const auto it = input.find(key); it != input.end()) return it->second;
        if (const auto it = stored.find(key); it != stored.end()) return it->second;
        return "null";
    };

    blog::api::RecordDto out;
    out["id"] = quote(id);
    for (const char* key : {"name", "event", "target_url", "secret"}) {
        out[key] = raw(key);
    }
    const std::string api_version = raw("api_version");
    out["api_version"] = api_version.empty() || api_version == "null"
                       ? quote(kDefaultApiVersion) : api_version;
    out["integration_id"] = raw("integration_id");
    const std::string created = [&] {
        if (const auto it = stored.find("created_at"); it != stored.end()) return it->second;
        return quote(now_iso());
    }();
    out["created_at"] = created;
    out["updated_at"] = touch_updated ? quote(now_iso()) : created;
    for (const char* key : {"last_triggered_at", "last_triggered_status", "last_triggered_error"}) {
        out[key] = raw(key);
    }
    return blog::api::record_to_json(out);
}

} // namespace blog::handlers::webhooks::support

export namespace blog::handlers::webhooks::create::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"webhooks_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::webhooks::support;
        auto& ctx = *s_ctx;

        const std::string raw = in.body_as_string().empty()
                              ? std::string{"{}"} : std::string{in.body_as_string()};
        const auto fields = support::unwrap(raw);
        if (fields.empty()) {
            return core::workflow::StepResponse<Output>{"__VALIDATION__"};
        }

        const std::string event = support::scalar(fields, "event");
        const std::string target = support::scalar(fields, "target_url");
        std::string integration = support::scalar(fields, "integration_id");
        if (event.empty() || target.empty()) {
            return core::workflow::StepResponse<Output>{"__VALIDATION__"};
        }

        // Ghost's input serializer injects integration_id from context.integration.id.
        // The context-local stand-in is the authenticated integration header.
        bool injected = false;
        if (integration.empty()) {
            const std::string header{in.find_header(support::kIntegrationHeader)};
            if (header.empty()) {
                return core::workflow::StepResponse<Output>{"__MISSING_INTEGRATION__"};
            }
            integration = header;
            injected = true;
        }

        // WebhooksService.add dedupe: getByEventAndTarget compares case-insensitively.
        for (const auto& record : blog::api::parse_array(ctx.store().resources().list("webhooks"))) {
            const auto e = record.find("event");
            const auto t = record.find("target_url");
            if (e != record.end() && t != record.end() &&
                support::lower(support::unquote(e->second)) == support::lower(event) &&
                support::lower(support::unquote(t->second)) == support::lower(target)) {
                return core::workflow::StepResponse<Output>{"__DUPLICATE__"};
            }
        }

        auto stored_fields = fields;
        if (injected) {
            stored_fields["integration_id"] = support::quote(integration);
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto created = ctx.store().create(
            "webhooks", blog::api::record_to_json(stored_fields));
        if (!created.ok() || created.status() >= 400) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{"__VALIDATION__"};
        }

        std::string id;
        const auto created_fields = blog::api::JsonBody{}.parse(created.body());
        const std::string created_json = created_fields.contains("webhooks")
                                       ? created_fields.at("webhooks") : created.body();
        for (const auto& record : blog::api::parse_array(created_json)) {
            if (const auto it = record.find("id"); it != record.end()) {
                id = support::unquote(it->second);
                break;
            }
        }
        if (id.empty()) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{"__VALIDATION__"};
        }

        const auto record = blog::api::JsonBody{}.parse_raw(
            ctx.store().resources().get("webhooks", id));
        const std::string body = "{\"webhooks\":["
                               + support::webhook_model(record, stored_fields, id, false) + "]}";
        // webhook.added == Ghost Webhook.onCreated -> emitChange('added').
        outbox.emit(blog::handlers::outbox::Event{"webhook.added", body, id});
        outbox.commit();
        return core::workflow::StepResponse<Output>{body};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::webhooks::create::workflow
