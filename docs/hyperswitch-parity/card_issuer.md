# Parity: card_issuer

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/card_issuer.rs` (4 handlers); path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:2092-2106` (`CardIssuers::server`, scope `/card_issuers`, resource `""` → POST/GET, resource `/{id}` → PUT/DELETE).
- Upstream core: `docs/hyperswitch/crates/router/src/core/card_issuer.rs` (add/update/delete/list). API shapes: `docs/hyperswitch/crates/api_models/src/card_issuer.rs:4-42`.
- C++: `include/app/handlers/card_issuer.cppm` (4 route wrappers + 4 direct core ports); DTOs in `include/app/dto/card_issuer.cppm`.
- Upstream SHA: `7736d55adaf235f7af74e133ae408573b1826823` (read-only).
- Checked: 2026-11-20. Re-verified after refactoring every route into a named local helper that is a direct port of the named upstream function.

## Summary

All four upstream `card_issuer` route functions and all four upstream core functions are ported into the handler as named local helpers with the same control flow, store calls, error mapping and wire shapes as upstream. The four public `handle_*` methods port the upstream route wrappers (auth + `server_wrap` concerns). External upstream infrastructure (Diesel/Postgres, tracing, superposition) is replaced with the explicitly permitted in-process substitutes supplied by `AppContext` (repository / metrics / superposition+config_store / ids). No C++-only route or behavior remains.

| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Four verbs on two resources exactly as `app.rs:2093-2105`: POST/GET on `""` → `/card_issuers` (`card_issuer.cppm:203-223`) and PUT/DELETE on `/:id` → `/card_issuers/{id}` (`:225-246`). `delt()` is the router's DELETE binding (`core/router/builder.cppm:540-548`), the local `web::delete()` equivalent — not a custom verb. |
| Input types | MATCH | Typed DTOs mirror upstream 1:1: `CardIssuerRequest{issuer_name}`, `CardIssuerUpdateRequest{issuer_name}`, path `CardIssuerId` (+ unused `CardIssuerDeleteRequest` wire shape). `api_models/card_issuer.rs:5,25,32`; `dto/card_issuer.cppm:28-34,47-53,66-72`, serde `:83-93,120-142`; extracted at `card_issuer.cppm:64-73,104-116`. |
| Output types | MATCH | `CardIssuerResponse{id,issuer_name}`, `CardIssuerListResponse{issuers}`, `CardIssuerDeleteResponse{id,deleted}` with matching serde fields. `api_models/card_issuer.rs:11-42`; `dto/card_issuer.cppm:36-45,55-64,74-80`, serde `:95-118,144-155`; emitted from the core helpers at `card_issuer.cppm:309,355,396-401,450-452`. |
| Auth/permissions | MATCH | Add/update/delete: `auth_rules().rule_for` + `authorize_route` → `PLATFORM_ADMIN`/`ADMIN` registry entries = upstream `AdminApiAuth` (`card_issuer.cppm:457-465`; `auth/registry.cppm:31,103,434`; `routes/card_issuer.rs:26,48,70`). List: API-key rule + JWT fallback with `MERCHANT_READ`, `allow_connected(false)`, `allow_platform(false)` = upstream `auth_type(HeaderAuth(ApiKeyAuth{false,false}), JWTAuth(MerchantAccountRead,false,false))` (`card_issuer.cppm:466-478`; `registry.cppm:198`; `routes/card_issuer.rs:85-96`). |
| Persistence | MATCH | All CRUD calls are upstream-exact: `create_card_issuer` (insert + duplicate→`GenericDuplicateError`), `find_card_issuer_by_string`, `update_card_issuer`, `remove_card_issuer` (drives `is_deleted`), `list_card_issuers` (`card_issuer.cppm:294-305,340-353,380-400,420-421`; `store/repository.cppm:1269-1273,783-786`). Backing store is the in-process repository (permitted local substitute for Diesel/Postgres) — see "Local in-process substitutes". |
| Domain logic | MATCH | 5-char generated id + internal-error on failure (`:278-284`, `new_id` `:510-526`), `created_at`/`last_modified_at` seeded on add and refreshed on update (`:289-292,348-350`), duplicate/not-found/invalid-data/internal error mapping (`:296-305,328-353,368-393,438-446`), dimension-config list limit + warn (`:418,423-428`, `configured_list_limit` `:547-563`), store `is_deleted` → wire `deleted` (`:388-400`), `ForeignTryInto` name validation on add/update/list (`:272-274,324-326,441-446`). Upstream: `core/card_issuer.rs:21-117`, `types/transformers.rs:3018-3047`. |

## Routes

| Verb + path | Upstream op | C++ handler / core port | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/card_issuers` | `add_card_issuer` | `handle_add_card_issuer` (`:50-87`) → `core_add_card_issuer` (`:266-310`) | `CardIssuerRequest{issuer_name}` | `CardIssuerResponse{id,issuer_name}` | `AdminApiAuth` → `PLATFORM_ADMIN`/`ADMIN` | `insert_card_issuer` + dup→duplicate error | MATCH | Up `routes/card_issuer.rs:12-30`, `app.rs:2097-2098`, `core/card_issuer.rs:16-39`; C++ `card_issuer.cppm:50-87,266-310`, route `:203-212` |
| GET `/card_issuers` | `list_card_issuers` | `handle_list_card_issuers` (`:166-192`) → `core_list_card_issuers` (`:415-453`) | none (no query params — matches upstream `()`) | `CardIssuerListResponse{issuers}` | `HeaderAuth(ApiKeyAuth)` or `JWTAuth(MerchantAccountRead)`, both allow flags false | `get_card_issuer_list_max_limit` + `list_card_issuers(limit)` + warn | MATCH | Up `routes/card_issuer.rs:76-100`, `app.rs:2099`, `core/card_issuer.rs:84-119`, `dimension_config.rs:1419-1429`; C++ `card_issuer.cppm:166-192,415-453,547-563`, route `:214-223` |
| PUT `/card_issuers/{id}` | `update_card_issuer` | `handle_update_card_issuer` (`:90-130`) → `core_update_card_issuer` (`:320-356`) | path `CardIssuerId` + `CardIssuerUpdateRequest` | `CardIssuerResponse{id,issuer_name}` | `AdminApiAuth` → `PLATFORM_ADMIN`/`ADMIN` | `update_card_issuer` → not-found map | MATCH | Up `routes/card_issuer.rs:32-52`, `app.rs:2102-2103`, `core/card_issuer.rs:41-61`; C++ `card_issuer.cppm:90-130,320-356`, route `:225-234` |
| DELETE `/card_issuers/{id}` | `delete_card_issuer` | `handle_delete_card_issuer` (`:133-160`) → `core_delete_card_issuer` (`:365-402`) | path `CardIssuerId` → `CardIssuerDeleteRequest{id}` | `CardIssuerDeleteResponse{id,deleted}` | `AdminApiAuth` → `PLATFORM_ADMIN`/`ADMIN` | `delete_card_issuer` → `is_deleted` → not-found map | MATCH | Up `routes/card_issuer.rs:54-74`, `app.rs:2104`, `core/card_issuer.rs:63-82`; C++ `card_issuer.cppm:133-160,365-402`, route `:236-246` |

## Ported upstream functions (named local helpers)

| Upstream function | Source | C++ local helper (line) |
|---|---|---|
| `routes::card_issuer::add_card_issuer` | `routes/card_issuer.rs:12-30` | `handle_add_card_issuer` `card_issuer.cppm:50-87` |
| `routes::card_issuer::update_card_issuer` | `routes/card_issuer.rs:32-52` | `handle_update_card_issuer` `card_issuer.cppm:90-130` |
| `routes::card_issuer::delete_card_issuer` | `routes/card_issuer.rs:54-74` | `handle_delete_card_issuer` `card_issuer.cppm:133-160` |
| `routes::card_issuer::list_card_issuers` | `routes/card_issuer.rs:76-100` | `handle_list_card_issuers` `card_issuer.cppm:166-192` |
| `core::card_issuer::add_card_issuer` | `core/card_issuer.rs:16-39` | `core_add_card_issuer` `card_issuer.cppm:266-310` |
| `core::card_issuer::update_card_issuer` | `core/card_issuer.rs:41-61` | `core_update_card_issuer` `card_issuer.cppm:320-356` |
| `core::card_issuer::delete_card_issuer` | `core/card_issuer.rs:63-82` | `core_delete_card_issuer` `card_issuer.cppm:365-402` |
| `core::card_issuer::list_card_issuers` | `core/card_issuer.rs:84-119` | `core_list_card_issuers` `card_issuer.cppm:415-453` |

Supporting ports: `ForeignTryInto<CardIssuer>` / `<CardIssuerListItem>` (`types/transformers.rs:3018-3047`) → `response_from` + `validate_issuer_name` (`card_issuer.cppm:580-592,490-503`); `CardIssuerId::generate` (`common_utils/id_type/card_issuer.rs:31-41`) → `new_id` (`:510-526`); `CardIssuerListMaxLimit` dimension (`dimension_config.rs:1419-1429`) → `configured_list_limit` (`:547-563`).

## Local in-process substitutes (external infrastructure only)

Per the chosen parity policy, external upstream infrastructure is replaced in-process; no upstream *behavior* is substituted. Each substitute is the sanctioned local equivalent:

- **Store**: upstream Diesel/Postgres `insert/update/delete/list_card_issuers` → `AppContext::repository()` over `app::store::card_issuers::CardIssuerRecord` (`card_issuer.cppm:294,340,380,420`; `store/repository.cppm:1269-1273,783-786`). Both duplicate detection and the store result are real, not faked.
- **Timestamps**: upstream `date_time::now()` columns `created_at`/`last_modified_at` → `now_stamp()` seconds string; `last_modified_at` lives in the record's `metadata` JSON because the local record has no dedicated column (`card_issuer.cppm:289-292,348-350,528-535`; `store/domain_records.cppm:257-304`). Not on the wire.
- **Instrumentation**: upstream `#[instrument(flow=…)]` + `logger::warn!` → `metrics().record_flow(...)` + named counters (`card_issuer.cppm:82-84,127-129,158-160,186-188,423-428`). Deterministic in-process trace/metric substitute.
- **Dimension config**: upstream superposition + DB-backed `CardIssuerListMaxLimit` (default 18000) → `superposition().get_config` then `config_store().get_value` with the same keys/default (`card_issuer.cppm:547-563`; `dimension_config.rs:1419-1429`).
- **Id generation**: upstream nanoid length 5 → injected `ids().uuid_v4()` truncated to 5 alphanumerics; empty id → `INTERNAL_ERROR` (`card_issuer.cppm:278-284,510-526`; `id_type/card_issuer.rs:31-41`, `consts.rs:283`).
- **Error transport**: upstream `ApiErrorResponse` variants map to the shared local `AppError` kinds/messages via `AppErrorBuilder`/`AppErrorStatus`, consistent with every other ported module (`card_issuer.cppm:482-488,594-605`; `core/error/kind.cppm:12-30,67-85`; upstream `api_error_response.rs:589-592,684-690`).

## Not ported

None. All four upstream route functions and all four upstream core functions are implemented in `card_issuer.cppm`. No C++-only routes remain. The former client-side `limit` query parameter and the delete-body fallback (both without upstream counterparts) were removed. The only non-literal aspects are the explicitly permitted in-process infrastructure substitutes listed above; none change a route's wire shape, auth, error or persistence semantics.

## Implementation disposition

- Persistence layer — MATCH — `core_add/update/delete/list` issue the exact upstream store operations through the in-process repository (`card_issuer.cppm:294-305,340-353,380-400,420-421`); backing engine is the repository substitute (see substitutes).
- Duplicate handling (`to_duplicate_response`) — MATCH — repository duplicate maps to `ALREADY_EXISTS` with upstream message "A card issuer with this name already exists" (`card_issuer.cppm:294-305`; upstream `core/card_issuer.rs:34-36`).
- Not-found handling (`to_not_found_response`) — MATCH — miss or empty path id → `NOT_FOUND` with upstream message "Card issuer with id {id} not found" on update/delete (`card_issuer.cppm:328-343,368-384`; upstream `core/card_issuer.rs:56-58,72-74`).
- Timestamps — MATCH — `created_at`/`last_modified_at` seeded on add, `last_modified_at` refreshed on update; persisted via the in-process record substitute (`card_issuer.cppm:289-292,348-350`; upstream `core/card_issuer.rs:21-28,47-50`).
- Id generation (`CardIssuerId::generate`) — MATCH — injected generator → 5-char id, failure → `INTERNAL_ERROR` (`card_issuer.cppm:278-284,510-526`; upstream `core/card_issuer.rs:23-24`, `id_type/card_issuer.rs:31-41`).
- Configurable list limit (`dimension_state`) — MATCH — same superposition/DB keys and 18000 default (`card_issuer.cppm:418,547-563`; upstream `core/card_issuer.rs:88-95`, `dimension_config.rs:1419-1429`); warn-on-limit via counter substitute (`:423-428`, upstream `:103-106`).
- `deleted` flag from store result — MATCH — `is_deleted = repository.remove_card_issuer(...)` drives `response.set_deleted`; failure → `INTERNAL_ERROR` (`card_issuer.cppm:386-400`; upstream `core/card_issuer.rs:68-81`).
- Error/status mapping (`ForeignTryInto`, `InvalidDataValue`) — MATCH — request-path name validation → `VALIDATION_FAILED` (upstream 422 serde error); list `ForeignTryInto` failure → `INVALID_REQUEST` "Invalid value provided: issuer_name" (upstream `InvalidDataValue` 400) (`card_issuer.cppm:272-274,324-326,438-446`; upstream `transformers.rs:3018-3047`, `api_error_response.rs:589-592`).
- Auth parity on list — MATCH — `auth_rules` API-key rule + JWT fallback (`MERCHANT_READ`, `allow_connected(false)`, `allow_platform(false)`) mirrors upstream `auth_type` (`card_issuer.cppm:166-179,457-480`; upstream `routes/card_issuer.rs:85-96`).
- Route path shape — MATCH — POST/GET on `""` → `/card_issuers`, PUT/DELETE on `/:id` → `/card_issuers/{id}`; `delt()` is the DELETE binding (`card_issuer.cppm:194-246`; upstream `app.rs:2093-2105`; `core/router/builder.cppm:540-548`).
- Instrumentation (`#[instrument]`) — MATCH (substitute) — `record_flow` + named counters per handler (`card_issuer.cppm:82-84,127-129,158-160,186-188`); deterministic in-process trace/metric substitute, no external tracing sink.

## Shared contract requests

None blocking. The port depends only on existing shared contracts, recorded here for the owning agents so they are not inadvertently changed:

- **Auth registry** (`include/app/auth/registry.cppm`): card_issuer operations `add/update/delete` = `PLATFORM_ADMIN`/`ADMIN` (`:31,103,434`) and `list` = `API_KEY`/`MERCHANT_READ` (`:198`). The handler's list JWT fallback and admin auth rely on these entries; keep them stable. (Note: `registry.cppm` is shared and was not edited.)
- **Router** (`include/core/router/builder.cppm`): `.delt()` is the canonical DELETE binding (`:540-548`); no change needed.
- **Store** (`include/app/store/repository.cppm`, `store/domain_records.cppm`): rely on `create_card_issuer` (`:1269-1273`) and `find/update/remove/list_card_issuer*` (`:783-786`) plus `CardIssuerRecord` (`:257-304`). No change needed.
- **Metrics/ids/superposition** (`AppContext`): rely on the existing accessors; no change needed.

## Files changed

- `include/app/handlers/card_issuer.cppm` — rewritten: 4 route wrappers + 4 named upstream-core ports; removed C++-only `limit` query param and delete body-fallback; upstream-exact errors/messages.
- `docs/hyperswitch-parity/card_issuer.md` — this report (all rows MATCH; evidence and shared requests recorded).
