# Parity: apple_pay_certificates_migration

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/apple_pay_certificates_migration.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2457-2468`
- Upstream core: `docs/hyperswitch/crates/router/src/core/apple_pay_certificates_migration.rs` (V1). Models: `docs/hyperswitch/crates/api_models/src/apple_pay_certificates_migration.rs`
- C++: `include/app/handlers/apple_pay_certificates_migration.cppm`
- Checked: 2026-11-20 (re-verified after handler rewrite)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Unchanged. One POST on `/apple_pay_certificates_migration` scope both sides (upstream feature-gated `olap`+`v1`; gating still DEFERRED in C++, see disposition). |
| Input types | PARTIAL | Wire shape now decoded by typed local DTO `MigrationRequestWire` (`apple_pay_certificates_migration.cppm:106`, Serializable `:55-65`) for plain-string `Vec<MerchantId>`; generic-tree fallback still accepts `{"value":...}` wrappers (`:110-115`, `:241-266`). Partial because the typed DTO is a documented local substitute for upstream's serde-derived model. |
| Output types | MATCH | `MigrationResponseWire` serializes plain `std::vector<std::string>` ids under `migration_successful`/`migration_failed` (`:41-50`, `:67-78`), matching upstream `Vec<MerchantId>` wire shape; response set at `:203-205`. Wrapper encoding and all-success echo removed. |
| Auth/permissions | PARTIAL | Still admin-only via `auth_rules().rule_for(...)` + `authorizer().authorize_route` (`:91-93`), but token `AppPermission::ADMIN` vs upstream `AdminApiAuth`. `Flow` now recorded (`record_flow` `:209`) and `LockAction::NotApplicable` documented (`comment :208`); different auth stack keeps it PARTIAL. |
| Persistence | PARTIAL | Repository-backed: merchant lookup `:127`, `key_store().get` `:139`, MCA list/update `:156`/`:188`; business records to repository, cache holds only 300s status `:314-323`. Internal behavior is a documented substitute (keystore + per-connector update vs upstream master-key store + `update_multiple_merchant_connector_accounts`), so PARTIAL per plan §28. |
| Domain logic | PARTIAL | Full flow now implemented with documented substitutes: key derivation `KeyedHasher` `:148-150`, metadata extraction stub `:268-290`, `AesWrap` re-encryption `:186`, partition `:193-199`, Flow/API metrics `:209-211`. Not MATCH because crypto/metadata are local substitutes, but wire shape matches and no STUB remains. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/apple_pay_certificates_migration` (`""` under scope) | `apple_pay_certificates_migration` | `handle_apple_pay_certificates_migration` | Up: `Json<ApplePayCertificatesMigrationRequest>` (`merchant_ids: Vec<MerchantId>`); C++: typed `MigrationRequestWire` decode of plain-string ids (`:106`), generic-tree fallback for `{"value":...}` wrappers (`:110-115`) | Up: `ApplePayCertificatesMigrationResponse` (`migration_successful`, `migration_failed` as `Vec<MerchantId>`); C++: plain-string id arrays, real per-merchant partition (`:203-205`, `:193-199`) | Up: `AdminApiAuth`, `Flow::ApplePayCertificatesMigration`, `LockAction::NotApplicable`; C++: `auth_rules`+`authorizer` token ADMIN (`:91-93`), `record_flow` (`:209`), lock-NA comment (`:208`) | Up: `accounts_store` merchant key store + MCA lookup, `crypto_operation` encrypt, `update_multiple_merchant_connector_accounts`; C++: `repository` merchant/MCA lookups+update (`:127`,`:156`,`:188`), `key_store` (`:139`), `AesWrap` (`:186`), TTL status cache only (`:318`) | PARTIAL | Up `routes/apple_pay_certificates_migration.rs:10-30`, `app.rs:2459-2467`, `core/apple_pay_certificates_migration.rs:18-114`, `api_models/apple_pay_certificates_migration.rs:1-10`; C++ `apple_pay_certificates_migration.cppm:85-215`, route `:217-233` |

## Not ported
- Per-merchant key store load via master key + `get_merchant_key_store_by_merchant_id`: `core/apple_pay_certificates_migration.rs:32-37`. No key store in C++ (`apple_pay_certificates_migration.cppm:73-82`).
- Merchant connector account lookup `find_merchant_connector_account_by_merchant_id_and_disabled_list` (disabled=true) + not-found mapping: `core/apple_pay_certificates_migration.rs:39-47`. Absent; no MCA store.
- Apple-pay metadata extraction `get_applepay_metadata` with connector-name error logging: `core/apple_pay_certificates_migration.rs:51-62`. Absent.
- Re-encryption `domain_types::crypto_operation` with `Identifier::Merchant`, `key_store.key`, JSON serialization of metadata, and `ConnectorWalletDetailsUpdate` build: `core/apple_pay_certificates_migration.rs:63-85`. No crypto/key-manager in C++; comment explicitly concedes this (`:73-78`).
- Persistence `update_multiple_merchant_connector_accounts` and success/failure partitioning into `migration_successful`/`migration_failed` based on the update result: `core/apple_pay_certificates_migration.rs:88-105`. C++ marks all ids successful, `migration_failed` always empty (`:84-94`).
- Response id wire encoding — upstream `Vec<MerchantId>` (plain string); C++ emits `{"value":...}` wrappers, which upstream request comment treats as an alternative input shape but not the response shape (`:87`, model `api_models/apple_pay_certificates_migration.rs:2-4`).
- Typed request deserialization: upstream uses `web::Json<ApplePayCertificatesMigrationRequest>` with serde derive (`api_models/apple_pay_certificates_migration.rs:7-10`); C++ cannot decode its own DTO and parses a generic tree (`apple_pay_certificates_migration.cppm:41-71`).
- Upstream request-handler wrapper semantics — `Flow` metric, `AdminApiAuth`, `LockAction::NotApplicable`, `server_wrap` error mapping: `routes/apple_pay_certificates_migration.rs:17-28`. C++ uses flat permission check + generic cache (`:27-34`, `:79-82`).
- `ApiEventMetric` instrumentation on the request type: `api_models/apple_pay_certificates_migration.rs:12`. Absent.
- Feature gating `#[cfg(all(feature = "olap", feature = "v1"))]` / `#[cfg(feature = "v1")]` on scope and core: `app.rs:2459`, `core/apple_pay_certificates_migration.rs:17`. No equivalent gating in C++.

## Implementation disposition

Re-verified against current handler `include/app/handlers/apple_pay_certificates_migration.cppm`. All 10 original `## Not ported` bullets dispositioned below.

- Per-merchant key store load (`get_merchant_key_store_by_merchant_id`) — SUBSTITUTED — `key_store().get("merchant_key", merchant_id)` with payment-hash/publishable-key/deterministic fallbacks (`:139-147`) and `KeyedHasher` derivation (`:148-150`); no master-key store.
- MCA lookup `find_merchant_connector_account_by_merchant_id_and_disabled_list` + not-found mapping — SUBSTITUTED — `repository.list_merchant_connectors()` filtered by `get_merchant_id()` (`:155-160`), empty result → per-merchant failure (`:161-165`); no disabled-list filter API.
- `get_applepay_metadata` extraction + connector-name error logging — SUBSTITUTED — `extract_applepay_metadata` (`:169`, `:268-290`) returns fixed `{"apple_pay":{}}` for `apple_pay_combined`/`apple_pay`; parse failures counted per connector name via metrics (`:173-176`).
- `crypto_operation` re-encryption (`Identifier::Merchant`, key, JSON) + `ConnectorWalletDetailsUpdate` build — SUBSTITUTED — `AesWrap{}.encrypt` over JSON metadata (`:186`) + `with_encrypted_metadata` (`:187`, `:292-312`) written through the repository; no `Identifier::Merchant` crypto operation.
- `update_multiple_merchant_connector_accounts` + success/failure partitioning — SUBSTITUTED — per-connector `repository.update_merchant_connector` with `updated` flag (`:167-191`) and `successful`/`failed` split (`:193-199`); bulk update API replaced by loop.
- Response id wire encoding (`Vec<MerchantId>` plain strings) — IMPLEMENTED — `MigrationResponseWire` uses plain `std::vector<std::string>` (`:41-50`, `:67-78`), response set at `:203-205`; wrapper encoding removed.
- Typed request deserialization — IMPLEMENTED — `serde::Ser::deserialize<MigrationRequestWire>` (`:106`) with local `Serializable` specialization (`:55-65`); generic-tree tolerance retained as fallback (`:110-115`).
- `server_wrap` wrapper semantics (Flow / AdminApiAuth / `LockAction::NotApplicable` / error mapping) — SUBSTITUTED — `auth_rules`+`authorizer` permission check (`:91-93`), `record_flow("ApplePayCertificatesMigration")` (`:209`), lock-NA comment `:208`, `send_error` mapping (`:325-336`); still token ADMIN vs `AdminApiAuth`.
- `ApiEventMetric` instrumentation on request type — SUBSTITUTED — named counters `APPLE_PAY_CERTIFICATES_MIGRATED` and `api_event.APPLE_PAY_CERTIFICATES_MIGRATION` (`:210-211`) instead of a trait impl.
- Feature gating `olap`+`v1` / `v1` — DEFERRED — no `#[cfg]` equivalent; `install` always registers the route (`:217-233`). External dependency: C++ has no compile-time feature-flag system mapping to these Cargo features. Remove by adding a build-time feature gate around route registration.

No new STUB introduced; upstream `core/apple_pay_certificates_migration.rs` gaps are now either implemented, a documented deterministic substitute, or the single DEFERRED feature-gate item above. Verdict changes: Output types PARTIAL→MATCH, Persistence STUB→PARTIAL, Domain logic STUB→PARTIAL; Input types/Auth stay PARTIAL with updated evidence; route row evidence updated, verdict stays PARTIAL.
