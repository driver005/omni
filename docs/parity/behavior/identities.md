# Behavior report: identities

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/identities.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:437`
- C++ folders: `include/blog/handlers/auth/identities/` (routes + `browse/` handler/flow/step); mounted at `include/blog/handlers/routes.cppm:26,98`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19 (manifest group 12)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

The strict-parity pass closed every divergence that has an in-folder C++
primitive: Ghost response envelope, `X-Cache-Invalidate` suppression, admin
`Cache-Control`, the Content-Type gate, the `permissions`/`docName` check and
the identity claims shape. The verdict stays `PARTIAL`: real RS256 JWT signing,
a real `User`+`roles` relation load, real admin-token verification and the
`/ghost/api/admin` mount prefix have no C++ primitive in this folder and are
recorded below, not hidden. Verdicts flip only when fully achieved.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/admin/identities` | `read` (docName `identities`) | None from client: no body/params/query; identity from `frame.user` (email) + loaded `roles` relation | `mw.authAdminApi` chain + `permissions:true` (`canThis().read.identity`) | 200; `Cache-Control: no-cache, private, no-store, ...`; no `X-Cache-Invalidate` | `IdentityTokenService.instance.getTokenForUser(email, role)` → `signingKeys.getSigningKey()` → `jsonwebtoken.sign` RS256 | `cacheInvalidate:false` → no invalidation; Admin API `cacheControl('private')` | `{ identities: [ { token } ] }` via output `default.all`; token = RS256 JWT, 5m | `auth/identities/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | 200; no `X-Cache-Invalidate`; full admin `Cache-Control`; body `{"identities":[{"token":"<claims JSON>"}]}`; auth = admin token→Administrator mapping then shared `GhostAuthorizer` for `identities`/`read` (401/403 Ghost envelopes) | PARTIAL | Ghost `identities.js:6-29`, `routes.js:437`; C++ `routes.cppm:23-62`, `handler.cppm:16-27`, `execute.cppm:24-57` |

## Route details

### GET /ghost/api/admin/identities — PARTIAL

**Ghost** (numbered facts 1–8):

1. **Identity**: `router.get('/identities', mw.authAdminApi, http(api.identities.read))` — `admin/routes.js:437`. Router-level mounts: api app at `BASE_API_PATH` `/ghost/api` (`web/parent/backend.js:15`, `ghost/core/shared/url-utils.ts:4`), then `/admin/` (`web/api/app.js:22`), then `/admin/app.js:44` uses `routes()`. Admin app also installs `cors` (`admin/routes.js:17`), `cacheControl('private')`, body parsers, `parseQueryBooleans`, `checkVersionMatch` (`admin/app.js:24,33-34,37,41`).
2. **Operation**: exported key `read`, `docName: 'identities'` (`identities.js:5-6`). No `statusCode`, no custom `response.format`, no `schema`/`options`/`data`/`validation`, no `unsafeAttrs`, no labs flag.
3. **Input**: no schema, no `options[]`, no `data[]`, no `validation{}`, no path params, no query aliases, no multipart, no headers read directly. Identity sourced from `frame.user.get('email')` (`identities.js:23`); role loaded via `await frame.user.load(['roles'])` then `frame.user.relations.roles.toJSON()[0].name` (`identities.js:16-17`). Any load failure is caught and logged (`logging.warn('Could not load role for identity')`, `identities.js:18-20`), leaving `role = null`.
4. **Auth/permissions**: `mw.authAdminApi` middleware chain = `authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, SSL/host redirect, `prettyUrls`, `tokenPermissionCheck` (`admin/middleware.js:110-118`). Plus `permissions: true` on the controller (`identities.js:10`): pipeline stage calls `permissions.handle` (`pipeline.ts:243-248`) → `canThis(context).read.identity(...)` (`endpoints/utils/permissions.js:47-48`; singular `identities`→`identity` at `:22-23`). `identity: ['read']` permission exists and is granted to Administrator (`permissions-data.ts:40,107`). Permission failure → `NoPermissionError` with message "You do not have permission to read identities" (`permissions.js:8,67-72`).
5. **Status + headers**: default status 200 (`http.ts:126-133`; no `statusCode` in controller). Headers from `apiImpl.headers` (`http.ts:135-137`): `cacheInvalidate: false` is falsy so `headers.get` emits no `X-Cache-Invalidate` (`headers.ts:135-144`; note `false` never reaches the `if`). No `disposition`, no `location` (read, no frame data). `frame.getHeaders()` merged (`headers.ts:187-189`). Admin app sets `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (`admin/app.js:24`, `cache-control.ts:16-18,32-34`). Response is JSON (`http.ts:146-147`).
6. **Full call chain**: `query(frame)` (`identities.js:11`) lazily requires `../../services/identity-tokens` (`identities.js:12`; re-export wrapper `identity-tokens/index.js:1`) → `IdentityTokenService.instance.getTokenForUser(email, role)` (`identities.js:22-25`). Wrapper initializes `instance` with `signingKeys.service.forPurpose('staff')` and issuer `urlUtils.urlFor('admin', true)` (`identity-token-service-wrapper.js:12-20`). Service builds claims `{sub: email, role?}` (`identity-token-service.ts:16-22`), calls `signingKeys.getSigningKey()` → `{privateKey, kid, legacy}` (`:24`), then `jsonwebtoken.sign(claims, privateKey, {issuer, expiresIn:'5m', algorithm:'RS256', allowInsecureKeySizes: legacy, keyid: kid})` (`:26-32`). Returns `{ token }` (`identities.js:27`). Side effects: DB `roles` relation load; `updateUserLastSeen` middleware; signing-key lookup. Errors: role-load errors swallowed with warning; permission/auth errors bubble to `errorHandler` (`admin/app.js:46-48`) → 401/403; no explicit NotFound/Validation/BadRequest branches.
7. **Cache**: controller `headers: { cacheInvalidate: false }` (`identities.js:7-9`) → no `X-Cache-Invalidate` emitted; no `apiImpl.cache` stage (`pipeline.ts:436-441,451-453`); no cache service/key/TTL/tag. Admin API-wide `Cache-Control: private/no-store` applies (see #5).
8. **Output**: pipeline returns `frame.response` (`pipeline.ts:443-446`); no `identities` serializer exists (`utils/serializers/output/index.js:1-186`, no `identities` getter), so `getBestMatchSerializer(...,'default','read')` falls back to `default.all` (`handle.ts:152-169,99-104`). `default.all` wraps as `{ identities: [ mapResponse(...) ] }`, no `meta` (`output/default.js:22-41`); `all.after` strips `published_by` (`output/all.js:20-27`). Field set: single object `{ token }`. Empty/missing: `getTokenForUser` either returns a string or throws; on success token always present.

**C++** (numbered facts, after strict-parity pass):

1. **Route**: `Router{router, "/identities"} .use(identities_routes_detail::require_permission) .get(...)` → `handler.identities_read().handle(...)` (`auth/identities/routes.cppm:53-62`). Installed via `IdentitiesRoutes{}.install(router, handlers)` (`handlers/routes.cppm:98`); `identities_read()` aliases the single `BrowseHandler` (`all.cppm:583`, member `:862`). The router-level `content_type_middleware`, `bearer_middleware` and bare `require_auth` were removed — Ghost has no Content-Type gate on this GET. No version/prefix mount in C++ (relative `/identities`).
2. **Operation**: internal type is `identities::browse::BrowseHandler`, workflow `"identities_browse"`, step `"identities_browse_execute"` (`browse/handler.cppm:9-15`, `browse/workflow/flow/main.cppm:7-10`, `browse/workflow/step/execute.cppm:22`). No Ghost `read`/`browse` distinction — one shared handler exposed through the `read`-named accessor.
3. **Input** (`execute.cppm:31-34`): `using Input = interfaces::io::IRequest&`. `email` from `x-blog-identity-email`, falling back to the authenticated bearer subject via `middleware::token(in)`; `role` from `x-blog-identity-role`, falling back to `x-blog-role` (mapped from the admin token by the route). No body/params/query parsing. No User model / `roles` relation load — accepted residual gap.
4. **Auth/permissions** (`routes.cppm:23-42`): `identities_routes_detail::require_permission` sets `x-blog-context=ADMIN_API`, maps a presented admin token to `x-blog-role=Administrator` + `x-blog-permissions=*`, then calls the shared `blog::handlers::middleware::require_ghost_permission(req, res, next, send, "identities", "read")`. That runs `core::auth::ghost::GhostAuthorizer` and writes Ghost-shaped 401 `UnauthorizedError` / 403 `NoPermissionError` envelopes on denial. The former inline bearer/non-empty-token check and the Content-Type 415 gate are gone. No `updateUserLastSeen`, SSL/host redirect or labs gate. No C++ `ctx.store()` calls in this route.
5. **Status/headers** (`handler.cppm:16-27`): handler sets `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0`, `Status::OK` (200), raw body bytes, then `send()`. No `X-Cache-Invalidate` marker — matches Ghost `cacheInvalidate:false` emitting no header.
6. **Call chain**: `handle()` runs `WorkflowDriver` with `LocalWorkflowDriver` (`handler.cppm:11-17`); `ExecuteStep::invoke` builds Ghost claims `{sub: email, role?}` as a `serde::Value::Object` (`execute.cppm:38-42`), encodes it with `core_serde::JsonCodec` (`:45`), and wraps it in the `default.all`-shaped `{identities:[{token}]}` body (`:50-57`). `compensate` no-op (`:60`). Static `s_ctx` set but unused (`:64,75`). No events/jobs/mail/webhooks/DB writes.
7. **Cache**: no `X-Cache-Invalidate` header and no `core::cache::Cache`, keys, tags or TTL anywhere in the route — both sides cache-free.
8. **Output** (`execute.cppm:50-57`): body is `{"identities":[{"token":"<claims JSON>"}]}`, no `meta`, status 200; on workflow failure with no typed output the handler falls back to `{}` (`handler.cppm:19`).

**Diff**:

Closed by the strict-parity pass:
- Response envelope: C++ now returns Ghost's `{"identities":[{"token":...}]}` (`default.all`) instead of the flat `{"token":...}`.
- `X-Cache-Invalidate`: the stand-in `false` marker is removed; Ghost emits no header for `cacheInvalidate:false`.
- `Cache-Control`: C++ now sets Ghost's full admin `no-cache, private, no-store, ...` directive.
- Content-Type gate: router-level `content_type_middleware` (415 on non-JSON) removed; Ghost has no such gate on this GET.
- Auth/permissions: admin-token→Administrator mapping + shared `GhostAuthorizer` for `identities`/`read` with Ghost 401/403 envelopes replaced the bare non-empty-token check.
- Claims shape: token payload is now Ghost's `{sub, role?}` instead of the earlier C++-named `{email, role, sub}`.

Residual gaps (Ghost behavior with no C++ primitive in this folder):
- Real RS256 JWT signing: Ghost `getTokenForUser` signs with `privateKey/kid/legacy`, issuer admin URL, `expiresIn:'5m'` (`identity-token-service.ts:24-32`); C++ emits a serde-encoded claims string — not signed, no issuer/expiry/kid/signature.
- Real admin-token verification / user lookup: `authAdminApi` resolves a token to a staff user and `identity:read` is granted only to Administrator (`admin/middleware.js:110-118`, `permissions-data.ts:40,107`); C++ maps token presence to Administrator (no token store), then enforces the permission through the shared authorizer. `updateUserLastSeen` and SSL/host redirect have no primitive here.
- User identity source: Ghost uses authenticated `frame.user` email + DB-loaded `roles` relation (`identities.js:16-17,23`); C++ uses `x-blog-identity-*` headers with authenticated-principal fallback.
- Endpoint mount: Ghost full path `/ghost/api/admin/identities` (`backend.js:15`, `web/api/app.js:22`); C++ mounts bare `/identities` — the flat router and route-catalog fixtures require the bare path; changing the mount is a central concern, out of scope.
- Denial message wording: shared `permission_message()` normalizes `method=read` to the action word `browse`, so the 403 envelope reads "You do not have permission to browse identities" instead of Ghost's "…read identities" (`permissions.js:8,67-72`); fixing it means changing shared code, out of scope.

Divergences (C++ differs from Ghost):
- C++ flow/step named `browse` while Ghost op is `read` (naming-only; same handler slot via `all.cppm:583`).
- No `identities` output mapper in Ghost means pass-through `{token}` is wrapped by `default.all`; C++ builds that envelope directly — behaviorally identical.

## Strict-parity implementation notes

### Shared abstractions used

- `core::auth::ghost::GhostAuthorizer` + `blog::handlers::middleware::require_ghost_permission`
  — full middleware shape in `identities_routes_detail::require_permission`
  (`routes.cppm:23-42`) enforces `permissions:true` for `docName=identities`,
  `method=read`; denials use Ghost `UnauthorizedError`/`NoPermissionError`
  envelopes from `write_ghost_denial`. No inline token check remains.
- `blog::handlers::middleware` header constants (`kGhostContextHeader`,
  `kGhostRoleHeader`, `kGhostPermissionsHeader`) and `token()` — reused for the
  admin-principal mapping and identity input; no local stand-ins.
- `core_serde::JsonCodec` + `serde::Value` — builds the Ghost `{sub, role?}`
  claims and the `{identities:[{token}]}` wire body with correct escaping;
  no hand-rolled JSON encoder left in the step.
- `core_workflow` / `LocalWorkflowDriver` — unchanged single-step flow;
  `Output = std::string` and the `s_ctx` pointer are retained per contract.
- `core_adapters`, `core_outbox`, `core_query`, `blog_handlers:mapping` — not
  relevant (no external call, no event/job, no query/options, no field mapper
  for this route); documented, not wired.

### Central registrations

None required. All four touched files were already registered partitions in
`include/blog/handlers/BUILD.bazel` (`:681-683`, `:986`), and the read-named
`identities_read()` accessor already exists on `blog::api::BlogHandlers`
(`all.cppm:583`). No `all.cppm`/`base.cppm`/`routes.cppm`/BUILD edit was made.

### New-partition needs (recorded, not created)

- A real identity-token partition (e.g. `core_auth:identity_tokens`) owning an
  RS256 signer, signing-key provider, admin issuer URL and 5m expiry would be
  required to emit a genuine JWT. No such partition exists; the need is
  recorded here and the signer is not fabricated locally.
- A real User+roles relation loader and admin-token verification live in the
  shared auth/store layer; if added, the identity source in `execute.cppm`
  should be switched from header/principal fallback to that abstraction.

### Residual gaps

As listed in **Diff** above: unsigned serde claims (no JWT), presence-based
token→Administrator mapping (no real token verification / `updateUserLastSeen`),
no `User`/`roles` relation load, and bare `/identities` mount. Verdict remains
`PARTIAL`; it is flipped only when these are fully achieved.

### Files changed

`include/blog/handlers/auth/identities/routes.cppm`,
`include/blog/handlers/auth/identities/browse/handler.cppm`,
`include/blog/handlers/auth/identities/browse/workflow/step/execute.cppm`,
and both parity docs. The flow file
(`browse/workflow/flow/main.cppm`) is unchanged. No shared file modified.
Bazel verification is blocked by the pre-existing `blog_handlers` ↔
`blog_handlers_routes` dependency cycle (explicitly out of scope); code was
reviewed against the established patterns in `admin/labels` and
`admin/notifications`.

## Module-level notes

Single-route module. Only handler is `identities::browse::BrowseHandler`, exposed both as `identities_browse()` and `identities_read()` (`all.cppm:582-583`); no other Ghost op in this controller. The `s_ctx` pointer is assigned in the handler constructor (`handler.cppm:12`) but never read by the step — no resource-store access, so no `ctx.store()` calls to compare. No `core::cache::Cache` usage in this module. Shared C++ middleware (`require_ghost_permission`, `require_auth`, bearer, serde, error) is documented at `include/blog/handlers/middleware/*.cppm`. Shared Ghost pipeline/serializer/permission/cache machinery: `packages/api-framework/src/{http,pipeline,headers}.ts`, `packages/api-framework/src/serializers/handle.ts`, `ghost/core/server/api/endpoints/utils/serializers/output/{default,all}.js`, `ghost/core/server/api/endpoints/utils/permissions.js`.
