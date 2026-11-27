export module app_auth:oidc_runtime_discovery;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_keystore;
import app_services;

import :oidc_input;
import :oidc_result;
import :oidc_runtime_common;

export namespace app::handlers::oidc {

class OidcDiscoveryRuntime
{
public:
    explicit OidcDiscoveryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::oidc::OidcDiscoveryResponse>
    discovery(const core::behavior::CallContext& /*ctx*/) const
    {
        m_context.logger().record_flow("oidc_discovery");
        m_context.logger().counter("oidc.oidc_discovery").increment();
        m_context.logger().counter("oidc.api_event.Oidc").increment();

        // Upstream: OidcDiscoveryResponse::new(state.tenant.base_url,
        // user_utils::get_base_url(&state)). Config store supplies both values.
        const auto base = m_support.tenant_base_url();
        const auto control = m_support.control_center_url();
        ::app::dto::oidc::OidcDiscoveryResponse document{};
        document.set_issuer(base);
        document.set_authorization_endpoint(control + "/oauth2/authorize");
        document.set_token_endpoint(base + "/oauth2/token");
        document.set_jwks_uri(base + "/oauth2/jwks");
        document.set_response_types_supported({::app::dto::oidc::ResponseType::Code});
        document.set_response_modes_supported({::app::dto::oidc::ResponseMode::Query});
        document.set_subject_types_supported({::app::dto::oidc::SubjectType::Public});
        document.set_id_token_signing_alg_values_supported(
            {::app::dto::oidc::SigningAlgorithm::Rs256}
        );
        document.set_grant_types_supported({::app::dto::oidc::GrantType::AuthorizationCode});
        document.set_scopes_supported({
            ::app::dto::oidc::Scope::Openid,
            ::app::dto::oidc::Scope::Email,
            ::app::dto::oidc::Scope::Profile,
        });
        document.set_token_endpoint_auth_methods_supported(
            {::app::dto::oidc::TokenAuthMethod::ClientSecretBasic}
        );
        document.set_claims_supported({
            ::app::dto::oidc::Claim::Aud,
            ::app::dto::oidc::Claim::Email,
            ::app::dto::oidc::Claim::EmailVerified,
            ::app::dto::oidc::Claim::Exp,
            ::app::dto::oidc::Claim::Iat,
            ::app::dto::oidc::Claim::Iss,
            ::app::dto::oidc::Claim::Sub,
            ::app::dto::oidc::Claim::PreferredUsername,
        });
        document.set_claims_parameter_supported(false);
        document.set_request_parameter_supported(false);
        document.set_request_uri_parameter_supported(false);
        return document;
    }

private:
    ::app::context::AppContext& m_context;
    OidcRuntimeSupport m_support;
};

} // namespace app::handlers::oidc
