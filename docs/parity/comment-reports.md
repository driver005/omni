# Parity: comment-reports

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-reports.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (route install) + `include/blog/handlers/discovery/comments/comment_reports/` (handler/flow/step)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id/reports` | `browse` (`api.commentReports.browse`) | GET `/comments/:id/reports` (`Router "/comments"` → `"/:id"` → `Route{"/reports"}`) | `comments_comment_reports()` → `CommentReportsHandler` | Ghost: `options ['id','page','limit']`, `id` required. C++: no query parsing; path hardcoded `"/comments/:id/reports"`. | Ghost: `findPage` result → `{comment_reports:[…], meta.pagination}` via API framework. C++: hardcoded 1 `comment_sub()` fixture + fixed `limit:15,page:1,pages:1,total:1`. | Ghost: `models.CommentReport.findPage({filter:"comment_id:'<id>'", withRelated:['member'], order:'created_at desc', page, limit})`. C++: no real call; fixture only; dead-code fallback `find_by_resource("comments","1")` (unreachable, `r.first==200`). | Ghost: `headers.cacheInvalidate:false` (no invalidation). C++: none. | Ghost: `mw.authAdminApi` + `permissions.docName:'comments'`. C++: `content_type_middleware` + handler `token(req).empty()` → 401 (no permission docName). | PARTIAL | Ghost: `admin/routes.js:53`, `comment-reports.js:7-27`, `comments-controller.js:401-407`, `comments-service.js:725-737`. C++: `comments/routes.cppm:19-20,38,88-98`, `comment_reports/handler.cppm:17-27`, `comment_reports/workflow/step/execute.cppm:12-30`, `fixtures.cppm:363-374`. |

## Route details

### GET /comments/:id/reports — PARTIAL

Route identity and core operation agree (both browse reporters for a comment; both emit a `comment_reports` resource key). Divergences below:

- **Input**: Ghost declares `options: ['id','page','limit']` with `id` required (`comment-reports.js:12-19`) and forwards `page`/`limit` to the service (`comments-controller.js:402-406`). C++ declares `using Input = interfaces::io::IRequest&` but parses no query string; the path string `"/comments/:id/reports"` is hardcoded at the lambda call site (`execute.cppm:12,16,26`). `id`, `page`, `limit` present on Ghost side only; no C++ validation of required `id`.
- **Output**: Ghost returns the `findPage` result; the API framework wraps it with a `comment_reports` array and real `meta.pagination` (`comments-service.js:729-737`). C++ returns a fixed envelope with one `comment_sub()` fixture object and hardcoded `limit:15, page:1, pages:1, prev:null, next:null, total:1` (`execute.cppm:21-25`). No real pagination.
- **DB call**: Ghost queries `models.CommentReport.findPage` filtering `comment_id`, eager-loading `member`, ordering `created_at desc` (`comments-service.js:729-735`). C++ performs no live DB read: the response is `GhostFixtures::comment_sub()` (`execute.cppm:23-24`, `fixtures.cppm:363-374`, which does include a `member` sub-object but is static). The only store call, `find_by_resource("comments","1")` (`execute.cppm:28`), is unreachable because `r.first` is always `200` and returns at line 27 — different resource string (`comments`, not `comment_reports`) and no filtering.
- **Cache**: Ghost explicitly sets `headers.cacheInvalidate:false` (`comment-reports.js:9-11`). C++ has no cache behavior (no `core::cache::Cache` key/namespace/TTL in handler, flow, or step). Functionally both avoid invalidation, but the directive exists only on the Ghost side.
- **Auth**: Ghost applies `mw.authAdminApi` (`admin/routes.js:53`) and `permissions.docName:'comments'` with a comment-permission rationale (`comment-reports.js:20-23`). C++ applies `content_type_middleware` at the `/comments` router (`comments/routes.cppm:20`) plus a bearer-token presence check in the handler returning `401 {"error":"Unauthorized"}` (`handler.cppm:18-22`) and no permission/docName enforcement. Auth mechanisms differ.
- **Confirmation of no other counterpart**: grep for `commentReports|comment-reports|comment_reports|reports` across `docs/Ghost/ghost/core/server/web` returned only `admin/routes.js:53`; grep across `include/blog/handlers` located the single matching C++ route at `comments/routes.cppm:89-97` bound to `comments_comment_reports()` (registry `all.cppm:381,630`).

## Missing in C++

None. The single Ghost route (GET `/comments/:id/reports`) has a C++ counterpart.

## Extra in C++

None for this domain. The `comment_reports` folder exposes one handler bound to one route (GET `/comments/:id/reports`), which has a Ghost controller route. Other routes in `discovery/comments/routes.cppm` (browse/create/read/edit, likes, dislikes, replies, post detail) belong to other Ghost controllers and are out of scope here.
