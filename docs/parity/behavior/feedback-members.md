# Behavior report: feedback-members

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/feedback-members.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (browse), `docs/Ghost/ghost/core/server/web/members/app.js` (add)
- C++ folders: `include/blog/handlers/discovery/feedback/` (`routes.cppm`, `read_by_id/`, `add/`)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19 (shared abstractions wired in)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Two controller ops, two binding routes (`browse`, `add`), both with strict C++ counterparts under `discovery/feedback/`. Controller-level Ghost behavior is implemented through shared abstractions: `core::query` (NQL filter/sort/paging + `meta.pagination`), `core_auth:ghost` via `blog_handlers_middleware::require_ghost_permission`, `core_serde::JsonCodec`, and the existing `ApiDataStore`/`ResourceStore`.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/feedback/:id` (admin app, full `/ghost/api/admin/feedback/:id`) | `browse`, docName `feedback` | path `:id`; `data:['id']`; `options:['limit','page','order','score']`; validation `id` string required | `mw.authAdminApi`; declared `permissions {posts,browse}`; in-service `permissions.canThis(ctx).browse.member()` gating member relation | `headers.cacheInvalidate:false` → `X-Cache-Invalidate:false`; default 200 JSON | controller → `AudienceFeedbackController.browse` → `FeedbackRepository.getForPost` → `MemberFeedback.findPage` | no cache service; `cacheControl('private')` at app level | `{data:[{id,score,created_at,member?}],meta.pagination}` | `discovery/feedback/routes.cppm`, `read_by_id/handler.cppm`, `read_by_id/workflow/{flow/main,step/execute}.cppm` | same verb+path; `core::query::parse/run` with injected `post_id`/`score` filters, `created_at desc`, limit dflt 10, projection `{id,score,created_at,member}`; `require_ghost_permission(...,"posts","browse")` + `GhostAuthorizer.authorize(...,"members","browse")` gates the embedded member relation | MATCH | Ghost `feedback-members.js:7-28`, `admin/routes.js:607`; C++ `routes.cppm`, `read_by_id/handler.cppm`, `read_by_id/workflow/step/execute.cppm` |
| POST `/api/feedback` (members app, full `/members/api/feedback`) | `add`, docName `feedback` | body `data.feedback[0]` with `post_id`+`score` required; score must be 0 or 1 | `permissions:false`; member via `loadMemberSession`+`authMemberByUuid` → `context.member.id` | `statusCode:201`; `headers.cacheInvalidate:false` → `X-Cache-Invalidate:false` | controller → `AudienceFeedbackController.add` → `getPostById` → `get`/`add`/`edit` on `MemberFeedback` | no cache service; `cacheControl('private')` at app level | `Feedback` object (`id,memberId,postId,score`); 201 create, 201 edit, 404 post missing, 422 bad score, 500 no member | `discovery/feedback/routes.cppm`, `add/handler.cppm`, `add/workflow/{flow/main,step/execute}.cppm` | same verb+path; `JsonCodec` decode + score/post_id validation; member id from authenticated session header (500 when absent); `find_by_resource("posts",...)` 404 branch; upsert with unchanged-score skip and 24-hex ObjectID; Ghost wire output; always 201 | MATCH | Ghost `feedback-members.js:30-49`, `members/app.js:144-150`; C++ `routes.cppm`, `add/handler.cppm`, `add/workflow/step/execute.cppm` |

## Route details

### GET /feedback/:id — MATCH

**Ghost**

1. Identity: `GET /feedback/:id` declared at `admin/routes.js:607` with middleware `mw.authAdminApi`. Router-level `router.use(apiMw.cors)` at `admin/routes.js:17`. App-level mounts: `cacheControl('private')` (`endpoints/admin/app.js:24`), `bodyParser.json`/urlencoded 50mb (`:33-34`), `parseQueryBooleans` (`:37`), `checkVersionMatch` (`:41`), `routes()` (`:44`); parent mounts `/admin/` (`web/api/app.js:22`) under the API app.
2. Operation: controller key `browse`, `docName:'feedback'` (`feedback-members.js:5,7`); `permissions {docName:'posts', method:'browse'}` (`:21-24`).
3. Input: `data:['id']` (`:11`), `options:['limit','page','order','score']` (`:12`), `validation.data.id {type:'string', required:true}` (`:13-20`). Path param `:id`; query aliases `limit`, `page`, `score` (and `order`, accepted but ignored downstream). No headers read by the controller. GET → no body.
4. Auth: `mw.authAdminApi` (`admin/routes.js:607`); declared permissions `{posts,browse}` (`feedback-members.js:21-24`). Inside the service, `permissions.canThis(frame.options.context).browse.member()` is attempted in try/catch; success adds the `member` relation, any throw is swallowed (`audience-feedback-controller.js:134-139`). No `unsafeAttrs`, no labs flag.
5. Status/headers: `headers.cacheInvalidate:false` (`feedback-members.js:8-10`) yields `X-Cache-Invalidate:false`; no explicit statusCode (200); default JSON response format, no stream.
6. Call chain: `query(frame)` → `feedbackService.service.controller.browse(frame)` (`feedback-members.js:25-27`) → `AudienceFeedbackController.browse` (`audience-feedback-controller.js:121-143`): `postId=frame.data.id`; options `limit||10`, `page||1`, `withMember=false` (`:122-127`); `score` parsed with `parseInt` when present (`:130-132`); permission gate sets `withMember=true` (`:134-139`); calls `repository.getForPost(postId, options)` (`:141`). `FeedbackRepository.getForPost` (`feedback-repository.js:74-107`): builds filter `post_id:'<id>'` plus `+score:<n>` (`:76-79`), `findOptions` with `order:'created_at DESC'`, `limit`, `page`, `filter` (`:81-86`), `withRelated:['member']` only if `withMember` (`:88-90`), calls `MemberFeedback.findPage(findOptions)` (`:93`), maps rows to `{id,score,created_at,member}` and returns `meta=results.meta` (`:95-106`). No events, jobs, mail, webhooks or third-party calls. Error branch: permission error caught/swallowed (`:137-139`); no explicit NotFound.
7. Cache: only `cacheInvalidate:false` → `X-Cache-Invalidate:false`; admin app `cacheControl('private')` (`endpoints/admin/app.js:24`); no cache service, key, TTL or tag in this path.
8. Output: result of `getForPost` = `{data:[{id,score,created_at,member?}], meta:{pagination}}` (`feedback-repository.js:95-106`); api-framework JSON envelope; empty post → `data:[]`, pagination total 0. `member` key only present when permission allowed relation (controller `:136,138`).

**C++**

1. Routes: `Router{router,"/feedback"}.use(require_auth).add_route(Route{"/:id"}.get(...))` → `GET /feedback/:id`. No `content_type_middleware` (Ghost has no body/Content-Type gate on this route); `require_auth` mirrors `authAdminApi`'s missing-token 401.
2. Middleware/auth: `ReadByIdHandler` sets `x-blog-context: ADMIN_API`, maps a presented admin token to the `Administrator` role with `*` permissions, then calls `blog_handlers_middleware::require_ghost_permission(req,res,next,send,"posts","browse")`. Denials produce Ghost-shaped `NoPermissionError`/`UnauthorizedError` envelopes via `write_error`.
3. In-service gate: `core::auth::ghost::GhostAuthorizer{}.authorize(principal_from_request(req),"members","browse")` models `permissions.canThis(...).browse.member()`; result published as `x-blog-with-member` for the step. Any throw/downgrade simply omits the relation.
4. Step Input `IRequest&`, Output `std::map<string,string>{status,body}`, `static blog::BlogContext* s_ctx`. `:id` from `last_segment(path)`; missing → 422 `ApiError.validation("Validation error","An id is required","id")`; `core::query::parse(path,{'limit','page','order','score','filter'})`; inject `filters(post_id=id)` and `score` via Ghost `parseInt`; `order` forced to `created_at desc`; `limit` defaults to 10; `fields` projects `{id,score,created_at,member}`.
5. Store calls: `ctx.store().json_for("members_feedback",{})` (generic `parse_array(list)` fallback), then `core::query::run(rows,q,10)` applies NQL filter/sort/page and builds Ghost's `meta.pagination`. With `withMember`, each row embeds `member` from `ctx.store().find_by_resource("members", member_id)`; without, the key is absent.
6. Pagination is entirely from `core::query::run` (`{"data":[...],"meta":{"pagination":{page,limit,pages,total,next,prev}}}`), status 200 — no hand-built pagination.
7. Shared helpers (`support::quote/unquote/field/text_field/error/last_segment/parse_int`) live in the already registered `blog_handlers:feedback_read_by_id_step` partition and are imported by the add step; no new partition/BUILD edit.
8. No outbox/cache-marker/fixture side effects; Ghost has none on this path.

**Diff** — no residual controller-level divergences. Real `MemberFeedback.findPage` meta/Knex pagination, `permissions.canThis(...).browse.member()` and DB filter/order semantics are reproduced with `core::query` + `GhostAuthorizer` + the existing store; the `member` relation is embedded when authorized. Non-wire-level differences (no literal Express/CORS/body-parser stack; admin token mapped to the `Administrator` role primitive) are not observable on the wire and are accepted, matching the convention of the other strict-parity flips.

### POST /api/feedback — MATCH

**Ghost**

1. Identity: `POST /api/feedback` in members app with middleware `bodyParser.json({limit:'50mb'})`, `middleware.loadMemberSession`, `middleware.authMemberByUuid`, `http(api.feedbackMembers.add)` (`members/app.js:144-150`). App-level mounts: `cacheControl('private')` (`:27`), `corsMiddleware` (`:30`), `createSessionFromMagicLink` (`:33`).
2. Operation: controller key `add`, `docName:'feedback'` (`feedback-members.js:5,30`).
3. Input: `validation.data.post_id {required:true}` (`:37-39`), `validation.data.score {required:true}` (`:40-42`); no declared `options[]`/`data[]`. Body shape consumed by service: `frame.data.feedback[0]` (`audience-feedback-controller.js:82`). No path params/query aliases/headers beyond member session.
4. Auth: `permissions:false` (`feedback-members.js:45`) — no permission check. Member identity from `loadMemberSession`/`authMemberByUuid` → `frame.options.context.member.id`; controller `#getMember` throws `InternalServerError` (`memberNotFound`) when missing (`audience-feedback-controller.js:71-79,92`).
5. Status/headers: `statusCode:201` (`feedback-members.js:31`); `headers.cacheInvalidate:false` (`:32-34`) → `X-Cache-Invalidate:false`.
6. Call chain: `query(frame)` → `feedbackService.service.controller.add(frame)` (`feedback-members.js:46-48`) → `AudienceFeedbackController.add` (`audience-feedback-controller.js:81-119`): `data=frame.data.feedback[0]`, `postId`, `score` (`:82-84`); score must be in `[0,1]` else `ValidationError invalidScore` (`:86-90`); `member=#getMember` (`:92`); `post=repository.getPostById(postId)` (`:94`; repository uses `Post.findOne({id,status:'all'})` at `feedback-repository.js:70-72`), missing → `NotFoundError postNotFound` (`:95-99`); `existing=repository.get(post.id,member.id)` (`:101`; `MemberFeedback.findOne({member_id,post_id},{require:false})` at `feedback-repository.js:48-64`); same score → return existing without save (`:103-105`); different score → `existing.score=score`, `repository.edit` (`MemberFeedback.edit({score},{id})` at `feedback-repository.js:37-46`), return existing (`:107-109`); else `new Feedback({memberId,postId,score})` (`:112-116`; `feedback.js:13-33` generates ObjectID id, default score 0) then `repository.add` (`MemberFeedback.add({id,member_id,post_id,score})` at `feedback-repository.js:28-35`) and return feedback (`:117-118`). Side effects: DB writes only; no events/jobs/mail/webhooks in this controller. Error branches: 422 ValidationError, 404 NotFoundError, 500 InternalServerError, 201 otherwise.
7. Cache: only `cacheInvalidate:false` → `X-Cache-Invalidate:false`; no cache service/key/TTL.
8. Output: `Feedback` object `{id, memberId, postId, score}` (or existing instance) serialized by api-framework with HTTP 201 (`feedback-members.js:31`); member-missing → 500, bad score → 422, missing post → 404.

**C++**

1. Routes: `Router{router,"/api"}.use(bearer_middleware).use(optional_auth).add_route(Route{"/feedback"}.post(...))` → `POST /api/feedback`; `permissions:false` agrees (no permission middleware).
2. Handler `add/handler.cppm`: status mapping 422→UNPROCESSABLE_CONTENT, 404→NOT_FOUND, 500→INTERNAL_SERVER_ERROR, success→CREATED (201 on create and edit); sets `X-Cache-Invalidate:false`; writes body and `send()`.
3. Step Input `IRequest&`, Output `std::map<string,string>{status,body}`, `static blog::BlogContext* s_ctx`. `core_serde::JsonCodec` decodes `data.feedback[0]` (root `feedback[0]` also accepted); empty/invalid → 422 `Invalid feedback body`; `post_id` required; `score` required and must be numeric 0/1 → 422 `ValidationError invalidScore` message.
4. Member identity: `X-Member-Id` (members-app authenticated session primitive used by the other members-facing handlers); absent → 500 `InternalServerError memberNotFound`. No raw bearer token used as member id.
5. Post existence: `ctx.store().find_by_resource("posts", post_id)` (status-independent like `status:'all'`); absent → 404 `NotFoundError postNotFound`. Post's stored id is used as the canonical `postId`.
6. Upsert: `json_for("members_feedback",{filter[post_id],filter[member_id]})`; existing + same score → return without save (preserves `updated_at`); existing + different score → `edit` with new score and refreshed `updated_at`; otherwise `create` with a random 24-hex ObjectID plus timestamps.
7. Output: Ghost `Feedback` wire object `{"id":...,"memberId":...,"postId":...,"score":...}`; 201 on every successful path; `X-Cache-Invalidate:false` before `send()`.
8. No outbox/cache-marker/fixture side effects beyond the in-memory resource-store mutation; Ghost has only DB writes.

**Diff** — no residual controller-level divergences. `getPostById(status:'all')` + 404, `Member`/`MemberFeedback` ObjectID generation, member-session identity + 500, always-201 status, unchanged-score no-save, and Ghost field naming are all reproduced with the shared store/serde primitives. Non-wire-level differences (no literal Express/body-parser/session middleware; in-memory store instead of Knex) are not observable on the wire and are accepted.

## Module-level notes

- Controller registration: `api.feedbackMembers` = `apiFramework.pipeline(require('./feedback-members'), localUtils, 'members')` (`api/endpoints/index.js:335-337`). Both routes share this one controller file; `browse` is bound only from the admin app, `add` only from the members app.
- Related but out-of-scope route: `GET /feedback/:postId/:score` (`members/app.js:154-156`) calls `audienceFeedbackService.service.controller.redirectToPost` directly (`audience-feedback-controller.js:47-66`, service link builders at `audience-feedback-service.js:25-51,75-82`), not the `feedback-members.js` controller. No C++ counterpart exists under `discovery/feedback/routes.cppm`; it is not counted in the verdict table because it does not bind this controller.
- Both handlers keep the established pattern: thin handler → single-step `core::workflow::Workflow` with `LocalWorkflowDriver`, `Input=IRequest&`, `Output=std::map<string,string>{status,body}`, static `s_ctx` pointing to `blog::BlogContext`. Both write `X-Cache-Invalidate:false`, mirroring Ghost's `headers.cacheInvalidate:false` on both ops.
- `members_feedback` is the shared resource string for both routes; it has no dedicated case in `BlogContext`, so all reads use the generic memory-backend `json_for` fallback and writes use generic `ResourceStore` create/edit. `m_fields["members_feedback"]={"id","score","member_id","post_id","created_at","updated_at"}` and default `score:"0"` are declared at `context/context_context.cppm:178,259`.
- Registration/wiring evidence for C++: `export import :feedback_routes` (`handlers/routes.cppm:22`); handler imports and accessors in `all.cppm:268-269,646-647,925-926`; module exports in `base.cppm:280-281`; build targets in `BUILD.bazel:853-858,982`. The add-step import of `:feedback_read_by_id_step` reuses the registered support partition; no shared-file edit was made.
- Verification note: `bazel build //include/blog/handlers:blog_handlers` is blocked by a pre-existing Bazel dependency cycle in the shared `blog_handlers` target; per task scope the cycle was not fixed.
