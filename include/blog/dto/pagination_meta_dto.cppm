module;
#include <rfl.hpp>
export module blog_dto:pagination_meta_dto;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: PaginationMetaDto. */
class PaginationMetaDto
{
public:
    void set_limit(int value) { limit = std::move(value); }
    [[nodiscard]] const int& get_limit() const noexcept { return limit; }
    void set_next(rfl::Generic value) { next = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_next() const noexcept { return next; }
    void set_page(int value) { page = std::move(value); }
    [[nodiscard]] const int& get_page() const noexcept { return page; }
    void set_pages(int value) { pages = std::move(value); }
    [[nodiscard]] const int& get_pages() const noexcept { return pages; }
    void set_prev(rfl::Generic value) { prev = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_prev() const noexcept { return prev; }
    void set_total(int value) { total = std::move(value); }
    [[nodiscard]] const int& get_total() const noexcept { return total; }

    int limit{15};
    rfl::Generic next{};
    int page{1};
    int pages{1};
    rfl::Generic prev{};
    int total{0};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::PaginationMetaDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"limit", &blog::dto::PaginationMetaDto::get_limit, &blog::dto::PaginationMetaDto::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"next", &blog::dto::PaginationMetaDto::get_next, &blog::dto::PaginationMetaDto::set_next, serde::FieldOptions{}>{},
            serde::FieldDesc<"page", &blog::dto::PaginationMetaDto::get_page, &blog::dto::PaginationMetaDto::set_page, serde::FieldOptions{}>{},
            serde::FieldDesc<"pages", &blog::dto::PaginationMetaDto::get_pages, &blog::dto::PaginationMetaDto::set_pages, serde::FieldOptions{}>{},
            serde::FieldDesc<"prev", &blog::dto::PaginationMetaDto::get_prev, &blog::dto::PaginationMetaDto::set_prev, serde::FieldOptions{}>{},
            serde::FieldDesc<"total", &blog::dto::PaginationMetaDto::get_total, &blog::dto::PaginationMetaDto::set_total, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pagination_meta_dto";
    }
};

