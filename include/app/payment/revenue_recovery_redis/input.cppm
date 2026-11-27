export module app_payment:revenue_recovery_redis_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

import :context;

export namespace app::handlers::revenue_recovery_redis {

/**
 * @brief Parse/auth/path/query support for the revenue recovery Redis handler.
 *
 * Turns request data into the plain Input consumed by the domain runtime. No
 * business logic and no HTTP response writes live here.
 */
class RevenueRecoveryRedisInputSupport
{
public:
    explicit RevenueRecoveryRedisInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream route uses V2AdminApiAuth (revenue_recovery_redis.rs:30). The
    // declarative rule in RouteAuthRegistry is PLATFORM_ADMIN / ADMIN; use it
    // instead of building a rule inline so the permission stays in one place.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("get_revenue_recovery_redis_data");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    // Upstream route: /:connector_customer_id with web::Path<String>. The id is
    // the final non-empty path segment, read via RequestPath rather than a manual
    // rfind on get_path (which also failed to strip a trailing query string).
    [[nodiscard]] core::behavior::Outcome<std::string>
    connector_customer_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        const auto id_index = segments.count() == 0 ? std::size_t{0} : segments.count() - 1;
        const auto connector_customer_id = std::string{segments.at(id_index).value_or("")};
        if (connector_customer_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector_customer_id"
            )};
        }
        return connector_customer_id;
    }

    // Upstream input: web::Query<GetRedisDataQuery>.key_type with enum
    // RedisKeyType { Status, Tokens } (api_models:204-213). serde rejects a
    // missing or unknown variant, so do the same instead of silently defaulting
    // to Status. Returns true for Tokens, false for Status.
    [[nodiscard]] core::behavior::Outcome<bool>
    key_type_from_query(interfaces::io::IRequest& req) const
    {
        const auto key_type_raw = ::app::http::RequestQuery{}(req).get("key_type");
        if (!key_type_raw) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key_type; expected Status or Tokens"
            )};
        }
        if (*key_type_raw == "Status") {
            return false;
        }
        if (*key_type_raw == "Tokens") {
            return true;
        }
        return std::unexpected{core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            "invalid key_type; expected Status or Tokens"
        )};
    }

private:
    ::app::context::AppContext& m_context;
};

struct GetRevenueRecoveryRedisDataData
{
    std::string connector_customer_id{};
    bool is_tokens{};

    explicit GetRevenueRecoveryRedisDataData(const core::behavior::CallContext& ctx) :
        connector_customer_id{::app::payment::detail::last_path(ctx)},
        is_tokens{}
    {
    }
};

/** @brief Request policy for the revenue_recovery_redis domain: wraps the domain auth rules. */
class RevenueRecoveryRedisPolicy
{
public:
    RevenueRecoveryRedisPolicy(
        ::app::context::AppContext& /*context*/,
        RevenueRecoveryRedisInputSupport& support
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
        return m_support.authorize(req);
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


private:
    RevenueRecoveryRedisInputSupport& m_support;
};

using RevenueRecoveryRedisRequestSupport =
    core::behavior::RequestSupport<RevenueRecoveryRedisPolicy>;
using RevenueRecoveryRedisPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] RevenueRecoveryRedisPolicySpec spec_for(std::string_view operation) noexcept
{
    return RevenueRecoveryRedisPolicySpec{.permission = operation};
}
} // namespace app::handlers::revenue_recovery_redis
