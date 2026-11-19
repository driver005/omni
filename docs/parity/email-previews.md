# Parity: email-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/email-previews.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/email_previews/routes.cppm` (handlers `get_detail/` + `post_detail/`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Controller exports two keys, `read` and `sendTestEmail` (`email-previews.js:7`, `:26`), with `docName: 'email_previews'` (`:5`). Both are bound in the admin URL table (`routes.js:547`, `:549-554`). Both have exact C++ path+verb counterparts under `membership/email_previews`; the C++ folder name was located by path/verb search, not assumed. No missing or extra routes.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/email_previews/posts/:id` | `read` → `emailService.controller.previewEmail` | `Router{"/email_previews"}/Router{"/posts"}/Route{"/:id"}.get()` | `email_previews_get_detail()` → `GetDetailHandler` | Ghost: `options[]` = `fields,member_status,member_tier,memberSegment,newsletter`; validation `fields∈{html,plaintext,subject}`, `member_status∈{free,paid}`; `data[]`=`id,status` (`email-previews.js:11-20`). C++: `using Input = IRequest&`; step reads **no** query/data params, ignores `:id` (`get_detail/.../execute.cppm:12-14`). | Ghost service returns `{subject,html,plaintext}` rendered from post+newsletter (`email-service.js:590-594`); framework wraps by docName. C++ emits `{"email_previews":[<stub>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with hardcoded `EmailPreview{}` = `subject:"Preview",html:"<p>Lorem ipsum.</p>",plaintext:"Lorem ipsum."` (`execute.cppm:17-23`, `subresources.cppm:10-32`). | Ghost: `Post.findOne(...,status:'all')` + `Newsletter.findOne/getDefaultNewsletter` + renderer/segmenter (`email-controller.js:76-104`, `email-service.js:578-588`). C++: `ctx.store().json_for("email_previews", {})` with empty params + canned response; no real post lookup/filter/pagination (`execute.cppm:16-23`). | Ghost: `headers.cacheInvalidate:false` → no invalidation (`email-previews.js:8-10`). C++: no `core::cache::Cache` use, no namespace/TTL/eviction. Both none. | Ghost: `mw.authAdminApi` (`routes.js:547`) + `permissions:true`/`docName:'email_previews'` (`email-previews.js:5,21`). C++: no auth middleware in route chain (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` check → 401 (`get_detail/handler.cppm:18-22`); no permission/docName check. | PARTIAL | Ghost: `email-previews.js:7-24`; `routes.js:547`. C++: `email_previews/routes.cppm:19-31`; `get_detail/handler.cppm:17-27`; `get_detail/workflow/step/execute.cppm:12-23`; `all.cppm:208,554,803`. |
| POST `/email_previews/posts/:id` | `sendTestEmail` → `emailService.controller.sendTestEmail` | `Router{"/email_previews"}/Router{"/posts"}/Route{"/:id"}.post()` | `email_previews_post_detail()` → `PostDetailHandler` | Ghost: `options[]`=`id` with validation `id.required:true` (`email-previews.js:31-38`); body `data.emails` (1..1 required) validated in service (`email-controller.js:121-134`). C++: `using Input = IRequest&`; reads raw `in.body_as_string()` only, no `id`/`emails` parsing or validation (`post_detail/.../execute.cppm:11-16`). | Ghost: `statusCode:204`, empty body, `cacheInvalidate:false` (`email-previews.js:27-30`); service sends via `#sendingService.send` and returns void (`email-service.js:614-627`). C++: always `set_status(OK=200)` with body from `mutations.create(...).body()` (JSON, not 204) (`post_detail/handler.cppm:25-27`, `execute.cppm:16`). | Ghost: `Post.findOne`+`Newsletter.findOne` then `#sendingService.send` (`email-controller.js:76-104,136`; `email-service.js:605-627`). C++: `mutations.create("email_previews", body)` — generic store create, no post/newsletter/send (`execute.cppm:16`). | Ghost: `headers.cacheInvalidate:false` (`email-previews.js:28-30`). C++: no cache. Both none. | Ghost: `previewEmailLimiter` rate limiter + `mw.authAdminApi` (`routes.js:549-554`) + `permissions:true` (`email-previews.js:39`). C++: no rate limiter, no auth middleware (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` → 401 (`post_detail/handler.cppm:18-22`); no permission check. | PARTIAL | Ghost: `email-previews.js:26-42`; `routes.js:549-554`. C++: `email_previews/routes.cppm:19-40`; `post_detail/handler.cppm:17-27`; `post_detail/workflow/step/execute.cppm:11-16`; `all.cppm:209,555,804`. |

## Route details

### GET /email_previews/posts/:id — PARTIAL

- **Route identity**: MATCH. Ghost `router.get('/email_previews/posts/:id', ...)` (`routes.js:547`) vs C++ `Router{router,"/email_previews"}.add_router(Router{router,"/posts"}.add_route(Route{"/:id"}.get(...)))` (`email_previews/routes.cppm:19-25`). `:id` placeholder agrees.
- **Operation mapping**: MATCH at intent. Ghost `api.email_previews.read` → controller `read` (`routes.js:547`, `email-previews.js:7`); C++ `handler.email_previews_get_detail()` (`routes.cppm:30`), accessor at `all.cppm:554`, instance at `all.cppm:803`.
- **Input**: DIFFER. Ghost accepts 5 options (`fields`, `member_status`, `member_tier`, `memberSegment`, `newsletter`), constrained `fields` and `member_status` values, and `data[id,status]` (`email-previews.js:11-20`); service reads `frame.options.id`/`frame.data`, newsletter, status/tier (`email-controller.js:63-99`). C++ `using Input=IRequest&` with no query/body parsing; `:id` ignored (`get_detail/../execute.cppm:12-14`).
- **Output**: DIFFER. Ghost returns rendered `{subject,html,plaintext}` (`email-service.js:590-594`). C++ returns fixed stub `EmailPreview{}` plus hardcoded `limit=15/page=1/pages=1/total=0` envelope (`execute.cppm:17-23`; `subresources.cppm:20-22`).
- **DB call**: DIFFER. Ghost `Post.findOne(status:'all')` + `Newsletter` lookup + renderer (`email-controller.js:82-104`). C++ `ctx.store().json_for("email_previews", {})` with empty params — no filtering by `:id`, no post resource (`execute.cppm:22`).
- **Cache**: no differences — both none. Ghost `cacheInvalidate:false` (`email-previews.js:8-10`); C++ no cache key/namespace/TTL.
- **Auth**: DIFFER. Ghost `mw.authAdminApi` + `permissions:true`/`docName` (`routes.js:547`; `email-previews.js:5,21`). C++ route chain has only `content_type_middleware` (`routes.cppm:20`); a manual `token(req).empty()` check returns 401 (`get_detail/handler.cppm:18-22`), with no permission/docName enforcement.

### POST /email_previews/posts/:id — PARTIAL

- **Route identity**: MATCH. Ghost `router.post('/email_previews/posts/:id', ...)` (`routes.js:549-554`) vs C++ same `Route{"/:id"}.post()` (`email_previews/routes.cppm:24,33-39`).
- **Operation mapping**: MATCH at intent. Ghost `api.email_previews.sendTestEmail` → controller `sendTestEmail` (`routes.js:553`; `email-previews.js:26`); C++ `handler.email_previews_post_detail()` (`routes.cppm:38`), accessor `all.cppm:555`, instance `all.cppm:804`.
- **Input**: DIFFER. Ghost `options[id]` with `id.required` and body `emails` validated to exactly 1 address (`email-previews.js:31-38`; `email-controller.js:121-134`). C++ `using Input=IRequest&` passes raw `body_as_string()` straight to the store with no `id`/`emails` parsing or validation (`post_detail/../execute.cppm:11-16`).
- **Output**: DIFFER. Ghost `statusCode:204`, no body (`email-previews.js:27`); service returns void (`email-service.js:605-627`). C++ `set_status(OK=200)` and a JSON body from `mutations.create(...).body()` (`post_detail/handler.cppm:25-27`; `execute.cppm:16`).
- **DB call**: DIFFER. Ghost `Post.findOne`+`Newsletter.findOne` then `this.#sendingService.send(...)` (`email-controller.js:82-104,136`; `email-service.js:614-627`). C++ `mutations.create("email_previews", ...)` — generic store create, no send operation (`execute.cppm:16`).
- **Cache**: no differences — both none. Ghost `cacheInvalidate:false` (`email-previews.js:28-30`); C++ no cache behavior.
- **Auth**: DIFFER. Ghost adds `shared.middleware.rateLimiting.previewEmailLimiter` (abuse prevention) + `mw.authAdminApi` (`routes.js:551-552`) + `permissions:true` (`email-previews.js:39`). C++ has no rate limiter and no auth middleware (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` check → 401 (`post_detail/handler.cppm:18-22`), no permission enforcement.

## Missing in C++

None. Both Ghost controller routes (`GET` and `POST` `/email_previews/posts/:id`) have C++ counterparts. Confirmed by path/verb search — C++ `Route{"/:id"}.get()`/`.post()` under `/email_previews/posts` (`email_previews/routes.cppm:19-40`) and the canonical catalog `RouteInfo{"GET","/email_previews/posts/:id",...}` / `RouteInfo{"POST","/email_previews/posts/:id",...}` (`include/blog/testing/route_catalog.cppm:250-251`). This is an admin-API domain, not public/content, so MISSING was not expected.

## Extra in C++

None. The matched folder `include/blog/handlers/membership/email_previews/` installs exactly the two routes that exist on the Ghost side (`routes.cppm:24-41`: one `/:id` route carrying `.get()` and `.post()`), registered centrally at `include/blog/handlers/routes.cppm:13` and in the handler registry at `all.cppm:208-209,554-555,803-804`. No additional C++ routes in that folder lack a Ghost counterpart.
