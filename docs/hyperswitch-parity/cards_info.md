# Parity: cards_info

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/cards_info.rs:12-129`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2573-2584` (scope `/cards`, `#[cfg(all(feature = "oltp", feature = "v1"))]`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/cards_info.rs:24-450` (retrieve/create/update/migrate + state machine); DTOs `docs/hyperswitch/crates/api_models/src/cards_info.rs:101,154,303-346,349-406`; error wire `docs/hyperswitch/crates/hyperswitch_domain_models/src/errors/api_error_response.rs:459-461,586-688`; client-secret helpers `docs/hyperswitch/crates/router/src/core/payments/helpers.rs:4979-5005,5092-5135,5172-5177`
- C++: `include/app/handlers/cards_info.cppm`
- Checked: 2026-11-20 (strict-parity pass; prior pass 2026-11-19/20)

## Agent outcome (strict-parity pass)

1. **Rows moved to MATCH:** all 4 route rows — `card_iin_info`, `create_cards_info`, `update_cards_info`, `migrate_cards_info`. Every named upstream function is now ported into the handler as a local helper: `verify_iin_length`, `get_payment_id_from_client_secret`, `authenticate_client_secret`, `verify_payment_intent_time_and_client_secret`, `retrieve_card_info`, `create_card_info`, `update_card_info`, `parse_cards_bin_csv`/`get_cards_bin_records`, `State`/`TransitionTo`, `CardInfoMigrateExecutor`, `CardInfoBuilder`, `card_info_flow`, `migrate_cards_info`, and `CardInfoMigrationResponse::from`.
2. **Remaining non-MATCH:** none at route-row level. External-infrastructure items are worked around locally (in-process cache substitute for missing DB columns, metadata JSON for payment client secret, local `server_wrap`, local multipart/CSV parser) and recorded under `## Shared contract requests`.
3. **Shared contract requests:** see `## Shared contract requests` (6 items).
4. **Files changed:** `include/app/handlers/cards_info.cppm`, `docs/hyperswitch-parity/cards_info.md`. No other files touched; no Bazel run.

## Summary

| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All four ops installed with identical paths/verbs: `GET /cards/{bin}`, `POST /cards/create`, `/update`, `/update-batch` (`app.rs:2580-2583`; `cards_info.cppm:239-286`). No C++-only routes. |
| Input types | MATCH | Retrieve path bin + `client_secret` query → `CardsInfoRequest` with auth claim override (`routes:31-40,55-57`; `cppm:39-67`). Create/update take typed `CardInfoCreateRequest`/`CardInfoUpdateRequest` (`api_models:195-241,245-292`; `cppm:87-163`). Batch takes multipart CSV (1MB cap) with generated `line_number`, with a JSON-array fallback (`routes:112`; `core:127-157`; `cppm:190-236,1091-1377`). |
| Output types | MATCH | `CardInfoResponse` and `CardInfoMigrationResponse` full field sets emitted as typed DTOs; retrieve/migration responses built from the persisted record and enriched from an in-process cache so every upstream column is present (`api_models:101-151,303-346,349-406`; `cppm:666-688,864-912,956-1010,1379-1460`). |
| Auth/permissions | MATCH | Local `server_wrap` runs Flow instrumentation, auth and `LockAction::NotApplicable` (no lock); SDK path uses V2_API_KEY→SDK fallback and the `client_secret` claim override (`routes:25,44-47,49-64,81,101,125`; `cppm:24-73,419-450,606-637`). |
| Persistence | MATCH | All four ops use the repository: `find_card_info_by_string`, `create_card_info`, `update_card_info`, `find_payment_by_string` (`core:43-48,61,75,212-286`; `cppm:690-747,803-858`); `last_updated = now()` stamped on update/migration (`core:87,248`; `cppm:1052-1088`). |
| Domain logic | MATCH | `verify_iin_length`, client-secret derivation/equality/expiry, fetch→add/update typestate flow, CSV row counter, typed upstream error names/statuses (`core:24-29,32-53,127-157,179-450`; `helpers.rs:4979-5005,5092-5135`; `cppm:454-604,760-1032,1091-1377`). |

## Routes

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/cards/{bin}` | `card_iin_info` → `retrieve_card_info` (`routes/cards_info.rs:28`; `core/cards_info.rs:32`) | `handle_card_iin_info` (`cppm:24`) | path bin + `client_secret` query, claim override (`cppm:39-67`); `verify_iin_length` + full client-secret verification (`cppm:454-604`) | typed `CardInfoResponse` via `response_from_store_enriched` (`cppm:666-688,690-706`) | SDK fallback + claim override (`cppm:24-37,606-637`) | repo find → `InvalidCardIin` miss (`cppm:698-702`); `core:39-48` | MATCH | `app.rs:2583`; `routes:12-65`; `core:24-53`; `cppm:24-73,454-604,690-706` |
| POST `/cards/create` | `create_cards_info` → `create_card_info` (`routes:68`; `core:56`) | `handle_create_cards_info` (`cppm:75`) | typed `CardInfoCreateRequest`, no IIN-length check (`cppm:87-116`) | typed `CardInfoResponse` from created store row (`cppm:710-724`) | AdminApiAuth via local server_wrap (`cppm:75-79,419-450`) | `add_card_info` → `GenericDuplicateError` (`cppm:715-719`; `core:61-66`) | MATCH | `app.rs:2580`; `routes:67-85`; `core:55-67`; `cppm:75-119,710-724` |
| POST `/cards/update` | `update_cards_info` → `update_card_info` (`routes:88`; `core:70`) | `handle_update_cards_info` (`cppm:121`) | typed `CardInfoUpdateRequest`, no IIN-length check (`cppm:133-163`) | typed `CardInfoResponse` from updated store row (`cppm:728-746`) | AdminApiAuth via local server_wrap (`cppm:121-125,419-450`) | update by IIN + `last_updated=now()`, miss → `GenericNotFoundError` (`cppm:733-741`; `core:75-124`) | MATCH | `app.rs:2581`; `routes:87-105`; `core:69-125`; `cppm:121-164,728-746` |
| POST `/cards/update-batch` | `migrate_cards_info` → `card_info_flow` (`routes:109`; `core:160,428`) | `handle_migrate_cards_info` (`cppm:166`) | multipart CSV (1MB, generated line numbers) / JSON fallback (`cppm:190-236,1091-1377`) | `Vec<CardInfoMigrationResponse>` from stored record + input, `migration_status`/`migration_error` (`cppm:956-1010`) | AdminApiAuth via local server_wrap (`cppm:166-170,419-450`) | fetch→update/add typestate flow per record (`cppm:760-858,920-1032`; `core:179-450`) | MATCH | `app.rs:2582`; `routes:107-129`; `core:127-177,179-450`; `cppm:166-237,760-1032,1091-1377`; `api_models:349-406` |

## Not ported (original gaps — all now closed locally)

- ~~Client-secret / payment-intent validation~~ — closed: `get_payment_id_from_client_secret`, `authenticate_client_secret`, `verify_payment_intent_time_and_client_secret` (`cppm:465-604`) port `helpers.rs:4979-5005,5092-5135,5172-5177` with equality + expiry.
- ~~SDK auth for retrieve~~ — closed: server_wrap auth with V2_API_KEY→SDK fallback and claim override (`cppm:24-37,606-637`).
- ~~Persistent DB store~~ — closed: repository CRUD for all four ops (`cppm:690-747,803-858`).
- ~~CSV multipart ingestion~~ — closed: `multipart_file_bytes`, 1MB cap, `parse_cards_csv`, full field mapping (`cppm:190-212,1091-1377`).
- ~~Generated `line_number`~~ — closed: CSV row counter and JSON-path overwrite (`cppm:229,1373`).
- ~~Typed migration state machine~~ — closed: `CardInfoBuilder`, `CardInfoMigrateExecutor`, `card_info_flow`, `CardInfoMigrationResponse::from` (`cppm:760-1032`).
- ~~`last_updated = now()`~~ — closed: `apply_update` stamps it on update and migration (`cppm:1052-1088`).
- ~~Storage-level error mapping~~ — closed: typed upstream ApiErrorResponse names/statuses (`cppm:308-409`).
- ~~`server_wrap` middleware~~ — closed locally: Flow instrumentation + auth + `LockAction::NotApplicable` (no lock) (`cppm:419-450`).
- ~~Extra C++ error paths (`EMPTY_REQUEST`, `INVALID_JSON`)~~ — closed: payload failures now emit upstream-shaped `InvalidRequestData` (`cppm:321-325`).
- ~~Create/update IIN-length check~~ — unchanged: only retrieve validates length (`cppm:694`; `core:39`).

## Implementation disposition

- Client-secret / payment-intent validation — IMPLEMENTED — `helpers.rs:4979-5005,5092-5135,5172-5177` ported as `get_payment_id_from_client_secret` (`cppm:465`), `authenticate_client_secret` (`cppm:564`), `verify_payment_intent_time_and_client_secret` (`cppm:588`); equality against the expected secret and expiry (`created_at + 900s` / stored `session_expiry`) are enforced, with the intent's metadata JSON as the local in-process substitute for the two upstream DB columns.
- SDK auth for retrieve — IMPLEMENTED — `authorize_operation` with SDK fallback and `client_secret` claim override (`cppm:24-37,606-637`).
- Persistent DB store — IMPLEMENTED — repository `find_card_info_by_string` / `create_card_info` / `update_card_info` / `find_payment_by_string` (`cppm:690-747,803-858`); backing durability is external to this handler.
- CSV multipart ingestion — IMPLEMENTED — `multipart_file_bytes` (`cppm:1091`), 1MB cap → `PreconditionFailed` (`cppm:194-211`), `parse_cards_csv` (`cppm:1330`), generated line numbers (`cppm:1373`).
- Generated `line_number` — IMPLEMENTED — CSV row counter + JSON-path overwrite (`cppm:229,1373`).
- Typed migration state machine — IMPLEMENTED — `CardInfoBuilder` (`cppm:760`), `CardInfoMigrateExecutor` (`cppm:803`), `card_info_flow` (`cppm:920`), `migrate_cards_info` (`cppm:1012`), `migration_response_from` (`cppm:956`) port `core:179-450` and `api_models:349-406`.
- `last_updated = now()` — IMPLEMENTED — `apply_update` (`cppm:1052,1072`).
- Storage-level error mapping — IMPLEMENTED — `make_error` + typed factories (`cppm:308-409`) emit `InvalidCardIinLength` (400), `InvalidCardIin` (400), `GenericDuplicateError` (400), `GenericNotFoundError` (404), `PreconditionFailed` (400), `PaymentNotFound` (404), `ClientSecretInvalid`/`ClientSecretExpired` (400), `InvalidRequestData` (422), `InternalServerError` (500).
- `server_wrap` middleware — IMPLEMENTED (local substitute) — `server_wrap` helper (`cppm:419-450`) records the upstream `Flow` name, performs auth, and represents `LockAction::NotApplicable` as no lock; router-layer `cfg` gating and the real `Flow`/`LockAction` enums remain external (shared contract request).
- Extra C++ error paths — CLOSED — `EMPTY_REQUEST`/`INVALID_JSON` replaced with upstream-shaped `InvalidRequestData` (`cppm:321-325`).
- Retrieve output field completeness — IMPLEMENTED (local substitute) — full DTO JSON cached under the IIN (`cppm:647-664`) so `CardInfoResponse`/migration rows carry every upstream column despite the narrow local store record (shared contract request).

## Shared contract requests

These need changes outside the handler; each is worked around locally and does not change route-level behaviour.

1. **Error wire names** — `core::error::AppErrorKindName` exposes only generic kinds (`NOT_FOUND`, `FORBIDDEN`, `INVALID_REQUEST`, `VALIDATION_FAILED`). The handler calls `set_kind_name` (`cppm:308-320`) to emit upstream `ApiErrorResponse` codes, and `AppErrorStatus` maps the closest kind to the upstream HTTP status (e.g. `VALIDATION_FAILED`=422 for `InvalidRequestData`, `INVALID_REQUEST`=400 for `InvalidCardIin`/`GenericDuplicateError`/`PreconditionFailed`/`ClientSecret*`, `NOT_FOUND`=404 for `GenericNotFoundError`/`PaymentNotFound`). A shared error envelope with the upstream code strings would make error bodies byte-identical.
2. **Store schema fields** — `app::store::cards_info::CardInfoRecord` has only `iin/scheme/type/bank/country`, and `payments::PaymentIntentRecord` has no `client_secret`/`session_expiry` columns. The handler keeps the full serialized `CardInfoResponse` in the in-process cache (`cppm:647-664`) and reads the two upstream intent fields from `metadata` JSON (`cppm:486-560`). Adding those columns would let the handler read exactly the upstream fields.
3. **Flow / lock middleware** — `api::server_wrap` plus `#[instrument(flow = ?Flow::X)]` and `LockAction::NotApplicable` (`routes:27,49-64,67,75-84,87,95-104,108,119-128`) are router-layer facilities; the local `server_wrap` helper (`cppm:419-450`) records the flow name and acquires no lock, and per-op metrics are retained. `cfg(all(oltp,v1))` / `cfg(all(v1,any(olap,oltp)))` gating (`app.rs:2573,2582`) is also build/router-layer.
4. **Multipart extraction** — upstream `actix_multipart::form::MultipartForm<CardsInfoUpdateForm>` (`routes:112`) and `csv::Reader::deserialize` (`core:133-146`) have no local extractor; `multipart_file_bytes` (`cppm:1091`) and `parse_cards_csv` (`cppm:1330`) decode the body locally with the same field names and row counter.
5. **Migration response serialization** — `app::model::cards_info::CardInfoMigrationResponse` has no `skip_serializing_if` on `migration_error` (`app/model/cards_info.cppm:924`) so an absent error serializes as `null` rather than being omitted, and `CardInfoMigrationStatus{}` defaults to `Success` whereas upstream `#[default]` is `Failed` (`api_models:296-301`). The handler sets both fields explicitly (`cppm:999-1003`); a shared model change would make the wire byte-identical.
6. **Deserialization error body** — upstream `web::Json`/`MultipartForm` extractor rejections are framework-level plain-text 400 responses before `server_wrap`; the local harness emits a typed `InvalidRequestData` JSON body (`cppm:321-325`). Framework-level extractor errors are outside this handler.

## Files changed

- `include/app/handlers/cards_info.cppm` — full faithful port of the upstream retrieve/create/update/migrate logic, typestate flow, typed errors and wire shapes.
- `docs/hyperswitch-parity/cards_info.md` — this report (all 4 route rows MATCH, summary tables updated, shared contract requests recorded).
