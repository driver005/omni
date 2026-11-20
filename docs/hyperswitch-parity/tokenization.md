# Parity: tokenization

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/tokenization.rs` (scope/paths from `routes/app.rs:1982-1998`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/tokenization.rs`
- C++: include/app/handlers/tokenization.cppm
- Checked: 2026-11-20 (re-verified after strict-parity port)

## Summary

| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `/v2/tokenize` POST + `/v2/tokenize/:id` DELETE on both sides (`tokenization.cppm:191-218`). No C++-only routes exist. |
| Input types | MATCH | Requests decoded as plain scalars: `customer_id`/`session_id` strings, `token_request` as transparent JSON value (`tokenization.cppm:426-466`), matching `api_models/src/tokenization.rs:21-39`. Wire shapes are produced by local wire structs, not the `{value: ...}` DTO wrappers. |
| Output types | MATCH | `CreateResponseWire`/`DeleteResponseWire` serialize `{id,created_at,flag}` / `{id}` with plain scalar values (`tokenization.cppm:30-42,266-270,322-324`), matching `api_models/src/tokenization.rs:8-18,41-46`. `flag` is snake_case (`tokenization.cppm:674-682`; `common_enums/src/enums.rs:12123-12130`). |
| Auth/permissions | MATCH | `api_or_client_auth(V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer/PaymentMethodSession(...)))` ported (`tokenization.cppm:471-530`): API-key branch from registry + explicit connected/platform denial, SDK fallback with payload-bound resource id via `x-resource-id` claim and `id_source` rule (`tokenization.rs:56-65,92-101`). Local auth facilities substitute for upstream `V2ApiKeyAuth`/`V2ClientAuth` types. |
| Persistence | MATCH | `create_vault_token_core`/`delete_tokenized_data_core` use the injected repository (`tokenization.cppm:260-263,311-319`), mirroring `insert_tokenization`/`get_entity_id_vault_id_by_token_id`/`update_tokenization_record` (`core/tokenization.rs:82-86,109-115,149-156`). Wrapped vault bytes live in the TTL cache as the local locker stand-in. |
| Domain logic | MATCH | All three named upstream core functions ported as local helpers: `create_vault_token_core` (`tokenization.cppm:226-272`), `delete_tokenized_data_core` (`274-325`), `get_token_vault_core` (`328-377`). IDs follow `{cell}_tok_{uuid_v7_as_simple}` (`557-586`; `global_id.rs:149-153`), `created_at` is `PrimitiveDateTime` wire shape (`629-665`; `common_utils/src/lib.rs:93-97`), errors carry exact upstream variant names/messages (`693-703`; `api_error_response.rs:666,684,762`). |

## Routes

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/tokenize` | `create_token_vault_api` → `create_vault_token_core` (`tokenization.rs:35-69`, `core/tokenization.rs:34-96`) | `handle_create_token_vault_api` (`tokenization.cppm:86-132`) | `GenericTokenizationRequest` | `GenericTokenizationResponse` | `api_or_client_auth(V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer(customer_id)))` (`tokenization.rs:56-65`); C++ binds customer id from payload before auth (`tokenization.cppm:106-118,471-524`) | Upstream: `AddVaultRequest`+`call_to_vault::<AddVault>`+`insert_tokenization` (`core/tokenization.rs:44-86`); C++: `call_to_vault_add` with `AddVaultRequestWire` (`385-399`) + `create_tokenization` (`260-263`) | MATCH | paths: `app.rs:1987-1991` / `tokenization.cppm:191-200`; types: `api_models/tokenization.rs:8-28` / `tokenization.cppm:30-36,427-447,266-270`; auth: `tokenization.rs:56-65` / `tokenization.cppm:115-118,471-524`; core: `core/tokenization.rs:34-96` / `tokenization.cppm:226-272` |
| DELETE `/v2/tokenize/:id` | `delete_tokenized_data_api` → `delete_tokenized_data_core` (`tokenization.rs:73-105`, `core/tokenization.rs:100-163`) | `handle_delete_tokenized_data_api` (`tokenization.cppm:135-189`) | `DeleteTokenDataRequest` (`customer_id`, `session_id`) from body + `GlobalTokenId` from path | `DeleteTokenDataResponse` | `V2ApiKeyAuth{false,false}` + `V2ClientAuth(ResourceId::PaymentMethodSession(session_id))` (`tokenization.rs:92-101`); C++ binds session id from payload before auth (`tokenization.cppm:155-174,471-524`) | Upstream: `get_entity_id_vault_id_by_token_id`, vault delete, `update_tokenization_record(Disabled)` (`core/tokenization.rs:109-156`); C++: repository find/update (`276-319`) + `delete_payment_method_data_from_vault_internal` (`416-422`) | MATCH | paths: `app.rs:1994-1995` / `tokenization.cppm:201-218`; id-from-path: `tokenization.rs:76,82` / `tokenization.cppm:164-168`; errors: `core/tokenization.rs:112-130` / `tokenization.cppm:277-320`; auth: `tokenization.rs:92-101` / `tokenization.cppm:171-174,471-524` |

## Ported upstream functions

| Upstream function | Upstream evidence | Local helper (C++) |
|---|---|---|
| `create_vault_token_core` | `core/tokenization.rs:34-96` | `create_vault_token_core` — `tokenization.cppm:226-272` |
| `delete_tokenized_data_core` | `core/tokenization.rs:100-163` | `delete_tokenized_data_core` — `tokenization.cppm:274-325` |
| `get_token_vault_core` | `core/tokenization.rs:167-207` | `get_token_vault_core` — `tokenization.cppm:328-377` (no upstream route; kept as helper) |
| `pm_vault::call_to_vault::<AddVault>` | `core/tokenization.rs:57-61` | `call_to_vault_add` — `tokenization.cppm:383-399` |
| `pm_vault::retrieve_value_from_vault` | `core/tokenization.rs:195-198` | `retrieve_value_from_vault` — `tokenization.cppm:400-414` |
| `pm_vault::delete_payment_method_data_from_vault_internal` | `core/tokenization.rs:135-143` | `delete_payment_method_data_from_vault_internal` — `tokenization.cppm:416-422` |
| `VaultId::generate` / `GlobalTokenId::generate` | `payment_methods.rs:44-55` / `global_id.rs:149-153, global_id/token.rs:38-40` | `uuid_v7`, `uuid_v7_simple`, `cell_id`, `global_token_id` — `tokenization.cppm:550-586` |
| `common_utils::date_time::now` | `common_utils/src/lib.rs:93-97` | `primitive_date_time_now` — `tokenization.cppm:629-665` |

## Implementation disposition

The eleven `Not ported` bullets from the prior pass are resolved below. Line refs are to `include/app/handlers/tokenization.cppm` unless stated.

- `get_token_vault_core` — PORTED — `tokenization.cppm:328-377`: record lookup, Disabled → `GenericNotFoundError`, `VaultRetrieveRequest`, `retrieve_value_from_vault`, `data` extraction with `unwrap_or(Null)` (`core/tokenization.rs:167-207`). No route, matching upstream (no route in `routes/tokenization.rs`). No longer called as a pre-delete step; delete now calls the upstream-equivalent `delete_payment_method_data_from_vault_internal` (`416-422`, `core:135-143`).
- Vault service integration — PORTED (local locker) — `AddVaultRequestWire` (`45-53`) encoded and written through `call_to_vault_add` (`383-399`) with `WriteMode::Insert` semantics and the configured TTL (`674-679`, upstream `core:44-61, conf.locker.ttl_for_storage_in_secs`). Local AES-wrapped TTL-cache entry replaces the external locker.
- `GcmAes256` / KMS encryption — PORTED (local crypto) — `vault_wrap_key` resolves from `KeyStore` with one-time seed (`597-614`), `encrypt_vault_data`/`decrypt_vault_data` use `AesWrap` (`616-627`). The processor key store / `GcmAes256` API has no local equivalent; see Shared contract requests.
- Real persistence — PORTED — `create_tokenization` (`260-263`), `find_tokenization_by_string` (`276`), `update_tokenization` (`311-319`). Cache holds only wrapped vault bytes; the business record is repository-backed.
- `VaultId::generate` / `GlobalTokenId::generate` — PORTED — `uuid_v7` (`565-578`), `uuid_v7_simple` (`580-586`, the 32-hex `as_simple()` form), `cell_id` (`550-555`), `global_token_id` (`557-562`) produce `{cell}_tok_{32hex}`, exactly the `GlobalId::generate` shape (`global_id.rs:149-153`, `global_id/token.rs:38-40`). Local v7 UUID and keyed-digest cell substitute external `generate_uuid_v7()` and `conf.cell_information.id`.
- `created_at` — PORTED — `primitive_date_time_now` (`629-665`) emits `time::PrimitiveDateTime` serde shape `YYYY-MM-DD HH:MM:SS.nnnnnnnnn`, matching `common_utils::date_time::now()` (`common_utils/src/lib.rs:93-97`). Prior ISO-8601 `T...Z` string removed.
- `merchant_id` source — PORTED (local provider) — `resolve_merchant_id` (`539-548`) uses the auth-derived merchant id canonicalized via `find_merchant_account_by_string`, substituting for `provider.get_account().get_id()` (`core:72`).
- `version` / `updated_at` / `locker_id` fields — DEFERRED (not wire-visible) — `TokenizationRecord` (`app/store/ops_records.cppm:269-320`) has no `updated_at`/`version` columns; `locker_id` maps to `vault_id` (`tokenization.cppm:255`). These fields originate in upstream `AddVaultResponse`/`Tokenization` (`core/tokenization.rs:70-79`) and never appear on either route's wire response, so they do not affect parity. See Shared contract requests.
- Auth resource binding — PORTED — `authorize_request` (`471-502`) runs the `api_or_client_auth` two-branch flow: API-key branch with explicit `allow_connected_scope_operation=false`/`allow_platform_self_operation=false` enforcement (`504-530`), SDK fallback with the payload-bound resource injected as `x-resource-id` and enforced via `.id_source("x-resource-id")` (`485-499`). Customer id (`106-118`) and session id (`155-174`) are extracted before auth exactly as in `routes/tokenization.rs:42,81`.
- Upstream error mapping — PORTED — `upstream_error` (`693-703`) sets the exact `ApiErrorResponse` variant name into `AppError.kind_name`: `TokenizationRecordNotFound` (`280-285`, message `core:112-114, api_error_response.rs:762-763`), `UnprocessableEntity` (`289-294`, `core:120-123`), `GenericNotFoundError` (`298-303,347-352`, `core:127-129,182-185`), `InternalServerError` (`314-318,338-342,406-410`). HTTP status comes from the matching `AppErrorKind` (`706-717`) — 404 / 422 / 404 / 500.
- ID newtype serialization — PORTED — local wire structs with plain `std::string` fields (`30-42`) are encoded with `serde::Ser::encode_json` (`129,188`); requests are decoded from plain scalars via `decode_generic` (`97-104,146-153,425-463`). The `{value: ...}` DTO wrappers in `app/dto/tokenization.cppm` are no longer on the wire for these routes. The DTO file was left untouched (not exclusive to this module).

## Shared contract requests

- `app/store/ops_records.cppm:269-320` — `TokenizationRecord` lacks `updated_at` and `version` columns present in upstream `Tokenization` (`hyperswitch_domain_models/src/tokenization.rs:12-21`). Adding them would complete the persisted model; not wire-visible on the two routes.
- `app/auth/registry.cppm:92,110` — the two tokenization rows use `AuthScheme::API_KEY`, not `V2_API_KEY`, so the authorizer does not enforce the V2 API-key claim branch natively. The handler enforces connected/platform flags locally (`504-530`); a registry scheme change would remove that workaround.
- `core_serde` / `app_dto` — no `FieldConverter`/`Reflector` exists for a plain-string id newtype; the converted DTO wrappers emit `{"value": ...}`. This handler uses local wire structs to bypass the wrapper; a shared transparent-newtype facility would let the DTO be the wire surface again.
- `app_crypto` — no `GcmAes256`/`EncodeMessage` and no processor key-store/KMS abstraction. Local `AesWrap` + `KeyStore` substitute (`597-627`).
- `app/services`/`connector` — no `pm_vault` service (`call_to_vault`, `retrieve_value_from_vault`, `delete_payment_method_data_from_vault_internal`). Local cache-backed helpers substitute (`383-422`).

## Files changed

- `include/app/handlers/tokenization.cppm` — rewritten: three upstream core functions ported as local helpers, wire shapes/auth/errors/persistence aligned, local in-process vault/crypto substitutes, no C++-only routes.
- `docs/hyperswitch-parity/tokenization.md` — this report.
- `include/app/dto/tokenization.cppm` — **unchanged** (not exclusive; wire shapes handled by local handler structs).

## Result

- Rows moved to MATCH: 2 (`POST /v2/tokenize`, `DELETE /v2/tokenize/:id`); all 6 summary dimensions moved to MATCH.
- Rows remaining: 0. C++-only routes removed: 0 (none existed).
- Shared requests: 5 (listed above); none block route parity.
