export module app_handlers:routes;

import std;
import core_router;
import :all;
import app_context;

export namespace app::api {

class AppRoutes
{
public:
    void install(core::router::RouterContext<>& router, AppHandlers& handlers)
    {
        app::context::SerdeSetup{}.install();
        handlers.admin().install(router);
        handlers.api_keys().install(router);
        handlers.app().install(router);
        handlers.apple_pay_certificates_migration().install(router);
        handlers.authentication().install(router);
        handlers.blocklist().install(router);
        handlers.cache().install(router);
        handlers.card_issuer().install(router);
        handlers.cards_info().install(router);
        handlers.client_secret().install(router);
        handlers.configs().install(router);
        handlers.connector_onboarding().install(router);
        handlers.currency().install(router);
        handlers.customers().install(router);
        handlers.disputes().install(router);
        handlers.dummy_connector().install(router);
        handlers.external_service_auth().install(router);
        handlers.feature_matrix().install(router);
        handlers.files().install(router);
        handlers.fraud_check().install(router);
        handlers.gsm().install(router);
        handlers.health().install(router);
        handlers.hierarchical_resources().install(router);
        handlers.hypersense().install(router);
        handlers.lock_utils().install(router);
        handlers.mandates().install(router);
        handlers.metrics().install(router);
        handlers.offer_engine().install(router);
        handlers.oidc().install(router);
        handlers.payment_link().install(router);
        handlers.payment_methods().install(router);
        handlers.payments().install(router);
        handlers.payout_link().install(router);
        handlers.payouts().install(router);
        handlers.pm_auth().install(router);
        handlers.poll().install(router);
        handlers.process_tracker().install(router);
        handlers.profile_acquirer().install(router);
        handlers.profiles().install(router);
        handlers.proxy().install(router);
        handlers.recovery_webhooks().install(router);
        handlers.refunds().install(router);
        handlers.relay().install(router);
        handlers.revenue_recovery().install(router);
        handlers.revenue_recovery_data_backfill().install(router);
        handlers.revenue_recovery_redis().install(router);
        handlers.routing().install(router);
        handlers.subscription().install(router);
        handlers.superposition_proxy().install(router);
        handlers.superposition_sdk_config().install(router);
        handlers.telephone_order().install(router);
        handlers.three_ds_decision_rule().install(router);
        handlers.tokenization().install(router);
        handlers.unified_connector_service().install(router);
        handlers.user().install(router);
        handlers.user_role().install(router);
        handlers.verification().install(router);
        handlers.verify_connector().install(router);
        handlers.webhook_events().install(router);
        handlers.webhooks().install(router);
    }
};

} // namespace app::api
