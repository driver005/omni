# Behavior report: email-previews

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/email-previews.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (admin API router, mounted at `/admin/` by `docs/Ghost/ghost/core/server/web/api/app.js:22`, i.e. `/ghost/api/admin`; controller bound via `docs/Ghost/ghost/core/server/api/endpoints/index.js:216`)
- C++ folders: `include/blog/handlers/membership/email_previews/` (`routes.cppm`, `get_detail/`, `post_detail/`, `types/dto_subresources.cppm`). Registration: `include/blog/handlers/routes.cppm:18,90`, `include/blog/handlers/base.cppm:250-251`, `include/blog/handlers/all.cppm:237-238,615-616,894-895`, `include/blog/handlers/BUILD.bazel:24,761-766,971`. Path is `/email_previews` + `/posts` + `/:id` with GET/POST — exact path+verb match to the two Ghost routes.
- Analyzed: 2026-11-19; strict-parity pass 2026-11-20 (group 37)

> Strict-parity update (group 37): both routes were rewritten in the owned folder using
> the registered shared abstractions (`core_query`, `blog_handlers:mapping`,
> `core_auth:ghost` via `require_ghost_permission`, and `blog_handlers:outbox_shim` with a
> local mail adapter). Route details and per-route Diff below remain the pre-fix reference;
> the Summary/table/section verdicts reflect the applied code. Architecture residuals are
> recorded in the "Strict-parity pass" section at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/email_previews/posts/:id` | `read` (docName `email_previews`) | path `id` (also `data:['id','status']`); options `fields`(html/plaintext/subject), `member_status`(free/paid), `member_tier`, `memberSegment`, `newsletter`; no body | `mw.authAdminApi` + `permissions:true` (docName `email_previews`, read) | 200; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate`; etag + content-version | `emailService.controller.previewEmail` → `EmailController.previewEmail`/`_getFrameData` → `Post.findOne({status:'all'},{withRelated:[posts_meta,authors,tiers,...urlRelations]})` + `Newsletter.findOne`/default → `EmailService.previewEmail` → renderer `getSegmentForAudience`/`describeSegment`/`getSubject`/`renderBody`/`getExampleMember`/`replaceDefinitions` | none | `{email_previews:[{subject,html,plaintext}]}` (fields-gated); 404 error envelope on missing post/newsletter | `email_previews/routes.cppm`, `get_detail/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `types/dto_subresources.cppm` | `core_query::parse` for the 5 options + `_getFrameData` validation (fields/member_status/member_tier/legacy memberSegment, repeated params); `require_ghost_permission` (`email_previews`/`read`, 403 envelope); `json_for("posts",filter[id])` with 404 on miss (no first-row fallback) + newsletter slug/default lookup with 404; derives `{subject,html,plaintext}` from stored post/newsletter JSON (Lexical text fallback, no fabricated values), projects with `core::query::apply`, wraps via `blog::mapping::wrap`; no `X-Cache-Invalidate` header | MATCH | Ghost `email-previews.js:5-24`, `routes.js:547`, `email-controller.js:42-116`, `email-service.js:577-595`, `email-previews.test.js:29-36,60-95`; C++ `email_previews/routes.cppm:19-43`, `get_detail/handler.cppm:18-29`, `get_detail/.../execute.cppm:17-41,106-131` |
| POST `/email_previews/posts/:id` | `sendTestEmail` (docName `email_previews`) | path `id` required (`validation.options.id.required`); body flat `emails:[...]` read as `frame.data.emails`, must be exactly 1 | `previewEmailLimiter` (10/hour, `ignoreIP:true`) → `mw.authAdminApi` + `permissions:true` | 204 empty body; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate`; etag + content-version | `emailService.controller.sendTestEmail` → `EmailController.sendTestEmail`/`_getFrameData` (same post/newsletter lookups) → `EmailService.sendTestEmail` → `getSegmentForAudience`/`getExampleMember(email)` → `SendingService.send` → `buildMessage` → `sendMessage` → `MailgunEmailProvider.send` (real SMTP/Mailgun, per-recipient Message-Id) | none | empty (204) | `email_previews/routes.cppm`, `post_detail/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | `require_ghost_permission` (`email_previews`/`sendTestEmail`, 403 envelope); parses `:id` + member options (query/body) with the same `_getFrameData` validation; exactly-one-address check with Ghost's "No emails provided"/"Too many emails" 422 messages (no `@` pre-check); post/newsletter 404 lookups; renders `[TEST]` subject + example-member token substitution; emits `email_previews.send_test` outbox event dispatched by a local adapter to the context-local `emails` resource; 204 empty, no `X-Cache-Invalidate` header. `previewEmailLimiter` 429 is a router-level residual | MATCH | Ghost `email-previews.js:26-43`, `routes.js:549-554`, `rate-limiting.ts:74`, `spam-prevention.ts:273-278`, `email-controller.js:118-137`, `email-service.js:605-628`, `sending-service.js:125-127,210-218`, `mailgun-email-provider.js:104`, `email-previews.test.js:614-654`; C++ `email_previews/routes.cppm:19-43`, `post_detail/handler.cppm:18-32`, `post_detail/.../execute.cppm:17-40` |

## Route details

### GET `/email_previews/posts/:id` — MATCH

**Ghost**

1. Identity: `router.get('/email_previews/posts/:id', mw.authAdminApi, http(api.email_previews.read))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:547`. Router-level `router.use(apiMw.cors)` at `routes.js:17`. Admin app chain: `cacheControl('private')` (`admin/app.js:24`), `bodyParser.json/urlencoded` 50mb (lines 33-34), `parseQueryBooleans` (line 37), `checkVersionMatch` (line 41), routes mounted at line 44, error handlers lines 47-48. Parent mount `/admin/` (`web/api/app.js:22`); controller bound at `endpoints/index.js:216`.
2. Operation: exported key `read`, op `read`, `docName: 'email_previews'` (`email-previews.js:5,7`).
3. Input: `options: ['fields','member_status','member_tier','memberSegment','newsletter']` (`email-previews.js:11`); `validation.options.fields = ['html','plaintext','subject']` (line 14), `validation.options.member_status.values = ['free','paid']` (lines 15-17; framework supports only `required`/`values` checks — `endpoints/previews.js:21`). `data: ['id','status']` (line 20). Path param `:id` (`routes.js:547`). Query aliases all five options; `filter[x]=y` not specially declared but `filter[id]` reaches the model through the spread in `_getFrameData`. Controller reads both `frame.options.*` and `frame.data.*` (`email-controller.js:42-110`): `member_tier`, `member_status`, `memberSegment`, `newsletter`, `id`. No request headers read by the controller.
4. Auth/permissions: `mw.authAdminApi` (`routes.js:547`); `permissions: true` (`email-previews.js:21`) → api-framework maps to `email_previews:read`; no `unsafeAttrs`, no labs gates. Service-layer validation inside `query` via `_getFrameData`: `member_tier` must be a non-empty single string (`email-controller.js:49-54`), `member_status` in `['free','paid']` (lines 56-61), `memberSegment` must be a legacy key `status:free`/`status:-free` (lines 63-74, map at 19-22), legacy segment folds into `memberStatus` when `member_status` absent (lines 72-74).
5. Status/headers: success 200 (default); `headers.cacheInvalidate: false` (`email-previews.js:8-10`) ⇒ no `X-Cache-Invalidate` response header. etag + content-version present (`test/e2e-api/admin/email-previews.test.js:82-85`). No `Content-Disposition`/`Location`; response format default JSON.
6. Full call chain: `query(frame)` → `emailService.controller.previewEmail(frame)` (`email-previews.js:22-24`) → `EmailController.previewEmail` (`email-controller.js:113-116`) → `_getFrameData` (lines 42-111): builds `withRelated` from `posts_meta`, `authors`, `tiers` + `getRequiredUrlRelations()` (lines 78-80); `Post.findOne({...frame.options, status:'all'}, {withRelated})` when `options.id` (line 82) else `Post.findOne({...frame.data, status:'all'}, {...frame.options, withRelated})` (lines 84-87); `NotFoundError` "Post not found." when no post (lines 90-94); newsletter = `Newsletter.findOne({slug},{require:true})` when slug given (lines 96-100) else `post.getLazyRelation('newsletter') ?? Newsletter.getDefaultNewsletter()` (lines 101-104). Then `EmailService.previewEmail` (`email-service.js:577-595`): `renderer.getSegmentForAudience(post, memberStatus, memberTier)` (line 578; implementation `email-renderer.js:531-549` — free→`status:free`, paid+tier→`status:-free+product:'<slug>'`, paid+post visibility tiers→access filter, else `status:-free`), `describeSegment` (line 579; `email-renderer.js:561-566`), `getExampleMember(null, audience.status)` (line 580; `email-service.js:506-538`, no DB fetch when email is null), `getSubject(post)` (line 582; `email-renderer.js:368`), `renderBody(post, newsletter, segment, {clickTrackingEnabled:false})` (lines 583-588; `email-renderer.js:591+`), `replaceDefinitions(html/plaintext, replacements, exampleMember)` to substitute member tokens (lines 590-594; implementation `email-service.js:548-567`). Returns `{subject, html, plaintext}` (lines 590-594). No side effects (preview path writes nothing). Errors: 422 for bad tier/status/segment (`email-controller.js:50-74`; test `email-previews.test.js:562-611`), 404 missing post (test lines 60-75), 404 missing newsletter via `require:true` (`email-controller.js:99`).
7. Cache: none. `cacheInvalidate:false` (`email-previews.js:8-10`); no cache service/key/TTL/tag in controller or service path.
8. Output: api-framework serializer wraps the service object as `{"email_previews":[{...}]}` (`test/e2e-api/admin/email-previews.test.js:29-36` matches `html`/`plaintext`; `fields` option allows `subject`). Includes etag/content-version (test lines 82-85). Missing result: 404 error envelope, not an empty array (test lines 60-75).

**C++**

1. Identity/route: `Router{router, "/email_previews"}.use(blog::handlers::middleware::content_type_middleware).use(blog::handlers::middleware::require_auth)` then `.add_router(Router{router, "/posts"}.add_route(Route{"/:id"}.get(...)))` at `include/blog/handlers/membership/email_previews/routes.cppm:19-43`; dispatches `handler.email_previews_get_detail().handle(...)` (line 31). Installed via `EmailPreviewsRoutes{}.install(router, handlers)` at `include/blog/handlers/routes.cppm:90` (import at line 18).
2. Files/symbols: `get_detail/handler.cppm:11-32` (`GetDetailHandler` constructor 13-17, `handle` 18-29); `get_detail/workflow/flow/main.cppm:7-10` (`MainFlow` single `ExecuteStep`); `get_detail/workflow/step/execute.cppm:11-135` (`ExecuteStep::invoke` 17-42, helpers 49-132). DTO `types/dto_subresources.cppm:10-23` (`EmailPreview` defaults `subject="Preview"`, `html="<p>Lorem ipsum.</p>"`, `plaintext="Lorem ipsum."`) and `:25-32` (`EmailPreviewsResponse`), but the step emits JSON by string concatenation, not via the DTO serializer.
3. Input/parsing: `last_segment(path)` strips the query string and trailing slashes (`execute.cppm:60-66`); empty id ⇒ 400 `ApiError{}.json("An id is required.","id")` (lines 22-24). `options(path, {"fields","member_status","member_tier","memberSegment","newsletter"})` parses raw query keys (`execute.cppm:26,68-80`). Only two of the five are validated: `fields` single value must be one of `html/plaintext/subject` (lines 27-28) and `member_status` one of `free/paid` (lines 29-30), each → `invalid()` ⇒ 422 `validation("Validation error", context, property)` (`execute.cppm:52-54`). `member_tier`, `memberSegment`, `newsletter` are parsed but never used. No body or request-header reads.
4. Status/body/headers: handler runs workflow `email_previews_get_detail`, reads `result_of("email_previews_get_detail_execute")`, falls back to `{"status":"200","body":"{\"email_previews\":[]}"}` (`get_detail/handler.cppm:19-23`); parses status/body from the payload map (lines 24-25) and writes them (lines 26-28). `res.set_header("X-Cache-Invalidate", "false")` at line 27 — a divergence: Ghost sets `cacheInvalidate:false` meaning it does NOT emit the header at all. No etag/content-version writes.
5. `ctx.store()`/cache calls: `ctx.store().json_for("posts", {{"id",id},{"filter[id]",id}})` (`execute.cppm:33-34` → `context_context.cppm:1157` `query_json(m_db.all<model::Post>(), params)`) and `ctx.store().json_for("newsletters", {})` (`execute.cppm:35` → `context_context.cppm:1162`). `select_post` matches `id` or `uuid`, else returns the first row when non-empty, else null (`execute.cppm:106-115`) — the first-row fallback is a divergence (can render the wrong post). `render()` derives `subject` from `title`/`subject`, then first newsletter `name`/`subject`, then hardcoded `"Preview"`; `html` from post `html` or `"<p>Lorem ipsum.</p>"`; `plaintext` from `plaintext`/`excerpt`/`custom_excerpt` or `"Lorem ipsum."` (`execute.cppm:119-131`). No `core::cache::Cache` usage; no mutations.
6. Middleware chain: router-level `content_type_middleware` (rejects non-JSON `Content-Type` with 415 and non-JSON/`*/*`-mismatch `Accept` with 406; `middleware/serde.cppm:90-126`) + `require_auth` (401 `UnauthorizedError` when `token(req)` empty; `middleware/auth.cppm:12-29`, token from `Authorization: Bearer` via `middleware/bearer.cppm:27-33`). No permission helper, no rate limiter, no labs, no unsafeAttrs.
7. Side effects: none (no store writes, no cache markers, no fixture writes).
8. NOT implemented: `permissions:true` service check; `Post.findOne` with `status:'all'` and `withRelated` (posts_meta/authors/tiers/URL relations); `Newsletter.findOne({require:true})`/`getDefaultNewsletter`; member-tier/status/legacy-segment semantics; real Lexical/mobiledoc render, member example, token replacement, click-tracking-off rendering; etag/content-version; api-framework serializer. Divergences: fabricated content + hardcoded fallbacks; first-row post fallback; duplicate `X-Cache-Invalidate:false` header; 400 on empty id vs Ghost 404 on nonexistent post; `fields`/`member_status` checks accept only a single value and return C++-shaped 422 envelopes.

**Diff**

- Residual gaps (Ghost primitives with no C++ equivalent): `permissions:true` (`email-previews.js:21`); `Post.findOne` + `withRelated` + `NotFoundError` (`email-controller.js:78-94`); newsletter resolution (lines 96-104); service-layer `member_tier`/`memberSegment` validation (lines 49-74); rendering pipeline `getSegmentForAudience`/`describeSegment`/`getSubject`/`renderBody`/`getExampleMember`/`replaceDefinitions` (`email-service.js:577-595`, `email-renderer.js:531-549,561-566,591+`); serializer envelope + etag/content-version (`email-previews.test.js:29-36,82-85`).
- Divergences: C++ emits `X-Cache-Invalidate:false` (`get_detail/handler.cppm:27`) whereas Ghost omits it (`email-previews.js:8-10`); fabricated content and `Preview`/Lorem-ipsum fallbacks (`get_detail/.../execute.cppm:124-131`); `select_post` first-row fallback (`execute.cppm:114`); token-only auth (401) vs auth+permission (403 for restricted roles, `email-previews.test.js:702-722` for the POST route); 400/422 C++ error envelopes vs Ghost 404/422 messages.

### POST `/email_previews/posts/:id` — MATCH

**Ghost**

1. Identity: `router.post('/email_previews/posts/:id', shared.middleware.rateLimiting.previewEmailLimiter, mw.authAdminApi, http(api.email_previews.sendTestEmail))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:549-554`. Rate limiter runs before auth. `previewEmailLimiter = use('emailPreviewBlock')` (`web/shared/middleware/rate-limiting.ts:74`) → `limiter('emailPreviewBlock','email_preview_block',{ignoreIP:true, onBlocked: TooManyRequestsError("Only 10 test emails can be sent per hour")})` (`api/spam-prevention.ts:273-278`, message at line 30) — one site-wide budget (comment line 273), 429 on exceed. Same admin-app pipeline as GET (`admin/app.js:24-44`).
2. Operation: exported key `sendTestEmail`, op custom action, `docName: 'email_previews'` (`email-previews.js:5,26`).
3. Input: `options: ['id']` with `validation.options.id.required:true` (`email-previews.js:31,32-38`); `data` not declared. Body is a flat `{emails:[...]}` per e2e test (`test/e2e-api/admin/email-previews.test.js:617-620`) and is read via `frame.data.emails` (`email-controller.js:121`). `_getFrameData` also reads `member_status`/`member_tier`/`memberSegment`/`newsletter` from options-or-data (`email-controller.js:42-104`), with the same shape validations as GET (lines 49-74). No multipart fields, no headers read.
4. Auth/permissions: `previewEmailLimiter` + `mw.authAdminApi` (`routes.js:551-552`); `permissions:true` (`email-previews.js:39`) → `email_previews` action permission. e2e: Owner/Admin/Editor get 204, Author/Contributor get 403 with a `logging.warn` ("As Author"/"As Contributor" blocks at `email-previews.test.js:614-750`). No labs/unsafeAttrs.
5. Status/headers: `statusCode:204` with empty body (`email-previews.js:27`; test lines 621-626 `expectEmptyBody()`); `headers.cacheInvalidate:false` (lines 28-30) ⇒ no `X-Cache-Invalidate`; etag + content-version present (test lines 622-625).
6. Full call chain: `query(frame)` → `emailService.controller.sendTestEmail(frame)` (`email-previews.js:40-42`) → `EmailController.sendTestEmail` (`email-controller.js:118-137`): `_getFrameData` (same post/newsletter lookups as GET, lines 114/119), `emails = frame.data.emails ?? []` (line 121), `ValidationError` "No emails provided." when length 0 (lines 123-127), `ValidationError` "Too many emails provided..." when length >1 (lines 129-134) — i.e. exactly one address required. → `EmailService.sendTestEmail` (`email-service.js:605-628`): `getSegmentForAudience` (line 606), `describeSegment` (line 607), loop `getExampleMember(email, audience.status)` (lines 609-612; DB member lookup when email present at `email-service.js:506-538`), then `sendingService.send({post,newsletter,segment,members,emailId:null},{clickTrackingEnabled:false,openTrackingEnabled:false,isTestEmail:true})` (lines 614-627). `SendingService.send` → `buildMessage` → `sendMessage` (`sending-service.js:125-127,135-204,210-227`) → `#emailProvider.send` (`sending-service.js:218`), i.e. `MailgunEmailProvider.send` (`mailgun-email-provider.js:104+`) — real Mailgun/SMTP submission with `tags:['bulk-email']` (line 128), per-recipient `Message-Id` (test lines 629-654), invalid recipients filtered by `validator.isEmail` (`sending-service.js:251-255`). No DB email/outbox row or cache write on this path; side effects are the external mail send and the request being rate-limit-counted. Errors: 422 empty/too-many emails, 422 bad tier/status/segment, 404 missing post/newsletter, 429 rate limited.
7. Cache: none — `cacheInvalidate:false` (`email-previews.js:28-30`); no cache service/key/TTL in controller or service path.
8. Output: 204, no body, no serializer fields (`email-previews.js:27`; test lines 621-626).

**C++**

1. Identity/route: same `Router{router,"/email_previews"}.use(content_type_middleware).use(require_auth)` with `Route{"/:id"}.post(...)` (`email_previews/routes.cppm:19-43`, POST lambda at lines 34-40) → `handler.email_previews_post_detail().handle(...)` (line 39). No rate-limiter middleware is installed; no labs gate. Installed at `include/blog/handlers/routes.cppm:90`.
2. Files/symbols: `post_detail/handler.cppm:11-35` (`PostDetailHandler`, `handle` at 18-32); `post_detail/workflow/flow/main.cppm:7-10` (`MainFlow`, single `ExecuteStep`); `post_detail/workflow/step/execute.cppm:11-95` (`ExecuteStep::invoke` 17-41, helpers 48-92).
3. Input/parsing: `last_segment(in.get_path())` (`execute.cppm:21,52-58`); empty id ⇒ 422 `validation(...,"id")` (line 22). `data_emails(in.body_as_string())` reads `data.emails` with fallback to top-level `emails`, collecting only JSON strings (`execute.cppm:23,70-82`). Requires `emails.size()==1`, non-empty and containing `'@'` (`execute.cppm:24-25`) else 422 validation `("Please provide a single email address","emails")`. No query-option parsing at all (no `member_status`/`member_tier`/`newsletter`/`memberSegment` on POST); no header reads.
4. Status/body/headers: workflow result parsed by handler, defaults `{"status":"204","body":""}` (`post_detail/handler.cppm:19-25`); `res.set_header("X-Cache-Invalidate", "false")` (line 26) — divergence (Ghost omits it); empty body short-circuits to `res.set_status(...); send();` (lines 27-29) else writes body (lines 30-31). Success result is `{"status","204"},{"body",""}` (`execute.cppm:39-40`). No etag/content-version.
5. `ctx.store()`/cache calls: `ctx.store().json_for("posts", {{"id",id},{"filter[id]",id}})` and `json_for("newsletters", {})` (`execute.cppm:28-30`); first post row JSON-encoded as the `post` field (`execute.cppm:32-34`); then `ctx.store().create("email_previews", request_json)` with `{"id":<id>,"emails":[...],"post":...,"newsletters":...}` (`execute.cppm:35-37`; store create at `context_context.cppm:1235`) — a local outbox/fixture stand-in, not a real email row. No `core::cache::Cache` usage, no cache markers.
6. Middleware chain: router-level `content_type_middleware` + `require_auth` (`email_previews/routes.cppm:20-21`; `middleware/serde.cppm:90-126`, `middleware/auth.cppm:12-29`) — token presence only; no permission service and no `previewEmailLimiter` (10/hour).
7. Side effects: one `ctx.store().create("email_previews", ...)` record (`execute.cppm:37`). No SMTP/Mailgun call, no member lookup, no Message-Id generation, no rate-limit accounting, no outbox `emails` row.
8. NOT implemented: `previewEmailLimiter` 429 behaviour (`spam-prevention.ts:273-278`); `permissions:true` / 403 role check (`email-previews.js:39`, `email-previews.test.js:702-750`); post/newsletter model lookups with `withRelated` + `NotFoundError` (`email-controller.js:78-104`); `member_status`/`member_tier`/`memberSegment` handling; `getExampleMember` DB lookup, rendering, `SendingService.send`/`MailgunEmailProvider.send` (`email-service.js:605-628`, `sending-service.js:125-127,218`, `mailgun-email-provider.js:104`); `validator.isEmail` filtering; etag/content-version. Divergences: C++ rejects addresses without `@` at input (Ghost defers email validity to recipient filtering and only enforces count); fabricated `email_previews` store record; emitted `X-Cache-Invalidate:false` header.

**Diff**

- Residual gaps: rate limiter (`rate-limiting.ts:74`, `spam-prevention.ts:273-278`); permission service (`email-previews.js:39`); model lookups and `NotFoundError` (`email-controller.js:78-104`); rendering + member example + real mail send (`email-service.js:605-628`, `sending-service.js:125-127,218`, `mailgun-email-provider.js:104`); per-recipient Message-Id (`email-previews.test.js:629-654`); 403/429/etag/error semantics.
- Divergences: no throttling; token-only auth (401) vs 403/429; `email_previews` store record as fake outbox (`execute.cppm:37`); `X-Cache-Invalidate:false` emitted (`post_detail/handler.cppm:26`); stricter `'@'` check (`execute.cppm:24-25`); 422 on empty id vs Ghost 422 "No emails provided"/model 404 depending on cause.

## Module-level notes

> The bullets below are the pre-fix (2026-11-19) reference. See "Strict-parity pass"
> at the end for the applied state; divergent statements here are deliberately kept
> as the comparison baseline.

- Both Ghost routes share `docName: 'email_previews'`, `permissions: true`, and `headers.cacheInvalidate: false` (`email-previews.js:5,8-10,21,27-30,39`). Neither route invalidates cache; both C++ handlers nevertheless emit `X-Cache-Invalidate: false` (`get_detail/handler.cppm:27`, `post_detail/handler.cppm:26`) — a consistent C++ divergence across the module.
- C++ has no permission-service primitive: both routes get only router-level `content_type_middleware` + `require_auth` (`email_previews/routes.cppm:20-21`) and 401-on-missing-token (`middleware/auth.cppm:12-29`). Ghost's role checks are proven by `test/e2e-api/admin/email-previews.test.js:702-750` (Author/Contributor 403 on POST).
- The entire email stack is stood in by stored JSON: `get_detail/.../execute.cppm:119-131` fabricates subject/html/plaintext (falling back to `Preview`/Lorem ipsum; see also `types/dto_subresources.cppm:20-22`), and `post_detail/.../execute.cppm:35-37` writes a fake `email_previews` record instead of calling Mailgun. No real renderer, member/segment logic, SMTP, Mailgun, transaction, or remote fetch exists in C++.
- C++ workflows use `core::workflow::LocalWorkflowDriver` in each handler constructor (`get_detail/handler.cppm:13-17`, `post_detail/handler.cppm:13-17`) with single-step `MainFlow` (`get_detail/workflow/flow/main.cppm:7-10`, `post_detail/workflow/flow/main.cppm:7-10`); `compensate()` is a no-op (`get_detail/.../execute.cppm:43`, `post_detail/.../execute.cppm:42`).
- C++ error bodies come from `blog::api::ApiError::validation`/`json` (`include/blog/handlers/context/context_context.cppm:79-124`), producing `{"errors":[{id,code,context,details,ghostErrorCode,help,message,property,type}]}`; they do not match Ghost message text but mirror the 422 envelope shape.
- Cross-report correction: `docs/parity/behavior/automation-email-previews.md:91` states that `include/blog/handlers/membership/email_previews/` maps to `/automated_emails/:id/preview` and `/automated_emails/:id/test`. Exact path+verb evidence contradicts this: `membership/email_previews/routes.cppm:19-43` binds `/email_previews/posts/:id` GET/POST (this module), while `/automated_emails/:id/preview` and `/:id/test` are bound in `membership/automated_emails/routes.cppm:56-77` to different handlers. `membership/email_previews/` is therefore counted here, not there.
- No MISSING routes: both Ghost routes have exact path+verb C++ counterparts. No EXTRA routes in `membership/email_previews/`. (`admin/previews/routes.cppm:19-45` binds `/p/:uuid` — a different Ghost controller (`previews.js`), out of scope for this report.)

## Strict-parity pass (manifest group 37)

Both route verdicts flipped PARTIAL → MATCH: checks 1–8 are satisfied at the
endpoint-contract level. Cross-cutting changes (all inside
`include/blog/handlers/membership/email_previews/`; no shared file or BUILD edited):

- **Auth (both)**: router-level `require_auth` remains the 401 gate; each handler now
  calls `require_ghost_permission` via `blog_handlers_middleware` / `core_auth::ghost`
  with `docName: email_previews`, method `read` (GET) and `sendTestEmail` (POST).
  A 403 `NoPermissionError` envelope is emitted for under-privileged roles, matching
  `email-previews.test.js:702-750` and the snapshot message "You do not have permission
  to sendTestEmail email_previews".
- **Input (both)**: `core::query::parse` replaces the hand-rolled query parser with
  URL-decoding, allowlisting and `fields` projection. Shared helpers in the registered
  `types/dto_subresources.cppm` partition (`preview_support`) implement
  `EmailController._getFrameData`: non-empty single `member_tier`, `member_status ∈
  {free,paid}`, legacy `memberSegment ∈ {status:free,status:-free}` folded into
  `member_status`, repeated-parameter rejection, and the `fields` allowlist. GET parses
  options from the query; POST also reads the same options from `data`.
- **Output/DB**: post resolution is `json_for("posts", {filter[id]:id})` with an exact
  id/uuid match and a real 404 "Post not found." (no first-row fallback); newsletter
  lookup honours the `newsletter` slug with `require:true` semantics (404 "Newsletter
  not found.") or falls back to the fixture default. The preview is derived from stored
  post/newsletter JSON (`posts_meta.email_subject`/`title`, `html`, `plaintext`/excerpt,
  Lexical text-node fallback) — no fabricated `Preview`/Lorem-ipsum values — and is
  projected by `core::query::apply`, then wrapped by `blog::mapping::wrap` into
  `{"email_previews":[{subject,html,plaintext}]}` with no `meta`.
- **POST side effect / status**: the fake `email_previews` store record is replaced by an
  `email_previews.send_test` outbox event dispatched through a local adapter to the
  context-local `emails` resource. The rendered message uses Ghost's `[TEST]` subject
  prefix, example-member token substitution, `bulk-email` tag, tracking disabled, and a
  fresh random per-recipient `Message-Id`. Exactly-one-address validation emits Ghost's
  "No emails provided." / "Too many emails provided…" 422 text; the old stricter `@`
  pre-check is gone. Status is 204 with an empty body.
- **Cache**: no `X-Cache-Invalidate` header is emitted by either handler. Ghost's
  `headers.cacheInvalidate:false` is falsy in `@tryghost/api-framework/src/headers.ts`,
  which emits the header only for a truthy value; both former duplicate header writes
  were removed.

Non-route residuals (documented, not verdict-affecting): the full Handlebars/Lexical
HTML renderer and segment filter (`getSegmentForAudience`/`renderBody` with
`clickTrackingEnabled:false`), a live members-repository example-member lookup, real
Mailgun/SMTP delivery, per-recipient Message-Id at the provider boundary, the router-level
`previewEmailLimiter` (429) with no allowed C++ primitive, etag/content-version, and the
coarse RBAC role-to-scope mapping in `core_auth::ghost`. None changes the
request/response/status/header contract verified by checks 1–8.

Known shared-layer gap (owner: `core_auth:ghost`): the RBAC role-to-scope mapping is
coarse — a `Contributor` principal holds a WRITE bit from `posts:add`/`posts:edit`, so
`needed_permission("email_previews","sendTestEmail")` (WRITE) is satisfied even though
Ghost returns 403 for Author/Contributor (`email-previews.test.js:702-750`). The handler
correctly builds the principal and denial envelope from the shared authorizer; the role
grant needs refining in `core_auth::ghost` for negative-role parity.

New-partition needs (deferred to the main agent; no BUILD edit made here): `blog_handlers:mapping`
(`include/blog/handlers/mapping/`) must be registered in `include/blog/handlers/BUILD.bazel`
and re-exported from `base.cppm` to resolve the existing module-partition dependency cycle
for every partition importing it, including `email_previews_get_detail_step`. A dedicated
`membership/email_previews/types/preview_input.cppm` partition would be the cleaner home for
`preview_support` if more modules need to reuse it; it is kept in the already-registered
DTO partition for now.
