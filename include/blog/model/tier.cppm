module;
#include "entity_macros.h"

export module blog_model:tier;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

struct Tier
{
    std::int64_t id{};
    std::string name{};
    std::string slug{};
    double price{};
};

GHOST_ENTITY(Tier, "tiers",
    GHOST_FIELD("id", &Tier::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &Tier::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Tier::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("price", &Tier::price, core::db::ColumnType::DOUBLE));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_tier"> blog_model_tier_suite = []
{
    "reflects Tier fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Tier>);
        expect(core::db::reflect::field_count_v<Tier> == 4);
        expect(core::db::reflect::EntityMeta<Tier>::table == "tiers");
    };
};

} // namespace blog::model::tests
#endif
