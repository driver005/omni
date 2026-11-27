module;
export module app_dto:three_ds_decision_rule;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::three_ds_decision_rule {

class CardDiscoveryValue : public core::behavior::Serializer<CardDiscoveryValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CardNetworkValue : public core::behavior::Serializer<CardNetworkValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CountryValue : public core::behavior::Serializer<CountryValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CurrencyValue : public core::behavior::Serializer<CurrencyValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDeviceDisplaySizeValue :
    public core::behavior::Serializer<CustomerDeviceDisplaySizeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDevicePlatformValue : public core::behavior::Serializer<CustomerDevicePlatformValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDeviceTypeValue : public core::behavior::Serializer<CustomerDeviceTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MinorUnitValue : public core::behavior::Serializer<MinorUnitValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RoutingIdValue : public core::behavior::Serializer<RoutingIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

/**
 * @brief Upstream `common_types::three_ds_decision_rule_engine::ThreeDSDecision`.
 * Enumerator names are the upstream `#[serde(rename_all = "snake_case")]` wire values, so
 * rfl's enum-by-name serialization emits the exact upstream JSON strings
 * (no_three_ds, challenge_requested, ...). Source:
 * common_types/three_ds_decision_rule_engine.rs:9-30.
 */
enum class ThreeDSDecision
{
    no_three_ds,
    challenge_requested,
    challenge_preferred,
    no_preference,
    three_ds_exemption_requested_tra,
    three_ds_exemption_requested_low_value,
    issuer_three_ds_exemption_requested,
};

class AcquirerData : public core::behavior::Serializer<AcquirerData>
{
public:
    void set_country(std::optional<CountryValue> value)
    {
        country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryValue>& get_country() const noexcept
    {
        return country;
    }

    void set_fraud_rate(std::optional<double> value)
    {
        fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_fraud_rate() const noexcept
    {
        return fraud_rate;
    }

    std::optional<CountryValue> country{};
    std::optional<double> fraud_rate{};
};

class CustomerDeviceData : public core::behavior::Serializer<CustomerDeviceData>
{
public:
    void set_platform(std::optional<CustomerDevicePlatformValue> value)
    {
        platform = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDevicePlatformValue>& get_platform() const noexcept
    {
        return platform;
    }

    void set_device_type(std::optional<CustomerDeviceTypeValue> value)
    {
        device_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDeviceTypeValue>& get_device_type() const noexcept
    {
        return device_type;
    }

    void set_display_size(std::optional<CustomerDeviceDisplaySizeValue> value)
    {
        display_size = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDeviceDisplaySizeValue>&
    get_display_size() const noexcept
    {
        return display_size;
    }

    std::optional<CustomerDevicePlatformValue> platform{};
    std::optional<CustomerDeviceTypeValue> device_type{};
    std::optional<CustomerDeviceDisplaySizeValue> display_size{};
};

class IssuerData : public core::behavior::Serializer<IssuerData>
{
public:
    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_country(std::optional<CountryValue> value)
    {
        country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryValue>& get_country() const noexcept
    {
        return country;
    }

    std::optional<std::string> name{};
    std::optional<CountryValue> country{};
};

class PaymentData : public core::behavior::Serializer<PaymentData>
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_card_bin(std::optional<std::string> value)
    {
        card_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_bin() const noexcept
    {
        return card_bin;
    }

    void set_extended_card_bin(std::optional<std::string> value)
    {
        extended_card_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_extended_card_bin() const noexcept
    {
        return extended_card_bin;
    }

    void set_business_country(std::optional<CountryValue> value)
    {
        business_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryValue>& get_business_country() const noexcept
    {
        return business_country;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::string> card_bin{};
    std::optional<std::string> extended_card_bin{};
    std::optional<CountryValue> business_country{};
};

class PaymentMethodMetaData : public core::behavior::Serializer<PaymentMethodMetaData>
{
public:
    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_discovery(std::optional<CardDiscoveryValue> value)
    {
        card_discovery = std::move(value);
    }

    [[nodiscard]] const std::optional<CardDiscoveryValue>& get_card_discovery() const noexcept
    {
        return card_discovery;
    }

    std::optional<CardNetworkValue> card_network{};
    std::optional<CardDiscoveryValue> card_discovery{};
};

class ThreeDsDecisionRuleExecuteResponse :
    public core::behavior::Serializer<ThreeDsDecisionRuleExecuteResponse>
{
public:
    void set_decision(ThreeDSDecision value)
    {
        decision = value;
    }

    [[nodiscard]] ThreeDSDecision get_decision() const noexcept
    {
        return decision;
    }

    ThreeDSDecision decision{ThreeDSDecision::no_three_ds};
};

class ThreeDsDecisionRuleExecuteRequest :
    public core::behavior::Serializer<ThreeDsDecisionRuleExecuteRequest>
{
public:
    void set_routing_id(RoutingIdValue value)
    {
        routing_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_routing_id() const noexcept
    {
        return routing_id;
    }

    void set_payment(PaymentData value)
    {
        payment = std::move(value);
    }

    [[nodiscard]] const PaymentData& get_payment() const noexcept
    {
        return payment;
    }

    void set_payment_method(std::optional<PaymentMethodMetaData> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodMetaData>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_customer_device(std::optional<CustomerDeviceData> value)
    {
        customer_device = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDeviceData>& get_customer_device() const noexcept
    {
        return customer_device;
    }

    void set_issuer(std::optional<IssuerData> value)
    {
        issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<IssuerData>& get_issuer() const noexcept
    {
        return issuer;
    }

    void set_acquirer(std::optional<AcquirerData> value)
    {
        acquirer = std::move(value);
    }

    [[nodiscard]] const std::optional<AcquirerData>& get_acquirer() const noexcept
    {
        return acquirer;
    }

    void set_metadata(std::optional<serde::Value> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_metadata() const noexcept
    {
        return metadata;
    }

    RoutingIdValue routing_id{};
    PaymentData payment{};
    std::optional<PaymentMethodMetaData> payment_method{};
    std::optional<CustomerDeviceData> customer_device{};
    std::optional<IssuerData> issuer{};
    std::optional<AcquirerData> acquirer{};
    std::optional<serde::Value> metadata{};
};
} // namespace app::dto::three_ds_decision_rule

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::PaymentData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::dto::three_ds_decision_rule::PaymentData::get_amount,
                &app::dto::three_ds_decision_rule::PaymentData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::dto::three_ds_decision_rule::PaymentData::get_currency,
                &app::dto::three_ds_decision_rule::PaymentData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_bin",
                &app::dto::three_ds_decision_rule::PaymentData::get_card_bin,
                &app::dto::three_ds_decision_rule::PaymentData::set_card_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_card_bin",
                &app::dto::three_ds_decision_rule::PaymentData::get_extended_card_bin,
                &app::dto::three_ds_decision_rule::PaymentData::set_extended_card_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "business_country",
                &app::dto::three_ds_decision_rule::PaymentData::get_business_country,
                &app::dto::three_ds_decision_rule::PaymentData::set_business_country,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_data";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::PaymentMethodMetaData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_network",
                &app::dto::three_ds_decision_rule::PaymentMethodMetaData::get_card_network,
                &app::dto::three_ds_decision_rule::PaymentMethodMetaData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_discovery",
                &app::dto::three_ds_decision_rule::PaymentMethodMetaData::get_card_discovery,
                &app::dto::three_ds_decision_rule::PaymentMethodMetaData::set_card_discovery,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_method_meta_data";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::CustomerDeviceData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "platform",
                &app::dto::three_ds_decision_rule::CustomerDeviceData::get_platform,
                &app::dto::three_ds_decision_rule::CustomerDeviceData::set_platform,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_type",
                &app::dto::three_ds_decision_rule::CustomerDeviceData::get_device_type,
                &app::dto::three_ds_decision_rule::CustomerDeviceData::set_device_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_size",
                &app::dto::three_ds_decision_rule::CustomerDeviceData::get_display_size,
                &app::dto::three_ds_decision_rule::CustomerDeviceData::set_display_size,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_device_data";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::IssuerData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::three_ds_decision_rule::IssuerData::get_name,
                &app::dto::three_ds_decision_rule::IssuerData::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country",
                &app::dto::three_ds_decision_rule::IssuerData::get_country,
                &app::dto::three_ds_decision_rule::IssuerData::set_country,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "issuer_data";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::AcquirerData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "country",
                &app::dto::three_ds_decision_rule::AcquirerData::get_country,
                &app::dto::three_ds_decision_rule::AcquirerData::set_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fraud_rate",
                &app::dto::three_ds_decision_rule::AcquirerData::get_fraud_rate,
                &app::dto::three_ds_decision_rule::AcquirerData::set_fraud_rate,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "acquirer_data";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routing_id",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    get_routing_id,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    set_routing_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::get_payment,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::set_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    get_payment_method,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_device",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    get_customer_device,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::
                    set_customer_device,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::get_issuer,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::set_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::get_acquirer,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::set_acquirer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::get_metadata,
                &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "three_ds_decision_rule_execute_request";
    }
};

template<>
struct serde::Serializable<app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "decision",
            &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse::get_decision,
            &app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse::set_decision,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "three_ds_decision_rule_execute_response";
    }
};

namespace app::dto::three_ds_decision_rule {

/**
 * @brief Explicit wire-name contract for the typed `ThreeDSDecision` enum.
 *
 * Upstream `ThreeDSDecision` uses serde `rename_all = "snake_case"`
 * (common_types/three_ds_decision_rule_engine.rs:13-30), emitting the seven strings below.
 * The production enum already uses those exact snake_case strings as its enumerator names,
 * so rfl's enum-by-name reflection emits them directly and the response DTO stays a real
 * typed DTO (`ThreeDsDecisionRuleExecuteResponse::decision` is `ThreeDSDecision`, not a
 * stringly-typed substitute). This helper makes that wire contract explicit and locally
 * verifiable without depending on an external serializer feature — the same pattern as
 * `app/dto/poll.cppm` `PollStatusWireName`.
 */
class ThreeDSDecisionWireName : public core::behavior::Serializer<ThreeDSDecisionWireName>
{
public:
    [[nodiscard]] constexpr const char* operator()(ThreeDSDecision decision) const noexcept
    {
        switch (decision) {
            case ThreeDSDecision::no_three_ds:
                return "no_three_ds";
            case ThreeDSDecision::challenge_requested:
                return "challenge_requested";
            case ThreeDSDecision::challenge_preferred:
                return "challenge_preferred";
            case ThreeDSDecision::no_preference:
                return "no_preference";
            case ThreeDSDecision::three_ds_exemption_requested_tra:
                return "three_ds_exemption_requested_tra";
            case ThreeDSDecision::three_ds_exemption_requested_low_value:
                return "three_ds_exemption_requested_low_value";
            case ThreeDSDecision::issuer_three_ds_exemption_requested:
                return "issuer_three_ds_exemption_requested";
        }
        return "no_three_ds";
    }
};

} // namespace app::dto::three_ds_decision_rule

#ifdef CONGELADO_TEST
namespace app::dto::three_ds_decision_rule::tests {
using namespace boost::ut;
suite<"app_dto_three_ds_decision_rule"> app_dto_three_ds_decision_rule_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::three_ds_decision_rule::PaymentData>::fields())> >
            0
        );
    };
    "typed decision wire names match upstream snake_case"_test = []
    {
        const ThreeDSDecisionWireName name;
        using E = app::dto::three_ds_decision_rule::ThreeDSDecision;
        expect(std::string_view{name(E::no_three_ds)} == "no_three_ds");
        expect(std::string_view{name(E::challenge_requested)} == "challenge_requested");
        expect(std::string_view{name(E::challenge_preferred)} == "challenge_preferred");
        expect(std::string_view{name(E::no_preference)} == "no_preference");
        expect(
            std::string_view{name(E::three_ds_exemption_requested_tra)} ==
            "three_ds_exemption_requested_tra"
        );
        expect(
            std::string_view{name(E::three_ds_exemption_requested_low_value)} ==
            "three_ds_exemption_requested_low_value"
        );
        expect(
            std::string_view{name(E::issuer_three_ds_exemption_requested)} ==
            "issuer_three_ds_exemption_requested"
        );
    };
    "execute response is a real typed DTO"_test = []
    {
        using R = app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse;
        static_assert(std::is_same_v<
                      decltype(std::declval<R&>().get_decision()),
                      app::dto::three_ds_decision_rule::ThreeDSDecision>);
        expect(std::tuple_size_v<decltype(serde::Serializable<R>::fields())> == 1u);
    };
};
} // namespace app::dto::three_ds_decision_rule::tests
#endif
