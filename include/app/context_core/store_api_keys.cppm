module;
export module app_context_core:store_api_keys;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::api_keys {

/** @brief Persisted API key mirror of the upstream ApiKey api model. */
class ApiKeyRecord
{
public:
    ApiKeyRecord() = default;

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

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_description(std::string value)
    {
        m_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    void set_prefix(std::string value)
    {
        m_prefix = std::move(value);
    }

    [[nodiscard]] const std::string& get_prefix() const noexcept
    {
        return m_prefix;
    }

    void set_api_key(std::string value)
    {
        m_api_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_api_key() const noexcept
    {
        return m_api_key;
    }

    void set_expiration(std::string value)
    {
        m_expiration = std::move(value);
    }

    [[nodiscard]] const std::string& get_expiration() const noexcept
    {
        return m_expiration;
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
    std::string m_name{};
    std::string m_description{};
    std::string m_prefix{};
    std::string m_api_key{};
    std::string m_expiration{};
    bool m_revoked{};
    std::string m_created_at{};
};

} // namespace app::store::api_keys

template<>
struct serde::Serializable<app::store::api_keys::ApiKeyRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::api_keys::ApiKeyRecord::get_id,
                &app::store::api_keys::ApiKeyRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "key_id",
                &app::store::api_keys::ApiKeyRecord::get_key_id,
                &app::store::api_keys::ApiKeyRecord::set_key_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::api_keys::ApiKeyRecord::get_merchant_id,
                &app::store::api_keys::ApiKeyRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::store::api_keys::ApiKeyRecord::get_name,
                &app::store::api_keys::ApiKeyRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::store::api_keys::ApiKeyRecord::get_description,
                &app::store::api_keys::ApiKeyRecord::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "prefix",
                &app::store::api_keys::ApiKeyRecord::get_prefix,
                &app::store::api_keys::ApiKeyRecord::set_prefix,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "api_key",
                &app::store::api_keys::ApiKeyRecord::get_api_key,
                &app::store::api_keys::ApiKeyRecord::set_api_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiration",
                &app::store::api_keys::ApiKeyRecord::get_expiration,
                &app::store::api_keys::ApiKeyRecord::set_expiration,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "revoked",
                &app::store::api_keys::ApiKeyRecord::get_revoked,
                &app::store::api_keys::ApiKeyRecord::set_revoked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::api_keys::ApiKeyRecord::get_created_at,
                &app::store::api_keys::ApiKeyRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_keys";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::api_keys::tests {

using namespace boost::ut;

suite<"app_store_api_key_record"> api_key_record_suite = []
{
    "stores API key fields and reports its table"_test = []
    {
        ApiKeyRecord record{};
        record.set_key_id("key_test");
        record.set_merchant_id("merchant_test");
        record.set_revoked(true);

        expect(record.get_key_id() == "key_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_revoked());
        expect(serde::Serializable<ApiKeyRecord>::table_name() == "api_keys");
    };
};

} // namespace app::store::api_keys::tests
#endif
