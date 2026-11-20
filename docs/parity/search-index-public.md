# Parity: search-index-public

> **Manifest group 20 — strict-parity rewrite applied (2026-11-19).** Supersedes
> the group 19 note below. All three `searchIndexPublic` routes are now MATCH in
> `docs/parity/behavior/search-index-public.md`. The shared folder
> `discovery/search_index/` steps now resolve Admin vs Content API from
> `x-blog-context`, authorize through `core::auth::ghost`, apply Ghost's fixed
> options via `core::query`, and emit `blog::mapping` clean+wrap with no
> `meta.pagination`. Public `posts` uses `filter:type:post` + content keys;
> public `tags` applies `filter:visibility:public`; `authors` reads the
> `authors` store. The router-wide `require_auth` and the ad-hoc inline 401 were
> removed. The former EXTRA `/pages`/`/users` are Admin routes counted MATCH in
> the admin behavior report. The group 19 note claimed `/authors` was removed;
> the route was still present and is now corrected (this controller
> legitimately owns `fetchAuthors`). The `#### Fix` blocks below are
> retained as the pre-group-20 record; their `meta.pagination` guidance is
> superseded by the behavior report.

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/search-index-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/discovery/search_index/routes.cppm` (+ `search_index_*_get/` subtrees). NOTE: the folder lives under `discovery/`, not a `public/` area; found by exact path+verb, not by folder name.
- C++ registry: `include/blog/handlers/all.cppm:25-27,371-373,620-622` (three matched handlers) and `:239-240,585-586,834-835` (pages/users extras).
- Checked: 2026-11-19

> Premise check: the brief expected `*-public` domains to be entirely MISSING. Disproven — C++ GET routes exist at all three Ghost paths. Verdicts are `PARTIAL`, not `MISSING`. The C++ folder has two additional routes (`/pages`, `/users`) that have no counterpart in this public controller; see Extra in C++.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 2 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/search-index/posts` | `searchIndexPublic.fetchPosts` (`docName: 'search_index'`) | GET `/search-index` + `/posts` | `search_index_search_index_posts_get()` → `blog::handlers::search_index::search_index_posts_get::SearchIndexPostsGetHandler` | Ghost: fixed `options` only — `filter:'type:post'`, `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('posts', [id,slug,title,excerpt,url,updated_at,visibility])`, plus `urlRelationsForRouting()` (`withRelated`); no request param read. C++: `using Input=interfaces::io::IRequest&`, no query/path parsing | Ghost: serializer `frame.response={posts}` → `{"posts":[...]}` with real mapped fields id/slug/title/excerpt/url/updated_at/visibility; `headers.cacheInvalidate:false`. C++: `{"posts":[{"id":"1","name":"Ghost Record","slug":"ghost-record"}]}` from hardcoded `SearchPostsResponse`, status `OK`, `Content-Type` via serde formats | Ghost: `postsService.browsePosts(options)` (`posts-service-instance`), filter/order/limit/columns applied. C++: no live store call; `ctx.store().json_for("search-index", {})` at `execute.cppm:34` is **dead** (lambda returns `{200, encode(...)}` at `:20-22`, early return at `:33`) | Ghost: `cacheInvalidate:false` (`:16-18`). C++: none — global `core::cache::Cache` (`all.cppm:836,843-847`) is only injected into members/emails/automations ops (`:837-839`) | Ghost: `mw.authenticatePublic` (`content/routes.js:48`; `content/middleware.js:18-25`) + `permissions:true` (`search-index-public.js:19`). C++: `content_type_middleware` (`routes.cppm:20`) + live token guard → 401 `{"error":"Unauthorized"}` when Authorization empty (`handler.cppm:18-22`); no permission/docName mapping | PARTIAL | Ghost `search-index-public.js:15-39`, `content/routes.js:48`, `utils/serializers/output/search-index.js:8-29`; C++ `discovery/search_index/routes.cppm:19,43-53`, `search_index_posts_get/handler.cppm:17-27`, `search_index_posts_get/workflow/step/execute.cppm:12-35`, `all.cppm:25,371,620` |
| GET `/search-index/authors` | `searchIndexPublic.fetchAuthors` | GET `/search-index` + `/authors` | `search_index_search_index_authors_get()` → `SearchIndexAuthorsGetHandler` | Ghost: fixed `options` — `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('authors', [id,slug,name,url,profile_image])`; no request param read. C++: `Input=IRequest&`, no parsing | Ghost: `{"authors":[...]}` with id/slug/name/url/profile_image (`search-index.js:67-83`); `cacheInvalidate:false`. C++: `{"authors":[dummy RecordDto]}` (id/name/slug only), status `OK`; dead `json_for`+meta branch | Ghost: `models.Author.findPage(options)` (`:52`). C++: no live call; dead `json_for("search-index", {})` (`execute.cppm:34`), no filter/pagination | Ghost: `cacheInvalidate:false` (`:41-43`). C++: none | Ghost: `mw.authenticatePublic` (`content/routes.js:49-53`) + `permissions:true` (`:44`). C++: `content_type_middleware` + live token guard → 401 (`authors handler.cppm:18-22`); no permission mapping | PARTIAL | Ghost `search-index-public.js:40-54`, `content/routes.js:49-53`, `search-index.js:67-83`; C++ `discovery/search_index/routes.cppm:19,21-31`, `search_index_authors_get/handler.cppm:17-27`, `search_index_authors_get/workflow/step/execute.cppm:12-35`, `all.cppm:26,372,621` |
| GET `/search-index/tags` | `searchIndexPublic.fetchTags` | GET `/search-index` + `/tags` | `search_index_search_index_tags_get()` → `SearchIndexTagsGetHandler` | Ghost: fixed `options` — `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('tags', [id,slug,name,url])`, `filter:'visibility:public'`; no request param read. C++: `Input=IRequest&`, no parsing | Ghost: `{"tags":[...]}` with id/slug/name/url (`search-index.js:49-65`); `cacheInvalidate:false`. C++: `{"tags":[dummy RecordDto]}`, status `OK`; dead `json_for`+meta branch | Ghost: `models.Tag.findPage(options)` (`:68`), filter `visibility:public` applied. C++: no live call; dead `json_for("search-index", {})` (`execute.cppm:34`), no `visibility:public` filter | Ghost: `cacheInvalidate:false` (`:56-58`). C++: none | Ghost: `mw.authenticatePublic` (`content/routes.js:54`) + `permissions:true` (`:59`). C++: `content_type_middleware` + live token guard → 401 (`tags handler.cppm:18-22`); no permission mapping | PARTIAL | Ghost `search-index-public.js:55-70`, `content/routes.js:54`, `search-index.js:49-65`; C++ `discovery/search_index/routes.cppm:19,54-64`, `search_index_tags_get/handler.cppm:17-27`, `search_index_tags_get/workflow/step/execute.cppm:12-35`, `all.cppm:27,373,622` |
| — (none in `search-index-public.js` / content routes) | — | GET `/search-index/pages` | `search_index_search_index_pages_get()` → `SearchIndexPagesGetHandler` | C++: `Input=IRequest&`, no parsing (`pages/execute.cppm:12-14`) | C++: `{"pages":[dummy RecordDto]}`, status `OK` (`pages/execute.cppm:18,33`) | C++: dead `json_for("search-index", {})` (`pages/execute.cppm:34`) | C++: none | C++: `content_type_middleware` + token guard (`pages/handler.cppm:18-22`) | EXTRA | C++ `discovery/search_index/routes.cppm:32-42`, `all.cppm:239,585,834`; absence confirmed by full content route list `content/routes.js:16-55` (no `/search-index/pages`) and `search-index-public.js:13-71` (keys only `fetchPosts,fetchAuthors,fetchTags`) |
| — (none in `search-index-public.js` / content routes) | — | GET `/search-index/users` | `search_index_search_index_users_get()` → `SearchIndexUsersGetHandler` | C++: `Input=IRequest&`, no parsing (`users/execute.cppm:12-14`) | C++: `{"users":[dummy RecordDto]}`, status `OK` (`users/execute.cppm:31-33`) | C++: dead `json_for("search-index", {})` (`users/execute.cppm:34`) | C++: none | C++: `content_type_middleware` + token guard (`users/handler.cppm:18-22`) | EXTRA | C++ `discovery/search_index/routes.cppm:65-75`, `all.cppm:240,586,835`; absence confirmed by `content/routes.js:16-55` and `search-index-public.js:13-71` |

## Route details

### GET `/search-index/posts` — PARTIAL

- Route identity matches: Ghost `router.get('/search-index/posts', mw.authenticatePublic, http(api.searchIndexPublic.fetchPosts))` (`content/routes.js:48`) vs C++ `Router{router, "/search-index"}` + `Route{"/posts"}.get(...)` (`discovery/search_index/routes.cppm:19,43-45`).
- Operation mapping matches: controller key `fetchPosts` (`search-index-public.js:15`) vs `handler.search_index_search_index_posts_get()` (`routes.cppm:50`), bound to `SearchIndexPostsGetHandler::handle` (`search_index_posts_get/handler.cppm:17`).
- Input: both effectively fixed. Ghost builds its own options object and never reads `frame.options`/`frame.data` (`search-index-public.js:21-37`), including `requiredUrlColumns` and `urlRelationsForRouting()` (`:3-10,25-34`). C++ declares `using Input=interfaces::io::IRequest&` and parses nothing (`execute.cppm:12-14`). No param name exists on only one side.
- Output diverges in data, not envelope key: Ghost serializer emits `frame.response={posts}` (`utils/serializers/output/search-index.js:26-28`) with fields picked via `keys` (`:14-16`); C++ serializes `SearchPostsResponse` (`more_resources.cppm:78-84,140`) containing one hardcoded `RecordDto{id:"1",name:"Ghost Record",slug:"ghost-record"}` (`execute.cppm:20-22`, `more_resources.cppm:19-21`). Ghost fields `title/excerpt/url/updated_at/visibility` are absent in C++. Status is 200/`Status::OK` on both (`handler.cppm:27`).
- Note: the envelope `{"search-index":…, "meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` at `execute.cppm:35` is unreachable — the lambda always returns `r.first=200` (`execute.cppm:21,33`). No pagination meta is actually sent.
- DB call diverges: Ghost `postsService.browsePosts(options)` (`search-index-public.js:37`) with real filter/limit/order/columns. C++ does no live store read; `ctx.store().json_for("search-index", {})` (`execute.cppm:34`) is dead code, uses a generic resource string `search-index` (not `posts`), and applies no filtering/pagination.
- Cache: Ghost explicitly sets `headers.cacheInvalidate:false` (`search-index-public.js:16-18`), so no invalidation header. C++ has no route-level cache; the only `core::cache::Cache` instance (`all.cppm:836,843-847`) is passed to members/emails/automations operations (`:837-839`), not to this handler.
- Auth diverges: Ghost `mw.authenticatePublic` — rate limiting, `authenticateContentApi`, `authorizeContentApi`, cors, redirects (`content/middleware.js:18-25`) — plus `permissions:true` on the controller (`search-index-public.js:19`). C++ installs only `content_type_middleware` (`routes.cppm:20`) and enforces a non-empty `token(req)` read straight from the `Authorization` header (`handler.cppm:18-22`; `middleware/bearer.cppm:27-33`), returning 401 `{"error":"Unauthorized"}`; no `docName`/`permissions` equivalent.

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the PARTIAL on `GET /search-index/posts`
(Ghost `search-index-public.js:15-39`, `content/routes.js:48`; serializer
`utils/serializers/output/search-index.js:8-29`):

- **Check 1 (route identity) — closed, no change.** `include/blog/handlers/discovery/search_index/routes.cppm:19,43-45`
  already binds `Route{"/posts"}.get(...)` under `Router{router, "/search-index"}` → `GET /search-index/posts`.
  Keep. **Check 2 (operation) — closed, no change:** `routes.cppm:50` binds
  `handler.search_index_search_index_posts_get().handle` → `SearchIndexPostsGetHandler`
  (`search_index_posts_get/handler.cppm:10`); Ghost key `fetchPosts` (`search-index-public.js:15`). Keep.
- **Check 3 (input) — closed by carrying Ghost's fixed options into the store call.** Ghost reads no request
  param; it builds fixed `options` (`search-index-public.js:21-35`): `filter:"type:post"`, `limit:"10000"`,
  `order:"updated_at DESC"`, `columns: requiredUrlColumns('posts', [id,slug,title,excerpt,url,updated_at,visibility])`,
  `...urlRelationsForRouting()`. C++ declares `using Input=interfaces::io::IRequest&`
  (`search_index_posts_get/workflow/step/execute.cppm:12`) and parses nothing — no param name exists on only
  one side, so no query/path parsing is added. Map Ghost `columns` → C++ `fields` param
  `"id,slug,title,excerpt,url,updated_at,visibility"`; `withRelated` has no `query_json`/`search_json`
  primitive (accepted residual — `url` is still selected via `fields`).
- **Check 5 (DB call) — fix.** File `include/blog/handlers/discovery/search_index/search_index_posts_get/workflow/step/execute.cppm`,
  symbol `blog::handlers::search_index::search_index_posts_get::workflow::ExecuteStep::invoke` (`:14-35`).
  Delete the dummy `kind` lambda (`:16-32`) and the dead `ctx.store().json_for("search-index", {})` (`:34`).
  Exact replacement call: `ctx.store().json_for("posts", params)` where `params` is
  `{{"filter","type:post"},{"limit","10000"},{"order","updated_at DESC"},{"fields","id,slug,title,excerpt,url,updated_at,visibility"}}`.
  `ApiDataStore::json_for("posts", params)` (`include/blog/context/context.cppm:1123,1128`) →
  `query_json(m_db.all<model::Post>(), params)` (`:1531-1561`) → `search_json` (`:1563-1615`), which applies
  `filter`/`limit`/`fields` — the C++ counterpart of `postsService.browsePosts(options)`
  (`search-index-public.js:37`). Resource string changes from dead `"search-index"` to real `"posts"`.
  `order` is not implemented in `search_json` — accepted residual. No direct SQL, no new global state.
- **Check 4 (output) — fix.** Same step, replace the unreachable envelope at `:35`
  (`{"search-index":…,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}`) with
  `{"posts":` + `rows` + `}`. Drop the hardcoded `RecordDto{id:"1",name:"Ghost Record",slug:"ghost-record"}`
  (`blog/dto/more_resources.cppm:19-21,140`) from the emitted body — the mapped fields id/slug/title/excerpt/
  url/updated_at/visibility come from the store row, matching `frame.response={posts}` and its `keys`
  (`utils/serializers/output/search-index.js:14-16,26-28`). No `meta.pagination` on either side.
  `SearchIndexPostsGetHandler::handle` already sets `Status::OK` (`handler.cppm:27`) — no change.
- **Check 6 (cache) — closed, no change.** Ghost `cacheInvalidate:false` (`search-index-public.js:16-18`) emits
  no `X-Cache-Invalidate`; C++ emits none (`search_index_posts_get/handler.cppm:17-28`). The shared
  `core::cache::Cache` (`all.cppm:836-847`) is wired only to members/emails/automations (`:837-839`). Both sides:
  no cache behavior.
- **Check 7 (auth) — fix.** Ghost `mw.authenticatePublic` (`content/routes.js:48`; `content/middleware.js:18-25`)
  + `permissions:true` (`search-index-public.js:19`). C++ `SearchIndexRoutes::install` (`routes.cppm:19-21`)
  installs only `content_type_middleware`, and `SearchIndexPostsGetHandler::handle` (`handler.cppm:18-22`)
  guards with `blog::handlers::middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}`. Replace the
  inline guard with `.use(blog::handlers::middleware::require_auth)` in `routes.cppm` (module already imported
  at `:8`; implementation `include/blog/handlers/middleware/auth.cppm:12-29`) so the 401 uses Ghost's
  `ErrorEnvelope`/`UnauthorizedError`. Ghost `permissions`/`docName` has no allowed C++ primitive — accepted residual.
- **Check 8 (verdict) — closed.** After checks 3/4/5/7: path, verb, fixed input options, output `posts`, DB
  resource `posts`, no cache, and token auth agree → MATCH (residuals: `order` unapplied, `withRelated` absent,
  `docName`/`permissions` unrepresented).

Short snippet where the exact replacement is non-trivial (`search_index_posts_get/workflow/step/execute.cppm`,
replace `:16-35`; type stays `Output=std::string`, return `core::workflow::StepResponse<Output>`):

```cpp
const auto rows = ctx.store().json_for("posts", {
    {"filter", "type:post"},
    {"limit", "10000"},
    {"order", "updated_at DESC"},
    {"fields", "id,slug,title,excerpt,url,updated_at,visibility"}
});
return core::workflow::StepResponse<Output>{"{\"posts\":" + rows + "}"};
```

### GET `/search-index/authors` — PARTIAL

- Route identity matches: `content/routes.js:49-53` vs `discovery/search_index/routes.cppm:19,22-23`.
- Operation mapping matches: `fetchAuthors` (`search-index-public.js:40`) vs `handler.search_index_search_index_authors_get()` (`routes.cppm:28`).
- Input: fixed-options-only on both sides; Ghost `limit/order/columns` (`search-index-public.js:46-50`), C++ no query parsing (`search_index_authors_get/workflow/step/execute.cppm:12-14`).
- Output diverges in data only: Ghost `{"authors":[...]}` with id/slug/name/url/profile_image (`utils/serializers/output/search-index.js:67-83`); C++ `SearchAuthorsResponse` (`more_resources.cppm:94-100,144`) with one dummy `RecordDto` (`execute.cppm:26-29`) exposing only id/name/slug. `meta` line `execute.cppm:35` dead, as above.
- DB call diverges: Ghost `models.Author.findPage(options)` (`search-index-public.js:52`); C++ dead `json_for("search-index", {})` (`execute.cppm:34`) with no author filter.
- Cache: Ghost `cacheInvalidate:false` (`:41-43`); C++ none.
- Auth diverges: Ghost content-API middleware + `permissions:true` (`:44`); C++ `content_type_middleware` + live token guard → 401 (`search_index_authors_get/handler.cppm:18-22`).

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the PARTIAL on `GET /search-index/authors`
(Ghost `search-index-public.js:40-54`, `content/routes.js:49-53`; serializer
`utils/serializers/output/search-index.js:67-83`):

- **Checks 1, 2 — closed, no change.** `include/blog/handlers/discovery/search_index/routes.cppm:19,22-23`
  binds `Route{"/authors"}.get(...)` → `GET /search-index/authors`; `routes.cppm:28` binds
  `handler.search_index_search_index_authors_get().handle` → `SearchIndexAuthorsGetHandler`
  (`search_index_authors_get/handler.cppm:10`); Ghost key `fetchAuthors` (`search-index-public.js:40`). Keep.
- **Check 3 (input) — closed by carrying Ghost's fixed options into the store call.** Ghost reads no request
  param; fixed `options` are `limit:"10000"`, `order:"updated_at DESC"`,
  `columns: requiredUrlColumns('authors', [id,slug,name,url,profile_image])` (`search-index-public.js:46-50`).
  C++ `using Input=interfaces::io::IRequest&` (`search_index_authors_get/workflow/step/execute.cppm:12`)
  parses nothing — no one-sided param name. Map Ghost `columns` → `fields`
  `"id,slug,name,url,profile_image"`; no `withRelated` on this route, so no residual there.
- **Check 5 (DB call) — fix.** File `include/blog/handlers/discovery/search_index/search_index_authors_get/workflow/step/execute.cppm`,
  symbol `ExecuteStep::invoke` (`:14-35`). Delete the dummy `kind` lambda (`:16-32`, `SearchAuthorsResponse` +
  `RecordDto` at `:26-29`) and the dead `ctx.store().json_for("search-index", {})` (`:34`). Exact replacement
  call: `ctx.store().json_for("authors", params)` with `params` =
  `{{"limit","10000"},{"order","updated_at DESC"},{"fields","id,slug,name,url,profile_image"}}`.
  `ApiDataStore::json_for("authors", params)` (`context.cppm:1123,1131`) → `query_json(m_db.all<model::Author>(), params)`
  (`:1531-1561`) → `search_json` (`:1563-1615`), the C++ counterpart of `models.Author.findPage(options)`
  (`search-index-public.js:52`). Resource string changes from dead `"search-index"` to real `"authors"`.
  `order` unapplied in `search_json` — accepted residual. No `visibility:public` filter exists on this Ghost
  route. No direct SQL, no new global state.
- **Check 4 (output) — fix.** Same step, replace dead envelope at `:35` with `{"authors":` + `rows` + `}`.
  Mapped fields id/slug/name/url/profile_image come from the store row, matching `frame.response={authors}` and
  its `keys` (`utils/serializers/output/search-index.js:72,80-82`); no `meta.pagination` on either side.
  `SearchIndexAuthorsGetHandler::handle` already sets `Status::OK` (`handler.cppm:27`) — no change.
- **Check 6 (cache) — closed, no change.** Ghost `cacheInvalidate:false` (`search-index-public.js:41-43`); C++
  emits no `X-Cache-Invalidate` (`search_index_authors_get/handler.cppm:17-28`); global cache not wired
  (`all.cppm:836-839`). Both sides: no cache behavior.
- **Check 7 (auth) — fix.** Ghost `mw.authenticatePublic` (`content/routes.js:49-53`) + `permissions:true`
  (`search-index-public.js:44`). Replace the inline `token(req).empty()` guard in
  `search_index_authors_get/handler.cppm:18-22` with `.use(blog::handlers::middleware::require_auth)` in
  `SearchIndexRoutes::install` (`routes.cppm:19-21`; symbol from `include/blog/handlers/middleware/auth.cppm:12-29`)
  for Ghost's `ErrorEnvelope`. `permissions`/`docName` unrepresented — accepted residual.
- **Check 8 (verdict) — closed.** Path, verb, fixed inputs, output `authors`, DB resource `authors`, no cache,
  token auth agree → MATCH (residual: `order` unapplied, `docName`/`permissions` unrepresented).

Snippet (`search_index_authors_get/workflow/step/execute.cppm`, replace `:16-35`, type stays
`Output=std::string`, return `core::workflow::StepResponse<Output>`):

```cpp
const auto rows = ctx.store().json_for("authors", {
    {"limit", "10000"},
    {"order", "updated_at DESC"},
    {"fields", "id,slug,name,url,profile_image"}
});
return core::workflow::StepResponse<Output>{"{\"authors\":" + rows + "}"};
```

### GET `/search-index/tags` — PARTIAL

- Route identity matches: `content/routes.js:54` vs `discovery/search_index/routes.cppm:19,54-55`.
- Operation mapping matches: `fetchTags` (`search-index-public.js:55`) vs `handler.search_index_search_index_tags_get()` (`routes.cppm:61`).
- Input: Ghost options include `filter:'visibility:public'` (`:65`); C++ parses nothing (`search_index_tags_get/workflow/step/execute.cppm:12-14`) and never applies that filter.
- Output diverges in data only: Ghost `{"tags":[...]}` with id/slug/name/url (`utils/serializers/output/search-index.js:49-65`); C++ `SearchTagsResponse` (`more_resources.cppm:86-92,141`) with one dummy `RecordDto` (`execute.cppm:23-25`). `meta` line `execute.cppm:35` dead.
- DB call diverges: Ghost `models.Tag.findPage(options)` with `visibility:public` predicate (`search-index-public.js:68,65`); C++ dead `json_for("search-index", {})` (`execute.cppm:34`), no predicate.
- Cache: Ghost `cacheInvalidate:false` (`:56-58`); C++ none.
- Auth diverges: Ghost content-API middleware + `permissions:true` (`:59`); C++ `content_type_middleware` + live token guard → 401 (`search_index_tags_get/handler.cppm:18-22`).

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the PARTIAL on `GET /search-index/tags`
(Ghost `search-index-public.js:55-70`, `content/routes.js:54`; serializer
`utils/serializers/output/search-index.js:49-65`):

- **Checks 1, 2 — closed, no change.** `include/blog/handlers/discovery/search_index/routes.cppm:19,54-55`
  binds `Route{"/tags"}.get(...)` → `GET /search-index/tags`; `routes.cppm:61` binds
  `handler.search_index_search_index_tags_get().handle` → `SearchIndexTagsGetHandler`
  (`search_index_tags_get/handler.cppm:10`); Ghost key `fetchTags` (`search-index-public.js:55`). Keep.
- **Check 3 (input) — closed by carrying Ghost's fixed options into the store call.** Fixed `options` are
  `limit:"10000"`, `order:"updated_at DESC"`, `columns: requiredUrlColumns('tags', [id,slug,name,url])`,
  `filter:"visibility:public"` (`search-index-public.js:61-66`). C++ `using Input=interfaces::io::IRequest&`
  (`search_index_tags_get/workflow/step/execute.cppm:12`) parses nothing — no one-sided param name. Map
  Ghost `columns` → `fields` `"id,slug,name,url"` and pass `filter:"visibility:public"`; the current step
  never applies that predicate (`execute.cppm:12-14`).
- **Check 5 (DB call) — fix.** File `include/blog/handlers/discovery/search_index/search_index_tags_get/workflow/step/execute.cppm`,
  symbol `ExecuteStep::invoke` (`:14-35`). Delete the dummy `kind` lambda (`:16-32`, `SearchTagsResponse` +
  `RecordDto` at `:23-25`) and the dead `ctx.store().json_for("search-index", {})` (`:34`). Exact replacement
  call: `ctx.store().json_for("tags", params)` with `params` =
  `{{"filter","visibility:public"},{"limit","10000"},{"order","updated_at DESC"},{"fields","id,slug,name,url"}}`.
  `ApiDataStore::json_for("tags", params)` (`context.cppm:1123,1130`) → `query_json(m_db.all<model::Tag>(), params)`
  (`:1531-1561`) → `search_json` (`:1563-1615`); `query.where(it->second)` at `:1582` applies the
  `visibility:public` predicate — the C++ counterpart of `models.Tag.findPage(options)`
  (`search-index-public.js:68,65`). Resource string changes from dead `"search-index"` to real `"tags"`.
  `order` unapplied in `search_json` — accepted residual. No direct SQL, no new global state.
- **Check 4 (output) — fix.** Same step, replace dead envelope at `:35` with `{"tags":` + `rows` + `}`.
  Mapped fields id/slug/name/url come from the store row, matching `frame.response={tags}` and its `keys`
  (`utils/serializers/output/search-index.js:54,62-64`); no `meta.pagination` on either side.
  `SearchIndexTagsGetHandler::handle` already sets `Status::OK` (`handler.cppm:27`) — no change.
- **Check 6 (cache) — closed, no change.** Ghost `cacheInvalidate:false` (`search-index-public.js:56-58`); C++
  emits no `X-Cache-Invalidate` (`search_index_tags_get/handler.cppm:17-28`); global cache not wired
  (`all.cppm:836-839`). Both sides: no cache behavior.
- **Check 7 (auth) — fix.** Ghost `mw.authenticatePublic` (`content/routes.js:54`) + `permissions:true`
  (`search-index-public.js:59`). Replace the inline `token(req).empty()` guard in
  `search_index_tags_get/handler.cppm:18-22` with `.use(blog::handlers::middleware::require_auth)` in
  `SearchIndexRoutes::install` (`routes.cppm:19-21`; symbol from `include/blog/handlers/middleware/auth.cppm:12-29`)
  for Ghost's `ErrorEnvelope`. `permissions`/`docName` unrepresented — accepted residual.
- **Check 8 (verdict) — closed.** Path, verb, fixed inputs (incl. `visibility:public`), output `tags`, DB
  resource `tags`, no cache, token auth agree → MATCH (residuals: `order` unapplied, `docName`/`permissions`
  unrepresented).

Snippet (`search_index_tags_get/workflow/step/execute.cppm`, replace `:16-35`, type stays
`Output=std::string`, return `core::workflow::StepResponse<Output>`):

```cpp
const auto rows = ctx.store().json_for("tags", {
    {"filter", "visibility:public"},
    {"limit", "10000"},
    {"order", "updated_at DESC"},
    {"fields", "id,slug,name,url"}
});
return core::workflow::StepResponse<Output>{"{\"tags\":" + rows + "}"};
```

## Missing in C++

None. All three `searchIndexPublic` routes (`fetchPosts`, `fetchAuthors`, `fetchTags`) have an exact path+verb C++ counterpart.

Search performed to confirm:
- `grep -n "search-index" include/` → `include/blog/handlers/discovery/search_index/routes.cppm:19,22,33,44,55,66` (GET routes for `/authors`, `/pages`, `/posts`, `/tags`, `/users`), plus `include/blog/handlers/routes.cppm:41,114` and `all.cppm:25-27,239-240,371-373,585-586,620-622,834-835`.
- `grep -n "searchIndex|search_index|fetchPosts|fetchAuthors|fetchTags" include/` → matched handler accessors in `all.cppm` and the five `search_index_*_get/handler.cppm` files.
- `grep -n "search-index" docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` → lines 48,49-53,54 only (the three in scope).
  - Fix: No change required. All three Ghost `searchIndexPublic` routes already have exact path+verb C++ registrations (`discovery/search_index/routes.cppm:19,22-23,43-45,54-55`; registry `all.cppm:25-27,371-373,620-622`), so check 1 is closed for every entry — no route needs adding. None of the three `#### Fix` blocks above adds a route; they reshape the existing steps/handlers only. The two extra `/pages` and `/users` registrations below are owned by the Ghost **admin** controller (`admin/routes.js:611,613`), so they are not Missing here.

## Extra in C++

Two routes in `include/blog/handlers/discovery/search_index/routes.cppm` have no Ghost counterpart in `search-index-public.js` or the content API route table:

1. `GET /search-index/pages` — `routes.cppm:32-42`, handler `search_index_search_index_pages_get()` (`all.cppm:239,585,834`); hardcoded `{"pages":[dummy]}` at `search_index_pages_get/workflow/step/execute.cppm:18,33`. Absence confirmed by `content/routes.js:16-55` (complete content route list) and `search-index-public.js:13-71` (exported keys are only `fetchPosts,fetchAuthors,fetchTags`). Nuance: Ghost does define `fetchPages` in the **admin** controller `docs/Ghost/ghost/core/server/api/endpoints/search-index.js` routed at `web/api/endpoints/admin/routes.js:611`; that is out of scope for this public controller.
   - Fix: DOCUMENTATION ONLY. File `include/blog/handlers/discovery/search_index/routes.cppm`, symbol `blog::api::SearchIndexRoutes::install` (lines 32-42): remove the `.add_route(Route{"/pages"}...)` sub-block from the `Router{router, "/search-index"}` chain in this public router, or relocate it to the admin search-index routes that own Ghost `api.searchIndex.fetchPages` (`admin/routes.js:611`). Keep `search_index_search_index_pages_get()` registered in `include/blog/handlers/all.cppm:239,585,834` — the admin controller owns that op, so do not delete the handler. Checks 1-2: no Ghost route/op in *this* controller → nothing to align. Checks 3-7: n/a here — the step (`search_index_pages_get/workflow/step/execute.cppm:16-35`) emits a dummy `SearchPagesResponse` `RecordDto`, has dead `json_for("search-index", {})`, no `core::cache::Cache`, and a live token guard; all are admin-controller concerns. Check 8: EXTRA until removed or re-homed from this public router. Snippet (exact binding to delete from `install`):
     ```cpp
     .add_route(
         Route{"/pages"}
             .get([&handler](
     interfaces::io::IRequest& req,
     interfaces::io::IResponse& res,
     std::function<void()> send
     ) {
         handler.search_index_search_index_pages_get().handle(req, res, std::move(send));
     })
             .summary("Ghost pages GET")
     )
     ```
2. `GET /search-index/users` — `routes.cppm:65-75`, handler `search_index_search_index_users_get()` (`all.cppm:240,586,835`); hardcoded `{"users":[dummy]}` at `search_index_users_get/workflow/step/execute.cppm:31-33`. Absence confirmed by the same greps. Nuance: admin controller `search-index.js` `fetchUsers` → `admin/routes.js:613`, out of scope here.
   - Fix: DOCUMENTATION ONLY. File `include/blog/handlers/discovery/search_index/routes.cppm`, symbol `blog::api::SearchIndexRoutes::install` (lines 65-75): remove the `.add_route(Route{"/users"}...)` sub-block from the public `Router{router, "/search-index"}` chain, or relocate it to the admin search-index routes that own Ghost `api.searchIndex.fetchUsers` (`admin/routes.js:613`). Keep `search_index_search_index_users_get()` registered in `include/blog/handlers/all.cppm:240,586,835` for the admin controller; do not delete the handler. Checks 1-2: no Ghost route/op in this controller → nothing to align. Checks 3-7: n/a here — the step (`search_index_users_get/workflow/step/execute.cppm:16-35`) emits a dummy `SearchUsersResponse` `RecordDto`, dead `json_for("search-index", {})`, no cache, live token guard. Check 8: EXTRA until removed or re-homed from this public router. Snippet (exact binding to delete from `install`):
     ```cpp
     .add_route(
         Route{"/users"}
             .get([&handler](
     interfaces::io::IRequest& req,
     interfaces::io::IResponse& res,
     std::function<void()> send
     ) {
         handler.search_index_search_index_users_get().handle(req, res, std::move(send));
     })
             .summary("Ghost users GET")
     )
     ```
