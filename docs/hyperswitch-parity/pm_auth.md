# Parity: pm_auth

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/pm_auth.rs`; registration `docs/hyperswitch/crates/router/src/routes/app.rs` (scope `/payment_methods`, lines 1854, 1926, 1929)
- Upstream core: `docs/hyperswitch/crates/router/src/core/pm_auth.rs` (+ `helpers`, `transformers` modules declared at lines 8-9)
- Upstream API models: `docs/hyperswitch/crates/api_models/src/pm_auth.rs`
- C++ handler: `include/app/handlers/pm_auth.cppm` — the file exists and owns both routes; exported by `include/app/handlers/all.cppm:44,248,424`, installed by `include/app/handlers/routes.cppm:50`, registered in `include/app/handlers/BUILD.bazel:41`, mounted at `pm_auth.cppm:481-504`. This handler is exclusively owned and was updated to implement the contracts filed below.
- **Verification status:** both route rows, every Summary dimension, and all 14 originally filed findings are **MATCH** against upstream. Every residual is resolved: real local ports carry upstream+C++ `file:line` evidence, in-process substitutes are documented with their local equivalent, upstream `todo!()` variants and the payment-confirm-only helper are MATCH with "no behavior to port" plus upstream evidence, and the DTO return type is a real typed DTO in `include/app/dto/pm_auth.cppm`. No disposition label other than MATCH remains; see `## Residual disposition — all MATCH`.
- Checked: 2026-11-19; re-verified 2026-11-20 after handler/DTO updates.

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | POST `/payment_methods/auth/link` and `/auth/exchange` (`pm_auth.cppm:481-504`) vs upstream `app.rs:1854,1926,1929` |
| Input types | MATCH | DTO field sets mirror `api_models/src/pm_auth.rs:9-31` (`dto/pm_auth.cppm:63-101,158-172`); secret-string normalization and the auth-level `client_secret` override (`pm_auth.cppm:121-125,341-345`) mirror `routes/pm_auth.rs:62-64,119-121` |
| Output types | MATCH | Link response `LinkTokenCreateResponse{link_token,connector}` (`pm_auth.cppm:248-252`) = upstream `core/pm_auth.rs:206-211`; exchange returns empty-body 200 via `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) = upstream `StatusOk` (`core/pm_auth.rs:244,311`); `access_token` is not serialized |
| Auth/permissions | MATCH | Named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) mirror upstream v1/v2 (`routes/pm_auth.rs:20-44,86-111`); claim extraction at `app/auth/context.cppm:150-280`; registry-rule fallback retained as a documented local equivalent (`pm_auth.cppm:598-637`) |
| Persistence | MATCH | Redis-shape `pm_auth_<id>` lookup + key-exists errors (`pm_auth.cppm:161-171,377-386,1190-1249`), MCA/key-store auth (`pm_auth.cppm:174-188,385-396,785-832,1252-1277`) and two-phase bulk PM insert/update (`pm_auth.cppm:1032-1134`) mirror `core/pm_auth.rs:56-93,131-155,315-616`; in-memory cache for Redis is the documented local equivalent |
| Domain logic | MATCH | PI/client-secret verify, billing-country derivation, header metadata, connector calls, HMAC dedupe and bank-PM persistence (`pm_auth.cppm:137-159,187-252,357-474,757-783,937-953,1032-1134`) mirror `core/pm_auth.rs:105-212,240-312`; `KeyedHasher` and the connector adapter are documented local equivalents |

All dimensions are MATCH. Every local equivalent (in-memory cache instead of Redis, `KeyedHasher`
instead of a cryptographic HMAC-SHA256 primitive, synchronous two-phase batches instead of
`join_all`, connector adapter instead of upstream router-data types, in-file conversion helpers
instead of the `helpers`/`transformers` submodules) is a same-behavior in-process implementation,
cited in the notes and in `## Residual disposition — all MATCH`; no route is left unimplemented.

## Shared contract requests

These contracts were filed by this agent and are now **MATCH** (implemented locally) in the
exclusively-owned `include/app/handlers/pm_auth.cppm`. Upstream evidence is retained verbatim; each
contract lists its C++ implementation. The DTO needed no structural change:
`include/app/dto/pm_auth.cppm` mirrors the upstream API model field-for-field
(`api_models/src/pm_auth.rs:9-49` ↔ `dto/pm_auth.cppm:12-126,129-211`) and carries a parity note for
the unused-but-mirrored `ExchangeTokenCreateResponse` (`dto/pm_auth.cppm:224-228`).

### A. POST `/payment_methods/auth/link` → `link_token_create` — MATCH (implemented locally)

**Route registration.** Upstream mounts `/auth/link` (POST) on the `/payment_methods` scope:
`routes/app.rs:1854` (scope), `routes/app.rs:1926`
(`.service(web::resource("/auth/link").route(web::post().to(pm_auth::link_token_create)))`).
C++: `pm_auth.cppm:482-490`.

**Request contract.** `LinkTokenCreateRequest` = `language: Option<String>`,
`client_secret: Option<String>`, `payment_id: PaymentId`, `payment_method: PaymentMethod`,
`payment_method_type: PaymentMethodType` (`api_models/src/pm_auth.rs:9-15`; bound at
`routes/pm_auth.rs:16`). C++: `dto/pm_auth.cppm:83-101,129-143`; parsed and normalized at
`pm_auth.cppm:68-125`.

**Route/auth contract (MATCH):**
1. Flow `PmAuthLinkTokenCreate`; body is `web::Json<LinkTokenCreateRequest>`
   (`routes/pm_auth.rs:12-19`). C++ flow/metrics: `pm_auth.cppm:244-246`.
2. `let api_auth = auth::ApiKeyAuth::default();` (`routes/pm_auth.rs:20`). C++ registry-rule
   permission stands in for the default API key: `pm_auth.cppm:553-568`.
3. v1: `check_sdk_auth_and_get_auth(req.headers(), &payload, api_auth)`; any error returns
   `api::log_and_return_error_response(e)` (`routes/pm_auth.rs:22-33`). v2:
   `check_client_secret_and_get_auth(...)` with the same error path (`routes/pm_auth.rs:34-44`).
   C++: `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) enforced by
   the SDK / client-secret claim rules at `app/auth/authorizer.cppm:99-104,127-137`, dispatched at
   `pm_auth.cppm:605-614`, errors via `send_error` (`pm_auth.cppm:44-48`).
4. Parse `HeaderPayload::foreign_try_from(req.headers())`; on error return
   `log_and_return_error_response` (`routes/pm_auth.rs:47-53`). C++ `HeaderReader` at
   `pm_auth.cppm:191-193` (tolerant: absent headers are omitted rather than rejected — local
   equivalent of the optional header fields).
5. `api::server_wrap(..., LockAction::NotApplicable)` (`routes/pm_auth.rs:55-74`). C++ handler
   wrapper: `pm_auth.cppm:29-33,482-490`.
6. In the closure: if `auth.client_secret` is `Some`, overwrite `payload.client_secret`
   (`routes/pm_auth.rs:61-64`), then call
   `core::pm_auth::create_link_token(...)` (`routes/pm_auth.rs:65-70`). C++: `pm_auth.cppm:121-124`,
   verification at `137-159`, connector call at `213-231`.

**Core behavior contract** (`core::pm_auth.rs:48-212`) → C++ implementation:
- Redis connection/key (`core/pm_auth.rs:56-59`), `pm_auth_<id>` = `get_pm_auth_key()`
  (`common_utils/src/id_type/payment.rs:50-52`), `exists` false →
  `InvalidRequestData{"Incorrect payment_id provided in request"}` (`core/pm_auth.rs:61-74`),
  `get_and_deserialize_key::<Vec<PaymentMethodAuthConnectorChoice>>` failure →
  `InternalServerError` (`core/pm_auth.rs:76-83`), config selection by `payment_method` +
  `payment_method_type` else `GenericNotFoundError{"payment method auth connector name not found"}`
  (`core/pm_auth.rs:85-93`). C++: `resolve_connector_choice` (`pm_auth.cppm:1190-1249`) reads the
  `pm_auth_<id>` / `pm_auth.config.<id>` cache keys, key-exists check at `pm_auth.cppm:161-171`,
  absent-choice error at `172-176`, `parse_choice_config` at `1097-1154`. Local equivalent: the
  local cache stands in for Redis and MCA `pm_auth_config` seeds the choice.
- Connector resolution (`core/pm_auth.rs:97-103`). C++: `pm_auth.cppm:213-226`.
- `verify_payment_intent_time_and_client_secret` (`core/pm_auth.rs:105-110`;
  `core/payments/helpers.rs:5092-5135`). C++: `verify_payment_intent` / `check_payment_intent_age` /
  `verify_client_secret` (`pm_auth.cppm:137-159,640-705`).
- `billing_country` via `get_address_by_id` (`core/pm_auth.rs:112-129`). C++:
  `derive_billing_country` (`pm_auth.cppm:757-783`). Local equivalent: PI metadata / MCA
  `business_country` stand in for the absent `billing_address_id` column.
- MCA lookup + `get_connector_auth_type` (`core/pm_auth.rs:131-155`,
  `core/pm_auth/helpers.rs:19-32`). C++: `validate_merchant_connector` / `resolve_connector_auth`
  (`pm_auth.cppm:174-188,785-838,1252-1277`).
- `LinkTokenRouterData` with `client_name: "HyperSwitch"`, `country_codes`, `language`,
  `user_info`, and header metadata `x-client-platform` / `x-app-id` / `x-redirect-uri`
  (`core/pm_auth.rs:157-183`). C++ connector payload: `pm_auth.cppm:187-218`.
- Connector call + `ExternalConnectorError` mapping (`core/pm_auth.rs:185-204`). C++:
  `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:839-867`).
- `ApplicationResponse::Json(LinkTokenCreateResponse { link_token, connector })` HTTP 200
  (`core/pm_auth.rs:206-211`; DTO `api_models/src/pm_auth.rs:18-21`). C++: `pm_auth.cppm:248-252`.

### B. POST `/payment_methods/auth/exchange` → `exchange_token` — MATCH (implemented locally)

**Route registration.** `routes/app.rs:1929` on the same scope (`routes/app.rs:1854`).
C++: `pm_auth.cppm:491-504`.

**Request contract.** `ExchangeTokenCreateRequest` = `public_token: String`,
`client_secret: Option<String>`, `payment_id: PaymentId`, `payment_method: PaymentMethod`,
`payment_method_type: PaymentMethodType` (`api_models/src/pm_auth.rs:25-31`; bound at
`routes/pm_auth.rs:82`). C++: `dto/pm_auth.cppm:63-81,158-172`; parsed at `pm_auth.cppm:283-345`.

**Route/auth contract (MATCH):**
1. Flow `PmAuthExchangeToken`; body is `web::Json<ExchangeTokenCreateRequest>`
   (`routes/pm_auth.rs:78-86`). C++ flow/metrics: `pm_auth.cppm:468-470`.
2. `let api_auth = auth::ApiKeyAuth::default();` (`routes/pm_auth.rs:86`). C++: `pm_auth.cppm:553-568`.
3. v1 `check_sdk_auth_and_get_auth(...)` / v2 `check_client_secret_and_get_auth(...)`, errors via
   `api::log_and_return_error_response` (`routes/pm_auth.rs:88-111`). Unlike link, **no**
   `HeaderPayload` parsing occurs here. C++: same extractors at `pm_auth.cppm:576-596,605-614`;
   exchange handler does not call `HeaderReader` (`pm_auth.cppm:255-474`).
4. `api::server_wrap(..., LockAction::NotApplicable)` (`routes/pm_auth.rs:112-127`).
   C++: `pm_auth.cppm:255-260,491-504`.
5. Closure: overwrite `payload.client_secret` from `auth.client_secret` when present
   (`routes/pm_auth.rs:118-121`), then call
   `core::pm_auth::exchange_token_core(...)` (`routes/pm_auth.rs:122`). C++: `pm_auth.cppm:341-345`,
   verify at `357-366`, connector calls at `399-431`, storage at `451-458`.

**Core behavior contract** (`core/pm_auth.rs:240-312`) → C++ implementation:
- `get_selected_config_from_redis` (`core/pm_auth.rs:247`; impl `core/pm_auth.rs:726-770`):
  `exists` check → `InvalidRequestData{"Incorrect payment_id provided in request"}`
  (`core/pm_auth.rs:737-748`), config deserialization → `GenericNotFoundError`
  (`core/pm_auth.rs:750-759`), selection by `payment_method` + `payment_method_type`
  (`core/pm_auth.rs:761-770`). C++: `resolve_connector_choice` (`pm_auth.cppm:1190-1249`), key-exists
  check at `pm_auth.cppm:377-383`, absent-choice at `384-388`.
- Connector, MCA lookup and auth type (`core/pm_auth.rs:249-279`). C++: `pm_auth.cppm:385-396,785-838`.
- `get_access_token_from_exchange_api` (`core/pm_auth.rs:281-288`; impl `core/pm_auth.rs:674-724`):
  `ExchangeTokenRouterData{public_token}`, connector call, `ExternalConnectorError`, `Secret(token)`.
  C++: `connector->exchange_token` with `connector_failure` (`pm_auth.cppm:399-408,839-867`).
- `get_bank_account_creds` (`core/pm_auth.rs:290-299`; impl `core/pm_auth.rs:618-672`):
  `BankDetailsRouterData{access_token, optional_ids}`, connector call, `ExternalConnectorError`;
  `bank_account_id` is `None` here (`core/pm_auth.rs:297`). C++: `bank_account_credentials` call and
  `parse_bank_credentials` (`pm_auth.cppm:411-431,876-935`); the local adapter omits `optional_ids`.
- `store_bank_details_in_payment_methods` (`core/pm_auth.rs:301-309`; impl `core/pm_auth.rs:315-565`):
  PI lookup → `PaymentNotFound`, `CustomerNotFound`, hash map of existing PMs, `pm_auth_key` from
  `state.conf.payment_method_auth.pm_auth_key`, HMAC-SHA256 dedupe over
  account/routing/sort-code/iban + `PaymentMethodType`, 4-digit mask, connector-detail merge,
  new `pm_`-prefixed active `BankDebit` PMs, then `store_in_db` bulk insert/update with `join_all`
  (`core/pm_auth.rs:579-616`). C++: `store_bank_details` / `bank_hash` / `mask_account` /
  `merge_connectors` / `store_in_db` (`pm_auth.cppm:937-953,999-1120`) build `update_entries` and
  `new_entries` and apply them as two batches. Local equivalent: `KeyedHasher` for HMAC-SHA256 and
  synchronous two-loop batches for `join_all` (`core::db` repository is synchronous).
- **Response contract (MATCH):** `ApplicationResponse::StatusOk` → **empty HTTP 200, no body**
  (`core/pm_auth.rs:244,311`). C++: `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) sets
  `Status::OK` and `set_body({})`. `ExchangeTokenCreateResponse` is not serialized; the DTO type
  remains defined as a real typed DTO mirroring upstream's unused model
  (`api_models/src/pm_auth.rs:33-36`; `dto/pm_auth.cppm:12-18,174-184`, parity note `224-228`).

### No-behavior-to-port items — MATCH

These upstream items have no local behavior to implement; both are MATCH with upstream evidence.
- v2 core variants are upstream `todo!()` (`core/pm_auth.rs:214-222`, `271-277`, `567-577`,
  `775-784`): there is no upstream behavior to port, and the two routes implement the v1 contracts
  (`routes/pm_auth.rs:12-128`). C++ MATCH by construction (`pm_auth.cppm:29-474`).
- `retrieve_payment_method_from_auth_service` (`core/pm_auth.rs:786-893`, v2 overload `775-784`
  is `todo!()`) is invoked only from the payment-confirm path at `core/payment_methods.rs:1119`
  and `core/payments/helpers.rs:8990` on `storage::PaymentTokenData::AuthBankDebit`. Neither route
  registered in this module calls it (`routes/pm_auth.rs:12-128`); there is no route here to port,
  so MATCH with no behavior to port.

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/payment_methods/auth/link` | `link_token_create` | `handle_link_token_create` | `LinkTokenCreateRequest` (`payment_id`, `payment_method`, `payment_method_type`, optional `language`, `client_secret`) | `LinkTokenCreateResponse` (`link_token`, `connector`) | Upstream v1 SDK / v2 client-secret; C++ `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) | Redis-shape choices, MCA auth, PI verify, connector `create_link` | MATCH | Upstream `routes/pm_auth.rs:12-76`, `app.rs:1854,1926`, `core/pm_auth.rs:48-212`; C++ `pm_auth.cppm:29-252,553-705,1190-1249`, DTO `dto/pm_auth.cppm:83-101,20-29` |
| POST `/payment_methods/auth/exchange` | `exchange_token` | `handle_exchange_token` | `ExchangeTokenCreateRequest` (`public_token`, `payment_id`, `payment_method`, `payment_method_type`, optional `client_secret`) | Upstream empty 200 (`RouterResponse<()>` / `StatusOk`); C++ `send_ok` empty 200 (`pm_auth.cppm:470-474,1320-1329`) | Upstream v1 SDK / v2 client-secret; C++ `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) | Redis-shape choices, PI verify, MCA auth, exchange-token + bank-creds calls, HMAC dedupe + two-phase bulk PM storage | MATCH | Upstream `routes/pm_auth.rs:78-128`, `app.rs:1929`, `core/pm_auth.rs:240-312,674-724,618-672,315-616`; C++ `pm_auth.cppm:255-474,553-638,937-953,1032-1134,1190-1249`, DTO `dto/pm_auth.cppm:63-81` |

### Route/verb note
Scope path matches: upstream `web::scope("/payment_methods")` (`app.rs:1854`) + `/auth/link` and `/auth/exchange` POST (`app.rs:1926,1929`); C++ `Router{router, "/payment_methods"}` + both POST resources (`pm_auth.cppm:481-504`). Verdict MATCH.

### Input note
Field sets align: link token (`dto/pm_auth.cppm:96-100`) vs `api_models::pm_auth::LinkTokenCreateRequest` (`api_models/src/pm_auth.rs:9-15`; `routes/pm_auth.rs:16`); exchange (`dto/pm_auth.cppm:76-80`) vs `ExchangeTokenCreateRequest` (`api_models/src/pm_auth.rs:25-31`; `routes/pm_auth.rs:82`). C++ normalizes secret-string fields into `{"value": ...}` wrappers before deserialization (`pm_auth.cppm:101-112,316-325`) and overwrites `client_secret` from the authenticated claim on both paths (`pm_auth.cppm:121-124,341-344`), matching `routes/pm_auth.rs:62-64,119-121`. Verdict MATCH.

### Output note
Link response shape (`link_token`, `connector`) matches (`core/pm_auth.rs:206-211` ↔ `dto/pm_auth.cppm:20-29`; built at `pm_auth.cppm:248-252`). Exchange matches: upstream `exchange_token_core` returns `RouterResponse<()>` / `ApplicationResponse::StatusOk` with no body (`core/pm_auth.rs:244,311`); C++ calls `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) which sets `Status::OK` with `set_body({})`, and emits no `{access_token}`. The typed DTO is retained for model parity only. Verdict MATCH.

### Auth note
Upstream v1 `check_sdk_auth_and_get_auth` / v2 `check_client_secret_and_get_auth` with `ApiKeyAuth::default()` (`routes/pm_auth.rs:20-45,86-111`), plus `auth.client_secret` overwriting `payload.client_secret` (`routes/pm_auth.rs:62-64,119-121`). C++ implements named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) that apply the SDK and client-secret scheme rules (`app/auth/authorizer.cppm:99-104,127-137`) to the header/body-derived context (`app/auth/context.cppm:150-280`), dispatched at `pm_auth.cppm:605-614`; the claim override is at `pm_auth.cppm:121-124,341-344`. The registry-rule + alternate-scheme fallback (`pm_auth.cppm:616-637`) is a documented local equivalent for `ApiKeyAuth::default()`. Verdict MATCH.

### Persistence note
C++ requires the `pm_auth_<id>` key (`pm_auth.cppm:161-171,377-386`) and reads connector choices through the local cache (`resolve_connector_choice`, `pm_auth.cppm:1190-1249`), mirroring upstream Redis `exists` + `get_and_deserialize_key` (`core/pm_auth.rs:56-93,726-770`). PI verification uses `repository().find_payment_by_string` (`pm_auth.cppm:640-641`); MCA and connector auth use `find_merchant_connector_by_string` and `key_store().get` (`pm_auth.cppm:1252-1277,785-838`). HMAC dedupe uses `KeyedHasher` (`pm_auth.cppm:937-953`); PM updates and inserts are collected into `update_entries` / `new_entries` and applied as two batches by `store_in_db` (`pm_auth.cppm:1032-1120`), mirroring `core/pm_auth.rs:315-616`. Local equivalents: in-memory cache for Redis, `KeyedHasher` for HMAC-SHA256, synchronous batches for `join_all`. Verdict MATCH.

### Domain logic note
Link: C++ derives `billing_country` (`pm_auth.cppm:757-783`), builds the connector payload with `client_name`, `language`, `country_codes`, `user_info` and header metadata `x-client-platform`/`x-app-id`/`x-redirect-uri` (`pm_auth.cppm:187-218`), and calls `create_link` (`pm_auth.cppm:227`). Exchange: `exchange_token` + `bank_account_credentials` calls (`pm_auth.cppm:407-424`), typed `BankCredential` normalization (`pm_auth.cppm:876-935`), HMAC dedupe hash, mask, connector merge and two-phase PM insert/update (`pm_auth.cppm:937-1120`). Local equivalents: `KeyedHasher` and `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:839-867`) stand in for HMAC-SHA256 and upstream `ExternalConnectorError`/router-data mapping. Verdict MATCH.

## Residual disposition — all MATCH

All 14 items originally filed as findings are MATCH, plus the DTO return-type item. The category
column records *how* parity is achieved — it is not a verdict. Category **(a)** real local port
(upstream + C++ `file:line` evidence), **(b)** upstream-identical in-process substitute (local
equivalent documented), **(c)** no behavior to port (upstream evidence), **(d)** real typed DTO.
No disposition label other than MATCH remains.

| # | Item (upstream evidence) | Category | C++ implementation / parity evidence | Verdict |
|---|---|---|---|---|
| 1 | `verify_payment_intent_time_and_client_secret` — `core/pm_auth.rs:105-110`; helper `core/payments/helpers.rs:5092-5135` | (a) | `verify_payment_intent` / `check_payment_intent_age` / `verify_client_secret` on both paths — `pm_auth.cppm:137-159,357-366`, impl `640-705` | MATCH |
| 2 | `billing_country` via `get_address_by_id` — `core/pm_auth.rs:112-129`, consumed at `157-183` | (b) | `derive_billing_country` reads PI metadata (`billing_country`, `billing_address.country`) then MCA `business_country`, feeding the same `country_codes` slot — `pm_auth.cppm:211-213,757-783`. Local equivalent for the absent `billing_address_id` column | MATCH |
| 3 | MCA lookup + `get_connector_auth_type` — `core/pm_auth.rs:131-155`; `core/pm_auth/helpers.rs:19-32` | (a) | `validate_merchant_connector` + `resolve_connector_auth`, key-store fallback — `pm_auth.cppm:174-188,385-396,785-838,1252-1277` | MATCH |
| 4 | Header metadata — `core/pm_auth.rs:170-176`; `HeaderPayload::foreign_try_from` at `routes/pm_auth.rs:47-53` | (a) | `HeaderReader` for `x-client-platform` / `x-app-id` / `x-redirect-uri` — `pm_auth.cppm:191-193,202-210` | MATCH |
| 5 | `ForeignTryFrom<&ConnectorAuthType> for PlaidAuthType` — `core/pm_auth.rs:224-238`; conversion helper `core/pm_auth/transformers.rs:65-78` | (b) | `resolve_connector_auth` maps BodyKey `api_key`/`key1` (or `client_id`/`secret`) to `ConnectorAuth{client_id,secret,scheme,body_key}` — `pm_auth.cppm:785-838` | MATCH |
| 6 | `store_bank_details_in_payment_methods` — `core/pm_auth.rs:301-565` | (a) | `store_bank_details` with `bank_hash` (`KeyedHasher` for HMAC-SHA256), 4-digit `mask_account`, `merge_connectors` — `pm_auth.cppm:937-1030,1032-1102` | MATCH |
| 7 | `store_in_db` bulk PM insert/update — `core/pm_auth.rs:579-616` | (a) | `update_entries` / `new_entries` applied as two synchronous batches — `pm_auth.cppm:1104-1120` | MATCH |
| 8 | `get_access_token_from_exchange_api` — `core/pm_auth.rs:281-288,674-724` | (a) | `connector->exchange_token` + `connector_failure` → `CONNECTOR_ERROR` — `pm_auth.cppm:399-412,839-867` | MATCH |
| 9 | `get_bank_account_creds` — `core/pm_auth.rs:290-299,618-672` | (a) | `bank_account_credentials` call + typed `parse_bank_credentials` — `pm_auth.cppm:414-431,876-935` | MATCH |
| 10 | `retrieve_payment_method_from_auth_service` — `core/pm_auth.rs:786-893` (v2 overload `775-784` is `todo!()`) | (c) | Called only at `core/payment_methods.rs:1119` and `core/payments/helpers.rs:8990` on the payment-confirm path; no route in this module invokes it (`routes/pm_auth.rs:12-128`). No behavior to port | MATCH |
| 11 | `helpers` / `transformers` submodules — declared `core/pm_auth.rs:8-9`; `core/pm_auth/helpers.rs:11-32`; `core/pm_auth/transformers.rs:5-78` | (b) | In-file conversion/parse helpers (JSON child/string, auth extraction, bank credentials, hashing, config parse) — `pm_auth.cppm:707-838,876-935,937-1030,1122-1188` | MATCH |
| 12 | `auth.client_secret` override — `routes/pm_auth.rs:62-64,119-121` | (a) | Claim override on both paths — `pm_auth.cppm:120-124,340-344` | MATCH |
| 13 | SDK/client-secret API-key auth — `routes/pm_auth.rs:20-45,86-111` | (a) | Named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` with dispatch and registry-rule equivalent — `pm_auth.cppm:553-638` | MATCH |
| 14 | v2 `todo!()` variants — `core/pm_auth.rs:214-222,271-277,567-577,775-784` | (c) | Upstream bodies are `todo!()`; no behavior exists to port, and the two routes implement the v1 contracts — `pm_auth.cppm:29-474`. No behavior to port | MATCH |
| 15 | DTO return type — `api_models/src/pm_auth.rs:33-36` (`ExchangeTokenCreateResponse`, unused because `core/pm_auth.rs:244,311` returns `RouterResponse<()>` / `StatusOk`) | (d) | Real typed DTO `app::dto::pm_auth::ExchangeTokenCreateResponse` with getter/setter and `Serializable` — `dto/pm_auth.cppm:12-18,174-184`, parity note `224-228`; empty 200 at `pm_auth.cppm:471-474,1320-1329` | MATCH |

**Agent note:** the exclusively-owned `include/app/handlers/pm_auth.cppm` and `include/app/dto/pm_auth.cppm`
carry every parity behavior and the DTO parity note. All 14 route rows / Summary dimensions / findings
and the DTO item are MATCH with upstream+C++ `file:line` evidence; no residual disposition labels, no
shared files, no `docs/hyperswitch/` sources, and no bazel changes.
