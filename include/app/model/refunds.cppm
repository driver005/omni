module;
export module app_model:refunds;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::refunds {

class AmountFilterValue : public serde::Serializer<AmountFilterValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CurrencyValue : public serde::Serializer<CurrencyValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class GlobalPaymentIdValue : public serde::Serializer<GlobalPaymentIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorAuthDetailsValue :
    public serde::Serializer<MerchantConnectorAuthDetailsValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorDetailsWrapValue :
    public serde::Serializer<MerchantConnectorDetailsWrapValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorInfoValue : public serde::Serializer<MerchantConnectorInfoValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantIdValue : public serde::Serializer<MerchantIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MinorUnitValue : public serde::Serializer<MinorUnitValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PageOffsetValue : public serde::Serializer<PageOffsetValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PageSizeValue : public serde::Serializer<PageSizeValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentIdValue : public serde::Serializer<PaymentIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodDataValue : public serde::Serializer<PaymentMethodDataValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PrimitiveDateTimeValue : public serde::Serializer<PrimitiveDateTimeValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileIdValue : public serde::Serializer<ProfileIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundAggregateResponse : public serde::Serializer<RefundAggregateResponse>
{
public:
    void set_status_with_count(std::unordered_map<std::string, std::int64_t> value)
    {
        status_with_count = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::int64_t>&
    get_status_with_count() const noexcept
    {
        return status_with_count;
    }

    std::unordered_map<std::string, std::int64_t> status_with_count{};
};

class RefundErrorDetails : public serde::Serializer<RefundErrorDetails>
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

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    std::string code{};
    std::string message{};
};

class RefundFeatureMetadataValue : public serde::Serializer<RefundFeatureMetadataValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundReferenceIdValue : public serde::Serializer<RefundReferenceIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundReverseRequest : public serde::Serializer<RefundReverseRequest>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_cancellation_reason(std::optional<std::string> value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    std::string refund_id{};
    std::optional<std::string> cancellation_reason{};
};

enum class RefundStatus
{
    Succeeded,
    Failed,
    Pending,
    Review,
};

class RefundStatusValue : public serde::Serializer<RefundStatusValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class RefundType
{
    Scheduled,
    Instant,
};

class RefundsRetrieveBody : public serde::Serializer<RefundsRetrieveBody>
{
public:
    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    std::optional<bool> force_sync{};
    std::optional<bool> all_keys_required{};
};

class SecretSerdeValueValue : public serde::Serializer<SecretSerdeValueValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SplitRefundValue : public serde::Serializer<SplitRefundValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class TimeRangeValue : public serde::Serializer<TimeRangeValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PlatformRefundListFilters : public serde::Serializer<PlatformRefundListFilters>
{
public:
    void set_connector(
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value
    )
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>&
    get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::vector<RefundStatusValue> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundStatusValue>& get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<RefundStatusValue> refund_status{};
};

class PlatformRefundListItem : public serde::Serializer<PlatformRefundListItem>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
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

    void set_connector_refund_id(std::optional<std::string> value)
    {
        connector_refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_refund_id() const noexcept
    {
        return connector_refund_id;
    }

    void set_attempt_id(std::string value)
    {
        attempt_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_attempt_id() const noexcept
    {
        return attempt_id;
    }

    void set_refund_amount(MinorUnitValue value)
    {
        refund_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_refund_amount() const noexcept
    {
        return refund_amount;
    }

    void set_total_amount(MinorUnitValue value)
    {
        total_amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_total_amount() const noexcept
    {
        return total_amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(RefundStatusValue value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const RefundStatusValue& get_refund_status() const noexcept
    {
        return refund_status;
    }

    void set_refund_reason(std::optional<std::string> value)
    {
        refund_reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_refund_reason() const noexcept
    {
        return refund_reason;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_modified_at(PrimitiveDateTimeValue value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_modified_at() const noexcept
    {
        return modified_at;
    }

    std::string refund_id{};
    PaymentIdValue payment_id{};
    MerchantIdValue merchant_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::string connector{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<std::string> connector_refund_id{};
    std::string attempt_id{};
    MinorUnitValue refund_amount{};
    MinorUnitValue total_amount{};
    CurrencyValue currency{};
    RefundStatusValue refund_status{};
    std::optional<std::string> refund_reason{};
    std::optional<std::string> description{};
    PrimitiveDateTimeValue created_at{};
    PrimitiveDateTimeValue modified_at{};
};

class PlatformRefundListRequest : public serde::Serializer<PlatformRefundListRequest>
{
public:
    void set_processor_merchant_id(std::optional<MerchantIdValue> value)
    {
        processor_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept
    {
        return processor_merchant_id;
    }

    void set_payment_id(std::optional<PaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_refund_id(std::optional<std::string> value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_limit(std::optional<PageSizeValue> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<PageSizeValue>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_start_amount(std::optional<std::int64_t> value)
    {
        start_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_start_amount() const noexcept
    {
        return start_amount;
    }

    void set_end_amount(std::optional<std::int64_t> value)
    {
        end_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_end_amount() const noexcept
    {
        return end_amount;
    }

    void set_connector(std::optional<std::vector<std::string>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(
        std::optional<std::vector<MerchantConnectorAccountIdValue>> value
    )
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<MerchantConnectorAccountIdValue>>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_currency(std::optional<std::vector<CurrencyValue>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::optional<std::vector<RefundStatusValue>> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<RefundStatusValue>>&
    get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::optional<MerchantIdValue> processor_merchant_id{};
    std::optional<PaymentIdValue> payment_id{};
    std::optional<std::string> refund_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<PageSizeValue> limit{};
    PageOffsetValue offset{};
    std::optional<TimeRangeValue> time_range{};
    std::optional<std::int64_t> start_amount{};
    std::optional<std::int64_t> end_amount{};
    std::optional<std::vector<std::string>> connector{};
    std::optional<std::vector<MerchantConnectorAccountIdValue>> merchant_connector_id{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<RefundStatusValue>> refund_status{};
};

class RefundListFilters : public serde::Serializer<RefundListFilters>
{
public:
    void set_connector(
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value
    )
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>&
    get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::vector<RefundStatusValue> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundStatusValue>& get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<RefundStatusValue> refund_status{};
};

class RefundListMetaData : public serde::Serializer<RefundListMetaData>
{
public:
    void set_connector(std::vector<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::vector<RefundStatusValue> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundStatusValue>& get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::vector<std::string> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<RefundStatusValue> refund_status{};
};

class RefundListRequest : public serde::Serializer<RefundListRequest>
{
public:
    void set_payment_id(std::optional<PaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_refund_id(std::optional<std::string> value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_limit(std::optional<PageSizeValue> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<PageSizeValue>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_amount_filter(std::optional<AmountFilterValue> value)
    {
        amount_filter = std::move(value);
    }

    [[nodiscard]] const std::optional<AmountFilterValue>& get_amount_filter() const noexcept
    {
        return amount_filter;
    }

    void set_connector(std::optional<std::vector<std::string>> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(
        std::optional<std::vector<MerchantConnectorAccountIdValue>> value
    )
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<MerchantConnectorAccountIdValue>>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_currency(std::optional<std::vector<CurrencyValue>> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::optional<std::vector<RefundStatusValue>> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<RefundStatusValue>>&
    get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::optional<PaymentIdValue> payment_id{};
    std::optional<std::string> refund_id{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<PageSizeValue> limit{};
    PageOffsetValue offset{};
    std::optional<TimeRangeValue> time_range{};
    std::optional<AmountFilterValue> amount_filter{};
    std::optional<std::vector<std::string>> connector{};
    std::optional<std::vector<MerchantConnectorAccountIdValue>> merchant_connector_id{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<std::vector<RefundStatusValue>> refund_status{};
};

class RefundManualUpdateRequest : public serde::Serializer<RefundManualUpdateRequest>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_connector_refund_id(std::optional<std::string> value)
    {
        connector_refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_refund_id() const noexcept
    {
        return connector_refund_id;
    }

    void set_status(std::optional<RefundStatus> value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundStatus>& get_status() const noexcept
    {
        return status;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    std::string refund_id{};
    MerchantIdValue merchant_id{};
    std::optional<std::string> connector_refund_id{};
    std::optional<RefundStatus> status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
};

class RefundMetadataUpdateRequest : public serde::Serializer<RefundMetadataUpdateRequest>
{
public:
    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    std::optional<std::string> reason{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class RefundRequest : public serde::Serializer<RefundRequest>
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

    void set_refund_id(std::optional<std::string> value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_refund_type(std::optional<RefundType> value)
    {
        refund_type = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundType>& get_refund_type() const noexcept
    {
        return refund_type;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_split_refunds(std::optional<SplitRefundValue> value)
    {
        split_refunds = std::move(value);
    }

    [[nodiscard]] const std::optional<SplitRefundValue>& get_split_refunds() const noexcept
    {
        return split_refunds;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataValue> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataValue>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_feature_metadata(std::optional<RefundFeatureMetadataValue> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundFeatureMetadataValue>&
    get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    PaymentIdValue payment_id{};
    std::optional<std::string> refund_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<MinorUnitValue> amount{};
    std::optional<std::string> reason{};
    std::optional<RefundType> refund_type{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
    std::optional<SplitRefundValue> split_refunds{};
    std::optional<bool> all_keys_required{};
    std::optional<PaymentMethodDataValue> payment_method_data{};
    std::optional<RefundFeatureMetadataValue> feature_metadata{};
};

class RefundResponse : public serde::Serializer<RefundResponse>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

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

    void set_net_amount(std::optional<MinorUnitValue> value)
    {
        net_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_net_amount() const noexcept
    {
        return net_amount;
    }

    void set_surcharge_refund_amount(std::optional<MinorUnitValue> value)
    {
        surcharge_refund_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_surcharge_refund_amount() const noexcept
    {
        return surcharge_refund_amount;
    }

    void set_currency(std::string value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(RefundStatus value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const RefundStatus& get_status() const noexcept
    {
        return status;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_unified_code(std::optional<std::string> value)
    {
        unified_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept
    {
        return unified_code;
    }

    void set_unified_message(std::optional<std::string> value)
    {
        unified_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept
    {
        return unified_message;
    }

    void set_created_at(std::optional<PrimitiveDateTimeValue> value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_updated_at(std::optional<PrimitiveDateTimeValue> value)
    {
        updated_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_updated_at() const noexcept
    {
        return updated_at;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
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

    void set_split_refunds(std::optional<SplitRefundValue> value)
    {
        split_refunds = std::move(value);
    }

    [[nodiscard]] const std::optional<SplitRefundValue>& get_split_refunds() const noexcept
    {
        return split_refunds;
    }

    void set_issuer_error_code(std::optional<std::string> value)
    {
        issuer_error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_code() const noexcept
    {
        return issuer_error_code;
    }

    void set_issuer_error_message(std::optional<std::string> value)
    {
        issuer_error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_error_message() const noexcept
    {
        return issuer_error_message;
    }

    void set_raw_connector_response(std::optional<std::string> value)
    {
        raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_raw_connector_response() const noexcept
    {
        return raw_connector_response;
    }

    void set_connector_refund_id(std::optional<std::string> value)
    {
        connector_refund_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_refund_id() const noexcept
    {
        return connector_refund_id;
    }

    void set_feature_metadata(std::optional<RefundFeatureMetadataValue> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundFeatureMetadataValue>&
    get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    std::string refund_id{};
    PaymentIdValue payment_id{};
    MinorUnitValue amount{};
    std::optional<MinorUnitValue> net_amount{};
    std::optional<MinorUnitValue> surcharge_refund_amount{};
    std::string currency{};
    RefundStatus status{};
    std::optional<std::string> reason{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> error_message{};
    std::optional<std::string> error_code{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<PrimitiveDateTimeValue> created_at{};
    std::optional<PrimitiveDateTimeValue> updated_at{};
    std::string connector{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<SplitRefundValue> split_refunds{};
    std::optional<std::string> issuer_error_code{};
    std::optional<std::string> issuer_error_message{};
    std::optional<std::string> raw_connector_response{};
    std::optional<std::string> connector_refund_id{};
    std::optional<RefundFeatureMetadataValue> feature_metadata{};
};

class RefundUpdateRequest : public serde::Serializer<RefundUpdateRequest>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    std::string refund_id{};
    std::optional<std::string> reason{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class RefundsCreateRequest : public serde::Serializer<RefundsCreateRequest>
{
public:
    void set_payment_id(GlobalPaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const GlobalPaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_merchant_reference_id(RefundReferenceIdValue value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const RefundReferenceIdValue& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_refund_type(std::optional<RefundType> value)
    {
        refund_type = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundType>& get_refund_type() const noexcept
    {
        return refund_type;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorAuthDetailsValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAuthDetailsValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_return_raw_connector_response(std::optional<bool> value)
    {
        return_raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_return_raw_connector_response() const noexcept
    {
        return return_raw_connector_response;
    }

    void set_feature_metadata(std::optional<RefundFeatureMetadataValue> value)
    {
        feature_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundFeatureMetadataValue>&
    get_feature_metadata() const noexcept
    {
        return feature_metadata;
    }

    GlobalPaymentIdValue payment_id{};
    RefundReferenceIdValue merchant_reference_id{};
    std::optional<MerchantIdValue> merchant_id{};
    std::optional<MinorUnitValue> amount{};
    std::optional<std::string> reason{};
    std::optional<RefundType> refund_type{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<MerchantConnectorAuthDetailsValue> merchant_connector_details{};
    std::optional<bool> return_raw_connector_response{};
    std::optional<RefundFeatureMetadataValue> feature_metadata{};
};

class RefundsRetrievePayload : public serde::Serializer<RefundsRetrievePayload>
{
public:
    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorAuthDetailsValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAuthDetailsValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_return_raw_connector_response(std::optional<bool> value)
    {
        return_raw_connector_response = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_return_raw_connector_response() const noexcept
    {
        return return_raw_connector_response;
    }

    std::optional<bool> force_sync{};
    std::optional<MerchantConnectorAuthDetailsValue> merchant_connector_details{};
    std::optional<bool> return_raw_connector_response{};
};

class RefundsRetrieveRequest : public serde::Serializer<RefundsRetrieveRequest>
{
public:
    void set_refund_id(std::string value)
    {
        refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return refund_id;
    }

    void set_force_sync(std::optional<bool> value)
    {
        force_sync = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept
    {
        return force_sync;
    }

    void set_merchant_connector_details(std::optional<MerchantConnectorDetailsWrapValue> value)
    {
        merchant_connector_details = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorDetailsWrapValue>&
    get_merchant_connector_details() const noexcept
    {
        return merchant_connector_details;
    }

    void set_all_keys_required(std::optional<bool> value)
    {
        all_keys_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_all_keys_required() const noexcept
    {
        return all_keys_required;
    }

    std::string refund_id{};
    std::optional<bool> force_sync{};
    std::optional<MerchantConnectorDetailsWrapValue> merchant_connector_details{};
    std::optional<bool> all_keys_required{};
};

class PlatformRefundListResponse : public serde::Serializer<PlatformRefundListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<PlatformRefundListItem> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<PlatformRefundListItem>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PlatformRefundListItem> data{};
};

class RefundListResponse : public serde::Serializer<RefundListResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::int64_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<RefundResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::int64_t total_count{};
    std::vector<RefundResponse> data{};
};
} // namespace app::model::refunds

template<>
struct serde::Serializable<app::model::refunds::RefundRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::RefundRequest::get_payment_id,
                &app::model::refunds::RefundRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundRequest::get_refund_id,
                &app::model::refunds::RefundRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::refunds::RefundRequest::get_merchant_id,
                &app::model::refunds::RefundRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::refunds::RefundRequest::get_amount,
                &app::model::refunds::RefundRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundRequest::get_reason,
                &app::model::refunds::RefundRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_type",
                &app::model::refunds::RefundRequest::get_refund_type,
                &app::model::refunds::RefundRequest::set_refund_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundRequest::get_metadata,
                &app::model::refunds::RefundRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::refunds::RefundRequest::get_merchant_connector_details,
                &app::model::refunds::RefundRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split_refunds",
                &app::model::refunds::RefundRequest::get_split_refunds,
                &app::model::refunds::RefundRequest::set_split_refunds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::refunds::RefundRequest::get_all_keys_required,
                &app::model::refunds::RefundRequest::set_all_keys_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::refunds::RefundRequest::get_payment_method_data,
                &app::model::refunds::RefundRequest::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::refunds::RefundRequest::get_feature_metadata,
                &app::model::refunds::RefundRequest::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundsCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::RefundsCreateRequest::get_payment_id,
                &app::model::refunds::RefundsCreateRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::refunds::RefundsCreateRequest::get_merchant_reference_id,
                &app::model::refunds::RefundsCreateRequest::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::refunds::RefundsCreateRequest::get_merchant_id,
                &app::model::refunds::RefundsCreateRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::refunds::RefundsCreateRequest::get_amount,
                &app::model::refunds::RefundsCreateRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundsCreateRequest::get_reason,
                &app::model::refunds::RefundsCreateRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_type",
                &app::model::refunds::RefundsCreateRequest::get_refund_type,
                &app::model::refunds::RefundsCreateRequest::set_refund_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundsCreateRequest::get_metadata,
                &app::model::refunds::RefundsCreateRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::refunds::RefundsCreateRequest::get_merchant_connector_details,
                &app::model::refunds::RefundsCreateRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_raw_connector_response",
                &app::model::refunds::RefundsCreateRequest::get_return_raw_connector_response,
                &app::model::refunds::RefundsCreateRequest::set_return_raw_connector_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::refunds::RefundsCreateRequest::get_feature_metadata,
                &app::model::refunds::RefundsCreateRequest::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_create_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundsRetrieveBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "force_sync",
                &app::model::refunds::RefundsRetrieveBody::get_force_sync,
                &app::model::refunds::RefundsRetrieveBody::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::refunds::RefundsRetrieveBody::get_all_keys_required,
                &app::model::refunds::RefundsRetrieveBody::set_all_keys_required,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_retrieve_body";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundsRetrievePayload>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "force_sync",
                &app::model::refunds::RefundsRetrievePayload::get_force_sync,
                &app::model::refunds::RefundsRetrievePayload::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::refunds::RefundsRetrievePayload::get_merchant_connector_details,
                &app::model::refunds::RefundsRetrievePayload::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_raw_connector_response",
                &app::model::refunds::RefundsRetrievePayload::get_return_raw_connector_response,
                &app::model::refunds::RefundsRetrievePayload::set_return_raw_connector_response,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_retrieve_payload";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundsRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundsRetrieveRequest::get_refund_id,
                &app::model::refunds::RefundsRetrieveRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_sync",
                &app::model::refunds::RefundsRetrieveRequest::get_force_sync,
                &app::model::refunds::RefundsRetrieveRequest::set_force_sync,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_details",
                &app::model::refunds::RefundsRetrieveRequest::get_merchant_connector_details,
                &app::model::refunds::RefundsRetrieveRequest::set_merchant_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "all_keys_required",
                &app::model::refunds::RefundsRetrieveRequest::get_all_keys_required,
                &app::model::refunds::RefundsRetrieveRequest::set_all_keys_required,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_retrieve_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundUpdateRequest::get_refund_id,
                &app::model::refunds::RefundUpdateRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundUpdateRequest::get_reason,
                &app::model::refunds::RefundUpdateRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundUpdateRequest::get_metadata,
                &app::model::refunds::RefundUpdateRequest::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_update_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundReverseRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundReverseRequest::get_refund_id,
                &app::model::refunds::RefundReverseRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &app::model::refunds::RefundReverseRequest::get_cancellation_reason,
                &app::model::refunds::RefundReverseRequest::set_cancellation_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_reverse_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundMetadataUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundMetadataUpdateRequest::get_reason,
                &app::model::refunds::RefundMetadataUpdateRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundMetadataUpdateRequest::get_metadata,
                &app::model::refunds::RefundMetadataUpdateRequest::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_metadata_update_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundManualUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundManualUpdateRequest::get_refund_id,
                &app::model::refunds::RefundManualUpdateRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::refunds::RefundManualUpdateRequest::get_merchant_id,
                &app::model::refunds::RefundManualUpdateRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_refund_id",
                &app::model::refunds::RefundManualUpdateRequest::get_connector_refund_id,
                &app::model::refunds::RefundManualUpdateRequest::set_connector_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::refunds::RefundManualUpdateRequest::get_status,
                &app::model::refunds::RefundManualUpdateRequest::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::refunds::RefundManualUpdateRequest::get_error_code,
                &app::model::refunds::RefundManualUpdateRequest::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::refunds::RefundManualUpdateRequest::get_error_message,
                &app::model::refunds::RefundManualUpdateRequest::set_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_manual_update_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundResponse::get_refund_id,
                &app::model::refunds::RefundResponse::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::RefundResponse::get_payment_id,
                &app::model::refunds::RefundResponse::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::refunds::RefundResponse::get_amount,
                &app::model::refunds::RefundResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "net_amount",
                &app::model::refunds::RefundResponse::get_net_amount,
                &app::model::refunds::RefundResponse::set_net_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_refund_amount",
                &app::model::refunds::RefundResponse::get_surcharge_refund_amount,
                &app::model::refunds::RefundResponse::set_surcharge_refund_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::RefundResponse::get_currency,
                &app::model::refunds::RefundResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::refunds::RefundResponse::get_status,
                &app::model::refunds::RefundResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundResponse::get_reason,
                &app::model::refunds::RefundResponse::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundResponse::get_metadata,
                &app::model::refunds::RefundResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::refunds::RefundResponse::get_error_message,
                &app::model::refunds::RefundResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::refunds::RefundResponse::get_error_code,
                &app::model::refunds::RefundResponse::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_code",
                &app::model::refunds::RefundResponse::get_unified_code,
                &app::model::refunds::RefundResponse::set_unified_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unified_message",
                &app::model::refunds::RefundResponse::get_unified_message,
                &app::model::refunds::RefundResponse::set_unified_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::refunds::RefundResponse::get_created_at,
                &app::model::refunds::RefundResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::model::refunds::RefundResponse::get_updated_at,
                &app::model::refunds::RefundResponse::set_updated_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::refunds::RefundResponse::get_connector,
                &app::model::refunds::RefundResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::refunds::RefundResponse::get_profile_id,
                &app::model::refunds::RefundResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::refunds::RefundResponse::get_merchant_connector_id,
                &app::model::refunds::RefundResponse::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split_refunds",
                &app::model::refunds::RefundResponse::get_split_refunds,
                &app::model::refunds::RefundResponse::set_split_refunds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_code",
                &app::model::refunds::RefundResponse::get_issuer_error_code,
                &app::model::refunds::RefundResponse::set_issuer_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_error_message",
                &app::model::refunds::RefundResponse::get_issuer_error_message,
                &app::model::refunds::RefundResponse::set_issuer_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "raw_connector_response",
                &app::model::refunds::RefundResponse::get_raw_connector_response,
                &app::model::refunds::RefundResponse::set_raw_connector_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_refund_id",
                &app::model::refunds::RefundResponse::get_connector_refund_id,
                &app::model::refunds::RefundResponse::set_connector_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "feature_metadata",
                &app::model::refunds::RefundResponse::get_feature_metadata,
                &app::model::refunds::RefundResponse::set_feature_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_response";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundErrorDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::model::refunds::RefundErrorDetails::get_code,
                &app::model::refunds::RefundErrorDetails::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::refunds::RefundErrorDetails::get_message,
                &app::model::refunds::RefundErrorDetails::set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_error_details";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::RefundListRequest::get_payment_id,
                &app::model::refunds::RefundListRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::RefundListRequest::get_refund_id,
                &app::model::refunds::RefundListRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::refunds::RefundListRequest::get_profile_id,
                &app::model::refunds::RefundListRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::refunds::RefundListRequest::get_limit,
                &app::model::refunds::RefundListRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::refunds::RefundListRequest::get_offset,
                &app::model::refunds::RefundListRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::refunds::RefundListRequest::get_time_range,
                &app::model::refunds::RefundListRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount_filter",
                &app::model::refunds::RefundListRequest::get_amount_filter,
                &app::model::refunds::RefundListRequest::set_amount_filter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::refunds::RefundListRequest::get_connector,
                &app::model::refunds::RefundListRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::refunds::RefundListRequest::get_merchant_connector_id,
                &app::model::refunds::RefundListRequest::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::RefundListRequest::get_currency,
                &app::model::refunds::RefundListRequest::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::RefundListRequest::get_refund_status,
                &app::model::refunds::RefundListRequest::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_list_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::refunds::RefundListResponse::get_count,
                &app::model::refunds::RefundListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::refunds::RefundListResponse::get_total_count,
                &app::model::refunds::RefundListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::refunds::RefundListResponse::get_data,
                &app::model::refunds::RefundListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_list_response";
    }
};

template<>
struct serde::Serializable<app::model::refunds::PlatformRefundListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::refunds::PlatformRefundListRequest::get_processor_merchant_id,
                &app::model::refunds::PlatformRefundListRequest::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::PlatformRefundListRequest::get_payment_id,
                &app::model::refunds::PlatformRefundListRequest::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::PlatformRefundListRequest::get_refund_id,
                &app::model::refunds::PlatformRefundListRequest::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::refunds::PlatformRefundListRequest::get_profile_id,
                &app::model::refunds::PlatformRefundListRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::refunds::PlatformRefundListRequest::get_limit,
                &app::model::refunds::PlatformRefundListRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::refunds::PlatformRefundListRequest::get_offset,
                &app::model::refunds::PlatformRefundListRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::refunds::PlatformRefundListRequest::get_time_range,
                &app::model::refunds::PlatformRefundListRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "start_amount",
                &app::model::refunds::PlatformRefundListRequest::get_start_amount,
                &app::model::refunds::PlatformRefundListRequest::set_start_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_amount",
                &app::model::refunds::PlatformRefundListRequest::get_end_amount,
                &app::model::refunds::PlatformRefundListRequest::set_end_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::refunds::PlatformRefundListRequest::get_connector,
                &app::model::refunds::PlatformRefundListRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::refunds::PlatformRefundListRequest::get_merchant_connector_id,
                &app::model::refunds::PlatformRefundListRequest::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::PlatformRefundListRequest::get_currency,
                &app::model::refunds::PlatformRefundListRequest::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::PlatformRefundListRequest::get_refund_status,
                &app::model::refunds::PlatformRefundListRequest::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_refund_list_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::PlatformRefundListItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "refund_id",
                &app::model::refunds::PlatformRefundListItem::get_refund_id,
                &app::model::refunds::PlatformRefundListItem::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::refunds::PlatformRefundListItem::get_payment_id,
                &app::model::refunds::PlatformRefundListItem::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::refunds::PlatformRefundListItem::get_merchant_id,
                &app::model::refunds::PlatformRefundListItem::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_merchant_id",
                &app::model::refunds::PlatformRefundListItem::get_processor_merchant_id,
                &app::model::refunds::PlatformRefundListItem::set_processor_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::refunds::PlatformRefundListItem::get_profile_id,
                &app::model::refunds::PlatformRefundListItem::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::refunds::PlatformRefundListItem::get_connector,
                &app::model::refunds::PlatformRefundListItem::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::refunds::PlatformRefundListItem::get_merchant_connector_id,
                &app::model::refunds::PlatformRefundListItem::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_refund_id",
                &app::model::refunds::PlatformRefundListItem::get_connector_refund_id,
                &app::model::refunds::PlatformRefundListItem::set_connector_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempt_id",
                &app::model::refunds::PlatformRefundListItem::get_attempt_id,
                &app::model::refunds::PlatformRefundListItem::set_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_amount",
                &app::model::refunds::PlatformRefundListItem::get_refund_amount,
                &app::model::refunds::PlatformRefundListItem::set_refund_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_amount",
                &app::model::refunds::PlatformRefundListItem::get_total_amount,
                &app::model::refunds::PlatformRefundListItem::set_total_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::PlatformRefundListItem::get_currency,
                &app::model::refunds::PlatformRefundListItem::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::PlatformRefundListItem::get_refund_status,
                &app::model::refunds::PlatformRefundListItem::set_refund_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_reason",
                &app::model::refunds::PlatformRefundListItem::get_refund_reason,
                &app::model::refunds::PlatformRefundListItem::set_refund_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::refunds::PlatformRefundListItem::get_description,
                &app::model::refunds::PlatformRefundListItem::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::refunds::PlatformRefundListItem::get_created_at,
                &app::model::refunds::PlatformRefundListItem::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::refunds::PlatformRefundListItem::get_modified_at,
                &app::model::refunds::PlatformRefundListItem::set_modified_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_refund_list_item";
    }
};

template<>
struct serde::Serializable<app::model::refunds::PlatformRefundListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::refunds::PlatformRefundListResponse::get_count,
                &app::model::refunds::PlatformRefundListResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::refunds::PlatformRefundListResponse::get_total_count,
                &app::model::refunds::PlatformRefundListResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::refunds::PlatformRefundListResponse::get_data,
                &app::model::refunds::PlatformRefundListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_refund_list_response";
    }
};

template<>
struct serde::Serializable<app::model::refunds::PlatformRefundListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::refunds::PlatformRefundListFilters::get_connector,
                &app::model::refunds::PlatformRefundListFilters::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::PlatformRefundListFilters::get_currency,
                &app::model::refunds::PlatformRefundListFilters::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::PlatformRefundListFilters::get_refund_status,
                &app::model::refunds::PlatformRefundListFilters::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "platform_refund_list_filters";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundListMetaData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::refunds::RefundListMetaData::get_connector,
                &app::model::refunds::RefundListMetaData::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::RefundListMetaData::get_currency,
                &app::model::refunds::RefundListMetaData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::RefundListMetaData::get_refund_status,
                &app::model::refunds::RefundListMetaData::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_list_meta_data";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::refunds::RefundListFilters::get_connector,
                &app::model::refunds::RefundListFilters::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::RefundListFilters::get_currency,
                &app::model::refunds::RefundListFilters::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::RefundListFilters::get_refund_status,
                &app::model::refunds::RefundListFilters::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_list_filters";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundAggregateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "status_with_count",
            &app::model::refunds::RefundAggregateResponse::get_status_with_count,
            &app::model::refunds::RefundAggregateResponse::set_status_with_count,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "refund_aggregate_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::refunds::tests {
using namespace boost::ut;
suite<"app_model_refunds"> app_model_refunds_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::model::refunds::RefundRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::refunds::tests
#endif
