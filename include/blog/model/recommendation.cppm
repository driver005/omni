module;
#include "entity_macros.h"

export module blog_model:recommendation;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost recommendations entity — full field set from data/schema/schema.js. */
struct Recommendation
{
    std::int64_t id{};
    std::string url{};
    std::string title{};
    std::string excerpt{};
    std::string featured_image{};
    std::string favicon{};
    std::string description{};
    bool one_click_subscribe{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Recommendation, "recommendations",
    GHOST_FIELD("id", &Recommendation::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("url", &Recommendation::url, core::db::ColumnType::TEXT),
    GHOST_FIELD("title", &Recommendation::title, core::db::ColumnType::TEXT),
    GHOST_FIELD("excerpt", &Recommendation::excerpt, core::db::ColumnType::TEXT),
    GHOST_FIELD("featured_image", &Recommendation::featured_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("favicon", &Recommendation::favicon, core::db::ColumnType::TEXT),
    GHOST_FIELD("description", &Recommendation::description, core::db::ColumnType::TEXT),
    GHOST_FIELD("one_click_subscribe", &Recommendation::one_click_subscribe, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("created_at", &Recommendation::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Recommendation::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_recommendation"> blog_model_recommendation_suite = []
{
    "reflects full Ghost recommendations schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Recommendation>);
        expect(core::db::reflect::field_count_v<Recommendation> == 10);
        expect(core::db::reflect::EntityMeta<Recommendation>::table == "recommendations");
    };
};
} // namespace blog::model::tests
#endif
