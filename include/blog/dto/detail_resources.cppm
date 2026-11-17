export module blog_dto:detail_resources;

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

#define DETAIL_RESPONSE(Type, Key)                                                            \
    class Type                                                                                \
    {                                                                                         \
    public:                                                                                    \
        void set_##Key(std::vector<DetailDto> value) { Key = std::move(value); }                \
        [[nodiscard]] const std::vector<DetailDto>& get_##Key() const noexcept { return Key; }  \
        std::vector<DetailDto> Key{};                                                           \
    };

DETAIL_RESPONSE(InvitesDetailResponse, invites)
DETAIL_RESPONSE(OffersDetailResponse, offers)
DETAIL_RESPONSE(TiersDetailResponse, tiers)
DETAIL_RESPONSE(LabelsDetailResponse, labels)
DETAIL_RESPONSE(SnippetsDetailResponse, snippets)
DETAIL_RESPONSE(RecommendationsDetailResponse, recommendations)

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

#define DETAIL_SERDE(Type, Key)                                                                \
    template<> struct serde::Serializable<blog::dto::Type>                                     \
    {                                                                                          \
        static constexpr auto fields()                                                         \
        {                                                                                      \
            return std::tuple{serde::FieldDesc<#Key, &blog::dto::Type::get_##Key,               \
                                               &blog::dto::Type::set_##Key,                    \
                                               serde::FieldOptions{}>{}};                       \
        }                                                                                      \
        static constexpr std::string_view table_name() { return #Key; }                        \
    };

DETAIL_SERDE(InvitesDetailResponse, invites)
DETAIL_SERDE(OffersDetailResponse, offers)
DETAIL_SERDE(TiersDetailResponse, tiers)
DETAIL_SERDE(LabelsDetailResponse, labels)
DETAIL_SERDE(SnippetsDetailResponse, snippets)
DETAIL_SERDE(RecommendationsDetailResponse, recommendations)
