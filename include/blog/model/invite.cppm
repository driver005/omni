module;
#include "entity_macros.h"

export module blog_model:invite;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost invites entity — full field set from data/schema/schema.js. */
class Invite
{
public:
    Invite() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_role_id(std::string value) { m_role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return m_role_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_token(std::string value) { m_token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return m_token; }
    void set_email(std::string value) { m_email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return m_email; }
    void set_expires(std::chrono::system_clock::time_point value) { m_expires = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_expires() const noexcept { return m_expires; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_role_id{};
    std::string m_status{};
    std::string m_token{};
    std::string m_email{};
    std::chrono::system_clock::time_point m_expires{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Invite>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Invite::get_id, &blog::model::Invite::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"role_id", &blog::model::Invite::get_role_id, &blog::model::Invite::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Invite::get_status, &blog::model::Invite::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"token", &blog::model::Invite::get_token, &blog::model::Invite::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &blog::model::Invite::get_email, &blog::model::Invite::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"expires", &blog::model::Invite::get_expires, &blog::model::Invite::set_expires, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Invite::get_created_at, &blog::model::Invite::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Invite::get_updated_at, &blog::model::Invite::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "invites"; }
};
} // namespace serde


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
