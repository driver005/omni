export module app_auth:client_secret_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_context_core;
import app_http;

export namespace app::handlers::client_secret {

struct AuthorizedRequest
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/**
 * @brief Parse/auth/path support for the client_secret handlers.
 *
 * Both routes use V2ApiKeyAuth { allow_connected_scope_operation: false,
 * allow_platform_self_operation: false } (routes/client_secret.rs:32-35, :56-59).
 */
class ClientSecretInputSupport
{
public:
    explicit ClientSecretInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] AuthorizedRequest
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        if (auto denied = m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return AuthorizedRequest{std::move(denied), std::move(auth_ctx)};
        }
        if (auto denied = enforce_v2_api_key_flags(auth_ctx)) {
            return AuthorizedRequest{std::move(denied), std::move(auth_ctx)};
        }
        return AuthorizedRequest{std::nullopt, std::move(auth_ctx)};
    }

    [[nodiscard]] std::string
    merchant_id_from(interfaces::io::IRequest& req, const ::app::auth::AuthContext& auth_ctx) const
    {
        std::string merchant_id = auth_ctx.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = ::app::http::merchant_id_from_header{}(req);
        }
        return merchant_id;
    }

    [[nodiscard]] std::string client_secret_id_from_request(interfaces::io::IRequest& req) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        if (segments.count() == 0) {
            return {};
        }
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto segment = segments.at(i);
            if (segment && *segment == "client-secret") {
                return std::string{segments.at(i + 1).value_or("")};
            }
        }
        return std::string{segments.at(segments.count() - 1).value_or("")};
    }

private:
    // The generic authorizer resolves scheme/permission but does not consume these flags
    // for API_KEY, so enforce them against auth-derived claims (revenue_recovery pattern).
    [[nodiscard]] std::optional<core::error::AppError>
    enforce_v2_api_key_flags(const ::app::auth::AuthContext& auth_ctx) const
    {
        const auto connected = auth_ctx.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "connected-scope operations are not allowed"
            );
        }
        const auto internal_key = auth_ctx.claim(::app::auth::kClaimInternalApiKey);
        if (internal_key.has_value() && !internal_key->empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "platform self-operations are not allowed"
            );
        }
        return std::nullopt;
    }

    ::app::context::AppContext& m_context;
};

struct ClientSecretCreateInput
{
    ::app::dto::client_secret::ClientSecretCreateRequest request{};
    std::string merchant_id{};
    AuthorizedRequest auth{};
};

struct ClientSecretDeleteInput
{
    std::string client_secret_id{};
    AuthorizedRequest auth{};
};

} // namespace app::handlers::client_secret
