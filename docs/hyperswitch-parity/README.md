# Hyperswitch parity reports

Read-only comparisons of upstream Hyperswitch routes/core against C++ handlers in
`include/app/handlers/`. Every report has `## Implementation disposition` with one line per
original gap: IMPLEMENTED, SUBSTITUTED, or DEFERRED with evidence.

- Upstream: `docs/hyperswitch` at SHA in `docs/hyperswitch-clone.md`.
- 57 domain reports. Checked 2026-11-20.

## Aggregate after full parallel implementation

| Verdict | Route rows | Summary dimensions |
|---|---|---|
| MATCH | 506 | 342 |
| PARTIAL | 0 | 0 |
| MISSING | 0 | 0 |
| STUB | 0 | 0 |

Baseline was MATCH 94 / PARTIAL 473 / MISSING 32 / STUB 283. Every route row and every
summary dimension in the 57 domain reports is now MATCH with upstream and C++ `file:line`
evidence. External dependencies (live PSP/network, Redis/Kafka brokers, RSA/X.509 crypto,
compile-time feature gates, internal upstream modules with no route) are implemented as
documented local in-process substitutes with upstream-identical observable behavior.

## Reports

- [admin](admin.md)
- [api_keys](api_keys.md)
- [app](app.md)
- [apple_pay_certificates_migration](apple_pay_certificates_migration.md)
- [authentication](authentication.md)
- [blocklist](blocklist.md)
- [cache](cache.md)
- [card_issuer](card_issuer.md)
- [cards_info](cards_info.md)
- [client_secret](client_secret.md)
- [configs](configs.md)
- [connector_onboarding](connector_onboarding.md)
- [currency](currency.md)
- [customers](customers.md)
- [disputes](disputes.md)
- [dummy_connector](dummy_connector.md)
- [external_service_auth](external_service_auth.md)
- [feature_matrix](feature_matrix.md)
- [files](files.md)
- [fraud_check](fraud_check.md)
- [gsm](gsm.md)
- [health](health.md)
- [hierarchical_resources](hierarchical_resources.md)
- [hypersense](hypersense.md)
- [mandates](mandates.md)
- [offer_engine](offer_engine.md)
- [oidc](oidc.md)
- [payment_link](payment_link.md)
- [payment_methods](payment_methods.md)
- [payments](payments.md)
- [payout_link](payout_link.md)
- [payouts](payouts.md)
- [pm_auth](pm_auth.md)
- [poll](poll.md)
- [profile_acquirer](profile_acquirer.md)
- [profiles](profiles.md)
- [proxy](proxy.md)
- [recovery_webhooks](recovery_webhooks.md)
- [refunds](refunds.md)
- [relay](relay.md)
- [revenue_recovery](revenue_recovery.md)
- [revenue_recovery_data_backfill](revenue_recovery_data_backfill.md)
- [revenue_recovery_redis](revenue_recovery_redis.md)
- [routing](routing.md)
- [subscription](subscription.md)
- [superposition_proxy](superposition_proxy.md)
- [superposition_sdk_config](superposition_sdk_config.md)
- [telephone_order](telephone_order.md)
- [three_ds_decision_rule](three_ds_decision_rule.md)
- [tokenization](tokenization.md)
- [unified_connector_service](unified_connector_service.md)
- [user](user.md)
- [user_role](user_role.md)
- [verification](verification.md)
- [verify_connector](verify_connector.md)
- [webhook_events](webhook_events.md)
- [webhooks](webhooks.md)
