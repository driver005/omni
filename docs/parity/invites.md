# Parity: invites

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/invites.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/auth/invites/routes.cppm` (installed via `include/blog/handlers/routes.cppm:25,98`; handlers registered in `include/blog/handlers/all.cppm:194-197,540-543,789-792`)
- Checked: 2026-11-19
- Strict-parity rewrite: 2026-11-19 (shared partitions: `core_query`, `core_auth:ghost`,
  `blog_handlers:mapping`, context store, `blog_handlers:outbox_shim`). All four route verdicts
  MATCH in `docs/parity/behavior/invites.md`. No shared file edited; Bazel cycle untouched.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Applied source fixes (2026-11-19)

All four `#### Fix` blocks applied, then extended to strict parity with the registered shared
partitions. Files edited (only under `include/blog/handlers/auth/invites/`):
- `routes.cppm` — unchanged (already installs `content_type`/`bearer`/`require_auth`; permission gate now lives per-handler).
- `types/dto_admin_resources.cppm` — added `blog::handlers::invites::support` helpers (clean/wrap,
  token/expires/timestamps, outbox emit, Location) built on `core_query`, `core_auth`,
  `blog_handlers:mapping`, `:domain_all`, `:outbox_shim`.
- `browse/{handler.cppm,workflow/step/execute.cppm}` — `core::query::parse/run` with restricted-field guard (400),
  `include` rejection (422), token-stripping `clean`, `blog::mapping::wrap/pagination`,
  `require_ghost_permission(invites,browse)`, no `X-Cache-Invalidate` header.
- `read_by_id/{handler.cppm,workflow/step/execute.cppm}` — path `:id` + `?email=` through
  `core::query::apply` data-key filters, `{"invites":[...]}` envelope, 404 `Invite not found.`,
  token stripped, permission gate, no cache header.
- `create/{handler.cppm,workflow/step/execute.cppm}` — unwrap `data.invites[0]`, required-field and
  `include` validation, `GhostAuthorizer` unsafeAttrs + Owner-role guard, duplicate-by-email
  destroy, token + expires(1w) + status pending→sent, full-model output + `Location`,
  `mail.send`/`invite.created` outbox events, permission gate, no cache header.
- `destroy/{handler.cppm,workflow/step/execute.cppm}` — real path `:id`, `include` check,
  store 200 remapped to 204 empty body, 404 `Invite not found.`, `invite.destroyed` outbox
  event, permission gate, no cache header.

Accepted residual gaps (no allowed primitive): real Knex/Bookshelf CRUD, `staffInviteLimiter`
and its `settings` counter transaction, real SMTP delivery / `EmailError` rewrite, full
role-allowlist `permissible`, real signed token. All are side-effect/internals only; the primary
observable contract for all four routes now agrees with Ghost. No shared files edited; no new
global state.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/invites` | `browse` | GET `/invites` (`routes.cppm:23-30`) | `invites_browse()` → `BrowseHandler` | Ghost `options=[include,page,limit,fields,filter,order,debug]`, `include=[]`; C++ `core::query::parse`/`run` (restricted-field guard → 400, `include` → 422) | `{invites:[...],meta.pagination}` via `blog::mapping::wrap/pagination`; token stripped by `clean` | `store().json_for("invites",{})` filtered/sorted/paginated by `core::query::run` | no `X-Cache-Invalidate` header | `require_ghost_permission(invites,browse)` | MATCH | Ghost `invites.js:18-32` ↔ C++ `browse/handler.cppm`, `browse/.../execute.cppm` |
| GET `/invites/:id` | `read` | GET `/invites/:id` (`routes.cppm:41-48`) | `invites_read_by_id()` → `ReadByIdHandler` | Ghost `options=[include]`, `data=[id,email]`; C++ path `:id` + `?email=` via `core::query` | `{"invites":[{...}]}` (token stripped); 404 `Invite not found.` | `core::query::apply` filter over `store().json_for("invites",{})` | no `X-Cache-Invalidate` header | `require_ghost_permission(invites,read)` | MATCH | Ghost `invites.js:34-56` ↔ C++ `read_by_id/handler.cppm`, `read_by_id/.../execute.cppm` |
| POST `/invites` | `add` | POST `/invites` (`routes.cppm:31-38`) | `invites_create()` → `CreateHandler` | Ghost `options=[include,email]`, required `role_id`/`email`, `unsafeAttrs=[role_id]`; C++ validates + `GhostAuthorizer` unsafeAttrs/Owner guard | 201 full model (`clean` strips token) + `Location`; 422/403 mapped | token/expires(1w)/status pending→sent, duplicate-by-email destroy, `mail.send` outbox event | no `X-Cache-Invalidate` header | `require_ghost_permission(invites,add)` | MATCH | Ghost `invites.js:75-109` ↔ C++ `create/handler.cppm`, `create/.../execute.cppm` |
| DELETE `/invites/:id` | `destroy` | DELETE `/invites/:id` (`routes.cppm:49-56`) | `invites_destroy()` → `DestroyHandler` | Ghost `options=[include,id]`, `include=[]`; C++ path `:id` via `core::query::parse` | **204** empty body; 404 `Invite not found.` | `store().destroy("invites",id)` + `invite.destroyed` outbox event | no `X-Cache-Invalidate` header | `require_ghost_permission(invites,destroy)` | MATCH | Ghost `invites.js:58-73` ↔ C++ `destroy/handler.cppm`, `destroy/.../execute.cppm` |

## Route details

> The per-route "C++" bullets and `#### Fix` blocks below record the original analysis and the
> first fix pass. The strict-parity rewrite (2026-11-19) supersedes their C++ descriptions —
> see **Applied source fixes** above and `docs/parity/behavior/invites.md` for current behavior.

### GET /invites — MATCH

- Route/verb: Ghost `router.get('/invites', …, http(api.invites.browse))` (`routes.js:494`) ↔ C++ `Router{router,"/invites"}.get(...)` (`routes.cppm:19-28`). Agree.
- Operation mapping: Ghost `browse` (`invites.js:18`) ↔ `handler.invites_browse()` (`routes.cppm:26`). Agree.
- Input: Ghost declares `include,page,limit,fields,filter,order,debug` with `include` restricted to `ALLOWED_INCLUDES=[]` (`invites.js:7,22-27`); C++ step declares `Input=IRequest&` but parses no query string (`browse/workflow/step/execute.cppm:12-14`). All Ghost query param names missing in C++.
- Output: Ghost returns `Invite.findPage(...)` through the api-framework serializer with `meta.pagination` (`invites.js:29-31`); C++ builds a `dto::InvitesResponse` containing one default `InviteDto{id:"1",email:"invite@example.com",status:"sent"}` (`execute.cppm:17-19`; defaults at `include/blog/dto/admin_resources.cppm:32-52`) and `encode`s it (`execute.cppm:19`, `middleware/serde.cppm:155-157`). Because `r.first=200` is always `!=0`, line 21 returns early and the `json_for` branch (line 22-23) is unreachable: C++ always emits one hardcoded invite and no `meta`.
- DB call: Ghost `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))` (`invites.js:30`); C++ `ctx.store().json_for("invites",{})` is dead code (`execute.cppm:21-23`); generic `json_for` fallback would be `parse_array(m_resources.list("invites"))` with no filtering (`context.cppm:1180-1181`).
- Cache: Ghost `headers.cacheInvalidate:false` (`invites.js:19-21`) — no invalidation. C++ invites code: none. Only the generic `ResourceStore` uses `record_key("invites",id)`, tag `resources`, TTL never (`context.cppm:645-646,725`).
- Auth: Ghost `mw.authAdminApi` (`routes.js:494`) + `permissions:true` (`invites.js:28`); C++ router-level `content_type_middleware` (`routes.cppm:20`) and per-handler bearer `token(req)` check → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; `middleware/bearer.cppm:27-33`). No Ghost-style permission docName/unsafeAttrs equivalent.

#### Fix

Target: `include/blog/handlers/auth/invites/browse/workflow/step/execute.cppm` — `blog::handlers::invites::browse::workflow::ExecuteStep::invoke` (`:13-23`); handler `include/blog/handlers/auth/invites/browse/handler.cppm:17-28` — `BrowseHandler::handle`; router `include/blog/handlers/auth/invites/routes.cppm:18-28`; store `blog::api::ApiDataStore::json_for` / `total` in `include/blog/context/context.cppm:1139-1198,1127-1137`.

Exact replacements (checks 1–8 closed):
- **1, 2 (route/op): no change.** `GET /invites` agrees (`routes.js:494` ↔ `routes.cppm:19-27`; `handler.invites_browse()` at `:25`). Ghost `browse` (`invites.js:18`) maps to this handler; registry `all.cppm:540` stays.
- **3 (input):** Ghost declares `options:['include','page','limit','fields','filter','order','debug']` with `include` restricted to `ALLOWED_INCLUDES=[]` (`invites.js:7,22-27`). Replace the unused `in` with the allowlist query parser from `admin/posts/browse/.../execute.cppm:21-29` — split `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='` into `std::map<std::string,std::string> params`; read only those seven keys. Because `ALLOWED_INCLUDES` is empty, reject any non-empty `include` with `Status::UNPROCESSABLE_CONTENT` + `ApiError{}.validation("Validation error", "include is not supported", "include")` (`context.cppm:92-100`). No `data[]`/multipart on either side.
- **5 (DB call):** delete the hardcoded early return (`execute.cppm:15-21`) that constructs `blog::dto::InvitesResponse` with one default `InviteDto{id:"1",email:"invite@example.com",status:"sent"}` and always returns `{200,...}` — it makes the real store call dead code. Replace with `const auto rows = ctx.store().json_for("invites", params);` (`:21`). Resource string `"invites"` stays; `json_for` falls through to `search_json(records, params)` honoring `filter/limit/page/order/fields` (`context.cppm:1196-1197,1599-1600`), and `total("invites")` falls through to `m_resources.count("invites")` (`:1127-1136`). This closes the Ghost `models.Invite.findPage(restrictAdminApiQueryOptions(frame.options))` call (`invites.js:30`) at the allowed-primitive level.
- **4 (output):** replace hardcoded pagination (`execute.cppm:22`) with real values: `total = ctx.store().total("invites")`, `limit`/`page` from `params` (Ghost defaults), `pages=(total+limit-1)/limit`, `prev`/`next` null-aware. Keep envelope `{"invites":[...],"meta":{"pagination":{...}}}` (`invites.js:29-31`) and `Status::OK` (`handler.cppm:27`).
- **6 (cache header):** add `res.set_header("X-Cache-Invalidate", "false");` before `send()` in `BrowseHandler::handle` (Ghost `headers.cacheInvalidate:false`, `invites.js:19-21`). Generic `ResourceStore` tag `resources` / TTL never (`context.cppm:645-646`) is unchanged; no invite-specific namespace introduced.
- **7 (auth):** at `routes.cppm:19` add `.use(blog::handlers::middleware::bearer_middleware)` and `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware`, and delete the inline 401 branch in `handler.cppm:18-22` (`middleware/bearer.cppm:11-24`, `middleware/auth.cppm:12-29`). Ghost `permissions:true` (`invites.js:28`) has no allowed C++ primitive — accepted residual gap.
- **8 (evidence/verdict):** evidence unchanged; path/verb/op, seven input names with `include` validation, `{invites,meta}` output with real pagination, param-filtered store call with real total, cache header and auth agree → MATCH.

```cpp
// invites/browse/workflow/step/execute.cppm — ExecuteStep::invoke
std::map<std::string, std::string> params;  // parse the 7 allowed options from in.get_path()
if (params.contains("include") && !params["include"].empty())
    return core::workflow::StepResponse<Output>{blog::api::ApiError{}.validation(
        "Validation error", "include is not supported", "include")};
const auto rows  = ctx.store().json_for("invites", params);   // delete hardcoded InvitesResponse block
const auto total = ctx.store().total("invites");
// meta.pagination = {limit, page, pages=(total+limit-1)/limit, prev, next, total}
```

### GET /invites/:id — MATCH

- Route/verb: Ghost `router.get('/invites/:id', …, http(api.invites.read))` (`routes.js:495`) ↔ C++ `Route{"/:id"}.get(...)` (`routes.cppm:38-45`). Agree.
- Operation mapping: Ghost `read` (`invites.js:34`) ↔ `handler.invites_read_by_id()` (`routes.cppm:44`). Agree.
- Input: Ghost `data=['id','email']`, `options=['include']` (`invites.js:38-39`); C++ never reads `:id` or `email` — it calls `find_by_resource("invites", std::string{}.empty()?"1":std::string{})`, i.e. hardcoded `"1"` (`read_by_id/workflow/step/execute.cppm:16`).
- Output/data shape: Ghost returns a single `Invite` model (or 404 `Invite not found.` via `errors.NotFoundError`, `invites.js:46-55`); C++ returns `{"invites":[]}` for every request (`execute.cppm:16-18`). Materially different data shape and no 404.
- DB call: Ghost `models.Invite.findOne(frame.data, frame.options)` (`invites.js:47`); C++ `find_by_resource("invites","1")`. `find_by_resource` has branches for posts/pages/tags/users/members/offers/notifications/webhooks/emails/newsletters/tiers only — no `"invites"` — so it returns `std::nullopt` (`context.cppm:1055-1092`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:35-37`); C++ none invite-specific.
- Auth: Ghost `authAdminApi` + `permissions:true` (`routes.js:495`, `invites.js:45`); C++ token check → 401 (`read_by_id/handler.cppm:18-22`). No permission model.

#### Fix

Target: `include/blog/handlers/auth/invites/read_by_id/workflow/step/execute.cppm:12-18` — `blog::handlers::invites::read_by_id::workflow::ExecuteStep::invoke`; handler `include/blog/handlers/auth/invites/read_by_id/handler.cppm:17-28` — `ReadByIdHandler::handle`; router `include/blog/handlers/auth/invites/routes.cppm:36-54`; store `blog::api::ApiDataStore::find_by_resource` in `include/blog/context/context.cppm:1071-1108` (no `"invites"` branch) and `json_for` (`:1139-1198`). Change step `Output` from `std::string` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern from `posts/create/workflow/step/execute.cppm:13,21-24`).

Exact replacements (checks 1–8 closed):
- **1, 2 (route/op): no change.** `GET /invites/:id` agrees (`routes.js:495` ↔ `routes.cppm:37-45`; `handler.invites_read_by_id()` at `:43`). Ghost `read` (`invites.js:34`) maps to this handler.
- **3 (input):** replace the hardcoded id `std::string{}.empty()?"1":std::string{}` (`execute.cppm:15`) with the path segment parsed from `in.get_path()` — take the last `/`-separated segment (`std::string{path}.substr(path.rfind('/')+1)`), the `posts/browse` parse style (`execute.cppm:22-28`). Ghost also accepts `data:['id','email']` (`invites.js:39`): additionally read `?email=` from the query into a `params` map and use it as a fallback lookup key. Reject an empty id/email with `Status::UNPROCESSABLE_CONTENT` + `ApiError{}.validation(...)`.
- **5 (DB call):** `find_by_resource("invites","1")` currently always returns `nullopt` because `"invites"` is absent from the branch list (`context.cppm:1071-1108`). Either add a `"invites"` branch returning the matching record from `resources().list("invites")` encoded with `core_serde::JsonCodec` (as the `"tiers"` branch does at `:1102-1106`), or call `ctx.store().json_for("invites", {{"id", id}})` / `{{"email", email}}` and take the first row — `json_for` falls through to `search_json` which honors those params (`:1196-1197,1599-1600`). This closes Ghost `models.Invite.findOne(frame.data, frame.options)` (`invites.js:47`) at the allowed-primitive level.
- **4 (output/status):** Ghost returns one `Invite` model or a 404 `Invite not found.` (`invites.js:46-55`). Replace `{"invites":[...]}` / `{"invites":[]}` (`execute.cppm:16-18`) with the single matched row object; on miss return `ApiError{}.json("Invite not found.", "Not Found")` (`context.cppm:82-89`) with `status=404`. In `ReadByIdHandler::handle` replace the unconditional `Status::OK` (`handler.cppm:27`) with the step's `status` (200 or 404) before `send()`.
- **6 (cache header):** add `res.set_header("X-Cache-Invalidate", "false");` before `send()` in the handler (Ghost `cacheInvalidate:false`, `invites.js:35-37`).
- **7 (auth):** at `routes.cppm:19` add `.use(blog::handlers::middleware::bearer_middleware)` and `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware`, and delete the inline 401 branch in `read_by_id/handler.cppm:18-22` (`middleware/bearer.cppm:11-24`, `middleware/auth.cppm:12-29`). Ghost `permissions:true` (`invites.js:45`) remains an accepted residual gap.
- **8 (evidence/verdict):** evidence unchanged; path/verb/op, id+email input, single-model/404 output, real `"invites"` lookup, cache header and auth agree → MATCH.

```cpp
// invites/read_by_id/workflow/step/execute.cppm — ExecuteStep::invoke
using Output = std::map<std::string, std::string>;
const auto path = std::string{in.get_path()};
const std::string id = path.substr(path.rfind('/') + 1);   // never hardcoded "1"
const auto one = ctx.store().find_by_resource("invites", id);   // add the "invites" branch
if (!one) return core::workflow::StepResponse<Output>{{{"status", "404"},
    {"body", blog::api::ApiError{}.json("Invite not found.", "Not Found")}}};
return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", *one}}};
```

### POST /invites — MATCH

- Route/verb: Ghost `router.post('/invites', mw.authAdminApi, staffInviteLimiter, http(api.invites.add))` (`routes.js:496-501`) ↔ C++ `.post(...)` on `/invites` (`routes.cppm:29-36`). Agree.
- Operation mapping: Ghost `add` (`invites.js:75`) ↔ `handler.invites_create()` (`routes.cppm:34`). Agree.
- Input: Ghost `options=['include','email']`, required `role_id` and `email` in `data`, `unsafeAttrs=['role_id']` (`invites.js:80-96`); C++ passes raw `in.body_as_string()` (default `"{}"` when empty) with no field validation (`create/workflow/step/execute.cppm:16`). Ghost also passes invoking user name/email to the service (`invites.js:103-106`); C++ ignores user.
- Output/status: Ghost `statusCode:201` (`invites.js:76`) with service result (`invites.js:97-108`); C++ `mutations.create` returns `MutationResult{201, {"invites":[{"id":"…"}]}}` (`context.cppm:1272-1276`) but the step keeps only `.body()` and the handler unconditionally sets `Status::OK` (200) (`execute.cppm:16`; `create/handler.cppm:27`; `context.cppm:34-49`). C++ also omits the `email`/`role_id`/`status` fields Ghost returns.
- DB call: Ghost `invites.service.add({InviteModel: models.Invite, …})` (`invites.js:3,98-101`) — service-level invite creation plus email delivery; C++ generic `ResourceStore::create("invites", body)` (`context.cppm:640-651`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:77-79`); C++ none invite-specific; generic store TTL never, tag `resources` (`context.cppm:645-646`).
- Auth: Ghost `authAdminApi` + `staffInviteLimiter` + `permissions:{unsafeAttrs}` (`routes.js:496-501`; `invites.js:94-96`); C++ token check only, no rate limiter or unsafeAttrs filtering (`create/handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/auth/invites/create/workflow/step/execute.cppm:12-16` — `blog::handlers::invites::create::workflow::ExecuteStep::invoke`; handler `include/blog/handlers/auth/invites/create/handler.cppm:17-28` — `CreateHandler::handle`; router `include/blog/handlers/auth/invites/routes.cppm:28-35`; store `blog::api::ApiDataStore::create` in `include/blog/context/context.cppm:1222-1293` (generic branch returns `MutationResult{201, {"invites":[{"id":...}]}}` at `:1288-1292`). Change step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern from `posts/create/workflow/step/execute.cppm:13,21-24`).

Exact replacements (checks 1–8 closed):
- **1, 2 (route/op): no change.** `POST /invites` agrees (`routes.js:496-501` ↔ `routes.cppm:28-35`; `handler.invites_create()` at `:33`). Ghost `add` (`invites.js:75`) maps to this handler.
- **3 (input):** Ghost requires `data.role_id` and `data.email`, accepts `options:['include','email']`, and marks `role_id` unsafe (`invites.js:80-96`); it passes the invoking user's name/email to the service (`invites.js:103-106`). Replace the raw-passthrough `mutations.create("invites", in.body_as_string().empty()?"{}":in.body_as_string())` (`execute.cppm:15`) with: parse the body with `core_serde::JsonCodec{}.decode<serde::Value>` (`json_codec.cppm:21-29`), unwrap `data.invites[0]`, and validate that `role_id` and `email` are present and non-empty; on failure return `Status::UNPROCESSABLE_CONTENT` + `ApiError{}.validation("Validation error", "<field> is required", "<field>")`. Parse `?include=` and reject any non-empty value (`ALLOWED_INCLUDES=[]`, `invites.js:7,83`). Keep the invoking user out of the created record (no user model access allowed).
- **5 (DB call):** keep `mutations.create("invites", innerJson)` — resource string `"invites"` already agrees and falls through to `ResourceStore::create` (`context.cppm:1288`). Ghost's `invites.service.add` + email delivery (`invites.js:3,98-101`) has no allowed C++ service boundary; document the missing email side effect as an accepted residual gap. No direct SQL.
- **4 (output/status):** the store already returns `MutationResult{201, ...}` (`context.cppm:1288-1292`) but the step keeps only `.body()` and the handler hardcodes `Status::OK` (`handler.cppm:27`). Return `{{"status", std::to_string(result.status())}, {"body", result.body()}}` from the step; in `CreateHandler::handle` set `res.set_status(Status::CREATED)` when `status=="201"`, `Status::UNPROCESSABLE_CONTENT` with the validation body when `!result.ok()`, else `Status::OK`. Include `role_id`/`email`/`status` fields in the created payload by passing them through `innerJson` (Ghost returns the created invite, `invites.js:97-108`).
- **6 (cache header):** add `res.set_header("X-Cache-Invalidate", "false");` before `send()` in the handler (Ghost `cacheInvalidate:false`, `invites.js:77-79`). Generic store TTL never / tag `resources` (`context.cppm:645-646`) unchanged.
- **7 (auth):** at `routes.cppm:19` add `.use(blog::handlers::middleware::bearer_middleware)` and `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware`, and delete the inline 401 branch in `create/handler.cppm:18-22` (`middleware/bearer.cppm:11-24`, `middleware/auth.cppm:12-29`). Ghost `staffInviteLimiter` (`routes.js:496-501`) and `permissions.unsafeAttrs=['role_id']` (`invites.js:94-96`) have no allowed C++ primitive — accepted residual gaps; no new global rate-limit state.
- **8 (evidence/verdict):** evidence unchanged; path/verb/op, validated required-field input, 201/created-model output, `"invites"` create call, cache header and auth agree → MATCH (with documented email/limiter residual).

```cpp
// invites/create/workflow/step/execute.cppm — ExecuteStep::invoke
using Output = std::map<std::string, std::string>;
const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
// unwrap parsed["data"]["invites"][0]; require non-empty "role_id" and "email"
const auto result = mutations.create("invites", innerJson);   // no "{}" fallback
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
                                             {"body", result.body()}}};
// invites/create/handler.cppm — CreateHandler::handle
res.set_header("X-Cache-Invalidate", "false");
res.set_status(status == "201" ? interfaces::io::types::Status::CREATED
             : status == "422" ? interfaces::io::types::Status::UNPROCESSABLE_CONTENT
                               : interfaces::io::types::Status::OK);
```

### DELETE /invites/:id — MATCH

- Route/verb: Ghost `router.delete('/invites/:id', …, http(api.invites.destroy))` (`routes.js:502`) ↔ C++ `Route{"/:id"}.delt(...)` (`routes.cppm:38,47-55`); `.delt()` maps to `Method::DELETE` (`utils/openapi/route.cppm:91-99`). Agree.
- Operation mapping: Ghost `destroy` (`invites.js:58`) ↔ `handler.invites_destroy()` (`routes.cppm:52`). Agree.
- Input: Ghost `options=['include','id']` with `include=[]` (`invites.js:63-68`); C++ never extracts `:id` and passes an empty string: `mutations.destroy("invites", std::string{})` (`destroy/workflow/step/execute.cppm:16`). The requested invite is therefore never targeted.
- Output/status: Ghost `statusCode:204`, no body (`invites.js:62`), 404 when missing via `require:true` (`invites.js:71`); C++ generic `destroy` returns `MutationResult{404, ApiError "Resource not found", true}` for the empty id (`context.cppm:1344-1347`), but the step keeps only `.body()` and handler sets 200 (`execute.cppm:16`; `destroy/handler.cppm:27`). Result: every DELETE returns 200 with a 404 error JSON body.
- DB call: Ghost `models.Invite.destroy({...frame.options, require:true})` (`invites.js:71`); C++ `ResourceStore::destroy("invites","")` → erase of `record_key("invites","")` fails (`context.cppm:665-672,1344-1347`).
- Cache: Ghost `cacheInvalidate:false` (`invites.js:59-61`); C++ none invite-specific; generic `ResourceStore::destroy` erases `record_key("invites",id)` (`context.cppm:667`).
- Auth: Ghost `authAdminApi` + `permissions:true` (`routes.js:502`; `invites.js:69`); C++ token check only (`destroy/handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/auth/invites/destroy/workflow/step/execute.cppm:12-16` — `blog::handlers::invites::destroy::workflow::ExecuteStep::invoke`; handler `include/blog/handlers/auth/invites/destroy/handler.cppm:17-28` — `DestroyHandler::handle`; router `include/blog/handlers/auth/invites/routes.cppm:46-53`; store `blog::api::ApiDataStore::destroy` in `include/blog/context/context.cppm:1343-1369` (404 `ApiError` on miss, 204 for labels, 200 `"{}"` otherwise). Change step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern from `posts/create/workflow/step/execute.cppm:13,21-24`).

Exact replacements (checks 1–8 closed):
- **1, 2 (route/op): no change.** `DELETE /invites/:id` (`.delt()` = DELETE, `utils/openapi/route.cppm:91-99`) agrees (`routes.js:502` ↔ `routes.cppm:46-53`; `handler.invites_destroy()` at `:51`). Ghost `destroy` (`invites.js:58`) maps to this handler.
- **3 (input):** replace the empty id `mutations.destroy("invites", std::string{})` (`execute.cppm:15`) with the path segment parsed from `in.get_path()` last `/`-separated component (`std::string{path}.substr(path.rfind('/')+1)`), the read-by-id parse style. Parse `?include=` and reject any non-empty value (`ALLOWED_INCLUDES=[]`, `invites.js:63,66`); reject an empty id with `Status::UNPROCESSABLE_CONTENT` + `ApiError{}.validation(...)`. Ghost `require:true` (`invites.js:71`) is already the store's behavior (404 on miss, `context.cppm:1362-1364`) — no extra code.
- **5 (DB call):** keep `mutations.destroy("invites", id)` with the real id — resource string `"invites"` agrees and falls through to `ResourceStore::destroy` (`context.cppm:1360`, erasing `record_key("invites",id)`). With `""` gone, the 404-on-miss path is no longer guaranteed; a real targeted delete now occurs.
- **4 (output/status):** Ghost returns `statusCode:204`, empty body, 404 when missing (`invites.js:62,70-72`). Return `{{"status", std::to_string(result.status())}, {"body", result.body()}}` from the step; in `DestroyHandler::handle` replace hardcoded `Status::OK` (`handler.cppm:27`) with: `Status::NO_CONTENT` and an empty body for `204`, `Status::NOT_FOUND` with `result.body()` for `404`, else `Status::OK`. Currently every DELETE returns 200 with a 404 error JSON body — this closes that.
- **6 (cache header):** add `res.set_header("X-Cache-Invalidate", "false");` before `send()` in the handler (Ghost `cacheInvalidate:false`, `invites.js:59-61`). Generic `ResourceStore::destroy` erase of `record_key("invites",id)` (`context.cppm:1360`) is unchanged.
- **7 (auth):** at `routes.cppm:19` add `.use(blog::handlers::middleware::bearer_middleware)` and `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware`, and delete the inline 401 branch in `destroy/handler.cppm:18-22` (`middleware/bearer.cppm:11-24`, `middleware/auth.cppm:12-29`). Ghost `permissions:true` (`invites.js:69`) remains an accepted residual gap.
- **8 (evidence/verdict):** evidence unchanged; path/verb/op, path-id input, 204/empty + 404 output, targeted `"invites"` destroy, cache header and auth agree → MATCH.

```cpp
// invites/destroy/workflow/step/execute.cppm — ExecuteStep::invoke
using Output = std::map<std::string, std::string>;
const auto path = std::string{in.get_path()};
const std::string id = path.substr(path.rfind('/') + 1);   // never empty string
const auto result = mutations.destroy("invites", id);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
                                             {"body", result.body()}}};
// invites/destroy/handler.cppm — DestroyHandler::handle
res.set_header("X-Cache-Invalidate", "false");
res.set_status(status == "204" ? interfaces::io::types::Status::NO_CONTENT
             : status == "404" ? interfaces::io::types::Status::NOT_FOUND
                               : interfaces::io::types::Status::OK);
if (status == "204") { send(); return; }  // empty body
```

## Missing in C++

None. All four Ghost controller routes have a C++ route with the same path and verb.

  Fix: No change required. Check 8 (MISSING) closed — all four routes already have exact path+verb registrations (`routes.cppm:19-55`) and handlers (`all.cppm:194-197,540-543,789-792`); nothing to add. The four `#### Fix` blocks above only reshape the existing `BrowseHandler` / `ReadByIdHandler` / `CreateHandler` / `DestroyHandler` + `ExecuteStep` + `InvitesRoutes` behavior. No new global state.

## Extra in C++

None. `include/blog/handlers/auth/invites/routes.cppm` installs exactly GET `/invites`, POST `/invites`, GET `/invites/:id`, DELETE `/invites/:id` (`routes.cppm:19-55`); no `edit`/`PUT` invite route exists in C++ and none exists in the Ghost controller. Absence confirmed by grep for `invite` (case-insensitive) over `include/blog/handlers/**` — only the four handlers/routes listed above plus their flow/step files and registry references (`all.cppm:194-197,540-543,789-792`; `routes.cppm:25,98`); no other invite path or verb found.

  Fix: No change required. Check 8 (EXTRA) closed — `InvitesRoutes::install` (`routes.cppm:14-55`) registers only the four counterparts of `api.invites.browse/read/add/destroy` (`routes.js:494-502`); nothing to delete or gate. Keep all four handler registrations; the Fix blocks above change behavior only. No new global state.
