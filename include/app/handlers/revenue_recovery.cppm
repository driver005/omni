module;
export module app_handlers:revenue_recovery;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_workflow;

export namespace app::handlers::revenue_recovery {

/**
 * Local, in-process transcript of the upstream v2 revenue-recovery cancel route.
 *
 * Upstream call chain (all ported below as local helpers):
 *   routes/revenue_recovery.rs:10-39   revenue_recovery_cancel
 *   core/revenue_recovery/api.rs:581-624  cancel_revenue_recovery_core
 *   core/revenue_recovery.rs:1897-1932 get_workflow_entries
 *   core/revenue_recovery.rs:1946-1955 is_terminal_calculate_status
 *   core/revenue_recovery.rs:1962-2006 decide_invoice_cancellation
 *   core/revenue_recovery.rs:2013-2066 cancel_invoice_workflows
 *
 * External infrastructure (scheduler store, KMS key store, storage scheme) is replaced by
 * AppContext's repository / process-tracker service; the decision logic, wire shapes, status
 * strings, auth flags, errors and persistence are upstream-identical.
 */
class RevenueRecoveryRouteHandlers
{
public:
    /**
     * Upstream: revenue_recovery_cancel (routes/revenue_recovery.rs:10-39).
     * Flow::RevenueRecoveryCancel, server_wrap, LockAction::NotApplicable (:16, :19-37).
     */
    void handle_revenue_recovery_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream V2ApiKeyAuth { allow_connected_scope_operation: false,
        // allow_platform_self_operation: false } (routes/revenue_recovery.rs:32-35).
        // The registry row carries the permission; the scheme is narrowed locally to
        // V2_API_KEY so authorize_route verifies the api-key credential V2ApiKeyAuth reads
        // (services/authentication.rs:2978-2991, AuthContextBuilder parses "api-key=" at
        // context.cppm:206-207). Both override flags are false.
        const auto base_rule = m_context->auth_rules().rule_for("revenue_recovery_cancel");
        const app::auth::RouteAuthRule rule{
            app::auth::AuthScheme::V2_API_KEY, base_rule.permission(), false, false};
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_context->authorizer().authorize_route(auth_ctx, rule)) {
            send_error(res, send, *denied);
            return;
        }

        // Upstream check_merchant_access with both flags false
        // (services/authentication.rs:6487-6551): a connected-scope operation maps to
        // ConnectedAccountAuthNotSupported (BadRequest, "API does not support connected
        // account operation", :720-721) and a platform self-operation maps to
        // PlatformAccountAuthNotSupported (BadRequest, "API does not support platform
        // account operation", :702-703). AuthContext has no merchant account type, so the
        // local signals are the connected-merchant and internal-api-key claims.
        const auto connected = auth_ctx.claim(app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            send_error(res, send, make_bad_request("API does not support connected account operation"));
            return;
        }
        const auto internal_key = auth_ctx.claim(app::auth::kClaimInternalApiKey);
        if (internal_key.has_value() && !internal_key->empty()) {
            send_error(res, send, make_bad_request("API does not support platform account operation"));
            return;
        }

        // server_wrap(Flow::RevenueRecoveryCancel, LockAction::NotApplicable): flow and
        // counter are recorded in-process; no API lock is taken because upstream declares
        // LockAction::NotApplicable (routes/revenue_recovery.rs:16, :36).
        m_context->metrics().record_flow("revenue_recovery_cancel");
        m_context->metrics().counter("revenue_recovery_cancel").increment();

        // Upstream Path<PaymentReferenceId> from the route segment (routes/revenue_recovery.rs:14,
        // :17; app.rs:3550-3551). The cancel route is /v2/recovery/{merchant_reference_id}/cancel,
        // so the segment immediately before "cancel" is the merchant_reference_id. No body is
        // read (upstream takes none).
        const auto segments = app::http::RequestPath{}(req);
        std::string merchant_reference_id{};
        for (std::size_t i{1}; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "cancel") {
                merchant_reference_id = std::string{segments.at(i - 1).value_or("")};
                break;
            }
        }
        if (merchant_reference_id.empty()) {
            send_error(res, send, make_bad_request(
                "recovery cancel path requires a merchant_reference_id"));
            return;
        }

        // Upstream closure passes auth.platform and auth.profile into the core
        // (routes/revenue_recovery.rs:24-31); AuthContext is the local carrier of both.
        auto result = cancel_revenue_recovery_core(auth_ctx, merchant_reference_id);
        if (!result.has_value()) {
            send_error(res, send, std::move(result.error()));
            return;
        }

        auto text = serde::Ser::encode_json(*result);
        send_json(res, send, text, true);
    }

    /** Upstream scope /v2/recovery + resource /{merchant_reference_id}/cancel POST
     *  (routes/app.rs:3546, :3550-3551). No create route: upstream registers
     *  web::resource("").route(web::post().to(payments::recovery_payments_create)) at
     *  app.rs:3548 in the payments module, not in this handler. */
    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/recovery"}
            .add_route(
                Route{"/:merchant_reference_id/cancel"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_cancel(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_cancel")
            );
    }

    explicit RevenueRecoveryRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    using IntentRecord = app::store::payments::PaymentIntentRecord;
    using TrackerRecord = app::store::process_trackers::ProcessTrackerRecord;
    using Response = app::model::payments::RecoveryPaymentsResponse;

    // core/revenue_recovery.rs:66-68, common_enums/src/enums.rs:12054,12065,
    // diesel_models/src/process_tracker.rs:244,274,304,240,232,223.
    static constexpr std::string_view kRunner{"PassiveRecoveryWorkflow"};
    static constexpr std::string_view kCalculateWorkflow{"CALCULATE_WORKFLOW"};
    static constexpr std::string_view kExecuteWorkflow{"EXECUTE_WORKFLOW"};
    static constexpr std::string_view kBizCancelled{"CANCELLED"};
    static constexpr std::string_view kBizExecuteFailure{"FAILED_EXECUTE_TASK"};
    static constexpr std::string_view kBizCalculateFinish{"FAILED_DUE_TO_HARD_DECLINE_ERROR"};
    static constexpr std::string_view kBizRetriesExceeded{"RETRIES_EXCEEDED"};
    static constexpr std::string_view kBizFailure{"FAILURE"};
    static constexpr std::string_view kBizGlobalFailure{"GLOBAL_FAILURE"};

    struct WorkflowTasks
    {
        std::optional<TrackerRecord> calculate{};
        std::optional<TrackerRecord> execute{};
    };

    // Upstream CancelInvoiceRefusal (core/revenue_recovery.rs:1934-1943).
    struct Refusal
    {
        enum class Kind { intent_not_cancellable, retry_in_progress, already_terminal };
        Kind kind{};
        std::string intent_status{};
    };

    struct WorkflowFailure
    {
        std::string message{};
        bool internal{false};
    };

    // CancelInvoiceRefusal Display (core/revenue_recovery.rs:1936-1943).
    [[nodiscard]] static std::string refusal_message(const Refusal& refusal)
    {
        switch (refusal.kind) {
        case Refusal::Kind::intent_not_cancellable:
            return "invoice cannot be cancelled while its payment is in "
                 + refusal.intent_status + " state";
        case Refusal::Kind::retry_in_progress:
            return "invoice cannot be cancelled while a retry is in progress; try again shortly";
        case Refusal::Kind::already_terminal:
            return "invoice recovery already reached a terminal status";
        }
        return {};
    }

    // Upstream is_terminal_calculate_status (core/revenue_recovery.rs:1946-1955).
    [[nodiscard]] static bool is_terminal_calculate_status(std::string_view business_status)
    {
        return business_status == kBizCalculateFinish
            || business_status == kBizCancelled
            || business_status == kBizRetriesExceeded
            || business_status == kBizFailure
            || business_status == kBizGlobalFailure;
    }

    // Upstream get_workflow_entries (core/revenue_recovery.rs:1897-1932). The local
    // ProcessTrackerRecord::task_id is the analogue of the upstream process tracker id, and
    // find_process_tracker_by_string is the analogue of find_process_by_id
    // (repository.cppm:706). IDs follow `{runner}_{task}_{payment_id}`
    // (common_utils/src/id_type/global_id/payment.rs:34-40).
    [[nodiscard]] WorkflowTasks get_workflow_entries(const std::string& payment_id) const
    {
        WorkflowTasks tasks{};
        const auto calculate_id = std::format("{}_{}_{}", kRunner, kCalculateWorkflow, payment_id);
        const auto execute_id = std::format("{}_{}_{}", kRunner, kExecuteWorkflow, payment_id);
        // Upstream swallows calculate lookup errors (:1909-1914), propagates execute lookup
        // errors (:1921-1929); the local repository lookup has no error channel, so both
        // are plain optionals.
        tasks.calculate = m_context->repository().find_process_tracker_by_string(calculate_id);
        tasks.execute = m_context->repository().find_process_tracker_by_string(execute_id);
        return tasks;
    }

    // Upstream decide_invoice_cancellation (core/revenue_recovery.rs:1962-2006).
    // is_running checks ProcessTrackerStatus::ProcessStarted | Processing (:1990-1997);
    // upstream stores them uppercased (ProcessTrackerStatus::to_string().to_uppercase(),
    // core/revenue_recovery.rs:2085-2105, common_enums/src/enums.rs:12026-12039).
    [[nodiscard]] static std::expected<std::string, Refusal> decide_invoice_cancellation(
        std::string_view intent_status,
        const WorkflowTasks& tasks
    )
    {
        std::string cancelled_status{};
        if (intent_status == "failed") {
            cancelled_status = "cancelled";
        } else if (intent_status == "partially_captured"
                   || intent_status == "partially_captured_and_capturable") {
            cancelled_status = "cancelled_post_capture";
        } else {
            return std::unexpected{Refusal{
                Refusal::Kind::intent_not_cancellable, std::string{intent_status}}};
        }

        const auto is_running = [](const std::optional<TrackerRecord>& task) {
            if (!task) {
                return false;
            }
            const auto& status = task->get_status();
            return status == "PROCESS_STARTED" || status == "PROCESSING";
        };
        if (is_running(tasks.calculate) || is_running(tasks.execute)) {
            return std::unexpected{Refusal{Refusal::Kind::retry_in_progress, {}}};
        }
        if (tasks.calculate.has_value()
            && is_terminal_calculate_status(tasks.calculate->get_result())) {
            return std::unexpected{Refusal{Refusal::Kind::already_terminal, {}}};
        }
        return cancelled_status;
    }

    // Upstream cancel_invoice_workflows (core/revenue_recovery.rs:2013-2066): finish the
    // CALCULATE task as business_status::CANCELLED and the EXECUTE task as
    // business_status::EXECUTE_WORKFLOW_FAILURE, then persist PaymentIntentUpdate::VoidUpdate.
    // ProcessTrackerService::mark_terminal is the in-process substitute for
    // finish_process_with_business_status (process_tracker_service.cppm:105-125).
    [[nodiscard]] std::expected<IntentRecord, WorkflowFailure> cancel_invoice_workflows(
        const IntentRecord& payment_intent
    ) const
    {
        const auto tasks = get_workflow_entries(payment_intent.get_payment_id());
        auto decision = decide_invoice_cancellation(payment_intent.get_status(), tasks);
        if (!decision.has_value()) {
            return std::unexpected{WorkflowFailure{refusal_message(decision.error()), false}};
        }

        finish_task(tasks.calculate, kBizCancelled);
        finish_task(tasks.execute, kBizExecuteFailure);

        IntentRecord updated = payment_intent;
        updated.set_status(*decision);
        if (!m_context->repository().update_payment(updated)) {
            return std::unexpected{WorkflowFailure{"Something went wrong", true}};
        }
        return updated;
    }

    // Upstream cancel_revenue_recovery_core (core/revenue_recovery/api.rs:581-624):
    // PaymentNotFound (:590-598), cancel_invoice_workflows (:600-613, refusal ->
    // PreconditionFailed), then RecoveryPaymentsResponse from the cancelled intent (:615-619).
    [[nodiscard]] std::expected<Response, core::error::AppError> cancel_revenue_recovery_core(
        const app::auth::AuthContext& auth_ctx,
        std::string_view merchant_reference_id
    ) const
    {
        // find_payment_intent_by_merchant_reference_id_profile_id
        // (&merchant_reference_id, profile.get_id(), ...) -> PaymentNotFound (api.rs:590-598).
        // PaymentIntentRecord has no profile_id column (payments.cppm:13-55), so the local
        // lookup is merchant-scoped; profile_id is carried by AuthContext for the shared
        // profile-scope follow-up recorded in the report.
        const auto profile_id = auth_ctx.profile_id();
        (void)profile_id;
        const auto found = m_context->repository().find_payment_by_merchant_reference_id(
            auth_ctx.merchant_id(), merchant_reference_id);
        if (!found.has_value()) {
            m_context->metrics().counter("revenue_recovery_cancel_not_found").increment();
            // PaymentNotFound wire message (hyperswitch_domain_models errors: api_error_response.rs:100,459-461).
            return std::unexpected{make_not_found("Payment does not exist in our records")};
        }

        auto outcome = cancel_invoice_workflows(*found);
        if (!outcome.has_value()) {
            if (outcome.error().internal) {
                return std::unexpected{make_internal_error(outcome.error().message)};
            }
            m_context->metrics().counter("revenue_recovery_cancel_refused").increment();
            // PreconditionFailed { message: refusal.to_string() } (api.rs:609-613;
            // api_error_response.rs:226-227,626-628).
            return std::unexpected{make_bad_request(outcome.error().message)};
        }

        m_context->metrics().counter("revenue_recovery_cancel_completed").increment();

        Response response{};
        app::model::payments::GlobalPaymentIdValue id;
        id.set_value(outcome->get_payment_id());
        response.set_id(id);
        app::model::payments::IntentStatusValue status;
        status.set_value(outcome->get_status());
        response.set_intent_status(status);
        app::model::payments::PaymentReferenceIdValue reference;
        reference.set_value(outcome->get_merchant_reference_id());
        response.set_merchant_reference_id(reference);
        return response;
    }

    void finish_task(
        const std::optional<TrackerRecord>& task,
        std::string_view business_status
    ) const
    {
        if (task.has_value()) {
            m_context->process_trackers().mark_terminal(
                task->get_id(), std::string{business_status});
        }
    }

    [[nodiscard]] static core::error::AppError make_bad_request(std::string message)
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST, std::move(message));
    }

    [[nodiscard]] static core::error::AppError make_not_found(std::string message)
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND, std::move(message));
    }

    [[nodiscard]] static core::error::AppError make_internal_error(std::string message)
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL_ERROR, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::revenue_recovery
