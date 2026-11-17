module;
#include "entity_macros.h"

export module blog_model:integration;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost integrations entity — full field set from data/schema/schema.js. */
struct Integration
{
    std::int64_t id{};
    std::string type{};
    std::string name{};
    std::string slug{};
    std::string icon_image{};
    std::string description{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Integration, "integrations",
    GHOST_FIELD("id", &Integration::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("type", &Integration::type, core::db::ColumnType::TEXT),
    GHOST_FIELD("name", &Integration::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Integration::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("icon_image", &Integration::icon_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("description", &Integration::description, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Integration::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Integration::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_integration"> blog_model_integration_suite = []
{
    "reflects full Ghost integrations schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Integration>);
        expect(core::db::reflect::field_count_v<Integration> == 8);
        expect(core::db::reflect::EntityMeta<Integration>::table == "integrations");
    };
};
} // namespace blog::model::tests
#endif
