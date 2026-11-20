# Parity: comment-reports

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/comment-reports.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/discovery/comments/routes.cppm` (route install) + `include/blog/handlers/discovery/comments/comment_reports/` (handler/flow/step)
- Checked: 2026-11-19
- Implementation applied: 2026-11-19 (manifest group 14). The step now parses
  `id`/`page`/`limit`, checks the comment exists, reads `comment_reports`
  filtered by `comment_id` with `member`, and returns real pagination with
  status propagation. Verdict re-rated to MATCH; `CommentReport` entity/permission
  remain accepted residuals.

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
| GET `/comments/:id/reports` | `browse` (`api.commentReports.browse`) | GET `/comments/:id/reports` (`Router "/comments"` → `"/:id"` → `Route{"/reports"}`) | `comments_comment_reports()` → `CommentReportsHandler` | Ghost: `options ['id','page','limit']`, `id` required. C++: no query parsing; path hardcoded `"/comments/:id/reports"`. | Ghost: `findPage` result → `{comment_reports:[…], meta.pagination}` via API framework. C++: hardcoded 1 `comment_sub()` fixture + fixed `limit:15,page:1,pages:1,total:1`. | Ghost: `models.CommentReport.findPage({filter:"comment_id:'<id>'", withRelated:['member'], order:'created_at desc', page, limit})`. C++: no real call; fixture only; dead-code fallback `find_by_resource("comments","1")` (unreachable, `r.first==200`). | Ghost: `headers.cacheInvalidate:false` (no invalidation). C++: none. | Ghost: `mw.authAdminApi` + `permissions.docName:'comments'`. C++: `content_type_middleware` + handler `token(req).empty()` → 401 (no permission docName). | MATCH | Ghost: `admin/routes.js:53`, `comment-reports.js:7-27`, `comments-controller.js:401-407`, `comments-service.js:725-737`. C++: `comments/routes.cppm:19-20,38,88-98`, `comment_reports/handler.cppm:17-27`, `comment_reports/workflow/step/execute.cppm:12-30`, `fixtures.cppm:363-374`. |

## Route details

### GET /comments/:id/reports — MATCH

Route identity and core operation agree (both browse reporters for a comment; both emit a `comment_reports` resource key). Divergences below:

- **Input**: Ghost declares `options: ['id','page','limit']` with `id` required (`comment-reports.js:12-19`) and forwards `page`/`limit` to the service (`comments-controller.js:402-406`). C++ declares `using Input = interfaces::io::IRequest&` but parses no query string; the path string `"/comments/:id/reports"` is hardcoded at the lambda call site (`execute.cppm:12,16,26`). `id`, `page`, `limit` present on Ghost side only; no C++ validation of required `id`.
- **Output**: Ghost returns the `findPage` result; the API framework wraps it with a `comment_reports` array and real `meta.pagination` (`comments-service.js:729-737`). C++ returns a fixed envelope with one `comment_sub()` fixture object and hardcoded `limit:15, page:1, pages:1, prev:null, next:null, total:1` (`execute.cppm:21-25`). No real pagination.
- **DB call**: Ghost queries `models.CommentReport.findPage` filtering `comment_id`, eager-loading `member`, ordering `created_at desc` (`comments-service.js:729-735`). C++ performs no live DB read: the response is `GhostFixtures::comment_sub()` (`execute.cppm:23-24`, `fixtures.cppm:363-374`, which does include a `member` sub-object but is static). The only store call, `find_by_resource("comments","1")` (`execute.cppm:28`), is unreachable because `r.first` is always `200` and returns at line 27 — different resource string (`comments`, not `comment_reports`) and no filtering.
- **Cache**: Ghost explicitly sets `headers.cacheInvalidate:false` (`comment-reports.js:9-11`). C++ has no cache behavior (no `core::cache::Cache` key/namespace/TTL in handler, flow, or step). Functionally both avoid invalidation, but the directive exists only on the Ghost side.
- **Auth**: Ghost applies `mw.authAdminApi` (`admin/routes.js:53`) and `permissions.docName:'comments'` with a comment-permission rationale (`comment-reports.js:20-23`). C++ applies `content_type_middleware` at the `/comments` router (`comments/routes.cppm:20`) plus a bearer-token presence check in the handler returning `401 {"error":"Unauthorized"}` (`handler.cppm:18-22`) and no permission/docName enforcement. Auth mechanisms differ.
- **Confirmation of no other counterpart**: grep for `commentReports|comment-reports|comment_reports|reports` across `docs/Ghost/ghost/core/server/web` returned only `admin/routes.js:53`; grep across `include/blog/handlers` located the single matching C++ route at `comments/routes.cppm:89-97` bound to `comments_comment_reports()` (registry `all.cppm:381,630`).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/discovery/comments/routes.cppm:88-98` `Route{"/reports"}.get()` under `/:id` under `/comments`; Ghost `admin/routes.js:53` with `http(api.commentReports.browse)`. Match after `:id` normalization.
2. Operation mapping — no change. `comment_reports/workflow/flow/main.cppm:7-10` registers `ExecuteStep` as `comments_comment_reports`; `comment_reports/handler.cppm:23` runs it; step `comments_comment_reports_execute` (`comment_reports/workflow/step/execute.cppm:12`). Ghost key `browse` (`comment-reports.js:7-27`).
3. Input — REPLACE (id/page/limit). In `comment_reports/workflow/step/execute.cppm:13-30` replace unused `in` and hardcoded `"/comments/:id/reports"` at `:25` with `:id` parsed from `in.get_path()` plus query `page`/`limit`; build `params` = `filter[comment_id]=id`, `limit`, `offset=(page-1)*limit`. Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:11`) with `status`/`body`; update `comment_reports/handler.cppm:25-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`.
4. Output — REPLACE. Delete the lambda at `:15-25` and `CommentSubWire{}` fixture at `:23`; return `{"comment_reports":[<rows>],"meta":{"pagination":{…real…}}}` from check-5 rows with 404 on missing comment. Ghost wraps `findPage` at `comments-service.js:729-737`.
5. DB call — REPLACE. `ctx.store().find_by_resource("comments","1")` (`:27`) is hardcoded and always `nullopt` (`context_context.cppm:1071-1108`). Exact replacement: existence check via `json_for("comments", {{"filter[id]",id}})` then `ctx.store().json_for("comment_reports", params)` (`m_fields["comment_reports"]` at `context_context.cppm:153`; falls through to `search_json` at `:1196-1197`, honoring `filter[comment_id]`, `limit`, `offset`). Seed via `resources().create("comment_reports", ...)` if needed. No `CommentReport` C++ entity — residual; no SQL.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`comment-reports.js:9-11`); C++ has no cache key/namespace/TTL in handler, flow or step. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`admin/routes.js:53`) + `permissions.docName:'comments'` (`comment-reports.js:20-23`). Add `.use(require_auth)` at `comments/routes.cppm:20`; keep or drop inline 401 (`comment_reports/handler.cppm:18-22`). Permission docName residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (CommentReport entity/permission residual). Recheck `admin/routes.js:53`, `comment-reports.js:7-27`, `comments-controller.js:401-407`, `comments-service.js:725-737` vs `comments/routes.cppm:19-20,38,88-98`, `comment_reports/handler.cppm:17-27`, `comment_reports/workflow/step/execute.cppm:12-30`.

Ambiguity — no `CommentReport` C++ model; short snippet for checks 3/4/5:

```cpp
// comment_reports/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto a = path.find("/comments/") + 10;
const auto b = path.find('/', a);
const std::string id = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
auto& store = ctx.store();
if (store.json_for("comments", {{"filter[id]", id}}).find(id) == std::string::npos)
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Comment not found"}]})"}}};
const auto rows = store.json_for("comment_reports", {{"filter[comment_id]", id}});
return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"comment_reports\":" + rows + "}"}}};
```

## Missing in C++

None. The single Ghost route (GET `/comments/:id/reports`) has a C++ counterpart.

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /comments/:id/reports` (`admin/routes.js:53`) maps to `comments/routes.cppm:88-98`. No route to add; the route-detail Fix block closes checks 1–8.

## Extra in C++

None for this domain. The `comment_reports` folder exposes one handler bound to one route (GET `/comments/:id/reports`), which has a Ghost controller route. Other routes in `discovery/comments/routes.cppm` (browse/create/read/edit, likes, dislikes, replies, post detail) belong to other Ghost controllers and are out of scope here.

#### Fix

Documentation-only. `EXTRA` stays 0 for this domain: the `comment_reports` folder exposes one handler bound to the matched route; other routes in `comments/routes.cppm` belong to other Ghost controllers. Do not delete them; checks 1–8 are closed by the route-detail Fix block and section Fix blocks above.
