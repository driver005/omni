export module app_auth:auth_handler;

import app_context;
import core_router;

import :apple_pay_certificates_migration_handler;
import :authentication_handler;
import :client_secret_handler;
import :connector_onboarding_handler;
import :external_service_auth_handler;
import :hypersense_handler;
import :oidc_handler;
import :unified_connector_service_handler;
import :verify_connector_handler;

export namespace app::api {

class AuthHandlers
{
public:
    explicit AuthHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_apple_pay_certificates_migration{ctx},
        m_authentication{ctx},
        m_client_secret{ctx},
        m_connector_onboarding{ctx},
        m_external_service_auth{ctx},
        m_hypersense{ctx},
        m_oidc{ctx},
        m_unified_connector_service{ctx},
        m_verify_connector{ctx}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_apple_pay_certificates_migration.install_routes(router);
        m_authentication.install_routes(router);
        m_client_secret.install_routes(router);
        m_connector_onboarding.install_routes(router);
        m_external_service_auth.install_routes(router);
        m_hypersense.install_routes(router);
        m_oidc.install_routes(router);
        m_unified_connector_service.install_routes(router);
        m_verify_connector.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;
    ::app::handlers::apple_pay_certificates_migration::ApplePayCertificatesMigrationRouteHandlers
        m_apple_pay_certificates_migration;
    ::app::handlers::authentication::AuthenticationRouteHandlers m_authentication;
    ::app::handlers::client_secret::ClientSecretRouteHandlers m_client_secret;
    ::app::handlers::connector_onboarding::ConnectorOnboardingRouteHandlers m_connector_onboarding;
    ::app::handlers::external_service_auth::ExternalServiceAuthRouteHandlers
        m_external_service_auth;
    ::app::handlers::hypersense::HypersenseRouteHandlers m_hypersense;
    ::app::handlers::oidc::OidcRouteHandlers m_oidc;
    ::app::handlers::unified_connector_service::UnifiedConnectorServiceRouteHandlers
        m_unified_connector_service;
    ::app::handlers::verify_connector::VerifyConnectorRouteHandlers m_verify_connector;
};

} // namespace app::api
