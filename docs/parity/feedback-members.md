# Parity: feedback-members

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/feedback-members.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (browse) and `docs/Ghost/ghost/core/server/web/members/app.js` (add)
- C++: `include/blog/handlers/discovery/feedback/routes.cppm` (only `GET /feedback/:id`); no route for `POST /api/feedback`
- Checked: 2026-11-19

Controller exports `browse` and `add` (`feedback-members.js:7,30`), bound as `api.feedbackMembers` (`endpoints/index.js:335-337`). Two Ghost routes reference those keys; only one C++ route exists.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 1 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/feedback/:id` (admin API base) | `browse` | GET `/feedback/:id` | `feedback_read_by_id().handle()` | GHOST: `data=['id']`, `options=['limit','page','order','score']`, validation `id` string required, postId from `frame.data.id` → service uses `limit`(dflt 10), `page`(dflt 1), `score` filter, `withMember` per permission. C++: `using Input=IRequest&` — parses no params, never reads `:id`, hardcodes id `"1"` in unreachable code. | GHOST: `{data:[{id,score,created_at,member}], meta:<pagination>}` via `MemberFeedback.findPage`. C++: always status 200, body = serde of hardcoded `FeedbackResponse{}` → `{"id":"1","message":"Ghost feedback"}`; no `data`/`meta`, no pagination. `{"feedback":[...]}` branches are dead code. | GHOST: `members_feedback` (MemberFeedback) `findPage` with `post_id:'<id>'+score:<n>` filter, `order:'created_at DESC'`. C++: intended `find_by_resource("feedback","1")` is unreachable (`r.first==200` always returns early); resource string `"feedback"` is not the `members_feedback` resource in context. No real filtering/pagination. | GHOST: `headers.cacheInvalidate:false`. C++: none. | GHOST: `mw.authAdminApi` + permissions `docName:'posts', method:'browse'`. C++: `content_type_middleware` + handler token check → 401 `{"error":"Unauthorized"}`; no permission/docName model. | MISMATCH | Ghost: `admin/routes.js:607`, `feedback-members.js:7-28`, `audience-feedback-controller.js:121-143`, `feedback-repository.js:74-106`. C++: `discovery/feedback/routes.cppm:19-28`, `discovery/feedback/read_by_id/handler.cppm:17-27`, `.../workflow/step/execute.cppm:12-22` |
| POST `/api/feedback` (members app) | `add` | none found | none found | GHOST: `data.feedback[0]` with `post_id`, `score` required; score must be 0 or 1; member id from `context.member.id`. C++: n/a. | GHOST: statusCode 201; returns created/updated feedback model (`{id,score,member_id,post_id,...}`). C++: n/a. | GHOST: `getPostById`, `get`, `edit`, `add` on feedback repository / `members_feedback`. C++: n/a. | GHOST: `headers.cacheInvalidate:false`. C++: n/a. | GHOST: member auth `loadMemberSession` + `authMemberByUuid`, `bodyParser.json({limit:'50mb'})`, `permissions:false`. C++: n/a. | MISSING | Ghost: `members/app.js:144-150`, `feedback-members.js:30-49`, `audience-feedback-controller.js:81-119`. C++: grep of `include/blog/handlers/**/routes.cppm` for `feedback`/`/api/feedback`/`POST` found only the GET route (`discovery/feedback/routes.cppm:19-28`); route catalog lists only `GET /feedback/:id` (`include/blog/testing/route_catalog.cppm:282`). |

## Route details

### GET `/feedback/:id` — MISMATCH

- **Route identity:** Ghost `router.get('/feedback/:id', mw.authAdminApi, http(api.feedbackMembers.browse))` (`admin/routes.js:607`). C++ `Router{router,"/feedback"}.add_route(Route{"/:id"}.get(...))` → `GET /feedback/:id` (`discovery/feedback/routes.cppm:19-23`). Paths + verb agree.
- **Operation mapping:** Ghost op `browse` (`feedback-members.js:7`) → service `controller.browse` (`feedback-members.js:26`). C++ binds `handler.feedback_read_by_id().handle(...)` (`routes.cppm:28`; registered `all.cppm:584,833`, installed `handlers/routes.cppm:90`). Read-by-id operation agrees in name only.
- **Input divergence:** Ghost declares `data:['id']`, `options:['limit','page','order','score']`, requires string `id` (`feedback-members.js:11-19`); service reads `frame.data.id`, `limit`(10), `page`(1), `score`, and `withMember` gated by permission (`audience-feedback-controller.js:122-136`), building a `post_id`/`score` filter (`feedback-repository.js:76-86`). C++ `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) — no query/body/path parsing; the `:id` is never read; id `"1"` is hardcoded only in dead code (`execute.cppm:20`).
- **Output divergence:** Ghost repository returns `{data:[{id,score,created_at,member}], meta:results.meta}` (`feedback-repository.js:95-106`). C++ handler sets status OK with body from `encode(FeedbackResponse{})` (`handler.cppm:27`; `execute.cppm:16-19`), which serializes the DTO's `id="1"`/`message="Ghost feedback"` to `{"id":"1","message":"Ghost feedback"}` (`dto/more_resources.cppm:59-68,158-167`). No `data` envelope, no `meta.pagination`.
- **Dead DB call / wrong resource:** `execute.cppm:16-19` always sets `r.first=200`, so `if(r.first!=0) return r.second` (`execute.cppm:19`) returns immediately; `find_by_resource("feedback","1")` (`execute.cppm:20`) is unreachable. The intended resource string `"feedback"` also does not match the real `members_feedback` resource declared in `context.cppm:162` (fields `id,score,member_id,post_id,created_at,updated_at`). Ghost queries `members_feedback` via `findPage` (`feedback-repository.js:93`).
- **Cache:** Ghost sets `headers.cacheInvalidate:false` (`feedback-members.js:8-10`); C++ has no cache behavior — no cache key/namespace/TTL. Effectively both avoid invalidation, but no explicit C++ counterpart.
- **Auth/permissions:** Ghost uses `mw.authAdminApi` (`admin/routes.js:607`) plus `permissions:{docName:'posts',method:'browse'}` (`feedback-members.js:21-24`); the service further downgrades `withMember` when the member-browse permission fails (`audience-feedback-controller.js:135-139`). C++ uses `content_type_middleware` (`routes.cppm:20`) and a bare token presence check returning 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no permissions model, no docName/method, no member-vs-anonymous data shaping.

### POST `/api/feedback` — MISSING

- Ghost members-app route `membersApp.post('/api/feedback', bodyParser.json({limit:'50mb'}), middleware.loadMemberSession, middleware.authMemberByUuid, http(api.feedbackMembers.add))` (`members/app.js:144-150`), controller `add` with `statusCode:201`, required `post_id`+`score`, `permissions:false` (`feedback-members.js:30-49`), service validates score ∈ {0,1} and upserts feedback for the authenticated member (`audience-feedback-controller.js:81-119`).
- No C++ route, handler, or step exists. Confirmed by searching `include/blog/handlers/**/routes.cppm` for `feedback`, `/api/feedback`, `POST`, and `add` variants — only `discovery/feedback/routes.cppm:19-28` (`GET /:id`) matched; the route catalog lists only `GET /feedback/:id` (`include/blog/testing/route_catalog.cppm:282`). No `members_feedback` write path exists in the C++ handlers.

## Missing in C++

- `POST /api/feedback` (members app) → `feedbackMembers.add`. Ghost evidence: `members/app.js:144-150`, `feedback-members.js:30-49`, `audience-feedback-controller.js:81-119`. Absence confirmed: no POST/feedback route in `include/blog/handlers/**/routes.cppm`; `include/blog/testing/route_catalog.cppm:282` lists only `GET /feedback/:id`.

(Note: the separate members route `GET /feedback/:postId/:score` → `controller.redirectToPost` at `members/app.js:154-156` does not reference `browse`/`add` and is outside this controller's exported keys; no C++ counterpart found either.)

## Extra in C++

- None. The only C++ route in the matched folder (`GET /feedback/:id`, `discovery/feedback/routes.cppm:19-28`) has a Ghost counterpart (`admin/routes.js:607`).
