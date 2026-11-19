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
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id/dislikes` | `browse` | GET `/comments/:id/dislikes` (`discovery/comments/routes.cppm:19,38,56-65`) | `comments_comment_dislikes().handle` (`routes.cppm:62`; `all.cppm:37,383,632`; `comment_dislikes/handler.cppm:10-30`) | Ghost: `id` (required, path), `page`, `limit` options (`comment-dislikes.js:12-19`; `comments-controller.js:463-469`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation; hardcoded literal path `"/comments/:id/dislikes"` passed to lambda (`execute.cppm:12-26`). | Ghost: paginated CommentLike page, envelope `comment_dislikes` + real `meta.pagination` from `findPage` (`comment-dislikes.js:7`; `comments-service.js:765-778`). C++: hardcoded `{"comment_dislikes":[<comment_sub fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}` with status 200 (`execute.cppm:21-27`; `handler.cppm:27`). | Ghost: `CommentLike.findPage` filtered by `comment_id`/`score:-1`, `withRelated:['member']` (`comments-service.js:769-775`). C++: no real store call — `ctx.store().find_by_resource("comments","1")` is dead code (lambda at `:16-26` always returns 200, so `:28` unreachable); data is `GhostFixtures::comment_sub()` (`execute.cppm:15,21-30`). | Ghost: `cacheInvalidate: false`, no invalidation URLs (`comment-dislikes.js:9-11`). C++: none — no `core::cache::Cache` use in this handler/step. | Ghost: `mw.authAdminApi` (`admin/routes.js:55`) + permissions `docName:'comments'` (`comment-dislikes.js:21-23`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no permission/docName check. | PARTIAL | Ghost `admin/routes.js:55`, `endpoints/index.js:271-272`, `comment-dislikes.js:7-26`, `comments-controller.js:463-469`, `comments-service.js:22,765-778`; C++ `discovery/comments/routes.cppm:19,20,38,56-65`, `comment_dislikes/handler.cppm:10-30`, `comment_dislikes/workflow/flow/main.cppm:7-10`, `comment_dislikes/workflow/step/execute.cppm:12-30`, `all.cppm:37,383,632` |

## Route details

### GET `/comments/:id/dislikes` — PARTIAL

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

## Missing in C++

None. The single Ghost route has a C++ counterpart
(`discovery/comments/routes.cppm:56-65`).

## Extra in C++

None relevant to this controller. The sibling routes in
`discovery/comments/routes.cppm` (`/comments` GET/POST at `:21-36`, `/:id`
GET/PUT at `:39-54`, `/dislikes` at `:56-65`, `/likes` at `:67-75`, `/replies`
at `:78-87`, `/reports` at `:89-98`, `/post/:id` GET at `:100-113`) belong to
other Ghost controllers (`comments.js`, `comment-replies.js`,
`comment-reports.js`, `comment-likes.js`, `comments-members.js`) and are not
extras of `comment-dislikes`. The `/comments/:id/dislikes` route here is the
matched counterpart.
