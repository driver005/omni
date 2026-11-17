module;
#include <rfl/Generic.hpp>
export module blog_dto:more_resources;

import std;
import core_serde;

export namespace blog::dto {

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

class SearchAuthorsResponse
{
public:
    void set_authors(std::vector<RecordDto> value) { authors = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_authors() const noexcept { return authors; }
    std::vector<RecordDto> authors{};
};

class SearchUsersResponse
{
public:
    void set_users(std::vector<RecordDto> value) { users = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_users() const noexcept { return users; }
    std::vector<RecordDto> users{};
};

} // namespace blog::dto

#define ARRAY_SERDE(Type, Table, JsonName, Field)                                                       \
    template<> struct serde::Serializable<blog::dto::Type>                                     \
    {                                                                                          \
        static constexpr auto fields()                                                         \
        { return std::tuple{serde::FieldDesc<JsonName, &blog::dto::Type::get_##Field,               \
                                              &blog::dto::Type::set_##Field,                    \
                                              serde::FieldOptions{}>{}}; }                      \
        static constexpr std::string_view table_name() { return Table; }                       \
    };

template<> struct serde::Serializable<blog::dto::RecordDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::RecordDto::get_id, &blog::dto::RecordDto::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::RecordDto::get_name, &blog::dto::RecordDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::dto::RecordDto::get_slug, &blog::dto::RecordDto::set_slug, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "record_dto"; }
};

ARRAY_SERDE(AutomatedEmailsResponse, "automated_emails_response", "automated_emails", automated_emails)
ARRAY_SERDE(CustomThemeSettingsResponse, "custom_theme_settings_response", "custom_theme_settings", custom_theme_settings)
ARRAY_SERDE(IntegrationsResponse, "integrations_response", "integrations", integrations)

ARRAY_SERDE(SearchPagesResponse, "search_pages", "pages", pages)
ARRAY_SERDE(SearchPostsResponse, "search_posts", "posts", posts)
ARRAY_SERDE(SearchTagsResponse, "search_tags", "tags", tags)
ARRAY_SERDE(SearchUsersResponse, "search_users", "users", users)

ARRAY_SERDE(SearchAuthorsResponse, "search_authors", "authors", authors)

template<> struct serde::Serializable<blog::dto::OembedResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"html", &blog::dto::OembedResponse::get_html, &blog::dto::OembedResponse::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::OembedResponse::get_url, &blog::dto::OembedResponse::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oembed_response"; }
};

template<> struct serde::Serializable<blog::dto::FeedbackResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::FeedbackResponse::get_id, &blog::dto::FeedbackResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &blog::dto::FeedbackResponse::get_message, &blog::dto::FeedbackResponse::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "feedback_response"; }
};
