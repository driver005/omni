export module blog_handlers:model_user;

import std;
import core_serde;
import core_database;
import :model_member;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost users entity — full field set from data/schema/schema.js. */
class User
{
public:
    User() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_password(std::string value) { m_password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return m_password; }
    void set_email(std::string value) { m_email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return m_email; }
    void set_profile_image(std::string value) { m_profile_image = std::move(value); }
    [[nodiscard]] const std::string& get_profile_image() const noexcept { return m_profile_image; }
    void set_cover_image(std::string value) { m_cover_image = std::move(value); }
    [[nodiscard]] const std::string& get_cover_image() const noexcept { return m_cover_image; }
    void set_bio(std::string value) { m_bio = std::move(value); }
    [[nodiscard]] const std::string& get_bio() const noexcept { return m_bio; }
    void set_website(std::string value) { m_website = std::move(value); }
    [[nodiscard]] const std::string& get_website() const noexcept { return m_website; }
    void set_location(std::string value) { m_location = std::move(value); }
    [[nodiscard]] const std::string& get_location() const noexcept { return m_location; }
    void set_facebook(std::string value) { m_facebook = std::move(value); }
    [[nodiscard]] const std::string& get_facebook() const noexcept { return m_facebook; }
    void set_twitter(std::string value) { m_twitter = std::move(value); }
    [[nodiscard]] const std::string& get_twitter() const noexcept { return m_twitter; }
    void set_threads(std::string value) { m_threads = std::move(value); }
    [[nodiscard]] const std::string& get_threads() const noexcept { return m_threads; }
    void set_bluesky(std::string value) { m_bluesky = std::move(value); }
    [[nodiscard]] const std::string& get_bluesky() const noexcept { return m_bluesky; }
    void set_mastodon(std::string value) { m_mastodon = std::move(value); }
    [[nodiscard]] const std::string& get_mastodon() const noexcept { return m_mastodon; }
    void set_tiktok(std::string value) { m_tiktok = std::move(value); }
    [[nodiscard]] const std::string& get_tiktok() const noexcept { return m_tiktok; }
    void set_youtube(std::string value) { m_youtube = std::move(value); }
    [[nodiscard]] const std::string& get_youtube() const noexcept { return m_youtube; }
    void set_instagram(std::string value) { m_instagram = std::move(value); }
    [[nodiscard]] const std::string& get_instagram() const noexcept { return m_instagram; }
    void set_linkedin(std::string value) { m_linkedin = std::move(value); }
    [[nodiscard]] const std::string& get_linkedin() const noexcept { return m_linkedin; }
    void set_accessibility(bool value) { m_accessibility = std::move(value); }
    [[nodiscard]] const bool& get_accessibility() const noexcept { return m_accessibility; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_locale(std::string value) { m_locale = std::move(value); }
    [[nodiscard]] const std::string& get_locale() const noexcept { return m_locale; }
    void set_visibility(std::string value) { m_visibility = std::move(value); }
    [[nodiscard]] const std::string& get_visibility() const noexcept { return m_visibility; }
    void set_meta_title(std::string value) { m_meta_title = std::move(value); }
    [[nodiscard]] const std::string& get_meta_title() const noexcept { return m_meta_title; }
    void set_meta_description(std::string value) { m_meta_description = std::move(value); }
    [[nodiscard]] const std::string& get_meta_description() const noexcept { return m_meta_description; }
    void set_tour(bool value) { m_tour = std::move(value); }
    [[nodiscard]] const bool& get_tour() const noexcept { return m_tour; }
    void set_last_seen(std::chrono::system_clock::time_point value) { m_last_seen = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_last_seen() const noexcept { return m_last_seen; }
    void set_comment_notifications(bool value) { m_comment_notifications = std::move(value); }
    [[nodiscard]] const bool& get_comment_notifications() const noexcept { return m_comment_notifications; }
    void set_free_member_signup_notification(std::string value) { m_free_member_signup_notification = std::move(value); }
    [[nodiscard]] const std::string& get_free_member_signup_notification() const noexcept { return m_free_member_signup_notification; }
    void set_paid_subscription_started_notification(std::string value) { m_paid_subscription_started_notification = std::move(value); }
    [[nodiscard]] const std::string& get_paid_subscription_started_notification() const noexcept { return m_paid_subscription_started_notification; }
    void set_paid_subscription_canceled_notification(std::string value) { m_paid_subscription_canceled_notification = std::move(value); }
    [[nodiscard]] const std::string& get_paid_subscription_canceled_notification() const noexcept { return m_paid_subscription_canceled_notification; }
    void set_mention_notifications(bool value) { m_mention_notifications = std::move(value); }
    [[nodiscard]] const bool& get_mention_notifications() const noexcept { return m_mention_notifications; }
    void set_recommendation_notifications(bool value) { m_recommendation_notifications = std::move(value); }
    [[nodiscard]] const bool& get_recommendation_notifications() const noexcept { return m_recommendation_notifications; }
    void set_milestone_notifications(bool value) { m_milestone_notifications = std::move(value); }
    [[nodiscard]] const bool& get_milestone_notifications() const noexcept { return m_milestone_notifications; }
    void set_donation_notifications(bool value) { m_donation_notifications = std::move(value); }
    [[nodiscard]] const bool& get_donation_notifications() const noexcept { return m_donation_notifications; }
    void set_gift_subscription_notifications(bool value) { m_gift_subscription_notifications = std::move(value); }
    [[nodiscard]] const bool& get_gift_subscription_notifications() const noexcept { return m_gift_subscription_notifications; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    [[nodiscard]] std::string wire_id() const { return serde::ghost_id(m_id); }
    [[nodiscard]] static std::string wire_accessibility() { return "Ghost Author"; }
    [[nodiscard]] std::string wire_description() const { return m_bio; }
    [[nodiscard]] serde::RawJson wire_timestamp() const { return {R"("2024-01-01T00:00:00.000Z")"}; }
    void set_wire_noop(serde::RawJson) {}
    void set_wire_string_noop(std::string) {}
    [[nodiscard]] std::string wire_accessibility_value() const { return "Ghost Author"; }

    std::int64_t m_id{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_password{};
    std::string m_email{};
    std::string m_profile_image{};
    std::string m_cover_image{};
    std::string m_bio{};
    std::string m_website{};
    std::string m_location{};
    std::string m_facebook{};
    std::string m_twitter{};
    std::string m_threads{};
    std::string m_bluesky{};
    std::string m_mastodon{};
    std::string m_tiktok{};
    std::string m_youtube{};
    std::string m_instagram{};
    std::string m_linkedin{};
    bool m_accessibility{};
    std::string m_status{};
    std::string m_locale{};
    std::string m_visibility{};
    std::string m_meta_title{};
    std::string m_meta_description{};
    bool m_tour{};
    std::chrono::system_clock::time_point m_last_seen{};
    bool m_comment_notifications{};
    std::string m_free_member_signup_notification{};
    std::string m_paid_subscription_started_notification{};
    std::string m_paid_subscription_canceled_notification{};
    bool m_mention_notifications{};
    bool m_recommendation_notifications{};
    bool m_milestone_notifications{};
    bool m_donation_notifications{};
    bool m_gift_subscription_notifications{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::User>
{
    using M = blog::model::User;
    static constexpr auto fields()
    {
        return std::tuple{
            FieldDesc<"id", &M::wire_id, &M::set_id>{},
            FieldDesc<"name", &M::get_name, &M::set_name>{},
            FieldDesc<"slug", &M::get_slug, &M::set_slug>{},
            FieldDesc<"email", &M::get_email, &M::set_email>{},
            FieldDesc<"accessibility", &M::wire_accessibility_value, &M::set_wire_string_noop>{},
            FieldDesc<"description", &M::wire_description, &M::set_wire_string_noop>{},
            FieldDesc<"created_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"updated_at", &M::wire_timestamp, &M::set_wire_noop>{}
        };
    }
    static constexpr auto db_fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::User::get_id, &blog::model::User::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"name", &blog::model::User::get_name, &blog::model::User::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::User::get_slug, &blog::model::User::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &blog::model::User::get_password, &blog::model::User::set_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &blog::model::User::get_email, &blog::model::User::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_image", &blog::model::User::get_profile_image, &blog::model::User::set_profile_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"cover_image", &blog::model::User::get_cover_image, &blog::model::User::set_cover_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"bio", &blog::model::User::get_bio, &blog::model::User::set_bio, serde::FieldOptions{}>{},
            serde::FieldDesc<"website", &blog::model::User::get_website, &blog::model::User::set_website, serde::FieldOptions{}>{},
            serde::FieldDesc<"location", &blog::model::User::get_location, &blog::model::User::set_location, serde::FieldOptions{}>{},
            serde::FieldDesc<"facebook", &blog::model::User::get_facebook, &blog::model::User::set_facebook, serde::FieldOptions{}>{},
            serde::FieldDesc<"twitter", &blog::model::User::get_twitter, &blog::model::User::set_twitter, serde::FieldOptions{}>{},
            serde::FieldDesc<"threads", &blog::model::User::get_threads, &blog::model::User::set_threads, serde::FieldOptions{}>{},
            serde::FieldDesc<"bluesky", &blog::model::User::get_bluesky, &blog::model::User::set_bluesky, serde::FieldOptions{}>{},
            serde::FieldDesc<"mastodon", &blog::model::User::get_mastodon, &blog::model::User::set_mastodon, serde::FieldOptions{}>{},
            serde::FieldDesc<"tiktok", &blog::model::User::get_tiktok, &blog::model::User::set_tiktok, serde::FieldOptions{}>{},
            serde::FieldDesc<"youtube", &blog::model::User::get_youtube, &blog::model::User::set_youtube, serde::FieldOptions{}>{},
            serde::FieldDesc<"instagram", &blog::model::User::get_instagram, &blog::model::User::set_instagram, serde::FieldOptions{}>{},
            serde::FieldDesc<"linkedin", &blog::model::User::get_linkedin, &blog::model::User::set_linkedin, serde::FieldOptions{}>{},
            serde::FieldDesc<"accessibility", &blog::model::User::get_accessibility, &blog::model::User::set_accessibility, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::User::get_status, &blog::model::User::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"locale", &blog::model::User::get_locale, &blog::model::User::set_locale, serde::FieldOptions{}>{},
            serde::FieldDesc<"visibility", &blog::model::User::get_visibility, &blog::model::User::set_visibility, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta_title", &blog::model::User::get_meta_title, &blog::model::User::set_meta_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta_description", &blog::model::User::get_meta_description, &blog::model::User::set_meta_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"tour", &blog::model::User::get_tour, &blog::model::User::set_tour, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_seen", &blog::model::User::get_last_seen, &blog::model::User::set_last_seen, serde::FieldOptions{}>{},
            serde::FieldDesc<"comment_notifications", &blog::model::User::get_comment_notifications, &blog::model::User::set_comment_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"free_member_signup_notification", &blog::model::User::get_free_member_signup_notification, &blog::model::User::set_free_member_signup_notification, serde::FieldOptions{}>{},
            serde::FieldDesc<"paid_subscription_started_notification", &blog::model::User::get_paid_subscription_started_notification, &blog::model::User::set_paid_subscription_started_notification, serde::FieldOptions{}>{},
            serde::FieldDesc<"paid_subscription_canceled_notification", &blog::model::User::get_paid_subscription_canceled_notification, &blog::model::User::set_paid_subscription_canceled_notification, serde::FieldOptions{}>{},
            serde::FieldDesc<"mention_notifications", &blog::model::User::get_mention_notifications, &blog::model::User::set_mention_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"recommendation_notifications", &blog::model::User::get_recommendation_notifications, &blog::model::User::set_recommendation_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"milestone_notifications", &blog::model::User::get_milestone_notifications, &blog::model::User::set_milestone_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"donation_notifications", &blog::model::User::get_donation_notifications, &blog::model::User::set_donation_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"gift_subscription_notifications", &blog::model::User::get_gift_subscription_notifications, &blog::model::User::set_gift_subscription_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::User::get_created_at, &blog::model::User::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::User::get_updated_at, &blog::model::User::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "users"; }
};
} // namespace serde


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
