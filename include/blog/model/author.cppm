module;
#include "entity_macros.h"

export module blog_model:author;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

struct Author
{
    std::int64_t id{};
    std::string name{};
    std::string slug{};
    std::string email{};
};

GHOST_ENTITY(Author, "authors",
    GHOST_FIELD("id", &Author::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &Author::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Author::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("email", &Author::email, core::db::ColumnType::TEXT));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_author"> blog_model_author_suite = []
{
    "reflects Author fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Author>);
        expect(core::db::reflect::field_count_v<Author> == 4);
        expect(core::db::reflect::EntityMeta<Author>::table == "authors");
    };
};

} // namespace blog::model::tests
#endif
