export module blog_handlers:dto_named_resources;

import std;
import core_serde;

export namespace blog::dto {

class NamedRecordDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_slug(std::string value) { slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return slug; }
    std::string id{"1"};
    std::string name{"Ghost Resource"};
    std::string slug{"ghost-resource"};
};

class LabelsResponse
{
public:
    void set_labels(std::vector<NamedRecordDto> value) { labels = std::move(value); }
    [[nodiscard]] const std::vector<NamedRecordDto>& get_labels() const noexcept { return labels; }
    std::vector<NamedRecordDto> labels{};
};

class SnippetsResponse
{
public:
    void set_snippets(std::vector<NamedRecordDto> value) { snippets = std::move(value); }
    [[nodiscard]] const std::vector<NamedRecordDto>& get_snippets() const noexcept { return snippets; }
    std::vector<NamedRecordDto> snippets{};
};

class RecommendationsResponse
{
public:
    void set_recommendations(std::vector<NamedRecordDto> value) { recommendations = std::move(value); }
    [[nodiscard]] const std::vector<NamedRecordDto>& get_recommendations() const noexcept { return recommendations; }
    std::vector<NamedRecordDto> recommendations{};
};

class WebhooksDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    std::string id{"1"};
    std::string name{"Ghost Webhook"};
};

class WebhooksListResponse
{
public:
    void set_webhooks(std::vector<WebhooksDto> value) { webhooks = std::move(value); }
    [[nodiscard]] const std::vector<WebhooksDto>& get_webhooks() const noexcept { return webhooks; }
    std::vector<WebhooksDto> webhooks{};
};

} // namespace blog::dto


template<> struct serde::Serializable<blog::dto::NamedRecordDto>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"name", &blog::dto::NamedRecordDto::get_name,
                                           &blog::dto::NamedRecordDto::set_name,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "named_record"; }
};
template<> struct serde::Serializable<blog::dto::LabelsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"labels", &blog::dto::LabelsResponse::get_labels,
                                           &blog::dto::LabelsResponse::set_labels,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "labels_response"; }
};
template<> struct serde::Serializable<blog::dto::SnippetsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"snippets", &blog::dto::SnippetsResponse::get_snippets,
                                           &blog::dto::SnippetsResponse::set_snippets,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "snippets_response"; }
};
template<> struct serde::Serializable<blog::dto::RecommendationsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"recommendations", &blog::dto::RecommendationsResponse::get_recommendations,
                                           &blog::dto::RecommendationsResponse::set_recommendations,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "recommendations_response"; }
};
template<> struct serde::Serializable<blog::dto::WebhooksDto>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"name", &blog::dto::WebhooksDto::get_name,
                                           &blog::dto::WebhooksDto::set_name,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "webhook_record"; }
};

template<>
struct serde::Serializable<blog::dto::WebhooksListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"webhooks", &blog::dto::WebhooksListResponse::get_webhooks,
                             &blog::dto::WebhooksListResponse::set_webhooks, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhooks_list_response"; }
};
