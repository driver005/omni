# Behavior report: tinybird

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/tinybird.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/tinybird/` (routes.cppm, tinybird_token_get/handler.cppm, tinybird_token_get/workflow/flow/main.cppm, tinybird_token_get/workflow/step/execute.cppm)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 31). Verdict stays PARTIAL; see **Strict-parity rewrite (group 31)** below.

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
| GET `/tinybird/token` | custom `token`, docName `tinybird` | none (no options/data/validation/params/query/body) | `mw.authAdminApi` (7 middlewares) + `permissions {docName:'members', method:'browse'}` | 200, JSON, no `X-Cache-Invalidate` (`cacheInvalidate:false`); Admin API `cacheControl('private')` | `query` → `TinybirdServiceWrapper.init()` → `instance.getToken()` → JWT/local/stats/null + in-memory JWT cache + `jwt.sign` | no cache adapter; no TTL/keys; no invalidation header | serializer `tinybird.all` → `{tinybird: <{token,exp ISO}\|{token}\|null>}` | external/tinybird/routes.cppm; tinybird_token_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm} | router `require_auth` + handler `require_ghost_permission("members","browse")`; 200 with `Cache-Control: private` and no `X-Cache-Invalidate`; body from env/config Tinybird service: JWT (HS256, 29 pipe scopes, 300s refresh buffer) → `{"tinybird":{"token":…,"exp":"ISO"}}`, local/stats → `{"tinybird":{"token":…}}`, unconfigured → `{"tinybird":null}` | PARTIAL | Ghost: admin/routes.js:411, tinybird.js:5-27, tinybird-service.js:47-189; C++: routes.cppm:19-37, handler.cppm:17-52, execute.cppm:1-450 |

## Route details

### GET /tinybird/token — PARTIAL

**Ghost** (facts 1–8):

1. **Identity**: `router.get('/tinybird/token', mw.authAdminApi, http(api.tinybird.token));` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:411`. Router-level: `router.use(apiMw.cors)` — routes.js:17. API app mounts admin at `/admin/` (`docs/Ghost/ghost/core/server/web/api/app.js:22`) ⇒ effective prefix `/ghost/api/admin/tinybird/token`. Admin app adds `cacheControl('private')`, `bodyParser.json/urlencoded` (50mb), `parseQueryBooleans`, `checkVersionMatch`, then `routes()` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/app.js:23-44`. Route middleware `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/middleware.js:110-118`.

2. **Operation**: controller key `token` with `docName: 'tinybird'` — `tinybird.js:5,7`; it is a custom action whose logic lives in `async query()` — `tinybird.js:15-27`. Controller registered into the framework pipeline via `apiFramework.pipeline(require('./tinybird'), localUtils)` — `docs/Ghost/ghost/core/server/api/endpoints/index.js:239-241`.

3. **Input**: the `token` controller declares **no** `options[]`, `data[]`, `validation{}`, `statusCode`, multipart fields, path params, query aliases (`filter[x]=y`) and reads no request headers directly — entire config is `tinybird.js:7-27` (only `headers`, `permissions`, `query`). Frame is built by the `http` wrapper from `req.body/file/files/query/params/user/session/url/context` — `packages/api-framework/src/http.ts:86-105`; no schema/parseRequest (`tinybird.js` has no `schema`) ⇒ legacy validation/input stages run with empty config — `pipeline.ts:400-433`.

4. **Auth/permissions**: `mw.authAdminApi` on the route (routes.js:411; middleware.js:110-118). Controller sets `permissions: {docName:'members', method:'browse'}` — `tinybird.js:11-14`. Pipeline permissions stage (required key present) calls `apiUtils.permissions.handle(...)` — `pipeline.ts:209-250`. `handle` parses context, passes through only for `context.public`, else `nonePublicAuth` — `docs/Ghost/ghost/core/server/api/endpoints/utils/permissions.js:94-107`. `nonePublicAuth` singularizes `members`→`member` and calls `permissions.canThis(frame.options.context).browse.member(...)` — permissions.js:22-48; `NoPermissionError` is reworded "You do not have permission to browse members" — permissions.js:7-9,66-73. No `unsafeAttrs`, no labs flag, no in-`query` permission service calls.

5. **Status + headers**: `headers: { cacheInvalidate: false }` — tinybird.js:8-10; `headers.get` emits `X-Cache-Invalidate` only when the value is truthy — `packages/api-framework/src/headers.ts:135-144` ⇒ **no** `X-Cache-Invalidate`. No `disposition`, no `location` config. `http` defaults `statusCode = 200` — http.ts:126-133; default JSON `res.json(result || {})` — http.ts:139-168. Router-level `Cache-Control: private` — admin/app.js:24.

6. **Full call chain**: `query(frame)` → `TinybirdServiceWrapper.init()` (tinybird.js:16; `docs/Ghost/ghost/core/server/services/tinybird/index.js:1` → `tinybird-service-wrapper.js:7-26`): reads `config.get('tinybird')` and `settingsCache.get('site_uuid')`; if either missing logs `logging.warn('Tinybird service not configured')` and sets `instance = null` — wrapper:12-19, otherwise `new TinybirdService({tinybirdConfig, siteUuid})` — wrapper:22-25. Then `TinybirdServiceWrapper.instance?.getToken() ?? null` — tinybird.js:17. `getToken` — `docs/Ghost/ghost/core/server/services/tinybird/tinybird-service.js:107-135` — branch order: (a) JWT when `workspaceId && adminToken` (109-120): reuses cached `_serverToken` unless `_isJWTExpired` (300s buffer, `jwt.verify` HS256 — 177-189), else `_generateToken` (112, 143-168) with payload `{workspace_id, name, exp, scopes: TINYBIRD_PIPES×{type:'PIPES:READ', resource, fixed_params.site_uuid}}` (47-78,146-160), `jwt.sign(payload, adminToken, {noTimestamp:true})` (162), exp in epoch seconds (144), cached into `_serverToken/_serverTokenExp` (113-114); (b) local token `stats.local.token` (122-126); (c) stats token `stats.token` (128-132); (d) `null` (134). `siteUuid` may be overridden by `tinybirdConfig.stats?.id` — service:90. Controller converts JWT `exp` seconds to ISO: `new Date(tokenData.exp * 1000).toISOString()` and returns `{token, exp}` — tinybird.js:19-24; otherwise returns `tokenData` unchanged (`{token}` or `null`) — tinybird.js:26. **Side effects**: warning log on unconfigured (wrapper:16); JWT signing (service:162); in-memory token cache + refresh (service:113-114). No model/DB call, no events, no jobs, no outbox/mail/webhook, no remote HTTP fetch. **Error branches**: 401/403 from `authAdminApi`, `NoPermissionError` from permissions (permissions.js:66-73); no NotFound/Validation/BadRequest in the controller itself; framework forwards errors via `next(err)` — http.ts:169-176.

7. **Cache**: no `cache` adapter on the controller ⇒ pipeline cache get/set skipped — `pipeline.ts:410-453`; `cacheInvalidate:false` ⇒ no invalidation header (headers.ts:135-144); no cache keys/TTLs/tags; Admin API `Cache-Control: private` — admin/app.js:24.

8. **Output**: serializer registry `tinybird` → `require('./tinybird')` — `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/index.js:179-181`; dispatch picks `tinybird.token` (absent) then `tinybird.all` — `packages/api-framework/src/serializers/handle.ts:99-105`. `all(data, apiConfig, frame)` sets `frame.response = {tinybird: data}` — `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/tinybird.js:3-10`. Envelope `{tinybird: {token, exp ISO}}` (JWT path), `{tinybird: {token}}` (local/stats), or `{tinybird: null}` (unconfigured/no token). No pagination meta/computed fields. Missing/null result still serialized to `{tinybird:null}`, and `res.json(result || {})` yields that object — http.ts:147.

**C++** (pre-group-31 snapshot; see **Strict-parity rewrite (group 31)** below for applied behavior):

1. **Files/symbols**: `TinybirdRoutes::install` — `include/blog/handlers/external/tinybird/routes.cppm:15-33`; registered in top-level router — `include/blog/handlers/routes.cppm:58,130`; handler accessor in `include/blog/handlers/all.cppm:204,575,855`. Handler `TinybirdTokenGetHandler` — `tinybird_token_get/handler.cppm:10-27` (ctor 12-16, `handle` 17-25); flow `MainFlow` — `workflow/flow/main.cppm:7-10`; step `ExecuteStep` — `workflow/step/execute.cppm:9-44`. Route catalog confirms GET `/tinybird/token` — `include/blog/testing/route_catalog.cppm:217`, `snapshot_cases.cppm:197`.

2. **Input type/parsing**: `ExecuteStep::Input = interfaces::io::IRequest&`, `Output = std::string` — execute.cppm:11. No body/query/params parsing in handler or step; only the raw request is forwarded to the workflow — handler.cppm:20. Data source: `mutations.json_for("settings", {})` — execute.cppm:14,19 — which resolves to a **hardcoded** `settings_json()` `[{"key","value"}]` fixture (`context/context_context.cppm:1152-1161`, `1682-1716`). That fixture contains no `tinybird_*` keys, so the `kv` map built at execute.cppm:20-25 never holds tinybird credentials.

3. **Status/body/header writes**: `res.set_status(Status::OK)`, `res.set_body(bytes)`, `send()` — handler.cppm:23-24. Body is the step string `{"tinybird":<token_json>}` — execute.cppm:39. No `X-Cache-Invalidate`, `Cache-Control`, `Content-Disposition` or `Location` headers written. Error branches from middleware: 401 `UnauthorizedError` — `middleware/auth.cppm:19-26`; 415 `UnsupportedMediaTypeError` — `middleware/serde.cppm:97-108`.

4. **`ctx.store()` / cache**: `auto& mutations = ctx.store()` and `mutations.json_for("settings", {})` — execute.cppm:14,19 (resource string `"settings"`, no params). No mutations written, no other resources accessed. No `core::cache::Cache` used by this handler (a `make_cache()` exists on `BlogContext` — context_context.cppm:1719 — but is not referenced here).

5. **Middleware chain**: router-level `.use(content_type_middleware).use(require_auth)` — routes.cppm:20-21; `content_type_middleware` rejects missing/non-JSON `Content-Type` with 415 — serde.cppm:90-109; `require_auth` rejects empty `token(request)` with 401 — auth.cppm:12-29, where `token()` reads `Authorization` (strips `Bearer `) — bearer.cppm:27-33. No `bearer_middleware` in this chain. Handler comment confirms auth moved to router chain — handler.cppm:18-19.

6. **Side effects implemented**: none beyond setting `ExecuteStep::s_ctx = &ctx` — handler.cppm:13. No config read, no JWT signing/verification, no in-memory token cache/refresh, no events/outbox/fixture writes, no webhook/HTTP calls.

7. **Branch emulation and its consequence**: step comment claims to mirror `tinybird-service.js:107-135` — execute.cppm:26-27 — `jwt` = both `tinybird_workspace_id` and `tinybird_admin_token` present (28), token = `tinybird_token` / `tinybird_stats_local_token` / `tinybird_stats_token` (29-31); emits `{"token":...}` plus `exp` only when `jwt && tinybird_exp_iso` set (32-38), else `"null"` (32). Because the settings fixture lacks all those keys, `jwt=false` and `token` empty ⇒ token_json `"null"` ⇒ body always `{"tinybird":null}`. `exp` is passed through as a preformatted string, never converted from epoch seconds to ISO.

8. **NOT implemented**: JWT generation/verification (`_generateToken`/`_isJWTExpired`, `jwt.sign`, TINYBIRD_PIPES scopes), config/`site_uuid`/`stats.id` handling, token cache + 300s refresh buffer, epoch→ISO `exp` conversion, `members:browse` permission check, and the rest of `authAdminApi` (`updateUserLastSeen`, CORS, SSL/host redirect, prettyUrls, tokenPermissionCheck, api_key/integration/user context construction).

**Diff** (as of group 31; older group-28 gaps listed in the rewrite section below):

- Residual gaps (no registered primitive): full Admin API auth chain (`authenticateAdminApi`/`authorizeAdminApi` signature verification, `updateUserLastSeen`, CORS, SSL/host redirect, prettyUrls, `tokenPermissionCheck`) — middleware.js:110-118 vs `require_auth` + `require_ghost_permission` in this folder. Structured `logging.warn('Tinybird service not configured')` on the unconfigured branch (wrapper:16) has no shared logging seam reachable from this target. These gaps have no wire effect on the success body.
- Divergences: none in body/status/output envelope/config branch order after group 31. `Cache-Control: private` now written in the handler (admin/app.js:24, cache-control.ts:16-19); no `X-Cache-Invalidate` because `cacheInvalidate:false` (tinybird.js:8-10). No 415/406 gate on the bodyless GET (admin/app.js:33-34).
- Aligned (group 31): verb+path GET `/tinybird/token` (routes.js:411 vs routes.cppm:19-37); 200 (http.ts:126 vs handler.cppm:49); serializer envelope `{tinybird: data}` with no `meta` (output/tinybird.js:3-10 vs execute.cppm:450); JWT HS256 signing with `noTimestamp:true`, 29 `TINYBIRD_PIPES` scopes carrying `fixed_params.site_uuid`, 180-minute `exp` in epoch seconds, and 300s refresh-buffer verify (tinybird-service.js:47-78,107-189 vs execute.cppm:146-356); controller `exp` epoch→ISO conversion (tinybird.js:19-23 vs execute.cppm:iso_from_epoch); JWT → local → stats → null branch order and `stats.id` siteUuid override (tinybird-service.js:88-135 vs execute.cppm:398-448); config from env (`tinybird__*`/`TINYBIRD_*`) + context-local config/settings stores (wrapper.js:12-13 vs execute.cppm:390-396); `members:browse` permission gate (tinybird.js:11-14 vs handler.cppm:38-45). Verdict stays PARTIAL because full admin-chain middlewares remain uncovered by registered primitives.

## Strict-parity rewrite (group 31, 2026-11-20)

Edits confined to `include/blog/handlers/external/tinybird/**` and the two parity
docs. No shared file, central route, `all.cppm`, middleware or BUILD file touched.

- `workflow/step/execute.cppm`: replaced the fixture-only `kv` stand-in with a full
  `TinybirdService` mirror. Config (`workspaceId`, `adminToken`, `stats.id`,
  `stats.token`, `stats.local.enabled`, `stats.local.token`, `site_uuid`) is read
  from nconf-style env (`tinybird__*`) and contract-style env (`TINYBIRD_*`) first,
  then the context-local `config`/`settings` stores (harness overrides). Wrapper
  guard: without tinybird config or `site_uuid` → `{"tinybird":null}`. Service:
  `siteUuid = stats.id || site_uuid`; JWT branch mints an HS256 token locally
  (std-only SHA-256/HMAC/base64url, same primitives as the featurebase group-24
  pass) with header `{"alg":"HS256","typ":"JWT"}`, no `iat`, payload
  `{workspace_id,name,exp,scopes}` where `exp = now + 180min` in epoch seconds and
  `scopes` covers all 29 `TINYBIRD_PIPES` with `fixed_params.site_uuid`. Cached
  token is verified (HMAC recompute, base64url payload `exp`, 300s buffer) before
  reuse; instance-local cache matches the wrapper re-creating the service per
  request. Controller: JWT `exp` converted with `new Date(exp*1000).toISOString()`
  shape (`iso_from_epoch`, `.000Z`), static branches keep `{token}` only. Output
  is the Ghost serializer envelope `{"tinybird": data}` with no `meta`.
  `Output=std::string`, static `s_ctx` retained.
- `handler.cppm`: added the controller permission gate via the shared
  `require_ghost_permission(req,res,next,send,"members","browse")`; admin bearer
  maps to Administrator with `*` (explicit `x-blog-*` headers preserved), denials
  use the Ghost 403 `NoPermissionError` envelope. Writes the admin-app
  `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0,
  post-check=0, pre-check=0` and no `X-Cache-Invalidate`. Body pass-through from
  the step's typed string, 200 status, `send()` unchanged.
- `routes.cppm`: removed `content_type_middleware` (no 415/406 gate on a bodyless
  GET, as in admin/app.js:33-34), kept `require_auth`; comments now point to the
  in-handler permission gate.

Shared abstractions used: `blog::api::{RecordDto,parse_array}` from registered
`:domain_all`; `blog_handlers_middleware::{require_auth,require_ghost_permission,
kGhost*Header}`; `core_serde::JsonCodec`; `core_workflow` step/flow/driver;
`interfaces::io::types::Token::CACHE_CONTROL`. `core_adapters::TinybirdAdapter`
and `core_adapters::tinybird_from_env()` would supply the real env/local-mock
config reader, but `//include/core/adapters:core_adapters` is not a dep of
`//include/blog/handlers:blog_handlers`; a local `env_first` with the same key
spellings is used and the dependency need is recorded below. The adapter's
`GET /v0/token` mock is not used because Ghost's token route never performs an
HTTP fetch — it mints/reads the token locally (tinybird-service.js:107-135).

New shared needs (NOT implemented here; no shared files touched):

1. Add `//include/core/adapters:core_adapters` to `//include/blog/handlers:blog_handlers`
   deps so `tinybird_from_env()`/`detail::env_first` can replace the local env helper
   (and the adapter can be reused for stats-pipe routes).
2. A secret-taking HS256 signer/verifier in `core_auth` (e.g. `JwtSigner::sign(payload, secret)`)
   so the hand-rolled SHA/HMAC/base64url block can be deleted; featurebase records the
   same need.
3. A registered admin-API auth partition providing `authenticateAdminApi`/`authorizeAdminApi`/
   `tokenPermissionCheck` (signature verification, integration-token block) so
   `require_auth` can be replaced; this is the remaining gap keeping verdict PARTIAL.

Verification: step and handler syntax-checked with `g++ -std=c++23 -fsyntax-only`
against local stubs; the generated JWT was independently verified with Python
`hmac`/`hashlib` (valid HS256 signature, 29 scopes, no `iat`, ISO `exp` matching
the epoch), and local/stats/null/stats.id branches were exercised. Full `bazel
build //include/blog/handlers:blog_handlers` remains blocked by the pre-existing
Bazel dependency cycle in that target (unrelated to these files). Verdict:
**unchanged PARTIAL** — wire-observable route facts are closed; the full admin
auth chain is not.

## Module-level notes

Single end-to-end route. Ghost wires `docName:'tinybird'` through the API-framework pipeline (`index.js:239-241`), where output serialization resolves to `tinybird.all` via `getBestMatchSerializer` (`serializers/handle.ts:99-105`). No cross-route side effects: the token is computed per request, cached only in the singleton `TinybirdService._serverToken`/`_serverTokenExp` (tinybird-service.js:97,113-114), with `instance` reset by `TinybirdServiceWrapper.reset()` (wrapper:28-31). C++ mirrors the shape via a single-step workflow whose `ExecuteStep` holds a static context pointer (handler.cppm:13, execute.cppm:42-44). The C++ `settings` resource is special-cased to a hardcoded fixture (`context_context.cppm:1161,1682-1716`), so no runtime configuration can populate tinybird credentials — the decisive functional gap for configured deployments.
