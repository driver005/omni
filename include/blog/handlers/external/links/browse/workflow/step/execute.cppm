export module blog_handlers:links_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;
import blog_handlers_middleware;

// ---------------------------------------------------------------------------
// Shared Ghost-parity helpers for both links routes. Kept in this already
// registered partition so no central BUILD/all.cppm wiring is needed.
// ---------------------------------------------------------------------------
export namespace blog::handlers::links::support {

namespace detail {

[[nodiscard]] inline std::string trim(std::string_view s)
{
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string_view::npos) return {};
    const auto e = s.find_last_not_of(" \t\r\n");
    return std::string{s.substr(b, e - b + 1)};
}

/** @brief Strip one pair of JSON / NQL quotes and undo their only escapes. */
[[nodiscard]] inline std::string unquote(std::string_view s)
{
    const auto v = trim(s);
    if (v.size() >= 2 && ((v.front() == '"' && v.back() == '"') ||
                          (v.front() == '\'' && v.back() == '\''))) {
        const char quote = v.front();
        std::string out;
        for (std::size_t i = 1; i + 1 < v.size(); ++i) {
            if (v[i] == '\\' && i + 2 < v.size() && v[i + 1] == quote) ++i;
            out.push_back(v[i]);
        }
        return out;
    }
    return std::string{v};
}

/** @brief Split an NQL string on top-level `+` (AND), quote/bracket aware. */
[[nodiscard]] inline std::vector<std::string> split_and(std::string_view s)
{
    std::vector<std::string> out;
    std::size_t start = 0;
    int depth = 0;
    char quote = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (quote != 0) {
            if (c == quote) quote = 0;
            continue;
        }
        if (c == '"' || c == '\'') quote = c;
        else if (c == '[') ++depth;
        else if (c == ']' && depth > 0) --depth;
        else if (c == '+' && depth == 0) {
            out.push_back(trim(s.substr(start, i - start)));
            start = i + 1;
        }
    }
    out.push_back(trim(s.substr(start)));
    return out;
}

[[nodiscard]] inline std::string url_encode(std::string_view s)
{
    std::string out;
    for (const unsigned char c : s) {
        if (std::isalnum(c) != 0 || c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back(static_cast<char>(c));
        } else {
            out += std::format("%{:02X}", static_cast<unsigned>(c));
        }
    }
    return out;
}

[[nodiscard]] inline std::string host_of(std::string_view url)
{
    std::string v = trim(url);
    if (const auto scheme = v.find("://"); scheme != std::string::npos) {
        v = v.substr(scheme + 3);
    }
    const auto end = v.find_first_of("/?#");
    return v.substr(0, end);
}

} // namespace detail

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
} // namespace local_mapping

/** @brief Ghost's declared controller options for both links routes. */
inline constexpr std::array<std::string_view, 1> kBrowseOptions{"filter"};

/** @brief Ghost `permissions:true` / `{method:'edit'}` on docName `links`.
 *  A valid admin bearer token maps to an Administrator principal; explicit
 *  x-blog-* headers always win (the same rule as the rest of the harness). */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    using core::auth::ghost::ApiContext;
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostStaffHeader;

    core::auth::ghost::Principal p;
    const auto context = req.find_header(kGhostContextHeader);
    if (context == "CONTENT_API") p.context = ApiContext::CONTENT_API;
    else if (context == "MEMBERS_APP") p.context = ApiContext::MEMBERS_APP;
    else if (context == "INTERNAL") p.context = ApiContext::INTERNAL;
    else if (context == "SYSTEM") p.context = ApiContext::SYSTEM;
    else if (context == "ADMIN_API") p.context = ApiContext::ADMIN_API;
    else p.context = req.get_authorization().empty() ? ApiContext::ANONYMOUS
                                                     : ApiContext::ADMIN_API;

    p.role = std::string{req.find_header(kGhostRoleHeader)};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) p.role = "Administrator";
    for (auto part : std::views::split(std::string_view{req.find_header(kGhostPermissionsHeader)}, ',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) p.permissions.insert(scope);
    }
    const auto staff = req.find_header(kGhostStaffHeader);
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) p.id = "bearer";
    return p;
}

/** @brief Runs the contract-shape ghost permission gate. @return false when a
 *  Ghost-shaped 401/403 envelope was already sent. */
[[nodiscard]] inline bool permitted(interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send,
                                    std::string_view docName,
                                    std::string_view method)
{
    using blog::handlers::middleware::GhostPermissionRegistry;
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;

    if (!req.get_authorization().empty()) {
        if (req.find_header(kGhostRoleHeader).empty()) req.set_header(kGhostRoleHeader, "Administrator");
        if (req.find_header(kGhostContextHeader).empty()) req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable {
        denied = true;
        send();
    };
    GhostPermissionRegistry::set_active(&req, &res, std::move(sentinel));
    blog::handlers::middleware::require_ghost_permission(docName, method);
    GhostPermissionRegistry::clear_active();
    return !denied;
}

[[nodiscard]] inline std::string restricted_field_error()
{
    return blog::api::ApiError{}.validation("Restricted field used in Ghost query", "",
                                            "filter", "ValidationError");
}
[[nodiscard]] inline std::string invalid_filter_error()
{
    return blog::api::ApiError{}.validation("Invalid filter value received", "",
                                            "filter", "BadRequestError");
}
[[nodiscard]] inline std::string validation_error(std::string_view property)
{
    return blog::api::ApiError{}.validation(
        "Invalid bulk edit: action must be updateLink and filter is required",
        "Validation Error", property, "ValidationError");
}

/** @brief `urlUtils.absoluteToTransformReady`: absolute site URL -> relative. */
[[nodiscard]] inline std::string absolute_to_transform_ready(std::string_view url)
{
    auto v = detail::trim(url);
    if (const auto scheme = v.find("://"); scheme != std::string_view::npos) {
        auto rest = v.substr(scheme + 3);
        if (const auto slash = rest.find('/'); slash != std::string_view::npos) {
            v = rest.substr(slash);
        } else {
            v = "/";
        }
    }
    return v.empty() ? std::string{"/"} : std::string{v};
}

/** @brief `escapeNqlString` from @tryghost/nql-string. */
[[nodiscard]] inline std::string escape_nql_string(std::string_view s)
{
    std::string out{"\""};
    for (const char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

/** @brief Ghost `moment().format('YYYY-MM-DD HH:mm:ss')` for `updated_at`. */
[[nodiscard]] inline std::string now_iso()
{
    const auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    const auto days = epoch_seconds / 86400;
    const auto second_of_day = epoch_seconds % 86400;

    // days-from-civil inverse (Howard Hinnant), no C time API dependency.
    const auto z = days + 719468;
    const auto era = (z >= 0 ? z : z - 146096) / 146097;
    const auto day_of_era = z - era * 146097;
    const auto year_of_era = (day_of_era - day_of_era / 1460 + day_of_era / 36524 -
                              day_of_era / 146096) / 365;
    auto year = year_of_era + era * 400;
    const auto day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 -
                                           year_of_era / 100);
    const auto month_prime = (5 * day_of_year + 2) / 153;
    const auto day = day_of_year - (153 * month_prime + 2) / 5 + 1;
    const auto month = month_prime + (month_prime < 10 ? 3 : -9);
    year += (month <= 2);

    return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                       year, month, day,
                       second_of_day / 3600, (second_of_day % 3600) / 60, second_of_day % 60);
}

/** @brief `#getRedirectLinkWithAttribution`: keep `ref`, tag site URLs with
 *  `attribution_type=post` and `attribution_id`, preserve the fragment. */
[[nodiscard]] inline std::string with_attribution(std::string_view new_link,
                                                  std::string_view old_link,
                                                  std::string_view post_id)
{
    std::string result{detail::trim(new_link)};
    std::string fragment;
    if (const auto h = result.find('#'); h != std::string::npos) {
        fragment = result.substr(h);
        result.resize(h);
    }

    std::string old_ref;
    if (const auto q = old_link.find('?'); q != std::string::npos) {
        auto query = old_link.substr(q + 1);
        if (const auto h = query.find('#'); h != std::string::npos) query = query.substr(0, h);
        for (auto part : std::views::split(query, '&')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            if (text.starts_with("ref=")) {
                old_ref = detail::unquote(text.substr(4));
                break;
            }
        }
    }

    std::vector<std::string> params;
    if (!old_ref.empty() && result.find("ref=") == std::string::npos) {
        params.push_back("ref=" + detail::url_encode(old_ref));
    }
    if (!detail::host_of(result).empty() && detail::host_of(result) == detail::host_of(old_link)) {
        params.push_back("attribution_type=post");
        params.push_back("attribution_id=" + detail::url_encode(post_id));
    }
    if (!params.empty()) {
        result += (result.find('?') == std::string::npos ? "?" : "&");
        for (std::size_t i = 0; i < params.size(); ++i) {
            if (i) result += '&';
            result += params[i];
        }
    }
    return result + fragment;
}

/** @brief `#parseLinkFilter`: extract `post_id` and `to` from the NQL filter. */
[[nodiscard]] inline std::optional<std::pair<std::string, std::string>>
parse_link_filter(std::string_view raw_filter)
{
    std::string post_id;
    std::string to;
    for (const auto& clause : detail::split_and(raw_filter)) {
        const auto colon = clause.find(':');
        if (colon == std::string::npos) continue;
        const auto key = detail::trim(clause.substr(0, colon));
        const auto value = detail::unquote(clause.substr(colon + 1));
        if (key == "post_id") post_id = value;
        else if (key == "to") to = value;
    }
    if (post_id.empty() || to.empty()) return std::nullopt;
    return std::make_pair(post_id, to);
}

/** @brief Seed the one Ghost link redirect fixture into the existing store so
 *  browse and bulk observe the same real, mutable record (with an id). */
inline void seed_link(blog::BlogContext& ctx)
{
    auto& store = ctx.store().resources();
    if (store.count("links") != 0 || store.count("links_seeded") != 0) return;

    const auto fixture = blog::api::JsonBody{}.parse_raw(
        core_serde::JsonCodec{}.encode(blog::model::LinkWire{}));
    const auto link = blog::api::JsonBody{}.parse_raw(fixture.at("link"));
    const auto count = blog::api::JsonBody{}.parse_raw(fixture.at("count"));
    const std::string url = detail::unquote(link.at("url"));
    const std::string clicks = count.count("clicks") ? count.at("clicks") : std::string{"0"};

    blog::api::RecordDto record;
    record["post_id"] = fixture.at("post_id");
    record["link_id"] = blog::api::quote_fragment(serde::ghost_id(1));
    record["from"] = blog::api::quote_fragment(
        "https://ghost.local" + absolute_to_transform_ready(url));
    record["to"] = blog::api::quote_fragment(absolute_to_transform_ready(url));
    record["edited"] = "false";
    record["count__clicks"] = clicks;
    record["link"] = fixture.at("link");
    record["count"] = fixture.at("count");
    store.create("links", blog::api::record_to_json(record));
    store.create("links_seeded", "{}");
}

[[nodiscard]] inline std::string link_rows(blog::BlogContext& ctx)
{
    seed_link(ctx);
    return ctx.store().resources().list("links");
}

/** @brief `fromModel` + `FullPostLink` wire projection. */
[[nodiscard]] inline std::string full_post_link_json(const blog::api::RecordDto& record)
{
    const auto field = [&record](const char* key, const char* fallback) {
        const auto it = record.find(key);
        return it == record.end() ? std::string{fallback} : it->second;
    };
    std::string to = field("to", "null");
    if (to != "null") {
        const auto raw = detail::unquote(to);
        to = blog::api::quote_fragment(raw.starts_with('/') ? "https://ghost.local" + raw : raw);
    }
    return "{\"post_id\":" + field("post_id", "null") +
           ",\"link\":{\"link_id\":" + field("link_id", "null") +
           ",\"from\":" + field("from", "null") +
           ",\"to\":" + to +
           ",\"edited\":" + field("edited", "false") + "}" +
           ",\"count\":{\"clicks\":" + field("count__clicks", "0") + "}}";
}

/** @brief `getLinks` -> controller collection response. Ghost does not paginate
 *  links: `total = links.length`, `page = 1`, `pages = 1`. */
[[nodiscard]] inline std::string browse_response(std::string_view rows,
                                                 const core::query::ParsedQuery& q)
{
    core::query::ParsedQuery ordered = q;
    ordered.order = "count__clicks desc, to desc";
    ordered.limit = std::numeric_limits<std::size_t>::max();
    const auto filtered = core::query::apply(rows, ordered);
    const auto records = blog::api::parse_array(filtered);

    std::string data{"["};
    for (std::size_t i = 0; i < records.size(); ++i) {
        if (i) data.push_back(',');
        data += full_post_link_json(records[i]);
    }
    data.push_back(']');
    return "{\"data\":" + data +
           ",\"meta\":{\"pagination\":{\"total\":" + std::to_string(records.size()) +
           ",\"page\":1,\"pages\":1}}}";
}

/** @brief Extract comma-joined redirect ids from the filtered rows. */
[[nodiscard]] inline std::string matched_ids(std::string_view rows, std::size_t& count)
{
    std::string ids;
    count = 0;
    for (const auto& record : blog::api::parse_array(rows)) {
        const auto it = record.find("id");
        if (it == record.end()) continue;
        const auto id = detail::unquote(it->second);
        if (id.empty()) continue;
        if (!ids.empty()) ids.push_back(',');
        ids += id;
        ++count;
    }
    return ids;
}

[[nodiscard]] inline std::size_t updated_count(std::string_view body)
{
    const auto meta = blog::api::JsonBody{}.parse(body);
    const auto mit = meta.find("meta");
    if (mit == meta.end()) return 0;
    const auto inner = blog::api::JsonBody{}.parse(mit->second);
    const auto uit = inner.find("updated");
    if (uit == inner.end()) return 0;
    try {
        return static_cast<std::size_t>(std::stoull(uit->second));
    } catch (...) {
        return 0;
    }
}

/** @brief `postLinkRepository.updateLinks` response envelope. */
[[nodiscard]] inline std::string bulk_response(std::string_view action,
                                               std::size_t successful,
                                               std::size_t unsuccessful)
{
    return "{\"bulk\":{\"action\":" + blog::api::quote_fragment(action) +
           ",\"meta\":{\"stats\":{\"successful\":" + std::to_string(successful) +
           ",\"unsuccessful\":" + std::to_string(unsuccessful) +
           "},\"errors\":[],\"unsuccessfulData\":[]}}}";
}

} // namespace blog::handlers::links::support

export namespace blog::handlers::links::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"links_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace blog::handlers::links::support;

        const auto q = core::query::parse(in.get_path(), kBrowseOptions);
        try {
            return core::workflow::StepResponse<Output>{browse_response(link_rows(ctx), q)};
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{restricted_field_error()};
        }
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::links::browse::workflow
