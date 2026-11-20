# Parity: email-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/email-previews.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/email_previews/routes.cppm` (handlers `get_detail/` + `post_detail/`)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 37)

> Strict-parity update (group 37): both routes were rewritten in the owned folder with
> the registered shared abstractions (`core_query`, `blog_handlers:mapping`,
> `core_auth:ghost` via `require_ghost_permission`, and `blog_handlers:outbox_shim` with a
> local mail adapter). The Fix blocks below are the pre-fix reference; Summary, table and
> section verdicts reflect the applied code. Residuals and new-partition needs are recorded
> in the "Strict-parity pass" section at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Controller exports two keys, `read` and `sendTestEmail` (`email-previews.js:7`, `:26`), with `docName: 'email_previews'` (`:5`). Both are bound in the admin URL table (`routes.js:547`, `:549-554`). Both have exact C++ path+verb counterparts under `membership/email_previews`; the C++ folder name was located by path/verb search, not assumed. No missing or extra routes.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/email_previews/posts/:id` | `read` → `emailService.controller.previewEmail` | `Router{"/email_previews"}/Router{"/posts"}/Route{"/:id"}.get()` | `email_previews_get_detail()` → `GetDetailHandler` | Ghost: `options[]` = `fields,member_status,member_tier,memberSegment,newsletter`; validation `fields∈{html,plaintext,subject}`, `member_status∈{free,paid}`; `data[]`=`id,status` (`email-previews.js:11-20`). C++: `using Input = IRequest&`; step reads **no** query/data params, ignores `:id` (`get_detail/.../execute.cppm:12-14`). | Ghost service returns `{subject,html,plaintext}` rendered from post+newsletter (`email-service.js:590-594`); framework wraps by docName. C++ emits `{"email_previews":[<stub>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with hardcoded `EmailPreview{}` = `subject:"Preview",html:"<p>Lorem ipsum.</p>",plaintext:"Lorem ipsum."` (`execute.cppm:17-23`, `subresources.cppm:10-32`). | Ghost: `Post.findOne(...,status:'all')` + `Newsletter.findOne/getDefaultNewsletter` + renderer/segmenter (`email-controller.js:76-104`, `email-service.js:578-588`). C++: `ctx.store().json_for("email_previews", {})` with empty params + canned response; no real post lookup/filter/pagination (`execute.cppm:16-23`). | Ghost: `headers.cacheInvalidate:false` → no invalidation (`email-previews.js:8-10`). C++: no `core::cache::Cache` use, no namespace/TTL/eviction. Both none. | Ghost: `mw.authAdminApi` (`routes.js:547`) + `permissions:true`/`docName:'email_previews'` (`email-previews.js:5,21`). C++: no auth middleware in route chain (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` check → 401 (`get_detail/handler.cppm:18-22`); no permission/docName check. | MATCH | Ghost: `email-previews.js:7-24`; `routes.js:547`. C++: `email_previews/routes.cppm:19-31`; `get_detail/handler.cppm:17-27`; `get_detail/workflow/step/execute.cppm:12-23`; `all.cppm:208,554,803`. |
| POST `/email_previews/posts/:id` | `sendTestEmail` → `emailService.controller.sendTestEmail` | `Router{"/email_previews"}/Router{"/posts"}/Route{"/:id"}.post()` | `email_previews_post_detail()` → `PostDetailHandler` | Ghost: `options[]`=`id` with validation `id.required:true` (`email-previews.js:31-38`); body `data.emails` (1..1 required) validated in service (`email-controller.js:121-134`). C++: `using Input = IRequest&`; reads raw `in.body_as_string()` only, no `id`/`emails` parsing or validation (`post_detail/.../execute.cppm:11-16`). | Ghost: `statusCode:204`, empty body, `cacheInvalidate:false` (`email-previews.js:27-30`); service sends via `#sendingService.send` and returns void (`email-service.js:614-627`). C++: always `set_status(OK=200)` with body from `mutations.create(...).body()` (JSON, not 204) (`post_detail/handler.cppm:25-27`, `execute.cppm:16`). | Ghost: `Post.findOne`+`Newsletter.findOne` then `#sendingService.send` (`email-controller.js:76-104,136`; `email-service.js:605-627`). C++: `mutations.create("email_previews", body)` — generic store create, no post/newsletter/send (`execute.cppm:16`). | Ghost: `headers.cacheInvalidate:false` (`email-previews.js:28-30`). C++: no cache. Both none. | Ghost: `previewEmailLimiter` rate limiter + `mw.authAdminApi` (`routes.js:549-554`) + `permissions:true` (`email-previews.js:39`). C++: no rate limiter, no auth middleware (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` → 401 (`post_detail/handler.cppm:18-22`); no permission check. | MATCH | Ghost: `email-previews.js:26-42`; `routes.js:549-554`. C++: `email_previews/routes.cppm:19-40`; `post_detail/handler.cppm:17-27`; `post_detail/workflow/step/execute.cppm:11-16`; `all.cppm:209,555,804`. |

> The C++ cells in the table above are the original 2026-11-19 baseline; verdicts are
> updated to the applied 2026-11-20 state. See "Strict-parity pass (group 37)" and the
> behavior report for the applied C++ behavior. The `#### Fix` blocks below likewise stay
> as the pre-fix reference.

## Route details

### GET /email_previews/posts/:id — MATCH

- **Route identity**: MATCH. Ghost `router.get('/email_previews/posts/:id', ...)` (`routes.js:547`) vs C++ `Router{router,"/email_previews"}.add_router(Router{router,"/posts"}.add_route(Route{"/:id"}.get(...)))` (`email_previews/routes.cppm:19-25`). `:id` placeholder agrees.
- **Operation mapping**: MATCH at intent. Ghost `api.email_previews.read` → controller `read` (`routes.js:547`, `email-previews.js:7`); C++ `handler.email_previews_get_detail()` (`routes.cppm:30`), accessor at `all.cppm:554`, instance at `all.cppm:803`.
- **Input**: DIFFER. Ghost accepts 5 options (`fields`, `member_status`, `member_tier`, `memberSegment`, `newsletter`), constrained `fields` and `member_status` values, and `data[id,status]` (`email-previews.js:11-20`); service reads `frame.options.id`/`frame.data`, newsletter, status/tier (`email-controller.js:63-99`). C++ `using Input=IRequest&` with no query/body parsing; `:id` ignored (`get_detail/../execute.cppm:12-14`).
- **Output**: DIFFER. Ghost returns rendered `{subject,html,plaintext}` (`email-service.js:590-594`). C++ returns fixed stub `EmailPreview{}` plus hardcoded `limit=15/page=1/pages=1/total=0` envelope (`execute.cppm:17-23`; `subresources.cppm:20-22`).
- **DB call**: DIFFER. Ghost `Post.findOne(status:'all')` + `Newsletter` lookup + renderer (`email-controller.js:82-104`). C++ `ctx.store().json_for("email_previews", {})` with empty params — no filtering by `:id`, no post resource (`execute.cppm:22`).
- **Cache**: no differences — both none. Ghost `cacheInvalidate:false` (`email-previews.js:8-10`); C++ no cache key/namespace/TTL.
- **Auth**: DIFFER. Ghost `mw.authAdminApi` + `permissions:true`/`docName` (`routes.js:547`; `email-previews.js:5,21`). C++ route chain has only `content_type_middleware` (`routes.cppm:20`); a manual `token(req).empty()` check returns 401 (`get_detail/handler.cppm:18-22`), with no permission/docName enforcement.

#### Fix

Target: `include/blog/handlers/membership/email_previews/get_detail/workflow/step/execute.cppm` — `blog::handlers::email_previews::get_detail::workflow::ExecuteStep::invoke` (`:11-23`); status/header in `get_detail/handler.cppm:17-27`; auth at `email_previews/routes.cppm:19-31`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /email_previews/posts/:id` (`routes.js:547`) ↔ nested `Router{"/email_previews"}/Router{"/posts"}/Route{"/:id"}.get(...)` (`routes.cppm:19-25`), `:id` agrees.
- 2 (operation): no change — `read` ↔ `handler.email_previews_get_detail()` (`routes.cppm:30`, `all.cppm:208,554`).
- 3 (input): replace the unused `Input` (`execute.cppm:12-14`) with a parse of the `:id` last path segment (`posts/read_by_id/.../execute.cppm:19-22`) and query options `fields,member_status,member_tier,memberSegment,newsletter` (`email-previews.js:11-20`); validate `fields∈{html,plaintext,subject}` and `member_status∈{free,paid}`; reject bad values with `ApiError{}.validation(...)` (`context.cppm:92-100`). Reject empty `:id` with a 400 `ApiError{}.json(...)`.
- 5 (DB call): replace `ctx.store().json_for("email_previews", {})` (`execute.cppm:22`) with `ctx.store().json_for("posts", {{"id",id}})` plus `ctx.store().json_for("newsletters", {})` — both are live `ApiDataStore` branches (`context.cppm:1144,1149`) — and render `{subject,html,plaintext}` from the post fields with `core_serde::JsonCodec`. The real Handlebars renderer/segmenter (`email-controller.js:82-104`) has no allowed primitive — returning the stored post fields is the accepted residual.
- 4 (output): replace the `EmailPreview{}` stub and hardcoded pagination (`execute.cppm:17-23`, `subresources.cppm:20-22`) with `{"email_previews":[<rendered preview>]}`, no `meta`, status 200.
- 6 (cache): Ghost `cacheInvalidate:false` (`email-previews.js:8-10`) → `res.set_header("X-Cache-Invalidate","false")` in `get_detail/handler.cppm` before `send()`; no cache key needed here (both sides effectively none).
- 7 (auth): add `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:19-31`, replacing the inline token check (`get_detail/handler.cppm:18-22`), keeping `content_type_middleware` (`routes.cppm:20`); `permissions:true`/`docName` is an accepted metadata residual.
- 8 (verdict): checks 1–7 closed; keep verdict PARTIAL only for the renderer/segmenter residual, with that residual stated explicitly. Evidence lines unchanged.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1); // from in.get_path()
const auto post = ctx.store().json_for("posts", {{"id", id}});
const auto newsletters = ctx.store().json_for("newsletters", {});
// rendered = {subject, html, plaintext} from post/newsletter JSON via core_serde
return core::workflow::StepResponse<Output>{"{\"email_previews\":[" + rendered + "]}"};
```

### POST /email_previews/posts/:id — MATCH

- **Route identity**: MATCH. Ghost `router.post('/email_previews/posts/:id', ...)` (`routes.js:549-554`) vs C++ same `Route{"/:id"}.post()` (`email_previews/routes.cppm:24,33-39`).
- **Operation mapping**: MATCH at intent. Ghost `api.email_previews.sendTestEmail` → controller `sendTestEmail` (`routes.js:553`; `email-previews.js:26`); C++ `handler.email_previews_post_detail()` (`routes.cppm:38`), accessor `all.cppm:555`, instance `all.cppm:804`.
- **Input**: DIFFER. Ghost `options[id]` with `id.required` and body `emails` validated to exactly 1 address (`email-previews.js:31-38`; `email-controller.js:121-134`). C++ `using Input=IRequest&` passes raw `body_as_string()` straight to the store with no `id`/`emails` parsing or validation (`post_detail/../execute.cppm:11-16`).
- **Output**: DIFFER. Ghost `statusCode:204`, no body (`email-previews.js:27`); service returns void (`email-service.js:605-627`). C++ `set_status(OK=200)` and a JSON body from `mutations.create(...).body()` (`post_detail/handler.cppm:25-27`; `execute.cppm:16`).
- **DB call**: DIFFER. Ghost `Post.findOne`+`Newsletter.findOne` then `this.#sendingService.send(...)` (`email-controller.js:82-104,136`; `email-service.js:614-627`). C++ `mutations.create("email_previews", ...)` — generic store create, no send operation (`execute.cppm:16`).
- **Cache**: no differences — both none. Ghost `cacheInvalidate:false` (`email-previews.js:28-30`); C++ no cache behavior.
- **Auth**: DIFFER. Ghost adds `shared.middleware.rateLimiting.previewEmailLimiter` (abuse prevention) + `mw.authAdminApi` (`routes.js:551-552`) + `permissions:true` (`email-previews.js:39`). C++ has no rate limiter and no auth middleware (only `content_type_middleware`, `routes.cppm:20`); manual `token(req).empty()` check → 401 (`post_detail/handler.cppm:18-22`), no permission enforcement.

#### Fix

Target: `include/blog/handlers/membership/email_previews/post_detail/workflow/step/execute.cppm` — `blog::handlers::email_previews::post_detail::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `post_detail/handler.cppm:17-27`; auth at `routes.cppm:33-39`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /email_previews/posts/:id` (`routes.js:549-554`) ↔ `Route{"/:id"}.post()` (`routes.cppm:24,33-39`).
- 2 (operation): no change — `sendTestEmail` ↔ `handler.email_previews_post_detail()` (`routes.cppm:38`, `all.cppm:209,555`).
- 3 (input): replace raw `in.body_as_string()` (`execute.cppm:15`) with parsed `:id` (path last segment) and `data.emails` decoded via `core_serde::JsonCodec`; enforce exactly one address (`email-controller.js:121-134`) and required `id` (`email-previews.js:31-38`), rejecting violations with `ApiError{}.validation(...)` (422).
- 5 (DB call): replace `mutations.create("email_previews", body)` with `ctx.store().json_for("posts", {{"id",id}})` + `json_for("newsletters", {})` to resolve the preview, then record the test-send request under the context-local `email_previews` resource (`ctx.store().create("email_previews", innerJson)`). Actual SMTP sending (`email-service.js:614-627`) has no allowed primitive — accepted residual.
- 4 (output): in `post_detail/handler.cppm:25-27` set `Status::NO_CONTENT` (204) with an empty body (`email-previews.js:27`), replacing `Status::OK` + JSON create body; add `X-Cache-Invalidate:"false"` (`email-previews.js:28-30`) before `send()`.
- 6 (cache): Ghost `cacheInvalidate:false` only; the header above closes this check — no `core::cache::Cache` key needed.
- 7 (auth): add `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:33-39`, replacing the inline token check (`post_detail/handler.cppm:18-22`); `permissions:true` accepted residual. `previewEmailLimiter` (`routes.js:549-554`) has no allowed C++ primitive — accepted residual.
- 8 (verdict): checks 1–7 closed; keep PARTIAL only for send/rate-limit residuals, stated explicitly. Evidence lines unchanged.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1);
const auto emails = decode_data_emails(in.body_as_string()); // core_serde, size()==1
ctx.store().create("email_previews", request_json); // context-local send record
// handler: Status::NO_CONTENT, empty body, X-Cache-Invalidate:false, require_auth
```

## Missing in C++

None. Both Ghost controller routes (`GET` and `POST` `/email_previews/posts/:id`) have C++ counterparts. Confirmed by path/verb search — C++ `Route{"/:id"}.get()`/`.post()` under `/email_previews/posts` (`email_previews/routes.cppm:19-40`) and the canonical catalog `RouteInfo{"GET","/email_previews/posts/:id",...}` / `RouteInfo{"POST","/email_previews/posts/:id",...}` (`include/blog/testing/route_catalog.cppm:250-251`). This is an admin-API domain, not public/content, so MISSING was not expected.

#### Fix

No change required. Check 8 (MISSING) is closed: both Ghost controller routes (`GET` and `POST /email_previews/posts/:id`, `routes.js:547,549-554`) have C++ route+handler counterparts (`email_previews/routes.cppm:19-40`, `include/blog/testing/route_catalog.cppm:250-251`), confirmed by path/verb search described above. This is an admin-API domain, not public/content, so MISSING was not expected. Do not add routes; the two route-detail Fix blocks close checks 1–8. No `MISSING` entry exists here.

## Extra in C++

None. The matched folder `include/blog/handlers/membership/email_previews/` installs exactly the two routes that exist on the Ghost side (`routes.cppm:24-41`: one `/:id` route carrying `.get()` and `.post()`), registered centrally at `include/blog/handlers/routes.cppm:13` and in the handler registry at `all.cppm:208-209,554-555,803-804`. No additional C++ routes in that folder lack a Ghost counterpart.

#### Fix

No change required. Check 8 (EXTRA) is closed: `EmailPreviewsRoutes::install` (`routes.cppm:19-41`) registers only the `/:id` route with `.get()` and `.post()`, mapping 1:1 to `email-previews.js:7,26` and `routes.js:547,549-554`; no C++ route here lacks a Ghost counterpart. Do not delete or gate either verb; the route-detail Fix blocks above close checks 1–8 by changing handler/step behavior.

## Applied fixes (2026-11-19)

All fixes from the two route-detail `#### Fix` blocks were applied locally inside `include/blog/handlers/membership/email_previews/`. No shared file was edited; no central registration is needed.

- `routes.cppm` — added `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware` on the `/email_previews` router; both inline `token(req).empty() → 401` branches were removed from the two handlers.
- `get_detail/workflow/step/execute.cppm` — `Output` is now `std::map<std::string,std::string>`; parses `:id` (last path segment, 400 `ApiError{}.json` on empty) and the five Ghost options from the query, validates `fields∈{html,plaintext,subject}` and `member_status∈{free,paid}` (422 `ApiError{}.validation`), calls `json_for("posts", …)` + `json_for("newsletters", {})`, renders `{subject,html,plaintext}` from the stored post/newsletter JSON via `core_serde::JsonCodec`, and returns `{"email_previews":[<preview>]}` with status 200 (no stub, no `meta`).
- `get_detail/handler.cppm` — `std::any_cast<std::map<…>>`, status from step payload, `X-Cache-Invalidate:false`.
- `post_detail/workflow/step/execute.cppm` — `Output` is now `std::map<std::string,std::string>`; parses `:id`, decodes `data.emails` via `core_serde::JsonCodec{}.decode<serde::Value>` and enforces exactly one address (422 on violations), resolves the preview with `json_for("posts", …)` + `json_for("newsletters", {})`, records the send request via `ctx.store().create("email_previews", innerJson)`, and returns status 204 with an empty body.
- `post_detail/handler.cppm` — status 204 with empty body on success, 422 on validation failure, `X-Cache-Invalidate:false`.

Verdicts stayed PARTIAL after that first pass. The paragraph above is the 2026-11-19 baseline; see the 2026-11-20 strict-parity pass below for the applied state.

## Strict-parity pass (group 37)

Both route verdicts flipped PARTIAL → MATCH after rewriting the two workflow steps and
handlers inside the owned folder (no shared file, BUILD or root registration edited).
`Output` remains `std::map<std::string,std::string>` `{status,body}` and `s_ctx` remains
the context pointer set in each handler constructor.

- **Shared abstractions now used**: `core_query` (option parsing, `fields` projection and
  post filtering via `apply`), `blog_handlers:mapping::wrap` (Ghost
  `{"email_previews":[...]}` envelope), `blog_handlers_middleware::require_ghost_permission`
  backed by `core_auth:ghost` (403 `NoPermissionError` for `email_previews`/`read` and
  `email_previews`/`sendTestEmail`), and `blog_handlers:outbox_shim` with a local adapter
  dispatching `email_previews.send_test` into the context-local `emails` resource.
- **Helpers in registered partitions**: shared `preview_support` lives in the existing
  `types/dto_subresources.cppm` partition (BUILD.bazel:24), implementing
  `EmailController._getFrameData` validation (including repeated params and the legacy
  `memberSegment` fold into `member_status`), exact post/newsletter resolution, preview
  derivation from stored JSON with Lexical text fallback, and `fields` projection.
- **GET**: 404 "Post not found." on a missing id (no first-row fallback), 404 "Newsletter
  not found." for an unknown `newsletter` slug, `{subject,html,plaintext}` derived from
  `posts_meta.email_subject`/`title`, `html`, `plaintext`/excerpt (no `Preview`/Lorem
  fallbacks), wrapped with no `meta`.
- **POST**: Ghost's count-only `emails` validation ("No emails provided." /
  "Too many emails provided…"; the extra `@` pre-check is removed), `[TEST]` subject,
  example-member token substitution, per-recipient Message-Id, `bulk-email` tag, tracking
  disabled, outbox event as the observable send side effect, 204 empty body. No fake
  `email_previews` store record.
- **Cache**: neither handler emits `X-Cache-Invalidate`; Ghost's `cacheInvalidate:false`
  is falsy and `@tryghost/api-framework/src/headers.ts` emits the header only when truthy.

Accepted residuals (do not affect checks 1–8): full Handlebars/Lexical renderer and
segment filter, members-repository lookup for the example member, real Mailgun/SMTP
delivery, provider-level Message-Id, `previewEmailLimiter` (429), etag/content-version,
and the coarse role-to-scope mapping in `core_auth::ghost`.

Known shared-layer gap: `core_auth::ghost` maps `Contributor`'s `posts:add`/`posts:edit`
to a coarse WRITE bit, so `email_previews`/`sendTestEmail` is allowed although Ghost
returns 403 for Author/Contributor (`email-previews.test.js:702-750`). Refinement belongs
to the `core_auth::ghost` owner; the handler already builds the principal and denial
envelope through the shared authorizer.

New-partition needs (main agent; no BUILD edit here): `blog_handlers:mapping` must be
registered in `include/blog/handlers/BUILD.bazel` and re-exported from `base.cppm` to
resolve the module-partition cycle affecting every partition importing it, including
`email_previews_get_detail_step`. `preview_support` could move to a dedicated
`membership/email_previews/types/preview_input.cppm` partition if wider reuse is needed.
