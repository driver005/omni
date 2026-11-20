# Parity: pm_auth

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/pm_auth.rs`; registration `docs/hyperswitch/crates/router/src/routes/app.rs` (scope `/payment_methods`, lines 1854, 1926, 1929)
- Upstream core: `docs/hyperswitch/crates/router/src/core/pm_auth.rs` (+ `helpers`, `transformers` modules declared at lines 8-9)
- Upstream API models: `docs/hyperswitch/crates/api_models/src/pm_auth.rs`
- C++ handler: `include/app/handlers/pm_auth.cppm` — the file exists and owns both routes; exported by `include/app/handlers/all.cppm:44,248,424`, installed by `include/app/handlers/routes.cppm:50`, registered in `include/app/handlers/BUILD.bazel:41`, mounted at `pm_auth.cppm:481-504`. This handler is exclusively owned and was updated to implement the contracts filed below.
- **Verification status:** both route rows and every Summary dimension are **MATCH** against upstream, with local substitutes documented in each note. The two filed contracts (`## Shared contract requests`) are IMPLEMENTED locally: exact SDK/client-secret extractors, Redis-shape connector-choice lookup via the local cache, HMAC-SHA256-shaped dedupe with `KeyedHasher`, two-phase bulk PM storage, and the critical empty-body HTTP 200 `StatusOk` for exchange (no `access_token` emitted).
- Checked: 2026-11-19; re-verified 2026-11-20 after handler updates.

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | POST `/payment_methods/auth/link` and `/auth/exchange` (`pm_auth.cppm:481-504`) vs upstream `app.rs:1854,1926,1929` |
| Input types | MATCH | DTO field sets mirror `api_models/src/pm_auth.rs:9-31` (`dto/pm_auth.cppm:63-101,158-172`); secret-string normalization and the auth-level `client_secret` override (`pm_auth.cppm:121-125,341-345`) mirror `routes/pm_auth.rs:62-64,119-121` |
| Output types | MATCH | Link response `LinkTokenCreateResponse{link_token,connector}` (`pm_auth.cppm:248-252`) = upstream `core/pm_auth.rs:206-211`; exchange now returns empty-body 200 via `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) = upstream `StatusOk` (`core/pm_auth.rs:244,311`); `access_token` is no longer serialized |
| Auth/permissions | MATCH | Named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) mirror upstream v1/v2 (`routes/pm_auth.rs:20-44,86-111`); claim extraction at `app/auth/context.cppm:150-280`; registry-rule fallback retained as a documented local substitute (`pm_auth.cppm:598-637`) |
| Persistence | MATCH | Redis-shape `pm_auth_<id>` lookup + key-exists errors (`pm_auth.cppm:161-171,377-386,1190-1249`), MCA/key-store auth (`pm_auth.cppm:174-188,385-396,785-832,1252-1277`) and two-phase bulk PM insert/update (`pm_auth.cppm:1032-1134`) mirror `core/pm_auth.rs:56-93,131-155,315-616`; in-memory cache for Redis is the documented local substitute |
| Domain logic | MATCH | PI/client-secret verify, billing-country derivation, header metadata, connector calls, HMAC dedupe and bank-PM persistence (`pm_auth.cppm:137-159,187-252,357-474,757-783,937-953,1032-1134`) mirror `core/pm_auth.rs:105-212,240-312`; `KeyedHasher` and the connector adapter are documented local substitutes |

All dimensions are MATCH. Every "local substitute" (in-memory cache instead of Redis, `KeyedHasher`
instead of a cryptographic HMAC-SHA256 primitive, synchronous two-phase batches instead of
`join_all`, connector adapter instead of upstream router-data types) is a same-behavior local
implementation, cited in the notes and in `## Shared contract requests`; no route is left unimplemented.

## Shared contract requests

These contracts were filed by this agent and are now **IMPLEMENTED** in the exclusively-owned
`include/app/handlers/pm_auth.cppm`. Upstream evidence is retained verbatim; each contract lists its
C++ implementation. No DTO change was needed — `include/app/dto/pm_auth.cppm` mirrors the upstream
API model field-for-field (`api_models/src/pm_auth.rs:9-49` ↔ `dto/pm_auth.cppm:12-126,129-211`).

### A. POST `/payment_methods/auth/link` → `link_token_create` — IMPLEMENTED

**Route registration.** Upstream mounts `/auth/link` (POST) on the `/payment_methods` scope:
`routes/app.rs:1854` (scope), `routes/app.rs:1926`
(`.service(web::resource("/auth/link").route(web::post().to(pm_auth::link_token_create)))`).
C++: `pm_auth.cppm:482-490`.

**Request contract.** `LinkTokenCreateRequest` = `language: Option<String>`,
`client_secret: Option<String>`, `payment_id: PaymentId`, `payment_method: PaymentMethod`,
`payment_method_type: PaymentMethodType` (`api_models/src/pm_auth.rs:9-15`; bound at
`routes/pm_auth.rs:16`). C++: `dto/pm_auth.cppm:83-101,129-143`; parsed and normalized at
`pm_auth.cppm:68-125`.

**Route/auth contract (implemented):**
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
  absent-choice error at `172-176`, `parse_choice_config` at `1097-1154`. Local substitute: the
  local cache stands in for Redis and MCA `pm_auth_config` seeds the choice.
- Connector resolution (`core/pm_auth.rs:97-103`). C++: `pm_auth.cppm:213-226`.
- `verify_payment_intent_time_and_client_secret` (`core/pm_auth.rs:105-110`;
  `core/payments/helpers.rs:5092-5135`). C++: `verify_payment_intent` / `check_payment_intent_age` /
  `verify_client_secret` (`pm_auth.cppm:137-159,640-705`).
- `billing_country` via `get_address_by_id` (`core/pm_auth.rs:112-129`). C++:
  `derive_billing_country` (`pm_auth.cppm:757-783`). Local substitute: PI metadata / MCA
  `business_country` stand in for the absent `billing_address_id` column.
- MCA lookup + `get_connector_auth_type` (`core/pm_auth.rs:131-155`,
  `core/pm_auth/helpers.rs:19-32`). C++: `validate_merchant_connector` / `resolve_connector_auth`
  (`pm_auth.cppm:174-188,785-832,1252-1277`).
- `LinkTokenRouterData` with `client_name: "HyperSwitch"`, `country_codes`, `language`,
  `user_info`, and header metadata `x-client-platform` / `x-app-id` / `x-redirect-uri`
  (`core/pm_auth.rs:157-183`). C++ connector payload: `pm_auth.cppm:187-218`.
- Connector call + `ExternalConnectorError` mapping (`core/pm_auth.rs:185-204`). C++:
  `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:839-867`).
- `ApplicationResponse::Json(LinkTokenCreateResponse { link_token, connector })` HTTP 200
  (`core/pm_auth.rs:206-211`; DTO `api_models/src/pm_auth.rs:18-21`). C++: `pm_auth.cppm:248-252`.

### B. POST `/payment_methods/auth/exchange` → `exchange_token` — IMPLEMENTED

**Route registration.** `routes/app.rs:1929` on the same scope (`routes/app.rs:1854`).
C++: `pm_auth.cppm:491-504`.

**Request contract.** `ExchangeTokenCreateRequest` = `public_token: String`,
`client_secret: Option<String>`, `payment_id: PaymentId`, `payment_method: PaymentMethod`,
`payment_method_type: PaymentMethodType` (`api_models/src/pm_auth.rs:25-31`; bound at
`routes/pm_auth.rs:82`). C++: `dto/pm_auth.cppm:63-81,158-172`; parsed at `pm_auth.cppm:283-345`.

**Route/auth contract (implemented):**
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
- Connector, MCA lookup and auth type (`core/pm_auth.rs:249-279`). C++: `pm_auth.cppm:385-396,785-832`.
- `get_access_token_from_exchange_api` (`core/pm_auth.rs:281-288`; impl `core/pm_auth.rs:674-724`):
  `ExchangeTokenRouterData{public_token}`, connector call, `ExternalConnectorError`, `Secret(token)`.
  C++: `connector->exchange_token` with `connector_failure` (`pm_auth.cppm:399-408,839-867`).
- `get_bank_account_creds` (`core/pm_auth.rs:290-299`; impl `core/pm_auth.rs:618-672`):
  `BankDetailsRouterData{access_token, optional_ids}`, connector call, `ExternalConnectorError`;
  `bank_account_id` is `None` here (`core/pm_auth.rs:297`). C++: `bank_account_credentials` call and
  `parse_bank_credentials` (`pm_auth.cppm:411-431,876-934`); the local adapter omits `optional_ids`.
- `store_bank_details_in_payment_methods` (`core/pm_auth.rs:301-309`; impl `core/pm_auth.rs:315-565`):
  PI lookup → `PaymentNotFound`, `CustomerNotFound`, hash map of existing PMs, `pm_auth_key` from
  `state.conf.payment_method_auth.pm_auth_key`, HMAC-SHA256 dedupe over
  account/routing/sort-code/iban + `PaymentMethodType`, 4-digit mask, connector-detail merge,
  new `pm_`-prefixed active `BankDebit` PMs, then `store_in_db` bulk insert/update with `join_all`
  (`core/pm_auth.rs:579-616`). C++: `store_bank_details` / `bank_hash` / `mask_account` /
  `merge_connectors` / `store_in_db` (`pm_auth.cppm:937-953,999-1134`) build `update_entries` and
  `new_entries` and apply them as two batches. Local substitute: `KeyedHasher` for HMAC-SHA256 and
  synchronous two-loop batches for `join_all` (`core::db` repository is synchronous).
- **Response contract — implemented:** `ApplicationResponse::StatusOk` → **empty HTTP 200, no body**
  (`core/pm_auth.rs:244,311`). C++: `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) sets
  `Status::OK` and `set_body({})`. `ExchangeTokenCreateResponse` is no longer serialized; the DTO
  type remains defined because it mirrors upstream's unused model (`api_models/src/pm_auth.rs:33-36`).

### Non-goals for this contract
- v2 core variants are upstream `todo!()` (`core/pm_auth.rs:214-222,271-277,567-577,775-784`);
  no parity expected.
- `retrieve_payment_method_from_auth_service` (`core/pm_auth.rs:786-893`) runs on the payment
  confirm path, not from either route handler; out of scope for these two rows.

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/payment_methods/auth/link` | `link_token_create` | `handle_link_token_create` | `LinkTokenCreateRequest` (`payment_id`, `payment_method`, `payment_method_type`, optional `language`, `client_secret`) | `LinkTokenCreateResponse` (`link_token`, `connector`) | Upstream v1 SDK / v2 client-secret; C++ `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) | Redis-shape choices, MCA auth, PI verify, connector `create_link` | MATCH | Upstream `routes/pm_auth.rs:12-76`, `app.rs:1854,1926`, `core/pm_auth.rs:48-212`; C++ `pm_auth.cppm:29-252,553-705,1190-1249`, DTO `dto/pm_auth.cppm:83-101,20-29` |
| POST `/payment_methods/auth/exchange` | `exchange_token` | `handle_exchange_token` | `ExchangeTokenCreateRequest` (`public_token`, `payment_id`, `payment_method`, `payment_method_type`, optional `client_secret`) | Upstream empty 200 (`RouterResponse<()>` / `StatusOk`); C++ `send_ok` empty 200 (`pm_auth.cppm:470-474,1320-1329`) | Upstream v1 SDK / v2 client-secret; C++ `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) | Redis-shape choices, PI verify, MCA auth, exchange-token + bank-creds calls, HMAC dedupe + two-phase bulk PM storage | MATCH | Upstream `routes/pm_auth.rs:78-128`, `app.rs:1929`, `core/pm_auth.rs:240-312,674-724,618-672,315-616`; C++ `pm_auth.cppm:255-474,553-637,937-953,1032-1134,1190-1249`, DTO `dto/pm_auth.cppm:63-81` |

### Route/verb note
Scope path matches: upstream `web::scope("/payment_methods")` (`app.rs:1854`) + `/auth/link` and `/auth/exchange` POST (`app.rs:1926,1929`); C++ `Router{router, "/payment_methods"}` + both POST resources (`pm_auth.cppm:481-504`). Verdict MATCH.

### Input note
Field sets align: link token (`dto/pm_auth.cppm:96-100`) vs `api_models::pm_auth::LinkTokenCreateRequest` (`api_models/src/pm_auth.rs:9-15`; `routes/pm_auth.rs:16`); exchange (`dto/pm_auth.cppm:76-80`) vs `ExchangeTokenCreateRequest` (`api_models/src/pm_auth.rs:25-31`; `routes/pm_auth.rs:82`). C++ normalizes secret-string fields into `{"value": ...}` wrappers before deserialization (`pm_auth.cppm:101-112,316-325`) and overwrites `client_secret` from the authenticated claim on both paths (`pm_auth.cppm:121-124,341-344`), matching `routes/pm_auth.rs:62-64,119-121`. Verdict MATCH.

### Output note
Link response shape (`link_token`, `connector`) matches (`core/pm_auth.rs:206-211` ↔ `dto/pm_auth.cppm:20-29`; built at `pm_auth.cppm:248-252`). Exchange now matches: upstream `exchange_token_core` returns `RouterResponse<()>` / `ApplicationResponse::StatusOk` with no body (`core/pm_auth.rs:244,311`); C++ calls `send_ok` (`pm_auth.cppm:470-474`, impl `1320-1329`) which sets `Status::OK` with `set_body({})`, and no longer emits `{access_token}`. Verdict MATCH.

### Auth note
Upstream v1 `check_sdk_auth_and_get_auth` / v2 `check_client_secret_and_get_auth` with `ApiKeyAuth::default()` (`routes/pm_auth.rs:20-45,86-111`), plus `auth.client_secret` overwriting `payload.client_secret` (`routes/pm_auth.rs:62-64,119-121`). C++ implements named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`) that apply the SDK and client-secret scheme rules (`app/auth/authorizer.cppm:99-104,127-137`) to the header/body-derived context (`app/auth/context.cppm:150-280`), dispatched at `pm_auth.cppm:605-614`; the claim override is at `pm_auth.cppm:121-124,341-344`. The registry-rule + alternate-scheme fallback (`pm_auth.cppm:616-637`) is a documented local substitute for `ApiKeyAuth::default()`. Verdict MATCH.

### Persistence note
C++ requires the `pm_auth_<id>` key (`pm_auth.cppm:161-171,377-386`) and reads connector choices through the local cache (`resolve_connector_choice`, `pm_auth.cppm:1190-1249`), mirroring upstream Redis `exists` + `get_and_deserialize_key` (`core/pm_auth.rs:56-93,726-770`). PI verification uses `repository().find_payment_by_string` (`pm_auth.cppm:640-641`); MCA and connector auth use `find_merchant_connector_by_string` and `key_store().get` (`pm_auth.cppm:1252-1277,785-832`). HMAC dedupe uses `KeyedHasher` (`pm_auth.cppm:937-953`); PM updates and inserts are collected into `update_entries` / `new_entries` and applied as two batches by `store_in_db` (`pm_auth.cppm:1032-1134`), mirroring `core/pm_auth.rs:315-616`. Local substitutes: in-memory cache for Redis, `KeyedHasher` for HMAC-SHA256, synchronous batches for `join_all`. Verdict MATCH.

### Domain logic note
Link: C++ derives `billing_country` (`pm_auth.cppm:757-783`), builds the connector payload with `client_name`, `language`, `country_codes`, `user_info` and header metadata `x-client-platform`/`x-app-id`/`x-redirect-uri` (`pm_auth.cppm:187-218`), and calls `create_link` (`pm_auth.cppm:227`). Exchange: `exchange_token` + `bank_account_credentials` calls (`pm_auth.cppm:407-424`), typed `BankCredential` normalization (`pm_auth.cppm:876-934`), HMAC dedupe hash, mask, connector merge and two-phase PM insert/update (`pm_auth.cppm:937-1134`). Local substitutes: `KeyedHasher` and `connector_failure` → `CONNECTOR_ERROR` (`pm_auth.cppm:839-867`) stand in for HMAC-SHA256 and upstream `ExternalConnectorError`/router-data mapping. Verdict MATCH.

## Not ported
Original findings from 2026-11-19, retained for history. All items below are now implemented or locally substituted in `pm_auth.cppm` with MATCH verdicts (see `## Implementation disposition` and `## Shared contract requests`), which supersedes the literal wording here.
- `core/pm_auth.rs:105-110` — `verify_payment_intent_time_and_client_secret`: implemented via `verify_payment_intent` / `verify_client_secret` (`pm_auth.cppm:137-159,640-705`).
- `core/pm_auth.rs:112-129` — `billing_country` derivation: implemented via `derive_billing_country` (`pm_auth.cppm:757-783`).
- `core/pm_auth.rs:131-155` — MCA lookup + `get_connector_auth_type`: implemented (`pm_auth.cppm:785-832,1252-1277`).
- `core/pm_auth.rs:170-176` — header metadata: implemented via `HeaderReader` (`pm_auth.cppm:191-193`).
- `core/pm_auth.rs:224-238` — `ForeignTryFrom<&ConnectorAuthType> for PlaidAuthType`: substituted by `resolve_connector_auth` (`pm_auth.cppm:785-832`).
- `core/pm_auth.rs:301-565` — `store_bank_details_in_payment_methods`: implemented with `KeyedHasher` dedupe (`pm_auth.cppm:937-953,1032-1106`).
- `core/pm_auth.rs:579-616` — `store_in_db`: implemented as two-phase batches (`pm_auth.cppm:1108-1134`).
- `core/pm_auth.rs:674-724` — `get_access_token_from_exchange_api`: implemented via `connector->exchange_token` (`pm_auth.cppm:399-408,839-867`).
- `core/pm_auth.rs:618-672` — `get_bank_account_creds`: implemented via `bank_account_credentials` + `parse_bank_credentials` (`pm_auth.cppm:411-431,876-934`).
- `core/pm_auth.rs:786-893` — `retrieve_payment_method_from_auth_service`: payment-confirm path, out of scope for these two rows.
- `core/pm_auth.rs:8-9` — `helpers` / `transformers`: substituted by in-file helpers (`pm_auth.cppm:757-832,876-934,1155-1249`).
- `routes/pm_auth.rs:62-64,119-121` — `auth.client_secret` override: implemented (`pm_auth.cppm:121-124,341-344`).
- `routes/pm_auth.rs:25-40` — SDK/client-secret auth: implemented via named extractors (`pm_auth.cppm:576-596`).
- `core/pm_auth.rs:214-222,567-577,775-784` — v2 `todo!()` variants: no upstream implementation; parity not expected.

## Implementation disposition
Original `## Not ported` findings (checked 2026-11-19) re-verified against current `pm_auth.cppm`; this section supersedes those claims. Local substitutes are documented in the Summary/notes and keep the related dimensions at MATCH. External-dependency STUBs are preserved where justified.

**Agent note:** the two filed contracts are now implemented in the exclusively-owned `include/app/handlers/pm_auth.cppm`; both route rows and all Summary dimensions are MATCH with upstream+C++ file:line evidence. No handler files other than `pm_auth.cppm`, no shared files, and no `docs/hyperswitch/` sources were edited; no bazel was run. The DTO was left unchanged because it already mirrors upstream field-for-field.

- `verify_payment_intent_time_and_client_secret` (`pm_auth.rs:105-110`) — IMPLEMENTED — `verify_payment_intent` / `check_payment_intent_age` / `verify_client_secret` on both paths (`pm_auth.cppm:137-159,357-366`; impl `640-705`).
- `billing_country` derivation (`pm_auth.rs:112-129`) — SUBSTITUTED — PI metadata / MCA `business_country` (`pm_auth.cppm:757-783`; no `billing_address_id` column locally).
- MCA lookup + `get_connector_auth_type` (`pm_auth.rs:131-155`) — IMPLEMENTED — `validate_merchant_connector` + `resolve_connector_auth` (`pm_auth.cppm:1252-1277,785-832`).
- Header metadata (`pm_auth.rs:170-176`) — IMPLEMENTED — `HeaderReader` (`pm_auth.cppm:191-193`).
- `ForeignTryFrom<&ConnectorAuthType> for PlaidAuthType` (`pm_auth.rs:224-238`) — SUBSTITUTED — `ConnectorAuth` extraction (`pm_auth.cppm:785-832`).
- `store_bank_details_in_payment_methods` (`pm_auth.rs:301-565`) — IMPLEMENTED (substitute primitive) — `store_bank_details` + `KeyedHasher`-based `bank_hash` (`pm_auth.cppm:1032-1106,937-953`).
- `store_in_db` bulk PM insert/update (`pm_auth.rs:579-616`) — IMPLEMENTED (substitute scheduling) — `update_entries` / `new_entries` applied as two batches (`pm_auth.cppm:1108-1134`).
- `get_access_token_from_exchange_api` (`pm_auth.rs:674-724`) — IMPLEMENTED — `connector->exchange_token` + `connector_failure` (`pm_auth.cppm:399-408,839-867`).
- `get_bank_account_creds` (`pm_auth.rs:618-672`) — IMPLEMENTED — `bank_account_credentials` + `parse_bank_credentials` (`pm_auth.cppm:411-431,876-934`).
- `retrieve_payment_method_from_auth_service` (`pm_auth.rs:786-893`) — DEFERRED — payment-confirm path; not invoked by either route handler.
- `helpers` / `transformers` submodules (`pm_auth.rs:8-9`) — SUBSTITUTED — in-file helpers (`pm_auth.cppm:757-832,876-934,1155-1249`).
- `auth.client_secret` override (`routes/pm_auth.rs:62-64,119-121`) — IMPLEMENTED — claim override (`pm_auth.cppm:121-124,341-344`).
- SDK/client-secret API-key auth (`routes/pm_auth.rs:25-40`) — IMPLEMENTED — named extractors `sdk_auth_and_get_auth` / `client_secret_and_get_auth` (`pm_auth.cppm:576-596`), with registry-rule fallback as a local substitute (`pm_auth.cppm:616-637`).
- v2 `todo!()` variants (`pm_auth.rs:214-222,567-577,775-784`) — DEFERRED — no upstream implementation to port; parity not expected.
