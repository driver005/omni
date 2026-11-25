module;
export module app_model:routing;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::routing {

enum class ComparisonType
{
    Equal,
    NotEqual,
    LessThan,
    LessThanEqual,
    GreaterThan,
    GreaterThanEqual,
};

enum class ConnectorSelection
{
    Priority,
    VolumeSplit,
};

enum class ContractBasedTimeScale
{
    Day,
    Month,
};

class CurrentBlockThreshold
{
public:
    void set_duration_in_mins(std::optional<std::uint64_t> value)
    {
        duration_in_mins = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_duration_in_mins() const noexcept
    {
        return duration_in_mins;
    }

    void set_max_total_count(std::optional<std::uint64_t> value)
    {
        max_total_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_max_total_count() const noexcept
    {
        return max_total_count;
    }

    std::optional<std::uint64_t> duration_in_mins{};
    std::optional<std::uint64_t> max_total_count{};
};

class DecisionEngineEliminationDataValue
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

enum class DecisionEngineRoutingTarget
{
    Volume,
    Rule,
    MultiObjective,
    Debit,
};

class DecisionEngineSuccessRateDataValue
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

class DynamicAlgorithmWithTimestamp
{
public:
    void set_algorithm_id(std::optional<std::string> value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_timestamp(std::int64_t value)
    {
        timestamp = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_timestamp() const noexcept
    {
        return timestamp;
    }

    std::optional<std::string> algorithm_id{};
    std::int64_t timestamp{};
};

enum class DynamicRoutingConfigParams
{
    PaymentMethod,
    PaymentMethodType,
    AuthenticationType,
    Currency,
    Country,
    CardNetwork,
    CardBin,
};

enum class DynamicRoutingFeatures
{
    Metrics,
    DynamicConnectorSelection,
    None,
};

enum class DynamicRoutingPayload
{
    SuccessBasedRoutingPayload,
    EliminationRoutingPayload,
};

enum class DynamicRoutingType
{
    SuccessRateBasedRouting,
    EliminationRouting,
    ContractBasedRouting,
};

class DynamicRoutingVolumeSplitQuery
{
public:
    void set_split(std::uint8_t value)
    {
        split = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_split() const noexcept
    {
        return split;
    }

    std::uint8_t split{};
};

class EliminationAnalyserConfig
{
public:
    void set_bucket_size(std::optional<std::uint64_t> value)
    {
        bucket_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_bucket_size() const noexcept
    {
        return bucket_size;
    }

    void set_bucket_leak_interval_in_secs(std::optional<std::uint64_t> value)
    {
        bucket_leak_interval_in_secs = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_bucket_leak_interval_in_secs() const noexcept
    {
        return bucket_leak_interval_in_secs;
    }

    std::optional<std::uint64_t> bucket_size{};
    std::optional<std::uint64_t> bucket_leak_interval_in_secs{};
};

class EliminationRoutingEventBucketConfig
{
public:
    void set_bucket_size(std::optional<std::uint64_t> value)
    {
        bucket_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_bucket_size() const noexcept
    {
        return bucket_size;
    }

    void set_bucket_leak_interval_in_secs(std::optional<std::uint64_t> value)
    {
        bucket_leak_interval_in_secs = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_bucket_leak_interval_in_secs() const noexcept
    {
        return bucket_leak_interval_in_secs;
    }

    std::optional<std::uint64_t> bucket_size{};
    std::optional<std::uint64_t> bucket_leak_interval_in_secs{};
};

class IfStatementValue
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

class LabelWithScoreEventResponse
{
public:
    void set_score(double value)
    {
        score = std::move(value);
    }

    [[nodiscard]] const double& get_score() const noexcept
    {
        return score;
    }

    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    double score{};
    std::string label{};
};

enum class LinkedRoutingConfigRetrieveResponse
{
    MerchantAccountBased,
    ProfileBased,
};

class MerchantConnectorAccountIdValue
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

class MerchantIdValue
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

class MetadataValue
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string key{};
    std::string value{};
};

class ProfileIdValue
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

enum class RoutableChoiceKind
{
    OnlyConnector,
    FullStruct,
};

enum class RoutableChoiceSerde
{
    OnlyConnector,
    FullStruct,
};

class RoutableConnectorsValue
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

enum class RoutingAlgorithmKind
{
    Single,
    Priority,
    VolumeSplit,
    Advanced,
    Dynamic,
    ThreeDsDecisionRule,
};

enum class RoutingAlgorithmSerde
{
    Single,
    Priority,
    VolumeSplit,
    Advanced,
    ThreeDsDecisionRule,
};

enum class RoutingAlgorithmWrapper
{
    Static,
    Dynamic,
};

enum class RoutingApproach
{
    Exploitation,
    Exploration,
    Elimination,
    ContractBased,
    Default,
};

class RoutingConfigName
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

class RoutingEntryResponse
{
public:
    void set_is_cutover(bool value)
    {
        is_cutover = std::move(value);
    }

    [[nodiscard]] const bool& get_is_cutover() const noexcept
    {
        return is_cutover;
    }

    void set_redirect_url(std::optional<std::string> value)
    {
        redirect_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_redirect_url() const noexcept
    {
        return redirect_url;
    }

    bool is_cutover{};
    std::optional<std::string> redirect_url{};
};

class RoutingIdValue
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

enum class RoutingKind
{
    Config,
    RoutingAlgorithm,
};

class RoutingMigrationPageTotals
{
public:
    void set_profiles(std::size_t value)
    {
        profiles = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_profiles() const noexcept
    {
        return profiles;
    }

    void set_pending(std::size_t value)
    {
        pending = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_pending() const noexcept
    {
        return pending;
    }

    void set_partial(std::size_t value)
    {
        partial = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_partial() const noexcept
    {
        return partial;
    }

    void set_migrated(std::size_t value)
    {
        migrated = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_migrated() const noexcept
    {
        return migrated;
    }

    void set_enabled(std::size_t value)
    {
        enabled = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_enabled() const noexcept
    {
        return enabled;
    }

    void set_enabled_without_rules(std::size_t value)
    {
        enabled_without_rules = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_enabled_without_rules() const noexcept
    {
        return enabled_without_rules;
    }

    void set_diverged(std::size_t value)
    {
        diverged = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_diverged() const noexcept
    {
        return diverged;
    }

    void set_unknown(std::size_t value)
    {
        unknown = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_unknown() const noexcept
    {
        return unknown;
    }

    std::size_t profiles{};
    std::size_t pending{};
    std::size_t partial{};
    std::size_t migrated{};
    std::size_t enabled{};
    std::size_t enabled_without_rules{};
    std::size_t diverged{};
    std::size_t unknown{};
};

enum class RoutingMigrationState
{
    Pending,
    Partial,
    Diverged,
    Migrated,
    Enabled,
    EnabledWithoutRules,
    Unknown,
};

class RoutingMigrationStatusQuery
{
public:
    void set_limit(std::optional<std::uint32_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    std::optional<std::uint32_t> limit{};
    std::optional<std::uint32_t> offset{};
};

enum class RoutingResultSource
{
    DecisionEngine,
    HyperswitchRouting,
};

enum class RoutingType
{
    Static,
    Dynamic,
};

class RoutingVolumeSplitResponse
{
public:
    void set_split(std::uint8_t value)
    {
        split = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_split() const noexcept
    {
        return split;
    }

    std::uint8_t split{};
};

class RuleMigrationTotals
{
public:
    void set_profiles(std::size_t value)
    {
        profiles = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_profiles() const noexcept
    {
        return profiles;
    }

    void set_rules_migrated(std::size_t value)
    {
        rules_migrated = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_rules_migrated() const noexcept
    {
        return rules_migrated;
    }

    void set_rules_skipped(std::size_t value)
    {
        rules_skipped = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_rules_skipped() const noexcept
    {
        return rules_skipped;
    }

    void set_rules_failed(std::size_t value)
    {
        rules_failed = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_rules_failed() const noexcept
    {
        return rules_failed;
    }

    void set_rules_not_applicable(std::size_t value)
    {
        rules_not_applicable = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_rules_not_applicable() const noexcept
    {
        return rules_not_applicable;
    }

    void set_profiles_not_attempted(std::size_t value)
    {
        profiles_not_attempted = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_profiles_not_attempted() const noexcept
    {
        return profiles_not_attempted;
    }

    std::size_t profiles{};
    std::size_t rules_migrated{};
    std::size_t rules_skipped{};
    std::size_t rules_failed{};
    std::size_t rules_not_applicable{};
    std::size_t profiles_not_attempted{};
};

enum class StaticRoutingAlgorithm
{
    Single,
    Priority,
    VolumeSplit,
    Advanced,
    ThreeDsDecisionRule,
};

enum class StraightThroughAlgorithm
{
    Single,
    Priority,
    VolumeSplit,
};

enum class StraightThroughAlgorithmInner
{
    Single,
    Priority,
    VolumeSplit,
};

enum class StraightThroughAlgorithmSerde
{
    Direct,
    Nested,
};

enum class SuccessRateSpecificityLevel
{
    Merchant,
    Global,
};

class ThreeDSDecisionRuleValue
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

class ThreeDSDecisionValue
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

class TransactionTypeValue
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

enum class ValueType
{
    Number,
    EnumVariant,
    MetadataVariant,
    StrValue,
    GlobalRef,
    NumberArray,
    EnumVariantArray,
    NumberComparisonArray,
};

class CalSuccessRateConfigEventRequest
{
public:
    void set_min_aggregates_size(std::optional<std::uint32_t> value)
    {
        min_aggregates_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_min_aggregates_size() const noexcept
    {
        return min_aggregates_size;
    }

    void set_default_success_rate(std::optional<double> value)
    {
        default_success_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_success_rate() const noexcept
    {
        return default_success_rate;
    }

    void set_specificity_level(SuccessRateSpecificityLevel value)
    {
        specificity_level = std::move(value);
    }

    [[nodiscard]] const SuccessRateSpecificityLevel& get_specificity_level() const noexcept
    {
        return specificity_level;
    }

    void set_exploration_percent(std::optional<double> value)
    {
        exploration_percent = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_exploration_percent() const noexcept
    {
        return exploration_percent;
    }

    std::optional<std::uint32_t> min_aggregates_size{};
    std::optional<double> default_success_rate{};
    SuccessRateSpecificityLevel specificity_level{};
    std::optional<double> exploration_percent{};
};

class CalSuccessRateEventResponse
{
public:
    void set_labels_with_score(std::vector<LabelWithScoreEventResponse> value)
    {
        labels_with_score = std::move(value);
    }

    [[nodiscard]] const std::vector<LabelWithScoreEventResponse>&
    get_labels_with_score() const noexcept
    {
        return labels_with_score;
    }

    void set_routing_approach(RoutingApproach value)
    {
        routing_approach = std::move(value);
    }

    [[nodiscard]] const RoutingApproach& get_routing_approach() const noexcept
    {
        return routing_approach;
    }

    std::vector<LabelWithScoreEventResponse> labels_with_score{};
    RoutingApproach routing_approach{};
};

class ContractBasedRoutingConfigBody
{
public:
    void set_constants(std::optional<std::vector<double>> value)
    {
        constants = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<double>>& get_constants() const noexcept
    {
        return constants;
    }

    void set_time_scale(std::optional<ContractBasedTimeScale> value)
    {
        time_scale = std::move(value);
    }

    [[nodiscard]] const std::optional<ContractBasedTimeScale>& get_time_scale() const noexcept
    {
        return time_scale;
    }

    std::optional<std::vector<double>> constants{};
    std::optional<ContractBasedTimeScale> time_scale{};
};

class ContractRoutingAlgorithm
{
public:
    void set_algorithm_id_with_timestamp(DynamicAlgorithmWithTimestamp value)
    {
        algorithm_id_with_timestamp = std::move(value);
    }

    [[nodiscard]] const DynamicAlgorithmWithTimestamp&
    get_algorithm_id_with_timestamp() const noexcept
    {
        return algorithm_id_with_timestamp;
    }

    void set_enabled_feature(DynamicRoutingFeatures value)
    {
        enabled_feature = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_enabled_feature() const noexcept
    {
        return enabled_feature;
    }

    DynamicAlgorithmWithTimestamp algorithm_id_with_timestamp{};
    DynamicRoutingFeatures enabled_feature{};
};

class CreateDynamicRoutingQuery
{
public:
    void set_enable(DynamicRoutingFeatures value)
    {
        enable = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_enable() const noexcept
    {
        return enable;
    }

    DynamicRoutingFeatures enable{};
};

class CreateDynamicRoutingWrapper
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_feature_to_enable(DynamicRoutingFeatures value)
    {
        feature_to_enable = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_feature_to_enable() const noexcept
    {
        return feature_to_enable;
    }

    void set_payload(std::optional<DynamicRoutingPayload> value)
    {
        payload = std::move(value);
    }

    [[nodiscard]] const std::optional<DynamicRoutingPayload>& get_payload() const noexcept
    {
        return payload;
    }

    ProfileIdValue profile_id{};
    DynamicRoutingFeatures feature_to_enable{};
    std::optional<DynamicRoutingPayload> payload{};
};

class DeRoutableConnectorChoice
{
public:
    void set_gateway_name(RoutableConnectorsValue value)
    {
        gateway_name = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorsValue& get_gateway_name() const noexcept
    {
        return gateway_name;
    }

    void set_gateway_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        gateway_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_gateway_id() const noexcept
    {
        return gateway_id;
    }

    RoutableConnectorsValue gateway_name{};
    std::optional<MerchantConnectorAccountIdValue> gateway_id{};
};

class DynamicRoutingUpdateConfigQuery
{
public:
    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    RoutingIdValue algorithm_id{};
    ProfileIdValue profile_id{};
};

class EliminationRoutingAlgorithm
{
public:
    void set_algorithm_id_with_timestamp(DynamicAlgorithmWithTimestamp value)
    {
        algorithm_id_with_timestamp = std::move(value);
    }

    [[nodiscard]] const DynamicAlgorithmWithTimestamp&
    get_algorithm_id_with_timestamp() const noexcept
    {
        return algorithm_id_with_timestamp;
    }

    void set_enabled_feature(DynamicRoutingFeatures value)
    {
        enabled_feature = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_enabled_feature() const noexcept
    {
        return enabled_feature;
    }

    DynamicAlgorithmWithTimestamp algorithm_id_with_timestamp{};
    DynamicRoutingFeatures enabled_feature{};
};

class EliminationRoutingConfig
{
public:
    void set_params(std::optional<std::vector<DynamicRoutingConfigParams>> value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DynamicRoutingConfigParams>>&
    get_params() const noexcept
    {
        return params;
    }

    void set_elimination_analyser_config(std::optional<EliminationAnalyserConfig> value)
    {
        elimination_analyser_config = std::move(value);
    }

    [[nodiscard]] const std::optional<EliminationAnalyserConfig>&
    get_elimination_analyser_config() const noexcept
    {
        return elimination_analyser_config;
    }

    void set_decision_engine_configs(std::optional<DecisionEngineEliminationDataValue> value)
    {
        decision_engine_configs = std::move(value);
    }

    [[nodiscard]] const std::optional<DecisionEngineEliminationDataValue>&
    get_decision_engine_configs() const noexcept
    {
        return decision_engine_configs;
    }

    std::optional<std::vector<DynamicRoutingConfigParams>> params{};
    std::optional<EliminationAnalyserConfig> elimination_analyser_config{};
    std::optional<DecisionEngineEliminationDataValue> decision_engine_configs{};
};

class EliminationRoutingEventRequest
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_params(std::string value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::string& get_params() const noexcept
    {
        return params;
    }

    void set_labels(std::vector<std::string> value)
    {
        labels = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_labels() const noexcept
    {
        return labels;
    }

    void set_config(std::optional<EliminationRoutingEventBucketConfig> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<EliminationRoutingEventBucketConfig>&
    get_config() const noexcept
    {
        return config;
    }

    std::string id{};
    std::string params{};
    std::vector<std::string> labels{};
    std::optional<EliminationRoutingEventBucketConfig> config{};
};

class LabelInformation
{
public:
    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    void set_target_count(std::uint64_t value)
    {
        target_count = std::move(value);
    }

    [[nodiscard]] const std::uint64_t& get_target_count() const noexcept
    {
        return target_count;
    }

    void set_target_time(std::uint64_t value)
    {
        target_time = std::move(value);
    }

    [[nodiscard]] const std::uint64_t& get_target_time() const noexcept
    {
        return target_time;
    }

    void set_mca_id(MerchantConnectorAccountIdValue value)
    {
        mca_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_mca_id() const noexcept
    {
        return mca_id;
    }

    std::string label{};
    std::uint64_t target_count{};
    std::uint64_t target_time{};
    MerchantConnectorAccountIdValue mca_id{};
};

class MerchantRoutingAlgorithm
{
public:
    void set_id(RoutingIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_algorithm(RoutingAlgorithmWrapper value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const RoutingAlgorithmWrapper& get_algorithm() const noexcept
    {
        return algorithm;
    }

    void set_created_at(std::int64_t value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_modified_at(std::int64_t value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_algorithm_for(TransactionTypeValue value)
    {
        algorithm_for = std::move(value);
    }

    [[nodiscard]] const TransactionTypeValue& get_algorithm_for() const noexcept
    {
        return algorithm_for;
    }

    RoutingIdValue id{};
    ProfileIdValue profile_id{};
    std::string name{};
    std::string description{};
    RoutingAlgorithmWrapper algorithm{};
    std::int64_t created_at{};
    std::int64_t modified_at{};
    TransactionTypeValue algorithm_for{};
};

class NumberComparison
{
public:
    void set_comparison_type(ComparisonType value)
    {
        comparison_type = std::move(value);
    }

    [[nodiscard]] const ComparisonType& get_comparison_type() const noexcept
    {
        return comparison_type;
    }

    void set_number(std::uint64_t value)
    {
        number = std::move(value);
    }

    [[nodiscard]] const std::uint64_t& get_number() const noexcept
    {
        return number;
    }

    ComparisonType comparison_type{};
    std::uint64_t number{};
};

class ProgramThreeDsDecisionRule
{
public:
    void set_default_selection(ThreeDSDecisionRuleValue value)
    {
        default_selection = std::move(value);
    }

    [[nodiscard]] const ThreeDSDecisionRuleValue& get_default_selection() const noexcept
    {
        return default_selection;
    }

    void set_rules(std::vector<std::string> value)
    {
        rules = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_rules() const noexcept
    {
        return rules;
    }

    void set_metadata(std::unordered_map<std::string, serde::Value> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, serde::Value>& get_metadata() const noexcept
    {
        return metadata;
    }

    ThreeDSDecisionRuleValue default_selection{};
    std::vector<std::string> rules{};
    std::unordered_map<std::string, serde::Value> metadata{};
};

class RoutableConnectorChoice
{
public:
    void set_choice_kind(RoutableChoiceKind value)
    {
        choice_kind = std::move(value);
    }

    [[nodiscard]] const RoutableChoiceKind& get_choice_kind() const noexcept
    {
        return choice_kind;
    }

    void set_connector(RoutableConnectorsValue value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorsValue& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    RoutableChoiceKind choice_kind{};
    RoutableConnectorsValue connector{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
};

class RoutingActivatePayload
{
public:
    void set_transaction_type(std::optional<TransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<TransactionTypeValue>& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::optional<TransactionTypeValue> transaction_type{};
};

class RoutingAlgorithmId
{
public:
    void set_routing_algorithm_id(RoutingIdValue value)
    {
        routing_algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_routing_algorithm_id() const noexcept
    {
        return routing_algorithm_id;
    }

    RoutingIdValue routing_algorithm_id{};
};

class RoutingAlgorithmRef
{
public:
    void set_algorithm_id(std::optional<RoutingIdValue> value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingIdValue>& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_timestamp(std::int64_t value)
    {
        timestamp = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_timestamp() const noexcept
    {
        return timestamp;
    }

    void set_config_algo_id(std::optional<std::string> value)
    {
        config_algo_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_config_algo_id() const noexcept
    {
        return config_algo_id;
    }

    void set_surcharge_config_algo_id(std::optional<std::string> value)
    {
        surcharge_config_algo_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_surcharge_config_algo_id() const noexcept
    {
        return surcharge_config_algo_id;
    }

    std::optional<RoutingIdValue> algorithm_id{};
    std::int64_t timestamp{};
    std::optional<std::string> config_algo_id{};
    std::optional<std::string> surcharge_config_algo_id{};
};

class RoutingConfigRequest
{
public:
    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_algorithm(StaticRoutingAlgorithm value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const StaticRoutingAlgorithm& get_algorithm() const noexcept
    {
        return algorithm;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    std::string name{};
    std::string description{};
    StaticRoutingAlgorithm algorithm{};
    ProfileIdValue profile_id{};
};

class RoutingDictionaryRecord
{
public:
    void set_id(RoutingIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_kind(RoutingAlgorithmKind value)
    {
        kind = std::move(value);
    }

    [[nodiscard]] const RoutingAlgorithmKind& get_kind() const noexcept
    {
        return kind;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_created_at(std::int64_t value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_modified_at(std::int64_t value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_algorithm_for(std::optional<TransactionTypeValue> value)
    {
        algorithm_for = std::move(value);
    }

    [[nodiscard]] const std::optional<TransactionTypeValue>& get_algorithm_for() const noexcept
    {
        return algorithm_for;
    }

    void set_decision_engine_routing_id(std::optional<std::string> value)
    {
        decision_engine_routing_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_decision_engine_routing_id() const noexcept
    {
        return decision_engine_routing_id;
    }

    RoutingIdValue id{};
    ProfileIdValue profile_id{};
    std::string name{};
    RoutingAlgorithmKind kind{};
    std::string description{};
    std::int64_t created_at{};
    std::int64_t modified_at{};
    std::optional<TransactionTypeValue> algorithm_for{};
    std::optional<std::string> decision_engine_routing_id{};
};

class RoutingEntryRequest
{
public:
    void set_target(std::optional<DecisionEngineRoutingTarget> value)
    {
        target = std::move(value);
    }

    [[nodiscard]] const std::optional<DecisionEngineRoutingTarget>& get_target() const noexcept
    {
        return target;
    }

    std::optional<DecisionEngineRoutingTarget> target{};
};

class RoutingMigrationProfileStatus
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_rules_hyperswitch(std::int64_t value)
    {
        rules_hyperswitch = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_rules_hyperswitch() const noexcept
    {
        return rules_hyperswitch;
    }

    void set_rules_out_of_scope(std::int64_t value)
    {
        rules_out_of_scope = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_rules_out_of_scope() const noexcept
    {
        return rules_out_of_scope;
    }

    void set_rules_decision_engine(std::optional<std::int64_t> value)
    {
        rules_decision_engine = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_rules_decision_engine() const noexcept
    {
        return rules_decision_engine;
    }

    void set_rules_missing_in_decision_engine(std::vector<std::string> value)
    {
        rules_missing_in_decision_engine = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>&
    get_rules_missing_in_decision_engine() const noexcept
    {
        return rules_missing_in_decision_engine;
    }

    void set_rules_only_in_decision_engine(std::vector<std::string> value)
    {
        rules_only_in_decision_engine = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_rules_only_in_decision_engine() const noexcept
    {
        return rules_only_in_decision_engine;
    }

    void set_routing_source(std::optional<RoutingResultSource> value)
    {
        routing_source = std::move(value);
    }

    [[nodiscard]] const std::optional<RoutingResultSource>& get_routing_source() const noexcept
    {
        return routing_source;
    }

    void set_state(RoutingMigrationState value)
    {
        state = std::move(value);
    }

    [[nodiscard]] const RoutingMigrationState& get_state() const noexcept
    {
        return state;
    }

    ProfileIdValue profile_id{};
    MerchantIdValue merchant_id{};
    std::int64_t rules_hyperswitch{};
    std::int64_t rules_out_of_scope{};
    std::optional<std::int64_t> rules_decision_engine{};
    std::vector<std::string> rules_missing_in_decision_engine{};
    std::vector<std::string> rules_only_in_decision_engine{};
    std::optional<RoutingResultSource> routing_source{};
    RoutingMigrationState state{};
};

class RoutingRetrieveLinkQuery
{
public:
    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_transaction_type(std::optional<TransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<TransactionTypeValue>& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::optional<ProfileIdValue> profile_id{};
    std::optional<TransactionTypeValue> transaction_type{};
};

class RoutingRetrieveQuery
{
public:
    void set_limit(std::optional<std::uint16_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint8_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint8_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_transaction_type(std::optional<TransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<TransactionTypeValue>& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::optional<std::uint16_t> limit{};
    std::optional<std::uint8_t> offset{};
    std::optional<TransactionTypeValue> transaction_type{};
};

class RoutingVolumeSplit
{
public:
    void set_routing_type(RoutingType value)
    {
        routing_type = std::move(value);
    }

    [[nodiscard]] const RoutingType& get_routing_type() const noexcept
    {
        return routing_type;
    }

    void set_split(std::uint8_t value)
    {
        split = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_split() const noexcept
    {
        return split;
    }

    RoutingType routing_type{};
    std::uint8_t split{};
};

class RuleMigrationError
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_error(std::string value)
    {
        error = std::move(value);
    }

    [[nodiscard]] const std::string& get_error() const noexcept
    {
        return error;
    }

    ProfileIdValue profile_id{};
    RoutingIdValue algorithm_id{};
    std::string error{};
};

class RuleMigrationNotApplicable
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_kind(RoutingAlgorithmKind value)
    {
        kind = std::move(value);
    }

    [[nodiscard]] const RoutingAlgorithmKind& get_kind() const noexcept
    {
        return kind;
    }

    void set_reason(std::string value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_reason() const noexcept
    {
        return reason;
    }

    ProfileIdValue profile_id{};
    RoutingIdValue algorithm_id{};
    RoutingAlgorithmKind kind{};
    std::string reason{};
};

class RuleMigrationQuery
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_limit(std::optional<std::uint32_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    ProfileIdValue profile_id{};
    MerchantIdValue merchant_id{};
    std::optional<std::uint32_t> limit{};
    std::optional<std::uint32_t> offset{};
};

class RuleMigrationRequest
{
public:
    void set_profile_ids(std::vector<ProfileIdValue> value)
    {
        profile_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<ProfileIdValue>& get_profile_ids() const noexcept
    {
        return profile_ids;
    }

    void set_limit(std::optional<std::uint32_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    std::vector<ProfileIdValue> profile_ids{};
    std::optional<std::uint32_t> limit{};
    std::optional<std::uint32_t> offset{};
};

class RuleMigrationResponse
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_euclid_algorithm_id(RoutingIdValue value)
    {
        euclid_algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_euclid_algorithm_id() const noexcept
    {
        return euclid_algorithm_id;
    }

    void set_decision_engine_algorithm_id(std::string value)
    {
        decision_engine_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_decision_engine_algorithm_id() const noexcept
    {
        return decision_engine_algorithm_id;
    }

    void set_is_active_rule(bool value)
    {
        is_active_rule = std::move(value);
    }

    [[nodiscard]] const bool& get_is_active_rule() const noexcept
    {
        return is_active_rule;
    }

    ProfileIdValue profile_id{};
    RoutingIdValue euclid_algorithm_id{};
    std::string decision_engine_algorithm_id{};
    bool is_active_rule{};
};

class RuleMigrationSkipped
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_decision_engine_algorithm_id(std::string value)
    {
        decision_engine_algorithm_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_decision_engine_algorithm_id() const noexcept
    {
        return decision_engine_algorithm_id;
    }

    void set_linked_active_rule(bool value)
    {
        linked_active_rule = std::move(value);
    }

    [[nodiscard]] const bool& get_linked_active_rule() const noexcept
    {
        return linked_active_rule;
    }

    ProfileIdValue profile_id{};
    RoutingIdValue algorithm_id{};
    std::string decision_engine_algorithm_id{};
    bool linked_active_rule{};
};

class RuleThreeDsDecisionRule
{
public:
    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_connector_selection(ThreeDSDecisionValue value)
    {
        connector_selection = std::move(value);
    }

    [[nodiscard]] const ThreeDSDecisionValue& get_connector_selection() const noexcept
    {
        return connector_selection;
    }

    void set_statements(std::vector<std::string> value)
    {
        statements = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_statements() const noexcept
    {
        return statements;
    }

    std::string name{};
    ThreeDSDecisionValue connector_selection{};
    std::vector<std::string> statements{};
};

class SuccessBasedAlgorithm
{
public:
    void set_algorithm_id_with_timestamp(DynamicAlgorithmWithTimestamp value)
    {
        algorithm_id_with_timestamp = std::move(value);
    }

    [[nodiscard]] const DynamicAlgorithmWithTimestamp&
    get_algorithm_id_with_timestamp() const noexcept
    {
        return algorithm_id_with_timestamp;
    }

    void set_enabled_feature(DynamicRoutingFeatures value)
    {
        enabled_feature = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_enabled_feature() const noexcept
    {
        return enabled_feature;
    }

    DynamicAlgorithmWithTimestamp algorithm_id_with_timestamp{};
    DynamicRoutingFeatures enabled_feature{};
};

class SuccessBasedRoutingConfigBody
{
public:
    void set_min_aggregates_size(std::optional<std::uint32_t> value)
    {
        min_aggregates_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_min_aggregates_size() const noexcept
    {
        return min_aggregates_size;
    }

    void set_default_success_rate(std::optional<double> value)
    {
        default_success_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_default_success_rate() const noexcept
    {
        return default_success_rate;
    }

    void set_max_aggregates_size(std::optional<std::uint32_t> value)
    {
        max_aggregates_size = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_max_aggregates_size() const noexcept
    {
        return max_aggregates_size;
    }

    void set_current_block_threshold(std::optional<CurrentBlockThreshold> value)
    {
        current_block_threshold = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrentBlockThreshold>&
    get_current_block_threshold() const noexcept
    {
        return current_block_threshold;
    }

    void set_specificity_level(SuccessRateSpecificityLevel value)
    {
        specificity_level = std::move(value);
    }

    [[nodiscard]] const SuccessRateSpecificityLevel& get_specificity_level() const noexcept
    {
        return specificity_level;
    }

    void set_exploration_percent(std::optional<double> value)
    {
        exploration_percent = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_exploration_percent() const noexcept
    {
        return exploration_percent;
    }

    void set_shuffle_on_tie_during_exploitation(std::optional<bool> value)
    {
        shuffle_on_tie_during_exploitation = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_shuffle_on_tie_during_exploitation() const noexcept
    {
        return shuffle_on_tie_during_exploitation;
    }

    std::optional<std::uint32_t> min_aggregates_size{};
    std::optional<double> default_success_rate{};
    std::optional<std::uint32_t> max_aggregates_size{};
    std::optional<CurrentBlockThreshold> current_block_threshold{};
    SuccessRateSpecificityLevel specificity_level{};
    std::optional<double> exploration_percent{};
    std::optional<bool> shuffle_on_tie_during_exploitation{};
};

class ToggleDynamicRoutingPath
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    ProfileIdValue profile_id{};
};

class ToggleDynamicRoutingQuery
{
public:
    void set_enable(DynamicRoutingFeatures value)
    {
        enable = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_enable() const noexcept
    {
        return enable;
    }

    DynamicRoutingFeatures enable{};
};

class ToggleDynamicRoutingWrapper
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_feature_to_enable(DynamicRoutingFeatures value)
    {
        feature_to_enable = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_feature_to_enable() const noexcept
    {
        return feature_to_enable;
    }

    ProfileIdValue profile_id{};
    DynamicRoutingFeatures feature_to_enable{};
};

class CalSuccessRateEventRequest
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_params(std::string value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::string& get_params() const noexcept
    {
        return params;
    }

    void set_labels(std::vector<std::string> value)
    {
        labels = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_labels() const noexcept
    {
        return labels;
    }

    void set_config(std::optional<CalSuccessRateConfigEventRequest> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<CalSuccessRateConfigEventRequest>& get_config() const noexcept
    {
        return config;
    }

    std::string id{};
    std::string params{};
    std::vector<std::string> labels{};
    std::optional<CalSuccessRateConfigEventRequest> config{};
};

class ConnectorVolumeSplit
{
public:
    void set_connector(RoutableConnectorChoice value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_connector() const noexcept
    {
        return connector;
    }

    void set_split(std::uint8_t value)
    {
        split = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_split() const noexcept
    {
        return split;
    }

    RoutableConnectorChoice connector{};
    std::uint8_t split{};
};

class ContractBasedRoutingConfig
{
public:
    void set_config(std::optional<ContractBasedRoutingConfigBody> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<ContractBasedRoutingConfigBody>& get_config() const noexcept
    {
        return config;
    }

    void set_label_info(std::optional<std::vector<LabelInformation>> value)
    {
        label_info = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<LabelInformation>>&
    get_label_info() const noexcept
    {
        return label_info;
    }

    std::optional<ContractBasedRoutingConfigBody> config{};
    std::optional<std::vector<LabelInformation>> label_info{};
};

enum class DynamicRoutingAlgorithm
{
    EliminationBasedAlgorithm,
    SuccessBasedAlgorithm,
    ContractBasedAlgorithm,
};

class DynamicRoutingAlgorithmRef
{
public:
    void set_success_based_algorithm(std::optional<SuccessBasedAlgorithm> value)
    {
        success_based_algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<SuccessBasedAlgorithm>&
    get_success_based_algorithm() const noexcept
    {
        return success_based_algorithm;
    }

    void set_dynamic_routing_volume_split(std::optional<std::uint8_t> value)
    {
        dynamic_routing_volume_split = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint8_t>&
    get_dynamic_routing_volume_split() const noexcept
    {
        return dynamic_routing_volume_split;
    }

    void set_elimination_routing_algorithm(std::optional<EliminationRoutingAlgorithm> value)
    {
        elimination_routing_algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<EliminationRoutingAlgorithm>&
    get_elimination_routing_algorithm() const noexcept
    {
        return elimination_routing_algorithm;
    }

    void set_contract_based_routing(std::optional<ContractRoutingAlgorithm> value)
    {
        contract_based_routing = std::move(value);
    }

    [[nodiscard]] const std::optional<ContractRoutingAlgorithm>&
    get_contract_based_routing() const noexcept
    {
        return contract_based_routing;
    }

    void set_is_merchant_created_in_decision_engine(bool value)
    {
        is_merchant_created_in_decision_engine = std::move(value);
    }

    [[nodiscard]] const bool& get_is_merchant_created_in_decision_engine() const noexcept
    {
        return is_merchant_created_in_decision_engine;
    }

    std::optional<SuccessBasedAlgorithm> success_based_algorithm{};
    std::optional<std::uint8_t> dynamic_routing_volume_split{};
    std::optional<EliminationRoutingAlgorithm> elimination_routing_algorithm{};
    std::optional<ContractRoutingAlgorithm> contract_based_routing{};
    bool is_merchant_created_in_decision_engine{};
};

class EliminationRoutingPayloadWrapper
{
public:
    void set_updated_config(EliminationRoutingConfig value)
    {
        updated_config = std::move(value);
    }

    [[nodiscard]] const EliminationRoutingConfig& get_updated_config() const noexcept
    {
        return updated_config;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    EliminationRoutingConfig updated_config{};
    RoutingIdValue algorithm_id{};
    ProfileIdValue profile_id{};
};

class ProfileDefaultRoutingConfig
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_connectors(std::vector<RoutableConnectorChoice> value)
    {
        connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutableConnectorChoice>& get_connectors() const noexcept
    {
        return connectors;
    }

    ProfileIdValue profile_id{};
    std::vector<RoutableConnectorChoice> connectors{};
};

class RoutableConnectorChoiceWithBucketName
{
public:
    void set_routable_connector_choice(RoutableConnectorChoice value)
    {
        routable_connector_choice = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_routable_connector_choice() const noexcept
    {
        return routable_connector_choice;
    }

    void set_bucket_name(std::string value)
    {
        bucket_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_bucket_name() const noexcept
    {
        return bucket_name;
    }

    RoutableConnectorChoice routable_connector_choice{};
    std::string bucket_name{};
};

class RoutableConnectorChoiceWithStatus
{
public:
    void set_routable_connector_choice(RoutableConnectorChoice value)
    {
        routable_connector_choice = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_routable_connector_choice() const noexcept
    {
        return routable_connector_choice;
    }

    void set_status(bool value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const bool& get_status() const noexcept
    {
        return status;
    }

    RoutableConnectorChoice routable_connector_choice{};
    bool status{};
};

class RoutingDictionary
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_active_id(std::optional<std::string> value)
    {
        active_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_active_id() const noexcept
    {
        return active_id;
    }

    void set_records(std::vector<RoutingDictionaryRecord> value)
    {
        records = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutingDictionaryRecord>& get_records() const noexcept
    {
        return records;
    }

    MerchantIdValue merchant_id{};
    std::optional<std::string> active_id{};
    std::vector<RoutingDictionaryRecord> records{};
};

class RoutingEvaluateRequest
{
public:
    void set_created_by(std::string value)
    {
        created_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_by() const noexcept
    {
        return created_by;
    }

    void set_payment_id(std::optional<std::string> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_parameters(std::unordered_map<std::string, std::optional<ValueType>> value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::optional<ValueType>>&
    get_parameters() const noexcept
    {
        return parameters;
    }

    void set_fallback_output(std::vector<DeRoutableConnectorChoice> value)
    {
        fallback_output = std::move(value);
    }

    [[nodiscard]] const std::vector<DeRoutableConnectorChoice>& get_fallback_output() const noexcept
    {
        return fallback_output;
    }

    void set_algorithm_for(std::optional<TransactionTypeValue> value)
    {
        algorithm_for = std::move(value);
    }

    [[nodiscard]] const std::optional<TransactionTypeValue>& get_algorithm_for() const noexcept
    {
        return algorithm_for;
    }

    std::string created_by{};
    std::optional<std::string> payment_id{};
    std::unordered_map<std::string, std::optional<ValueType>> parameters{};
    std::vector<DeRoutableConnectorChoice> fallback_output{};
    std::optional<TransactionTypeValue> algorithm_for{};
};

class RoutingEvaluateResponse
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

    void set_output(serde::Value value)
    {
        output = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_output() const noexcept
    {
        return output;
    }

    void set_evaluated_output(std::vector<RoutableConnectorChoice> value)
    {
        evaluated_output = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutableConnectorChoice>& get_evaluated_output() const noexcept
    {
        return evaluated_output;
    }

    void set_eligible_connectors(std::vector<RoutableConnectorChoice> value)
    {
        eligible_connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutableConnectorChoice>&
    get_eligible_connectors() const noexcept
    {
        return eligible_connectors;
    }

    std::string status{};
    serde::Value output{};
    std::vector<RoutableConnectorChoice> evaluated_output{};
    std::vector<RoutableConnectorChoice> eligible_connectors{};
};

class RoutingLinkWrapper
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_algorithm_id(RoutingAlgorithmId value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingAlgorithmId& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    ProfileIdValue profile_id{};
    RoutingAlgorithmId algorithm_id{};
};

class RoutingMigrationStatusResponse
{
public:
    void set_profiles(std::vector<RoutingMigrationProfileStatus> value)
    {
        profiles = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutingMigrationProfileStatus>& get_profiles() const noexcept
    {
        return profiles;
    }

    void set_limit(std::uint32_t value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::uint32_t value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_offset() const noexcept
    {
        return offset;
    }

    void set_has_more(bool value)
    {
        has_more = std::move(value);
    }

    [[nodiscard]] const bool& get_has_more() const noexcept
    {
        return has_more;
    }

    void set_page_totals(RoutingMigrationPageTotals value)
    {
        page_totals = std::move(value);
    }

    [[nodiscard]] const RoutingMigrationPageTotals& get_page_totals() const noexcept
    {
        return page_totals;
    }

    std::vector<RoutingMigrationProfileStatus> profiles{};
    std::uint32_t limit{};
    std::uint32_t offset{};
    bool has_more{};
    RoutingMigrationPageTotals page_totals{};
};

class RoutingPayloadWrapper
{
public:
    void set_updated_config(std::vector<RoutableConnectorChoice> value)
    {
        updated_config = std::move(value);
    }

    [[nodiscard]] const std::vector<RoutableConnectorChoice>& get_updated_config() const noexcept
    {
        return updated_config;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    std::vector<RoutableConnectorChoice> updated_config{};
    ProfileIdValue profile_id{};
};

class RoutingRetrieveLinkQueryWrapper
{
public:
    void set_routing_query(RoutingRetrieveQuery value)
    {
        routing_query = std::move(value);
    }

    [[nodiscard]] const RoutingRetrieveQuery& get_routing_query() const noexcept
    {
        return routing_query;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    RoutingRetrieveQuery routing_query{};
    ProfileIdValue profile_id{};
};

class RoutingRetrieveResponse
{
public:
    void set_algorithm(std::optional<MerchantRoutingAlgorithm> value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantRoutingAlgorithm>& get_algorithm() const noexcept
    {
        return algorithm;
    }

    std::optional<MerchantRoutingAlgorithm> algorithm{};
};

class RoutingVolumeSplitWrapper
{
public:
    void set_routing_info(RoutingVolumeSplit value)
    {
        routing_info = std::move(value);
    }

    [[nodiscard]] const RoutingVolumeSplit& get_routing_info() const noexcept
    {
        return routing_info;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    RoutingVolumeSplit routing_info{};
    ProfileIdValue profile_id{};
};

class RuleMigrationProfileResult
{
public:
    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_success(std::vector<RuleMigrationResponse> value)
    {
        success = std::move(value);
    }

    [[nodiscard]] const std::vector<RuleMigrationResponse>& get_success() const noexcept
    {
        return success;
    }

    void set_skipped(std::vector<RuleMigrationSkipped> value)
    {
        skipped = std::move(value);
    }

    [[nodiscard]] const std::vector<RuleMigrationSkipped>& get_skipped() const noexcept
    {
        return skipped;
    }

    void set_errors(std::vector<RuleMigrationError> value)
    {
        errors = std::move(value);
    }

    [[nodiscard]] const std::vector<RuleMigrationError>& get_errors() const noexcept
    {
        return errors;
    }

    void set_not_applicable(std::vector<RuleMigrationNotApplicable> value)
    {
        not_applicable = std::move(value);
    }

    [[nodiscard]] const std::vector<RuleMigrationNotApplicable>& get_not_applicable() const noexcept
    {
        return not_applicable;
    }

    void set_not_attempted(std::optional<std::string> value)
    {
        not_attempted = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_not_attempted() const noexcept
    {
        return not_attempted;
    }

    ProfileIdValue profile_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::vector<RuleMigrationResponse> success{};
    std::vector<RuleMigrationSkipped> skipped{};
    std::vector<RuleMigrationError> errors{};
    std::vector<RuleMigrationNotApplicable> not_applicable{};
    std::optional<std::string> not_attempted{};
};

class SuccessBasedRoutingConfig
{
public:
    void set_params(std::optional<std::vector<DynamicRoutingConfigParams>> value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<DynamicRoutingConfigParams>>&
    get_params() const noexcept
    {
        return params;
    }

    void set_config(std::optional<SuccessBasedRoutingConfigBody> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<SuccessBasedRoutingConfigBody>& get_config() const noexcept
    {
        return config;
    }

    void set_decision_engine_configs(std::optional<DecisionEngineSuccessRateDataValue> value)
    {
        decision_engine_configs = std::move(value);
    }

    [[nodiscard]] const std::optional<DecisionEngineSuccessRateDataValue>&
    get_decision_engine_configs() const noexcept
    {
        return decision_engine_configs;
    }

    std::optional<std::vector<DynamicRoutingConfigParams>> params{};
    std::optional<SuccessBasedRoutingConfigBody> config{};
    std::optional<DecisionEngineSuccessRateDataValue> decision_engine_configs{};
};

class CalContractScoreEventRequest
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_params(std::string value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::string& get_params() const noexcept
    {
        return params;
    }

    void set_labels(std::vector<std::string> value)
    {
        labels = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_labels() const noexcept
    {
        return labels;
    }

    void set_config(std::optional<ContractBasedRoutingConfig> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<ContractBasedRoutingConfig>& get_config() const noexcept
    {
        return config;
    }

    std::string id{};
    std::string params{};
    std::vector<std::string> labels{};
    std::optional<ContractBasedRoutingConfig> config{};
};

class ContractBasedRoutingPayloadWrapper
{
public:
    void set_updated_config(ContractBasedRoutingConfig value)
    {
        updated_config = std::move(value);
    }

    [[nodiscard]] const ContractBasedRoutingConfig& get_updated_config() const noexcept
    {
        return updated_config;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    ContractBasedRoutingConfig updated_config{};
    RoutingIdValue algorithm_id{};
    ProfileIdValue profile_id{};
};

class ContractBasedRoutingSetupPayloadWrapper
{
public:
    void set_config(std::optional<ContractBasedRoutingConfig> value)
    {
        config = std::move(value);
    }

    [[nodiscard]] const std::optional<ContractBasedRoutingConfig>& get_config() const noexcept
    {
        return config;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_features_to_enable(DynamicRoutingFeatures value)
    {
        features_to_enable = std::move(value);
    }

    [[nodiscard]] const DynamicRoutingFeatures& get_features_to_enable() const noexcept
    {
        return features_to_enable;
    }

    std::optional<ContractBasedRoutingConfig> config{};
    ProfileIdValue profile_id{};
    DynamicRoutingFeatures features_to_enable{};
};

class RuleMigrationResult
{
public:
    void set_profiles(std::vector<RuleMigrationProfileResult> value)
    {
        profiles = std::move(value);
    }

    [[nodiscard]] const std::vector<RuleMigrationProfileResult>& get_profiles() const noexcept
    {
        return profiles;
    }

    void set_totals(RuleMigrationTotals value)
    {
        totals = std::move(value);
    }

    [[nodiscard]] const RuleMigrationTotals& get_totals() const noexcept
    {
        return totals;
    }

    std::vector<RuleMigrationProfileResult> profiles{};
    RuleMigrationTotals totals{};
};

class SuccessBasedRoutingPayloadWrapper
{
public:
    void set_updated_config(SuccessBasedRoutingConfig value)
    {
        updated_config = std::move(value);
    }

    [[nodiscard]] const SuccessBasedRoutingConfig& get_updated_config() const noexcept
    {
        return updated_config;
    }

    void set_algorithm_id(RoutingIdValue value)
    {
        algorithm_id = std::move(value);
    }

    [[nodiscard]] const RoutingIdValue& get_algorithm_id() const noexcept
    {
        return algorithm_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    SuccessBasedRoutingConfig updated_config{};
    RoutingIdValue algorithm_id{};
    ProfileIdValue profile_id{};
};
} // namespace app::model::routing

template<>
struct serde::Serializable<app::model::routing::RoutingConfigRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::routing::RoutingConfigRequest::get_name,
                &app::model::routing::RoutingConfigRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::routing::RoutingConfigRequest::get_description,
                &app::model::routing::RoutingConfigRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::model::routing::RoutingConfigRequest::get_algorithm,
                &app::model::routing::RoutingConfigRequest::set_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingConfigRequest::get_profile_id,
                &app::model::routing::RoutingConfigRequest::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_config_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingConfigName>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::model::routing::RoutingConfigName::get_value,
            &app::model::routing::RoutingConfigName::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_config_name";
    }
};

template<>
struct serde::Serializable<app::model::routing::ProfileDefaultRoutingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::ProfileDefaultRoutingConfig::get_profile_id,
                &app::model::routing::ProfileDefaultRoutingConfig::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connectors",
                &app::model::routing::ProfileDefaultRoutingConfig::get_connectors,
                &app::model::routing::ProfileDefaultRoutingConfig::set_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_default_routing_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingRetrieveQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::routing::RoutingRetrieveQuery::get_limit,
                &app::model::routing::RoutingRetrieveQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::routing::RoutingRetrieveQuery::get_offset,
                &app::model::routing::RoutingRetrieveQuery::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::routing::RoutingRetrieveQuery::get_transaction_type,
                &app::model::routing::RoutingRetrieveQuery::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_retrieve_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingActivatePayload>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "transaction_type",
            &app::model::routing::RoutingActivatePayload::get_transaction_type,
            &app::model::routing::RoutingActivatePayload::set_transaction_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_activate_payload";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingRetrieveLinkQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingRetrieveLinkQuery::get_profile_id,
                &app::model::routing::RoutingRetrieveLinkQuery::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::routing::RoutingRetrieveLinkQuery::get_transaction_type,
                &app::model::routing::RoutingRetrieveLinkQuery::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_retrieve_link_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingRetrieveLinkQueryWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routing_query",
                &app::model::routing::RoutingRetrieveLinkQueryWrapper::get_routing_query,
                &app::model::routing::RoutingRetrieveLinkQueryWrapper::set_routing_query,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingRetrieveLinkQueryWrapper::get_profile_id,
                &app::model::routing::RoutingRetrieveLinkQueryWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_retrieve_link_query_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingRetrieveResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "algorithm",
            &app::model::routing::RoutingRetrieveResponse::get_algorithm,
            &app::model::routing::RoutingRetrieveResponse::set_algorithm,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_retrieve_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::MerchantRoutingAlgorithm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::routing::MerchantRoutingAlgorithm::get_id,
                &app::model::routing::MerchantRoutingAlgorithm::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::MerchantRoutingAlgorithm::get_profile_id,
                &app::model::routing::MerchantRoutingAlgorithm::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::routing::MerchantRoutingAlgorithm::get_name,
                &app::model::routing::MerchantRoutingAlgorithm::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::routing::MerchantRoutingAlgorithm::get_description,
                &app::model::routing::MerchantRoutingAlgorithm::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::model::routing::MerchantRoutingAlgorithm::get_algorithm,
                &app::model::routing::MerchantRoutingAlgorithm::set_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::routing::MerchantRoutingAlgorithm::get_created_at,
                &app::model::routing::MerchantRoutingAlgorithm::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::routing::MerchantRoutingAlgorithm::get_modified_at,
                &app::model::routing::MerchantRoutingAlgorithm::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_for",
                &app::model::routing::MerchantRoutingAlgorithm::get_algorithm_for,
                &app::model::routing::MerchantRoutingAlgorithm::set_algorithm_for,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_routing_algorithm";
    }
};

template<>
struct serde::Serializable<app::model::routing::ConnectorVolumeSplit>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::routing::ConnectorVolumeSplit::get_connector,
                &app::model::routing::ConnectorVolumeSplit::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split",
                &app::model::routing::ConnectorVolumeSplit::get_split,
                &app::model::routing::ConnectorVolumeSplit::set_split,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_volume_split";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "choice_kind",
                &app::model::routing::RoutableConnectorChoice::get_choice_kind,
                &app::model::routing::RoutableConnectorChoice::set_choice_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::routing::RoutableConnectorChoice::get_connector,
                &app::model::routing::RoutableConnectorChoice::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::routing::RoutableConnectorChoice::get_merchant_connector_id,
                &app::model::routing::RoutableConnectorChoice::set_merchant_connector_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoiceWithStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routable_connector_choice",
                &app::model::routing::RoutableConnectorChoiceWithStatus::
                    get_routable_connector_choice,
                &app::model::routing::RoutableConnectorChoiceWithStatus::
                    set_routable_connector_choice,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::routing::RoutableConnectorChoiceWithStatus::get_status,
                &app::model::routing::RoutableConnectorChoiceWithStatus::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice_with_status";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingPayloadWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "updated_config",
                &app::model::routing::RoutingPayloadWrapper::get_updated_config,
                &app::model::routing::RoutingPayloadWrapper::set_updated_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingPayloadWrapper::get_profile_id,
                &app::model::routing::RoutingPayloadWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_payload_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::ProgramThreeDsDecisionRule>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "defaultSelection",
                &app::model::routing::ProgramThreeDsDecisionRule::get_default_selection,
                &app::model::routing::ProgramThreeDsDecisionRule::set_default_selection,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules",
                &app::model::routing::ProgramThreeDsDecisionRule::get_rules,
                &app::model::routing::ProgramThreeDsDecisionRule::set_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::routing::ProgramThreeDsDecisionRule::get_metadata,
                &app::model::routing::ProgramThreeDsDecisionRule::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "program_three_ds_decision_rule";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleThreeDsDecisionRule>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::routing::RuleThreeDsDecisionRule::get_name,
                &app::model::routing::RuleThreeDsDecisionRule::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connectorSelection",
                &app::model::routing::RuleThreeDsDecisionRule::get_connector_selection,
                &app::model::routing::RuleThreeDsDecisionRule::set_connector_selection,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statements",
                &app::model::routing::RuleThreeDsDecisionRule::get_statements,
                &app::model::routing::RuleThreeDsDecisionRule::set_statements,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_three_ds_decision_rule";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingAlgorithmRef>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::RoutingAlgorithmRef::get_algorithm_id,
                &app::model::routing::RoutingAlgorithmRef::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timestamp",
                &app::model::routing::RoutingAlgorithmRef::get_timestamp,
                &app::model::routing::RoutingAlgorithmRef::set_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config_algo_id",
                &app::model::routing::RoutingAlgorithmRef::get_config_algo_id,
                &app::model::routing::RoutingAlgorithmRef::set_config_algo_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_config_algo_id",
                &app::model::routing::RoutingAlgorithmRef::get_surcharge_config_algo_id,
                &app::model::routing::RoutingAlgorithmRef::set_surcharge_config_algo_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_algorithm_ref";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingDictionaryRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::routing::RoutingDictionaryRecord::get_id,
                &app::model::routing::RoutingDictionaryRecord::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingDictionaryRecord::get_profile_id,
                &app::model::routing::RoutingDictionaryRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::routing::RoutingDictionaryRecord::get_name,
                &app::model::routing::RoutingDictionaryRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "kind",
                &app::model::routing::RoutingDictionaryRecord::get_kind,
                &app::model::routing::RoutingDictionaryRecord::set_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::routing::RoutingDictionaryRecord::get_description,
                &app::model::routing::RoutingDictionaryRecord::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::routing::RoutingDictionaryRecord::get_created_at,
                &app::model::routing::RoutingDictionaryRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::routing::RoutingDictionaryRecord::get_modified_at,
                &app::model::routing::RoutingDictionaryRecord::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_for",
                &app::model::routing::RoutingDictionaryRecord::get_algorithm_for,
                &app::model::routing::RoutingDictionaryRecord::set_algorithm_for,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_engine_routing_id",
                &app::model::routing::RoutingDictionaryRecord::get_decision_engine_routing_id,
                &app::model::routing::RoutingDictionaryRecord::set_decision_engine_routing_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_dictionary_record";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingDictionary>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::routing::RoutingDictionary::get_merchant_id,
                &app::model::routing::RoutingDictionary::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "active_id",
                &app::model::routing::RoutingDictionary::get_active_id,
                &app::model::routing::RoutingDictionary::set_active_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "records",
                &app::model::routing::RoutingDictionary::get_records,
                &app::model::routing::RoutingDictionary::set_records,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_dictionary";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingAlgorithmId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "routing_algorithm_id",
            &app::model::routing::RoutingAlgorithmId::get_routing_algorithm_id,
            &app::model::routing::RoutingAlgorithmId::set_routing_algorithm_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_algorithm_id";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingLinkWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingLinkWrapper::get_profile_id,
                &app::model::routing::RoutingLinkWrapper::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::RoutingLinkWrapper::get_algorithm_id,
                &app::model::routing::RoutingLinkWrapper::set_algorithm_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_link_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::DynamicAlgorithmWithTimestamp>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::DynamicAlgorithmWithTimestamp::get_algorithm_id,
                &app::model::routing::DynamicAlgorithmWithTimestamp::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timestamp",
                &app::model::routing::DynamicAlgorithmWithTimestamp::get_timestamp,
                &app::model::routing::DynamicAlgorithmWithTimestamp::set_timestamp,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dynamic_algorithm_with_timestamp";
    }
};

template<>
struct serde::Serializable<app::model::routing::DynamicRoutingAlgorithmRef>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "success_based_algorithm",
                &app::model::routing::DynamicRoutingAlgorithmRef::get_success_based_algorithm,
                &app::model::routing::DynamicRoutingAlgorithmRef::set_success_based_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dynamic_routing_volume_split",
                &app::model::routing::DynamicRoutingAlgorithmRef::get_dynamic_routing_volume_split,
                &app::model::routing::DynamicRoutingAlgorithmRef::set_dynamic_routing_volume_split,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "elimination_routing_algorithm",
                &app::model::routing::DynamicRoutingAlgorithmRef::get_elimination_routing_algorithm,
                &app::model::routing::DynamicRoutingAlgorithmRef::set_elimination_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "contract_based_routing",
                &app::model::routing::DynamicRoutingAlgorithmRef::get_contract_based_routing,
                &app::model::routing::DynamicRoutingAlgorithmRef::set_contract_based_routing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_merchant_created_in_decision_engine",
                &app::model::routing::DynamicRoutingAlgorithmRef::
                    get_is_merchant_created_in_decision_engine,
                &app::model::routing::DynamicRoutingAlgorithmRef::
                    set_is_merchant_created_in_decision_engine,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dynamic_routing_algorithm_ref";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingVolumeSplit>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routing_type",
                &app::model::routing::RoutingVolumeSplit::get_routing_type,
                &app::model::routing::RoutingVolumeSplit::set_routing_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split",
                &app::model::routing::RoutingVolumeSplit::get_split,
                &app::model::routing::RoutingVolumeSplit::set_split,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_volume_split";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingVolumeSplitWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routing_info",
                &app::model::routing::RoutingVolumeSplitWrapper::get_routing_info,
                &app::model::routing::RoutingVolumeSplitWrapper::set_routing_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingVolumeSplitWrapper::get_profile_id,
                &app::model::routing::RoutingVolumeSplitWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_volume_split_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::SuccessBasedAlgorithm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id_with_timestamp",
                &app::model::routing::SuccessBasedAlgorithm::get_algorithm_id_with_timestamp,
                &app::model::routing::SuccessBasedAlgorithm::set_algorithm_id_with_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_feature",
                &app::model::routing::SuccessBasedAlgorithm::get_enabled_feature,
                &app::model::routing::SuccessBasedAlgorithm::set_enabled_feature,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "success_based_algorithm";
    }
};

template<>
struct serde::Serializable<app::model::routing::ContractRoutingAlgorithm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id_with_timestamp",
                &app::model::routing::ContractRoutingAlgorithm::get_algorithm_id_with_timestamp,
                &app::model::routing::ContractRoutingAlgorithm::set_algorithm_id_with_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_feature",
                &app::model::routing::ContractRoutingAlgorithm::get_enabled_feature,
                &app::model::routing::ContractRoutingAlgorithm::set_enabled_feature,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "contract_routing_algorithm";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationRoutingAlgorithm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id_with_timestamp",
                &app::model::routing::EliminationRoutingAlgorithm::get_algorithm_id_with_timestamp,
                &app::model::routing::EliminationRoutingAlgorithm::set_algorithm_id_with_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_feature",
                &app::model::routing::EliminationRoutingAlgorithm::get_enabled_feature,
                &app::model::routing::EliminationRoutingAlgorithm::set_enabled_feature,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_routing_algorithm";
    }
};

template<>
struct serde::Serializable<app::model::routing::ToggleDynamicRoutingQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "enable",
            &app::model::routing::ToggleDynamicRoutingQuery::get_enable,
            &app::model::routing::ToggleDynamicRoutingQuery::set_enable,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_dynamic_routing_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::CreateDynamicRoutingQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "enable",
            &app::model::routing::CreateDynamicRoutingQuery::get_enable,
            &app::model::routing::CreateDynamicRoutingQuery::set_enable,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "create_dynamic_routing_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::DynamicRoutingVolumeSplitQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "split",
            &app::model::routing::DynamicRoutingVolumeSplitQuery::get_split,
            &app::model::routing::DynamicRoutingVolumeSplitQuery::set_split,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "dynamic_routing_volume_split_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::DynamicRoutingUpdateConfigQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::DynamicRoutingUpdateConfigQuery::get_algorithm_id,
                &app::model::routing::DynamicRoutingUpdateConfigQuery::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::DynamicRoutingUpdateConfigQuery::get_profile_id,
                &app::model::routing::DynamicRoutingUpdateConfigQuery::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dynamic_routing_update_config_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::ToggleDynamicRoutingWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::ToggleDynamicRoutingWrapper::get_profile_id,
                &app::model::routing::ToggleDynamicRoutingWrapper::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_to_enable",
                &app::model::routing::ToggleDynamicRoutingWrapper::get_feature_to_enable,
                &app::model::routing::ToggleDynamicRoutingWrapper::set_feature_to_enable,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_dynamic_routing_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::ToggleDynamicRoutingPath>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "profile_id",
            &app::model::routing::ToggleDynamicRoutingPath::get_profile_id,
            &app::model::routing::ToggleDynamicRoutingPath::set_profile_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_dynamic_routing_path";
    }
};

template<>
struct serde::Serializable<app::model::routing::CreateDynamicRoutingWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::CreateDynamicRoutingWrapper::get_profile_id,
                &app::model::routing::CreateDynamicRoutingWrapper::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_to_enable",
                &app::model::routing::CreateDynamicRoutingWrapper::get_feature_to_enable,
                &app::model::routing::CreateDynamicRoutingWrapper::set_feature_to_enable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::model::routing::CreateDynamicRoutingWrapper::get_payload,
                &app::model::routing::CreateDynamicRoutingWrapper::set_payload,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_dynamic_routing_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingVolumeSplitResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "split",
            &app::model::routing::RoutingVolumeSplitResponse::get_split,
            &app::model::routing::RoutingVolumeSplitResponse::set_split,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_volume_split_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationRoutingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "params",
                &app::model::routing::EliminationRoutingConfig::get_params,
                &app::model::routing::EliminationRoutingConfig::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "elimination_analyser_config",
                &app::model::routing::EliminationRoutingConfig::get_elimination_analyser_config,
                &app::model::routing::EliminationRoutingConfig::set_elimination_analyser_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_engine_configs",
                &app::model::routing::EliminationRoutingConfig::get_decision_engine_configs,
                &app::model::routing::EliminationRoutingConfig::set_decision_engine_configs,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_routing_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationAnalyserConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bucket_size",
                &app::model::routing::EliminationAnalyserConfig::get_bucket_size,
                &app::model::routing::EliminationAnalyserConfig::set_bucket_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bucket_leak_interval_in_secs",
                &app::model::routing::EliminationAnalyserConfig::get_bucket_leak_interval_in_secs,
                &app::model::routing::EliminationAnalyserConfig::set_bucket_leak_interval_in_secs,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_analyser_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::SuccessBasedRoutingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "params",
                &app::model::routing::SuccessBasedRoutingConfig::get_params,
                &app::model::routing::SuccessBasedRoutingConfig::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config",
                &app::model::routing::SuccessBasedRoutingConfig::get_config,
                &app::model::routing::SuccessBasedRoutingConfig::set_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_engine_configs",
                &app::model::routing::SuccessBasedRoutingConfig::get_decision_engine_configs,
                &app::model::routing::SuccessBasedRoutingConfig::set_decision_engine_configs,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "success_based_routing_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::SuccessBasedRoutingConfigBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "min_aggregates_size",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_min_aggregates_size,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_min_aggregates_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_success_rate",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_default_success_rate,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_default_success_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_aggregates_size",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_max_aggregates_size,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_max_aggregates_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "current_block_threshold",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_current_block_threshold,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_current_block_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "specificity_level",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_specificity_level,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_specificity_level,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "exploration_percent",
                &app::model::routing::SuccessBasedRoutingConfigBody::get_exploration_percent,
                &app::model::routing::SuccessBasedRoutingConfigBody::set_exploration_percent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shuffle_on_tie_during_exploitation",
                &app::model::routing::SuccessBasedRoutingConfigBody::
                    get_shuffle_on_tie_during_exploitation,
                &app::model::routing::SuccessBasedRoutingConfigBody::
                    set_shuffle_on_tie_during_exploitation,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "success_based_routing_config_body";
    }
};

template<>
struct serde::Serializable<app::model::routing::CurrentBlockThreshold>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "duration_in_mins",
                &app::model::routing::CurrentBlockThreshold::get_duration_in_mins,
                &app::model::routing::CurrentBlockThreshold::set_duration_in_mins,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_total_count",
                &app::model::routing::CurrentBlockThreshold::get_max_total_count,
                &app::model::routing::CurrentBlockThreshold::set_max_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "current_block_threshold";
    }
};

template<>
struct serde::Serializable<app::model::routing::SuccessBasedRoutingPayloadWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "updated_config",
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::get_updated_config,
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::set_updated_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::get_algorithm_id,
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::get_profile_id,
                &app::model::routing::SuccessBasedRoutingPayloadWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "success_based_routing_payload_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationRoutingPayloadWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "updated_config",
                &app::model::routing::EliminationRoutingPayloadWrapper::get_updated_config,
                &app::model::routing::EliminationRoutingPayloadWrapper::set_updated_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::EliminationRoutingPayloadWrapper::get_algorithm_id,
                &app::model::routing::EliminationRoutingPayloadWrapper::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::EliminationRoutingPayloadWrapper::get_profile_id,
                &app::model::routing::EliminationRoutingPayloadWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_routing_payload_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::ContractBasedRoutingPayloadWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "updated_config",
                &app::model::routing::ContractBasedRoutingPayloadWrapper::get_updated_config,
                &app::model::routing::ContractBasedRoutingPayloadWrapper::set_updated_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::ContractBasedRoutingPayloadWrapper::get_algorithm_id,
                &app::model::routing::ContractBasedRoutingPayloadWrapper::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::ContractBasedRoutingPayloadWrapper::get_profile_id,
                &app::model::routing::ContractBasedRoutingPayloadWrapper::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "contract_based_routing_payload_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::ContractBasedRoutingSetupPayloadWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "config",
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::get_config,
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::set_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::get_profile_id,
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "features_to_enable",
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::
                    get_features_to_enable,
                &app::model::routing::ContractBasedRoutingSetupPayloadWrapper::
                    set_features_to_enable,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "contract_based_routing_setup_payload_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::routing::ContractBasedRoutingConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "config",
                &app::model::routing::ContractBasedRoutingConfig::get_config,
                &app::model::routing::ContractBasedRoutingConfig::set_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "label_info",
                &app::model::routing::ContractBasedRoutingConfig::get_label_info,
                &app::model::routing::ContractBasedRoutingConfig::set_label_info,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "contract_based_routing_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::ContractBasedRoutingConfigBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "constants",
                &app::model::routing::ContractBasedRoutingConfigBody::get_constants,
                &app::model::routing::ContractBasedRoutingConfigBody::set_constants,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_scale",
                &app::model::routing::ContractBasedRoutingConfigBody::get_time_scale,
                &app::model::routing::ContractBasedRoutingConfigBody::set_time_scale,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "contract_based_routing_config_body";
    }
};

template<>
struct serde::Serializable<app::model::routing::LabelInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "label",
                &app::model::routing::LabelInformation::get_label,
                &app::model::routing::LabelInformation::set_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "target_count",
                &app::model::routing::LabelInformation::get_target_count,
                &app::model::routing::LabelInformation::set_target_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "target_time",
                &app::model::routing::LabelInformation::get_target_time,
                &app::model::routing::LabelInformation::set_target_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mca_id",
                &app::model::routing::LabelInformation::get_mca_id,
                &app::model::routing::LabelInformation::set_mca_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "label_information";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoiceWithBucketName>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routable_connector_choice",
                &app::model::routing::RoutableConnectorChoiceWithBucketName::
                    get_routable_connector_choice,
                &app::model::routing::RoutableConnectorChoiceWithBucketName::
                    set_routable_connector_choice,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bucket_name",
                &app::model::routing::RoutableConnectorChoiceWithBucketName::get_bucket_name,
                &app::model::routing::RoutableConnectorChoiceWithBucketName::set_bucket_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice_with_bucket_name";
    }
};

template<>
struct serde::Serializable<app::model::routing::CalSuccessRateConfigEventRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "min_aggregates_size",
                &app::model::routing::CalSuccessRateConfigEventRequest::get_min_aggregates_size,
                &app::model::routing::CalSuccessRateConfigEventRequest::set_min_aggregates_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_success_rate",
                &app::model::routing::CalSuccessRateConfigEventRequest::get_default_success_rate,
                &app::model::routing::CalSuccessRateConfigEventRequest::set_default_success_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "specificity_level",
                &app::model::routing::CalSuccessRateConfigEventRequest::get_specificity_level,
                &app::model::routing::CalSuccessRateConfigEventRequest::set_specificity_level,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "exploration_percent",
                &app::model::routing::CalSuccessRateConfigEventRequest::get_exploration_percent,
                &app::model::routing::CalSuccessRateConfigEventRequest::set_exploration_percent,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cal_success_rate_config_event_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::CalSuccessRateEventRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::routing::CalSuccessRateEventRequest::get_id,
                &app::model::routing::CalSuccessRateEventRequest::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "params",
                &app::model::routing::CalSuccessRateEventRequest::get_params,
                &app::model::routing::CalSuccessRateEventRequest::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "labels",
                &app::model::routing::CalSuccessRateEventRequest::get_labels,
                &app::model::routing::CalSuccessRateEventRequest::set_labels,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config",
                &app::model::routing::CalSuccessRateEventRequest::get_config,
                &app::model::routing::CalSuccessRateEventRequest::set_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cal_success_rate_event_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationRoutingEventBucketConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bucket_size",
                &app::model::routing::EliminationRoutingEventBucketConfig::get_bucket_size,
                &app::model::routing::EliminationRoutingEventBucketConfig::set_bucket_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bucket_leak_interval_in_secs",
                &app::model::routing::EliminationRoutingEventBucketConfig::
                    get_bucket_leak_interval_in_secs,
                &app::model::routing::EliminationRoutingEventBucketConfig::
                    set_bucket_leak_interval_in_secs,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_routing_event_bucket_config";
    }
};

template<>
struct serde::Serializable<app::model::routing::EliminationRoutingEventRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::routing::EliminationRoutingEventRequest::get_id,
                &app::model::routing::EliminationRoutingEventRequest::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "params",
                &app::model::routing::EliminationRoutingEventRequest::get_params,
                &app::model::routing::EliminationRoutingEventRequest::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "labels",
                &app::model::routing::EliminationRoutingEventRequest::get_labels,
                &app::model::routing::EliminationRoutingEventRequest::set_labels,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config",
                &app::model::routing::EliminationRoutingEventRequest::get_config,
                &app::model::routing::EliminationRoutingEventRequest::set_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "elimination_routing_event_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::CalContractScoreEventRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::routing::CalContractScoreEventRequest::get_id,
                &app::model::routing::CalContractScoreEventRequest::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "params",
                &app::model::routing::CalContractScoreEventRequest::get_params,
                &app::model::routing::CalContractScoreEventRequest::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "labels",
                &app::model::routing::CalContractScoreEventRequest::get_labels,
                &app::model::routing::CalContractScoreEventRequest::set_labels,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config",
                &app::model::routing::CalContractScoreEventRequest::get_config,
                &app::model::routing::CalContractScoreEventRequest::set_config,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cal_contract_score_event_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::LabelWithScoreEventResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "score",
                &app::model::routing::LabelWithScoreEventResponse::get_score,
                &app::model::routing::LabelWithScoreEventResponse::set_score,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "label",
                &app::model::routing::LabelWithScoreEventResponse::get_label,
                &app::model::routing::LabelWithScoreEventResponse::set_label,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "label_with_score_event_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::CalSuccessRateEventResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "labels_with_score",
                &app::model::routing::CalSuccessRateEventResponse::get_labels_with_score,
                &app::model::routing::CalSuccessRateEventResponse::set_labels_with_score,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_approach",
                &app::model::routing::CalSuccessRateEventResponse::get_routing_approach,
                &app::model::routing::CalSuccessRateEventResponse::set_routing_approach,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cal_success_rate_event_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationQuery::get_profile_id,
                &app::model::routing::RuleMigrationQuery::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::routing::RuleMigrationQuery::get_merchant_id,
                &app::model::routing::RuleMigrationQuery::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::routing::RuleMigrationQuery::get_limit,
                &app::model::routing::RuleMigrationQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::routing::RuleMigrationQuery::get_offset,
                &app::model::routing::RuleMigrationQuery::set_offset,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_ids",
                &app::model::routing::RuleMigrationRequest::get_profile_ids,
                &app::model::routing::RuleMigrationRequest::set_profile_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::routing::RuleMigrationRequest::get_limit,
                &app::model::routing::RuleMigrationRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::routing::RuleMigrationRequest::get_offset,
                &app::model::routing::RuleMigrationRequest::set_offset,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profiles",
                &app::model::routing::RuleMigrationResult::get_profiles,
                &app::model::routing::RuleMigrationResult::set_profiles,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "totals",
                &app::model::routing::RuleMigrationResult::get_totals,
                &app::model::routing::RuleMigrationResult::set_totals,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_result";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationProfileResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationProfileResult::get_profile_id,
                &app::model::routing::RuleMigrationProfileResult::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::routing::RuleMigrationProfileResult::get_merchant_id,
                &app::model::routing::RuleMigrationProfileResult::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "success",
                &app::model::routing::RuleMigrationProfileResult::get_success,
                &app::model::routing::RuleMigrationProfileResult::set_success,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skipped",
                &app::model::routing::RuleMigrationProfileResult::get_skipped,
                &app::model::routing::RuleMigrationProfileResult::set_skipped,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "errors",
                &app::model::routing::RuleMigrationProfileResult::get_errors,
                &app::model::routing::RuleMigrationProfileResult::set_errors,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "not_applicable",
                &app::model::routing::RuleMigrationProfileResult::get_not_applicable,
                &app::model::routing::RuleMigrationProfileResult::set_not_applicable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "not_attempted",
                &app::model::routing::RuleMigrationProfileResult::get_not_attempted,
                &app::model::routing::RuleMigrationProfileResult::set_not_attempted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_profile_result";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationNotApplicable>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationNotApplicable::get_profile_id,
                &app::model::routing::RuleMigrationNotApplicable::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::RuleMigrationNotApplicable::get_algorithm_id,
                &app::model::routing::RuleMigrationNotApplicable::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "kind",
                &app::model::routing::RuleMigrationNotApplicable::get_kind,
                &app::model::routing::RuleMigrationNotApplicable::set_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::routing::RuleMigrationNotApplicable::get_reason,
                &app::model::routing::RuleMigrationNotApplicable::set_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_not_applicable";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationTotals>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profiles",
                &app::model::routing::RuleMigrationTotals::get_profiles,
                &app::model::routing::RuleMigrationTotals::set_profiles,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_migrated",
                &app::model::routing::RuleMigrationTotals::get_rules_migrated,
                &app::model::routing::RuleMigrationTotals::set_rules_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_skipped",
                &app::model::routing::RuleMigrationTotals::get_rules_skipped,
                &app::model::routing::RuleMigrationTotals::set_rules_skipped,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_failed",
                &app::model::routing::RuleMigrationTotals::get_rules_failed,
                &app::model::routing::RuleMigrationTotals::set_rules_failed,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_not_applicable",
                &app::model::routing::RuleMigrationTotals::get_rules_not_applicable,
                &app::model::routing::RuleMigrationTotals::set_rules_not_applicable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profiles_not_attempted",
                &app::model::routing::RuleMigrationTotals::get_profiles_not_attempted,
                &app::model::routing::RuleMigrationTotals::set_profiles_not_attempted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_totals";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingMigrationStatusQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::routing::RoutingMigrationStatusQuery::get_limit,
                &app::model::routing::RoutingMigrationStatusQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::routing::RoutingMigrationStatusQuery::get_offset,
                &app::model::routing::RoutingMigrationStatusQuery::set_offset,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_migration_status_query";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingMigrationProfileStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RoutingMigrationProfileStatus::get_profile_id,
                &app::model::routing::RoutingMigrationProfileStatus::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::routing::RoutingMigrationProfileStatus::get_merchant_id,
                &app::model::routing::RoutingMigrationProfileStatus::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_hyperswitch",
                &app::model::routing::RoutingMigrationProfileStatus::get_rules_hyperswitch,
                &app::model::routing::RoutingMigrationProfileStatus::set_rules_hyperswitch,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_out_of_scope",
                &app::model::routing::RoutingMigrationProfileStatus::get_rules_out_of_scope,
                &app::model::routing::RoutingMigrationProfileStatus::set_rules_out_of_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_decision_engine",
                &app::model::routing::RoutingMigrationProfileStatus::get_rules_decision_engine,
                &app::model::routing::RoutingMigrationProfileStatus::set_rules_decision_engine,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_missing_in_decision_engine",
                &app::model::routing::RoutingMigrationProfileStatus::
                    get_rules_missing_in_decision_engine,
                &app::model::routing::RoutingMigrationProfileStatus::
                    set_rules_missing_in_decision_engine,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "rules_only_in_decision_engine",
                &app::model::routing::RoutingMigrationProfileStatus::
                    get_rules_only_in_decision_engine,
                &app::model::routing::RoutingMigrationProfileStatus::
                    set_rules_only_in_decision_engine,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_source",
                &app::model::routing::RoutingMigrationProfileStatus::get_routing_source,
                &app::model::routing::RoutingMigrationProfileStatus::set_routing_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state",
                &app::model::routing::RoutingMigrationProfileStatus::get_state,
                &app::model::routing::RoutingMigrationProfileStatus::set_state,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_migration_profile_status";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingMigrationPageTotals>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profiles",
                &app::model::routing::RoutingMigrationPageTotals::get_profiles,
                &app::model::routing::RoutingMigrationPageTotals::set_profiles,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pending",
                &app::model::routing::RoutingMigrationPageTotals::get_pending,
                &app::model::routing::RoutingMigrationPageTotals::set_pending,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "partial",
                &app::model::routing::RoutingMigrationPageTotals::get_partial,
                &app::model::routing::RoutingMigrationPageTotals::set_partial,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "migrated",
                &app::model::routing::RoutingMigrationPageTotals::get_migrated,
                &app::model::routing::RoutingMigrationPageTotals::set_migrated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled",
                &app::model::routing::RoutingMigrationPageTotals::get_enabled,
                &app::model::routing::RoutingMigrationPageTotals::set_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enabled_without_rules",
                &app::model::routing::RoutingMigrationPageTotals::get_enabled_without_rules,
                &app::model::routing::RoutingMigrationPageTotals::set_enabled_without_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "diverged",
                &app::model::routing::RoutingMigrationPageTotals::get_diverged,
                &app::model::routing::RoutingMigrationPageTotals::set_diverged,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unknown",
                &app::model::routing::RoutingMigrationPageTotals::get_unknown,
                &app::model::routing::RoutingMigrationPageTotals::set_unknown,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_migration_page_totals";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingMigrationStatusResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profiles",
                &app::model::routing::RoutingMigrationStatusResponse::get_profiles,
                &app::model::routing::RoutingMigrationStatusResponse::set_profiles,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::routing::RoutingMigrationStatusResponse::get_limit,
                &app::model::routing::RoutingMigrationStatusResponse::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::routing::RoutingMigrationStatusResponse::get_offset,
                &app::model::routing::RoutingMigrationStatusResponse::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "has_more",
                &app::model::routing::RoutingMigrationStatusResponse::get_has_more,
                &app::model::routing::RoutingMigrationStatusResponse::set_has_more,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "page_totals",
                &app::model::routing::RoutingMigrationStatusResponse::get_page_totals,
                &app::model::routing::RoutingMigrationStatusResponse::set_page_totals,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_migration_status_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationSkipped>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationSkipped::get_profile_id,
                &app::model::routing::RuleMigrationSkipped::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::RuleMigrationSkipped::get_algorithm_id,
                &app::model::routing::RuleMigrationSkipped::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_engine_algorithm_id",
                &app::model::routing::RuleMigrationSkipped::get_decision_engine_algorithm_id,
                &app::model::routing::RuleMigrationSkipped::set_decision_engine_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "linked_active_rule",
                &app::model::routing::RuleMigrationSkipped::get_linked_active_rule,
                &app::model::routing::RuleMigrationSkipped::set_linked_active_rule,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_skipped";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationResponse::get_profile_id,
                &app::model::routing::RuleMigrationResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "euclid_algorithm_id",
                &app::model::routing::RuleMigrationResponse::get_euclid_algorithm_id,
                &app::model::routing::RuleMigrationResponse::set_euclid_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_engine_algorithm_id",
                &app::model::routing::RuleMigrationResponse::get_decision_engine_algorithm_id,
                &app::model::routing::RuleMigrationResponse::set_decision_engine_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_active_rule",
                &app::model::routing::RuleMigrationResponse::get_is_active_rule,
                &app::model::routing::RuleMigrationResponse::set_is_active_rule,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::RuleMigrationError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::model::routing::RuleMigrationError::get_profile_id,
                &app::model::routing::RuleMigrationError::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_id",
                &app::model::routing::RuleMigrationError::get_algorithm_id,
                &app::model::routing::RuleMigrationError::set_algorithm_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error",
                &app::model::routing::RuleMigrationError::get_error,
                &app::model::routing::RuleMigrationError::set_error,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "rule_migration_error";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingEvaluateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "created_by",
                &app::model::routing::RoutingEvaluateRequest::get_created_by,
                &app::model::routing::RoutingEvaluateRequest::set_created_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::routing::RoutingEvaluateRequest::get_payment_id,
                &app::model::routing::RoutingEvaluateRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &app::model::routing::RoutingEvaluateRequest::get_parameters,
                &app::model::routing::RoutingEvaluateRequest::set_parameters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fallback_output",
                &app::model::routing::RoutingEvaluateRequest::get_fallback_output,
                &app::model::routing::RoutingEvaluateRequest::set_fallback_output,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm_for",
                &app::model::routing::RoutingEvaluateRequest::get_algorithm_for,
                &app::model::routing::RoutingEvaluateRequest::set_algorithm_for,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_evaluate_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingEvaluateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "status",
                &app::model::routing::RoutingEvaluateResponse::get_status,
                &app::model::routing::RoutingEvaluateResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "output",
                &app::model::routing::RoutingEvaluateResponse::get_output,
                &app::model::routing::RoutingEvaluateResponse::set_output,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "evaluated_output",
                &app::model::routing::RoutingEvaluateResponse::get_evaluated_output,
                &app::model::routing::RoutingEvaluateResponse::set_evaluated_output,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eligible_connectors",
                &app::model::routing::RoutingEvaluateResponse::get_eligible_connectors,
                &app::model::routing::RoutingEvaluateResponse::set_eligible_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_evaluate_response";
    }
};

template<>
struct serde::Serializable<app::model::routing::DeRoutableConnectorChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gateway_name",
                &app::model::routing::DeRoutableConnectorChoice::get_gateway_name,
                &app::model::routing::DeRoutableConnectorChoice::set_gateway_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_id",
                &app::model::routing::DeRoutableConnectorChoice::get_gateway_id,
                &app::model::routing::DeRoutableConnectorChoice::set_gateway_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "de_routable_connector_choice";
    }
};

template<>
struct serde::Serializable<app::model::routing::MetadataValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "key",
                &app::model::routing::MetadataValue::get_key,
                &app::model::routing::MetadataValue::set_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::model::routing::MetadataValue::get_value,
                &app::model::routing::MetadataValue::set_value,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "metadata_value";
    }
};

template<>
struct serde::Serializable<app::model::routing::NumberComparison>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "comparison_type",
                &app::model::routing::NumberComparison::get_comparison_type,
                &app::model::routing::NumberComparison::set_comparison_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "number",
                &app::model::routing::NumberComparison::get_number,
                &app::model::routing::NumberComparison::set_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "number_comparison";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingEntryRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "target",
            &app::model::routing::RoutingEntryRequest::get_target,
            &app::model::routing::RoutingEntryRequest::set_target,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "routing_entry_request";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutingEntryResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "is_cutover",
                &app::model::routing::RoutingEntryResponse::get_is_cutover,
                &app::model::routing::RoutingEntryResponse::set_is_cutover,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_url",
                &app::model::routing::RoutingEntryResponse::get_redirect_url,
                &app::model::routing::RoutingEntryResponse::set_redirect_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routing_entry_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::routing::tests {
using namespace boost::ut;
suite<"app_model_routing"> app_model_routing_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::model::routing::RoutingConfigRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::routing::tests
#endif
