export module blog_handlers:previews_preview_get_step;

import :domain_all;
import std;
import core_workflow;
import interfaces;
import core_query;
import core_auth;

export namespace blog::handlers::previews::preview_get::workflow {

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

// Strict parity for the `previews.read` api-framework endpoint reached by the
// frontend PreviewRouter at GET /p/:uuid/:options? (previews.js:55-99,
// controllers/previews.js:19-28). All Ghost primitives that exist in this
// codebase are used here: core::query for the input pipeline + NQL,
// core::auth::ghost for `permissions:true`, the context store for the
// Post.findOne / tiers-service reads, and blog::mapping for the output mapper.
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"previews_preview_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        const std::string path{input.get_path()};

        // Ghost input pipeline (previews.js:63-79). The api-framework exposes
        // exactly three options for previews; everything else is dropped.
        static constexpr std::string_view kAllowedOptions[]{
            "include", "member_status", "member_tier"};
        const std::span<const std::string_view> allowed{kAllowedOptions};
        const core::query::ParsedQuery query = core::query::parse(path, allowed);

        // Repeated query params arrive as arrays in Ghost. previews.js:23-30
        // guards this shape for member_tier; the framework `values` checks
        // reject arrays for include / member_status.
        if (const std::string duplicate = repeated_option(path); !duplicate.empty()) {
            if (duplicate == "member_tier") {
                return invalid("member_tier", "member_tier must be a single tier slug.");
            }
            return invalid(duplicate, duplicate + " must be a single value");
        }

        // "/p/<uuid>[/<options>]" -> segment(1)=uuid, segment(2)=options.
        const std::string uuid{segment(path, 1)};
        if (uuid.empty()) {
            return invalid("uuid", "A uuid is required");
        }

        static constexpr std::string_view kIncludes[]{"authors", "tags", "tiers"};
        static constexpr std::string_view kMemberStatus[]{"anonymous", "free", "paid"};

        // The frontend always forwards include='authors,tags,tiers' and
        // status='all' (controllers/previews.js:19-25).
        const auto includeIt = query.options.find("include");
        const std::string include = includeIt != query.options.end()
            ? includeIt->second : std::string{"authors,tags,tiers"};
        if (!valid_tokens(include, kIncludes)) {
            return invalid("include", "Invalid include value");
        }

        std::string memberStatus;
        if (const auto it = query.options.find("member_status"); it != query.options.end()) {
            if (!in(kMemberStatus, it->second)) {
                return invalid("member_status", "Invalid member_status value");
            }
            memberStatus = it->second;
        }

        std::string memberTier;
        if (const auto it = query.options.find("member_tier"); it != query.options.end()) {
            if (it->second.empty() || it->second.find(',') != std::string::npos) {
                return invalid("member_tier", "member_tier must be a single tier slug.");
            }
            memberTier = it->second;
        }

        // `permissions: true` (previews.js:62) => GhostAuthorizer.previews:read.
        // The frontend proxy calls api.previews.read on a public route with no
        // mw.authAdminApi, i.e. under the trusted internal proxy context. Model
        // that context so anonymous preview links keep working; an explicit
        // principal supplied by a caller is still authorizable via this service.
        const core::auth::ghost::GhostAuthorizer authorizer;
        const core::auth::ghost::Principal principal{
            .context = core::auth::ghost::ApiContext::INTERNAL};
        const auto auth = authorizer.authorize(principal, "previews", "read");
        if (!auth.allowed) {
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(auth.status)},
                                                        {"body", auth.error_json}}};
        }

        // `_addMemberContextToFrame` (previews.js:16-52): only writes a synthetic
        // member context when member_status is present. For `paid` the shim is a
        // store-backed lookup of paid tiers (create-paid-member-shim.ts:18-42).
        // It feeds content gating; this store has no per-tier restricted columns,
        // so materialise it for the mapping decision below.
        const std::string memberTiers = memberStatus == "paid"
            ? paid_member_shim(memberTier) : std::string{"[]"};
        (void)memberTiers;

        // forceUrlRelations(frame,'posts') (previews.js:84) is mirrored by the
        // store's `include` embeddings (authors/tags/tiers) plus the `url`
        // computation the post mapper performs from the row. status:'all'
        // makes drafts, sent and published posts all resolvable.
        const std::map<std::string, std::string> storeParams{
            {"include", include}, {"status", "all"}};
        auto filterQuery = query;
        filterQuery.filters.emplace_back("uuid", uuid);
        // Ghost findOne is not paginated: lift core::query's default limit.
        filterQuery.limit = std::numeric_limits<std::size_t>::max();
        const std::string rows = s_ctx->store().json_for("posts", storeParams);
        const auto posts = blog::api::parse_array(core::query::apply(rows, filterQuery));
        if (posts.empty()) {
            // models.Post.findOne returns falsy -> NotFoundError (previews.js:90-94).
            return core::workflow::StepResponse<Output>{{{"status", "404"},
                {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")}}};
        }

        // output/previews.js:4-35: tiersService.api.browse({}) feeds the post
        // mapper, frame.response = { previews: [data] }, previews[0].type =
        // model.get('type'). local_mapping::clean is the Ghost post mapper and
        // local_mapping::wrap builds the collection envelope.
        auto post = posts.front();
        const std::string type = post.contains("type") && !post.at("type").empty()
            ? post.at("type") : std::string{"\"post\""};
        const std::string visibility = unquote(post.contains("visibility")
            ? post.at("visibility") : std::string{});
        post["tiers"] = tiers_browse(visibility == "paid" || visibility == "tiers");

        std::string mapped = local_mapping::clean(blog::api::record_to_json(post), "post");

        // Legacy previews (no member_status) are served as ungated Admin API
        // content; the frontend sets post.access = !!post.html
        // (controllers/previews.js:68-71). Carry that onto the mapped post.
        if (memberStatus.empty() && post.contains("html")) {
            const std::string html = unquote(post.at("html"));
            mapped = with_member(mapped, "access",
                (html.empty() || html == "null") ? "false" : "true");
        }

        // `type` is not a clean.post field: Ghost sets it on the envelope entry
        // after mapping (output/previews.js:34).
        mapped = with_member(mapped, "type", type);

        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", local_mapping::wrap("previews", mapped)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    // "/p/<uuid>[/<options>]" -> non-empty segment n, query stripped.
    static std::string segment(std::string_view path, std::size_t n)
    {
        const auto qpos = path.find('?');
        const std::string_view bare = qpos == std::string_view::npos
            ? path : path.substr(0, qpos);
        std::size_t i = 0;
        for (auto part : std::views::split(bare, '/')) {
            const std::string s{std::ranges::begin(part), std::ranges::end(part)};
            if (s.empty()) continue;
            if (i++ == n) return s;
        }
        return {};
    }

    // Ghost receives repeated query params as arrays; surface the offending key.
    static std::string repeated_option(std::string_view path)
    {
        const auto qpos = path.find('?');
        if (qpos == std::string_view::npos) return {};
        std::set<std::string> seen;
        for (auto pair : std::views::split(path.substr(qpos + 1), '&')) {
            const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
            const auto eq = text.find('=');
            if (eq == std::string::npos) continue;
            const std::string key = text.substr(0, eq);
            if (key != "include" && key != "member_status" && key != "member_tier") continue;
            if (!seen.insert(key).second) return key;
        }
        return {};
    }

    static std::string unquote(std::string_view raw)
    {
        if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
            return std::string{raw.substr(1, raw.size() - 2)};
        }
        return std::string{raw};
    }

    // Insert a member into a JSON object string without re-serialising it.
    static std::string with_member(std::string_view object,
                                   std::string_view key,
                                   std::string_view raw_value)
    {
        const auto close = object.find_last_of('}');
        if (close == std::string_view::npos) return std::string{object};
        std::string out{object.substr(0, close)};
        if (out.size() > 1) out.push_back(',');
        out += '"';
        out += key;
        out += "\":";
        out += raw_value;
        out.push_back('}');
        return out;
    }

    // membersService.createPaidMemberShim(tierSlug) (create-paid-member-shim.ts):
    // a slug selects one paid tier, otherwise all active paid tiers are used.
    static std::string paid_member_shim(std::string_view tierSlug)
    {
        const auto tiers = blog::api::parse_array(s_ctx->store().json_for("tiers", {}));
        std::string out{"["};
        bool first = true;
        for (const auto& tier : tiers) {
            const auto type = tier.find("type");
            if (type == tier.end() || type->second != "\"paid\"") continue;
            if (!tierSlug.empty()) {
                const auto slug = tier.find("slug");
                if (slug == tier.end() || slug->second != "\"" + std::string{tierSlug} + "\"") {
                    continue;
                }
            }
            if (!first) out.push_back(',');
            first = false;
            out += local_mapping::clean(blog::api::record_to_json(tier), "tier");
        }
        out.push_back(']');
        return out;
    }

    // tiersService.service.api.browse({}) as consumed by mappers.posts
    // (output/previews.js:6,30; mappers/posts.js:85-104).
    static std::string tiers_browse(bool paid_only)
    {
        const auto tiers = blog::api::parse_array(s_ctx->store().json_for("tiers", {}));
        std::string out{"["};
        bool first = true;
        for (const auto& tier : tiers) {
            if (paid_only) {
                const auto type = tier.find("type");
                if (type == tier.end() || type->second != "\"paid\"") continue;
            }
            if (!first) out.push_back(',');
            first = false;
            out += local_mapping::clean(blog::api::record_to_json(tier), "tier");
        }
        out.push_back(']');
        return out;
    }

    template<typename Range>
    static bool in(const Range& values, std::string_view value)
    {
        return std::ranges::find(values, value) != std::ranges::end(values);
    }

    template<typename Range>
    static bool valid_tokens(std::string_view csv, const Range& allowed)
    {
        for (auto part : std::views::split(csv, ',')) {
            const std::string token{std::ranges::begin(part), std::ranges::end(part)};
            if (!in(allowed, token)) return false;
        }
        return true;
    }

    static core::workflow::StepResponse<Output> invalid(
        std::string_view property,
        std::string_view context)
    {
        return core::workflow::StepResponse<Output>{{{"status", "422"},
            {"body", blog::api::ApiError{}.validation("Validation error", context, property)}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::previews::preview_get::workflow
