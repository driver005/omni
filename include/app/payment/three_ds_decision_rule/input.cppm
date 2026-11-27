export module app_payment:three_ds_decision_rule_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;

import :context;
export namespace app::handlers::three_ds_decision_rule {

namespace Dto = ::app::dto::three_ds_decision_rule;
struct ExecuteDecisionRuleAuthorizedData
{
    std::optional<core::error::AppError> error{};
    std::string merchant_id{};
    explicit ExecuteDecisionRuleAuthorizedData(const core::behavior::CallContext& ctx) :
        error{},
        merchant_id{ctx.merchant_id}
    {
    }
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
    explicit ThreeDsDecisionRuleInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] ExecuteDecisionRuleAuthorizedInput
    authorize_execute_decision_rule(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("execute_decision_rule");
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
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
    ::app::context::AppContext& m_context;
};
struct ExecuteDecisionRuleData
{
    Dto::ThreeDsDecisionRuleExecuteRequest request{};
    std::string processor_merchant_id{};
    template<class Dto>
    explicit ExecuteDecisionRuleData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Dto::ThreeDsDecisionRuleExecuteRequest>(dto)},
        processor_merchant_id{ctx.merchant_id}
    {
    }
};


/** @brief Request policy for the three_ds_decision_rule domain: wraps the domain auth rules. */
class ThreeDsDecisionRulePolicy
{
public:
    ThreeDsDecisionRulePolicy(
        ::app::context::AppContext& /*context*/,
        ThreeDsDecisionRuleInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        m_last = m_support.authorize_execute_decision_rule(req);
        return m_last->error;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] const ExecuteDecisionRuleAuthorizedInput& last() const noexcept
    {
        return *m_last;
    }

private:
    ThreeDsDecisionRuleInputSupport& m_support;
    mutable std::optional<ExecuteDecisionRuleAuthorizedInput> m_last{};
};

using ThreeDsDecisionRuleRequestSupport = core::behavior::RequestSupport<ThreeDsDecisionRulePolicy>;
using ThreeDsDecisionRulePolicySpec = core::behavior::AuthSpec;

[[nodiscard]] ThreeDsDecisionRulePolicySpec spec_for(std::string_view operation) noexcept
{
    return ThreeDsDecisionRulePolicySpec{.permission = operation};
}
} // namespace app::handlers::three_ds_decision_rule
