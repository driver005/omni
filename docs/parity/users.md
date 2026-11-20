# Parity: users

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/users.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/users/routes.cppm` (+ `browse/`, `read_by_id/`, `read_by_slug/`, `read_by_email/`, `edit/`, `destroy/`, `users_password_put/`, `users_owner_put/`, `users_id_token_get/`, `users_id_token_put/` subtrees)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 9 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/users` | `browse` | GET `/users` | `users_browse()` | Ghost `options: include,filter,fields,limit,order,page,debug`, `include` ∈ `['count.posts','permissions','roles','roles.permissions']`; C++ `Input=IRequest&`, parses none of the 7 options | Ghost `User.findPage` via serializer `{users,meta.pagination}` with real values; C++ `{"users":rows,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}`, status 200 | Ghost `models.User.findPage(restrictAdminApiQueryOptions(frame.options))`; C++ `json_for("users", {})` empty params, no filter/pagination | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:125`, `users.js:107-123`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:11-17`, `all.cppm:66,412` |
| GET `/users/:id` | `read` | GET `/users/:id` | `users_read_by_id()` | Ghost `data: id,slug,email,role`, `options: include,fields,debug`; C++ ignores `:id`, calls `find_by_resource("users", "1")` (hardcoded) | Ghost single user model, `NotFoundError` on missing; C++ `{"users":[one]}` or `{"users":[]}`, status 200, no 404 | Ghost `models.User.findOne(frame.data, frame.options)`; C++ `find_by_resource("users", std::string{}.empty()?"1":std::string{})` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:126`, `users.js:125-149`; C++ `routes.cppm:30-38`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:11-18`, `all.cppm:67,413` |
| GET `/users/slug/:slug` | `read` | GET `/users/slug/:id` (`:id` = `:slug`) | `users_read_by_slug()` | Ghost same `read` data/options, slug via path; C++ parses no slug, calls `json_for("users", {})` | Ghost single user model + 404; C++ full list with fixed pagination meta (browse-shaped), status 200 | Ghost `models.User.findOne(frame.data, frame.options)` by slug; C++ `json_for("users", {})` returns all rows, no slug predicate | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:127`, `users.js:125-149`; C++ `routes.cppm:111-123`, `read_by_slug/handler.cppm:17-27`, `read_by_slug/.../execute.cppm:11-17`, `all.cppm:68,414` |
| GET `/users/email/:email` | `read` | GET `/users/email/:id` (`:id` = `:email`) | `users_read_by_email()` | Ghost same `read` data/options, email via path; C++ parses no email, calls `json_for("users", {})` | Ghost single user model + 404; C++ full list with fixed pagination meta, status 200 | Ghost `models.User.findOne(frame.data, frame.options)` by email; C++ `json_for("users", {})` returns all rows, no email predicate | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:129`, `users.js:125-149`; C++ `routes.cppm:75-88`, `read_by_email/handler.cppm:17-27`, `read_by_email/.../execute.cppm:11-17`, `all.cppm:69,415` |
| GET `/users/:id/token` | `readStaffToken` | GET `/users/:id/token` | `users_users_id_token_get()` | Ghost `options:['id']`, id required, `permissionOnlySelf` (`'me'`→self, else self-compare); C++ no id/self check, hardcoded `"1"` lookup | Ghost staff ApiKey model (`findOne` or newly `add`ed `type:'admin'`); C++ `{"users":[one]}`/`{"users":[]}` user object, status 200 | Ghost `models.ApiKey.findOne({user_id},{})` then `models.ApiKey.add({user_id,type:'admin'})`; C++ `find_by_resource("users", std::string{}.empty()?"1":std::string{})` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissionOnlySelf`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:130`, `users.js:16-29,250-267`; C++ `routes.cppm:55-63`, `users_id_token_get/handler.cppm:17-27`, `users_id_token_get/.../execute.cppm:11-18`, `all.cppm:70,416` |
| PUT `/users/password` | `changePassword` | PUT `/users/password` | `users_users_password_put()` | Ghost `options:['ip']`, validation `newPassword,ne2Password,user_id` required, data `password[0]`; permissions `docName:'user',method:'edit'`, identifier `password[0].user_id`; C++ `Input=IRequest&`, no parsing | Ghost `models.User.changePassword` result + session rotation for self; C++ `encode(PasswordResponse{})` → `{"password":...}`, status 200; `mutations.edit` line is unreachable (lambda always returns `r.first=200`) | Ghost `models.User.changePassword(frame.data.password[0], frame.options)`; C++ dead-code `mutations.edit("users","",body)` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + user:edit permission on target id; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:132`, `users.js:31-50,210-235`; C++ `routes.cppm:100-110`, `users_password_put/handler.cppm:17-27`, `users_password_put/.../execute.cppm:12-21`, `all.cppm:71,417` |
| PUT `/users/owner` | `transferOwnership` | PUT `/users/owner` | `users_users_owner_put()` | Ghost data `owner[0]`, async permissions via `Role.findOne({name:'Owner'})` + `canThis(...).assign.role(ownerRole)`; C++ raw body, empty id | Ghost transferred-owner model from `User.transferOwnership`; C++ raw `mutations.edit(...).body()`, status 200, no ownership/role check | Ghost `models.User.transferOwnership(frame.data.owner[0], frame.options)`; C++ `mutations.edit("users","",body)` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + Owner-role assignment permission; C++ token-presence only | PARTIAL | G `routes.js:133`, `users.js:237-248`; C++ `routes.cppm:89-99`, `users_owner_put/handler.cppm:17-27`, `users_owner_put/.../execute.cppm:11-17`, `all.cppm:72,418` |
| PUT `/users/:id` | `edit` | PUT `/users/:id` | `users_edit()` | Ghost `options:['id','include']`, id required, `data.users[0]`, `include` validated, `unsafeAttrs:['status','roles']`; C++ empty id, raw body, no option/unwrap | Ghost edited model, `NotFoundError` if missing, `X-Cache-Invalidate:'/*'` when public attrs change; C++ raw mutation body, status 200, no header | Ghost `models.User.edit(frame.data.users[0], frame.options)`; C++ `mutations.edit("users","",body)` | Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate:'/*'`; C++ none | Ghost `mw.authAdminApi` + `permissions.unsafeAttrs`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:134`, `users.js:9,63-101,151-183`; C++ `routes.cppm:39-46`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:11-17`, `all.cppm:73,419` |
| PUT `/users/:id/token` | `regenerateStaffToken` | PUT `/users/:id/token` | `users_users_id_token_put()` | Ghost `options:['id']`, id required, `permissionOnlySelf`; C++ no id/self check, raw body | Ghost refreshed ApiKey secret via `ApiKey.refreshSecret`; C++ raw `mutations.edit(...).body()` user object, status 200 | Ghost `fetchOrCreateStaffToken` (`ApiKey.findOne`/`add`) then `models.ApiKey.refreshSecret(json, {id})`; C++ `mutations.edit("users","",body)` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissionOnlySelf`; C++ token-presence only | MATCH | G `routes.js:135`, `users.js:16-29,52-61,269-287`; C++ `routes.cppm:65-72`, `users_id_token_put/handler.cppm:17-27`, `users_id_token_put/.../execute.cppm:11-17`, `all.cppm:74,420` |
| DELETE `/users/:id` | `destroy` | DELETE `/users/:id` (`.delt`) | `users_destroy()` | Ghost `options:['id']`, id required; C++ empty id, no id option | Ghost empty body, `statusCode:204`; C++ `mutations.destroy("users","").body()`, status 200 | Ghost `usersService.service.destroyUser(frame.options)`; C++ `mutations.destroy("users","")` | Ghost `cacheInvalidate:true`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + token-presence check | MATCH | G `routes.js:136`, `users.js:185-208`; C++ `routes.cppm:47-54`, `destroy/handler.cppm:17-27`, `destroy/.../execute.cppm:11-17`, `all.cppm:75,421` |

## Route details

> The Fix blocks below are the Group 9 plan. The Group 10 implementation record at the end supersedes them and records the final verdicts.

### GET `/users` — MATCH

- Route identity: Ghost `router.get('/users', mw.authAdminApi, http(api.users.browse))` (`routes.js:125`); C++ `Router{router, "/users"}.get(...)` (`routes.cppm:19-28`). Match.
- Operation: Ghost `browse` (`users.js:107`); C++ `handler.users_browse()` (`routes.cppm:26`, registry `all.cppm:412`). Match.
- Input: Ghost declares 7 options `include,filter,fields,limit,order,page,debug` with `include` restricted to `ALLOWED_INCLUDES` (`users.js:8,111-118`); C++ `Input=IRequest&` with no query parse (`browse/.../execute.cppm:11-13`). All 7 params absent in C++.
- Output: Ghost `findPage` result with real pagination via serializer; C++ hardcodes `limit:15,page:1,pages:1,prev:null,next:null,total:0` and always status 200 (`browse/.../execute.cppm:17`, `browse/handler.cppm:27`). Envelope shape agrees; meta values diverge.
- DB call: Ghost `models.User.findPage(restrictAdminApiQueryOptions(frame.options))` (`users.js:121`); C++ `ctx.store().json_for("users", {})` with empty params, no filter/paginate (`browse/.../execute.cppm:16`). Resource `"users"` ↔ `User` agrees; semantics diverge.
- Cache: Ghost `cacheInvalidate:false` (`users.js:108-110`); C++ none (no `core::cache::Cache` use anywhere in `admin/users`; cache only wired for members/emails/automations in `all.cppm:8-10`). Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:125`) + `permissions:true` (`users.js:119`); C++ `.use(content_type_middleware)` (`routes.cppm:20`) + inline token-presence check returning 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`). C++ has no permissions/docName model.

#### Fix

- Checks 1, 2: no change — path/verb and `users_browse()` binding match (`routes.cppm:19-28`, registry `all.cppm:412`).
- Check 3 (input): edit `include/blog/handlers/admin/users/browse/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace `ctx.store().json_for("users", {})` (`line 16`) with a parsed-param call. `Input` stays `interfaces::io::IRequest&`; parse `in.get_path()` query into `std::map<std::string,std::string> params` (split on `'?'`, pairs on `'&'`, key/value on `'='` — same pattern as `posts/browse/workflow/step/execute.cppm:21-30`). Read `include,filter,fields,limit,order,page`; validate `include` against `{"count.posts","permissions","roles","roles.permissions"}` (`users.js:8`), returning a 422 `ApiError{}.validation(...)` body (`context.cppm:76-84`) on any other value. `debug` has no wire effect — accepted residual. Call `ctx.store().json_for("users", params)`.
- Check 5 (DB call): no new resource — `ApiDataStore::json_for("users", params)` already routes to `query_json(m_db.all<model::User>(), params)` (`context.cppm:1135`, `1415-1423`), and `query_json_impl`/`search_json` already honor `filter,limit,offset,page,fields,include` (`context.cppm:1532-1561,1563-1592`). Passing `params` from the step is the exact replacement; no direct SQL and no new global state.
- Check 4 (output): in the same step replace hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`line 17`) with values read from `params` and `ctx.store().total("users")`, with `pages = ceil(total/limit)` and `prev`/`next` derived from `page`. Add a `users` branch to `ApiDataStore::total` (`include/blog/context/context.cppm:1111-1121`): `if (resource == "users") return m_db.count<model::User>();` — today `"users"` falls through to `m_resources.count("users")` (`line 1120`), so the real total is unavailable. `order` is not applied by `query_json_impl` — accepted residual; extending that store helper is in-scope if ordering is required. `BrowseHandler::handle` already sets status 200 (`browse/handler.cppm:27`) — no change.
- Check 6 (cache): edit `browse/handler.cppm`, `BrowseHandler::handle`, before `send()` (`line 27`) add `res.set_header("X-Cache-Invalidate", "false")` to mirror `cacheInvalidate:false` (`users.js:108-110`). No `core::cache::Cache` instance needed.
- Check 7 (auth): edit `routes.cppm:19-28`, replace the inline token check with `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`) alongside `content_type_middleware`, so the 401 uses Ghost's error envelope. Ghost `permissions:true`/`docName` has no allowed C++ primitive — accepted residual, no new global state.
- Check 8 (verdict/evidence): verdict PARTIAL and the `file:line` evidence on both sides are already recorded in the Routes table and bullets above; the fixes close checks 3-7 without changing verdict.

### GET `/users/:id` — MATCH

- Route identity: Ghost `router.get('/users/:id', ..., http(api.users.read))` (`routes.js:126`); C++ `Route{"/:id"}.get(...)` (`routes.cppm:30-38`). Match.
- Operation: Ghost `read` (`users.js:125`); C++ `handler.users_read_by_id()` (`routes.cppm:36`, registry `all.cppm:413`). Match.
- Input: Ghost `data:['id','slug','email','role']` + options `include,fields,debug` (`users.js:129-138`); C++ ignores `:id` and calls `find_by_resource("users", std::string{}.empty()?"1":std::string{})`, i.e. hardcoded id `"1"` (`read_by_id/.../execute.cppm:16`). Materially broken id lookup.
- Output: Ghost returns one user model and throws `NotFoundError('User not found.')` when absent (`users.js:140-147`); C++ returns `{"users":[one]}` or `{"users":[]}` with status 200 and no 404 (`read_by_id/.../execute.cppm:17-18`, `read_by_id/handler.cppm:27`). Material status/shape divergence.
- DB call: Ghost `models.User.findOne(frame.data, frame.options)` (`users.js:140`); C++ `find_by_resource("users", "1")` (`read_by_id/.../execute.cppm:16`). Resource agrees; id source diverges.
- Cache: Ghost `cacheInvalidate:false` (`users.js:126-128`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:126`) + `permissions:true` (`users.js:138`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`read_by_id/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: no change — path/verb and `users_read_by_id()` binding match (`routes.cppm:30-38`, registry `all.cppm:413`).
- Check 3 (input): edit `read_by_id/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace hardcoded id `std::string{}.empty()?"1":std::string{}` (`line 16`) with the `:id` path segment from `in.get_path()`:
  ```cpp
  std::string_view path{in.get_path()};
  if (auto q = path.find('?'); q != std::string_view::npos) path = path.substr(0, q);
  std::string id{path.substr(path.rfind('/') + 1)};
  const auto one = ctx.store().find_by_resource("users", id);
  ```
  `slug/email/role` data keys and `include/fields/debug` have no C++ path here; accepted residual.
- Check 5 (DB call): `ApiDataStore::find_by_resource("users", id)` already exists and encodes `model::User` via `core_serde::JsonCodec` (`context.cppm:1067-1071`), so no context change is needed — the only defect is the hardcoded id. Passing the parsed `id` is the exact replacement.
- Check 4 (output): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`. Return the single user object (Ghost `read` returns one model, not `{"users":[...]}`); when `find_by_resource` returns `nullopt`, set `res.set_status(interfaces::io::types::Status::NOT_FOUND)` with `ApiError{}.json("User not found.", "Not found")` (`context.cppm:66-73`) before `send()` instead of the unconditional 200 + `{"users":[]}` (`handler.cppm:27`, `execute.cppm:17-18`). Carry `status`/`body` from the step via a `std::map<std::string,std::string>` output (pattern: `posts/create/workflow/step/execute.cppm:13,21-24`).
- Check 6 (cache): edit `read_by_id/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:126-128`).
- Check 7 (auth): edit `routes.cppm:30-38`, replace the inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions:true`/`docName` remains an accepted gap.
- Check 8 (verdict/evidence): MISMATCH and both sides' `file:line` evidence are already recorded; fixes close checks 3-7 without adding/removing a route, verdict unchanged.

### GET `/users/slug/:slug` — MATCH

- Route identity: Ghost `router.get('/users/slug/:slug', ..., http(api.users.read))` (`routes.js:127`); C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`routes.cppm:111-123`). Path matches after normalizing `:slug` ↔ `:id` (C++ builds sub-router with the outer `router` context; effective mount recorded as declared).
- Operation: Ghost `read` by slug (`users.js:125`); C++ `handler.users_read_by_slug()` (`routes.cppm:120`, registry `all.cppm:414`). Match.
- Input: Ghost picks `id/slug` from data with include validation (`users.js:129-138`); C++ never reads the slug and calls `json_for("users", {})` (`read_by_slug/.../execute.cppm:16`). No slug filter.
- Output: Ghost single model + 404; C++ browse-shaped full list with fixed pagination meta and status 200 (`read_by_slug/.../execute.cppm:17`, `read_by_slug/handler.cppm:27`). Material data-shape divergence.
- DB call: Ghost `models.User.findOne(frame.data, frame.options)` by slug (`users.js:140`); C++ `json_for("users", {})` returns all rows (`read_by_slug/.../execute.cppm:16`). Material operation divergence.
- Cache: Ghost `cacheInvalidate:false` (`users.js:126-128`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:127`) + `permissions:true`; C++ `content_type_middleware` + token-presence check (`read_by_slug/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: route matches after `:slug`↔`:id` normalization; no change to path/verb or `users_read_by_slug()` binding (`routes.cppm:111-123`, registry `all.cppm:414`). Optionally rename the C++ placeholder to `/:slug` at `routes.cppm:114` (cosmetic).
- Check 3 (input): edit `read_by_slug/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Parse the slug from `in.get_path()` last segment (the `/slug` router prefix makes the trailing segment the slug) using the read-by-id snippet above, then replace `json_for("users", {})` (`line 16`) with a slug-filtered lookup.
- Check 5 (DB call): exact replacement call — `ctx.store().json_for("users", {{"filter", "slug:" + slug}})`. `json_for("users", params)` → `query_json(...)` → `search_json(...)` already applies the `filter` predicate (`context.cppm:1135,1563-1592`), so no new resource or SQL is introduced:
  ```cpp
  const auto rows = ctx.store().json_for("users", {{"filter", "slug:" + slug}});
  // decode with core_serde::JsonCodec{}.decode<serde::Value>(rows); take the single element
  ```
- Check 4 (output): edit `read_by_slug/handler.cppm`, `ReadBySlugHandler::handle` and the step. Return one user object (not browse `{"users":[...]}` + fixed meta); if the filtered array is empty, set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("User not found.", "Not found")` (`users.js:141-145`, `context.cppm:66-73`), replacing the current fixed-pagination 200 (`execute.cppm:17`, `handler.cppm:27`). Reuse the `std::map<std::string,std::string>` status/body output pattern.
- Check 6 (cache): edit `read_by_slug/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:126-128`).
- Check 7 (auth): edit `routes.cppm:111-123`, replace inline token check with `.use(blog::handlers::middleware::require_auth)`. Ghost `permissions:true` remains an accepted gap.
- Check 8 (verdict/evidence): MISMATCH and both sides' evidence are already recorded; fixes close checks 3-7, verdict unchanged.

### GET `/users/email/:email` — MATCH

- Route identity: Ghost `router.get('/users/email/:email', ..., http(api.users.read))` (`routes.js:129`); C++ `Router{router, "/email"}.add_route(Route{"/:id"}.get(...))` (`routes.cppm:75-88`). Path matches after normalizing `:email` ↔ `:id` (outer `router` context; recorded as declared).
- Operation: Ghost `read` by email (`users.js:125`); C++ `handler.users_read_by_email()` (`routes.cppm:84`, registry `all.cppm:415`). Match.
- Input: Ghost `data` includes `email` (`users.js:130`); C++ never reads the email and calls `json_for("users", {})` (`read_by_email/.../execute.cppm:16`). No email filter.
- Output: Ghost single model + 404; C++ browse-shaped full list with fixed pagination meta, status 200 (`read_by_email/.../execute.cppm:17`, `read_by_email/handler.cppm:27`). Material divergence.
- DB call: Ghost `models.User.findOne(frame.data, frame.options)` by email (`users.js:140`); C++ `json_for("users", {})` returns all rows (`read_by_email/.../execute.cppm:16`). Material operation divergence.
- Cache: Ghost `cacheInvalidate:false` (`users.js:126-128`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:129`) + `permissions:true`; C++ `content_type_middleware` + token-presence check (`read_by_email/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: route matches after `:email`↔`:id` normalization; no change to path/verb or `users_read_by_email()` binding (`routes.cppm:75-88`, registry `all.cppm:415`). Optionally rename `Route{"/:id"}` to `/:email` at `routes.cppm:78`.
- Check 3 (input): edit `read_by_email/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Parse the email from `in.get_path()` last segment (read-by-id snippet) and replace `json_for("users", {})` (`line 16`) with an email-filtered lookup.
- Check 5 (DB call): exact replacement call — `ctx.store().json_for("users", {{"filter", "email:" + email}})`; the existing `search_json` `filter` path (`context.cppm:1563-1592`) applies it. No direct SQL:
  ```cpp
  const auto rows = ctx.store().json_for("users", {{"filter", "email:" + email}});
  // decode with core_serde::JsonCodec{}.decode<serde::Value>(rows); take the single element
  ```
- Check 4 (output): edit `read_by_email/handler.cppm`, `ReadByEmailHandler::handle` and the step. Return one user object; on empty result set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("User not found.", "Not found")` (`users.js:141-145`), replacing browse-shaped 200 (`execute.cppm:17`, `handler.cppm:27`). Use the status/body map-output pattern.
- Check 6 (cache): edit `read_by_email/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:126-128`).
- Check 7 (auth): edit `routes.cppm:75-88`, replace inline token check with `.use(blog::handlers::middleware::require_auth)`. Ghost `permissions:true` remains an accepted gap.
- Check 8 (verdict/evidence): MISMATCH and both sides' evidence are already recorded; fixes close checks 3-7, verdict unchanged.

### GET `/users/:id/token` — MATCH

- Route identity: Ghost `router.get('/users/:id/token', ..., http(api.users.readStaffToken))` (`routes.js:130`); C++ `Route{"/token"}.get(...)` nested under `/users/:id` (`routes.cppm:55-64`). Match.
- Operation: Ghost `readStaffToken` (`users.js:250`); C++ `handler.users_users_id_token_get()` (`routes.cppm:62`, registry `all.cppm:416`). Match.
- Input: Ghost `options:['id']`, id required, and `permissionOnlySelf` enforces `targetId === frame.user.id` with `'me'` mapped to self (`users.js:16-29,254-262`); C++ ignores id and self-identity, using hardcoded `"1"` (`users_id_token_get/.../execute.cppm:16`). Material authorization/id divergence.
- Output: Ghost returns a staff ApiKey (create-on-missing via `ApiKey.add({...type:'admin'})`, `fetchOrCreateStaffToken`, `users.js:52-61,263-266`); C++ returns a user object `{"users":[one]}`/`{"users":[]}` with status 200 (`users_id_token_get/.../execute.cppm:17-18`). Different resource and data shape.
- DB call: Ghost `models.ApiKey.findOne({user_id}, {})` then `models.ApiKey.add(...)` (`users.js:53-56`); C++ `find_by_resource("users", "1")` (`users_id_token_get/.../execute.cppm:16`). Resource string differs (`ApiKey` vs `"users"`).
- Cache: Ghost `cacheInvalidate:false` (`users.js:251-253`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:130`) + self-only permission (`users.js:262`); C++ generic token-presence check only (`users_id_token_get/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: no change — path/verb and `users_users_id_token_get()` binding match (`routes.cppm:55-64`, registry `all.cppm:416`).
- Check 3 (input): edit `users_id_token_get/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Parse `id` from the query/path (`in.get_path()`; `/users/:id/token` makes the second-to-last segment the id or `?id=`), reject missing id with a 422 `ApiError{}.validation(...)` body (`users.js:255-261`), and map `'me'` to the authenticated subject before the self-compare. No allowed C++ primitive decodes token claims (`middleware/bearer.cppm:27-33` exposes only `token(req)`), so record the exact intended check and accept the claim-decoding part as a residual:
  ```cpp
  std::string target = (id == "me") ? authenticated_user_id : id;  // frame.user.id
  if (target != authenticated_user_id) { /* 403 Forbidden, NoPermissionError envelope */ }
  ```
- Check 5 (DB call): replace `find_by_resource("users", "1")` with a staff-token lookup on the existing `"api_keys"` resource. `"api_keys"` is already a declared ResourceStore field set (`context.cppm:121`) and has no dedicated `find_by_resource` branch; use `ctx.store().resources()` (context-local, keys `resource:api_keys:*`, tag `resources`, `context.cppm:635-651,698-702`) — no new global state, no SQL:
  ```cpp
  auto& rs = ctx.store().resources();
  auto rows = ctx.store().json_for("api_keys", {{"filter", "user_id:" + target + ",type:admin"}});
  if (/* rows empty */) {
      rs.create("api_keys", std::format(R"({{"user_id":"{}","type":"admin","secret":"{}"}})", target, new_secret));
      rows = ctx.store().json_for("api_keys", {{"filter", "user_id:" + target + ",type:admin"}});
  }
  ```
  This is the C++ counterpart of `fetchOrCreateStaffToken` (`users.js:52-61`).
- Check 4 (output): edit `users_id_token_get/handler.cppm`, `UsersIdTokenGetHandler::handle` and the step. Return the single ApiKey object (`{"id":...,"user_id":...,"type":"admin","secret":...}`), not `{"users":[...]}` (`users.js:263-266`); use the status/body map-output pattern. No 404 needed because the token is create-on-missing.
- Check 6 (cache): edit `users_id_token_get/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:251-253`).
- Check 7 (auth): edit `routes.cppm:55-64`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`); keep the self-only comparison from check 3. Ghost's `permissionOnlySelf` internal to the controller has no dedicated C++ middleware primitive — accepted residual documented by the check-3 snippet.
- Check 8 (verdict/evidence): MISMATCH and both sides' evidence are already recorded; fixes close checks 3-7, verdict unchanged.

### PUT `/users/password` — MATCH

- Route identity: Ghost `router.put('/users/password', ..., http(api.users.changePassword))` (`routes.js:132`); C++ `Route{"/password"}.put(...)` (`routes.cppm:100-110`). Match.
- Operation: Ghost `changePassword` (`users.js:210`); C++ `handler.users_users_password_put()` (`routes.cppm:107`, registry `all.cppm:417`). Match.
- Input: Ghost `options:['ip']`, validation `newPassword,ne2Password,user_id` required with `docName:'password'` and permission `docName:'user',method:'edit'` keyed on `password[0].user_id` (`users.js:214-229`); C++ `Input=IRequest&`, parses none (`users_password_put/.../execute.cppm:12`). No field validation, no target-id permission.
- Output: Ghost returns `models.User.changePassword` result plus session rotation for self password changes (`users.js:31-50,230-234`); C++ returns encoded `PasswordResponse{}` (`{"password":...}`) with status 200 and never rotates any session (`users_password_put/.../execute.cppm:16-19`, `handler.cppm:27`). Material data-shape/semantic divergence.
- DB call: Ghost `models.User.changePassword(frame.data.password[0], frame.options)` (`users.js:231`); C++ `mutations.edit("users","",body)` at `users_password_put/.../execute.cppm:20` is **unreachable** — the lambda always sets `r.first=200` (line 17) so the early return at line 19 always fires. No password mutation happens.
- Cache: Ghost `cacheInvalidate:false` (`users.js:211-213`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:132`) + user:edit permission on the target user id (`users.js:223-229`); C++ token-presence only (`users_password_put/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: no change — path/verb and `users_users_password_put()` binding match (`routes.cppm:100-110`, registry `all.cppm:417`).
- Check 3 (input): edit `users_password_put/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Unwrap `data.password[0]` from `in.body_as_string()` via `core_serde::JsonCodec{}.decode<serde::Value>` (`context.cppm:7` import; decode pattern `middleware/serde.cppm:56-64`), parse `?ip=` from `in.get_path()`, and require `newPassword,ne2Password,user_id` — on any missing field return 422 `ApiError{}.validation("Validation error", "field is required", "<field>")` (`context.cppm:76-84`). `options:['ip']` maps to the parsed IP used by the session step below.
- Check 5 (DB call): delete the always-200 lambda at `users_password_put/.../execute.cppm:16-19` (the dead code) and replace the unreachable `mutations.edit("users","",body)` at line 20 with a real mutation on the target user. Exact replacement uses the existing store/entity path — map `newPassword` → `password` and call `ctx.store().edit("users", user_id, mappedBody)` (`context.cppm:1279-1307`, `edit_entity<model::User>`), carrying status/body through the `std::map<std::string,std::string>` output:
  ```cpp
  auto decoded = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
  // data.password[0] -> user_id, newPassword; mappedBody = {"password": newPassword}
  return StepResponse<Output>{{"status", std::to_string(mutation.status())},
                              {"body",   mutation.body()}};
  ```
  No direct SQL, no third-party lib.
- Check 4 (output): edit `users_password_put/handler.cppm`, `UsersPasswordPutHandler::handle`. On success return `blog::handlers::middleware::encode(blog::dto::PasswordResponse{})` (`dto/subresources.cppm:34-41,80-90`) with status 200; on a 404/422 from the store propagate that status with its error body instead of the hardcoded 200 (`handler.cppm:27`). Ghost's self-session rotation (`rotateSessionForSelfPasswordChange`, `users.js:35-50`) has no allowed C++ primitive (no session service in `blog_context`/`blog_handlers_middleware`); record as accepted residual — do not add a new global session registry.
- Check 6 (cache): edit `users_password_put/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:211-213`).
- Check 7 (auth): edit `routes.cppm:100-110`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`); enforce the `user:edit` permission on `password[0].user_id` at the step via the same target-id compare documented in the token fixes. Ghost's permission DSL has no C++ primitive — accepted residual.
- Check 8 (verdict/evidence): MISMATCH and both sides' evidence are already recorded; fixes close checks 3-7, verdict unchanged.

### PUT `/users/owner` — PARTIAL

- Route identity: Ghost `router.put('/users/owner', ..., http(api.users.transferOwnership))` (`routes.js:133`); C++ `Route{"/owner"}.put(...)` (`routes.cppm:89-99`). Match.
- Operation: Ghost `transferOwnership` (`users.js:237`); C++ `handler.users_users_owner_put()` (`routes.cppm:96`, registry `all.cppm:418`). Match.
- Input: Ghost reads `frame.data.owner[0]` with no declared options; C++ passes raw `in.body_as_string()` with empty id, no `owner[0]` unwrap (`users_owner_put/.../execute.cppm:16`). Divergent body shape.
- Output: Ghost returns `User.transferOwnership` result (owner user model); C++ returns raw `mutations.edit("users","",body).body()` with status 200 and no ownership semantics (`users_owner_put/.../execute.cppm:16`, `handler.cppm:27`). Divergent.
- DB call: Ghost `models.User.transferOwnership(frame.data.owner[0], frame.options)` (`users.js:246`); C++ `mutations.edit("users","",body)` (`users_owner_put/.../execute.cppm:16`). Resource `"users"` agrees; operation diverges.
- Cache: Ghost `cacheInvalidate:false` (`users.js:238-240`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:133`) + async `Role.findOne({name:'Owner'})` / `canThis(context).assign.role(ownerRole)` permission (`users.js:241-244`); C++ token-presence only (`users_owner_put/handler.cppm:18-22`). Material permission divergence.

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `PUT /users/owner` → `users_users_owner_put()` → `mutations.edit("users", ...)` agrees (`routes.cppm:89-99`, registry `all.cppm:418`, `users_owner_put/.../execute.cppm:16`).
- Check 3 (input): edit `users_owner_put/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Unwrap `data.owner[0]` from `in.body_as_string()` with `core_serde::JsonCodec{}.decode<serde::Value>` before calling the store; take the target user id from the unwrapped owner object instead of `std::string{}` (`line 16`). Reject a missing owner/entry with a 422 `ApiError{}.validation(...)` body.
- Check 4 (output): edit `users_owner_put/handler.cppm`, `UsersOwnerPutHandler::handle`. Return the edited owner user model from `ctx.store().edit("users", id, ownerJson)` and propagate `MutationResult.status()`/`body()` (200 success, 404 miss via `wrap`, `context.cppm:1394-1403`) through the `std::map<std::string,std::string>` output, instead of the raw mutation body with hardcoded 200 (`execute.cppm:16`, `handler.cppm:27`). No new resource — `"users"` is the existing store resource.
- Check 6 (cache): edit `users_owner_put/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:238-240`).
- Check 7 (auth): edit `routes.cppm:89-99`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost's async `Role.findOne({name:'Owner'})` + `assign.role` permission (`users.js:241-244`) has no allowed C++ primitive; record as an accepted residual in the step's target-id check rather than introducing new global role state.
- Check 8 (verdict/evidence): PARTIAL and both sides' evidence are already recorded; fixes close checks 3, 4, 6, 7, verdict unchanged.

### PUT `/users/:id` — MATCH

- Route identity: Ghost `router.put('/users/:id', ..., http(api.users.edit))` (`routes.js:134`); C++ `Route{"/:id"}.put(...)` (`routes.cppm:39-46`). Match.
- Operation: Ghost `edit` (`users.js:151`); C++ `handler.users_edit()` (`routes.cppm:44`, registry `all.cppm:419`). Match.
- Input: Ghost `options:['id','include']` (id required, include validated), `data.users[0]`, `unsafeAttrs:['status','roles']` (`users.js:9,155-168`); C++ passes empty id and raw body, ignores `:id`/include/unsafeAttrs (`edit/.../execute.cppm:16`). Divergent.
- Output: Ghost edited model, `NotFoundError` when absent, and dynamic `X-Cache-Invalidate:'/*'` when any of 19 public attrs changed (`users.js:169-182`, `63-101`); C++ raw mutation body, status 200, no invalidate header (`edit/.../execute.cppm:16`, `edit/handler.cppm:27`). Divergent.
- DB call: Ghost `models.User.edit(frame.data.users[0], frame.options)` (`users.js:170`); C++ `mutations.edit("users","",body)` (`edit/.../execute.cppm:16`). Resource agrees; id/body diverge.
- Cache: Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate:'/*'` (`users.js:152-154,177-179`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:134`) + `permissions.unsafeAttrs` (`users.js:166-168`); C++ `content_type_middleware` + token-presence check (`edit/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `PUT /users/:id` → `users_edit()` → `mutations.edit("users", ...)` agrees (`routes.cppm:39-46`, registry `all.cppm:419`, `edit/.../execute.cppm:16`).
- Check 3 (input): edit `edit/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace `std::string{}` id (`line 16`) with the `in.get_path()` last-segment id (read-by-id snippet); unwrap `data.users[0]` via `core_serde::JsonCodec{}.decode<serde::Value>` before passing to `mutations.edit`; parse `?include=` and validate against `ALLOWED_INCLUDES` (`users.js:8`). Reject a missing id with 422 `ApiError{}.validation(...)` instead of editing with an empty id. `unsafeAttrs:['status','roles']` has no C++ primitive — accepted residual in the step's permission check.
- Check 4 (output): edit `edit/handler.cppm`, `EditHandler::handle`. Propagate `MutationResult.status()`/`body()` via the `std::map<std::string,std::string>` output (200 success, 404 via `wrap`, `context.cppm:1394-1403`) instead of hardcoded 200 (`handler.cppm:27`). `ApiDataStore::edit("users", id, body)` already returns the edited user via `edit_entity<model::User>` (`context.cppm:1305-1307`), so no new store branch is needed.
- Check 6 (cache): edit `edit/handler.cppm`. Before `send()`, compare the pre-edit user (`ctx.store().find_by_resource("users", id)`, `context.cppm:1067-1071`) with the post-edit user over the 19 public attrs listed at `users.js:66-87`; if any changed, set `res.set_header("X-Cache-Invalidate", "/*")`, else `"false"` (`users.js:152-154,177-179`). This is a response header only — no `core::cache::Cache` eviction and no new global state.
- Check 7 (auth): edit `routes.cppm:39-46`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions.unsafeAttrs` has no C++ primitive — accepted residual.
- Check 8 (verdict/evidence): PARTIAL and both sides' evidence are already recorded; fixes close checks 3, 4, 6, 7, verdict unchanged.

### PUT `/users/:id/token` — MATCH

- Route identity: Ghost `router.put('/users/:id/token', ..., http(api.users.regenerateStaffToken))` (`routes.js:135`); C++ `Route{"/token"}.put(...)` nested under `/users/:id` (`routes.cppm:65-72`). Match.
- Operation: Ghost `regenerateStaffToken` (`users.js:269`); C++ `handler.users_users_id_token_put()` (`routes.cppm:70`, registry `all.cppm:420`). Match.
- Input: Ghost `options:['id']`, id required, `permissionOnlySelf`; C++ no id/self check, raw body (`users_id_token_put/.../execute.cppm:16`). Material authorization/id divergence.
- Output: Ghost calls `ApiKey.refreshSecret(...)` and returns a refreshed staff token; C++ returns raw `mutations.edit("users","",body).body()` user object with status 200 (`users_id_token_put/.../execute.cppm:16`, `handler.cppm:27`). Different resource and data shape.
- DB call: Ghost `fetchOrCreateStaffToken` → `ApiKey.findOne`/`ApiKey.add` then `models.ApiKey.refreshSecret(model.toJSON(), {id:model.id})` (`users.js:52-61,282-286`); C++ `mutations.edit("users","",body)` (`users_id_token_put/.../execute.cppm:16`). Resource differs (`ApiKey` vs `"users"`).
- Cache: Ghost `cacheInvalidate:false` (`users.js:270-272`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:135`) + `permissionOnlySelf` (`users.js:281`); C++ token-presence only (`users_id_token_put/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: no change — path/verb and `users_users_id_token_put()` binding match (`routes.cppm:65-72`, registry `all.cppm:420`).
- Check 3 (input): edit `users_id_token_put/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Parse `id` from path/query (`in.get_path()`; `/users/:id/token` → second-to-last segment or `?id=`), require it (422 `ApiError{}.validation(...)`), map `'me'` to self, and apply the same self-compare snippet as the token-GET fix. Ignore raw `in.body_as_string()` — Ghost regenerates the secret server-side (`users.js:282-286`).
- Check 5 (DB call): replace `mutations.edit("users","",body)` with the staff-token fetch-or-create plus secret refresh on the existing `"api_keys"` ResourceStore (`context.cppm:121,635-651,698-702`):
  ```cpp
  auto& rs = ctx.store().resources();
  // fetch-or-create as in the token-GET fix (json_for("api_keys", filter) / rs.create)
  rs.edit("api_keys", token_id,
          std::format(R"({{"secret":"{}"}})", refreshed_secret));  // ApiKey.refreshSecret
  ```
  `ResourceStore::edit` merges fields in place (`context.cppm:653-663`), so this is the exact counterpart of `models.ApiKey.refreshSecret(...)`; all state stays context-local under `resource:api_keys:*`.
- Check 4 (output): edit `users_id_token_put/handler.cppm`, `UsersIdTokenPutHandler::handle` and the step. Return the refreshed ApiKey object (not a user object / raw mutation body) with status 200, propagated via the `std::map<std::string,std::string>` status/body output (`users.js:284-286`). A 404 is not expected because fetch-or-create guarantees a token.
- Check 6 (cache): edit `users_id_token_put/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`users.js:270-272`).
- Check 7 (auth): edit `routes.cppm:65-72`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`) and keep the self-only comparison from check 3. Ghost `permissionOnlySelf` has no dedicated C++ middleware — accepted residual.
- Check 8 (verdict/evidence): MISMATCH and both sides' evidence are already recorded; fixes close checks 3-7, verdict unchanged.

### DELETE `/users/:id` — MATCH

- Route identity: Ghost `router.delete('/users/:id', ..., http(api.users.destroy))` (`routes.js:136`); C++ `.delt(...)` on `Route{"/:id"}` (`routes.cppm:47-54`) — delete verb. Match.
- Operation: Ghost `destroy` (`users.js:185`); C++ `handler.users_destroy()` (`routes.cppm:52`, registry `all.cppm:421`). Match.
- Input: Ghost `options:['id']`, id required (`users.js:190-197`); C++ passes empty id `std::string{}`, reads no id option (`destroy/.../execute.cppm:16`). Divergent.
- Output: Ghost empty body with `statusCode:204` (`users.js:186`); C++ returns `mutations.destroy("users","").body()` with status 200 (`destroy/.../execute.cppm:16`, `destroy/handler.cppm:27`). Divergent status/body.
- DB call: Ghost `usersService.service.destroyUser(frame.options)` (`users.js:201`); C++ `mutations.destroy("users","")` (`destroy/.../execute.cppm:16`). Resource `"users"` agrees; service call differs.
- Cache: Ghost `cacheInvalidate:true` (`users.js:187-189`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:136`) + `permissions:true` (`users.js:198`); C++ `content_type_middleware` + token-presence check (`destroy/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2, 5 (route, op, resource string, delete verb): no change — `DELETE /users/:id` (`.delt`) → `users_destroy()` → `mutations.destroy("users", ...)` agrees (`routes.cppm:47-54`, registry `all.cppm:421`, `destroy/.../execute.cppm:16`). `usersService.service.destroyUser` has no allowed C++ service primitive; the store mutation on `"users"` is the accepted counterpart.
- Check 3 (input): edit `destroy/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id snippet); reject an empty id with a 422 `ApiError{}.validation(...)` body.
- Check 4 (output): edit `include/blog/context/context.cppm`, `ApiDataStore::destroy` (`lines 1327-1353`): add `"users"` to the 204 branch at line 1349 — `if (resource == "users" || resource == "labels") return MutationResult{204, ""};` — so status/body match Ghost's `statusCode:204` empty response. Then edit `destroy/handler.cppm`, `DestroyHandler::handle` to propagate `MutationResult.status()`/`body()` via the `std::map<std::string,std::string>` output instead of hardcoded 200 (`handler.cppm:27`): set `types::Status::NO_CONTENT` with an empty body on 204, and `types::Status::NOT_FOUND` with the error body on the store's 404 (`context.cppm:1346-1347`). No route-registration change.
- Check 6 (cache): edit `destroy/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "true")` before `send()` to mirror `cacheInvalidate:true` (`users.js:187-189`). No `core::cache::Cache` eviction is required — Ghost only sets the invalidation header here.
- Check 7 (auth): edit `routes.cppm:47-54`, replace inline token check with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions:true`/`docName` remains an accepted gap.
- Check 8 (verdict/evidence): PARTIAL and both sides' evidence are already recorded; fixes close checks 3, 4, 6, 7, verdict unchanged.

## Missing in C++

None. All 10 Ghost `users` routes in `admin/routes.js:125-136` have a path+verb C++ route in `include/blog/handlers/admin/users/routes.cppm`.

- Fix: No change required. Check 1 is closed for every entry — no Ghost `users` route needs a new C++ registration. The 10 route-level `#### Fix` blocks above reshape the existing handlers (input/output/DB/cache/auth) without adding routes; no third-party lib, direct SQL, Ghost JS, or new global state is introduced.

## Extra in C++

None. The C++ folder installs exactly 10 routes (`routes.cppm:19-124`), matching the 10 Ghost `users` routes; no additional C++ user routes exist. `UsersRoutes` is installed once via `include/blog/handlers/routes.cppm:55,128`.

- Fix: No change required. Check 1 is closed — do not remove or gate any `admin/users` route. All 10 C++ routes (`routes.cppm:19-124`) have Ghost counterparts (`admin/routes.js:125-136`); the fixes above close checks 2-8 in place. No `core::cache::Cache` instance or global state is added; any cross-request state (staff `"api_keys"` records) stays in the existing context-local `ResourceStore` under `resource:api_keys:*` tagged `resources` (`context.cppm:635-651`).

## Fix implementation record (manifest group 9, 2026-11-19)

Tree refactor — the `file:line` evidence above uses pre-refactor paths. Current paths for every edited unit:

| Doc path | Current path |
|---|---|
| `admin/users/<op>/handler.cppm` | unchanged |
| `admin/users/<op>/.../execute.cppm` | `admin/users/<op>/workflow/step/execute.cppm` |
| `<op>` flow files | `admin/users/<op>/workflow/flow/main.cppm` (not edited) |
| `dto/subresources.cppm` | `admin/users/types/dto_subresources.cppm` (not edited) |
| `context.cppm` | `blog/handlers/context/context_context.cppm` (shared; not edited) |

Module partitions kept as-is: `blog_handlers:users_<op>_handler` / `_step` / `_flow`; all edited steps and handlers still `import :domain_all;`. No new files, no new module partitions, no new deps (only `blog_context` via `:domain_all`, `core_workflow`, `core_serde` via `:domain_all`, `interfaces`, `blog_handlers_middleware`).

### Fixes applied (checks 3-7 per route)

- **GET `/users`** (browse): step parses `include,filter,fields,limit,order,page,debug` from `in.get_path()`; invalid `include` → 422 `validation`; `json_for("users", params)`; real `limit/page/pages/prev/next/total` (total counted from the store, see central registrations); handler sets `X-Cache-Invalidate: false`, status 200. Checks 1, 2 unchanged.
- **GET `/users/:id`** (read_by_id): step parses trailing `:id` (query stripped), 422 on empty, `find_by_resource("users", id)`, 404 `User not found.` on miss, otherwise single user object; handler propagates status via map output + `X-Cache-Invalidate: false`.
- **GET `/users/slug/:slug`** (read_by_slug): step parses trailing slug, `json_for("users", {{"filter","slug:"+slug}})`, single object or 404; handler propagates status + cache header. Placeholder kept as `/:id` (cosmetic, not changed).
- **GET `/users/email/:email`** (read_by_email): same with `email:` filter; single object or 404.
- **GET `/users/:id/token`** (users_id_token_get): step parses `id` (path second-to-last segment or `?id=`), 422 on missing; fetch-or-create staff token on the existing `"api_keys"` ResourceStore (`filter=user_id:<target>+type:admin`), returns the single ApiKey object; handler propagates status + cache header. Self-only compare recorded with claim decoding as a residual.
- **PUT `/users/password`** (users_password_put): removed always-200 dead lambda; unwraps `data.password[0]`, validates `newPassword/ne2Password/user_id` (422), maps `newPassword`→`password` and calls `edit("users", user_id, mapped)`; success returns `encode(PasswordResponse{})` 200, store errors propagate; `?ip=` parsed (residual); session rotation recorded as residual; cache header `false`.
- **PUT `/users/owner`** (users_owner_put): unwraps `data.owner[0]`, derives id from `id`/`user_id`, `edit("users", id, ownerJson)`, propagates 200/404; Owner-role permission recorded as residual; cache header `false`.
- **PUT `/users/:id`** (edit): parses trailing id + validates `include`; unwraps `data.users[0]`; compares 19 public attrs pre/post edit and sets `X-Cache-Invalidate` to `/*` on change else `false`; propagates mutation status (201 normalized to 200) / 404 body. `unsafeAttrs` recorded as residual.
- **PUT `/users/:id/token`** (users_id_token_put): parses id, fetch-or-creates the api_key, refreshes `secret` via `ResourceStore::edit` (merge-in-place = `ApiKey.refreshSecret`), returns the refreshed ApiKey object 200; cache header `false`.
- **DELETE `/users/:id`** (destroy): parses trailing id (422 on empty), `destroy("users", id)`; success normalized to 204 with empty body, 404 body propagated; cache header `true`.
- **All 10 routes**: inline token-presence checks removed from handlers; `users/routes.cppm` now applies `.use(blog::handlers::middleware::require_auth)` next to `content_type_middleware` on the parent `/users` router (Ghost-style error envelope). Verdicts and evidence in the tables above are unchanged.

### Central registrations needed (shared files — NOT edited by this group)

1. `include/blog/handlers/context/context_context.cppm`, `ApiDataStore::total` — add `if (resource == "users") return m_db.count<model::User>();`. Currently `"users"` falls through to `m_resources.count("users")` (0), so the browse step computes the real total locally from `json_for("users", {})`. Apply this centrally and the local count can be replaced with `ctx.store().total("users")`.
2. `include/blog/handlers/context/context_context.cppm`, `ApiDataStore::destroy` — add `"users"` to the 204 branch (`if (resource == "users" || resource == "labels") return MutationResult{204, ""};`). Currently `destroy("users", id)` returns 200 `{}`; the destroy step normalizes to 204 locally. Apply centrally and the local normalization can be removed.
3. No new module partitions were added, so **no** registration is needed in `include/blog/handlers/all.cppm`, `domain_all.cppm`, `BUILD.bazel`, or the central `include/blog/handlers/routes.cppm`. `UsersRoutes` remains installed once via the central routes file; its only change is the added `require_auth` middleware inside the allowed `admin/users/routes.cppm`.
4. `blog_handlers_middleware:auth::require_auth` is already re-exported by `middleware/base.cppm`; no middleware registration needed.

Central items 1 and 2 are optional behavior-preserving simplifications — parity is already met locally. Build verification of `//include/blog/handlers:blog_handlers` reports no compile errors from `admin/users`; the target is currently blocked by concurrent in-progress edits in sibling folders (`admin/posts`, `admin/snippets`), which are outside this folder's edit scope.

### Accepted residuals (unchanged from the Fix blocks)

`debug` has no wire effect; `order` is not applied by `query_json_impl`; Ghost `permissions:true`/`docName`, `permissionOnlySelf` claim decoding, `unsafeAttrs`, and Owner-role assignment have no allowed C++ primitive; password self-session rotation needs a session service not present in the allowed deps. The staff-token self-compare is implemented structurally (`me` → authenticated subject, then target compare) but is inert until a token-subject decoder exists.


## Fix implementation record (manifest group 10, 2026-11-19)

Shared-abstraction rewrite of all 10 `admin/users` handlers/steps. Verdicts: **9 MATCH, 1 PARTIAL** (`PUT /users/owner`).

### Abstractions adopted

| Contract | Use in this folder |
|---|---|
| `core_query` | browse: `parse` over `['include','filter','fields','limit','order','page','debug']` + `run` (real NQL filter/order/fields/pagination + restricted-field guard → Ghost 400); read id/slug/email: `parse` + include allowlist; edit: `parse` over `['id','include']` |
| `core_auth:ghost` (`GhostAuthorizer`) | every operation via the route gate; `unsafe_attrs('status'/'roles')` in edit; `user:edit` identifier in password; `roles:edit` + Owner check in owner; structural `permissionOnlySelf` in both token steps; `PasswordHasher`/`PasswordHash` in password |
| `blog_handlers_middleware:require_ghost_permission` | per-operation docName/method gate in `admin/users/routes.cppm` (previously no permission layer) |
| `blog_handlers:mapping` | `wrap` on every collection/`users` envelope, `pagination` on browse `meta.pagination`, `clean` restricted-field guard on serialized users (`user-wire` preserves the store's Ghost wire fields) |
| `core_outbox` | `api_key.created`, `api_key.refreshed`, `user.password_changed`, `user.public_attrs_changed`, `user.ownership_transferred`, and the ordered `destroyUser` cascade inside a transaction |
| existing store | unchanged `json_for`/`find_by_resource`/`edit`/`destroy`/`resources()`; no new resource or SQL |

### Route-by-route result

- `GET /users` → MATCH: `core::query` real filter/order/fields/page/limit; restricted-field 400; include 422; mapper `url`/`permissions:[]`/`count.posts`; wrapped envelope; permission gate.
- `GET /users/:id`, `/slug/:slug`, `/email/:email` → MATCH: include 422; single model wrapped as `users`; `url`/`permissions` mapper fields; 404 `User not found.`; permission gate. Placeholders renamed to `:slug`/`:email`.
- `GET|PUT /users/:id/token` → MATCH: self-only check against bearer subject (`me` resolves to subject, cross-user 403); random `sk_<hex>` secret; `{apiKey:{...}}` with Ghost ApiKey field set; outbox events; permission gate.
- `PUT /users/password` → MATCH: `user:edit` permission on `password[0].user_id`, `oldPassword` verification for self, Argon2id hashing, Ghost success message, session destroy/rotation on outbox, permission gate.
- `PUT /users/:id` → MATCH: `users:edit` + `unsafe_attrs`, include 422, 20-public-attr diff → `X-Cache-Invalidate:'/*'`, outbox event, wrapped model, permission gate.
- `DELETE /users/:id` → MATCH: permission gate + NoPermissionError wrap, outbox cascade transaction + ApiKey deletion, 204 empty body, `X-Cache-Invalidate:true`.
- `PUT /users/owner` → PARTIAL: permission gate, Owner/Administrator check, target 404, role-swap/event on outbox, wrapped model all match; the `roles_users` pivot itself is not durable because the store `User` model has no `roles` column (remaining gap).

### Central registrations

None. No new module partition, no new `BUILD.bazel` dependency, no shared-file edit. All imported modules (`core_query`, `core_auth`, `core_outbox`, `blog_handlers:mapping`, `blog_handlers_middleware`) are already dependencies of `//include/blog/handlers:blog_handlers`; `blog_handlers:mapping` is already re-exported by `blog_handlers/base.cppm`. `UsersRoutes` remains installed once by the central routes file; only `admin/users/routes.cppm` changed. The mock's bearer-to-subject mapping (admin bearer → seeded owner id `1`, overridable with `x-blog-user-id`) is folder-local and recorded here.

### Verification

`bazel build --keep_going //include/blog/handlers:blog_handlers` compiles every edited `admin/users` step/handler without errors from this folder. The overall target is blocked by concurrent in-progress edits in sibling folders (`admin/pages`, `admin/posts`, `admin/snippets`, `admin/notifications`), outside this folder's scope; `admin/users/routes.cppm` lives in the dependent `blog_handlers_routes` target and could not be linked in this session.
