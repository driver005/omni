# Behavior report: featurebase

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/featurebase.js` (56 lines; only op = `token`)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:414` (GET `/featurebase/token`)
- C++ folders: `include/blog/handlers/external/featurebase/` (`routes.cppm` + `featurebase_token_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 24). Verdict stays PARTIAL; see **Strict-parity rewrite (group 24)** below.

## Scope note

`featurebase.js` exports exactly one operation, `token` (`featurebase.js:32-53`). The pipeline namespace
`api.featurebase` is built from this controller (`endpoints/index.js:243-244`) and bound only at
`GET /featurebase/token` (`admin/routes.js:414`). The similarly named `GET /config/featurebase`
(`admin/routes.js:24`) binds `api.config.featurebase`, not this controller — it is a dead binding on a
different controller and is covered in `docs/parity/behavior/config.md`. The C++ folder
`include/blog/handlers/system/config/config_featurebase_get/` is that `/config/featurebase` route,
not the token route; the current counterpart for this module is
`include/blog/handlers/external/featurebase/featurebase_token_get/`, matched by exact path + verb.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/featurebase/token` | `featurebase.token` (`docName:'featurebase'`) | no `options`/`data`/`validation`; reads `frame.user` (authenticated user model) | `mw.authAdminApi` (route, full admin chain); `permissions:false` (op); integration tokens blocked (`featurebase` not in allowlist) | 200 default; `headers.cacheInvalidate:false` → **no** `X-Cache-Invalidate`; admin app `Cache-Control: private` | `query(frame)` → guards `featurebase:enabled`/`:jwtSecret` → `generateJWTToken(frame)` → `jsonwebtoken.sign(userData, secret, {HS256, expiresIn:'7d'})` with user `{name,email,profilePicture}` + `companies[0]` from `settingsCache`/`urlUtils`; no model/service/DB call | none (no cache service/key/TTL); `cacheInvalidate:false` suppresses header | `output/featurebase.js.all` → `{featurebase:{token}}` | `external/featurebase/routes.cppm`, `featurebase_token_get/handler.cppm`, `featurebase_token_get/workflow/{flow/main.cppm,step/execute.cppm}` | route+verb, 200/422, HS256 JWT with 7d `exp`, base64url signing, payload field names, config guard, `{featurebase:{token}}` envelope, `Cache-Control: private` with no `X-Cache-Invalidate`, no 415/406 gate all implemented; but user resolved from store/headers rather than `frame.user` (no signature verification / integration-token block / full admin chain), config from env + context-local store rather than nconf+settingsCache, std-only SHA/HMAC instead of a shared signer | PARTIAL | Ghost: `featurebase.js:7-53`, `admin/routes.js:414,17`, `admin/app.js:24,33-48`, `admin/middleware.js:55-118`, `pipeline.ts:230-233,431-449`, `http.ts:126-148`, `headers.ts:135-144`, `output/featurebase.js:4-10`, `cache-control.ts:16-19,32-35`; C++: `routes.cppm:19-37`, `handler.cppm:16-34`, `execute.cppm:154-279`, `middleware/auth.cppm:12-29` |

## Route details

### GET /featurebase/token — PARTIAL

**Ghost** (facts 1–8, `file:line`):

1. **Identity**: `GET /featurebase/token` with `mw.authAdminApi` (`admin/routes.js:414`, section comment `413`). Router-level mount `router.use(apiMw.cors)` (`admin/routes.js:17`). Admin API app mounted at `/ghost/api/admin/` (`web/api/app.js:22` `lazyUse('/admin/')`; `admin/app.js:44` `apiApp.use(routes())`). App-level chain before routes: `cacheControl('private')` (`admin/app.js:24`), `bodyParser.json/urlencoded` (`admin/app.js:33-34`), `parseQueryBooleans` (`admin/app.js:37`), `checkVersionMatch` (`admin/app.js:41`), `errorHandler.resourceNotFound`/`handleJSONResponse` (`admin/app.js:47-48`). `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, apiMw.cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). `tokenPermissionCheck` rejects integration tokens unless `{entity: [method]}` allowlisted (`admin/middleware.js:17-101`); allowlist has `config: ['GET']` etc. but **no `featurebase` entry** (`admin/middleware.js:55-83`), so integration tokens get 403 `NoPermissionError`; staff/user tokens pass to the permission system (`admin/middleware.js:19-47`).
2. **Operation**: exported key `token`, op `token`, `docName: 'featurebase'` (`featurebase.js:30,32`); pipeline wrapper registered at `endpoints/index.js:243-244`.
3. **Input**: no `options`, no `data`, no `validation` block (`featurebase.js:32-53`). No path params, no query aliases / `filter[x]=y`, no multipart fields, no body shape used. Body is parsed globally but ignored (`admin/app.js:33-34`). The only input consumed is `frame.user` — the authenticated user doc populated by `authenticateAdminApi` and carried into the frame (`http.ts:82-105`, `frame.user` at `http.ts:92`). No headers explicitly read in the controller.
4. **Auth/permissions**: route-level `mw.authAdminApi` (fact 1). Op-level `permissions: false` (`featurebase.js:36`) → permissions stage skipped entirely (`pipeline.ts:229-233`); no permission services called inside `query`. No `unsafeAttrs`, no labs flag. `headers.cacheInvalidate: false` (`featurebase.js:33-35`).
5. **Status + headers**: no `statusCode` → default 200 (`http.ts:126-133`). `headers.get` guard `if (apiConfigHeaders.cacheInvalidate)` is false (`headers.ts:135-144`), so **no `X-Cache-Invalidate`** emitted; no `disposition`, no `location` (`headers.ts:124,146-185`). No `response.format`/stream → `res.json(result)` (`http.ts:139-148,168`). App-level `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (`admin/app.js:24`, `cache-control.ts:16-19,32-35`).
6. **Full call chain**: `query(frame)` (`featurebase.js:37-52`) reads `config.get('featurebase:enabled')` and `config.get('featurebase:jwtSecret')` (`featurebase.js:38-39`; nconf keys `featurebase__enabled`/`featurebase__jwtSecret` via `loader.ts:35-42`, no defaults registered in `shared/config`). If either is falsy → `errors.ValidationError({message:'Featurebase is not configured'})` (`featurebase.js:41-45`; rendered as HTTP 422 `ValidationError`). Otherwise calls `generateJWTToken(frame)` (`featurebase.js:47`). `generateJWTToken` (`featurebase.js:7-26`) builds `userData = {name: frame.user.get('name'), email: frame.user.get('email'), profilePicture: frame.user.get('profile_image'), companies:[{id: settingsCache.get('site_uuid'), name: settingsCache.get('title'), website: urlUtils.urlFor('home', true)}]}` (`featurebase.js:8-20`) and returns `jwt.sign(userData, config.get('featurebase:jwtSecret'), {algorithm:'HS256', expiresIn:'7d'})` (`featurebase.js:22-25`; jsonwebtoken auto-adds `iat` and `exp = iat + 7d`). Returns `{token}` (`featurebase.js:49-51`). **No model/repository call, no transaction, no events, no jobs, no mail/outbox, no webhook, no third-party HTTP.** Error branches: 422 ValidationError (`featurebase.js:42-45`), 401/403 from auth middleware (`admin/middleware.js:110-118,95-100`), 404 `resourceNotFound`/500 via `admin/app.js:47-48` and `http.ts:169-176`.
7. **Cache**: no cache service, key, TTL, or tag anywhere in the op. `cacheInvalidate:false` (`featurebase.js:34`) suppresses the invalidation header (`headers.ts:135-144`). Admin app forces `Cache-Control: private` (`admin/app.js:24`).
8. **Output**: query result `{token}` is passed to output serializer `featurebase.all` (`output/featurebase.js:4-10`), which sets `frame.response = {featurebase: data}` (`output/featurebase.js:7-9`) → wire body `{"featurebase":{"token":"<jwt>"}}`. No pagination meta, no computed fields beyond the JWT. Empty/missing results: never empty — either a token or a thrown ValidationError.

**C++** (`file:line`, after group 24):

- Route: `Router{router, "/featurebase"}` (exact path match) with `.use(require_auth)`; nested `Route{"/token"}.get(...)` → `handler.featurebase_featurebase_token_get().handle(...)`; summary `"Ghost token GET"` (`external/featurebase/routes.cppm:19-37`). The former `content_type_middleware` 415/406 gate was removed: Ghost parses bodies globally and accepts a bodyless GET (`admin/app.js:33-44`). Installed via `ApiRoutes::install` (`include/blog/handlers/routes.cppm:21,93`). Catalog marks it authenticated (`testing/route_catalog.cppm:218`) with snapshot case (`testing/snapshot_cases.cppm:198`).
- Middleware chain: `require_auth` rejects an empty bearer token with 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`; token read from `Authorization` at `middleware/bearer.cppm:27-33`). No `require_ghost_permission` gate — Ghost's op declares `permissions:false` (`featurebase.js:36`) so the permission stage is skipped. Still no `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`/cors/redirect/`tokenPermissionCheck` equivalent; no signature verification. Any non-empty Authorization value passes.
- Handler (`featurebase_token_get/handler.cppm:16-34`): ctor registers `ExecuteStep` on `MainFlow` with a `LocalWorkflowDriver`. `handle` runs workflow `"featurebase_featurebase_token_get"`, reads `result_of("featurebase_featurebase_token_get_execute")`, casts `typed_output` to `map<string,string>`, parses status/body, converts body to bytes. Sets `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (admin app `cacheControl('private')`), and writes **no** `X-Cache-Invalidate` (`cacheInvalidate:false` is falsy). Then `set_status`/`set_body`/`send`.
- Flow: `MainFlow.build()` registers only `ExecuteStep` (`workflow/flow/main.cppm:7-10`).
- Step `featurebase_featurebase_token_get_execute` (`workflow/step/execute.cppm`; `Input = IRequest&`, `Output = map<string,string>`, `name` at `:153`):
  - std-only SHA-256/HMAC-SHA256/base64url primitives (`execute.cppm:14-121`); header comment records the need for a secret-taking shared signer. `find_setting` scans `RecordDto` records for `key`/`value` or a direct key (`:142-152`); `clean_field` treats `null`/`undefined` as absent (`:129-134`); `env_first` reads environment variables (`:154-167`); `truthy` implements nconf `parseValues` truthiness (`:169-172`).
  - Config: `featurebase:enabled` from env `FEATUREBASE__ENABLED`/`FEATUREBASE_ENABLED` (nconf `__` separator), then context-local `config`, then `settings` (`:190-195`). `featurebase:jwtSecret` from `FEATUREBASE__JWTSECRET`/`FEATUREBASE_JWT_SECRET` then the same stores (`:196-198`). Missing/falsy → status 422 with body `ApiError{}.validation("Featurebase is not configured", "", "")`, i.e. Ghost's `ValidationError` message with no `property`/`context` override (`:200-207`).
  - User: authenticated subject from `x-blog-user-id` or admin bearer → `"1"` (same mapping as `admin/users`), then the matching row from `ctx.store().json_for("users", {})` supplies `name`/`email`/`profile_image`; the `x-blog-user-*` headers are only a fallback when no user row matches (`:209-231`).
  - Company: `title`/`site_uuid` from settings then config (fallbacks `"Ghost"` and the harness seed UUID) (`execute.cppm:233-240`); `home` from env `URL`, then config `url`, then request `scheme://authority/`, then `http://localhost:2368/` (`execute.cppm:242-251`).
  - `iat = now seconds`, `exp = iat + 7*24*3600` (`:253-255`); header `{"alg":"HS256","typ":"JWT"}` and payload `{name,email,profilePicture,companies:[{id,name,website}],iat,exp}` JSON-encoded via `core_serde::JsonCodec` (`:257-263`); `signing_input = base64url(header) + "." + base64url(payload)`, signature `hmac_sha256(secret, signing_input)`, token `signing_input + "." + base64url(signature)` (`:265-268`).
  - Body `{"featurebase":{"token":<jwt>}}` with status 200 (`:270-274`). `compensate` is a no-op (`:276`).
  - `ctx` is a static `BlogContext*` (`:277-279`).
- Side effects: none beyond the response. No outbox records, no fixture writes, no events/jobs. `ctx.store()` reads are cache-backed store lookups; there is no direct cache key/tag/TTL use.

**Diff**:

- Residual gaps (no C++ primitive / not implemented): full Ghost admin auth chain (`authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `tokenPermissionCheck` with the integration-token block for `featurebase`) is still replaced by bare `require_auth` token presence (`auth.cppm:19` vs `admin/middleware.js:110-118,55-83,95-100`); there is no signature verification. Real config (`config.get('featurebase:*')` via nconf, `loader.ts:35-42`) and `settingsCache`/`urlUtils.urlFor('home', true)` are approximated by env keys + context-local `config`/`settings` namespaces (`:190-251`). `frame.user` from the DB-authenticated user (`featurebase.js:9-11`) is approximated by a store lookup keyed by the harness subject (`x-blog-user-id` or admin bearer → id `1`) with header fallback. The `jsonwebtoken` library is replaced by hand-rolled SHA/HMAC over `import std` (`:14-121`).
- Divergences (C++ differs from Ghost):
  - **User source**: the store row is the harness's seeded user, not the live authenticated model; if the subject cannot be resolved the `x-blog-user-*` headers are still used, so a client can inject values when no row is found (`execute.cppm:209-231` vs `featurebase.js:9-11`).
  - **Config source and fallbacks**: harness fallbacks remain (hardcoded UUID `"d5432f87-..."`, title `"Ghost"`, home `http://localhost:2368/`) where Ghost has real `settingsCache`/`urlUtils` values (`execute.cppm:233-251` vs `featurebase.js:15-17`).
  - **Auth status**: C++ `require_auth` returns 401 `UnauthorizedError` for a missing token; Ghost's admin chain may answer 403 (`test/e2e-api/admin/featurebase.test.js:14-18`).
- Matched: exact path + verb; `{"featurebase":{"token":...}}` output envelope (`output/featurebase.js:7-9`); JWT `alg:HS256`, base64url three-segment signing, payload field names (`name`, `email`, `profilePicture`, `companies[0].id/name/website`), `iat`+`exp` with 7-day lifetime; 200 success / 422 not-configured status codes; Ghost-shaped 422 `ValidationError` message; `permissions:false` equivalent (no framework permission stage); no `X-Cache-Invalidate` plus app-level `Cache-Control`; no 415/406 gate on a bodyless GET; stateless operation with no side effects.

## Strict-parity rewrite (group 24, 2026-11-20)

Edits confined to `include/blog/handlers/external/featurebase/**` and this docs file.

- `workflow/step/execute.cppm`: output is now the Ghost serializer envelope `{"featurebase":{"token":...}}`; the 422 body is `ApiError{}.validation("Featurebase is not configured", "", "")` (message-only Ghost `ValidationError`, no `property`); config reads nconf-style env keys first (`FEATUREBASE__ENABLED`, `FEATUREBASE__JWTSECRET`) with the context-local `config`/`settings` namespaces as harness overrides; home URL prefers `URL`/config `url`; user fields come from the authenticated subject's store row (`x-blog-user-id` or admin bearer → id `1`, as in `admin/users`) with header fallback. `Output` remains `std::map<std::string,std::string>` carrying `status`/`body`; static `s_ctx` retained.
- `handler.cppm`: removed the stand-in `X-Cache-Invalidate: false`; writes the admin-app `Cache-Control` value (`interfaces::io::types::Token::CACHE_CONTROL`). Status/body still propagate from the step map, so 422 is preserved.
- `routes.cppm`: removed `content_type_middleware` (no 415/406 gate on a bodyless GET), kept `require_auth`. `require_ghost_permission` is deliberately not installed because the op declares `permissions:false`.

Shared abstractions used: `core_serde::JsonCodec` (payload/body encoding), `blog::api::{ApiError,RecordDto,parse_array}` from the registered `:domain_all` partition, `blog_handlers_middleware::require_auth`, and `blog::handlers::middleware::principal_from_request`'s subject convention (admin bearer → `1`). `core_adapters::detail::env_first` would be the shared config reader, but `//include/core/adapters:core_adapters` is not a dep of `//include/blog/handlers:blog_handlers`; std::getenv with the same env key spellings is used instead and the dependency need is recorded below.

New shared needs (NOT implemented here; no shared files touched):

1. `core_auth`: a secret-taking HS256 signer (`JwtSigner::sign(payload, secret)` or a free `sign_jwt`) so the hand-rolled SHA/HMAC/base64url block in `execute.cppm` can be deleted. Existing `core_auth:jwt_signer` generates a random key at construction and cannot sign with `featurebase:jwtSecret`.
2. `core_adapters`: add `//include/core/adapters:core_adapters` to `//include/blog/handlers:blog_handlers` deps so `env_first`/`from_env` can replace the local env helper.
3. A registered admin-API auth partition providing `authenticateAdminApi`/`authorizeAdminApi`/`tokenPermissionCheck` (with the featurebase integration-token block) and signature verification, so `require_auth` can be replaced.

Verdict: **unchanged PARTIAL**. All wire-observable route facts except the full admin auth chain are closed; the auth chain and the three shared-helper needs above keep the route short of MATCH, so no verdict was flipped.

Build verification: `bazel build //include/blog/handlers:blog_handlers` remains blocked by the pre-existing Bazel dependency cycle in that target (unrelated to these files); no shared BUILD, `all.cppm`, `base.cppm`, central route file or middleware file was edited.

## Module-level notes

- `featurebase.js` is a single-op controller; no shared services, no cross-route side effects. Its only external reads are `config.get`, `settingsCache`, `urlUtils`, and `frame.user`; its only external write is `jwt.sign` (pure crypto, no I/O).
- C++ counterpart wiring is local: `ApiRoutes::install` → `FeaturebaseRoutes::install` → `BlogHandlers::featurebase_featurebase_token_get()` → `FeaturebaseTokenGetHandler::handle` → `LocalWorkflowDriver` → `MainFlow` → `ExecuteStep`. The step uses a static `BlogContext*` set in the handler ctor.
- The `/config/featurebase` route in both trees is a separate controller (`config.js`/`config_featurebase_get`) and is intentionally excluded here; see `docs/parity/behavior/config.md`.
