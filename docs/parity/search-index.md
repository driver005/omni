# Parity: search-index

> **Manifest group 20 — strict-parity rewrite applied (2026-11-19).** Supersedes
> the group 19 note below. All four Admin routes are now MATCH in
> `docs/parity/behavior/search-index.md`. Each step builds a
> `core::auth::ghost::Principal`, authorizes the Ghost docName via
> `GhostAuthorizer`, fetches store rows, and runs Ghost's fixed options through
> `core::query::apply` (NQL filter, `updated_at DESC` order, `limit:10000`,
> `fields`) before `blog::mapping` clean+wrap. The fabricated `meta.pagination`
> was removed: Ghost's `output/search-index.js` serializers emit no meta. The
> group 19 note claimed `/authors` was removed; the route was still present in
> the folder and is now corrected — `/authors` is a legitimate Content API
> route (`search-index-public.js:40-54`), and its step now reads the `authors`
> store. The router-wide `require_auth` was replaced by
> per-request Ghost authorization so anonymous public reads pass. `core::outbox`
> is intentionally unused (no side effects in these controllers). The audit
> tables and `#### Fix` blocks below are retained as the pre-group-20 record;
> their `meta.pagination` guidance is superseded by the behavior report.

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/search-index.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/discovery/search_index/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 4 |
| MISSING | 0 |
| EXTRA | 1 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /search-index/posts | `fetchPosts` | GET /search-index/posts | `search_index_search_index_posts_get` | Ghost: hardcoded options only (filter `type:post+status:[draft,published,scheduled,sent]`, limit `10000`, order `updated_at DESC`, columns via `requiredUrlColumns('posts',…)` + url relations) — no request params. C++: `Input=IRequest&`, `in` never read — no parsing. | Ghost: `postsService.browsePosts` result via api-framework (real records + pagination meta), `cacheInvalidate:false`. C++: `encode(SearchPostsResponse)` = one hardcoded `RecordDto{id:"1",name:"Ghost Record",slug:"ghost-record"}` under key `posts`, no meta; `{"search-index":…,"meta":…}` envelope is dead code. | Ghost: `postsService.browsePosts` (search-index.js:41). C++: none effective — `json_for("search-index",{})` unreachable (execute.cppm:34-35). | Ghost: `cacheInvalidate:false` (search-index.js:16-18). C++: none. | Ghost: `mw.authAdminApi` + permissions `posts/browse` (routes.js:610; search-index.js:19-22). C++: token-nonempty only, no docName/method permissions (handler.cppm:18-22). | MISMATCH | Ghost: search-index.js:15-43, routes.js:610. C++: routes.cppm:44-53, posts handler.cppm:17-28, posts execute.cppm:12-35, more_resources.cppm:10-22/78-84/140. |
| GET /search-index/pages | `fetchPages` | GET /search-index/pages | `search_index_search_index_pages_get` | Ghost: hardcoded options (filter `type:page+status:[draft,published,scheduled]`, limit `10000`, order `updated_at DESC`, columns `requiredUrlColumns('pages',…)` + url relations). C++: no parsing. | Ghost: `postsService.browsePosts` result + pagination meta. C++: one placeholder `RecordDto` under `pages`, no meta; dead `json_for` envelope. | Ghost: `postsService.browsePosts` (search-index.js:70). C++: none effective (execute.cppm:34-35). | Ghost: `cacheInvalidate:false` (search-index.js:45-47). C++: none. | Ghost: `mw.authAdminApi` + permissions `posts/browse` (routes.js:611; search-index.js:48-51). C++: token-nonempty only (handler.cppm:18-22). | MISMATCH | Ghost: search-index.js:44-72, routes.js:611. C++: routes.cppm:33-42, pages handler.cppm:17-28, pages execute.cppm:12-35, more_resources.cppm:70-76/139. |
| GET /search-index/tags | `fetchTags` | GET /search-index/tags | `search_index_search_index_tags_get` | Ghost: hardcoded options (limit `10000`, order `updated_at DESC`, columns `requiredUrlColumns('tags',['id','slug','name','url'])`). C++: no parsing. | Ghost: `models.Tag.findPage` result + pagination meta. C++: one placeholder `RecordDto` under `tags`, no meta; dead `json_for` envelope. | Ghost: `models.Tag.findPage` (search-index.js:88). C++: none effective (execute.cppm:34-35). | Ghost: `cacheInvalidate:false` (search-index.js:74-76). C++: none. | Ghost: `mw.authAdminApi` + permissions `tags/browse` (routes.js:612; search-index.js:77-80). C++: token-nonempty only (handler.cppm:18-22). | MISMATCH | Ghost: search-index.js:73-90, routes.js:612. C++: routes.cppm:55-64, tags handler.cppm:17-28, tags execute.cppm:12-35, more_resources.cppm:86-92/141. |
| GET /search-index/users | `fetchUsers` | GET /search-index/users | `search_index_search_index_users_get` | Ghost: hardcoded options (limit `10000`, order `updated_at DESC`, columns `requiredUrlColumns('authors',['id','slug','url','name','profile_image'])`). C++: no parsing. | Ghost: `models.User.findPage` result + pagination meta. C++: one placeholder `RecordDto` under `users`, no meta; dead `json_for` envelope. | Ghost: `models.User.findPage` (search-index.js:106). C++: none effective (execute.cppm:34-35). | Ghost: `cacheInvalidate:false` (search-index.js:92-94). C++: none. | Ghost: `mw.authAdminApi` + permissions `users/browse` (routes.js:613; search-index.js:95-98). C++: token-nonempty only (handler.cppm:18-22). | MISMATCH | Ghost: search-index.js:91-108, routes.js:613. C++: routes.cppm:66-75, users handler.cppm:17-28, users execute.cppm:12-35, more_resources.cppm:102-108/142. |

## Route details

### GET /search-index/posts — MISMATCH

- Route identity agrees (GET both): Ghost `routes.js:610` vs C++ `routes.cppm:44-53` (`Router{router,"/search-index"}` at `routes.cppm:19`, `Route{"/posts"}.get(...)` at `44-45`).
- Data shape materially different. Ghost returns real post records with url columns and pagination meta via `postsService.browsePosts` (`search-index.js:23-42`). C++ step calls the lambda with hardcoded `"posts"` (`posts execute.cppm:32`); lambda returns `{200, encode(SearchPostsResponse)}` with one empty `RecordDto` (`:21`), and `if(r.first!=0)` at `:33` returns immediately. Effective body is `{"posts":[{"id":"1","name":"Ghost Record","slug":"ghost-record"}]}` — no `data`, no `meta.pagination`, wrong fields vs Ghost's id/uuid/url/title/slug/status/published_at/visibility (`search-index.js:28-37`).
- DB call missing: `ctx.store().json_for("search-index", {})` at `posts execute.cppm:34` is unreachable because of the early return at `:33`. No filter/limit/order/columns applied.
- Input: Ghost hardcodes filter `type:post+status:[draft,published,scheduled,sent]`, `limit:'10000'`, `order:'updated_at DESC'` (`search-index.js:25-27`); C++ declares `Input=IRequest&` (`execute.cppm:12`) but never reads `in` — no query parsing.
- Cache: Ghost sets `headers.cacheInvalidate=false` (`search-index.js:16-18`); C++ has no cache behavior (grep for `cache|Cache|namespace|json_for` over `discovery/search_index/` found only `json_for` at `:34`).
- Auth: Ghost `mw.authAdminApi` (`routes.js:610`) + `permissions.docName='posts', method='browse'` (`search-index.js:19-22`); C++ checks only `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (`posts handler.cppm:18-22`), no docName/method permission model.

#### Fix

- **File + symbol:** `include/blog/handlers/discovery/search_index/search_index_posts_get/workflow/step/execute.cppm` → `blog::handlers::search_index::search_index_posts_get::workflow::ExecuteStep::invoke` (`:14`); router `.../search_index/routes.cppm:19/44-53`; handler `.../search_index_posts_get/handler.cppm:10-28`; DTO `include/blog/dto/more_resources.cppm:78-84/140`.
- **Replacement call/type:** delete the hardcoded lambda + `RecordDto`/`SearchPostsResponse` block (`:16-32`) and the `if(r.first!=0) return` early exit (`:33`). Replace unreachable `json_for("search-index",{})` (`:34`) with `ctx.store().json_for("posts", params)` where `params : std::map<std::string,std::string>` carries Ghost's hardcoded `filter`, `limit`, `fields`. Emit `{"posts":<rows>,"meta":{"pagination":{...}}}` directly (same shape as `admin/posts/browse` execute.cppm:30-31); drop the `SearchPostsResponse` DTO usage. `Output` stays `std::string`.
- **Checks 1–8 closed:** (1) path/verb already agree — no router path change. (2) op mapping already correct. (3) input = Ghost hardcoded options as `params` (`search-index.js:25-38`); no request body/query parse needed. (4) output = real post array + `meta.pagination` with Ghost's fields (`id,uuid,url,title,slug,status,published_at,visibility`). (5) DB = `json_for("posts", params)` with filter/limit applied — was unreachable. (6) no C++ cache behavior; matches Ghost `cacheInvalidate:false` (no invalidation, no `core::cache::Cache` key/namespace). (7) add `.use(blog::handlers::middleware::require_auth)` to the `/search-index` Router (`routes.cppm:20`) to match `mw.authAdminApi` (`routes.js:610`) and remove the redundant inline token check (`handler.cppm:18-22`); `docName/method` permission has no C++ facility in the allowed set — token-auth is the documented ceiling. (8) verdict becomes MATCH once real records + meta are returned.
- **Snippet (ambiguous part — `order`):**
  ```cpp
  auto& ctx = *s_ctx;
  std::map<std::string, std::string> params{
    {"filter", "type:post+status:[draft,published,scheduled,sent]"},
    {"limit",  "10000"},
    {"fields", "id,uuid,url,title,slug,status,published_at,visibility"}};
  const auto rows = ctx.store().json_for("posts", params);   // filter/limit/fields honored
  // json_for ignores "order"; to honor updated_at DESC, decode rows with
  // core_serde::JsonCodec{}.decode<serde::Value>, sort on "updated_at", re-encode.
  return core::workflow::StepResponse<Output>{
    "{\"posts\":" + rows +
    ",\"meta\":{\"pagination\":{\"limit\":10000,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":" +
    std::to_string(ctx.store().total("posts")) + "}}}"};
  ```

### GET /search-index/pages — MISMATCH

- Route identity agrees (GET both): Ghost `routes.js:611` vs C++ `routes.cppm:33-42` (`Route{"/pages"}.get(...)`).
- Data shape materially different. Ghost: real page records + meta via `postsService.browsePosts` (`search-index.js:52-71`). C++ lambda called with `"pages"` (`pages execute.cppm:32`), returns one placeholder `RecordDto` under `pages` (`:18`), early return at `:33`; no meta/pagination, wrong fields vs Ghost's page columns (`search-index.js:57-66`).
- DB call missing: `json_for("search-index",{})` unreachable (`pages execute.cppm:34-35`).
- Input: Ghost filter `type:page+status:[draft,published,scheduled]`, `limit:'10000'`, `order:'updated_at DESC'` (`search-index.js:54-56`); C++ parses nothing (`execute.cppm:12`).
- Cache: Ghost `cacheInvalidate:false` (`search-index.js:45-47`); C++ none.
- Auth: Ghost `mw.authAdminApi` + `posts/browse` (`routes.js:611`, `search-index.js:48-51`); C++ token-nonempty only (`pages handler.cppm:18-22`).

#### Fix

- **File + symbol:** `include/blog/handlers/discovery/search_index/search_index_pages_get/workflow/step/execute.cppm` → `blog::handlers::search_index::search_index_pages_get::workflow::ExecuteStep::invoke` (`:14`); router `.../search_index/routes.cppm:19/33-42`; handler `.../search_index_pages_get/handler.cppm:10-28`; DTO `include/blog/dto/more_resources.cppm:70-76/139`.
- **Replacement call/type:** same as posts: delete lambda + `SearchPagesResponse`/`RecordDto` (`:16-32`) and `if(r.first!=0) return` (`:33`); replace `json_for("search-index",{})` (`:34`) with `ctx.store().json_for("pages", params)` and emit `{"pages":<rows>,"meta":{"pagination":{...}}}` directly. `Output` stays `std::string`.
- **Checks 1–8 closed:** (1) GET `/search-index/pages` agrees (`routes.js:611` vs `routes.cppm:33-42`). (2) `fetchPages` → `search_index_search_index_pages_get` correct. (3) input = Ghost hardcoded options (`search-index.js:54-67`): `filter="type:page+status:[draft,published,scheduled]"`, `limit=10000`, `fields=id,uuid,url,title,slug,status,published_at,visibility`; no parse. (4) output = real pages + pagination meta. (5) DB = `json_for("pages", params)`. (6) none both sides (`cacheInvalidate:false`). (7) add `.use(require_auth)` on the Router (`routes.cppm:20`), drop inline token check (`pages handler.cppm:18-22`); `posts/browse` permission unrepresentable — documented ceiling. (8) MATCH after fix.
- **Snippet (ambiguous part — `order`):**
  ```cpp
  auto& ctx = *s_ctx;
  std::map<std::string, std::string> params{
    {"filter", "type:page+status:[draft,published,scheduled]"},
    {"limit",  "10000"},
    {"fields", "id,uuid,url,title,slug,status,published_at,visibility"}};
  const auto rows = ctx.store().json_for("pages", params);
  // json_for ignores "order"; sort decoded rows by updated_at DESC via core_serde if strict parity needed.
  return core::workflow::StepResponse<Output>{
    "{\"pages\":" + rows +
    ",\"meta\":{\"pagination\":{\"limit\":10000,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":" +
    std::to_string(ctx.store().total("pages")) + "}}}"};
  ```

### GET /search-index/tags — MISMATCH

- Route identity agrees (GET both): Ghost `routes.js:612` vs C++ `routes.cppm:55-64` (`Route{"/tags"}.get(...)`).
- Data shape materially different. Ghost: `models.Tag.findPage` real tags + meta (`search-index.js:81-89`). C++ lambda called with `"tags"` (`tags execute.cppm:32`), one placeholder under `tags` (`:24`), early return at `:33`; no meta/pagination, wrong fields vs Ghost's `['id','slug','name','url']` (`search-index.js:85`).
- DB call missing: `json_for("search-index",{})` unreachable (`tags execute.cppm:34-35`). Ghost resource string `Tag.findPage` (`search-index.js:88`); C++ has no effective resource string.
- Input: Ghost `limit:'10000'`, `order:'updated_at DESC'` (`search-index.js:83-84`); C++ parses nothing (`execute.cppm:12`).
- Cache: Ghost `cacheInvalidate:false` (`search-index.js:74-76`); C++ none.
- Auth: Ghost `mw.authAdminApi` + `tags/browse` (`routes.js:612`, `search-index.js:77-80`); C++ token-nonempty only (`tags handler.cppm:18-22`).

#### Fix

- **File + symbol:** `include/blog/handlers/discovery/search_index/search_index_tags_get/workflow/step/execute.cppm` → `blog::handlers::search_index::search_index_tags_get::workflow::ExecuteStep::invoke` (`:14`); router `.../search_index/routes.cppm:19/55-64`; handler `.../search_index_tags_get/handler.cppm:10-28`; DTO `include/blog/dto/more_resources.cppm:86-92/141`.
- **Replacement call/type:** delete lambda + `SearchTagsResponse`/`RecordDto` (`:16-32`) and `if(r.first!=0) return` (`:33`); replace `json_for("search-index",{})` (`:34`) with `ctx.store().json_for("tags", params)` and emit `{"tags":<rows>,"meta":{"pagination":{...}}}` directly. `Output` stays `std::string`.
- **Checks 1–8 closed:** (1) GET `/search-index/tags` agrees (`routes.js:612` vs `routes.cppm:55-64`). (2) `fetchTags` → `search_index_search_index_tags_get` correct. (3) input = Ghost hardcoded options (`search-index.js:82-86`): `limit=10000`, `fields=id,slug,name,url`; no `filter`. (4) output = real tags (`models.Tag.findPage`) + meta. (5) DB = `json_for("tags", params)` (maps to `m_db.all<model::Tag>()`, context.cppm:1130). (6) none both sides (`cacheInvalidate:false`). (7) add `.use(require_auth)` on Router (`routes.cppm:20`), drop inline token check (`tags handler.cppm:18-22`); `tags/browse` permission unrepresentable — documented ceiling. (8) MATCH after fix.
- **Snippet (ambiguous part — `order`):**
  ```cpp
  auto& ctx = *s_ctx;
  std::map<std::string, std::string> params{
    {"limit",  "10000"},
    {"fields", "id,slug,name,url"}};
  const auto rows = ctx.store().json_for("tags", params);
  // json_for ignores "order"; sort decoded rows by updated_at DESC via core_serde if strict parity needed.
  return core::workflow::StepResponse<Output>{
    "{\"tags\":" + rows +
    ",\"meta\":{\"pagination\":{\"limit\":10000,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":" +
    std::to_string(ctx.store().total("tags")) + "}}}"};
  ```

### GET /search-index/users — MISMATCH

- Route identity agrees (GET both): Ghost `routes.js:613` vs C++ `routes.cppm:66-75` (`Route{"/users"}.get(...)`).
- Data shape materially different. Ghost: `models.User.findPage` real users + meta (`search-index.js:99-107`). C++ lambda called with `"users"` (`users execute.cppm:32`), one placeholder under `users` (`:31`), early return at `:33`; no meta/pagination, wrong fields vs Ghost's author columns `['id','slug','url','name','profile_image']` (`search-index.js:103`).
- DB call missing: `json_for("search-index",{})` unreachable (`users execute.cppm:34-35`). Ghost resource string `User.findPage` (`search-index.js:106`); C++ has no effective resource string.
- Input: Ghost `limit:'10000'`, `order:'updated_at DESC'` (`search-index.js:101-102`); C++ parses nothing (`execute.cppm:12`).
- Cache: Ghost `cacheInvalidate:false` (`search-index.js:92-94`); C++ none.
- Auth: Ghost `mw.authAdminApi` + `users/browse` (`routes.js:613`, `search-index.js:95-98`); C++ token-nonempty only (`users handler.cppm:18-22`).

#### Fix

- **File + symbol:** `include/blog/handlers/discovery/search_index/search_index_users_get/workflow/step/execute.cppm` → `blog::handlers::search_index::search_index_users_get::workflow::ExecuteStep::invoke` (`:14`); router `.../search_index/routes.cppm:19/66-75`; handler `.../search_index_users_get/handler.cppm:10-28`; DTO `include/blog/dto/more_resources.cppm:102-108/142`.
- **Replacement call/type:** delete lambda + `SearchUsersResponse`/`RecordDto` (`:16-32`) and `if(r.first!=0) return` (`:33`); replace `json_for("search-index",{})` (`:34`) with `ctx.store().json_for("users", params)` (maps to `m_db.all<model::User>()`, context.cppm:1135) and emit `{"users":<rows>,"meta":{"pagination":{...}}}` directly. `Output` stays `std::string`.
- **Checks 1–8 closed:** (1) GET `/search-index/users` agrees (`routes.js:613` vs `routes.cppm:66-75`). (2) `fetchUsers` → `search_index_search_index_users_get` correct. (3) input = Ghost hardcoded options (`search-index.js:100-104`): `limit=10000`, `fields=id,slug,url,name,profile_image`; no `filter`. (4) output = real users (`models.User.findPage`) + meta. (5) DB = `json_for("users", params)`. (6) none both sides (`cacheInvalidate:false`). (7) add `.use(require_auth)` on Router (`routes.cppm:20`), drop inline token check (`users handler.cppm:18-22`); `users/browse` permission unrepresentable — documented ceiling. (8) MATCH after fix.
- **Snippet (ambiguous part — `order`):**
  ```cpp
  auto& ctx = *s_ctx;
  std::map<std::string, std::string> params{
    {"limit",  "10000"},
    {"fields", "id,slug,url,name,profile_image"}};
  const auto rows = ctx.store().json_for("users", params);
  // json_for ignores "order"; sort decoded rows by updated_at DESC via core_serde if strict parity needed.
  return core::workflow::StepResponse<Output>{
    "{\"users\":" + rows +
    ",\"meta\":{\"pagination\":{\"limit\":10000,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":" +
    std::to_string(ctx.store().total("users")) + "}}}"};
  ```

## Missing in C++

None. All four Ghost controller routes (fetchPosts, fetchPages, fetchTags, fetchUsers — `search-index.js:15`, `:44`, `:73`, `:91`; wired at `routes.js:610-613`) have exact path+verb C++ counterparts in `discovery/search_index/routes.cppm:44-53`, `:33-42`, `:55-64`, `:66-75`.

## Extra in C++

- **GET /search-index/authors** — C++ `Route{"/authors"}.get(...)` at `include/blog/handlers/discovery/search_index/routes.cppm:22-31`, bound to `search_index_search_index_authors_get` (`routes.cppm:28`; handler `search_index_authors_get/handler.cppm:17-28`; step `search_index_authors_get/workflow/step/execute.cppm:16-35` with hardcoded kind `"authors"` at `:32` and `SearchAuthorsResponse` DTO at `more_resources.cppm:94-100/144`). No Ghost controller route: `search-index.js` exports only `fetchPosts`/`fetchPages`/`fetchTags`/`fetchUsers` (`:15`,`:44`,`:73`,`:91`) and `routes.js:609-613` lists only `/search-index/posts`, `/pages`, `/tags`, `/users`. Confirmed via grep `search-index` over admin `routes.js` (only lines 610-613) and grep `fetchPosts|fetchPages|fetchTags|fetchUsers` over the controller (only the four exported ops).
  - **Fix:** remove the orphaned C++ surface, no Ghost counterpart exists. Exact symbols: delete `Route{"/authors"}.get(...)` in `include/blog/handlers/discovery/search_index/routes.cppm:22-31` (keep `Router{router,"/search-index"}` at `:19` and the four real routes); delete `search_index_search_index_authors_get` flow/step/handler files (`.../search_index_authors_get/**`), its `import`/accessor/member in `include/blog/handlers/all.cppm:26,372,621`, and `SearchAuthorsResponse` + `ARRAY_SERDE(SearchAuthorsResponse, "search_authors", "authors", authors)` in `include/blog/dto/more_resources.cppm:94-100/144`. No replacement call/type — route removed. Checks 1–8 closed: (1) no route identity remains; (2) no op mapping; (3)–(6) no input/output/DB/cache; (7) no auth needs; (8) EXTRA resolved (grep from the Extra section above already proves absence on Ghost side). If the route is intentionally kept as an alias, document it as a deliberate non-parity extra instead — but no Ghost `fetchAuthors` exists, so removal is the parity-preserving fix.
