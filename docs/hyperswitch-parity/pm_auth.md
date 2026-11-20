# Parity: pm_auth

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/pm_auth.rs`; registration `docs/hyperswitch/crates/router/src/routes/app.rs` (scope `/payment_methods`, lines 1854, 1926, 1929)
- Upstream core: `docs/hyperswitch/crates/router/src/core/pm_auth.rs` (+ `helpers`, `transformers` modules declared at lines 8-9)
- C++: include/app/handlers/pm_auth.cppm
- Checked: 2026-11-19; re-verified 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both POST `/payment_methods/auth/link` and `/payment_methods/auth/exchange` (`pm_auth.cppm:474-496`) |
| Input types | MATCH | Typed DTOs mirror upstream field sets (incl. optional `language`/`client_secret`); secret-string normalization `pm_auth.cppm:101-112,316-325`; auth-level `client_secret` override `pm_auth.cppm:120-124,333-337` |
| Output types | PARTIAL | Link token response matches (`pm_auth.cppm:241-245`); upstream exchange returns empty 200, C++ returns `access_token` JSON (`pm_auth.cppm:464-467`) |
| Auth/permissions | PARTIAL | `authorize_request` retries SDK/CLIENT_SECRET/API_KEY/V2_API_KEY over the registry rule permission (`pm_auth.cppm:540-570,563-568`); no `ApiKeyAuth::default()` — documented local substitute, keep PARTIAL |
| Persistence | PARTIAL | PI verify via repository, MCA lookup, key_store, bank-PM insert/update (`pm_auth.cppm:579,751-765,1177,1006,1024`); Redis still substituted by in-memory cache + MCA config (`pm_auth.cppm:1105-1119,1133-1150`) — keep PARTIAL |
| Domain logic | PARTIAL | Upgraded from STUB: PI/client-secret verify, MCA/auth, header metadata, HMAC-shaped dedupe, bank-PM persistence (`pm_auth.cppm:137-159,180-211,869-1026`) — local substitutes, keep PARTIAL |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/payment_methods/auth/link` | `link_token_create` | `handle_link_token_create` | `LinkTokenCreateRequest` (`payment_id`, `payment_method`, `payment_method_type`, optional `language`, `client_secret`) | `LinkTokenCreateResponse` (`link_token`, `connector`) | Upstream v1 SDK / v2 client-secret; C++ `authorize_request` + SDK/CS alternates (`pm_auth.cppm:540-570`) | Redis choices, MCA auth, PI verify, connector `create_link` call | PARTIAL | Upstream `routes/pm_auth.rs:12-76`, `app.rs:1854,1926`, `core/pm_auth.rs:48-212`; C++ `pm_auth.cppm:29-246,540-570,1096-1192`, DTO `dto/pm_auth.cppm:83-101,20-29` |
| POST `/payment_methods/auth/exchange` | `exchange_token` | `handle_exchange_token` | `ExchangeTokenCreateRequest` (`public_token`, `payment_id`, `payment_method`, `payment_method_type`, optional `client_secret`) | Upstream empty 200 (`RouterResponse<()>` / `StatusOk`); C++ `ExchangeTokenCreateResponse` (`access_token`) | Upstream v1 SDK / v2 client-secret; C++ `authorize_request` + SDK/CS alternates (`pm_auth.cppm:540-570`) | Redis choices, PI verify, MCA auth, exchange-token + bank-creds calls, HMAC-shaped dedupe + PM insert/update | PARTIAL | Upstream `routes/pm_auth.rs:78-128`, `app.rs:1929`, `core/pm_auth.rs:240-312`; C++ `pm_auth.cppm:248-468,540-570,962-1026`, DTO `dto/pm_auth.cppm:63-81,12-18` |

### Route/verb note
Scope path matches: upstream `web::scope("/payment_methods")` (`app.rs:1854`) + `/auth/link` and `/auth/exchange` POST (`app.rs:1926,1929`); C++ `Router{router, "/payment_methods"}` + `/auth/link` and `/auth/exchange` POST (`pm_auth.cppm:474-496`). Verdict MATCH.

### Input note
Field sets align: link token (`dto/pm_auth.cppm:96-100`) vs upstream `api_models::pm_auth::LinkTokenCreateRequest` (`routes/pm_auth.rs:16`); exchange (`dto/pm_auth.cppm:76-80`) vs `ExchangeTokenCreateRequest` (`routes/pm_auth.rs:82`). C++ normalizes secret-string fields into `{"value": ...}` wrappers before typed deserialization (`pm_auth.cppm:101-112,241-257,316-325`) and overwrites `client_secret` from the authenticated claim on both paths (`pm_auth.cppm:120-124,333-337`).

### Output note
Link token response shape (`link_token`, `connector`) matches (`core/pm_auth.rs:206-209` ↔ `dto/pm_auth.cppm:20-29`; response built at `pm_auth.cppm:241-245`). Divergence: upstream `exchange_token_core` returns `RouterResponse<()>` / `ApplicationResponse::StatusOk` with no body (`core/pm_auth.rs:244,311`); C++ serializes `{access_token}` and returns 200 with body (`pm_auth.cppm:464-467`). Verdict PARTIAL.

### Auth note
Upstream v1 `check_sdk_auth_and_get_auth` / v2 `check_client_secret_and_get_auth` with `ApiKeyAuth::default()` (`routes/pm_auth.rs:20-45,86-111`), plus `auth.client_secret` overwriting `payload.client_secret` (`routes/pm_auth.rs:62-64,119-121`). C++ `authorize_request` applies the `auth_rules()` rule then retries SDK / CLIENT_SECRET / API_KEY / V2_API_KEY schemes with that rule's permission (`pm_auth.cppm:540-570,563-568`); the `client_secret` override is implemented via `auth.context.claim(kClaimClientSecret)` (`pm_auth.cppm:120-124,333-337`). Base rule remains permission-based, no `ApiKeyAuth::default()` — local substitute, verdict PARTIAL.

### Persistence note
C++ now verifies the stored PI via `repository().find_payment_by_string` (`pm_auth.cppm:579`), checks a 24h TTL key `pm_auth.pi_seen.<id>` (`pm_auth.cppm:601-618`) and HMAC-verifies the client secret (`pm_auth.cppm:620-637,153-159`); resolves MCA via `repository().find_merchant_connector_by_string` (`pm_auth.cppm:1177`) and connector auth via `key_store().get` (`pm_auth.cppm:751-765`); persists bank details with `repository().save_payment_method` / `update_payment_method` (`pm_auth.cppm:1006,1024`). Connector choices still come from in-memory cache keys `pm_auth_<id>` / `pm_auth.config.<id>` or MCA config, not Redis (`pm_auth.cppm:1105-1119,1133-1150`). Verdict PARTIAL.

### Domain logic note
Link: C++ derives `billing_country` from PI metadata/billing address or MCA business country (`pm_auth.cppm:689-715,180`), builds a connector payload with `client_name`, `language`, `country_codes`, `user_info` and header metadata `x-client-platform`/`x-app-id`/`x-redirect-uri` (`pm_auth.cppm:187-211`), calls `create_link` (`pm_auth.cppm:213-226`) with a synthetic `lt_` fallback (`pm_auth.cppm:227-231`). Exchange: `exchange_token` + `bank_account_credentials` connector calls (`pm_auth.cppm:392-424`), typed `BankCredential` normalization (`pm_auth.cppm:808-867,528-538`), HMAC-shaped dedupe hash, mask, connector merge and PM insert/update (`pm_auth.cppm:869-1026`). No true HMAC-SHA256 primitive (`pm_auth.cppm:881-883`); connector router-data/`ExternalConnectorError` mapping substituted by `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:771-799`). Verdict upgraded STUB → PARTIAL.

## Not ported
- `core/pm_auth.rs:105-110` — `verify_payment_intent_time_and_client_secret`: C++ never verifies the payment intent or client secret against a stored PI.
- `core/pm_auth.rs:112-129` — `billing_country` derivation from billing address; not present in C++ link-token request.
- `core/pm_auth.rs:131-155` — MCA lookup by merchant/conlector id and `get_connector_auth_type`; C++ has no MCA/auth-type resolution.
- `core/pm_auth.rs:170-176` — header metadata (`x_client_platform`, `x_app_id`, `x_redirect_uri`) forwarded to connector; absent in C++.
- `core/pm_auth.rs:224-238` — `ForeignTryFrom<&ConnectorAuthType> for PlaidAuthType` (BodyKey auth extraction); no equivalent.
- `core/pm_auth.rs:301-565` — `store_bank_details_in_payment_methods`: HMAC-SHA256 hash dedupe, mask generation, connector-detail merge, new/update PM construction; not ported.
- `core/pm_auth.rs:579-616` — `store_in_db`: bulk PM insert/update with `join_all`; not ported.
- `core/pm_auth.rs:674-724` — `get_access_token_from_exchange_api` router-data/connector-error mapping; C++ collapses this into a generic connector call.
- `core/pm_auth.rs:618-672` — `get_bank_account_creds` router-data and `ExternalConnectorError` mapping; no typed error mapping in C++.
- `core/pm_auth.rs:786-893` — `retrieve_payment_method_from_auth_service`: bank-account retrieval during payment confirm, balance sufficiency check (`PreconditionFailed`), and `PaymentMethodData` construction; no C++ equivalent.
- `core/pm_auth.rs:8-9` — `helpers` and `transformers` submodules (connector auth/config helpers, Plaid transformers); not present.
- `routes/pm_auth.rs:62-64,119-121` — `auth.client_secret` overriding request `client_secret`; C++ reads only the body value.
- `routes/pm_auth.rs:25-40` — SDK/client-secret API-key authentication scheme replaced by generic `PAYMENT_WRITE` permission.
- `core/pm_auth.rs:214-222,567-577,775-784` — upstream v2 variants are `todo!()`; parity not expected.

## Implementation disposition
Original `## Not ported` findings (checked 2026-11-19) re-verified against current `pm_auth.cppm`; this section supersedes those claims. Local substitutes keep related Summary verdicts at PARTIAL, not MATCH. External-dependency STUBs are preserved where justified.

- `verify_payment_intent_time_and_client_secret` (`pm_auth.rs:105-110`) — IMPLEMENTED — `verify_payment_intent` / `check_payment_intent_age` / `verify_client_secret` called on both paths (`pm_auth.cppm:137-159,351-366`; impl `572-637`) via `repository().find_payment_by_string` (`579`) + cache TTL/hash (`605-616,625-635`).
- `billing_country` derivation (`pm_auth.rs:112-129`) — SUBSTITUTED — `derive_billing_country` reads PI metadata/billing address or MCA `business_country`; no `billing_address_id` column (`pm_auth.cppm:689-715,180`).
- MCA lookup + `get_connector_auth_type` (`pm_auth.rs:131-155`) — IMPLEMENTED — `validate_merchant_connector` (`pm_auth.cppm:1167-1192`, calls `167-171,379-383`) + `resolve_connector_auth` (`717-769`, calls `181,389`), `repository().find_merchant_connector_by_string` (`1177`).
- Header metadata (`pm_auth.rs:170-176`) — IMPLEMENTED — `HeaderReader` reads `x-client-platform` / `x-app-id` / `x-redirect-uri` into connector payload (`pm_auth.cppm:184-211`).
- `ForeignTryFrom<&ConnectorAuthType> for PlaidAuthType` (`pm_auth.rs:224-238`) — SUBSTITUTED — `ConnectorAuth` extracts api_key/key1/client_id/secret + `body_key` flag with key_store fallback (`pm_auth.cppm:513-520,717-769,751-765`).
- `store_bank_details_in_payment_methods` (`pm_auth.rs:301-565`) — SUBSTITUTED — `store_bank_details`, `bank_hash` (KeyedHasher HMAC-shaped, no HMAC-SHA256 primitive), `mask_account`, `bank_details_json`, `merge_connectors` (`pm_auth.cppm:962-1026,869-884,886-910,924-960`); PM insert/update via repository (`1006,1024`).
- `store_in_db` bulk PM insert/update (`pm_auth.rs:579-616`) — SUBSTITUTED — synchronous per-credential loop calling `save_payment_method` / `update_payment_method`; no `join_all` (`pm_auth.cppm:990-1025`).
- `get_access_token_from_exchange_api` (`pm_auth.rs:674-724`) — SUBSTITUTED — `ConnectorRequestBuilder.operation("exchange_token")` + `connector->exchange_token` + `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:392-405,771-799`).
- `get_bank_account_creds` (`pm_auth.rs:618-672`) — SUBSTITUTED — `operation("bank_account_credentials")` + `authorize` + `connector_failure`, normalized into typed `BankCredential` (`pm_auth.cppm:407-431,529-538,808-867`); no `ExternalConnectorError` type.
- `retrieve_payment_method_from_auth_service` (`pm_auth.rs:786-893`) — DEFERRED — belongs to the payment-confirm path; not invoked by either handler, no balance-sufficiency/`PreconditionFailed` check here (`pm_auth.cppm:29-468`).
- `helpers` / `transformers` submodules (`pm_auth.rs:8-9`) — SUBSTITUTED — equivalent in-file helpers `resolve_connector_auth` (`pm_auth.cppm:717-769`), `parse_choice_config` (`1028-1094`), `parse_bank_credentials` (`808-867`).
- `auth.client_secret` override (`routes/pm_auth.rs:62-64,119-121`) — IMPLEMENTED — `auth.context.claim(kClaimClientSecret)` overwrites parsed secret on both handlers (`pm_auth.cppm:120-124,333-337`).
- SDK/client-secret API-key auth (`routes/pm_auth.rs:25-40`) — SUBSTITUTED — `authorize_request` retries SDK / CLIENT_SECRET / API_KEY / V2_API_KEY schemes over the registry rule permission; still permission-based, no `ApiKeyAuth::default()` (`pm_auth.cppm:540-570,563-568`).
- v2 `todo!()` variants (`pm_auth.rs:214-222,567-577,775-784`) — DEFERRED — no upstream implementation to port; parity not expected.
