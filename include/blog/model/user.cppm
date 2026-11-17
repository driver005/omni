module;
#include "entity_macros.h"

export module blog_model:user;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost users entity — full field set from data/schema/schema.js. */
struct User
{
    std::int64_t id{};
    std::string name{};
    std::string slug{};
    std::string password{};
    std::string email{};
    std::string profile_image{};
    std::string cover_image{};
    std::string bio{};
    std::string website{};
    std::string location{};
    std::string facebook{};
    std::string twitter{};
    std::string threads{};
    std::string bluesky{};
    std::string mastodon{};
    std::string tiktok{};
    std::string youtube{};
    std::string instagram{};
    std::string linkedin{};
    bool accessibility{};
    std::string status{};
    std::string locale{};
    std::string visibility{};
    std::string meta_title{};
    std::string meta_description{};
    bool tour{};
    std::chrono::system_clock::time_point last_seen{};
    bool comment_notifications{};
    std::string free_member_signup_notification{};
    std::string paid_subscription_started_notification{};
    std::string paid_subscription_canceled_notification{};
    bool mention_notifications{};
    bool recommendation_notifications{};
    bool milestone_notifications{};
    bool donation_notifications{};
    bool gift_subscription_notifications{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(User, "users",
    GHOST_FIELD("id", &User::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("name", &User::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &User::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("password", &User::password, core::db::ColumnType::TEXT),
    GHOST_FIELD("email", &User::email, core::db::ColumnType::TEXT),
    GHOST_FIELD("profile_image", &User::profile_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("cover_image", &User::cover_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("bio", &User::bio, core::db::ColumnType::TEXT),
    GHOST_FIELD("website", &User::website, core::db::ColumnType::TEXT),
    GHOST_FIELD("location", &User::location, core::db::ColumnType::TEXT),
    GHOST_FIELD("facebook", &User::facebook, core::db::ColumnType::TEXT),
    GHOST_FIELD("twitter", &User::twitter, core::db::ColumnType::TEXT),
    GHOST_FIELD("threads", &User::threads, core::db::ColumnType::TEXT),
    GHOST_FIELD("bluesky", &User::bluesky, core::db::ColumnType::TEXT),
    GHOST_FIELD("mastodon", &User::mastodon, core::db::ColumnType::TEXT),
    GHOST_FIELD("tiktok", &User::tiktok, core::db::ColumnType::TEXT),
    GHOST_FIELD("youtube", &User::youtube, core::db::ColumnType::TEXT),
    GHOST_FIELD("instagram", &User::instagram, core::db::ColumnType::TEXT),
    GHOST_FIELD("linkedin", &User::linkedin, core::db::ColumnType::TEXT),
    GHOST_FIELD("accessibility", &User::accessibility, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("status", &User::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("locale", &User::locale, core::db::ColumnType::TEXT),
    GHOST_FIELD("visibility", &User::visibility, core::db::ColumnType::TEXT),
    GHOST_FIELD("meta_title", &User::meta_title, core::db::ColumnType::TEXT),
    GHOST_FIELD("meta_description", &User::meta_description, core::db::ColumnType::TEXT),
    GHOST_FIELD("tour", &User::tour, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("last_seen", &User::last_seen, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("comment_notifications", &User::comment_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("free_member_signup_notification", &User::free_member_signup_notification, core::db::ColumnType::TEXT),
    GHOST_FIELD("paid_subscription_started_notification", &User::paid_subscription_started_notification, core::db::ColumnType::TEXT),
    GHOST_FIELD("paid_subscription_canceled_notification", &User::paid_subscription_canceled_notification, core::db::ColumnType::TEXT),
    GHOST_FIELD("mention_notifications", &User::mention_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("recommendation_notifications", &User::recommendation_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("milestone_notifications", &User::milestone_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("donation_notifications", &User::donation_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("gift_subscription_notifications", &User::gift_subscription_notifications, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("created_at", &User::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &User::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_user"> blog_model_user_suite = []
{
    "reflects full Ghost users schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<User>);
        expect(core::db::reflect::field_count_v<User> == 38);
        expect(core::db::reflect::EntityMeta<User>::table == "users");
    };
};
} // namespace blog::model::tests
#endif
