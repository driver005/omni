module;
export module blog_dto:pagination_envelope;

import std;
import core_serde;
import :pagination_meta_dto;

export namespace blog::dto {

/** @brief Ghost API DTO: PaginationEnvelope. */
class PaginationEnvelope
{
public:
    void set_pagination(PaginationMetaDto value) { pagination = std::move(value); }
    [[nodiscard]] const PaginationMetaDto& get_pagination() const noexcept { return pagination; }

    PaginationMetaDto pagination{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::PaginationEnvelope>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"pagination", &blog::dto::PaginationEnvelope::get_pagination, &blog::dto::PaginationEnvelope::set_pagination, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pagination_envelope";
    }
};

