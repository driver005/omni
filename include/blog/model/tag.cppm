module;
#include "entity_macros.h"

export module blog_model:tag;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

struct Tag
{
    std::int64_t id{};
    std::string name{};
    std::string slug{};
    std::string description{};
};

GHOST_ENTITY(Tag, "tags",
    GHOST_FIELD("id", &Tag::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &Tag::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Tag::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("description", &Tag::description, core::db::ColumnType::TEXT));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_tag"> blog_model_tag_suite = []
{
    "reflects Tag fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Tag>);
        expect(core::db::reflect::field_count_v<Tag> == 4);
        expect(core::db::reflect::EntityMeta<Tag>::table == "tags");
    };
};

} // namespace blog::model::tests
#endif
