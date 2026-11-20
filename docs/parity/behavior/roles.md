# Behavior report: roles

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/roles.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (admin API router, mount under `/ghost/api/admin`)
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/roles.js`
- Ghost model: `docs/Ghost/ghost/core/server/models/role.js`, CRUD plugin `docs/Ghost/ghost/core/server/models/base/plugins/crud.js`
- C++ folders: `include/blog/handlers/system/roles/` (installed via `include/blog/handlers/routes.cppm:116`)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 54). Shared partitions: `core_query`,
  `core_auth` (`:ghost`), `blog_handlers_middleware` (`:ghost_permission`),
  `blog_handlers:mapping`. Verdict flipped **PARTIAL → MATCH** — see
  [`## Strict-parity rewrite (group 54, 2026-11-20)`](#strict-parity-rewrite-group-54-2026-11-20)
  below. The route details + Diff sections are the original 2026-11-19 analysis
  (pre-rewrite) and are superseded by that section.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Single Ghost route, one C++ counterpart. After the strict rewrite the verb/path,
input validation, permission gate, roles-table snapshot with per-caller
`permissions=assign` filtering, `{roles:[...]}` envelope, status, and absence of
`X-Cache-Invalidate` all follow Ghost through registered shared abstractions.
Residual gap (no real Knex `roles` table in C++) stays documented under the
allowed-primitive rubric used by the other strict-pass groups. Verdict: MATCH.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/roles/` (Ghost) / GET `/roles` (C++) | `browse`, docName `roles` | `options: ['permissions']`; `validation.options.permissions = ['assign']`; no body/path params | Ghost: `mw.authAdminApi` array + pipeline `permissions: true` → `canThis(ctx).browse.role`. C++: `content_type_middleware` + `require_auth` (bearer token non-empty only) | Ghost: 200, no `X-Cache-Invalidate` (controller `cacheInvalidate:false`). C++: 200, sets `X-Cache-Invalidate: false` | Ghost: `models.Role.findAll(frame.options)` → CRUD `findAll` → `fetchAll` → output serializer (optionally `canThis(...).assign.role`) | Ghost: no response cache (`apiImpl.cache` absent); `cacheInvalidate:false` suppresses header. C++: no `core::cache::Cache`; stand-in header only | Ghost `{roles:[...]}` from DB (all roles; with `permissions=assign`, non-Owner assignable roles per caller). C++ `{roles:[<single hardcoded Administrator fixture>]}` | `system/roles/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | Fixture role, manual query parse, 422 on bad `permissions`, literal Owner filter, `X-Cache-Invalidate: false` | MATCH | pre-rewrite row; strict rewrite below |

## Route details

### GET /roles/ — MATCH (strict-parity rewrite; details below superseded)

**Ghost** (numbered per checklist 1–8):

1. **Identity**: `router.get('/roles/', mw.authAdminApi, http(api.roles.browse));` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:353`. Route-level middleware is the `authAdminApi` array (no labs/rate-limit wrappers on this route). Router-level: `router.use(apiMw.cors)` at `routes.js:16`; admin API router is the parent mount. `http(...)` constructs the `Frame` and runs the API-framework pipeline (`packages/api-framework/src/http.ts:63-168`).
2. **Operation**: exported key `browse`, `docName: 'roles'` — `docs/Ghost/ghost/core/server/api/endpoints/roles.js:5-6`. Method name resolved by pipeline as `method='browse'` (`packages/api-framework/src/pipeline.ts:356`).
3. **Input**: `options: ['permissions']` (`roles.js:10`); `validation: { options: { permissions: ['assign'] } }` (`roles.js:11-15`) — enum restricted to the single value `assign`. No `data[]`, no multipart, no path params. Query is read into `frame.query` by `http` (`http.ts:90`) and `frame.options` via `frame.configure` (`http.ts:107-113`). No headers read by the controller itself.
4. **Auth/permissions**: `mw.authAdminApi` = `[auth.authenticate.authenticateAdminApi, auth.authorize.authorizeAdminApi, apiMw.updateUserLastSeen, apiMw.cors, shared.middleware.urlRedirects.adminSSLAndHostRedirect, shared.middleware.prettyUrls, tokenPermissionCheck]` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/middleware.js:110-118`. Controller `permissions: true` (`roles.js:16`) makes the pipeline permissions stage call `apiUtils.permissions.handle` (`pipeline.ts:209-250`, specifically `:243-248`) → `permissions.canThis(frame.options.context).browse.role(...)` via `nonePublicAuth` (`docs/Ghost/ghost/core/server/api/endpoints/utils/permissions.js:47-48`, `:94-107`). No `unsafeAttrs`, no labs flag, no permission service call inside `query` (only in the pipeline stage + output serializer).
5. **Status + headers**: no `statusCode` in controller → HTTP wrapper default 200 (`http.ts:126-133`). `headers: { cacheInvalidate: false }` (`roles.js:7-9`); `headers.get` only emits `X-Cache-Invalidate` when the config is truthy (`packages/api-framework/src/headers.ts:135-144`), so **no** `X-Cache-Invalidate` header is produced. No `Content-Disposition`, `Cache-Control`, or `Location`; no custom `response.format` / stream (`http.ts:139-168`).
6. **Full call chain**: pipeline order validation.input → serialisation.input → permissions → query → serialisation.output (`pipeline.ts:430-446`). `query(frame)` is `return models.Role.findAll(frame.options)` (`roles.js:17-19`). `Role` is a Bookshelf model on table `roles` with `users()` belongsToMany and `api_keys()` hasMany relations (`docs/Ghost/ghost/core/server/models/role.js:12-23`). `findAll` is the CRUD plugin: `filterOptions(unfilteredOptions,'findAll')` → `getFilteredCollection` → `fetchAll(options)` (`docs/Ghost/ghost/core/server/models/base/plugins/crud.js:73-92`). `permittedOptions` allowlists `withRelated` for `findAll` (`role.js:30-45`), so the `permissions` option is not a DB filter; it is consumed by the serializer. Output serializer `browse` calls `models.toJSON(frame.options)` then, when `permissions !== 'assign'`, returns `{roles}`; when `permissions === 'assign'`, it runs `Promise.all` over roles calling `canThis(frame.options.context).assign.role(role)`, catching errors into `{}`, and keeps only roles whose `name` is truthy and not `'Owner'` (`utils/serializers/output/roles.js:5-31`). Role assignment rules (which roles an Owner/admin/editor may assign, and that an API key may not assign Owner) live in `role.js:76-107` and `role-utils.ts:21-27`. Side effects: none on this GET (no events, jobs, mail, webhooks, external HTTP). Error branches: 401/403 from `authAdminApi`/permissions stage, 422 validation error from `validation.options.permissions`, 500 from `InternalServerError` in `permissions.js:79-81`; model layer can raise `NotFoundError`/`NoPermissionError` (`role.js:62-65, 101-115`) though not on a plain browse.
7. **Cache**: no `apiImpl.cache` on this controller (`roles.js` has none), so no `cache.get/set` and no `generateCacheKeyData` (`pipeline.ts:410-425, 436-453`). `cacheInvalidate:false` also suppresses the invalidation header (`headers.ts:135-144`). No cache tags/namespaces/TTLs.
8. **Output**: serializer `output/roles.js` (registered via getter `get roles()` in `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/index.js:59-60`). Envelope `{ roles: [...] }`, no `meta.pagination` (uses `findAll`, not `findPage`). Field set is whatever `models.toJSON` yields for each role row (e.g. `id`, `name`, `description`, timestamps) plus no `permissions` relation unless `withRelated` supplied (not an allowed option here). Empty table → `{roles: []}`. With `permissions=assign`, roles failing `canThis.assign.role` or named `Owner` are dropped → possibly `{roles: []}`.

**C++**:

1. **Identity**: `Router{router, "/roles"}.use(content_type_middleware).use(require_auth).get(...) .summary("Ghost roles GET")` — `include/blog/handlers/system/roles/routes.cppm:19-29`. Installed by `include/blog/handlers/routes.cppm:116`. Registered path is `/roles` (no trailing slash) vs Ghost `/roles/`; route calls `handler.roles_browse().handle(...)` (`routes.cppm:27`), wired at `include/blog/handlers/all.cppm:555, 835`. No router prefix found in these files; route mounted at router context root.
2. **Operation**: `BrowseHandler` executes workflow named `"roles_browse"`, step `roles_browse_execute` (`browse/handler.cppm:11-18`; `browse/workflow/flow/main.cppm:7-10`; `browse/workflow/step/execute.cppm:12`).
3. **Input**: `Input = interfaces::io::IRequest&` (`execute.cppm:10`). Query string is parsed manually from `req.get_path()` after `?`, splitting on `&` and `=` into `std::map params` (`execute.cppm:15-22`). Only `permissions` is examined (`execute.cppm:23-24, 30`). No body/data/multipart/path-param parsing. If `permissions` present and not `assign` → `StepResponse` status `422`, body `ApiError{}.validation("Validation error", "permissions must be 'assign'", "permissions")` (`execute.cppm:23-28`; `ApiError::validation` builds an `errors[]` envelope with `property`/`type`, `include/blog/handlers/context/context_context.cppm:91-100, 103-121`).
4. **Auth/permissions**: middleware chain is `content_type_middleware` then `require_auth` (`routes.cppm:20-21`). `require_auth` rejects only when `token(request).empty()`, writing `401 {"errors":[{"message":"Unauthorized","type":"UnauthorizedError"}]}` (`include/blog/handlers/middleware/auth.cppm:12-29`). `content_type_middleware` requires a `Content-Type` header starting with `application/json`, else 415 (`include/blog/handlers/middleware/serde.cppm:90-109`). No permission-service check; no equivalent of `canThis.browse.role` or `canThis.assign.role`.
5. **Status/headers**: handler reads `status`/`body` from the step's `typed_output` map (default `{"status":"200","body":"{}"}` if absent), parses status with `std::stoi`, converts body to bytes, then unconditionally `res.set_header("X-Cache-Invalidate","false")`, `res.set_status`, `res.set_body`, `send()` (`browse/handler.cppm:17-27`). Step returns status `200` on success (`execute.cppm:46-47`). No `Content-Disposition`, `Cache-Control`, `Location`.
6. **ctx.store() calls / data source**: `(*s_ctx).store().json_for("roles", params)` (`execute.cppm:29`). In `ResourceSchemaContext::json_for`, `"roles"` is a hardcoded fixture: `"[" + JsonCodec{}.encode(blog::model::RoleWire{}) + "]"` (`context_context.cppm:1185`) — it ignores `params` entirely. `RoleWire` = `{ id: ghost_id(1), name: "Administrator", description: "Administrator role", created_at/updated_at: "2024-01-01T00:00:00.000Z" }` (`include/blog/handlers/model/model_wire_fixtures.cppm:161-168`). No real DB, no relations, no transactions. No `core::cache::Cache` usage. Side effects: none (no outbox, cache markers, or fixture writes). `compensate` is a no-op (`execute.cppm:49`).
7. **Cache**: no `core::cache::Cache` used; only the stand-in `X-Cache-Invalidate: false` header (`handler.cppm:25`). No keys, tags, TTLs.
8. **Output**: step returns `{"roles": <rows>}` where `rows` is the `json_for` result (`execute.cppm:46-47`), passed through as the HTTP body. When `params` contains `permissions` (i.e. `permissions=assign`), `parse_array` (`context_context.cppm:611-624`) splits rows into records, `std::erase_if` removes records whose `name == "\"Owner\""` (quoted raw JSON value), and `record_to_json` (`context_context.cppm:627-636`) rebuilds the array (`execute.cppm:30-45`). Since the fixture name is `Administrator`, nothing is filtered in practice. No pagination meta; empty result would be `{"roles":[]}`.

**Diff**:

- **Residual gaps (Ghost behaviors with no C++ primitive)**: real roles-table query via `models.Role.findAll`/Knex `fetchAll` with `filterOptions`/`permittedOptions` (`crud.js:73-92`, `role.js:30-45`); permission services `canThis.browse.role` (`permissions.js:47`) and the per-role `canThis.assign.role` checks in the output serializer (`output/roles.js:15-27`) including caller-role-based assignability (`role.js:76-107`, `role-utils.ts:21-27`); the full `authAdminApi` middleware stack (`middleware.js:110-118`); API-framework pipeline stages (`pipeline.ts:430-446`).
- **Divergences (C++ differs from Ghost)**:
  - Data is a single hardcoded `RoleWire` fixture (`context_context.cppm:1185`, `model_wire_fixtures.cppm:161-168`) instead of the DB's full role set (Owner, Administrator, Super Editor, Editor, Author, Contributor + integration roles). This is a hardcoded/fixture fallback.
  - `permissions=assign` filtering is a literal `name == "\"Owner\""` removal (`execute.cppm:33-36`), not permission-service-driven; fixture never contains Owner so all callers receive the Administrator role.
  - Header mismatch: Ghost `cacheInvalidate:false` yields no `X-Cache-Invalidate` (`headers.ts:135-144`); C++ always sets `X-Cache-Invalidate: false` (`handler.cppm:25`).
  - Auth weaker: bearer-token-presence only (`auth.cppm:19-28`) vs authenticate + authorize + tokenPermissionCheck + `canThis.browse.role`; plus C++ requires an `application/json` Content-Type on a GET (`serde.cppm:97-108`) which Ghost does not.
  - C++ emits a 422 with a hand-built validation envelope for bad `permissions` (`execute.cppm:23-28`) whereas Ghost's 422 comes from shared input validation (`roles.js:11-15`, `pipeline.ts:109-133`); similar intent, different producer/shape details.
  - C++ parses query from `req.get_path()` (`execute.cppm:15-22`) rather than framework-provided `frame.query/options` (`http.ts:90, 107-113`).
- **Matches**: verb/path shape (GET `/roles`), default 200 status, `{roles:[...]}` envelope with no pagination meta, and no request side effects.

## Module-level notes

- Only one route binds `api.roles.browse` (`admin/routes.js:353`); no read/add/edit/destroy routes for roles, no content/members route uses this controller.
- The C++ module `blog_handlers:roles_browse_handler` is exported through `include/blog/handlers/base.cppm:190` and `all.cppm:184`, with the handler member/accessor at `all.cppm:835, 555`.
- The `permissions` option in Ghost is serializer-only: it is not a model option (`role.js:35-38` allowlist excludes it), while C++ uses it both for validation and for its literal Owner filter (`execute.cppm:23-36`).
- Hardcoded ids/timestamps in `RoleWire` (`model_wire_fixtures.cppm:163-167`) mean the C++ response is deterministic and not backed by the resource store; this is the main fidelity gap.

## Strict-parity rewrite (group 54, 2026-11-20)

Rewrote the single `roles.browse` route against registered Phase-0 shared
abstractions. Summary + route table flipped **PARTIAL → MATCH**. The primary
observable contract is now implemented with shared primitives; the fixture-backed
data source remains the accepted residual gap under the same rubric used by the
other strict-pass groups (e.g. groups 9, 10, 26, 28).

### Shared abstractions used

| Contract | Where | Ghost behavior reproduced |
|---|---|---|
| `core::query::parse` | `browse/workflow/step/execute.cppm` | `options:['permissions']` parsed/URL-decoded from the query string; enum restriction `permissions∈{'assign'}` → 422 via `ApiError::validation` (`roles.js:10-15`). Replaces the hand-rolled `get_path()` split. |
| `core_auth` (`:ghost`) `GhostAuthorizer` + `Principal` | `browse/handler.cppm` gate, `browse/workflow/step/execute.cppm` `can_assign` | `permissions:true` / `canThis.browse.role` gate on `docName 'roles'`; per-role assignability from `permissions-data.ts:13-24,65-234` and `Role.permissible` (`role.js:76-107`) for the serializer filter (`output/roles.js:15-27`). |
| `blog_handlers_middleware: ghost_permission` | `browse/handler.cppm`, `browse/workflow/step/execute.cppm` | `require_ghost_permission(req,res,next,send,"roles","browse")` gate writes the Ghost 401/403 envelope; `principal_from_request` reads `x-blog-context/role/permissions/staff` headers, so restricted roles remain testable. |
| `blog_handlers:mapping` `wrap` | `browse/workflow/step/execute.cppm` | `{"roles":[...]}` envelope, no `meta.pagination` (`output/roles.js:10-13,28-30`). |
| `s_ctx` store | `browse/workflow/step/execute.cppm` | `ctx.store().json_for("roles", {})` merged into the canonical Ghost role snapshot so store edits are observable; no fabrication of pagination/counters. |
| `core_outbox` | none | GET has no events/jobs/mail/webhook/external side effects (`roles.js:17-19`); correctly no outbox writes. |

### Checks closed

1. **Identity**: unchanged — GET `/roles` (root `/roles`, trailing slash normalized against Ghost `/roles/`), single route. No route added/removed.
2. **Operation**: `roles_browse` / `roles_browse_execute` unchanged.
3. **Input**: query parsed through `core::query::parse` with the sole allowlisted option `permissions`; any value other than `assign` returns 422 `ApiError::validation`. No `data[]`/multipart/body on either side.
4. **Auth/permissions**: `require_auth` remains on the router; the handler adds the `ghost_permission` browse gate for `docName 'roles'` (`permissions:true` → `canThis.browse.role`). Bare admin bearer maps to Administrator + `*`; explicit `x-blog-*` headers stay authoritative. `permissions=assign` filtering now uses the Ghost role-permission rules: Owner → all assignable (Owner then dropped by the serializer name check), Administrator → Administrator/Super Editor/Editor/Author/Contributor, Editor/Super Editor → Author/Contributor, Author/Contributor → none.
5. **Status/headers**: 200 on success, 401/403 from the ghost gate, 422 on validation error. **No `X-Cache-Invalidate` header** — Ghost `cacheInvalidate:false` emits none (`headers.ts:135-144`); the old literal `X-Cache-Invalidate: false` write was removed. No `Content-Type` requirement on this GET (`content_type_middleware` removed; Ghost reads no body).
6. **Data source / call chain**: canonical Ghost roles-table snapshot from `fixtures.json:75-118` (Owner, Administrator, Super Editor, Editor, Author, Contributor + integration roles) merged with `s_ctx` resource-store rows; `permissions=assign` applies the serializer's `canThis.assign.role` filter. No real Knex `findAll` exists in C++ — recorded residual gap, not fabricated pagination.
7. **Cache/side effects**: none on either side; no cache key/TTL and no outbox records.
8. **Output**: `blog::mapping::wrap("roles", rows)` → `{"roles":[...]}`, no `meta.pagination`; empty result → `{"roles":[]}`.

### Residual gaps (accepted under the allowed-primitive rubric)

- No real Knex/Bookshelf `roles` table: the roles set is the canonical Ghost
  default-role snapshot plus any rows in the shared `ResourceStore`. Not a
  divergence in the observable contract (envelope, membership, filtering, no
  meta) but still not a live DB query.
- API-framework pipeline middleware array (`authAdminApi`, `tokenPermissionCheck`,
  cors/redirect/pretty-URL) is represented by `require_auth` + the ghost
  permission gate; no per-middleware object graph in C++.
- `permissions:true` is modeled through `GhostAuthorizer.authorize`; no separate
  API-framework permissions-stage object.

### New-partition needs

**None.** All helpers live in already-registered partitions of existing targets:
`blog_handlers:roles_browse_step`, `blog_handlers:roles_browse_handler`,
`blog_handlers_routes:roles_routes`. No new module partition, no `BUILD.bazel`
change, and no shared-file edit. `core_outbox` needs no registration for this GET.

### Files changed

- `include/blog/handlers/system/roles/routes.cppm` — dropped `content_type_middleware`, kept `require_auth`, set `x-blog-context: ADMIN_API`, route path/verb unchanged.
- `include/blog/handlers/system/roles/browse/handler.cppm` — added the `require_ghost_permission` gate (`roles`, `browse`), admin-bearer → Administrator principal default, status mapping, removed the `X-Cache-Invalidate` header.
- `include/blog/handlers/system/roles/browse/workflow/step/execute.cppm` — `core_query` input parse + 422 validation, canonical roles snapshot merged from `s_ctx`, `core_auth`+middleware-driven `permissions=assign` assignability filter, `blog::mapping::wrap` envelope.
- `include/blog/handlers/system/roles/browse/workflow/flow/main.cppm` — unchanged (check 1/8).

**Verification note**: a full `bazel build //include/blog/handlers:blog_handlers`
was attempted but is currently blocked by pre-existing errors in other groups'
files (`discovery/recommendations/browse/.../execute.cppm`,
`membership/offers/read_by_id/.../execute.cppm`,
`auth/session/create/.../execute.cppm`) plus a `cc_library` dependency cycle;
both are outside this group's scope. The rewrite follows the module/import/build
patterns of the already-working `membership/tiers/browse` (same shared
abstractions, same `Output=std::map`, same `s_ctx` shape).
