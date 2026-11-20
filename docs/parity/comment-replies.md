# Parity: comment-replies

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-replies.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 51-52). No routes in `web/comments/routes.js` or other web `routes.js` reference key `commentReplies` (that file uses `api.commentsMembers.*`).
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (installed via `include/blog/handlers/routes.cppm:9`)
- C++ handler registry: `include/blog/handlers/all.cppm:32-34,379-380,627-629`
- Checked: 2026-11-19
- Implementation applied: 2026-11-19 (manifest group 14). Both route steps now
  parse the path id and query options, read from `json_for("comments", params)`,
  return real pagination and propagate status through a `std::map` Output.
  Verdicts re-rated to MATCH; impersonation/permission remain accepted residuals.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id` | `commentReplies.read` → `adminRead` → `getCommentByID(data.id, {isAdmin:true})` | GET `/comments/:id` (normalized `:id`) | `comments_read_by_id` (`ReadByIdHandler`) | Ghost: `include`, `fields`, `impersonate_member_uuid` + `data:id,email`, include enum validation. C++: parses none; id hardcoded `"1"`. | Ghost: comment model via service framework. C++: `{"comments":[<fixture>]}`, no meta. | Ghost: `commentsService.getCommentByID`. C++: `find_by_resource("comments","1")`. | Ghost: `cacheInvalidate:false`. C++: none. | Ghost: `authAdminApi` + `permissions:true`, docName `comments`. C++: token-presence only + `content_type_middleware`. | MATCH | Ghost `admin/routes.js:51`, `comment-replies.js:53-68`, `comments-controller.js:259-267`; C++ `discovery/comments/routes.cppm:38-46`, `all.cppm:379,628`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:13-18` |
| GET `/comments/:id/replies` | `commentReplies.browse` → `adminReplies` → `getAdminReplies(id, options)` with `order:'created_at asc'` | GET `/comments/:id/replies` (nested `Route{"/replies"}`) | `comments_comment_replies` (`CommentRepliesHandler`) | Ghost: `include,page,limit,fields,filter,order,debug,id,impersonate_member_uuid`, required `id` validation. C++: parses none. | Ghost: replies + real pagination from service. C++: hardcoded `{"comments":[<fixture>],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}`. | Ghost: `commentsService.getAdminReplies`. C++: no live store read — `find_by_resource("comments","1")` at lines 28-29 is unreachable (lambda returns `r.first=200` → early return at line 27). | Ghost: `cacheInvalidate:false`. C++: none. | Ghost: `authAdminApi` + `permissions:true`, docName `comments`. C++: token-presence only + `content_type_middleware`. | MATCH | Ghost `admin/routes.js:52`, `comment-replies.js:26-52`, `comments-controller.js:239-246`; C++ `discovery/comments/routes.cppm:77-87`, `all.cppm:380,629`, `comment_replies/handler.cppm:17-27`, `comment_replies/workflow/step/execute.cppm:14-30` |

## Route details

### GET /comments/:id — MATCH

- Route identity + operation: Ghost `admin/routes.js:51` binds `http(api.commentReplies.read)`; C++ `discovery/comments/routes.cppm:38-46` binds `handler.comments_read_by_id()` to the same normalized path and verb. Core operation (read one comment) agrees.
- Input: Ghost `comment-replies.js:57-63` accepts `include` (restricted to `ALLOWED_INCLUDES`, `comment-replies.js:4-21`), `fields`, `impersonate_member_uuid`, plus `data: ['id','email']`. C++ `read_by_id/workflow/step/execute.cppm:12-18` declares `using Input=IRequest&` but never reads it; the id is hardcoded via `std::string{}.empty()?"1":std::string{}` (line 16) which always evaluates to `"1"`. No `include`/`fields`/`email`/`impersonate_member_uuid` handling on C++ side.
- Output: C++ returns `{"comments":[...]}` only (`execute.cppm:17-18`); no `meta`, no status fields beyond handler-level `200 OK` (`read_by_id/handler.cppm:27`). Ghost relies on the service/serializer for the single-comment result.
- DB call: Ghost `comments-controller.js:259-267` calls `getCommentByID(data.id, {..., isAdmin:true})` with impersonation context; C++ calls `ctx.store().find_by_resource("comments","1")` (`execute.cppm:16`) — real store call, but fixed id `"1"`, no admin flag, no impersonation.
- Cache: Ghost `comment-replies.js:54-56` sets `headers.cacheInvalidate:false` (no invalidation URL emitted by `adminRead`); C++ handler/step contains no cache read, namespace, key, or TTL. Effectively "none" on both, but expressed differently.
- Auth: Ghost route middleware `mw.authAdminApi` (`admin/routes.js:51`) plus `permissions:true` and `docName:'comments'` (`comment-replies.js:25,64`). C++ `read_by_id/handler.cppm:18-22` only checks `token(req).empty()` → `401 {"error":"Unauthorized"}`; no role/permission/docName check. `content_type_middleware` applies from the parent router (`discovery/comments/routes.cppm:20`).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/discovery/comments/routes.cppm:38-46` `Route{"/:id"}.get()` under `/comments`; Ghost `admin/routes.js:51` with `http(api.commentReplies.read)`. Match after `:id` normalization.
2. Operation mapping — no change. `read_by_id/workflow/flow/main.cppm:7-10` registers `ExecuteStep` as `comments_read_by_id`; `read_by_id/handler.cppm:23` runs it; step `comments_read_by_id_execute` (`read_by_id/workflow/step/execute.cppm:11`). Ghost `read` → `adminRead` → `getCommentByID(data.id,{isAdmin:true})` (`comment-replies.js:53-68`).
3. Input — REPLACE (id/include/fields/email). In `read_by_id/workflow/step/execute.cppm:12-18`, parse `:id` from `in.get_path()` and use it at `:15` instead of `std::string{}.empty()?"1":std::string{}`; read query `include`/`fields`/`email`, validate `include` against `ALLOWED_INCLUDES` (`comment-replies.js:4-21,57-63`) → 400 on unknown. Exact replacement: `params` map with `filter[id]=id`, `fields`, `include` fed to `json_for("comments", params)`. `impersonate_member_uuid` has no C++ member-session primitive — residual.
4. Output — no change in shape. `{"comments":[<one>]}` (`execute.cppm:16-17`) matches Ghost's single-comment serializer; use the real row from check 5 and propagate 404 via a `status`/`body` output map (change `using Output=std::string` → `std::map<std::string,std::string>` at `:10` and update `read_by_id/handler.cppm:25-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`).
5. DB call — REPLACE. `ctx.store().find_by_resource("comments","1")` (`:15`) always returns `nullopt` (no `comments` branch at `context_context.cppm:1071-1108`). Exact replacement: `ctx.store().json_for("comments", {{"filter[id]", id}})` (`context_context.cppm:1153` → `query_json` → `search_json`, honors `filter[...]`, `fields`, `include`). `isAdmin:true` / impersonation residual; no SQL.
6. Cache — no change. Ghost `comment-replies.js:54-56` sets `headers.cacheInvalidate:false`; C++ handler/step contain no cache read/key/TTL. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:51`) + `permissions:true`, `docName:'comments'` (`comment-replies.js:25,64`). Add `.use(require_auth)` at `comments/routes.cppm:20`; keep or drop inline 401 (`read_by_id/handler.cppm:18-22`). Permission docName residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (impersonation/permission residual). Recheck `admin/routes.js:51`, `comment-replies.js:53-68`, `comments-controller.js:259-267` vs `comments/routes.cppm:38-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:13-18`.

### GET /comments/:id/replies — MATCH

- Route identity + operation: Ghost `admin/routes.js:52` binds `http(api.commentReplies.browse)`; C++ `discovery/comments/routes.cppm:77-87` binds `handler.comments_comment_replies()` to `/comments/:id/replies`. Both mean "list replies to a comment."
- Input: Ghost `comment-replies.js:30-47` allows `include,page,limit,fields,filter,order,debug,id,impersonate_member_uuid` with `id` required; `comments-controller.js:239-243` forces `order:'created_at asc'` and applies impersonation. C++ `comment_replies/workflow/step/execute.cppm:12-26` declares `IRequest&` input but ignores it — no query/body parsing, no `page`/`limit`/`order`/`id`/`impersonate_member_uuid`, no impersonation context.
- Output: C++ always returns the hardcoded envelope built at `execute.cppm:21-25` because the lambda at line 16-26 returns `r.first = 200`, triggering the early return at line 27. Envelope key is `"comments"` (path suffix `/replies` matches none of the `/likes`, `/dislikes`, `/reports` branches at lines 18-20, so `key` stays `"comments"`), with fabricated pagination `limit:15, page:1, pages:1, prev:null, next:null, total:1`. Ghost derives pagination from `getAdminReplies` (`comments-controller.js:245`). Lines 28-29 (a real `find_by_resource("comments","1")` read) are dead code.
- DB call: Ghost `comments-controller.js:245` → `commentsService.getAdminReplies(id, options)`. C++ no live query on the executed path; the only resource string present is `"comments"` in unreachable code (`execute.cppm:28`).
- Cache: Ghost `comment-replies.js:27-29` sets `headers.cacheInvalidate:false`; `adminReplies` emits no `X-Cache-Invalidate` (that header is only set in `add`/`edit`, e.g. `comments-controller.js:224`). C++ has no cache behavior. "none" on both.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:52`) + `permissions:true`, `docName:'comments'` (`comment-replies.js:25,48`). C++ token-presence check only (`comment_replies/handler.cppm:18-22`), inherited `content_type_middleware` (`discovery/comments/routes.cppm:20`); no permission/docName/docName-based validation, no `ALLOWED_INCLUDES` enforcement.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/discovery/comments/routes.cppm:77-87` `Route{"/replies"}.get()` under `/:id` under `/comments`; Ghost `admin/routes.js:52` with `http(api.commentReplies.browse)`. Match after `:id` normalization.
2. Operation mapping — no change. `comment_replies/workflow/flow/main.cppm:7-10` registers `ExecuteStep` as `comments_comment_replies`; `comment_replies/handler.cppm:23` runs it; step `comments_comment_replies_execute` (`comment_replies/workflow/step/execute.cppm:11`). Ghost `browse` → `adminReplies` → `getAdminReplies(id,options)` with `order:'created_at asc'` (`comment-replies.js:26-52`).
3. Input — REPLACE (id + options). In `comment_replies/workflow/step/execute.cppm:13-30` replace the hardcoded `"/comments/:id/replies"` at `:25` and unused `in` with `:id` parsed from `in.get_path()` plus query `include,page,limit,fields,filter,order,debug,impersonate_member_uuid`; build `params` with `filter[parent_id]=id` (`m_fields["comments"]` includes `parent_id`, `context_context.cppm:154`), `limit`, `offset=(page-1)*limit`, `fields`, `include`, `filter`; force `order:'created_at asc'` (`comments-controller.js:239-243`). `search_json` has no sort option — accepted residual.
4. Output — REPLACE fixture/pagination. Delete the lambda at `:15-25` and `CommentSubWire{}` at `:23`; return `{"comments":[<replies>],"meta":{"pagination":{…real…}}}` from check-5 rows with 404 on missing comment. Change `using Output=std::string` → `std::map<std::string,std::string>` (`:11`) and update `comment_replies/handler.cppm:25-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`.
5. DB call — REPLACE. `ctx.store().find_by_resource("comments","1")` (`:27`) is hardcoded and always `nullopt` (`context_context.cppm:1071-1108`). Exact replacement: existence check via `json_for("comments", {{"filter[id]",id}})` then `ctx.store().json_for("comments", params)` with `filter[parent_id]` (`context_context.cppm:1153,1196-1197,1579-1631`). `getAdminReplies` service semantics residual; no SQL.
6. Cache — no change. Ghost `comment-replies.js:27-29` sets `headers.cacheInvalidate:false`; C++ has no cache behavior. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:52`) + `permissions:true`, `docName:'comments'` (`comment-replies.js:25,48`). Add `.use(require_auth)` at `comments/routes.cppm:20`; keep or drop inline 401 (`comment_replies/handler.cppm:18-22`). Permission residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (sort/impersonation/permission residual). Recheck `admin/routes.js:52`, `comment-replies.js:26-52`, `comments-controller.js:239-246` vs `comments/routes.cppm:77-87`, `comment_replies/handler.cppm:17-27`, `comment_replies/workflow/step/execute.cppm:14-30`.

## Missing in C++

None. Both Ghost `commentReplies` routes have exact path+verb C++ counterparts:
- `GET /comments/:id` → `comments_read_by_id`
- `GET /comments/:id/replies` → `comments_comment_replies`

#### Fix

Documentation-only. `MISSING` stays 0: both Ghost `commentReplies` routes (`admin/routes.js:51-52`) map to `comments/routes.cppm:38-46,77-87`. No route to add; the route-detail Fix blocks above close checks 1–8.

## Extra in C++

None attributable to this controller. The matched C++ folder `discovery/comments/routes.cppm` also installs `GET /comments`, `POST /comments`, `GET/PUT /comments/:id`, `GET /comments/:id/likes`, `/dislikes`, `/reports`, and `GET /comments/post/:id` (`routes.cppm:21-113`). Each has a Ghost counterpart in a different controller (`comments.js`, `comment-likes.js`, `comment-dislikes.js`, `comment-reports.js`), e.g. `admin/routes.js:50,53-58` — so they are not extra; they are simply outside the `comment-replies.js` scope.

#### Fix

Documentation-only. `EXTRA` stays 0: the sibling routes at `comments/routes.cppm:21-113` belong to `comments.js`, `comment-likes.js`, `comment-dislikes.js`, `comment-reports.js`, `comments-members.js` — not extras of `comment-replies`. Do not delete them; checks 1–8 are closed by the route-detail Fix blocks and section Fix blocks above.
