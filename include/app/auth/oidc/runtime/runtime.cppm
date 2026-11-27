export module app_auth:oidc_runtime;

import std;
import core_error;
import core_serde;
import app_context;
import app_dto;

import :oidc_input;
import :oidc_result;
import :oidc_runtime_common;
import :oidc_runtime_discovery;
import :oidc_runtime_jwks;
import :oidc_runtime_authorize;
import :oidc_runtime_token;

export namespace app::handlers::oidc {

class OidcRuntime
{
public:
    explicit OidcRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_discovery{context},
        m_jwks{context},
        m_authorize{context},
        m_token{context}
    {
    }

    [[nodiscard]] std::expected<::app::dto::oidc::OidcDiscoveryResponse, core::error::AppError>
    discovery(const OidcDiscoveryInput& input) const
    {
        return m_discovery.discovery(input);
    }

    [[nodiscard]] std::expected<::app::dto::oidc::JwksResponse, core::error::AppError>
    jwks(const JwksEndpointInput& input) const
    {
        return m_jwks.jwks(input);
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    authorize(const OidcAuthorizeInput& input) const
    {
        return m_authorize.authorize(input);
    }

    [[nodiscard]] std::expected<::app::dto::oidc::OidcTokenResponse, core::error::AppError>
    token(const OidcTokenInput& input) const
    {
        return m_token.token(input);
    }

private:
    ::app::context::AppContext& m_context;
    OidcDiscoveryRuntime m_discovery;
    OidcJwksRuntime m_jwks;
    OidcAuthorizeRuntime m_authorize;
    OidcTokenRuntime m_token;
};

} // namespace app::handlers::oidc
