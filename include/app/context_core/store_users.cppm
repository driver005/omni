module;
export module app_context_core:store_users;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::users {

/** @brief Persisted user mirror of the upstream User api model. */
class UserRecord
{
public:
    UserRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_user_id(std::string value)
    {
        m_user_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_user_id() const noexcept
    {
        return m_user_id;
    }

    void set_email(std::string value)
    {
        m_email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return m_email;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_role(std::string value)
    {
        m_role = std::move(value);
    }

    [[nodiscard]] const std::string& get_role() const noexcept
    {
        return m_role;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_user_id{};
    std::string m_email{};
    std::string m_name{};
    std::string m_status{};
    std::string m_role{};
    std::string m_merchant_id{};
    std::string m_created_at{};
};

} // namespace app::store::users

template<>
struct serde::Serializable<app::store::users::UserRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::users::UserRecord::get_id,
                &app::store::users::UserRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "user_id",
                &app::store::users::UserRecord::get_user_id,
                &app::store::users::UserRecord::set_user_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::store::users::UserRecord::get_email,
                &app::store::users::UserRecord::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::store::users::UserRecord::get_name,
                &app::store::users::UserRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::users::UserRecord::get_status,
                &app::store::users::UserRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "role",
                &app::store::users::UserRecord::get_role,
                &app::store::users::UserRecord::set_role,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::users::UserRecord::get_merchant_id,
                &app::store::users::UserRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::users::UserRecord::get_created_at,
                &app::store::users::UserRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "users";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::users::tests {

using namespace boost::ut;

suite<"app_store_user_record"> user_record_suite = []
{
    "stores user fields and reports its table"_test = []
    {
        UserRecord record{};
        record.set_user_id("usr_test");
        record.set_email("admin@example.com");
        record.set_status("active");

        expect(record.get_user_id() == "usr_test");
        expect(record.get_email() == "admin@example.com");
        expect(record.get_status() == "active");
        expect(serde::Serializable<UserRecord>::table_name() == "users");
    };
};

} // namespace app::store::users::tests
#endif
