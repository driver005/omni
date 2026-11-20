# Parity: feedback-members

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/feedback-members.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (browse) and `docs/Ghost/ghost/core/server/web/members/app.js` (add)
- C++: `include/blog/handlers/discovery/feedback/routes.cppm` (`GET /feedback/:id` + `POST /api/feedback`)
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-19 (shared abstractions wired in)

Controller exports `browse` and `add` (`feedback-members.js:7,30`), bound as `api.feedbackMembers` (`endpoints/index.js:335-337`). Both Ghost routes have strict C++ counterparts.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Shared abstractions used (all already centrally registered; no shared-file edits):
`core::query` (`parse`/`run` → Ghost NQL filter + `meta.pagination`), `core_auth:ghost`
(`GhostAuthorizer`, `principal_from_request`) via
`blog_handlers_middleware::require_ghost_permission`, `core_serde::JsonCodec`,
existing `ApiDataStore`/`ResourceStore`. Shared local helpers live in the already
registered `blog_handlers:feedback_read_by_id_step` partition and are imported by
the add step — no new partition/BUILD entry.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/feedback/:id` (admin API base) | `browse` | GET `/feedback/:id` | `feedback_read_by_id().handle()` | GHOST: `data=['id']`, `options=['limit','page','order','score']`, validation `id` string required; service uses `limit`(dflt 10), `page`(dflt 1), `score` filter, `withMember` per permission. C++: `:id` from `last_segment(path)`; `core::query::parse(path,{'limit','page','order','score','filter'})`; injected `post_id`/`score` filters; `order` forced to `created_at desc`; limit defaults to 10; projection `{id,score,created_at,member}`. Missing id → 422 `ApiError.validation`. | GHOST: `{data:[{id,score,created_at,member?}], meta:<pagination>}` via `MemberFeedback.findPage`. C++: `core::query::run` emits the identical envelope (`page,limit,pages,total,next,prev`); status 200. | GHOST: `members_feedback` `findPage` with `post_id:'<id>'+score:<n>`, `order:'created_at DESC'`, `withRelated:['member']` when allowed. C++: `json_for("members_feedback",{})` → `core::query` filter/sort/page; member relation embedded via `find_by_resource("members", member_id)` when the in-service member-browse permission passes. | GHOST: `headers.cacheInvalidate:false`. C++: `res.set_header("X-Cache-Invalidate","false")`. | GHOST: `mw.authAdminApi` + permissions `{docName:'posts',method:'browse'}`; in-service `canThis(...).browse.member()` gates member relation. C++: `require_auth` + `require_ghost_permission(...,"posts","browse")` (Ghost 401/403 envelope); `GhostAuthorizer.authorize(principal,"members","browse")` decides `withMember` via `x-blog-with-member`. | MATCH | Ghost: `admin/routes.js:607`, `feedback-members.js:7-28`, `audience-feedback-controller.js:121-143`, `feedback-repository.js:74-106`. C++: `discovery/feedback/routes.cppm`, `read_by_id/handler.cppm`, `read_by_id/workflow/step/execute.cppm` |
| POST `/api/feedback` (members app) | `add` | POST `/api/feedback` | `feedback_add().handle()` | GHOST: `data.feedback[0]` with `post_id`, `score` required; score ∈ {0,1}; member id from `context.member.id`. C++: `core_serde::JsonCodec` decodes `data.feedback[0]`; validates `post_id` + numeric score ∈ {0,1}; member id from the members-app authenticated session (`X-Member-Id`, missing → 500 InternalServerError); `ApiError.validation` on invalid input. | GHOST: 201 + `Feedback` wire object `{id,memberId,postId,score}` on create **and** edit. C++: `AddHandler` maps 201/422/404/500 and returns exactly that wire object. | GHOST: `getPostById(postId,status:'all')` → 404 when absent; `get`/`edit`/`add` on `members_feedback`; skip save when score unchanged (preserve `updated_at`); 24-hex ObjectID on create. C++: `find_by_resource("posts",post_id)` existence check; upsert in `members_feedback`; unchanged-score short circuit; random 24-hex ObjectID. | GHOST: `headers.cacheInvalidate:false`. C++: `res.set_header("X-Cache-Invalidate","false")`. | GHOST: `loadMemberSession` + `authMemberByUuid`, `bodyParser.json({limit:'50mb'})`, `permissions:false`. C++: `bearer_middleware` + `optional_auth`, authenticated member-id header, `permissions:false` agrees. | MATCH | Ghost: `members/app.js:144-150`, `feedback-members.js:30-49`, `audience-feedback-controller.js:81-119`. C++: `discovery/feedback/routes.cppm`, `add/handler.cppm`, `add/workflow/flow/main.cppm`, `add/workflow/step/execute.cppm` |

## Route details

### GET `/feedback/:id` — MATCH

- **Route identity:** `router.get('/feedback/:id', mw.authAdminApi, ...)` (`admin/routes.js:607`) ↔ `Router{router,"/feedback"}.Route{"/:id"}.get(...)`. No `content_type_middleware` (Ghost has no body/Content-Type gate on this route).
- **Auth/permissions:** CLOSED. `require_auth` mirrors `authAdminApi`'s missing-token 401; `ReadByIdHandler` maps a presented admin token to the `Administrator` role and calls `require_ghost_permission(...,"posts","browse")`, producing Ghost-shaped NoPermissionError/UnauthorizedError envelopes.
- **Input:** CLOSED. `:id` from path; options allowlisted; `score` parsed with Ghost `parseInt` semantics and injected as an NQL filter; `limit` defaults to 10; `order` accepted but ignored (repository hard-codes `created_at DESC`).
- **Service gate / member relation:** CLOSED. `GhostAuthorizer.authorize(principal,"members","browse")` models `canThis(...).browse.member()`; on success each row embeds `member` from `find_by_resource("members", member_id)`, on denial the key is absent — matching Ghost's try/catch downgrade.
- **Query / pagination / output:** CLOSED. `core::query::run` handles NQL filtering, `created_at desc` sorting, page/offset and Ghost's `meta.pagination`; `fields` projects to `{id,score,created_at,member}`.
- **Cache:** CLOSED. `X-Cache-Invalidate:false` before `send()`.

### POST `/api/feedback` — MATCH

- **Route identity / op:** CLOSED. `Router{router,"/api"}.Route{"/feedback"}.post(...)` ↔ `handler.feedback_add()` → `blog::handlers::feedback::add::AddHandler`.
- **Input:** CLOSED. `data.feedback[0]` decoded with `core_serde::JsonCodec`; `post_id` required; `score` must be numeric 0/1 (Ghost `ValidationError invalidScore`); errors are Ghost envelopes.
- **Member identity:** CLOSED. Authenticated member id from the members-app session (`X-Member-Id`), not the raw bearer token; missing member → 500 `InternalServerError memberNotFound`, matching `#getMember`.
- **Post existence:** CLOSED. `find_by_resource("posts", post_id)` (status-independent, like `status:'all'`); absent → 404 `NotFoundError postNotFound`.
- **Upsert semantics:** CLOSED. Existing `(member,post)` pair with same score → returned without a save (preserves `updated_at`); different score → edit with new score; otherwise create with a random 24-hex ObjectID. Output is the Ghost `Feedback` wire object `{id,memberId,postId,score}`.
- **Status / cache:** CLOSED. `add` always answers 201 (create and edit); 422/404/500 on the documented error branches; `X-Cache-Invalidate:false` before `send()`.
- **Permissions:** agrees — `permissions:false`; no permission middleware.

## Missing in C++

None. Both controller keys (`browse`, `add`) have strict local route + handler/step/flow implementations.

(Out of scope: members-app `GET /feedback/:postId/:score` → `controller.redirectToPost` (`members/app.js:154-156`), which does not bind this controller's exported keys.)

## Extra in C++

None. `FeedbackRoutes::install` registers only the two Ghost-bound routes.

## Local files changed (strict-parity pass)

- `read_by_id/workflow/step/execute.cppm` — exported `blog::handlers::feedback::support` helpers (reused by add) + `core::query::run` browse step with member relation.
- `read_by_id/handler.cppm` — Ghost permission middleware + in-service member-browse gate.
- `routes.cppm` — removed `content_type_middleware` from the GET router.
- `add/handler.cppm` — 201/422/404/500 status mapping.
- `add/workflow/step/execute.cppm` — member-session id, post existence, ObjectID, unchanged-score skip, Ghost wire output.

## Shared-file edits

None. Central registrations from the earlier fix pass remain in place and were not touched (`all.cppm`, `base.cppm`, `routes.cppm`, `BUILD.bazel`, middleware/, context/).

## Verification note

`bazel build //include/blog/handlers:blog_handlers` is blocked by a pre-existing Bazel dependency cycle in the shared `blog_handlers` target; per task scope the cycle was not fixed. Parity is argued from the shared-abstraction contracts and existing registered usage patterns (labels, notifications).
