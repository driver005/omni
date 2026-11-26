export module app_payment:fraud_check_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_metrics;

import :fraud_check_input;
import :fraud_check_result;
import :fraud_check_runtime;

export namespace app::handlers::fraud_check {

/**
 * Strict upstream port of the single fraud-check route.
 *
 * Upstream call graph (all v1):
 *   routes/fraud_check.rs:11-32 frm_fulfillment
 *     -> core/fraud_check.rs:1175-1241 frm_fulfillment_core
 *       -> core/fraud_check.rs:1245-1305 make_fulfillment_api_call
 *         -> FraudCheckConnectorData::get_connector_by_name
 *         -> core/fraud_check/flows/fulfillment_flow.rs:33-151 construct_fulfillment_router_data
 *         -> services::execute_connector_processing_step + to_payment_failed_response
 *         -> db.update_fraud_check_response_with_frm_id
 *
 * The v1/v2 `todo!()` variants have no behavior to port and are documented in
 * docs/hyperswitch-parity/fraud_check.md.
 */
class FraudCheckRouteHandlers
{
public:
    // routes/fraud_check.rs:11-32 -- server_wrap(Flow::FrmFulfillment, ApiKeyAuth{false,false},
    // LockAction::NotApplicable).
    void handle_frm_fulfillment(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth = m_input_support.authorize_frm_fulfillment(req)) {
            res.set_status(auth->get_status());
            res.set_body(serde::Ser::serialize("application/json", *auth));
            send();
            return;
        }

        // Flow::FrmFulfillment accounting; LockAction::NotApplicable -> no API lock.
        m_context.metrics().record_flow("FrmFulfillment");
        m_context.metrics().counter("FRM_FULFILLMENT").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto request = m_input_support.parse_and_validate_request(body);
        if (!request) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid FrmFulfillmentRequest payload"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const std::string org_id{::app::http::org_id_from_header{}(req)};
        const std::string header_merchant_id{::app::http::merchant_id_from_header{}(req)};

        FrmFulfillmentInput
            input{*request, std::string{body}, std::move(header_merchant_id), std::move(org_id)};
        auto result = m_runtime.frm_fulfillment(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "webhooks"}.add_route(
            Route{"frm_fulfillment"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_frm_fulfillment(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch frm_fulfillment")
        );
    }

    explicit FraudCheckRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    ::app::context::AppContext& m_context;
    FraudCheckRuntime m_runtime;
    FrmInputSupport m_input_support;
};

} // namespace app::handlers::fraud_check
