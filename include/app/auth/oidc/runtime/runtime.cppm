export module app_auth:oidc_runtime;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<::app::dto::oidc::OidcDiscoveryResponse>
    discovery(const core::behavior::CallContext& ctx) const
    {
        return m_discovery.discovery(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::oidc::JwksResponse>
    jwks(const core::behavior::CallContext& ctx) const
    {
        return m_jwks.jwks(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> authorize(
        const ::app::dto::oidc::OidcAuthorizeQuery& payload,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_authorize.authorize(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::oidc::OidcTokenResponse> token(
        const ::app::dto::oidc::OidcTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_token.token(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    OidcDiscoveryRuntime m_discovery;
    OidcJwksRuntime m_jwks;
    OidcAuthorizeRuntime m_authorize;
    OidcTokenRuntime m_token;
};

} // namespace app::handlers::oidc
