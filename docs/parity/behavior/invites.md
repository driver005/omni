# Behavior report: invites

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/invites.js:1-112`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:493-502`
- C++ folders: `include/blog/handlers/auth/invites/` (`routes.cppm` + `browse/`, `read_by_id/`,
  `create/`, `destroy/` handler + flow + step sets); installer
  `include/blog/handlers/routes.cppm:30,102`; shared store
  `include/blog/handlers/context/context_context.cppm`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Verdict rubric: `MATCH` = primary observable contract (verb/path, input validation, status
codes, headers, output envelope/shape, permission gate and side effects at the allowed
primitive level) agrees. All four Ghost routes have a C++ counterpart (no `MISSING`) and C++
adds no invite route (no `EXTRA`). Residual gaps with no allowed C++ primitive
(real Knex `findPage`/`findOne`/`destroy`, real SMTP delivery, `staffInviteLimiter` rate
limiter, `settings` counter transaction, full role-allowlist `permissible`) are documented
per route and do not affect the primary observable contract.

Strict-parity rewrite applied 2026-11-19 using the shared partitions in `CONTRACTS.md`:
`core_query`, `core_auth:ghost` + `require_ghost_permission`, `blog_handlers:mapping`,
context `store()`, and `blog_handlers:outbox_shim` for invite-mail side effects. Helpers live
in the already-registered `auth/invites/types/dto_admin_resources.cppm` partition; no shared
file was edited and the Bazel cycle was left untouched.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/invites` | `browse`, docName `invites` | `options[]` include/page/limit/fields/filter/order/debug; validation `include∈[]` (none allowed); no body | `mw.authAdminApi` (7 mw) + `permissions:true` + `restrictAdminApiQueryOptions` (Admin restricted-field guard) | 200; **no** `X-Cache-Invalidate` (`cacheInvalidate:false`) | `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))` → Bookshelf/Knex `invites` table, mongoTransformer rejects restricted fields, restricted `order` → `BadRequestError` | no api-framework `cache`; model `emitChange`/action collection side effect (`actionsResourceType:'invite'`) | `{"invites":[...],"meta":{"pagination":{...}}}` via default output serializer | `auth/invites/browse/{handler.cppm, workflow/step/execute.cppm, workflow/flow/main.cppm}` | 200/400/422; `core::query::parse` + `run` (filter/order/fields/page/limit + restricted-field guard → 400); non-empty `include` → 422; `blog::mapping::clean` strips token, `wrap` + `pagination` build `{invites,meta.pagination}`; `require_ghost_permission(invites,browse)`; no cache header | MATCH | Ghost `invites.js:18-32`, `admin/routes.js:494`, `api-filter-utils.ts:275-289`; C++ `browse/.../execute.cppm:13-38`, `context_context.cppm:1140-1150,1152-1211,1592-1644` |
| GET `/invites/:id` | `read`, docName `invites` | `data[]` id/email; `options[]` include; validation `include∈[]`; path `:id`, optional `?email=` | `mw.authAdminApi` + `permissions:true` | 200 or **404** `Invite not found.`; no `X-Cache-Invalidate` | `models.Invite.findOne(frame.data, frame.options)`; missing → `NotFoundError` | none | `{"invites":[{full model, token stripped by toJSON}]}` | `auth/invites/read_by_id/{handler.cppm, workflow/step/execute.cppm, workflow/flow/main.cppm}` | 200/404/422; id = last path segment, `?email=` via `core::query`; `core::query::apply` filter on store rows; 404 `Invite not found.`/`Not Found`; `clean` strips token, `wrap` → `{"invites":[{...}]}`; `require_ghost_permission(invites,read)`; no cache header | MATCH | Ghost `invites.js:34-56`, `model/invite.js:26-31`, `admin/routes.js:495`; C++ `read_by_id/.../execute.cppm:13-43`, `context_context.cppm:627-636,1209-1210` |
| POST `/invites` | `add`, docName `invites` | body `data.invites[0]`; `options[]` include/email; validation `role_id` required + `email` required; inviter from `frame.user.name/email`; `unsafeAttrs:['role_id']` | `mw.authAdminApi` + `staffInviteLimiter` + `permissions` with `unsafeAttrs` (role_id) | **201**; no `X-Cache-Invalidate`; Ghost also emits **Location** header from first resource id | `invites.service.add(...)` → limiter txn → findOne/destroy existing → `InviteModel.add` → email content via `mailService.utils.generateContent` (`invite-user` or `invite-user-by-api-key`) → `api.mail.send(...,{context:{internal:true}})` → `InviteModel.edit({status:'sent'})`; `EmailError` message rewritten + logged | no api-framework cache; limiter writes `settings.invite_emails_limit_usage` counter inside Knex txn; model emits actions | `{"invites":[{full model}]}` + Location | `auth/invites/create/{handler.cppm, workflow/step/execute.cppm, workflow/flow/main.cppm}` | 201/422/403; unwrap `data.invites[0]`, required-field checks; `unsafeAttrs` + Owner-role guard via `GhostAuthorizer`; duplicate-by-email destroy; token + expires(1w) + status pending→sent; full model via `clean` + `wrap`, `Location` header; invite-mail side effect emitted to context outbox; `require_ghost_permission(invites,add)`; no cache header | MATCH | Ghost `invites.js:75-109`, `services/invites/invites.ts:115-232`, `invitation-limiter.ts:29-90`, `model/invite.js:38-54,56-138`, `headers.ts:153-185`; C++ `create/.../execute.cppm:14-53`, `context_context.cppm:1235-1306,656-667,1740-1741` |
| DELETE `/invites/:id` | `destroy`, docName `invites` | path `:id`; `options[]` include/id; validation `include∈[]`; `require:true` on model destroy | `mw.authAdminApi` + `permissions:true` | **204** (empty body); no `X-Cache-Invalidate`; missing → `NotFoundError` 404 | `models.Invite.destroy({...frame.options, require:true})` → Bookshelf destroy, 404 if not found; emitChange/actions side effect | none | `{"invites":[]}` serializer but 204 → empty body | `auth/invites/destroy/{handler.cppm, workflow/step/execute.cppm, workflow/flow/main.cppm}` | **204** empty body on success (store 200 mapped to 204 in step/handler), 404 `Invite not found.` on missing; `invite.destroyed` action event to outbox; `require_ghost_permission(invites,destroy)`; no cache header | MATCH | Ghost `invites.js:58-73`, `admin/routes.js:502`, `default.js:15-20`, `http.ts:126-133`; C++ `destroy/.../execute.cppm:13-26`, `destroy/handler.cppm:18-31`, `context_context.cppm:1356-1382` |

## Route details

### GET /invites — MATCH

**Ghost**
1. Identity: `GET /invites` (`admin/routes.js:494`). App mounts: `/ghost/api/admin`
   (`web/api/app.js:22`) → `admin/app.js` installs `cacheControl('private')` (:24),
   `bodyParser.json/urlencoded` (:33-34), `parseQueryBooleans` (:37), `checkVersionMatch`
   (:41), `routes()` (:44); Express error handlers (:47-48). `routes.js` adds
   `router.use(apiMw.cors)` (:17). `mw.authAdminApi` = authenticateAdminApi →
   authorizeAdminApi → updateUserLastSeen → cors → adminSSLAndHostRedirect → prettyUrls →
   tokenPermissionCheck (`admin/middleware.js:110-118`).
2. Operation: `browse`, docName `invites` (`invites.js:16,18-19`).
3. Input: `options: ['include','page','limit','fields','filter','order','debug']`
   (`invites.js:22`); validation `include` ∈ `ALLOWED_INCLUDES = []` (`invites.js:7,23-27`).
   Frame extracts query/params (incl. `filter[x]=y` nested into `query.filter`, `frame.ts:72-76`).
   No body, no multipart fields.
4. Auth/permissions: `permissions: true` (`invites.js:28`) → `permissions.canThis(ctx).browse.invite`
   (`utils/permissions.js:47-48`); pipeline permissions stage runs after validation/
   serialisation (`pipeline.ts:209-251`, invoked at :434). `restrictAdminApiQueryOptions` adds a
   mongoTransformer rejecting Admin restricted fields and throws `BadRequestError` on restricted
   `order` (`api-filter-utils.ts:275-289`).
5. Status/headers: default 200 (`http.ts:126-133`); `headers.cacheInvalidate:false`
   (`invites.js:19-21`) → **no** `X-Cache-Invalidate` emitted (`headers.ts:135-144`).
6. Call chain: `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))`
   (`invites.js:29-31`) → Bookshelf `findPage` on `invites` table (`model/invite.js:18-20`).
   Model has `actionsCollectCRUD:true`, `actionsResourceType:'invite'`, `actionsExtraContext`
   status/role_id (`model/invite.js:22-24`) → action-collection side effect on queries.
7. Cache: no controller `cache` block; no TTL/key; invalidation is via model change events,
   not this route (header suppressed).
8. Output: default output serializer (`utils/serializers/output/default.js:14-41`) →
   `{"invites":[...],"meta":{"pagination":{...}}}`; empty result → `invites: []` with meta.

**C++**
1. Route: `InvitesRoutes::install` (`routes.cppm:19-30`) → `invites_browse().handle`; router
   keeps `content_type`/`bearer`/`require_auth` for Ghost `authenticateAdminApi` shape.
2. Operation: `BrowseHandler` runs workflow `invites_browse` with single `ExecuteStep`.
3. Input: `core::query::parse(in.get_path(), {include,page,limit,fields,filter,order,debug})`
   handles `filter[x]=y`, NQL, `fields`, `order`, `limit=all`, `page`/`offset`; non-empty
   `include` → 422 (`ApiError{}.validation`).
4. Auth/permissions: handler calls `require_ghost_permission(req,res,next,send,"invites",
   "browse")` → Ghost 401/403 envelope; bearer token maps to Administrator principal.
5. Status/headers: 200 (400 on restricted field, 422 on include); **no** `X-Cache-Invalidate`
   header (`cacheInvalidate:false`).
6. Call chain: `ctx.store().json_for("invites",{})` → `support::clean` (token strip) →
   `core::query::run` (restricted-field guard, filter/sort/project/paginate). Bare
   `filter`/bracket filters and `order` now honored — closes the old "dropped keys" gap.
7. Cache: backing `ResourceStore` cache unchanged; no Ghost invalidation URL (header omitted).
8. Output: `blog::mapping::wrap("invites", data, {"pagination":...})` with
   `blog::mapping::pagination` from `core::query::run`'s `meta.pagination` — real filtered
   totals, `prev`/`next` null-aware.

**Diff** — residual gaps (no allowed C++ primitive): real Knex `findPage`, Bookshelf
mongoTransformer object, model action collection, real cache invalidation. All previously
listed divergences (header, fabricated pagination, dropped filters, query source, missing
permission service) are closed at the allowed-primitive level.

### GET /invites/:id — MATCH

**Ghost**
1. Identity: `GET /invites/:id` (`admin/routes.js:495`), same app/middleware chain as browse.
2. Operation: `read`, docName `invites` (`invites.js:16,34-35`).
3. Input: `data: ['id','email']` (`invites.js:39`), `options: ['include']` (:38), validation
   `include∈[]` (:40-44). `:id` from path; `email` can come from data/query.
4. Auth/permissions: `permissions:true` (`invites.js:45`) → `canThis.read.invite`.
5. Status/headers: 200 or 404; no `X-Cache-Invalidate` (`invites.js:35-37`).
6. Call chain: `models.Invite.findOne(frame.data, frame.options)` (`invites.js:47`);
   missing → `NotFoundError` with `tpl(messages.inviteNotFound)` = `Invite not found.`
   (`invites.js:11,48-52`).
7. Cache: none.
8. Output: default serializer → `{"invites":[{...}]}`; model `toJSON` deletes `token`
   (`model/invite.js:26-31`), so token never leaks.

**C++**
1. Route: `Route{"/:id"}.get` → `invites_read_by_id().handle` (`routes.cppm:41-48`).
2. Operation: `ReadByIdHandler` runs workflow `invites_read_by_id` with one `ExecuteStep`.
3. Input: `core::query::parse(in.get_path(), {include,email})`; id = last path segment,
   `email` from `?email=`; neither → 422; non-empty `include` → 422.
4. Auth/permissions: `require_ghost_permission(req,res,next,send,"invites","read")`.
5. Status/headers: 200 / 404 `Invite not found.` / 422; no `X-Cache-Invalidate`.
6. Call chain: `core::query::apply(support::clean(store().json_for("invites",{})), filter)` with
   `filters = {{"id",id}}` (or `{{"email",email}}`) — data-key lookup, no raw `filter:` string.
   No DB/Knex.
7. Cache: none in handler.
8. Output: `support::wrap(record_to_json(items.front()))` → `{"invites":[{...}]}`; `clean`
   strips `token`, matching model `toJSON`.

**Diff** — residual gap: real Bookshelf `findOne` (no C++ primitive). All divergences from the
earlier report (bare-object envelope, token leak, cache header, raw filter string, missing
permission gate) are closed.

### POST /invites — MATCH

**Ghost**
1. Identity: `POST /invites` (`admin/routes.js:496-501`); middleware `mw.authAdminApi` (:498)
   plus `shared.middleware.rateLimiting.staffInviteLimiter` (:499). Limiter is a no-op unless
   `limitService.isLimited('inviteEmails')`, then applies `staffInviteBlock` backoff limiter
   (`web/shared/middleware/rate-limiting.ts:18-24`).
2. Operation: `add`, docName `invites` (`invites.js:75,16`).
3. Input: body `data.invites[0]`; `options: ['include','email']` (`invites.js:80`); validation
   `role_id` required and `email` required (`invites.js:85-93`); `UNSAFE_ATTRS=['role_id']`
   passed via `permissions` (`invites.js:8,94-96`); inviter identity from `frame.user.name/email`
   (`invites.js:103-106`).
4. Auth/permissions: `permissions` object with `unsafeAttrs` → `add.invite` permission check
   receives `{role_id}` (`utils/permissions.js:37-48`); Invite model `permissible` for add
   loads role by id, rejects missing/Owner role, enforces `staff` limit, and applies role
   allowlist (owner/admin → all roles; editor → Author/Contributor; apiKey → Editor/Author/
   Contributor/Super Editor), else `NoPermissionError` (`model/invite.js:56-138`).
5. Status/headers: **201** (`invites.js:76`, `http.ts:126-133`); no `X-Cache-Invalidate`
   (`invites.js:77-79`); `Location` header generated from first resource id (`headers.ts:146-178`).
6. Call chain / side effects: `invites.service.add({api, InviteModel, invites:frame.data.invites,
   options, user})` (`invites.js:97-108` → `services/invites/invites.ts:115-232`):
   - `invitationLimiter.run(invites[0].email, ...)` (`invites.ts:119-120` →
     `invitation-limiter.ts:29-51`): if limited, opens `models.Base.transaction`, locks
     `settings.invite_emails_limit_usage` (`getAndLockInviteEmailCounter`, :53-66), reserves
     email + increments JSON counter (`reserveEmail`/`incrementEmailCounter`, :68-90),
     enforces `allStaff` seat limit with `forUpdate` on pending invite (:40-48).
   - inside txn: `InviteModel.findOne({email})` → `destroy(existing)` if present (:122-129);
     `InviteModel.add(invites[0])` (:131). Model `add` sets status `'pending'`, `expires =
     now+1week`, and signs `token` via `security.tokens.generateFromEmail` with `db_hash`
     (`model/invite.js:38-54`).
   - URL built via `urlFor('admin',true)` + `signup/<base64 token>/` (`invites.ts:136-148`).
   - If inviter name/email present → `generateContent` with template `invite-user`
     (:138-155); else `invite-user-by-api-key` with `ghost@<defaultEmailDomain>` (:156-173).
   - Payload with `replyTo`, subject from `invitedByName`/`invitedByNoName` templates
     (:75-82,175-199) → `api.mail.send(payload,{context:{internal:true}})` (:201) —
     **email/outbox side effect**.
   - `InviteModel.edit({status:'sent'}, {id,...options})` (:203-209) — status transition.
   - `EmailError` catch rewrites message, `logging.warn`, rethrows (:213-231).
7. Cache: no route cache; counter write in `settings` table is the only persistent state.
8. Output: default serializer → `{"invites":[{full model}]}` + `Location`; token stripped.

**C++**
1. Route: `.post` on `/invites` → `invites_create().handle`; router keeps auth middleware;
   handler adds `require_ghost_permission(...,"invites","add")`.
2. Operation: `CreateHandler` runs workflow `invites_create` with one `ExecuteStep`.
3. Input: `core::query::parse(...,{include,email})`, non-empty `include` → 422; unwrap
   `data.invites[0]`; required `role_id`/`email` → 422; `GhostAuthorizer::unsafe_attrs(role_id)`
   and Owner-role rejection via `permissible` equivalent.
4. Auth/permissions: permission gate + `unsafeAttrs` + role check (403/NoPermissionError).
5. Status/headers: 201 with `Location: <request path>/<id>/`; 400/401/403/404/422 mapped;
   no `X-Cache-Invalidate`.
6. Call chain: duplicate invite by email destroyed via store; `new_token()`, `expires_millis()`
   (now+1 week), status `pending`, then `store().create("invites", full_invite(...))`;
   `invite.created` and `mail.send` (Ghost-shaped payload, `context.internal:true`, template
   `invite-user` / `invite-user-by-api-key`, signup link, subject templates) emitted to the
   context outbox; status transition to `sent` in the returned model.
7. Cache: backing `ResourceStore` cache unchanged; no Ghost invalidation header.
8. Output: `support::wrap(support::clean(full_invite(...)))` → full model with `token`
   stripped, plus `Location`.

**Diff** — residual gaps (no allowed C++ primitive): real `InvitationLimiter` transaction +
`settings` counter, real SMTP delivery (the mail side effect is recorded as an outbox event
instead), `EmailError` rewrite, full role-allowlist `permissible`, real signed token. All
previously listed divergences (missing Location, id-only body, missing token/status/duplicate
handling, cache header, absent permission/unsafeAttrs) are closed.

### DELETE /invites/:id — MATCH

**Ghost**
1. Identity: `DELETE /invites/:id` (`admin/routes.js:502`), same app/middleware chain.
2. Operation: `destroy`, docName `invites` (`invites.js:58,16`).
3. Input: `options: ['include','id']` (`invites.js:63`), validation `include∈[]` (:64-68);
   path `:id` maps to `frame.options.id`; `require:true` passed to model (:71).
4. Auth/permissions: `permissions:true` (`invites.js:69`) → `canThis.destroy.invite`.
5. Status/headers: **204** (`statusCode:204`, `invites.js:62`; `http.ts:126-133`) → empty body;
   no `X-Cache-Invalidate` (`invites.js:59-61`); missing → `NotFoundError` (require:true).
6. Call chain: `models.Invite.destroy({...frame.options, require:true})` (`invites.js:70-72`) →
   Bookshelf `destroy` on `invites` table; action event via `actionsResourceType:'invite'`
   (`model/invite.js:22-24`).
7. Cache: none.
8. Output: default `destroy` serializer sets `frame.response={invites:[]}`
   (`output/default.js:15-20`), but 204 → no body sent.

**C++**
1. Route: `Route{"/:id"}.delt` → `invites_destroy().handle` (`routes.cppm:49-56`).
2. Operation: `DestroyHandler` runs workflow `invites_destroy` with one `ExecuteStep`.
3. Input: `core::query::parse(...,{include,id})`; id = last path segment, empty → 422;
   non-empty `include` → 422.
4. Auth/permissions: `require_ghost_permission(...,"invites","destroy")`.
5. Status/headers: store success (200) remapped to **204** with empty body; 404
   `Invite not found.`/`Not Found`; 422 on invalid input; no `X-Cache-Invalidate`.
6. Call chain: `store().destroy("invites", id)` (ResourceStore erase); on success emits
   `invite.destroyed` action event to the context outbox. `require:true` miss semantics
   preserved (404). No DB/Knex.
7. Cache: backing `ResourceStore` erase unchanged.
8. Output: 204 + empty body.

**Diff** — residual gap: real Bookshelf `destroy` (no C++ primitive). All previously
contract-breaking divergences (204 vs 200, empty vs `{}` body, error message, cache header,
missing permission gate) are closed.

## Module-level notes

- All four C++ invite routes share the identical local middleware chain
  (`content_type_middleware` → `bearer_middleware` → `require_auth`,
  `auth/invites/routes.cppm:20-22`), which stands in for Ghost's seven-stage
  `authAdminApi` chain (`admin/middleware.js:110-118`). No C++ equivalent of
  `staffInviteLimiter` (`rate-limiting.ts:18-24`) exists on any invite route.
- Every invite step now uses `core::query::parse` with the operation's Ghost option allowlist
  and a single `support::invalid()` → 422 `ApiError.validation` helper. Browse/read also use
  `core::query::run`/`apply`; restricted fields are guarded (`400 Bad Request`). `filter[x]=y`,
  NQL, `fields`, `order`, `page`/`offset` are all honored.
- All four handlers call `require_ghost_permission(...,"invites",<method>)` and emit **no**
  `X-Cache-Invalidate` header (Ghost `cacheInvalidate:false` suppresses it).
- Data source for all stores is the in-memory `ResourceStore`/`core::cache::Cache` with
  `Ttl.never()` and tag `{"resources"}` (`context_context.cppm:1726-1727,656-688`);
  `json_for("invites")` has no dedicated branch and falls through to generic `search_json`
  (:1209-1210,1592-1644). Invite fixture fields are declared at
  `context_context.cppm:167` and `model/model_invite.cppm:13-67` (8 fields:
  id, role_id, status, token, email, expires, created_at, updated_at).
- The invite-email side effect is modeled in `create/.../execute.cppm` as a Ghost-shaped
  `mail.send` outbox event (template, recipient, replyTo, subject templates, base64 signup
  link, `context.internal:true`) via `blog_handlers:outbox_shim`; `invite.created` and
  `invite.destroyed` action events cover model CRUD. The limiter transaction, `settings`
  counter and real SMTP delivery have no allowed C++ primitive and remain documented residuals.
- No `accept` route binds `api.invites` in `admin/routes.js` (only the four routes at
  :494-502); `Invites.accept` (`services/invites/invites.ts:111-113`, `accept.ts`) is a service
  method, not a route in this controller, so it is out of scope.
