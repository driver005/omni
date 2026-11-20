# Parity: tokenization

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/tokenization.rs` (scope/paths from `routes/app.rs:1982-1998`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/tokenization.rs`
- C++: include/app/handlers/tokenization.cppm
- Checked: 2026-11-20 (re-verified against current `tokenization.cppm`)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | `/v2/tokenize` POST + `/v2/tokenize/:id` DELETE on both sides (`tokenization.cppm:184-211`) |
| Input types | PARTIAL | Field names align; upstream newtype IDs (`GlobalCustomerId`, `Secret` JSON) flattened into `{value}` string wrappers / `std::string` in C++ |
| Output types | PARTIAL | `{id, created_at, flag}` / `{id}` fields present but each scalar wrapped as `{value: ...}` in C++ (`tokenization.cppm:96-105,176-179`) |
| Auth/permissions | PARTIAL | Client-auth binding now enforced: customer id / session id from payload bound via injected `x-resource-id` claim + `id_source` client rule, with `allow_connected(false)`/`allow_platform(false)` (`tokenization.cppm:46-54,128-145,219-250`). Local auth substitute (RouteAuthRegistry + claim injection, not upstream `V2ClientAuth` type) → PARTIAL |
| Persistence | PARTIAL | Was STUB; now repository-backed `create_tokenization`/`find_tokenization_by_string`/`update_tokenization` (`tokenization.cppm:87,147,168`), vault bytes in TTL cache (`72-76`), KeyStore-resolved wrap key (`302-316`). External vault/DB/KMS still substituted |
| Domain logic | PARTIAL | Was STUB; now uuid_v7 stand-in (`59,271-287`), keyed-digest cell id (`65,263-269`), ISO-8601 `now_value` (`66,355-386`), TTL (`72-76,393-397`), retrieve/unwrap (`164,330-345`), Enabled→Disabled transition via repository (`84,156-168`). External locker/KMS substituted |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/tokenize` | `create_token_vault_api` → `create_vault_token_core` (`tokenization.rs:35-69`, `core/tokenization.rs:34-96`) | `handle_create_token_vault_api` (`tokenization.cppm:26-108`) | `GenericTokenizationRequest` | `GenericTokenizationResponse` | `api_or_client_auth(V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer(customer_id)))` (`tokenization.rs:56-65`); C++ binds customer id from payload (`tokenization.cppm:46-54`) | Upstream: `call_to_vault`, `insert_tokenization` (`core/tokenization.rs:57-86`); C++: `AesWrap` + TTL cache put (`tokenization.cppm:71-76`), `repository.create_tokenization` (`87`) | PARTIAL | paths: `app.rs:1987-1991` / `tokenization.cppm:184-199`; types: `core/tokenization.rs:37-38,90-94` / `dto/tokenization.cppm:71-94`; auth: `tokenization.rs:56-65` / `tokenization.cppm:46-54,219-250` |
| DELETE `/v2/tokenize/:id` | `delete_tokenized_data_api` → `delete_tokenized_data_core` (`tokenization.rs:73-105`, `core/tokenization.rs:100-163`) | `handle_delete_tokenized_data_api` (`tokenization.cppm:110-182`) | `DeleteTokenDataRequest` (`customer_id`, `session_id`) from body + `GlobalTokenId` from path | `DeleteTokenDataResponse` | `V2ApiKeyAuth{false,false}` + `V2ClientAuth(ResourceId::PaymentMethodSession(session_id))` (`tokenization.rs:92-101`); C++ now enforces session-id binding via `x-resource-id` claim + `id_source` rule (`tokenization.cppm:128-145,142,241-248`) | Upstream: `get_entity_id_vault_id_by_token_id`, vault delete, `update_tokenization_record(Disabled)` (`core/tokenization.rs:109-156`); C++: `retrieve_vault_data` pre-check (`164`), cache erase (`165`), `repository` find/update Disabled (`147,167-171`) | PARTIAL | paths: `app.rs:1994-1995` / `tokenization.cppm:200-209`; id-from-path: `tokenization.rs:76,82` / `tokenization.cppm:135-141`; errors: `core/tokenization.rs:112-130` / `tokenization.cppm:147-159`; auth: `tokenization.rs:92-101` / `tokenization.cppm:128-145,219-250` |

## Not ported
- `get_token_vault_core` (`core/tokenization.rs:167-207`): retrieve tokenization record, reject Disabled, `retrieve_value_from_vault`, return vault `data` JSON. No route in `routes/tokenization.rs` and no C++ handler.
- Vault service integration: `pm_vault::call_to_vault::<AddVault>` with `VaultQueryParam::WriteMode::Insert` and TTL `conf.locker.ttl_for_storage_in_secs` (`core/tokenization.rs:44-61`) — replaced by local `AesWrap` (`tokenization.cppm:61-63`), no TTL, no external locker.
- `GcmAes256` / KMS-based encryption with processor key store (`common_utils::crypto::{GcmAes256, EncodeMessage}`, `core/tokenization.rs:6-7,106,110,152`) — C++ uses hard-coded `"hyperswitch-vault-wrap-key"` (`tokenization.cppm:253-256`).
- Real persistence: `insert_tokenization`, `get_entity_id_vault_id_by_token_id`, `update_tokenization_record` (`core/tokenization.rs:82-86,109-115,149-156`) — C++ uses flat cache keys via `token_key()` (`tokenization.cppm:268-319`).
- `VaultId::generate(uuid_v7)` and `GlobalTokenId::generate(cell_id)` (`core/tokenization.rs:40,71`) — C++ uses `uuid_v4()` and `payment_id("tok_")` (`tokenization.cppm:54-55`).
- `created_at` = `common_utils::date_time::now()` (`core/tokenization.rs:75-76`) vs C++ epoch-seconds string (`tokenization.cppm:245-251`).
- `merchant_id` from `provider.get_account().get_id()` (`core/tokenization.rs:72`) vs C++ `x-merchant-id` header (`tokenization.cppm:57`).
- `version: ApiVersion::V2`, `updated_at`, `locker_id` from `AddVaultResponse` (`core/tokenization.rs:74-78`) not represented in `TokenState` (`tokenization.cppm:189-197`).
- Auth resource binding: `V2ClientAuth(ResourceId::PaymentMethodSession(session_id))` (`tokenization.rs:97-99`) — C++ parses `session_id` in the DTO (`dto/tokenization.cppm:57-60,191`) but never uses it in `authorize_request` (`tokenization.cppm:204-227`); `allow_connected_scope_operation:false` / `allow_platform_self_operation:false` flags also dropped.
- Upstream error mapping: `TokenizationRecordNotFound`, `UnprocessableEntity`, `GenericNotFoundError` (`core/tokenization.rs:112-130`) mapped to generic `NOT_FOUND`/`VALIDATION_FAILED` AppError kinds in C++ (`tokenization.cppm:123-133`).
- ID newtype serialization: upstream IDs serialize as plain strings; C++ wraps them as `{value: ...}` objects (`dto/tokenization.cppm:12-50,97-155`).

## Implementation disposition

Eleven bullets from `## Not ported`, re-verified against current `include/app/handlers/tokenization.cppm`. Line refs are to that file unless stated.

- `get_token_vault_core` — SUBSTITUTED — `retrieve_vault_data` rejects Disabled, fetches the wrapped entry from cache and unwraps it (`330-345`); exercised as a pre-delete verification (`164`). No route, matching upstream (no route in `routes/tokenization.rs`). Real `retrieve_value_from_vault` still needs an external locker.
- Vault service integration — SUBSTITUTED — `encrypt_vault_data` uses `AesWrap` (`318-322`) with TTL cache put (`71-76`, TTL stand-in `86400` at `393-397`). Local locker stand-in; no `pm_vault::call_to_vault` or external `conf.locker.ttl_for_storage_in_secs`.
- `GcmAes256` / KMS encryption — SUBSTITUTED — `vault_wrap_key()` resolves the wrap key from `m_context->key_store().get("vault","wrap")`, seeding once if absent (`302-316`); `AesWrap` encrypt/decrypt (`318-328`). Local key store + AES wrap, not KMS/`GcmAes256`. Hard-coded string survives only as a one-time seed (`388-391`).
- Real persistence — IMPLEMENTED — business records use injected `repository`: `create_tokenization` (`78-91`), `find_tokenization_by_string` (`147`), `update_tokenization` (`167-171`). Cache holds only wrapped vault bytes, not business records. Upstream-specific DB ops abstracted by the repository service; no flat `token_key()` cache remains.
- `VaultId::generate` / `GlobalTokenId::generate` — SUBSTITUTED — `uuid_v7()` is a deterministic v7-shaped stand-in over `IdGenerator::uuid_v4` (`59,271-287`); `global_token_id()` uses a `KeyedHasher` digest of merchant id as the external `cell_id` stand-in (`65,263-269`).
- `created_at` — IMPLEMENTED — `now_value()` produces UTC ISO-8601 with millisecond precision, matching `common_utils::date_time::now()` shape (`66,355-386`); prior epoch-seconds string removed.
- `merchant_id` source — SUBSTITUTED — `resolve_merchant_id()` reads `x-merchant-id` then canonicalizes via `repository.find_merchant_account_by_string` (`60,252-261`). Upstream provider-account id not available locally.
- `version` / `updated_at` / `locker_id` fields — DEFERRED — `TokenizationRecord` sets token_id, merchant_id, customer_id, entity_id, vault_id, status, created_at only (`78-85`). Those three fields originate in upstream `AddVaultResponse` from the external locker, so they cannot be populated without it. Justified external-dependency deferral: needs a real locker returning `AddVaultResponse`.
- Auth resource binding — IMPLEMENTED — session id parsed and required (`128-133`), passed as `bound_resource` (`142`), injected as `x-resource-id` claim (`241`) and enforced via `.id_source("x-resource-id")` with `.allow_connected(false).allow_platform(false)` (`242-248`); registry row carries false scope flags (`226-227`). Customer id bound the same way on create (`46-54`). Local mechanism is claim injection + RouteAuthRegistry rather than upstream `V2ClientAuth` type.
- Upstream error mapping — SUBSTITUTED — still generic AppError kinds: `NOT_FOUND` (`149,157,338,342`), `VALIDATION_FAILED` (`48,62,131,153`), `INVALID_REQUEST` (`139`), via `AppErrorBuilder` (`347-353`). No `TokenizationRecordNotFound` / `UnprocessableEntity` / `GenericNotFoundError` variants. Unchanged by this pass.
- ID newtype serialization — SUBSTITUTED — DTO-level wire shape unchanged: handlers still set `{value: ...}` wrappers on `GlobalTokenIdValue`, `PrimitiveDateTimeValue`, `TokenizationFlagValue` (`97-105,176-179`).
