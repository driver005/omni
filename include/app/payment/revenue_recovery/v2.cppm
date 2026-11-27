export module app_payment:revenue_recovery_handlers_v2;

import std;
import core_behavior;
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
import :revenue_recovery_runtime;

export namespace app::handlers::revenue_recovery {

/**
 * Local, in-process transcript of the upstream v2 revenue-recovery cancel route.
 *
 * Upstream call chain (all ported below into the runtime partition):
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
class RevenueRecoveryHandlersV2
{
public:
    void bind_request_support(RevenueRecoveryRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RevenueRecoveryRequestSupport* m_request_support{nullptr};

    void bind_policy(RevenueRecoveryPolicy& p) noexcept
    {
        m_policy = &p;
    }

    RevenueRecoveryPolicy* m_policy{nullptr};

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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // 1. request -> Input: auth (incl. both check_merchant_access flags) + path.
        const auto auth_error = m_request_support->authorize(
            req,
            "revenue_recovery_cancel",
            spec_for("revenue_recovery_cancel")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto& auth = m_policy->last();

        // server_wrap(Flow::RevenueRecoveryCancel, LockAction::NotApplicable): flow and
        // counter are recorded in-process; no API lock is taken because upstream declares
        // LockAction::NotApplicable (routes/revenue_recovery.rs:16, :36).
        m_context.metrics().record_flow("revenue_recovery_cancel");
        m_context.metrics().counter("revenue_recovery_cancel").increment();

        auto merchant_reference_id = m_input_support.merchant_reference_id_from_path(req);
        if (!merchant_reference_id) {
            const auto& error = merchant_reference_id.error();
            error.send(sink);
            return;
        }

        // Upstream closure passes auth.platform and auth.profile into the core
        // (routes/revenue_recovery.rs:24-31); AuthContext is the local carrier of both.

        // 2. Input -> Result.
        auto result = m_runtime.cancel(m_request_support->context(req));
        result.handle(sink);
    }

    /** Upstream scope /v2/recovery + resource /{merchant_reference_id}/cancel POST
     *  (routes/app.rs:3546, :3550-3551). No create route: upstream registers
     *  web::resource("").route(web::post().to(payments::recovery_payments_create)) at
     *  app.rs:3548 in the payments module, not in this handler. */
    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/recovery"}.add_router(
            Router{router, ":merchant_reference_id"}.add_route(
                Route{"cancel"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_revenue_recovery_cancel(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch revenue_recovery_cancel")
            )
        );
    }

    RevenueRecoveryHandlersV2(
        ::app::context::AppContext& ctx,
        RevenueRecoveryRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
    {
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryInputSupport m_input_support;
    RevenueRecoveryRuntime& m_runtime;
};

} // namespace app::handlers::revenue_recovery
