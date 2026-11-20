# Behavior report: site

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/site.js` (18 lines; only op = `read`)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:19-20` (GET `/site`); second binding in the members app `docs/Ghost/ghost/core/server/web/members/app.js:188-193` (GET `/api/site`, mounted under `/members`). Content API has **no** site route (`content/routes.js` — no `site` match).
- C++ folders: `include/blog/handlers/system/site/` (`routes.cppm` + `browse/handler.cppm` + `browse/workflow/{flow/main.cppm,step/execute.cppm}` + `types/dto_site_response.cppm`); registered in `include/blog/handlers/routes.cppm:49,121`, `base.cppm:40`, `all.cppm:34,404,685`. No C++ route for `/members/api/site`.
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 57). Verdicts **unchanged** (GET `/site` PARTIAL, GET `/members/api/site` MISSING); see **Strict-parity rewrite (group 57)** below.

## Scope note

`site.js` exports exactly one operation, `read` (`site.js:5,7-15`), and the pipeline namespace `api.site` is
built from it (`endpoints/index.js:223-225`). That single op is bound to **two** Ghost routes:

1. `GET /site` on the admin API (`admin/routes.js:20`) — public admin endpoint, `mw.publicAdminApi`.
2. `GET /api/site` on the members app (`members/app.js:193`), mounted at `/members` by
   `web/parent/frontend.js:20` -> effective path `GET /members/api/site`, with no per-route middleware.

Both call the same op and therefore share call chain, cache, and output. They differ only in route-level
middleware and app-level headers. The C++ folder `system/site/` matches only the admin path (wire path
`/site`, scheme `admin`; `testing/route_catalog.cppm:46`, `testing/snapshot_replay_test.cc:211-212`). No
C++ handler is mounted at `/members/api/site`, so that route is `MISSING`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 1 |
| EXTRA | 0 |

No verdict flipped. The strict-parity rewrite closed every divergence that has an in-folder primitive (fixture-sourced values, HOST-derived `url`, hardcoded `site_uuid`/`version`/`cover_image`/`allow_external_signup`, empty `icon`/`logo`/sentry, missing `Cache-Control`), but the real in-memory config source and the admin auth chain / `tokenPermissionCheck` allowlist remain residual gaps with no registered primitive, and the members-app route needs shared wiring that this group cannot touch.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/site` (admin) | `site.read` (`docName:'site'`) | none (no `options`/`data`/`validation`; `query()` ignores frame) | public: no auth required; `mw.publicAdminApi` chain + `tokenPermissionCheck` allowlist `site:['GET']` (`site.js:11`, `admin/middleware.js:141-146,56,85-92`) | 200 default; `headers.cacheInvalidate:false` → **no** `X-Cache-Invalidate`; admin app `Cache-Control: private` | `query()` → `publicConfig.site` getter → `getSiteProperties()` in-memory from `settingsCache`/`urlUtils`/`ghostVersion.safe`/`config.get('client_sentry')`; no model/service/DB/transaction/events | none (no cache service/key/TTL/tag); `cacheInvalidate:false` | `output/site.js:5-26` → `{site: <14 picked fields>}`, no pagination meta | `system/site/routes.cppm`, `browse/handler.cppm`, `browse/workflow/{flow/main.cppm,step/execute.cppm}`, `types/dto_site_response.cppm` | route+verb, 200, `{site:…}` envelope, 14-field set, no auth gate, no `X-Cache-Invalidate` all match; after group 57 values are sourced context-locally from `config`/`settings` with env + DTO fallbacks, `url` from configured home URL (`URL`/`url`/scheme+authority), `version` from `GHOST_BUILD_VERSION`, `allow_external_signup` computed from the portal settings, `site_uuid` from `config`/`settings`, `icon`/`logo` populated, `sentry_dsn`/`sentry_env` conditionally emitted, and admin-app `Cache-Control: private` written. Residual: real config/settingsCache source and admin auth chain / token allowlist have no registered primitive | PARTIAL | Ghost: `site.js:5-15`, `admin/routes.js:17,20`, `admin/middleware.js:56,141-146`, `admin/app.js:24,33-48`, `public-config/site.js:6-39`, `output/site.js:5-26`, `http.ts:126-133`, `headers.ts:135-144`, `cache-control.ts:16-19`; C++: `routes.cppm:19-28`, `browse/handler.cppm:13-24`, `browse/.../execute.cppm:15-36`, `dto_site_response.cppm:42-55,60-82` |
| GET `/members/api/site` | `site.read` (`docName:'site'`) | none (same op) | public: no per-route middleware at all (`members/app.js:193`); app-level cors + magic-link session only (`members/app.js:27-33`) | 200 default; `cacheInvalidate:false` → no `X-Cache-Invalidate`; members app `Cache-Control: private` + CORS (`members/app.js:27,30`) | identical to admin row: `query()` → `publicConfig.site` → `getSiteProperties()` (`public-config/site.js:6-39`) | none | identical `{site: <14 fields>}` (`output/site.js:5-26`) | none | no C++ route at `/members/api/site`; only the admin-path `/site` route exists (`system/site/routes.cppm:19`), which is a different wire path and is not mounted on the members app | MISSING | Ghost: `members/app.js:193,27-33`, `web/parent/frontend.js:20`, `site.js:7-15`, `public-config/site.js:6-39`, `output/site.js:5-26`; C++: `system/site/routes.cppm:19-28` (admin path only), `testing/route_catalog.cppm:46` |

## Route details

### GET /site (admin) — PARTIAL

**Ghost** (facts 1–8, `file:line`):

1. **Identity**: `GET /site` under section `// ## Public` (`admin/routes.js:19-20`). Router-level mount
   `router.use(apiMw.cors)` (`admin/routes.js:17`). Admin API app mounted at `/ghost/api/admin/`
   (`web/api/app.js:22`; `admin/app.js:44` `apiApp.use(routes())`). App-level chain before routes:
   `cacheControl('private')` (`admin/app.js:24`; value `no-cache, private, no-store, must-revalidate,
   max-stale=0, post-check=0, pre-check=0` at `web/shared/middleware/cache-control.ts:16-19,32-35`),
   `bodyParser.json/urlencoded` (`admin/app.js:33-34`), `parseQueryBooleans` (`:37`),
   `checkVersionMatch` (`:41`), error handlers (`:47-48`). Route middleware `mw.publicAdminApi` =
   `[apiMw.cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]`
   (`admin/middleware.js:141-146`). `tokenPermissionCheck` allowlists `site: ['GET']`
   (`admin/middleware.js:55-56,85-92`) so integration tokens pass; user/staff tokens pass to the
   permission system (`:19-47`); other integration tokens get 403 `NoPermissionError` (`:95-100`).
   No `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen` on this route (contrast
   `authAdminApi` at `admin/middleware.js:110-118`).
2. **Operation**: exported key `read`, op `read`, `docName: 'site'` (`site.js:5,7`). Pipeline wrapper
   registered at `endpoints/index.js:223-225`.
3. **Input**: no `options`, no `data`, no `validation` block on the op (`site.js:7-15`); `query()`
   takes no frame use (`site.js:12-14`). No path params, no query aliases / `filter[x]=y`, no multipart
   fields, no body shape, no headers read in the controller. Body is parsed globally but ignored.
4. **Auth/permissions**: `permissions: false` (`site.js:11`) → permissions stage skipped
   (`packages/api-framework/src/pipeline.ts:229-233`). No `unsafeAttrs`, no labs flag, no permission
   services inside `query`. Auth is deliberately open; only `tokenPermissionCheck` applies (fact 1).
5. **Status + headers**: no `statusCode` → default 200 (`http.ts:126-133`). `headers:
   {cacheInvalidate: false}` (`site.js:8-10`) → guard `if (apiConfigHeaders.cacheInvalidate)` is false
   (`headers.ts:135-144`), so **no `X-Cache-Invalidate`**; no `disposition`, no `location`
   (`headers.ts:124,146-185`). No `response.format`/`stream` → `res.json(result)` (`http.ts:139-148,168`).
   App-level `Cache-Control: private` (fact 1).
6. **Full call chain**: `query()` returns `publicConfig.site` (`site.js:13`) → getter calls
   `require('./site')()` (`utils/public-config/index.js:5-7`) → `getSiteProperties()`
   (`utils/public-config/site.js:6-39`), a synchronous in-memory computation: `settingsCache.get` for
   `title` (`:8`), `description` (`:9`), `logo` (`:10`), `icon` (`:11`), `cover_image` (`:12`),
   `accent_color` (`:13`), `locale` (`:14`), `timezone` (`:15`), `site_uuid` (`:24`);
   `urlUtils.urlFor('home', true)` (`:16`); `ghostVersion.safe` (`:17`);
   `allow_external_signup = allow_self_signup && !(portal_signup_checkbox_required &&
   portal_signup_terms_html)` (`:18-23`); conditional `sentry_dsn`/`sentry_env` guarded by
   `config.get('client_sentry') && !config.get('client_sentry').disabled`, environment from `PRO_ENV`
   then `env` (`:27-36`). **No model/repository/DB call, no transaction, no events, no jobs, no mail,
   no webhook, no third-party HTTP.** Error branches: only middleware 401/403 (fact 1); no op-level
   thrown errors (pure computation).
7. **Cache**: no cache service, key, TTL, or tag anywhere in the op. `cacheInvalidate:false`
   (`site.js:9`) suppresses the invalidation header (`headers.ts:135-144`). Admin app forces
   `Cache-Control: private` (`admin/app.js:24`).
8. **Output**: computed object goes to output serializer `site.read` (`utils/serializers/output/site.js:5-26`),
   which sets `frame.response = {site: _.pick(data, [14 keys])}` (`:8-25`): `title`, `description`,
   `logo`, `icon`, `cover_image`, `accent_color`, `locale`, `timezone`, `url`, `version`,
   `allow_external_signup`, `sentry_dsn`, `sentry_env`, `site_uuid`. No pagination meta, no computed
   fields beyond the util. Empty/missing: base 12 keys always present (picked from always-populated
   object); `sentry_dsn`/`sentry_env` absent when `client_sentry` disabled/unset; if a setting is
   `undefined`, lodash `pick` still creates the key with `undefined` (omitted by JSON.stringify).

**C++** (`file:line`):

- Route: `Router{router, "/site"}` with `.use(optional_auth)` then `.get(...)` →
  `handler.site_browse().handle(req, res, send)`, summary `"Ghost site GET"`
  (`system/site/routes.cppm:12-29`, call at `:26`). Installed via `ApiRoutes::install`
  (`include/blog/handlers/routes.cppm:49,121`); handler accessor/member in `all.cppm:404,685` and
  module export in `base.cppm:40`. Catalog entry `RouteInfo{"GET","/site","site","get",true}`
  (`testing/route_catalog.cppm:46`); snapshot cases `GET /site` (`testing/snapshot_cases.cppm:38,1046-1074,1267-1269,1342`).
- Middleware: `optional_auth` is a **no-op** that unconditionally calls `next`
  (`middleware/auth.cppm:31-40`) — matches the public, unauthenticated Ghost route. No `require_auth`
  (`:12-29`), no `content_type_middleware` 415/406 gate (`middleware/serde.cppm:90-126`), no
  admin token-allowlist, no CORS/redirect/prettyUrls equivalent.
- Handler (`browse/handler.cppm:11-26`): ctor binds static `BlogContext*` to `ExecuteStep::s_ctx`,
  installs a `LocalWorkflowDriver` and registers `MainFlow` (`:13-17`). `handle` runs workflow
  `"site_browse"` (`:19`), reads `result_of("site_browse_execute")` (`:20`), casts `typed_output` to
  `std::string` with fallback `"{}"` (`:21`), converts to bytes (`:22`), writes
  `Status::OK` + body + `send()` (`:23`). Writes the admin-app `Cache-Control: private` value via `interfaces::io::types::Token::CACHE_CONTROL` (group 57) and **no** `X-Cache-Invalidate`.
- Flow: `MainFlow.build()` registers only `ExecuteStep` (`browse/workflow/flow/main.cppm:7-10`).
- Step `site_browse_execute` (`browse/workflow/step/execute.cppm`; `Input = IRequest&`,
  `Output = std::string`, `name = "site_browse_execute"`): after group 57 reproduces
  `getSiteProperties()` + `output/site.js#read` into a `serde::Value::Object` encoded by
  `core_serde::JsonCodec`. Reads `ctx.store().json_for("settings", {})` and
  `ctx.store().resources().list("config")` via `blog::api::parse_array`; `config.get` precedence
  is mirrored by a `lookup()` helper (config namespace wins over settings). Sources all 14
  picked keys: `title`/`description`/`logo`/`icon`/`cover_image`/`accent_color`/`locale`/
  `timezone` from context-local settings; `url` from the configured home URL
  (`URL` env -> config `url` -> request scheme+authority fallback, always trailing slash) rather
  than the `Host` header; `version` from `GHOST_BUILD_VERSION` then store then harness default;
  `site_uuid` from config/settings with the DTO default as fallback; `allow_external_signup`
  computed as `allow_self_signup && !(portal_signup_checkbox_required &&
  portal_signup_terms_html)`; `sentry_dsn`/`sentry_env` emitted only when `client_sentry` is
  configured and not disabled (`PRO_ENV` then `env`). DTO defaults survive only as fallbacks.
  `compensate` is a no-op; `ctx` is a static `BlogContext*`; helper `build_site_body(ctx, in)` is
  exported for reuse by a future members-app route.
- DTO (`types/dto_site_response.cppm:10-83`): still defines the 14 fields and their defaults, now
  used only as fallback values by the step (it is no longer the wire-path body source).
- Side effects: none beyond the response. No outbox/fixture writes, no events/jobs. `ctx.store()`
  reads are context-local (`context_context.cppm`); there is **no direct `core::cache::Cache`
  key/tag/TTL use** and no site-specific cache marker.

**Diff**:

- Residual gaps (no C++ primitive / not implemented): Ghost's real `settingsCache`/`config`
  in-memory computation (`public-config/site.js:8-16,24`) is still replaced by a context-local
  `config`/`settings` store read with `URL`/`GHOST_BUILD_VERSION` env + DTO fallbacks; there is no
  registered config/settings service adapter (a `core_adapters::Config` dependency on
  `//include/blog/handlers:blog_handlers` would close this). The admin auth chain
  (`authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`) and the
  `tokenPermissionCheck` integration-token allowlist `site:['GET']`
  (`admin/middleware.js:55-56,85-92,110-118`) have no registered primitive; `optional_auth` being a
  no-op keeps the route public, so the allowlist omission is benign for `/site`.
- Divergences remaining: none at the body/header/status level. The handler's `{}` fallback on a
  missing step result (`:21`) is now unreachable — the step always emits the picked object.
- Closed divergences (group 57): `url` now comes from the configured home URL instead of the
  request `Host` header; `site_uuid`/`version`/`cover_image` are sourced from config/settings/env
  rather than hardcoded DTO fixtures; `allow_external_signup` is computed from the portal-signup
  settings; `icon`/`logo` are populated from settings instead of empty `rfl::Generic`;
  `sentry_dsn`/`sentry_env` are conditionally emitted from `client_sentry`; and the admin-app
  `Cache-Control: private` header is written. C++ still writes no `X-Cache-Invalidate` — this
  **matches** Ghost (`cacheInvalidate:false`).
- Matched: exact path + verb, public/no-auth behavior, default 200, `{"site": …}` envelope, the 14
  output field names (12 when sentry is disabled, as Ghost omits undefined keys), no pagination
  meta, no cache service/TTL, no `X-Cache-Invalidate`, and `Cache-Control: private`.

### GET /members/api/site — MISSING

**Ghost** (facts 1–8, `file:line`):

1. **Identity**: `membersApp.get('/api/site', http(api.site.read))` (`members/app.js:193`), mounted at
   `/members` by `web/parent/frontend.js:20` (`frontendApp.lazyUse('/members', require('../members'))`)
   → effective path `GET /members/api/site`. Comment at `members/app.js:188-192` states the purpose:
   let external systems read public settings without CORS issues or an integration token. No
   per-route middleware. App-level chain: `cacheControl('private')` (`:27`), `corsMiddleware` (`:30`,
   module at `:17`), `createSessionFromMagicLink` (`:33`), error handlers on `/api` (`:196-197`).
2. **Operation**: same `site.read` (`docName:'site'`), `site.js:5,7`; pipeline `endpoints/index.js:223-225`.
3. **Input**: none (same op); no params, query aliases, body, or headers consumed.
4. **Auth/permissions**: none at route level; `permissions:false` (`site.js:11`). The magic-link
   session middleware (`:33`) may populate a member session but the op never reads it.
5. **Status + headers**: default 200 (`http.ts:126-133`); `cacheInvalidate:false` → no
   `X-Cache-Invalidate` (`site.js:9`, `headers.ts:135-144`); members app `Cache-Control: private`
   (`members/app.js:27`) plus CORS headers from `corsMiddleware` (`:30`).
6. **Full call chain**: identical to the admin route — `query()` → `publicConfig.site`
   (`site.js:13`) → `getSiteProperties()` (`public-config/site.js:6-39`), in-memory only; no model,
   DB, transaction, events, jobs, mail, webhook, or HTTP calls.
7. **Cache**: none; `cacheInvalidate:false`; app-level `Cache-Control: private`.
8. **Output**: `output/site.js:5-26` → `{site: <14 fields>}`; same empty/missing behavior as admin row.

**C++** (`file:line`):

- No handler is mounted at `/members/api/site`. The only site route is `Router{router, "/site"}`
  with `optional_auth` (`system/site/routes.cppm:19-28`), installed once via
  `include/blog/handlers/routes.cppm:121` and catalogued only as `GET /site`
  (`testing/route_catalog.cppm:46`). No members-app site handler exists under
  `include/blog/handlers/` (no `/api/site` or `members/api` route match).
- Therefore none of the Ghost behaviors above (middleware, headers, call chain, output) have a C++
  counterpart on this path. The admin `/site` handler (`browse/handler.cppm`) is a different wire
  path and is not a substitute.

**Diff**:

- Residual gaps: entire members-app route is unimplemented — public read of site settings over the
  members app with CORS support (`members/app.js:188-193,27,30`) and its `{site:…}` response are absent.
  Closing this needs a members-app router/mount plus the dynamic CORS delegate
  (`web/members/middleware/cors.js:4-56`), neither of which exists as a C++ primitive; the
  members-app handler would also need a new registration in shared `all.cppm`/`routes.cppm` (main
  agent wiring, out of this group's scope). Recorded as a new-partition/shared-wiring need; the
  exported `build_site_body(ctx, in)` from the site step can produce the identical body.
- Divergences: none to compare (no C++ route).
- Note: a bare `/members/api/site` path alias to the admin handler would still not be a full match —
  the admin C++ app has no members-app CORS/magic-link session layer or its own `Cache-Control`
  mount (those are app-level in Ghost).

## Strict-parity rewrite (group 57, 2026-11-20)

Edits confined to `include/blog/handlers/system/site/**` and the two docs
(`docs/parity/behavior/site.md`, `docs/parity/site.md`). No shared file (`all.cppm`, `base.cppm`,
central `routes.cppm`, `BUILD.bazel`, middleware files) was edited; no Bazel-cycle fix attempted.

- `browse/workflow/step/execute.cppm`: rewritten to reproduce `getSiteProperties()` +
  `output/site.js#read` in a `serde::Value::Object`. All 14 picked keys are now sourced from the
  context-local `config`/`settings` namespaces via `blog::api::parse_array` / `RecordDto`, with
  `URL`/`GHOST_BUILD_VERSION`/`PRO_ENV` env overrides and the `SiteResponse` defaults as fallbacks.
  `url` uses the configured home URL (not `Host`); `version` uses `GHOST_BUILD_VERSION`;
  `allow_external_signup` is computed from the portal settings; `site_uuid` comes from
  config/settings; `icon`/`logo` are populated; `sentry_dsn`/`sentry_env` are conditionally emitted
  from `client_sentry`. Encoded with `core_serde::JsonCodec`. Exports `build_site_body(ctx, in)`.
  `Output = std::string`, `static s_ctx` retained.
- `browse/handler.cppm`: writes the admin-app `Cache-Control: private` value via
  `interfaces::io::types::Token::CACHE_CONTROL`; writes no `X-Cache-Invalidate`. Still runs the
  `site_browse` workflow and posts the string body at `Status::OK`.
- `routes.cppm`, `browse/workflow/flow/main.cppm`, `types/dto_site_response.cppm`: unchanged. The
  public `optional_auth` route and single-step flow already matched; the DTO now serves only as a
  fallback-value source and keeps its partition reachable via the existing imports.

Shared abstractions used (all in already-registered partitions): `core_serde::JsonCodec` and
`serde::Value` (`core_serde`); `blog::api::{parse_array,RecordDto}` (`:domain_all`);
`blog::dto::site::SiteResponse` defaults (`:site_dto_site_response`);
`interfaces::io::types::Token::CACHE_CONTROL` and `IRequest::{get_scheme,get_authority}`
(`interfaces`). No new module or partition was introduced; no new BUILD entry is needed.

New shared needs (recorded, NOT implemented here — no shared files touched):

1. A real config/settings service primitive (or a `core_adapters::Config` dependency added to
   `//include/blog/handlers:blog_handlers`) to replace the context-local store + env fallbacks
   with Ghost's `config.get`/`settingsCache` computation.
2. A registered admin-API auth partition providing `authenticateAdminApi`/`authorizeAdminApi`/
   `updateUserLastSeen`/`tokenPermissionCheck` (including `site:['GET']`) so the admin auth chain
   residual can be closed.
3. A members-app router/mount with the dynamic CORS delegate plus shared handler registration
   (`all.cppm`/`routes.cppm`) to add `GET /members/api/site`; the exported `build_site_body` can
   supply the identical body.

Verdicts: **unchanged**. GET `/site` stays PARTIAL (real config source + admin auth chain remain
residual). GET `/members/api/site` stays MISSING (members-app route needs shared wiring). No MATCH
flip claimed because neither route is fully achieved.

Build verification: isolated `clang -x c++-module` compiles of the rewritten step, handler, flow
and DTO all pass. The full `bazel build //include/blog/handlers:blog_handlers` remains blocked by
pre-existing errors/cycles in other groups' in-progress files; no shared file was edited.

## Module-level notes

- Both Ghost routes share the same op and therefore the exact same `getSiteProperties()` computation
  (`public-config/site.js:6-39`) and the same output whitelist (`output/site.js:5-26`). The only
  Ghost-side differences are route middleware (admin `publicAdminApi` + token allowlist vs members
  bare route + CORS) and which app sets `Cache-Control: private` (`admin/app.js:24` vs
  `members/app.js:27`).
- `site.js` has no `options`/`data`/`validation`, no permissions, no model/service, no cache, and no
  side effects — it is a pure in-memory config projection. This is why the C++ implementation is
  also a single synchronous workflow step with no store mutation and no `core::cache::Cache` use.
- C++ `optional_auth` (`middleware/auth.cppm:31-40`) is the correct primitive for a public endpoint
  and matches both Ghost routes' lack of authentication; the token-permission allowlist
  (`admin/middleware.js:55-56`) and members CORS/magic-link session (`members/app.js:30,33`) have no
  C++ equivalents.
- The strict-parity value pipeline (group 57) is the same fixture-substitution-with-fallbacks pattern
  used by adjacent `config.md` / `featurebase.md` reports: context-local `config`/`settings` reads,
  env overrides (`URL`, `GHOST_BUILD_VERSION`, `PRO_ENV`) and DTO defaults, all rebuilt into the
  Ghost 14-key pick. No cross-route side effects, cache markers, outbox writes or fixture writes exist.
- No `EXTRA` C++ route was found: the C++ site folder contains only the matched admin `/site` route.
