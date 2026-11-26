module;
export module app_context_core:store_merchant_connectors;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::merchant_connectors {

/** @brief Persisted merchant connector mirror of the upstream MerchantConnector api model. */
class MerchantConnectorRecord
{
public:
    MerchantConnectorRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_merchant_connector_id(std::string value)
    {
        m_merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_connector_id() const noexcept
    {
        return m_merchant_connector_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_connector_name(std::string value)
    {
        m_connector_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_name() const noexcept
    {
        return m_connector_name;
    }

    void set_connector_type(std::string value)
    {
        m_connector_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_type() const noexcept
    {
        return m_connector_type;
    }

    void set_connector_label(std::string value)
    {
        m_connector_label = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_label() const noexcept
    {
        return m_connector_label;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_connector_account_details(std::string value)
    {
        m_connector_account_details = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_account_details() const noexcept
    {
        return m_connector_account_details;
    }

    void set_connector_webhook_details(std::string value)
    {
        m_connector_webhook_details = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_webhook_details() const noexcept
    {
        return m_connector_webhook_details;
    }

    void set_payment_methods_enabled(std::string value)
    {
        m_payment_methods_enabled = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_methods_enabled() const noexcept
    {
        return m_payment_methods_enabled;
    }

    void set_frm_configs(std::string value)
    {
        m_frm_configs = std::move(value);
    }

    [[nodiscard]] const std::string& get_frm_configs() const noexcept
    {
        return m_frm_configs;
    }

    void set_pm_auth_config(std::string value)
    {
        m_pm_auth_config = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_auth_config() const noexcept
    {
        return m_pm_auth_config;
    }

    void set_metadata(std::string value)
    {
        m_metadata = std::move(value);
    }

    [[nodiscard]] const std::string& get_metadata() const noexcept
    {
        return m_metadata;
    }

    void set_applepay_verified_domains(std::string value)
    {
        m_applepay_verified_domains = std::move(value);
    }

    [[nodiscard]] const std::string& get_applepay_verified_domains() const noexcept
    {
        return m_applepay_verified_domains;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_disabled(bool value)
    {
        m_disabled = std::move(value);
    }

    [[nodiscard]] const bool& get_disabled() const noexcept
    {
        return m_disabled;
    }

    void set_test_mode(bool value)
    {
        m_test_mode = std::move(value);
    }

    [[nodiscard]] const bool& get_test_mode() const noexcept
    {
        return m_test_mode;
    }

    void set_business_country(std::string value)
    {
        m_business_country = std::move(value);
    }

    [[nodiscard]] const std::string& get_business_country() const noexcept
    {
        return m_business_country;
    }

    void set_business_label(std::string value)
    {
        m_business_label = std::move(value);
    }

    [[nodiscard]] const std::string& get_business_label() const noexcept
    {
        return m_business_label;
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
    std::string m_merchant_connector_id{};
    std::string m_merchant_id{};
    std::string m_connector_name{};
    std::string m_connector_type{};
    std::string m_connector_label{};
    std::string m_profile_id{};
    std::string m_connector_account_details{};
    std::string m_connector_webhook_details{};
    std::string m_payment_methods_enabled{};
    std::string m_frm_configs{};
    std::string m_pm_auth_config{};
    std::string m_metadata{};
    std::string m_applepay_verified_domains{};
    std::string m_status{};
    bool m_disabled{};
    bool m_test_mode{};
    std::string m_business_country{};
    std::string m_business_label{};
    std::string m_created_at{};
};

} // namespace app::store::merchant_connectors

template<>
struct serde::Serializable<app::store::merchant_connectors::MerchantConnectorRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_id,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    get_merchant_connector_id,
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_merchant_id,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_name",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_connector_name,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_connector_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_type",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_connector_type,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_connector_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_label",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_connector_label,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_connector_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_profile_id,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_account_details",
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    get_connector_account_details,
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    set_connector_account_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_webhook_details",
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    get_connector_webhook_details,
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    set_connector_webhook_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_enabled",
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    get_payment_methods_enabled,
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    set_payment_methods_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_configs",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_frm_configs,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_frm_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pm_auth_config",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_pm_auth_config,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_pm_auth_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_metadata,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "applepay_verified_domains",
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    get_applepay_verified_domains,
                &app::store::merchant_connectors::MerchantConnectorRecord::
                    set_applepay_verified_domains,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_status,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "disabled",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_disabled,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_disabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "test_mode",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_test_mode,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_test_mode,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_country",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_business_country,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_label",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_business_label,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_business_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::merchant_connectors::MerchantConnectorRecord::get_created_at,
                &app::store::merchant_connectors::MerchantConnectorRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_connectors";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::merchant_connectors::tests {

using namespace boost::ut;

suite<"app_store_merchant_connector_record"> merchant_connector_record_suite = []
{
    "stores merchant connector fields and reports its table"_test = []
    {
        MerchantConnectorRecord record{};
        record.set_merchant_connector_id("mca_test");
        record.set_merchant_id("merchant_test");
        record.set_connector_name("stripe");
        record.set_connector_type("payment_processor");

        expect(record.get_merchant_connector_id() == "mca_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_connector_name() == "stripe");
        expect(record.get_connector_type() == "payment_processor");
        expect(serde::Serializable<MerchantConnectorRecord>::table_name() == "merchant_connectors");
    };
};

} // namespace app::store::merchant_connectors::tests
#endif
