module;
#include "entity_macros.h"

export module blog_model:setting;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

struct Setting
{
    std::int64_t id{};
    std::string key{};
    std::string value{};
};

GHOST_ENTITY(Setting, "settings",
    GHOST_FIELD("id", &Setting::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("key", &Setting::key, core::db::ColumnType::TEXT),
    GHOST_FIELD("value", &Setting::value, core::db::ColumnType::TEXT));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_setting"> blog_model_setting_suite = []
{
    "reflects Setting fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Setting>);
        expect(core::db::reflect::field_count_v<Setting> == 3);
        expect(core::db::reflect::EntityMeta<Setting>::table == "settings");
    };
};

} // namespace blog::model::tests
#endif
