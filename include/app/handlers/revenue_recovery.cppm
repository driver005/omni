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

class RevenueRecoveryRouteHandlers
{
public:
    void handle_revenue_recovery_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: V2ApiKeyAuth { allow_connected_scope_operation: false,
        // allow_platform_self_operation: false } (routes/revenue_recovery.rs:32-35).
        // The registry row carries the same scheme/permission and both override flags false
        // (registry.cppm:387). authorize_route enforces the API key scheme and permission.
        const auto rule = m_context->auth_rules().rule_for("revenue_recovery_cancel");
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // The two V2ApiKeyAuth override flags are part of the rule; the generic authorizer
        // does not consume them for API_KEY, so enforce them here against auth-derived claims.
        const auto connected = auth_ctx.claim(app::auth::kClaimConnectedMerchantId);
        if (!rule.allow_connected() && connected.has_value() && !connected->empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "connected-scope operations are not allowed for revenue recovery cancel"));
            return;
        }
        const auto internal_key = auth_ctx.claim(app::auth::kClaimInternalApiKey);
        if (!rule.allow_platform() && internal_key.has_value() && !internal_key->empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "platform self-operations are not allowed for revenue recovery cancel"));
            return;
        }

        // api::server_wrap(Flow::RevenueRecoveryCancel, LockAction::NotApplicable):
        // flow/accounting is recorded here; no API lock is taken because upstream declares
        // LockAction::NotApplicable (routes/revenue_recovery.rs:16, :36).
        m_context->metrics().record_flow("revenue_recovery_cancel");
        m_context->metrics().counter("revenue_recovery_cancel").increment();

        // Upstream cancel carries no body. The invoice is named solely by the path segment and
        // scoped by the auth-derived merchant/profile context.
        const auto segments = app::http::RequestPath{}(req);
        std::string merchant_reference_id{};
        for (std::size_t i{1}; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "cancel") {
                merchant_reference_id = std::string{segments.at(i - 1).value_or("")};
                break;
            }
        }
        if (merchant_reference_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "recovery cancel path requires a merchant_reference_id"));
            return;
        }

        const std::string merchant_id = auth_ctx.merchant_id();
        const std::string profile_id = auth_ctx.profile_id();

        // Upstream: find_payment_intent_by_merchant_reference_id_profile_id ->
        // PaymentNotFound (core/revenue_recovery/api.rs:590-598). Use the repository's
        // indexed reference-id query, then the exact payment-id lookup as a fallback.
        // profile_id is carried from auth; PaymentIntentRecord has no profile_id column yet,
        // so merchant scope is the available local scope (shared-file follow-up for Lane A).
        std::optional<app::store::payments::PaymentIntentRecord> found;
        if (!merchant_id.empty()) {
            found = m_context->repository().find_payment_by_merchant_reference_id(
                merchant_id, merchant_reference_id);
        } else {
            const auto matches = m_context->repository().list_payments(
                app::store::PaymentQuery{}.merchant_reference_id(merchant_reference_id));
            if (!matches.empty()) {
                found = matches.front();
            }
        }
        if (!found && !merchant_reference_id.empty()) {
            found = m_context->repository().find_payment_by_string(merchant_reference_id);
        }
        if (!found) {
            m_context->metrics().counter("revenue_recovery_cancel_not_found").increment();
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "invoice not found"));
            return;
        }
        auto intent = *found;
        if (!merchant_id.empty() && intent.get_merchant_id() != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "invoice belongs to another merchant"));
            return;
        }
        (void)profile_id;

        // Upstream: get_workflow_entries + decide_invoice_cancellation
        // (core/revenue_recovery.rs:1897-2006). CALCULATE/EXECUTE task status drives the
        // RetryInProgress / AlreadyTerminal refusals before any state is changed.
        const auto tasks = find_workflow_tasks(intent.get_payment_id(), merchant_reference_id);
        const auto decision = decide_invoice_cancellation(intent.get_status(), tasks);
        if (!decision.refusal.empty()) {
            m_context->metrics().counter("revenue_recovery_cancel_refused").increment();
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::PRECONDITION_FAILED, decision.refusal));
            return;
        }

        // Upstream: finish_process_with_business_status for CALCULATE (CANCELLED) and
        // EXECUTE (EXECUTE_WORKFLOW_FAILURE) (core/revenue_recovery.rs:2041-2051).
        finish_task(tasks.calculate, "CANCELLED");
        finish_task(tasks.execute, "EXECUTE_WORKFLOW_FAILURE");

        // Upstream: PaymentIntentUpdate::VoidUpdate + update_payment_intent
        // (core/revenue_recovery.rs:2053-2063). Persist the transition in the repository;
        // no business status is kept in cache.
        intent.set_status(decision.status);
        if (!m_context->repository().update_payment(intent)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "failed to persist cancelled payment intent"));
            return;
        }
        m_context->metrics().counter("revenue_recovery_cancel_completed").increment();

        using Response = app::model::payments::RecoveryPaymentsResponse;
        Response response;
        app::model::payments::GlobalPaymentIdValue id;
        id.set_value(intent.get_payment_id().empty() ? merchant_reference_id : intent.get_payment_id());
        response.set_id(id);
        app::model::payments::IntentStatusValue status;
        status.set_value(decision.status);
        response.set_intent_status(status);
        app::model::payments::PaymentReferenceIdValue reference;
        reference.set_value(intent.get_merchant_reference_id().empty()
                                ? merchant_reference_id
                                : intent.get_merchant_reference_id());
        response.set_merchant_reference_id(reference);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

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
    struct WorkflowTasks
    {
        std::optional<app::store::process_trackers::ProcessTrackerRecord> calculate{};
        std::optional<app::store::process_trackers::ProcessTrackerRecord> execute{};
    };

    struct CancellationDecision
    {
        std::string status{};
        std::string refusal{};
    };

    [[nodiscard]] WorkflowTasks find_workflow_tasks(
        const std::string& payment_id,
        const std::string& merchant_reference_id
    ) const
    {
        WorkflowTasks tasks{};
        const auto matches = [&payment_id, &merchant_reference_id](
            const app::store::process_trackers::ProcessTrackerRecord& record
        ) {
            const auto& payload = record.get_payload();
            return (!payment_id.empty() && payload.find(payment_id) != std::string::npos)
                || (!merchant_reference_id.empty() &&
                    payload.find(merchant_reference_id) != std::string::npos);
        };
        for (const auto& record : m_context->repository().list_process_trackers(
                 app::store::ProcessTrackerQuery{}.name("CALCULATE_WORKFLOW"))) {
            if (matches(record)) {
                tasks.calculate = record;
                break;
            }
        }
        for (const auto& record : m_context->repository().list_process_trackers(
                 app::store::ProcessTrackerQuery{}.name("EXECUTE_WORKFLOW"))) {
            if (matches(record)) {
                tasks.execute = record;
                break;
            }
        }
        return tasks;
    }

    [[nodiscard]] CancellationDecision decide_invoice_cancellation(
        const std::string& source_status,
        const WorkflowTasks& tasks
    ) const
    {
        CancellationDecision decision{};
        if (source_status == "failed") {
            decision.status = "cancelled";
        } else if (source_status == "partially_captured"
                   || source_status == "partially_captured_and_capturable") {
            decision.status = "cancelled_post_capture";
        } else {
            decision.refusal = "invoice cannot be cancelled while its payment is in "
                + source_status + " state";
            return decision;
        }

        const app::workflow::ProcessTrackerStatus tracker_status{};
        const auto is_running = [&tracker_status](
            const std::optional<app::store::process_trackers::ProcessTrackerRecord>& task
        ) {
            if (!task) {
                return false;
            }
            const auto& status = task->get_status();
            return status == tracker_status.running()
                || status == tracker_status.retrying()
                || status == tracker_status.pending();
        };
        if (is_running(tasks.calculate) || is_running(tasks.execute)) {
            decision.status.clear();
            decision.refusal =
                "invoice cannot be cancelled while a retry is in progress; try again shortly";
            return decision;
        }

        const auto is_terminal_calculate = [](
            const std::optional<app::store::process_trackers::ProcessTrackerRecord>& task
        ) {
            if (!task) {
                return false;
            }
            const auto& business_status = task->get_result();
            return business_status == "CALCULATE_WORKFLOW_FINISH"
                || business_status == "CANCELLED"
                || business_status == "RETRIES_EXCEEDED"
                || business_status == "FAILURE"
                || business_status == "GLOBAL_FAILURE";
        };
        if (is_terminal_calculate(tasks.calculate)) {
            decision.status.clear();
            decision.refusal = "invoice recovery already reached a terminal status";
        }
        return decision;
    }

    void finish_task(
        const std::optional<app::store::process_trackers::ProcessTrackerRecord>& task,
        const std::string& business_status
    ) const
    {
        if (task) {
            m_context->process_trackers().mark_terminal(task->get_id(), business_status);
        }
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
