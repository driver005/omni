export module app_handlers_oidc:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_discovery;
import :runtime_jwks;
import :runtime_authorize;
import :runtime_token;

export namespace app::handlers::oidc {

class OidcRuntime
{
public:
    explicit OidcRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_discovery{context},
        m_jwks{context},
        m_authorize{context},
        m_token{context}
    {
    }

    [[nodiscard]] std::expected<OidcDiscoveryResult, core::error::AppError>
    discovery(const OidcDiscoveryInput& input) const
    {
        return m_discovery.discovery(input);
    }

    [[nodiscard]] std::expected<JwksEndpointResult, core::error::AppError>
    jwks(const JwksEndpointInput& input) const
    {
        return m_jwks.jwks(input);
    }

    [[nodiscard]] std::expected<OidcAuthorizeResult, core::error::AppError>
    authorize(const OidcAuthorizeInput& input) const
    {
        return m_authorize.authorize(input);
    }

    [[nodiscard]] std::expected<OidcTokenResult, core::error::AppError>
    token(const OidcTokenInput& input) const
    {
        return m_token.token(input);
    }

private:
    app::context::AppContext& m_context;
    OidcDiscoveryRuntime m_discovery;
    OidcJwksRuntime m_jwks;
    OidcAuthorizeRuntime m_authorize;
    OidcTokenRuntime m_token;
};

} // namespace app::handlers::oidc
