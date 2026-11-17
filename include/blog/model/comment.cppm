module;
#include "entity_macros.h"

export module blog_model:comment;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost comments entity — full field set from data/schema/schema.js. */
struct Comment
{
    std::int64_t id{};
    std::string post_id{};
    std::string member_id{};
    std::string parent_id{};
    std::string in_reply_to_id{};
    std::string status{};
    std::string html{};
    std::chrono::system_clock::time_point edited_at{};
    std::chrono::system_clock::time_point pinned_at{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Comment, "comments",
    GHOST_FIELD("id", &Comment::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("post_id", &Comment::post_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("member_id", &Comment::member_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("parent_id", &Comment::parent_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("in_reply_to_id", &Comment::in_reply_to_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Comment::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("html", &Comment::html, core::db::ColumnType::TEXT),
    GHOST_FIELD("edited_at", &Comment::edited_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("pinned_at", &Comment::pinned_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("created_at", &Comment::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Comment::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_comment"> blog_model_comment_suite = []
{
    "reflects full Ghost comments schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Comment>);
        expect(core::db::reflect::field_count_v<Comment> == 11);
        expect(core::db::reflect::EntityMeta<Comment>::table == "comments");
    };
};
} // namespace blog::model::tests
#endif
