export module app_payment:three_ds_decision_rule_handler;

import std;
import core_behavior;
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
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // Upstream route uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
        // false, allow_platform_self_operation: false }). The ApiKey scheme is resolved
        // from the auditable RouteAuthRegistry. For API_KEY the two scope flags do not
        // affect authorization (AppAuthorizer consults them only for
        // HEADER_MERCHANT/HEADER_ORG); the registry row is recorded as a shared-file
        // follow-up so its evidence matches upstream exactly.
        const auto auth_error = m_request_support.authorize(
            req,
            "execute_decision_rule",
            spec_for("execute_decision_rule")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Flow::ThreeDsDecisionRuleExecute (routes/three_ds_decision_rule.rs:10,17).
        m_context.logger().record_flow("ThreeDsDecisionRuleExecute");
        m_context.logger().counter("THREE_DS_DECISION_RULE_EXECUTED").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }

        auto parsed = core::behavior::Serializer<
            ::app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }

        const auto& dto = *parsed;
        auto result = m_runtime.execute_decision_rule(dto, m_request_support.context(req));
        result.handle(sink);
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
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_execute_decision_rule(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch execute_decision_rule")
        );
    }

    explicit ThreeDsDecisionRuleRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    ::app::context::AppContext& m_context;
    ThreeDsDecisionRuleRuntime m_runtime;
    ThreeDsDecisionRuleInputSupport m_input_support;
    ThreeDsDecisionRulePolicy m_policy;
    ThreeDsDecisionRuleRequestSupport m_request_support;
};

} // namespace app::handlers::three_ds_decision_rule
