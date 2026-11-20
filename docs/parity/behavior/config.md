# Behavior report: config

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/config.js` (18 lines; only op = `read`)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:23-24`
- C++ folders: `include/blog/handlers/system/config/` (routes + `browse/` + `config_featurebase_get/` + `types/`); related `include/blog/handlers/external/featurebase/` (different controller, context only)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 25, `system/config` half). Verdicts **unchanged** (GET `/config` PARTIAL, GET `/config/featurebase` MISMATCH); see **Strict-parity rewrite (group 25)** below.

## Scope note

`config.js` exports exactly one operation, `read` (`config.js:7-15`). Two admin routes are bound under the
`api.config` pipeline namespace:

1. `GET /config` -> `api.config.read` (`admin/routes.js:23`) — **valid** binding.
2. `GET /config/featurebase` -> `api.config.featurebase` (`admin/routes.js:24`) — **dead binding**: the
   pipeline exposes methods only for controller keys (`pipeline.ts:334,339-461`), and `config.js` has no
   `featurebase` key, so `api.config.featurebase === undefined`. `http(undefined)` is registered; on request
   `await apiImpl(frame)` (`http.ts:116`) throws `TypeError`, caught at `http.ts:169-176` -> `next(err)` -> 500
   via the admin error handler (`admin/app.js:48`).

Both route declarations are documented below; the real Ghost featurebase token op lives under a different
controller (`api.featurebase.token` at `GET /featurebase/token`, `admin/routes.js:414`; `endpoints/index.js:243-244`;
`endpoints/featurebase.js:30`), i.e. outside this module.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

No verdict flipped: the strict-parity rewrite closed the wire-level divergences that have an in-folder
primitive (415 gate, stand-in `X-Cache-Invalidate`, fixture/conditional-key shape, fabricated pagination),
but the full admin auth chain and the real in-memory config source remain residual gaps with no registered
primitive, and the harness pins `/config/featurebase` to a functional 200 body (see record below).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/config` | `config.read` (`docName:'config'`) | none (no `options`/`data`/`validation`; query ignores frame) | `mw.authAdminApi` (route); `permissions:false` (op); integration-token allowlist includes `config:['GET']` | 200 default; `headers.cacheInvalidate:false` -> **no** `X-Cache-Invalidate`; admin app `Cache-Control: private` | `query()` -> `publicConfig.config` getter -> `getConfigProperties()` in-memory (config/settings-cache/labs/databaseInfo/ghostVersion); no model call | none (no cache service/TTL); `cacheInvalidate:false` | `output/config.js.all` -> `{config: <20 picked keys>}`, no pagination meta | `system/config/routes.cppm`, `browse/handler.cppm`, `browse/workflow/{flow,step}`, `types/dto_config_response.cppm`, `types/dto_config_auth.cppm` | route+verb, 200, `{config:…}` envelope and 20-key set match; 415 gate and stand-in `X-Cache-Invalidate` removed; step now builds the picked object with conditional keys + `sanitizeHostSettings`/Tinybird/featurebase/auth picks; `useGravatar` defaults true. Residual: real admin auth chain and real config computation replaced by `require_auth` + context-local store | PARTIAL | Ghost: `config.js:5-15`, `admin/routes.js:23`, `admin/middleware.js:110-118,73`, `public-config/config.js:51-92`, `output/config.js:8-33`, `http.ts:126-137`, `headers.ts:135-144`; C++: `routes.cppm:19-28`, `browse/handler.cppm`, `browse/.../execute.cppm`, `browse/handler.cppm` |
| GET `/config/featurebase` | `config.featurebase` — **op does not exist** (dead route) | none declared (no op to carry input) | `mw.authAdminApi` (route); op-level auth/permissions absent (no op) | Ghost: 500 via `next(err)` (undefined op invoked); no custom headers | `http(undefined)` -> `apiImpl(frame)` TypeError -> 500; no `query`, no service call | none | none (error, no serializer reached) | `system/config/routes.cppm`, `config_featurebase_get/handler.cppm`, `config_featurebase_get/workflow/{flow,step}` | C++ keeps a functional 200 config body (harness-pinned, see record): shared local token check (`{"error":"Unauthorized"}` removed), same strict `{config:…}` body as GET `/config`, **no** fabricated pagination or empty-envelope fixture, admin-app `Cache-Control` only. Divergence remains: C++ functional where Ghost's declared route 500s | MISMATCH | Ghost: `admin/routes.js:24`, `pipeline.ts:334,339-461`, `http.ts:116,169-176`, `admin/app.js:48`; C++: `routes.cppm:30-40`, `config_featurebase_get/handler.cppm`, `config_featurebase_get/.../execute.cppm` |

## Route details

### GET /config — PARTIAL

**Ghost** (facts 1–8, `file:line`):

1. **Identity**: `GET /config` (`admin/routes.js:23`). Router-level mount `router.use(apiMw.cors)` (`admin/routes.js:17`). App mounts admin API at `/ghost/api/admin/` (`web/api/app.js:22` `lazyUse('/admin/')`; `admin/app.js:44` `apiApp.use(routes())`). App-level chain before routes: `cacheControl('private')` (`admin/app.js:24`), body parsers (`admin/app.js:33-34`), `parseQueryBooleans` (`admin/app.js:37`), `checkVersionMatch` (`admin/app.js:41`), error handlers (`admin/app.js:47-48`).
   Route middleware: `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, apiMw.cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). `tokenPermissionCheck` allowlists `config: ['GET']` for integration tokens (`admin/middleware.js:73,85-92`).
2. **Operation**: exported key `read`, op `read`, `docName: 'config'` (`config.js:5,7`); pipeline wrapper in `endpoints/index.js:203-205`.
3. **Input**: no `options`, no `data`, no `validation` block on the op (`config.js:7-15`); `query()` takes no frame use (`config.js:12-14`). No path params, no query aliases/`filter[x]`, no multipart fields, no header reads in controller. Body parsed globally but unused.
4. **Auth/permissions**: `permissions: false` (`config.js:11`) -> permissions stage skipped (`pipeline.ts:229-233`). No `unsafeAttrs`, no labs flag, no permission services inside `query`. Auth is entirely the `mw.authAdminApi` middleware chain (fact 1).
5. **Status + headers**: no `statusCode` -> default 200 (`http.ts:126-133`). `headers: {cacheInvalidate: false}` (`config.js:8-10`) -> `headers.get` guard at `headers.ts:135` is false, so **no `X-Cache-Invalidate`** emitted; no `disposition`/`location` (`headers.ts:124,146-185`). No `response.format`/`stream` -> JSON send (`http.ts:139-148,168`). App-level `Cache-Control: private` (`admin/app.js:24`).
6. **Full call chain**: `query(frame)` returns `publicConfig.config` (`config.js:13`) -> getter calls `require('./config')()` (`utils/public-config/index.js:2-4`) -> `getConfigProperties()` (`utils/public-config/config.js:51-92`), a synchronous in-memory computation from `config.get`, `settingsCache`, `labs.getAll()`, `databaseInfo.getEngine()`, `@tryghost/version`. Fields: `version` (`53`), `environment` (`54`), `database` (`55`), `mail` (`56`), `useGravatar` (`57`), `labs` (`58`), `clientExtensions` (`59`), `enableDeveloperExperiments` (`60`), `stripeDirect` (`61`), `mailgunIsConfigured` (`62`), `emailAnalytics` (`63`), `hostSettings` via `sanitizeHostSettings` which omits `export.webhookSecret`/`emailVerification.webhookSecret` (`64`, `12-18`), `klipy` (`65`), `pintura` (`66`), `signupForm` (`67`), `security` (`68`), `auth = pick(config.get('auth'), ['engine'])` (`70`), plus conditional `exploreTestimonialsUrl` (`73-75`), `stats` (Tinybird payload, `28-49,77-81`), `featurebase` (`83-89`). **No model/repository call, no transaction, no events, no jobs, no mail/webhook/HTTP side effects.** No explicit error branches (pure computation; a `config.get` throw would propagate to `next(err)` at `http.ts:169-176`).
7. **Cache**: `cacheInvalidate:false` suppresses invalidation header; no cache service, key, TTL, or tag. Admin app forces `Cache-Control: private` (`admin/app.js:24`).
8. **Output**: `publicConfig.config` object is passed to output serializer `config.all` (`endpoints/utils/serializers/output/config.js:4-34`), which `_.pick`s exactly 20 keys (`8-29`) and sets `frame.response = {config: <picked>}` (`31-33`). No pagination meta. Empty/missing: only keys present in the computed object appear; base keys always present.

**C++** (`file:line`, after group 25):

- Route: `Router{router,"/config"}` with only `.use(require_auth)` then `.get(...)` -> `handler.config_browse().handle(...)` (`system/config/routes.cppm:19-28`). The former `content_type_middleware` 415/406 gate was removed (Ghost accepts a bodyless GET). No `require_ghost_permission` — Ghost's op declares `permissions:false`. Installed via `ApiRoutes::install`; wiring in `all.cppm:406-407,687` (`config_read()` alias at `:407`).
- Middleware: `require_auth` rejects an empty bearer token with a 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`); token read from `Authorization` (`middleware/bearer.cppm:27-33`). No authorization/updateUserLastSeen/cors/token-allowlist equivalent.
- Handler (`browse/handler.cppm`): runs workflow `config_browse`, casts typed output to string body (default `"{}"`), writes admin-app `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` and **no** `X-Cache-Invalidate` (the stand-in header was removed), `Status::OK`, body, `send()`.
- Step `config_browse_execute` (`browse/workflow/step/execute.cppm`): reads `ctx.store().resources().list("config")` and `ctx.store().json_for("settings",{})`. Builds a `serde::Value::Object` reproducing Ghost's picked set: 17 base keys always (with `useGravatar` defaulting to `!privacy:useGravatar` = true, `sanitizeHostSettings` scrubbing both webhook secrets, Tinybird-style `stats` property picks + `id`/`local`, and `auth` picked to `engine`), plus conditional `exploreTestimonialsUrl`, `stats`, `featurebase` only when their config is present. `build_config_body()` wraps as `{"config": <picked>}` with no `meta`; JSON-encoded with `core_serde::JsonCodec`. `Output=std::string`, `static s_ctx` retained; exported helper is reused by the featurebase step. The four former DTO fixtures are no longer on the wire path for these values.

**Diff**:

- Residual gaps (no C++ primitive): real Ghost admin auth chain (`authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `tokenPermissionCheck` integration allowlist `config:['GET']`) is replaced by bare bearer-token presence (`auth.cppm:19` vs `admin/middleware.js:110-118,73`). Real in-memory config computation from `config`/`settingsCache`/`labs`/`databaseInfo`/`@tryghost/version` (`public-config/config.js:51-92`) is replaced by a context-local store read with harness fallbacks for `version`/`database`/`environment`/`auth.engine`; there is no real config service. `GHOST_BUILD_VERSION` is read from the environment with `std::getenv` (no shared config adapter dependency).
- Divergences: none at the body/header/status level remain. Key order in the JSON object follows `serde::Value::Object` (sorted) rather than Ghost's `_.pick` array order — not wire-significant. `stripeDirect`/`emailAnalytics`/bools default to Ghost's config defaults when absent.
- Matched: path+verb, 200 status, `{config: <20 keys>}` envelope with no pagination meta, conditional key emission, secret scrubbing, Tinybird/featurebase/auth property picks, `useGravatar` default, `Cache-Control: private` with **no** `X-Cache-Invalidate`, and no 415/406 gate.

### GET /config/featurebase — MISMATCH

**Ghost** (facts 1–8, `file:line`):

1. **Identity**: declared `GET /config/featurebase` with `mw.authAdminApi` (`admin/routes.js:24`), same router/app-level chain as above (`admin/routes.js:17`, `admin/app.js:24,33-44`).
2. **Operation**: route references `api.config.featurebase`, but controller `config.js` defines only `read` (`config.js:7-15`); no `featurebase` op, no `docName`-scoped action.
3. **Input**: none — there is no op to declare `options`/`data`/`validation`.
4. **Auth/permissions**: route-level `mw.authAdminApi` applies (`admin/middleware.js:110-118`); no op-level `permissions` because op is absent; pipeline permission stage never runs for a method that doesn't exist.
5. **Status + headers**: no op -> no `statusCode`/`headers` config. On request, `http(undefined)` calls `await apiImpl(frame)` (`http.ts:116`) which throws `TypeError`; caught at `http.ts:169-176` -> `next(err)` -> 500 via `errorHandler.handleJSONResponse` (`admin/app.js:48`). No custom headers.
6. **Full call chain**: `pipeline` builds methods only from controller keys (`pipeline.ts:334,339-461`), so `api.config.featurebase` is `undefined` at route registration; no `query`, service, model, or side effect executes. Ghost's actual featurebase token generation is a separate controller/op (`endpoints/featurebase.js:22,30-39`, `endpoints/index.js:243-244`, bound at `GET /featurebase/token`, `admin/routes.js:414`) — outside this module.
7. **Cache**: none.
8. **Output**: none — error path, serializer not reached.

**C++** (`file:line`, after group 25):

- Route: nested `Route{"/featurebase"}.get(...)` under the `/config` router (`system/config/routes.cppm:30-40`) -> `handler.config_config_featurebase_get().handle(...)`. Inherits the parent `/config` middleware (`require_auth` only at `routes.cppm:20`).
- Handler (`config_featurebase_get/handler.cppm`): the former local `if (token(req).empty())` check (401 `{"error":"Unauthorized"}`) was removed in favour of the shared `require_auth` middleware; runs workflow `config_config_featurebase_get`, casts typed output to string body (default `"{}"`), writes admin-app `Cache-Control`, `Status::OK`, body, `send()`.
- Step (`config_featurebase_get/workflow/step/execute.cppm`): now delegates to the exported `build_config_body(*s_ctx)` from `:config_browse_step`, yielding the same strict `{config: <picked keys>}` body as GET `/config`. The earlier `ConfigEnvelope{}` empty fixture and the unreachable fabricated-pagination branch (`{"meta":{"pagination":{...}}}`) are gone. `Output=std::string`, `static s_ctx` retained. No real featurebase JWT/token logic (that lives in `external/featurebase`, a different controller/path).

**Diff**:

- Residual gaps: Ghost has no working op here (dead route -> 500); the genuine featurebase token behavior is bound to a different path+controller and is a residual gap only for that other module. There is no Ghost body to match for this path.
- Divergences: C++ still fabricates a functioning 200 config endpoint where Ghost's declared route is non-functional (500). This cannot be removed without editing shared harness tests (`include/blog/testing/api_test.cc:296-302` and `snapshot_replay_test.cc:213-214` both assert a 200 config body at this path), which is outside this group's edit scope. The fabricated pagination and empty-envelope stand-in divergences are closed; the non-standard 401 token body is closed by delegating auth to `require_auth`.

## Strict-parity rewrite (group 25, `system/config` half, 2026-11-20)

Edits confined to `include/blog/handlers/system/config/**` and the two docs (`docs/parity/behavior/config.md`, `docs/parity/config.md`). No shared file (`all.cppm`, `base.cppm`, central `routes.cppm`, `BUILD.bazel`, middleware files) was edited; no Bazel-cycle fix attempted.

- `routes.cppm`: removed `content_type_middleware` (Ghost parses bodies globally and accepts a bodyless GET); kept only `require_auth`; deliberately did **not** install `require_ghost_permission` because `config.read` declares `permissions:false`.
- `browse/handler.cppm`: removed the stand-in `X-Cache-Invalidate: false`; writes only the admin-app `Cache-Control` value via `interfaces::io::types::Token::CACHE_CONTROL`.
- `browse/workflow/step/execute.cppm`: rewritten to reproduce `getConfigProperties()` + `output/config.js.all` in a `serde::Value::Object`: 17 unconditional base keys plus conditional `exploreTestimonialsUrl`/`stats`/`featurebase`; `sanitizeHostSettings` omits `export.webhookSecret`/`emailVerification.webhookSecret` (flat and nested forms); Tinybird `stats` copies `endpoint`/`endpointBrowser`/`version`/`datasource` + `id` (+ optional `local` block); `auth` is `pick(..., ['engine'])`; `useGravatar` defaults to true (`!privacyDisabled`). Values are sourced from the context-local `config`/`settings` namespaces with `core_serde::JsonCodec`; `GHOST_BUILD_VERSION` via `std::getenv`. Exposes `build_config_body(blog::BlogContext&)` for reuse; `Output=std::string`, `static s_ctx` retained.
- `config_featurebase_get/handler.cppm`: removed the local `{"error":"Unauthorized"}` token check (now just the shared `require_auth` on the router chain); writes only admin-app `Cache-Control`; no `X-Cache-Invalidate`.
- `config_featurebase_get/workflow/step/execute.cppm`: replaced the empty-envelope fixture and fabricated pagination with a call to the exported `build_config_body(*s_ctx)`; `Output=std::string`, `static s_ctx` retained.

Shared abstractions used (all in already-registered partitions): `core_serde::JsonCodec` and `serde::Value` (`core_serde`), `blog::api::{RecordDto,parse_array}` (`:domain_all`), `blog_handlers_middleware::require_auth` (registered middleware partition), `interfaces::io::types::Token::CACHE_CONTROL`. No new module or partition was introduced; no new BUILD entry is needed.

New shared needs (recorded, NOT implemented here — no shared files touched):

1. A registered admin-API auth partition providing `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`/`tokenPermissionCheck` (including the `config:['GET']` integration-token allowlist) so `require_auth` can be replaced and the auth residual closed.
2. A real config/settings/labs/databaseInfo service primitive (or a `core_adapters::Config` dependency added to `//include/blog/handlers:blog_handlers`) so `std::getenv` + context-local store fallbacks can be replaced with Ghost's in-memory config computation.

Verdicts: **unchanged**. GET `/config` stays PARTIAL (auth chain + real config source remain residual). GET `/config/featurebase` stays MISMATCH (harness-pinned 200 where Ghost's route 500s; shared tests out of scope). No MATCH flip claimed because neither route is fully achieved.

Build verification: `bazel build //include/blog/handlers:blog_handlers` remains blocked by the pre-existing Bazel dependency cycle in that target (unrelated to these files); no shared BUILD, `all.cppm`, `base.cppm`, central route file or middleware file was edited.

## Module-level notes

- Both routes share the admin API app chain (`cacheControl('private')`, JSON/urlencoded body parsers, query-bool parsing, version check) at `admin/app.js:24-48`, and `router.use(apiMw.cors)` at `admin/routes.js:17`. The C++ side mirrors only the observable header (`Cache-Control`) plus `require_auth`; no cors/last-seen/redirect/version/token-allowlist equivalents.
- Both handlers share the same pattern: `LocalWorkflowDriver` + single-step flow, `s_ctx` static pointer to `BlogContext`, string body from `typed_output` defaulting to `"{}"`. The two steps now share `build_config_body()` from `:config_browse_step` so the strict config body has one implementation.
- The 20-field output contract is duplicated on the Ghost side (`public-config/config.js:52-71,73-89` producer; `output/config.js:8-29` allowlist) and reproduced in C++ by the `build_config()` object builder, which now also conditionally emits the three optional keys and applies the same picks/scrubbing as Ghost.
- No cross-route side effects, cache markers, outbox writes, or fixture writes exist in this module. The former stand-in `X-Cache-Invalidate: false` header is gone.
- `/featurebase/token` is intentionally excluded: it binds `api.featurebase`, not `api.config` (`admin/routes.js:414`; C++ `include/blog/handlers/external/featurebase/routes.cppm:19-32`); it is covered in `docs/parity/behavior/featurebase.md`.
