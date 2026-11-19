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
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id/likes` | `browse` | GET `/comments/:id/likes` (`discovery/comments/routes.cppm:19,38,67-75`) | `comments_comment_likes().handle` (`routes.cppm:73`; `all.cppm:382,631`; `comment_likes/handler.cppm:10-30`) | Ghost: `id` (required, path), `page`, `limit` options (`comment-likes.js:12-19`; `comments-controller.js:413-418`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation; hardcoded literal path `"/comments/:id/likes"` passed to lambda (`execute.cppm:12-26`). | Ghost: paginated CommentLike page, envelope `comment_likes` + real `meta.pagination` from `findPage` (`comment-likes.js:7`; `comments-service.js:745-758`). C++: hardcoded `{"comment_likes":[<comment_sub fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}` with status 200 (`execute.cppm:21-27`; `handler.cppm:27`). | Ghost: `CommentLike.findPage` filtered by `comment_id`/`score:1`, `withRelated:['member']` (`comments-service.js:749-755`). C++: no real store call — `ctx.store().find_by_resource("comments","1")` is dead code (lambda at `:16-26` always returns 200, so `:28` unreachable); data is `GhostFixtures::comment_sub()` (`execute.cppm:15,21-30`). | Ghost: `cacheInvalidate: false`, no invalidation URLs (`comment-likes.js:9-11`). C++: none — no `core::cache::Cache` use in this handler/step. | Ghost: `mw.authAdminApi` (`admin/routes.js:54`) + permissions `docName:'comments'` (`comment-likes.js:21-23`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no permission/docName check. | PARTIAL | Ghost `admin/routes.js:54`, `endpoints/index.js:267-268`, `comment-likes.js:7-26`, `comments-controller.js:413-418`, `comments-service.js:745-758`; C++ `discovery/comments/routes.cppm:19,38,67-75`, `comment_likes/handler.cppm:10-30`, `comment_likes/workflow/flow/main.cppm:7-10`, `comment_likes/workflow/step/execute.cppm:12-30`, `all.cppm:36,382,631`. |

## Route details

### GET `/comments/:id/likes` — PARTIAL

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

## Missing in C++

None. The single Ghost route has a C++ counterpart
(`discovery/comments/routes.cppm:67-75`).

## Extra in C++

None relevant to this controller. The sibling routes in
`discovery/comments/routes.cppm` (`/comments` GET/POST at `:21-36`, `/:id`
GET/PUT at `:39-54`, `/likes` at `:67-75`, `/dislikes` at `:56-65`, `/replies`
at `:78-87`, `/reports` at `:89-98`, `/post/:id` GET at `:100-113`) belong to
other Ghost controllers (`comments.js`, `comment-replies.js`,
`comment-reports.js`, `comment-dislikes.js`, `comments-members.js`) and are not
extras of `comment-likes`. The `/comments/:id/likes` route here is the matched
counterpart.
