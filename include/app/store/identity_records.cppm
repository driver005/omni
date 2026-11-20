module;
export module app_store:identity_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::roles {

/** @brief Persisted dashboard role definition. */
class RoleRecord
{
public:
    RoleRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_role_id(std::string value) { m_role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return m_role_id; }
    void set_role_name(std::string value) { m_role_name = std::move(value); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return m_role_name; }
    void set_entity_type(std::string value) { m_entity_type = std::move(value); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return m_entity_type; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_organization_id(std::string value) { m_organization_id = std::move(value); }
    [[nodiscard]] const std::string& get_organization_id() const noexcept { return m_organization_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_is_internal(std::int64_t value) { m_is_internal = std::move(value); }
    [[nodiscard]] const std::int64_t& get_is_internal() const noexcept { return m_is_internal; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_role_id{};
    std::string m_role_name{};
    std::string m_entity_type{};
    std::string m_merchant_id{};
    std::string m_organization_id{};
    std::string m_profile_id{};
    std::int64_t m_is_internal{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::roles

template<>
struct serde::Serializable<app::store::roles::RoleRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::roles::RoleRecord::get_id, &app::store::roles::RoleRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"role_id", &app::store::roles::RoleRecord::get_role_id, &app::store::roles::RoleRecord::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::store::roles::RoleRecord::get_role_name, &app::store::roles::RoleRecord::set_role_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::store::roles::RoleRecord::get_entity_type, &app::store::roles::RoleRecord::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::roles::RoleRecord::get_merchant_id, &app::store::roles::RoleRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_id", &app::store::roles::RoleRecord::get_organization_id, &app::store::roles::RoleRecord::set_organization_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::roles::RoleRecord::get_profile_id, &app::store::roles::RoleRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_internal", &app::store::roles::RoleRecord::get_is_internal, &app::store::roles::RoleRecord::set_is_internal, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::roles::RoleRecord::get_status, &app::store::roles::RoleRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::roles::RoleRecord::get_created_at, &app::store::roles::RoleRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "roles"; }
};

export namespace app::store::user_roles {

/** @brief Persisted assignment of a role to a user within an entity scope. */
class UserRoleRecord
{
public:
    UserRoleRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_user_role_id(std::string value) { m_user_role_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_role_id() const noexcept { return m_user_role_id; }
    void set_user_id(std::string value) { m_user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return m_user_id; }
    void set_role_id(std::string value) { m_role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return m_role_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_organization_id(std::string value) { m_organization_id = std::move(value); }
    [[nodiscard]] const std::string& get_organization_id() const noexcept { return m_organization_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_user_role_id{};
    std::string m_user_id{};
    std::string m_role_id{};
    std::string m_merchant_id{};
    std::string m_organization_id{};
    std::string m_profile_id{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::user_roles

template<>
struct serde::Serializable<app::store::user_roles::UserRoleRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::user_roles::UserRoleRecord::get_id, &app::store::user_roles::UserRoleRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"user_role_id", &app::store::user_roles::UserRoleRecord::get_user_role_id, &app::store::user_roles::UserRoleRecord::set_user_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_id", &app::store::user_roles::UserRoleRecord::get_user_id, &app::store::user_roles::UserRoleRecord::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_id", &app::store::user_roles::UserRoleRecord::get_role_id, &app::store::user_roles::UserRoleRecord::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::user_roles::UserRoleRecord::get_merchant_id, &app::store::user_roles::UserRoleRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_id", &app::store::user_roles::UserRoleRecord::get_organization_id, &app::store::user_roles::UserRoleRecord::set_organization_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::user_roles::UserRoleRecord::get_profile_id, &app::store::user_roles::UserRoleRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::user_roles::UserRoleRecord::get_status, &app::store::user_roles::UserRoleRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::user_roles::UserRoleRecord::get_created_at, &app::store::user_roles::UserRoleRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_roles"; }
};

export namespace app::store::authentication_records {

/** @brief Persisted authentication attempt / challenge record. */
class AuthenticationRecord
{
public:
    AuthenticationRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_auth_id(std::string value) { m_auth_id = std::move(value); }
    [[nodiscard]] const std::string& get_auth_id() const noexcept { return m_auth_id; }
    void set_user_id(std::string value) { m_user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return m_user_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_method(std::string value) { m_method = std::move(value); }
    [[nodiscard]] const std::string& get_method() const noexcept { return m_method; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_challenge(std::string value) { m_challenge = std::move(value); }
    [[nodiscard]] const std::string& get_challenge() const noexcept { return m_challenge; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }
    void set_expires_at(std::string value) { m_expires_at = std::move(value); }
    [[nodiscard]] const std::string& get_expires_at() const noexcept { return m_expires_at; }

    std::int64_t m_id{};
    std::string m_auth_id{};
    std::string m_user_id{};
    std::string m_merchant_id{};
    std::string m_method{};
    std::string m_status{};
    std::string m_challenge{};
    std::string m_created_at{};
    std::string m_expires_at{};
};

} // namespace app::store::authentication_records

template<>
struct serde::Serializable<app::store::authentication_records::AuthenticationRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::authentication_records::AuthenticationRecord::get_id, &app::store::authentication_records::AuthenticationRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"auth_id", &app::store::authentication_records::AuthenticationRecord::get_auth_id, &app::store::authentication_records::AuthenticationRecord::set_auth_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_id", &app::store::authentication_records::AuthenticationRecord::get_user_id, &app::store::authentication_records::AuthenticationRecord::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::authentication_records::AuthenticationRecord::get_merchant_id, &app::store::authentication_records::AuthenticationRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"method", &app::store::authentication_records::AuthenticationRecord::get_method, &app::store::authentication_records::AuthenticationRecord::set_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::authentication_records::AuthenticationRecord::get_status, &app::store::authentication_records::AuthenticationRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge", &app::store::authentication_records::AuthenticationRecord::get_challenge, &app::store::authentication_records::AuthenticationRecord::set_challenge, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::authentication_records::AuthenticationRecord::get_created_at, &app::store::authentication_records::AuthenticationRecord::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"expires_at", &app::store::authentication_records::AuthenticationRecord::get_expires_at, &app::store::authentication_records::AuthenticationRecord::set_expires_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authentication_records"; }
};

#ifdef CONGELADO_TEST
namespace app::store::identity_records::tests {

using namespace boost::ut;

suite<"app_store_identity_records"> identity_records_suite = []
{
    "role fields and table"_test = []
    {
        roles::RoleRecord record{};
        record.set_role_id("role_admin");
        record.set_role_name("admin");
        expect(record.get_role_id() == "role_admin");
        expect(serde::Serializable<roles::RoleRecord>::table_name() == "roles");
    };

    "user role fields and table"_test = []
    {
        user_roles::UserRoleRecord record{};
        record.set_user_id("user_1");
        record.set_role_id("role_admin");
        expect(record.get_user_id() == "user_1");
        expect(serde::Serializable<user_roles::UserRoleRecord>::table_name() == "user_roles");
    };

    "authentication record fields and table"_test = []
    {
        authentication_records::AuthenticationRecord record{};
        record.set_auth_id("auth_1");
        record.set_method("TOTP");
        expect(record.get_method() == "TOTP");
        expect(serde::Serializable<authentication_records::AuthenticationRecord>::table_name() == "authentication_records");
    };
};

} // namespace app::store::identity_records::tests
#endif
