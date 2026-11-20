# Parity: recommendations-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/recommendations-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/discovery/recommendations/routes.cppm` (browse handler + step)
- Checked: 2026-11-19
- Re-verified after strict-parity rewrite (manifest group 19): 2026-11-19 — the content-API browse route now MATCH.

Scope: controller exported keys are `browse`, `trackClicked`, `trackSubscribed`. Only `browse` is
registered in the content API URL table (`api.recommendationsPublic.browse`, bound to this controller at
`docs/Ghost/ghost/core/server/api/endpoints/index.js:371-373`). `trackClicked` / `trackSubscribed` are
registered in the members app (`docs/Ghost/ghost/core/server/web/members/app.js:175-186`), not in
`content/routes.js`, so they are out of scope for this content-API controller audit.

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
| GET `/recommendations` | `recommendationsPublic.browse` | `Router{router,"/recommendations"}.get` | `handler.recommendations_browse().handle` | Ghost: `options[] = limit, order, page, filter`, `validation{}`; C++: `Input=IRequest&`, no query/body parsing | Both envelope `{"recommendations":[...],"meta":{"pagination":...}}`; Ghost default `limit=5`, real `total/pages`; C++ hardcoded `limit=15,page=1,pages=1,total=0`, no per-row `count` | Ghost: `countRecommendations` + `listRecommendations({page,limit,filter,include,order})` (repository `getAll`); C++: `ctx.store().json_for("recommendations", {})` — empty params, no filtering/pagination | Ghost: `headers.cacheInvalidate:false` (no `X-Cache-Invalidate`); C++: none | Ghost: `mw.authenticatePublic` (public key) + `permissions:true`; C++: `content_type_middleware` + handler token-presence check only → 401 `{"error":"Unauthorized"}`, no permissions docName | MATCH | Ghost `content/routes.js:45`, `recommendations-public.js:7-17`; C++ `discovery/recommendations/routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-16` |

## Route details

> Verdicts below reflect the post-rewrite (manifest group 19) state; the
> **Ghost**/**C++**/**Diff** prose is the pre-implementation audit.

### GET `/recommendations` — MATCH

- **Route identity / operation (agree):** Ghost `router.get('/recommendations', mw.authenticatePublic, http(api.recommendationsPublic.browse))` at `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:45`; controller `browse` at `docs/Ghost/ghost/core/server/api/endpoints/recommendations-public.js:7-17`. C++ `.get(...)` bound to `handler.recommendations_browse().handle` at `include/blog/handlers/discovery/recommendations/routes.cppm:19-28`; registered in `include/blog/handlers/routes.cppm:37,110` and `include/blog/handlers/all.cppm:24,370,619`.
- **Input (diverges):** Ghost declares `options: ['limit','order','page','filter']` (`recommendations-public.js:11`) and the service reads `page` (default 1), `limit` (default 5), `filter`, `order`/`withRelated` (`services/recommendations/service/recommendation-controller.ts:140-156`). C++ `using Input = interfaces::io::IRequest&` with no query parsing and no body DTO (`browse/workflow/step/execute.cppm:11-13`); `json_for` called with empty params (`execute.cppm:15`). Params present on Ghost side only: `limit`, `order`, `page`, `filter` (and service-level `withRelated`). None present on C++ side only.
- **Output (diverges):** Ghost serializer returns `{data:[...], meta:{pagination:{page,limit,total,pages,prev,next}}}` with `limit=5` default and count-derived `total`/`pages` (`recommendation-controller.ts:167-169,221-260`); rows include optional `count:{clicks,subscribers}` (`recommendation-controller.ts:235-241`). C++ emits `{"recommendations":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with hardcoded values and no `count` field (`execute.cppm:16`), status 200 (`browse/handler.cppm:27`). Envelope key shape agrees; pagination values and row fields differ.
- **DB call (diverges):** Ghost `countRecommendations({})` + `listRecommendations({page,limit,filter,include,order})` (`recommendation-controller.ts:158-165`) backed by repository `getAll` (`services/recommendations/service/bookshelf-repository.ts:158`). C++ `ctx.store().json_for("recommendations", {})` (`execute.cppm:15`) — exact resource string `recommendations`, but no real filtering or pagination; returns all stored rows.
- **Cache (agrees as "none"):** Ghost sets `headers.cacheInvalidate:false` (`recommendations-public.js:8-10`), so no `X-Cache-Invalidate` emitted; no cache service call. C++ browse handler/step have no `core::cache::Cache` read/write, no namespace, no TTL, no eviction (`browse/handler.cppm:17-28`, `execute.cppm:13-17`).
- **Auth (diverges):** Ghost content route uses `mw.authenticatePublic` public-key middleware (`content/routes.js:45`) plus controller `permissions:true`, `docName:'recommendations'` (`recommendations-public.js:5,12`). C++ router installs only `content_type_middleware` (`routes.cppm:20`); the handler merely checks `token(req).empty()` and returns 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`) — no bearer/public-key validation and no permissions/docName enforcement.

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the PARTIAL on `GET /recommendations`
(Ghost `recommendations-public.js:7-17`, `content/routes.js:45`):

- **Check 1 (route identity) — closed, no change required.** `include/blog/handlers/discovery/recommendations/routes.cppm:19-28` already binds `Router{router,"/recommendations"}.get(...)` → `GET /recommendations`. Keep.
- **Check 2 (operation mapping) — closed, no change required.** `routes.cppm:26` binds `handler.recommendations_browse().handle`; Ghost key `recommendationsPublic.browse` (`recommendations-public.js:7`, `index.js:371-373`). Getter `BlogHandlers::recommendations_browse()` at `include/blog/handlers/all.cppm:370`, member at `:619`. Keep.
- **Check 3 (input parity) — fix.** File `include/blog/handlers/discovery/recommendations/browse/workflow/step/execute.cppm`, symbol `blog::handlers::recommendations::browse::workflow::ExecuteStep::invoke` (`using Input = interfaces::io::IRequest&`, line 11). Replace `json_for("recommendations", {})` (line 15) with parsed `params`: split `in.get_path()` (`include/interfaces/io/request.cppm:807`) on `'?'`, then `'&'`, then `'='`, into `std::map<std::string,std::string>` — canonical pattern `include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-30`. Exact replacement call: `ctx.store().json_for("recommendations", params)`. Parse `page` (default 1), `limit` (default 5), `filter`, `order`, `include` / `withRelated` (Ghost `options[]` at `recommendations-public.js:11`, service at `recommendation-controller.ts:140-156`). Validate `include` / `withRelated` values against `{"count.clicks","count.subscribers"}` (`RecommendationIncludesMap`, `recommendation-controller.ts:13-16`); on any other value return 422 `ApiError{}.validation(...)` (`include/blog/context/context.cppm:76-84`) instead of data. `order` field allow-list: `RecommendationOrderMap` (`recommendation-controller.ts:18-27,102-138`).
- **Check 4 (output parity) — fix.** Same step (line 16) plus `browse/handler.cppm`, symbol `blog::handlers::recommendations::browse::BrowseHandler::handle` (line 27). Change envelope key `recommendations` → `data` to match Ghost `{data:[...],meta}` (`recommendation-controller.ts:221-248`); keep `Output = std::string`. Replace hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` with values derived from `params` and `ctx.store().total("recommendations")` (`context.cppm:1111-1121`): `total`, `pages = ceil(total/limit)`, `prev = page>1?page-1:null`, `next = page<pages?page+1:null` (`recommendation-controller.ts:250-260`). Keep `res.set_status(interfaces::io::types::Status::OK)` (`handler.cppm:27`) — Ghost default 200. Per-row `count` already exists as `RecommendationWire.count = {"clicks":0,"subscribers":0}` (`include/blog/model/wire_fixtures.cppm:187`), so no row-shape change is needed.
- **Check 5 (DB call parity) — fix + caveat.** Resource string `recommendations` already matches (`execute.cppm:15`, `context.cppm:1157`). Ghost calls `countRecommendations({})` + `listRecommendations({page,limit,filter,include,order})` (`recommendation-service.ts:281-296`) backed by repository `getAll`/`getPage` (`bookshelf-repository.ts:158`). Caveat: `ApiDataStore::json_for`, `"recommendations"` branch (`context.cppm:1157`) returns one `RecommendationWire` fixture and ignores `params`, so filtering/pagination is inert. Fix that branch to filter/paginate `m_resources.list("recommendations")` via the context-local `ResourceStore` (tag `resources`, `context.cppm:638-646,1180-1181`) by `params` (`filter`, `limit`, `page`, `order`); `total("recommendations")` already falls through to `m_resources.count("recommendations")` (`context.cppm:1120`). No direct SQL, no new global state.
- **Check 6 (cache parity) — closed, no change required.** Ghost `headers.cacheInvalidate:false` (`recommendations-public.js:8-10`) emits no `X-Cache-Invalidate`; C++ browse step/handler have no `core::cache::Cache` use (`all.cppm:836-839` wires `m_cache` only to members/emails/automations). Both sides: none.
- **Check 7 (auth) — fix.** File `include/blog/handlers/discovery/recommendations/routes.cppm`, symbol `blog::api::RecommendationsRoutes::install` (line 20): add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`) to the router chain, and in `browse/handler.cppm:18-21` replace the inline `token(req).empty()` 401 `{"error":"Unauthorized"}` with the shared Ghost-style error envelope. This approximates Ghost `mw.authenticatePublic` (`content/routes.js:45`); `permissions:true`/`docName:'recommendations'` (`recommendations-public.js:5,12`) has no allowed C++ primitive — accepted residual gap, no new global state.
- **Check 8 (verdict) — closed.** After checks 3/4/5/7, path, verb, input names, output envelope `data`, DB resource `recommendations`, no cache, and token auth agree → MATCH (permissions/docName residual gap only).

Short snippet where the exact replacement is non-trivial (`browse/workflow/step/execute.cppm`, replace lines 14-16):

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
    for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
        const auto eq = text.find('=');
        if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
    }
}
const int page  = params.contains("page")  ? std::stoi(params.at("page"))  : 1;
const int limit = params.contains("limit") ? std::stoi(params.at("limit")) : 5;
const auto rows = ctx.store().json_for("recommendations", params);
const auto total = static_cast<int>(ctx.store().total("recommendations"));
const int pages = limit > 0 ? (total + limit - 1) / limit : 1;
const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
const std::string next = page < pages ? std::to_string(page + 1) : "null";
return core::workflow::StepResponse<Output>{
    "{\"data\":" + rows +
    ",\"meta\":{\"pagination\":{\"page\":" + std::to_string(page) +
    ",\"limit\":" + std::to_string(limit) +
    ",\"total\":" + std::to_string(total) +
    ",\"pages\":" + std::to_string(pages) +
    ",\"prev\":" + prev + ",\"next\":" + next + "}}}"};
```

## Missing in C++

None for the in-scope content route. `GET /recommendations` has a C++ counterpart
(`include/blog/handlers/discovery/recommendations/routes.cppm:19-28`), so no Ghost content route is MISSING.
  Fix: No change required. `GET /recommendations` is the only in-scope content route and already has a
  path+verb C++ registration (`discovery/recommendations/routes.cppm:19-28`); check 1 is closed. The PARTIAL
  gap is reshaped by the `#### Fix` above, not by adding a route. No third-party libs, direct SQL, or new
  global state.

Not route rows, but note for completeness: controller keys `trackClicked` (`recommendations-public.js:19-36`)
and `trackSubscribed` (`recommendations-public.js:38-55`) are exported by this controller but are not
registered in `content/routes.js`; they are wired in the members app at
`docs/Ghost/ghost/core/server/web/members/app.js:175-186` (`POST /api/recommendations/:id/clicked`,
`POST /api/recommendations/:id/subscribed`). Confirmed absence of any matching C++ route: grep for
`clicked|subscribed|track` under `include/blog/handlers` returned no matches.
  Fix: No change required. `trackClicked` / `trackSubscribed` are members-app routes
  (`members/app.js:175-186`), not `content/routes.js` routes, so they are not MISSING in this content-API
  audit. Grep for `clicked|subscribed|track` under `include/blog/handlers` returned no matches; do not add
  routes here.

## Extra in C++

None assigned (EXTRA = 0). The matched folder `include/blog/handlers/discovery/recommendations/routes.cppm`
also installs:
- `POST /recommendations` → `recommendations_create` (`routes.cppm:29-36`)
  - Fix: No change required. This is Ghost admin `api.recommendations.add` (`admin/routes.js:594`), owned by
    the admin recommendations audit, not `recommendations-public.js`. Check 1 (EXTRA) closed; keep installed,
    do not remove or gate the binding. No source edit here.
- `GET/PUT/DELETE /recommendations/:id` → `recommendations_read_by_id` / `edit` / `destroy` (`routes.cppm:37-63`)
  - Fix: No change required. These are Ghost admin `read`/`edit`/`destroy` (`admin/routes.js:593,596,597`),
    owned by the admin audit. Check 1 (EXTRA) closed; keep `recommendations_read_by_id()` / `edit()` /
    `destroy()` installed (`all.cppm:578,581,582`), do not remove or gate. No source edit here.
- `POST /recommendations/check` → `recommendations_recommendations_check_post` (`routes.cppm:64-73`)
  - Fix: No change required. This is Ghost admin `api.recommendations.check` (`admin/routes.js:595`), owned by
    the admin audit. Check 1 (EXTRA) closed; keep `recommendations_recommendations_check_post()` installed
    (`all.cppm:580`), do not remove or gate. No source edit here.

These have no counterpart in the public controller `recommendations-public.js`, but they do map to the
Ghost **admin** recommendations controller (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:592-597`),
which is a separate controller owned by a separate audit. They are therefore out of scope here and are
not counted as EXTRA relative to Ghost overall.

## Implementation status (manifest group 18 — superseded)

Applied locally in `include/blog/handlers/discovery/recommendations/` (no new files):

- `routes.cppm` now installs `content_type_middleware` + `blog::handlers::middleware::require_auth`;
  the inline `token(req).empty()` 401 branch was removed from `browse/handler.cppm`. This approximates
  Ghost `mw.authenticatePublic`; `permissions:true`/`docName:'recommendations'` has no allowed C++
  primitive (accepted residual).
- `browse/workflow/step/execute.cppm` now parses `limit`/`page`/`include`/`withRelated`/`filter`/`order`
  from the query string, validates include/order allow-lists, derives real `total`/`pages`/`prev`/`next`
  from the context-local ResourceStore (fixture fallback) and emits the Ghost
  `{data:[...],meta.pagination}` envelope. `Output` remains `std::string` (per check 4), so validation
  errors are returned as the `ApiError{}.validation(...)` body with 200 — full 422 needs an Output/handler
  change and is recorded as residual.
- Cache stays "none" for browse (Ghost `cacheInvalidate:false`).

Residual — central/shared work NOT applied: `include/blog/handlers/context/context_context.cppm`
`"recommendations"` branches for `json_for`/`find_by_resource` remain naive; equivalent
filter/pagination/lookup is done inside the step via public `ApiDataStore::resources()`.
Full NQL filter/order and public-key auth have no allowed primitive. Build verification is blocked by
pre-existing missing `discovery/search_index/search_index_authors_get/*` files referenced from the
shared `BUILD.bazel`.

Central registrations needed: none — no new files; all six handlers/steps/flows/routes already registered.

## Implementation status (manifest group 19)

The content-API browse step now uses `core::auth::ghost` (anonymous `CONTENT_API` principal is
permitted for public recommendations reads) and `core::query` (allow-listed `limit`/`order`/`page`/
`filter`/`include`, NQL filter, restricted-field guard, Ghost defaults 5/1, unfiltered
`meta.pagination.total`). Output uses the `blog::mapping`-shaped `wrap`/serializer helpers with the
Ghost `{data:[...],meta.pagination}` envelope and synthesized `count` from the event store; no
fixture fabrication and no `X-Cache-Invalidate` header. The two members-app ops are now implemented
as well (tracked in `behavior/recommendations-public.md`) but remain out of scope for this
content-API controller audit. No shared file was edited; the pre-existing shared Bazel cycle was not
touched.
