# Parity: config

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/config.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/config/routes.cppm` (found)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (manifest group 25, `system/config` half)

Ghost controller exports one operation key: `read` (`config.js:7-15`; `docName: 'config'`
at `config.js:5`). The only route referencing that key is `GET /config` in the
admin routes (`routes.js:23`). `GET /config/featurebase` (`routes.js:24`) references
`api.config.featurebase`, which is **not** exported by this controller (it is a dead
binding; the real token op belongs to the separate `featurebase` controller). It is
kept installed in C++ because the harness pins a 200 config body there; see
*Extra in C++* and `docs/parity/behavior/config.md`.

Authoritative evidence and the full Ghost behavior matrix live in
`docs/parity/behavior/config.md`; this file records the current C++ parity state.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

The strict-parity pass closed the body/header/status divergences on both routes, but
neither route is fully achieved: GET `/config` still lacks the real admin auth chain
and real config source (PARTIAL), and GET `/config/featurebase` is a functional 200
body where Ghost's declared route is dead (MISMATCH; harness tests pin the 200).
Per "flip verdicts only when fully achieved", the earlier MATCH rating for GET `/config`
is downgraded to PARTIAL to stay consistent with the behavior report.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/config` | `read` (`query` returns `publicConfig.config`, `config.js:12-14`) | GET `/config` (`system/config/routes.cppm:19-28`) | `config_browse` → `BrowseHandler::handle` (`all.cppm:406,687`; `browse/handler.cppm`) | Both none. Ghost `read` takes no `options`/`data`/params (`config.js:7-15`). C++ `using Input=IRequest&`, no parsing (`browse/.../execute.cppm`). | Both `{"config": <20 picked keys>}`, no `meta`, status 200. C++ builds the object in `serde::Value::Object` with 17 base keys + conditional `exploreTestimonialsUrl`/`stats`/`featurebase`, secret scrubbing and Tinybird/featurebase/auth picks (`browse/.../execute.cppm`); admin-app `Cache-Control` only, no `X-Cache-Invalidate`. | Ghost: no model/service call — in-memory config/settingsCache/labs (`public-config/config.js:51-92`). C++: `ctx.store().resources().list("config")` + `ctx.store().json_for("settings",{})`, decoded with `core_serde::JsonCodec`; no SQL, no model, no pagination. | Ghost `cacheInvalidate:false`; C++ writes only app-level `Cache-Control: private`, no invalidation header. No cache service either side. | Ghost `mw.authAdminApi`, `permissions:false`. C++ `require_auth` only (no permission stage, agreeing with `permissions:false`); 401 `UnauthorizedError` envelope via middleware. No 415/406 gate. Real key validation remains a residual. | PARTIAL | Ghost: `config.js:5-15`, `routes.js:23`, `public-config/config.js:51-92`, `output/config.js:8-33`; C++: `system/config/routes.cppm:19-28`, `browse/handler.cppm`, `browse/.../execute.cppm` |
| GET `/config/featurebase` | `config.featurebase` — **op does not exist** (dead route) | GET `/config/featurebase` (`system/config/routes.cppm:30-40`) | `config_config_featurebase_get` → `ConfigFeaturebaseGetHandler::handle` (`all.cppm:408,688`) | None on either side (no op to carry input; C++ ignores `IRequest&`). | Ghost: 500 via `next(err)` (undefined op), no body/serializer. C++: 200 `{config: <picked keys>}` via the shared `build_config_body()`; no fabricated pagination/empty fixture. | Ghost: none (error path). C++: same context-local config/settings reads as GET `/config`. | none | Ghost `mw.authAdminApi` (no op-level auth). C++ `require_auth` inherited from parent router; former local `{"error":"Unauthorized"}` check removed. | MISMATCH | Ghost: `routes.js:24`, `pipeline.ts:334,339-461`, `http.ts:116,169-176`; C++: `system/config/routes.cppm:30-40`, `config_featurebase_get/handler.cppm`, `config_featurebase_get/.../execute.cppm` |

## Route details

### GET `/config` — PARTIAL

- Route identity: MATCH. Ghost `GET /config` (`admin/routes.js:23`) ↔ `Router{router,"/config"}.get(...)` with `require_auth` (`system/config/routes.cppm:19-28`); central install at `include/blog/handlers/routes.cppm`. Path and verb normalize identically.
- Operation: MATCH. Ghost `read` (`config.js:7`); C++ binds `handler.config_browse()` (`system/config/routes.cppm:27`) with a `config_read()` alias at `all.cppm:407`. Both perform one full-config read.
- Input parity: no differences. Neither side parses params; `query()` takes no arguments (`config.js:12-14`), C++ ignores `IRequest&`.
- Output parity: MATCH. Both emit `{"config": <20 keys>}` with no `meta` and status 200. C++ `build_config()` emits the 17 always-present base keys and conditionally emits `exploreTestimonialsUrl`, `stats`, `featurebase` only when backing config exists; `sanitizeHostSettings` scrubs `export.webhookSecret`/`emailVerification.webhookSecret`, Tinybird `stats` copies the allowlisted props + `id`/`local`, `auth` is picked to `engine`, and `useGravatar` defaults true. Values come from the context-local `config`/`settings` namespaces with harness fallbacks (residual: Ghost computes them from real config/settingsCache/labs/databaseInfo).
- DB call parity: MATCH (no real DB work). Ghost computes in memory (`public-config/config.js:51-92`); C++ reads the two context-local store namespaces with `core_serde::JsonCodec`; no SQL, no model, no filtering/pagination.
- Cache parity: MATCH. Ghost `cacheInvalidate:false` emits no invalidation header; C++ writes only the app-level `Cache-Control: private` value and no `X-Cache-Invalidate`. No cache service either side.
- Auth / permissions: PARTIAL. Ghost `mw.authAdminApi`, `permissions:false`; C++ `require_auth` only (correctly no permission stage), 401 `UnauthorizedError` via middleware. No `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`/`tokenPermissionCheck`/cors equivalents; no signature verification. Accepted residual — no registered primitive closes it, recorded in `docs/parity/behavior/config.md`.

#### Fix — APPLIED (strict-parity, 2026-11-20)

Files edited (all inside `include/blog/handlers/system/config/`):
- `routes.cppm` — removed `content_type_middleware` (no 415/406 gate on a bodyless GET); kept `require_auth`; no `require_ghost_permission` (`permissions:false`).
- `browse/handler.cppm` — removed stand-in `X-Cache-Invalidate: false`; writes only admin-app `Cache-Control`.
- `browse/workflow/step/execute.cppm` — rewritten to build the strict Ghost picked object via `serde::Value::Object` with conditional keys, secret scrubbing and Tinybird/featurebase/auth picks; `useGravatar` defaults true; exposes `build_config_body()`; `Output=std::string`, `static s_ctx` retained.
- `config_featurebase_get/handler.cppm` — removed local `{"error":"Unauthorized"}` token check; writes only admin-app `Cache-Control`.
- `config_featurebase_get/workflow/step/execute.cppm` — replaced empty-envelope fixture + fabricated pagination with `build_config_body(*s_ctx)`.

Shared abstractions used (all already registered): `core_serde::JsonCodec`/`serde::Value`, `blog::api::{RecordDto,parse_array}` (`:domain_all`), `blog_handlers_middleware::require_auth`, `interfaces::io::types::Token::CACHE_CONTROL`. No new module/partition/BUILD entry needed.

New shared needs (recorded, not implemented — no shared files touched): (1) an admin-API auth partition providing `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`/`tokenPermissionCheck`; (2) a real config/settings/labs/databaseInfo primitive or `core_adapters` dependency so `std::getenv` + store fallbacks can be replaced.

### GET `/config/featurebase` — MISMATCH

- Route identity: MATCH at the wire level — Ghost declares `GET /config/featurebase` (`admin/routes.js:24`) and C++ installs it (`system/config/routes.cppm:30-40`).
- Operation: MISMATCH. Ghost's binding is dead (`api.config.featurebase` undefined -> `TypeError` -> 500). C++ keeps a functional handler because shared tests (`include/blog/testing/api_test.cc:296-302`, `snapshot_replay_test.cc:213-214`) assert a 200 config body at this path; those files are outside this group's edit scope.
- Output parity: C++ now returns the same strict `{config: <picked keys>}` body as GET `/config` via `build_config_body()`; the fabricated pagination and empty-envelope fixture were removed. No Ghost body exists to match (error path).
- Cache/auth: admin-app `Cache-Control` only, no `X-Cache-Invalidate`; `require_auth` inherited from the parent router (local `{"error":"Unauthorized"}` check removed).
- Verdict stays MISMATCH: C++ functional where Ghost's declared route 500s; this cannot be closed without editing shared tests.

### Central registrations (needed / already present)

- `include/blog/handlers/all.cppm:406-408,687-688` — `config_browse()`, `config_read()` alias, `config_config_featurebase_get()` and their members are already present; no action needed. (Earlier "add `config_read()` alias" note is obsolete — it now exists.)
- `include/blog/handlers/routes.cppm` — central install of `ConfigRoutes` already present; no action needed.
- `blog_handlers_middleware::require_auth` is already exported by `include/blog/handlers/middleware/auth.cppm` and imported in `system/config/routes.cppm`; no central registration needed.

## Missing in C++

None. The only Ghost route referencing an exported key of this controller (`GET /config`, `admin/routes.js:23`) has a C++ route and handler counterpart (`system/config/routes.cppm:19-28`, `browse/handler.cppm`).

## Extra in C++

None counted. The folder installs two routes: `GET /config` (matched above) and `GET /config/featurebase` (`routes.cppm:30-40` → `config_config_featurebase_get`, `all.cppm:408,688`). The latter has a Ghost route at the same path (`admin/routes.js:24`) but that route references `api.config.featurebase`, a key not exported by `config.js`; the real featurebase token is owned by the separate `featurebase` controller (`docs/parity/featurebase.md` / `docs/parity/behavior/featurebase.md`). It is therefore not EXTRA for `config`. Its parity is tracked right here and in `docs/parity/behavior/config.md` (the earlier note that it was "closed in `docs/parity/featurebase.md`" was incorrect — that report explicitly excludes it).
