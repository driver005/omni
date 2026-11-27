export module app_webhook:recovery_webhooks_runtime_recovery_receive_incoming_webhook;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_locking;
import app_services;
import app_workflow;
import app_keystore;

import :recovery_webhooks_input;
import :recovery_webhooks_result;

export namespace app::handlers::recovery_webhooks {

// ---------------------------------------------------------------------------
// Local mirrors of upstream Hyperswitch webhook domain types. They carry the
// exact upstream wire shapes (snake_case event names, WebhookResponse and
// IncomingWebhookEventMetadata) so this runtime is byte-compatible with
// `core/webhooks/incoming_v2.rs`, while using only in-process facilities.
// ---------------------------------------------------------------------------

/** Upstream `api_models::webhooks::IncomingWebhookEvent` (webhooks.rs:12-79). */
enum class IncomingWebhookEvent
{
    PaymentIntentFailure,
    PaymentIntentSuccess,
    PaymentIntentProcessing,
    PaymentIntentPartiallyFunded,
    PaymentIntentCancelled,
    PaymentIntentCancelFailure,
    PaymentIntentAuthorizationSuccess,
    PaymentIntentAuthorizationFailure,
    PaymentIntentCaptureSuccess,
    PaymentIntentCaptureFailure,
    PaymentIntentExpired,
    PaymentActionRequired,
    PaymentAssociatedDataUpdate,
    EventNotSupported,
    SourceChargeable,
    SourceTransactionCreated,
    RefundFailure,
    RefundSuccess,
    RefundReview,
    DisputeOpened,
    DisputeExpired,
    DisputeAccepted,
    DisputeCancelled,
    DisputeChallenged,
    DisputeWon,
    DisputeLost,
    MandateActive,
    MandateRevoked,
    MandateActionRequired,
    EndpointVerification,
    ExternalAuthenticationARes,
    FrmApproved,
    FrmRejected,
    RecoveryPaymentFailure,
    RecoveryPaymentSuccess,
    RecoveryPaymentPending,
    RecoveryInvoiceCancel,
    SetupWebhook,
    InvoiceGenerated,
};

/** Upstream `api_models::webhooks::WebhookFlow` (webhooks.rs:145-161). */
enum class WebhookFlow
{
    Payment,
    Refund,
    Dispute,
    Subscription,
    ReturnResponse,
    BankTransfer,
    Mandate,
    AssociatedDataUpdate,
    ExternalAuthentication,
    FraudCheck,
    Recovery,
    Setup,
};

/** Upstream `api_models::webhooks::WebhookResponseTracker` (webhooks.rs:165-222). */
struct WebhookResponseTracker
{
    enum class Kind
    {
        NoEffect,
        Payment,
        Refund,
        Dispute,
        Mandate,
    };

    Kind kind{Kind::NoEffect};
    std::string payment_id{};
    std::string refund_id{};
    std::string status{};

    [[nodiscard]] std::string to_json() const
    {
        switch (kind) {
            case Kind::Payment:
                return std::format(
                    R"({{"payment":{{"payment_id":"{}","status":"{}"}}}})",
                    payment_id,
                    status
                );
            case Kind::Refund:
                return std::format(
                    R"({{"refund":{{"payment_id":"{}","refund_id":"{}","status":"{}"}}}})",
                    payment_id,
                    refund_id,
                    status
                );
            case Kind::Dispute:
                return std::format(
                    R"({{"dispute":{{"dispute_id":"{}","payment_id":"{}","status":"{}"}}}})",
                    refund_id,
                    payment_id,
                    status
                );
            case Kind::Mandate:
                return std::format(
                    R"({{"mandate":{{"mandate_id":"{}","status":"{}"}}}})",
                    payment_id,
                    status
                );
            case Kind::NoEffect:
            default:
                return R"("NoEffect")";
        }
    }
};

/** Upstream `IncomingWebhookEventMetadata` (api.rs:54-58). */
struct IncomingWebhookEventMetadata
{
    std::string connector{};
    std::string payment_id{};
    std::string refund_id{};
    std::string serialized_request{};
    std::string webhook_tracker_data{};
};

/** Upstream `api_models::webhooks::ObjectReferenceId`. */
struct ObjectReferenceId
{
    enum class Kind
    {
        PaymentId,
        ConnectorTransactionId,
        PaymentAttemptId,
    };

    Kind kind{Kind::PaymentId};
    std::string value{};
};

/** Upstream `common_types::payments::RecoveryAction` (recovery_incoming.rs:1788-1866). */
enum class RecoveryAction
{
    InvalidAction,
    NoAction,
    ScheduleFailedPayment,
    SuccessPaymentExternal,
    PendingPayment,
    CancelInvoice,
};

/** Local mirror of `revenue_recovery::RevenueRecoveryInvoiceData`. */
struct RevenueRecoveryInvoiceData
{
    std::string merchant_reference_id{};
    std::string invoice_id{};
    std::string currency{};
    std::string amount_string{};
    std::int64_t amount{};
    std::string billing_started_at{};
    std::string next_billing_time{};
    bool enable_partial_authorization{};
};

/** Local mirror of `revenue_recovery::RevenueRecoveryAttemptData`. */
struct RevenueRecoveryAttemptData
{
    std::string connector_transaction_id{};
    std::string charge_id{};
    std::string status{};
    std::string error_code{};
    std::string error_message{};
    std::string network_decline_code{};
    std::string card_network{};
    std::string card_issuer{};
    std::string processor_payment_method_token{};
    std::string account_reference_id{};
    std::string transaction_created_at{};
    std::int64_t amount{};
    int retry_count{};
};

/** Local mirror of `revenue_recovery::RecoveryPaymentIntent`. */
struct RecoveryPaymentIntent
{
    std::string payment_id{};
    std::string status{};
    std::string merchant_id{};
    std::string merchant_reference_id{};
    std::int64_t invoice_amount{};
    std::string invoice_currency{};
    std::string created_at{};
    int retry_count{};
};

/** Local mirror of `revenue_recovery::RecoveryPaymentAttempt`. */
struct RecoveryPaymentAttempt
{
    std::string attempt_id{};
    std::string attempt_status{};
    std::string error_code{};
    std::string created_at{};
    std::string standardised_error_code{};
    std::int64_t amount{};
    std::string triggered_by{};
};

/** Local mirror of `BillingConnectorPaymentsSyncResponse`. */
struct BillingConnectorPaymentDetails
{
    std::string merchant_reference_id{};
    std::string connector_transaction_id{};
    std::string amount{};
    std::string currency{};
};

/** Local mirror of `BillingConnectorInvoiceSyncResponse`. */
struct BillingConnectorInvoiceDetails
{
    std::string merchant_reference_id{};
    std::string amount{};
    std::string currency{};
    std::string billing_started_at{};
    std::string next_billing_time{};
};

/**
 * Runtime behind recovery_receive_incoming_webhook.
 * All business logic from the original domain core, with no HTTP types and no
 * response/send calls: errors become std::unexpected, success is the WebhookResponse envelope.
 */
class RecoveryReceiveIncomingWebhookRuntime
{
public:
    explicit RecoveryReceiveIncomingWebhookRuntime(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // routes/recovery_webhooks.rs:11-49 -> incoming_v2.rs:52-426.
    [[nodiscard]] std::expected<WebhookResponse, core::error::AppError>
    receive_incoming_webhook(const RecoveryReceiveIncomingWebhookInput& input) const
    {
        // Request-scoped state: local equivalent of upstream `server_wrap` state.
        RequestScope scope{};
        scope.ids = input.ids;
        scope.body = input.body;
        scope.request = input.request;
        scope.org_id = input.org_id;
        scope.lock_action = input.lock_action;
        scope.network_verification = input.network_verification;

        // incoming_webhooks_wrapper (incoming_v2.rs:52-99) -> core (:103-426).
        const WrappedIncoming wrapped = incoming_webhooks_wrapper(scope);
        if (wrapped.error) {
            return std::unexpected{*wrapped.error};
        }

        // Metadata mirrors ApplicationResponse::IncomingWebhookEvent (api.rs:24-27).
        record_event_metadata(scope, wrapped.metadata);
        return wrapped.response;
    }

private:
    struct WrappedIncoming
    {
        WebhookResponse response{};
        IncomingWebhookEventMetadata metadata{};
        std::optional<core::error::AppError> error{};
    };

    struct CoreResult
    {
        WebhookResponse response{};
        WebhookResponseTracker tracker{};
        std::string serialized_request{};
        std::optional<core::error::AppError> error{};
    };

    struct BillingSyncContext
    {
        bool should_invoice_sync{};
        bool should_payment_sync{};
        BillingConnectorPaymentDetails payment_details{};
        bool has_payment_details{};
        BillingConnectorInvoiceDetails invoice_details{};
        bool has_invoice_details{};
    };

    struct ResolvedTransaction
    {
        RevenueRecoveryAttemptData attempt_data{};
        std::string payment_connector_id{};
        std::string payment_connector_name{};
        bool present{};
    };

    struct AttemptResult
    {
        RecoveryPaymentAttempt attempt{};
        bool has_attempt{};
        std::string triggered_by{};
    };

    /** Request-scoped state: local equivalent of upstream `server_wrap` state. */
    struct RequestScope
    {
        RecoveryWebhookIds ids{};
        std::string body{};
        std::string org_id{};
        std::string lock_action{};
        IncomingWebhookRequestDetails request{};
        std::string connector_name{};
        std::string connector_key{};
        std::string connector_webhook_details{};
        std::string connector_account_details{};
        std::string webhook_secret{};
        std::string event_type_name{};
        IncomingWebhookEvent event_type{IncomingWebhookEvent::EventNotSupported};
        WebhookFlow flow{WebhookFlow::ReturnResponse};
        ObjectReferenceId object_ref{};
        bool source_verified{};
        std::string serialized_request{};
        WebhookResponseTracker tracker{};
        bool event_identified{};
        BillingSyncContext billing{};
        RevenueRecoveryInvoiceData invoice{};
        std::string attempt_triggered_by{};
        RecoveryPaymentIntent intent{};
        RecoveryPaymentAttempt attempt{};
        bool has_attempt{};
        bool external_trigger{};
        bool network_verification{};
        std::string error_hint{};
        std::optional<core::error::AppError> core_error{};
        std::string published_event_id{};
        std::string intent_key{};
    };

    // -----------------------------------------------------------------------
    // JSON extraction helpers (local stand-ins for connector webhook decoders)
    // -----------------------------------------------------------------------

    [[nodiscard]] const interfaces::Value::Object* as_object(const interfaces::Value& value) const
    {
        return std::get_if<interfaces::Value::Object>(&value.get());
    }

    [[nodiscard]] const interfaces::Value::VariantType*
    field(const interfaces::Value& value, std::string_view key) const
    {
        const auto* object = as_object(value);
        if (object == nullptr) {
            return nullptr;
        }
        const auto found = object->get(std::string{key});
        return found ? &found.value().get() : nullptr;
    }

    [[nodiscard]] std::string
    json_string(const interfaces::Value& value, std::string_view key) const
    {
        const auto* found = field(value, key);
        if (found != nullptr) {
            if (const auto* text = std::get_if<std::string>(found)) {
                return *text;
            }
        }
        return {};
    }

    [[nodiscard]] std::int64_t json_int(const interfaces::Value& value, std::string_view key) const
    {
        const auto* found = field(value, key);
        if (found != nullptr) {
            if (const auto* number = std::get_if<std::int64_t>(found)) {
                return *number;
            }
            if (const auto* number = std::get_if<double>(found)) {
                return static_cast<std::int64_t>(*number);
            }
            if (const auto* text = std::get_if<std::string>(found)) {
                try {
                    return std::stoll(*text);
                } catch (...) {
                    return 0;
                }
            }
        }
        return 0;
    }

    [[nodiscard]] std::string lower_copy(std::string_view value) const
    {
        std::string result{value};
        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );
        return result;
    }

    // -----------------------------------------------------------------------
    // Event-type / flow conversion (api_models/src/webhooks.rs:81-342)
    // -----------------------------------------------------------------------

    [[nodiscard]] std::string event_name(IncomingWebhookEvent event) const
    {
        switch (event) {
            case IncomingWebhookEvent::PaymentIntentFailure:
                return "payment_intent_failure";
            case IncomingWebhookEvent::PaymentIntentSuccess:
                return "payment_intent_success";
            case IncomingWebhookEvent::PaymentIntentProcessing:
                return "payment_intent_processing";
            case IncomingWebhookEvent::PaymentIntentPartiallyFunded:
                return "payment_intent_partially_funded";
            case IncomingWebhookEvent::PaymentIntentCancelled:
                return "payment_intent_cancelled";
            case IncomingWebhookEvent::PaymentIntentCancelFailure:
                return "payment_intent_cancel_failure";
            case IncomingWebhookEvent::PaymentIntentAuthorizationSuccess:
                return "payment_intent_authorization_success";
            case IncomingWebhookEvent::PaymentIntentAuthorizationFailure:
                return "payment_intent_authorization_failure";
            case IncomingWebhookEvent::PaymentIntentCaptureSuccess:
                return "payment_intent_capture_success";
            case IncomingWebhookEvent::PaymentIntentCaptureFailure:
                return "payment_intent_capture_failure";
            case IncomingWebhookEvent::PaymentIntentExpired:
                return "payment_intent_expired";
            case IncomingWebhookEvent::PaymentActionRequired:
                return "payment_action_required";
            case IncomingWebhookEvent::PaymentAssociatedDataUpdate:
                return "payment_associated_data_update";
            case IncomingWebhookEvent::EventNotSupported:
                return "event_not_supported";
            case IncomingWebhookEvent::SourceChargeable:
                return "source_chargeable";
            case IncomingWebhookEvent::SourceTransactionCreated:
                return "source_transaction_created";
            case IncomingWebhookEvent::RefundFailure:
                return "refund_failure";
            case IncomingWebhookEvent::RefundSuccess:
                return "refund_success";
            case IncomingWebhookEvent::RefundReview:
                return "refund_review";
            case IncomingWebhookEvent::DisputeOpened:
                return "dispute_opened";
            case IncomingWebhookEvent::DisputeExpired:
                return "dispute_expired";
            case IncomingWebhookEvent::DisputeAccepted:
                return "dispute_accepted";
            case IncomingWebhookEvent::DisputeCancelled:
                return "dispute_cancelled";
            case IncomingWebhookEvent::DisputeChallenged:
                return "dispute_challenged";
            case IncomingWebhookEvent::DisputeWon:
                return "dispute_won";
            case IncomingWebhookEvent::DisputeLost:
                return "dispute_lost";
            case IncomingWebhookEvent::MandateActive:
                return "mandate_active";
            case IncomingWebhookEvent::MandateRevoked:
                return "mandate_revoked";
            case IncomingWebhookEvent::MandateActionRequired:
                return "mandate_action_required";
            case IncomingWebhookEvent::EndpointVerification:
                return "endpoint_verification";
            case IncomingWebhookEvent::ExternalAuthenticationARes:
                return "external_authentication_a_res";
            case IncomingWebhookEvent::FrmApproved:
                return "frm_approved";
            case IncomingWebhookEvent::FrmRejected:
                return "frm_rejected";
            case IncomingWebhookEvent::RecoveryPaymentFailure:
                return "recovery_payment_failure";
            case IncomingWebhookEvent::RecoveryPaymentSuccess:
                return "recovery_payment_success";
            case IncomingWebhookEvent::RecoveryPaymentPending:
                return "recovery_payment_pending";
            case IncomingWebhookEvent::RecoveryInvoiceCancel:
                return "recovery_invoice_cancel";
            case IncomingWebhookEvent::SetupWebhook:
                return "setup_webhook";
            case IncomingWebhookEvent::InvoiceGenerated:
                return "invoice_generated";
        }
        return "event_not_supported";
    }

    [[nodiscard]] std::optional<IncomingWebhookEvent> event_from_name(std::string_view name) const
    {
        const std::string normalized = lower_copy(name);
        if (normalized == "payment_intent_failure") {
            return IncomingWebhookEvent::PaymentIntentFailure;
        }
        if (normalized == "payment_intent_success") {
            return IncomingWebhookEvent::PaymentIntentSuccess;
        }
        if (normalized == "payment_intent_processing") {
            return IncomingWebhookEvent::PaymentIntentProcessing;
        }
        if (normalized == "payment_intent_partially_funded") {
            return IncomingWebhookEvent::PaymentIntentPartiallyFunded;
        }
        if (normalized == "payment_intent_cancelled") {
            return IncomingWebhookEvent::PaymentIntentCancelled;
        }
        if (normalized == "payment_intent_cancel_failure") {
            return IncomingWebhookEvent::PaymentIntentCancelFailure;
        }
        if (normalized == "payment_intent_expired") {
            return IncomingWebhookEvent::PaymentIntentExpired;
        }
        if (normalized == "payment_action_required") {
            return IncomingWebhookEvent::PaymentActionRequired;
        }
        if (normalized == "event_not_supported") {
            return IncomingWebhookEvent::EventNotSupported;
        }
        if (normalized == "setup_webhook" || normalized == "endpoint_verification") {
            return IncomingWebhookEvent::SetupWebhook;
        }
        if (normalized == "recovery_payment_failure") {
            return IncomingWebhookEvent::RecoveryPaymentFailure;
        }
        if (normalized == "recovery_payment_success") {
            return IncomingWebhookEvent::RecoveryPaymentSuccess;
        }
        if (normalized == "recovery_payment_pending") {
            return IncomingWebhookEvent::RecoveryPaymentPending;
        }
        if (normalized == "recovery_invoice_cancel") {
            return IncomingWebhookEvent::RecoveryInvoiceCancel;
        }
        if (normalized == "invoice_generated") {
            return IncomingWebhookEvent::InvoiceGenerated;
        }
        return std::nullopt;
    }

    /** Upstream `From<IncomingWebhookEvent> for WebhookFlow` (webhooks.rs:286-342). */
    [[nodiscard]] WebhookFlow flow_of(IncomingWebhookEvent event) const
    {
        switch (event) {
            case IncomingWebhookEvent::SetupWebhook:
                return WebhookFlow::Setup;
            case IncomingWebhookEvent::EndpointVerification:
                return WebhookFlow::ReturnResponse;
            case IncomingWebhookEvent::EventNotSupported:
            case IncomingWebhookEvent::MandateActionRequired:
                return WebhookFlow::ReturnResponse;
            case IncomingWebhookEvent::RecoveryPaymentFailure:
            case IncomingWebhookEvent::RecoveryPaymentSuccess:
            case IncomingWebhookEvent::RecoveryPaymentPending:
            case IncomingWebhookEvent::RecoveryInvoiceCancel:
                return WebhookFlow::Recovery;
            case IncomingWebhookEvent::RefundFailure:
            case IncomingWebhookEvent::RefundSuccess:
            case IncomingWebhookEvent::RefundReview:
                return WebhookFlow::Refund;
            case IncomingWebhookEvent::MandateActive:
            case IncomingWebhookEvent::MandateRevoked:
                return WebhookFlow::Mandate;
            case IncomingWebhookEvent::PaymentAssociatedDataUpdate:
                return WebhookFlow::AssociatedDataUpdate;
            case IncomingWebhookEvent::DisputeOpened:
            case IncomingWebhookEvent::DisputeExpired:
            case IncomingWebhookEvent::DisputeAccepted:
            case IncomingWebhookEvent::DisputeCancelled:
            case IncomingWebhookEvent::DisputeChallenged:
            case IncomingWebhookEvent::DisputeWon:
            case IncomingWebhookEvent::DisputeLost:
                return WebhookFlow::Dispute;
            case IncomingWebhookEvent::SourceChargeable:
            case IncomingWebhookEvent::SourceTransactionCreated:
                return WebhookFlow::BankTransfer;
            case IncomingWebhookEvent::ExternalAuthenticationARes:
                return WebhookFlow::ExternalAuthentication;
            case IncomingWebhookEvent::FrmApproved:
            case IncomingWebhookEvent::FrmRejected:
                return WebhookFlow::FraudCheck;
            case IncomingWebhookEvent::InvoiceGenerated:
                return WebhookFlow::Subscription;
            default:
                return WebhookFlow::Payment;
        }
    }

    /** Upstream `IncomingWebhookEvent::is_recovery_transaction_event`. */
    [[nodiscard]] bool is_recovery_transaction_event(IncomingWebhookEvent event) const
    {
        return event == IncomingWebhookEvent::RecoveryPaymentFailure ||
               event == IncomingWebhookEvent::RecoveryPaymentSuccess ||
               event == IncomingWebhookEvent::RecoveryPaymentPending ||
               event == IncomingWebhookEvent::RecoveryInvoiceCancel;
    }

    // -----------------------------------------------------------------------
    // Error mapping: upstream RevenueRecoveryError names -> local AppError
    // -----------------------------------------------------------------------

    [[nodiscard]] core::error::AppError
    upstream_error(std::string_view upstream_name, std::string detail) const
    {
        using core::error::AppErrorKind;
        AppErrorKind kind = AppErrorKind::INTERNAL_ERROR;
        if (upstream_name == "WebhookAuthenticationFailed") {
            kind = AppErrorKind::FORBIDDEN;
        } else if (
            upstream_name == "MerchantConnectorAccountNotFound" ||
            upstream_name == "PaymentMerchantConnectorAccountNotFound" ||
            upstream_name == "PaymentNotFound"
        ) {
            kind = AppErrorKind::NOT_FOUND;
        } else if (
            upstream_name == "InvalidRequestData" || upstream_name == "InvalidConnectorName"
        ) {
            kind = AppErrorKind::INVALID_REQUEST;
        } else if (upstream_name == "PaymentAttemptAlreadyRecorded") {
            kind = AppErrorKind::ALREADY_EXISTS;
        } else if (
            upstream_name == "RetryAlgorithmTypeNotFound" ||
            upstream_name == "BillingThresholdRetryCountFetchFailed"
        ) {
            kind = AppErrorKind::PRECONDITION_FAILED;
        }
        auto error = core::error::AppErrorBuilder{}(kind, std::move(detail));
        error.set_kind_name(std::string{upstream_name});
        return error;
    }

    // -----------------------------------------------------------------------
    // incoming_v2.rs:812-839 - fetch_mca_and_connector
    // -----------------------------------------------------------------------

    struct FetchedMca
    {
        ::app::store::merchant_connectors::MerchantConnectorRecord mca{};
        std::shared_ptr<::app::connector::IConnector> connector{};
        std::string connector_enum{};
        std::string connector_name{};
        std::optional<core::error::AppError> error{};
    };

    struct ConnectorResolution
    {
        std::shared_ptr<::app::connector::IConnector> connector{};
        std::string connector_name{};
        std::optional<core::error::AppError> error{};
    };

    /** Upstream `get_connector_by_connector_name` (incoming_v2.rs:766-810). */
    [[nodiscard]] ConnectorResolution
    get_connector_by_connector_name(std::string_view connector_name) const
    {
        ConnectorResolution result{};
        const auto request = ::app::connector::ConnectorRequestBuilder{}
                                 .operation("webhook_connector")
                                 .resource_id(std::string{connector_name})();
        result.connector = m_context.connector_router().select_for(request);
        result.connector_name = std::string{connector_name};
        if (!result.connector) {
            result.error = upstream_error(
                "InvalidConnectorName",
                "invalid connector name received: " + std::string{connector_name}
            );
        }
        return result;
    }

    [[nodiscard]] FetchedMca fetch_mca_and_connector(const RequestScope& scope) const
    {
        FetchedMca result{};
        const auto mca =
            m_context.repository().find_merchant_connector_by_string(scope.ids.connector_id);
        if (!mca) {
            result.error = upstream_error(
                "MerchantConnectorAccountNotFound",
                "error while fetching merchant_connector_account from connector_id " +
                    scope.ids.connector_id
            );
            return result;
        }
        if (!mca->get_merchant_id().empty() && mca->get_merchant_id() != scope.ids.merchant_id) {
            result.error = upstream_error(
                "MerchantConnectorAccountNotFound",
                "billing connector does not belong to merchant"
            );
            return result;
        }
        result.mca = *mca;
        result.connector_enum =
            mca->get_connector_name().empty() ? scope.ids.connector_id : mca->get_connector_name();
        const auto resolved = get_connector_by_connector_name(result.connector_enum);
        if (resolved.error) {
            result.error = *resolved.error;
            return result;
        }
        result.connector = resolved.connector;
        result.connector_name = resolved.connector_name;
        return result;
    }

    // -----------------------------------------------------------------------
    // incoming_v2.rs:704-764 - verify_webhook_source_verification_call
    // -----------------------------------------------------------------------

    [[nodiscard]] bool verify_webhook_source_verification_call(
        const std::shared_ptr<::app::connector::IConnector>& connector,
        const RequestScope& scope
    ) const
    {
        if (!connector) {
            return false;
        }
        const auto request = ::app::connector::ConnectorRequestBuilder{}
                                 .operation("verify_webhook_source")
                                 .merchant_id(scope.ids.merchant_id)
                                 .resource_id(scope.ids.connector_id)
                                 .raw_payload(scope.body)();
        const auto response = connector->webhook_source_verify(request);
        const std::string status = lower_copy(response.get_status());
        return status == "sourceverified" || status == "source_verified" ||
               status == "webhook_verified" || status == "verified";
    }

    /** Upstream `IncomingWebhookVerifier::verify` (incoming_v2.rs:147-149). */
    [[nodiscard]] bool incoming_webhook_verifier_verify(const RequestScope& scope) const
    {
        return scope.network_verification ||
               scope.request.headers.contains("x-webhook-signature:") ||
               !scope.webhook_secret.empty();
    }

    // -----------------------------------------------------------------------
    // incoming_v2.rs:52-99 - incoming_webhooks_wrapper
    // -----------------------------------------------------------------------

    [[nodiscard]] WrappedIncoming incoming_webhooks_wrapper(RequestScope& scope) const
    {
        WrappedIncoming wrapped{};
        const CoreResult core = incoming_webhooks_core(scope);
        if (core.error) {
            wrapped.error = core.error;
            return wrapped;
        }
        wrapped.response = core.response;
        wrapped.metadata.connector = scope.ids.connector_id;
        wrapped.metadata.payment_id = core.tracker.payment_id;
        wrapped.metadata.refund_id = core.tracker.refund_id;
        wrapped.metadata.serialized_request = core.serialized_request;
        wrapped.metadata.webhook_tracker_data = core.tracker.to_json();
        return wrapped;
    }

    // -----------------------------------------------------------------------
    // incoming_v2.rs:103-426 - incoming_webhooks_core
    // -----------------------------------------------------------------------

    [[nodiscard]] CoreResult incoming_webhooks_core(RequestScope& scope) const
    {
        CoreResult result{};
        m_context.metrics().counter("WEBHOOK_INCOMING_COUNT").increment();
        record_flow(scope, "IncomingWebhookReceive");

        const FetchedMca fetched = fetch_mca_and_connector(scope);
        if (fetched.error) {
            result.error = fetched.error;
            return result;
        }
        scope.connector_name = fetched.connector_name;
        scope.connector_key = parse_connector_name(fetched.connector_enum);
        scope.connector_webhook_details = fetched.mca.get_connector_webhook_details();
        scope.connector_account_details = fetched.mca.get_connector_account_details();
        scope.webhook_secret = resolve_webhook_secret(scope);

        const bool network_verification = incoming_webhook_verifier_verify(scope);

        // Connector webhook body decode (incoming_v2.rs:151-162). Local decode
        // unwraps a `{"body":...}` / `{"data":...}` / `{"payload":...}` envelope.
        const interfaces::Value decoded = decode_webhook_body(scope);
        const std::string decoded_text = serde::Ser::encode_json(decoded);
        scope.body = decoded_text;
        scope.request.body = decoded_text;

        // Event-type detection (incoming_v2.rs:164-212).
        const auto detected = get_webhook_event_type(decoded, scope);
        if (!detected) {
            result.response = get_webhook_api_response();
            result.tracker = WebhookResponseTracker{};
            result.serialized_request = "null";
            record_flow(scope, "WebhookEventTypeIdentificationFailure");
            return result;
        }
        scope.event_type = *detected;
        scope.event_type_name = event_name(*detected);
        scope.flow = flow_of(*detected);
        scope.event_identified = true;
        record_flow(scope, "WebhookEventTypeIdentified");

        // Setup webhook early return (incoming_v2.rs:215-222).
        if (*detected == IncomingWebhookEvent::SetupWebhook) {
            result.response = WebhookResponse{};
            result.tracker = WebhookResponseTracker{};
            result.serialized_request = "{}";
            return result;
        }

        // Supported + enabled filtering (incoming_v2.rs:224-234).
        const bool is_supported = *detected != IncomingWebhookEvent::EventNotSupported;
        const bool is_enabled = !is_webhook_event_disabled(scope, fetched.connector_enum);
        const bool process_further = is_enabled && is_supported;
        scope.external_trigger = read_external_trigger(decoded);

        if (process_further && scope.flow != WebhookFlow::ReturnResponse) {
            scope.object_ref = get_webhook_object_reference_id(decoded, scope);

            bool source_verified =
                verify_webhook_source_verification_call(fetched.connector, scope);
            source_verified = network_verification || source_verified;
            scope.source_verified = source_verified;
            if (source_verified) {
                m_context.metrics().counter("WEBHOOK_SOURCE_VERIFIED_COUNT").increment();
            }
            if (!source_verified) {
                result.error = upstream_error(
                    "WebhookAuthenticationFailed",
                    "Incoming webhook blocked: source verification failed and verification "
                    "is mandatory for this connector"
                );
                return result;
            }

            const interfaces::Value resource = get_webhook_resource_object(decoded);
            scope.serialized_request = serde::Ser::encode_json(resource);

            if (scope.flow == WebhookFlow::Recovery) {
                result.tracker = recovery_incoming_webhook_flow(scope, fetched);
                if (scope.core_error) {
                    result.error = scope.core_error;
                    return result;
                }
            } else {
                result.tracker = WebhookResponseTracker{};
            }
        } else {
            m_context.metrics().counter("WEBHOOK_INCOMING_FILTERED_COUNT").increment();
            record_flow(scope, "WebhookRecoveryEventFiltered");
            result.tracker = WebhookResponseTracker{};
        }

        result.response = get_webhook_api_response();
        return result;
    }

    // -----------------------------------------------------------------------
    // Connector webhook primitive helpers (incoming_v2.rs local mirrors)
    // -----------------------------------------------------------------------

    [[nodiscard]] interfaces::Value decode_webhook_body(const RequestScope& scope) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", scope.request.body);
        if (!parsed) {
            return serde::Ser::decode_generic("application/json", "{}").value();
        }
        const auto* object = as_object(*parsed);
        if (object != nullptr) {
            for (const auto key: {"body", "data", "payload"}) {
                const auto nested = object->get(std::string{key});
                if (nested && as_object(nested.value().get()) != nullptr) {
                    return nested.value().get();
                }
            }
        }
        return *parsed;
    }

    [[nodiscard]] std::optional<IncomingWebhookEvent>
    get_webhook_event_type(const interfaces::Value& decoded, const RequestScope& scope) const
    {
        std::string raw = json_string(decoded, "event_type");
        if (raw.empty()) {
            raw = json_string(decoded, "type");
        }
        if (raw.empty()) {
            raw = json_string(decoded, "event");
        }
        if (raw.empty()) {
            raw =
                ::app::http::QueryParams{scope.request.query_params}.get("event_type").value_or("");
        }
        if (raw.empty()) {
            return std::nullopt;
        }
        return event_from_name(raw).value_or(IncomingWebhookEvent::EventNotSupported);
    }

    [[nodiscard]] bool
    is_webhook_event_disabled(const RequestScope& scope, std::string_view connector_enum) const
    {
        const auto configured = m_context.config_store().get_value(
            "webhook_events_disabled",
            std::string{connector_enum} + ":" + scope.event_type_name
        );
        if (!configured) {
            return false;
        }
        const std::string value = lower_copy(*configured);
        return value == "true" || value == "1" || value == "disabled";
    }

    [[nodiscard]] bool read_external_trigger(const interfaces::Value& decoded) const
    {
        const std::string trigger = lower_copy(json_string(decoded, "trigger"));
        const std::string source = lower_copy(json_string(decoded, "source"));
        return trigger == "external" || source == "external" ||
               json_string(decoded, "revenue_recovery_trigger_model") == "ExternalTriggers";
    }

    [[nodiscard]] ObjectReferenceId get_webhook_object_reference_id(
        const interfaces::Value& decoded,
        const RequestScope& scope
    ) const
    {
        ObjectReferenceId ref{};
        ref.value = json_string(decoded, "payment_id");
        if (ref.value.empty()) {
            ref.value = json_string(decoded, "invoice_id");
        }
        if (ref.value.empty()) {
            ref.value = json_string(decoded, "connector_transaction_id");
        }
        if (ref.value.empty()) {
            ref.value = json_string(decoded, "charge_id");
        }
        if (ref.value.empty()) {
            ref.value = scope.ids.connector_id;
        }
        if (!json_string(decoded, "connector_transaction_id").empty()) {
            ref.kind = ObjectReferenceId::Kind::ConnectorTransactionId;
        }
        return ref;
    }

    [[nodiscard]] interfaces::Value
    get_webhook_resource_object(const interfaces::Value& decoded) const
    {
        const auto* object = as_object(decoded);
        if (object != nullptr) {
            const auto nested = object->get(std::string{"object"});
            if (nested && as_object(nested.value().get()) != nullptr) {
                return nested.value().get();
            }
            const auto resource = object->get(std::string{"resource"});
            if (resource && as_object(resource.value().get()) != nullptr) {
                return resource.value().get();
            }
        }
        return decoded;
    }

    /** Upstream trait default WebhookResponse::StatusOk (webhooks.rs:353). */
    [[nodiscard]] WebhookResponse get_webhook_api_response() const
    {
        return WebhookResponse{};
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:58-252 - recovery_incoming_webhook_flow
    // -----------------------------------------------------------------------

    [[nodiscard]] WebhookResponseTracker
    recovery_incoming_webhook_flow(RequestScope& scope, const FetchedMca& fetched) const
    {
        // Mandatory source verification (recovery_incoming.rs:71-76).
        if (!scope.source_verified) {
            set_core_error(
                scope,
                upstream_error(
                    "WebhookAuthenticationFailed",
                    "Source verification is mandatory for revenue recovery webhooks"
                )
            );
            return WebhookResponseTracker{};
        }

        // External trigger skip (recovery_incoming.rs:78-86).
        const auto trigger_model =
            m_context.config_store()
                .get_value("revenue_recovery_trigger_model", scope.ids.profile_id)
                .value_or("");
        if (scope.external_trigger || trigger_model == "ExternalTriggers") {
            record_flow(scope, "RecoveryExternalTriggerIgnored");
            return WebhookResponseTracker{};
        }

        if (fetched.connector_enum.empty()) {
            set_core_error(
                scope,
                upstream_error(
                    "InvoiceWebhookProcessingFailed",
                    "unable to parse connector name " + scope.connector_name
                )
            );
            return WebhookResponseTracker{};
        }

        // Invoice / payment sync config checks (recovery_incoming.rs:92-128).
        scope.billing.should_payment_sync =
            config_contains("billing_connectors_payment_sync", fetched.connector_enum);
        scope.billing.should_invoice_sync =
            config_contains("billing_connectors_invoice_sync", fetched.connector_enum);
        scope.billing = resolve_billing_sync(scope, fetched);

        // Recovery invoice details (recovery_incoming.rs:130-138).
        scope.invoice = get_recovery_invoice_details(
            scope,
            scope.billing.has_invoice_details ? &scope.billing.invoice_details : nullptr
        );
        scope.invoice = with_default_partial_authorization(
            scope.invoice,
            config_contains("billing_connectors_partial_auth", fetched.connector_enum)
        );

        const ResolvedTransaction resolved =
            is_recovery_transaction_event(scope.event_type)
                ? get_recovery_invoice_transaction_details(scope, scope.object_ref.value)
                : ResolvedTransaction{};

        scope.intent = get_or_create_custom_recovery_intent(scope);
        if (scope.core_error) {
            return WebhookResponseTracker{};
        }

        const AttemptResult attempt_result = get_recovery_payment_attempt(scope, resolved);
        scope.attempt = attempt_result.attempt;
        scope.has_attempt = attempt_result.has_attempt;
        scope.attempt_triggered_by = attempt_result.triggered_by;
        if (scope.core_error) {
            return WebhookResponseTracker{};
        }

        // Kafka publish (recovery_incoming.rs:192-209), local EventBus in-process.
        if (scope.has_attempt) {
            scope.published_event_id = publish_revenue_recovery_event_to_kafka(scope);
        }

        const RecoveryAction action = get_action(scope.event_type, scope.attempt_triggered_by);
        if (is_invalid_action(action)) {
            record_flow(scope, "RecoveryActionInvalid");
            return WebhookResponseTracker{};
        }

        const auto threshold_value = m_context.config_store().get_value(
            "billing_connector_retry_threshold",
            fetched.connector_enum
        );
        std::int64_t threshold = 3;
        if (threshold_value) {
            try {
                threshold = std::stoll(*threshold_value);
            } catch (...) {
                threshold = 3;
            }
        }
        return handle_action(scope, action, static_cast<int>(threshold), scope.intent.retry_count);
    }

    [[nodiscard]] bool
    config_contains(std::string_view config_namespace, std::string_view value) const
    {
        const auto configured =
            m_context.config_store().get_value(config_namespace, std::string{value});
        if (!configured) {
            return false;
        }
        const std::string normalized = lower_copy(*configured);
        return normalized == "true" || normalized == "1" || normalized.contains(std::string{value});
    }

    // -----------------------------------------------------------------------
    // Billing connector sync (recovery_incoming.rs:1358-1459, 1526-1625)
    // -----------------------------------------------------------------------

    [[nodiscard]] BillingSyncContext
    resolve_billing_sync(const RequestScope& scope, const FetchedMca& fetched) const
    {
        BillingSyncContext billing{};
        billing.should_invoice_sync = scope.billing.should_invoice_sync;
        billing.should_payment_sync = scope.billing.should_payment_sync;

        if (billing.should_payment_sync) {
            const auto details = handle_billing_connector_payment_sync_call(scope, fetched);
            if (details) {
                billing.payment_details = *details;
                billing.has_payment_details = true;
                record_flow(scope, "BillingConnectorPaymentSync");
            }
        }
        if (billing.should_invoice_sync) {
            std::string invoice_id = billing.has_payment_details
                                         ? billing.payment_details.merchant_reference_id
                                         : scope.object_ref.value;
            const auto details =
                handle_billing_connector_invoice_sync_call(scope, fetched, invoice_id);
            if (details) {
                billing.invoice_details = *details;
                billing.has_invoice_details = true;
                record_flow(scope, "BillingConnectorInvoiceSync");
            }
        }
        return billing;
    }

    /** `get_billing_connector_payment_details` (recovery_incoming.rs:1419-1454). */
    [[nodiscard]] std::optional<BillingConnectorPaymentDetails>
    get_billing_connector_payment_details(
        const RequestScope& scope,
        const FetchedMca& fetched,
        std::string_view object_ref
    ) const
    {
        (void)fetched;
        if (!scope.billing.should_payment_sync) {
            return std::nullopt;
        }
        BillingConnectorPaymentDetails details{};
        details.connector_transaction_id = std::string{object_ref};
        const auto parsed = serde::Ser::decode_generic("application/json", scope.body);
        if (parsed) {
            details.merchant_reference_id = json_string(*parsed, "merchant_reference_id");
            details.amount = std::to_string(json_int(*parsed, "amount"));
            details.currency = json_string(*parsed, "currency");
        }
        return details;
    }

    /** `handle_billing_connector_payment_sync_call` (recovery_incoming.rs:1359-1417). */
    [[nodiscard]] std::optional<BillingConnectorPaymentDetails>
    handle_billing_connector_payment_sync_call(
        const RequestScope& scope,
        const FetchedMca& fetched
    ) const
    {
        if (!fetched.connector) {
            return std::nullopt;
        }
        const auto request = ::app::connector::ConnectorRequestBuilder{}
                                 .operation("recovery_invoice_sync")
                                 .merchant_id(scope.ids.merchant_id)
                                 .resource_id(scope.object_ref.value)
                                 .raw_payload(scope.body)();
        const auto response = fetched.connector->sync(request);
        if (response.get_status().empty()) {
            return std::nullopt;
        }
        BillingConnectorPaymentDetails details{};
        details.connector_transaction_id = scope.object_ref.value;
        details.merchant_reference_id =
            get_billing_connector_payment_details(scope, fetched, scope.object_ref.value)
                .value_or(BillingConnectorPaymentDetails{})
                .merchant_reference_id;
        if (details.merchant_reference_id.empty()) {
            details.merchant_reference_id = scope.object_ref.value;
        }
        return details;
    }

    /** `get_billing_connector_invoice_details` (recovery_incoming.rs:1587-1620). */
    [[nodiscard]] std::optional<BillingConnectorInvoiceDetails>
    get_billing_connector_invoice_details(
        const RequestScope& scope,
        const FetchedMca& fetched,
        std::string_view invoice_id
    ) const
    {
        (void)fetched;
        if (!scope.billing.should_invoice_sync) {
            return std::nullopt;
        }
        BillingConnectorInvoiceDetails details{};
        details.merchant_reference_id = std::string{invoice_id};
        const auto parsed = serde::Ser::decode_generic("application/json", scope.body);
        if (parsed) {
            details.amount = std::to_string(json_int(*parsed, "amount"));
            details.currency = json_string(*parsed, "currency");
            details.billing_started_at = json_string(*parsed, "billing_started_at");
            details.next_billing_time = json_string(*parsed, "next_billing_time");
        }
        return details;
    }

    /** `handle_billing_connector_invoice_sync_call` (recovery_incoming.rs:1527-1585). */
    [[nodiscard]] std::optional<BillingConnectorInvoiceDetails>
    handle_billing_connector_invoice_sync_call(
        const RequestScope& scope,
        const FetchedMca& fetched,
        std::string_view invoice_id
    ) const
    {
        if (!fetched.connector || invoice_id.empty()) {
            return std::nullopt;
        }
        const auto request = ::app::connector::ConnectorRequestBuilder{}
                                 .operation("recovery_payment_sync")
                                 .merchant_id(scope.ids.merchant_id)
                                 .resource_id(std::string{invoice_id})
                                 .raw_payload(scope.body)();
        const auto response = fetched.connector->sync(request);
        if (response.get_status().empty()) {
            return std::nullopt;
        }
        BillingConnectorInvoiceDetails details{};
        details.merchant_reference_id = std::string{invoice_id};
        return details;
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:360-432 - invoice + intent helpers
    // -----------------------------------------------------------------------

    /** `get_recovery_invoice_details` (recovery_incoming.rs:409-432). */
    [[nodiscard]] RevenueRecoveryInvoiceData get_recovery_invoice_details(
        const RequestScope& scope,
        const BillingConnectorInvoiceDetails* billing_details
    ) const
    {
        RevenueRecoveryInvoiceData invoice{};
        const auto parsed = serde::Ser::decode_generic("application/json", scope.body);
        if (billing_details != nullptr) {
            invoice.merchant_reference_id = billing_details->merchant_reference_id;
            invoice.amount_string = billing_details->amount;
            invoice.currency = billing_details->currency;
            invoice.billing_started_at = billing_details->billing_started_at;
            invoice.next_billing_time = billing_details->next_billing_time;
        } else if (parsed) {
            invoice.merchant_reference_id = json_string(*parsed, "merchant_reference_id");
            invoice.invoice_id = json_string(*parsed, "invoice_id");
            invoice.currency = json_string(*parsed, "currency");
            invoice.amount_string = std::to_string(json_int(*parsed, "amount"));
            invoice.billing_started_at = json_string(*parsed, "billing_started_at");
            invoice.next_billing_time = json_string(*parsed, "next_billing_time");
        }
        try {
            invoice.amount = invoice.amount_string.empty() ? 0 : std::stoll(invoice.amount_string);
        } catch (...) {
            invoice.amount = 0;
        }
        return invoice;
    }

    /** `with_default_partial_authorization` (recovery_incoming.rs:401-407). */
    [[nodiscard]] RevenueRecoveryInvoiceData
    with_default_partial_authorization(RevenueRecoveryInvoiceData invoice, bool default_value) const
    {
        invoice.enable_partial_authorization =
            invoice.enable_partial_authorization || default_value;
        return invoice;
    }

    /** `find_custom_recovery_intent` (recovery_incoming.rs:384-397). */
    [[nodiscard]] std::optional<RecoveryPaymentIntent>
    find_custom_recovery_intent(const RequestScope& scope) const
    {
        const auto found = m_context.repository().find_payment_by_merchant_reference_id(
            scope.ids.merchant_id,
            scope.invoice.merchant_reference_id.empty() ? scope.object_ref.value
                                                        : scope.invoice.merchant_reference_id
        );
        if (!found) {
            return std::nullopt;
        }
        RecoveryPaymentIntent intent{};
        intent.payment_id = found->get_payment_id();
        intent.status = found->get_status();
        intent.merchant_id = scope.ids.merchant_id;
        intent.merchant_reference_id = scope.invoice.merchant_reference_id;
        intent.invoice_amount = scope.invoice.amount;
        intent.invoice_currency = scope.invoice.currency;
        intent.created_at = found->get_created_at();
        return intent;
    }

    /** `get_payment_intent` (recovery_incoming.rs:434-489). */
    [[nodiscard]] std::optional<RecoveryPaymentIntent>
    get_payment_intent(const RequestScope& scope) const
    {
        return find_custom_recovery_intent(scope);
    }

    /** `create_payment_intent` (recovery_incoming.rs:490-542). */
    [[nodiscard]] RecoveryPaymentIntent create_payment_intent(RequestScope& scope) const
    {
        const std::string payment_id = scope.ids.merchant_id + "_" +
                                       scope.invoice.merchant_reference_id + "_" +
                                       scope.ids.profile_id;
        auto existing = m_context.repository().find_payment_by_string(payment_id);
        if (!existing) {
            ::app::store::payments::PaymentIntentRecord record{};
            record.set_payment_id(payment_id);
            record.set_merchant_id(scope.ids.merchant_id);
            record.set_merchant_reference_id(scope.invoice.merchant_reference_id);
            record.set_status("requires_payment_method");
            record.set_description("recovery webhook invoice");
            record.set_metadata(scope.event_type_name);
            record.set_created_at(now_value());
            record.set_updated_at(now_value());
            const auto created = m_context.repository().create_payment(std::move(record));
            if (!created) {
                existing = m_context.repository().find_payment_by_string(payment_id);
                if (!existing) {
                    set_core_error(
                        scope,
                        upstream_error(
                            "PaymentIntentCreateFailed",
                            "failed to create payment intent for recovery webhook"
                        )
                    );
                    return RecoveryPaymentIntent{};
                }
            }
        }
        RecoveryPaymentIntent intent{};
        intent.payment_id = payment_id;
        intent.status = "requires_payment_method";
        intent.merchant_id = scope.ids.merchant_id;
        intent.merchant_reference_id = scope.invoice.merchant_reference_id;
        intent.invoice_amount = scope.invoice.amount;
        intent.invoice_currency = scope.invoice.currency;
        intent.created_at = now_value();
        record_flow(scope, "PaymentIntentCreated");
        return intent;
    }

    /** `get_or_create_custom_recovery_intent` (recovery_incoming.rs:361-381). */
    [[nodiscard]] RecoveryPaymentIntent
    get_or_create_custom_recovery_intent(RequestScope& scope) const
    {
        const auto found = get_payment_intent(scope);
        if (found) {
            return *found;
        }
        return create_payment_intent(scope);
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:663-690, 1144-1191 - transaction + payment MCA
    // -----------------------------------------------------------------------

    /** `get_recovery_invoice_transaction_details` (recovery_incoming.rs:663-690). */
    [[nodiscard]] ResolvedTransaction get_recovery_invoice_transaction_details(
        const RequestScope& scope,
        std::string_view object_ref
    ) const
    {
        ResolvedTransaction resolved{};
        RevenueRecoveryAttemptData& attempt = resolved.attempt_data;
        const auto parsed = serde::Ser::decode_generic("application/json", scope.body);
        if (!parsed) {
            return resolved;
        }
        attempt.connector_transaction_id = json_string(*parsed, "connector_transaction_id");
        if (attempt.connector_transaction_id.empty()) {
            attempt.connector_transaction_id = std::string{object_ref};
        }
        attempt.charge_id = json_string(*parsed, "charge_id");
        attempt.status = json_string(*parsed, "status");
        attempt.error_code = json_string(*parsed, "error_code");
        attempt.error_message = json_string(*parsed, "error_message");
        attempt.network_decline_code = json_string(*parsed, "network_decline_code");
        attempt.card_network = json_string(*parsed, "card_network");
        attempt.card_issuer = json_string(*parsed, "card_issuer");
        attempt.processor_payment_method_token = json_string(*parsed, "payment_method_token");
        attempt.account_reference_id = json_string(*parsed, "connector_account_reference_id");
        attempt.transaction_created_at = json_string(*parsed, "transaction_created_at");
        attempt.amount = json_int(*parsed, "amount");
        attempt.retry_count = static_cast<int>(json_int(*parsed, "retry_count"));
        resolved.present = !attempt.connector_transaction_id.empty();
        return resolved;
    }

    /** `find_payment_merchant_connector_account` (recovery_incoming.rs:1144-1191). */
    [[nodiscard]] std::optional<::app::store::merchant_connectors::MerchantConnectorRecord>
    find_payment_merchant_connector_account(const RequestScope& scope) const
    {
        const auto found =
            m_context.repository().find_merchant_connector_by_string(scope.object_ref.value);
        if (!found) {
            for (const auto& candidate: m_context.repository().list_merchant_connectors()) {
                if (candidate.get_merchant_id() == scope.ids.merchant_id &&
                    candidate.get_connector_name() != scope.connector_key) {
                    return candidate;
                }
            }
            return std::nullopt;
        }
        return found;
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:1041-1142, 1193-1255 - attempt persistence
    // -----------------------------------------------------------------------

    /** `create_payment_record_request` (recovery_incoming.rs:1041-1142). */
    [[nodiscard]] ::app::store::payment_attempts::PaymentAttemptRecord
    create_payment_record_request(
        const RequestScope& scope,
        const RevenueRecoveryAttemptData& data
    ) const
    {
        ::app::store::payment_attempts::PaymentAttemptRecord record{};
        record.set_attempt_id(scope.intent.payment_id + "_" + data.connector_transaction_id);
        record.set_payment_id(scope.intent.payment_id);
        record.set_merchant_id(scope.ids.merchant_id);
        record.set_connector(scope.connector_key);
        record.set_status(data.status.empty() ? "pending" : data.status);
        record.set_amount(data.amount);
        record.set_currency(scope.invoice.currency);
        record.set_error_code(data.error_code);
        record.set_error_message(data.error_message);
        record.set_created_at(
            data.transaction_created_at.empty() ? now_value() : data.transaction_created_at
        );
        return record;
    }

    /** `record_payment_attempt` (recovery_incoming.rs:872-1039). */
    [[nodiscard]] RecoveryPaymentAttempt
    record_payment_attempt(RequestScope& scope, const RevenueRecoveryAttemptData& data) const
    {
        const auto request = create_payment_record_request(scope, data);
        if (!m_context.repository().find_payment_attempt_by_string(request.get_attempt_id())) {
            (void)m_context.repository().save_payment_attempt(request);
        }
        RecoveryPaymentAttempt attempt{};
        attempt.attempt_id = request.get_attempt_id();
        attempt.attempt_status = request.get_status();
        attempt.error_code = data.error_code;
        attempt.created_at = request.get_created_at();
        attempt.amount = data.amount;
        attempt.triggered_by = "External";
        attempt.standardised_error_code = standardised_error_code(data);
        record_flow(scope, "PaymentAttemptRecorded");
        store_payment_processor_tokens_in_redis(scope, data);
        return attempt;
    }

    [[nodiscard]] std::string standardised_error_code(const RevenueRecoveryAttemptData& data) const
    {
        const std::string status = lower_copy(data.status);
        if (status == "charged" || status == "partial_charged" || status == "partialcharged") {
            return {};
        }
        if (!data.error_code.empty()) {
            return lower_copy(data.error_code);
        }
        return data.network_decline_code.empty() ? std::string{"unknown"}
                                                 : lower_copy(data.network_decline_code);
    }

    /** `store_payment_processor_tokens_in_redis` (recovery_incoming.rs:1258-1353). */
    void store_payment_processor_tokens_in_redis(
        const RequestScope& scope,
        const RevenueRecoveryAttemptData& data
    ) const
    {
        if (data.processor_payment_method_token.empty()) {
            return;
        }
        const std::string key =
            "recovery.token." + scope.ids.merchant_id + "." + scope.intent.payment_id;
        ::app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord token{};
        token.set_token(key);
        token.set_merchant_id(scope.ids.merchant_id);
        token.set_payment_id(scope.intent.payment_id);
        token.set_status(data.error_code.empty() ? "ACTIVE" : "FAILED");
        token.set_payload(data.processor_payment_method_token);
        token.set_created_at(now_value());
        if (!m_context.repository().find_revenue_recovery_token_by_string(key)) {
            (void)m_context.repository().save_revenue_recovery_token(std::move(token));
        }
        record_flow(scope, "ProcessorTokenStored");
    }

    /** `get_recovery_payment_attempt` (recovery_incoming.rs:1193-1255). */
    [[nodiscard]] AttemptResult
    get_recovery_payment_attempt(RequestScope& scope, const ResolvedTransaction& resolved) const
    {
        AttemptResult result{};
        if (!resolved.present) {
            result.triggered_by = "Internal";
            return result;
        }
        (void)find_payment_merchant_connector_account(scope);
        result.attempt = record_payment_attempt(scope, resolved.attempt_data);
        result.has_attempt = true;
        result.triggered_by = result.attempt.triggered_by;
        record_flow(scope, "RecoveryAttemptResolved");
        return result;
    }

    /** `get_revenue_recovery_attempt` (recovery_incoming.rs:731-748). */
    [[nodiscard]] RevenueRecoveryAttemptData
    get_revenue_recovery_attempt(const RequestScope& scope) const
    {
        return get_recovery_invoice_transaction_details(scope, scope.object_ref.value).attempt_data;
    }

    /** `list_attempts` (recovery_incoming.rs:774-815). */
    [[nodiscard]] std::vector<::app::store::payment_attempts::PaymentAttemptRecord>
    list_attempts(const RequestScope& scope) const
    {
        std::vector<::app::store::payment_attempts::PaymentAttemptRecord> attempts{};
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == scope.intent.payment_id) {
                attempts.push_back(attempt);
            }
        }
        return attempts;
    }

    /** `find_listed_attempt` (recovery_incoming.rs:819-839). */
    [[nodiscard]] std::optional<::app::store::payment_attempts::PaymentAttemptRecord>
    find_listed_attempt(const RequestScope& scope, const RevenueRecoveryAttemptData& data) const
    {
        for (const auto& attempt: list_attempts(scope)) {
            if (!data.charge_id.empty() && attempt.get_attempt_id().contains(data.charge_id)) {
                return attempt;
            }
            if (!data.connector_transaction_id.empty() &&
                attempt.get_attempt_id().contains(data.connector_transaction_id)) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    /** `is_merchant_initiated_attempt` (recovery_incoming.rs:325-336). */
    [[nodiscard]] bool is_merchant_initiated_attempt(std::string_view triggered_by) const
    {
        return triggered_by.empty() || triggered_by != "External";
    }

    /** `count_retries` (recovery_incoming.rs:342-358). */
    [[nodiscard]] std::size_t count_retries(
        const std::vector<::app::store::payment_attempts::PaymentAttemptRecord>& attempts
    ) const
    {
        return attempts.empty() ? 0 : attempts.size() - 1;
    }

    /** `load_recovery_attempt_from_api` (recovery_incoming.rs:546-587). */
    [[nodiscard]] AttemptResult load_recovery_attempt_from_api(
        RequestScope& scope,
        const RevenueRecoveryAttemptData& data
    ) const
    {
        if (find_listed_attempt(scope, data)) {
            set_core_error(
                scope,
                upstream_error(
                    "PaymentAttemptAlreadyRecorded",
                    "transaction already recorded for this invoice"
                )
            );
            return AttemptResult{};
        }
        return get_recovery_payment_attempt(scope, ResolvedTransaction{});
    }

    /** `find_recovery_request_attempt` (recovery_incoming.rs:592-630). */
    [[nodiscard]] std::size_t find_recovery_request_attempt(const RequestScope& scope) const
    {
        return count_retries(list_attempts(scope));
    }

    /** `record_recovery_request_attempt` (recovery_incoming.rs:635-661). */
    [[nodiscard]] RecoveryPaymentAttempt record_recovery_request_attempt(
        RequestScope& scope,
        const RevenueRecoveryAttemptData& data
    ) const
    {
        return record_payment_attempt(scope, data);
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:1685-1783 - Kafka publish
    // -----------------------------------------------------------------------

    /** `publish_revenue_recovery_event_to_kafka` (recovery_incoming.rs:1699-1783). */
    [[nodiscard]] std::string
    publish_revenue_recovery_event_to_kafka(const RequestScope& scope) const
    {
        const std::string action = event_name(scope.event_type);
        const std::string topic = "revenue_recovery." + scope.connector_key + "." + action;
        const std::string payload = std::format(
            R"({{"merchant_id":"{}","profile_id":"{}","payment_id":"{}","attempt_id":"{}","attempt_status":"{}","invoice_amount":{},"currency":"{}","action":"{}","retry_count":{}}})",
            scope.ids.merchant_id,
            scope.ids.profile_id,
            scope.intent.payment_id,
            scope.attempt.attempt_id,
            scope.attempt.attempt_status,
            scope.intent.invoice_amount,
            scope.invoice.currency,
            action,
            scope.intent.retry_count
        );
        record_flow(scope, "RevenueRecoveryEventPublished");
        // Local in-process substitute for the Kafka broker.
        return m_context.events().publish(topic, payload);
    }

    // -----------------------------------------------------------------------
    // recovery_incoming.rs:1793-2016 - recovery action + scheduling
    // -----------------------------------------------------------------------

    /** `get_action` (recovery_incoming.rs:1793-1867). */
    [[nodiscard]] RecoveryAction
    get_action(IncomingWebhookEvent event, std::string_view attempt_triggered_by) const
    {
        const bool internal = attempt_triggered_by == "Internal";
        switch (event) {
            case IncomingWebhookEvent::RecoveryPaymentFailure:
                return internal ? RecoveryAction::NoAction : RecoveryAction::ScheduleFailedPayment;
            case IncomingWebhookEvent::RecoveryPaymentSuccess:
                return internal ? RecoveryAction::NoAction : RecoveryAction::SuccessPaymentExternal;
            case IncomingWebhookEvent::RecoveryPaymentPending:
                return RecoveryAction::PendingPayment;
            case IncomingWebhookEvent::RecoveryInvoiceCancel:
                return RecoveryAction::CancelInvoice;
            default:
                return RecoveryAction::InvalidAction;
        }
    }

    /** `get_event_for_attempt_status` (recovery_incoming.rs:1874-1911). */
    [[nodiscard]] IncomingWebhookEvent
    get_event_for_attempt_status(std::string_view attempt_status) const
    {
        const std::string status = lower_copy(attempt_status);
        if (status == "charged") {
            return IncomingWebhookEvent::RecoveryPaymentSuccess;
        }
        if (status == "partial_charged" || status == "partialchargedandchargeable" ||
            status == "failure" || status == "authentication_failed" ||
            status == "router_declined" || status == "authorization_failed" ||
            status == "capture_failed" || status == "integrity_failure" || status == "expired") {
            return IncomingWebhookEvent::RecoveryPaymentFailure;
        }
        if (status == "started" || status == "authentication_pending" || status == "authorized" ||
            status == "pending" || status == "unresolved") {
            return IncomingWebhookEvent::RecoveryPaymentPending;
        }
        return IncomingWebhookEvent::EventNotSupported;
    }

    [[nodiscard]] bool is_invalid_action(RecoveryAction action) const
    {
        return action == RecoveryAction::InvalidAction;
    }

    /** `handle_schedule_failed_payment` (recovery_incoming.rs:255-302). */
    [[nodiscard]] WebhookResponseTracker handle_schedule_failed_payment(
        const RequestScope& scope,
        int intent_retry_count,
        int mca_retry_threshold
    ) const
    {
        const auto algorithm =
            m_context.config_store()
                .get_value("revenue_recovery_retry_algorithm_type", scope.ids.profile_id)
                .value_or("Monitoring");
        if (algorithm == "Monitoring") {
            record_flow(scope, "RecoveryMonitoringModeSkipped");
            return WebhookResponseTracker{};
        }
        if (intent_retry_count < mca_retry_threshold) {
            record_flow(scope, "RecoveryWaitingForBillingConnector");
            return WebhookResponseTracker{};
        }
        const std::string task_id = "passive_recovery:" + scope.intent.payment_id;
        if (!m_context.repository().find_process_tracker_by_string(task_id)) {
            (void)m_context.repository()
                .record_process_tracker("PassiveRecoveryWorkflow", task_id, now_value());
        }
        WebhookResponseTracker tracker{};
        tracker.kind = WebhookResponseTracker::Kind::Payment;
        tracker.payment_id = scope.intent.payment_id;
        tracker.status = scope.intent.status;
        record_flow(scope, "RecoveryScheduled");
        return tracker;
    }

    /** `handle_action` (recovery_incoming.rs:1921-2016). */
    [[nodiscard]] WebhookResponseTracker handle_action(
        RequestScope& scope,
        RecoveryAction action,
        int mca_retry_threshold,
        int intent_retry_count
    ) const
    {
        switch (action) {
            case RecoveryAction::CancelInvoice:
                {
                    WebhookResponseTracker tracker{};
                    tracker.kind = WebhookResponseTracker::Kind::Payment;
                    tracker.payment_id = scope.intent.payment_id;
                    tracker.status = "cancelled";
                    const auto intent =
                        m_context.repository().find_payment_by_string(scope.intent.payment_id);
                    if (intent) {
                        ::app::store::payments::PaymentIntentRecord updated = *intent;
                        updated.set_status("cancelled");
                        (void)m_context.repository().update_payment(updated);
                    }
                    record_flow(scope, "RecoveryInvoiceCancelled");
                    return tracker;
                }
            case RecoveryAction::ScheduleFailedPayment:
                return handle_schedule_failed_payment(
                    scope,
                    intent_retry_count,
                    mca_retry_threshold
                );
            case RecoveryAction::SuccessPaymentExternal:
                record_flow(scope, "RecoverySuccessExternal");
                return WebhookResponseTracker{};
            case RecoveryAction::PendingPayment:
                record_flow(scope, "RecoveryPending");
                return WebhookResponseTracker{};
            case RecoveryAction::NoAction:
            case RecoveryAction::InvalidAction:
            default:
                record_flow(scope, "RecoveryNoAction");
                return WebhookResponseTracker{};
        }
    }

    // -----------------------------------------------------------------------
    // Misc helpers
    // -----------------------------------------------------------------------

    [[nodiscard]] std::string parse_connector_name(std::string_view connector) const
    {
        const std::size_t split = connector.find('_');
        return split == std::string_view::npos ? std::string{connector}
                                               : std::string{connector.substr(0, split)};
    }

    [[nodiscard]] std::string resolve_webhook_secret(const RequestScope& scope) const
    {
        if (!scope.connector_webhook_details.empty()) {
            return scope.connector_webhook_details;
        }
        for (const auto& webhook: m_context.repository().list_connector_webhooks()) {
            if (webhook.get_merchant_id() == scope.ids.merchant_id &&
                (webhook.get_connector() == scope.connector_key ||
                 webhook.get_connector() == scope.connector_name) &&
                !webhook.get_secret().empty()) {
                return webhook.get_secret();
            }
        }
        const std::string key_id = scope.ids.connector_id + ":" + scope.ids.profile_id;
        if (const auto stored = m_context.key_store().get("webhook_recovery", key_id)) {
            return stored->value();
        }
        return key_id;
    }

    void set_core_error(RequestScope& scope, core::error::AppError error) const
    {
        scope.core_error = std::move(error);
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        return std::to_string(seconds);
    }

    void record_flow(const RequestScope& scope, std::string_view name) const
    {
        m_context.metrics().record_flow(name);
        const std::string dimensioned =
            std::string{name} + ":merchant:" + scope.ids.merchant_id + ":" + scope.connector_key;
        m_context.metrics().record_flow(dimensioned);
        m_context.metrics().counter(dimensioned).increment();
    }

    void record_event_metadata(
        const RequestScope& scope,
        const IncomingWebhookEventMetadata& metadata
    ) const
    {
        const std::string event = std::format(
            "IncomingWebhookEvent:connector:{}:payment:{}:refund:{}:tracker:{}",
            metadata.connector,
            metadata.payment_id,
            metadata.refund_id,
            metadata.webhook_tracker_data
        );
        m_context.metrics().record_flow(event);
        (void)scope;
    }

    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::recovery_webhooks
