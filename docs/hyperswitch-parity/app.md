# Parity: app

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/app.rs` — **defines no HTTP scope of its own**. The file contains state types (`ReqState` `app.rs:127-130`, `SessionState` `:132-161`, `AppState` `:394-422`), their `*Info` traits (`:267-280`, `:431-439`), internal startup helpers (`:183-265`, `:477-807`), and ~50 `impl Xxx::server` scope builders (`:810-3638`) that register **sibling domains'** handlers (553 `.to(...)` registrations, all resolving to handlers imported from other route modules — e.g. `Health` `:813-830`, `OfferEngine` `:836-849`, `Payments` `:908-929`). Those builders are covered in their own per-domain reports.
- Upstream core: no `core/app.rs`. All behavior lives in `routes/app.rs` as state/startup helpers.
- C++: `include/app/handlers/app.cppm`
- Checked: 2026-11-20 (strict-parity pass: synthetic `/app` route surface removed; zero upstream HTTP routes matched by zero C++ routes)

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Upstream `app.rs` defines no `/app` HTTP scope; C++ `install()` is now a no-op with zero routes (`app.cppm:376-379`, called at `routes.cppm:18`). The five former synthetic `/app/*` routes had no upstream counterpart and were removed. 0 upstream / 0 C++ = exact match. |
| Input types | MATCH | No HTTP payload/params exist on either side: upstream helpers take typed Rust arguments (`Settings<SecuredSecret>`, `StorageImpl`, `oneshot::Sender`, `&dyn TenantConfig`) with no wire shape (`app.rs:501-507`, `:649-659`, `:729-734`); the former synthetic body/query/header parsing is gone with the removed routes. |
| Output types | MATCH | No HTTP responses exist on either side: upstream helpers return `Self` state objects / `Box<dyn EmailService>` / `Box<dyn CommonStorageInterface>` (`app.rs:477-495`, `:497-643`, `:648-727`), not wire bodies; the former synthetic `partition`/`op` JSON envelopes are gone. |
| Auth/permissions | MATCH | No HTTP auth exists on either side: upstream internal helpers have no auth middleware (the real per-scope auth lives in sibling route modules); C++ mounts no authenticated routes. Leftover NO_AUTH registry rows are inert and documented under Shared contract requests. |
| Persistence | MATCH | No route-driven persistence exists on either side: upstream assembles stores/clients in-process at startup (`app.rs:497-727`), consumed by other domains; C++ retains no HTTP-triggered store/cache/event access. |
| Domain logic | MATCH | Upstream `app.rs` is state/startup/scope-registration only — no HTTP handler is defined in the file. C++ mirrors this with an empty route surface; the retained in-process local helpers (`app.cppm:51-368`) mirror the upstream internal helper behavior, and genuine HTTP routes are hosted by their own modules. |

## Routes
**There are no route rows.** Upstream `routes/app.rs` contains no `web::scope("/app")` and no `.to(...)` registration targeting any function defined in `app.rs` — all 553 registrations target handlers imported from sibling modules. Strict parity therefore requires this module to mount zero HTTP routes, which it now does (`app.cppm:376-379`). The five former synthetic diagnostic routes are withdrawn because they had no upstream counterpart:

| Withdrawn synthetic route (no upstream counterpart) | Upstream reality | Disposition |
|---|---|---|
| GET `/app/payment_callback_config` | Internal `SessionState::payment_callback_config` `OnceCell` load, no wire route (`app.rs:207-212`) | Removed from `install()` |
| POST `/app/create_email_client` | Internal `create_email_client` free fn, no wire route (`app.rs:477-495`) | Removed from `install()` |
| GET `/app/with_storage` | Internal `AppState::with_storage` async constructor, no wire route (`app.rs:497-643`) | Removed from `install()` |
| GET `/app/get_store_interface` | Internal `AppState::get_store_interface` async constructor, no wire route (`app.rs:648-727`) | Removed from `install()` |
| GET `/app/new` | Internal `AppState::new` async constructor, no wire route (`app.rs:729-743`) | Removed from `install()` |

## Not ported (upstream surface outside this module's HTTP scope)
These are genuine upstream behaviors, but none has an HTTP wire route in `app.rs`; they are internal startup helpers or scope builders that belong to other modules. They are intentionally not hosted here.

- `AppState::with_storage` state assembly (secret-management client, `fetch_raw_secrets`, encryption client, cache store + shared `Caches`, global/tenant/accounts stores, file/theme/CRM/gRPC/superposition clients, `process_env_mappings`): `app.rs:497-643`. No wire route; internal startup constructor.
- `AppState::get_store_interface`: `KeyManagerState` construction and `KafkaStore`/`get_store`/`MockDb` selection, mTLS cert/CA fields: `app.rs:648-727`. No wire route.
- `create_email_client` SES/SMTP/NoEmailClient selection: `app.rs:477-495`. No wire route.
- `SessionState::payment_callback_config` lazy `OnceCell` load: `app.rs:207-212`. No wire route.
- `AppState::get_session_state`: tenant lookup, event-handler `add_tenant`, key-manager state sync, locale default: `app.rs:745-787`. No wire route.
- `AppState::process_env_mappings`: env-var → `serde_json::Value` resolution: `app.rs:789-807`. No wire route.
- `SessionState` helpers (mTLS webhook base URL `:190-205`, gRPC headers incl. shadow/proxy mode `:222-251`, Google Pay gateway id `:258-264`, partial-auth BLAKE3 detached checksum `:322-353`, read-ledger attach `:287-296`). All internal methods with no wire route.
- State/traits themselves (`ReqState` `:127-130`, `SessionState` fields `:132-161`, `AppState` fields `:394-422`, `SessionStateInfo` `:267-280`, `AppStateInfo` `:431-439`). No C++ struct equivalents in `app.cppm`; internal Rust types have no HTTP wire shape.
- Whole scope-registration surface (~50 `impl Xxx::server` builders, `app.rs:810-3638`). Registers sibling domains' routes; hosted by their own handlers and tracked in their own parity reports.
- Auth model: upstream's real per-scope middleware auth (JWT/platform/merchant) lives in the sibling route modules, not in `app.rs`; the former synthetic NO_AUTH checks left no HTTP surface to secure.

## Implementation disposition
Re-verified 2026-11-20 against `include/app/handlers/app.cppm` and upstream `app.rs` at SHA 7736d55adaf235f7af74e133ae408573b1826823.

- Synthetic `/app` route surface — IMPLEMENTED — all five route registrations removed; `install()` is a no-op (`app.cppm:376-379`). Upstream `app.rs` has no `/app` HTTP scope; 0 upstream routes = 0 C++ routes.
- `AppState::with_storage` state assembly — DEFERRED — no wire route upstream (`app.rs:497-643`); internal startup constructor. Former substitute handler retained as an in-process local helper only (`app.cppm:194-250`); no HTTP surface to match.
- `AppState::get_store_interface` KeyManager/Kafka/MockDb — DEFERRED — no wire route upstream (`app.rs:648-727`); former substitute retained as a local helper (`app.cppm:252-328`).
- `create_email_client` SES/SMTP/NoEmailClient selection — DEFERRED — no wire route upstream (`app.rs:477-495`); former substitute retained as a local helper (`app.cppm:103-192`).
- `SessionState::payment_callback_config` OnceCell load — DEFERRED — no wire route upstream (`app.rs:207-212`); former local config loader retained at `app.cppm:409-442`.
- `AppState::get_session_state` — DEFERRED — no wire route upstream (`app.rs:745-787`); internal helper has no HTTP wire shape.
- `AppState::process_env_mappings` — DEFERRED — no wire route upstream (`app.rs:789-807`); process env/config bootstrap, not an HTTP handler.
- `SessionState` helpers (mTLS webhook URL, gRPC headers, Google Pay id, BLAKE3 checksum, read ledger) — DEFERRED — internal methods with no wire route (`app.rs:190-205`, `:222-251`, `:258-264`, `:322-353`, `:287-296`).
- State/traits (`ReqState`, `SessionState`, `AppState`, `SessionStateInfo`, `AppStateInfo`) — DEFERRED — internal Rust types with no HTTP wire shape (`app.rs:127-161`, `:394-439`); `app.cppm` uses `AppContext*` + generic `serde::Value` (`app.cppm:481`).
- ~50 `server()` scope builders — DEFERRED — out of scope by design; registered in per-domain handlers, not `app.cppm` (evidence `app.rs:810-3638`, `routes.cppm:16-80`).
- Auth model mismatch — RESOLVED — no HTTP routes remain, so there is no auth surface to mismatch; upstream's per-scope auth is hosted in the sibling modules. Former flat-ADMIN/NO_AUTH handler checks are gone with the removed routes.

## Shared contract requests
- `include/app/auth/registry.cppm` (outside this module, unedited) still contains five now-inert entries — `create_email_client` (`:81`), `get_store_interface` (`:176`), `new` (`:242`), `payment_callback_config` (`:252`), `with_storage` (`:461`) — plus their NO_AUTH lists (`:467-496`). No route resolves them now, so they are harmless. Request: leave as-is, or remove them in a later cross-module cleanup; no change is required for this module to be MATCH.
- `include/app/handlers/routes.cppm:18` calls `handlers.app().install(router)`; the no-op `install()` at `app.cppm:376-379` keeps that call site valid. No change needed.
- No changes requested to `context/`, `services/`, `auth/`, `store/`, `all.cppm`, `base.cppm`, or any other module. `docs/hyperswitch/` was not modified.

## Report
1. **Rows moved / route surface** — the route-surface verdict moved from MISSING to MATCH. All five synthetic `/app/*` route rows were withdrawn because upstream `app.rs` defines no `/app` HTTP scope (evidence `app.rs:810-3638`, 553 `.to(...)` registrations all targeting sibling-module handlers). `app.cppm:376-379` now mounts zero routes. All six summary dimensions moved to MATCH (no HTTP surface exists on either side). No per-route rows remain, so every (zero) route row is strictly MATCH.
2. **Remaining non-MATCH** — none at the HTTP/route level. Upstream `app.rs` has no HTTP handlers of its own; its state types, internal startup helpers, and ~50 `server()` builders have no wire surface in this module and are tracked in their own per-domain reports. The retained methods at `app.cppm:51-368` are in-process local helpers, not HTTP routes.
3. **Shared contract requests** — none required. Five inert NO_AUTH registry entries and the `routes.cppm:18` install call are documented above; neither requires a shared edit for MATCH.
4. **Files changed** — `include/app/handlers/app.cppm` (class documentation updated; five synthetic route registrations removed, `install()` reduced to a no-op; local helpers retained) and `docs/hyperswitch-parity/app.md` (this report: summary tables, route surface, dispositions, shared requests updated). No other files edited; `docs/hyperswitch/` left read-only; Bazel was not run.
