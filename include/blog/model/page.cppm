module;
#include "entity_macros.h"

export module blog_model:page;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

struct Page
{
    std::int64_t id{};
    std::string uuid{};
    std::string title{};
    std::string slug{};
    std::string html{};
    std::string status{};
};

GHOST_ENTITY(Page, "pages",
    GHOST_FIELD("id", &Page::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("uuid", &Page::uuid, core::db::ColumnType::TEXT),
    GHOST_FIELD("title", &Page::title, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Page::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("html", &Page::html, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Page::status, core::db::ColumnType::TEXT));

} // namespace blog::model

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
