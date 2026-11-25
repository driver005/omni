export module blog_handlers:comments_model_member;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model::comments {

/** @brief Ghost members entity — full field set from data/schema/schema.js. */
class Member
{
public:
    Member() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_uuid(std::string value)
    {
        m_uuid = std::move(value);
    }

    [[nodiscard]] const std::string& get_uuid() const noexcept
    {
        return m_uuid;
    }

    void set_transient_id(std::string value)
    {
        m_transient_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_transient_id() const noexcept
    {
        return m_transient_id;
    }

    void set_email(std::string value)
    {
        m_email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return m_email;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_expertise(std::string value)
    {
        m_expertise = std::move(value);
    }

    [[nodiscard]] const std::string& get_expertise() const noexcept
    {
        return m_expertise;
    }

    void set_note(std::string value)
    {
        m_note = std::move(value);
    }

    [[nodiscard]] const std::string& get_note() const noexcept
    {
        return m_note;
    }

    void set_geolocation(std::string value)
    {
        m_geolocation = std::move(value);
    }

    [[nodiscard]] const std::string& get_geolocation() const noexcept
    {
        return m_geolocation;
    }

    void set_enable_comment_notifications(bool value)
    {
        m_enable_comment_notifications = std::move(value);
    }

    [[nodiscard]] const bool& get_enable_comment_notifications() const noexcept
    {
        return m_enable_comment_notifications;
    }

    void set_enable_updates_and_announcements(bool value)
    {
        m_enable_updates_and_announcements = std::move(value);
    }

    [[nodiscard]] const bool& get_enable_updates_and_announcements() const noexcept
    {
        return m_enable_updates_and_announcements;
    }

    void set_email_count(std::int64_t value)
    {
        m_email_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_email_count() const noexcept
    {
        return m_email_count;
    }

    void set_email_opened_count(std::int64_t value)
    {
        m_email_opened_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_email_opened_count() const noexcept
    {
        return m_email_opened_count;
    }

    void set_email_open_rate(std::int64_t value)
    {
        m_email_open_rate = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_email_open_rate() const noexcept
    {
        return m_email_open_rate;
    }

    void set_email_disabled(bool value)
    {
        m_email_disabled = std::move(value);
    }

    [[nodiscard]] const bool& get_email_disabled() const noexcept
    {
        return m_email_disabled;
    }

    void set_last_seen_at(std::chrono::system_clock::time_point value)
    {
        m_last_seen_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_last_seen_at() const noexcept
    {
        return m_last_seen_at;
    }

    void set_last_commented_at(std::chrono::system_clock::time_point value)
    {
        m_last_commented_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point&
    get_last_commented_at() const noexcept
    {
        return m_last_commented_at;
    }

    void set_commenting(bool value)
    {
        m_commenting = std::move(value);
    }

    [[nodiscard]] const bool& get_commenting() const noexcept
    {
        return m_commenting;
    }

    void set_created_at(std::chrono::system_clock::time_point value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::chrono::system_clock::time_point value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    // Ghost wire-shape accessors (serde FieldDesc uses these for API JSON).
    [[nodiscard]] std::string wire_uuid() const
    {
        return m_uuid.empty() ? uuid_for(m_id) : m_uuid;
    }

    [[nodiscard]] serde::RawJson wire_note() const
    {
        return {m_note.empty() ? std::string{"null"} : std::string{"\"" + m_note + "\""}};
    }

    [[nodiscard]] serde::RawJson wire_commenting() const
    {
        return {R"({"disabled":false,"disabled_reason":null,"disabled_until":null})"};
    }

    [[nodiscard]] serde::RawJson wire_null() const
    {
        return {"null"};
    }

    [[nodiscard]] serde::RawJson wire_empty_array() const
    {
        return {"[]"};
    }

    [[nodiscard]] bool wire_comped() const
    {
        return false;
    }

    [[nodiscard]] bool wire_can_comment() const
    {
        return true;
    }

    [[nodiscard]] bool wire_subscribed() const
    {
        return m_status == "paid";
    }

    [[nodiscard]] serde::RawJson wire_timestamp() const
    {
        return {R"("2024-01-01T00:00:00.000Z")"};
    }

    [[nodiscard]] std::string wire_unsubscribe_url() const
    {
        return "http://domain.com/unsubscribe/?uuid=memberuuid&key=abc123dontstealme";
    }

    [[nodiscard]] serde::RawJson wire_attribution() const
    {
        return {
            R"({"id":null,"referrer_medium":"Ghost Admin","referrer_source":"Created manually",)"
            R"("referrer_url":null,"title":null,"type":null,"url":null})"
        };
    }

    [[nodiscard]] serde::RawJson wire_email_suppression() const
    {
        return {R"({"info":null,"suppressed":false})"};
    }

    void set_wire_noop(serde::RawJson) {}

    void set_wire_string_noop(std::string) {}

    [[nodiscard]] static std::string ghost_id(std::int64_t value)
    {
        static constexpr std::string_view digits{"0123456789abcdef"};
        std::string out(24, '0');
        auto n = static_cast<std::uint64_t>(value < 0 ? 0 : value);
        for (int i = 23; i >= 0 && n > 0; --i) {
            out[static_cast<std::size_t>(i)] = digits[n & 0xF];
            n >>= 4;
        }
        for (std::size_t i = 0; i < out.size(); ++i) {
            if (out[i] == '0') {
                out[i] = digits[(static_cast<std::uint64_t>(value) + i) & 0xF];
            }
        }
        return out;
    }

    [[nodiscard]] static std::string uuid_for(std::int64_t id)
    {
        const std::string hex = ghost_id(id);
        return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-" +
               hex.substr(16, 4) + "-" + hex.substr(20, 4) + "-" + hex.substr(0, 12);
    }

public:
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


} // namespace blog::model::comments

namespace serde {

inline std::string ghost_id(std::int64_t value)
{
    static constexpr std::string_view digits{"0123456789abcdef"};
    std::string out(24, '0');
    auto n = static_cast<std::uint64_t>(value < 0 ? 0 : value);
    for (int i = 23; i >= 0 && n > 0; --i) {
        out[static_cast<std::size_t>(i)] = digits[n & 0xF];
        n >>= 4;
    }
    for (std::size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '0') {
            out[i] = digits[(static_cast<std::uint64_t>(value) + i) & 0xF];
        }
    }
    return out;
}

inline std::string uuid_for(std::int64_t id)
{
    const std::string hex = ghost_id(id);
    return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-" +
           hex.substr(16, 4) + "-" + hex.substr(20, 4) + "-" + hex.substr(0, 12);
}

struct comments_zero_normalizer
{
    [[nodiscard]] std::int64_t operator()(std::int64_t) const
    {
        return 0;
    }
};

struct comments_wire_id_normalizer
{
    [[nodiscard]] std::string operator()(std::int64_t value) const
    {
        return ghost_id(value);
    }
};

template<>
struct Serializable<blog::model::comments::Member>
{
    using M = blog::model::comments::Member;

    static constexpr auto fields()
    {
        return std::tuple{
            FieldDesc<
                "id",
                &M::get_id,
                &M::set_id,
                FieldOptions::init().with_db(FieldOptionsDb::init().pk()),
                comments_wire_id_normalizer{}>{},
            FieldDesc<"uuid", &M::wire_uuid, &M::set_uuid>{},
            FieldDesc<"email", &M::get_email, &M::set_email>{},
            FieldDesc<"name", &M::get_name, &M::set_name>{},
            FieldDesc<"status", &M::get_status, &M::set_status>{},
            FieldDesc<"note", &M::wire_note, &M::set_wire_string_noop>{},
            FieldDesc<"avatar_image", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"comped", &M::wire_comped, &M::set_wire_noop>{},
            FieldDesc<"can_comment", &M::wire_can_comment, &M::set_wire_noop>{},
            FieldDesc<"subscribed", &M::wire_subscribed, &M::set_wire_noop>{},
            FieldDesc<"email_count", &M::get_email_count, &M::set_email_count>{},
            FieldDesc<
                "email_opened_count",
                &M::get_email_opened_count,
                &M::set_email_opened_count>{},
            FieldDesc<"email_open_rate", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<
                "enable_comment_notifications",
                &M::get_enable_comment_notifications,
                &M::set_enable_comment_notifications>{},
            FieldDesc<"enable_updates_and_announcements", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"geolocation", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"description", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"last_seen_at", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"created_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"updated_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"unsubscribe_url", &M::wire_unsubscribe_url, &M::set_wire_string_noop>{},
            FieldDesc<"attribution", &M::wire_attribution, &M::set_wire_noop>{},
            FieldDesc<"commenting", &M::wire_commenting, &M::set_wire_noop>{},
            FieldDesc<"email_suppression", &M::wire_email_suppression, &M::set_wire_noop>{},
            FieldDesc<"current_subscription", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"labels", &M::wire_empty_array, &M::set_wire_noop>{},
            FieldDesc<"newsletters", &M::wire_empty_array, &M::set_wire_noop>{},
            FieldDesc<"subscriptions", &M::wire_empty_array, &M::set_wire_noop>{},
            FieldDesc<"tiers", &M::wire_empty_array, &M::set_wire_noop>{}
        };
    }

    static constexpr auto db_fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::comments::Member::get_id,
                &blog::model::comments::Member::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "uuid",
                &blog::model::comments::Member::get_uuid,
                &blog::model::comments::Member::set_uuid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transient_id",
                &blog::model::comments::Member::get_transient_id,
                &blog::model::comments::Member::set_transient_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &blog::model::comments::Member::get_email,
                &blog::model::comments::Member::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &blog::model::comments::Member::get_status,
                &blog::model::comments::Member::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &blog::model::comments::Member::get_name,
                &blog::model::comments::Member::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expertise",
                &blog::model::comments::Member::get_expertise,
                &blog::model::comments::Member::set_expertise,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "note",
                &blog::model::comments::Member::get_note,
                &blog::model::comments::Member::set_note,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "geolocation",
                &blog::model::comments::Member::get_geolocation,
                &blog::model::comments::Member::set_geolocation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_comment_notifications",
                &blog::model::comments::Member::get_enable_comment_notifications,
                &blog::model::comments::Member::set_enable_comment_notifications,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_updates_and_announcements",
                &blog::model::comments::Member::get_enable_updates_and_announcements,
                &blog::model::comments::Member::set_enable_updates_and_announcements,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_count",
                &blog::model::comments::Member::get_email_count,
                &blog::model::comments::Member::set_email_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_opened_count",
                &blog::model::comments::Member::get_email_opened_count,
                &blog::model::comments::Member::set_email_opened_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_open_rate",
                &blog::model::comments::Member::get_email_open_rate,
                &blog::model::comments::Member::set_email_open_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_disabled",
                &blog::model::comments::Member::get_email_disabled,
                &blog::model::comments::Member::set_email_disabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_seen_at",
                &blog::model::comments::Member::get_last_seen_at,
                &blog::model::comments::Member::set_last_seen_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_commented_at",
                &blog::model::comments::Member::get_last_commented_at,
                &blog::model::comments::Member::set_last_commented_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "commenting",
                &blog::model::comments::Member::get_commenting,
                &blog::model::comments::Member::set_commenting,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &blog::model::comments::Member::get_created_at,
                &blog::model::comments::Member::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &blog::model::comments::Member::get_updated_at,
                &blog::model::comments::Member::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "members";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::comments::tests {
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
} // namespace blog::model::comments::tests
#endif
