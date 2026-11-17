module;
#include "entity_macros.h"

export module blog_model:page;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Page
{
public:
    Page() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_uuid(std::string value) { m_uuid = std::move(value); }
    [[nodiscard]] const std::string& get_uuid() const noexcept { return m_uuid; }
    void set_title(std::string value) { m_title = std::move(value); }
    [[nodiscard]] const std::string& get_title() const noexcept { return m_title; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_html(std::string value) { m_html = std::move(value); }
    [[nodiscard]] const std::string& get_html() const noexcept { return m_html; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }

    std::int64_t m_id{};
    std::string m_uuid{};
    std::string m_title{};
    std::string m_slug{};
    std::string m_html{};
    std::string m_status{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Page>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Page::get_id, &blog::model::Page::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"uuid", &blog::model::Page::get_uuid, &blog::model::Page::set_uuid, serde::FieldOptions{}>{},
            serde::FieldDesc<"title", &blog::model::Page::get_title, &blog::model::Page::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::Page::get_slug, &blog::model::Page::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"html", &blog::model::Page::get_html, &blog::model::Page::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Page::get_status, &blog::model::Page::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "pages"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_page"> blog_model_page_suite = []
{
    "reflects Page fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Page>);
        expect(core::db::reflect::field_count_v<Page> == 6);
        expect(core::db::reflect::EntityMeta<Page>::table == "pages");
    };
};

} // namespace blog::model::tests
#endif
