module;
export module app_dto:superposition_sdk_config;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::superposition_sdk_config {

// Upstream superposition_types::Config (superposition_types 0.119.0):
//   pub struct Config {
//       pub contexts: Vec<Context>,
//       pub overrides: HashMap<String, Overrides>,
//       pub default_configs: ExtendedMap,
//       pub dimensions: HashMap<String, DimensionInfo>,
//   }
// The structured fields are modelled as serde::Value (rfl::Generic) so the JSON wire shape
// matches the upstream object instead of wrapping the raw JSON in a string.
class Config
{
public:
    void set_contexts(serde::Value value)
    {
        contexts = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_contexts() const noexcept
    {
        return contexts;
    }

    void set_overrides(serde::Value value)
    {
        overrides = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_overrides() const noexcept
    {
        return overrides;
    }

    void set_default_configs(serde::Value value)
    {
        default_configs = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_default_configs() const noexcept
    {
        return default_configs;
    }

    void set_dimensions(serde::Value value)
    {
        dimensions = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_dimensions() const noexcept
    {
        return dimensions;
    }

    serde::Value contexts{serde::Value::Array{}};
    serde::Value overrides{serde::Value::Object{}};
    serde::Value default_configs{serde::Value::Object{}};
    serde::Value dimensions{serde::Value::Object{}};
};

enum class PaymentMethodCriteria
{
    CardNetwork,
    BankName,
    PaymentExperience,
};

class PaymentMethodTypeValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SdkCriteriaRule
{
public:
    void set_criteria_value(std::string value)
    {
        criteria_value = std::move(value);
    }

    [[nodiscard]] const std::string& get_criteria_value() const noexcept
    {
        return criteria_value;
    }

    void set_eligible_connectors(std::vector<std::string> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    std::string criteria_value{};
    std::vector<std::string> eligible_connectors{};
};

class SecretStringValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class VaultingAction
{
    Tokenize,
    Skip,
};

class ProfileAccountConfig
{
public:
    void set_collect_shipping_details_from_wallet_connector(bool value)
    {
        collect_shipping_details_from_wallet_connector = std::move(value);
    }

    [[nodiscard]] const bool& get_collect_shipping_details_from_wallet_connector() const noexcept
    {
        return collect_shipping_details_from_wallet_connector;
    }

    void set_collect_billing_details_from_wallet_connector(bool value)
    {
        collect_billing_details_from_wallet_connector = std::move(value);
    }

    [[nodiscard]] const bool& get_collect_billing_details_from_wallet_connector() const noexcept
    {
        return collect_billing_details_from_wallet_connector;
    }

    void set_always_collect_billing_details_from_wallet_connector(bool value)
    {
        always_collect_billing_details_from_wallet_connector = std::move(value);
    }

    [[nodiscard]] const bool&
    get_always_collect_billing_details_from_wallet_connector() const noexcept
    {
        return always_collect_billing_details_from_wallet_connector;
    }

    void set_always_collect_shipping_details_from_wallet_connector(bool value)
    {
        always_collect_shipping_details_from_wallet_connector = std::move(value);
    }

    [[nodiscard]] const bool&
    get_always_collect_shipping_details_from_wallet_connector() const noexcept
    {
        return always_collect_shipping_details_from_wallet_connector;
    }

    void set_vaulting_action(VaultingAction value)
    {
        vaulting_action = std::move(value);
    }

    [[nodiscard]] const VaultingAction& get_vaulting_action() const noexcept
    {
        return vaulting_action;
    }

    bool collect_shipping_details_from_wallet_connector{};
    bool collect_billing_details_from_wallet_connector{};
    bool always_collect_billing_details_from_wallet_connector{};
    bool always_collect_shipping_details_from_wallet_connector{};
    VaultingAction vaulting_action{};
};

class SdkConfigRequest
{
public:
    void set_client_secret(std::optional<SecretStringValue> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    std::optional<SecretStringValue> client_secret{};
};

class SdkPaymentMethodType
{
public:
    void set_payment_method_type(PaymentMethodTypeValue value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const PaymentMethodTypeValue& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_criteria(std::optional<PaymentMethodCriteria> value)
    {
        payment_method_criteria = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodCriteria>&
    get_payment_method_criteria() const noexcept
    {
        return payment_method_criteria;
    }

    void set_criteria_rules(std::vector<SdkCriteriaRule> value)
    {
        criteria_rules = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkCriteriaRule>& get_criteria_rules() const noexcept
    {
        return criteria_rules;
    }

    PaymentMethodTypeValue payment_method_type{};
    std::optional<PaymentMethodCriteria> payment_method_criteria{};
    std::vector<SdkCriteriaRule> criteria_rules{};
};

class AccountConfig
{
public:
    void set_profile(ProfileAccountConfig value)
    {
        profile = std::move(value);
    }

    [[nodiscard]] const ProfileAccountConfig& get_profile() const noexcept
    {
        return profile;
    }

    ProfileAccountConfig profile{};
};

class SdkPaymentMethod
{
public:
    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_types(std::vector<SdkPaymentMethodType> value)
    {
        payment_method_types = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkPaymentMethodType>& get_payment_method_types() const noexcept
    {
        return payment_method_types;
    }

    PaymentMethodValue payment_method{};
    std::vector<SdkPaymentMethodType> payment_method_types{};
};

class SuperPositionConfigResponse
{
public:
    void set_raw_configs(Config value)
    {
        raw_configs = std::move(value);
    }

    [[nodiscard]] const Config& get_raw_configs() const noexcept
    {
        return raw_configs;
    }

    void set_resolved_configs(std::optional<serde::Value> value)
    {
        resolved_configs = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_resolved_configs() const noexcept
    {
        return resolved_configs;
    }

    void set_context_used(serde::Value value)
    {
        context_used = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_context_used() const noexcept
    {
        return context_used;
    }

    void set_payment_methods(std::optional<std::vector<SdkPaymentMethod>> value)
    {
        payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SdkPaymentMethod>>&
    get_payment_methods() const noexcept
    {
        return payment_methods;
    }

    void set_account_config(AccountConfig value)
    {
        account_config = std::move(value);
    }

    [[nodiscard]] const AccountConfig& get_account_config() const noexcept
    {
        return account_config;
    }

    Config raw_configs{};
    std::optional<serde::Value> resolved_configs{};
    serde::Value context_used{};
    std::optional<std::vector<SdkPaymentMethod>> payment_methods{};
    AccountConfig account_config{};
};
} // namespace app::dto::superposition_sdk_config

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::SdkConfigRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "client_secret",
            &app::dto::superposition_sdk_config::SdkConfigRequest::get_client_secret,
            &app::dto::superposition_sdk_config::SdkConfigRequest::set_client_secret,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_config_request";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::Config>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "contexts",
                &app::dto::superposition_sdk_config::Config::get_contexts,
                &app::dto::superposition_sdk_config::Config::set_contexts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "overrides",
                &app::dto::superposition_sdk_config::Config::get_overrides,
                &app::dto::superposition_sdk_config::Config::set_overrides,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_configs",
                &app::dto::superposition_sdk_config::Config::get_default_configs,
                &app::dto::superposition_sdk_config::Config::set_default_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dimensions",
                &app::dto::superposition_sdk_config::Config::get_dimensions,
                &app::dto::superposition_sdk_config::Config::set_dimensions,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::SdkCriteriaRule>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "criteria_value",
                &app::dto::superposition_sdk_config::SdkCriteriaRule::get_criteria_value,
                &app::dto::superposition_sdk_config::SdkCriteriaRule::set_criteria_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_connectors",
                &app::dto::superposition_sdk_config::SdkCriteriaRule::get_eligible_connectors,
                &app::dto::superposition_sdk_config::SdkCriteriaRule::set_eligible_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_criteria_rule";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::SdkPaymentMethodType>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method_type",
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::get_payment_method_type,
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_criteria",
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::
                    get_payment_method_criteria,
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::
                    set_payment_method_criteria,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "criteria_rules",
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::get_criteria_rules,
                &app::dto::superposition_sdk_config::SdkPaymentMethodType::set_criteria_rules,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_payment_method_type";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::SdkPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::dto::superposition_sdk_config::SdkPaymentMethod::get_payment_method,
                &app::dto::superposition_sdk_config::SdkPaymentMethod::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_types",
                &app::dto::superposition_sdk_config::SdkPaymentMethod::get_payment_method_types,
                &app::dto::superposition_sdk_config::SdkPaymentMethod::set_payment_method_types,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_payment_method";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::ProfileAccountConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "collect_shipping_details_from_wallet_connector",
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    get_collect_shipping_details_from_wallet_connector,
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    set_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_billing_details_from_wallet_connector",
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    get_collect_billing_details_from_wallet_connector,
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    set_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_billing_details_from_wallet_connector",
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    get_always_collect_billing_details_from_wallet_connector,
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    set_always_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_shipping_details_from_wallet_connector",
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    get_always_collect_shipping_details_from_wallet_connector,
                &app::dto::superposition_sdk_config::ProfileAccountConfig::
                    set_always_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "vaulting_action",
                &app::dto::superposition_sdk_config::ProfileAccountConfig::get_vaulting_action,
                &app::dto::superposition_sdk_config::ProfileAccountConfig::set_vaulting_action,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_account_config";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::AccountConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "profile",
            &app::dto::superposition_sdk_config::AccountConfig::get_profile,
            &app::dto::superposition_sdk_config::AccountConfig::set_profile,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "account_config";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_sdk_config::SuperPositionConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "raw_configs",
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::get_raw_configs,
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::set_raw_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resolved_configs",
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    get_resolved_configs,
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    set_resolved_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "context_used",
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::get_context_used,
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::set_context_used,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods",
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    get_payment_methods,
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    set_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_config",
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    get_account_config,
                &app::dto::superposition_sdk_config::SuperPositionConfigResponse::
                    set_account_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "super_position_config_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::superposition_sdk_config::tests {
using namespace boost::ut;
suite<app_dto_superposition_sdk_config> app_dto_superposition_sdk_config_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::superposition_sdk_config::SdkConfigRequest>::fields())> > 0
        );
    };
};
} // namespace app::dto::superposition_sdk_config::tests
#endif
