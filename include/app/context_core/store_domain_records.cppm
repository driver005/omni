module;
export module app_context_core:store_domain_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::disputes {

/** @brief Persisted dispute record backing the dispute handler family. */
class DisputeRecord
{
public:
    DisputeRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_dispute_id(std::string value)
    {
        m_dispute_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_dispute_id() const noexcept
    {
        return m_dispute_id;
    }

    void set_payment_id(std::string value)
    {
        m_payment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return m_payment_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_connector(std::string value)
    {
        m_connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return m_connector;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_stage(std::string value)
    {
        m_stage = std::move(value);
    }

    [[nodiscard]] const std::string& get_stage() const noexcept
    {
        return m_stage;
    }

    void set_amount(std::int64_t value)
    {
        m_amount = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_amount() const noexcept
    {
        return m_amount;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_reason(std::string value)
    {
        m_reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_reason() const noexcept
    {
        return m_reason;
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
    std::string m_dispute_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_connector{};
    std::string m_status{};
    std::string m_stage{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_reason{};
    std::string m_created_at{};
};

} // namespace app::store::disputes

template<>
struct serde::Serializable<app::store::disputes::DisputeRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::disputes::DisputeRecord::get_id,
                &app::store::disputes::DisputeRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "dispute_id",
                &app::store::disputes::DisputeRecord::get_dispute_id,
                &app::store::disputes::DisputeRecord::set_dispute_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::store::disputes::DisputeRecord::get_payment_id,
                &app::store::disputes::DisputeRecord::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::disputes::DisputeRecord::get_merchant_id,
                &app::store::disputes::DisputeRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::disputes::DisputeRecord::get_profile_id,
                &app::store::disputes::DisputeRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::store::disputes::DisputeRecord::get_connector,
                &app::store::disputes::DisputeRecord::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::disputes::DisputeRecord::get_status,
                &app::store::disputes::DisputeRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stage",
                &app::store::disputes::DisputeRecord::get_stage,
                &app::store::disputes::DisputeRecord::set_stage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::store::disputes::DisputeRecord::get_amount,
                &app::store::disputes::DisputeRecord::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::store::disputes::DisputeRecord::get_currency,
                &app::store::disputes::DisputeRecord::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::store::disputes::DisputeRecord::get_reason,
                &app::store::disputes::DisputeRecord::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::disputes::DisputeRecord::get_created_at,
                &app::store::disputes::DisputeRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "disputes";
    }
};

export namespace app::store::payment_methods {

/** @brief Persisted payment-method record. */
class PaymentMethodRecord
{
public:
    PaymentMethodRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_payment_method_id(std::string value)
    {
        m_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return m_payment_method_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_type(std::string value)
    {
        m_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    void set_network(std::string value)
    {
        m_network = std::move(value);
    }

    [[nodiscard]] const std::string& get_network() const noexcept
    {
        return m_network;
    }

    void set_last4(std::string value)
    {
        m_last4 = std::move(value);
    }

    [[nodiscard]] const std::string& get_last4() const noexcept
    {
        return m_last4;
    }

    void set_vault_id(std::string value)
    {
        m_vault_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_vault_id() const noexcept
    {
        return m_vault_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
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
    std::string m_payment_method_id{};
    std::string m_merchant_id{};
    std::string m_customer_id{};
    std::string m_profile_id{};
    std::string m_type{};
    std::string m_network{};
    std::string m_last4{};
    std::string m_vault_id{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::payment_methods

template<>
struct serde::Serializable<app::store::payment_methods::PaymentMethodRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::payment_methods::PaymentMethodRecord::get_id,
                &app::store::payment_methods::PaymentMethodRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::store::payment_methods::PaymentMethodRecord::get_payment_method_id,
                &app::store::payment_methods::PaymentMethodRecord::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::payment_methods::PaymentMethodRecord::get_merchant_id,
                &app::store::payment_methods::PaymentMethodRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::payment_methods::PaymentMethodRecord::get_customer_id,
                &app::store::payment_methods::PaymentMethodRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::payment_methods::PaymentMethodRecord::get_profile_id,
                &app::store::payment_methods::PaymentMethodRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &app::store::payment_methods::PaymentMethodRecord::get_type,
                &app::store::payment_methods::PaymentMethodRecord::set_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::store::payment_methods::PaymentMethodRecord::get_network,
                &app::store::payment_methods::PaymentMethodRecord::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last4",
                &app::store::payment_methods::PaymentMethodRecord::get_last4,
                &app::store::payment_methods::PaymentMethodRecord::set_last4,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "vault_id",
                &app::store::payment_methods::PaymentMethodRecord::get_vault_id,
                &app::store::payment_methods::PaymentMethodRecord::set_vault_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::payment_methods::PaymentMethodRecord::get_status,
                &app::store::payment_methods::PaymentMethodRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::payment_methods::PaymentMethodRecord::get_created_at,
                &app::store::payment_methods::PaymentMethodRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_methods";
    }
};

export namespace app::store::payment_method_sessions {

/** @brief Persisted payment-method collection session. */
class PaymentMethodSessionRecord
{
public:
    PaymentMethodSessionRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_session_id(std::string value)
    {
        m_session_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_id() const noexcept
    {
        return m_session_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_client_secret(std::string value)
    {
        m_client_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_secret() const noexcept
    {
        return m_client_secret;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_expires_at(std::string value)
    {
        m_expires_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_expires_at() const noexcept
    {
        return m_expires_at;
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
    std::string m_session_id{};
    std::string m_merchant_id{};
    std::string m_client_secret{};
    std::string m_status{};
    std::string m_expires_at{};
    std::string m_created_at{};
};

} // namespace app::store::payment_method_sessions

template<>
struct serde::Serializable<app::store::payment_method_sessions::PaymentMethodSessionRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_id,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "session_id",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_session_id,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_session_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_merchant_id,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_client_secret,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_status,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_expires_at,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::get_created_at,
                &app::store::payment_method_sessions::PaymentMethodSessionRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_sessions";
    }
};

export namespace app::store::profiles {

/** @brief Persisted business profile record. */
class ProfileRecord
{
public:
    ProfileRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_name(std::string value)
    {
        m_profile_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_name() const noexcept
    {
        return m_profile_name;
    }

    void set_type(std::string value)
    {
        m_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
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
    std::string m_profile_id{};
    std::string m_merchant_id{};
    std::string m_profile_name{};
    std::string m_type{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::profiles

template<>
struct serde::Serializable<app::store::profiles::ProfileRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::profiles::ProfileRecord::get_id,
                &app::store::profiles::ProfileRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::profiles::ProfileRecord::get_profile_id,
                &app::store::profiles::ProfileRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::profiles::ProfileRecord::get_merchant_id,
                &app::store::profiles::ProfileRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_name",
                &app::store::profiles::ProfileRecord::get_profile_name,
                &app::store::profiles::ProfileRecord::set_profile_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &app::store::profiles::ProfileRecord::get_type,
                &app::store::profiles::ProfileRecord::set_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::profiles::ProfileRecord::get_status,
                &app::store::profiles::ProfileRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::profiles::ProfileRecord::get_created_at,
                &app::store::profiles::ProfileRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profiles";
    }
};

export namespace app::store::card_issuers {

/** @brief Persisted card issuer configuration. */
class CardIssuerRecord
{
public:
    CardIssuerRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_issuer_id(std::string value)
    {
        m_issuer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_issuer_id() const noexcept
    {
        return m_issuer_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_issuer_type(std::string value)
    {
        m_issuer_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_issuer_type() const noexcept
    {
        return m_issuer_type;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_metadata(std::string value)
    {
        m_metadata = std::move(value);
    }

    [[nodiscard]] const std::string& get_metadata() const noexcept
    {
        return m_metadata;
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
    std::string m_issuer_id{};
    std::string m_name{};
    std::string m_issuer_type{};
    std::string m_status{};
    std::string m_metadata{};
    std::string m_created_at{};
};

} // namespace app::store::card_issuers

template<>
struct serde::Serializable<app::store::card_issuers::CardIssuerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::card_issuers::CardIssuerRecord::get_id,
                &app::store::card_issuers::CardIssuerRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "issuer_id",
                &app::store::card_issuers::CardIssuerRecord::get_issuer_id,
                &app::store::card_issuers::CardIssuerRecord::set_issuer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::store::card_issuers::CardIssuerRecord::get_name,
                &app::store::card_issuers::CardIssuerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_type",
                &app::store::card_issuers::CardIssuerRecord::get_issuer_type,
                &app::store::card_issuers::CardIssuerRecord::set_issuer_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::card_issuers::CardIssuerRecord::get_status,
                &app::store::card_issuers::CardIssuerRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::card_issuers::CardIssuerRecord::get_metadata,
                &app::store::card_issuers::CardIssuerRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::card_issuers::CardIssuerRecord::get_created_at,
                &app::store::card_issuers::CardIssuerRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuers";
    }
};

export namespace app::store::hierarchical_resources {

/** @brief Persisted hierarchical resource link. */
class HierarchicalResourceRecord
{
public:
    HierarchicalResourceRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_resource_id(std::string value)
    {
        m_resource_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_resource_id() const noexcept
    {
        return m_resource_id;
    }

    void set_scope_id(std::string value)
    {
        m_scope_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_scope_id() const noexcept
    {
        return m_scope_id;
    }

    void set_resource_type(std::string value)
    {
        m_resource_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_resource_type() const noexcept
    {
        return m_resource_type;
    }

    void set_data(std::string value)
    {
        m_data = std::move(value);
    }

    [[nodiscard]] const std::string& get_data() const noexcept
    {
        return m_data;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
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
    std::string m_resource_id{};
    std::string m_scope_id{};
    std::string m_resource_type{};
    std::string m_data{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::hierarchical_resources

template<>
struct serde::Serializable<app::store::hierarchical_resources::HierarchicalResourceRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_id,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "resource_id",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_resource_id,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_resource_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope_id",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_scope_id,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_scope_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resource_type",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_resource_type,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_resource_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_data,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_status,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::hierarchical_resources::HierarchicalResourceRecord::get_created_at,
                &app::store::hierarchical_resources::HierarchicalResourceRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchical_resources";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::domain_records::tests {

using namespace boost::ut;

suite<"app_store_domain_records"> domain_records_suite = []
{
    "dispute fields and table"_test = []
    {
        disputes::DisputeRecord record{};
        record.set_dispute_id("dp_1");
        expect(record.get_dispute_id() == "dp_1");
        expect(serde::Serializable<disputes::DisputeRecord>::table_name() == "disputes");
    };

    "payment method fields and table"_test = []
    {
        payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id("pm_1");
        expect(record.get_payment_method_id() == "pm_1");
        expect(
            serde::Serializable<payment_methods::PaymentMethodRecord>::table_name() ==
            "payment_methods"
        );
    };

    "payment method session fields and table"_test = []
    {
        payment_method_sessions::PaymentMethodSessionRecord record{};
        record.set_session_id("pms_1");
        expect(record.get_session_id() == "pms_1");
        expect(
            serde::Serializable<
                payment_method_sessions::PaymentMethodSessionRecord>::table_name() ==
            "payment_method_sessions"
        );
    };

    "profile fields and table"_test = []
    {
        profiles::ProfileRecord record{};
        record.set_profile_id("pro_1");
        expect(record.get_profile_id() == "pro_1");
        expect(serde::Serializable<profiles::ProfileRecord>::table_name() == "profiles");
    };

    "card issuer fields and table"_test = []
    {
        card_issuers::CardIssuerRecord record{};
        record.set_issuer_id("iss_1");
        expect(record.get_issuer_id() == "iss_1");
        expect(serde::Serializable<card_issuers::CardIssuerRecord>::table_name() == "card_issuers");
    };

    "hierarchical resource fields and table"_test = []
    {
        hierarchical_resources::HierarchicalResourceRecord record{};
        record.set_resource_id("hr_1");
        expect(record.get_resource_id() == "hr_1");
        expect(
            serde::Serializable<hierarchical_resources::HierarchicalResourceRecord>::table_name() ==
            "hierarchical_resources"
        );
    };
};

} // namespace app::store::domain_records::tests
#endif
