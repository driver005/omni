module;
#include "entity_macros.h"

export module blog_model:member;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost members entity — full field set from data/schema/schema.js. */
class Member
{
public:
    Member() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_uuid(std::string value) { m_uuid = std::move(value); }
    [[nodiscard]] const std::string& get_uuid() const noexcept { return m_uuid; }
    void set_transient_id(std::string value) { m_transient_id = std::move(value); }
    [[nodiscard]] const std::string& get_transient_id() const noexcept { return m_transient_id; }
    void set_email(std::string value) { m_email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return m_email; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_expertise(std::string value) { m_expertise = std::move(value); }
    [[nodiscard]] const std::string& get_expertise() const noexcept { return m_expertise; }
    void set_note(std::string value) { m_note = std::move(value); }
    [[nodiscard]] const std::string& get_note() const noexcept { return m_note; }
    void set_geolocation(std::string value) { m_geolocation = std::move(value); }
    [[nodiscard]] const std::string& get_geolocation() const noexcept { return m_geolocation; }
    void set_enable_comment_notifications(bool value) { m_enable_comment_notifications = std::move(value); }
    [[nodiscard]] const bool& get_enable_comment_notifications() const noexcept { return m_enable_comment_notifications; }
    void set_enable_updates_and_announcements(bool value) { m_enable_updates_and_announcements = std::move(value); }
    [[nodiscard]] const bool& get_enable_updates_and_announcements() const noexcept { return m_enable_updates_and_announcements; }
    void set_email_count(std::int64_t value) { m_email_count = std::move(value); }
    [[nodiscard]] const std::int64_t& get_email_count() const noexcept { return m_email_count; }
    void set_email_opened_count(std::int64_t value) { m_email_opened_count = std::move(value); }
    [[nodiscard]] const std::int64_t& get_email_opened_count() const noexcept { return m_email_opened_count; }
    void set_email_open_rate(std::int64_t value) { m_email_open_rate = std::move(value); }
    [[nodiscard]] const std::int64_t& get_email_open_rate() const noexcept { return m_email_open_rate; }
    void set_email_disabled(bool value) { m_email_disabled = std::move(value); }
    [[nodiscard]] const bool& get_email_disabled() const noexcept { return m_email_disabled; }
    void set_last_seen_at(std::chrono::system_clock::time_point value) { m_last_seen_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_last_seen_at() const noexcept { return m_last_seen_at; }
    void set_last_commented_at(std::chrono::system_clock::time_point value) { m_last_commented_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_last_commented_at() const noexcept { return m_last_commented_at; }
    void set_commenting(bool value) { m_commenting = std::move(value); }
    [[nodiscard]] const bool& get_commenting() const noexcept { return m_commenting; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_uuid{};
    std::string m_transient_id{};
    std::string m_email{};
    std::string m_status{};
    std::string m_name{};
    std::string m_expertise{};
    std::string m_note{};
    std::string m_geolocation{};
    bool m_enable_comment_notifications{};
    bool m_enable_updates_and_announcements{};
    std::int64_t m_email_count{};
    std::int64_t m_email_opened_count{};
    std::int64_t m_email_open_rate{};
    bool m_email_disabled{};
    std::chrono::system_clock::time_point m_last_seen_at{};
    std::chrono::system_clock::time_point m_last_commented_at{};
    bool m_commenting{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Member>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Member::get_id, &blog::model::Member::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"uuid", &blog::model::Member::get_uuid, &blog::model::Member::set_uuid, serde::FieldOptions{}>{},
            serde::FieldDesc<"transient_id", &blog::model::Member::get_transient_id, &blog::model::Member::set_transient_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &blog::model::Member::get_email, &blog::model::Member::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Member::get_status, &blog::model::Member::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::model::Member::get_name, &blog::model::Member::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"expertise", &blog::model::Member::get_expertise, &blog::model::Member::set_expertise, serde::FieldOptions{}>{},
            serde::FieldDesc<"note", &blog::model::Member::get_note, &blog::model::Member::set_note, serde::FieldOptions{}>{},
            serde::FieldDesc<"geolocation", &blog::model::Member::get_geolocation, &blog::model::Member::set_geolocation, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_comment_notifications", &blog::model::Member::get_enable_comment_notifications, &blog::model::Member::set_enable_comment_notifications, serde::FieldOptions{}>{},
            serde::FieldDesc<"enable_updates_and_announcements", &blog::model::Member::get_enable_updates_and_announcements, &blog::model::Member::set_enable_updates_and_announcements, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_count", &blog::model::Member::get_email_count, &blog::model::Member::set_email_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_opened_count", &blog::model::Member::get_email_opened_count, &blog::model::Member::set_email_opened_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_open_rate", &blog::model::Member::get_email_open_rate, &blog::model::Member::set_email_open_rate, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_disabled", &blog::model::Member::get_email_disabled, &blog::model::Member::set_email_disabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_seen_at", &blog::model::Member::get_last_seen_at, &blog::model::Member::set_last_seen_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_commented_at", &blog::model::Member::get_last_commented_at, &blog::model::Member::set_last_commented_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"commenting", &blog::model::Member::get_commenting, &blog::model::Member::set_commenting, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Member::get_created_at, &blog::model::Member::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Member::get_updated_at, &blog::model::Member::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "members"; }
};
} // namespace serde


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
