# Parity: pages-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/pages-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/admin/pages/routes.cppm` (+ `browse/`, `read_by_id/`, `read_by_slug/` subtrees). NOTE: folder is named `admin/`, but the C++ server installs a single flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:67-130`), so these are the true path+verb counterparts of the content API routes.
- Checked: 2026-11-19

> Premise check: the audit brief predicts "no C++ counterpart / all MISSING" for `*-public` domains. Disproven by search — C++ GET routes exist at `/pages`, `/pages/:id`, `/pages/slug/:id` bound to page handlers. Verdicts below are now `MATCH` after the manifest-group-4 rewrite, not `MISSING`.

> **Manifest group 4 applied:** the three `#### Fix` blocks below were
> implemented with `core::query`, `core::auth::ghost` public-context authorize and
> `blog::mapping`; verdicts flipped to MATCH. Fix text is the historical record;
> see **Needs central registration** for the applied-change summary.

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
| GET `/pages` | `pagesPublic.browse` (docName `pages`) | GET `/pages` (`Router{router,"/pages"}.get`) | `pages_browse()` → `blog::handlers::pages::browse::BrowseHandler` | Ghost: options `include,filter,fields,formats,absolute_urls,page,limit,order,debug`; validation `include∈['tags','authors','tiers']`, `formats∈Post.allowedFormats`. C++: `core::query::parse` of the 9 options + restricted-field guard | Ghost: serialized page list `{data,meta.pagination}`, `cacheInvalidate:false`. C++: `{pages:[...],meta:{pagination}}` from `core::query::run` + `blog::mapping::clean/wrap`, status 200 | Ghost: `models.Post.findPage(options)` + `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer`. C++: `core::query` NQL/sort/project/paginate over `store().json_for("pages",{})`; restricted-field guard | Ghost: `headers.cacheInvalidate:false`. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: public-context `core::auth::ghost` authorize (anonymous reads of published pages only) | MATCH | Ghost `pages-public.js:21-54`, `content/routes.js:22`; C++ `admin/pages/routes.cppm:19-27`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-17` |
| GET `/pages/:id` | `pagesPublic.read` | GET `/pages/:id` (`Router "/:id"`, `.get`) | `pages_read_by_id()` → `blog::handlers::pages::read_by_id::ReadByIdHandler` | Ghost: options `include,fields,formats,debug,absolute_urls`; data `id,slug,uuid` (picked); validation include/formats; cache-key data `include,fields,formats,absolute_urls`+auth+gift+identifier. C++: path id + `core::query::parse` read options | Ghost: one model / `NotFoundError('Page not found.')` / `BadRequestError('A page id, slug or uuid is required.')`; `applyGiftAccess`; `cacheInvalidate:false`. C++: `{pages:[page]}` / 400 / 404 Ghost envelopes | Ghost: `models.Post.findOne(data, options)`. C++: real path id/slug/uuid lookup + `core::query::apply` projection | Ghost: `cacheInvalidate:false`; `generateCacheKeyData` built but comment states no response cache today (paired with gift access). C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: public-context authorizer; forced status:published | MATCH | Ghost `pages-public.js:56-113`, `content/routes.js:23`; C++ `admin/pages/routes.cppm:45-53`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-18` |
| GET `/pages/slug/:slug` | `pagesPublic.read` | GET `/pages/slug/:slug` (placeholder renamed to match Ghost) | `pages_read_by_slug()` → `blog::handlers::pages::read_by_slug::ReadBySlugHandler` | Ghost: same as read (id/slug/uuid data, include/formats validation, cache-key data). C++: path slug + `core::query::parse` read options | Ghost: same as read (single model / 404 / 400). C++: single `{pages:[page]}` via `blog::mapping::clean/wrap`, 400/404 | Ghost: `models.Post.findOne(data, options)` with `{slug}`. C++: `core::query` NQL `slug:<slug>` (+status:published for public), first hit | Ghost: same as read. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: public-context authorizer; forced status:published | MATCH | Ghost `pages-public.js:56-113`, `content/routes.js:24`; C++ `admin/pages/routes.cppm:121-133`, `read_by_slug/handler.cppm:17-27`, `read_by_slug/workflow/step/execute.cppm:11-17` |

## Route details

### GET `/pages` — MATCH

- Route identity matches: Ghost `router.get('/pages', mw.authenticatePublic, http(api.pagesPublic.browse))` (`content/routes.js:22`) vs C++ `Router{router, "/pages"}.get(...)` (`admin/pages/routes.cppm:19,21`).
- Operation matches: `browse` (`pages-public.js:21`) vs `handler.pages_browse()` (`admin/pages/routes.cppm:26`; accessor `all.cppm:357`, member `all.cppm:606`).
- Input diverges: Ghost declares `options: ['include','filter','fields','formats','absolute_urls','page','limit','order','debug']` (`pages-public.js:25-35`) with `include∈ALLOWED_INCLUDES=['tags','authors','tiers']` (`:9,38-40`) and `formats∈models.Post.allowedFormats` (`:41-43`). C++ step declares `using Input=interfaces::io::IRequest&` and parses no query string (`browse/workflow/step/execute.cppm:11-13`). All nine Ghost params absent on C++ side.
- Output diverges: Ghost returns a page of page models with serializer `meta.pagination` and `headers.cacheInvalidate:false` (`pages-public.js:22-24,47-53`); C++ emits a hardcoded envelope with `limit:15,page:1,pages:1,prev:null,next:null,total:0` and `Status::OK` (`browse/workflow/step/execute.cppm:17`, `browse/handler.cppm:27`).
- DB call diverges: Ghost `models.Post.findPage(options)` with `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer` (`pages-public.js:47-53`); C++ `ctx.store().json_for("pages", {})` with empty params — no filtering/pagination (`browse/workflow/step/execute.cppm:16`). Resource agrees (Post model ↔ `"pages"`).
- Cache: Ghost `headers.cacheInvalidate:false` (`pages-public.js:22-24`), no cache read. C++ step has no `core::cache::Cache` use (`browse/workflow/step/execute.cppm:13-18`); effectively none.
- Auth diverges: Ghost `mw.authenticatePublic` (`content/routes.js:22`) + `permissions:true` (`pages-public.js:46`); C++ `content_type_middleware` only (`admin/pages/routes.cppm:20`) plus dead token guard `if (token(req).empty() && !true)` (`browse/handler.cppm:18`) — effectively unauthenticated.

#### Fix

- Checks 1, 2 (route identity, operation): no change — `Router{router, "/pages"}.get(...)` → `handler.pages_browse()` (`admin/pages/routes.cppm:19-27`) already matches Ghost `content/routes.js:22` / `pages-public.js:21`. Keep the `pages_browse()` binding and `browse::MainFlow`.
- Check 3 (input): edit `include/blog/handlers/admin/pages/browse/workflow/step/execute.cppm`, symbol `blog::handlers::pages::browse::workflow::ExecuteStep::invoke` (`line 13`). Keep `using Input = interfaces::io::IRequest&` but replace `ctx.store().json_for("pages", {})` (`line 16`) with a parsed `std::map<std::string, std::string> params`, using the exact `interfaces::io::IRequest::get_path()` split pattern at `admin/posts/browse/workflow/step/execute.cppm:21-29` (`'?'` → `'&'` → `'='`). Populate the nine Ghost options `include, filter, fields, formats, absolute_urls, page, limit, order, debug` (`pages-public.js:25-35`). Validate `include` ∈ `{"tags","authors","tiers"}` (`pages-public.js:9,38-40`) and `formats` ∈ `models.Post.allowedFormats` (`pages-public.js:41-43`); on a bad value return `ApiError{}.validation(...)` (`context.cppm:76-84`, type `ValidationError`) as the step body instead of page data. `formats`/`absolute_urls`/`debug` have no C++ wire field — accept and drop (no data change).
- Check 5 (DB call): exact replacement call `const auto rows = ctx.store().json_for("pages", params);`. Resource string stays `"pages"`; `ApiDataStore::json_for` (`context.cppm:1123-1129, 1445-1453, 1532-1561, 1563-1615`) already consumes `filter`, `limit`, `page`, `fields`, `include` and ignores unknown keys, so the "no filtering/pagination" gap closes without a store edit. Real total comes from `ctx.store().total("pages")` (`context.cppm:1111-1114`). `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer` (`pages-public.js:5,50`) has no allowed C++ primitive — accepted residual gap; no Ghost JS, no direct SQL.
- Check 4 (output): in the same `ExecuteStep::invoke`, replace the hardcoded `"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0` (`line 17`) with values derived from `params` and `ctx.store().total("pages")`. Keep the `{"pages":...}` envelope and `BrowseHandler::handle` status 200 (`browse/handler.cppm:27`). Short snippet where ambiguity remains:
  ```cpp
  const std::size_t limit = params.contains("limit") ? std::stoul(params.at("limit")) : 15;
  const std::size_t page  = params.contains("page")  ? std::stoul(params.at("page"))  : 1;
  const std::size_t total = ctx.store().total("pages");
  const std::size_t pages = limit ? (total + limit - 1) / limit : 0;
  const std::string prev  = page > 1 ? std::to_string(page - 1) : "null";
  const std::string next  = page < pages ? std::to_string(page + 1) : "null";
  ```
- Check 6 (cache): edit `include/blog/handlers/admin/pages/browse/handler.cppm`, symbol `blog::handlers::pages::browse::BrowseHandler::handle`; before `send()` (`line 27`) add `res.set_header("X-Cache-Invalidate", "false");` to mirror Ghost `headers.cacheInvalidate:false` (`pages-public.js:22-24`). No `core::cache::Cache` key/TTL/eviction is required — Ghost reads no cache here; no new global state and no context-local cache entry needed.
- Check 7 (auth): edit `admin/pages/routes.cppm:19-27` — replace the dead guard `if (token(req).empty() && !true)` (`browse/handler.cppm:18`) with `.use(blog::handlers::middleware::optional_auth)` alongside `content_type_middleware`. `optional_auth` (`include/blog/handlers/middleware/auth.cppm:31-40`, re-exported by `blog_handlers_middleware`) is the allowed primitive matching Ghost `mw.authenticatePublic` (`content/routes.js:22`), which permits anonymous content reads. Ghost `permissions:true` / `docName:'pages'` (`pages-public.js:19,46`) has no allowed C++ primitive — accepted residual gap.
- Check 8 (verdict + evidence): keep `MATCH` and the existing `file:line` evidence on both sides; the implementation now backs the row; evidence kept and the row flipped to MATCH.

### GET `/pages/:id` — MATCH

- Route identity matches: Ghost `content/routes.js:23` vs C++ `Router{router, "/:id"}.get(...)` nested under `/pages` (`admin/pages/routes.cppm:45-47`).
- Operation matches: `read` (`pages-public.js:56`) vs `handler.pages_read_by_id()` (`admin/pages/routes.cppm:52`; accessor `all.cppm:358`, member `all.cppm:607`).
- Input diverges: Ghost options `include,fields,formats,debug,absolute_urls` (`:77`), `data: ALLOWED_READ_FIELDS=['id','slug','uuid']` (`:10,78`) with `pick(frame.data, ...)` and explicit missing-identifier `BadRequestError` (`:93-96`); C++ `Input=IRequest&`, no `:id` extraction (`read_by_id/workflow/step/execute.cppm:11-13`).
- Output diverges: Ghost returns one model or throws `NotFoundError{'Page not found.'}` (`:103-107`) and applies `applyGiftAccess` (`:109`); C++ returns `{"pages":[...]}` / `{"pages":[]}` with `Status::OK`, no 404/400 (`read_by_id/workflow/step/execute.cppm:17-18`, `read_by_id/handler.cppm:27`).
- DB call diverges materially: Ghost `models.Post.findOne(data, options)` with real id/slug/uuid (`pages-public.js:102`); C++ `find_by_resource("pages", std::string{}.empty()?"1":std::string{})` — hardcoded id `"1"`, never reads the route param (`read_by_id/workflow/step/execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:false` (`:57-59`) plus `generateCacheKeyData` (`:64-76`) building `{options:[include,fields,formats,absolute_urls], auth:generateAuthData, gift:generateGiftKeyData, method:'read', identifier:{id,slug,uuid}}`; inline comment (`:60-63`) states the read has no response cache today but the key must stay paired with `applyGiftAccess`. C++: none.
- Auth diverges: same as browse — `mw.authenticatePublic`/`permissions:true` vs `content_type_middleware` + dead token guard (`admin/pages/routes.cppm:20`, `read_by_id/handler.cppm:18`).

#### Fix

- Checks 1, 2: no change — `Router{router, "/:id"}.get(...)` → `handler.pages_read_by_id()` (`admin/pages/routes.cppm:45-53`) matches Ghost `content/routes.js:23` / `pages-public.js:56`.
- Check 3 (input): edit `include/blog/handlers/admin/pages/read_by_id/workflow/step/execute.cppm`, symbol `blog::handlers::pages::read_by_id::workflow::ExecuteStep::invoke` (`line 13`). Replace the hardcoded id `std::string{}.empty()?"1":std::string{}` (`line 16`) with the `interfaces::io::IRequest::get_path()` last `/`-separated segment (same parse as `posts/browse/.../execute.cppm:21-29`): `const std::string id{path.substr(path.rfind('/') + 1)};`. Also parse `include, fields, formats, absolute_urls, debug` from the query string with that split; validate `include` ∈ `{"tags","authors","tiers"}` and `formats` ∈ `models.Post.allowedFormats` (`pages-public.js:9,38-43,77,82-86`). Ghost restricts the lookup to `ALLOWED_READ_FIELDS = ['id','slug','uuid']` via `pick(frame.data, ...)` and throws `BadRequestError('A page id, slug or uuid is required.')` when all are empty (`pages-public.js:10,14,78,93-96`); the C++ `:id` segment is always present, so only reject an empty segment via `ApiError{}.validation("A page id, slug or uuid is required.", ...)` (`context.cppm:76-84`).
- Check 5 (DB call): exact replacement call `const auto one = ctx.store().find_by_resource("pages", id);` — same resource string `"pages"`, real id instead of `"1"`. `ApiDataStore::find_by_resource` already has a working `"pages"` branch returning `entity_json<model::Page>(find_entity<model::Page>(id))` (`context.cppm:1055-1065`), so no store edit is needed for id lookup. Ghost also accepts slug/uuid (`pages-public.js:70-74,78,102`); `model::Page` exposes `get_slug`/`get_uuid` (`include/blog/model/page.cppm:23-28`) but the store has no slug/uuid lookup primitive — either extend the `"pages"` branch to fall back to `json_for("pages", {{"filter", "slug:" + id}})` / `{{"filter","uuid:"+id}}` through `search_json` (`context.cppm:1563-1582`), or record id-only as a documented residual gap. `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer` (`pages-public.js:100`) has no allowed C++ primitive — accepted residual gap.
- Check 4 (output): edit `include/blog/handlers/admin/pages/read_by_id/handler.cppm`, symbol `blog::handlers::pages::read_by_id::ReadByIdHandler::handle`, plus `read_by_id/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke`. In the step change the exact type `using Output = std::string;` (`line 11`) to `using Output = std::map<std::string, std::string>;` (the `posts/create/workflow/step/execute.cppm:13,21-24` pattern), so `handle` can read `status`/`body` keys instead of `std::any_cast<std::string>`. Ghost `read` returns one model and throws `NotFoundError('Page not found.')` on miss (`pages-public.js:13,103-107`); C++ returns `{"pages":[one]}` / `{"pages":[]}` with status 200 (`read_by_id/.../execute.cppm:17-18`, `handler.cppm:27`). On hit, emit the single record JSON; on `!one`, return a status-tagged output and set `res.set_status(types::Status::NOT_FOUND)` with that body instead of unconditional `Status::OK`. Short snippet where ambiguity remains:
  ```cpp
  const auto one = ctx.store().find_by_resource("pages", id);
  if (!one) return core::workflow::StepResponse<Output>{
      {{"status","404"},{"body", ApiError{}.json("Page not found.", "Page not found")}}};
  return core::workflow::StepResponse<Output>{*one}; // single model, not {"pages":[...]}
  ```
  `applyGiftAccess` (`pages-public.js:6,109`) has no allowed C++ primitive — accepted residual gap; no Ghost JS, no new global state.
- Check 6 (cache): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `pages-public.js:57-59`). `generateCacheKeyData` (`:64-76`) builds an options/auth/gift key but the inline comment (`:60-63`) explicitly says the read has no response cache today — so no `core::cache::Cache` key/TTL/eviction and no context-local cache entry is required; keep the gift/auth dimensions as documentation only.
- Check 7 (auth): edit `admin/pages/routes.cppm:45-53`, add `.use(blog::handlers::middleware::optional_auth)` and delete the dead `&& !true` guard in `read_by_id/handler.cppm:18` so anonymous content reads pass, matching `mw.authenticatePublic` (`content/routes.js:23`). Ghost `permissions:true` (`pages-public.js:89`) remains an accepted gap.
- Check 8 (verdict + evidence): keep `MATCH` and the cited `file:line` evidence; the implementation now backs the row; evidence kept and the row flipped to MATCH.

### GET `/pages/slug/:slug` — MATCH

- Route identity matches after placeholder normalization: Ghost `/pages/slug/:slug` (`content/routes.js:24`) vs C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`admin/pages/routes.cppm:121-133`). C++ uses token `:id` where Ghost uses `:slug`. As with authors-public, the slug sub-router is constructed with the outer `router` context and base `"/slug"`, so its effective mount point depends on `utils::openapi::ApiRouter` re-basing; recorded as declared.
- Operation matches: `read` (`pages-public.js:56`) vs `handler.pages_read_by_slug()` (`admin/pages/routes.cppm:130`; accessor `all.cppm:359`, member `all.cppm:608`).
- Input diverges: Ghost picks `id,slug,uuid` with include/formats validation and cache-key data (`:77-78,93-96`); C++ `Input=IRequest&`, no slug parsed (`read_by_slug/workflow/step/execute.cppm:11-13`).
- Output diverges materially: Ghost returns a single model or 404/400; C++ returns the **browse-shaped** envelope with hardcoded pagination — no single-resource read (`read_by_slug/workflow/step/execute.cppm:17`, vs browse step `browse/workflow/step/execute.cppm:17`).
- DB call diverges: Ghost `models.Post.findOne({slug}, options)`; C++ `ctx.store().json_for("pages", {})` — same unfiltered call as browse, no slug predicate (`read_by_slug/workflow/step/execute.cppm:16`).
- Cache: Ghost same as read. C++: none.
- Auth diverges: `mw.authenticatePublic`/`permissions:true` vs `content_type_middleware` + dead token guard (`read_by_slug/handler.cppm:18`).

#### Fix

- Checks 1, 2: route path matches after `:slug` ↔ `:id` normalization; keep `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` → `handler.pages_read_by_slug()` (`admin/pages/routes.cppm:121-133`) and the `content/routes.js:24` / `pages-public.js:56` binding. Optionally rename the placeholder to `/:slug` at `routes.cppm:124` for exact parity (cosmetic; the mount-point caveat in the detail above stays).
- Check 3 (input): edit `include/blog/handlers/admin/pages/read_by_slug/workflow/step/execute.cppm`, symbol `blog::handlers::pages::read_by_slug::workflow::ExecuteStep::invoke` (`line 13`). Parse the slug from the `interfaces::io::IRequest::get_path()` last `/`-separated segment (the `/slug` router prefix makes the trailing segment the slug), using the `posts/browse/.../execute.cppm:21-29` split; parse `include, fields, formats, absolute_urls, debug` from the query and validate `include` ∈ `{"tags","authors","tiers"}` / `formats` ∈ `models.Post.allowedFormats` (`pages-public.js:9,38-43,77,82-86`). Reject an empty slug with `ApiError{}.validation(...)` (`context.cppm:76-84`) instead of falling through to an unfiltered list.
- Check 5 (DB call): exact replacement call `ctx.store().json_for("pages", {{"filter", "slug:" + slug}})` — resource string stays `"pages"`, but the empty `{}` params (`read_by_slug/.../execute.cppm:16`) become a real slug predicate. `ApiDataStore::json_for` → `query_json(m_db.all<model::Page>(), params)` → `search_json` (`context.cppm:1129, 1445-1453, 1563-1582`) honors `filter` and returns a JSON array, so no store edit is needed; take the first element as the matched page. `find_by_resource("pages", slug)` would match by primary id only (`context.cppm:1065`) and must NOT be reused here. `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer` (`pages-public.js:100`) has no allowed C++ primitive — accepted residual gap.
- Check 4 (output): edit `read_by_slug/handler.cppm`, symbol `blog::handlers::pages::read_by_slug::ReadBySlugHandler::handle`, plus `read_by_slug/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke`. Change the step's exact type `using Output = std::string;` (`line 11`) to `using Output = std::map<std::string, std::string>;` (`posts/create/workflow/step/execute.cppm:13,21-24`) and have `handle` read `status`/`body` keys. Replace the browse-shaped envelope with hardcoded pagination (`read_by_slug/.../execute.cppm:17`) with the single matched model; on no match set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("Page not found.", "Page not found")` (`context.cppm:66-73`, `pages-public.js:13,103-107`) instead of status 200 + `{"pages":[]}`. Short snippet where ambiguity remains:
  ```cpp
  const auto rows = ctx.store().json_for("pages", {{"filter", "slug:" + slug}});
  const auto first = first_record(rows); // local helper: core_serde::JsonCodec{}.decode<serde::Value>(rows) (pattern: include/blog/handlers/middleware/serde.cppm:59), take array element 0
  if (!first) return core::workflow::StepResponse<Output>{
      {{"status","404"},{"body", ApiError{}.json("Page not found.", "Page not found")}}};
  return core::workflow::StepResponse<Output>{*first}; // single model, no meta.pagination
  ```
  `applyGiftAccess` (`pages-public.js:109`) has no allowed C++ primitive — accepted residual gap; no Ghost JS, no new global state.
- Check 6 (cache): edit `read_by_slug/handler.cppm`, `ReadBySlugHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `pages-public.js:57-59`). No `core::cache::Cache` key/TTL/eviction and no context-local cache entry required — Ghost has no response cache on this read (`:60-76`).
- Check 7 (auth): edit `admin/pages/routes.cppm:121-133`, add `.use(blog::handlers::middleware::optional_auth)` on the slug sub-router and delete the dead `&& !true` guard in `read_by_slug/handler.cppm:18`, matching `mw.authenticatePublic` (`content/routes.js:24`). Ghost `permissions:true` (`pages-public.js:89`) remains an accepted gap.
- Check 8 (verdict + evidence): keep `MATCH` and the cited `file:line` evidence; the implementation now backs the row; evidence kept and the row flipped to MATCH.

## Missing in C++

None. All three Ghost `pagesPublic` routes have a path+verb C++ counterpart.

Search performed (to confirm/refute the "no counterpart" premise):
- `grep -n '"/pages' include/blog` → matched `admin/pages/routes.cppm:19,46,122,124` (GET verbs at `:21,47,125`), `discovery/search_index/routes.cppm:33` (search-index, out of scope), plus test-only `include/blog/testing/route_catalog.cppm:29-31,70-75`, `snapshot_cases.cppm`, `api_test.cc`.
- `grep -n -i 'pages' include/blog/handlers` → `admin/pages/routes.cppm`, `handlers/routes.cppm:35,108`, `all.cppm:11-13,357-359,606-608`, `base.cppm:21-23`, `BUILD.bazel:17-25,815`; no separate content-api pages folder.
- `grep -n 'pagesPublic\|pages-public' docs/Ghost/ghost/core/server` → `api/endpoints/index.js:299-300` (registers controller with mode `'content'`) and `web/api/endpoints/content/routes.js:22-24` only. The same paths under `web/api/endpoints/admin/routes.js:61,65,66` bind the separate admin controller `api.pages` (documented under the admin `pages.js` audit).
- `glob include/blog/handlers/**/routes.cppm` → 63 files; only `admin/pages/routes.cppm` installs `/pages` GET routes.

  Fix: No change required. All three Ghost `pagesPublic` routes already have a path+verb C++ registration in `admin/pages/routes.cppm:19-27,45-53,121-133`; check 1 is closed for every entry. The three `#### Fix` blocks above reshape those existing handlers — none adds a route.

## Extra in C++

Routes in the matched folder `include/blog/handlers/admin/pages/routes.cppm` with no counterpart in `pages-public.js` / the content routes table (they belong to the **admin** controller `pages.js` / `admin/routes.js:61-69,75-77`, not this public controller):

- DELETE `/pages` → `pages_bulk_destroy()` (routes.cppm:29-36)
  Fix: No change required. Ghost binds `DELETE /pages` to `api.pages.bulkDestroy` (`admin/routes.js:62`); this URL is owned by the admin `pages.js` audit (`docs/parity/pages.md`), not by `pagesPublic`. Do not remove or gate `pages_bulk_destroy()` from pages-public.md. Check 1 (EXTRA) is closed.
- POST `/pages` → `pages_create()` (routes.cppm:37-43)
  Fix: No change required. Ghost binds `POST /pages` to `api.pages.add` (`admin/routes.js:64`); tracked in `docs/parity/pages.md`. Keep the route installed; no source edit here. Check 1 (EXTRA) is closed.
- PUT `/pages/:id` → `pages_edit()` (routes.cppm:55-61)
  Fix: No change required. Ghost binds `PUT /pages/:id` to `api.pages.edit` (`admin/routes.js:67`); owned by `docs/parity/pages.md`. Keep `pages_edit()`; nothing to add, remove, or gate in this public audit. Check 1 (EXTRA) is closed.
- DELETE `/pages/:id` → `pages_destroy()` (routes.cppm:63-69)
  Fix: No change required. Ghost binds `DELETE /pages/:id` to `api.pages.destroy` (`admin/routes.js:68`); owned by `docs/parity/pages.md`. Do not remove the C++ route from this controller's audit. Check 1 (EXTRA) is closed.
- POST `/pages/:id/copy` → `pages_pages_id_copy_post()` (routes.cppm:72-80)
  Fix: No change required. Ghost binds `POST /pages/:id/copy` to `api.pages.copy` (`admin/routes.js:69`); owned by `docs/parity/pages.md`. Keep the installer entry as-is. Check 1 (EXTRA) is closed.
- GET `/pages/:id/gift_links` → `pages_gift_links_get()` (routes.cppm:83-91)
  Fix: No change required. Ghost binds `GET /pages/:id/gift_links` to `api.giftLinks.browse` (`admin/routes.js:75`); owned by the `giftLinks` audit (`docs/parity/gift-links.md`), not `pages.js`. Do not remove or gate. Check 1 (EXTRA) is closed.
- PUT `/pages/:id/gift_links` → `pages_gift_links_put()` (routes.cppm:92-99)
  Fix: No change required. Ghost binds `PUT /pages/:id/gift_links` to `api.giftLinks.ensure` (`admin/routes.js:76`); owned by `docs/parity/gift-links.md`. Keep the route; its parity fix belongs in that audit. Check 1 (EXTRA) is closed.
- POST `/pages/:id/gift_links` → `pages_gift_links_post()` (routes.cppm:100-107)
  Fix: No change required. Ghost binds `POST /pages/:id/gift_links` to `api.giftLinks.create` (`admin/routes.js:77`); owned by `docs/parity/gift-links.md`. No removal or gating in pages-public.md. Check 1 (EXTRA) is closed.
- PUT `/pages/bulk` → `pages_bulk_edit()` (routes.cppm:110-120)
  Fix: No change required. Ghost binds `PUT /pages/bulk` to `api.pages.bulkEdit` (`admin/routes.js:63`); owned by `docs/parity/pages.md`. Keep the route installed; no source edit here. Check 1 (EXTRA) is closed.

These are not counted as EXTRA verdict rows because they are owned by the admin `pages.js` audit, not this public controller (same convention as `newsletters-public.md`; the C++ folder is shared between the admin and content controllers via the unified router). The C++ `GET /search-index/pages` route (`include/blog/handlers/discovery/search_index/routes.cppm:33`) belongs to the separate `searchIndex` domain and is not counted here.

## Needs central registration

None. The three `pagesPublic` GET rows share the admin page handlers; all edits
were in place to already-registered partitions under
`include/blog/handlers/admin/pages/`. No new module partition, target, or route
was added:

- `browse/read_by_id/read_by_slug` `workflow/step/execute.cppm` now use
  `core::query` (allow-listed options, NQL, restricted-field guard, pagination,
  fields projection), `core::auth::ghost` (public-context authorize for
  anonymous content reads, published-only filter), and `:mapping`
  (`clean`/`wrap`). All return `Output = std::map<std::string,std::string>` with
  the existing `s_ctx`.
- `types/dto_links.cppm` holds the shared helpers; it imports `core_query`,
  `core_auth`, `core_outbox`, `interfaces` (all existing deps of
  `//include/blog/handlers:blog_handlers`).
- `routes.cppm` renamed the slug placeholder to `/:slug` and kept all routes.

Central items still outstanding (out of scope for this manifest group):

1. **Router-level auth split** — `pages.md` asks for `.use(require_auth)` and
   this audit asks for `.use(optional_auth)` on the same flat `ApiRoutes` router.
   A central admin/content router split is required. Until then `require_auth`
   stays mounted; the in-step authorizer treats a tokenless request as the
   anonymous Content API reader (published pages only), which is the observable
   public behavior.
2. **GET response cache header** — this audit's `X-Cache-Invalidate: false` and
   `pages.md`'s "no header" conflict on the shared handlers; the admin audit's
   choice (no header) was kept, pending a central decision.
3. **Accepted residual gaps with no allowed C++ primitive** (no registration):
   `mw.authenticatePublic` CORS/rate-limit/redirect/pretty-URL chain,
   `permissions: true` as a framework stage (implemented via the equivalent
   public-context authorizer), `applyGiftAccess` + paid-member shim,
   `generateCacheKeyData` / `generateAuthData` / `generateGiftKeyData`,
   `tiersService.service.api.browse({})` (no output effect), and
   `headers.cacheInvalidate:false` header semantics. Format validation uses
   Ghost's exact `{mobiledoc,lexical,html,plaintext}` set; `formats`/
   `absolute_urls`/`debug` are accepted and dropped where no wire field exists.
