module;
export module app_dto:analytics;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::analytics {

class ActivePaymentsMetricsValue
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

class ApiEventDimensionsValue
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

class ApiEventFiltersValue
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

class ApiEventMetricsValue
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

class AuthEventDimensionsValue
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

class AuthEventFiltersValue
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

class AuthEventMetricsValue
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

class AuthEventsAnalyticsMetadata
{
public:
    void set_total_error_message_count(std::optional<std::uint64_t> value)
    {
        total_error_message_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_total_error_message_count() const noexcept
    {
        return total_error_message_count;
    }

    std::optional<std::uint64_t> total_error_message_count{};
};

class AuthInfoValue
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

class DisputeDimensionsValue
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

class DisputeFiltersValue
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

class DisputeMetricsValue
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

class DisputesAnalyticsMetadata
{
public:
    void set_total_disputed_amount(std::optional<std::uint64_t> value)
    {
        total_disputed_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_total_disputed_amount() const noexcept
    {
        return total_disputed_amount;
    }

    void set_total_dispute_lost_amount(std::optional<std::uint64_t> value)
    {
        total_dispute_lost_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_total_dispute_lost_amount() const noexcept
    {
        return total_dispute_lost_amount;
    }

    std::optional<std::uint64_t> total_disputed_amount{};
    std::optional<std::uint64_t> total_dispute_lost_amount{};
};

class EmailValue
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

class FrmDimensionsValue
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

class FrmFiltersValue
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

class FrmMetricsValue
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

enum class Granularity
{
    OneMin,
    FiveMin,
    FifteenMin,
    ThirtyMin,
    OneHour,
    OneDay,
};

class MerchantIdValue
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

class NameDescription
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

    void set_desc(std::string value)
    {
        desc = std::move(value);
    }

    [[nodiscard]] const std::string& get_desc() const noexcept
    {
        return desc;
    }

    std::string name{};
    std::string desc{};
};

class PaymentDimensionsValue
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

class PaymentDistributionsValue
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

class PaymentFiltersValue
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

class PaymentIntentDimensionsValue
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

class PaymentIntentFiltersValue
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

class PaymentIntentMetricsValue
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

class PaymentIntentsAnalyticsMetadata
{
public:
    void set_total_success_rate(std::optional<double> value)
    {
        total_success_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_total_success_rate() const noexcept
    {
        return total_success_rate;
    }

    void set_total_success_rate_without_smart_retries(std::optional<double> value)
    {
        total_success_rate_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<double>&
    get_total_success_rate_without_smart_retries() const noexcept
    {
        return total_success_rate_without_smart_retries;
    }

    void set_total_smart_retried_amount(std::optional<std::uint64_t> value)
    {
        total_smart_retried_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_smart_retried_amount() const noexcept
    {
        return total_smart_retried_amount;
    }

    void set_total_smart_retried_amount_without_smart_retries(std::optional<std::uint64_t> value)
    {
        total_smart_retried_amount_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_smart_retried_amount_without_smart_retries() const noexcept
    {
        return total_smart_retried_amount_without_smart_retries;
    }

    void set_total_payment_processed_amount(std::optional<std::uint64_t> value)
    {
        total_payment_processed_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount() const noexcept
    {
        return total_payment_processed_amount;
    }

    void set_total_payment_processed_amount_without_smart_retries(
        std::optional<std::uint64_t> value
    )
    {
        total_payment_processed_amount_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_without_smart_retries() const noexcept
    {
        return total_payment_processed_amount_without_smart_retries;
    }

    void set_total_smart_retried_amount_in_usd(std::optional<std::uint64_t> value)
    {
        total_smart_retried_amount_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_smart_retried_amount_in_usd() const noexcept
    {
        return total_smart_retried_amount_in_usd;
    }

    void set_total_smart_retried_amount_without_smart_retries_in_usd(
        std::optional<std::uint64_t> value
    )
    {
        total_smart_retried_amount_without_smart_retries_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_smart_retried_amount_without_smart_retries_in_usd() const noexcept
    {
        return total_smart_retried_amount_without_smart_retries_in_usd;
    }

    void set_total_payment_processed_amount_in_usd(std::optional<std::uint64_t> value)
    {
        total_payment_processed_amount_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_in_usd() const noexcept
    {
        return total_payment_processed_amount_in_usd;
    }

    void set_total_payment_processed_amount_without_smart_retries_in_usd(
        std::optional<std::uint64_t> value
    )
    {
        total_payment_processed_amount_without_smart_retries_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_without_smart_retries_in_usd() const noexcept
    {
        return total_payment_processed_amount_without_smart_retries_in_usd;
    }

    void set_total_payment_processed_count(std::optional<std::uint64_t> value)
    {
        total_payment_processed_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_count() const noexcept
    {
        return total_payment_processed_count;
    }

    void set_total_payment_processed_count_without_smart_retries(std::optional<std::uint64_t> value)
    {
        total_payment_processed_count_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_count_without_smart_retries() const noexcept
    {
        return total_payment_processed_count_without_smart_retries;
    }

    std::optional<double> total_success_rate{};
    std::optional<double> total_success_rate_without_smart_retries{};
    std::optional<std::uint64_t> total_smart_retried_amount{};
    std::optional<std::uint64_t> total_smart_retried_amount_without_smart_retries{};
    std::optional<std::uint64_t> total_payment_processed_amount{};
    std::optional<std::uint64_t> total_payment_processed_amount_without_smart_retries{};
    std::optional<std::uint64_t> total_smart_retried_amount_in_usd{};
    std::optional<std::uint64_t> total_smart_retried_amount_without_smart_retries_in_usd{};
    std::optional<std::uint64_t> total_payment_processed_amount_in_usd{};
    std::optional<std::uint64_t> total_payment_processed_amount_without_smart_retries_in_usd{};
    std::optional<std::uint64_t> total_payment_processed_count{};
    std::optional<std::uint64_t> total_payment_processed_count_without_smart_retries{};
};

class PaymentMetricsValue
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

class PaymentsAnalyticsMetadata
{
public:
    void set_total_payment_processed_amount(std::optional<std::uint64_t> value)
    {
        total_payment_processed_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount() const noexcept
    {
        return total_payment_processed_amount;
    }

    void set_total_payment_processed_amount_in_usd(std::optional<std::uint64_t> value)
    {
        total_payment_processed_amount_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_in_usd() const noexcept
    {
        return total_payment_processed_amount_in_usd;
    }

    void set_total_payment_processed_amount_without_smart_retries(
        std::optional<std::uint64_t> value
    )
    {
        total_payment_processed_amount_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_without_smart_retries() const noexcept
    {
        return total_payment_processed_amount_without_smart_retries;
    }

    void set_total_payment_processed_amount_without_smart_retries_usd(
        std::optional<std::uint64_t> value
    )
    {
        total_payment_processed_amount_without_smart_retries_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_amount_without_smart_retries_usd() const noexcept
    {
        return total_payment_processed_amount_without_smart_retries_usd;
    }

    void set_total_payment_processed_count(std::optional<std::uint64_t> value)
    {
        total_payment_processed_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_count() const noexcept
    {
        return total_payment_processed_count;
    }

    void set_total_payment_processed_count_without_smart_retries(std::optional<std::uint64_t> value)
    {
        total_payment_processed_count_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_payment_processed_count_without_smart_retries() const noexcept
    {
        return total_payment_processed_count_without_smart_retries;
    }

    void set_total_failure_reasons_count(std::optional<std::uint64_t> value)
    {
        total_failure_reasons_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_failure_reasons_count() const noexcept
    {
        return total_failure_reasons_count;
    }

    void set_total_failure_reasons_count_without_smart_retries(std::optional<std::uint64_t> value)
    {
        total_failure_reasons_count_without_smart_retries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_failure_reasons_count_without_smart_retries() const noexcept
    {
        return total_failure_reasons_count_without_smart_retries;
    }

    std::optional<std::uint64_t> total_payment_processed_amount{};
    std::optional<std::uint64_t> total_payment_processed_amount_in_usd{};
    std::optional<std::uint64_t> total_payment_processed_amount_without_smart_retries{};
    std::optional<std::uint64_t> total_payment_processed_amount_without_smart_retries_usd{};
    std::optional<std::uint64_t> total_payment_processed_count{};
    std::optional<std::uint64_t> total_payment_processed_count_without_smart_retries{};
    std::optional<std::uint64_t> total_failure_reasons_count{};
    std::optional<std::uint64_t> total_failure_reasons_count_without_smart_retries{};
};

enum class QueryLimit
{
    Top5,
    Top10,
};

class RefundDimensionsValue
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

class RefundDistributionsValue
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

class RefundFiltersValue
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

class RefundMetricsValue
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

class RefundsAnalyticsMetadata
{
public:
    void set_total_refund_success_rate(std::optional<double> value)
    {
        total_refund_success_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_total_refund_success_rate() const noexcept
    {
        return total_refund_success_rate;
    }

    void set_total_refund_processed_amount(std::optional<std::uint64_t> value)
    {
        total_refund_processed_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_refund_processed_amount() const noexcept
    {
        return total_refund_processed_amount;
    }

    void set_total_refund_processed_amount_in_usd(std::optional<std::uint64_t> value)
    {
        total_refund_processed_amount_in_usd = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_refund_processed_amount_in_usd() const noexcept
    {
        return total_refund_processed_amount_in_usd;
    }

    void set_total_refund_processed_count(std::optional<std::uint64_t> value)
    {
        total_refund_processed_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_refund_processed_count() const noexcept
    {
        return total_refund_processed_count;
    }

    void set_total_refund_reason_count(std::optional<std::uint64_t> value)
    {
        total_refund_reason_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_total_refund_reason_count() const noexcept
    {
        return total_refund_reason_count;
    }

    void set_total_refund_error_message_count(std::optional<std::uint64_t> value)
    {
        total_refund_error_message_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_total_refund_error_message_count() const noexcept
    {
        return total_refund_error_message_count;
    }

    std::optional<double> total_refund_success_rate{};
    std::optional<std::uint64_t> total_refund_processed_amount{};
    std::optional<std::uint64_t> total_refund_processed_amount_in_usd{};
    std::optional<std::uint64_t> total_refund_processed_count{};
    std::optional<std::uint64_t> total_refund_reason_count{};
    std::optional<std::uint64_t> total_refund_error_message_count{};
};

enum class ReportColumns
{
    Payment,
};

enum class ReportType
{
    V2Payments,
    RevenueRecovery,
};

class SankeyResponse
{
public:
    void set_count(std::int64_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_count() const noexcept
    {
        return count;
    }

    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    void set_refunds_status(std::optional<std::string> value)
    {
        refunds_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_refunds_status() const noexcept
    {
        return refunds_status;
    }

    void set_dispute_status(std::optional<std::string> value)
    {
        dispute_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_dispute_status() const noexcept
    {
        return dispute_status;
    }

    void set_first_attempt(std::int64_t value)
    {
        first_attempt = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_first_attempt() const noexcept
    {
        return first_attempt;
    }

    std::int64_t count{};
    std::string status{};
    std::optional<std::string> refunds_status{};
    std::optional<std::string> dispute_status{};
    std::int64_t first_attempt{};
};

class SdkEventDimensionsValue
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

class SdkEventFiltersValue
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

class SdkEventMetricsValue
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

class TimeRangeValue
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

class UrlValue
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

class AnalyticsMetadata
{
public:
    void set_current_time_range(TimeRangeValue value)
    {
        current_time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_current_time_range() const noexcept
    {
        return current_time_range;
    }

    TimeRangeValue current_time_range{};
};

class ApiEventFilterValue
{
public:
    void set_dimension(ApiEventDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const ApiEventDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    ApiEventDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class AuthEventFilterValue
{
public:
    void set_dimension(AuthEventDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const AuthEventDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    AuthEventDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class AuthEventMetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<AuthEventsAnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<AuthEventsAnalyticsMetadata, 1>& get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<AuthEventsAnalyticsMetadata, 1> meta_data{};
};

class DisputeFilterValue
{
public:
    void set_dimension(DisputeDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const DisputeDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    DisputeDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class DisputesMetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<DisputesAnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<DisputesAnalyticsMetadata, 1>& get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<DisputesAnalyticsMetadata, 1> meta_data{};
};

class FilterValue
{
public:
    void set_dimension(PaymentDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const PaymentDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    PaymentDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class FrmFilterValue
{
public:
    void set_dimension(FrmDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const FrmDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    FrmDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class GetActivePaymentsMetricRequest
{
public:
    void set_metrics(std::vector<ActivePaymentsMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<ActivePaymentsMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    std::vector<ActivePaymentsMetricsValue> metrics{};
    TimeRangeValue time_range{};
};

class GetApiEventFiltersRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<ApiEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<ApiEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<ApiEventDimensionsValue> group_by_names{};
};

class GetAuthEventFilterRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<AuthEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<AuthEventDimensionsValue> group_by_names{};
};

class GetDisputeFilterRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<DisputeDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<DisputeDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<DisputeDimensionsValue> group_by_names{};
};

class GetFrmFilterRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<FrmDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<FrmDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<FrmDimensionsValue> group_by_names{};
};

class GetInfoResponse
{
public:
    void set_metrics(std::vector<NameDescription> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<NameDescription>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_download_dimensions(std::optional<std::vector<NameDescription>> value)
    {
        download_dimensions = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<NameDescription>>&
    get_download_dimensions() const noexcept
    {
        return download_dimensions;
    }

    void set_dimensions(std::vector<NameDescription> value)
    {
        dimensions = std::move(value);
    }

    [[nodiscard]] const std::vector<NameDescription>& get_dimensions() const noexcept
    {
        return dimensions;
    }

    std::vector<NameDescription> metrics{};
    std::optional<std::vector<NameDescription>> download_dimensions{};
    std::vector<NameDescription> dimensions{};
};

class GetPaymentFiltersRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<PaymentDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<PaymentDimensionsValue> group_by_names{};
};

class GetPaymentIntentFiltersRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<PaymentIntentDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentIntentDimensionsValue>&
    get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<PaymentIntentDimensionsValue> group_by_names{};
};

class GetRefundFilterRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<RefundDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<RefundDimensionsValue> group_by_names{};
};

class GetSdkEventFiltersRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<SdkEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    TimeRangeValue time_range{};
    std::vector<SdkEventDimensionsValue> group_by_names{};
};

class MerchantWebhookUrl
{
public:
    void set_value(UrlValue v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const UrlValue& get_value() const noexcept
    {
        return value;
    }

    UrlValue value{};
};

class PaymentDistributionBody
{
public:
    void set_distribution_for(PaymentDistributionsValue value)
    {
        distribution_for = std::move(value);
    }

    [[nodiscard]] const PaymentDistributionsValue& get_distribution_for() const noexcept
    {
        return distribution_for;
    }

    void set_distribution_cardinality(QueryLimit value)
    {
        distribution_cardinality = std::move(value);
    }

    [[nodiscard]] const QueryLimit& get_distribution_cardinality() const noexcept
    {
        return distribution_cardinality;
    }

    PaymentDistributionsValue distribution_for{};
    QueryLimit distribution_cardinality{};
};

class PaymentIntentFilterValue
{
public:
    void set_dimension(PaymentIntentDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const PaymentIntentDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    PaymentIntentDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class PaymentIntentsMetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<PaymentIntentsAnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<PaymentIntentsAnalyticsMetadata, 1>&
    get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<PaymentIntentsAnalyticsMetadata, 1> meta_data{};
};

class PaymentsMetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<PaymentsAnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<PaymentsAnalyticsMetadata, 1>& get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<PaymentsAnalyticsMetadata, 1> meta_data{};
};

class RefundDistributionBody
{
public:
    void set_distribution_for(RefundDistributionsValue value)
    {
        distribution_for = std::move(value);
    }

    [[nodiscard]] const RefundDistributionsValue& get_distribution_for() const noexcept
    {
        return distribution_for;
    }

    void set_distribution_cardinality(QueryLimit value)
    {
        distribution_cardinality = std::move(value);
    }

    [[nodiscard]] const QueryLimit& get_distribution_cardinality() const noexcept
    {
        return distribution_cardinality;
    }

    RefundDistributionsValue distribution_for{};
    QueryLimit distribution_cardinality{};
};

class RefundFilterValue
{
public:
    void set_dimension(RefundDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const RefundDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    RefundDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class RefundsMetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<RefundsAnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<RefundsAnalyticsMetadata, 1>& get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<RefundsAnalyticsMetadata, 1> meta_data{};
};

class SdkEventFilterValue
{
public:
    void set_dimension(SdkEventDimensionsValue value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const SdkEventDimensionsValue& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_values(std::vector<std::string> value)
    {
        values = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_values() const noexcept
    {
        return values;
    }

    SdkEventDimensionsValue dimension{};
    std::vector<std::string> values{};
};

class TimeSeries
{
public:
    void set_granularity(Granularity value)
    {
        granularity = std::move(value);
    }

    [[nodiscard]] const Granularity& get_granularity() const noexcept
    {
        return granularity;
    }

    Granularity granularity{};
};

class ApiEventFiltersResponse
{
public:
    void set_query_data(std::vector<ApiEventFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<ApiEventFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<ApiEventFilterValue> query_data{};
};

class AuthEventFiltersResponse
{
public:
    void set_query_data(std::vector<AuthEventFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthEventFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<AuthEventFilterValue> query_data{};
};

class DisputeFiltersResponse
{
public:
    void set_query_data(std::vector<DisputeFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<DisputeFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<DisputeFilterValue> query_data{};
};

class FrmFiltersResponse
{
public:
    void set_query_data(std::vector<FrmFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<FrmFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<FrmFilterValue> query_data{};
};

class GetApiEventMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<ApiEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<ApiEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(ApiEventFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const ApiEventFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<ApiEventMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<ApiEventMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<ApiEventDimensionsValue> group_by_names{};
    ApiEventFiltersValue filters{};
    std::vector<ApiEventMetricsValue> metrics{};
    bool delta{};
};

class GetAuthEventMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<AuthEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(AuthEventFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const AuthEventFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<AuthEventMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<AuthEventMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<AuthEventDimensionsValue> group_by_names{};
    AuthEventFiltersValue filters{};
    std::vector<AuthEventMetricsValue> metrics{};
    bool delta{};
};

class GetDisputeMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<DisputeDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<DisputeDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(DisputeFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const DisputeFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<DisputeMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<DisputeMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<DisputeDimensionsValue> group_by_names{};
    DisputeFiltersValue filters{};
    std::vector<DisputeMetricsValue> metrics{};
    bool delta{};
};

class GetFrmMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<FrmDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<FrmDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(FrmFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const FrmFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<FrmMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<FrmMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<FrmDimensionsValue> group_by_names{};
    FrmFiltersValue filters{};
    std::vector<FrmMetricsValue> metrics{};
    bool delta{};
};

class GetPaymentIntentMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<PaymentIntentDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentIntentDimensionsValue>&
    get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(PaymentIntentFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const PaymentIntentFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<PaymentIntentMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentIntentMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<PaymentIntentDimensionsValue> group_by_names{};
    PaymentIntentFiltersValue filters{};
    std::vector<PaymentIntentMetricsValue> metrics{};
    bool delta{};
};

class GetPaymentMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<PaymentDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(PaymentFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const PaymentFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<PaymentMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_distribution(std::optional<PaymentDistributionBody> value)
    {
        distribution = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentDistributionBody>& get_distribution() const noexcept
    {
        return distribution;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<PaymentDimensionsValue> group_by_names{};
    PaymentFiltersValue filters{};
    std::vector<PaymentMetricsValue> metrics{};
    std::optional<PaymentDistributionBody> distribution{};
    bool delta{};
};

class GetRefundMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<RefundDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(RefundFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const RefundFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<RefundMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_distribution(std::optional<RefundDistributionBody> value)
    {
        distribution = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundDistributionBody>& get_distribution() const noexcept
    {
        return distribution;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<RefundDimensionsValue> group_by_names{};
    RefundFiltersValue filters{};
    std::vector<RefundMetricsValue> metrics{};
    std::optional<RefundDistributionBody> distribution{};
    bool delta{};
};

class GetSdkEventMetricRequest
{
public:
    void set_time_series(std::optional<TimeSeries> value)
    {
        time_series = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeSeries>& get_time_series() const noexcept
    {
        return time_series;
    }

    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_group_by_names(std::vector<SdkEventDimensionsValue> value)
    {
        group_by_names = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkEventDimensionsValue>& get_group_by_names() const noexcept
    {
        return group_by_names;
    }

    void set_filters(SdkEventFiltersValue value)
    {
        filters = std::move(value);
    }

    [[nodiscard]] const SdkEventFiltersValue& get_filters() const noexcept
    {
        return filters;
    }

    void set_metrics(std::vector<SdkEventMetricsValue> value)
    {
        metrics = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkEventMetricsValue>& get_metrics() const noexcept
    {
        return metrics;
    }

    void set_delta(bool value)
    {
        delta = std::move(value);
    }

    [[nodiscard]] const bool& get_delta() const noexcept
    {
        return delta;
    }

    std::optional<TimeSeries> time_series{};
    TimeRangeValue time_range{};
    std::vector<SdkEventDimensionsValue> group_by_names{};
    SdkEventFiltersValue filters{};
    std::vector<SdkEventMetricsValue> metrics{};
    bool delta{};
};

class MetricsResponse
{
public:
    void set_query_data(std::vector<std::string> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_query_data() const noexcept
    {
        return query_data;
    }

    void set_meta_data(std::array<AnalyticsMetadata, 1> value)
    {
        meta_data = std::move(value);
    }

    [[nodiscard]] const std::array<AnalyticsMetadata, 1>& get_meta_data() const noexcept
    {
        return meta_data;
    }

    std::vector<std::string> query_data{};
    std::array<AnalyticsMetadata, 1> meta_data{};
};

class PaymentFiltersResponse
{
public:
    void set_query_data(std::vector<FilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<FilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<FilterValue> query_data{};
};

class PaymentIntentFiltersResponse
{
public:
    void set_query_data(std::vector<PaymentIntentFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<PaymentIntentFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<PaymentIntentFilterValue> query_data{};
};

class RefundFiltersResponse
{
public:
    void set_query_data(std::vector<RefundFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<RefundFilterValue> query_data{};
};

class ReportRequest
{
public:
    void set_time_range(TimeRangeValue value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const TimeRangeValue& get_time_range() const noexcept
    {
        return time_range;
    }

    void set_emails(std::optional<std::vector<EmailValue>> value)
    {
        emails = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<EmailValue>>& get_emails() const noexcept
    {
        return emails;
    }

    void set_return_url(std::optional<MerchantWebhookUrl> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantWebhookUrl>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_report_type(ReportType value)
    {
        report_type = std::move(value);
    }

    [[nodiscard]] const ReportType& get_report_type() const noexcept
    {
        return report_type;
    }

    void set_columns(std::optional<ReportColumns> value)
    {
        columns = std::move(value);
    }

    [[nodiscard]] const std::optional<ReportColumns>& get_columns() const noexcept
    {
        return columns;
    }

    TimeRangeValue time_range{};
    std::optional<std::vector<EmailValue>> emails{};
    std::optional<MerchantWebhookUrl> return_url{};
    ReportType report_type{};
    std::optional<ReportColumns> columns{};
};

class SdkEventFiltersResponse
{
public:
    void set_query_data(std::vector<SdkEventFilterValue> value)
    {
        query_data = std::move(value);
    }

    [[nodiscard]] const std::vector<SdkEventFilterValue>& get_query_data() const noexcept
    {
        return query_data;
    }

    std::vector<SdkEventFilterValue> query_data{};
};

class AnalyticsRequest
{
public:
    void set_payment_intent(std::optional<GetPaymentIntentMetricRequest> value)
    {
        payment_intent = std::move(value);
    }

    [[nodiscard]] const std::optional<GetPaymentIntentMetricRequest>&
    get_payment_intent() const noexcept
    {
        return payment_intent;
    }

    void set_payment_attempt(std::optional<GetPaymentMetricRequest> value)
    {
        payment_attempt = std::move(value);
    }

    [[nodiscard]] const std::optional<GetPaymentMetricRequest>& get_payment_attempt() const noexcept
    {
        return payment_attempt;
    }

    void set_refund(std::optional<GetRefundMetricRequest> value)
    {
        refund = std::move(value);
    }

    [[nodiscard]] const std::optional<GetRefundMetricRequest>& get_refund() const noexcept
    {
        return refund;
    }

    void set_dispute(std::optional<GetDisputeMetricRequest> value)
    {
        dispute = std::move(value);
    }

    [[nodiscard]] const std::optional<GetDisputeMetricRequest>& get_dispute() const noexcept
    {
        return dispute;
    }

    std::optional<GetPaymentIntentMetricRequest> payment_intent{};
    std::optional<GetPaymentMetricRequest> payment_attempt{};
    std::optional<GetRefundMetricRequest> refund{};
    std::optional<GetDisputeMetricRequest> dispute{};
};

class GenerateReportRequest
{
public:
    void set_request(ReportRequest value)
    {
        request = std::move(value);
    }

    [[nodiscard]] const ReportRequest& get_request() const noexcept
    {
        return request;
    }

    void set_merchant_id(std::optional<MerchantIdValue> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_auth(AuthInfoValue value)
    {
        auth = std::move(value);
    }

    [[nodiscard]] const AuthInfoValue& get_auth() const noexcept
    {
        return auth;
    }

    void set_email(EmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const EmailValue& get_email() const noexcept
    {
        return email;
    }

    void set_payment_response_hash_key(std::optional<std::string> value)
    {
        payment_response_hash_key = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_response_hash_key() const noexcept
    {
        return payment_response_hash_key;
    }

    ReportRequest request{};
    std::optional<MerchantIdValue> merchant_id{};
    AuthInfoValue auth{};
    EmailValue email{};
    std::optional<std::string> payment_response_hash_key{};
};
} // namespace app::dto::analytics

template<>
struct serde::Serializable<app::dto::analytics::NameDescription>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::analytics::NameDescription::get_name,
                &app::dto::analytics::NameDescription::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "desc",
                &app::dto::analytics::NameDescription::get_desc,
                &app::dto::analytics::NameDescription::set_desc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "name_description";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetInfoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetInfoResponse::get_metrics,
                &app::dto::analytics::GetInfoResponse::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "downloadDimensions",
                &app::dto::analytics::GetInfoResponse::get_download_dimensions,
                &app::dto::analytics::GetInfoResponse::set_download_dimensions,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dimensions",
                &app::dto::analytics::GetInfoResponse::get_dimensions,
                &app::dto::analytics::GetInfoResponse::set_dimensions,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_info_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::TimeSeries>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "granularity",
            &app::dto::analytics::TimeSeries::get_granularity,
            &app::dto::analytics::TimeSeries::set_granularity,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "time_series";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentDistributionBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "distributionFor",
                &app::dto::analytics::PaymentDistributionBody::get_distribution_for,
                &app::dto::analytics::PaymentDistributionBody::set_distribution_for,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "distributionCardinality",
                &app::dto::analytics::PaymentDistributionBody::get_distribution_cardinality,
                &app::dto::analytics::PaymentDistributionBody::set_distribution_cardinality,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_distribution_body";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::RefundDistributionBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "distributionFor",
                &app::dto::analytics::RefundDistributionBody::get_distribution_for,
                &app::dto::analytics::RefundDistributionBody::set_distribution_for,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "distributionCardinality",
                &app::dto::analytics::RefundDistributionBody::get_distribution_cardinality,
                &app::dto::analytics::RefundDistributionBody::set_distribution_cardinality,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_distribution_body";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetPaymentMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetPaymentMetricRequest::get_time_series,
                &app::dto::analytics::GetPaymentMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetPaymentMetricRequest::get_time_range,
                &app::dto::analytics::GetPaymentMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetPaymentMetricRequest::get_group_by_names,
                &app::dto::analytics::GetPaymentMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetPaymentMetricRequest::get_filters,
                &app::dto::analytics::GetPaymentMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetPaymentMetricRequest::get_metrics,
                &app::dto::analytics::GetPaymentMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "distribution",
                &app::dto::analytics::GetPaymentMetricRequest::get_distribution,
                &app::dto::analytics::GetPaymentMetricRequest::set_distribution,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetPaymentMetricRequest::get_delta,
                &app::dto::analytics::GetPaymentMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_payment_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetPaymentIntentMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_time_series,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_time_range,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_group_by_names,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_filters,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_metrics,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetPaymentIntentMetricRequest::get_delta,
                &app::dto::analytics::GetPaymentIntentMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_payment_intent_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetRefundMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetRefundMetricRequest::get_time_series,
                &app::dto::analytics::GetRefundMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetRefundMetricRequest::get_time_range,
                &app::dto::analytics::GetRefundMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetRefundMetricRequest::get_group_by_names,
                &app::dto::analytics::GetRefundMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetRefundMetricRequest::get_filters,
                &app::dto::analytics::GetRefundMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetRefundMetricRequest::get_metrics,
                &app::dto::analytics::GetRefundMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "distribution",
                &app::dto::analytics::GetRefundMetricRequest::get_distribution,
                &app::dto::analytics::GetRefundMetricRequest::set_distribution,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetRefundMetricRequest::get_delta,
                &app::dto::analytics::GetRefundMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_refund_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetFrmMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetFrmMetricRequest::get_time_series,
                &app::dto::analytics::GetFrmMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetFrmMetricRequest::get_time_range,
                &app::dto::analytics::GetFrmMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetFrmMetricRequest::get_group_by_names,
                &app::dto::analytics::GetFrmMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetFrmMetricRequest::get_filters,
                &app::dto::analytics::GetFrmMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetFrmMetricRequest::get_metrics,
                &app::dto::analytics::GetFrmMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetFrmMetricRequest::get_delta,
                &app::dto::analytics::GetFrmMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_frm_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetSdkEventMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetSdkEventMetricRequest::get_time_series,
                &app::dto::analytics::GetSdkEventMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetSdkEventMetricRequest::get_time_range,
                &app::dto::analytics::GetSdkEventMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetSdkEventMetricRequest::get_group_by_names,
                &app::dto::analytics::GetSdkEventMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetSdkEventMetricRequest::get_filters,
                &app::dto::analytics::GetSdkEventMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetSdkEventMetricRequest::get_metrics,
                &app::dto::analytics::GetSdkEventMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetSdkEventMetricRequest::get_delta,
                &app::dto::analytics::GetSdkEventMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_sdk_event_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetAuthEventMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetAuthEventMetricRequest::get_time_series,
                &app::dto::analytics::GetAuthEventMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetAuthEventMetricRequest::get_time_range,
                &app::dto::analytics::GetAuthEventMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetAuthEventMetricRequest::get_group_by_names,
                &app::dto::analytics::GetAuthEventMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetAuthEventMetricRequest::get_filters,
                &app::dto::analytics::GetAuthEventMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetAuthEventMetricRequest::get_metrics,
                &app::dto::analytics::GetAuthEventMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetAuthEventMetricRequest::get_delta,
                &app::dto::analytics::GetAuthEventMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_auth_event_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetDisputeMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetDisputeMetricRequest::get_time_series,
                &app::dto::analytics::GetDisputeMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetDisputeMetricRequest::get_time_range,
                &app::dto::analytics::GetDisputeMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetDisputeMetricRequest::get_group_by_names,
                &app::dto::analytics::GetDisputeMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetDisputeMetricRequest::get_filters,
                &app::dto::analytics::GetDisputeMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetDisputeMetricRequest::get_metrics,
                &app::dto::analytics::GetDisputeMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetDisputeMetricRequest::get_delta,
                &app::dto::analytics::GetDisputeMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_dispute_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetActivePaymentsMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetActivePaymentsMetricRequest::get_metrics,
                &app::dto::analytics::GetActivePaymentsMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetActivePaymentsMetricRequest::get_time_range,
                &app::dto::analytics::GetActivePaymentsMetricRequest::set_time_range,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_active_payments_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AnalyticsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "paymentIntent",
                &app::dto::analytics::AnalyticsRequest::get_payment_intent,
                &app::dto::analytics::AnalyticsRequest::set_payment_intent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentAttempt",
                &app::dto::analytics::AnalyticsRequest::get_payment_attempt,
                &app::dto::analytics::AnalyticsRequest::set_payment_attempt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund",
                &app::dto::analytics::AnalyticsRequest::get_refund,
                &app::dto::analytics::AnalyticsRequest::set_refund,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dispute",
                &app::dto::analytics::AnalyticsRequest::get_dispute,
                &app::dto::analytics::AnalyticsRequest::set_dispute,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "current_time_range",
            &app::dto::analytics::AnalyticsMetadata::get_current_time_range,
            &app::dto::analytics::AnalyticsMetadata::set_current_time_range,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentsAnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_payment_processed_amount",
                &app::dto::analytics::PaymentsAnalyticsMetadata::get_total_payment_processed_amount,
                &app::dto::analytics::PaymentsAnalyticsMetadata::set_total_payment_processed_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_in_usd",
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    get_total_payment_processed_amount_in_usd,
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    set_total_payment_processed_amount_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_without_smart_retries",
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    get_total_payment_processed_amount_without_smart_retries,
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    set_total_payment_processed_amount_without_smart_retries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_without_smart_retries_usd",
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    get_total_payment_processed_amount_without_smart_retries_usd,
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    set_total_payment_processed_amount_without_smart_retries_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_count",
                &app::dto::analytics::PaymentsAnalyticsMetadata::get_total_payment_processed_count,
                &app::dto::analytics::PaymentsAnalyticsMetadata::set_total_payment_processed_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_count_without_smart_retries",
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    get_total_payment_processed_count_without_smart_retries,
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    set_total_payment_processed_count_without_smart_retries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_failure_reasons_count",
                &app::dto::analytics::PaymentsAnalyticsMetadata::get_total_failure_reasons_count,
                &app::dto::analytics::PaymentsAnalyticsMetadata::set_total_failure_reasons_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_failure_reasons_count_without_smart_retries",
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    get_total_failure_reasons_count_without_smart_retries,
                &app::dto::analytics::PaymentsAnalyticsMetadata::
                    set_total_failure_reasons_count_without_smart_retries,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentIntentsAnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_success_rate",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::get_total_success_rate,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::set_total_success_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_success_rate_without_smart_retries",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_success_rate_without_smart_retries,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_success_rate_without_smart_retries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_smart_retried_amount",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_smart_retried_amount,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_smart_retried_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_smart_retried_amount_without_smart_retries",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_smart_retried_amount_without_smart_retries,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_smart_retried_amount_without_smart_retries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_amount,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_without_smart_retries",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_amount_without_smart_retries,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_amount_without_smart_retries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_smart_retried_amount_in_usd",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_smart_retried_amount_in_usd,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_smart_retried_amount_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_smart_retried_amount_without_smart_retries_in_usd",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_smart_retried_amount_without_smart_retries_in_usd,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_smart_retried_amount_without_smart_retries_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_in_usd",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_amount_in_usd,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_amount_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_amount_without_smart_retries_in_usd",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_amount_without_smart_retries_in_usd,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_amount_without_smart_retries_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_count",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_count,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_payment_processed_count_without_smart_retries",
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    get_total_payment_processed_count_without_smart_retries,
                &app::dto::analytics::PaymentIntentsAnalyticsMetadata::
                    set_total_payment_processed_count_without_smart_retries,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_intents_analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::RefundsAnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_refund_success_rate",
                &app::dto::analytics::RefundsAnalyticsMetadata::get_total_refund_success_rate,
                &app::dto::analytics::RefundsAnalyticsMetadata::set_total_refund_success_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_refund_processed_amount",
                &app::dto::analytics::RefundsAnalyticsMetadata::get_total_refund_processed_amount,
                &app::dto::analytics::RefundsAnalyticsMetadata::set_total_refund_processed_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_refund_processed_amount_in_usd",
                &app::dto::analytics::RefundsAnalyticsMetadata::
                    get_total_refund_processed_amount_in_usd,
                &app::dto::analytics::RefundsAnalyticsMetadata::
                    set_total_refund_processed_amount_in_usd,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_refund_processed_count",
                &app::dto::analytics::RefundsAnalyticsMetadata::get_total_refund_processed_count,
                &app::dto::analytics::RefundsAnalyticsMetadata::set_total_refund_processed_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_refund_reason_count",
                &app::dto::analytics::RefundsAnalyticsMetadata::get_total_refund_reason_count,
                &app::dto::analytics::RefundsAnalyticsMetadata::set_total_refund_reason_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_refund_error_message_count",
                &app::dto::analytics::RefundsAnalyticsMetadata::
                    get_total_refund_error_message_count,
                &app::dto::analytics::RefundsAnalyticsMetadata::
                    set_total_refund_error_message_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::DisputesAnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_disputed_amount",
                &app::dto::analytics::DisputesAnalyticsMetadata::get_total_disputed_amount,
                &app::dto::analytics::DisputesAnalyticsMetadata::set_total_disputed_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_dispute_lost_amount",
                &app::dto::analytics::DisputesAnalyticsMetadata::get_total_dispute_lost_amount,
                &app::dto::analytics::DisputesAnalyticsMetadata::set_total_dispute_lost_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "disputes_analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AuthEventsAnalyticsMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "total_error_message_count",
            &app::dto::analytics::AuthEventsAnalyticsMetadata::get_total_error_message_count,
            &app::dto::analytics::AuthEventsAnalyticsMetadata::set_total_error_message_count,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "auth_events_analytics_metadata";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::FilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::FilterValue::get_dimension,
                &app::dto::analytics::FilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::FilterValue::get_values,
                &app::dto::analytics::FilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::PaymentFiltersResponse::get_query_data,
            &app::dto::analytics::PaymentFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentIntentFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::PaymentIntentFilterValue::get_dimension,
                &app::dto::analytics::PaymentIntentFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::PaymentIntentFilterValue::get_values,
                &app::dto::analytics::PaymentIntentFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_intent_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentIntentFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::PaymentIntentFiltersResponse::get_query_data,
            &app::dto::analytics::PaymentIntentFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "payment_intent_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::RefundFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::RefundFilterValue::get_dimension,
                &app::dto::analytics::RefundFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::RefundFilterValue::get_values,
                &app::dto::analytics::RefundFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::RefundFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::RefundFiltersResponse::get_query_data,
            &app::dto::analytics::RefundFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "refund_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::FrmFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::FrmFilterValue::get_dimension,
                &app::dto::analytics::FrmFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::FrmFilterValue::get_values,
                &app::dto::analytics::FrmFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::FrmFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::FrmFiltersResponse::get_query_data,
            &app::dto::analytics::FrmFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "frm_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::SdkEventFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::SdkEventFilterValue::get_dimension,
                &app::dto::analytics::SdkEventFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::SdkEventFilterValue::get_values,
                &app::dto::analytics::SdkEventFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_event_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::SdkEventFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::SdkEventFiltersResponse::get_query_data,
            &app::dto::analytics::SdkEventFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "sdk_event_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::DisputeFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::DisputeFilterValue::get_dimension,
                &app::dto::analytics::DisputeFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::DisputeFilterValue::get_values,
                &app::dto::analytics::DisputeFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dispute_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::DisputeFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::DisputeFiltersResponse::get_query_data,
            &app::dto::analytics::DisputeFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "dispute_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AuthEventFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::AuthEventFilterValue::get_dimension,
                &app::dto::analytics::AuthEventFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::AuthEventFilterValue::get_values,
                &app::dto::analytics::AuthEventFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "auth_event_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AuthEventFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::AuthEventFiltersResponse::get_query_data,
            &app::dto::analytics::AuthEventFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "auth_event_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::ApiEventFilterValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::analytics::ApiEventFilterValue::get_dimension,
                &app::dto::analytics::ApiEventFilterValue::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "values",
                &app::dto::analytics::ApiEventFilterValue::get_values,
                &app::dto::analytics::ApiEventFilterValue::set_values,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "api_event_filter_value";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::ApiEventFiltersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "queryData",
            &app::dto::analytics::ApiEventFiltersResponse::get_query_data,
            &app::dto::analytics::ApiEventFiltersResponse::set_query_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "api_event_filters_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetPaymentFiltersRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetPaymentFiltersRequest::get_time_range,
                &app::dto::analytics::GetPaymentFiltersRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetPaymentFiltersRequest::get_group_by_names,
                &app::dto::analytics::GetPaymentFiltersRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_payment_filters_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetPaymentIntentFiltersRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetPaymentIntentFiltersRequest::get_time_range,
                &app::dto::analytics::GetPaymentIntentFiltersRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetPaymentIntentFiltersRequest::get_group_by_names,
                &app::dto::analytics::GetPaymentIntentFiltersRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_payment_intent_filters_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetRefundFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetRefundFilterRequest::get_time_range,
                &app::dto::analytics::GetRefundFilterRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetRefundFilterRequest::get_group_by_names,
                &app::dto::analytics::GetRefundFilterRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_refund_filter_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetFrmFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetFrmFilterRequest::get_time_range,
                &app::dto::analytics::GetFrmFilterRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetFrmFilterRequest::get_group_by_names,
                &app::dto::analytics::GetFrmFilterRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_frm_filter_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetSdkEventFiltersRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetSdkEventFiltersRequest::get_time_range,
                &app::dto::analytics::GetSdkEventFiltersRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetSdkEventFiltersRequest::get_group_by_names,
                &app::dto::analytics::GetSdkEventFiltersRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_sdk_event_filters_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetDisputeFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetDisputeFilterRequest::get_time_range,
                &app::dto::analytics::GetDisputeFilterRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetDisputeFilterRequest::get_group_by_names,
                &app::dto::analytics::GetDisputeFilterRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_dispute_filter_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetAuthEventFilterRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetAuthEventFilterRequest::get_time_range,
                &app::dto::analytics::GetAuthEventFilterRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetAuthEventFilterRequest::get_group_by_names,
                &app::dto::analytics::GetAuthEventFilterRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_auth_event_filter_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetApiEventFiltersRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetApiEventFiltersRequest::get_time_range,
                &app::dto::analytics::GetApiEventFiltersRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetApiEventFiltersRequest::get_group_by_names,
                &app::dto::analytics::GetApiEventFiltersRequest::set_group_by_names,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_api_event_filters_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::MetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::MetricsResponse::get_query_data,
                &app::dto::analytics::MetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::MetricsResponse::get_meta_data,
                &app::dto::analytics::MetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentsMetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::PaymentsMetricsResponse::get_query_data,
                &app::dto::analytics::PaymentsMetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::PaymentsMetricsResponse::get_meta_data,
                &app::dto::analytics::PaymentsMetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payments_metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::PaymentIntentsMetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::PaymentIntentsMetricsResponse::get_query_data,
                &app::dto::analytics::PaymentIntentsMetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::PaymentIntentsMetricsResponse::get_meta_data,
                &app::dto::analytics::PaymentIntentsMetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_intents_metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::RefundsMetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::RefundsMetricsResponse::get_query_data,
                &app::dto::analytics::RefundsMetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::RefundsMetricsResponse::get_meta_data,
                &app::dto::analytics::RefundsMetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refunds_metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::DisputesMetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::DisputesMetricsResponse::get_query_data,
                &app::dto::analytics::DisputesMetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::DisputesMetricsResponse::get_meta_data,
                &app::dto::analytics::DisputesMetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "disputes_metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::AuthEventMetricsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "queryData",
                &app::dto::analytics::AuthEventMetricsResponse::get_query_data,
                &app::dto::analytics::AuthEventMetricsResponse::set_query_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metaData",
                &app::dto::analytics::AuthEventMetricsResponse::get_meta_data,
                &app::dto::analytics::AuthEventMetricsResponse::set_meta_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "auth_event_metrics_response";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::MerchantWebhookUrl>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::analytics::MerchantWebhookUrl::get_value,
            &app::dto::analytics::MerchantWebhookUrl::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_webhook_url";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::ReportRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::ReportRequest::get_time_range,
                &app::dto::analytics::ReportRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "emails",
                &app::dto::analytics::ReportRequest::get_emails,
                &app::dto::analytics::ReportRequest::set_emails,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "returnUrl",
                &app::dto::analytics::ReportRequest::get_return_url,
                &app::dto::analytics::ReportRequest::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reportType",
                &app::dto::analytics::ReportRequest::get_report_type,
                &app::dto::analytics::ReportRequest::set_report_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "columns",
                &app::dto::analytics::ReportRequest::get_columns,
                &app::dto::analytics::ReportRequest::set_columns,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "report_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GenerateReportRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "request",
                &app::dto::analytics::GenerateReportRequest::get_request,
                &app::dto::analytics::GenerateReportRequest::set_request,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchantId",
                &app::dto::analytics::GenerateReportRequest::get_merchant_id,
                &app::dto::analytics::GenerateReportRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth",
                &app::dto::analytics::GenerateReportRequest::get_auth,
                &app::dto::analytics::GenerateReportRequest::set_auth,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::dto::analytics::GenerateReportRequest::get_email,
                &app::dto::analytics::GenerateReportRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paymentResponseHashKey",
                &app::dto::analytics::GenerateReportRequest::get_payment_response_hash_key,
                &app::dto::analytics::GenerateReportRequest::set_payment_response_hash_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "generate_report_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::GetApiEventMetricRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "timeSeries",
                &app::dto::analytics::GetApiEventMetricRequest::get_time_series,
                &app::dto::analytics::GetApiEventMetricRequest::set_time_series,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeRange",
                &app::dto::analytics::GetApiEventMetricRequest::get_time_range,
                &app::dto::analytics::GetApiEventMetricRequest::set_time_range,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "groupByNames",
                &app::dto::analytics::GetApiEventMetricRequest::get_group_by_names,
                &app::dto::analytics::GetApiEventMetricRequest::set_group_by_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "filters",
                &app::dto::analytics::GetApiEventMetricRequest::get_filters,
                &app::dto::analytics::GetApiEventMetricRequest::set_filters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metrics",
                &app::dto::analytics::GetApiEventMetricRequest::get_metrics,
                &app::dto::analytics::GetApiEventMetricRequest::set_metrics,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delta",
                &app::dto::analytics::GetApiEventMetricRequest::get_delta,
                &app::dto::analytics::GetApiEventMetricRequest::set_delta,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_api_event_metric_request";
    }
};

template<>
struct serde::Serializable<app::dto::analytics::SankeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::dto::analytics::SankeyResponse::get_count,
                &app::dto::analytics::SankeyResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::dto::analytics::SankeyResponse::get_status,
                &app::dto::analytics::SankeyResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refunds_status",
                &app::dto::analytics::SankeyResponse::get_refunds_status,
                &app::dto::analytics::SankeyResponse::set_refunds_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dispute_status",
                &app::dto::analytics::SankeyResponse::get_dispute_status,
                &app::dto::analytics::SankeyResponse::set_dispute_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_attempt",
                &app::dto::analytics::SankeyResponse::get_first_attempt,
                &app::dto::analytics::SankeyResponse::set_first_attempt,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sankey_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::analytics::tests {
using namespace boost::ut;
suite<"app_dto_analytics"> app_dto_analytics_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::analytics::NameDescription>::fields())> > 0
        );
    };
};
} // namespace app::dto::analytics::tests
#endif
