module;
export module app_model:open_router;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::open_router {

enum class AlgorithmType
{
    SuccessRate,
    Elimination,
    DebitRouting,
};

class CardNetworkValue : public core::behavior::Serializer<CardNetworkValue>
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

class CardTypeValue : public core::behavior::Serializer<CardTypeValue>
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

class CountryAlpha2Value : public core::behavior::Serializer<CountryAlpha2Value>
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

class CurrencyValue : public core::behavior::Serializer<CurrencyValue>
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

enum class DecisionEngineConfigVariant
{
    SuccessRate,
    Elimination,
};

enum class DecisionEngineDynamicAlgorithmType
{
    SuccessRate,
    Elimination,
};

class DecisionEngineEliminationData :
    public core::behavior::Serializer<DecisionEngineEliminationData>
{
public:
    void set_threshold(double value)
    {
        threshold = std::move(value);
    }

    [[nodiscard]] const double& get_threshold() const noexcept
    {
        return threshold;
    }

    double threshold{};
};

class DecisionEngineGatewayWiseExtraScore :
    public core::behavior::Serializer<DecisionEngineGatewayWiseExtraScore>
{
public:
    void set_gateway_name(std::string value)
    {
        gateway_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_gateway_name() const noexcept
    {
        return gateway_name;
    }

    void set_gateway_sigma_factor(double value)
    {
        gateway_sigma_factor = std::move(value);
    }

    [[nodiscard]] const double& get_gateway_sigma_factor() const noexcept
    {
        return gateway_sigma_factor;
    }

    std::string gateway_name{};
    double gateway_sigma_factor{};
};

class DecisionEngineMerchantCategoryCodeValue :
    public core::behavior::Serializer<DecisionEngineMerchantCategoryCodeValue>
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

enum class GrantLevel
{
    Profile,
    Merchant,
    Org,
};

class HierarchyProfile : public core::behavior::Serializer<HierarchyProfile>
{
public:
    void set_profile_id(std::string value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_profile_name(std::optional<std::string> value)
    {
        profile_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_profile_name() const noexcept
    {
        return profile_name;
    }

    std::string profile_id{};
    std::optional<std::string> profile_name{};
};

class MerchantAccount : public core::behavior::Serializer<MerchantAccount>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_gateway_success_rate_based_decider_input(std::optional<std::string> value)
    {
        gateway_success_rate_based_decider_input = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_gateway_success_rate_based_decider_input() const noexcept
    {
        return gateway_success_rate_based_decider_input;
    }

    std::string merchant_id{};
    std::optional<std::string> gateway_success_rate_based_decider_input{};
};

class MinorUnitValue : public core::behavior::Serializer<MinorUnitValue>
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

class PaymentIdValue : public core::behavior::Serializer<PaymentIdValue>
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

class PaymentMethodValue : public core::behavior::Serializer<PaymentMethodValue>
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

class PriorityLogicData : public core::behavior::Serializer<PriorityLogicData>
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

    void set_status(std::optional<std::string> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept
    {
        return status;
    }

    void set_failure_reason(std::optional<std::string> value)
    {
        failure_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_failure_reason() const noexcept
    {
        return failure_reason;
    }

    std::optional<std::string> name{};
    std::optional<std::string> status{};
    std::optional<std::string> failure_reason{};
};

class ProfileIdValue : public core::behavior::Serializer<ProfileIdValue>
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

enum class RankingAlgorithm
{
    SrBasedRouting,
    PlBasedRouting,
    NtwBasedRouting,
};

class RegulatedNameValue : public core::behavior::Serializer<RegulatedNameValue>
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

enum class TxnStatus
{
    Started,
    AuthenticationFailed,
    JuspayDeclined,
    PendingVbv,
    VBVSuccessful,
    Authorized,
    AuthorizationFailed,
    Charged,
    Authorizing,
    CODInitiated,
    Voided,
    VoidedPostCharge,
    VoidInitiated,
    Nop,
    CaptureInitiated,
    CaptureFailed,
    VoidFailed,
    AutoRefunded,
    PartialCharged,
    ToBeCharged,
    Pending,
    Failure,
    Declined,
};

class UnifiedError : public core::behavior::Serializer<UnifiedError>
{
public:
    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_user_message(std::string value)
    {
        user_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_user_message() const noexcept
    {
        return user_message;
    }

    void set_developer_message(std::string value)
    {
        developer_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_developer_message() const noexcept
    {
        return developer_message;
    }

    std::string code{};
    std::string user_message{};
    std::string developer_message{};
};

class UpdateScoreResponse : public core::behavior::Serializer<UpdateScoreResponse>
{
public:
    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    std::string message{};
};

class CoBadgedCardNetworksInfo : public core::behavior::Serializer<CoBadgedCardNetworksInfo>
{
public:
    void set_network(CardNetworkValue value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const CardNetworkValue& get_network() const noexcept
    {
        return network;
    }

    void set_saving_percentage(std::optional<double> value)
    {
        saving_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_saving_percentage() const noexcept
    {
        return saving_percentage;
    }

    CardNetworkValue network{};
    std::optional<double> saving_percentage{};
};

class DebitRoutingRequestData : public core::behavior::Serializer<DebitRoutingRequestData>
{
public:
    void set_co_badged_card_networks_info(std::vector<CardNetworkValue> value)
    {
        co_badged_card_networks_info = std::move(value);
    }

    [[nodiscard]] const std::vector<CardNetworkValue>&
    get_co_badged_card_networks_info() const noexcept
    {
        return co_badged_card_networks_info;
    }

    void set_issuer_country(CountryAlpha2Value value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_is_regulated(bool value)
    {
        is_regulated = std::move(value);
    }

    [[nodiscard]] const bool& get_is_regulated() const noexcept
    {
        return is_regulated;
    }

    void set_regulated_name(std::optional<RegulatedNameValue> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<RegulatedNameValue>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_card_type(CardTypeValue value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const CardTypeValue& get_card_type() const noexcept
    {
        return card_type;
    }

    std::vector<CardNetworkValue> co_badged_card_networks_info{};
    CountryAlpha2Value issuer_country{};
    bool is_regulated{};
    std::optional<RegulatedNameValue> regulated_name{};
    CardTypeValue card_type{};
};

class DecisionEngineConfigSetupRequest :
    public core::behavior::Serializer<DecisionEngineConfigSetupRequest>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_config(DecisionEngineConfigVariant value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const DecisionEngineConfigVariant& get_config() const noexcept
    {
        return config;
    }

    std::string merchant_id{};
    DecisionEngineConfigVariant config{};
};

class DecisionEngineSRSubLevelInputConfig :
    public core::behavior::Serializer<DecisionEngineSRSubLevelInputConfig>
{
public:
    void set_payment_method_type(std::optional<std::string> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method(std::optional<std::string> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_latency_threshold(std::optional<double> value)
    {
        latency_threshold = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_latency_threshold() const noexcept
    {
        return latency_threshold;
    }

    void set_bucket_size(std::optional<std::int32_t> value)
    {
        bucket_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_bucket_size() const noexcept
    {
        return bucket_size;
    }

    void set_hedging_percent(std::optional<double> value)
    {
        hedging_percent = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_hedging_percent() const noexcept
    {
        return hedging_percent;
    }

    void set_lower_reset_factor(std::optional<double> value)
    {
        lower_reset_factor = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_lower_reset_factor() const noexcept
    {
        return lower_reset_factor;
    }

    void set_upper_reset_factor(std::optional<double> value)
    {
        upper_reset_factor = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_upper_reset_factor() const noexcept
    {
        return upper_reset_factor;
    }

    void set_gateway_extra_score(
        std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>> value
    )
    {
        gateway_extra_score = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>>&
    get_gateway_extra_score() const noexcept
    {
        return gateway_extra_score;
    }

    std::optional<std::string> payment_method_type{};
    std::optional<std::string> payment_method{};
    std::optional<double> latency_threshold{};
    std::optional<std::int32_t> bucket_size{};
    std::optional<double> hedging_percent{};
    std::optional<double> lower_reset_factor{};
    std::optional<double> upper_reset_factor{};
    std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>> gateway_extra_score{};
};

class ErrorResponse : public core::behavior::Serializer<ErrorResponse>
{
public:
    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    void set_error_code(std::string value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::string value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_priority_logic_tag(std::optional<std::string> value)
    {
        priority_logic_tag = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_priority_logic_tag() const noexcept
    {
        return priority_logic_tag;
    }

    void set_filter_wise_gateways(std::optional<serde::Value> value)
    {
        filter_wise_gateways = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_filter_wise_gateways() const noexcept
    {
        return filter_wise_gateways;
    }

    void set_error_info(UnifiedError value)
    {
        error_info = std::move(value);
    }

    [[nodiscard]] const UnifiedError& get_error_info() const noexcept
    {
        return error_info;
    }

    void set_is_dynamic_mga_enabled(bool value)
    {
        is_dynamic_mga_enabled = std::move(value);
    }

    [[nodiscard]] const bool& get_is_dynamic_mga_enabled() const noexcept
    {
        return is_dynamic_mga_enabled;
    }

    std::string status{};
    std::string error_code{};
    std::string error_message{};
    std::optional<std::string> priority_logic_tag{};
    std::optional<serde::Value> filter_wise_gateways{};
    UnifiedError error_info{};
    bool is_dynamic_mga_enabled{};
};

class FetchRoutingConfig : public core::behavior::Serializer<FetchRoutingConfig>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_algorithm(AlgorithmType value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const AlgorithmType& get_algorithm() const noexcept
    {
        return algorithm;
    }

    std::string merchant_id{};
    AlgorithmType algorithm{};
};

class GetDecisionEngineConfigRequest :
    public core::behavior::Serializer<GetDecisionEngineConfigRequest>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_algorithm(DecisionEngineDynamicAlgorithmType value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const DecisionEngineDynamicAlgorithmType& get_algorithm() const noexcept
    {
        return algorithm;
    }

    std::string merchant_id{};
    DecisionEngineDynamicAlgorithmType algorithm{};
};

class HierarchyMerchant : public core::behavior::Serializer<HierarchyMerchant>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_profiles(std::vector<HierarchyProfile> value)
    {
        profiles = std::move(value);
    }

    [[nodiscard]] const std::vector<HierarchyProfile>& get_profiles() const noexcept
    {
        return profiles;
    }

    std::string merchant_id{};
    std::optional<std::string> merchant_name{};
    std::vector<HierarchyProfile> profiles{};
};

class MerchantTokenRequest : public core::behavior::Serializer<MerchantTokenRequest>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_grant_level(std::optional<GrantLevel> value)
    {
        grant_level = std::move(value);
    }

    [[nodiscard]] const std::optional<GrantLevel>& get_grant_level() const noexcept
    {
        return grant_level;
    }

    void set_grant_id(std::optional<std::string> value)
    {
        grant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_grant_id() const noexcept
    {
        return grant_id;
    }

    void set_permissions(std::optional<std::vector<std::string>> value)
    {
        permissions = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_permissions() const noexcept
    {
        return permissions;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    std::string merchant_id{};
    std::optional<GrantLevel> grant_level{};
    std::optional<std::string> grant_id{};
    std::optional<std::vector<std::string>> permissions{};
    std::optional<std::string> email{};
};

class PaymentInfo : public core::behavior::Serializer<PaymentInfo>
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

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

    void set_preferred_connectors(std::optional<std::vector<std::string>> value)
    {
        preferred_connectors = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_preferred_connectors() const noexcept
    {
        return preferred_connectors;
    }

    void set_payment_type(std::string value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_metadata(std::optional<std::string> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_payment_method_type(std::string value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
    }

    PaymentIdValue payment_id{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::vector<std::string>> preferred_connectors{};
    std::string payment_type{};
    std::optional<std::string> metadata{};
    std::string payment_method_type{};
    PaymentMethodValue payment_method{};
    std::optional<std::string> card_isin{};
};

class PriorityLogicOutput : public core::behavior::Serializer<PriorityLogicOutput>
{
public:
    void set_is_enforcement(std::optional<bool> value)
    {
        is_enforcement = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_enforcement() const noexcept
    {
        return is_enforcement;
    }

    void set_gws(std::optional<std::vector<std::string>> value)
    {
        gws = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_gws() const noexcept
    {
        return gws;
    }

    void set_priority_logic_tag(std::optional<std::string> value)
    {
        priority_logic_tag = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_priority_logic_tag() const noexcept
    {
        return priority_logic_tag;
    }

    void set_gateway_reference_ids(
        std::optional<std::unordered_map<std::string, std::string>> value
    )
    {
        gateway_reference_ids = std::move(value);
    }

    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::string>>&
    get_gateway_reference_ids() const noexcept
    {
        return gateway_reference_ids;
    }

    void set_primary_logic(std::optional<PriorityLogicData> value)
    {
        primary_logic = std::move(value);
    }

    [[nodiscard]] const std::optional<PriorityLogicData>& get_primary_logic() const noexcept
    {
        return primary_logic;
    }

    void set_fallback_logic(std::optional<PriorityLogicData> value)
    {
        fallback_logic = std::move(value);
    }

    [[nodiscard]] const std::optional<PriorityLogicData>& get_fallback_logic() const noexcept
    {
        return fallback_logic;
    }

    std::optional<bool> is_enforcement{};
    std::optional<std::vector<std::string>> gws{};
    std::optional<std::string> priority_logic_tag{};
    std::optional<std::unordered_map<std::string, std::string>> gateway_reference_ids{};
    std::optional<PriorityLogicData> primary_logic{};
    std::optional<PriorityLogicData> fallback_logic{};
};

class UpdateScorePayload : public core::behavior::Serializer<UpdateScorePayload>
{
public:
    void set_merchant_id(ProfileIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_gateway(std::string value)
    {
        gateway = std::move(value);
    }

    [[nodiscard]] const std::string& get_gateway() const noexcept
    {
        return gateway;
    }

    void set_status(TxnStatus value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const TxnStatus& get_status() const noexcept
    {
        return status;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    ProfileIdValue merchant_id{};
    std::string gateway{};
    TxnStatus status{};
    PaymentIdValue payment_id{};
};

class CoBadgedCardNetworks : public core::behavior::Serializer<CoBadgedCardNetworks>
{
public:
    void set_value(std::vector<CoBadgedCardNetworksInfo> value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::vector<CoBadgedCardNetworksInfo>& get_value() const noexcept
    {
        return value;
    }

    std::vector<CoBadgedCardNetworksInfo> value{};
};

class CoBadgedCardRequest : public core::behavior::Serializer<CoBadgedCardRequest>
{
public:
    void set_merchant_category_code(DecisionEngineMerchantCategoryCodeValue value)
    {
        merchant_category_code = std::move(value);
    }

    [[nodiscard]] const DecisionEngineMerchantCategoryCodeValue&
    get_merchant_category_code() const noexcept
    {
        return merchant_category_code;
    }

    void set_acquirer_country(CountryAlpha2Value value)
    {
        acquirer_country = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_acquirer_country() const noexcept
    {
        return acquirer_country;
    }

    void set_co_badged_card_data(std::optional<DebitRoutingRequestData> value)
    {
        co_badged_card_data = std::move(value);
    }

    [[nodiscard]] const std::optional<DebitRoutingRequestData>&
    get_co_badged_card_data() const noexcept
    {
        return co_badged_card_data;
    }

    DecisionEngineMerchantCategoryCodeValue merchant_category_code{};
    CountryAlpha2Value acquirer_country{};
    std::optional<DebitRoutingRequestData> co_badged_card_data{};
};

class DecideGatewayResponse : public core::behavior::Serializer<DecideGatewayResponse>
{
public:
    void set_decided_gateway(std::optional<std::string> value)
    {
        decided_gateway = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_decided_gateway() const noexcept
    {
        return decided_gateway;
    }

    void set_gateway_priority_map(std::optional<serde::Value> value)
    {
        gateway_priority_map = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_gateway_priority_map() const noexcept
    {
        return gateway_priority_map;
    }

    void set_filter_wise_gateways(std::optional<serde::Value> value)
    {
        filter_wise_gateways = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_filter_wise_gateways() const noexcept
    {
        return filter_wise_gateways;
    }

    void set_priority_logic_tag(std::optional<std::string> value)
    {
        priority_logic_tag = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_priority_logic_tag() const noexcept
    {
        return priority_logic_tag;
    }

    void set_routing_approach(std::optional<std::string> value)
    {
        routing_approach = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_routing_approach() const noexcept
    {
        return routing_approach;
    }

    void set_gateway_before_evaluation(std::optional<std::string> value)
    {
        gateway_before_evaluation = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_gateway_before_evaluation() const noexcept
    {
        return gateway_before_evaluation;
    }

    void set_priority_logic_output(std::optional<PriorityLogicOutput> value)
    {
        priority_logic_output = std::move(value);
    }

    [[nodiscard]] const std::optional<PriorityLogicOutput>&
    get_priority_logic_output() const noexcept
    {
        return priority_logic_output;
    }

    void set_reset_approach(std::optional<std::string> value)
    {
        reset_approach = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reset_approach() const noexcept
    {
        return reset_approach;
    }

    void set_routing_dimension(std::optional<std::string> value)
    {
        routing_dimension = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_routing_dimension() const noexcept
    {
        return routing_dimension;
    }

    void set_routing_dimension_level(std::optional<std::string> value)
    {
        routing_dimension_level = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_routing_dimension_level() const noexcept
    {
        return routing_dimension_level;
    }

    void set_is_scheduled_outage(std::optional<bool> value)
    {
        is_scheduled_outage = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_scheduled_outage() const noexcept
    {
        return is_scheduled_outage;
    }

    void set_is_dynamic_mga_enabled(std::optional<bool> value)
    {
        is_dynamic_mga_enabled = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_dynamic_mga_enabled() const noexcept
    {
        return is_dynamic_mga_enabled;
    }

    void set_gateway_mga_id_map(std::optional<serde::Value> value)
    {
        gateway_mga_id_map = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_gateway_mga_id_map() const noexcept
    {
        return gateway_mga_id_map;
    }

    std::optional<std::string> decided_gateway{};
    std::optional<serde::Value> gateway_priority_map{};
    std::optional<serde::Value> filter_wise_gateways{};
    std::optional<std::string> priority_logic_tag{};
    std::optional<std::string> routing_approach{};
    std::optional<std::string> gateway_before_evaluation{};
    std::optional<PriorityLogicOutput> priority_logic_output{};
    std::optional<std::string> reset_approach{};
    std::optional<std::string> routing_dimension{};
    std::optional<std::string> routing_dimension_level{};
    std::optional<bool> is_scheduled_outage{};
    std::optional<bool> is_dynamic_mga_enabled{};
    std::optional<serde::Value> gateway_mga_id_map{};
};

class DecisionEngineSuccessRateData :
    public core::behavior::Serializer<DecisionEngineSuccessRateData>
{
public:
    void set_default_latency_threshold(std::optional<double> value)
    {
        default_latency_threshold = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_latency_threshold() const noexcept
    {
        return default_latency_threshold;
    }

    void set_default_bucket_size(std::optional<std::int32_t> value)
    {
        default_bucket_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>& get_default_bucket_size() const noexcept
    {
        return default_bucket_size;
    }

    void set_default_hedging_percent(std::optional<double> value)
    {
        default_hedging_percent = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_hedging_percent() const noexcept
    {
        return default_hedging_percent;
    }

    void set_default_lower_reset_factor(std::optional<double> value)
    {
        default_lower_reset_factor = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_lower_reset_factor() const noexcept
    {
        return default_lower_reset_factor;
    }

    void set_default_upper_reset_factor(std::optional<double> value)
    {
        default_upper_reset_factor = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_upper_reset_factor() const noexcept
    {
        return default_upper_reset_factor;
    }

    void set_default_gateway_extra_score(
        std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>> value
    )
    {
        default_gateway_extra_score = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>>&
    get_default_gateway_extra_score() const noexcept
    {
        return default_gateway_extra_score;
    }

    void set_sub_level_input_config(
        std::optional<std::vector<DecisionEngineSRSubLevelInputConfig>> value
    )
    {
        sub_level_input_config = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DecisionEngineSRSubLevelInputConfig>>&
    get_sub_level_input_config() const noexcept
    {
        return sub_level_input_config;
    }

    std::optional<double> default_latency_threshold{};
    std::optional<std::int32_t> default_bucket_size{};
    std::optional<double> default_hedging_percent{};
    std::optional<double> default_lower_reset_factor{};
    std::optional<double> default_upper_reset_factor{};
    std::optional<std::vector<DecisionEngineGatewayWiseExtraScore>> default_gateway_extra_score{};
    std::optional<std::vector<DecisionEngineSRSubLevelInputConfig>> sub_level_input_config{};
};

class HierarchyOrg : public core::behavior::Serializer<HierarchyOrg>
{
public:
    void set_org_id(std::string value)
    {
        org_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_org_id() const noexcept
    {
        return org_id;
    }

    void set_org_name(std::optional<std::string> value)
    {
        org_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_org_name() const noexcept
    {
        return org_name;
    }

    void set_merchants(std::vector<HierarchyMerchant> value)
    {
        merchants = std::move(value);
    }

    [[nodiscard]] const std::vector<HierarchyMerchant>& get_merchants() const noexcept
    {
        return merchants;
    }

    std::string org_id{};
    std::optional<std::string> org_name{};
    std::vector<HierarchyMerchant> merchants{};
};

class OpenRouterDecideGatewayRequest :
    public core::behavior::Serializer<OpenRouterDecideGatewayRequest>
{
public:
    void set_payment_info(PaymentInfo value)
    {
        payment_info = std::move(value);
    }

    [[nodiscard]] const PaymentInfo& get_payment_info() const noexcept
    {
        return payment_info;
    }

    void set_merchant_id(ProfileIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_eligible_gateway_list(std::optional<std::vector<std::string>> value)
    {
        eligible_gateway_list = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_eligible_gateway_list() const noexcept
    {
        return eligible_gateway_list;
    }

    void set_ranking_algorithm(std::optional<RankingAlgorithm> value)
    {
        ranking_algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<RankingAlgorithm>& get_ranking_algorithm() const noexcept
    {
        return ranking_algorithm;
    }

    PaymentInfo payment_info{};
    ProfileIdValue merchant_id{};
    std::optional<std::vector<std::string>> eligible_gateway_list{};
    std::optional<RankingAlgorithm> ranking_algorithm{};
};

class DebitRoutingOutput : public core::behavior::Serializer<DebitRoutingOutput>
{
public:
    void set_co_badged_card_networks_info(CoBadgedCardNetworks value)
    {
        co_badged_card_networks_info = std::move(value);
    }

    [[nodiscard]] const CoBadgedCardNetworks& get_co_badged_card_networks_info() const noexcept
    {
        return co_badged_card_networks_info;
    }

    void set_issuer_country(CountryAlpha2Value value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_is_regulated(bool value)
    {
        is_regulated = std::move(value);
    }

    [[nodiscard]] const bool& get_is_regulated() const noexcept
    {
        return is_regulated;
    }

    void set_regulated_name(std::optional<RegulatedNameValue> value)
    {
        regulated_name = std::move(value);
    }

    [[nodiscard]] const std::optional<RegulatedNameValue>& get_regulated_name() const noexcept
    {
        return regulated_name;
    }

    void set_card_type(CardTypeValue value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const CardTypeValue& get_card_type() const noexcept
    {
        return card_type;
    }

    CoBadgedCardNetworks co_badged_card_networks_info{};
    CountryAlpha2Value issuer_country{};
    bool is_regulated{};
    std::optional<RegulatedNameValue> regulated_name{};
    CardTypeValue card_type{};
};

class HierarchySyncRequest : public core::behavior::Serializer<HierarchySyncRequest>
{
public:
    void set_orgs(std::vector<HierarchyOrg> value)
    {
        orgs = std::move(value);
    }

    [[nodiscard]] const std::vector<HierarchyOrg>& get_orgs() const noexcept
    {
        return orgs;
    }

    void set_report_stranded(bool value)
    {
        report_stranded = std::move(value);
    }

    [[nodiscard]] const bool& get_report_stranded() const noexcept
    {
        return report_stranded;
    }

    std::vector<HierarchyOrg> orgs{};
    bool report_stranded{};
};

class DecidedGateway : public core::behavior::Serializer<DecidedGateway>
{
public:
    void set_gateway_priority_map(std::optional<std::unordered_map<std::string, double>> value)
    {
        gateway_priority_map = std::move(value);
    }

    [[nodiscard]] const std::optional<std::unordered_map<std::string, double>>&
    get_gateway_priority_map() const noexcept
    {
        return gateway_priority_map;
    }

    void set_debit_routing_output(std::optional<DebitRoutingOutput> value)
    {
        debit_routing_output = std::move(value);
    }

    [[nodiscard]] const std::optional<DebitRoutingOutput>& get_debit_routing_output() const noexcept
    {
        return debit_routing_output;
    }

    void set_routing_approach(std::string value)
    {
        routing_approach = std::move(value);
    }

    [[nodiscard]] const std::string& get_routing_approach() const noexcept
    {
        return routing_approach;
    }

    std::optional<std::unordered_map<std::string, double>> gateway_priority_map{};
    std::optional<DebitRoutingOutput> debit_routing_output{};
    std::string routing_approach{};
};
} // namespace app::model::open_router

template<>
struct serde::Serializable<app::model::open_router::OpenRouterDecideGatewayRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "paymentInfo",
                &app::model::open_router::OpenRouterDecideGatewayRequest::get_payment_info,
                &app::model::open_router::OpenRouterDecideGatewayRequest::set_payment_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchantId",
                &app::model::open_router::OpenRouterDecideGatewayRequest::get_merchant_id,
                &app::model::open_router::OpenRouterDecideGatewayRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligibleGatewayList",
                &app::model::open_router::OpenRouterDecideGatewayRequest::get_eligible_gateway_list,
                &app::model::open_router::OpenRouterDecideGatewayRequest::set_eligible_gateway_list,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rankingAlgorithm",
                &app::model::open_router::OpenRouterDecideGatewayRequest::get_ranking_algorithm,
                &app::model::open_router::OpenRouterDecideGatewayRequest::set_ranking_algorithm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "open_router_decide_gateway_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecideGatewayResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "decided_gateway",
                &app::model::open_router::DecideGatewayResponse::get_decided_gateway,
                &app::model::open_router::DecideGatewayResponse::set_decided_gateway,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_priority_map",
                &app::model::open_router::DecideGatewayResponse::get_gateway_priority_map,
                &app::model::open_router::DecideGatewayResponse::set_gateway_priority_map,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filter_wise_gateways",
                &app::model::open_router::DecideGatewayResponse::get_filter_wise_gateways,
                &app::model::open_router::DecideGatewayResponse::set_filter_wise_gateways,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "priority_logic_tag",
                &app::model::open_router::DecideGatewayResponse::get_priority_logic_tag,
                &app::model::open_router::DecideGatewayResponse::set_priority_logic_tag,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_approach",
                &app::model::open_router::DecideGatewayResponse::get_routing_approach,
                &app::model::open_router::DecideGatewayResponse::set_routing_approach,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_before_evaluation",
                &app::model::open_router::DecideGatewayResponse::get_gateway_before_evaluation,
                &app::model::open_router::DecideGatewayResponse::set_gateway_before_evaluation,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "priority_logic_output",
                &app::model::open_router::DecideGatewayResponse::get_priority_logic_output,
                &app::model::open_router::DecideGatewayResponse::set_priority_logic_output,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reset_approach",
                &app::model::open_router::DecideGatewayResponse::get_reset_approach,
                &app::model::open_router::DecideGatewayResponse::set_reset_approach,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_dimension",
                &app::model::open_router::DecideGatewayResponse::get_routing_dimension,
                &app::model::open_router::DecideGatewayResponse::set_routing_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_dimension_level",
                &app::model::open_router::DecideGatewayResponse::get_routing_dimension_level,
                &app::model::open_router::DecideGatewayResponse::set_routing_dimension_level,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_scheduled_outage",
                &app::model::open_router::DecideGatewayResponse::get_is_scheduled_outage,
                &app::model::open_router::DecideGatewayResponse::set_is_scheduled_outage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_dynamic_mga_enabled",
                &app::model::open_router::DecideGatewayResponse::get_is_dynamic_mga_enabled,
                &app::model::open_router::DecideGatewayResponse::set_is_dynamic_mga_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_mga_id_map",
                &app::model::open_router::DecideGatewayResponse::get_gateway_mga_id_map,
                &app::model::open_router::DecideGatewayResponse::set_gateway_mga_id_map,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decide_gateway_response";
    }
};

template<>
struct serde::Serializable<app::model::open_router::PriorityLogicOutput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "isEnforcement",
                &app::model::open_router::PriorityLogicOutput::get_is_enforcement,
                &app::model::open_router::PriorityLogicOutput::set_is_enforcement,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gws",
                &app::model::open_router::PriorityLogicOutput::get_gws,
                &app::model::open_router::PriorityLogicOutput::set_gws,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "priorityLogicTag",
                &app::model::open_router::PriorityLogicOutput::get_priority_logic_tag,
                &app::model::open_router::PriorityLogicOutput::set_priority_logic_tag,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gatewayReferenceIds",
                &app::model::open_router::PriorityLogicOutput::get_gateway_reference_ids,
                &app::model::open_router::PriorityLogicOutput::set_gateway_reference_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "primaryLogic",
                &app::model::open_router::PriorityLogicOutput::get_primary_logic,
                &app::model::open_router::PriorityLogicOutput::set_primary_logic,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fallbackLogic",
                &app::model::open_router::PriorityLogicOutput::get_fallback_logic,
                &app::model::open_router::PriorityLogicOutput::set_fallback_logic,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "priority_logic_output";
    }
};

template<>
struct serde::Serializable<app::model::open_router::PriorityLogicData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::open_router::PriorityLogicData::get_name,
                &app::model::open_router::PriorityLogicData::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::open_router::PriorityLogicData::get_status,
                &app::model::open_router::PriorityLogicData::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "failure_reason",
                &app::model::open_router::PriorityLogicData::get_failure_reason,
                &app::model::open_router::PriorityLogicData::set_failure_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "priority_logic_data";
    }
};

template<>
struct serde::Serializable<app::model::open_router::PaymentInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "paymentId",
                &app::model::open_router::PaymentInfo::get_payment_id,
                &app::model::open_router::PaymentInfo::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::open_router::PaymentInfo::get_amount,
                &app::model::open_router::PaymentInfo::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::open_router::PaymentInfo::get_currency,
                &app::model::open_router::PaymentInfo::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "preferredConnectors",
                &app::model::open_router::PaymentInfo::get_preferred_connectors,
                &app::model::open_router::PaymentInfo::set_preferred_connectors,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentType",
                &app::model::open_router::PaymentInfo::get_payment_type,
                &app::model::open_router::PaymentInfo::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::open_router::PaymentInfo::get_metadata,
                &app::model::open_router::PaymentInfo::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentMethodType",
                &app::model::open_router::PaymentInfo::get_payment_method_type,
                &app::model::open_router::PaymentInfo::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentMethod",
                &app::model::open_router::PaymentInfo::get_payment_method,
                &app::model::open_router::PaymentInfo::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardIsin",
                &app::model::open_router::PaymentInfo::get_card_isin,
                &app::model::open_router::PaymentInfo::set_card_isin,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_info";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecidedGateway>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gateway_priority_map",
                &app::model::open_router::DecidedGateway::get_gateway_priority_map,
                &app::model::open_router::DecidedGateway::set_gateway_priority_map,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "debit_routing_output",
                &app::model::open_router::DecidedGateway::get_debit_routing_output,
                &app::model::open_router::DecidedGateway::set_debit_routing_output,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_approach",
                &app::model::open_router::DecidedGateway::get_routing_approach,
                &app::model::open_router::DecidedGateway::set_routing_approach,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decided_gateway";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DebitRoutingOutput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "co_badged_card_networks_info",
                &app::model::open_router::DebitRoutingOutput::get_co_badged_card_networks_info,
                &app::model::open_router::DebitRoutingOutput::set_co_badged_card_networks_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::open_router::DebitRoutingOutput::get_issuer_country,
                &app::model::open_router::DebitRoutingOutput::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_regulated",
                &app::model::open_router::DebitRoutingOutput::get_is_regulated,
                &app::model::open_router::DebitRoutingOutput::set_is_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::open_router::DebitRoutingOutput::get_regulated_name,
                &app::model::open_router::DebitRoutingOutput::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::open_router::DebitRoutingOutput::get_card_type,
                &app::model::open_router::DebitRoutingOutput::set_card_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "debit_routing_output";
    }
};

template<>
struct serde::Serializable<app::model::open_router::CoBadgedCardNetworksInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network",
                &app::model::open_router::CoBadgedCardNetworksInfo::get_network,
                &app::model::open_router::CoBadgedCardNetworksInfo::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "saving_percentage",
                &app::model::open_router::CoBadgedCardNetworksInfo::get_saving_percentage,
                &app::model::open_router::CoBadgedCardNetworksInfo::set_saving_percentage,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "co_badged_card_networks_info";
    }
};

template<>
struct serde::Serializable<app::model::open_router::CoBadgedCardNetworks>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::model::open_router::CoBadgedCardNetworks::get_value,
            &app::model::open_router::CoBadgedCardNetworks::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "co_badged_card_networks";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DebitRoutingRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "co_badged_card_networks_info",
                &app::model::open_router::DebitRoutingRequestData::get_co_badged_card_networks_info,
                &app::model::open_router::DebitRoutingRequestData::set_co_badged_card_networks_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::open_router::DebitRoutingRequestData::get_issuer_country,
                &app::model::open_router::DebitRoutingRequestData::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_regulated",
                &app::model::open_router::DebitRoutingRequestData::get_is_regulated,
                &app::model::open_router::DebitRoutingRequestData::set_is_regulated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regulated_name",
                &app::model::open_router::DebitRoutingRequestData::get_regulated_name,
                &app::model::open_router::DebitRoutingRequestData::set_regulated_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::open_router::DebitRoutingRequestData::get_card_type,
                &app::model::open_router::DebitRoutingRequestData::set_card_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "debit_routing_request_data";
    }
};

template<>
struct serde::Serializable<app::model::open_router::CoBadgedCardRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_category_code",
                &app::model::open_router::CoBadgedCardRequest::get_merchant_category_code,
                &app::model::open_router::CoBadgedCardRequest::set_merchant_category_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country",
                &app::model::open_router::CoBadgedCardRequest::get_acquirer_country,
                &app::model::open_router::CoBadgedCardRequest::set_acquirer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "co_badged_card_data",
                &app::model::open_router::CoBadgedCardRequest::get_co_badged_card_data,
                &app::model::open_router::CoBadgedCardRequest::set_co_badged_card_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "co_badged_card_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::UnifiedError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::open_router::UnifiedError::get_code,
                &app::model::open_router::UnifiedError::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "user_message",
                &app::model::open_router::UnifiedError::get_user_message,
                &app::model::open_router::UnifiedError::set_user_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "developer_message",
                &app::model::open_router::UnifiedError::get_developer_message,
                &app::model::open_router::UnifiedError::set_developer_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "unified_error";
    }
};

template<>
struct serde::Serializable<app::model::open_router::ErrorResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::model::open_router::ErrorResponse::get_status,
                &app::model::open_router::ErrorResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::open_router::ErrorResponse::get_error_code,
                &app::model::open_router::ErrorResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::open_router::ErrorResponse::get_error_message,
                &app::model::open_router::ErrorResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "priority_logic_tag",
                &app::model::open_router::ErrorResponse::get_priority_logic_tag,
                &app::model::open_router::ErrorResponse::set_priority_logic_tag,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filter_wise_gateways",
                &app::model::open_router::ErrorResponse::get_filter_wise_gateways,
                &app::model::open_router::ErrorResponse::set_filter_wise_gateways,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_info",
                &app::model::open_router::ErrorResponse::get_error_info,
                &app::model::open_router::ErrorResponse::set_error_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_dynamic_mga_enabled",
                &app::model::open_router::ErrorResponse::get_is_dynamic_mga_enabled,
                &app::model::open_router::ErrorResponse::set_is_dynamic_mga_enabled,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "error_response";
    }
};

template<>
struct serde::Serializable<app::model::open_router::UpdateScorePayload>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchantId",
                &app::model::open_router::UpdateScorePayload::get_merchant_id,
                &app::model::open_router::UpdateScorePayload::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway",
                &app::model::open_router::UpdateScorePayload::get_gateway,
                &app::model::open_router::UpdateScorePayload::set_gateway,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::open_router::UpdateScorePayload::get_status,
                &app::model::open_router::UpdateScorePayload::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentId",
                &app::model::open_router::UpdateScorePayload::get_payment_id,
                &app::model::open_router::UpdateScorePayload::set_payment_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "update_score_payload";
    }
};

template<>
struct serde::Serializable<app::model::open_router::UpdateScoreResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "message",
            &app::model::open_router::UpdateScoreResponse::get_message,
            &app::model::open_router::UpdateScoreResponse::set_message,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "update_score_response";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecisionEngineGatewayWiseExtraScore>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gatewayName",
                &app::model::open_router::DecisionEngineGatewayWiseExtraScore::get_gateway_name,
                &app::model::open_router::DecisionEngineGatewayWiseExtraScore::set_gateway_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gatewaySigmaFactor",
                &app::model::open_router::DecisionEngineGatewayWiseExtraScore::
                    get_gateway_sigma_factor,
                &app::model::open_router::DecisionEngineGatewayWiseExtraScore::
                    set_gateway_sigma_factor,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_engine_gateway_wise_extra_score";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecisionEngineSRSubLevelInputConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "paymentMethodType",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    get_payment_method_type,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentMethod",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::get_payment_method,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "latencyThreshold",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    get_latency_threshold,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    set_latency_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bucketSize",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::get_bucket_size,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::set_bucket_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "hedgingPercent",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::get_hedging_percent,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::set_hedging_percent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "lowerResetFactor",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    get_lower_reset_factor,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    set_lower_reset_factor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "upperResetFactor",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    get_upper_reset_factor,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    set_upper_reset_factor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gatewayExtraScore",
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    get_gateway_extra_score,
                &app::model::open_router::DecisionEngineSRSubLevelInputConfig::
                    set_gateway_extra_score,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_engine_sr_sub_level_input_config";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecisionEngineSuccessRateData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "defaultLatencyThreshold",
                &app::model::open_router::DecisionEngineSuccessRateData::
                    get_default_latency_threshold,
                &app::model::open_router::DecisionEngineSuccessRateData::
                    set_default_latency_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "defaultBucketSize",
                &app::model::open_router::DecisionEngineSuccessRateData::get_default_bucket_size,
                &app::model::open_router::DecisionEngineSuccessRateData::set_default_bucket_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "defaultHedgingPercent",
                &app::model::open_router::DecisionEngineSuccessRateData::
                    get_default_hedging_percent,
                &app::model::open_router::DecisionEngineSuccessRateData::
                    set_default_hedging_percent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "defaultLowerResetFactor",
                &app::model::open_router::DecisionEngineSuccessRateData::
                    get_default_lower_reset_factor,
                &app::model::open_router::DecisionEngineSuccessRateData::
                    set_default_lower_reset_factor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "defaultUpperResetFactor",
                &app::model::open_router::DecisionEngineSuccessRateData::
                    get_default_upper_reset_factor,
                &app::model::open_router::DecisionEngineSuccessRateData::
                    set_default_upper_reset_factor,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "defaultGatewayExtraScore",
                &app::model::open_router::DecisionEngineSuccessRateData::
                    get_default_gateway_extra_score,
                &app::model::open_router::DecisionEngineSuccessRateData::
                    set_default_gateway_extra_score,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "subLevelInputConfig",
                &app::model::open_router::DecisionEngineSuccessRateData::get_sub_level_input_config,
                &app::model::open_router::DecisionEngineSuccessRateData::set_sub_level_input_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_engine_success_rate_data";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecisionEngineEliminationData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "threshold",
            &app::model::open_router::DecisionEngineEliminationData::get_threshold,
            &app::model::open_router::DecisionEngineEliminationData::set_threshold,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "decision_engine_elimination_data";
    }
};

template<>
struct serde::Serializable<app::model::open_router::DecisionEngineConfigSetupRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::DecisionEngineConfigSetupRequest::get_merchant_id,
                &app::model::open_router::DecisionEngineConfigSetupRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config",
                &app::model::open_router::DecisionEngineConfigSetupRequest::get_config,
                &app::model::open_router::DecisionEngineConfigSetupRequest::set_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_engine_config_setup_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::GetDecisionEngineConfigRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::GetDecisionEngineConfigRequest::get_merchant_id,
                &app::model::open_router::GetDecisionEngineConfigRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::model::open_router::GetDecisionEngineConfigRequest::get_algorithm,
                &app::model::open_router::GetDecisionEngineConfigRequest::set_algorithm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_decision_engine_config_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::MerchantAccount>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::MerchantAccount::get_merchant_id,
                &app::model::open_router::MerchantAccount::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_success_rate_based_decider_input",
                &app::model::open_router::MerchantAccount::
                    get_gateway_success_rate_based_decider_input,
                &app::model::open_router::MerchantAccount::
                    set_gateway_success_rate_based_decider_input,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_account";
    }
};

template<>
struct serde::Serializable<app::model::open_router::MerchantTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::MerchantTokenRequest::get_merchant_id,
                &app::model::open_router::MerchantTokenRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "grant_level",
                &app::model::open_router::MerchantTokenRequest::get_grant_level,
                &app::model::open_router::MerchantTokenRequest::set_grant_level,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "grant_id",
                &app::model::open_router::MerchantTokenRequest::get_grant_id,
                &app::model::open_router::MerchantTokenRequest::set_grant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "permissions",
                &app::model::open_router::MerchantTokenRequest::get_permissions,
                &app::model::open_router::MerchantTokenRequest::set_permissions,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::open_router::MerchantTokenRequest::get_email,
                &app::model::open_router::MerchantTokenRequest::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_token_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::HierarchySyncRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "orgs",
                &app::model::open_router::HierarchySyncRequest::get_orgs,
                &app::model::open_router::HierarchySyncRequest::set_orgs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "report_stranded",
                &app::model::open_router::HierarchySyncRequest::get_report_stranded,
                &app::model::open_router::HierarchySyncRequest::set_report_stranded,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchy_sync_request";
    }
};

template<>
struct serde::Serializable<app::model::open_router::HierarchyOrg>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "org_id",
                &app::model::open_router::HierarchyOrg::get_org_id,
                &app::model::open_router::HierarchyOrg::set_org_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "org_name",
                &app::model::open_router::HierarchyOrg::get_org_name,
                &app::model::open_router::HierarchyOrg::set_org_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchants",
                &app::model::open_router::HierarchyOrg::get_merchants,
                &app::model::open_router::HierarchyOrg::set_merchants,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchy_org";
    }
};

template<>
struct serde::Serializable<app::model::open_router::HierarchyMerchant>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::HierarchyMerchant::get_merchant_id,
                &app::model::open_router::HierarchyMerchant::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::open_router::HierarchyMerchant::get_merchant_name,
                &app::model::open_router::HierarchyMerchant::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profiles",
                &app::model::open_router::HierarchyMerchant::get_profiles,
                &app::model::open_router::HierarchyMerchant::set_profiles,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchy_merchant";
    }
};

template<>
struct serde::Serializable<app::model::open_router::HierarchyProfile>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::open_router::HierarchyProfile::get_profile_id,
                &app::model::open_router::HierarchyProfile::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_name",
                &app::model::open_router::HierarchyProfile::get_profile_name,
                &app::model::open_router::HierarchyProfile::set_profile_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchy_profile";
    }
};

template<>
struct serde::Serializable<app::model::open_router::FetchRoutingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::open_router::FetchRoutingConfig::get_merchant_id,
                &app::model::open_router::FetchRoutingConfig::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::model::open_router::FetchRoutingConfig::get_algorithm,
                &app::model::open_router::FetchRoutingConfig::set_algorithm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "fetch_routing_config";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::open_router::tests {
using namespace boost::ut;
suite<"app_model_open_router"> app_model_open_router_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::model::open_router::OpenRouterDecideGatewayRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::open_router::tests
#endif
