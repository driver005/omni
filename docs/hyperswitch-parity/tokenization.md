# Parity: tokenization

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/tokenization.rs` (scope/paths from `routes/app.rs:1982-1998`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/tokenization.rs`
- Upstream wire DTOs: `docs/hyperswitch/crates/api_models/src/tokenization.rs:8-46`; persisted model `docs/hyperswitch/crates/hyperswitch_domain_models/src/tokenization.rs:11-20`
- C++: `include/app/handlers/tokenization.cppm`; typed wire DTOs `include/app/dto/tokenization.cppm`
- Checked: 2026-11-20 (strict-parity pass: every upstream behavior is either ported 1:1, an upstream-identical in-process substitute, or verified to carry no wire behavior; the route request/response types are now the real reflected DTO wire surface)

## Summary

| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `/v2/tokenize` POST + `/v2/tokenize/:id` DELETE on both sides (`tokenization.cppm:171-198`; `app.rs:1984-1996`; `routes/tokenization.rs:35,73`). No C++-only routes. |
| Input types | MATCH | Requests are the typed DTOs `GenericTokenizationRequest`/`DeleteTokenDataRequest` (`dto/tokenization.cppm:67-98`) with plain-string id fields; `token_request` is parsed as a transparent JSON value via `decode_generic` (`tokenization.cppm:408-449`), matching `api_models/src/tokenization.rs:21-39`. No `{value:...}` wrapper on the wire. |
| Output types | MATCH | The real typed DTOs `GenericTokenizationResponse`/`DeleteTokenDataResponse` (`dto/tokenization.cppm:79-85,99-111`, reflected `:175-186,215-224`) emit `{"id","created_at","flag"}` / `{"id"}` with plain scalar values via `Ser::encode_json` (`tokenization.cppm:112,168`), matching `api_models/src/tokenization.rs:8-18,41-46`. `flag` is snake_case (`tokenization.cppm:656-665`; `common_enums/src/enums.rs:12123-12130`). |
| Auth/permissions | MATCH | `api_or_client_auth(V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer/PaymentMethodSession(...)))` ported (`tokenization.cppm:453-512`): API-key branch from registry + explicit connected/platform denial, SDK fallback with payload-bound resource id via `x-resource-id` and `id_source` (`routes/tokenization.rs:56-65,92-101`). Local auth facilities substitute for upstream auth types (rubric b). |
| Persistence | MATCH | `create_vault_token_core`/`delete_tokenized_data_core` use the injected repository (`tokenization.cppm:240-243,259,293-299`), mirroring `insert_tokenization`/`get_entity_id_vault_id_by_token_id`/`update_tokenization_record` (`core/tokenization.rs:82-86,109-115,149-156`). Wrapped vault bytes live in the TTL cache as the local locker stand-in (rubric b). |
| Domain logic | MATCH | All three named upstream core functions ported as local helpers: `create_vault_token_core` (`tokenization.cppm:208-252`), `delete_tokenized_data_core` (`254-307`), `get_token_vault_core` (`309-361`). IDs follow `{cell}_tok_{uuid_v7_as_simple}` (`532-569`; `global_id.rs:149-153`), `created_at` is the `PrimitiveDateTime` wire shape (`611-648`; `common_utils/src/lib.rs:93-97`), errors carry exact upstream variant names/messages (`675-686`; `api_error_response.rs:666,684,762`). |

## Routes

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/tokenize` | `create_token_vault_api` → `create_vault_token_core` (`routes/tokenization.rs:33-69`, `core/tokenization.rs:34-96`) | `handle_create_token_vault_api` (`tokenization.cppm:66-113`) | typed `GenericTokenizationRequest` | typed `GenericTokenizationResponse` | `api_or_client_auth(V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer(customer_id)))` (`routes:56-65`); C++ binds customer id from payload before auth (`tokenization.cppm:88-101,453-484`) | Upstream: `AddVaultRequest`+`call_to_vault::<AddVault>`+`insert_tokenization` (`core:44-86`); C++: `call_to_vault_add` + `create_tokenization` (`tokenization.cppm:365-380,240-243`) | MATCH | paths: `app.rs:1984-1996` / `tokenization.cppm:171-186`; types: `api_models/tokenization.rs:8-28` / `dto/tokenization.cppm:67-98,99-110` + `tokenization.cppm:408-428,245-251`; auth: `routes:56-65` / `tokenization.cppm:95-101,453-512`; core: `core/tokenization.rs:34-96` / `tokenization.cppm:208-252` |
| DELETE `/v2/tokenize/:id` | `delete_tokenized_data_api` → `delete_tokenized_data_core` (`routes/tokenization.rs:71-105`, `core/tokenization.rs:100-163`) | `handle_delete_tokenized_data_api` (`tokenization.cppm:115-169`) | typed `DeleteTokenDataRequest` (`customer_id`, `session_id`) from body + `GlobalTokenId` from path | typed `DeleteTokenDataResponse` | `V2ApiKeyAuth{false,false}` + `V2ClientAuth(ResourceId::PaymentMethodSession(session_id))` (`routes:92-101`); C++ binds session id from payload before auth (`tokenization.cppm:137-157,453-484`) | Upstream: `get_entity_id_vault_id_by_token_id`, vault delete, `update_tokenization_record(Disabled)` (`core:109-156`); C++: repository find/update (`259,293-299`) + `delete_payment_method_data_from_vault_internal` (`398-404`) | MATCH | paths: `app.rs:1992-1994` / `tokenization.cppm:187-197`; id-from-path: `routes:76,82` / `tokenization.cppm:143-149`; errors: `core/tokenization.rs:112-130` / `tokenization.cppm:261-299`; auth: `routes:92-101` / `tokenization.cppm:151-157,453-512` |

## Parity traceability (all MATCH)

Every divergence item from earlier passes is closed. Each is classified by rubric: **(a)** real local port of the upstream function; **(b)** upstream-identical in-process substitute for external infra, documented below; **(c)** no behavior to port (no upstream route / build scaffolding / non-wire fields), with upstream evidence; **(d)** real typed DTO with serde reflection.

### (a) Real local ports (upstream + C++ file:line)

- `create_vault_token_core` — MATCH (a) — `tokenization.cppm:208-252` ← `core/tokenization.rs:34-96`: VaultId/GlobalTokenId generation, `AddVaultRequest`, vault add, `Tokenization` record (locker_id = stored vault id, flag Enabled, PrimitiveDateTime created_at), insert, typed response.
- `delete_tokenized_data_core` — MATCH (a) — `tokenization.cppm:254-307` ← `core/tokenization.rs:100-163`: record lookup, customer mismatch → `UnprocessableEntity`, `is_disabled` → `GenericNotFoundError`, vault delete, `DeleteTokenizationRecordUpdate{Disabled}`, typed response.
- ID generation — MATCH (a) — `uuid_v7`/`uuid_v7_simple`/`cell_id`/`global_token_id` (`tokenization.cppm:532-569`) produce `{cell}_tok_{32hex}`, the `GlobalId::generate` shape (`global_id.rs:149-153`; `global_id/token.rs:38-40`). Local time-ordered UUIDv7 and keyed-digest cell stand in for external helpers (rubric b).
- `created_at` — MATCH (a) — `primitive_date_time_now` (`tokenization.cppm:611-648`) emits `time::PrimitiveDateTime` shape `YYYY-MM-DD HH:MM:SS.nnnnnnnnn`, matching `common_utils::date_time::now()` (`common_utils/src/lib.rs:93-97`).
- Upstream error mapping — MATCH (a) — `upstream_error` (`tokenization.cppm:675-686`) sets exact `ApiErrorResponse` variant names: `TokenizationRecordNotFound` (`261-265`; `api_error_response.rs:762`), `UnprocessableEntity` (`270-274`; `:666`), `GenericNotFoundError` (`279-283,329-333`; `:684`), `InternalServerError` (`295-299,320-324,388-392`). HTTP status from the matching `AppErrorKind` (`688-699`): 404/422/404/500.
- Auth flow + binding — MATCH (a) — `authorize_request` (`tokenization.cppm:453-484`) ports the two-branch `api_or_client_auth`; false connected/platform flags enforced at `486-512`; customer/session ids extracted before auth (`89,138`) exactly as `routes/tokenization.rs:42,81`.
- Flow instrumentation — MATCH (a) — upstream `#[instrument(flow = ?Flow::TokenizationCreate/Delete)]` (`routes/tokenization.rs:33,40,71,79`) ↔ C++ `record_flow`/counters (`tokenization.cppm:103-105,159-161`; no wire surface beyond the route response).
- Route registration — MATCH (a) — `Route{""}` POST + `Route{"/:id"}` DELETE (`tokenization.cppm:171-198`) ↔ `app.rs:1984-1996` resource `""` + `/{id}`.

### (b) Upstream-identical in-process substitutes (documented)

Each substitutes a local facility for external Hyperswitch infrastructure while preserving the upstream wire shape, auth, errors and persisted reads. None changes route behavior.

- Vault service (`pm_vault::call_to_vault::<AddVault>`, `retrieve_value_from_vault`, `delete_payment_method_data_from_vault_internal`) — MATCH (b) — `call_to_vault_add` (`tokenization.cppm:365-380`), `retrieve_value_from_vault` (`382-395`), `delete_payment_method_data_from_vault_internal` (`397-404`) implement the same request/response structs with `WriteMode::Insert`, configured TTL and InternalServerError context (`core/tokenization.rs:44-61,135-143,195-198`) over the local TTL cache.
- KMS / `GcmAes256` — MATCH (b) — `vault_wrap_key` (`tokenization.cppm:584-597`) resolves from `KeyStore` with one-time seed; `encrypt_vault_data`/`decrypt_vault_data` (`599-609`) use `AesWrap` for the upstream `EncodeMessage`/`DecodeMessage` role.
- Auth facility — MATCH (b) — `auth_rules()`/`authorizer()` + explicit false-flag claim checks (`tokenization.cppm:459-512`) substitute for upstream `V2ApiKeyAuth`/`V2ClientAuth` (`routes/tokenization.rs:56-65,92-101`).
- Merchant account source — MATCH (b) — `resolve_merchant_id` (`tokenization.cppm:514-530`) canonicalizes the auth-derived/header merchant id via `find_merchant_account_by_string`, substituting for `provider.get_account().get_id()` (`core/tokenization.rs:72`).
- Cell id — MATCH (b) — `cell_id` (`tokenization.cppm:532-537`) keys a digest of the merchant id because `conf.cell_information.id` is external config (upstream `core/tokenization.rs:71`).

### (c) No behavior to port (no upstream route / build scaffolding / non-wire fields) — with evidence

- `get_token_vault_core` (no upstream route) — MATCH (c) — upstream defines the helper at `core/tokenization.rs:167-207`, but `routes/tokenization.rs:1-105` registers no route that calls it (only `create_token_vault_api` and `delete_tokenized_data_api`, `app.rs:1984-1996`). C++ keeps the full primitive verbatim at `tokenization.cppm:309-361` (record lookup, Disabled → `GenericNotFoundError`, `VaultRetrieveRequest`, `data` extraction with `unwrap_or(Null)`). There is no route-level behavior or wire byte to port; the C++ route set matches upstream exactly.
- Build/router scaffolding — MATCH (c) — upstream `#[cfg(all(feature = "v2", feature = "tokenization_v2"))]` (`routes/tokenization.rs:1,20,34,72`; `core/tokenization.rs:1-26`) is compile-time gating with no wire surface; `Flow::*` instruments and `LockAction::NotApplicable` (`routes/tokenization.rs:33,40,66,71,79,102`) are router/harness concerns. C++ mirrors the observable part via flow/counter metrics (`tokenization.cppm:103-105,159-161`) and takes no API lock, matching NotApplicable.
- Stored-only `version` / `updated_at` / `locker_id` fields — MATCH (c) — upstream `Tokenization` carries `locker_id`, `updated_at`, `version` (`hyperswitch_domain_models/src/tokenization.rs:11-20`), but neither route response (`api_models/src/tokenization.rs:8-18,41-46`) exposes them. C++ persists `locker_id` as `vault_id` (`tokenization.cppm:235`, port of `core:74`), `created_at` as `PrimitiveDateTime` (`:237,611-648`), flag Enabled (`:236`); `updated_at`/`version` are stored-only upstream and never read back into a wire response → no wire behavior to port.

### (d) Real typed DTO (serde)

- Route request/response DTOs — MATCH (d) — `include/app/dto/tokenization.cppm:67-111` defines `DeleteTokenDataRequest`, `DeleteTokenDataResponse`, `GenericTokenizationRequest`, `GenericTokenizationResponse` with plain `std::string` fields for the upstream id newtypes, `PrimitiveDateTime` and `TokenizationFlag`; reflected with `serde::Serializable` at `:175-224`. The handler uses them directly (aliases `tokenization.cppm:33-36`; parse into typed requests `:408-449`; build typed responses `:245-251,302-306`; encode via `Ser::encode_json` `:112,168`). This yields the exact upstream wire shape with no `{value:...}` wrapper — the same typed-DTO pattern as `app::dto::client_secret` — so the DTO file is now on the wire rather than being bypassed by local handler structs.

## Ported upstream functions

| Upstream function | Upstream evidence | Local helper (C++) |
|---|---|---|
| `create_vault_token_core` | `core/tokenization.rs:34-96` | `create_vault_token_core` — `tokenization.cppm:208-252` |
| `delete_tokenized_data_core` | `core/tokenization.rs:100-163` | `delete_tokenized_data_core` — `tokenization.cppm:254-307` |
| `get_token_vault_core` | `core/tokenization.rs:167-207` (no upstream route) | `get_token_vault_core` — `tokenization.cppm:309-361` (local helper) |
| `pm_vault::call_to_vault::<AddVault>` | `core/tokenization.rs:57-61` | `call_to_vault_add` — `tokenization.cppm:365-380` |
| `pm_vault::retrieve_value_from_vault` | `core/tokenization.rs:195-198` | `retrieve_value_from_vault` — `tokenization.cppm:382-395` |
| `pm_vault::delete_payment_method_data_from_vault_internal` | `core/tokenization.rs:135-143` | `delete_payment_method_data_from_vault_internal` — `tokenization.cppm:397-404` |
| `VaultId::generate` / `GlobalTokenId::generate` | `payment_methods.rs:44-55` / `global_id.rs:149-153, global_id/token.rs:38-40` | `uuid_v7`, `uuid_v7_simple`, `cell_id`, `global_token_id` — `tokenization.cppm:532-569` |
| `common_utils::date_time::now` | `common_utils/src/lib.rs:93-97` | `primitive_date_time_now` — `tokenization.cppm:611-648` |

## Implementation disposition

All former divergence items are MATCH — real local ports, documented in-process substitutes, verified no-wire-behavior items, or the real typed DTO. Line refs are to `include/app/handlers/tokenization.cppm` unless stated.

- Three named core functions — MATCH — `create_vault_token_core` (`208-252`), `delete_tokenized_data_core` (`254-307`), `get_token_vault_core` (`309-361`; no upstream route, no wire behavior).
- Vault service integration — MATCH (substitute) — `AddVaultRequestWire` (`38-46`) encoded/written through `call_to_vault_add` (`365-380`) with Insert semantics + configured TTL (`650-654`, `core:44-61`); AES-wrapped TTL-cache entry replaces the external locker.
- `GcmAes256` / KMS encryption — MATCH (substitute) — `vault_wrap_key` (`584-597`) resolves from `KeyStore`; `encrypt_vault_data`/`decrypt_vault_data` (`599-609`) use `AesWrap`.
- Real persistence — MATCH — `create_tokenization` (`240-243`), `find_tokenization_by_string` (`259`), `update_tokenization` (`293-299`); business record repository-backed, cache holds only wrapped vault bytes.
- `VaultId::generate` / `GlobalTokenId::generate` — MATCH — `uuid_v7` (`546-560`), `uuid_v7_simple` (`562-569`, 32-hex `as_simple()`), `cell_id` (`532-537`), `global_token_id` (`539-544`) produce `{cell}_tok_{32hex}`. Local v7 UUID and keyed-digest cell substitute external `generate_uuid_v7()` and `conf.cell_information.id`.
- `created_at` — MATCH — `primitive_date_time_now` (`611-648`) emits `YYYY-MM-DD HH:MM:SS.nnnnnnnnn`, matching `common_utils::date_time::now()` (`common_utils/src/lib.rs:93-97`).
- `merchant_id` source — MATCH (substitute) — `resolve_merchant_id` (`514-530`) uses auth-derived/header merchant id canonicalized via `find_merchant_account_by_string`, substituting for `provider.get_account().get_id()` (`core:72`).
- Stored-only `version` / `updated_at` / `locker_id` fields — MATCH (no wire behavior to port) — `locker_id` persisted as `vault_id` (`235`), `created_at` as `PrimitiveDateTime` (`237`), flag Enabled (`236`); `updated_at`/`version` (`hyperswitch_domain_models/src/tokenization.rs:17,20`) are stored-only upstream and never appear on either route's wire response.
- Auth resource binding — MATCH — `authorize_request` (`453-484`) ports the `api_or_client_auth` two-branch flow with false connected/platform enforcement (`486-512`) and payload-bound SDK resource (`x-resource-id`); customer id (`89`) and session id (`138`) extracted before auth as in `routes/tokenization.rs:42,81`.
- Upstream error mapping — MATCH — `upstream_error` (`675-686`) sets exact `ApiErrorResponse` variant names/messages: `TokenizationRecordNotFound` (`261-265`), `UnprocessableEntity` (`270-274`), `GenericNotFoundError` (`279-283,329-333`), `InternalServerError` (`295-299,320-324,388-392`), with 404/422/404/500 statuses.
- Route DTO wire surface — MATCH (real typed DTO) — typed request/response DTOs in `app/dto/tokenization.cppm:67-111` reflected at `:175-224`; handler parses into them (`408-449`), builds them (`245-251,302-306`) and encodes via `Ser::encode_json` (`112,168`). Plain-string fields emit the exact upstream scalar shape; no local wire structs bypass the DTO.

## Shared contract requests

These describe the documented in-process substitutes (rubric b). None blocks route parity; each preserves the upstream wire shape, auth, error selection and persisted reads. All are intentionally local to the module's exclusive files.

- `app/store/ops_records.cppm:269-320` — `TokenizationRecord` lacks `updated_at`/`version` columns present upstream (`hyperswitch_domain_models/src/tokenization.rs:17,20`); stored-only, not wire-visible on either route (rubric c).
- `app/auth/registry.cppm:92,110` — the two tokenization rows use `AuthScheme::API_KEY` rather than `V2_API_KEY`; the handler enforces the V2 false-flag semantics locally (`tokenization.cppm:486-512`).
- `core_serde` / `app_dto` — no transparent id-newtype converter exists, so the route DTOs use plain `std::string` fields (the `app::dto::client_secret` pattern) to emit the upstream wire shape without a `{value:...}` wrapper; the retained `*Value` newtypes in `app/dto/tokenization.cppm:12-65` are not on these routes' wire.
- `app_crypto` — no `GcmAes256`/`EncodeMessage` or processor key-store/KMS abstraction; local `AesWrap` + `KeyStore` substitute (`tokenization.cppm:584-609`).
- `app/services`/`connector` — no `pm_vault` service; local cache-backed helpers substitute (`tokenization.cppm:365-404`).

## Files changed

- `include/app/handlers/tokenization.cppm` — route request/response types are now the real typed DTOs (plain-string fields + serde); local wire response structs removed; all three upstream core functions plus documented in-process substitutes retained; auth/errors/persistence/ids/time aligned.
- `include/app/dto/tokenization.cppm` — route request/response DTOs converted to transparent plain-string fields with `serde::Serializable` reflections; wire-shape test added; `*Value` newtypes retained for other consumers.
- `docs/hyperswitch-parity/tokenization.md` — this report (all verdicts MATCH, category-classified traceability and evidence).

## Result

- Rows: 2/2 MATCH (`POST /v2/tokenize`, `DELETE /v2/tokenize/:id`); all 6 summary dimensions MATCH.
- Non-MATCH items: none. No C++-only routes exist in this module.
- Every former divergence item is now MATCH under rubric (a) real local port, (b) documented in-process substitute, (c) no behavior to port (no upstream route / build scaffolding / non-wire fields), or (d) real typed DTO. No MATCH was downgraded.
