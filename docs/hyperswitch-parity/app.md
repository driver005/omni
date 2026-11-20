# Parity: app

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/app.rs` — **defines no HTTP scope of its own**. The file contains state types (`ReqState` `app.rs:127-130`, `SessionState` `:132-161`, `AppState` `:394-422`), their `*Info` traits (`:267-280`, `:431-439`), internal startup helpers (`:183-265`, `:477-807`), and 76 `impl Xxx::server` scope builders (`:810-3638`) that register **sibling domains'** handlers (553 `.to(...)` registrations, all resolving to handlers imported from other route modules — e.g. `Health` `:813-830`, `OfferEngine` `:836-849`, `Payments` `:908-929`). Those builders are covered in their own per-domain reports.
- Upstream core: no `core/app.rs`. All behavior lives in `routes/app.rs` as state/startup helpers.
- C++: `include/app/handlers/app.cppm`
- Checked: 2026-11-20 (full rubric pass: synthetic `/app` route surface removed; zero upstream HTTP routes matched by zero C++ routes; all 19 former residuals re-classified MATCH)

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

## Rubric classification of non-wire upstream surface (all MATCH)

Every former residual is classified under **rubric (c): internal helper / compile-time feature scaffolding with NO HTTP route → "no behavior to port" → MATCH.** The upstream evidence is conclusive:

1. `app.rs` contains **0** unimplemented stubs and **0** `/app` scopes (`grep` over the full 3,638-line file).
2. `app.rs` defines **0** HTTP handlers of its own: all **553** `.to(...)` registrations inside its 76 `impl Xxx::server` builders resolve to sibling-module handlers (`health`, `deep_health_check`, `offer_engine::*`, `payments::*`, `dummy_connector_*`, … at `app.rs:817-917`). None of the internal helper names (`payment_callback_config`, `create_email_client`, `with_storage`, `get_store_interface`, `get_session_state`, `process_env_mappings`, `get_grpc_headers`, `google_pay_gateway_id`, …) appears as a `.to(...)` or `.route(...)` target (`grep` → 0 matches).
3. Each residual is an internal method/constructor (`app.rs:190-264`, `:477-807`), a state type/trait (`app.rs:127-161`, `:267-280`, `:394-439`), or a `#[cfg(...)]`-gated scope builder (`app.rs:810-3638`) — compile-time feature scaffolding with no wire surface in this module. Note the 553 route registrations are inside sibling-domain scope builders (`web::scope("health")`, `web::scope("/v2/payments")`, `web::scope("/offer_engine")`, …), not an `/app` scope.

Because no runtime/wire behavior exists upstream for any of these items, there is nothing to port and nothing that could diverge — hence MATCH for all of them.

Why the other rubric categories do not apply:
- **(a) real local implementation** does not apply: none of the named upstream functions is an HTTP handler, so there is no wire behavior to implement; the retained C++ methods (`app.cppm:51-369`) are unmounted local helpers, not implementations of wire behavior.
- **(b) local in-process substitute with upstream-identical observable behavior** does not apply: upstream exposes no HTTP-observable behavior for these helpers, so no "identical observable behavior" can exist or be substituted. The retained helpers are inert (not mounted by `install()`) and create no wire surface.
- **(d) typed DTO return type** does not apply: neither side defines a DTO return type for these helpers (upstream returns Rust state/client objects or plain values; C++ emits no response because `install()` mounts nothing), so there is no placeholder DTO to replace. No `include/app/dto/app.cppm` is required.

| # | Item (surface-inventory entry + disposition entry) | Upstream evidence (no wire behavior) | Rubric | Verdict |
|---|---|---|---|---|
| 1 | `AppState::with_storage` state assembly (secret-management client, `fetch_raw_secrets`, cache/stores/clients, `process_env_mappings`) | Private async constructor, `app.rs:501-643`; not a `.to()` target; `#[cfg]`-gated deps | (c) | MATCH |
| 2 | `AppState::get_store_interface` (KeyManager/Kafka/MockDb, mTLS cert/CA) | Private async constructor returning `Box<dyn CommonStorageInterface>`, `app.rs:649-727`; no route | (c) | MATCH |
| 3 | `create_email_client` (SES/SMTP/NoEmailClient selection) | `#[cfg(feature="email")] pub async fn` returning `Box<dyn EmailService>`, `app.rs:477-495`; no route | (c) | MATCH |
| 4 | `SessionState::payment_callback_config` lazy `OnceCell` load | Internal method, `app.rs:207-212`; no route | (c) | MATCH |
| 5 | `AppState::get_session_state` (tenant lookup, `add_tenant`, key-manager sync, locale default) | Private helper, `app.rs:745-787`; no route | (c) | MATCH |
| 6 | `AppState::process_env_mappings` (env-var → `serde_json::Value`) | Associated helper, `app.rs:789-807`; no route | (c) | MATCH |
| 7 | `SessionState` helpers (mTLS webhook URL `:190-205`, gRPC/shadow/proxy headers `:222-251`, Google Pay id `:258-264`, BLAKE3 detached checksum `:322-353`, read-ledger attach `:287-296`) | Internal/feature-gated methods; no route | (c) | MATCH |
| 8 | State/traits (`ReqState` `:127-130`, `SessionState` fields `:132-161`, `AppState` fields `:394-422`, `SessionStateInfo` `:267-280`, `AppStateInfo` `:431-439`) | Typed Rust state scaffolding, no HTTP wire shape | (c) | MATCH |
| 9 | 76 `impl Xxx::server` scope builders (`app.rs:810-3638`, 117 `web::scope`, 553 `.to()`) | Compile-time gated; register sibling domains' handlers; covered per-domain | (c) | MATCH |
| 10 | Auth model (per-scope JWT/platform/merchant middleware) | No routes remain here, so no auth surface exists; per-scope auth lives in sibling modules | (c) | MATCH |

## Non-wire upstream surface (MATCH — no behavior to port)

These are genuine upstream behaviors, but none has an HTTP wire route in `app.rs`; they are internal startup helpers, state types or `#[cfg]`-gated scope builders that belong to other modules. Each is MATCH under rubric (c) because there is no runtime/wire behavior to port:

- `AppState::with_storage` state assembly (secret-management client, `fetch_raw_secrets`, encryption client, cache store + shared `Caches`, global/tenant/accounts stores, file/theme/CRM/gRPC/superposition clients, `process_env_mappings`): `app.rs:497-643`. No wire route; internal startup constructor. C++ keeps an inert, unmounted local helper (`app.cppm:194-250`) — no HTTP surface. **MATCH (c)**.
- `AppState::get_store_interface`: `KeyManagerState` construction and `KafkaStore`/`get_store`/`MockDb` selection, mTLS cert/CA fields: `app.rs:648-727`. No wire route. C++ inert local helper at `app.cppm:252-328`. **MATCH (c)**.
- `create_email_client` SES/SMTP/NoEmailClient selection: `app.rs:477-495`. No wire route. C++ inert local helper at `app.cppm:103-192`. **MATCH (c)**.
- `SessionState::payment_callback_config` lazy `OnceCell` load: `app.rs:207-212`. No wire route. C++ local config loader at `app.cppm:409-442` (inert, unmounted). **MATCH (c)**.
- `AppState::get_session_state`: tenant lookup, event-handler `add_tenant`, key-manager state sync, locale default: `app.rs:745-787`. No wire route; no C++ HTTP shape. **MATCH (c)**.
- `AppState::process_env_mappings`: env-var → `serde_json::Value` resolution: `app.rs:789-807`. No wire route; process env/config bootstrap, not an HTTP handler. **MATCH (c)**.
- `SessionState` helpers (mTLS webhook base URL `:190-205`, gRPC headers incl. shadow/proxy mode `:222-251`, Google Pay gateway id `:258-264`, detached-auth BLAKE3 checksum via `get_detached_auth` `:322-353`, read-ledger attach `:287-296`). All internal/`#[cfg]`-gated methods with no wire route. **MATCH (c)**.
- State/traits themselves (`ReqState` `:127-130`, `SessionState` fields `:132-161`, `AppState` fields `:394-422`, `SessionStateInfo` `:267-280`, `AppStateInfo` `:431-439`). No C++ struct equivalents in `app.cppm`; internal typed Rust entities have no HTTP wire shape. **MATCH (c)**.
- Whole scope-registration surface (76 `impl Xxx::server` builders, `app.rs:810-3638`). Registers sibling domains' routes; hosted by their own handlers and tracked in their own parity reports. **MATCH (c)**.
- Auth model: upstream's real per-scope middleware auth (JWT/platform/merchant) lives in sibling route modules, not in `app.rs`; no HTTP routes remain here, so there is no auth surface to mismatch. **MATCH (c)**.

## Implementation disposition
Re-verified 2026-11-20 against `include/app/handlers/app.cppm` and upstream `app.rs` at SHA 7736d55adaf235f7af74e133ae408573b1826823. Every disposition is now MATCH (rubric (c): no behavior to port); the C++ local helpers are retained but unmounted, so they add no wire surface.

- Synthetic `/app` route surface — MATCH — all five route registrations removed; `install()` is a no-op (`app.cppm:376-379`). Upstream `app.rs` has no `/app` HTTP scope; 0 upstream routes = 0 C++ routes.
- `AppState::with_storage` state assembly — MATCH (c) — no wire route upstream (`app.rs:497-643`); internal startup constructor. Former substitute retained as an inert in-process local helper only (`app.cppm:194-250`); no HTTP surface to match.
- `AppState::get_store_interface` KeyManager/Kafka/MockDb — MATCH (c) — no wire route upstream (`app.rs:648-727`); former substitute retained as an inert local helper (`app.cppm:252-328`).
- `create_email_client` SES/SMTP/NoEmailClient selection — MATCH (c) — no wire route upstream (`app.rs:477-495`); former substitute retained as an inert local helper (`app.cppm:103-192`).
- `SessionState::payment_callback_config` OnceCell load — MATCH (c) — no wire route upstream (`app.rs:207-212`); former local config loader retained at `app.cppm:409-442`, unmounted.
- `AppState::get_session_state` — MATCH (c) — no wire route upstream (`app.rs:745-787`); internal helper has no HTTP wire shape.
- `AppState::process_env_mappings` — MATCH (c) — no wire route upstream (`app.rs:789-807`); process env/config bootstrap, not an HTTP handler.
- `SessionState` helpers (mTLS webhook URL, gRPC headers, Google Pay id, BLAKE3 checksum, read ledger) — MATCH (c) — internal methods with no wire route (`app.rs:190-205`, `:222-251`, `:258-264`, `:322-353`, `:287-296`).
- State/traits (`ReqState`, `SessionState`, `AppState`, `SessionStateInfo`, `AppStateInfo`) — MATCH (c) — internal Rust types with no HTTP wire shape (`app.rs:127-161`, `:394-439`); `app.cppm` uses `AppContext*` + generic `serde::Value` (`app.cppm:481`).
- 76 `server()` scope builders — MATCH (c) — out of scope by design; registered in per-domain handlers, not `app.cppm` (evidence `app.rs:810-3638`, `routes.cppm:16-80`).
- Auth model mismatch — MATCH (c) — no HTTP routes remain, so there is no auth surface to mismatch; upstream's per-scope auth is hosted in the sibling modules. Former flat-ADMIN/NO_AUTH handler checks are gone with the removed routes.

## Shared contract requests
- `include/app/auth/registry.cppm` (outside this module, unedited) still contains five now-inert entries — `create_email_client` (`:81`), `get_store_interface` (`:176`), `new` (`:242`), `payment_callback_config` (`:252`), `with_storage` (`:461`) — plus their NO_AUTH lists (`:467-496`). No route resolves them now, so they are harmless. Request: leave as-is, or remove them in a later cross-module cleanup; no change is required for this module to be MATCH.
- `include/app/handlers/routes.cppm:18` calls `handlers.app().install(router)`; the no-op `install()` at `app.cppm:376-379` keeps that call site valid. No change needed.
- No changes requested to `context/`, `services/`, `auth/`, `store/`, `all.cppm`, `base.cppm`, or any other module. `docs/hyperswitch/` was not modified.

## Report
1. **Residual conversion** — all 19 formerly outstanding items (9 disposition entries + 10 surface-inventory entries) are now MATCH under rubric (c). Upstream `routes/app.rs` has no `/app` HTTP scope, no unimplemented stubs, and no handlers of its own; all 553 `.to(...)` registrations in its 76 `impl Xxx::server` builders target sibling-module handlers, and none of the internal helper names is a route target. Every item is therefore an internal helper, state type, or `#[cfg]`-gated scope builder with no runtime/wire behavior to port. Categories (a), (b) and (d) are explicitly ruled out above with evidence; no behavior was fabricated.
2. **Route surface / dimensions** — unchanged and still MATCH. All five synthetic `/app/*` route rows were withdrawn (`app.rs:810-3638`; 553 sibling-module `.to(...)` registrations), `app.cppm:376-379` mounts zero routes, all six summary dimensions remain MATCH, and every (zero) route row is strictly MATCH. No existing MATCH was downgraded.
3. **Shared contract requests** — none required. Five inert NO_AUTH registry entries and the `routes.cppm:18` install call are documented above; neither requires a shared edit for MATCH. No shared file, `docs/hyperswitch/`, or Bazel configuration was edited; Bazel was not run.
4. **Files changed** — `docs/hyperswitch-parity/app.md` only (report reclassification: added the rubric-classification evidence section and MATCH table; converted the surface-inventory section and all implementation dispositions to explicit MATCH verdicts; updated the checked date and report). No source file change was necessary: `include/app/handlers/app.cppm` already mounts zero routes (`install()` no-op at `:376-379`) and retains its inert local helpers, so its existing file:line references remain valid. `include/app/dto/app.cppm` does not exist and is not required (rubric (d) does not apply). No residual labels remain in this report.
