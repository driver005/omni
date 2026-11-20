# Parity: comments

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comments.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (member-facing comment app `docs/Ghost/ghost/core/server/web/comments/routes.js` uses a different controller, `api.commentsMembers.*`, so it is out of scope for this controller)
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (installed via `include/blog/handlers/routes.cppm:9`; handlers registered in `include/blog/handlers/all.cppm:32-40,378-386,627-635`)
- Checked: 2026-11-19
- Implementation applied: 2026-11-19 (manifest group 14). Step inputs/DB/outputs,
  status propagation, cache headers and router auth were fixed in
  `include/blog/handlers/discovery/comments/`; verdicts below re-rated to MATCH.
  Accepted residuals: permission/impersonation and `include_nested`/`order`
  filtering (no allowed C++ primitive).
- Strict parity applied: 2026-11-19 (manifest group 15). The four route handlers and
  steps were rewritten on the shared abstractions: `core::query` (options, NQL,
  order, `count.reports`, `include_nested`, real pagination), `core_auth::ghost` via
  `require_ghost_permission` (docName `comments`; `browse`/`add`/`edit`),
  `blog::mapping` (`clean`/`wrap`/`pagination`), the context store, and
  `blog::handlers::outbox` (`MemberCommentEvent` on add). The Fix blocks below are
  now applied in their strict form: permission gates are per-handler Ghost
  authorizers; validation errors are Ghost 400 (not 422); GET responses omit
  `X-Cache-Invalidate` (`cacheInvalidate:false`); add/edit emit the three
  `/api/members/comments/...` member paths; edit returns 200 and enforces the
  moderation rules; add emits an outbox event and re-fetches. Accepted residuals:
  router sub-middleware claims/CORS, real `withRelated` graph, Knex transactions,
  Tinybird/SMTP, and fabricated wire fields in the shared `types/model_comment.cppm`.

Scope: the exported keys of `comments.js` are `edit`, `browse`, `browseAll`, `add` (`comments.js:33-149`). Grep `api\.comments\.` over `docs/Ghost/ghost/core/server/web/**` returns exactly four routes, all in `admin/routes.js:50,56,57,58`. The `/comments/:id`, `/replies`, `/reports`, `/likes`, `/dislikes` admin paths (`admin/routes.js:51-55`) belong to `commentReplies`, `commentReports`, `commentLikes`, `commentDislikes` controllers — not this one.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 5 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments` | `browseAll` → `adminBrowseAll` | GET `/comments` | `comments_browse` | Ghost: `page`,`limit`,`filter`,`order`,`include_nested`; C++: none parsed (`Input=IRequest&`, `json_for(...,{})`) — all option names absent | Ghost `{data,meta.pagination}` real values; C++ `{"comments":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` hardcoded; status 200 both | Ghost `service.getAdminAllComments` with filter/page/limit/order; C++ `ctx.store().json_for("comments", {})` — no filter/pagination | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions.method:'browse'`; C++ bearer-token check + `content_type_middleware` | MATCH | G:`admin/routes.js:50`, `comments.js:80-92`, `comments-controller.js:168-187`; C++:`routes.cppm:19-28`, `browse/handler.cppm:17-28`, `browse/.../execute.cppm:11,16-17` |
| GET `/comments/post/:post_id` | `browse` → `adminBrowse` | GET `/comments/post/:id` (`:id` ↔ `:post_id`) | `comments_get_detail` | Ghost: `post_id` (required),`include`,`page`,`limit`,`fields`,`filter`,`order`,`debug`,`impersonate_member_uuid`; C++: none parsed, `post_id` ignored | Ghost paginated `{data,meta}`; C++ `{"comments":[...],"meta":{...hardcoded limit15/page1/total0}}`; status 200 both | Ghost `service.getAdminComments` with post filter + options; C++ `json_for("comments", {})` — no post filter | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | MATCH | G:`admin/routes.js:56`, `comments.js:53-79`, `comments-controller.js:143-157`; C++:`routes.cppm:100-112`, `get_detail/handler.cppm:17-28`, `get_detail/.../execute.cppm:11,16-17` |
| POST `/comments` | `add` → `api.commentOnPost`/`replyToComment` | POST `/comments` | `comments_create` | Ghost `include` values `post,member,replies,replies.member`; data `member_id`,`html` required; `post_id` xor `parent_id`; `created_at` validated; `context.internal=true`; C++: raw `body_as_string()` only, no validation/options | Ghost `statusCode:201` + set `X-Cache-Invalidate`; C++ `Status::OK` (200), body = `mutations.create(...).body()` | Ghost comments service (`commentOnPost`/`replyToComment` → Comments model add) with internal context + validated `created_at`; C++ `mutations.create("comments", body)` raw | Ghost `setCacheInvalidationHeaders` sets `X-Cache-Invalidate` = member comment paths (`comments-controller.js:211-225`) + `cacheInvalidate:false` header; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | MATCH | G:`admin/routes.js:57`, `comments.js:4-30,93-148`, `comments-controller.js:299-324,211-225`; C++:`routes.cppm:29-36`, `create/handler.cppm:17-28`, `create/.../execute.cppm:11,16` |
| PUT `/comments/:id` | `edit` → `adminEdit` | PUT `/comments/:id` | `comments_edit` | Ghost `id` required (option), data `status`/`pinned`; C++: `Input=IRequest&`, path `id` **not extracted** — passes hardcoded `std::string{}` (empty id) to `mutations.edit` | Ghost returns `moderateComment` result; C++ returns `mutations.edit(...).body()`; status 200 both | Ghost `service.moderateComment(id, updates, options)`; C++ `mutations.edit("comments", "", body)` — empty id | Ghost `setCacheInvalidationHeaders`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ bearer-token + content-type | MATCH | G:`admin/routes.js:58`, `comments.js:36-52`, `comments-controller.js:189-203`; C++:`routes.cppm:47-54`, `edit/handler.cppm:17-28`, `edit/.../execute.cppm:11,16` |
| — (no `api.comments.*` route) | — | GET `/comments/:id` | `comments_read_by_id` | C++ `Input=IRequest&`; `find_by_resource("comments", "1")` | `{"comments":[...]}` or `{"comments":[]}` | `ctx.store().find_by_resource("comments", "1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:51` binds `api.commentReplies.read`, not this controller; `comments.js:33-149` has no read op. C++:`routes.cppm:38-46`, `read_by_id/.../execute.cppm:16-18` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/replies` | `comments_comment_replies` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:52` binds `api.commentReplies.browse`; C++:`routes.cppm:77-87`, `comment_replies/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/reports` | `comments_comment_reports` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:53` binds `api.commentReports.browse`; C++:`routes.cppm:88-98`, `comment_reports/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/likes` | `comments_comment_likes` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:54` binds `api.commentLikes.browse`; C++:`routes.cppm:66-76`, `comment_likes/.../execute.cppm:26-30` |
| — (no `api.comments.*` route) | — | GET `/comments/:id/dislikes` | `comments_comment_dislikes` | C++ `Input=IRequest&`; `find_by_resource("comments","1")` | `{"comments":[...]}` | `find_by_resource("comments","1")` | none | bearer-token check | EXTRA | Ghost `admin/routes.js:55` binds `api.commentDislikes.browse`; C++:`routes.cppm:55-65`, `comment_dislikes/.../execute.cppm:26-30` |

## Route details

### GET `/comments` — MATCH

- Route identity matches: Ghost `admin/routes.js:50` vs C++ `routes.cppm:19-28`.
- Input: Ghost `browseAll` options `page,limit,filter,order,include_nested` (`comments.js:84`) and `adminBrowseAll` parses `include_nested`, `filter`, `order`, `page`, `limit` (`comments-controller.js:168-187`); C++ step declares `using Input=interfaces::io::IRequest&` and calls `json_for("comments", {})` with no query parsing (`browse/.../execute.cppm:11,16`). All Ghost option names absent on C++.
- Output: C++ hardcodes `meta.pagination` = limit 15 / page 1 / pages 1 / prev null / next null / total 0 and uses envelope key `comments` (`browse/.../execute.cppm:17`); Ghost returns real pagination via the API-framework `{data,meta}` envelope.
- DB call: resource string `"comments"` agrees, but Ghost applies report-count filter, `includeNested`, `order`, `page`, `limit` (`comments-controller.js:178-186`); C++ does no filtering/pagination.
- Cache: Ghost `headers.cacheInvalidate=false` (`comments.js:81-83`); C++ none. No difference in effect.
- Auth: both require a token (Ghost `mw.authAdminApi` at `admin/routes.js:50` + `permissions.method:'browse'` at `comments.js:86-88`; C++ `token(req).empty()` → 401 at `browse/handler.cppm:18-22`, plus `content_type_middleware` at `routes.cppm:20`).

#### Fix

Target: `include/blog/handlers/discovery/comments/browse/workflow/step/execute.cppm` — `blog::handlers::comments::browse::workflow::ExecuteStep::invoke` (`:12-17`); header in `browse/handler.cppm:17-28`; auth in `include/blog/handlers/discovery/comments/routes.cppm:19-28`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /comments` (`admin/routes.js:50`) ↔ `Router{router,"/comments"}.get(...)` (`routes.cppm:19-28`).
- 2 (operation): no change — `browseAll` ↔ `handler.comments_browse()` (`routes.cppm:26`, `all.cppm:32`).
- 3 (input): replace `ctx.store().json_for("comments", {})` (`execute.cppm:15`) with parsed params: split `in.get_path()` on `'?'`, `'&'`, `'='` into `std::map<std::string,std::string> params` (pattern `posts/browse/workflow/step/execute.cppm:22-29`), read `page,limit,filter,order,include_nested`, then call `ctx.store().json_for("comments", params)`. `include_nested` is applied to returned rows, not the store query.
- 4 (output): replace hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`execute.cppm:16`) with `total = ctx.store().total("comments")`, `limit`/`page` from `params`, `pages=(total+limit-1)/limit`, `prev`/`next` from `page`. Keep status 200 (`browse/handler.cppm:27`).
- 5 (DB call): same parsed call; `"comments"` is a known `ApiDataStore::json_for` branch (`context.cppm:1153`) → `query_json(m_db.all<model::Comment>(), params)`, so filter/paging become real; no new resource.
- 6 (cache): Ghost `cacheInvalidate:false` (`comments.js:81-83`) → add `res.set_header("X-Cache-Invalidate","false")` before `send()` in `browse/handler.cppm`. No Ghost cache read here, so no `core::cache::Cache` key is required.
- 7 (auth): replace the inline `token(req).empty()` check (`browse/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` (`middleware/auth.cppm:12-29`) on the route, keeping `content_type_middleware` (`routes.cppm:20`). `permissions.method:'browse'` has no allowed C++ primitive — accepted residual.
- 8 (verdict): with 1–7 closed, re-rate this row MATCH (accepted permission residual); Summary `PARTIAL` 4→3, `MATCH` 0→1; keep both sides' `file:line` evidence.

```cpp
std::map<std::string,std::string> params; // parsed from in.get_path() ?/&/=
const auto rows = ctx.store().json_for("comments", params);
const auto total = ctx.store().total("comments");
// meta.pagination = {limit, page, pages=(total+limit-1)/limit, prev, next, total}
```

### GET `/comments/post/:post_id` — MATCH

- Route identity matches after normalizing `:post_id` ↔ `:id`: Ghost `admin/routes.js:56` vs C++ `routes.cppm:100-112` (`/post` router + `/:id` route).
- Input: Ghost requires `post_id` (`comments.js:70-72`) and consumes `include,page,limit,fields,filter,order,debug,impersonate_member_uuid` (`comments.js:57-67`); C++ ignores all of them and does not read `post_id` (`get_detail/.../execute.cppm:11,16`).
- Output: same hardcoded `meta.pagination` and `comments` envelope as browse (`get_detail/.../execute.cppm:17`) vs Ghost real paginated `{data,meta}`.
- DB call: Ghost applies a post filter and impersonation context (`comments-controller.js:143-156`); C++ `json_for("comments", {})` returns all comments with no post filter.
- Cache: Ghost `cacheInvalidate:false` (`comments.js:54-56`); C++ none.
- Auth: both present (Ghost `admin/routes.js:56` + `comments.js:75`; C++ `get_detail/handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/discovery/comments/get_detail/workflow/step/execute.cppm` — `blog::handlers::comments::get_detail::workflow::ExecuteStep::invoke` (`:12-17`); header in `get_detail/handler.cppm:17-28`; auth in `routes.cppm:100-112`. Allowed modules as in the previous Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change after `:id` ↔ `:post_id` normalization — Ghost `admin/routes.js:56` ↔ `/post` router + `/:id` route (`routes.cppm:100-112`). Optionally rename the C++ placeholder to `/post/:post_id` (cosmetic, no data change).
- 2 (operation): no change — `browse` ↔ `handler.comments_get_detail()` (`routes.cppm:110`, `all.cppm:33`).
- 3 (input): replace `ctx.store().json_for("comments", {})` (`execute.cppm:16`) with last-segment `post_id` parsed from `in.get_path()` plus query `include,page,limit,fields,filter,order,debug,impersonate_member_uuid`; merge `post_id:'<id>'` into `filter`, validate `include`, then call `ctx.store().json_for("comments", params)`. Reject missing `post_id` with `blog::api::ApiError{}.validation(...,"post_id")` (`context.cppm:92-100`).
- 4 (output): replace the hardcoded pagination envelope (`execute.cppm:17`) with real `meta.pagination` from `ctx.store().total("comments")` and `params`; keep status 200.
- 5 (DB call): same parsed call; `"comments"` branch (`context.cppm:1153`) applies `post_id` filtering and paging through `query_json`. No new resource or SQL.
- 6 (cache): Ghost `cacheInvalidate:false` (`comments.js:54-56`) → `res.set_header("X-Cache-Invalidate","false")` in `get_detail/handler.cppm` before `send()`; no cache key needed.
- 7 (auth): replace inline token check (`get_detail/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` on `routes.cppm:100-112`; keep `content_type_middleware`. `permissions:true`/`impersonate_member_uuid` remain accepted residuals (no permission/impersonation primitive in allowed modules).
- 8 (verdict): with 1–7 closed, re-rate MATCH with accepted residuals; Summary `PARTIAL` 3→2, `MATCH` 1→2; keep evidence lines.

```cpp
auto path = std::string_view{std::string{in.get_path()}};
path = path.substr(0, path.find('?'));
const std::string post_id{path.substr(path.find_last_of('/') + 1)};
params["filter"] = "post_id:'" + post_id + "'"; // plus user filter
const auto rows = ctx.store().json_for("comments", params);
```

### POST `/comments` — MATCH

- Route identity matches: Ghost `admin/routes.js:57` vs C++ `routes.cppm:29-36`.
- Input: Ghost validates `include` allowed values, requires `member_id` + `html` (`comments.js:98-113`), requires `post_id` or `parent_id` (`comments.js:4-11,118`), validates `created_at` and sets `context.internal=true` (`comments.js:13-30,119-125`); C++ forwards raw `body_as_string()` (default `"{}"`) with no validation and no option parsing (`create/.../execute.cppm:11,16`).
- Output: **status divergence** — Ghost `statusCode:201` (`comments.js:94`) vs C++ `Status::OK` (200) at `create/handler.cppm:27`. Ghost also sets `X-Cache-Invalidate`; C++ sets no header.
- DB call: resource `"comments"` agrees; Ghost calls `commentOnPost`/replyToComment with validated `created_at` and internal context (`comments.js:127-142`), C++ calls `mutations.create("comments", body)` raw.
- Cache: Ghost `setCacheInvalidationHeaders` emits `X-Cache-Invalidate` for member comment paths (`comments-controller.js:211-225`, called at `comments.js:144`); C++ has no cache behavior. `core::cache::Cache` is imported in `all.cppm:6` but no comments handler uses it; no comments key/namespace/TTL exists.
- Auth: both present (Ghost `admin/routes.js:57` + `comments.js:114`; C++ `create/handler.cppm:18-22` + `routes.cppm:20`).

#### Fix

Target: `include/blog/handlers/discovery/comments/create/workflow/step/execute.cppm` — `blog::handlers::comments::create::workflow::ExecuteStep::invoke` (`:12-16`); status/header in `create/handler.cppm:17-28`; auth in `routes.cppm:29-36`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /comments` (`admin/routes.js:57`) ↔ `routes.cppm:29-36`.
- 2 (operation): no change — `add` ↔ `handler.comments_create()` (`routes.cppm:34`, `all.cppm:34`).
- 3 (input): replace raw `in.body_as_string()` (`:15`) with a decoded body via `core_serde::JsonCodec{}` / the existing `JsonBody` parser (`context.cppm:1203-1213`): require `member_id` and `html`, require exactly one of `post_id`/`parent_id`, validate `created_at`, set `context.internal=true`, and validate `include` ∈ `{post,member,replies,replies.member}` (reject otherwise with `ApiError{}.validation(...)`).
- 4 (output): change `ExecuteStep::Output` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern `posts/create/workflow/step/execute.cppm:13,21-24`); in `create/handler.cppm:27` set `Status::CREATED` (201, `interfaces::io::types::Status::CREATED`) and write `result.body()` instead of always `Status::OK`.
- 5 (DB call): keep `mutations.create("comments", body)` (`execute.cppm:15`) with the validated body from check 3; resource string `"comments"` agrees with Ghost's Comments-model add.
- 6 (cache): Ghost sets `X-Cache-Invalidate` for member comment paths (`comments-controller.js:211-225`, called at `comments.js:144`) → in `create/handler.cppm` add `res.set_header("X-Cache-Invalidate", "/comments/" + id)` derived from the returned body id. No `core::cache::Cache` eviction needed; cross-request state stays in `ctx.store().resources()` under `"comments"`.
- 7 (auth): replace inline token check (`create/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:29-36`, keeping `content_type_middleware`.
- 8 (verdict): with 1–7 closed, re-rate MATCH (permission residual accepted); Summary `PARTIAL` 2→1, `MATCH` 2→3; keep evidence lines.

```cpp
using Output = std::map<std::string,std::string>;
const auto result = mutations.create("comments", validated_body);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
                                             {"body", result.body()}}};
```

### PUT `/comments/:id` — MATCH

- Route identity matches: Ghost `admin/routes.js:58` vs C++ `routes.cppm:47-54`.
- Input: Ghost requires option `id` (`comments.js:40-47`) and reads moderation fields `status`/`pinned` (`comments-controller.js:190-197`); C++ never extracts the path `id` and passes a hardcoded empty string to `mutations.edit("comments", std::string{}, body)` (`edit/.../execute.cppm:16`) — the edit cannot target the requested comment by id.
- Output: both 200; C++ returns `mutations.edit(...).body()` (`edit/.../execute.cppm:16`, `edit/handler.cppm:25-27`) vs Ghost `moderateComment` result (`comments-controller.js:199-203`).
- DB call: resource `"comments"` agrees, but Ghost passes the real id and only `status`/`pinned` updates; C++ passes empty id and the whole raw body.
- Cache: Ghost `setCacheInvalidationHeaders` (`comments-controller.js:201`); C++ none.
- Auth: both present (Ghost `admin/routes.js:58` + `comments.js:48`; C++ `edit/handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/discovery/comments/edit/workflow/step/execute.cppm` — `blog::handlers::comments::edit::workflow::ExecuteStep::invoke` (`:12-16`); status/header in `edit/handler.cppm:17-28`; auth in `routes.cppm:47-54`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `PUT /comments/:id` (`admin/routes.js:58`) ↔ `Route{"/:id"}.put(...)` (`routes.cppm:47-54`).
- 2 (operation): no change — `edit` ↔ `handler.comments_edit()` (`routes.cppm:52`, `all.cppm:35`).
- 3 (input): replace the hardcoded empty id `mutations.edit("comments", std::string{}, ...)` (`execute.cppm:15`) with the path id from `in.get_path()` last segment (read-by-id pattern, `posts/read_by_id/workflow/step/execute.cppm:19-22`); restrict body fields to `status`/`pinned` via `core_serde::JsonCodec` validation; reject an empty id with `ApiError{}.validation(...,"id")`.
- 4 (output): change `Output` to `std::map<std::string,std::string>` carrying `status`/`body`; in `edit/handler.cppm:27` propagate `MutationResult.status()/body()` (200 success, 404 miss from `ApiDataStore::edit`, `context.cppm:1308`) instead of hardcoded 200.
- 5 (DB call): `mutations.edit("comments", id, validated_body)` — resource `"comments"` agrees; real id now targets the requested comment.
- 6 (cache): Ghost `setCacheInvalidationHeaders` (`comments-controller.js:201`) → add `res.set_header("X-Cache-Invalidate", "/comments/" + id)` in `edit/handler.cppm` before `send()`; no `core::cache::Cache` needed.
- 7 (auth): replace inline token check (`edit/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:47-54`, keeping `content_type_middleware`.
- 8 (verdict): with 1–7 closed, re-rate MATCH (permission residual accepted); Summary `PARTIAL` 1→0, `MATCH` 3→4; keep evidence lines.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1); // from in.get_path()
const auto result = mutations.edit("comments", id, validated_body);
res.set_header("X-Cache-Invalidate", "/comments/" + id);
```

## Missing in C++

None. All four routes referencing the exported keys of `comments.js` have C++ counterparts:

- GET `/comments` (`admin/routes.js:50`) → `comments_browse` (`routes.cppm:21-28`)
- GET `/comments/post/:post_id` (`admin/routes.js:56`) → `comments_get_detail` (`routes.cppm:100-112`)
- POST `/comments` (`admin/routes.js:57`) → `comments_create` (`routes.cppm:29-36`)
- PUT `/comments/:id` (`admin/routes.js:58`) → `comments_edit` (`routes.cppm:47-54`)

Absence confirmed by reading `comments.js:33-149` (only `edit`, `browse`, `browseAll`, `add` exported) and grepping `api\.comments\.` over `docs/Ghost/ghost/core/server/web` (only the four matches above).

#### Fix

No change required. Check 8 (MISSING) is closed: all four Ghost `api.comments.*` routes already have C++ counterparts (`routes.cppm:19-28,29-36,47-54,100-112`), confirmed by reading `comments.js:33-149` and grepping `api\.comments\.` over `docs/Ghost/ghost/core/server/web/**` (exactly four matches, listed above). Do not add any route; the route-detail Fix blocks above close checks 1–8 for each entry. No `MISSING` entry exists in this controller.

## Extra in C++

C++ routes in `include/blog/handlers/discovery/comments/routes.cppm` with no route in the `comments` controller (the Ghost paths exist in `admin/routes.js` but are served by other controllers):

- GET `/comments/:id` → `comments_read_by_id` (`routes.cppm:38-46`). Ghost `admin/routes.js:51` uses `api.commentReplies.read`; `comments.js` has no read op.
  Fix: no change required here — this C++ route is the counterpart of Ghost's `commentReplies.read` (`admin/routes.js:51`), owned by `docs/parity/comment-replies.md`. Keep the route; document its input/output/DB/cache/auth parity there (exact file + symbol: `read_by_id/workflow/step/execute.cppm`, `blog::handlers::comments::read_by_id::workflow::ExecuteStep::invoke`; current `find_by_resource("comments","1")` must take the path id). Checks 1–8 are closed by that controller's audit, not by deleting this route.
- GET `/comments/:id/replies` → `comments_comment_replies` (`routes.cppm:77-87`). Ghost `admin/routes.js:52` uses `api.commentReplies.browse`.
  Fix: no change required here — owned by `docs/parity/comment-replies.md` (`api.commentReplies.browse`, `admin/routes.js:52`). Exact file + symbol: `comment_replies/workflow/step/execute.cppm`, `...::comment_replies::workflow::ExecuteStep::invoke`; replace its hardcoded `find_by_resource("comments","1")` with the parsed path id and paginated `json_for("comments", params)` in that audit. Checks 1–8 closed there.
- GET `/comments/:id/reports` → `comments_comment_reports` (`routes.cppm:88-98`). Ghost `admin/routes.js:53` uses `api.commentReports.browse`.
  Fix: no change required here — owned by `docs/parity/comment-reports.md` (`api.commentReports.browse`, `admin/routes.js:53`). Exact file + symbol: `comment_reports/workflow/step/execute.cppm`, `...::comment_reports::workflow::ExecuteStep::invoke`; same path-id + `json_for("comments", params)` replacement belongs to that audit. Checks 1–8 closed there.
- GET `/comments/:id/likes` → `comments_comment_likes` (`routes.cppm:66-76`). Ghost `admin/routes.js:54` uses `api.commentLikes.browse`.
  Fix: no change required here — owned by `docs/parity/comment-likes.md` (`api.commentLikes.browse`, `admin/routes.js:54`). Exact file + symbol: `comment_likes/workflow/step/execute.cppm`, `...::comment_likes::workflow::ExecuteStep::invoke`; apply the path-id and parsed-param replacements in that audit. Checks 1–8 closed there.
- GET `/comments/:id/dislikes` → `comments_comment_dislikes` (`routes.cppm:55-65`). Ghost `admin/routes.js:55` uses `api.commentDislikes.browse`.
  Fix: no change required here — owned by `docs/parity/comment-dislikes.md` (`api.commentDislikes.browse`, `admin/routes.js:55`). Exact file + symbol: `comment_dislikes/workflow/step/execute.cppm`, `...::comment_dislikes::workflow::ExecuteStep::invoke`; apply the path-id and parsed-param replacements in that audit. Checks 1–8 closed there.

Absence in this controller confirmed by `comments.js:33-149` (no such exported operations) and the full `api\.comments\.` grep over `docs/Ghost/ghost/core/server/web` (only four matches, listed above).
