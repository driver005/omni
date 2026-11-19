export module blog_handlers:dto_detail_resources;

import std;
import core_serde;

export namespace blog::dto {

class DetailDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_slug(std::string value) { slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return slug; }
    std::string id{"1"};
    std::string name{"Ghost Detail"};
    std::string slug{"ghost-detail"};
};


class InvitesDetailResponse
{
public:
    void set_invites(std::vector<DetailDto> value) { invites = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_invites() const noexcept { return invites; }
    std::vector<DetailDto> invites{};
};
class OffersDetailResponse
{
public:
    void set_offers(std::vector<DetailDto> value) { offers = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_offers() const noexcept { return offers; }
    std::vector<DetailDto> offers{};
};
class TiersDetailResponse
{
public:
    void set_tiers(std::vector<DetailDto> value) { tiers = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_tiers() const noexcept { return tiers; }
    std::vector<DetailDto> tiers{};
};
class LabelsDetailResponse
{
public:
    void set_labels(std::vector<DetailDto> value) { labels = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_labels() const noexcept { return labels; }
    std::vector<DetailDto> labels{};
};
class SnippetsDetailResponse
{
public:
    void set_snippets(std::vector<DetailDto> value) { snippets = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_snippets() const noexcept { return snippets; }
    std::vector<DetailDto> snippets{};
};
class RecommendationsDetailResponse
{
public:
    void set_recommendations(std::vector<DetailDto> value) { recommendations = std::move(value); }
    [[nodiscard]] const std::vector<DetailDto>& get_recommendations() const noexcept { return recommendations; }
    std::vector<DetailDto> recommendations{};
};

} // namespace blog::dto

template<> struct serde::Serializable<blog::dto::DetailDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::DetailDto::get_id, &blog::dto::DetailDto::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::DetailDto::get_name, &blog::dto::DetailDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::dto::DetailDto::get_slug, &blog::dto::DetailDto::set_slug, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "detail_dto"; }
};


template<> struct serde::Serializable<blog::dto::InvitesDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"invites", &blog::dto::InvitesDetailResponse::get_invites,
                                           &blog::dto::InvitesDetailResponse::set_invites,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "invites"; }
};
template<> struct serde::Serializable<blog::dto::OffersDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"offers", &blog::dto::OffersDetailResponse::get_offers,
                                           &blog::dto::OffersDetailResponse::set_offers,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "offers"; }
};
template<> struct serde::Serializable<blog::dto::TiersDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"tiers", &blog::dto::TiersDetailResponse::get_tiers,
                                           &blog::dto::TiersDetailResponse::set_tiers,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "tiers"; }
};
template<> struct serde::Serializable<blog::dto::LabelsDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"labels", &blog::dto::LabelsDetailResponse::get_labels,
                                           &blog::dto::LabelsDetailResponse::set_labels,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "labels"; }
};
template<> struct serde::Serializable<blog::dto::SnippetsDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"snippets", &blog::dto::SnippetsDetailResponse::get_snippets,
                                           &blog::dto::SnippetsDetailResponse::set_snippets,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "snippets"; }
};
template<> struct serde::Serializable<blog::dto::RecommendationsDetailResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"recommendations", &blog::dto::RecommendationsDetailResponse::get_recommendations,
                                           &blog::dto::RecommendationsDetailResponse::set_recommendations,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "recommendations"; }
};
