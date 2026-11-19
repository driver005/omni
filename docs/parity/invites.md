# Parity: invites

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/invites.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/auth/invites/routes.cppm` (installed via `include/blog/handlers/routes.cppm:25,98`; handlers registered in `include/blog/handlers/all.cppm:194-197,540-543,789-792`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 2 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/invites` | `browse` | GET `/invites` (`routes.cppm:19-28`) | `invites_browse()` → `BrowseHandler` (`routes.cppm:26`; `all.cppm:540`) | Ghost `options=['include','page','limit','fields','filter','order','debug']`, `validation.options.include=[]` (`invites.js:22-27`); C++ `using Input=interfaces::io::IRequest&`, no query parsing (`browse/.../execute.cppm:12-14`) | Ghost `models.Invite.findPage(...)` serialized `{invites,…meta.pagination}` status 200 (`invites.js:29-31`); C++ hardcoded `InvitesResponse` with one default `InviteDto{id:"1",email:"invite@example.com",status:"sent"}` via early return, no meta (`execute.cppm:16-21`) | Ghost `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))` (`invites.js:30`); C++ `ctx.store().json_for("invites",{})` is dead code after early return, no filtering/pagination (`execute.cppm:21-23`) | Ghost `headers.cacheInvalidate:false` (`invites.js:19-21`); C++ none invite-specific; generic `ResourceStore` uses `record_key("invites",id)`, tag `resources`, TTL never (`context.cppm:645-646,725`) | Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:494`; `invites.js:28`); C++ `content_type_middleware` + `token(req)` empty → 401 `{"error":"Unauthorized"}` (`routes.cppm:20`; `browse/handler.cppm:18-22`) | PARTIAL | Ghost `invites.js:18-32`, `routes.js:494` ↔ C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:12-23` |
| GET `/invites/:id` | `read` | GET `/invites/:id` (`routes.cppm:37-46`) | `invites_read_by_id()` → `ReadByIdHandler` (`routes.cppm:44`; `all.cppm:541`) | Ghost `options=['include']`, `data=['id','email']`, `include=[]` (`invites.js:38-44`); C++ `Input=IRequest&`, ignores `:id`, no `email` (`read_by_id/.../execute.cppm:11-16`) | Ghost single `Invite` model, 404 `Invite not found.` if missing (`invites.js:46-55`); C++ always `{"invites":[]}` (`execute.cppm:16-18`) | Ghost `models.Invite.findOne(frame.data, frame.options)` (`invites.js:47`); C++ `find_by_resource("invites", "1")` — hardcoded `"1"`, and `"invites"` has no branch in `find_by_resource` → always null (`execute.cppm:16`; `context.cppm:1055-1092`) | Ghost `headers.cacheInvalidate:false` (`invites.js:35-37`); C++ none invite-specific | Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:495`; `invites.js:45`); C++ token check only, 401 on empty token (`read_by_id/handler.cppm:18-22`) | MISMATCH | Ghost `invites.js:34-56`, `routes.js:495` ↔ C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:13-18`, `context.cppm:1055-1092` |
| POST `/invites` | `add` | POST `/invites` (`routes.cppm:29-36`) | `invites_create()` → `CreateHandler` (`routes.cppm:34`; `all.cppm:542`) | Ghost `options=['include','email']`, required `data.role_id` + `data.email`, `permissions.unsafeAttrs=['role_id']` (`invites.js:80-96`); C++ raw `in.body_as_string()` (`create/.../execute.cppm:16`), no validation/unsafeAttrs | Ghost `invites.service.add({api,InviteModel,invites,options,user})` status 201, sends invite (`invites.js:97-108`, `statusCode` at `:76`); C++ `{"invites":[{"id":"N"}]}` status 200 — handler ignores `MutationResult{201,…}` and sets `Status::OK` (`create/.../execute.cppm:16`; `create/handler.cppm:27`; `context.cppm:1272-1276,41-49`) | Ghost `invites.service.add` + `models.Invite` (`invites.js:3,98-101`); C++ generic `mutations.create("invites", body)` → `ResourceStore::create` (`execute.cppm:16`; `context.cppm:1272`) | Ghost `headers.cacheInvalidate:false` (`invites.js:77-79`); C++ none invite-specific; generic store TTL never, tag `resources` (`context.cppm:645-646`) | Ghost `mw.authAdminApi` + `staffInviteLimiter` + `permissions.unsafeAttrs` (`routes.js:496-501`; `invites.js:94-96`); C++ token check only, no rate limiter (`create/handler.cppm:18-22`) | PARTIAL | Ghost `invites.js:75-109`, `routes.js:496-501` ↔ C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/.../execute.cppm:13-16` |
| DELETE `/invites/:id` | `destroy` | DELETE `/invites/:id` (`routes.cppm:47-55`, `.delt()` = DELETE per `utils/openapi/route.cppm:96-99`) | `invites_destroy()` → `DestroyHandler` (`routes.cppm:52`; `all.cppm:543`) | Ghost `options=['include','id']`, `include=[]` (`invites.js:63-68`); C++ `Input=IRequest&`, ignores `:id` (`destroy/.../execute.cppm:11-16`) | Ghost `statusCode:204` empty body, 404 if missing (`invites.js:62,70-72`); C++ 200 with `MutationResult` body of a 404 `Resource not found` error — handler discards status (`execute.cppm:16`; `destroy/handler.cppm:27`; `context.cppm:1346-1352,41-49`) | Ghost `models.Invite.destroy({...frame.options, require:true})` (`invites.js:71`); C++ `mutations.destroy("invites", "")` — empty id, never the path `:id` → generic destroy fails → 404 (`execute.cppm:16`; `context.cppm:1344-1347`) | Ghost `headers.cacheInvalidate:false` (`invites.js:59-61`); C++ none invite-specific; generic `ResourceStore::destroy` erases `record_key("invites",id)` (`context.cppm:665-672`) | Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:502`; `invites.js:69`); C++ token check only (`destroy/handler.cppm:18-22`) | MISMATCH | Ghost `invites.js:58-73`, `routes.js:502` ↔ C++ `routes.cppm:47-55`, `destroy/handler.cppm:17-27`, `destroy/.../execute.cppm:13-16`, `context.cppm:1327-1352` |

## Route details

### GET /invites — PARTIAL

- Route/verb: Ghost `router.get('/invites', …, http(api.invites.browse))` (`routes.js:494`) ↔ C++ `Router{router,"/invites"}.get(...)` (`routes.cppm:19-28`). Agree.
- Operation mapping: Ghost `browse` (`invites.js:18`) ↔ `handler.invites_browse()` (`routes.cppm:26`). Agree.
- Input: Ghost declares `include,page,limit,fields,filter,order,debug` with `include` restricted to `ALLOWED_INCLUDES=[]` (`invites.js:7,22-27`); C++ step declares `Input=IRequest&` but parses no query string (`browse/workflow/step/execute.cppm:12-14`). All Ghost query param names missing in C++.
- Output: Ghost returns `Invite.findPage(...)` through the api-framework serializer with `meta.pagination` (`invites.js:29-31`); C++ builds a `dto::InvitesResponse` containing one default `InviteDto{id:"1",email:"invite@example.com",status:"sent"}` (`execute.cppm:17-19`; defaults at `include/blog/dto/admin_resources.cppm:32-52`) and `encode`s it (`execute.cppm:19`, `middleware/serde.cppm:155-157`). Because `r.first=200` is always `!=0`, line 21 returns early and the `json_for` branch (line 22-23) is unreachable: C++ always emits one hardcoded invite and no `meta`.
- DB call: Ghost `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))` (`invites.js:30`); C++ `ctx.store().json_for("invites",{})` is dead code (`execute.cppm:21-23`); generic `json_for` fallback would be `parse_array(m_resources.list("invites"))` with no filtering (`context.cppm:1180-1181`).
- Cache: Ghost `headers.cacheInvalidate:false` (`invites.js:19-21`) — no invalidation. C++ invites code: none. Only the generic `ResourceStore` uses `record_key("invites",id)`, tag `resources`, TTL never (`context.cppm:645-646,725`).
- Auth: Ghost `mw.authAdminApi` (`routes.js:494`) + `permissions:true` (`invites.js:28`); C++ router-level `content_type_middleware` (`routes.cppm:20`) and per-handler bearer `token(req)` check → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; `middleware/bearer.cppm:27-33`). No Ghost-style permission docName/unsafeAttrs equivalent.

### GET /invites/:id — MISMATCH

- Route/verb: Ghost `router.get('/invites/:id', …, http(api.invites.read))` (`routes.js:495`) ↔ C++ `Route{"/:id"}.get(...)` (`routes.cppm:38-45`). Agree.
- Operation mapping: Ghost `read` (`invites.js:34`) ↔ `handler.invites_read_by_id()` (`routes.cppm:44`). Agree.
- Input: Ghost `data=['id','email']`, `options=['include']` (`invites.js:38-39`); C++ never reads `:id` or `email` — it calls `find_by_resource("invites", std::string{}.empty()?"1":std::string{})`, i.e. hardcoded `"1"` (`read_by_id/workflow/step/execute.cppm:16`).
- Output/data shape: Ghost returns a single `Invite` model (or 404 `Invite not found.` via `errors.NotFoundError`, `invites.js:46-55`); C++ returns `{"invites":[]}` for every request (`execute.cppm:16-18`). Materially different data shape and no 404.
- DB call: Ghost `models.Invite.findOne(frame.data, frame.options)` (`invites.js:47`); C++ `find_by_resource("invites","1")`. `find_by_resource` has branches for posts/pages/tags/users/members/offers/notifications/webhooks/emails/newsletters/tiers only — no `"invites"` — so it returns `std::nullopt` (`context.cppm:1055-1092`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:35-37`); C++ none invite-specific.
- Auth: Ghost `authAdminApi` + `permissions:true` (`routes.js:495`, `invites.js:45`); C++ token check → 401 (`read_by_id/handler.cppm:18-22`). No permission model.

### POST /invites — PARTIAL

- Route/verb: Ghost `router.post('/invites', mw.authAdminApi, staffInviteLimiter, http(api.invites.add))` (`routes.js:496-501`) ↔ C++ `.post(...)` on `/invites` (`routes.cppm:29-36`). Agree.
- Operation mapping: Ghost `add` (`invites.js:75`) ↔ `handler.invites_create()` (`routes.cppm:34`). Agree.
- Input: Ghost `options=['include','email']`, required `role_id` and `email` in `data`, `unsafeAttrs=['role_id']` (`invites.js:80-96`); C++ passes raw `in.body_as_string()` (default `"{}"` when empty) with no field validation (`create/workflow/step/execute.cppm:16`). Ghost also passes invoking user name/email to the service (`invites.js:103-106`); C++ ignores user.
- Output/status: Ghost `statusCode:201` (`invites.js:76`) with service result (`invites.js:97-108`); C++ `mutations.create` returns `MutationResult{201, {"invites":[{"id":"…"}]}}` (`context.cppm:1272-1276`) but the step keeps only `.body()` and the handler unconditionally sets `Status::OK` (200) (`execute.cppm:16`; `create/handler.cppm:27`; `context.cppm:34-49`). C++ also omits the `email`/`role_id`/`status` fields Ghost returns.
- DB call: Ghost `invites.service.add({InviteModel: models.Invite, …})` (`invites.js:3,98-101`) — service-level invite creation plus email delivery; C++ generic `ResourceStore::create("invites", body)` (`context.cppm:640-651`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:77-79`); C++ none invite-specific; generic store TTL never, tag `resources` (`context.cppm:645-646`).
- Auth: Ghost `authAdminApi` + `staffInviteLimiter` + `permissions:{unsafeAttrs}` (`routes.js:496-501`; `invites.js:94-96`); C++ token check only, no rate limiter or unsafeAttrs filtering (`create/handler.cppm:18-22`).

### DELETE /invites/:id — MISMATCH

- Route/verb: Ghost `router.delete('/invites/:id', …, http(api.invites.destroy))` (`routes.js:502`) ↔ C++ `Route{"/:id"}.delt(...)` (`routes.cppm:38,47-55`); `.delt()` maps to `Method::DELETE` (`utils/openapi/route.cppm:91-99`). Agree.
- Operation mapping: Ghost `destroy` (`invites.js:58`) ↔ `handler.invites_destroy()` (`routes.cppm:52`). Agree.
- Input: Ghost `options=['include','id']` with `include=[]` (`invites.js:63-68`); C++ never extracts `:id` and passes an empty string: `mutations.destroy("invites", std::string{})` (`destroy/workflow/step/execute.cppm:16`). The requested invite is therefore never targeted.
- Output/status: Ghost `statusCode:204`, no body (`invites.js:62`), 404 when missing via `require:true` (`invites.js:71`); C++ generic `destroy` returns `MutationResult{404, ApiError "Resource not found", true}` for the empty id (`context.cppm:1344-1347`), but the step keeps only `.body()` and handler sets 200 (`execute.cppm:16`; `destroy/handler.cppm:27`). Result: every DELETE returns 200 with a 404 error JSON body.
- DB call: Ghost `models.Invite.destroy({...frame.options, require:true})` (`invites.js:71`); C++ `ResourceStore::destroy("invites","")` → erase of `record_key("invites","")` fails (`context.cppm:665-672,1344-1347`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:59-61`); C++ none invite-specific; generic `ResourceStore::destroy` erases `record_key("invites",id)` (`context.cppm:667`).
- Auth: Ghost `authAdminApi` + `permissions:true` (`routes.js:502`; `invites.js:69`); C++ token check only (`destroy/handler.cppm:18-22`).

## Missing in C++

None. All four Ghost controller routes have a C++ route with the same path and verb.

## Extra in C++

None. `include/blog/handlers/auth/invites/routes.cppm` installs exactly GET `/invites`, POST `/invites`, GET `/invites/:id`, DELETE `/invites/:id` (`routes.cppm:19-55`); no `edit`/`PUT` invite route exists in C++ and none exists in the Ghost controller. Absence confirmed by grep for `invite` (case-insensitive) over `include/blog/handlers/**` — only the four handlers/routes listed above plus their flow/step files and registry references (`all.cppm:194-197,540-543,789-792`; `routes.cppm:25,98`); no other invite path or verb found.
