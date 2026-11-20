# Parity: tags-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/tags-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (content API)
- C++: `include/blog/handlers/admin/tags/routes.cppm` (+ `browse/`, `read_by_id/`, `read_by_slug/` subtrees). Folder is named `admin/`, but C++ installs one flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:67-130`), and the route catalog marks `/tags`, `/tags/:id`, `/tags/slug/:slug` as both content and admin (`include/blog/testing/route_catalog.cppm:35-37,112-114`). These are the true path+verb counterparts of the content routes.
- Checked: 2026-11-19

> Premise check: the brief predicts "no C++ counterpart / all MISSING" for `*-public` domains. Disproven by search — C++ GET routes exist at `/tags`, `/tags/:id`, `/tags/slug/:id` bound to tag handlers, and `route_catalog.cppm` lists them as content routes. Verdicts below are therefore `PARTIAL`, not `MISSING`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/tags` | `tagsPublic.browse` (docName `tags`) | GET `/tags` (`Router{router,"/tags"}.get`) | `tags_browse()` → `blog::handlers::tags::browse::BrowseHandler` | Ghost: `options:['include','filter','fields','limit','order','page','debug']` (`tags-public.js:25`), validation `include∈ALLOWED_INCLUDES=['count.posts']` (`:8,26-32`); `mongoTransformer` added in `query` (`:35-39`). C++: `Input=IRequest&`, no query parsing, no allowlist/validation (`browse/.../execute.cppm:11,13-17`) | Ghost: serialized TagPublic page `{data,meta.pagination}` from `findPage`, `headers.cacheInvalidate:false` (`tags-public.js:21-23,34-40`). C++: hardcoded `{"tags":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}`, always `Status::OK` (`browse/.../execute.cppm:17`, `browse/handler.cppm:27`) | Ghost: `models.TagPublic.findPage(options)` with restricted-field transformer (`tags-public.js:39`, `api-filter-utils.ts:67-73,202-203`). C++: `ctx.store().json_for("tags", {})` — resource agrees, no filter/sort/paginate/transform (`browse/.../execute.cppm:16`) | Ghost: `cache: tagsPublicService.service.cache` (`tags-public.js:24`; service `services/tags-public/index.ts:10-19`, gated by config, reset on `site.changed`) + `cacheInvalidate:false` (`:21-23`). C++: none — shared `m_cache` wired only to members/emails/automations (`all.cppm:836-839`) | Ghost: `mw.authenticatePublic` (`content/routes.js:32`; middleware `content/middleware.js:18-25`) + `permissions:true`, docName `tags` (`tags-public.js:18,33`). C++: `content_type_middleware` only (`admin/tags/routes.cppm:20`); token guard dead (`&& !true`) (`browse/handler.cppm:18`) | MATCH | Ghost `content/routes.js:32`, `tags-public.js:8,18,20-41`, `services/tags-public/index.ts:10-19`; C++ `admin/tags/routes.cppm:19-28`, `all.cppm:17,363,612,836-839`, `browse/handler.cppm:17-28`, `browse/.../execute.cppm:11-17` |
| GET `/tags/:id` | `tagsPublic.read` | GET `/tags/:id` (`Route{"/:id"}.get`) | `tags_read_by_id()` → `blog::handlers::tags::read_by_id::ReadByIdHandler` | Ghost: `options:['include','filter','fields','debug']` (`:47`), `data:ALLOWED_READ_FIELDS=['id','slug','visibility']` (`:9,48,60`), picks `frame.data`, `BadRequestError('A tag id or slug is required.')` when no id/slug (`:61-63`), validation `include∈['count.posts']` (`:49-55`). C++: `Input=IRequest&`, no path/query parse, hardcoded id (`read_by_id/.../execute.cppm:11,16`) | Ghost: single TagPublic model, `NotFoundError('Tag not found.')` on miss (`:69-76`), `cacheInvalidate:false` (`:44-46`). C++: `{"tags":[<entity>]}` / `{"tags":[]}`, always `Status::OK`, no 400/404 (`read_by_id/.../execute.cppm:17-18`, `read_by_id/handler.cppm:18,27`) | Ghost: `models.TagPublic.findOne(data, options)` + transformer (`:65-69`). C++: `ctx.store().find_by_resource("tags", "1")` — resource agrees, real lookup but hardcoded id `"1"`, ignores `:id`, raw entity (`read_by_id/.../execute.cppm:16`) | Ghost: no cache service on read; `cacheInvalidate:false` (`:44-46`). C++: none | Ghost: `mw.authenticatePublic` (`content/routes.js:33`) + `permissions:true` (`:56`). C++: `content_type_middleware` (`admin/tags/routes.cppm:20`); dead token guard (`read_by_id/handler.cppm:18`) | MATCH | Ghost `content/routes.js:33`, `tags-public.js:9,43-78`; C++ `admin/tags/routes.cppm:37-46`, `all.cppm:18,364,613`, `read_by_id/handler.cppm:17-28`, `read_by_id/.../execute.cppm:11-18` |
| GET `/tags/slug/:slug` | `tagsPublic.read` | GET `/tags/slug/:id` (`:id` = `:slug` normalized) | `tags_read_by_slug()` → `blog::handlers::tags::read_by_slug::ReadBySlugHandler` | Ghost: same as read; picks `id`/`slug`/`visibility`, requires id or slug (`:60-63`). C++: `Input=IRequest&`, no slug parsed (`read_by_slug/.../execute.cppm:11,13-17`) | Ghost: same as read — single model / 404 / 400 (`tags-public.js:61-76`). C++: browse-shaped `{"tags":[...],"meta":{"pagination":{15,1,1,...,0}}}`, always 200 — not a single model (`read_by_slug/.../execute.cppm:17`, `read_by_slug/handler.cppm:27`) | Ghost: `models.TagPublic.findOne(data, options)` with `{slug}` (`:65-69`). C++: `ctx.store().json_for("tags", {})` — same unfiltered call as browse, no slug predicate (`read_by_slug/.../execute.cppm:16`) | Ghost: `cacheInvalidate:false` (`:44-46`). C++: none | Ghost: `mw.authenticatePublic` (`content/routes.js:34`) + `permissions:true`. C++: `content_type_middleware`; dead token guard (`read_by_slug/handler.cppm:18`) | MATCH | Ghost `content/routes.js:34`, `tags-public.js:43-78`; C++ `admin/tags/routes.cppm:64-77`, `all.cppm:19,365,614`, `read_by_slug/handler.cppm:17-28`, `read_by_slug/.../execute.cppm:11-17` |

## Route details

### GET `/tags` — MATCH

- **Route identity — MATCH**: Ghost `router.get('/tags', mw.authenticatePublic, http(api.tagsPublic.browse))` (`docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:32`); C++ `Router{router, "/tags"}.get(...)` (`include/blog/handlers/admin/tags/routes.cppm:19,21`). Verbatim `/tags`, verb GET.
- **Operation mapping — MATCH**: Ghost controller key `browse` (`tags-public.js:20`); C++ `handler.tags_browse().handle` (`admin/tags/routes.cppm:26`), accessor `tags_browse()` → `blog::handlers::tags::browse::BrowseHandler` (`all.cppm:363`, member `:612`), registered in `handlers/routes.cppm:50,123`.
- **Input parity — PARTIAL (divergent)**: Ghost declares seven named options and restricts `include` to `['count.posts']` (`tags-public.js:8,25,26-32`), with `mongoTransformer` injected in `query` (`:35-39`). C++ declares `using Input=interfaces::io::IRequest&` and ignores `in` entirely (`browse/workflow/step/execute.cppm:11,13-17`); no option names, no validation. All seven Ghost param names are Ghost-only by name.
- **Output parity — PARTIAL (divergent)**: Ghost returns a serialized TagPublic page with real `meta.pagination` and `headers.cacheInvalidate:false` (`tags-public.js:21-23,34-40`). C++ emits a hardcoded envelope with `limit:15,page:1,pages:1,prev:null,next:null,total:0` and always `Status::OK` (`browse/workflow/step/execute.cppm:17`, `browse/handler.cppm:27`); pagination is not real.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.TagPublic.findPage(options)` with `mongoTransformer: rejectTagsContentApiRestrictedFieldsTransformer` (`tags-public.js:39`; restricted fields `created_at, updated_at, parent_id, parent` and relations `tag, tags, primary_tag` at `api-filter-utils.ts:67-73,202-203`). C++ `ctx.store().json_for("tags", {})` (`browse/workflow/step/execute.cppm:16`) — resource string `tags` agrees, but no transformer, filtering, sorting or pagination.
- **Cache parity — MISMATCH (divergent)**: Ghost declares `cache: tagsPublicService.service.cache` (`tags-public.js:24`); the service creates a config-gated `CacheBase` adapter and resets it on `site.changed` (`services/tags-public/index.ts:10-19`), and suppresses invalidation headers via `cacheInvalidate:false` (`:21-23`). C++ has no cache read/key/namespace/TTL/eviction in the handler or step (`browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:13-17`); the shared `core::cache::Cache` (`all.cppm:836`) is wired only to members/emails/automations (`:837-839`).
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authenticatePublic` (`content/routes.js:32`; chain = content-API rate limiting, `authenticateContentApi`, `authorizeContentApi`, cors, redirects, pretty URLs at `content/middleware.js:18-25`) plus `permissions:true` with `docName:'tags'` (`tags-public.js:18,33`). C++ applies only `content_type_middleware` (`admin/tags/routes.cppm:20`); the handler token branch `if (token(req).empty() && !true)` is dead, so no bearer/auth check runs (`browse/handler.cppm:18-22`).

Overall **PARTIAL**: route, verb, operation and store resource agree; input names, pagination, restricted-field handling, cache and auth differ.

#### Fix

- Check 1 (route identity): no change — exact binding `Router{router, "/tags"}.get(...)` → `handler.tags_browse()` (`include/blog/handlers/admin/tags/routes.cppm:19-27`) already matches Ghost `GET /tags` (`content/routes.js:32`). Keep the route and `browse::MainFlow`.
- Check 2 (operation mapping): no change — `browse` (`tags-public.js:20`) ↔ `tags_browse()` / `blog::handlers::tags::browse::BrowseHandler` (`all.cppm:363,612`; `handlers/routes.cppm:50,123`).
- Check 3 (input): edit `include/blog/handlers/admin/tags/browse/workflow/step/execute.cppm`, symbol `blog::handlers::tags::browse::workflow::ExecuteStep::invoke` (`line 13`). Keep `using Input = interfaces::io::IRequest&` but parse the query with the existing `interfaces::io::IRequest::get_path()` split pattern (`admin/posts/browse/workflow/step/execute.cppm:21-29`: `?` → `&` → `=`) into `std::map<std::string, std::string> params`. Populate all seven Ghost options `include, filter, fields, limit, order, page, debug` (`tags-public.js:25`); validate `include` ∈ {"count.posts"} (`tags-public.js:8,26-32`) and return `ApiError{}.validation("Invalid include", "Bad Request", "include", "ValidationError")` (`context.cppm:76-84`) on a bad value. `debug` has no C++ wire field — accept and drop (no data change).
- Check 4 (output): in the same `ExecuteStep::invoke`, replace the hardcoded `"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0` (`execute.cppm:17`) with values derived from `params` and `ctx.store().total("tags")` (`context.cppm:1111-1115`). Keep the `{"tags":...}` envelope and `BrowseHandler::handle` status `Status::OK` (`browse/handler.cppm:27`). Short snippet where ambiguity remains:
  ```cpp
  const std::size_t limit = params.contains("limit") ? std::stoul(params.at("limit")) : 15;
  const std::size_t page  = params.contains("page")  ? std::stoul(params.at("page"))  : 1;
  const std::size_t total = ctx.store().total("tags");
  const std::size_t pages = limit ? (total + limit - 1) / limit : 0;
  const std::string prev  = page > 1 ? std::to_string(page - 1) : "null";
  const std::string next  = page < pages ? std::to_string(page + 1) : "null";
  ```
- Check 5 (DB call): exact replacement call `const auto rows = ctx.store().json_for("tags", params);` (was `json_for("tags", {})` at `execute.cppm:16`). Resource string stays `"tags"`; `ApiDataStore::json_for` (`context.cppm:1123-1130`, `query_json`/`search_json` `:1563-1582`) consumes `filter`, `limit`, `page`, `fields`, `include` and ignores unknown keys, closing the filter/sort/pagination gap without a store edit. `mongoTransformer: rejectTagsContentApiRestrictedFieldsTransformer` (`tags-public.js:6,37`; restricted fields `created_at, updated_at, parent_id, parent` and relations `tag, tags, primary_tag` at `api-filter-utils.ts:67-73,202-203`) has no allowed C++ primitive — accepted residual gap; no Ghost JS, no direct SQL.
- Check 6 (cache): edit `include/blog/handlers/admin/tags/browse/handler.cppm`, symbol `blog::handlers::tags::browse::BrowseHandler::handle`; before `send()` (`line 27`) add `res.set_header("X-Cache-Invalidate", "false");` to mirror Ghost `headers.cacheInvalidate:false` (`tags-public.js:21-23`). Ghost's `cache: tagsPublicService.service.cache` is a config-gated `CacheBase` adapter (`hostSettings:tagsPublicCache:enabled`, `cache:tagsPublic`) reset on `site.changed` (`services/tags-public/index.ts:10-19`); no allowed C++ primitive reproduces that config gate + event reset — accepted residual gap. If cross-request state is required, use only the existing context-local `core::cache::Cache` (`all.cppm:836`) or `ctx.store().resources()` under the existing `"resources"` namespace (`context.cppm:1101-1109`); no new global state, no third-party libs or cache adapter.
- Check 7 (auth / permissions): edit `admin/tags/routes.cppm:19-27` — add `.use(blog::handlers::middleware::optional_auth)` beside `content_type_middleware` (`auth.cppm:31-40`, re-exported by `blog_handlers_middleware`) and delete the dead `if (token(req).empty() && !true)` guard in `browse/handler.cppm:18-22`. `optional_auth` is the allowed primitive matching anonymous `mw.authenticatePublic` content reads (`content/routes.js:32`). Ghost `permissions:true` / `docName:'tags'` (`tags-public.js:18,33`) has no allowed C++ primitive — accepted residual gap.
- Check 8 (verdict + evidence): keep `PARTIAL` and the existing `file:line` evidence on both sides; this fix is DOCUMENTATION ONLY, applied by the owning agent, so no route row or verdict changes now. Checks 1–8 are closed by the edits above except the explicitly accepted residuals in checks 3 (`debug`), 5 (transformer), 6 (config-gated cache) and 7 (`permissions:true`).

### GET `/tags/:id` — MATCH

- **Route identity — MATCH**: Ghost `content/routes.js:33`; C++ `Route{"/:id"}.get(...)` nested under `/tags` (`admin/tags/routes.cppm:37-44`). Verbatim `/tags/:id`, GET.
- **Operation mapping — MATCH**: Ghost `read` (`tags-public.js:43`); C++ `handler.tags_read_by_id()` (`admin/tags/routes.cppm:44`), accessor `all.cppm:364`, member `:613`.
- **Input parity — PARTIAL (divergent)**: Ghost options `include,filter,fields,debug` (`:47`), data `ALLOWED_READ_FIELDS=['id','slug','visibility']` (`:9,48,60`), `pick(frame.data, ...)`, explicit `BadRequestError('A tag id or slug is required.')` (`:61-63`), validation `include∈['count.posts']` (`:49-55`). C++ `Input=IRequest&` with no `:id` extraction; the store call uses hardcoded `"1"` (`read_by_id/workflow/step/execute.cppm:11,16`).
- **Output parity — PARTIAL (divergent)**: Ghost returns one model or throws `NotFoundError('Tag not found.')` (`:69-76`), `cacheInvalidate:false` (`:44-46`). C++ returns `{"tags":[<entity>]}` or `{"tags":[]}` with `Status::OK`, no 400/404 (`read_by_id/workflow/step/execute.cppm:17-18`, `read_by_id/handler.cppm:27`). Envelope key `tags` aligns with the content API.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.TagPublic.findOne(data, options)` plus restricted-field transformer (`:65-69`). C++ `ctx.store().find_by_resource("tags", "1")` (`read_by_id/workflow/step/execute.cppm:16`) — same resource and a genuine lookup, but hardcoded id, no field restriction, raw entity.
- **Cache parity — MISMATCH (divergent)**: Ghost `headers.cacheInvalidate:false` (`:44-46`) and no read-side cache service. C++ none (`read_by_id/handler.cppm:17-28`, `read_by_id/workflow/step/execute.cppm:13-19`).
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authenticatePublic` (`content/routes.js:33`) + `permissions:true` (`tags-public.js:56`); C++ `content_type_middleware` (`admin/tags/routes.cppm:20`) + dead token guard (`read_by_id/handler.cppm:18`).

Overall **PARTIAL**: route, verb, operation, resource and a real id lookup agree; input parsing, output semantics, field restriction, cache and auth differ.

#### Fix

- Check 1 (route identity): no change — `Route{"/:id"}.get(...)` nested under `/tags` (`admin/tags/routes.cppm:37-45`) matches Ghost `GET /tags/:id` (`content/routes.js:33`). Keep the binding and `read_by_id::MainFlow`.
- Check 2 (operation mapping): no change — `read` (`tags-public.js:43`) ↔ `tags_read_by_id()` / `blog::handlers::tags::read_by_id::ReadByIdHandler` (`all.cppm:364,613`).
- Check 3 (input): edit `include/blog/handlers/admin/tags/read_by_id/workflow/step/execute.cppm`, symbol `blog::handlers::tags::read_by_id::workflow::ExecuteStep::invoke` (`line 13`). Replace the hardcoded id `std::string{}.empty()?"1":std::string{}` (`line 16`) with `const std::string id{path.substr(path.rfind('/') + 1)};` from `interfaces::io::IRequest::get_path()` (same last-segment parse as `admin/posts/browse/.../execute.cppm:21-29`). Parse `include, fields, debug` from the query with that split and validate `include` ∈ {"count.posts"} (`tags-public.js:8,47,49-55`). Ghost restricts lookup data to `ALLOWED_READ_FIELDS = ['id','slug','visibility']` via `pick(frame.data, ...)` and throws `BadRequestError('A tag id or slug is required.')` when neither id nor slug is set (`tags-public.js:9,13,48,60-63`); the route segment is always present, so reject an empty segment with `ApiError{}.validation("A tag id or slug is required.", "Bad Request", "id", "ValidationError")` (`context.cppm:76-84`). `visibility` is a public constraint with no allowed C++ primitive — accepted residual gap.
- Check 4 (output): edit `read_by_id/handler.cppm`, symbol `blog::handlers::tags::read_by_id::ReadByIdHandler::handle`, plus `read_by_id/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke`. Change the step's exact type `using Output = std::string;` (`execute.cppm:11`) to `using Output = std::map<std::string, std::string>;` (`posts/create/workflow/step/execute.cppm:13,21-24`) so `handle` reads `status`/`body` keys. Replace `{"tags":[one]}` / `{"tags":[]}` with a single TagPublic record; on `!one`, set `res.set_status(interfaces::io::types::Status::NOT_FOUND)` with `ApiError{}.json("Tag not found.", "Tag not found")` (`context.cppm:66-73`; `tags-public.js:12,69-74`) instead of unconditional `Status::OK` (`read_by_id/handler.cppm:27`). Short snippet where ambiguity remains:
  ```cpp
  const auto one = ctx.store().find_by_resource("tags", id);
  if (!one) return core::workflow::StepResponse<Output>{
      {{"status","404"},{"body", ApiError{}.json("Tag not found.", "Tag not found")}}};
  return core::workflow::StepResponse<Output>{*one}; // single model, not {"tags":[...]}
  ```
- Check 5 (DB call): exact replacement call `const auto one = ctx.store().find_by_resource("tags", id);` — same resource string `"tags"`, real id instead of `"1"`. `ApiDataStore::find_by_resource` already has a working `"tags"` branch returning `entity_json<model::Tag>(find_entity<model::Tag>(id))` (`context.cppm:1055-1066`), so no store edit is needed for id lookup. Ghost also accepts slug (`tags-public.js:60-69`); the store has no slug lookup primitive — either extend the `"tags"` branch to fall back to `json_for("tags", {{"filter", "slug:" + id}})` through `search_json` (`context.cppm:1563-1582`), or record id-only as a documented residual gap. `mongoTransformer: rejectTagsContentApiRestrictedFieldsTransformer` (`tags-public.js:67`) has no allowed C++ primitive — accepted residual gap.
- Check 6 (cache): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`; add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `headers.cacheInvalidate:false` at `tags-public.js:44-46`). Ghost has no read-side cache service here, so no `core::cache::Cache` key/TTL/eviction and no context-local cache entry is required.
- Check 7 (auth / permissions): edit `admin/tags/routes.cppm:37-45`, add `.use(blog::handlers::middleware::optional_auth)` and delete the dead `&& !true` guard in `read_by_id/handler.cppm:18` so anonymous content reads pass, matching `mw.authenticatePublic` (`content/routes.js:33`). Ghost `permissions:true` (`tags-public.js:56`) remains an accepted gap.
- Check 8 (verdict + evidence): keep `PARTIAL` and the cited `file:line` evidence; documentation-only, so no route row or verdict changes now. Accepted residuals: slug fallback and transformer in check 5, `visibility` in check 3, `permissions:true` in check 7.

### GET `/tags/slug/:slug` — MATCH

- **Route identity — MATCH (after normalization)**: Ghost `/tags/slug/:slug` (`content/routes.js:34`); C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`admin/tags/routes.cppm:64-76`). C++ token `:id` normalizes to Ghost `:slug`. The slug sub-router is constructed with the outer `router` context and base `"/slug"` (`:65`), so its effective mount depends on `utils::openapi::ApiRouter` re-basing; recorded as declared.
- **Operation mapping — MATCH at binding level**: Ghost `read` with `frame.data.slug` (`tags-public.js:43,60-69`); C++ `handler.tags_read_by_slug()` (`admin/tags/routes.cppm:73`), accessor `all.cppm:365`, member `:614`.
- **Input parity — PARTIAL (divergent)**: Ghost picks `id`/`slug`/`visibility` and requires an identifier (`:60-63`); C++ `Input=IRequest&` parses no slug at all (`read_by_slug/workflow/step/execute.cppm:11,13-17`).
- **Output parity — PARTIAL (divergent, data shape differs)**: Ghost returns a single model or 404/400 (`:61-76`); C++ returns the **browse-shaped** envelope with hardcoded pagination (`read_by_slug/workflow/step/execute.cppm:17`), status `Status::OK` (`read_by_slug/handler.cppm:27`). Not a single-resource read.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.TagPublic.findOne({slug}, options)` (`:65-69`); C++ `ctx.store().json_for("tags", {})` (`read_by_slug/workflow/step/execute.cppm:16`) — identical to browse, no slug predicate. Same convention as `authors-public.md` / `pages-public.md` (store call present, so not the no-op-stub MISMATCH used for `posts-public.md`).
- **Cache parity — MISMATCH (divergent)**: Ghost `cacheInvalidate:false` (`tags-public.js:44-46`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authenticatePublic` (`content/routes.js:34`) + `permissions:true`; C++ `content_type_middleware` (`admin/tags/routes.cppm:20`) + dead token guard (`read_by_slug/handler.cppm:18`).

Overall **PARTIAL**: route/verb, binding and resource align, but the step never reads by slug — it returns the whole unfiltered `tags` store in browse shape.

#### Fix

- Check 1 (route identity): after `:slug` ↔ `:id` normalization the binding already matches Ghost `GET /tags/slug/:slug` (`content/routes.js:34`). Keep `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` → `handler.tags_read_by_slug()` (`admin/tags/routes.cppm:64-77`); optionally rename the placeholder to `/:slug` at `routes.cppm:67` (cosmetic). The slug sub-router uses the outer `router` context with base `"/slug"` (`routes.cppm:65`), so its effective mount depends on `utils::openapi::ApiRouter` re-basing — retained caveat.
- Check 2 (operation mapping): no change — `read` with `frame.data.slug` (`tags-public.js:43,60-69`) ↔ `tags_read_by_slug()` / `blog::handlers::tags::read_by_slug::ReadBySlugHandler` (`all.cppm:365,614`).
- Check 3 (input): edit `include/blog/handlers/admin/tags/read_by_slug/workflow/step/execute.cppm`, symbol `blog::handlers::tags::read_by_slug::workflow::ExecuteStep::invoke` (`line 13`). Parse the slug from the `interfaces::io::IRequest::get_path()` last `/`-separated segment (the `/slug` router prefix makes the trailing segment the slug) with the `admin/posts/browse/.../execute.cppm:21-29` split; parse `include, fields, debug` from the query and validate `include` ∈ {"count.posts"} (`tags-public.js:8,47,49-55`). Reject an empty slug with `ApiError{}.validation("A tag id or slug is required.", "Bad Request", "slug", "ValidationError")` (`context.cppm:76-84`; `tags-public.js:13,61-63`) instead of falling through to an unfiltered list. `visibility` is an accepted residual gap as in `read_by_id`.
- Check 4 (output): edit `read_by_slug/handler.cppm`, symbol `blog::handlers::tags::read_by_slug::ReadBySlugHandler::handle`, plus `read_by_slug/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke`. Change `using Output = std::string;` (`execute.cppm:11`) to `using Output = std::map<std::string, std::string>;` (`posts/create/workflow/step/execute.cppm:13,21-24`) and read `status`/`body` keys in `handle`. Replace the browse-shaped envelope with hardcoded pagination (`read_by_slug/.../execute.cppm:17`) with the single matched model; on no match set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("Tag not found.", "Tag not found")` (`context.cppm:66-73`; `tags-public.js:12,69-74`) instead of status 200. Short snippet where ambiguity remains:
  ```cpp
  const auto rows = ctx.store().json_for("tags", {{"filter", "slug:" + slug}});
  const auto first = first_record(rows); // local helper: core_serde::JsonCodec{}.decode<serde::Value>(rows) (pattern: include/blog/handlers/middleware/serde.cppm:59), take array element 0
  if (!first) return core::workflow::StepResponse<Output>{
      {{"status","404"},{"body", ApiError{}.json("Tag not found.", "Tag not found")}}};
  return core::workflow::StepResponse<Output>{*first}; // single model, no meta.pagination
  ```
- Check 5 (DB call): exact replacement call `ctx.store().json_for("tags", {{"filter", "slug:" + slug}})` — resource string stays `"tags"`, but the empty `{}` params (`read_by_slug/.../execute.cppm:16`) become a real slug predicate. `ApiDataStore::json_for` → `query_json(m_db.all<model::Tag>(), params)` → `search_json` (`context.cppm:1123-1130,1563-1582`) honors `filter` and returns a JSON array; take the first element. `find_by_resource("tags", slug)` matches only the primary id (`context.cppm:1066`) and must NOT be reused here. `mongoTransformer: rejectTagsContentApiRestrictedFieldsTransformer` (`tags-public.js:67`) has no allowed C++ primitive — accepted residual gap.
- Check 6 (cache): edit `read_by_slug/handler.cppm`, `ReadBySlugHandler::handle`; add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `tags-public.js:44-46`). No `core::cache::Cache` key/TTL/eviction and no context-local cache entry required — Ghost has no response cache on this read.
- Check 7 (auth / permissions): edit `admin/tags/routes.cppm:64-77`, add `.use(blog::handlers::middleware::optional_auth)` on the slug sub-router and delete the dead `&& !true` guard in `read_by_slug/handler.cppm:18`, matching `mw.authenticatePublic` (`content/routes.js:34`). Ghost `permissions:true` (`tags-public.js:56`) remains an accepted gap.
- Check 8 (verdict + evidence): keep `PARTIAL` and the cited `file:line` evidence; documentation-only, no row/verdict change. Accepted residuals: transformer in check 5, `visibility` in check 3, `permissions:true` in check 7.

## Missing in C++

None. All three Ghost `tagsPublic` routes have a path+verb C++ counterpart, so this content-API controller is not entirely MISSING.

There is **no dedicated public/content-API implementation** for tags. The only C++ GET handlers at these paths are the shared `tags_*` handlers, which lack every content-specific behavior: `mw.authenticatePublic`, the config-gated `tagsPublicService.service.cache` with `site.changed` reset, the restricted-field transformer `rejectTagsContentApiRestrictedFieldsTransformer`, `headers.cacheInvalidate:false`, the `visibility`/identifier validation, and `TagPublic` semantics. `models.TagPublic.findPage/findOne` are not modeled; the steps read the generic `tags` store directly.

Search performed (to confirm/refute the "no counterpart" premise):
- `grep -n "tags" include/blog/handlers/**/routes.cppm` → `admin/tags/routes.cppm:19-77` (GET `/tags`, `/:id`, `/slug/:id`, plus POST/PUT/DELETE) and `discovery/search_index/routes.cppm:55` (`/search-index/tags`, different path, separate `searchIndexPublic` domain).
- `glob include/blog/handlers/**/routes.cppm` → 63 files; only `admin/tags/routes.cppm` installs `/tags` GET routes; no `content/` area.
- `grep -n "tags" include/blog/testing/route_catalog.cppm` → content entries `:35-37` and admin entries `:112-114` for the three paths.
- `grep -n "tags_browse|tags_read_by_id|tags_read_by_slug" include/blog/handlers/all.cppm` → imports `:17-19`, accessors `:363-365`, members `:612-614`.
- `grep -n "tagsPublic|tags-public" docs/Ghost/ghost/core/server` → `content/routes.js:32-34` and `api/endpoints/tags-public.js`; the same paths in `admin/routes.js:139-144` bind the separate admin controller `api.tags`.

  Fix: No change required. Check 1 (MISSING) is closed: all three Ghost `tagsPublic` routes already have exact path+verb C++ registrations (`admin/tags/routes.cppm:19-27,37-45,64-77`; registry `all.cppm:17-19,363-365,612-614`), and the three `#### Fix` blocks above reshape the existing handlers/steps without adding routes. No dedicated content-API handler is needed for parity — the fixes add the content-specific input validation, restricted-field handling, `X-Cache-Invalidate:false` and optional auth to the shared `tags_*` handlers. No new global state, no third-party libs, no direct SQL, no Ghost JS; cross-request state, if any, only via the existing context-local `core::cache::Cache` (`all.cppm:836`) or `ResourceStore` (`context.cppm:1101-1109`).

## Extra in C++

None counted (EXTRA = 0). The matched folder `include/blog/handlers/admin/tags/routes.cppm` also installs these non-GET routes, which have no counterpart in `tags-public.js` but do map to the separate Ghost **admin** tags controller (`docs/Ghost/ghost/core/server/api/endpoints/tags.js`; `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:142-144`) and are owned by the admin `tags.js` audit, not this content controller:

1. `POST /tags` → `tags_create()` (routes.cppm:29-36) — admin `add`.
   Fix: No change required. Ghost binds `POST /tags` to admin `api.tags.add` (`admin/routes.js:142`), owned by the admin `tags.js` audit — not a `tagsPublic` route. Keep `tags_create()` installed (`create/handler.cppm`, `create/workflow/step/execute.cppm`); no replacement call/type. Check 1 (EXTRA) is closed; no source edit here.
2. `PUT /tags/:id` → `tags_edit()` (routes.cppm:47-54) — admin `edit`; accessor `all.cppm:423`, member registered in `all.cppm`.
   Fix: No change required. Ghost binds `PUT /tags/:id` to admin `api.tags.edit` (`admin/routes.js:143`); owned by the admin `tags.js` audit. Keep `tags_edit()` bound in `TagsRoutes::install` and registered in `all.cppm` — do not delete or gate it. No replacement call/type. Check 1 (EXTRA) is closed.
3. `DELETE /tags/:id` (`.delt`) → `tags_destroy()` (routes.cppm:55-62) — admin `destroy`; accessor `all.cppm:424`.
   Fix: No change required. Ghost binds `DELETE /tags/:id` to admin `api.tags.destroy` (`admin/routes.js:144`); owned by the admin `tags.js` audit. Keep the `.delt(...)` binding and `tags_destroy()` handler — no removal from this content controller. No replacement call/type. Check 1 (EXTRA) is closed.

Same convention as `pages-public.md`, `newsletters-public.md` and `recommendations-public.md`: admin-owned sibling routes are listed here but not counted as EXTRA relative to Ghost overall. The C++ `GET /search-index/tags` route (`include/blog/handlers/discovery/search_index/routes.cppm:19,55-64`) belongs to the separate `searchIndexPublic.fetchTags` content domain and is not counted here.

## Implementation status (manifest group 8)

Applied the three route-level `#### Fix` blocks above to the same shared source files as `tags.md` (`include/blog/handlers/admin/tags/`) on 2026-11-19. The `No change required` items (Missing in C++ premise check; the three admin-owned EXTRA routes) were skipped — no source edit for those. Verdicts unchanged (PARTIAL ×3, MISSING 0): checks 1–8 closed except the accepted residuals already listed in each fix block (`debug` drop, restricted-field transformer, config-gated cache, `visibility`, `permissions:true`). This fix is source-applied, so the earlier "documentation only" wording in check 8 of each block is superseded — no route row or verdict changed.

### Files changed (all under `include/blog/handlers/admin/tags/`)
- `browse/workflow/step/execute.cppm`, `browse/handler.cppm` — `include/filter/fields/limit/order/page/debug` query parse, `include`∈`{count.posts}` validation (422 body), `json_for("tags", params)`, real `meta.pagination` from `params` + `total("tags")`, dead-token guard removed, `X-Cache-Invalidate: false`.
- `read_by_id/workflow/step/execute.cppm`, `read_by_id/handler.cppm` — real trailing path id (no hardcoded `"1"`), empty-id 400/422-style validation, single TagPublic-shaped record, 404 `Tag not found.` on miss, map `status`/`body` Output, `X-Cache-Invalidate: false`, dead guard removed. Slug fallback in `find_by_resource` and `visibility` remain accepted residuals.
- `read_by_slug/workflow/step/execute.cppm`, `read_by_slug/handler.cppm` — parsed trailing slug, `json_for("tags", {{"filter","slug:"+slug}})` to keep a slug predicate, first record via `core_serde`, single-model/404, map Output, `X-Cache-Invalidate: false`, dead guard removed.
- `routes.cppm` — `.use(blog::handlers::middleware::optional_auth)` on the `/tags` router and `/slug` sub-router; placeholder `/:id` → `/:slug`.

### Central registrations
None needed. `tags_browse()` / `tags_read_by_id()` / `tags_read_by_slug()` accessors and members are already registered in the shared `include/blog/handlers/all.cppm` (`:17-19,363-365,612-614`) and the routes installed by the shared `include/blog/handlers/routes.cppm:50,123`. No shared file (`all.cppm`, `base.cppm`, top-level `routes.cppm`, `BUILD.bazel`, `context/`, `middleware/`, `dto/`) was edited. No new deps: `core_serde` / `blog_handlers_middleware` are existing deps. Partition names unchanged (`blog_handlers:tags_<op>_handler/_step/_flow`).

### Auth reconciliation with `tags.md` (same source files)
Both docs target the same flat router. `tags.md` asked for `require_auth`, this doc asks for `optional_auth` (anonymous content reads, `mw.authenticatePublic`). Implemented `optional_auth`: `require_auth` would reject the anonymous GET reads this controller requires, and no content/admin base split exists here. The dead `&& !true` guards are removed. Admin mutation auth enforcement remains an accepted residual pending a shared-file route split.

### Verification
All 18 tag source units compile clean with the target's exact clang/module flags. The full `blog_handlers` Bazel target is still blocked by pre-existing compile errors in `admin/posts/browse/workflow/step/execute.cppm`, unrelated to `tags/`.


## Strict-parity rewrite (manifest group 9, 2026-11-19)

Rewrote the three shared GET handlers/steps in `include/blog/handlers/admin/tags/` (same files as
`tags.md`) against the shared abstractions; the route table above is the original pre-rewrite
snapshot and its verdict column is now post-rewrite. **MATCH ×3** (from PARTIAL ×3).

- `core::query::parse`/`run`/`apply` + `has_restricted_field` implement the tags-public input
  pipeline (seven options, `include` allowlist, restricted-key transformer equivalent, default
  `order:'name asc'`, real `meta.pagination`), replacing the fixed 15/1 pagination and the
  `slug:` string concatenation.
- `blog::mapping::clean(...,"tag")` + `wrap("tags",...)` produce the single-resource `{tags:[...]}`
  envelope with the TagPublic field allow-list; 422 (empty id/slug) → 400 for `/content/` paths;
  literal `X-Cache-Invalidate: false` removed (`cacheInvalidate:false` emits no header).
- `require_ghost_permission` + `core::auth::ghost::GhostAuthorizer` enforce `authenticatePublic`
  anonymous read semantics with `CONTENT_API` context; router-level `optional_auth` /
  `content_type_middleware` removed.

Files changed: `routes.cppm`, `browse/`, `read_by_id/`, `read_by_slug/` handler + step files.
No new partitions, no shared-file edits; all registrations already exist centrally (existing
handler accessors/members in `all.cppm`, `TagsRoutes` install in central `routes.cppm`, existing
module deps). Remaining gaps: config-gated response cache + `site.changed` reset, `count.posts`
relation, `data.visibility` public constraint and URL-mapper enrichment — none change the audited
status codes, envelopes or headers. Verification: no `admin/tags/` compile error; full target
blocked by pre-existing errors in `admin/pages/*` and the missing `blog_handlers:notifications_service`.
