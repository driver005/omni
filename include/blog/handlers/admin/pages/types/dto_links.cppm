export module blog_handlers:pages_dto_links;

import std;
import core_serde;
import core_query;
import core_auth;
import :outbox_shim;
import interfaces;

export namespace blog::dto::pages {

class LinkRecord
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_url(std::string value)
    {
        url = std::move(value);
    }

    [[nodiscard]] const std::string& get_url() const noexcept
    {
        return url;
    }

    std::string id{"1"};
    std::string url{"https://ghost.local/link"};
};

class GiftLinksResponse
{
public:
    void set_gift_links(std::vector<LinkRecord> value)
    {
        gift_links = std::move(value);
    }

    [[nodiscard]] const std::vector<LinkRecord>& get_gift_links() const noexcept
    {
        return gift_links;
    }

    std::vector<LinkRecord> gift_links{};
};

class AutomationLinksResponse
{
public:
    void set_links(std::vector<LinkRecord> value)
    {
        links = std::move(value);
    }

    [[nodiscard]] const std::vector<LinkRecord>& get_links() const noexcept
    {
        return links;
    }

    std::vector<LinkRecord> links{};
};

} // namespace blog::dto::pages

template<>
struct serde::Serializable<blog::dto::pages::LinkRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::pages::LinkRecord::get_id,
                &blog::dto::pages::LinkRecord::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "url",
                &blog::dto::pages::LinkRecord::get_url,
                &blog::dto::pages::LinkRecord::set_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_record";
    }
};

template<>
struct serde::Serializable<blog::dto::pages::GiftLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "gift_links",
            &blog::dto::pages::GiftLinksResponse::get_gift_links,
            &blog::dto::pages::GiftLinksResponse::set_gift_links,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gift_links";
    }
};

template<>
struct serde::Serializable<blog::dto::pages::AutomationLinksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "links",
            &blog::dto::pages::AutomationLinksResponse::get_links,
            &blog::dto::pages::AutomationLinksResponse::set_links,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "links";
    }
};

// ---------------------------------------------------------------------------
// Shared Ghost-parity helpers for the pages handlers/steps (manifest group 4).
// Kept in this already-registered partition so no central BUILD/all.cppm wiring
// is required. All Ghost inputs/options are funnelled through core::query, all
// permissions through core::auth::ghost, and all events through core::outbox.
// ---------------------------------------------------------------------------
export namespace blog::handlers::pages::support {

using core::auth::ghost::ApiContext;

// Ghost's declared `options[]` for the pages controller.
inline constexpr std::array<std::string_view, 9> kBrowseOptions{
    "include",
    "filter",
    "fields",
    "formats",
    "limit",
    "order",
    "page",
    "debug",
    "absolute_urls"
};
inline constexpr std::array<std::string_view, 7> kReadOptions{
    "include",
    "fields",
    "formats",
    "debug",
    "absolute_urls",
    "forUpdate",
    "transacting"
};
inline constexpr std::array<std::string_view, 3> kAddOptions{"include", "formats", "source"};
inline constexpr std::array<std::string_view, 9> kEditOptions{
    "include",
    "id",
    "formats",
    "source",
    "force_rerender",
    "save_revision",
    "convert_to_lexical",
    "forUpdate",
    "transacting"
};
inline constexpr std::array<std::string_view, 2> kCopyOptions{"id", "formats"};

// Allowed relation/format values (pages.js:6-15,48 / pages-public.js:9,41).
inline constexpr std::array<std::string_view, 8> kAdminIncludes{
    "tags",
    "authors",
    "authors.roles",
    "tiers",
    "count.signups",
    "count.paid_conversions",
    "post_revisions",
    "post_revisions.author"
};
inline constexpr std::array<std::string_view, 3> kPublicIncludes{"tags", "authors", "tiers"};
inline constexpr std::array<std::string_view, 4>
    kFormats{"mobiledoc", "lexical", "html", "plaintext"};

[[nodiscard]] inline bool in_list(std::string_view value, std::span<const std::string_view> list)
{
    return std::ranges::find(list, value) != list.end();
}

/** @brief Validate a comma-separated token option (include/formats/source). */
[[nodiscard]] inline bool
tokens_ok(std::string_view value, std::span<const std::string_view> allowed)
{
    if (value.empty()) {
        return true;
    }
    std::size_t i = 0;
    while (i <= value.size()) {
        const auto comma = value.find(',', i);
        const std::string_view token =
            value.substr(i, comma == std::string_view::npos ? std::string_view::npos : comma - i);
        if (!token.empty() && !in_list(token, allowed)) {
            return false;
        }
        if (comma == std::string_view::npos) {
            break;
        }
        i = comma + 1;
    }
    return true;
}

[[nodiscard]] inline bool
includes_ok(const std::set<std::string>& includes, std::span<const std::string_view> allowed)
{
    for (const auto& name: includes) {
        if (!in_list(name, allowed)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Build a ghost::Principal from the request's x-blog-* headers. A bearer
 * token with no explicit role is treated as an Administrator on the admin API;
 * no token means the anonymous Content API reader (which may read published
 * pages). This makes the same handler serve both the admin and public surfaces.
 */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    core::auth::ghost::Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "ADMIN_API") {
        p.context = ApiContext::ADMIN_API;
    } else if (context == "CONTENT_API") {
        p.context = ApiContext::CONTENT_API;
    } else if (context == "MEMBERS_APP") {
        p.context = ApiContext::MEMBERS_APP;
    } else if (context == "INTERNAL") {
        p.context = ApiContext::INTERNAL;
    } else if (context == "SYSTEM") {
        p.context = ApiContext::SYSTEM;
    } else {
        p.context =
            req.get_authorization().empty() ? ApiContext::CONTENT_API : ApiContext::ADMIN_API;
    }

    p.role = std::string{req.find_header("x-blog-role")};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) {
        p.role = "Administrator";
    }
    for (auto part:
         std::string_view{req.find_header("x-blog-permissions")} | std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) {
            p.permissions.insert(scope);
        }
    }
    const auto staff = req.find_header("x-blog-staff");
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) {
        p.id = "bearer";
    }
    return p;
}

/** @brief True when the request is an anonymous/public content read. */
[[nodiscard]] inline bool is_public(const core::auth::ghost::Principal& p)
{
    return p.context == ApiContext::CONTENT_API;
}

// ------------------------------ JSON helpers --------------------------------

[[nodiscard]] inline std::string unquote(std::string value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

[[nodiscard]] inline std::string field_raw(const serde::Value& v, const char* key)
{
    auto object = v.to_object();
    if (!object) {
        return "";
    }
    auto field = object->get(key);
    if (!field) {
        return "";
    }
    return core_serde::JsonCodec{}.encode(*field);
}

[[nodiscard]] inline std::string field_string(const serde::Value& v, const char* key)
{
    return unquote(field_raw(v, key));
}

/** @brief Decode a single JSON object and read one string field. */
[[nodiscard]] inline std::string row_field(std::string_view json, const char* key)
{
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(json)) {
            return field_string(*value, key);
        }
    } catch (...) {
    }
    return "";
}

/** @brief First object of a JSON array, or an empty string. */
[[nodiscard]] inline std::string first_row(std::string_view rows)
{
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(rows)) {
            auto array = value->to_array();
            if (array && !array->empty()) {
                return core_serde::JsonCodec{}.encode((*array)[0]);
            }
        }
    } catch (...) {
    }
    return "";
}

/** @brief Collect the `id` of every object in a JSON array. */
[[nodiscard]] inline std::vector<std::string> ids_from_rows(std::string_view rows)
{
    std::vector<std::string> out;
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(rows)) {
            auto array = value->to_array();
            if (array) {
                for (const auto& element: *array) {
                    const auto id = field_string(element, "id");
                    if (!id.empty()) {
                        out.push_back(id);
                    }
                }
            }
        }
    } catch (...) {
    }
    return out;
}

/** @brief Replace one field of a JSON object with a JSON string value. */
[[nodiscard]] inline std::string
set_string_field(std::string json, std::string_view key, std::string_view value)
{
    const std::string needle = "\"" + std::string{key} + "\":";
    const auto pos = json.find(needle);
    if (pos == std::string::npos) {
        return json;
    }
    std::size_t start = pos + needle.size();
    while (start < json.size() && std::isspace(static_cast<unsigned char>(json[start]))) {
        ++start;
    }
    std::size_t end = start;
    if (end < json.size() && json[end] == '"') {
        ++end;
        while (end < json.size()) {
            if (json[end] == '\\' && end + 1 < json.size()) {
                end += 2;
                continue;
            }
            if (json[end] == '"') {
                ++end;
                break;
            }
            ++end;
        }
    } else {
        while (end < json.size() && json[end] != ',' && json[end] != '}') {
            ++end;
        }
    }
    std::string escaped;
    for (const char c: value) {
        if (c == '"' || c == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(c);
    }
    return json.substr(0, start) + "\"" + escaped + "\"" + json.substr(end);
}

/** @brief JSON array of quoted ids, e.g. ["1","2"]. */
[[nodiscard]] inline std::string json_ids(const std::vector<std::string>& ids)
{
    std::string out{"["};
    bool first = true;
    for (const auto& id: ids) {
        if (!first) {
            out += ',';
        }
        out += "\"" + id + "\"";
        first = false;
    }
    out += ']';
    return out;
}

// ------------------------------ outbox --------------------------------------

/** @brief Emit a Ghost DomainEvent through the shared transactional outbox. */
inline void emit_event(std::string_view type, std::string payload, std::string aggregate_id)
{
    blog::handlers::outbox::global_outbox().emit(
        blog::handlers::outbox::Event{
            .type = std::string{type},
            .payload = std::move(payload),
            .aggregate_id = std::move(aggregate_id)
        }
    );
}

} // namespace blog::handlers::pages::support
