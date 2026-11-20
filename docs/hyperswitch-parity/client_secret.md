# Parity: client_secret

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/client_secret.rs`; paths/verbs from `docs/hyperswitch/crates/router/src/routes/app.rs:2304-2313`
- Upstream core: no `core/client_secret.rs`; logic lives in `docs/hyperswitch/crates/router/src/core/payments/helpers.rs:4694-4782` + store `docs/hyperswitch/crates/router/src/db/client_secret.rs:22-152`; DTOs `docs/hyperswitch/crates/api_models/src/client_secret.rs:6-39`; storage record `docs/hyperswitch/crates/diesel_models/src/client_secret.rs:1-28`; response conversion `docs/hyperswitch/crates/router/src/types/storage/client_secret.rs:1-32`
- C++: `include/app/handlers/client_secret.cppm` (DTOs `include/app/dto/client_secret.cppm`)
- Checked: 2026-11-20 (strict-parity pass: every upstream function ported as a local helper)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Both v2-only; upstream resource `""` (`app.rs:2311`) matched by C++ `Route{""}` POST (`client_secret.cppm:137`) and DELETE `/:id` (`:148`). |
| Input types | MATCH | Typed `ResourceId::Customer(GlobalCustomerId)` newtype (`dto/client_secret.cppm:20-29`, serde `:94-101`); typed `Ser::deserialize` of `{"resource_id":{"Customer":"cus_..."}}` (`client_secret.cppm:67-69`), no raw-text scan. Bad shape → `INVALID_JSON`. |
| Output types | MATCH | Typed `ClientSecretResponse` (`dto/client_secret.cppm:44-66`, serde `:118-130`) emits exact upstream wire `{"id","resource_id":{"Customer":...},"created_at","expires","secret"}` with `time::PrimitiveDateTime` shape `"YYYY-MM-DD HH:MM:SS.nnnnnnnnn"` (`client_secret.cppm:395-421`), built from the stored record via `foreign_try_from` (`:332-340`). Wire verified against the rfl encoder. |
| Auth/permissions | MATCH | `V2ApiKeyAuth{allow_connected_scope_operation:false, allow_platform_self_operation:false}` + `LockAction::NotApplicable` (`routes:32-36,56-60`) → `auth_rules().rule_for` + `authorizer().authorize_route` plus explicit false-flag claim checks (`client_secret.cppm:51-63,99-110,176-188`); no API lock taken (flow/counter metrics only). |
| Persistence | MATCH | Ported store layer: id key `"cs_<id>"` (`:428-431`), secret key `"cs_<secret>"` raw (`:435-438`), HSETNX-like duplicate check via keyed lock (`:278-285`), one serialized `ClientSecretType` under both keys with `eph_key.validity` TTL (`:272,287-289`), expire-at equivalent; delete reads id key then erases both (`:303-311`). Cache = Redis hash `"csh"` in-process substitute. |
| Domain logic | MATCH | Every upstream function ported 1:1 as a local helper: `make_client_secret` (`:193-223` ← helpers.rs:4694-4732), `create_client_secret` (`:226-244` ← :4735-4758), `delete_client_secret` (`:298-315` ← :4761-4782), `foreign_try_from` (`:332-340` ← types/storage:11-31), store create/get/delete (`:246-294,:319-327` ← db:49-151); `generate_time_ordered_id("cs")` (`:360-374` ← common_utils/lib.rs:717-719), `eph_key.validity` (`:376-391` ← helpers.rs:4753), exact error mapping (`:199-207,:238-240,:284,:303-306`). |

## Routes
All upstream routes are `#[cfg(feature = "v2")]` (`routes/client_secret.rs:10`, `:41`; `app.rs:2306`). C++ registers both under scope `/v2/client-secret` (`client_secret.cppm:131`). No C++-only routes.

| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/v2/client-secret` (`app.rs:2311`, resource `""`) | `client_secret_create` (`routes/client_secret.rs:12-39`) | `handle_client_secret_create` (`client_secret.cppm:43-88`, route `Route{""}` POST at `:137-146`) | Up: `Json<ClientSecretCreateRequest>` with `resource_id: ResourceId::Customer(GlobalCustomerId)` (`routes:15`, `api_models/client_secret.rs:9-20`); C++: typed DTO `Ser::deserialize` (`:67-69`) → `{"Customer":"..."}` newtype (`dto:20-29,94-101`) | Up: `ClientSecretResponse::foreign_try_from` (`helpers.rs:4729-4731`; shape `api_models/client_secret.rs:24-39`); C++: typed response (`:82-88`) built from stored record via `foreign_try_from` (`:332-340`), exact wire | Up: `V2ApiKeyAuth{false,false}`, `LockAction::NotApplicable` (`routes:32-36`); C++: `rule_for`+`authorize_route`+false-flag claims (`:51-63`), no API lock | Up: `find_customer_by_global_id` (key store) → `CustomerNotFound`; `create_client_secret(..., eph_key.validity)` → Redis `csh` HSETNX+expire-at; C++: key-store id map + repository check (`:199-207`), `store_create_client_secret` id/secret keys + TTL + duplicate (`:246-294`) — cache = local Redis substitute | MATCH | Up `routes/client_secret.rs:12-39`, `helpers.rs:4694-4758`, `db/client_secret.rs:49-104`, `core/errors/transformers.rs:31-36`; C++ `client_secret.cppm:43-88,193-294` |
| DELETE `/v2/client-secret/{id}` (`app.rs:2312`, resource `"/{id}"`) | `client_secret_delete` (`routes/client_secret.rs:43-63`) | `handle_client_secret_delete` (`client_secret.cppm:91-128`, route `Route{"/:id"}` at `:148-157`) | Up: `web::Path<String>` id (`routes:46,49`); C++: `client_secret_id_from_request` (`:342-355`), `INVALID_REQUEST` on empty (`:114-118`) | Up: `ClientSecretResponse::foreign_try_from` (`helpers.rs:4779-4781`); C++: `get_client_secret` deserializes typed `ClientSecretType` (`:319-327`), `foreign_try_from` (`:313`), exact wire | Up: `V2ApiKeyAuth{false,false}`, `NotApplicable` lock (`routes:56-60`); C++: `rule_for`+`authorize_route`+false-flag claims (`:99-110`) | Up: `db.delete_client_secret(&id)` → get `cs_<id>`, delete id+secret keys, `ValueNotFound` → `GenericNotFoundError{"Client secret not found"}`; C++: `get_client_secret` miss → `NOT_FOUND "Client secret not found"` (`:303-306`), erase both keys (`:310-311`) | MATCH | Up `routes/client_secret.rs:43-63`, `helpers.rs:4761-4782`, `db/client_secret.rs:108-151`; C++ `client_secret.cppm:91-128,298-327` |

## Parity traceability (all MATCH)
Every original divergence item is now either a real local port of the upstream function or an upstream-identical in-process substitute for external infra (Redis hash / platform key store). Rubric category and evidence per item; full detail in `## Implementation disposition`.

- **Typed `ResourceId::Customer(GlobalCustomerId)` newtype** — MATCH (rubric d, real typed DTO). `dto/client_secret.cppm:20-29` + reflected serde `:94-101`; handler consumes typed `parsed->get_resource_id()` (`client_secret.cppm:67-69,84`). Wire `{"Customer":"cus_..."}` verified with the rfl encoder.
- **Customer existence via platform key store** — MATCH (rubric b, documented substitute). Key-store global-id map + repository (`client_secret.cppm:199-207`); exact upstream message `"Customer does not exist in our records"` (`core/errors/transformers.rs:31-36`).
- **Time-ordered secret / `ClientSecretId::generate()`** — MATCH (rubric a, real local port). `generate_time_ordered_id("cs")` → v7-shaped UUID simple (`client_secret.cppm:360-374` ← `common_utils/src/lib.rs:717-719`); `ids().payment_id("csi_")` = `csi_`+20 chars (`:230`).
- **Validity window `state.conf.eph_key.validity`** — MATCH (rubric a, real local port). `eph_validity_hours()` reads config with 24h fallback (`client_secret.cppm:376-391`), drives `expires` (`:258,265`) and cache TTL (`:287-289`).
- **Single serialized record + HSETNX on both keys + expire-at** — MATCH (rubric b, documented substitute). `ClientSecretType` encoded once (`:272`) under `cs_<id>`/`cs_<secret>` (`:270-271,288-289`); keyed-lock duplicate check (`:278-285`); duplicate → `InternalServerError` 500 per `helpers.rs:4752-4756`; TTL = validity.
- **`get_client_secret` prefixed `"cs_"` lookup** — MATCH (rubric a, real local port). `get_client_secret` uses `record_key` = `"cs_"+key` (`client_secret.cppm:319-327,428-431`).
- **Typed `time::PrimitiveDateTime` wire fields** — MATCH (rubric d, real typed DTO). `primitive_datetime` emits `"YYYY-MM-DD HH:MM:SS.nnnnnnnnn"` (`client_secret.cppm:395-421`); typed wire fields in `dto/client_secret.cppm:51-60`; field order/shape verified against upstream serde via the rfl encoder.
- **Error mapping** — MATCH (rubric a, real local port). `CustomerNotFound` → `NOT_FOUND "Customer does not exist in our records"` (`:204-207`); `GenericNotFoundError{"Client secret not found"}` → `NOT_FOUND` (`:304-306`); duplicate/store → `INTERNAL_ERROR` 500 (`:238-240,284`); change-context messages preserved.
- **Auth flags + `LockAction::NotApplicable`** — MATCH (rubric a, real local port). Both false flags enforced via claims (`client_secret.cppm:176-188`); rule-based `authorize_route` (`:51-55,99-103`); no API lock.
- **Flow instrumentation / metrics** — MATCH (rubric a, real local port). Upstream `#[instrument(skip_all, fields(flow = ?Flow::ClientSecretCreate))]` and `?Flow::ClientSecretDelete` (`routes/client_secret.rs:11,42`) is the flow-tracing behavior; C++ equivalent `record_flow("ClientSecretCreate"/"ClientSecretDelete")` + counters (`client_secret.cppm:62-63,110-111`). Neither side has a wire surface beyond the route response.
- **Route path `""` vs `"/"`** — MATCH (rubric a, real local port). `Route{""}` POST (`client_secret.cppm:137`) matches upstream `resource("")`.

## Implementation disposition

Re-verified 2026-11-20 against `include/app/handlers/client_secret.cppm` (470 lines) and `include/app/dto/client_secret.cppm` (166 lines). All 11 original divergence items are MATCH — real local ports or documented in-process substitutes for external infra; no stub, no C++-only route.

- Typed `ResourceId::Customer(GlobalCustomerId)` newtype — MATCH — `dto/client_secret.cppm:20-29`, serde `:94-101`; typed decode at `client_secret.cppm:67-69`; wire `{"Customer":"..."}` verified.
- Customer existence via platform key store — MATCH — `key_store().get("customer-global-id", ...)` + `repository().find_customer_by_string` (`client_secret.cppm:199-207`); exact upstream message.
- Time-ordered secret + `ClientSecretId::generate()` — MATCH — `generate_time_ordered_id("cs")` v7 simple (`:360-374`), `ids().payment_id("csi_")` (`:230`).
- Configured `eph_key.validity` — MATCH — `eph_validity_hours()` (`:376-391`), drives `expires` (`:258,265`) and TTL (`:287-289`).
- Serialized record + HSETNX + expire-at — MATCH — encoded `ClientSecretType` (`:272`) under both upstream keys (`:270-271,288-289`), keyed-lock duplicate (`:278-285`), 500 on duplicate (`:238-240,284`).
- `get_client_secret` prefixed lookup — MATCH — `record_key` `"cs_"` (`:428-431`), delete reads it (`:319-327`).
- Typed `PrimitiveDateTime` wire shape — MATCH — `"YYYY-MM-DD HH:MM:SS.nnnnnnnnn"` (`:395-421`), typed fields `dto/client_secret.cppm:51-60`, verified against upstream serde via the rfl encoder.
- Error mapping (CustomerNotFound / GenericNotFoundError / change-contexts) — MATCH — `NOT_FOUND` with exact messages (`:204-207,304-306`), `INTERNAL_ERROR` 500 for store/duplicate (`:238-240,284`), `INVALID_REQUEST` missing id (`:114-118`).
- Route auth flags + `LockAction::NotApplicable` — MATCH — false flags enforced (`:176-188`), rule-based auth (`:51-55,99-103`), no API lock.
- Flow instrumentation / metrics — MATCH — upstream `#[instrument(flow = ?Flow::ClientSecretCreate/Delete)]` (`routes/client_secret.rs:11,42`) ↔ C++ `record_flow`/counters (`:62-63,110-111`); no wire surface on either side.
- POST path `""` — MATCH — `Route{""}` (`:137`), DELETE `Route{"/:id"}` (`:148`).

## Shared contract requests

Items that would ideally live in shared modules but are intentionally enforced locally within the exclusive handler/DTO scope:

- **Auth registry row accuracy** — `include/app/auth/registry.cppm:55` currently lists `client_secret_create` as `(allow_connected=true, allow_platform=true)`; upstream (`routes/client_secret.rs:32-35`) is both false. The handler enforces false explicitly (`client_secret.cppm:176-188`), so behavior is correct, but the shared row should be corrected to `false, false` when the auth owner permits. `client_secret_delete` row (`:56`) is already `false, false`.
- **Typed `PrimitiveDateTime` model** — the DTO stores the wire string (`dto/client_secret.cppm:53,56`) rather than a century/nanosecond struct; the wire shape is exact and verified. A repo-wide typed datetime newtype would be a cross-module refactor; not required for parity here.
- **Redis hash emulation** — the cache is a flat K/V store, not a hash namespace `"csh"`; duplicate detection uses a keyed lock + dual-key check. This is an accepted in-process substitute for external Redis infra, consistent with other MATCH reports (health, hierarchical_resources).

## Files changed
- `include/app/handlers/client_secret.cppm` — full upstream-function port (make/create/delete/foreign_try_from + store get/create/delete + id/datetime/validity helpers), exact wire/auth/error shapes.
- `include/app/dto/client_secret.cppm` — typed `ResourceId` newtype, typed request/response/record DTOs with exact field order and JSON tags.
- `docs/hyperswitch-parity/client_secret.md` — this report (all verdicts MATCH, full disposition, shared requests).
