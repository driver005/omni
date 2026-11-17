module;
#include "entity_macros.h"

export module blog_model:label;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost labels entity — full field set from data/schema/schema.js. */
struct Label
{
    std::int64_t id{};
    std::string name{};
    std::string slug{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Label, "labels",
    GHOST_FIELD("id", &Label::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &Label::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Label::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Label::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Label::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_label"> blog_model_label_suite = []
{
    "reflects full Ghost labels schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Label>);
        expect(core::db::reflect::field_count_v<Label> == 5);
        expect(core::db::reflect::EntityMeta<Label>::table == "labels");
    };
};
} // namespace blog::model::tests
#endif
