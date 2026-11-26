export module app_payment:payment_handler;

import std;
import core_router;
import app_context;

import :card_issuer_handler;
import :cards_info_handler;
import :disputes_handler;
import :dummy_connector_handler;
import :fraud_check_handler;
import :mandates_handler;
import :payment_link_handler;
import :payment_methods_handler;
import :payments_handler;
import :payout_link_handler;
import :payouts_handler;
import :pm_auth_handler;
import :poll_handler;
import :refunds_handler;
import :relay_handler;
import :revenue_recovery_handler;
import :revenue_recovery_data_backfill_handler;
import :revenue_recovery_redis_handler;
import :three_ds_decision_rule_handler;
import :telephone_order_handler;
import :tokenization_handler;

export namespace app::api {

class PaymentHandlers
{
public:
    explicit PaymentHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_card_issuer{ctx},
        m_cards_info{ctx},
        m_disputes{ctx},
        m_dummy_connector{ctx},
        m_fraud_check{ctx},
        m_mandates{ctx},
        m_payment_link{ctx},
        m_payment_methods{ctx},
        m_payments{ctx},
        m_payout_link{ctx},
        m_payouts{ctx},
        m_pm_auth{ctx},
        m_poll{ctx},
        m_refunds{ctx},
        m_relay{ctx},
        m_revenue_recovery{ctx},
        m_revenue_recovery_data_backfill{ctx},
        m_revenue_recovery_redis{ctx},
        m_three_ds_decision_rule{ctx},
        m_telephone_order{ctx},
        m_tokenization{ctx}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_card_issuer.install_routes(router);
        m_cards_info.install_routes(router);
        m_disputes.install_routes(router);
        m_dummy_connector.install_routes(router);
        m_fraud_check.install_routes(router);
        m_mandates.install_routes(router);
        m_payment_link.install_routes(router);
        m_payment_methods.install_routes(router);
        m_payments.install_routes(router);
        m_payout_link.install_routes(router);
        m_payouts.install_routes(router);
        m_pm_auth.install_routes(router);
        m_poll.install_routes(router);
        m_refunds.install_routes(router);
        m_relay.install_routes(router);
        m_revenue_recovery.install_routes(router);
        m_revenue_recovery_data_backfill.install_routes(router);
        m_revenue_recovery_redis.install_routes(router);
        m_telephone_order.install_routes(router);
        m_three_ds_decision_rule.install_routes(router);
        m_tokenization.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;
    ::app::handlers::card_issuer::CardIssuerRouteHandlers m_card_issuer;
    ::app::handlers::cards_info::CardsInfoRouteHandlers m_cards_info;
    ::app::handlers::disputes::DisputesRouteHandlers m_disputes;
    ::app::handlers::dummy_connector::DummyConnectorRouteHandlers m_dummy_connector;
    ::app::handlers::fraud_check::FraudCheckRouteHandlers m_fraud_check;
    ::app::handlers::mandates::MandatesRouteHandlers m_mandates;
    ::app::handlers::payment_link::PaymentLinkRouteHandlers m_payment_link;
    ::app::handlers::payment_methods::PaymentMethodsRouteHandlers m_payment_methods;
    ::app::handlers::payments::PaymentsRouteHandlers m_payments;
    ::app::handlers::payout_link::PayoutLinkRouteHandlers m_payout_link;
    ::app::handlers::payouts::PayoutsRouteHandlers m_payouts;
    ::app::handlers::pm_auth::PmAuthRouteHandlers m_pm_auth;
    ::app::handlers::poll::PollRouteHandlers m_poll;
    ::app::handlers::refunds::RefundsRouteHandlers m_refunds;
    ::app::handlers::relay::RelayRouteHandlers m_relay;
    ::app::handlers::revenue_recovery::RevenueRecoveryRouteHandlers m_revenue_recovery;
    ::app::handlers::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillRouteHandlers
        m_revenue_recovery_data_backfill;
    ::app::handlers::revenue_recovery_redis::RevenueRecoveryRedisRouteHandlers
        m_revenue_recovery_redis;
    ::app::handlers::telephone_order::TelephoneOrderRouteHandlers m_telephone_order;
    ::app::handlers::three_ds_decision_rule::ThreeDsDecisionRuleRouteHandlers
        m_three_ds_decision_rule;
    ::app::handlers::tokenization::TokenizationRouteHandlers m_tokenization;
};

} // namespace app::api
