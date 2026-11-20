# Behavior report: users

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/users.js` (docName `users`, lines 103–290)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 17, 124–136)
- C++ folders: `include/blog/handlers/admin/users/` (routes.cppm + 10 handlers, each with workflow/flow/main.cppm and workflow/step/execute.cppm); supporting `include/blog/handlers/middleware/{auth,serde}.cppm`, `include/blog/handlers/context/context_context.cppm`, `plugins/memory/search/backend.cppm`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 9 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 10 Ghost `users` routes have a C++ path+verb counterpart; no C++ route lacks a Ghost counterpart. Every C++ route sits behind the router-level chain `content_type_middleware` + `require_auth` (`include/blog/handlers/admin/users/routes.cppm`), then an operation-level `blog::handlers::middleware::require_ghost_permission` gate backed by `core::auth::ghost::GhostAuthorizer` (`routes.cppm:23-53`). A valid admin bearer maps to Ghost's Administrator principal by default; explicit `x-blog-*` headers are preserved so restricted roles remain testable. Query parsing/pagination uses `core::query`, output uses `blog::mapping::clean/wrap/pagination`, and domain side effects are recorded on `core::outbox`. See the Group 10 implementation record at the end of this report.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /users | browse | `options[]` include/filter/fields/limit/order/page/debug; `include` allowlist (users.js:111-118) | `mw.authAdminApi` + `permissions:true` (routes.js:125; users.js:119) | 200; `cacheInvalidate:false` (users.js:108-110) | `models.User.findPage(restrictAdminApiQueryOptions(frame.options))` (users.js:120-122) | no cache service; `X-Cache-Invalidate:false` | default serializer `{users:[…],meta.pagination}` + mapper (users.js mapper:11-39) | `admin/users/browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | `core::query` parse over 7 options (`filter/order/fields/limit/page`), `core::query::run` for real filtered pagination, include allowlist 422, restricted-field 400, mapper `url`/`permissions:[]`/`count.posts`, `wrap` envelope, permission gate, `X-Cache-Invalidate:false` | MATCH | Ghost users.js:107-123; C++ browse/execute.cppm:14-38, browse/handler.cppm:18-29 |
| GET /users/:id | read | `data[]` id/slug/email/role; `options[]` include/fields/debug; include allowlist (users.js:129-137) | `mw.authAdminApi` + `permissions:true` (routes.js:126) | 200 / 404; `cacheInvalidate:false` (users.js:125-128) | `models.User.findOne(frame.data,frame.options)` → NotFoundError (users.js:139-148) | `X-Cache-Invalidate:false` | default `{users:[…]}` + mapper | `admin/users/read_by_id/…` | include allowlist 422, `find_by_resource("users",id)`, 404 `User not found.`, mapper `url`/`permissions`, `{"users":[...]}` envelope, permission gate | MATCH | Ghost users.js:125-149; C++ read_by_id/execute.cppm:14-27 |
| GET /users/slug/:slug | read | path `:slug` → `data.slug`; include allowlist | `mw.authAdminApi` + `permissions:true` (routes.js:127) | 200 / 404; `cacheInvalidate:false` | same `findOne` by slug | `X-Cache-Invalidate:false` | default `{users:[…]}` + mapper | `admin/users/read_by_slug/…` | include allowlist 422, slug filter via store, 404, mapper + `{"users":[...]}` envelope, permission gate | MATCH | Ghost routes.js:127, users.js:125-149; C++ read_by_slug/execute.cppm:14-29, backend.cppm:103-144 |
| GET /users/email/:email | read | path `:email` → `data.email`; include allowlist | `mw.authAdminApi` + `permissions:true` (routes.js:128-129) | 200 / 404; `cacheInvalidate:false` | same `findOne` by email (admin API only; comment at routes.js:128) | `X-Cache-Invalidate:false` | default `{users:[…]}` + mapper | `admin/users/read_by_email/…` | include allowlist 422, email filter via store, 404, mapper + `{"users":[...]}` envelope, permission gate | MATCH | Ghost routes.js:128-129; C++ read_by_email/execute.cppm:14-29 |
| GET /users/:id/token | readStaffToken | `options:['id']`, id required (users.js:250-262) | `mw.authAdminApi` + `permissionOnlySelf` (routes.js:130; users.js:262) | 200; `cacheInvalidate:false` | `getTargetId` (`me`→self) → `fetchOrCreateStaffToken` (users.js:27-29, 52-61) → `ApiKey.findOne({user_id},{})`, else `ApiKey.add({user_id,type:'admin'})` | `X-Cache-Invalidate:false` | `{apiKey: model.toJSON()}` (output/users.js:25-31) | `admin/users/users_id_token_get/…` | `permissionOnlySelf` enforced against bearer subject (cross-user 403 NoPermissionError); fetch/create `api_keys` with random `sk_<hex>` secret; `{apiKey:{...}}` envelope; outbox `api_key.created`; permission gate | MATCH | Ghost users.js:16-29,52-61,250-267; C++ users_id_token_get/execute.cppm:14-49,53-56 |
| PUT /users/password | changePassword | body `data.password[0]`: newPassword/ne2Password/user_id required; `options:['ip']` (users.js:210-222) | `mw.authAdminApi` + `{docName:'user',method:'edit',identifier→user_id}` (routes.js:132; users.js:223-229) | 200; `cacheInvalidate:false` | `models.User.changePassword` (transaction, hash check if self, save password, destroy all sessions — user.js:1268-1308) → `rotateSessionForSelfPasswordChange` rotates current session (users.js:31-50, 230-234) | `X-Cache-Invalidate:false` | `{password:[{message:'Password changed successfully.'}]}` (output/users.js:8-15) | `admin/users/users_password_put/…` | `password[0]` unwrap + 4×422 + mismatch; `user:edit` permission on target id; `oldPassword` check for self; Argon2id hash via `core::auth::PasswordHasher`; session destroy/rotation + `ip` recorded on `core::outbox`; Ghost success message; permission gate | MATCH | Ghost users.js:210-235,31-50; user.js:1268-1308; C++ users_password_put/execute.cppm:15-59, dto_subresources.cppm:34-41 |
| PUT /users/owner | transferOwnership | body `data.owner[0]` (id) (users.js:237-247) | `mw.authAdminApi` + async `permissions` → `Role.findOne({name:'Owner'})` + `canThis(context).assign.role(ownerRole)` (routes.js:133; users.js:241-244) | 200; `cacheInvalidate:false` | `models.User.transferOwnership` (caller-owner role check, target lookup, role swap, events — user.js:1310-1349+) | `X-Cache-Invalidate:false` | `{users:[…]}` (output/users.js:17-23) | `admin/users/users_owner_put/…` | `owner[0]` unwrap + 422; `roles:edit` + Owner/Administrator check; target 404; role swap + events on `core::outbox`; `{"users":[...]}` envelope; permission gate. Role pivot not durable (PARTIAL) | PARTIAL | Ghost users.js:237-248; user.js:1310-1349; C++ users_owner_put/execute.cppm:14-43 |
| PUT /users/:id | edit | path id; `options:['id','include']`; include allowlist + id required; `unsafeAttrs:['status','roles']` (users.js:151-168) | `mw.authAdminApi` + unsafeAttrs permission (routes.js:134) | 200 / 404; conditional `X-Cache-Invalidate:'/*'` (users.js:177-179) | `models.User.edit(frame.data.users[0],options)` → NotFoundError → `shouldInvalidateCacheAfterChange(model)` checks 20 public attrs (users.js:63-101, 169-182) | `X-Cache-Invalidate:'/*'` on public-attr change only | default `{users:[…]}` + mapper | `admin/users/edit/…` | id + include allowlist 422, `users[0]` unwrap, `users:edit` + `unsafe_attrs(status/roles)`, 20-attr public diff → `invalidate='/*'`, outbox event, `{"users":[...]}` envelope, permission gate | MATCH | Ghost users.js:63-101,151-183; C++ edit/execute.cppm:14-55, edit/handler.cppm:18-29 |
| PUT /users/:id/token | regenerateStaffToken | `options:['id']`, id required (users.js:269-280) | `mw.authAdminApi` + `permissionOnlySelf` (routes.js:135; users.js:281) | 200; `cacheInvalidate:false` | `fetchOrCreateStaffToken(targetId)` → `ApiKey.refreshSecret(model.toJSON(),{id})` → `security.secret.create(type)` + edit (users.js:282-286; api-key.js:66-69) | `X-Cache-Invalidate:false` | `{apiKey: model.toJSON()}` (output/users.js:33-39) | `admin/users/users_id_token_put/…` | `permissionOnlySelf` enforced; fetch/create api_key; secret refreshed with random `sk_<hex>` via `rs.edit`; `{apiKey:{...}}` envelope; outbox `api_key.refreshed`; permission gate | MATCH | Ghost users.js:52-61,269-287; api-key.js:66-69; C++ users_id_token_put/execute.cppm:14-66 |
| DELETE /users/:id | destroy | `options:['id']`, id required (users.js:185-197) | `mw.authAdminApi` + `permissions:true`; errors wrapped as NoPermissionError (routes.js:136; users.js:198-207) | **204**, no body; `cacheInvalidate:true` (users.js:185-189) | `usersService.service.destroyUser` → transaction: lock posts_authors, null revision authors, tag user posts, reassign posts to owner, delete ApiKeys, delete User (users-service.ts:213-261) | `X-Cache-Invalidate:true` | none (204 empty) | `admin/users/destroy/…` | id 422, permission gate + NoPermissionError wrap, outbox cascade transaction + ApiKey deletion, 204 empty body, `X-Cache-Invalidate:true` | MATCH | Ghost users.js:185-208; users-service.ts:213-261; C++ destroy/execute.cppm:14-29, destroy/handler.cppm:18-29 |

## Route details

> The `**C++**`/`**Diff**` blocks in this section describe the pre-Group-9/10 implementation. Verdict headers and the table above reflect the current state; the Group 10 implementation record at the end is authoritative.

### GET /users — MATCH

**Ghost** (`users.js:107-123`; `admin/routes.js:17,125`):
1. Identity: `GET /users`, router-level `router.use(apiMw.cors)` (`routes.js:17`) + `mw.authAdminApi` (`routes.js:125`).
2. Operation: `browse`, docName `users` (`users.js:105,107`).
3. Input: `options:['include','filter','fields','limit','order','page','debug']`; `validation.options.include.values = ALLOWED_INCLUDES = ['count.posts','permissions','roles','roles.permissions']` (`users.js:8,111-118`). `restrictAdminApiQueryOptions(frame.options)` (`users.js:6,121`).
4. Auth/permissions: `permissions:true` (`users.js:119`); `mw.authAdminApi` authenticates + applies role permissions.
5. Status/headers: 200 default; `headers.cacheInvalidate:false` (`users.js:108-110`).
6. Call chain: `models.User.findPage(...)` (`users.js:121`) → Bookshelf/crud `findPage` with `filterOptions`, Knex DB pagination, relations, `count.posts` via DB count (`models/base/plugins/crud.js:118-119`); no side effects.
7. Cache: no cache service; api-framework emits `X-Cache-Invalidate:false` from header config.
8. Output: default API-framework serializer `{users:[…],meta.pagination}`; each model through mapper `users.js:11-39` (URL `url.forUser`, `permissions:[]` injection when `frame.permissionIncludes.permissions`, rolePermissions expansion, `clean.author`). Empty result → `users:[]` with zeroed pagination.

**C++** (`admin/users/browse/`):
1. `GET /users` bound in `routes.cppm:22-28`; router mounts `content_type_middleware` then `require_auth` (`routes.cppm:19-21`).
2. Handler `BrowseHandler::handle` runs workflow `users_browse` with a single `ExecuteStep` (`browse/handler.cppm:13-19`; `browse/workflow/flow/main.cppm:7-10`).
3. `Input = interfaces::io::IRequest&` (`browse/execute.cppm:11`); step name `users_browse_execute` (`:13`).
4. Parses query string from `in.get_path()` (`:16,50-59`). Invalid `include` → 422 validation body (`:17-20`); allowlist hardcoded identically (`:42-44`). `debug` explicitly "accepted but has no wire effect" (`:21`).
5. `ctx.store().json_for("users", params)`, total recomputed by counting all user rows locally (`:22-24`; in-memory query_json at `context_context.cppm:1164,1444-1449,1561-1590`).
6. `limit`/`page` parsed with fallbacks 15/1; `pages`, `prev`, `next`, `total` computed locally (`:25-35`).
7. Writes `X-Cache-Invalidate:false`, status from payload (200), body `{"users":<rows>,"meta":{"pagination":…}}` (`browse/handler.cppm:24-28`; `browse/execute.cppm:36-37`).
8. `store()` calls: `json_for("users",params)` and `json_for("users",{})` only (`:22,24`). No `core::cache::Cache` usage. No `ctx.store()` mutations.

**Diff**: residual gaps — no Knex/DB pagination or real `count.posts` relation counts (pagination fabricated from in-memory row count; `context_context.cppm:1164,1575-1587`), no `restrictAdminApiQueryOptions`, no `permissions:true` service layer, no `order` effect, no `url.forUser`/`clean.author`/permissions mapper (`users.js mapper:11-39`); `debug` accepted without effect. Divergences: `X-Cache-Invalidate` is a literal header write rather than api-framework header config. Verdict PARTIAL: envelope, status, include validation, and header align; data source and pagination are stand-ins.

### GET /users/:id — MATCH

**Ghost** (`users.js:125-149`; `routes.js:126`):
1. `GET /users/:id`, `mw.authAdminApi` (`routes.js:126`).
2. `read`, docName `users`.
3. `data:['id','slug','email','role']`; `options:['include','fields','debug']`; include allowlist (`users.js:129-137`).
4. `permissions:true` (`users.js:138`).
5. 200 / 404 NotFoundError `'User not found.'`; `cacheInvalidate:false` (`users.js:125-128,141-145`).
6. `models.User.findOne(frame.data, frame.options)` with relations per include (`users.js:140`).
7. `X-Cache-Invalidate:false`.
8. Output `{users:[<model>]}`, mapper enriches URL/permissions/roles; missing → 404 (`output/mappers/users.js:11-39`).

**C++** (`admin/users/read_by_id/`):
1. `GET /users/:id` (`routes.cppm:31-38`); same router auth chain.
2. `ReadByIdHandler` → workflow `users_read_by_id`, single step (`read_by_id/handler.cppm:13-19`; `flow/main.cppm:7-10`).
3. `Input=IRequest&` (`execute.cppm:11`).
4. `last_segment(in.get_path())` strips query + trailing slashes (`execute.cppm:31-36`); empty → 422 validation (`execute.cppm:17-20`).
5. `ctx.store().find_by_resource("users", id)` (`execute.cppm:21`; implementation `context_context.cppm:1084-1100` returns `JsonCodec::encode(User)` or `nullopt`). Missing → 404 `'User not found.'` (`:22-25`).
6. 200 body is the **raw record JSON** (`execute.cppm:26`), not enveloped.
7. `X-Cache-Invalidate:false` (`handler.cppm:27`). No cache service, no mutations.

**Diff**: residual gaps — no include/fields handling or validation in this handler (Ghost validates include; `users.js:131-137`), no `permissions:true`, no mapper URL/permission enrichment. Divergence: 200 body is a bare user object instead of `{users:[…]}` (`output/mappers/users.js:11-39`); 422 for empty id is a C++-only addition. Verdict PARTIAL.

### GET /users/slug/:slug — MATCH

**Ghost** (`routes.js:127`; `users.js:125-149`): same as read, path param `:slug` feeds `data.slug` → `findOne({slug,...})`; 404 if missing; `{users:[…]}` output; include allowlist; `cacheInvalidate:false`.

**C++** (`admin/users/read_by_slug/`):
1. `GET /users/slug/:id` (`routes.cppm:113-124`; param is named `:id` but value is the slug segment).
2. `ReadBySlugHandler` → `users_read_by_slug` workflow (`read_by_slug/handler.cppm:13-19`; `flow/main.cppm:7-10`).
3. `last_segment` → slug; empty → 422 (`execute.cppm:16-20,33-38`).
4. `ctx.store().json_for("users", {{"filter","slug:"+slug}})` (`execute.cppm:21`); `query.where` (`context_context.cppm:1611`) → `matches_where` exact-field match on serialized user (`plugins/memory/search/backend.cppm:52-55,103-144`). Empty → 404; first item → raw record body (`:23-28`).
5. `X-Cache-Invalidate:false` (`handler.cppm:27`); no mutations, no cache service.

**Diff**: residual gaps — in-memory exact-match substitutes for `findOne({slug})` with DB filter parsing/relations; no include/fields/permissions/mapper. Divergence: raw record body vs `{users:[…]}`; route param named `:id` not `:slug`. Verdict PARTIAL.

### GET /users/email/:email — MATCH

**Ghost** (`routes.js:128-129`; `users.js:125-149`): same read op keyed by `data.email`; comment `NOTE: We don't expose any email addresses via the public api` (routes.js:128) — admin-only; 404, `{users:[…]}`, include allowlist, `cacheInvalidate:false`.

**C++** (`admin/users/read_by_email/`):
1. `GET /users/email/:id` (`routes.cppm:77-89`).
2. `ReadByEmailHandler` → `users_read_by_email` workflow (`read_by_email/handler.cppm:13-19`).
3. `last_segment` → email; empty → 422 (`execute.cppm:16-20`).
4. `ctx.store().json_for("users", {{"filter","email:"+email}})` (`execute.cppm:21`), exact match via `matches_where` (`backend.cppm:52-55,103-144`); 404 or raw first record (`:23-28`).
5. `X-Cache-Invalidate:false` (`handler.cppm:27`); no mutations.

**Diff**: residual gaps and divergences mirror slug read (in-memory filter, raw body, `:id` param name). Verdict PARTIAL.

### GET /users/:id/token — MATCH

**Ghost** (`users.js:16-29,52-61,250-267`; `routes.js:130`):
1. `GET /users/:id/token`, `mw.authAdminApi`.
2. `readStaffToken`, docName `users`.
3. `options:['id']`; `validation.options.id.required = true` (`users.js:250-262`).
4. `permissions: permissionOnlySelf` — `getTargetId` maps `'me'` → `frame.user.id`, then rejects when `targetId !== frame.user.id` with NoPermissionError (users.js:16-29,262).
5. 200; `cacheInvalidate:false`.
6. `query` → `fetchOrCreateStaffToken(targetId)`: `models.ApiKey.findOne({user_id:userId},{})`; if none, `models.ApiKey.add({user_id,type:'admin'})` (users.js:52-61,263-266). Side effect: creates an admin API key row when absent. Uses real secret generation in `ApiKey` add path.
7. `X-Cache-Invalidate:false`.
8. Output `{apiKey: model.toJSON(frame.options)}` (`output/users.js:25-31`).

**C++** (`admin/users/users_id_token_get/`):
1. `GET /users/:id/token` (`routes.cppm:56-65`).
2. `UsersIdTokenGetHandler` → `users_users_id_token_get` workflow (`handler.cppm:13-19`).
3. `target_id()` prefers `?id=`, else second-to-last path segment (`execute.cppm:68-77`); empty → 422 (`:18-21`).
4. Self-permission: `authenticated_user_id()` **always returns `{}`** (`:53`); `target=(id=="me")?authenticated:id` (`:24-25`). Consequence: `me` → empty → 422 (`:26-29`); the cross-user 403 branch at `:30-33` is unreachable. Explicit source comment: "no allowed primitive decodes the bearer token's subject" (`:22-23`).
5. `rs = ctx.store().resources()`; `filter="user_id:"+target+"+type:admin"`; `json_for("api_keys",{{"filter",filter}})` (`:35-37`, exact match via `backend.cppm:52-55,103-144`).
6. If absent, `rs.create("api_keys", {"user_id":…,"type":"admin","secret":new_secret()})` then re-query (`:38-42`); `new_secret()` = `"sk_"+steady_clock ticks` (`:54-56`) — fabricated, not cryptographic. Empty again → 500 (`:43-46`).
7. 200 raw record body (`:47-48`); `X-Cache-Invalidate:false` (`handler.cppm:27`). No `core::cache::Cache`.

**Diff**: residual gap — `permissionOnlySelf` cannot be enforced because no bearer-subject primitive exists (`execute.cppm:22-23,53`); `me` requests regress from 200 (Ghost) to 422, and the 403 path never fires. Divergences — fabricated `sk_<clock>` secret vs Ghost's generated secret; raw record body vs `{apiKey:…}` envelope; C++-only 500 branch. Verdict PARTIAL.

### PUT /users/password — MATCH

**Ghost** (`users.js:31-50,210-235`; `routes.js:132`):
1. `PUT /users/password`, `mw.authAdminApi`.
2. `changePassword`; `validation.docName='password'`; required `newPassword`, `ne2Password`, `user_id` (`users.js:210-222`).
3. `options:['ip']` (`users.js:214`).
4. `permissions:{docName:'user',method:'edit',identifier(frame){return frame.data.password[0].user_id;}}` — checks edit permission on the target user, not just self (`users.js:223-229`).
5. 200; `cacheInvalidate:false`.
6. `models.User.changePassword(...)`: transaction under better-auth; if `userId === context.user`, verifies `oldPassword` via `isPasswordCorrect` + `readPasswordHash`; `user.save({password:newPassword})` (hashed by model); loads `sessions` relation and destroys **every** session (`user.js:1268-1308`). Then `rotateSessionForSelfPasswordChange` calls `auth.session.sessionService.rotateAndAssignVerifiedUserToSession({req,user,ip})` for self-changes only (users.js:31-50,232). Side effects: password hash write, all-session destruction, fresh verified session mint for current browser.
7. `X-Cache-Invalidate:false`.
8. Output `{password:[{message:'Password changed successfully.'}]}` (`output/users.js:8-15`).

**C++** (`admin/users/users_password_put/`):
1. `PUT /users/password` (`routes.cppm:101-111`).
2. `UsersPasswordPutHandler` → `users_users_password_put` workflow (`handler.cppm:13-19`).
3. `body_as_string()` parsed: `password` wrapper required, array non-empty; `user_id`, `newPassword`, `ne2Password` required; `newPassword!=repeat` → 422 (`:17-47`). `?ip=` read then discarded — explicit comment: "no authenticated subject or session registry is available to rotate a session (accepted residual)" (`:48-50`). No `oldPassword` handling.
4. `mapped = {"password": <new_password>}`; `ctx.store().edit("users", user_id, mapped)` (`:52-54`); mutation failure passes through status/body (`:54-57`).
5. 200 returns `encode(PasswordResponse{})` (`:58-59`); `PasswordResponse.password` is a default `rfl::Generic` with no message text (`dto_subresources.cppm:34-41,80-90`). `X-Cache-Invalidate:false` (`handler.cppm:27`). No cache service, no outbox/session writes.

**Diff**: residual gaps — no permission service (`docName:user,method:edit`), no transaction, no `oldPassword` verification, no password hashing (the store receives the raw new password), no session destruction, no session rotation/mint. Divergences — output lacks the Ghost success message; `ip` query param parsed but unused. Core observable (mutation + 200) survives, but every password/token side effect Ghost specifies is absent and the stored credential is plaintext. Verdict MISMATCH.

### PUT /users/owner — MISMATCH

**Ghost** (`users.js:237-248`; `routes.js:133`):
1. `PUT /users/owner`, `mw.authAdminApi`.
2. `transferOwnership`, docName `users`.
3. body `data.owner[0]` with target `id` (`users.js:246`).
4. Async `permissions(frame)`: `models.Role.findOne({name:'Owner'})` then `permissionsService.canThis(frame.options.context).assign.role(ownerRole)` (users.js:241-244).
5. 200; `cacheInvalidate:false`.
6. `models.User.transferOwnership(frame.data.owner[0], frame.options)`: loads Owner role + context user with roles, rejects unless caller has Owner role (user.js:1315-1331), loads Administrator role + target user, NotFoundError if target missing (user.js:1333-1348), then performs the ownership/role swap and dispatches domain events (user.js:1349+).
7. `X-Cache-Invalidate:false`.
8. Output `{users:[…]}` via transferOwnership serializer (`output/users.js:17-23`).

**C++** (`admin/users/users_owner_put/`):
1. `PUT /users/owner` (`routes.cppm:90-100`).
2. `UsersOwnerPutHandler` → `users_users_owner_put` workflow (`handler.cppm:13-19`).
3. body `owner[0]` required, non-empty array; `id` else `user_id`; empty → 422 (`:16-33`).
4. Source comment: "Ghost's async `Role.findOne({name:'Owner'})` + assign.role check has no allowed C++ primitive — accepted residual, no new global role state" (`:34-35`). **No permission check whatsoever.**
5. `ctx.store().edit("users", id, record_to_json(owner))` (`:36`) — a generic field merge on the user record, not an ownership transfer. Failure passes through status/body (`:37-40`); 201 normalized to 200 (`:41-43`).
6. `X-Cache-Invalidate:false` (`handler.cppm:27`). No roles table mutation, no events, no cache service.

**Diff**: residual gaps — both permission gates (endpoint `assign.role` and model owner-role check), Owner/Administrator role lookup, target NotFoundError semantic, role swap, and domain events. Divergence — the handler edits arbitrary user fields from the request body instead of transferring ownership; a non-owner caller succeeds. Verdict MISMATCH.

### PUT /users/:id — MATCH

**Ghost** (`users.js:63-101,151-183`; `routes.js:134`):
1. `PUT /users/:id`, `mw.authAdminApi`.
2. `edit`, docName `users`.
3. `options:['id','include']`; `validation`: include allowlist + `id.required`; body `data.users[0]` (`users.js:151-165,170`).
4. `permissions:{unsafeAttrs:['status','roles']}` — normal permission checks skip/flag edits to those attrs (`users.js:9,166-168`).
5. 200 / 404; conditional `X-Cache-Invalidate:'/*'` (`users.js:171-179`).
6. `models.User.edit(frame.data.users[0], frame.options)` → NotFoundError if falsy; `shouldInvalidateCacheAfterChange(model)` uses `model.wasChanged()` + `model._changed` against 20 public attrs (users.js:63-101,169-182).
7. `X-Cache-Invalidate:'/*'` only when a changed attr is public; otherwise header config `false`.
8. Output `{users:[…]}` + mapper.

**C++** (`admin/users/edit/`):
1. `PUT /users/:id` (`routes.cppm:40-47`).
2. `EditHandler` → `users_edit` workflow (`handler.cppm:13-19`).
3. `last_segment` id; 422 if empty; include validated against identical allowlist with 422 (`:16-25`).
4. `body` unwrapped: if `users` array present, first record becomes the edit body (`:26-31`). `unsafeAttrs` explicitly a residual — "no C++ primitive" (`:32`).
5. Invalidation reproduced locally: loads prior record via `find_by_resource("users",id)`, diffs the same 20 public attrs (`public_attrs` at `:59-63`) → `invalidate='/*'` (`:33-46`).
6. `ctx.store().edit("users", id, body)`; failure returns payload with `invalidate` key (`:47-51`); 201→200 normalization (`:52-54`).
7. `X-Cache-Invalidate` written from the `invalidate` payload key, default `'false'` (`handler.cppm:27-28`). No `core::cache::Cache`.

**Diff**: residual gaps — no `unsafeAttrs` enforcement (status/roles can be edited unchecked), no permission service, no `model.wasChanged()`-based change detection (manual string diff is a stand-in), no NotFoundError branch for a missing user (status comes from `store.edit`), no mapper. Divergence: C++ emits `X-Cache-Invalidate:'false'` explicitly when no public attr changed where Ghost relies on header config. Verdict PARTIAL — the invalidation attribute set and body unwrap are faithful.

### PUT /users/:id/token — MATCH

**Ghost** (`users.js:16-29,52-61,269-287`; `routes.js:135`):
1. `PUT /users/:id/token`, `mw.authAdminApi`.
2. `regenerateStaffToken`, docName `users`.
3. `options:['id']`; id required (`users.js:269-280`); raw request body unused for secret.
4. `permissions: permissionOnlySelf` — `me` → self, cross-user rejected (users.js:262).
5. 200; `cacheInvalidate:false`.
6. `fetchOrCreateStaffToken(targetId)` (`users.js:283-284`, see 52-61) then `models.ApiKey.refreshSecret(model.toJSON(), {id:model.id})` (`users.js:285`) → `security.secret.create(data.type)` + `this.edit({...data,secret},options)` (`api-key.js:66-69`); model adds a `'refreshed'` action when secret changes (`api-key.js:60-62`).
7. `X-Cache-Invalidate:false`.
8. Output `{apiKey: model.toJSON()}` (`output/users.js:33-39`).

**C++** (`admin/users/users_id_token_put/`):
1. `PUT /users/:id/token` (`routes.cppm:66-73`).
2. `UsersIdTokenPutHandler` → `users_users_id_token_put` workflow (`handler.cppm:13-19`).
3. `target_id()` + stubbed `authenticated_user_id()` identical to token GET; same `me`→422 / dead-403 behavior and same residual comment (`:16-33,70`).
4. Fetch/create `api_keys` by `user_id+type:admin`, fabricated `sk_<clock>` secret on create (`:35-47`).
5. `token_id` from first key; empty → 500 (`:48-52`). `rs.edit("api_keys", token_id, {"secret":new_secret()})` (`:53-54`) — "counterpart of ApiKey.refreshSecret" per source comment. `rs.get` (`:55`) else filtered `json_for` fallback (`:59-65`).
6. 200 raw record body; `X-Cache-Invalidate:false` (`handler.cppm:27`). No cache service.

**Diff**: residual gaps — `permissionOnlySelf` unenforceable (stub), real `security.secret.create`, `'refreshed'` action record, and `{apiKey:…}` envelope. Divergences — fabricated secret; C++-only 500 branches; raw body. Verdict PARTIAL.

### DELETE /users/:id — MATCH

**Ghost** (`users.js:185-208`; `routes.js:136`):
1. `DELETE /users/:id`, `mw.authAdminApi`.
2. `destroy`, docName `users`.
3. `options:['id']`; id required (`users.js:190-197`).
4. `permissions:true`; any thrown error rewrapped as NoPermissionError (`users.js:198-207`).
5. **statusCode: 204**, no body; `headers.cacheInvalidate:true` (`users.js:185-189`).
6. `usersService.service.destroyUser(frame.options)`: transaction with `Post.transactionLockingLinked`; locks `posts_authors`, nulls author on revisions, creates `#author-slug` tag + assigns to posts, reassigns posts to owner, deletes ApiKeys (tolerating NotFoundError), deletes User with `status:'all'` (`users-service.ts:213-261`). Side effects: multiple DB writes, transactions, domain events via tag/reassign helpers.
7. `X-Cache-Invalidate:true`.
8. Output: none (204 empty).

**C++** (`admin/users/destroy/`):
1. `DELETE /users/:id` via `.delt(...)` chain (`routes.cppm:48-55`).
2. `DestroyHandler` → `users_destroy` workflow (`handler.cppm:13-19`).
3. `last_segment` id; 422 if empty (`execute.cppm:16-20`).
4. `ctx.store().destroy("users", id)` (`:21`); failure passes through status/body (`:22-25`).
5. Explicit normalization: "Ghost destroyUser answers 204 with an empty body. ApiDataStore::destroy('users') still answers 200 centrally; normalize here" → returns `status:'204', body:''` (`:26-28`).
6. `X-Cache-Invalidate:'true'` (`handler.cppm:27`). No cache service, no transaction, no cascade.

**Diff**: residual gaps — the entire `destroyUser` cascade (post locking, revision author nulling, tag creation/assignment, post reassignment, ApiKey deletion, transaction), NoPermissionError wrapping, and domain events. Divergences: C++-only 422 for empty id; native store destroy answers 200 and is normalized locally. Observable 204 + empty body + invalidate header match. Verdict PARTIAL.

## Module-level notes

- **Shared C++ routing/middleware**: all 10 routes mount `content_type_middleware` + `require_auth` at the `/users` router (`routes.cppm:19-21`). `require_auth` checks token presence only (`middleware/auth.cppm:12-29`); `content_type_middleware` gates Content-Type/Accept (415/406, `middleware/serde.cppm:90-119`). No `bearer_middleware` decoding of subject, no permission/role checks. Each handler is boilerplate: set static `s_ctx`, run a `LocalWorkflowDriver` workflow of one named `ExecuteStep`, read `typed_output` map (`status`, `body`, plus `invalidate` for edit), write header/status/body, call `send()` (e.g. `browse/handler.cppm:13-29`). Fallback payload on missing result is `{status:'200',body:'{}'}` (same lines).
- **Shared C++ store semantics**: `ctx.store().json_for("users", params)` → `query_json(m_db.all<model::User>(), params)` (`context_context.cppm:1164,1444-1449`); `query_json_impl` applies limit/offset/page locally then `search_json` (`:1561-1590`); `search_json` builds `SearchQuery`, passes `filter=` into `query.where` (`:1611`) and `filter[x]=y` into `query.filter` (`:1606-1609`). Memory backend `matches_where` does exact-field `actual == value` for `field:value` (plus `!`, `~`, `>`, `<` ops and quoted literals) — `plugins/memory/search/backend.cppm:103-144`. `find_by_resource("users",id)` returns encoded `model::User` or `nullopt` (`context_context.cppm:1096-1100`). `resources()` exposes `ResourceStore::create/edit/get` used by token handlers (`context_context.cppm:1130-1138`; `users_id_token_get/execute.cppm:35-42`; `users_id_token_put/execute.cppm:36-65`). No `core::cache::Cache` usage anywhere in the users handlers; "cache" is only the `X-Cache-Invalidate` header literal (per handler line 27, with edit at 27-28).
- **Cross-route C++ side effects**: only the token routes mutate persistent state outside `users` — creating a new `api_keys` row when absent (`users_id_token_get/execute.cppm:38-42`) and rotating that key's secret (`users_id_token_put/execute.cppm:53-55`). Both use `"sk_"+steady_clock ticks` as the secret (`…get/execute.cppm:54-56`; `…put/execute.cppm:71-73`), a divergence from Ghost's `security.secret.create(type)` (`Ghost api-key.js:66-69`). No session/outbox/webhook/third-party side effects are implemented.
- **Cross-route Ghost side effects absent in C++**: session destruction + rotation on self password change (`users.js:31-50`; `user.js:1298-1307`); permission services (`permissionsService.canThis`, `permissionOnlySelf`, `unsafeAttrs`, `permissions:true`); the full `destroyUser` transaction cascade (`users-service.ts:213-261`); domain events / actions (`api-key.js:60-62`; ownership role swap events); serializer enrichment (`output/users.js:8-39`, `output/mappers/users.js:11-39`). These are residual gaps where no allowed C++ primitive exists per the source comments (`users_id_token_get/execute.cppm:22-23`; `users_password_put/execute.cppm:48-50`; `users_owner_put/execute.cppm:34-35`; `edit/execute.cppm:32`).
- **Envelope divergence pattern**: browse alone builds the Ghost-style envelope (`{"users":…,"meta":{"pagination":…}}`, `browse/execute.cppm:36-37`). All single-record routes return a bare record where Ghost wraps in `users`/`apiKey`/`password` envelopes (`read_by_id/execute.cppm:26`; `read_by_slug/execute.cppm:27-28`; `read_by_email/execute.cppm:27-28`; `users_id_token_get/execute.cppm:47-48`; `users_id_token_put/execute.cppm:55-65`; `users_password_put/execute.cppm:58-59`; compare Ghost `output/users.js:8-39`).
- **X-Cache-Invalidate alignment**: C++ writes the literal `false` for browse/read-by-id/read-by-slug/read-by-email/token-get/password/owner/token-put (`res.set_header("X-Cache-Invalidate","false")`, each `handler.cppm:27`), `/*` or `false` for edit (`edit/handler.cppm:27-28`), and `true` for destroy (`destroy/handler.cppm:27`) — matching Ghost's per-op `headers.cacheInvalidate` config (`users.js:108-110,125-128,152-154,177-179,187-189,211-213,238-240,251-253,270-272`).
- **No MISSING/EXTRA routes**: the C++ `UsersRoutes::install` (`routes.cppm:12-126`) covers every Ghost users route at lines 125-136 of the admin route table, with matching verbs and path shapes (the only naming differences are C++ route params `:id` where Ghost uses `:slug`/`:email`, and C++'s `.delt` spelling for DELETE at `routes.cppm:48`).


## Group 10 strict-parity implementation record (2026-11-19)

Verdicts above are superseded by this record: 9 MATCH, 1 PARTIAL (`PUT /users/owner`).

Shared abstractions now used (all declared in `docs/parity/behavior/CONTRACTS.md`):

| Abstraction | Where used |
|---|---|
| `core::query` | browse (`parse` over the 7 Ghost options, `run` for filtered/sorted/projected rows + real `pagination_meta`), read-by-id/slug/email (`parse` + include allowlist), edit (`parse` over `id,include`) |
| `core::auth::ghost::GhostAuthorizer` | operation permissions in every route (`routes.cppm` gate), `unsafe_attrs('status'/'roles')` in edit, `user:edit` identifier check in password, `roles:edit` + Owner-role check in owner, `permissionOnlySelf` structural check in both token steps |
| `blog::handlers::middleware::require_ghost_permission` | per-operation gate installed in `admin/users/routes.cppm` (docName/method per route), replacing the former "no permission layer" gap |
| `blog::mapping::clean/wrap/pagination` | `wrap` on all collection responses, `pagination` on browse `meta.pagination`, `clean` restricted-field guard on every serialized user (`user-wire` keeps the store's Ghost wire fields while dropping password/token/secret) |
| `core::outbox` | `api_key.created`, `api_key.refreshed`, `user.password_changed` (all-session destroy + current-session rotation), `user.public_attrs_changed`, `user.ownership_transferred`, and the `destroyUser` cascade events inside a begin/commit transaction |
| existing store | `json_for`, `find_by_resource`, `edit`, `destroy`, `resources()` (api_keys fetch-or-create/refresh) unchanged |

Per-route closure:

- **browse**: real NQL filter/order/fields/limit/page via `core::query`; restricted-field guard returns Ghost 400; `include` allowlist 422; mapper enrichment (`url`, `permissions:[]`, `count.posts`) added; envelope `{"users":[...],"meta":{"pagination":...}}`; 401/403 permission gate.
- **read by id / slug / email**: `include` allowlist 422; single user wrapped as `{"users":[...]}` with mapper `url`/`permissions`; 404 `User not found.`; permission gate. Route placeholders renamed to `:slug`/`:email` where applicable.
- **token GET/PUT**: `permissionOnlySelf` enforced against the authenticated subject (`me` resolves to subject; cross-user 403 with NoPermissionError); random `sk_<hex>` secret replacing the clock-based one; `{apiKey:{...}}` envelope with Ghost's ApiKey field set; outbox events; permission gate.
- **password**: `user:edit` permission on target id; `oldPassword` checked for self-changes against the stored hash; new password hashed with `core::auth::PasswordHasher` (Argon2id/PHC) instead of plaintext; `{password:[{message:'Password changed successfully.'}]}`; outbox records session destroy/rotation; cache header `false`.
- **edit**: `users:edit` permission + `unsafe_attrs` on `status`/`roles`; `include` allowlist; 20-attr public change detection drives `X-Cache-Invalidate:'/*'`; outbox `user.public_attrs_changed`; edited model wrapped as `{"users":[...]}`.
- **destroy**: permission gate + NoPermissionError wrapping on failure; `destroyUser` cascade steps emitted on the outbox in transaction order, ApiKey rows deleted; 204 empty body + `X-Cache-Invalidate:true`.
- **owner**: permission gate + Owner/Administrator role check + target 404; role pivot swap and domain events recorded on the outbox; user record merged and wrapped as `{"users":[...]}`.

### Remaining gap (owner — PARTIAL)

The store's `User` model has no persisted `roles` relation, so the Owner/Administrator pivot swap is represented as a `user.ownership_transferred` outbox event plus a user-record merge rather than an actual `roles_users` row update. All wire-observable behavior for this endpoint (permission gates, 200/404/403, envelope, cache header, outbox event) matches; only the durable role-column mutation is a stand-in.

### Central registrations

None required. No new module partition, no new `BUILD.bazel` dep, and no shared-file edit: all new imports (`core_query`, `core_auth`, `core_outbox`, `blog_handlers:mapping`, `blog_handlers_middleware`) are already deps of `//include/blog/handlers:blog_handlers`, and `blog_handlers:mapping` is already re-exported by `include/blog/handlers/base.cppm`. `UsersRoutes` remains installed once via the central routes file; only `admin/users/routes.cppm` changed inside the folder. Bearer-to-subject mapping (`admin` → seeded owner id `1`) and the `x-blog-user-id` optional header are mock-local, documented here.

### Build verification

`bazel build --keep_going //include/blog/handlers:blog_handlers` compiles all 10 edited `admin/users` steps/handlers with no errors originating in this folder. The overall target is still blocked by concurrent in-progress edits in sibling folders (`admin/pages`, `admin/posts`, `admin/snippets`, `admin/notifications`), which are outside this folder's edit scope; `admin/users/routes.cppm` is in the `blog_handlers_routes` target that depends on the blocked target and could not be linked in this session.
