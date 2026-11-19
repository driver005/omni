module;
#include <rfl/Generic.hpp>
export module blog_handlers:oembed_dto_more_resources;

import std;
import core_serde;

export namespace blog::dto::oembed {

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

} // namespace blog::dto::oembed


template<> struct serde::Serializable<blog::dto::oembed::RecordDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::oembed::RecordDto::get_id, &blog::dto::oembed::RecordDto::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::oembed::RecordDto::get_name, &blog::dto::oembed::RecordDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::dto::oembed::RecordDto::get_slug, &blog::dto::oembed::RecordDto::set_slug, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "record_dto"; }
};

template<> struct serde::Serializable<blog::dto::oembed::AutomatedEmailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"automated_emails", &blog::dto::oembed::AutomatedEmailsResponse::get_automated_emails,
                                           &blog::dto::oembed::AutomatedEmailsResponse::set_automated_emails,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "automated_emails_response"; }
};
template<> struct serde::Serializable<blog::dto::oembed::CustomThemeSettingsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"custom_theme_settings", &blog::dto::oembed::CustomThemeSettingsResponse::get_custom_theme_settings,
                                           &blog::dto::oembed::CustomThemeSettingsResponse::set_custom_theme_settings,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "custom_theme_settings_response"; }
};
template<> struct serde::Serializable<blog::dto::oembed::IntegrationsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"integrations", &blog::dto::oembed::IntegrationsResponse::get_integrations,
                                           &blog::dto::oembed::IntegrationsResponse::set_integrations,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "integrations_response"; }
};

template<> struct serde::Serializable<blog::dto::oembed::SearchPagesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"pages", &blog::dto::oembed::SearchPagesResponse::get_pages,
                                           &blog::dto::oembed::SearchPagesResponse::set_pages,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_pages"; }
};
template<> struct serde::Serializable<blog::dto::oembed::SearchPostsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"posts", &blog::dto::oembed::SearchPostsResponse::get_posts,
                                           &blog::dto::oembed::SearchPostsResponse::set_posts,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_posts"; }
};
template<> struct serde::Serializable<blog::dto::oembed::SearchTagsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"tags", &blog::dto::oembed::SearchTagsResponse::get_tags,
                                           &blog::dto::oembed::SearchTagsResponse::set_tags,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_tags"; }
};
template<> struct serde::Serializable<blog::dto::oembed::SearchUsersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"users", &blog::dto::oembed::SearchUsersResponse::get_users,
                                           &blog::dto::oembed::SearchUsersResponse::set_users,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_users"; }
};

template<> struct serde::Serializable<blog::dto::oembed::SearchAuthorsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"authors", &blog::dto::oembed::SearchAuthorsResponse::get_authors,
                                           &blog::dto::oembed::SearchAuthorsResponse::set_authors,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_authors"; }
};

template<> struct serde::Serializable<blog::dto::oembed::OembedResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"html", &blog::dto::oembed::OembedResponse::get_html, &blog::dto::oembed::OembedResponse::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::oembed::OembedResponse::get_url, &blog::dto::oembed::OembedResponse::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oembed_response"; }
};

template<> struct serde::Serializable<blog::dto::oembed::FeedbackResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::oembed::FeedbackResponse::get_id, &blog::dto::oembed::FeedbackResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &blog::dto::oembed::FeedbackResponse::get_message, &blog::dto::oembed::FeedbackResponse::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "feedback_response"; }
};
