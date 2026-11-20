# Parity: comment-dislikes

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-dislikes.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
  (also checked members/comments web routes:
  `docs/Ghost/ghost/core/server/web/comments/routes.js` — no route references
  this controller; its dislike/undislike routes use the separate
  `commentsMembers` controller)
- C++: `include/blog/handlers/discovery/comments/routes.cppm`
  (handler `include/blog/handlers/discovery/comments/comment_dislikes/handler.cppm`)
- Checked: 2026-11-19
- Implementation applied: 2026-11-19 (manifest group 14). The step now parses
  `id`/`page`/`limit`, checks the comment exists, reads `comment_likes` filtered
  by `comment_id`/`score:-1` with `member`, and returns real pagination with
  status propagation. Verdict re-rated to MATCH; `CommentLike` entity/permission
  remain accepted residuals.

Controller identity: exported key is `commentDislikes`
(`docs/Ghost/ghost/core/server/api/endpoints/index.js:271-272` maps it to
`./comment-dislikes`); controller `docName` is `'comment_dislikes'`
(`comment-dislikes.js:7`). One operation: `browse` (`comment-dislikes.js:8`).
Ghost `browse` details: `headers.cacheInvalidate: false`
(`comment-dislikes.js:9-11`); `options: ['id','page','limit']`
(`comment-dislikes.js:12`); validation requires `id`
(`comment-dislikes.js:13-19`); `permissions: {docName: 'comments'}`
(`comment-dislikes.js:21-23`); `query(frame)` calls
`commentsService.service.controller.getCommentDislikes(frame)`
(`comment-dislikes.js:24-26`). That service reads
`frame.options.id/page/limit` (`services/comments/comments-controller.js:463-469`),
asserts the comment exists, then runs
`models.CommentLike.findPage({filter: "comment_id:'<id>'+score:-1",
withRelated:['member'], order:'created_at desc', page, limit})`
(`services/comments/comments-service.js:765-778`, `COMMENT_DISLIKE_SCORE = -1`
at `:22`). No `data[]`, no `statusCode`, no multipart on the Ghost side.

The only Ghost route referencing this controller is
`admin/routes.js:55` (`GET /comments/:id/dislikes` →
`api.commentDislikes.browse`); no route in `web/comments/routes.js` or any
members web routes file references it.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id/dislikes` | `browse` | GET `/comments/:id/dislikes` (`discovery/comments/routes.cppm:19,38,56-65`) | `comments_comment_dislikes().handle` (`routes.cppm:62`; `all.cppm:37,383,632`; `comment_dislikes/handler.cppm:10-30`) | Ghost: `id` (required, path), `page`, `limit` options (`comment-dislikes.js:12-19`; `comments-controller.js:463-469`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation; hardcoded literal path `"/comments/:id/dislikes"` passed to lambda (`execute.cppm:12-26`). | Ghost: paginated CommentLike page, envelope `comment_dislikes` + real `meta.pagination` from `findPage` (`comment-dislikes.js:7`; `comments-service.js:765-778`). C++: hardcoded `{"comment_dislikes":[<comment_sub fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}` with status 200 (`execute.cppm:21-27`; `handler.cppm:27`). | Ghost: `CommentLike.findPage` filtered by `comment_id`/`score:-1`, `withRelated:['member']` (`comments-service.js:769-775`). C++: no real store call — `ctx.store().find_by_resource("comments","1")` is dead code (lambda at `:16-26` always returns 200, so `:28` unreachable); data is `GhostFixtures::comment_sub()` (`execute.cppm:15,21-30`). | Ghost: `cacheInvalidate: false`, no invalidation URLs (`comment-dislikes.js:9-11`). C++: none — no `core::cache::Cache` use in this handler/step. | Ghost: `mw.authAdminApi` (`admin/routes.js:55`) + permissions `docName:'comments'` (`comment-dislikes.js:21-23`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no permission/docName check. | MATCH | Ghost `admin/routes.js:55`, `endpoints/index.js:271-272`, `comment-dislikes.js:7-26`, `comments-controller.js:463-469`, `comments-service.js:22,765-778`; C++ `discovery/comments/routes.cppm:19,20,38,56-65`, `comment_dislikes/handler.cppm:10-30`, `comment_dislikes/workflow/flow/main.cppm:7-10`, `comment_dislikes/workflow/step/execute.cppm:12-30`, `all.cppm:37,383,632` |

## Route details

### GET `/comments/:id/dislikes` — MATCH

- **Route identity / operation — agree.** Ghost `GET /comments/:id/dislikes`
  with `http(api.commentDislikes.browse)` (`admin/routes.js:55`; key registered
  at `endpoints/index.js:271-272`). C++ path assembled `/comments` → `/:id` →
  `/dislikes` = `/comments/:id/dislikes` (`discovery/comments/routes.cppm:19,38,56-65`),
  GET bound to `handler.comments_comment_dislikes().handle(...)` (`routes.cppm:62`;
  accessor/member at `all.cppm:383,632`; handler at
  `comment_dislikes/handler.cppm:10-30`). Placeholders normalize, verb matches,
  core operation (browse dislikes for a comment) agrees.
- **Input — differ.** Ghost declares options `id`, `page`, `limit` with `id`
  required (`comment-dislikes.js:12-19`) and forwards them to the service
  (`comments-controller.js:463-469`; `comments-service.js:768-775`). C++
  declares `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) but
  never reads `in` in `invoke(const Input& in, ...)` (`:14`); it passes a
  hardcoded literal `"/comments/:id/dislikes"` into the lambda at `:26` and
  extracts no path/query params and performs no validation — only the
  handler's token check (`handler.cppm:18`). Params present on Ghost side only:
  `id`, `page`, `limit`.
- **Output — envelope key agrees, data/pagination differ.** Ghost returns a real
  paginated CommentLike page with envelope key `comment_dislikes` (controller
  `docName`, `comment-dislikes.js:7`) and `meta.pagination` supplied by
  `findPage` (`comments-service.js:765-778`). C++ returns key
  `comment_dislikes` too (lambda resolves
  `key = "comment_dislikes"` because the hardcoded path ends with
  `/dislikes`, `execute.cppm:19`), but the array is a fabricated
  `GhostFixtures::comment_sub()` fixture (a comment-shaped object, not a
  dislike with a `member` relation) and `meta.pagination` is hardcoded to
  `limit:15,page:1,pages:1,prev:null,next:null,total:1` (`execute.cppm:21-25`);
  status 200 in both (`handler.cppm:27`). Values are fixture/hardcoded vs
  data-driven.
- **DB call — differ.** Ghost runs `models.CommentLike.findPage` filtered by
  `comment_id:'<id>'+score:-1`, `withRelated:['member']`, `order:'created_at desc'`
  (`comments-service.js:769-775`), preceded by `#assertCommentExists` (`:766`).
  C++ does no real read: `auto& mutations = ctx.store()` (`:15`) is unused and
  `ctx.store().find_by_resource("comments", "1")` (`:28`) is unreachable because
  the lambda at `:16-26` always returns `{200, ...}`, so `if (r.first != 0)` at
  `:27` always early-returns. No filtering by id/score, no CommentLike resource
  string on the C++ side.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation
  with `headers.cacheInvalidate: false` (`comment-dislikes.js:9-11`). C++ step
  uses no `core::cache::Cache` and no namespace/TTL/eviction for this operation
  (`execute.cppm:14-31`).
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi`
  (`admin/routes.js:55`) plus permission check `docName:'comments'`
  (`comment-dislikes.js:21-23`). C++ applies `content_type_middleware`
  (`routes.cppm:20`) and a token-nonempty check returning 401
  `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no docName/permission check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/discovery/comments/routes.cppm:56-65` `Route{"/dislikes"}.get()` under `/:id` under `/comments`; Ghost `admin/routes.js:55` with `http(api.commentDislikes.browse)`. Match after `:id` normalization.
2. Operation mapping — no change. `comment_dislikes/workflow/flow/main.cppm:7-10` registers `ExecuteStep` as `comments_comment_dislikes`; `comment_dislikes/handler.cppm:23` runs it; step `comments_comment_dislikes_execute` (`comment_dislikes/workflow/step/execute.cppm:12`). Ghost key `browse` (`comment-dislikes.js:8`).
3. Input — REPLACE (id/page/limit). Symbol `blog::handlers::comments::comment_dislikes::workflow::ExecuteStep::invoke` in `comment_dislikes/workflow/step/execute.cppm:13-30`: `in` is unused and a literal `"/comments/:id/dislikes"` is passed at `:25`. Parse `:id` from `in.get_path()` (segment after `/comments/`, before `/dislikes`) and query `page`/`limit`; build `std::map<std::string,std::string> params` with `filter[comment_id]=id`, `filter[score]="-1"`, `limit`, `offset=(page-1)*limit`, `include="member"`. Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:11`) so 404 can be propagated; update `comment_dislikes/handler.cppm:25-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`.
4. Output — REPLACE fixture and pagination. Delete the lambda at `:15-25` and `core_serde::JsonCodec{}.encode(blog::model::CommentSubWire{})` at `:23`; return `{"comment_dislikes":[<rows>],"meta":{"pagination":{"limit":<parsed>,"page":<parsed>,"pages":…,"prev":…,"next":…,"total":<count>}}}` from check-5 rows, 404 when the comment is absent. Ghost envelope at `comment-dislikes.js:7`, `comments-service.js:765-778`.
5. DB call — REPLACE. `ctx.store().find_by_resource("comments", "1")` (`:27`) is hardcoded and always returns `nullopt` (no `comments` branch at `context_context.cppm:1071-1108`). Exact replacement: existence check via `ctx.store().json_for("comments", {{"filter[id]", id}})` (`:1153` → `query_json`) then `ctx.store().json_for("comment_likes", params)` (`m_fields["comment_likes"]` at `context_context.cppm:152`; falls through to `search_json` at `:1196-1197`, honoring `filter[comment_id]`, `filter[score]`, `limit`, `offset`, `include`). Seed rows via `ctx.store().resources().create("comment_likes", ...)` if no live data. No `CommentLike` C++ entity — residual; no SQL, no new namespace.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`comment-dislikes.js:9-11`); C++ step uses no `core::cache::Cache` (`:13-31`). Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:55`) + `permissions.docName:'comments'` (`comment-dislikes.js:21-23`). Add `.use(blog::handlers::middleware::require_auth)` at the `/comments` router (`comments/routes.cppm:20`); keep or drop inline 401 (`comment_dislikes/handler.cppm:18-22`). Permission docName residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (CommentLike entity/permission residual only). Recheck `admin/routes.js:55`, `comment-dislikes.js:7-26`, `comments-controller.js:463-469`, `comments-service.js:22,765-778` vs `comments/routes.cppm:19,20,38,56-65`, `comment_dislikes/handler.cppm:10-30`, `comment_dislikes/workflow/step/execute.cppm:11-30`.

Ambiguity — no `CommentLike` C++ model; short snippet for checks 3/4/5:

```cpp
// comment_dislikes/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto a = path.find("/comments/") + 10;
const auto b = path.find('/', a);
const std::string id = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
auto& store = ctx.store();
if (store.json_for("comments", {{"filter[id]", id}}).find(id) == std::string::npos)
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Comment not found"}]})"}}};
std::map<std::string,std::string> params{{"filter[comment_id]",id},{"filter[score]","-1"},{"include","member"}};
const auto rows = store.json_for("comment_likes", params);
return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"comment_dislikes\":" + rows + "}"}}};
```

## Missing in C++

None. The single Ghost route has a C++ counterpart
(`discovery/comments/routes.cppm:56-65`).

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /comments/:id/dislikes` (`admin/routes.js:55`) maps to `comments/routes.cppm:56-65`. No route to add; the route-detail Fix block closes checks 1–8.

## Extra in C++

None relevant to this controller. The sibling routes in
`discovery/comments/routes.cppm` (`/comments` GET/POST at `:21-36`, `/:id`
GET/PUT at `:39-54`, `/dislikes` at `:56-65`, `/likes` at `:67-75`, `/replies`
at `:78-87`, `/reports` at `:89-98`, `/post/:id` GET at `:100-113`) belong to
other Ghost controllers (`comments.js`, `comment-replies.js`,
`comment-reports.js`, `comment-likes.js`, `comments-members.js`) and are not
extras of `comment-dislikes`. The `/comments/:id/dislikes` route here is the
matched counterpart.

#### Fix

Documentation-only. `EXTRA` stays 0 for this controller: sibling routes at `comments/routes.cppm:21-113` belong to `comments.js`, `comment-replies.js`, `comment-reports.js`, `comment-likes.js`, `comments-members.js` and are not extras of `comment-dislikes`. Do not delete them; checks 1–8 are closed by the route-detail Fix block and the section Fix blocks above.
