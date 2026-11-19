# Parity: comments

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comments.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (member-facing comment app `docs/Ghost/ghost/core/server/web/comments/routes.js` uses a different controller, `api.commentsMembers.*`, so it is out of scope for this controller)
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (installed via `include/blog/handlers/routes.cppm:9`; handlers registered in `include/blog/handlers/all.cppm:32-40,378-386,627-635`)
- Checked: 2026-11-19

Scope: the exported keys of `comments.js` are `edit`, `browse`, `browseAll`, `add` (`comments.js:33-149`). Grep `api\.comments\.` over `docs/Ghost/ghost/core/server/web/**` returns exactly four routes, all in `admin/routes.js:50,56,57,58`. The `/comments/:id`, `/replies`, `/reports`, `/likes`, `/dislikes` admin paths (`admin/routes.js:51-55`) belong to `commentReplies`, `commentReports`, `commentLikes`, `commentDislikes` controllers — not this one.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 4 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 5 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments` | `browseAll` → `adminBrowseAll` | GET `/comments` | `comments_browse` | Ghost: `page`,`limit`,`filter`,`order`,`include_nested`; C++: none parsed (`Input=IRequest&`, `json_for(...,{})`) — all option names absent | Ghost `{data,meta.pagination}` real values; C++ `{"comments":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` hardcoded; status 200 both | Ghost `service.getAdminAllComments` with filter/page/limit/order; C++ `ctx.store().json_for("comments", {})` — no filter/pagination | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions.method:'browse'`; C++ bearer-token check + `content_type_middleware` | PARTIAL | G:`admin/routes.js:50`, `comments.js:80-92`, `comments-controller.js:168-187`; C++:`routes.cppm:19-28`, `browse/handler.cppm:17-28`, `browse/.../execute.cppm:11,16-17` |
| GET `/comments/post/:post_id` | `browse` → `adminBrowse` | GET `/comments/post/:id` (`:id` ↔ `:post_id`) | `comments_get_detail` | Ghost: `post_id` (required),`include`,`page`,`limit`,`fields`,`filter`,`order`,`debug`,`impersonate_member_uuid`; C++: none parsed, `post_id` ignored | Ghost paginated `{data,meta}`; C++ `{"comments":[...],"meta":{...hardcoded limit15/page1/total0}}`; status 200 both | Ghost `service.getAdminComments` with post filter + options; C++ `json_for("comments", {})` — no post filter | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | PARTIAL | G:`admin/routes.js:56`, `comments.js:53-79`, `comments-controller.js:143-157`; C++:`routes.cppm:100-112`, `get_detail/handler.cppm:17-28`, `get_detail/.../execute.cppm:11,16-17` |
| POST `/comments` | `add` → `api.commentOnPost`/`replyToComment` | POST `/comments` | `comments_create` | Ghost `include` values `post,member,replies,replies.member`; data `member_id`,`html` required; `post_id` xor `parent_id`; `created_at` validated; `context.internal=true`; C++: raw `body_as_string()` only, no validation/options | Ghost `statusCode:201` + set `X-Cache-Invalidate`; C++ `Status::OK` (200), body = `mutations.create(...).body()` | Ghost comments service (`commentOnPost`/`replyToComment` → Comments model add) with internal context + validated `created_at`; C++ `mutations.create("comments", body)` raw | Ghost `setCacheInvalidationHeaders` sets `X-Cache-Invalidate` = member comment paths (`comments-controller.js:211-225`) + `cacheInvalidate:false` header; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | PARTIAL | G:`admin/routes.js:57`, `comments.js:4-30,93-148`, `comments-controller.js:299-324,211-225`; C++:`routes.cppm:29-36`, `create/handler.cppm:17-28`, `create/.../execute.cppm:11,16` |
| PUT `/comments/:id` | `edit` → `adminEdit` | PUT `/comments/:id` | `comments_edit` | Ghost `id` required (option), data `status`/`pinned`; C++: `Input=IRequest&`, path `id` **not extracted** — passes hardcoded `std::string{}` (empty id) to `mutations.edit` | Ghost returns `moderateComment` result; C++ returns `mutations.edit(...).body()`; status 200 both | Ghost `service.moderateComment(id, updates, options)`; C++ `mutations.edit("comments", "", body)` — empty id | Ghost `setCacheInvalidationHeaders`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | PARTIAL | G:`admin/routes.js:58`, `comments.js:36-52`, `comments-controller.js:189-203`; C++:`routes.cppm:47-54`, `edit/handler.cppm:17-28`, `edit/.../execute.cppm:11,16` |
| — (no `api.comments.*` route) | — | GET `/comments/:id` | `comments_read_by_id` | C++ `Input=IRequest&`; `find_by_resource("comments", "1")` | `{"comments":[...]}` or `{"comments":[]}` | `ctx.store().find_by_resource("comments", "1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:51` binds `api.commentReplies.read`, not this controller; `comments.js:33-149` has no read op. C++:`routes.cppm:38-46`, `read_by_id/.../execute.cppm:16-18` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/replies` | `comments_comment_replies` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:52` binds `api.commentReplies.browse`; C++:`routes.cppm:77-87`, `comment_replies/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/reports` | `comments_comment_reports` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:53` binds `api.commentReports.browse`; C++:`routes.cppm:88-98`, `comment_reports/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/likes` | `comments_comment_likes` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:54` binds `api.commentLikes.browse`; C++:`routes.cppm:66-76`, `comment_likes/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/dislikes` | `comments_comment_dislikes` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:55` binds `api.commentDislikes.browse`; C++:`routes.cppm:55-65`, `comment_dislikes/.../execute.cppm:26-30` |

## Route details

### GET `/comments` — PARTIAL

- Route identity matches: Ghost `admin/routes.js:50` vs C++ `routes.cppm:19-28`.
- Input: Ghost `browseAll` options `page,limit,filter,order,include_nested` (`comments.js:84`) and `adminBrowseAll` parses `include_nested`, `filter`, `order`, `page`, `limit` (`comments-controller.js:168-187`); C++ step declares `using Input=interfaces::io::IRequest&` and calls `json_for("comments", {})` with no query parsing (`browse/.../execute.cppm:11,16`). All Ghost option names absent on C++.
- Output: C++ hardcodes `meta.pagination` = limit 15 / page 1 / pages 1 / prev null / next null / total 0 and uses envelope key `comments` (`browse/.../execute.cppm:17`); Ghost returns real pagination via the API-framework `{data,meta}` envelope.
- DB call: resource string `"comments"` agrees, but Ghost applies report-count filter, `includeNested`, `order`, `page`, `limit` (`comments-controller.js:178-186`); C++ does no filtering/pagination.
- Cache: Ghost `headers.cacheInvalidate=false` (`comments.js:81-83`); C++ none. No difference in effect.
- Auth: both require a token (Ghost `mw.authAdminApi` at `admin/routes.js:50` + `permissions.method:'browse'` at `comments.js:86-88`; C++ `token(req).empty()` → 401 at `browse/handler.cppm:18-22`, plus `content_type_middleware` at `routes.cppm:20`).

### GET `/comments/post/:post_id` — PARTIAL

- Route identity matches after normalizing `:post_id` ↔ `:id`: Ghost `admin/routes.js:56` vs C++ `routes.cppm:100-112` (`/post` router + `/:id` route).
- Input: Ghost requires `post_id` (`comments.js:70-72`) and consumes `include,page,limit,fields,filter,order,debug,impersonate_member_uuid` (`comments.js:57-67`); C++ ignores all of them and does not read `post_id` (`get_detail/.../execute.cppm:11,16`).
- Output: same hardcoded `meta.pagination` and `comments` envelope as browse (`get_detail/.../execute.cppm:17`) vs Ghost real paginated `{data,meta}`.
- DB call: Ghost applies a post filter and impersonation context (`comments-controller.js:143-156`); C++ `json_for("comments", {})` returns all comments with no post filter.
- Cache: Ghost `cacheInvalidate:false` (`comments.js:54-56`); C++ none.
- Auth: both present (Ghost `admin/routes.js:56` + `comments.js:75`; C++ `get_detail/handler.cppm:18-22`).

### POST `/comments` — PARTIAL

- Route identity matches: Ghost `admin/routes.js:57` vs C++ `routes.cppm:29-36`.
- Input: Ghost validates `include` allowed values, requires `member_id` + `html` (`comments.js:98-113`), requires `post_id` or `parent_id` (`comments.js:4-11,118`), validates `created_at` and sets `context.internal=true` (`comments.js:13-30,119-125`); C++ forwards raw `body_as_string()` (default `"{}"`) with no validation and no option parsing (`create/.../execute.cppm:11,16`).
- Output: **status divergence** — Ghost `statusCode:201` (`comments.js:94`) vs C++ `Status::OK` (200) at `create/handler.cppm:27`. Ghost also sets `X-Cache-Invalidate`; C++ sets no header.
- DB call: resource `"comments"` agrees; Ghost calls `commentOnPost`/replyToComment with validated `created_at` and internal context (`comments.js:127-142`), C++ calls `mutations.create("comments", body)` raw.
- Cache: Ghost `setCacheInvalidationHeaders` emits `X-Cache-Invalidate` for member comment paths (`comments-controller.js:211-225`, called at `comments.js:144`); C++ has no cache behavior. `core::cache::Cache` is imported in `all.cppm:6` but no comments handler uses it; no comments key/namespace/TTL exists.
- Auth: both present (Ghost `admin/routes.js:57` + `comments.js:114`; C++ `create/handler.cppm:18-22` + `routes.cppm:20`).

### PUT `/comments/:id` — PARTIAL

- Route identity matches: Ghost `admin/routes.js:58` vs C++ `routes.cppm:47-54`.
- Input: Ghost requires option `id` (`comments.js:40-47`) and reads moderation fields `status`/`pinned` (`comments-controller.js:190-197`); C++ never extracts the path `id` and passes a hardcoded empty string to `mutations.edit("comments", std::string{}, body)` (`edit/.../execute.cppm:16`) — the edit cannot target the requested comment by id.
- Output: both 200; C++ returns `mutations.edit(...).body()` (`edit/.../execute.cppm:16`, `edit/handler.cppm:25-27`) vs Ghost `moderateComment` result (`comments-controller.js:199-203`).
- DB call: resource `"comments"` agrees, but Ghost passes the real id and only `status`/`pinned` updates; C++ passes empty id and the whole raw body.
- Cache: Ghost `setCacheInvalidationHeaders` (`comments-controller.js:201`); C++ none.
- Auth: both present (Ghost `admin/routes.js:58` + `comments.js:48`; C++ `edit/handler.cppm:18-22`).

## Missing in C++

None. All four routes referencing the exported keys of `comments.js` have C++ counterparts:

- GET `/comments` (`admin/routes.js:50`) → `comments_browse` (`routes.cppm:21-28`)
- GET `/comments/post/:post_id` (`admin/routes.js:56`) → `comments_get_detail` (`routes.cppm:100-112`)
- POST `/comments` (`admin/routes.js:57`) → `comments_create` (`routes.cppm:29-36`)
- PUT `/comments/:id` (`admin/routes.js:58`) → `comments_edit` (`routes.cppm:47-54`)

Absence confirmed by reading `comments.js:33-149` (only `edit`, `browse`, `browseAll`, `add` exported) and grepping `api\.comments\.` over `docs/Ghost/ghost/core/server/web` (only the four matches above).

## Extra in C++

C++ routes in `include/blog/handlers/discovery/comments/routes.cppm` with no route in the `comments` controller (the Ghost paths exist in `admin/routes.js` but are served by other controllers):

- GET `/comments/:id` → `comments_read_by_id` (`routes.cppm:38-46`). Ghost `admin/routes.js:51` uses `api.commentReplies.read`; `comments.js` has no read op.
- GET `/comments/:id/replies` → `comments_comment_replies` (`routes.cppm:77-87`). Ghost `admin/routes.js:52` uses `api.commentReplies.browse`.
- GET `/comments/:id/reports` → `comments_comment_reports` (`routes.cppm:88-98`). Ghost `admin/routes.js:53` uses `api.commentReports.browse`.
- GET `/comments/:id/likes` → `comments_comment_likes` (`routes.cppm:66-76`). Ghost `admin/routes.js:54` uses `api.commentLikes.browse`.
- GET `/comments/:id/dislikes` → `comments_comment_dislikes` (`routes.cppm:55-65`). Ghost `admin/routes.js:55` uses `api.commentDislikes.browse`.

Absence in this controller confirmed by `comments.js:33-149` (no such exported operations) and the full `api\.comments\.` grep over `docs/Ghost/ghost/core/server/web` (only four matches, listed above).
