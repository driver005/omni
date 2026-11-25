module;
export module app_store:api_key_expiries;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::api_key_expiries {

/** @brief Persisted API key expiry mirror of the upstream ApiKeyExpiration api model. */
class ApiKeyExpiryRecord
{
public:
    ApiKeyExpiryRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_key_id(std::string value)
    {
        m_key_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_key_id() const noexcept
    {
        return m_key_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_expiration(std::string value)
    {
        m_expiration = std::move(value);
    }

    [[nodiscard]] const std::string& get_expiration() const noexcept
    {
        return m_expiration;
    }

    void set_expires_at(std::string value)
    {
        m_expires_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_expires_at() const noexcept
    {
        return m_expires_at;
    }

    void set_revoked(bool value)
    {
        m_revoked = std::move(value);
    }

    [[nodiscard]] const bool& get_revoked() const noexcept
    {
        return m_revoked;
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
    std::string m_key_id{};
    std::string m_merchant_id{};
    std::string m_expiration{};
    std::string m_expires_at{};
    bool m_revoked{};
    std::string m_created_at{};
};

} // namespace app::store::api_key_expiries

template<>
struct serde::Serializable<app::store::api_key_expiries::ApiKeyExpiryRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_id,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "key_id",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_key_id,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_key_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_merchant_id,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiration",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_expiration,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_expiration,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_expires_at,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "revoked",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_revoked,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_revoked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::api_key_expiries::ApiKeyExpiryRecord::get_created_at,
                &app::store::api_key_expiries::ApiKeyExpiryRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_key_expiries";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::api_key_expiries::tests {

using namespace boost::ut;

suite<"app_store_api_key_expiry_record"> api_key_expiry_record_suite = []
{
    "stores API key expiry fields and reports its table"_test = []
    {
        ApiKeyExpiryRecord record{};
        record.set_key_id("key_test");
        record.set_merchant_id("merchant_test");
        record.set_expiration("DateTime");
        record.set_expires_at("2030-01-01T00:00:00Z");

        expect(record.get_key_id() == "key_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_expiration() == "DateTime");
        expect(record.get_expires_at() == "2030-01-01T00:00:00Z");
        expect(serde::Serializable<ApiKeyExpiryRecord>::table_name() == "api_key_expiries");
    };
};

} // namespace app::store::api_key_expiries::tests
#endif
