export module app_handlers_refunds:runtime_common;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_locking;
import app_events;
import app_workflow;
import app_services;

import :input;
import :result;

export namespace app::handlers::refunds {

// Upstream consts.rs values used by the error/status state machine.
inline constexpr std::string_view kUnifiedErrorCode{"UE_9000"};
inline constexpr std::string_view kUnifiedErrorMsg{"Something went wrong"};
inline constexpr std::string_view kTimeoutCode{"TIMEOUT"};
inline constexpr std::string_view kIntegrityCode{"IE"};

// Upstream refund statuses (common_enums RefundStatus + api serde aliases).
inline constexpr std::array<std::string_view, 5>
    kAllRefundStatuses{"Failure", "ManualReview", "Pending", "Success", "TransactionFailure"};

// Upstream Currency::iter() (common_enums/src/enums.rs:1093-1255), in declaration order.
inline constexpr std::array<std::string_view, 160> kAllCurrencies{
    "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN", "BAM", "BBD", "BDT",
    "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL", "BSD", "BTN", "BWP", "BYN", "BZD", "CAD",
    "CDF", "CHF", "CLF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP", "CVE", "CZK", "DJF", "DKK",
    "DOP", "DZD", "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP", "GEL", "GHS", "GIP", "GMD",
    "GNF", "GTQ", "GYD", "HKD", "HNL", "HRK", "HTG", "HUF", "IDR", "ILS", "INR", "IQD", "IRR",
    "ISK", "JMD", "JOD", "JPY", "KES", "KGS", "KHR", "KMF", "KPW", "KRW", "KWD", "KYD", "KZT",
    "LAK", "LBP", "LKR", "LRD", "LSL", "LYD", "MAD", "MDL", "MGA", "MKD", "MMK", "MNT", "MOP",
    "MRU", "MUR", "MVR", "MWK", "MXN", "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR", "NZD",
    "OMR", "PAB", "PEN", "PGK", "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF",
    "SAR", "SBD", "SCR", "SDG", "SEK", "SGD", "SHP", "SLE", "SLL", "SOS", "SRD", "SSP", "STD",
    "STN", "SVC", "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP", "TRY", "TTD", "TWD", "TZS",
    "UAH", "UGX", "USD", "UYU", "UZS", "VES", "VND", "VUV", "WST", "XAF", "XCD", "XOF", "XPF",
    "YER", "ZAR", "ZMW", "ZWL"
};

/**
 * @brief In-process model of the upstream diesel Refund columns not present in the local
 *        refund store row. Persisted as a JSON blob in the in-process cache.
 */
struct RefundExt
{
    std::string internal_reference_id{};
    std::string external_reference_id{};
    std::string connector_transaction_id{};
    std::string attempt_id{};
    std::string profile_id{};
    std::string merchant_connector_id{};
    std::string connector_refund_id{};
    std::string metadata{};
    std::string feature_metadata{};
    std::string refund_type{"Instant"};
    std::string split_refunds{};
    std::string description{};
    std::string unified_code{};
    std::string unified_message{};
    std::string error_code{};
    std::string error_message{};
    std::string issuer_error_code{};
    std::string issuer_error_message{};
    std::string raw_connector_response{};
    std::string net_amount{};
    std::string surcharge_amount{};
    std::string created_by{};
    bool sent_to_gateway{false};
};

struct RefundUpdate
{
    std::optional<std::string> status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<std::string> issuer_error_code{};
    std::optional<std::string> issuer_error_message{};
    std::optional<std::string> connector_refund_id{};
    bool sent_to_gateway{false};
};

struct RetrieveOutcome
{
    Record record{};
    RefundExt ext{};
    std::optional<std::string> raw{};
};

struct ParsedConnectorError
{
    int status_code{};
    std::string code{};
    std::string message{};
    std::string issuer_code{};
    std::string issuer_message{};
};

struct RefundFilter
{
    std::string refund_id{}, payment_id{}, merchant_id{}, connector{}, currency{}, status{};
};

/**
 * @brief Shared business helpers for the refunds sub-runtimes. Mirrors the upstream core
 *        helpers function-for-function; never touches HTTP types.
 */
class RefundRuntimeSupport
{
public:
    explicit RefundRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // =================================================================================
    // CORE CREATE — core/refunds.rs:405-500, :1677-1945; core/refunds_v2.rs:44-134.
    // =================================================================================

    /** @brief core/refunds.rs:1677-1945 — full validation + insert + execute. */
    std::expected<Record, Error> validate_and_create_refund(
        std::string_view payment_id,
        const Intent& intent,
        const Attempt& attempt,
        std::int64_t refund_amount,
        std::string_view creds_identifier,
        std::optional<std::string> requested_refund_id,
        std::optional<app::model::refunds::MerchantIdValue> req_merchant_id,
        std::optional<std::string> reason,
        std::optional<std::string> metadata,
        std::optional<std::string> split_refunds,
        std::optional<app::model::refunds::RefundType> refund_type,
        std::optional<std::string> feature_metadata,
        std::optional<bool> all_keys_required,
        std::optional<std::string> payment_method_data,
        const RefundRequestFacts& facts
    ) const
    {
        // core/refunds.rs:1696-1705 — scheduled refund cannot carry payment method data.
        if (refund_type.value_or(app::model::refunds::RefundType::Instant) ==
                app::model::refunds::RefundType::Scheduled &&
            payment_method_data.has_value()) {
            return std::unexpected(
                err(Kind::INVALID_REQUEST,
                    "payment_method_data is not supported for scheduled refunds")
            );
        }
        // core/refunds.rs:1707-1709 — get_or_generate_id.
        const std::string refund_id =
            requested_refund_id.value_or(m_context.ids().payment_id("ref_"));

        // core/refunds.rs:1711-1722 — merchant_id in request must equal processor merchant.
        const std::string processor_merchant = actor_merchant(facts);
        if (req_merchant_id && !req_merchant_id->get_value().empty() &&
            req_merchant_id->get_value() != processor_merchant) {
            return std::unexpected(err(Kind::INVALID_REQUEST, "invalid merchant_id in request"));
        }
        // core/refunds.rs:1724-1740, :1870-1887 — duplicate refund request.
        if (find_refund_record(refund_id)) {
            return std::unexpected(err(Kind::DUPLICATE_REQUEST, "DuplicateRefundRequest"));
        }
        const auto all_refunds = m_context.repository().find_refunds_by_payment_id(payment_id);
        // core/refunds.rs:1744-1751 — validate_payment_order_age.
        if (const auto e = validate_payment_order_age(intent)) {
            return std::unexpected(*e);
        }
        // core/refunds.rs:1753-1761 — validate_refund_amount.
        const std::int64_t total_captured = intent.get_amount();
        if (const auto e = validate_refund_amount(total_captured, all_refunds, refund_amount)) {
            return std::unexpected(*e);
        }
        // core/refunds.rs:1763-1767 — validate_maximum_refund_against_payment_attempt.
        if (const auto e = validate_maximum_refund_against_payment_attempt(all_refunds)) {
            return std::unexpected(*e);
        }
        // core/refunds.rs:1807-1812 — validate_for_valid_refunds.
        if (const auto e = validate_for_valid_refunds(attempt, attempt.get_connector())) {
            return std::unexpected(*e);
        }

        Record record{};
        record.set_refund_id(refund_id);
        record.set_payment_id(std::string{payment_id});
        record.set_merchant_id(processor_merchant);
        record.set_amount(refund_amount);
        record.set_currency(
            attempt.get_currency().empty() ? intent.get_currency() : attempt.get_currency()
        );
        record.set_status("Pending");
        if (reason) {
            record.set_reason(*reason);
        }
        record = m_context.repository().save_refund(record);

        RefundExt ext{};
        ext.internal_reference_id = m_context.ids().payment_id("refid_");
        ext.external_reference_id = refund_id;
        ext.connector_transaction_id = m_context.ids().payment_id("txn_");
        ext.attempt_id = attempt.get_attempt_id();
        ext.profile_id =
            ext_of(intent).profile_id.empty() ? actor_profile(facts) : ext_of(intent).profile_id;
        ext.merchant_connector_id = attempt.get_connector();
        ext.refund_type = refund_type.value_or(app::model::refunds::RefundType::Instant) ==
                                  app::model::refunds::RefundType::Scheduled
                              ? "Scheduled"
                              : "Instant";
        ext.description = reason.value_or(std::string{});
        if (metadata) {
            ext.metadata = *metadata;
        }
        if (split_refunds) {
            ext.split_refunds = *split_refunds;
        }
        if (feature_metadata) {
            ext.feature_metadata = *feature_metadata;
        }
        if (payment_method_data) {
            ext.connector_transaction_id += "|pmd";
        }
        ext.net_amount = std::to_string(refund_amount);
        save_ext(refund_id, ext);

        // core/refunds.rs:1888-1900 → schedule_refund_execution → trigger_refund_to_gateway.
        const auto update = trigger_refund_to_gateway(
            record,
            ext,
            creds_identifier,
            split_refunds,
            all_keys_required
        );
        apply_refund_update(record, ext, update);

        // core/refunds.rs:1924-1942 — unified translation of stored message.
        ext.unified_message = get_unified_translation(ext.unified_code, ext.unified_message);
        save_ext(refund_id, ext);
        return record;
    }

    // =================================================================================
    // CORE EXECUTE — core/refunds.rs:504-864 (trigger_refund_to_gateway + state machine).
    // =================================================================================

    /** @brief core/refunds.rs:504-699 — connector selection + gateway decision + execute. */
    RefundUpdate trigger_refund_to_gateway(
        Record& record,
        RefundExt& ext,
        std::string_view creds_identifier,
        std::optional<std::string> split_refunds,
        std::optional<bool> all_keys_required
    ) const
    {
        const auto routed = routed_connector(record);
        m_context.metrics().counter("REFUND_COUNT").increment();

        app::connector::ConnectorRequest request{};
        request.set_operation("refund");
        request.set_merchant_id(record.get_merchant_id());
        request.set_amount(record.get_amount());
        request.set_currency(record.get_currency());
        request.set_resource_id(record.get_payment_id());
        std::string raw = std::format(
            "{{\"flow\":\"refund\",\"refund_id\":\"{}\",\"connector\":\"{}\",\"all_keys_required\":"
            "{}}}",
            record.get_refund_id(),
            routed,
            all_keys_required.value_or(false) ? "true" : "false"
        );
        if (!creds_identifier.empty()) {
            raw += std::format(",\"creds_identifier\":\"{}\"", creds_identifier);
        }
        if (split_refunds) {
            raw += std::format(",\"split_refunds\":\"{}\"", *split_refunds);
        }
        request.set_raw_payload(std::move(raw));

        auto connector = m_context.connector_router().select_for(request);
        if (!connector) {
            connector = m_context.connector_router().select(record.get_merchant_id());
        }
        if (!connector) {
            m_context.metrics().record_flow("refund.execute.no_connector");
            m_context.metrics().counter("refunds_execute_no_connector").increment();
            return {};
        }
        record.set_connector(std::string{connector->name()});
        const std::string name{connector->name()};
        apply_gateway_decision(name, creds_identifier, /*flow=*/"refund");

        const auto tracker = m_context.process_trackers().schedule(
            std::string{"refund_refund"},
            std::format(
                "{{\"refund_id\":\"{}\",\"connector\":\"{}\"}}",
                record.get_refund_id(),
                name
            ),
            "0"
        );
        const auto response = connector->refund(request);
        m_context.process_trackers().mark_terminal(
            tracker,
            build_refund_update_for_response(response, /*sync=*/false).status.value_or("Pending")
        );
        m_context.metrics().record_flow("refund.execute.refund");
        return build_refund_update_for_response(response, /*sync=*/false);
    }

    /** @brief core/refunds.rs:701-836 — mirror of `match router_data_res.response`. */
    RefundUpdate build_refund_update_for_response(
        const app::connector::ConnectorResponse& response,
        bool sync
    ) const
    {
        RefundUpdate update{};
        const auto error = parse_connector_error(response);
        if (error.has_value()) {
            const int status_code = error->status_code;
            if (sync) {
                if (status_code >= 200 && status_code <= 299) {
                    update.status = "Failure";
                }
            } else {
                if (!(status_code >= 500 && status_code <= 511)) {
                    update.status = "Failure";
                }
            }
            if (error->code != kTimeoutCode) {
                update.error_message = error->message;
                update.error_code = error->code;
            }
            if (!sync) {
                const auto gsm = get_gsm_record(response, error->code, error->message);
                update.unified_code = gsm.first;
                update.unified_message = gsm.second;
            }
            update.issuer_error_code = error->issuer_code;
            update.issuer_error_message = error->issuer_message;
            return update;
        }

        if (!integrity_check_ok(response)) {
            update.status = "ManualReview";
            update.error_code = std::string{kIntegrityCode};
            update.error_message = std::format(
                "Integrity Check Failed! as data mismatched for fields {}",
                integrity_failed_fields(response)
            );
            update.connector_refund_id =
                response.get_reference_id().empty()
                    ? std::nullopt
                    : std::optional<std::string>{response.get_reference_id()};
            m_context.metrics().counter("INTEGRITY_CHECK_FAILED").increment();
            return update;
        }

        update.status =
            status_from_text(response.get_status()) == Status::Succeeded ? "Success" : "Pending";
        update.connector_refund_id = response.get_reference_id().empty()
                                         ? std::nullopt
                                         : std::optional<std::string>{response.get_reference_id()};
        update.sent_to_gateway = true;
        if (status_from_text(response.get_status()) == Status::Succeeded) {
            m_context.metrics().counter("SUCCESSFUL_REFUND").increment();
        }
        return update;
    }

    /** @brief core/refunds.rs:839-863 — persist RefundUpdate + webhook + offer engine. */
    void apply_refund_update(Record& record, RefundExt& ext, const RefundUpdate& update) const
    {
        if (update.status) {
            record.set_status(*update.status);
        }
        if (update.connector_refund_id) {
            ext.connector_refund_id = *update.connector_refund_id;
        }
        if (update.error_code) {
            ext.error_code = *update.error_code;
        }
        if (update.error_message) {
            ext.error_message = *update.error_message;
        }
        if (update.unified_code) {
            ext.unified_code = *update.unified_code;
        }
        if (update.unified_message) {
            ext.unified_message = *update.unified_message;
        }
        if (update.issuer_error_code) {
            ext.issuer_error_code = *update.issuer_error_code;
        }
        if (update.issuer_error_message) {
            ext.issuer_error_message = *update.issuer_error_message;
        }
        if (update.sent_to_gateway) {
            ext.sent_to_gateway = true;
        }
        m_context.repository().update_refund(record);
        save_ext(record.get_refund_id(), ext);

        trigger_refund_outgoing_webhook(record, ext);
        if (status_from_text(record.get_status()) == Status::Succeeded) {
            const std::string payload = std::format(
                "{{\"refund_id\":\"{}\",\"payment_id\":\"{}\"}}",
                record.get_refund_id(),
                record.get_payment_id()
            );
            (void)m_context.events().publish("offer_engine.refund_notification", payload);
            m_context.metrics().counter("refunds_offer_engine_notified").increment();
        }
    }

    // =================================================================================
    // CORE RETRIEVE / SYNC — core/refunds.rs:1100-1212, :1216-1518.
    // =================================================================================

    std::expected<RetrieveOutcome, Error> refund_retrieve_core_with_refund_id(
        const RefundRequestFacts& facts,
        const app::model::refunds::RefundsRetrieveRequest& request
    ) const
    {
        const auto found = find_refund_record(request.get_refund_id());
        if (!found) {
            return std::unexpected(err(Kind::NOT_FOUND, "RefundNotFound"));
        }
        return refund_retrieve_core(facts, request, *found);
    }

    std::expected<RetrieveOutcome, Error> refund_retrieve_core(
        const RefundRequestFacts& facts,
        const app::model::refunds::RefundsRetrieveRequest& request,
        const Record& refund
    ) const
    {
        // core/refunds.rs:1112 — validate_profile_id_from_auth_layer.
        auto ext = load_ext(refund.get_refund_id());
        if (const auto e = validate_profile_id_from_auth_layer(facts.ctx_profile_id, ext)) {
            return std::unexpected(*e);
        }
        const auto intent = find_payment_intent(refund.get_payment_id());
        if (!intent) {
            return std::unexpected(err(Kind::NOT_FOUND, "PaymentNotFound"));
        }
        auto attempt = find_attempt_by_id(refund.get_payment_id(), ext.attempt_id);
        if (!attempt) {
            attempt = find_last_successful_attempt(refund.get_payment_id());
        }
        if (!attempt) {
            return std::unexpected(err(Kind::INTERNAL, "InternalServerError"));
        }

        std::string creds_identifier{};
        if (request.get_merchant_connector_details()) {
            const auto value = request.get_merchant_connector_details()->get_value();
            creds_identifier = extract_creds_identifier(value);
            insert_merchant_connector_creds_to_config(value);
        }
        ext.unified_message = get_unified_translation(ext.unified_code, ext.unified_message);

        Record record = refund;
        std::optional<std::string> raw{};
        if (should_call_refund(
                record,
                request.get_force_sync().value_or(false),
                request.get_all_keys_required().value_or(false)
            )) {
            auto synced = sync_refund_with_gateway(
                record,
                ext,
                *intent,
                *attempt,
                creds_identifier,
                ext.split_refunds,
                request.get_all_keys_required()
            );
            if (!synced) {
                return std::unexpected(synced.error());
            }
            record = synced->record;
            ext = synced->ext;
            raw = synced->raw;
        }
        return RetrieveOutcome{record, ext, raw};
    }

    /** @brief core/refunds.rs:1199-1212 — exact force_sync/all_keys gating. */
    bool should_call_refund(const Record& record, bool force_sync, bool all_keys_required) const
    {
        const bool terminal = record.get_status() == "Success" ||
                              record.get_status() == "Failure" ||
                              record.get_status() == "succeeded" || record.get_status() == "failed";
        return all_keys_required || (force_sync && !terminal);
    }

    /** @brief core/refunds.rs:1216-1518 — RSync connector flow + update. */
    std::expected<RetrieveOutcome, Error> sync_refund_with_gateway(
        Record& record,
        RefundExt& ext,
        const Intent& intent,
        const Attempt& attempt,
        std::string_view creds_identifier,
        std::optional<std::string> split_refunds,
        std::optional<bool> all_keys_required
    ) const
    {
        (void)intent;
        (void)attempt;
        const auto routed = routed_connector(record);
        app::connector::ConnectorRequest request{};
        request.set_operation("sync");
        request.set_merchant_id(record.get_merchant_id());
        request.set_amount(record.get_amount());
        request.set_currency(record.get_currency());
        request.set_resource_id(record.get_payment_id());
        std::string raw = std::format(
            "{{\"flow\":\"RSync\",\"refund_id\":\"{}\",\"connector\":\"{}\",\"all_keys_required\":{"
            "}}}",
            record.get_refund_id(),
            routed,
            all_keys_required.value_or(false) ? "true" : "false"
        );
        if (!creds_identifier.empty()) {
            raw += std::format(",\"creds_identifier\":\"{}\"", creds_identifier);
        }
        if (split_refunds) {
            raw += std::format(",\"split_refunds\":\"{}\"", *split_refunds);
        }
        request.set_raw_payload(std::move(raw));

        auto connector = m_context.connector_router().select_for(request);
        if (!connector) {
            connector = m_context.connector_router().select(record.get_merchant_id());
        }
        if (!connector) {
            return std::unexpected(err(Kind::INTERNAL, "Failed to get the connector"));
        }
        apply_gateway_decision(connector->name(), creds_identifier, /*flow=*/"sync");
        const auto tracker = m_context.process_trackers().schedule(
            std::string{"refund_sync"},
            std::format(
                "{{\"refund_id\":\"{}\",\"connector\":\"{}\"}}",
                record.get_refund_id(),
                connector->name()
            ),
            "0"
        );
        const auto response = connector->sync(request);
        const auto update = build_refund_update_for_response(response, /*sync=*/true);
        m_context.process_trackers().mark_terminal(
            tracker,
            update.status.value_or(record.get_status())
        );
        apply_refund_update(record, ext, update);
        if (!response.get_raw_json().empty()) {
            ext.raw_connector_response = response.get_raw_json();
        }
        return RetrieveOutcome{
            record,
            ext,
            response.get_raw_json().empty() ? std::nullopt
                                            : std::optional<std::string>{response.get_raw_json()}
        };
    }

    // =================================================================================
    // CORE UPDATE / REVERSE / MANUAL / METADATA.
    // =================================================================================

    std::expected<Record, Error>
    refund_update_core(const app::model::refunds::RefundUpdateRequest& req) const
    {
        const auto found = find_refund_record(req.get_refund_id());
        if (!found) {
            return std::unexpected(err(Kind::NOT_FOUND, "RefundNotFound"));
        }
        Record record = *found;
        auto ext = load_ext(record.get_refund_id());
        if (req.get_reason()) {
            record.set_reason(*req.get_reason());
            ext.description = *req.get_reason();
        }
        if (req.get_metadata()) {
            ext.metadata = req.get_metadata()->get_value();
        }
        m_context.repository().update_refund(record);
        save_ext(record.get_refund_id(), ext);
        return record;
    }

    std::expected<Record, Error>
    refund_reverse_core(const app::model::refunds::RefundReverseRequest& payload) const
    {
        const auto found = find_refund_record(payload.get_refund_id());
        if (!found) {
            return std::unexpected(err(Kind::NOT_FOUND, "RefundNotFound"));
        }
        Record record = *found;
        if (status_from_text(record.get_status()) != Status::Succeeded) {
            return std::unexpected(
                err(Kind::PRECONDITION_FAILED,
                    "refund reverse is only supported for successful refunds (refund_status)")
            );
        }
        const auto intent = find_payment_intent(record.get_payment_id());
        if (!intent) {
            return std::unexpected(err(Kind::NOT_FOUND, "PaymentNotFound"));
        }
        const auto attempt = find_last_successful_attempt(record.get_payment_id());
        if (!attempt) {
            return std::unexpected(err(Kind::NOT_FOUND, "SuccessfulPaymentNotFound"));
        }
        (void)intent;
        (void)attempt;
        auto ext = load_ext(record.get_refund_id());

        app::connector::ConnectorRequest request{};
        request.set_operation("cancel");
        request.set_merchant_id(record.get_merchant_id());
        request.set_amount(record.get_amount());
        request.set_currency(record.get_currency());
        request.set_resource_id(record.get_payment_id());
        request.set_raw_payload(
            std::format(
                "{{\"flow\":\"void_post_refund\",\"refund_id\":\"{}\",\"reason\":\"{}\"}}",
                record.get_refund_id(),
                payload.get_cancellation_reason().value_or(std::string{})
            )
        );

        apply_gateway_decision(
            record.get_connector(),
            ext.merchant_connector_id,
            /*flow=*/"reverse"
        );

        auto connector = m_context.connector_router().select(record.get_merchant_id());
        if (!connector) {
            return std::unexpected(err(Kind::INTERNAL, "Failed to get the connector"));
        }
        const auto response = connector->cancel(request);

        ext.metadata = build_void_post_refund_metadata(ext.metadata, response);
        if (payload.get_cancellation_reason()) {
            record.set_reason(*payload.get_cancellation_reason());
        }

        if (const auto failure = parse_connector_error(response)) {
            ext.error_code = failure->code;
            ext.error_message = failure->message;
            m_context.repository().update_refund(record);
            save_ext(record.get_refund_id(), ext);
            return std::unexpected(err(Kind::CONNECTOR_ERROR, failure->message));
        }
        if (!response.get_reference_id().empty()) {
            ext.connector_refund_id = response.get_reference_id();
        }
        m_context.repository().update_refund(record);
        save_ext(record.get_refund_id(), ext);
        return record;
    }

    /** @brief core/refunds_v2.rs:662-698 — metadata-only state update. */
    std::expected<Record, Error> refund_metadata_update_core(
        std::string_view refund_id,
        const app::model::refunds::RefundMetadataUpdateRequest& payload
    ) const
    {
        const auto found = find_refund_record(refund_id);
        if (!found) {
            return std::unexpected(err(Kind::NOT_FOUND, "RefundNotFound"));
        }
        Record record = *found;
        auto ext = load_ext(record.get_refund_id());
        if (payload.get_reason()) {
            record.set_reason(*payload.get_reason());
            ext.description = *payload.get_reason();
        }
        if (payload.get_metadata()) {
            ext.metadata = payload.get_metadata()->get_value();
        }
        m_context.repository().update_refund(record);
        save_ext(record.get_refund_id(), ext);
        return record;
    }

    /** @brief core/refunds.rs:2223-2278 — admin ManualUpdate (SetOrUnset semantics). */
    std::optional<Error>
    refund_manual_update(const app::model::refunds::RefundManualUpdateRequest& req) const
    {
        const std::string merchant = req.get_merchant_id().get_value().empty()
                                         ? actor_merchant_hint()
                                         : req.get_merchant_id().get_value();
        const auto account = m_context.repository().find_merchant_account_by_string(merchant);
        if (!account) {
            return err(Kind::NOT_FOUND, "MerchantAccountNotFound");
        }
        const auto found = find_refund_record(req.get_refund_id());
        if (!found) {
            return err(Kind::NOT_FOUND, "RefundNotFound");
        }
        Record record = *found;
        auto ext = load_ext(record.get_refund_id());
        if (req.get_connector_refund_id()) {
            ext.connector_refund_id = *req.get_connector_refund_id();
        }
        if (req.get_status()) {
            record.set_status(std::string{status_to_text(*req.get_status())});
        }
        if (req.get_error_code()) {
            const auto& code = *req.get_error_code();
            ext.error_code = (code == "Unset") ? std::string{} : code;
        }
        if (req.get_error_message()) {
            const auto& msg = *req.get_error_message();
            ext.error_message = (msg == "Unset") ? std::string{} : msg;
        }
        m_context.repository().update_refund(record);
        save_ext(record.get_refund_id(), ext);
        return std::nullopt;
    }

    // =================================================================================
    // LIST / FILTER / AGGREGATE — core/refunds.rs:1953-2353.
    // =================================================================================

    std::expected<app::model::refunds::RefundListResponse, Error> refund_list(
        const RefundRequestFacts& facts,
        const app::model::refunds::RefundListRequest& payload,
        bool profile_scoped
    ) const
    {
        const auto limit = validate_refund_list(
            payload.get_limit()
                ? std::optional<std::int64_t>{parse_i64(payload.get_limit()->get_value())}
                : std::nullopt
        );
        if (!limit) {
            return std::unexpected(
                err(Kind::INVALID_REQUEST, "limit should be in between 1 and 100")
            );
        }
        const std::size_t offset = parse_usize(payload.get_offset().get_value());

        RefundFilter filter{};
        if (payload.get_payment_id()) {
            filter.payment_id = payload.get_payment_id()->get_value();
        }
        if (payload.get_refund_id()) {
            filter.refund_id = *payload.get_refund_id();
        }
        if (payload.get_connector() && !payload.get_connector()->empty()) {
            filter.connector = payload.get_connector()->front();
        }
        if (payload.get_currency() && !payload.get_currency()->empty()) {
            filter.currency = payload.get_currency()->front().get_value();
        }
        if (payload.get_refund_status() && !payload.get_refund_status()->empty()) {
            filter.status = payload.get_refund_status()->front().get_value();
        }

        const auto all = merchant_records(facts, profile_scoped);
        const auto matched = apply_refund_filter(all, filter);
        const auto paged = paginate(matched, *limit, offset);
        app::model::refunds::RefundListResponse response{};
        std::vector<Response> data{};
        data.reserve(paged.size());
        for (const auto& record: paged) {
            data.push_back(to_refund_response(record));
        }
        response.set_count(data.size());
        response.set_total_count(static_cast<std::int64_t>(matched.size()));
        response.set_data(std::move(data));
        return response;
    }

    std::expected<app::model::refunds::PlatformRefundListResponse, Error> refund_list_for_platform(
        const RefundRequestFacts& facts,
        const app::model::refunds::PlatformRefundListRequest& payload
    ) const
    {
        const std::string merchant = actor_merchant(facts);
        if (!is_platform_account(merchant)) {
            return std::unexpected(
                err(Kind::UNAUTHORIZED,
                    "Platform refunds list is only accessible to platform merchants")
            );
        }
        const std::int64_t limit =
            payload.get_limit() ? parse_i64(payload.get_limit()->get_value()) : 10;
        const std::size_t offset = parse_usize(payload.get_offset().get_value());
        RefundFilter filter{};
        if (payload.get_payment_id()) {
            filter.payment_id = payload.get_payment_id()->get_value();
        }
        if (payload.get_refund_id()) {
            filter.refund_id = *payload.get_refund_id();
        }
        if (payload.get_connector() && !payload.get_connector()->empty()) {
            filter.connector = payload.get_connector()->front();
        }
        if (payload.get_currency() && !payload.get_currency()->empty()) {
            filter.currency = payload.get_currency()->front().get_value();
        }
        if (payload.get_refund_status() && !payload.get_refund_status()->empty()) {
            filter.status = payload.get_refund_status()->front().get_value();
        }

        std::vector<Record> records{};
        for (const auto& account: m_context.repository().list_merchant_accounts()) {
            if (account.get_parent_merchant_id() != merchant &&
                account.get_merchant_id() != merchant) {
                continue;
            }
            for (const auto& r: m_context.repository().list_refunds()) {
                if (r.get_merchant_id() == account.get_merchant_id()) {
                    records.push_back(r);
                }
            }
        }
        const auto matched = apply_refund_filter(records, filter);
        const auto paged =
            paginate(matched, limit < 1 ? 10 : static_cast<std::size_t>(limit), offset);
        std::vector<app::model::refunds::PlatformRefundListItem> data{};
        data.reserve(paged.size());
        for (const auto& r: paged) {
            data.push_back(to_platform_item(r));
        }
        app::model::refunds::PlatformRefundListResponse response{};
        response.set_count(data.size());
        response.set_total_count(static_cast<std::int64_t>(matched.size()));
        response.set_data(std::move(data));
        return response;
    }

    std::expected<app::model::refunds::PlatformRefundListFilters, Error>
    get_platform_refund_filters(const RefundRequestFacts& facts) const
    {
        const std::string merchant = actor_merchant(facts);
        if (!is_platform_account(merchant)) {
            return std::unexpected(
                err(Kind::UNAUTHORIZED,
                    "Platform refund filters are only accessible to platform merchants")
            );
        }
        app::model::refunds::PlatformRefundListFilters filters{};
        filters.set_connector(
            connector_filter_map(merchant, true, /*platform=*/true, facts.ctx_profile_id)
        );
        filters.set_currency(all_currencies());
        filters.set_refund_status(all_refund_status_values());
        return filters;
    }

    app::model::refunds::RefundListFilters
    get_filters_for_refunds(const RefundRequestFacts& facts, bool profile_scoped) const
    {
        app::model::refunds::RefundListFilters filters{};
        filters.set_connector(connector_filter_map(
            actor_merchant(facts),
            profile_scoped,
            /*platform=*/false,
            facts.ctx_profile_id
        ));
        filters.set_currency(all_currencies());
        filters.set_refund_status(all_refund_status_values());
        return filters;
    }

    app::model::refunds::RefundAggregateResponse get_aggregates_for_refunds(
        const RefundRequestFacts& facts,
        std::optional<std::string> start_time,
        std::optional<std::string> end_time,
        bool profile_scoped
    ) const
    {
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto status: kAllRefundStatuses) {
            counts[std::string{status}] = 0;
        }
        for (const auto& record: merchant_records(facts, profile_scoped)) {
            if (!in_time_range(record, start_time, end_time)) {
                continue;
            }
            const std::string canonical = canonical_status(record.get_status());
            ++counts[canonical];
        }
        app::model::refunds::RefundAggregateResponse response{};
        response.set_status_with_count(std::move(counts));
        return response;
    }

    // =================================================================================
    // VALIDATORS — core/utils/refunds_validator.rs and core/refunds.rs:426-465, 1677-1812.
    // =================================================================================

    std::optional<Error>
    validate_profile_id_from_auth_layer(std::string_view profile_id, const Intent& intent) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        if (ext_of(intent).profile_id.empty() || ext_of(intent).profile_id == profile_id) {
            return std::nullopt;
        }
        return err(Kind::FORBIDDEN, "profile is not authorized for this payment");
    }

    std::optional<Error> validate_profile_id_from_auth_layer(
        std::string_view profile_id,
        const RefundExt& refund_ext
    ) const
    {
        if (profile_id.empty() || refund_ext.profile_id.empty()) {
            return std::nullopt;
        }
        if (refund_ext.profile_id == profile_id) {
            return std::nullopt;
        }
        return err(Kind::FORBIDDEN, "profile is not authorized for this refund");
    }

    std::optional<Error> validate_refundable_status(const Intent& intent) const
    {
        const auto& status = lowercase(intent.get_status());
        if (status == "succeeded" || status == "partially_captured" ||
            status == "partiallycaptured") {
            return std::nullopt;
        }
        return err(
            Kind::PRECONDITION_FAILED,
            "unable to refund for a unsuccessful payment intent (status)"
        );
    }

    std::optional<Error> validate_amount_against_intent_state_metadata(
        const Intent& intent,
        std::optional<std::int64_t> requested
    ) const
    {
        const auto blocked = parse_amount_from_metadata(intent.get_metadata(), "blocked_amount");
        const std::int64_t amount = requested.value_or(intent.get_amount());
        if (blocked + amount > intent.get_amount()) {
            return err(Kind::PRECONDITION_FAILED, "RefundAmountExceedsPaymentAmount");
        }
        return std::nullopt;
    }

    std::optional<Error> prevent_refund_after_post_capture_void(const Intent& intent) const
    {
        if (intent.get_metadata().find("post_capture_void") != std::string::npos) {
            return err(
                Kind::PRECONDITION_FAILED,
                "refund void cannot be performed after a post-capture void"
            );
        }
        return std::nullopt;
    }

    std::optional<Error> validate_payment_order_age(const Intent& intent) const
    {
        const auto max_age = parse_i64(config_value("refund_max_age", "refund").value_or("60"));
        const auto age_days = days_between_iso(intent.get_created_at());
        if (age_days > max_age) {
            return err(Kind::PRECONDITION_FAILED, "The order has expired");
        }
        return std::nullopt;
    }

    std::optional<Error> validate_refund_amount(
        std::int64_t captured,
        const std::vector<Record>& all_refunds,
        std::int64_t amount
    ) const
    {
        std::int64_t already{0};
        for (const auto& r: all_refunds) {
            const auto status = status_from_text(r.get_status());
            if (status != Status::Failed) {
                already += r.get_amount();
            }
        }
        if (already + amount > captured) {
            return err(Kind::PRECONDITION_FAILED, "RefundAmountExceedsPaymentAmount");
        }
        return std::nullopt;
    }

    std::optional<Error>
    validate_maximum_refund_against_payment_attempt(const std::vector<Record>& all_refunds) const
    {
        const auto max_attempts =
            parse_i64(config_value("refund_max_attempts", "refund").value_or("10"));
        if (static_cast<std::int64_t>(all_refunds.size()) > max_attempts) {
            return err(Kind::PRECONDITION_FAILED, "MaximumRefundCount");
        }
        return std::nullopt;
    }

    std::optional<Error>
    validate_for_valid_refunds(const Attempt& attempt, std::string_view connector) const
    {
        const std::string key = std::format("refund_not_supported.{}", connector);
        if (config_bool("refund_not_supported", std::string{connector})) {
            return err(
                Kind::INVALID_REQUEST,
                std::format(
                    "Refunds are currently not supported for transactions via {}",
                    connector
                )
            );
        }
        (void)attempt;
        (void)key;
        return std::nullopt;
    }

    std::optional<std::int64_t> validate_refund_list(std::optional<std::int64_t> limit) const
    {
        if (!limit) {
            return 10;
        }
        if (*limit < 1 || *limit > 100) {
            return std::nullopt;
        }
        return limit;
    }

    // =================================================================================
    // IN-PROCESS INFRA SUBSTITUTES — UCS / GSM / access token / locale / integrity / webhook.
    // =================================================================================

    std::optional<ParsedConnectorError>
    parse_connector_error(const app::connector::ConnectorResponse& response) const
    {
        const std::string raw = response.get_raw_json();
        if (!response.get_status().empty()) {
            return std::nullopt;
        }
        if (raw.empty() && response.get_message().empty()) {
            return std::nullopt;
        }
        ParsedConnectorError error{};
        error.status_code = parse_i64(json_number(raw, "status_code"));
        error.code = json_string(raw, "code");
        error.message = json_string(raw, "message");
        if (error.message.empty()) {
            error.message = response.get_message();
        }
        if (error.code.empty()) {
            error.code = response.get_message().empty() ? "UE_9000" : response.get_message();
        }
        error.issuer_code = json_string(raw, "issuer_error_code");
        error.issuer_message = json_string(raw, "issuer_error_message");
        return error;
    }

    std::pair<std::string, std::string> get_gsm_record(
        const app::connector::ConnectorResponse& response,
        std::string_view code,
        std::string_view message
    ) const
    {
        const std::string raw = response.get_raw_json();
        std::string unified_code = json_string(raw, "unified_code");
        std::string unified_message = json_string(raw, "unified_message");
        if (unified_code.empty()) {
            unified_code = config_value(std::format("gsm.{}", code), std::string{code})
                               .value_or(std::string{kUnifiedErrorCode});
        }
        if (unified_message.empty()) {
            unified_message = config_value(std::format("gsm.{}", message), std::string{message})
                                  .value_or(std::string{kUnifiedErrorMsg});
        }
        return {
            unified_code.empty() ? std::string{kUnifiedErrorCode} : unified_code,
            unified_message.empty() ? std::string{kUnifiedErrorMsg} : unified_message
        };
    }

    std::string get_unified_translation(std::string_view code, std::string_view message) const
    {
        if (code.empty() || message.empty()) {
            return std::string{message};
        }
        return config_value(std::format("locale.{}.{}", code, message), "locale")
            .value_or(std::string{message});
    }

    bool integrity_check_ok(const app::connector::ConnectorResponse& response) const
    {
        return json_string(response.get_raw_json(), "integrity_check").empty() ||
               json_string(response.get_raw_json(), "integrity_check") == "ok";
    }

    std::string integrity_failed_fields(const app::connector::ConnectorResponse& response) const
    {
        const auto fields = json_string(response.get_raw_json(), "integrity_failed_fields");
        return fields.empty() ? std::string{"connector_refund_id"} : fields;
    }

    void apply_gateway_decision(
        std::string_view connector,
        std::string_view creds_identifier,
        std::string_view flow
    ) const
    {
        m_context.metrics().record_flow(std::format("refund.{}.gateway_decision", flow));
        if (m_context.config_store().is_killed(connector)) {
            m_context.metrics().counter("refunds_connector_killed").increment();
        }
        if (m_context.config_store().is_shadow_killed(connector)) {
            m_context.metrics().counter("refunds_connector_shadow_killed").increment();
        }
        if (!creds_identifier.empty()) {
            m_context.metrics().counter("refunds_gateway_creds_used").increment();
        }
    }

    std::string build_void_post_refund_metadata(
        std::string_view existing_metadata,
        const app::connector::ConnectorResponse& response
    ) const
    {
        std::string out{existing_metadata};
        if (!out.empty() && out.back() == '}') {
            out.pop_back();
        }
        if (!out.empty() && out.back() != '{') {
            out += ',';
        }
        out += std::format(
            "\"state_metadata\":{{\"status\":\"{}\",\"connector_refund_id\":\"{}\"}}}}",
            response.get_status().empty() ? "Failure" : response.get_status(),
            response.get_reference_id()
        );
        return out;
    }

    void trigger_refund_outgoing_webhook(const Record& record, const RefundExt& ext) const
    {
        const std::string topic =
            std::format("refund.{}", lowercase(canonical_status(record.get_status())));
        const std::string payload = std::format(
            "{{\"refund_id\":\"{}\",\"payment_id\":\"{}\",\"status\":\"{}\"}}",
            record.get_refund_id(),
            record.get_payment_id(),
            canonical_status(record.get_status())
        );
        const std::string id = m_context.events().publish(topic, payload);
        const std::string endpoint =
            m_context.config_store()
                .get_value("refund_webhook_endpoint", record.get_merchant_id())
                .value_or(std::string{"local://refund-webhook/"} + record.get_merchant_id());
        m_context.webhook_delivery().enqueue(id, endpoint);
        m_context.metrics().counter("refunds_webhook_notified").increment();
        (void)ext;
    }

    // =================================================================================
    // PERSISTENCE / LOOKUP HELPERS.
    // =================================================================================

    std::optional<Intent> find_payment_intent(std::string_view payment_id) const
    {
        if (payment_id.empty()) {
            return std::nullopt;
        }
        return m_context.repository().find_payment_by_string(payment_id);
    }

    RefundExt ext_of(const Intent& intent) const
    {
        RefundExt ext{};
        ext.profile_id = m_context.config_store()
                             .get_value("refund_profile_scope", intent.get_merchant_id())
                             .value_or(std::string{});
        const auto recorded = load_ext(intent.get_payment_id());
        if (ext.profile_id.empty()) {
            ext.profile_id = recorded.profile_id;
        }
        return ext;
    }

    std::optional<Attempt> find_last_successful_attempt(std::string_view payment_id) const
    {
        std::optional<Attempt> best{};
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (std::string_view{attempt.get_payment_id()} != payment_id) {
                continue;
            }
            const auto status = lowercase(attempt.get_status());
            const bool success = status == "succeeded" || status == "success" ||
                                 status == "captured" || status == "partially_captured" ||
                                 status == "charged";
            if (!success) {
                continue;
            }
            if (!best || attempt.get_created_at() > best->get_created_at()) {
                best = attempt;
            }
        }
        return best;
    }

    std::optional<Attempt>
    find_attempt_by_id(std::string_view payment_id, std::string_view attempt_id) const
    {
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (std::string_view{attempt.get_payment_id()} == payment_id &&
                std::string_view{attempt.get_attempt_id()} == attempt_id) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    std::optional<Record> find_refund_record(std::string_view refund_id) const
    {
        if (refund_id.empty()) {
            return std::nullopt;
        }
        return m_context.repository().find_refund_by_string(refund_id);
    }

    std::string routed_connector(const Record& record) const
    {
        if (!record.get_connector().empty()) {
            return record.get_connector();
        }
        const auto attempt = find_last_successful_attempt(record.get_payment_id());
        return attempt ? attempt->get_connector() : std::string{};
    }

    RefundExt load_ext(std::string_view refund_id) const
    {
        RefundExt ext{};
        const auto blob = m_context.cache()
                              .get(std::string{"refund.ext."} + std::string{refund_id})
                              .value_or(std::string{});
        if (blob.empty()) {
            return ext;
        }
        ext.internal_reference_id = json_string(blob, "internal_reference_id");
        ext.external_reference_id = json_string(blob, "external_reference_id");
        ext.connector_transaction_id = json_string(blob, "connector_transaction_id");
        ext.attempt_id = json_string(blob, "attempt_id");
        ext.profile_id = json_string(blob, "profile_id");
        ext.merchant_connector_id = json_string(blob, "merchant_connector_id");
        ext.connector_refund_id = json_string(blob, "connector_refund_id");
        ext.metadata = json_string(blob, "metadata");
        ext.feature_metadata = json_string(blob, "feature_metadata");
        ext.refund_type = json_string(blob, "refund_type");
        ext.split_refunds = json_string(blob, "split_refunds");
        ext.description = json_string(blob, "description");
        ext.unified_code = json_string(blob, "unified_code");
        ext.unified_message = json_string(blob, "unified_message");
        ext.error_code = json_string(blob, "error_code");
        ext.error_message = json_string(blob, "error_message");
        ext.issuer_error_code = json_string(blob, "issuer_error_code");
        ext.issuer_error_message = json_string(blob, "issuer_error_message");
        ext.raw_connector_response = json_string(blob, "raw_connector_response");
        ext.net_amount = json_string(blob, "net_amount");
        ext.surcharge_amount = json_string(blob, "surcharge_amount");
        ext.created_by = json_string(blob, "created_by");
        ext.sent_to_gateway = json_string(blob, "sent_to_gateway") == "true";
        return ext;
    }

    void save_ext(std::string_view refund_id, const RefundExt& ext) const
    {
        std::string blob{"{"};
        const auto put = [this, &blob](std::string_view key, std::string_view value)
        {
            blob += std::format("\"{}\":\"{}\",", key, json_escape(value));
        };
        put("internal_reference_id", ext.internal_reference_id);
        put("external_reference_id", ext.external_reference_id);
        put("connector_transaction_id", ext.connector_transaction_id);
        put("attempt_id", ext.attempt_id);
        put("profile_id", ext.profile_id);
        put("merchant_connector_id", ext.merchant_connector_id);
        put("connector_refund_id", ext.connector_refund_id);
        put("metadata", ext.metadata);
        put("feature_metadata", ext.feature_metadata);
        put("refund_type", ext.refund_type);
        put("split_refunds", ext.split_refunds);
        put("description", ext.description);
        put("unified_code", ext.unified_code);
        put("unified_message", ext.unified_message);
        put("error_code", ext.error_code);
        put("error_message", ext.error_message);
        put("issuer_error_code", ext.issuer_error_code);
        put("issuer_error_message", ext.issuer_error_message);
        put("raw_connector_response", ext.raw_connector_response);
        put("net_amount", ext.net_amount);
        put("surcharge_amount", ext.surcharge_amount);
        put("created_by", ext.created_by);
        blob += std::format("\"sent_to_gateway\":\"{}\"}}", ext.sent_to_gateway ? "true" : "false");
        m_context.cache().put(
            std::string{"refund.ext."} + std::string{refund_id},
            core::cache::to_cache_value(blob),
            core::cache::Ttl{}.hours(24)
        );
    }

    // =================================================================================
    // RESPONSE MAPPING — core/refunds.rs:2355-2466.
    // =================================================================================

    Response to_refund_response(const Record& record, bool include_raw = false) const
    {
        const auto ext = load_ext(record.get_refund_id());
        Response response{};
        response.set_refund_id(record.get_refund_id());
        app::model::refunds::PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        response.set_payment_id(payment);
        app::model::refunds::MinorUnitValue amount{};
        amount.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount);
        response.set_currency(record.get_currency());
        response.set_status(status_from_text(record.get_status()));
        if (!record.get_reason().empty()) {
            response.set_reason(record.get_reason());
        }
        if (!ext.metadata.empty()) {
            app::model::refunds::SecretSerdeValueValue meta{};
            meta.set_value(ext.metadata);
            response.set_metadata(meta);
        }
        if (!ext.feature_metadata.empty()) {
            app::model::refunds::RefundFeatureMetadataValue feature{};
            feature.set_value(ext.feature_metadata);
            response.set_feature_metadata(feature);
        }
        if (!ext.error_message.empty()) {
            response.set_error_message(ext.error_message);
        }
        if (!ext.error_code.empty()) {
            response.set_error_code(ext.error_code);
        }
        if (!ext.unified_code.empty()) {
            response.set_unified_code(ext.unified_code);
        }
        if (!ext.unified_message.empty()) {
            response.set_unified_message(ext.unified_message);
        }
        if (!ext.issuer_error_code.empty()) {
            response.set_issuer_error_code(ext.issuer_error_code);
        }
        if (!ext.issuer_error_message.empty()) {
            response.set_issuer_error_message(ext.issuer_error_message);
        }
        if (!ext.net_amount.empty()) {
            app::model::refunds::MinorUnitValue net{};
            net.set_value(ext.net_amount);
            response.set_net_amount(net);
        }
        if (!ext.surcharge_amount.empty()) {
            app::model::refunds::MinorUnitValue surcharge{};
            surcharge.set_value(ext.surcharge_amount);
            response.set_surcharge_refund_amount(surcharge);
        }
        app::model::refunds::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created_at(created);
        app::model::refunds::PrimitiveDateTimeValue updated{};
        updated.set_value(
            record.get_updated_at().empty() ? record.get_created_at() : record.get_updated_at()
        );
        response.set_updated_at(updated);
        response.set_connector(record.get_connector());
        if (!ext.profile_id.empty()) {
            app::model::refunds::ProfileIdValue profile{};
            profile.set_value(ext.profile_id);
            response.set_profile_id(profile);
        }
        if (!ext.merchant_connector_id.empty()) {
            app::model::refunds::MerchantConnectorAccountIdValue mca{};
            mca.set_value(ext.merchant_connector_id);
            response.set_merchant_connector_id(mca);
        }
        if (!ext.split_refunds.empty()) {
            app::model::refunds::SplitRefundValue split{};
            split.set_value(ext.split_refunds);
            response.set_split_refunds(split);
        }
        if (!ext.connector_refund_id.empty()) {
            response.set_connector_refund_id(ext.connector_refund_id);
        }
        if (include_raw && !ext.raw_connector_response.empty()) {
            response.set_raw_connector_response(ext.raw_connector_response);
        }
        return response;
    }

    app::model::refunds::PlatformRefundListItem to_platform_item(const Record& record) const
    {
        const auto ext = load_ext(record.get_refund_id());
        app::model::refunds::PlatformRefundListItem item{};
        item.set_refund_id(record.get_refund_id());
        app::model::refunds::PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        item.set_payment_id(payment);
        app::model::refunds::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(merchant);
        if (!ext.merchant_connector_id.empty()) {
            item.set_merchant_connector_id(app::model::refunds::MerchantConnectorAccountIdValue{});
        }
        item.set_connector(record.get_connector());
        if (!ext.connector_refund_id.empty()) {
            item.set_connector_refund_id(ext.connector_refund_id);
        }
        item.set_attempt_id(ext.attempt_id);
        app::model::refunds::MinorUnitValue amount{};
        amount.set_value(std::to_string(record.get_amount()));
        item.set_refund_amount(amount);
        app::model::refunds::MinorUnitValue total{};
        total.set_value(std::to_string(record.get_amount()));
        item.set_total_amount(total);
        app::model::refunds::CurrencyValue currency{};
        currency.set_value(record.get_currency());
        item.set_currency(currency);
        app::model::refunds::RefundStatusValue status{};
        status.set_value(record.get_status());
        item.set_refund_status(status);
        if (!record.get_reason().empty()) {
            item.set_refund_reason(record.get_reason());
        }
        if (!ext.description.empty()) {
            item.set_description(ext.description);
        }
        app::model::refunds::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(created);
        app::model::refunds::PrimitiveDateTimeValue modified{};
        modified.set_value(record.get_updated_at());
        item.set_modified_at(modified);
        return item;
    }

    // =================================================================================
    // FILTER / AGGREGATE SHAPES.
    // =================================================================================

    std::vector<Record>
    apply_refund_filter(std::vector<Record> records, const RefundFilter& filter) const
    {
        std::vector<Record> selected{};
        for (const auto& r: records) {
            if (!filter.refund_id.empty() && filter.refund_id != r.get_refund_id()) {
                continue;
            }
            if (!filter.payment_id.empty() && filter.payment_id != r.get_payment_id()) {
                continue;
            }
            if (!filter.merchant_id.empty() && filter.merchant_id != r.get_merchant_id()) {
                continue;
            }
            if (!filter.connector.empty() && filter.connector != r.get_connector()) {
                continue;
            }
            if (!filter.currency.empty() && filter.currency != r.get_currency()) {
                continue;
            }
            if (!filter.status.empty() && filter.status != r.get_status() &&
                filter.status != canonical_status(r.get_status())) {
                continue;
            }
            selected.push_back(r);
        }
        return selected;
    }

    std::vector<Record>
    paginate(const std::vector<Record>& records, std::size_t limit, std::size_t offset) const
    {
        if (offset >= records.size()) {
            return {};
        }
        const std::size_t count = std::min(limit, records.size() - offset);
        return std::vector<Record>{
            records.begin() + static_cast<std::ptrdiff_t>(offset),
            records.begin() + static_cast<std::ptrdiff_t>(offset + count)
        };
    }

    std::vector<Record> merchant_records(const RefundRequestFacts& facts, bool profile_scoped) const
    {
        std::string merchant = facts.header_merchant_id;
        if (merchant.empty()) {
            merchant = facts.ctx_merchant_id;
        }
        std::string profile = facts.ctx_profile_id;
        if (profile.empty()) {
            profile = actor_profile(facts);
        }
        if (profile_scoped && !profile.empty()) {
            merchant = m_context.config_store()
                           .get_value("refund_profile_scope", profile)
                           .value_or(merchant);
        }
        std::vector<Record> out{};
        for (const auto& r: m_context.repository().list_refunds()) {
            if (merchant.empty() || r.get_merchant_id().empty() ||
                r.get_merchant_id() == merchant) {
                out.push_back(r);
            }
        }
        return out;
    }

    std::unordered_map<std::string, std::vector<app::model::refunds::MerchantConnectorInfoValue>>
    connector_filter_map(
        std::string_view merchant,
        bool profile_scoped,
        bool platform,
        std::string_view profile_id
    ) const
    {
        std::
            unordered_map<std::string, std::vector<app::model::refunds::MerchantConnectorInfoValue>>
                map{};
        std::vector<std::string> allowed_merchants{};
        if (platform) {
            for (const auto& account: m_context.repository().list_merchant_accounts()) {
                if (lowercase(account.get_merchant_account_type()) == "connected" &&
                    (account.get_parent_merchant_id() == merchant ||
                     account.get_organization_id() == platform_org(merchant))) {
                    allowed_merchants.push_back(account.get_merchant_id());
                }
            }
        }
        for (const auto& connector: m_context.repository().list_merchant_connectors()) {
            if (connector.get_connector_label().empty()) {
                continue;
            }
            if (profile_scoped && !profile_id.empty() && !connector.get_profile_id().empty() &&
                connector.get_profile_id() != profile_id) {
                continue;
            }
            if (platform) {
                if (std::find(
                        allowed_merchants.begin(),
                        allowed_merchants.end(),
                        connector.get_merchant_id()
                    ) == allowed_merchants.end()) {
                    continue;
                }
            } else if (
                !merchant.empty() && !connector.get_merchant_id().empty() &&
                connector.get_merchant_id() != merchant
            ) {
                continue;
            }
            app::model::refunds::MerchantConnectorInfoValue info{};
            info.set_value(connector.get_connector_label());
            map[connector.get_connector_name()].push_back(std::move(info));
        }
        return map;
    }

    std::string platform_org(std::string_view merchant) const
    {
        const auto account = m_context.repository().find_merchant_account_by_string(merchant);
        return account ? account->get_organization_id() : std::string{};
    }

    std::vector<app::model::refunds::CurrencyValue> all_currencies() const
    {
        std::vector<app::model::refunds::CurrencyValue> out{};
        out.reserve(kAllCurrencies.size());
        for (const auto code: kAllCurrencies) {
            app::model::refunds::CurrencyValue value{};
            value.set_value(std::string{code});
            out.push_back(std::move(value));
        }
        return out;
    }

    std::vector<app::model::refunds::RefundStatusValue> all_refund_status_values() const
    {
        std::vector<app::model::refunds::RefundStatusValue> out{};
        out.reserve(kAllRefundStatuses.size());
        for (const auto status: kAllRefundStatuses) {
            app::model::refunds::RefundStatusValue value{};
            value.set_value(std::string{status});
            out.push_back(std::move(value));
        }
        return out;
    }

    app::model::refunds::RefundListMetaData
    filter_list_metadata(const std::vector<Record>& records) const
    {
        std::vector<std::string> connectors{};
        std::vector<app::model::refunds::CurrencyValue> currencies{};
        std::vector<app::model::refunds::RefundStatusValue> statuses{};
        std::unordered_set<std::string> seen_connector, seen_currency, seen_status{};
        for (const auto& r: records) {
            if (seen_connector.insert(r.get_connector()).second) {
                connectors.push_back(r.get_connector());
            }
            if (seen_currency.insert(r.get_currency()).second) {
                app::model::refunds::CurrencyValue value{};
                value.set_value(r.get_currency());
                currencies.push_back(std::move(value));
            }
            const auto canonical = canonical_status(r.get_status());
            if (seen_status.insert(canonical).second) {
                app::model::refunds::RefundStatusValue value{};
                value.set_value(canonical);
                statuses.push_back(std::move(value));
            }
        }
        app::model::refunds::RefundListMetaData response{};
        response.set_connector(std::move(connectors));
        response.set_currency(std::move(currencies));
        response.set_refund_status(std::move(statuses));
        return response;
    }

    std::vector<Record> apply_time_range(
        const std::vector<Record>& records,
        std::string_view start,
        std::string_view end
    ) const
    {
        std::vector<Record> out{};
        for (const auto& r: records) {
            const auto& created = r.get_created_at();
            if (!start.empty() && !created.empty() && created < start) {
                continue;
            }
            if (!end.empty() && !created.empty() && created > end) {
                continue;
            }
            out.push_back(r);
        }
        return out;
    }

    bool in_time_range(
        const Record& record,
        std::optional<std::string> start,
        std::optional<std::string> end
    ) const
    {
        if (!start && !end) {
            return true;
        }
        const auto& created = record.get_created_at();
        if (created.empty()) {
            return true;
        }
        if (start && created < *start) {
            return false;
        }
        if (end && created > *end) {
            return false;
        }
        return true;
    }

    // =================================================================================
    // SMALL UTILITIES.
    // =================================================================================

    Error err(Kind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    std::string actor_merchant(const RefundRequestFacts& facts) const
    {
        auto merchant = facts.header_merchant_id;
        if (merchant.empty()) {
            merchant = facts.ctx_merchant_id;
        }
        if (merchant.empty()) {
            merchant = actor_merchant_hint();
        }
        return merchant;
    }

    std::string actor_merchant_hint() const
    {
        return m_context.config_store()
            .get_value("default_merchant_id", "refund")
            .value_or(std::string{});
    }

    std::string actor_profile(const RefundRequestFacts& facts) const
    {
        return m_context.config_store()
            .get_value("refund_request_profile", facts.request_path)
            .value_or(std::string{});
    }

    std::string actor_profile_hint() const
    {
        return m_context.config_store()
            .get_value("refund_profile_scope", "current")
            .value_or(std::string{});
    }

    bool is_platform_account(std::string_view merchant) const
    {
        if (merchant.empty()) {
            return false;
        }
        const auto account = m_context.repository().find_merchant_account_by_string(merchant);
        if (!account) {
            return false;
        }
        return lowercase(account->get_merchant_account_type()) == "platform";
    }

    std::optional<std::string> config_value(std::string_view key, std::string_view ns) const
    {
        return m_context.config_store().get_value(ns, key);
    }

    bool config_bool(std::string_view key, std::string_view ns) const
    {
        return config_value(key, ns).value_or("false") == "true";
    }

    void insert_merchant_connector_creds_to_config(std::string_view mcd) const
    {
        if (mcd.empty()) {
            return;
        }
        const auto id = extract_creds_identifier(mcd);
        if (!id.empty()) {
            m_context.config_store().put("merchant_connector_details", id, std::string{mcd});
        }
    }

    std::string extract_creds_identifier(std::string_view value) const
    {
        constexpr std::string_view key{"\"creds_identifier\""};
        const auto pos = value.find(key);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto cursor = pos + key.size();
        while (cursor < value.size() &&
               (value[cursor] == ':' || std::isspace(static_cast<unsigned char>(value[cursor])))) {
            ++cursor;
        }
        if (cursor >= value.size() || value[cursor] != '"') {
            return {};
        }
        ++cursor;
        const auto end = value.find('"', cursor);
        return end == std::string_view::npos ? std::string{}
                                             : std::string{value.substr(cursor, end - cursor)};
    }

    std::string lowercase(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            if (ch == '"') {
                out += "\\\"";
            } else if (ch == '\\') {
                out += "\\\\";
            } else if (ch == '\n') {
                out += "\\n";
            } else {
                out.push_back(ch);
            }
        }
        return out;
    }

    std::int64_t parse_i64(std::string_view raw) const
    {
        std::int64_t value{};
        const auto [end, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        (void)end;
        return ec == std::errc{} ? value : 0;
    }

    std::size_t parse_usize(std::string_view raw) const
    {
        std::size_t value{};
        const auto [end, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        (void)end;
        return ec == std::errc{} ? value : 0;
    }

    std::int64_t parse_amount(std::string_view raw) const
    {
        return parse_i64(raw);
    }

    std::int64_t parse_amount_from_metadata(std::string_view metadata, std::string_view key) const
    {
        const std::string pattern = std::format("\"{}\"", key);
        const auto pos = metadata.find(pattern);
        if (pos == std::string_view::npos) {
            return 0;
        }
        auto cursor = pos + pattern.size();
        while (cursor < metadata.size() &&
               (metadata[cursor] == ':' ||
                std::isspace(static_cast<unsigned char>(metadata[cursor])))) {
            ++cursor;
        }
        return parse_amount(metadata.substr(cursor));
    }

    std::string json_string(std::string_view json, std::string_view key) const
    {
        const std::string pattern = std::format("\"{}\"", key);
        const auto pos = json.find(pattern);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto cursor = pos + pattern.size();
        while (cursor < json.size() &&
               (json[cursor] == ':' || std::isspace(static_cast<unsigned char>(json[cursor])))) {
            ++cursor;
        }
        if (cursor >= json.size() || json[cursor] != '"') {
            return {};
        }
        ++cursor;
        const auto end = json.find('"', cursor);
        return end == std::string_view::npos ? std::string{}
                                             : std::string{json.substr(cursor, end - cursor)};
    }

    std::string json_number(std::string_view json, std::string_view key) const
    {
        const std::string pattern = std::format("\"{}\"", key);
        const auto pos = json.find(pattern);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto cursor = pos + pattern.size();
        while (cursor < json.size() &&
               (json[cursor] == ':' || std::isspace(static_cast<unsigned char>(json[cursor])))) {
            ++cursor;
        }
        const auto end = json.find_first_of(",}", cursor);
        return std::string{
            json.substr(cursor, end == std::string_view::npos ? json.size() - cursor : end - cursor)
        };
    }

    std::int64_t days_between_iso(std::string_view created) const
    {
        const auto number = [](std::string_view text, std::size_t pos, std::size_t len) -> int
        {
            if (pos + len > text.size()) {
                return 0;
            }
            int value{0};
            for (std::size_t i{0}; i < len; ++i) {
                if (!std::isdigit(static_cast<unsigned char>(text[pos + i]))) {
                    return 0;
                }
                value = value * 10 + (text[pos + i] - '0');
            }
            return value;
        };
        const int year = number(created, 0, 4);
        const int month = number(created, 5, 2);
        const int day = number(created, 8, 2);
        if (year < 1'970 || month < 1 || month > 12 || day < 1 || day > 31) {
            return 0;
        }
        const auto days_from_civil = [](int y, unsigned m, unsigned d) -> std::int64_t
        {
            y -= m <= 2;
            const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
            const unsigned yoe = static_cast<unsigned>(y - era * 400);
            const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
            const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            return era * 146'097 + static_cast<std::int64_t>(doe) - 719'468;
        };
        const std::int64_t created_days =
            days_from_civil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
        const auto now = std::chrono::system_clock::now();
        const auto today = std::chrono::floor<std::chrono::days>(now);
        const auto ymd = std::chrono::year_month_day{today};
        const std::int64_t now_days = days_from_civil(
            static_cast<int>(ymd.year()),
            static_cast<unsigned>(ymd.month()),
            static_cast<unsigned>(ymd.day())
        );
        return now_days - created_days;
    }

    Status status_from_text(std::string_view text) const
    {
        const auto value = lowercase(text);
        if (value == "success" || value == "succeeded" || value == "successful" ||
            value == "refunded") {
            return Status::Succeeded;
        }
        if (value == "failure" || value == "failed" || value == "transaction_failure" ||
            value == "transactionfailure") {
            return Status::Failed;
        }
        if (value == "manual_review" || value == "manualreview" || value == "review") {
            return Status::Review;
        }
        return Status::Pending;
    }

    std::string_view status_to_text(Status status) const
    {
        switch (status) {
            case Status::Succeeded:
                return "Success";
            case Status::Failed:
                return "Failure";
            case Status::Review:
                return "ManualReview";
            case Status::Pending:
            default:
                return "Pending";
        }
    }

    std::string canonical_status(std::string_view status) const
    {
        switch (status_from_text(status)) {
            case Status::Succeeded:
                return "Success";
            case Status::Failed:
                return "Failure";
            case Status::Review:
                return "ManualReview";
            default:
                return "Pending";
        }
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::refunds
