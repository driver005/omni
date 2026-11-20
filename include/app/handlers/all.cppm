export module app_handlers:all;

import std;
import app_context;
import core_cache;
import core_error;
import core_database;
import core_workflow;
import memory_backends;
import :admin;
import :api_keys;
import :app;
import :apple_pay_certificates_migration;
import :authentication;
import :blocklist;
import :cache;
import :card_issuer;
import :cards_info;
import :client_secret;
import :configs;
import :connector_onboarding;
import :currency;
import :customers;
import :disputes;
import :dummy_connector;
import :external_service_auth;
import :feature_matrix;
import :files;
import :fraud_check;
import :gsm;
import :health;
import :hierarchical_resources;
import :hypersense;
import :lock_utils;
import :mandates;
import :metrics;
import :offer_engine;
import :oidc;
import :payment_link;
import :payment_methods;
import :payments;
import :payout_link;
import :payouts;
import :pm_auth;
import :poll;
import :process_tracker;
import :profile_acquirer;
import :profiles;
import :proxy;
import :recovery_webhooks;
import :refunds;
import :relay;
import :revenue_recovery;
import :revenue_recovery_data_backfill;
import :revenue_recovery_redis;
import :routing;
import :subscription;
import :superposition_proxy;
import :superposition_sdk_config;
import :telephone_order;
import :three_ds_decision_rule;
import :tokenization;
import :unified_connector_service;
import :user;
import :user_role;
import :verification;
import :verify_connector;
import :webhook_events;
import :webhooks;

export namespace app::api {

class AppHandlers
{
public:
    AppHandlers() = default;

    [[nodiscard]] app::handlers::admin::AdminRouteHandlers& admin() noexcept
    {
        return m_admin;
    }

    [[nodiscard]] app::handlers::api_keys::ApiKeysRouteHandlers& api_keys() noexcept
    {
        return m_api_keys;
    }

    [[nodiscard]] app::handlers::app::AppRouteHandlers& app() noexcept
    {
        return m_app;
    }

    [[nodiscard]] app::handlers::apple_pay_certificates_migration::ApplePayCertificatesMigrationRouteHandlers& apple_pay_certificates_migration() noexcept
    {
        return m_apple_pay_certificates_migration;
    }

    [[nodiscard]] app::handlers::authentication::AuthenticationRouteHandlers& authentication() noexcept
    {
        return m_authentication;
    }

    [[nodiscard]] app::handlers::blocklist::BlocklistRouteHandlers& blocklist() noexcept
    {
        return m_blocklist;
    }

    [[nodiscard]] app::handlers::cache::CacheRouteHandlers& cache() noexcept
    {
        return m_cache;
    }

    [[nodiscard]] app::handlers::card_issuer::CardIssuerRouteHandlers& card_issuer() noexcept
    {
        return m_card_issuer;
    }

    [[nodiscard]] app::handlers::cards_info::CardsInfoRouteHandlers& cards_info() noexcept
    {
        return m_cards_info;
    }

    [[nodiscard]] app::handlers::client_secret::ClientSecretRouteHandlers& client_secret() noexcept
    {
        return m_client_secret;
    }

    [[nodiscard]] app::handlers::configs::ConfigsRouteHandlers& configs() noexcept
    {
        return m_configs;
    }

    [[nodiscard]] app::handlers::connector_onboarding::ConnectorOnboardingRouteHandlers& connector_onboarding() noexcept
    {
        return m_connector_onboarding;
    }

    [[nodiscard]] app::handlers::currency::CurrencyRouteHandlers& currency() noexcept
    {
        return m_currency;
    }

    [[nodiscard]] app::handlers::customers::CustomersRouteHandlers& customers() noexcept
    {
        return m_customers;
    }

    [[nodiscard]] app::handlers::disputes::DisputesRouteHandlers& disputes() noexcept
    {
        return m_disputes;
    }

    [[nodiscard]] app::handlers::dummy_connector::DummyConnectorRouteHandlers& dummy_connector() noexcept
    {
        return m_dummy_connector;
    }

    [[nodiscard]] app::handlers::external_service_auth::ExternalServiceAuthRouteHandlers& external_service_auth() noexcept
    {
        return m_external_service_auth;
    }

    [[nodiscard]] app::handlers::feature_matrix::FeatureMatrixRouteHandlers& feature_matrix() noexcept
    {
        return m_feature_matrix;
    }

    [[nodiscard]] app::handlers::files::FilesRouteHandlers& files() noexcept
    {
        return m_files;
    }

    [[nodiscard]] app::handlers::fraud_check::FraudCheckRouteHandlers& fraud_check() noexcept
    {
        return m_fraud_check;
    }

    [[nodiscard]] app::handlers::gsm::GsmRouteHandlers& gsm() noexcept
    {
        return m_gsm;
    }

    [[nodiscard]] app::handlers::health::HealthRouteHandlers& health() noexcept
    {
        return m_health;
    }

    [[nodiscard]] app::handlers::hierarchical_resources::HierarchicalResourcesRouteHandlers& hierarchical_resources() noexcept
    {
        return m_hierarchical_resources;
    }

    [[nodiscard]] app::handlers::hypersense::HypersenseRouteHandlers& hypersense() noexcept
    {
        return m_hypersense;
    }

    [[nodiscard]] app::handlers::lock_utils::LockUtilsRouteHandlers& lock_utils() noexcept
    {
        return m_lock_utils;
    }

    [[nodiscard]] app::handlers::mandates::MandatesRouteHandlers& mandates() noexcept
    {
        return m_mandates;
    }

    [[nodiscard]] app::handlers::metrics::MetricsRouteHandlers& metrics() noexcept
    {
        return m_metrics;
    }

    [[nodiscard]] app::handlers::offer_engine::OfferEngineRouteHandlers& offer_engine() noexcept
    {
        return m_offer_engine;
    }

    [[nodiscard]] app::handlers::oidc::OidcRouteHandlers& oidc() noexcept
    {
        return m_oidc;
    }

    [[nodiscard]] app::handlers::payment_link::PaymentLinkRouteHandlers& payment_link() noexcept
    {
        return m_payment_link;
    }

    [[nodiscard]] app::handlers::payment_methods::PaymentMethodsRouteHandlers& payment_methods() noexcept
    {
        return m_payment_methods;
    }

    [[nodiscard]] app::handlers::payments::PaymentsRouteHandlers& payments() noexcept
    {
        return m_payments;
    }

    [[nodiscard]] app::handlers::payout_link::PayoutLinkRouteHandlers& payout_link() noexcept
    {
        return m_payout_link;
    }

    [[nodiscard]] app::handlers::payouts::PayoutsRouteHandlers& payouts() noexcept
    {
        return m_payouts;
    }

    [[nodiscard]] app::handlers::pm_auth::PmAuthRouteHandlers& pm_auth() noexcept
    {
        return m_pm_auth;
    }

    [[nodiscard]] app::handlers::poll::PollRouteHandlers& poll() noexcept
    {
        return m_poll;
    }

    [[nodiscard]] app::handlers::process_tracker::ProcessTrackerRouteHandlers& process_tracker() noexcept
    {
        return m_process_tracker;
    }

    [[nodiscard]] app::handlers::profile_acquirer::ProfileAcquirerRouteHandlers& profile_acquirer() noexcept
    {
        return m_profile_acquirer;
    }

    [[nodiscard]] app::handlers::profiles::ProfilesRouteHandlers& profiles() noexcept
    {
        return m_profiles;
    }

    [[nodiscard]] app::handlers::proxy::ProxyRouteHandlers& proxy() noexcept
    {
        return m_proxy;
    }

    [[nodiscard]] app::handlers::recovery_webhooks::RecoveryWebhooksRouteHandlers& recovery_webhooks() noexcept
    {
        return m_recovery_webhooks;
    }

    [[nodiscard]] app::handlers::refunds::RefundsRouteHandlers& refunds() noexcept
    {
        return m_refunds;
    }

    [[nodiscard]] app::handlers::relay::RelayRouteHandlers& relay() noexcept
    {
        return m_relay;
    }

    [[nodiscard]] app::handlers::revenue_recovery::RevenueRecoveryRouteHandlers& revenue_recovery() noexcept
    {
        return m_revenue_recovery;
    }

    [[nodiscard]] app::handlers::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillRouteHandlers& revenue_recovery_data_backfill() noexcept
    {
        return m_revenue_recovery_data_backfill;
    }

    [[nodiscard]] app::handlers::revenue_recovery_redis::RevenueRecoveryRedisRouteHandlers& revenue_recovery_redis() noexcept
    {
        return m_revenue_recovery_redis;
    }

    [[nodiscard]] app::handlers::routing::RoutingRouteHandlers& routing() noexcept
    {
        return m_routing;
    }

    [[nodiscard]] app::handlers::subscription::SubscriptionRouteHandlers& subscription() noexcept
    {
        return m_subscription;
    }

    [[nodiscard]] app::handlers::superposition_proxy::SuperpositionProxyRouteHandlers& superposition_proxy() noexcept
    {
        return m_superposition_proxy;
    }

    [[nodiscard]] app::handlers::superposition_sdk_config::SuperpositionSdkConfigRouteHandlers& superposition_sdk_config() noexcept
    {
        return m_superposition_sdk_config;
    }

    [[nodiscard]] app::handlers::telephone_order::TelephoneOrderRouteHandlers& telephone_order() noexcept
    {
        return m_telephone_order;
    }

    [[nodiscard]] app::handlers::three_ds_decision_rule::ThreeDsDecisionRuleRouteHandlers& three_ds_decision_rule() noexcept
    {
        return m_three_ds_decision_rule;
    }

    [[nodiscard]] app::handlers::tokenization::TokenizationRouteHandlers& tokenization() noexcept
    {
        return m_tokenization;
    }

    [[nodiscard]] app::handlers::unified_connector_service::UnifiedConnectorServiceRouteHandlers& unified_connector_service() noexcept
    {
        return m_unified_connector_service;
    }

    [[nodiscard]] app::handlers::user::UserRouteHandlers& user() noexcept
    {
        return m_user;
    }

    [[nodiscard]] app::handlers::user_role::UserRoleRouteHandlers& user_role() noexcept
    {
        return m_user_role;
    }

    [[nodiscard]] app::handlers::verification::VerificationRouteHandlers& verification() noexcept
    {
        return m_verification;
    }

    [[nodiscard]] app::handlers::verify_connector::VerifyConnectorRouteHandlers& verify_connector() noexcept
    {
        return m_verify_connector;
    }

    [[nodiscard]] app::handlers::webhook_events::WebhookEventsRouteHandlers& webhook_events() noexcept
    {
        return m_webhook_events;
    }

    [[nodiscard]] app::handlers::webhooks::WebhooksRouteHandlers& webhooks() noexcept
    {
        return m_webhooks;
    }

private:
    app::context::AppContext m_context{
        app::context::AppContextBuilder{}
            .database(core::db::DatabaseBuilder{}.use_backend(std::make_unique<core::db::InMemoryBackend>()).build())
            .cache(core::cache::CacheBuilder{}.use_backend(std::make_unique<core::cache::LocalCacheBackend>()).build())
            .workflow([&] {
                core::workflow::Workflow workflow;
                workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
                return workflow;
            }())
            .build()
    };
    app::handlers::admin::AdminRouteHandlers m_admin{m_context};
    app::handlers::api_keys::ApiKeysRouteHandlers m_api_keys{m_context};
    app::handlers::app::AppRouteHandlers m_app{m_context};
    app::handlers::apple_pay_certificates_migration::ApplePayCertificatesMigrationRouteHandlers m_apple_pay_certificates_migration{m_context};
    app::handlers::authentication::AuthenticationRouteHandlers m_authentication{m_context};
    app::handlers::blocklist::BlocklistRouteHandlers m_blocklist{m_context};
    app::handlers::cache::CacheRouteHandlers m_cache{m_context};
    app::handlers::card_issuer::CardIssuerRouteHandlers m_card_issuer{m_context};
    app::handlers::cards_info::CardsInfoRouteHandlers m_cards_info{m_context};
    app::handlers::client_secret::ClientSecretRouteHandlers m_client_secret{m_context};
    app::handlers::configs::ConfigsRouteHandlers m_configs{m_context};
    app::handlers::connector_onboarding::ConnectorOnboardingRouteHandlers m_connector_onboarding{m_context};
    app::handlers::currency::CurrencyRouteHandlers m_currency{m_context};
    app::handlers::customers::CustomersRouteHandlers m_customers{m_context};
    app::handlers::disputes::DisputesRouteHandlers m_disputes{m_context};
    app::handlers::dummy_connector::DummyConnectorRouteHandlers m_dummy_connector{m_context};
    app::handlers::external_service_auth::ExternalServiceAuthRouteHandlers m_external_service_auth{m_context};
    app::handlers::feature_matrix::FeatureMatrixRouteHandlers m_feature_matrix{m_context};
    app::handlers::files::FilesRouteHandlers m_files{m_context};
    app::handlers::fraud_check::FraudCheckRouteHandlers m_fraud_check{m_context};
    app::handlers::gsm::GsmRouteHandlers m_gsm{m_context};
    app::handlers::health::HealthRouteHandlers m_health{m_context};
    app::handlers::hierarchical_resources::HierarchicalResourcesRouteHandlers m_hierarchical_resources{m_context};
    app::handlers::hypersense::HypersenseRouteHandlers m_hypersense{m_context};
    app::handlers::lock_utils::LockUtilsRouteHandlers m_lock_utils{m_context};
    app::handlers::mandates::MandatesRouteHandlers m_mandates{m_context};
    app::handlers::metrics::MetricsRouteHandlers m_metrics{m_context};
    app::handlers::offer_engine::OfferEngineRouteHandlers m_offer_engine{m_context};
    app::handlers::oidc::OidcRouteHandlers m_oidc{m_context};
    app::handlers::payment_link::PaymentLinkRouteHandlers m_payment_link{m_context};
    app::handlers::payment_methods::PaymentMethodsRouteHandlers m_payment_methods{m_context};
    app::handlers::payments::PaymentsRouteHandlers m_payments{m_context};
    app::handlers::payout_link::PayoutLinkRouteHandlers m_payout_link{m_context};
    app::handlers::payouts::PayoutsRouteHandlers m_payouts{m_context};
    app::handlers::pm_auth::PmAuthRouteHandlers m_pm_auth{m_context};
    app::handlers::poll::PollRouteHandlers m_poll{m_context};
    app::handlers::process_tracker::ProcessTrackerRouteHandlers m_process_tracker{m_context};
    app::handlers::profile_acquirer::ProfileAcquirerRouteHandlers m_profile_acquirer{m_context};
    app::handlers::profiles::ProfilesRouteHandlers m_profiles{m_context};
    app::handlers::proxy::ProxyRouteHandlers m_proxy{m_context};
    app::handlers::recovery_webhooks::RecoveryWebhooksRouteHandlers m_recovery_webhooks{m_context};
    app::handlers::refunds::RefundsRouteHandlers m_refunds{m_context};
    app::handlers::relay::RelayRouteHandlers m_relay{m_context};
    app::handlers::revenue_recovery::RevenueRecoveryRouteHandlers m_revenue_recovery{m_context};
    app::handlers::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillRouteHandlers m_revenue_recovery_data_backfill{m_context};
    app::handlers::revenue_recovery_redis::RevenueRecoveryRedisRouteHandlers m_revenue_recovery_redis{m_context};
    app::handlers::routing::RoutingRouteHandlers m_routing{m_context};
    app::handlers::subscription::SubscriptionRouteHandlers m_subscription{m_context};
    app::handlers::superposition_proxy::SuperpositionProxyRouteHandlers m_superposition_proxy{m_context};
    app::handlers::superposition_sdk_config::SuperpositionSdkConfigRouteHandlers m_superposition_sdk_config{m_context};
    app::handlers::telephone_order::TelephoneOrderRouteHandlers m_telephone_order{m_context};
    app::handlers::three_ds_decision_rule::ThreeDsDecisionRuleRouteHandlers m_three_ds_decision_rule{m_context};
    app::handlers::tokenization::TokenizationRouteHandlers m_tokenization{m_context};
    app::handlers::unified_connector_service::UnifiedConnectorServiceRouteHandlers m_unified_connector_service{m_context};
    app::handlers::user::UserRouteHandlers m_user{m_context};
    app::handlers::user_role::UserRoleRouteHandlers m_user_role{m_context};
    app::handlers::verification::VerificationRouteHandlers m_verification{m_context};
    app::handlers::verify_connector::VerifyConnectorRouteHandlers m_verify_connector{m_context};
    app::handlers::webhook_events::WebhookEventsRouteHandlers m_webhook_events{m_context};
    app::handlers::webhooks::WebhooksRouteHandlers m_webhooks{m_context};
};

} // namespace app::api
