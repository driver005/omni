# Parity: mandates

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/mandates.rs`; registration `/mandates` in `docs/hyperswitch/crates/router/src/routes/app.rs:2316-2336` (`#[cfg(all(any(feature = "olap", feature = "oltp"), feature = "v1"))]`). Adjacent `GET /customers/{customer_id}/mandates` registered at `app.rs:1599-1600` (owned by the customers module, out of this handler's scope).
- Upstream core: `docs/hyperswitch/crates/router/src/core/mandate.rs` (+ `core/mandate/helpers.rs`, `core/mandate/utils.rs`, `router/src/types/api/mandates.rs`). Input/output DTOs: `docs/hyperswitch/crates/api_models/src/mandates.rs`.
- C++: include/app/handlers/mandates.cppm
- Checked: 2026-11-20 (strict-parity pass: every route row ported to an upstream-exact local helper)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All 3 `/mandates` ops with same paths+verbs: `GET /list`, `GET /:id`, `POST /revoke/:id` (`mandates.cppm:282-320` vs `app.rs:2321-2333`). No C++-only routes to remove. Adjacent `GET /customers/{customer_id}/mandates` is owned by the customers module (`customers.cppm:642-685,904-912`). |
| Input types | MATCH | Upstream `web::Path<String>` → `MandateId` (`routes/mandates.rs:19-24,47-52`) mirrored by `mandate_id_from_path` (`mandates.cppm:680-703`); upstream `web::Query<MandateListConstraints>` (`routes/mandates.rs:74`; `api_models/mandates.rs:139-179`) mirrored by typed `Constraints` population incl. all five `created_time*` filters (`mandates.cppm:195-230`). |
| Output types | MATCH | `from_db_mandate` (`mandates.cppm:583-624`) reproduces the upstream v1 wire mapping (`types/api/mandates.rs:36-135`): `amount`/`currency`/`profile_id`/`mandate_metadata` are `None` (skipped), `card` only for the Card payment method, `customer_acceptance` always emitted. `revoked_response_json` (`mandates.cppm:626-634`) mirrors `MandateRevokedResponse` (`api_models/mandates.rs:15-33`). List is a bare `ApplicationResponse::Json` array (`mandates.cppm:259-279`). Card/acceptance payloads come from the in-process KeyStore stand-in for the processor key store / locker (sanctioned local substitute). |
| Auth/permissions | MATCH | `authorize` (`mandates.cppm:346-387`) mirrors all three extractors: get/revoke = `HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false, allow_platform_self_operation:false})` (`routes/mandates.rs:33-36,61-64`); list = `auth_type(HeaderAuth(...), JWTAuth{MerchantMandateRead, allow_connected:false, allow_platform:false})` (`routes/mandates.rs:86-97`), with `check_mandate_permission` mirroring `services/authorization.rs:115-128` (`mandates.cppm:389-421`). Stale registry rows noted under Shared contract requests. |
| Persistence | MATCH | `find_mandate_for_merchant` applies the merchant id as a DB predicate (`mandates.cppm:439-454`; upstream `find_mandate_by_merchant_id_mandate_id`, `db/mandate.rs:12-17`). List uses `MandateQuery` at the DB boundary (`mandates.cppm:244-257`; upstream `find_mandates_by_merchant_id`, `core/mandate.rs:409-415`). Revoke resolves profile → MCA → router data → connector trigger → `StatusUpdate{Revoked}` (`mandates.cppm:123-179`; upstream `core/mandate.rs:80-145`). `MandateNotFound` maps to 404, not a C++-only 403 ownership error. |
| Domain logic | MATCH | Full state machine over the four upstream enum variants (`common_enums/enums.rs:3633-3639`; `mandates.cppm:110-121,638-647`); `get_profile_id_for_mandate` (`mandates.cppm:458-480`; `helpers.rs:13-44`); `get_mandate_type` validator (`mandates.cppm:482-510`; `helpers.rs:46-85`); `construct_mandate_revoke_router_data` with upstream constants (`mandates.cppm:536-571`; `utils.rs:13-104`); `ExternalConnectorError` mapping (`mandates.cppm:156-172`; `core/mandate.rs:148-155`); bare-array list with `created_time` filters (`mandates.cppm:259-279,705-727`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/mandates/{id}` | `get_mandate` (`routes/mandates.rs:16`) | `handle_get_mandate` (`mandates.cppm:42`) | `MandateId` path | v1 `MandateResponse` wire via `from_db_mandate` | `HeaderAuth(ApiKeyAuth{false,false})` | DB-scoped mandate find + KeyStore card/acceptance stand-in | MATCH | `app.rs:2327`; `mandates.cppm:42-76,439-454,583-624`; `core/mandate.rs:33-58`; `types/api/mandates.rs:36-135` |
| GET `/mandates/list` | `retrieve_mandates_list` (`routes/mandates.rs:71`) | `handle_retrieve_mandates_list` (`mandates.cppm:183`) | `MandateListConstraints` query incl. `created_time*` | bare `Vec<MandateResponse>` | ApiKey OR JWT `MerchantMandateRead{false,false}` | `MandateQuery` DB filter + connector/created_time in-memory + bare array | MATCH | `app.rs:2326`; `mandates.cppm:183-280`; `core/mandate.rs:403-426`; `api_models/mandates.rs:139-179` |
| POST `/mandates/revoke/{id}` | `revoke_mandate` (`routes/mandates.rs:44`, `#[cfg(feature="v1")]`) | `handle_revoke_mandate` (`mandates.cppm:79`) | `MandateId` path | `MandateRevokedResponse` | `HeaderAuth(ApiKeyAuth{false,false})` | profile → MCA → router data → connector `Trigger` → `StatusUpdate{Revoked}`, `ExternalConnectorError` on failure | MATCH | `app.rs:2332`; `mandates.cppm:79-180,458-571`; `core/mandate.rs:60-165`; `helpers.rs:13-44`; `utils.rs:13-104` |

## Not ported
Only internal upstream machinery not reachable from a route row in this runtime; everything from the previous gap list that is reachable is now ported above.

- Platform/processor account + `storage_scheme` + KV fallback: `platform.get_processor().get_account().get_id()/storage_scheme` (`core/mandate.rs:42-46,69-75,410-413`) has no platform domain here; the authenticated merchant id is the local equivalent and is applied as a DB predicate. Shared request #2.
- Processor key-manager / locker decryption: upstream `get_card_from_locker` (`types/api/mandates.rs:68-105`); the in-process `AppContext::key_store()` `(purpose="mandate_card"|"customer_acceptance", key_id=mandate_id)` is the sanctioned local substitute. The fallback `customer_acceptance` mirrors `types/api/mandates.rs:117-128`.
- Connector DB events / workflow / `MANDATE_COUNT` + `SUBSEQUENT_MANDATE_PAYMENT` metrics (`core/mandate.rs:336-339,397`): belong to `mandate_procedure`, which is invoked from payment flows, not from a mandate route.
- `update_connector_mandate_id` (`core/mandate.rs:167-220`): backfills `connector_mandate_id` when existing is `None`; not a route, and `MandateRecord` has no `connector_mandate_id`/`mandate_type`/`amount_captured` columns (`app/store/mandates.cppm:13-64`). Shared request #3.
- `mandate_procedure` (`core/mandate.rs:275-401`): payment-flow helper (SingleUse → Revoked, MultiUse → `CaptureAmountUpdate`, setup-mandate insertion); no mandate route invokes it.
- `GET /customers/{customer_id}/mandates` (`core/mandate.rs:221-261`): owned by the customers module (`customers.cppm:642-685,904-912`), not registrable from this handler.

## Implementation disposition
- Connector-side revoke chain — IMPLEMENTED — `mandates.cppm:130-172`: `get_profile_id_for_mandate` → `get_merchant_connector_account` → `construct_mandate_revoke_router_data` (upstream constants `utils.rs:13-17`) → `connector_router().select_for()/select()` → `connector->mandate_revoke()` → `ExternalConnectorError` mapping. Local substitutes limited to profile id carried on the record and MCA/connector lookup via `AppRepository`/`ConnectorRouter`.
- Revocable-status state machine — IMPLEMENTED — `mandates.cppm:110-121,638-647`: exactly the four upstream `MandateStatus` variants; Revoked and other values produce `MandateValidationFailed` (400), matching `api_error_response.rs:511-513`.
- Key-store response mapping — IMPLEMENTED (local infra substitute) — `mandates.cppm:583-624`: exact v1 wire fields and optionality from `types/api/mandates.rs:110-134`; card/acceptance from in-process `KeyStore`.
- `created_time` filters — IMPLEMENTED — `mandates.cppm:211-230,705-727`: all five constraints populated from `RequestQuery` and applied.
- List response shape — IMPLEMENTED — `mandates.cppm:259-279`: bare JSON array (`core/mandate.rs:425`).
- Platform/processor account + `storage_scheme` scoping — SUBSTITUTED — `mandates.cppm:439-454`: DB-level merchant predicate via `MandateQuery`; no platform domain locally (Shared request #2).
- Upstream auth flags — IMPLEMENTED — `mandates.cppm:333-421`: ApiKey-only for get/revoke, ApiKey-or-JWT `MerchantMandateRead` for list, connected/platform scope rejected; `check_mandate_permission` mirrors `services/authorization.rs:115-128`.
- Merchant ownership check — REMOVED — upstream has no header comparison; the merchant id is now a DB lookup predicate and a mismatch yields `MandateNotFound` (404), matching `api_error_response.rs:486-488`.
- Flow instrumentation + `LockAction::NotApplicable` — IMPLEMENTED (local substitute) — `mandates.cppm:64,99,238,742-747`: `record_flow("MandatesRetrieve"|"MandatesRevoke"|"MandatesList")` with upstream `Flow` names; no API-locking concept locally.
- `get_mandate_type` — IMPLEMENTED — `mandates.cppm:482-510`: local port of `helpers.rs:46-85` (`[[maybe_unused]]`, not invoked by a mandate route).

## Shared contract requests
Changes needed outside files owned by this module (not edited here):
1. `include/app/auth/registry.cppm:149,382,394` — rows `get_mandate`/`revoke_mandate`/`retrieve_mandates_list` still read `JWT/MANDATE_READ`. Upstream is ApiKey-only for get/revoke and ApiKey-or-JWT `MerchantMandateRead` for list (`routes/mandates.rs:33-36,61-64,86-97`). The handler implements the exact modes locally; registry should be updated for one source of truth.
2. `include/app/context/context.cppm` — no `platform`/processor-account/`storage_scheme` domain; adding one would let mandate lookups drop the merchant-id-header fallback at `mandates.cppm:423-430`.
3. `include/app/store/mandates.cppm` + `include/app/store/repository.cppm` — `MandateRecord` lacks `connector_mandate_id`, `mandate_type`, `amount_captured`, `customer_ip_address`, `customer_accepted_at`, `user_agent`, and the repository has no `find_mandate_by_merchant_id_mandate_id`. Adding them would allow exact `update_connector_mandate_id`/`mandate_procedure` and exact `from_db_mandate` card/acceptance derivation instead of the KeyStore stand-in.
4. `include/app/model/mandates.cppm` (shared with customers module) — `*Value` wrappers have no `serde::Serializable` specialization, so typed-DTO encoding would emit `{"value":...}`. This handler builds exact wire JSON directly (`mandates.cppm:583-634`), so no edit was made; a shared fix could let both handlers use typed DTOs.

## Report
- Rows moved to MATCH: 3/3 route rows; 6/6 summary dimensions.
- Remaining: only non-route internal upstream machinery (platform/storage_scheme, locker/key manager, `mandate_procedure`, `update_connector_mandate_id`, customer-mandates route owned by customers module) — listed above and in Shared contract requests.
- Shared requests: 4 (auth registry rows; platform domain; store record/repository fields; shared model value-wrapper serde).
- Files changed: `include/app/handlers/mandates.cppm`, `docs/hyperswitch-parity/mandates.md`. (`include/app/model/mandates.cppm` is shared with the customers module, so it was not edited.)
