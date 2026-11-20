module;
#include <rfl/Generic.hpp>
export module blog_handlers:search_index_dto_more_resources;

import std;
import core_serde;
import interfaces;
import core_auth;
import core_query;

export namespace blog::dto::search_index {

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

class RecordDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_slug(std::string value) { slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return slug; }
    std::string id{"1"};
    std::string name{"Ghost Record"};
    std::string slug{"ghost-record"};
};

class AutomatedEmailsResponse
{
public:
    void set_automated_emails(std::vector<RecordDto> value) { automated_emails = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_automated_emails() const noexcept { return automated_emails; }
    std::vector<RecordDto> automated_emails{};
};

class CustomThemeSettingsResponse
{
public:
    void set_custom_theme_settings(std::vector<RecordDto> value) { custom_theme_settings = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_custom_theme_settings() const noexcept { return custom_theme_settings; }
    std::vector<RecordDto> custom_theme_settings{};
};

class IntegrationsResponse
{
public:
    void set_integrations(std::vector<RecordDto> value) { integrations = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_integrations() const noexcept { return integrations; }
    std::vector<RecordDto> integrations{};
};

class OembedResponse
{
public:
    void set_html(std::string value) { html = std::move(value); }
    [[nodiscard]] const std::string& get_html() const noexcept { return html; }
    void set_url(std::string value) { url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return url; }
    std::string html{"<iframe src=\"https://ghost.local/embed\"></iframe>"};
    std::string url{"https://ghost.local/embed"};
};

class FeedbackResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    std::string id{"1"};
    std::string message{"Ghost feedback"};
};

class SearchPagesResponse
{
public:
    void set_pages(std::vector<RecordDto> value) { pages = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_pages() const noexcept { return pages; }
    std::vector<RecordDto> pages{};
};

class SearchPostsResponse
{
public:
    void set_posts(std::vector<RecordDto> value) { posts = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_posts() const noexcept { return posts; }
    std::vector<RecordDto> posts{};
};

class SearchTagsResponse
{
public:
    void set_tags(std::vector<RecordDto> value) { tags = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_tags() const noexcept { return tags; }
    std::vector<RecordDto> tags{};
};

class SearchUsersResponse
{
public:
    void set_users(std::vector<RecordDto> value) { users = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_users() const noexcept { return users; }
    std::vector<RecordDto> users{};
};

// ---------------------------------------------------------------------------
// Shared Ghost search-index semantics.
//
// These helpers carry the hardcoded options Ghost builds in
// `search-index.js` (Admin API) and `search-index-public.js` (Content API)
// into the C++ steps. They live in this folder-local, already-registered
// partition so every search_index step uses the same rules without touching
// shared files. A single C++ route serves both API surfaces at the same
// path+verb, so the surface is resolved from the request's `x-blog-context`
// header (the same mechanism the rest of the handlers use).
// ---------------------------------------------------------------------------

using core::auth::ghost::ApiContext;
using core::auth::ghost::Principal;

/** @brief Build a Ghost principal for a search-index request.
 *
 *  Mirrors `admin/pages/types/dto_links.cppm#principal_for`: an explicit
 *  x-blog-context wins; otherwise a bearer token implies the Admin API and
 *  no token implies the anonymous Content API reader. An admin request
 *  without a role is treated as Administrator so the Ghost permission check
 *  can still be exercised meaningfully. */
[[nodiscard]] inline Principal principal_for(interfaces::io::IRequest& req)
{
    Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "ADMIN_API") p.context = ApiContext::ADMIN_API;
    else if (context == "CONTENT_API") p.context = ApiContext::CONTENT_API;
    else if (context == "MEMBERS_APP") p.context = ApiContext::MEMBERS_APP;
    else if (context == "INTERNAL") p.context = ApiContext::INTERNAL;
    else if (context == "SYSTEM") p.context = ApiContext::SYSTEM;
    else p.context = req.get_authorization().empty() ? ApiContext::CONTENT_API
                                                      : ApiContext::ADMIN_API;

    p.role = std::string{req.find_header("x-blog-role")};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) p.role = "Administrator";
    for (auto part : std::string_view{req.find_header("x-blog-permissions")} |
                         std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) p.permissions.insert(scope);
    }
    const auto staff = req.find_header("x-blog-staff");
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) p.id = "bearer";
    return p;
}

/** @brief True for an anonymous/public Content API read. */
[[nodiscard]] inline bool is_content(const Principal& p) noexcept
{
    return p.context == ApiContext::CONTENT_API;
}

/** @brief Ghost's hardcoded options for one search-index operation. */
struct GhostOptions
{
    std::string resource;   // ApiDataStore resource (posts/pages/tags/users/authors)
    std::string doc_name;   // docName used by the Ghost permission check
    std::string entity;     // local_mapping::clean entity
    std::string filter;     // NQL raw filter (empty = no filter)
    std::string fields;     // comma-separated Ghost `columns` -> C++ `fields`
};

/** @brief Resolve Ghost's fixed options for @p kind on the @p content surface.
 *
 *  `posts` and `tags` exist in both APIs and branch on @p content; `pages`
 *  and `users` are Admin-only, `authors` is Content-only. */
[[nodiscard]] inline GhostOptions ghost_options(std::string_view kind, bool content)
{
    if (kind == "posts") {
        return content
            ? GhostOptions{"posts", "posts", "post", "type:post",
                           "id,slug,title,excerpt,url,updated_at,visibility"}
            : GhostOptions{"posts", "posts", "post",
                           "type:post+status:[draft,published,scheduled,sent]",
                           "id,uuid,url,title,slug,status,published_at,visibility"};
    }
    if (kind == "pages") {
        return {"pages", "posts", "post", "type:page+status:[draft,published,scheduled]",
                "id,uuid,url,title,slug,status,published_at,visibility"};
    }
    if (kind == "tags") {
        return content
            ? GhostOptions{"tags", "tags", "tag", "visibility:public",
                           "id,slug,name,url"}
            : GhostOptions{"tags", "tags", "tag", "", "id,slug,name,url"};
    }
    if (kind == "users") {
        return {"users", "users", "user", "", "id,slug,url,name,profile_image"};
    }
    // "authors" (Content API only).
    return {"authors", "authors", "author", "", "id,slug,name,url,profile_image"};
}

/** @brief Build the core::query pipeline object for Ghost's fixed options.
 *
 *  `limit:10000`, `order:updated_at DESC` and the allow-listed `fields`
 *  projection are the same on every search-index operation. core::query
 *  supplies the NQL filtering, ordering and projection Ghost applies in
 *  `Post.findPage` / `Tag.findPage` / `Author.findPage` / `User.findPage`. */
[[nodiscard]] inline core::query::ParsedQuery query_for(const GhostOptions& spec)
{
    core::query::ParsedQuery q;
    q.raw_filter = spec.filter;
    q.order = "updated_at DESC";
    q.limit = 10000;
    std::size_t start = 0;
    while (start <= spec.fields.size()) {
        const auto comma = spec.fields.find(',', start);
        const auto part = spec.fields.substr(
            start, comma == std::string::npos ? std::string::npos : comma - start);
        if (!part.empty()) q.fields.emplace(std::string{part});
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return q;
}

} // namespace blog::dto::search_index


template<> struct serde::Serializable<blog::dto::search_index::RecordDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::search_index::RecordDto::get_id, &blog::dto::search_index::RecordDto::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::search_index::RecordDto::get_name, &blog::dto::search_index::RecordDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::dto::search_index::RecordDto::get_slug, &blog::dto::search_index::RecordDto::set_slug, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "record_dto"; }
};

template<> struct serde::Serializable<blog::dto::search_index::AutomatedEmailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"automated_emails", &blog::dto::search_index::AutomatedEmailsResponse::get_automated_emails,
                                           &blog::dto::search_index::AutomatedEmailsResponse::set_automated_emails,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "automated_emails_response"; }
};
template<> struct serde::Serializable<blog::dto::search_index::CustomThemeSettingsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"custom_theme_settings", &blog::dto::search_index::CustomThemeSettingsResponse::get_custom_theme_settings,
                                           &blog::dto::search_index::CustomThemeSettingsResponse::set_custom_theme_settings,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "custom_theme_settings_response"; }
};
template<> struct serde::Serializable<blog::dto::search_index::IntegrationsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"integrations", &blog::dto::search_index::IntegrationsResponse::get_integrations,
                                           &blog::dto::search_index::IntegrationsResponse::set_integrations,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "integrations_response"; }
};

template<> struct serde::Serializable<blog::dto::search_index::SearchPagesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"pages", &blog::dto::search_index::SearchPagesResponse::get_pages,
                                           &blog::dto::search_index::SearchPagesResponse::set_pages,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_pages"; }
};
template<> struct serde::Serializable<blog::dto::search_index::SearchPostsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"posts", &blog::dto::search_index::SearchPostsResponse::get_posts,
                                           &blog::dto::search_index::SearchPostsResponse::set_posts,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_posts"; }
};
template<> struct serde::Serializable<blog::dto::search_index::SearchTagsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"tags", &blog::dto::search_index::SearchTagsResponse::get_tags,
                                           &blog::dto::search_index::SearchTagsResponse::set_tags,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_tags"; }
};
template<> struct serde::Serializable<blog::dto::search_index::SearchUsersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"users", &blog::dto::search_index::SearchUsersResponse::get_users,
                                           &blog::dto::search_index::SearchUsersResponse::set_users,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_users"; }
};

template<> struct serde::Serializable<blog::dto::search_index::OembedResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"html", &blog::dto::search_index::OembedResponse::get_html, &blog::dto::search_index::OembedResponse::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::search_index::OembedResponse::get_url, &blog::dto::search_index::OembedResponse::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oembed_response"; }
};

template<> struct serde::Serializable<blog::dto::search_index::FeedbackResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::search_index::FeedbackResponse::get_id, &blog::dto::search_index::FeedbackResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &blog::dto::search_index::FeedbackResponse::get_message, &blog::dto::search_index::FeedbackResponse::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "feedback_response"; }
};
