module;
#include "entity_macros.h"

export module blog_model:invite;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost invites entity — full field set from data/schema/schema.js. */
struct Invite
{
    std::int64_t id{};
    std::string role_id{};
    std::string status{};
    std::string token{};
    std::string email{};
    std::chrono::system_clock::time_point expires{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Invite, "invites",
    GHOST_FIELD("id", &Invite::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("role_id", &Invite::role_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Invite::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("token", &Invite::token, core::db::ColumnType::TEXT),
    GHOST_FIELD("email", &Invite::email, core::db::ColumnType::TEXT),
    GHOST_FIELD("expires", &Invite::expires, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("created_at", &Invite::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Invite::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_invite"> blog_model_invite_suite = []
{
    "reflects full Ghost invites schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Invite>);
        expect(core::db::reflect::field_count_v<Invite> == 8);
        expect(core::db::reflect::EntityMeta<Invite>::table == "invites");
    };
};
} // namespace blog::model::tests
#endif
