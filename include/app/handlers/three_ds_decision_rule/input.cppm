export module app_handlers_three_ds_decision_rule:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;

export namespace app::handlers::three_ds_decision_rule {

namespace Dto = app::dto::three_ds_decision_rule;

// Upstream route uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
// false, allow_platform_self_operation: false }). For API_KEY the two scope flags do not
// affect authorization (AppAuthorizer consults them only for HEADER_MERCHANT/HEADER_ORG).
struct ExecuteDecisionRuleAuthorizedInput
{
    std::optional<core::error::AppError> error{};
    std::string merchant_id{};
};

/**
 * @brief Parse/auth support for the three_ds_decision_rule handler.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * struct consumed by the domain runtime.
 */
class ThreeDsDecisionRuleInputSupport
{
public:
    explicit ThreeDsDecisionRuleInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] ExecuteDecisionRuleAuthorizedInput
    authorize_execute_decision_rule(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("execute_decision_rule");
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        auto denied = m_context.authorizer().authorize_route(auth_context, rule);
        // Upstream passes auth.platform.get_processor().get_account().get_id() as the
        // processor merchant id (core:29-34); locally the authenticated AuthContext is
        // the single identity source used by authorization.
        return ExecuteDecisionRuleAuthorizedInput{
            std::move(denied),
            std::string{auth_context.merchant_id()}
        };
    }

private:
    app::context::AppContext& m_context;
};

struct ExecuteDecisionRuleInput
{
    Dto::ThreeDsDecisionRuleExecuteRequest request{};
    std::string processor_merchant_id{};
};

} // namespace app::handlers::three_ds_decision_rule
