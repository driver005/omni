module;
export module app_context_core:store_merchant_accounts;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::merchant_accounts {

/** @brief Persisted merchant account mirror of the upstream MerchantAccount api model. */
class MerchantAccountRecord
{
public:
    MerchantAccountRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_merchant_name(std::string value)
    {
        m_merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return m_merchant_name;
    }

    void set_organization_id(std::string value)
    {
        m_organization_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_id() const noexcept
    {
        return m_organization_id;
    }

    void set_parent_merchant_id(std::string value)
    {
        m_parent_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_parent_merchant_id() const noexcept
    {
        return m_parent_merchant_id;
    }

    void set_merchant_account_type(std::string value)
    {
        m_merchant_account_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_account_type() const noexcept
    {
        return m_merchant_account_type;
    }

    void set_return_url(std::string value)
    {
        m_return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return m_return_url;
    }

    void set_publishable_key(std::string value)
    {
        m_publishable_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_publishable_key() const noexcept
    {
        return m_publishable_key;
    }

    void set_locker_id(std::string value)
    {
        m_locker_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_locker_id() const noexcept
    {
        return m_locker_id;
    }

    void set_metadata(std::string value)
    {
        m_metadata = std::move(value);
    }

    [[nodiscard]] const std::string& get_metadata() const noexcept
    {
        return m_metadata;
    }

    void set_routing_algorithm(std::string value)
    {
        m_routing_algorithm = std::move(value);
    }

    [[nodiscard]] const std::string& get_routing_algorithm() const noexcept
    {
        return m_routing_algorithm;
    }

    void set_payout_routing_algorithm(std::string value)
    {
        m_payout_routing_algorithm = std::move(value);
    }

    [[nodiscard]] const std::string& get_payout_routing_algorithm() const noexcept
    {
        return m_payout_routing_algorithm;
    }

    void set_frm_routing_algorithm(std::string value)
    {
        m_frm_routing_algorithm = std::move(value);
    }

    [[nodiscard]] const std::string& get_frm_routing_algorithm() const noexcept
    {
        return m_frm_routing_algorithm;
    }

    void set_default_profile(std::string value)
    {
        m_default_profile = std::move(value);
    }

    [[nodiscard]] const std::string& get_default_profile() const noexcept
    {
        return m_default_profile;
    }

    void set_is_recon_enabled(bool value)
    {
        m_is_recon_enabled = std::move(value);
    }

    [[nodiscard]] const bool& get_is_recon_enabled() const noexcept
    {
        return m_is_recon_enabled;
    }

    void set_sub_merchants_enabled(bool value)
    {
        m_sub_merchants_enabled = std::move(value);
    }

    [[nodiscard]] const bool& get_sub_merchants_enabled() const noexcept
    {
        return m_sub_merchants_enabled;
    }

    void set_enable_payment_response_hash(bool value)
    {
        m_enable_payment_response_hash = std::move(value);
    }

    [[nodiscard]] const bool& get_enable_payment_response_hash() const noexcept
    {
        return m_enable_payment_response_hash;
    }

    void set_payment_response_hash_key(std::string value)
    {
        m_payment_response_hash_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_response_hash_key() const noexcept
    {
        return m_payment_response_hash_key;
    }

    void set_redirect_to_merchant_with_http_post(bool value)
    {
        m_redirect_to_merchant_with_http_post = std::move(value);
    }

    [[nodiscard]] const bool& get_redirect_to_merchant_with_http_post() const noexcept
    {
        return m_redirect_to_merchant_with_http_post;
    }

    void set_recon_status(std::string value)
    {
        m_recon_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_recon_status() const noexcept
    {
        return m_recon_status;
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
    std::string m_merchant_id{};
    std::string m_merchant_name{};
    std::string m_organization_id{};
    std::string m_parent_merchant_id{};
    std::string m_merchant_account_type{};
    std::string m_return_url{};
    std::string m_publishable_key{};
    std::string m_locker_id{};
    std::string m_metadata{};
    std::string m_routing_algorithm{};
    std::string m_payout_routing_algorithm{};
    std::string m_frm_routing_algorithm{};
    std::string m_default_profile{};
    bool m_is_recon_enabled{};
    bool m_sub_merchants_enabled{};
    bool m_enable_payment_response_hash{};
    std::string m_payment_response_hash_key{};
    bool m_redirect_to_merchant_with_http_post{};
    std::string m_recon_status{};
    std::string m_created_at{};
};

} // namespace app::store::merchant_accounts

template<>
struct serde::Serializable<app::store::merchant_accounts::MerchantAccountRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::merchant_accounts::MerchantAccountRecord::get_id,
                &app::store::merchant_accounts::MerchantAccountRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::merchant_accounts::MerchantAccountRecord::get_merchant_id,
                &app::store::merchant_accounts::MerchantAccountRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::store::merchant_accounts::MerchantAccountRecord::get_merchant_name,
                &app::store::merchant_accounts::MerchantAccountRecord::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_id",
                &app::store::merchant_accounts::MerchantAccountRecord::get_organization_id,
                &app::store::merchant_accounts::MerchantAccountRecord::set_organization_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parent_merchant_id",
                &app::store::merchant_accounts::MerchantAccountRecord::get_parent_merchant_id,
                &app::store::merchant_accounts::MerchantAccountRecord::set_parent_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_account_type",
                &app::store::merchant_accounts::MerchantAccountRecord::get_merchant_account_type,
                &app::store::merchant_accounts::MerchantAccountRecord::set_merchant_account_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::store::merchant_accounts::MerchantAccountRecord::get_return_url,
                &app::store::merchant_accounts::MerchantAccountRecord::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "publishable_key",
                &app::store::merchant_accounts::MerchantAccountRecord::get_publishable_key,
                &app::store::merchant_accounts::MerchantAccountRecord::set_publishable_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "locker_id",
                &app::store::merchant_accounts::MerchantAccountRecord::get_locker_id,
                &app::store::merchant_accounts::MerchantAccountRecord::set_locker_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::merchant_accounts::MerchantAccountRecord::get_metadata,
                &app::store::merchant_accounts::MerchantAccountRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm",
                &app::store::merchant_accounts::MerchantAccountRecord::get_routing_algorithm,
                &app::store::merchant_accounts::MerchantAccountRecord::set_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_routing_algorithm",
                &app::store::merchant_accounts::MerchantAccountRecord::get_payout_routing_algorithm,
                &app::store::merchant_accounts::MerchantAccountRecord::set_payout_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_routing_algorithm",
                &app::store::merchant_accounts::MerchantAccountRecord::get_frm_routing_algorithm,
                &app::store::merchant_accounts::MerchantAccountRecord::set_frm_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_profile",
                &app::store::merchant_accounts::MerchantAccountRecord::get_default_profile,
                &app::store::merchant_accounts::MerchantAccountRecord::set_default_profile,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_recon_enabled",
                &app::store::merchant_accounts::MerchantAccountRecord::get_is_recon_enabled,
                &app::store::merchant_accounts::MerchantAccountRecord::set_is_recon_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sub_merchants_enabled",
                &app::store::merchant_accounts::MerchantAccountRecord::get_sub_merchants_enabled,
                &app::store::merchant_accounts::MerchantAccountRecord::set_sub_merchants_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_payment_response_hash",
                &app::store::merchant_accounts::MerchantAccountRecord::
                    get_enable_payment_response_hash,
                &app::store::merchant_accounts::MerchantAccountRecord::
                    set_enable_payment_response_hash,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_response_hash_key",
                &app::store::merchant_accounts::MerchantAccountRecord::
                    get_payment_response_hash_key,
                &app::store::merchant_accounts::MerchantAccountRecord::
                    set_payment_response_hash_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_to_merchant_with_http_post",
                &app::store::merchant_accounts::MerchantAccountRecord::
                    get_redirect_to_merchant_with_http_post,
                &app::store::merchant_accounts::MerchantAccountRecord::
                    set_redirect_to_merchant_with_http_post,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recon_status",
                &app::store::merchant_accounts::MerchantAccountRecord::get_recon_status,
                &app::store::merchant_accounts::MerchantAccountRecord::set_recon_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::merchant_accounts::MerchantAccountRecord::get_created_at,
                &app::store::merchant_accounts::MerchantAccountRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_accounts";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::merchant_accounts::tests {

using namespace boost::ut;

suite<"app_store_merchant_account_record"> merchant_account_record_suite = []
{
    "stores merchant account fields and reports its table"_test = []
    {
        MerchantAccountRecord record{};
        record.set_merchant_id("merchant_test");
        record.set_merchant_name("Merchant Test");
        record.set_organization_id("org_test");

        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_merchant_name() == "Merchant Test");
        expect(record.get_organization_id() == "org_test");
        expect(serde::Serializable<MerchantAccountRecord>::table_name() == "merchant_accounts");
    };
};

} // namespace app::store::merchant_accounts::tests
#endif
