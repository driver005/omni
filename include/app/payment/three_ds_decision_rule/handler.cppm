export module app_payment:three_ds_decision_rule_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_context;
import app_dto;
import app_metrics;

import :three_ds_decision_rule_input;
import :three_ds_decision_rule_result;
import :three_ds_decision_rule_runtime;

export namespace app::handlers::three_ds_decision_rule {

class ThreeDsDecisionRuleRouteHandlers
{
public:
    void handle_execute_decision_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
        // false, allow_platform_self_operation: false }). The ApiKey scheme is resolved
        // from the auditable RouteAuthRegistry. For API_KEY the two scope flags do not
        // affect authorization (AppAuthorizer consults them only for
        // HEADER_MERCHANT/HEADER_ORG); the registry row is recorded as a shared-file
        // follow-up so its evidence matches upstream exactly.
        const auto authorized = m_input_support.authorize_execute_decision_rule(req);
        if (authorized.error) {
            const auto& error = *authorized.error;
            error.send(res, send);
            return;
        }

        // Flow::ThreeDsDecisionRuleExecute (routes/three_ds_decision_rule.rs:10,17).
        m_context.metrics().record_flow("ThreeDsDecisionRuleExecute");
        m_context.metrics().counter("THREE_DS_DECISION_RULE_EXECUTED").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }

        using Input = ::app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        ExecuteDecisionRuleInput input{*parsed, authorized.merchant_id};
        auto result = m_runtime.execute_decision_rule(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
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
        Router{router, "three_ds_decision"}.add_route(
            Route{"execute"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_execute_decision_rule(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch execute_decision_rule")
        );
    }

    explicit ThreeDsDecisionRuleRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    ::app::context::AppContext& m_context;
    ThreeDsDecisionRuleRuntime m_runtime;
    ThreeDsDecisionRuleInputSupport m_input_support;
};

} // namespace app::handlers::three_ds_decision_rule
