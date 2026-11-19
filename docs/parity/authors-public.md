# Parity: authors-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/authors-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/admin/authors/routes.cppm` (+ `browse/`, `read_by_id/`, `read_by_slug/` subtrees). NOTE: folder is named `admin/`, but the Ghost monorepo has **no admin authors controller** and C++ installs one flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:67-130`), so these are the true path+verb counterparts of the content API routes.
- Checked: 2026-11-19

> Premise check: the audit brief predicted "no C++ counterpart / all MISSING". Disproven by search — C++ GET routes exist at `/authors`, `/authors/:id`, `/authors/slug/:id` and are bound to author handlers. Verdicts below are therefore `PARTIAL`, not `MISSING`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/authors` | `authorsPublic.browse` (docName `authors`) | GET `/authors` | `authors_browse()` → `blog::handlers::authors::browse::BrowseHandler` | Ghost: options `include,filter,fields,limit,order,page`; validation `include∈['count.posts']`. C++: `Input=IRequest&`, **no query parsing at all** | Ghost: model list `{data,meta}` via serializer, `headers.cacheInvalidate:false`. C++: `{"authors":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}`, status 200 | Ghost: `models.Author.findPage(options)` + `mongoTransformer: rejectAuthorsContentApiRestrictedFieldsTransformer`. C++: `ctx.store().json_for("authors", {})` (no filter/pagination) | Ghost: none (`cacheInvalidate:false`). C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: `content_type_middleware` only; token check dead (`&& !true`) | PARTIAL | Ghost `authors-public.js:19-38`, `content/routes.js:27`; C++ `admin/authors/routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-17` |
| GET `/authors/:id` | `authorsPublic.read` | GET `/authors/:id` | `authors_read_by_id()` → `blog::handlers::authors::read_by_id::ReadByIdHandler` | Ghost: options `include,filter,fields`; data `id,slug` (picked); validation `include∈['count.posts']`. C++: `Input=IRequest&`, **no path/query parsing** | Ghost: single model / `NotFoundError` "Author not found." / `BadRequestError` on missing identifier; `cacheInvalidate:false`. C++: `{"authors":[...]}` or `{"authors":[]}`, status 200, no 404/400 | Ghost: `models.Author.findOne(data, options)` + mongoTransformer. C++: `ctx.store().find_by_resource("authors", "1")` — hardcoded id `"1"`, ignores `:id` | Ghost: none. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: `content_type_middleware`; dead token check | PARTIAL | Ghost `authors-public.js:41-76`, `content/routes.js:28`; C++ `admin/authors/routes.cppm:29-39`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-18` |
| GET `/authors/slug/:slug` | `authorsPublic.read` | GET `/authors/slug/:id` (`:id` = `:slug` normalized) | `authors_read_by_slug()` → `blog::handlers::authors::read_by_slug::ReadBySlugHandler` | Ghost: same as read (id/slug data, include validation). C++: `Input=IRequest&`, **no slug parsing** | Ghost: same as read (single model / 404 / 400). C++: `{"authors":[...],"meta":{pagination:{15,1,1,...,0}}}` — **browse-shaped output, not single model**, status 200 | Ghost: `models.Author.findOne(data, options)`. C++: `ctx.store().json_for("authors", {})` — identical to browse, no slug filter | Ghost: none. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: `content_type_middleware`; dead token check | PARTIAL | Ghost `authors-public.js:41-76`, `content/routes.js:29`; C++ `admin/authors/routes.cppm:40-53`, `read_by_slug/handler.cppm:17-27`, `read_by_slug/workflow/step/execute.cppm:11-17` |

## Route details

### GET `/authors` — PARTIAL

- Route identity matches: Ghost `router.get('/authors', ..., http(api.authorsPublic.browse))` (`content/routes.js:27`) vs C++ `Router{router, "/authors"}.get(...)` (`admin/authors/routes.cppm:19,21`).
- Operation matches: `browse` (`authors-public.js:19`) vs `handler.authors_browse()` (`admin/authors/routes.cppm:26`).
- Input diverges: Ghost declares `options: ['include','filter','fields','limit','order','page']` (`authors-public.js:23`) with `include` restricted to `ALLOWED_INCLUDES=['count.posts']` (`:7,:24-30`); C++ step declares `using Input=interfaces::io::IRequest&` and parses no query string (`browse/workflow/step/execute.cppm:11-13`).
- Output diverges: Ghost returns a page of author models with `meta.pagination` from the serializer and `headers.cacheInvalidate:false` (`authors-public.js:20-22,32-38`); C++ emits a hardcoded envelope with `limit:15,page:1,pages:1,prev:null,next:null,total:0` and `Status::OK` (`browse/workflow/step/execute.cppm:17`, `browse/handler.cppm:27`).
- DB call diverges: Ghost `models.Author.findPage(options)` with `mongoTransformer: rejectAuthorsContentApiRestrictedFieldsTransformer` (`authors-public.js:33-37`); C++ `ctx.store().json_for("authors", {})` with empty params — no filtering/pagination (`browse/workflow/step/execute.cppm:16`). Resource name agrees (`Author` ↔ `"authors"`).
- Cache: none on either side (Ghost `:20-22`, C++ step has no `core::cache::Cache` usage).
- Auth diverges: Ghost `mw.authenticatePublic` (`content/routes.js:27`) + `permissions:true` (`authors-public.js:31`); C++ `content_type_middleware` only (`admin/authors/routes.cppm:20`) plus a dead token guard `if (token(req).empty() && !true)` (`browse/handler.cppm:18`) — effectively unauthenticated.

### GET `/authors/:id` — PARTIAL

- Route identity matches: Ghost `content/routes.js:28` vs C++ `Route{"/:id"}.get(...)` nested under `/authors` (`admin/authors/routes.cppm:29-31`).
- Operation matches: `read` (`authors-public.js:41`) vs `handler.authors_read_by_id()` (`admin/authors/routes.cppm:36`).
- Input diverges: Ghost `data: ALLOWED_READ_FIELDS` = `['id','slug']` with `pick(frame.data, ...)` and an explicit missing-identifier `BadRequestError` (`authors-public.js:8,46,58-61`); C++ `Input=IRequest&`, no `:id` extraction (`read_by_id/workflow/step/execute.cppm:11-13`).
- Output diverges: Ghost returns one model or throws `NotFoundError{'Author not found.'}` (`authors-public.js:11,68-73`); C++ returns `{"authors":[...]}` / `{"authors":[]}` with `Status::OK`, no 404/400 (`read_by_id/workflow/step/execute.cppm:17-18`, `read_by_id/handler.cppm:27`).
- DB call diverges materially: Ghost `models.Author.findOne(data, options)` with real id/slug (`authors-public.js:68`); C++ `find_by_resource("authors", std::string{}.empty()?"1":std::string{})` — hardcoded id `"1"`, never reads the route param (`read_by_id/workflow/step/execute.cppm:16`).
- Cache: none on either side.
- Auth diverges: same as browse — `mw.authenticatePublic`/`permissions:true` vs `content_type_middleware` + dead token guard (`admin/authors/routes.cppm:20`, `read_by_id/handler.cppm:18`).

### GET `/authors/slug/:slug` — PARTIAL

- Route identity matches after placeholder normalization: Ghost `/authors/slug/:slug` (`content/routes.js:29`) vs C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`admin/authors/routes.cppm:41-44`). C++ uses token `:id` where Ghost uses `:slug`. Note: C++ constructs the slug sub-router with the outer `router` context and base `"/slug"`, so its effective mount point (`/slug/:id` vs `/authors/slug/:id`) depends on `utils::openapi::ApiRouter` re-basing; recorded as declared.
- Operation matches: `read` (`authors-public.js:41`) vs `handler.authors_read_by_slug()` (`admin/authors/routes.cppm:49`).
- Input diverges: Ghost picks `id,slug` from data with include validation (`authors-public.js:45-46,58`); C++ `Input=IRequest&`, no slug parsed (`read_by_slug/workflow/step/execute.cppm:11-13`).
- Output diverges materially: Ghost returns a single model or 404/400; C++ returns the **browse-shaped** envelope with hardcoded pagination — no single-resource read (`read_by_slug/workflow/step/execute.cppm:17`, vs browse step `browse/workflow/step/execute.cppm:17`).
- DB call diverges: Ghost `models.Author.findOne({slug}, options)`; C++ `ctx.store().json_for("authors", {})` — same unfiltered call as browse, no slug predicate (`read_by_slug/workflow/step/execute.cppm:16`).
- Cache: none on either side.
- Auth diverges: `mw.authenticatePublic`/`permissions:true` vs `content_type_middleware` + dead token guard (`read_by_slug/handler.cppm:18`).

## Missing in C++

None. All three Ghost `authorsPublic` routes have a path+verb C++ counterpart.

Search performed (to confirm/refute the "no counterpart" premise):
- `grep -n -i "author" include/blog/handlers/**` → found `admin/authors/routes.cppm:19,30,41-43` with GET verbs; `include/blog/handlers/routes.cppm:6,79`; `all.cppm:14-16,360-362,609-611`; `base.cppm:24-26`; `BUILD.bazel:26-34,786`.
- `grep -n -i "authors" include/blog/handlers/**` → 114 matches, no separate content-api author folder.
- `grep -n -i "authors" docs/Ghost/ghost/core/server/web/api/endpoints` → only `content/routes.js:27-29` (plus `search-index/authors` which belongs to `searchIndexPublic`, out of scope).
- `glob docs/Ghost/ghost/core/server/api/endpoints/authors*.js` → only `authors-public.js`; no admin authors controller exists, so the C++ `admin/authors` routes cannot be an admin-API counterpart.

## Extra in C++

None in the matched folder (`admin/authors/`). Its three routes (`GET /authors`, `GET /authors/:id`, `GET /authors/slug/:id`) each have a Ghost `authorsPublic` route. The C++ `GET /search-index/authors` route (`include/blog/handlers/discovery/search_index/routes.cppm:22-30`) belongs to the separate `searchIndexPublic.fetchAuthors` domain and is not counted here.
