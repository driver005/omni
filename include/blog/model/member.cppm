module;
#include "entity_macros.h"

export module blog_model:member;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost members entity — full field set from data/schema/schema.js. */
struct Member
{
    std::int64_t id{};
    std::string uuid{};
    std::string transient_id{};
    std::string email{};
    std::string status{};
    std::string name{};
    std::string expertise{};
    std::string note{};
    std::string geolocation{};
    bool enable_comment_notifications{};
    bool enable_updates_and_announcements{};
    std::int64_t email_count{};
    std::int64_t email_opened_count{};
    std::int64_t email_open_rate{};
    bool email_disabled{};
    std::chrono::system_clock::time_point last_seen_at{};
    std::chrono::system_clock::time_point last_commented_at{};
    bool commenting{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Member, "members",
    GHOST_FIELD("id", &Member::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("uuid", &Member::uuid, core::db::ColumnType::TEXT),
    GHOST_FIELD("transient_id", &Member::transient_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("email", &Member::email, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Member::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("name", &Member::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("expertise", &Member::expertise, core::db::ColumnType::TEXT),
    GHOST_FIELD("note", &Member::note, core::db::ColumnType::TEXT),
    GHOST_FIELD("geolocation", &Member::geolocation, core::db::ColumnType::TEXT),
    GHOST_FIELD("enable_comment_notifications", &Member::enable_comment_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("enable_updates_and_announcements", &Member::enable_updates_and_announcements, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("email_count", &Member::email_count, core::db::ColumnType::INTEGER),
    GHOST_FIELD("email_opened_count", &Member::email_opened_count, core::db::ColumnType::INTEGER),
    GHOST_FIELD("email_open_rate", &Member::email_open_rate, core::db::ColumnType::INTEGER),
    GHOST_FIELD("email_disabled", &Member::email_disabled, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("last_seen_at", &Member::last_seen_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("last_commented_at", &Member::last_commented_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("commenting", &Member::commenting, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("created_at", &Member::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Member::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_member"> blog_model_member_suite = []
{
    "reflects full Ghost members schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Member>);
        expect(core::db::reflect::field_count_v<Member> == 20);
        expect(core::db::reflect::EntityMeta<Member>::table == "members");
    };
};
} // namespace blog::model::tests
#endif
