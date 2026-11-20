module;
export module app_model:disputes;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::disputes {

class AdditionalDetailsValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CurrencyValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

/**
 * @brief Upstream `DisputeFetchQueryData` (types/api/disputes.rs).
 * @note Upstream parses the timestamps as `YYYY-MM-DDTHH:MM:SS`; the handler
 * validates them with `parse_fetch_window` before the connector step.
 */
class DisputeFetchQueryData
{
public:
    void set_fetch_from(std::string value) { fetch_from = std::move(value); }
    [[nodiscard]] const std::string& get_fetch_from() const noexcept { return fetch_from; }
    void set_fetch_till(std::string value) { fetch_till = std::move(value); }
    [[nodiscard]] const std::string& get_fetch_till() const noexcept { return fetch_till; }
    std::string fetch_from{};
    std::string fetch_till{};
};

/**
 * @brief Upstream `common_utils::types::TimeRange` used by dispute aggregates
 * and flattened into `DisputeListGetConstraints` (api_models/disputes.rs:174-176).
 */
class TimeRange
{
public:
    void set_start_time(std::optional<std::string> value) { start_time = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_start_time() const noexcept { return start_time; }
    void set_end_time(std::optional<std::string> value) { end_time = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_end_time() const noexcept { return end_time; }
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
};

class DisputeRetrieveBody
{
public:
    void set_force_sync(std::optional<bool> value) { force_sync = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept { return force_sync; }
    std::optional<bool> force_sync{};
};

/** @brief Upstream `types::api::disputes::DisputeId` path payload. */
class DisputeId
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    std::string dispute_id{};
};

class DisputeRetrieveRequest
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_force_sync(std::optional<bool> value) { force_sync = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_force_sync() const noexcept { return force_sync; }
    std::string dispute_id{};
    std::optional<bool> force_sync{};
};

class DisputeStageValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DisputeStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DisputesAggregateResponse
{
public:
    void set_status_with_count(std::unordered_map<std::string, std::int64_t> value) { status_with_count = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::int64_t>& get_status_with_count() const noexcept { return status_with_count; }
    std::unordered_map<std::string, std::int64_t> status_with_count{};
};

enum class EvidenceType
{
    CancellationPolicy,
    CustomerCommunication,
    CustomerSignature,
    Receipt,
    RefundPolicy,
    ServiceDocumentation,
    ShippingDocumentation,
    InvoiceShowingDistinctTransactions,
    RecurringTransactionAgreement,
    UncategorizedFile,
};

/**
 * @brief Upstream `api_models::files::FileMetadataResponse` wire shape
 * (api_models/files.rs:10-16): file_id, file_name, file_size, file_type,
 * available. Formerly a single opaque string, which did not match upstream
 * `DisputeEvidenceBlock.file_metadata_response`.
 */
class FileMetadataResponseValue
{
public:
    void set_file_id(std::string value) { file_id = std::move(value); }
    [[nodiscard]] const std::string& get_file_id() const noexcept { return file_id; }
    void set_file_name(std::optional<std::string> value) { file_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept { return file_name; }
    void set_file_size(std::int32_t value) { file_size = std::move(value); }
    [[nodiscard]] const std::int32_t& get_file_size() const noexcept { return file_size; }
    void set_file_type(std::string value) { file_type = std::move(value); }
    [[nodiscard]] const std::string& get_file_type() const noexcept { return file_type; }
    void set_available(bool value) { available = std::move(value); }
    [[nodiscard]] const bool& get_available() const noexcept { return available; }
    std::string file_id{};
    std::optional<std::string> file_name{};
    std::int32_t file_size{};
    std::string file_type{};
    bool available{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantConnectorInfoValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PageOffsetValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PageSizeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ProfileIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class StringMinorUnitValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SubmitEvidenceRequest
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_access_activity_log(std::optional<std::string> value) { access_activity_log = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_access_activity_log() const noexcept { return access_activity_log; }
    void set_billing_address(std::optional<std::string> value) { billing_address = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_billing_address() const noexcept { return billing_address; }
    void set_cancellation_policy(std::optional<std::string> value) { cancellation_policy = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cancellation_policy() const noexcept { return cancellation_policy; }
    void set_cancellation_policy_disclosure(std::optional<std::string> value) { cancellation_policy_disclosure = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cancellation_policy_disclosure() const noexcept { return cancellation_policy_disclosure; }
    void set_cancellation_rebuttal(std::optional<std::string> value) { cancellation_rebuttal = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cancellation_rebuttal() const noexcept { return cancellation_rebuttal; }
    void set_customer_communication(std::optional<std::string> value) { customer_communication = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_communication() const noexcept { return customer_communication; }
    void set_customer_email_address(std::optional<std::string> value) { customer_email_address = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_email_address() const noexcept { return customer_email_address; }
    void set_customer_name(std::optional<std::string> value) { customer_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_name() const noexcept { return customer_name; }
    void set_customer_purchase_ip(std::optional<std::string> value) { customer_purchase_ip = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_purchase_ip() const noexcept { return customer_purchase_ip; }
    void set_customer_signature(std::optional<std::string> value) { customer_signature = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_signature() const noexcept { return customer_signature; }
    void set_product_description(std::optional<std::string> value) { product_description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_product_description() const noexcept { return product_description; }
    void set_receipt(std::optional<std::string> value) { receipt = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_receipt() const noexcept { return receipt; }
    void set_refund_policy(std::optional<std::string> value) { refund_policy = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_refund_policy() const noexcept { return refund_policy; }
    void set_refund_policy_disclosure(std::optional<std::string> value) { refund_policy_disclosure = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_refund_policy_disclosure() const noexcept { return refund_policy_disclosure; }
    void set_refund_refusal_explanation(std::optional<std::string> value) { refund_refusal_explanation = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_refund_refusal_explanation() const noexcept { return refund_refusal_explanation; }
    void set_service_date(std::optional<std::string> value) { service_date = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_service_date() const noexcept { return service_date; }
    void set_service_documentation(std::optional<std::string> value) { service_documentation = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_service_documentation() const noexcept { return service_documentation; }
    void set_shipping_address(std::optional<std::string> value) { shipping_address = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipping_address() const noexcept { return shipping_address; }
    void set_shipping_carrier(std::optional<std::string> value) { shipping_carrier = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipping_carrier() const noexcept { return shipping_carrier; }
    void set_shipping_date(std::optional<std::string> value) { shipping_date = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipping_date() const noexcept { return shipping_date; }
    void set_shipping_documentation(std::optional<std::string> value) { shipping_documentation = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipping_documentation() const noexcept { return shipping_documentation; }
    void set_shipping_tracking_number(std::optional<std::string> value) { shipping_tracking_number = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_shipping_tracking_number() const noexcept { return shipping_tracking_number; }
    void set_invoice_showing_distinct_transactions(std::optional<std::string> value) { invoice_showing_distinct_transactions = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_invoice_showing_distinct_transactions() const noexcept { return invoice_showing_distinct_transactions; }
    void set_recurring_transaction_agreement(std::optional<std::string> value) { recurring_transaction_agreement = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_recurring_transaction_agreement() const noexcept { return recurring_transaction_agreement; }
    void set_uncategorized_file(std::optional<std::string> value) { uncategorized_file = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_uncategorized_file() const noexcept { return uncategorized_file; }
    void set_uncategorized_text(std::optional<std::string> value) { uncategorized_text = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_uncategorized_text() const noexcept { return uncategorized_text; }
    std::string dispute_id{};
    std::optional<std::string> access_activity_log{};
    std::optional<std::string> billing_address{};
    std::optional<std::string> cancellation_policy{};
    std::optional<std::string> cancellation_policy_disclosure{};
    std::optional<std::string> cancellation_rebuttal{};
    std::optional<std::string> customer_communication{};
    std::optional<std::string> customer_email_address{};
    std::optional<std::string> customer_name{};
    std::optional<std::string> customer_purchase_ip{};
    std::optional<std::string> customer_signature{};
    std::optional<std::string> product_description{};
    std::optional<std::string> receipt{};
    std::optional<std::string> refund_policy{};
    std::optional<std::string> refund_policy_disclosure{};
    std::optional<std::string> refund_refusal_explanation{};
    std::optional<std::string> service_date{};
    std::optional<std::string> service_documentation{};
    std::optional<std::string> shipping_address{};
    std::optional<std::string> shipping_carrier{};
    std::optional<std::string> shipping_date{};
    std::optional<std::string> shipping_documentation{};
    std::optional<std::string> shipping_tracking_number{};
    std::optional<std::string> invoice_showing_distinct_transactions{};
    std::optional<std::string> recurring_transaction_agreement{};
    std::optional<std::string> uncategorized_file{};
    std::optional<std::string> uncategorized_text{};
};

class TimeRangeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DeleteEvidenceRequest
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_evidence_type(EvidenceType value) { evidence_type = std::move(value); }
    [[nodiscard]] const EvidenceType& get_evidence_type() const noexcept { return evidence_type; }
    std::string dispute_id{};
    EvidenceType evidence_type{};
};

class DisputeEvidenceBlock
{
public:
    void set_evidence_type(EvidenceType value) { evidence_type = std::move(value); }
    [[nodiscard]] const EvidenceType& get_evidence_type() const noexcept { return evidence_type; }
    void set_file_metadata_response(FileMetadataResponseValue value) { file_metadata_response = std::move(value); }
    [[nodiscard]] const FileMetadataResponseValue& get_file_metadata_response() const noexcept { return file_metadata_response; }
    EvidenceType evidence_type{};
    FileMetadataResponseValue file_metadata_response{};
};

class DisputeListFilters
{
public:
    void set_connector(std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value) { connector = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>& get_connector() const noexcept { return connector; }
    void set_currency(std::vector<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_dispute_status(std::vector<DisputeStatusValue> value) { dispute_status = std::move(value); }
    [[nodiscard]] const std::vector<DisputeStatusValue>& get_dispute_status() const noexcept { return dispute_status; }
    void set_dispute_stage(std::vector<DisputeStageValue> value) { dispute_stage = std::move(value); }
    [[nodiscard]] const std::vector<DisputeStageValue>& get_dispute_stage() const noexcept { return dispute_stage; }
    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<DisputeStatusValue> dispute_status{};
    std::vector<DisputeStageValue> dispute_stage{};
};

class DisputeListGetConstraints
{
public:
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }
    void set_payment_id(std::optional<PaymentIdValue> value) { payment_id = std::move(value); }
    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept { return payment_id; }
    void set_limit(PageSizeValue value) { limit = std::move(value); }
    [[nodiscard]] const PageSizeValue& get_limit() const noexcept { return limit; }
    void set_offset(PageOffsetValue value) { offset = std::move(value); }
    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept { return offset; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_dispute_status(std::optional<std::vector<DisputeStatusValue>> value) { dispute_status = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStatusValue>>& get_dispute_status() const noexcept { return dispute_status; }
    void set_dispute_stage(std::optional<std::vector<DisputeStageValue>> value) { dispute_stage = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStageValue>>& get_dispute_stage() const noexcept { return dispute_stage; }
    void set_reason(std::optional<std::string> value) { reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept { return reason; }
    void set_connector(std::optional<std::vector<std::string>> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connector() const noexcept { return connector; }
    void set_currency(std::optional<std::vector<CurrencyValue>> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept { return currency; }
    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_time_range(std::optional<TimeRangeValue> value) { time_range = std::move(value); }
    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept { return time_range; }
    std::optional<std::string> dispute_id{};
    std::optional<PaymentIdValue> payment_id{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<std::vector<DisputeStatusValue>> dispute_status{};
    std::optional<std::vector<DisputeStageValue>> dispute_stage{};
    std::optional<std::string> reason{};
    std::optional<std::vector<std::string>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<TimeRangeValue> time_range{};
};

class DisputeResponse
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_payment_id(PaymentIdValue value) { payment_id = std::move(value); }
    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept { return payment_id; }
    void set_attempt_id(std::string value) { attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return attempt_id; }
    void set_amount(StringMinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const StringMinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_dispute_stage(DisputeStageValue value) { dispute_stage = std::move(value); }
    [[nodiscard]] const DisputeStageValue& get_dispute_stage() const noexcept { return dispute_stage; }
    void set_dispute_status(DisputeStatusValue value) { dispute_status = std::move(value); }
    [[nodiscard]] const DisputeStatusValue& get_dispute_status() const noexcept { return dispute_status; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_connector_status(std::string value) { connector_status = std::move(value); }
    [[nodiscard]] const std::string& get_connector_status() const noexcept { return connector_status; }
    void set_connector_dispute_id(std::string value) { connector_dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_dispute_id() const noexcept { return connector_dispute_id; }
    void set_connector_reason(std::optional<std::string> value) { connector_reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason() const noexcept { return connector_reason; }
    void set_connector_reason_code(std::optional<std::string> value) { connector_reason_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason_code() const noexcept { return connector_reason_code; }
    void set_challenge_required_by(std::optional<PrimitiveDateTimeValue> value) { challenge_required_by = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_challenge_required_by() const noexcept { return challenge_required_by; }
    void set_connector_created_at(std::optional<PrimitiveDateTimeValue> value) { connector_created_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_created_at() const noexcept { return connector_created_at; }
    void set_connector_updated_at(std::optional<PrimitiveDateTimeValue> value) { connector_updated_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_updated_at() const noexcept { return connector_updated_at; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_is_already_refunded(bool value) { is_already_refunded = std::move(value); }
    [[nodiscard]] const bool& get_is_already_refunded() const noexcept { return is_already_refunded; }
    void set_additional_details(std::optional<AdditionalDetailsValue> value) { additional_details = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalDetailsValue>& get_additional_details() const noexcept { return additional_details; }
    std::string dispute_id{};
    PaymentIdValue payment_id{};
    std::string attempt_id{};
    StringMinorUnitValue amount{};
    CurrencyValue currency{};
    DisputeStageValue dispute_stage{};
    DisputeStatusValue dispute_status{};
    std::string connector{};
    std::string connector_status{};
    std::string connector_dispute_id{};
    std::optional<std::string> connector_reason{};
    std::optional<std::string> connector_reason_code{};
    std::optional<PrimitiveDateTimeValue> challenge_required_by{};
    std::optional<PrimitiveDateTimeValue> connector_created_at{};
    std::optional<PrimitiveDateTimeValue> connector_updated_at{};
    PrimitiveDateTimeValue created_at{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    bool is_already_refunded{};
    std::optional<AdditionalDetailsValue> additional_details{};
};

class DisputeResponsePaymentsRetrieve
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_amount(StringMinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const StringMinorUnitValue& get_amount() const noexcept { return amount; }
    void set_dispute_stage(DisputeStageValue value) { dispute_stage = std::move(value); }
    [[nodiscard]] const DisputeStageValue& get_dispute_stage() const noexcept { return dispute_stage; }
    void set_dispute_status(DisputeStatusValue value) { dispute_status = std::move(value); }
    [[nodiscard]] const DisputeStatusValue& get_dispute_status() const noexcept { return dispute_status; }
    void set_connector_status(std::string value) { connector_status = std::move(value); }
    [[nodiscard]] const std::string& get_connector_status() const noexcept { return connector_status; }
    void set_connector_dispute_id(std::string value) { connector_dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_dispute_id() const noexcept { return connector_dispute_id; }
    void set_connector_reason(std::optional<std::string> value) { connector_reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason() const noexcept { return connector_reason; }
    void set_connector_reason_code(std::optional<std::string> value) { connector_reason_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason_code() const noexcept { return connector_reason_code; }
    void set_challenge_required_by(std::optional<PrimitiveDateTimeValue> value) { challenge_required_by = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_challenge_required_by() const noexcept { return challenge_required_by; }
    void set_connector_created_at(std::optional<PrimitiveDateTimeValue> value) { connector_created_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_created_at() const noexcept { return connector_created_at; }
    void set_connector_updated_at(std::optional<PrimitiveDateTimeValue> value) { connector_updated_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_updated_at() const noexcept { return connector_updated_at; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_additional_details(std::optional<AdditionalDetailsValue> value) { additional_details = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalDetailsValue>& get_additional_details() const noexcept { return additional_details; }
    std::string dispute_id{};
    StringMinorUnitValue amount{};
    DisputeStageValue dispute_stage{};
    DisputeStatusValue dispute_status{};
    std::string connector_status{};
    std::string connector_dispute_id{};
    std::optional<std::string> connector_reason{};
    std::optional<std::string> connector_reason_code{};
    std::optional<PrimitiveDateTimeValue> challenge_required_by{};
    std::optional<PrimitiveDateTimeValue> connector_created_at{};
    std::optional<PrimitiveDateTimeValue> connector_updated_at{};
    PrimitiveDateTimeValue created_at{};
    std::optional<AdditionalDetailsValue> additional_details{};
};

class PlatformDisputeListConstraints
{
public:
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }
    void set_payment_id(std::optional<PaymentIdValue> value) { payment_id = std::move(value); }
    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept { return payment_id; }
    void set_processor_merchant_id(std::optional<MerchantIdValue> value) { processor_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept { return processor_merchant_id; }
    void set_limit(PageSizeValue value) { limit = std::move(value); }
    [[nodiscard]] const PageSizeValue& get_limit() const noexcept { return limit; }
    void set_offset(PageOffsetValue value) { offset = std::move(value); }
    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept { return offset; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_dispute_status(std::optional<std::vector<DisputeStatusValue>> value) { dispute_status = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStatusValue>>& get_dispute_status() const noexcept { return dispute_status; }
    void set_dispute_stage(std::optional<std::vector<DisputeStageValue>> value) { dispute_stage = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<DisputeStageValue>>& get_dispute_stage() const noexcept { return dispute_stage; }
    void set_reason(std::optional<std::string> value) { reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept { return reason; }
    void set_connector(std::optional<std::vector<std::string>> value) { connector = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connector() const noexcept { return connector; }
    void set_currency(std::optional<std::vector<CurrencyValue>> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<CurrencyValue>>& get_currency() const noexcept { return currency; }
    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    void set_time_range(std::optional<TimeRangeValue> value) { time_range = std::move(value); }
    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept { return time_range; }
    std::optional<std::string> dispute_id{};
    std::optional<PaymentIdValue> payment_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<std::vector<DisputeStatusValue>> dispute_status{};
    std::optional<std::vector<DisputeStageValue>> dispute_stage{};
    std::optional<std::string> reason{};
    std::optional<std::vector<std::string>> connector{};
    std::optional<std::vector<CurrencyValue>> currency{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<TimeRangeValue> time_range{};
};

class PlatformDisputeListFilters
{
public:
    void set_connector(std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> value) { connector = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>>& get_connector() const noexcept { return connector; }
    void set_currency(std::vector<CurrencyValue> value) { currency = std::move(value); }
    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept { return currency; }
    void set_dispute_status(std::vector<DisputeStatusValue> value) { dispute_status = std::move(value); }
    [[nodiscard]] const std::vector<DisputeStatusValue>& get_dispute_status() const noexcept { return dispute_status; }
    void set_dispute_stage(std::vector<DisputeStageValue> value) { dispute_stage = std::move(value); }
    [[nodiscard]] const std::vector<DisputeStageValue>& get_dispute_stage() const noexcept { return dispute_stage; }
    std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<DisputeStatusValue> dispute_status{};
    std::vector<DisputeStageValue> dispute_stage{};
};

class PlatformDisputeListItem
{
public:
    void set_dispute_id(std::string value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_dispute_id() const noexcept { return dispute_id; }
    void set_payment_id(PaymentIdValue value) { payment_id = std::move(value); }
    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept { return payment_id; }
    void set_attempt_id(std::string value) { attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return attempt_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_processor_merchant_id(std::optional<MerchantIdValue> value) { processor_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantIdValue>& get_processor_merchant_id() const noexcept { return processor_merchant_id; }
    void set_amount(StringMinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const StringMinorUnitValue& get_amount() const noexcept { return amount; }
    void set_currency(CurrencyValue value) { currency = std::move(value); }
    [[nodiscard]] const CurrencyValue& get_currency() const noexcept { return currency; }
    void set_dispute_stage(DisputeStageValue value) { dispute_stage = std::move(value); }
    [[nodiscard]] const DisputeStageValue& get_dispute_stage() const noexcept { return dispute_stage; }
    void set_dispute_status(DisputeStatusValue value) { dispute_status = std::move(value); }
    [[nodiscard]] const DisputeStatusValue& get_dispute_status() const noexcept { return dispute_status; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_connector_status(std::string value) { connector_status = std::move(value); }
    [[nodiscard]] const std::string& get_connector_status() const noexcept { return connector_status; }
    void set_connector_dispute_id(std::string value) { connector_dispute_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_dispute_id() const noexcept { return connector_dispute_id; }
    void set_connector_reason(std::optional<std::string> value) { connector_reason = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason() const noexcept { return connector_reason; }
    void set_connector_reason_code(std::optional<std::string> value) { connector_reason_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_reason_code() const noexcept { return connector_reason_code; }
    void set_challenge_required_by(std::optional<PrimitiveDateTimeValue> value) { challenge_required_by = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_challenge_required_by() const noexcept { return challenge_required_by; }
    void set_connector_created_at(std::optional<PrimitiveDateTimeValue> value) { connector_created_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_created_at() const noexcept { return connector_created_at; }
    void set_connector_updated_at(std::optional<PrimitiveDateTimeValue> value) { connector_updated_at = std::move(value); }
    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_connector_updated_at() const noexcept { return connector_updated_at; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_profile_id(std::optional<ProfileIdValue> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept { return profile_id; }
    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value) { merchant_connector_id = std::move(value); }
    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>& get_merchant_connector_id() const noexcept { return merchant_connector_id; }
    std::string dispute_id{};
    PaymentIdValue payment_id{};
    std::string attempt_id{};
    MerchantIdValue merchant_id{};
    std::optional<MerchantIdValue> processor_merchant_id{};
    StringMinorUnitValue amount{};
    CurrencyValue currency{};
    DisputeStageValue dispute_stage{};
    DisputeStatusValue dispute_status{};
    std::string connector{};
    std::string connector_status{};
    std::string connector_dispute_id{};
    std::optional<std::string> connector_reason{};
    std::optional<std::string> connector_reason_code{};
    std::optional<PrimitiveDateTimeValue> challenge_required_by{};
    std::optional<PrimitiveDateTimeValue> connector_created_at{};
    std::optional<PrimitiveDateTimeValue> connector_updated_at{};
    PrimitiveDateTimeValue created_at{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
};

class PlatformDisputeListResponse
{
public:
    void set_count(std::size_t value) { count = std::move(value); }
    [[nodiscard]] const std::size_t& get_count() const noexcept { return count; }
    void set_total_count(std::int64_t value) { total_count = std::move(value); }
    [[nodiscard]] const std::int64_t& get_total_count() const noexcept { return total_count; }
    void set_data(std::vector<PlatformDisputeListItem> value) { data = std::move(value); }
    [[nodiscard]] const std::vector<PlatformDisputeListItem>& get_data() const noexcept { return data; }
    std::size_t count{};
    std::int64_t total_count{};
    std::vector<PlatformDisputeListItem> data{};
};
} // namespace app::model::disputes

template<>
struct serde::Serializable<app::model::disputes::DisputeResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DisputeResponse::get_dispute_id, &app::model::disputes::DisputeResponse::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::model::disputes::DisputeResponse::get_payment_id, &app::model::disputes::DisputeResponse::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempt_id", &app::model::disputes::DisputeResponse::get_attempt_id, &app::model::disputes::DisputeResponse::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::disputes::DisputeResponse::get_amount, &app::model::disputes::DisputeResponse::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::DisputeResponse::get_currency, &app::model::disputes::DisputeResponse::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::DisputeResponse::get_dispute_stage, &app::model::disputes::DisputeResponse::set_dispute_stage, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::DisputeResponse::get_dispute_status, &app::model::disputes::DisputeResponse::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::disputes::DisputeResponse::get_connector, &app::model::disputes::DisputeResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_status", &app::model::disputes::DisputeResponse::get_connector_status, &app::model::disputes::DisputeResponse::set_connector_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_dispute_id", &app::model::disputes::DisputeResponse::get_connector_dispute_id, &app::model::disputes::DisputeResponse::set_connector_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason", &app::model::disputes::DisputeResponse::get_connector_reason, &app::model::disputes::DisputeResponse::set_connector_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason_code", &app::model::disputes::DisputeResponse::get_connector_reason_code, &app::model::disputes::DisputeResponse::set_connector_reason_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge_required_by", &app::model::disputes::DisputeResponse::get_challenge_required_by, &app::model::disputes::DisputeResponse::set_challenge_required_by, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_created_at", &app::model::disputes::DisputeResponse::get_connector_created_at, &app::model::disputes::DisputeResponse::set_connector_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_updated_at", &app::model::disputes::DisputeResponse::get_connector_updated_at, &app::model::disputes::DisputeResponse::set_connector_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::model::disputes::DisputeResponse::get_created_at, &app::model::disputes::DisputeResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::disputes::DisputeResponse::get_profile_id, &app::model::disputes::DisputeResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::disputes::DisputeResponse::get_merchant_connector_id, &app::model::disputes::DisputeResponse::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_already_refunded", &app::model::disputes::DisputeResponse::get_is_already_refunded, &app::model::disputes::DisputeResponse::set_is_already_refunded, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_details", &app::model::disputes::DisputeResponse::get_additional_details, &app::model::disputes::DisputeResponse::set_additional_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_response"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeResponsePaymentsRetrieve>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_dispute_id, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_amount, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_dispute_stage, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_dispute_stage, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_dispute_status, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_status", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_status, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_dispute_id", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_dispute_id, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_reason, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason_code", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_reason_code, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_reason_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge_required_by", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_challenge_required_by, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_challenge_required_by, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_created_at", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_created_at, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_updated_at", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_connector_updated_at, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_connector_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_created_at, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"additional_details", &app::model::disputes::DisputeResponsePaymentsRetrieve::get_additional_details, &app::model::disputes::DisputeResponsePaymentsRetrieve::set_additional_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_response_payments_retrieve"; }
};

template<>
struct serde::Serializable<app::model::disputes::FileMetadataResponseValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file_id", &app::model::disputes::FileMetadataResponseValue::get_file_id, &app::model::disputes::FileMetadataResponseValue::set_file_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_name", &app::model::disputes::FileMetadataResponseValue::get_file_name, &app::model::disputes::FileMetadataResponseValue::set_file_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_size", &app::model::disputes::FileMetadataResponseValue::get_file_size, &app::model::disputes::FileMetadataResponseValue::set_file_size, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_type", &app::model::disputes::FileMetadataResponseValue::get_file_type, &app::model::disputes::FileMetadataResponseValue::set_file_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"available", &app::model::disputes::FileMetadataResponseValue::get_available, &app::model::disputes::FileMetadataResponseValue::set_available, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "file_metadata_response_value"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeFetchQueryData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"fetch_from", &app::model::disputes::DisputeFetchQueryData::get_fetch_from, &app::model::disputes::DisputeFetchQueryData::set_fetch_from, serde::FieldOptions{}>{},
            serde::FieldDesc<"fetch_till", &app::model::disputes::DisputeFetchQueryData::get_fetch_till, &app::model::disputes::DisputeFetchQueryData::set_fetch_till, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_fetch_query_data"; }
};

template<>
struct serde::Serializable<app::model::disputes::TimeRange>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"start_time", &app::model::disputes::TimeRange::get_start_time, &app::model::disputes::TimeRange::set_start_time, serde::FieldOptions{}>{},
            serde::FieldDesc<"end_time", &app::model::disputes::TimeRange::get_end_time, &app::model::disputes::TimeRange::set_end_time, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "time_range"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeEvidenceBlock>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"evidence_type", &app::model::disputes::DisputeEvidenceBlock::get_evidence_type, &app::model::disputes::DisputeEvidenceBlock::set_evidence_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_metadata_response", &app::model::disputes::DisputeEvidenceBlock::get_file_metadata_response, &app::model::disputes::DisputeEvidenceBlock::set_file_metadata_response, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_evidence_block"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeListGetConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DisputeListGetConstraints::get_dispute_id, &app::model::disputes::DisputeListGetConstraints::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::model::disputes::DisputeListGetConstraints::get_payment_id, &app::model::disputes::DisputeListGetConstraints::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::disputes::DisputeListGetConstraints::get_limit, &app::model::disputes::DisputeListGetConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"offset", &app::model::disputes::DisputeListGetConstraints::get_offset, &app::model::disputes::DisputeListGetConstraints::set_offset, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::disputes::DisputeListGetConstraints::get_profile_id, &app::model::disputes::DisputeListGetConstraints::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::DisputeListGetConstraints::get_dispute_status, &app::model::disputes::DisputeListGetConstraints::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::DisputeListGetConstraints::get_dispute_stage, &app::model::disputes::DisputeListGetConstraints::set_dispute_stage, serde::FieldOptions{}>{},
            serde::FieldDesc<"reason", &app::model::disputes::DisputeListGetConstraints::get_reason, &app::model::disputes::DisputeListGetConstraints::set_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::disputes::DisputeListGetConstraints::get_connector, &app::model::disputes::DisputeListGetConstraints::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::DisputeListGetConstraints::get_currency, &app::model::disputes::DisputeListGetConstraints::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::disputes::DisputeListGetConstraints::get_merchant_connector_id, &app::model::disputes::DisputeListGetConstraints::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"time_range", &app::model::disputes::DisputeListGetConstraints::get_time_range, &app::model::disputes::DisputeListGetConstraints::set_time_range, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_list_get_constraints"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::model::disputes::DisputeListFilters::get_connector, &app::model::disputes::DisputeListFilters::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::DisputeListFilters::get_currency, &app::model::disputes::DisputeListFilters::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::DisputeListFilters::get_dispute_status, &app::model::disputes::DisputeListFilters::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::DisputeListFilters::get_dispute_stage, &app::model::disputes::DisputeListFilters::set_dispute_stage, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_list_filters"; }
};

template<>
struct serde::Serializable<app::model::disputes::PlatformDisputeListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::PlatformDisputeListConstraints::get_dispute_id, &app::model::disputes::PlatformDisputeListConstraints::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::model::disputes::PlatformDisputeListConstraints::get_payment_id, &app::model::disputes::PlatformDisputeListConstraints::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"processor_merchant_id", &app::model::disputes::PlatformDisputeListConstraints::get_processor_merchant_id, &app::model::disputes::PlatformDisputeListConstraints::set_processor_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::disputes::PlatformDisputeListConstraints::get_limit, &app::model::disputes::PlatformDisputeListConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"offset", &app::model::disputes::PlatformDisputeListConstraints::get_offset, &app::model::disputes::PlatformDisputeListConstraints::set_offset, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::disputes::PlatformDisputeListConstraints::get_profile_id, &app::model::disputes::PlatformDisputeListConstraints::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::PlatformDisputeListConstraints::get_dispute_status, &app::model::disputes::PlatformDisputeListConstraints::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::PlatformDisputeListConstraints::get_dispute_stage, &app::model::disputes::PlatformDisputeListConstraints::set_dispute_stage, serde::FieldOptions{}>{},
            serde::FieldDesc<"reason", &app::model::disputes::PlatformDisputeListConstraints::get_reason, &app::model::disputes::PlatformDisputeListConstraints::set_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::disputes::PlatformDisputeListConstraints::get_connector, &app::model::disputes::PlatformDisputeListConstraints::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::PlatformDisputeListConstraints::get_currency, &app::model::disputes::PlatformDisputeListConstraints::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::disputes::PlatformDisputeListConstraints::get_merchant_connector_id, &app::model::disputes::PlatformDisputeListConstraints::set_merchant_connector_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"time_range", &app::model::disputes::PlatformDisputeListConstraints::get_time_range, &app::model::disputes::PlatformDisputeListConstraints::set_time_range, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_dispute_list_constraints"; }
};

template<>
struct serde::Serializable<app::model::disputes::PlatformDisputeListItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::PlatformDisputeListItem::get_dispute_id, &app::model::disputes::PlatformDisputeListItem::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::model::disputes::PlatformDisputeListItem::get_payment_id, &app::model::disputes::PlatformDisputeListItem::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempt_id", &app::model::disputes::PlatformDisputeListItem::get_attempt_id, &app::model::disputes::PlatformDisputeListItem::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::disputes::PlatformDisputeListItem::get_merchant_id, &app::model::disputes::PlatformDisputeListItem::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"processor_merchant_id", &app::model::disputes::PlatformDisputeListItem::get_processor_merchant_id, &app::model::disputes::PlatformDisputeListItem::set_processor_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::model::disputes::PlatformDisputeListItem::get_amount, &app::model::disputes::PlatformDisputeListItem::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::PlatformDisputeListItem::get_currency, &app::model::disputes::PlatformDisputeListItem::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::PlatformDisputeListItem::get_dispute_stage, &app::model::disputes::PlatformDisputeListItem::set_dispute_stage, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::PlatformDisputeListItem::get_dispute_status, &app::model::disputes::PlatformDisputeListItem::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::model::disputes::PlatformDisputeListItem::get_connector, &app::model::disputes::PlatformDisputeListItem::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_status", &app::model::disputes::PlatformDisputeListItem::get_connector_status, &app::model::disputes::PlatformDisputeListItem::set_connector_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_dispute_id", &app::model::disputes::PlatformDisputeListItem::get_connector_dispute_id, &app::model::disputes::PlatformDisputeListItem::set_connector_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason", &app::model::disputes::PlatformDisputeListItem::get_connector_reason, &app::model::disputes::PlatformDisputeListItem::set_connector_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_reason_code", &app::model::disputes::PlatformDisputeListItem::get_connector_reason_code, &app::model::disputes::PlatformDisputeListItem::set_connector_reason_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"challenge_required_by", &app::model::disputes::PlatformDisputeListItem::get_challenge_required_by, &app::model::disputes::PlatformDisputeListItem::set_challenge_required_by, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_created_at", &app::model::disputes::PlatformDisputeListItem::get_connector_created_at, &app::model::disputes::PlatformDisputeListItem::set_connector_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_updated_at", &app::model::disputes::PlatformDisputeListItem::get_connector_updated_at, &app::model::disputes::PlatformDisputeListItem::set_connector_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::model::disputes::PlatformDisputeListItem::get_created_at, &app::model::disputes::PlatformDisputeListItem::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::disputes::PlatformDisputeListItem::get_profile_id, &app::model::disputes::PlatformDisputeListItem::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_connector_id", &app::model::disputes::PlatformDisputeListItem::get_merchant_connector_id, &app::model::disputes::PlatformDisputeListItem::set_merchant_connector_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_dispute_list_item"; }
};

template<>
struct serde::Serializable<app::model::disputes::PlatformDisputeListFilters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::model::disputes::PlatformDisputeListFilters::get_connector, &app::model::disputes::PlatformDisputeListFilters::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::model::disputes::PlatformDisputeListFilters::get_currency, &app::model::disputes::PlatformDisputeListFilters::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_status", &app::model::disputes::PlatformDisputeListFilters::get_dispute_status, &app::model::disputes::PlatformDisputeListFilters::set_dispute_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_stage", &app::model::disputes::PlatformDisputeListFilters::get_dispute_stage, &app::model::disputes::PlatformDisputeListFilters::set_dispute_stage, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_dispute_list_filters"; }
};

template<>
struct serde::Serializable<app::model::disputes::PlatformDisputeListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"count", &app::model::disputes::PlatformDisputeListResponse::get_count, &app::model::disputes::PlatformDisputeListResponse::set_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"total_count", &app::model::disputes::PlatformDisputeListResponse::get_total_count, &app::model::disputes::PlatformDisputeListResponse::set_total_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::model::disputes::PlatformDisputeListResponse::get_data, &app::model::disputes::PlatformDisputeListResponse::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_dispute_list_response"; }
};

template<>
struct serde::Serializable<app::model::disputes::SubmitEvidenceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::SubmitEvidenceRequest::get_dispute_id, &app::model::disputes::SubmitEvidenceRequest::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"access_activity_log", &app::model::disputes::SubmitEvidenceRequest::get_access_activity_log, &app::model::disputes::SubmitEvidenceRequest::set_access_activity_log, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_address", &app::model::disputes::SubmitEvidenceRequest::get_billing_address, &app::model::disputes::SubmitEvidenceRequest::set_billing_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"cancellation_policy", &app::model::disputes::SubmitEvidenceRequest::get_cancellation_policy, &app::model::disputes::SubmitEvidenceRequest::set_cancellation_policy, serde::FieldOptions{}>{},
            serde::FieldDesc<"cancellation_policy_disclosure", &app::model::disputes::SubmitEvidenceRequest::get_cancellation_policy_disclosure, &app::model::disputes::SubmitEvidenceRequest::set_cancellation_policy_disclosure, serde::FieldOptions{}>{},
            serde::FieldDesc<"cancellation_rebuttal", &app::model::disputes::SubmitEvidenceRequest::get_cancellation_rebuttal, &app::model::disputes::SubmitEvidenceRequest::set_cancellation_rebuttal, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_communication", &app::model::disputes::SubmitEvidenceRequest::get_customer_communication, &app::model::disputes::SubmitEvidenceRequest::set_customer_communication, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_email_address", &app::model::disputes::SubmitEvidenceRequest::get_customer_email_address, &app::model::disputes::SubmitEvidenceRequest::set_customer_email_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_name", &app::model::disputes::SubmitEvidenceRequest::get_customer_name, &app::model::disputes::SubmitEvidenceRequest::set_customer_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_purchase_ip", &app::model::disputes::SubmitEvidenceRequest::get_customer_purchase_ip, &app::model::disputes::SubmitEvidenceRequest::set_customer_purchase_ip, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_signature", &app::model::disputes::SubmitEvidenceRequest::get_customer_signature, &app::model::disputes::SubmitEvidenceRequest::set_customer_signature, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_description", &app::model::disputes::SubmitEvidenceRequest::get_product_description, &app::model::disputes::SubmitEvidenceRequest::set_product_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"receipt", &app::model::disputes::SubmitEvidenceRequest::get_receipt, &app::model::disputes::SubmitEvidenceRequest::set_receipt, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_policy", &app::model::disputes::SubmitEvidenceRequest::get_refund_policy, &app::model::disputes::SubmitEvidenceRequest::set_refund_policy, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_policy_disclosure", &app::model::disputes::SubmitEvidenceRequest::get_refund_policy_disclosure, &app::model::disputes::SubmitEvidenceRequest::set_refund_policy_disclosure, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_refusal_explanation", &app::model::disputes::SubmitEvidenceRequest::get_refund_refusal_explanation, &app::model::disputes::SubmitEvidenceRequest::set_refund_refusal_explanation, serde::FieldOptions{}>{},
            serde::FieldDesc<"service_date", &app::model::disputes::SubmitEvidenceRequest::get_service_date, &app::model::disputes::SubmitEvidenceRequest::set_service_date, serde::FieldOptions{}>{},
            serde::FieldDesc<"service_documentation", &app::model::disputes::SubmitEvidenceRequest::get_service_documentation, &app::model::disputes::SubmitEvidenceRequest::set_service_documentation, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping_address", &app::model::disputes::SubmitEvidenceRequest::get_shipping_address, &app::model::disputes::SubmitEvidenceRequest::set_shipping_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping_carrier", &app::model::disputes::SubmitEvidenceRequest::get_shipping_carrier, &app::model::disputes::SubmitEvidenceRequest::set_shipping_carrier, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping_date", &app::model::disputes::SubmitEvidenceRequest::get_shipping_date, &app::model::disputes::SubmitEvidenceRequest::set_shipping_date, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping_documentation", &app::model::disputes::SubmitEvidenceRequest::get_shipping_documentation, &app::model::disputes::SubmitEvidenceRequest::set_shipping_documentation, serde::FieldOptions{}>{},
            serde::FieldDesc<"shipping_tracking_number", &app::model::disputes::SubmitEvidenceRequest::get_shipping_tracking_number, &app::model::disputes::SubmitEvidenceRequest::set_shipping_tracking_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"invoice_showing_distinct_transactions", &app::model::disputes::SubmitEvidenceRequest::get_invoice_showing_distinct_transactions, &app::model::disputes::SubmitEvidenceRequest::set_invoice_showing_distinct_transactions, serde::FieldOptions{}>{},
            serde::FieldDesc<"recurring_transaction_agreement", &app::model::disputes::SubmitEvidenceRequest::get_recurring_transaction_agreement, &app::model::disputes::SubmitEvidenceRequest::set_recurring_transaction_agreement, serde::FieldOptions{}>{},
            serde::FieldDesc<"uncategorized_file", &app::model::disputes::SubmitEvidenceRequest::get_uncategorized_file, &app::model::disputes::SubmitEvidenceRequest::set_uncategorized_file, serde::FieldOptions{}>{},
            serde::FieldDesc<"uncategorized_text", &app::model::disputes::SubmitEvidenceRequest::get_uncategorized_text, &app::model::disputes::SubmitEvidenceRequest::set_uncategorized_text, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "submit_evidence_request"; }
};

template<>
struct serde::Serializable<app::model::disputes::DeleteEvidenceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DeleteEvidenceRequest::get_dispute_id, &app::model::disputes::DeleteEvidenceRequest::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"evidence_type", &app::model::disputes::DeleteEvidenceRequest::get_evidence_type, &app::model::disputes::DeleteEvidenceRequest::set_evidence_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "delete_evidence_request"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DisputeRetrieveRequest::get_dispute_id, &app::model::disputes::DisputeRetrieveRequest::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"force_sync", &app::model::disputes::DisputeRetrieveRequest::get_force_sync, &app::model::disputes::DisputeRetrieveRequest::set_force_sync, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_retrieve_request"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputesAggregateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status_with_count", &app::model::disputes::DisputesAggregateResponse::get_status_with_count, &app::model::disputes::DisputesAggregateResponse::set_status_with_count, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "disputes_aggregate_response"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeId>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::model::disputes::DisputeId::get_dispute_id, &app::model::disputes::DisputeId::set_dispute_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_id"; }
};

template<>
struct serde::Serializable<app::model::disputes::DisputeRetrieveBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"force_sync", &app::model::disputes::DisputeRetrieveBody::get_force_sync, &app::model::disputes::DisputeRetrieveBody::set_force_sync, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "dispute_retrieve_body"; }
};

#ifdef CONGELADO_TEST
namespace app::model::disputes::tests {
using namespace boost::ut;
suite<"app_model_disputes"> app_model_disputes_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::model::disputes::DisputeResponse>::fields())> > 0);
    };
};
}
#endif
