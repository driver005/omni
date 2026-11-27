export module app_auth:unified_connector_service_input;

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
import core_behavior;
import app_context_core;
import app_http;
import app_metrics;
import app_services;

import :call_context;

export namespace app::handlers::unified_connector_service {

// Constants mirrored verbatim from upstream consts.rs:389,393,399,405,407,409
// and kill_switch.rs:23,29. Kept here so the path parser and every sub-runtime
// share one definition.
inline constexpr std::string_view kRolloutConfigPrefix{"ucs_rollout_config"};
inline constexpr std::string_view kNotConfigured{"not_configured"};
inline constexpr std::string_view kKillSwitchPrefix{"ucs_kill_switch"};
inline constexpr std::string_view kShadowKey{"ucs_shadow_killed"};
inline constexpr std::string_view kCounterField{"counter"};
inline constexpr std::string_view kDeclineField{"counter_connector_decline"};
inline constexpr std::int64_t kPrimaryTtlSeconds{7 * 24 * 60 * 60}; // consts.rs:405
inline constexpr std::int64_t kShadowTtlSeconds{24 * 60 * 60};      // consts.rs:409
inline constexpr std::string_view kSuperpositionThresholdKey{
    "ucs.shadow_kill_switch_threshold"
}; // consts.rs:615

/**
 * @brief Upstream `listed_key`/`rollout_scope` pair.
 *
 * `listed_key` is the original final path segment (what upstream compares to
 * UCS_SHADOW_KILL_SWITCH_REDIS_KEY); `scope` is `rollout_scope_in(listed_key)`.
 * Shadow is decided from `listed_key`, NOT the stripped scope: upstream checks
 * the original key (kill_switch.rs:545,584), so a tenant key whose scope
 * happens to equal `ucs_shadow_killed` must stay in the tenant store.
 */
struct ScopeInfo
{
    std::string listed_key{};
    std::string scope{};
    bool shadow{false};
};

/**
 * @brief Parse/auth/path support for the unified-connector-service handlers.
 *
 * Both routes are body-less; the only request-derived data is the admin auth
 * decision and the final path segment resolved into @ref ScopeInfo. Business
 * logic stays in the runtime.
 */
class UnifiedConnectorServiceInputSupport
{
public:
    explicit UnifiedConnectorServiceInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream routes/unified_connector_service.rs:22-30 and :44-52:
    // server_wrap(flow, ..., AdminApiAuth, LockAction::NotApplicable). The
    // declarative rule is PLATFORM_ADMIN / AppPermission::ADMIN
    // (auth/registry.cppm:191), the local AdminApiAuth equivalent.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_admin(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Domain policy: the declarative AdminApiAuth rule for both routes. */
class UnifiedConnectorServicePolicy
{
public:
    explicit UnifiedConnectorServicePolicy(::app::context::AppContext& context) noexcept :
        m_support{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        return m_support.authorize_admin(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        (void)req;
        return {};
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }

private:
    UnifiedConnectorServiceInputSupport m_support;
};

} // namespace app::handlers::unified_connector_service
