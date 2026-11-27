export module app_payment:revenue_recovery_runtime_cancel;

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

import :revenue_recovery_input;
import :revenue_recovery_result;

export namespace app::handlers::revenue_recovery {

/**
 * In-process port of the upstream revenue-recovery cancel core chain:
 *   core/revenue_recovery/api.rs:581-624  cancel_revenue_recovery_core
 *   core/revenue_recovery.rs:1897-1932 get_workflow_entries
 *   core/revenue_recovery.rs:1946-1955 is_terminal_calculate_status
 *   core/revenue_recovery.rs:1962-2006 decide_invoice_cancellation
 *   core/revenue_recovery.rs:2013-2066 cancel_invoice_workflows
 * Returns a plain Result or AppError; no HTTP types, no response writing.
 */
class RevenueRecoveryCancelRuntime
{
public:
    explicit RevenueRecoveryCancelRuntime(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Core: revenue_recovery_cancel ─────────────────────────
    // Upstream cancel_revenue_recovery_core (core/revenue_recovery/api.rs:581-624):
    // PaymentNotFound (:590-598), cancel_invoice_workflows (:600-613, refusal ->
    // PreconditionFailed), then RecoveryPaymentsResponse from the cancelled intent (:615-619).
    [[nodiscard]] core::behavior::Outcome<::app::model::payments::RecoveryPaymentsResponse>
        cancel(const core::behavior::CallContext& ctx) const
    {
        const auto input = RevenueRecoveryCancelData{ctx};
        // find_payment_intent_by_merchant_reference_id_profile_id
        // (&merchant_reference_id, profile.get_id(), ...) -> PaymentNotFound (api.rs:590-598).
        // PaymentIntentRecord has no profile_id column (payments.cppm:13-55), so the local
        // lookup is merchant-scoped; profile_id is carried by AuthContext for the shared
        // profile-scope follow-up recorded in the report.
        const auto profile_id = input.auth.profile_id();
        (void)profile_id;
        const auto found = m_context.repository().find_payment_by_merchant_reference_id(
            input.auth.merchant_id(),
            input.merchant_reference_id
        );
        if (!found.has_value()) {
            m_context.metrics().counter("revenue_recovery_cancel_not_found").increment();
            // PaymentNotFound wire message (hyperswitch_domain_models errors:
            // api_error_response.rs:100,459-461).
            return std::unexpected{make_not_found("Payment does not exist in our records")};
        }

        auto outcome = cancel_invoice_workflows(*found);
        if (!outcome.has_value()) {
            if (outcome.error().internal) {
                return std::unexpected{make_internal_error(outcome.error().message)};
            }
            m_context.metrics().counter("revenue_recovery_cancel_refused").increment();
            // PreconditionFailed { message: refusal.to_string() } (api.rs:609-613;
            // api_error_response.rs:226-227,626-628).
            return std::unexpected{make_bad_request(outcome.error().message)};
        }

        m_context.metrics().counter("revenue_recovery_cancel_completed").increment();

        Response response{};
        ::app::model::payments::GlobalPaymentIdValue id;
        id.set_value(outcome->get_payment_id());
        response.set_id(id);
        ::app::model::payments::IntentStatusValue status;
        status.set_value(outcome->get_status());
        response.set_intent_status(status);
        ::app::model::payments::PaymentReferenceIdValue reference;
        reference.set_value(outcome->get_merchant_reference_id());
        response.set_merchant_reference_id(reference);
        return ::app::model::payments::RecoveryPaymentsResponse{std::move(response)};
    }

private:
    using IntentRecord = ::app::store::payments::PaymentIntentRecord;
    using TrackerRecord = ::app::store::process_trackers::ProcessTrackerRecord;
    using Response = ::app::model::payments::RecoveryPaymentsResponse;

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
        enum class Kind
        {
            intent_not_cancellable,
            retry_in_progress,
            already_terminal
        };
        Kind kind{};
        std::string intent_status{};
    };

    struct WorkflowFailure
    {
        std::string message{};
        bool internal{false};
    };

    // CancelInvoiceRefusal Display (core/revenue_recovery.rs:1936-1943).
    [[nodiscard]] std::string refusal_message(const Refusal& refusal) const
    {
        switch (refusal.kind) {
            case Refusal::Kind::intent_not_cancellable:
                return "invoice cannot be cancelled while its payment is in " +
                       refusal.intent_status + " state";
            case Refusal::Kind::retry_in_progress:
                return "invoice cannot be cancelled while a retry is in progress; try again "
                       "shortly";
            case Refusal::Kind::already_terminal:
                return "invoice recovery already reached a terminal status";
        }
        return {};
    }

    // Upstream is_terminal_calculate_status (core/revenue_recovery.rs:1946-1955).
    [[nodiscard]] bool is_terminal_calculate_status(std::string_view business_status) const
    {
        return business_status == kBizCalculateFinish || business_status == kBizCancelled ||
               business_status == kBizRetriesExceeded || business_status == kBizFailure ||
               business_status == kBizGlobalFailure;
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
        tasks.calculate = m_context.repository().find_process_tracker_by_string(calculate_id);
        tasks.execute = m_context.repository().find_process_tracker_by_string(execute_id);
        return tasks;
    }

    // Upstream decide_invoice_cancellation (core/revenue_recovery.rs:1962-2006).
    // is_running checks ProcessTrackerStatus::ProcessStarted | Processing (:1990-1997);
    // upstream stores them uppercased (ProcessTrackerStatus::to_string().to_uppercase(),
    // core/revenue_recovery.rs:2085-2105, common_enums/src/enums.rs:12026-12039).
    [[nodiscard]] std::expected<std::string, Refusal>
    decide_invoice_cancellation(std::string_view intent_status, const WorkflowTasks& tasks) const
    {
        std::string cancelled_status{};
        if (intent_status == "failed") {
            cancelled_status = "cancelled";
        } else if (
            intent_status == "partially_captured" ||
            intent_status == "partially_captured_and_capturable"
        ) {
            cancelled_status = "cancelled_post_capture";
        } else {
            return std::unexpected{
                Refusal{Refusal::Kind::intent_not_cancellable, std::string{intent_status}}
            };
        }

        const auto is_running = [](const std::optional<TrackerRecord>& task)
        {
            if (!task) {
                return false;
            }
            const auto& status = task->get_status();
            return status == "PROCESS_STARTED" || status == "PROCESSING";
        };
        if (is_running(tasks.calculate) || is_running(tasks.execute)) {
            return std::unexpected{Refusal{Refusal::Kind::retry_in_progress, {}}};
        }
        if (tasks.calculate.has_value() &&
            is_terminal_calculate_status(tasks.calculate->get_result())) {
            return std::unexpected{Refusal{Refusal::Kind::already_terminal, {}}};
        }
        return cancelled_status;
    }

    // Upstream cancel_invoice_workflows (core/revenue_recovery.rs:2013-2066): finish the
    // CALCULATE task as business_status::CANCELLED and the EXECUTE task as
    // business_status::EXECUTE_WORKFLOW_FAILURE, then persist PaymentIntentUpdate::VoidUpdate.
    // ProcessTrackerService::mark_terminal is the in-process substitute for
    // finish_process_with_business_status (process_tracker_service.cppm:105-125).
    [[nodiscard]] std::expected<IntentRecord, WorkflowFailure>
    cancel_invoice_workflows(const IntentRecord& payment_intent) const
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
        if (!m_context.repository().update_payment(updated)) {
            return std::unexpected{WorkflowFailure{"Something went wrong", true}};
        }
        return updated;
    }

    void finish_task(
        const std::optional<TrackerRecord>& task,
        std::string_view business_status
    ) const
    {
        if (task.has_value()) {
            m_context.process_trackers().mark_terminal(
                task->get_id(),
                std::string{business_status}
            );
        }
    }

    [[nodiscard]] core::error::AppError make_bad_request(std::string message) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(message)
        );
    }

    [[nodiscard]] core::error::AppError make_not_found(std::string message) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            std::move(message)
        );
    }

    [[nodiscard]] core::error::AppError make_internal_error(std::string message) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::move(message)
        );
    }

    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::revenue_recovery
