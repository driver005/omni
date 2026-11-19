# Parity: comment-replies

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-replies.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 51-52). No routes in `web/comments/routes.js` or other web `routes.js` reference key `commentReplies` (that file uses `api.commentsMembers.*`).
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (installed via `include/blog/handlers/routes.cppm:9`)
- C++ handler registry: `include/blog/handlers/all.cppm:32-34,379-380,627-629`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id` | `commentReplies.read` → `adminRead` → `getCommentByID(data.id, {isAdmin:true})` | GET `/comments/:id` (normalized `:id`) | `comments_read_by_id` (`ReadByIdHandler`) | Ghost: `include`, `fields`, `impersonate_member_uuid` + `data:id,email`, include enum validation. C++: parses none; id hardcoded `"1"`. | Ghost: comment model via service framework. C++: `{"comments":[<fixture>]}`, no meta. | Ghost: `commentsService.getCommentByID`. C++: `find_by_resource("comments","1")`. | Ghost: `cacheInvalidate:false`. C++: none. | Ghost: `authAdminApi` + `permissions:true`, docName `comments`. C++: token-presence only + `content_type_middleware`. | PARTIAL | Ghost `admin/routes.js:51`, `comment-replies.js:53-68`, `comments-controller.js:259-267`; C++ `discovery/comments/routes.cppm:38-46`, `all.cppm:379,628`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:13-18` |
| GET `/comments/:id/replies` | `commentReplies.browse` → `adminReplies` → `getAdminReplies(id, options)` with `order:'created_at asc'` | GET `/comments/:id/replies` (nested `Route{"/replies"}`) | `comments_comment_replies` (`CommentRepliesHandler`) | Ghost: `include,page,limit,fields,filter,order,debug,id,impersonate_member_uuid`, required `id` validation. C++: parses none. | Ghost: replies + real pagination from service. C++: hardcoded `{"comments":[<fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}`. | Ghost: `commentsService.getAdminReplies`. C++: no live store read — `find_by_resource("comments","1")` at lines 28-29 is unreachable (lambda returns `r.first=200` → early return at line 27). | Ghost: `cacheInvalidate:false`. C++: none. | Ghost: `authAdminApi` + `permissions:true`, docName `comments`. C++: token-presence only + `content_type_middleware`. | PARTIAL | Ghost `admin/routes.js:52`, `comment-replies.js:26-52`, `comments-controller.js:239-246`; C++ `discovery/comments/routes.cppm:77-87`, `all.cppm:380,629`, `comment_replies/handler.cppm:17-27`, `comment_replies/workflow/step/execute.cppm:14-30` |

## Route details

### GET /comments/:id — PARTIAL

- Route identity + operation: Ghost `admin/routes.js:51` binds `http(api.commentReplies.read)`; C++ `discovery/comments/routes.cppm:38-46` binds `handler.comments_read_by_id()` to the same normalized path and verb. Core operation (read one comment) agrees.
- Input: Ghost `comment-replies.js:57-63` accepts `include` (restricted to `ALLOWED_INCLUDES`, `comment-replies.js:4-21`), `fields`, `impersonate_member_uuid`, plus `data: ['id','email']`. C++ `read_by_id/workflow/step/execute.cppm:12-18` declares `using Input=IRequest&` but never reads it; the id is hardcoded via `std::string{}.empty()?"1":std::string{}` (line 16) which always evaluates to `"1"`. No `include`/`fields`/`email`/`impersonate_member_uuid` handling on C++ side.
- Output: C++ returns `{"comments":[...]}` only (`execute.cppm:17-18`); no `meta`, no status fields beyond handler-level `200 OK` (`read_by_id/handler.cppm:27`). Ghost relies on the service/serializer for the single-comment result.
- DB call: Ghost `comments-controller.js:259-267` calls `getCommentByID(data.id, {..., isAdmin:true})` with impersonation context; C++ calls `ctx.store().find_by_resource("comments","1")` (`execute.cppm:16`) — real store call, but fixed id `"1"`, no admin flag, no impersonation.
- Cache: Ghost `comment-replies.js:54-56` sets `headers.cacheInvalidate:false` (no invalidation URL emitted by `adminRead`); C++ handler/step contains no cache read, namespace, key, or TTL. Effectively "none" on both, but expressed differently.
- Auth: Ghost route middleware `mw.authAdminApi` (`admin/routes.js:51`) plus `permissions:true` and `docName:'comments'` (`comment-replies.js:25,64`). C++ `read_by_id/handler.cppm:18-22` only checks `token(req).empty()` → `401 {"error":"Unauthorized"}`; no role/permission/docName check. `content_type_middleware` applies from the parent router (`discovery/comments/routes.cppm:20`).

### GET /comments/:id/replies — PARTIAL

- Route identity + operation: Ghost `admin/routes.js:52` binds `http(api.commentReplies.browse)`; C++ `discovery/comments/routes.cppm:77-87` binds `handler.comments_comment_replies()` to `/comments/:id/replies`. Both mean "list replies to a comment."
- Input: Ghost `comment-replies.js:30-47` allows `include,page,limit,fields,filter,order,debug,id,impersonate_member_uuid` with `id` required; `comments-controller.js:239-243` forces `order:'created_at asc'` and applies impersonation. C++ `comment_replies/workflow/step/execute.cppm:12-26` declares `IRequest&` input but ignores it — no query/body parsing, no `page`/`limit`/`order`/`id`/`impersonate_member_uuid`, no impersonation context.
- Output: C++ always returns the hardcoded envelope built at `execute.cppm:21-25` because the lambda at line 16-26 returns `r.first = 200`, triggering the early return at line 27. Envelope key is `"comments"` (path suffix `/replies` matches none of the `/likes`, `/dislikes`, `/reports` branches at lines 18-20, so `key` stays `"comments"`), with fabricated pagination `limit:15, page:1, pages:1, prev:null, next:null, total:1`. Ghost derives pagination from `getAdminReplies` (`comments-controller.js:245`). Lines 28-29 (a real `find_by_resource("comments","1")` read) are dead code.
- DB call: Ghost `comments-controller.js:245` → `commentsService.getAdminReplies(id, options)`. C++ no live query on the executed path; the only resource string present is `"comments"` in unreachable code (`execute.cppm:28`).
- Cache: Ghost `comment-replies.js:27-29` sets `headers.cacheInvalidate:false`; `adminReplies` emits no `X-Cache-Invalidate` (that header is only set in `add`/`edit`, e.g. `comments-controller.js:224`). C++ has no cache behavior. "none" on both.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:52`) + `permissions:true`, `docName:'comments'` (`comment-replies.js:25,48`). C++ token-presence check only (`comment_replies/handler.cppm:18-22`), inherited `content_type_middleware` (`discovery/comments/routes.cppm:20`); no permission/docName/docName-based validation, no `ALLOWED_INCLUDES` enforcement.

## Missing in C++

None. Both Ghost `commentReplies` routes have exact path+verb C++ counterparts:
- `GET /comments/:id` → `comments_read_by_id`
- `GET /comments/:id/replies` → `comments_comment_replies`

## Extra in C++

None attributable to this controller. The matched C++ folder `discovery/comments/routes.cppm` also installs `GET /comments`, `POST /comments`, `GET/PUT /comments/:id`, `GET /comments/:id/likes`, `/dislikes`, `/reports`, and `GET /comments/post/:id` (`routes.cppm:21-113`). Each has a Ghost counterpart in a different controller (`comments.js`, `comment-likes.js`, `comment-dislikes.js`, `comment-reports.js`), e.g. `admin/routes.js:50,53-58` — so they are not extra; they are simply outside the `comment-replies.js` scope.
