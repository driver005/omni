export module app_auth:client_secret_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

import :call_context;

export namespace app::handlers::client_secret {

// Both routes use V2ApiKeyAuth { allow_connected_scope_operation: false,
// allow_platform_self_operation: false } (routes/client_secret.rs:32-35, :56-59).
inline constexpr core::behavior::AuthSpec kClientSecretApiKeySpec{
    "PAYMENT_WRITE",
    false,
    false,
    false,
    false,
};

/**
 * @brief Domain policy for client secrets: registry rule plus the two V2 flags
 * that the generic authorizer does not consume for API_KEY.
 */
class ClientSecretPolicy
{
public:
    explicit ClientSecretPolicy(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return denied;
        }

        // The generic authorizer resolves scheme/permission but does not consume these flags
        // for API_KEY, so enforce them against auth-derived claims (revenue_recovery pattern).
        if (!spec.allow_connected_scope_operation) {
            const auto connected = auth_ctx.claim(::app::auth::kClaimConnectedMerchantId);
            if (connected.has_value() && !connected->empty()) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "connected-scope operations are not allowed"
                );
            }
        }
        if (!spec.allow_platform_self_operation) {
            const auto internal_key = auth_ctx.claim(::app::auth::kClaimInternalApiKey);
            if (internal_key.has_value() && !internal_key->empty()) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "platform self-operations are not allowed"
                );
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope out{};
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        out.merchant_id = auth_ctx.merchant_id().empty()
                              ? ::app::http::merchant_id_from_header{}(req)
                              : std::string{auth_ctx.merchant_id()};
        const auto connected = auth_ctx.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            out.connected_merchant_id = *connected;
        }
        return out;
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
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::client_secret
