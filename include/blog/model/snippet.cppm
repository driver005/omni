module;
#include "entity_macros.h"

export module blog_model:snippet;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost snippets entity — full field set from data/schema/schema.js. */
struct Snippet
{
    std::int64_t id{};
    std::string name{};
    std::string mobiledoc{};
    std::string lexical{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Snippet, "snippets",
    GHOST_FIELD("id", &Snippet::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &Snippet::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("mobiledoc", &Snippet::mobiledoc, core::db::ColumnType::TEXT),
    GHOST_FIELD("lexical", &Snippet::lexical, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Snippet::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Snippet::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_snippet"> blog_model_snippet_suite = []
{
    "reflects full Ghost snippets schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Snippet>);
        expect(core::db::reflect::field_count_v<Snippet> == 6);
        expect(core::db::reflect::EntityMeta<Snippet>::table == "snippets");
    };
};
} // namespace blog::model::tests
#endif
