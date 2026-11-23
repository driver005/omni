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

## Runtime route reachability

The `MATCH` verdict above is a **static source comparison** (path, verb, input, output, DB
call, cache, auth) — it does not prove that a route can be dispatched at runtime. The
runtime half is checked by `//include/app/testing:route_reachability_test`, which builds
the real `AppHandlers` route table, enumerates every (method, path) pair in the compiled
trie, and calls `RouteHandler::match()` on each with a concrete path.

Latest result (after the tree-based route table refactor):

| Metric | Value |
|---|---|
| OpenAPI registry operations | 554 |
| Dispatch entries in compiled trie | 554 |
| Probed | 554 |
| Routed to a handler | 554 |
| Unrouted (`Route not found`) | 0 |
| Matched but no handler for method | 0 |

The route table is built from a canonical `RouteTree` (`core_router:node` / `core_router:tree`):
duplicate scope registrations (e.g. the 78 `v2` / 56 `payments` nodes the old flat build
produced) are merged into one node with the union of their method handlers, and
`RouteTree::flatten()` emits a contiguous, correctly-offset vector for the matcher.

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
