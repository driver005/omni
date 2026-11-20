// posts_support.cppm — strict-Ghost query/serialization/auth helpers for the
// admin posts handlers. Shared partition: every posts step imports this instead
// of re-implementing NQL parsing, restricted-field guards or output mapping.
//
// Shared abstractions used (CONTRACTS.md):
//   core::query                  — NQL/option parsing, filtering, pagination
//   core::auth::ghost            — permission decisions + Ghost error envelopes
//   blog::mapping (clean/wrap)   — post allow-list + response envelopes
//   core::outbox                 — post domain events / side effects
export module blog_handlers:posts_support;

import :domain_all;
import :outbox_shim;

import std;
import interfaces;
import core_auth;
import core_serde;
import blog_handlers_middleware;

// shared local_query/local_outbox now provided by :outbox_shim

export namespace blog::handlers::posts::support {

using Query = local_query::ParsedQuery;

// ---- Ghost option / value sets (posts.js:7-23, posts-public.js:12) ---------

inline constexpr std::array<std::string_view, 10> kAdminBrowseOptions{
    "include", "filter", "fields", "collection", "formats", "limit", "order", "page",
    "debug", "absolute_urls"
};
inline constexpr std::array<std::string_view, 11> kContentBrowseOptions{
    "include", "filter", "fields", "formats", "limit", "order", "page", "debug",
    "absolute_urls", "collection", "skipPagination"
};
inline constexpr std::array<std::string_view, 7> kAdminReadOptions{
    "include", "fields", "formats", "debug", "absolute_urls", "forUpdate", "transacting"
};
inline constexpr std::array<std::string_view, 5> kPublicReadOptions{
    "include", "fields", "formats", "debug", "absolute_urls"
};
inline constexpr std::array<std::string_view, 3> kAddOptions{"include", "formats", "source"};
inline constexpr std::array<std::string_view, 11> kEditOptions{
    "include", "id", "formats", "source", "email_segment", "newsletter",
    "force_rerender", "save_revision", "convert_to_lexical", "forUpdate", "transacting"
};
inline constexpr std::array<std::string_view, 3> kExportOptions{"limit", "filter", "order"};
inline constexpr std::array<std::string_view, 1> kBulkOptions{"filter"};
inline constexpr std::array<std::string_view, 2> kCopyOptions{"id", "formats"};
inline constexpr std::array<std::string_view, 2> kDestroyOptions{"include", "id"};

inline constexpr std::array<std::string_view, 15> kAdminIncludes{
    "tags", "authors", "authors.roles", "email", "tiers", "newsletter",
    "count.conversions", "count.signups", "count.paid_conversions", "count.clicks",
    "sentiment", "count.positive_feedback", "count.negative_feedback", "post_revisions",
    "post_revisions.author"
};
inline constexpr std::array<std::string_view, 4> kContentIncludes{
    "tags", "authors", "tiers", "sentiment"
};
inline constexpr std::array<std::string_view, 4> kFormats{
    "mobiledoc", "lexical", "html", "plaintext"
};

// Fields withheld from every response (ADMIN_API_RESTRICTED_FIELDS, api-filter-utils.ts:78).
inline constexpr std::array<std::string_view, 4> kAdminRestricted{
    "password", "token", "gift_link_token", "secret"
};
// Content API additionally withholds these (api-filter-utils.ts:9-26).
inline constexpr std::array<std::string_view, 14> kContentRestricted{
    "password", "email", "email_only", "html", "lexical", "locale", "mobiledoc",
    "newsletter_id", "plaintext", "email_recipient_filter", "published_by",
    "mobiledoc_revisions", "post_revisions", "show_title_and_feature_image"
};
// Gated content fields stripped without member/gift entitlement.
inline constexpr std::array<std::string_view, 4> kGatedFields{
    "html", "plaintext", "lexical", "mobiledoc"
};

struct ParseResult
{
    Query query;
    std::string error;                         // non-empty => denial/envelope body
    std::string status{"400"};
};

[[nodiscard]] std::string bad_request(std::string_view context);
[[nodiscard]] std::string validation(std::string_view property, std::string_view message);
[[nodiscard]] bool restricted(std::string_view text, bool content = false);

[[nodiscard]] ParseResult parse(std::string_view path,
                                std::span<const std::string_view> allowed,
                                bool bulk = false,
                                bool content = false);

[[nodiscard]] std::optional<std::string> validate_options(const Query& q, bool content = false);

[[nodiscard]] std::string clean_rows(std::string_view rows, std::string_view entity = "post");
[[nodiscard]] std::string query_apply(std::string_view rows, const Query& q);
[[nodiscard]] std::size_t filtered_total(std::string_view rows, const Query& q);
[[nodiscard]] std::string browse_body(blog::BlogContext& ctx, const Query& q, bool content = false);
[[nodiscard]] std::string read_body(std::string_view entity,
                                    const Query& q,
                                    bool content = false);

[[nodiscard]] std::vector<std::string> ids_for_filter(blog::BlogContext& ctx,
                                                      std::string_view filter);
[[nodiscard]] std::optional<serde::Value> decode(std::string_view json);
[[nodiscard]] std::string object_field(const serde::Value::Object& object, std::string_view key);
[[nodiscard]] std::string field_of(std::string_view json, std::string_view key);
[[nodiscard]] std::string first_post_id(std::string_view wrapped_body);
[[nodiscard]] std::string erase_fields(std::string_view json,
                                       std::span<const std::string_view> keys);
[[nodiscard]] std::string gift_access(std::string_view entity,
                                      interfaces::io::IRequest& request,
                                      blog::BlogContext& ctx);
[[nodiscard]] std::string path_tail(std::string_view path);
[[nodiscard]] bool is_content(interfaces::io::IRequest& request);

void emit(std::string_view type, std::string_view payload, std::string_view aggregate_id);

[[nodiscard]] std::optional<std::string> unsafe_attrs_error(interfaces::io::IRequest& request,
                                                            std::string_view payload);

/** @brief Run the Ghost permission guard for one handler.
 *  Returns false after writing a Ghost 401/403 envelope and calling `send`; the
 *  caller must return immediately. Returns true when the request may proceed.
 */
[[nodiscard]] bool authorize(interfaces::io::IRequest& request,
                             interfaces::io::IResponse& response,
                             std::function<void()> send,
                             std::string_view doc,
                             std::string_view method);

} // export namespace blog::handlers::posts::support

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------
namespace blog::handlers::posts::support {

namespace detail {

[[nodiscard]] inline std::string_view trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return text;
}

[[nodiscard]] inline bool in(std::span<const std::string_view> values, std::string_view value)
{
    return std::ranges::find(values, value) != std::ranges::end(values);
}

[[nodiscard]] inline bool field_restricted(std::string_view field, bool content)
{
    return local_query::has_restricted_field(field) ||
           (content && in(kContentRestricted, field));
}

/** @brief Split an NQL string on top-level separators, keeping separators for rebuild. */
struct Segment
{
    std::string text;
    char separator{0}; // separator that preceded this segment
};

[[nodiscard]] inline std::vector<Segment> split_statements(std::string_view nql)
{
    std::vector<Segment> out;
    std::string current;
    char quote = 0;
    int depth = 0;
    char pending = 0;
    for (const char c : nql) {
        if (quote != 0) {
            current += c;
            if (c == quote) quote = 0;
            continue;
        }
        if (c == '\'' || c == '"') {
            quote = c;
            current += c;
        } else if (c == '[') {
            ++depth;
            current += c;
        } else if (c == ']') {
            if (depth > 0) --depth;
            current += c;
        } else if (depth == 0 && (c == ',' || c == '+')) {
            out.push_back({std::move(current), pending});
            current.clear();
            pending = c;
        } else {
            current += c;
        }
    }
    out.push_back({std::move(current), pending});
    return out;
}

/** @brief Drop NQL statements whose field is a restricted column (mongoTransformer). */
[[nodiscard]] inline std::string strip_restricted(std::string_view nql,
                                                  bool content,
                                                  bool& found)
{
    std::string out;
    for (auto& segment : split_statements(nql)) {
        const auto colon = segment.text.find(':');
        std::string field = colon == std::string::npos
                                ? segment.text
                                : segment.text.substr(0, colon);
        for (char& c : field) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (field_restricted(field, content)) {
            found = true;
            continue;
        }
        if (!out.empty()) out += segment.separator;
        out += segment.text;
    }
    return out;
}

} // namespace detail

std::string bad_request(std::string_view context)
{
    return blog::api::ApiError{}.validation("Bad Request", context, "", "BadRequestError");
}

std::string validation(std::string_view property, std::string_view message)
{
    return blog::api::ApiError{}.validation("Validation error", message, property);
}

bool restricted(std::string_view text, bool content)
{
    if (local_query::has_restricted_field(text)) return true;
    if (!content) return false;

    // Content API withholds a wider field set in both filter and order.
    std::size_t start = 0;
    while (start < text.size()) {
        const unsigned char c = static_cast<unsigned char>(text[start]);
        if (!(std::isalnum(c) || text[start] == '_')) {
            ++start;
            continue;
        }
        const std::size_t begin = start;
        while (start < text.size()) {
            const unsigned char d = static_cast<unsigned char>(text[start]);
            if (!(std::isalnum(d) || text[start] == '_' || text[start] == '.')) break;
            ++start;
        }
        std::string token{text.substr(begin, start - begin)};
        std::ranges::transform(token, token.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        const auto dot = token.find_last_of('.');
        const std::string_view segment = dot == std::string::npos
                                             ? std::string_view{token}
                                             : std::string_view{token}.substr(dot + 1);
        if (detail::in(kContentRestricted, segment)) return true;
    }
    return false;
}

ParseResult parse(std::string_view path,
                  std::span<const std::string_view> allowed,
                  bool bulk,
                  bool content)
{
    ParseResult result;
    result.query = local_query::parse(path, allowed);
    auto& q = result.query;

    // Order is compiled to SQL ORDER BY: a restricted column is an oracle -> 400.
    if (restricted(q.order, content)) {
        result.error = bad_request("Restricted fields cannot be used in order.");
        return result;
    }

    const bool original_filter_blocked = restricted(q.raw_filter, content) ||
        std::ranges::any_of(q.filters, [&](const auto& pair) {
            return detail::field_restricted(pair.first, content);
        });

    bool found = false;
    q.raw_filter = detail::strip_restricted(q.raw_filter, content, found);
    std::erase_if(q.filters, [&](const auto& pair) {
        return detail::field_restricted(pair.first, content);
    });
    std::erase_if(q.fields, [&](const std::string& name) {
        return detail::field_restricted(name, content);
    });

    // Bulk filters use a stricter transformer that rejects rather than strips.
    if (bulk && (original_filter_blocked || found)) {
        result.error = bad_request("Restricted fields cannot be used in bulk operation filters.");
    }
    return result;
}

std::optional<std::string> validate_options(const Query& q, bool content)
{
    const auto& includes = content ? std::span<const std::string_view>{kContentIncludes}
                                   : std::span<const std::string_view>{kAdminIncludes};
    if (const auto it = q.options.find("include"); it != q.options.end() &&
        !detail::in(includes, it->second)) {
        // Compound include lists are comma separated; validate each token.
        for (auto part : std::string_view{it->second} | std::views::split(',')) {
            const std::string token{std::ranges::begin(part), std::ranges::end(part)};
            if (!detail::in(includes, detail::trim(token))) {
                return validation("include", "Invalid include value");
            }
        }
    }
    if (const auto it = q.options.find("formats"); it != q.options.end() &&
        !detail::in(kFormats, it->second)) {
        for (auto part : std::string_view{it->second} | std::views::split(',')) {
            const std::string token{std::ranges::begin(part), std::ranges::end(part)};
            if (!detail::in(kFormats, detail::trim(token))) {
                return validation("formats", "Invalid formats value");
            }
        }
    }
    if (const auto it = q.options.find("source"); it != q.options.end() && it->second != "html") {
        return validation("source", "Invalid source value");
    }
    return std::nullopt;
}

std::string clean_rows(std::string_view rows, std::string_view entity)
{
    const auto parsed = decode(rows);
    if (!parsed || !std::holds_alternative<serde::Value::Array>(parsed->get())) return "[]";
    std::string out = "[";
    bool first = true;
    for (const auto& row : std::get<serde::Value::Array>(parsed->get())) {
        if (!first) out += ',';
        out += local_mapping::clean(core_serde::JsonCodec{}.encode(row), entity);
        first = false;
    }
    return out + "]";
}

std::string query_apply(std::string_view rows, const Query& q)
{
    try {
        return local_query::apply(rows, q);
    } catch (...) {
        return "[]";
    }
}

std::size_t filtered_total(std::string_view rows, const Query& q)
{
    Query count = q;
    count.limit = std::numeric_limits<std::size_t>::max();
    count.page = 1;
    count.offset = 0;
    count.fields.clear();
    const auto applied = query_apply(rows, count);
    const auto parsed = decode(applied);
    if (!parsed || !std::holds_alternative<serde::Value::Array>(parsed->get())) return 0;
    return std::get<serde::Value::Array>(parsed->get()).size();
}

std::string browse_body(blog::BlogContext& ctx, const Query& q, bool content)
{
    std::map<std::string, std::string> store_params;
    if (const auto it = q.options.find("include"); it != q.options.end()) {
        store_params["include"] = it->second;
    }
    const auto all = ctx.store().json_for("posts", store_params);

    const bool skip = content && q.options.contains("skipPagination");
    if (skip) {
        Query unbounded = q;
        unbounded.limit = std::numeric_limits<std::size_t>::max();
        unbounded.page = 1;
        unbounded.offset = 0;
        return local_mapping::wrap("posts", clean_rows(query_apply(all, unbounded)));
    }

    const auto data = clean_rows(query_apply(all, q));
    const auto total = filtered_total(all, q);
    const auto meta = "{\"pagination\":" + local_query::pagination_meta(total, q) + "}";
    return local_mapping::wrap("posts", data, meta);
}

std::string read_body(std::string_view entity, const Query& q, bool content)
{
    (void)content;
    Query projection;
    projection.fields = q.fields;
    const auto one = "[" + std::string{entity} + "]";
    return local_mapping::wrap("posts", clean_rows(query_apply(one, projection)));
}

std::vector<std::string> ids_for_filter(blog::BlogContext& ctx, std::string_view filter)
{
    std::vector<std::string> ids;
    Query q;
    q.raw_filter = filter;
    q.limit = std::numeric_limits<std::size_t>::max();
    q.page = 1;
    const auto rows = ctx.store().json_for("posts", {});
    const auto applied = query_apply(rows, q);
    const auto parsed = decode(applied);
    if (!parsed || !std::holds_alternative<serde::Value::Array>(parsed->get())) return ids;
    for (const auto& row : std::get<serde::Value::Array>(parsed->get())) {
        if (!std::holds_alternative<serde::Value::Object>(row.get())) continue;
        const auto id = object_field(std::get<serde::Value::Object>(row.get()), "id");
        if (!id.empty()) ids.push_back(id);
    }
    return ids;
}

std::optional<serde::Value> decode(std::string_view json)
{
    return core_serde::JsonCodec{}.decode<serde::Value>(json);
}

std::string object_field(const serde::Value::Object& object, std::string_view key)
{
    for (const auto& [name, value] : object) {
        if (name != key) continue;
        if (const auto* text = std::get_if<std::string>(&value.get())) return *text;
        return core_serde::JsonCodec{}.encode(value);
    }
    return {};
}

std::string field_of(std::string_view json, std::string_view key)
{
    const auto parsed = decode(json);
    if (!parsed || !std::holds_alternative<serde::Value::Object>(parsed->get())) return {};
    return object_field(std::get<serde::Value::Object>(parsed->get()), key);
}

std::string first_post_id(std::string_view wrapped_body)
{
    const auto parsed = decode(wrapped_body);
    if (!parsed || !std::holds_alternative<serde::Value::Object>(parsed->get())) return {};
    const auto& root = std::get<serde::Value::Object>(parsed->get());
    for (const auto& [name, value] : root) {
        if (name != "posts" || !std::holds_alternative<serde::Value::Array>(value.get())) continue;
        for (const auto& row : std::get<serde::Value::Array>(value.get())) {
            if (!std::holds_alternative<serde::Value::Object>(row.get())) continue;
            const auto id = object_field(std::get<serde::Value::Object>(row.get()), "id");
            if (!id.empty()) return id;
        }
    }
    return {};
}

std::string erase_fields(std::string_view json, std::span<const std::string_view> keys)
{
    const auto parsed = decode(json);
    if (!parsed || !std::holds_alternative<serde::Value::Object>(parsed->get())) {
        return std::string{json};
    }
    const auto& object = std::get<serde::Value::Object>(parsed->get());
    serde::Value::Object filtered;
    for (const auto& [key, value] : object) {
        if (std::ranges::find(keys, std::string_view{key}) == std::ranges::end(keys)) {
            filtered.insert(key, value);
        }
    }
    return core_serde::JsonCodec{}.encode(filtered);
}

std::string gift_access(std::string_view entity,
                        interfaces::io::IRequest& request,
                        blog::BlogContext& ctx)
{
    Query q = local_query::parse(request.get_path(), std::array<std::string_view, 1>{"gift"});
    const auto gift_it = q.options.find("gift");
    const std::string token = gift_it == q.options.end() ? std::string{} : gift_it->second;
    const std::string post_id = field_of(entity, "id");
    const std::string visibility = field_of(entity, "visibility");

    bool entitled = false;
    if (!token.empty()) {
        const auto links = ctx.store().resources().list("post_gift_links");
        const auto parsed = decode(links);
        if (parsed && std::holds_alternative<serde::Value::Array>(parsed->get())) {
            for (const auto& record : std::get<serde::Value::Array>(parsed->get())) {
                if (!std::holds_alternative<serde::Value::Object>(record.get())) continue;
                const auto& object = std::get<serde::Value::Object>(record.get());
                if (object_field(object, "gift_link_token") == token &&
                    object_field(object, "post_id") == post_id) {
                    entitled = true;
                    break;
                }
            }
        }
    }

    // Public posts are readable; gated posts need a matching gift link.
    if (entitled || (visibility != "paid" && visibility != "members")) return std::string{entity};
    return erase_fields(entity, kGatedFields);
}

std::string path_tail(std::string_view path)
{
    auto bare = path.substr(0, path.find('?'));
    while (bare.size() > 1 && bare.back() == '/') bare.remove_suffix(1);
    const auto slash = bare.find_last_of('/');
    return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
}

bool is_content(interfaces::io::IRequest& request)
{
    return request.find_header(blog::handlers::middleware::kGhostContextHeader) == "CONTENT_API";
}

void emit(std::string_view type, std::string_view payload, std::string_view aggregate_id)
{
    local_outbox::global_outbox().emit(local_outbox::Event{
        .type = std::string{type},
        .payload = std::string{payload},
        .aggregate_id = std::string{aggregate_id}
    });
}

std::optional<std::string> unsafe_attrs_error(interfaces::io::IRequest& request,
                                              std::string_view payload)
{
    const auto principal = blog::handlers::middleware::principal_from_request(request);
    static const core::auth::ghost::GhostAuthorizer authorizer;
    constexpr std::array<std::string_view, 3> attrs{"status", "authors", "visibility"};
    for (const auto attr : attrs) {
        if (payload.find("\"" + std::string{attr} + "\"") == std::string_view::npos) continue;
        const auto result = authorizer.unsafe_attrs(principal, attr, "");
        if (!result.allowed) return result.error_json;
    }
    return std::nullopt;
}

bool authorize(interfaces::io::IRequest& request,
               interfaces::io::IResponse& response,
               std::function<void()> send,
               std::string_view doc,
               std::string_view method)
{
    using blog::handlers::middleware::GhostPermissionRegistry;
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::principal_from_request;
    using blog::handlers::middleware::write_ghost_denial;

    // A bearer token on the admin API is an authenticated Administrator unless the
    // caller supplied explicit role/permission headers (integration/staff tokens).
    if (request.find_header(kGhostRoleHeader).empty() && !request.get_authorization().empty() &&
        request.find_header(kGhostContextHeader) != "CONTENT_API") {
        request.set_header(kGhostContextHeader, "ADMIN_API");
        request.set_header(kGhostRoleHeader, "Administrator");
    }

    const auto result = GhostPermissionRegistry::authorizer().authorize(
        principal_from_request(request), doc, method);
    if (!result.allowed) {
        write_ghost_denial(response, result, doc, method, std::move(send));
        return false;
    }

    // Contract-shape guard (CONTRACTS.md §3): same decision, no-op when allowed.
    GhostPermissionRegistry::set_active(&request, &response, send);
    blog::handlers::middleware::require_ghost_permission(doc, method);
    GhostPermissionRegistry::clear_active();
    return true;
}

} // namespace blog::handlers::posts::support
