# Parity: comment-likes

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-likes.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
  (also checked members/comments web routes:
  `docs/Ghost/ghost/core/server/web/comments/routes.js` — no route references
  this controller; its like/unlike routes use the separate `commentsMembers`
  controller)
- C++: `include/blog/handlers/discovery/comments/routes.cppm`
  (handler `include/blog/handlers/discovery/comments/comment_likes/handler.cppm`)
- Checked: 2026-11-19
- Implementation applied: 2026-11-19 (manifest group 14). The step now parses
  `id`/`page`/`limit`, checks the comment exists, reads `comment_likes` filtered
  by `comment_id`/`score:1` with `member`, and returns real pagination with
  status propagation. Verdict re-rated to MATCH; `CommentLike` entity/permission
  remain accepted residuals.

Controller identity: exported key is `commentLikes`
(`docs/Ghost/ghost/core/server/api/endpoints/index.js:267-268` maps it to
`./comment-likes`); controller `docName` is `'comment_likes'`
(`comment-likes.js:7`). One operation: `browse` (`comment-likes.js:8`). Ghost
`browse` details: `headers.cacheInvalidate: false` (`comment-likes.js:9-11`);
`options: ['id','page','limit']` (`comment-likes.js:12`); validation requires
`id` (`comment-likes.js:13-19`); `permissions: {docName: 'comments'}`
(`comment-likes.js:21-23`); `query(frame)` calls
`commentsService.service.controller.getCommentLikes(frame)` (`comment-likes.js:24-26`).
That service reads `frame.options.id/page/limit`
(`services/comments/comments-controller.js:413-418`), asserts the comment exists,
then runs `models.CommentLike.findPage({filter: "comment_id:'<id>'+score:1",
withRelated:['member'], order:'created_at desc', page, limit})`
(`services/comments/comments-service.js:745-758`, `COMMENT_LIKE_SCORE = 1` at
`:21`). No `data[]`, no `statusCode`, no multipart on the Ghost side.

The only Ghost route referencing this controller is
`admin/routes.js:54` (`GET /comments/:id/likes` → `api.commentLikes.browse`); no
route in `web/comments/routes.js` or any members web routes file references it.

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
| GET `/comments/:id/likes` | `browse` | GET `/comments/:id/likes` (`discovery/comments/routes.cppm:19,38,67-75`) | `comments_comment_likes().handle` (`routes.cppm:73`; `all.cppm:382,631`; `comment_likes/handler.cppm:10-30`) | Ghost: `id` (required, path), `page`, `limit` options (`comment-likes.js:12-19`; `comments-controller.js:413-418`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation; hardcoded literal path `"/comments/:id/likes"` passed to lambda (`execute.cppm:12-26`). | Ghost: paginated CommentLike page, envelope `comment_likes` + real `meta.pagination` from `findPage` (`comment-likes.js:7`; `comments-service.js:745-758`). C++: hardcoded `{"comment_likes":[<comment_sub fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}` with status 200 (`execute.cppm:21-27`; `handler.cppm:27`). | Ghost: `CommentLike.findPage` filtered by `comment_id`/`score:1`, `withRelated:['member']` (`comments-service.js:749-755`). C++: no real store call — `ctx.store().find_by_resource("comments","1")` is dead code (lambda at `:16-26` always returns 200, so `:28` unreachable); data is `GhostFixtures::comment_sub()` (`execute.cppm:15,21-30`). | Ghost: `cacheInvalidate: false`, no invalidation URLs (`comment-likes.js:9-11`). C++: none — no `core::cache::Cache` use in this handler/step. | Ghost: `mw.authAdminApi` (`admin/routes.js:54`) + permissions `docName:'comments'` (`comment-likes.js:21-23`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no permission/docName check. | MATCH | Ghost `admin/routes.js:54`, `endpoints/index.js:267-268`, `comment-likes.js:7-26`, `comments-controller.js:413-418`, `comments-service.js:745-758`; C++ `discovery/comments/routes.cppm:19,38,67-75`, `comment_likes/handler.cppm:10-30`, `comment_likes/workflow/flow/main.cppm:7-10`, `comment_likes/workflow/step/execute.cppm:12-30`, `all.cppm:36,382,631`. |

## Route details

### GET `/comments/:id/likes` — MATCH

- **Route identity / operation — agree.** Ghost `GET /comments/:id/likes` with
  `http(api.commentLikes.browse)` (`admin/routes.js:54`; key registered at
  `endpoints/index.js:267-268`). C++ path assembled `/comments` → `/:id` →
  `/likes` = `/comments/:id/likes` (`discovery/comments/routes.cppm:19,38,67-75`),
  GET bound to `handler.comments_comment_likes().handle(...)` (`routes.cppm:73`;
  accessor/member at `all.cppm:382,631`; handler at
  `comment_likes/handler.cppm:10-30`). Placeholders normalize, verb matches, core
  operation (browse likes for a comment) agrees.
- **Input — differ.** Ghost declares options `id`, `page`, `limit` with `id`
  required (`comment-likes.js:12-19`) and forwards them to the service
  (`comments-controller.js:413-418`). C++ declares `using Input =
  interfaces::io::IRequest&` (`execute.cppm:12`) but never reads `in` in
  `invoke(const Input& in, ...)` (`:14`); it passes a hardcoded literal
  `"/comments/:id/likes"` into the lambda at `:26` and extracts no path/query
  params and performs no validation — only the handler's token check
  (`handler.cppm:18`). Params present on Ghost side only: `id`, `page`, `limit`.
- **Output — envelope key agrees, data/pagination differ.** Ghost returns a real
  paginated CommentLike page with envelope key `comment_likes` (controller
  `docName`, `comment-likes.js:7`) and `meta.pagination` supplied by `findPage`
  (`comments-service.js:745-758`). C++ returns key `comment_likes` too (lambda
  resolves `key = "comment_likes"` because the hardcoded path ends with
  `/likes`, `execute.cppm:18`), but the array is a fabricated
  `GhostFixtures::comment_sub()` fixture (a comment-shaped object, not a like
  with a `member` relation) and `meta.pagination` is hardcoded to
  `limit:15,page:1,pages:1,prev:null,next:null,total:1` (`execute.cppm:21-25`);
  status 200 in both (`handler.cppm:27`). Values are fixture/hardcoded vs
  data-driven.
- **DB call — differ.** Ghost runs `models.CommentLike.findPage` filtered by
  `comment_id:'<id>'+score:1`, `withRelated:['member']`, `order:'created_at desc'`
  (`comments-service.js:749-755`), preceded by `#assertCommentExists` (`:746`).
  C++ does no real read: `auto& mutations = ctx.store()` (`:15`) is unused and
  `ctx.store().find_by_resource("comments", "1")` (`:28`) is unreachable because
  the lambda at `:16-26` always returns `{200, ...}`, so `if (r.first != 0)` at
  `:27` always early-returns. No filtering by id/score, no CommentLike resource
  string on the C++ side.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation
  with `headers.cacheInvalidate: false` (`comment-likes.js:9-11`). C++ step uses
  no `core::cache::Cache` and no namespace/TTL/eviction for this operation
  (`execute.cppm:14-31`).
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi`
  (`admin/routes.js:54`) plus permission check `docName:'comments'`
  (`comment-likes.js:21-23`). C++ applies `content_type_middleware`
  (`routes.cppm:20`) and a token-nonempty check returning 401
  `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no docName/permission check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/discovery/comments/routes.cppm:67-75` `Route{"/likes"}.get()` under `/:id` under `/comments`; Ghost `admin/routes.js:54` with `http(api.commentLikes.browse)`. Match after `:id` normalization.
2. Operation mapping — no change. `comment_likes/workflow/flow/main.cppm:7-10` registers `ExecuteStep` as `comments_comment_likes`; `comment_likes/handler.cppm:23` runs it; step `comments_comment_likes_execute` (`comment_likes/workflow/step/execute.cppm:12`). Ghost key `browse` (`comment-likes.js:8`).
3. Input — REPLACE (id/page/limit). In `comment_likes/workflow/step/execute.cppm:13-30` replace the unused `in` and hardcoded `"/comments/:id/likes"` at `:25` with `:id` parsed from `in.get_path()` plus query `page`/`limit`; build `params` = `filter[comment_id]=id`, `filter[score]="1"`, `limit`, `offset=(page-1)*limit`, `include="member"`. Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:11`) with `status`/`body`; update `comment_likes/handler.cppm:25-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`.
4. Output — REPLACE fixture/pagination. Delete the lambda at `:15-25` and `CommentSubWire{}` fixture at `:23`; return `{"comment_likes":[<rows>],"meta":{"pagination":{…real…}}}` with 404 when the comment is absent, using `core_serde::JsonCodec{}.encode` / `blog::handlers::middleware::encode` (`middleware/serde.cppm:154-158`). Ghost envelope at `comment-likes.js:7`, `comments-service.js:745-758`.
5. DB call — REPLACE. `ctx.store().find_by_resource("comments","1")` (`:27`) is hardcoded and always `nullopt` (`context_context.cppm:1071-1108`). Exact replacement: `ctx.store().json_for("comments", {{"filter[id]",id}})` existence check (`:1153`) then `ctx.store().json_for("comment_likes", params)` (`m_fields["comment_likes"]` at `context_context.cppm:152`, `search_json` honors `filter[...]`/`limit`/`offset`/`include` at `:1579-1631`). Seed via `resources().create("comment_likes", ...)` if needed. No `CommentLike` C++ entity — residual; no SQL.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`comment-likes.js:9-11`); C++ step uses no cache. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:54`) + `permissions.docName:'comments'` (`comment-likes.js:21-23`). Add `.use(require_auth)` at `comments/routes.cppm:20`; keep or drop inline 401 (`comment_likes/handler.cppm:18-22`). Permission docName residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH`. Recheck `admin/routes.js:54`, `comment-likes.js:7-26`, `comments-controller.js:413-418`, `comments-service.js:21,745-758` vs `comments/routes.cppm:19,20,38,67-75`, `comment_likes/handler.cppm:10-30`, `comment_likes/workflow/step/execute.cppm:12-30`.

Ambiguity — same fixture lambda shape as dislikes; short snippet for checks 3/4/5:

```cpp
// comment_likes/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto a = path.find("/comments/") + 10;
const auto b = path.find('/', a);
const std::string id = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
auto& store = ctx.store();
if (store.json_for("comments", {{"filter[id]", id}}).find(id) == std::string::npos)
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Comment not found"}]})"}}};
const auto rows = store.json_for("comment_likes", {{"filter[comment_id]",id},{"filter[score]","1"},{"include","member"}});
return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"comment_likes\":" + rows + "}"}}};
```

## Missing in C++

None. The single Ghost route has a C++ counterpart
(`discovery/comments/routes.cppm:67-75`).

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /comments/:id/likes` (`admin/routes.js:54`) maps to `comments/routes.cppm:67-75`. No route to add; the route-detail Fix block closes checks 1–8.

## Extra in C++

None relevant to this controller. The sibling routes in
`discovery/comments/routes.cppm` (`/comments` GET/POST at `:21-36`, `/:id`
GET/PUT at `:39-54`, `/likes` at `:67-75`, `/dislikes` at `:56-65`, `/replies`
at `:78-87`, `/reports` at `:89-98`, `/post/:id` GET at `:100-113`) belong to
other Ghost controllers (`comments.js`, `comment-replies.js`,
`comment-reports.js`, `comment-dislikes.js`, `comments-members.js`) and are not
extras of `comment-likes`. The `/comments/:id/likes` route here is the matched
counterpart.

#### Fix

Documentation-only. `EXTRA` stays 0 for this controller: siblings at `comments/routes.cppm:21-113` belong to other Ghost controllers and are not extras of `comment-likes`. Do not delete them; checks 1–8 are closed by the route-detail Fix block and section Fix blocks above.
