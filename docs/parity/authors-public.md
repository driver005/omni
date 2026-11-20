# Parity: authors-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/authors-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/admin/authors/routes.cppm` (+ `browse/`, `read_by_id/`, `read_by_slug/` subtrees). NOTE: folder is named `admin/`, but the Ghost monorepo has **no admin authors controller** and C++ installs one flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:67-130`), so these are the true path+verb counterparts of the content API routes.
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-19 (shared abstractions; see below).

> Premise check: the audit brief predicted "no C++ counterpart / all MISSING". Disproven by search — C++ GET routes exist at `/authors`, `/authors/:id`, `/authors/slug/:slug` and are bound to author handlers. Verdicts below are therefore `PARTIAL`, not `MISSING`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Verdicts stay `PARTIAL`: all controller-level Ghost behavior is implemented (mostly via shared abstractions), but router-level `authenticatePublic` sub-middleware (content-API key, rate limit, host/SSL redirect, prettyUrls) and CORS `maxAge` have no shared C++ abstraction.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/authors` | `authorsPublic.browse` (docName `authors`) | GET `/authors` | `authors_browse()` → `blog::handlers::authors::browse::BrowseHandler` | Ghost: options `include,filter,fields,limit,order,page`, `include∈['count.posts']`, restricted filter/order stripping, `name asc` default. C++: `core::query::parse` allowlist + local authors restriction set + `include` validation (400) + `name asc`/slug autoOrder | Ghost: `{authors:[...],meta:{pagination}}`, `cacheInvalidate:false`. C++: same via `blog::mapping::clean/wrap/pagination`, real prev/next, `url` computed, `count.posts` embedded, no `X-Cache-Invalidate` | Ghost: `models.Author.findPage(options)` + mongoTransformer. C++: one `ctx.store().json_for("authors")` + `core::query::apply` (single pass) | Ghost: none. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: `require_ghost_permission("authors","browse")` on CONTENT_API surface + CORS `*` | PARTIAL | Ghost `authors-public.js:19-38`, `content/routes.js:27`; C++ `admin/authors/routes.cppm:19-31`, `browse/handler.cppm:16-23`, `browse/.../execute.cppm` |
| GET `/authors/:id` | `authorsPublic.read` | GET `/authors/:id` | `authors_read_by_id()` → `blog::handlers::authors::read_by_id::ReadByIdHandler` | Ghost: options `include,filter,fields`; data `id,slug` picked; 400/404. C++: `core::query::parse`, id-only predicate, include validation, 400/404 | Ghost: `{authors:[model]}`, no meta. C++: `blog::mapping::wrap("authors", clean.author(model))` | Ghost: `models.Author.findOne({id})` + mongoTransformer. C++: `json_for("authors")` + `core::query::apply` with explicit `id` filter AND user filter | Ghost: none. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: inherited `require_ghost_permission("authors","read")` | PARTIAL | Ghost `authors-public.js:41-76`, `content/routes.js:28`; C++ `admin/authors/routes.cppm:33-42`, `read_by_id/handler.cppm:16-28`, `read_by_id/.../execute.cppm` |
| GET `/authors/slug/:slug` | `authorsPublic.read` | GET `/authors/slug/:slug` (was `/:id`) | `authors_read_by_slug()` → `blog::handlers::authors::read_by_slug::ReadBySlugHandler` | Ghost: same as read (id/slug picked, include validation). C++: `core::query::parse`, slug-only predicate, include validation, 400/404 | Ghost: same as read. C++: `blog::mapping::wrap("authors", clean.author(model))` | Ghost: `models.Author.findOne({slug})`. C++: `json_for("authors")` + explicit `slug` filter | Ghost: none. C++: none | Ghost: `mw.authenticatePublic` + `permissions:true`. C++: inherited `require_ghost_permission("authors","read")` | PARTIAL | Ghost `authors-public.js:41-76`, `content/routes.js:29`; C++ `admin/authors/routes.cppm:43-54`, `read_by_slug/handler.cppm:16-28`, `read_by_slug/.../execute.cppm` |

## Route details

### GET `/authors` — PARTIAL

- Route identity and operation match: Ghost `router.get('/authors', ..., http(api.authorsPublic.browse))` (`content/routes.js:27`) ↔ `Router{router,"/authors"}.get(...)` → `handler.authors_browse()` (`admin/authors/routes.cppm:19-31`). Kept.
- Input now matches Ghost's declared pipeline: `core::query::parse` enforces the allowlist `include,filter,fields,limit,order,page`; `include` values must be `count.posts` (400 `ApiError.validation` otherwise); `detail::strip_restricted` mirrors `rejectAuthorsContentApiRestrictedFieldsTransformer` (`api-filter-utils.ts:9-64,137-141,198-200`) on `filter`/`order` and additionally calls `core::query::has_restricted_field`; `detail::slug_filter_order` ports `input/authors.js:7-15` (`autoOrder` for `slug:[…]`, else `name asc`).
- DB call: one `ctx.store().json_for("authors")` then `core::query::apply` — filtering, ordering, projection and pagination applied once (the prior double-slice from `query_json_impl` + `search_json` is gone). No events/outbox: Ghost's browse has none.
- Output: `detail::author_rows` rebuilds the full `clean.author` allow-list, converts empty `bio` to `null`, computes `url` from request scheme/authority, and adds `count.posts` when requested; `blog::mapping::clean(rows,"author")` + `blog::mapping::wrap("authors", rows, {"pagination":meta})` produce the Ghost envelope. `blog::mapping::pagination` supplies the object; numeric prev/next are restored by `detail::ghost_pagination_links`.
- Auth: permission guard is `blog::handlers::middleware::require_ghost_permission(req,res,next,send,"authors","browse")`; the request is marked `x-blog-context: CONTENT_API` when unset so `core::auth::ghost::GhostAuthorizer` grants the public read and emits a 401/403 Ghost envelope otherwise. `content_type_middleware`/`optional_auth` no-ops were removed. CORS `access-control-allow-origin: *` is set.
- Cache: none on either side; `X-Cache-Invalidate` is no longer written.
- Residual gaps: content-API key/rate-limit, adminSSLAndHostRedirect, prettyUrls, configured CORS `maxAge`, real per-author `count.posts`, configured site URL for `url`, fixture IDs/timestamps.

#### Fix (first pass — superseded by the strict-parity pass below)

- Original Fix blocks parsed the query manually, used empty `{}` store params, fabricated pagination, added a literal `X-Cache-Invalidate:false` header, and kept `optional_auth`. Those edits were applied in the first pass, then replaced wholesale by the shared-abstraction implementation described here.

### GET `/authors/:id` — PARTIAL

- Route identity and operation match (`content/routes.js:28` ↔ `routes.cppm:33-42`; `read` ↔ `authors_read_by_id()`). Kept.
- Input: `core::query::parse(full, {include,filter,fields})`; id = last path segment; include validation and empty-id `BadRequestError` ("An author id or slug is required.", property `id`); restricted filters stripped.
- DB/output: `json_for("authors")` + `core::query::apply` with an explicit `id:<id>` predicate (Ghost `pick(frame.data,['id','slug'])` on the `:id` route supplies only `id`) ANDed with any user filter. Empty → 404 `Author not found.`; otherwise `{authors:[clean.author(model)]}` via `blog::mapping::wrap`, no `meta`.
- Auth/cache: inherited permission guard; no `X-Cache-Invalidate`.
- Residual gaps: `authenticatePublic` sub-chain / CORS `maxAge`; `count.posts` value; request-derived `url`; fixture IDs/timestamps; malformed-NQL SQL errors (no C++ primitive).

#### Fix (first pass — superseded)

- Original Fix blocks used `find_by_resource("authors", id)` / a raw single model as `Output`; the current store has no `"authors"` branch in `find_by_resource`, and Ghost's default serializer returns `{authors:[model]}`, not a bare model. Both points were corrected in the strict-parity pass.

### GET `/authors/slug/:slug` — PARTIAL

- Route identity: placeholder renamed to `/:slug` to match Ghost `content/routes.js:29` (path unchanged). Operation `read` ↔ `authors_read_by_slug()`.
- Input: slug = last path segment; `core::query::parse` with `include,filter,fields`; include validation; empty-slug 400 with property `slug`; restricted filters stripped.
- DB/output: `json_for("authors")` + `core::query::apply` with explicit `slug:<slug>` predicate; empty → 404; otherwise `{authors:[clean.author(model)]}` via `blog::mapping::wrap`, no `meta`.
- Auth/cache: inherited permission guard applied once (the old nested-router `optional_auth` duplication is gone); no `X-Cache-Invalidate`.
- Residual gaps: same as `/authors/:id`.

#### Fix (first pass — superseded)

- Original Fix block used `json_for("authors", {{"filter","slug:"+slug}})` and browse-shaped pagination; replaced by the `core::query` pipeline and single-resource envelope.

## Missing in C++

None. All three Ghost `authorsPublic` routes have a path+verb C++ counterpart (`admin/authors/routes.cppm:19-54`; registry `all.cppm:14-16,360-362,609-611`).

## Extra in C++

None in the matched folder (`admin/authors/`). The `GET /search-index/authors` route (`include/blog/handlers/discovery/search_index/routes.cppm`) belongs to the separate `searchIndexPublic.fetchAuthors` domain and is not counted here.

## Needs central registration

None. All edits were made in place; no new module partitions were created, so no shared-file registration was required (and no shared file was edited by this pass).

Verified existing central registration (read-only, current tree):
- `include/blog/handlers/base.cppm:26-28` — `export import :authors_browse_handler;`, `:authors_read_by_id_handler;`, `:authors_read_by_slug_handler;`; `:6` — `export import :mapping;`.
- `include/blog/handlers/all.cppm:14-16` (imports), `:360-362` (accessors), `:609-611` (handler members).
- `include/blog/handlers/routes.cppm:6` (`export import :authors_routes;`) and `:78` (`AuthorsRoutes{}.install(router, handlers);`).
- `include/blog/handlers/domain_all.cppm:3-21` exports `blog_handlers:context_context` (`BlogContext`, `blog::api::ApiError`, `parse_array`, `record_to_json`) plus the author model.
- `include/blog/handlers/middleware/base.cppm:8` exports `:ghost_permission`, so `require_ghost_permission` and `kGhostContextHeader` reach `admin/authors/routes.cppm`.
- `include/blog/handlers/BUILD.bazel` already depends on `core_query`, `core_auth`, `blog_handlers_middleware`; `blog_handlers:mapping` partitions are already listed.

## Strict-parity implementation (shared abstractions)

Files changed in this pass (all under `include/blog/handlers/admin/authors/`):
- `routes.cppm` — replaced `content_type_middleware`/`optional_auth` with a single Ghost permission middleware (`require_ghost_permission("authors","browse"/"read")`), marks the CONTENT_API surface, sets `access-control-allow-origin: *`, renamed `/:id` → `/:slug` on the slug sub-router, removed duplicated `optional_auth`.
- `browse/handler.cppm`, `read_by_id/handler.cppm`, `read_by_slug/handler.cppm` — unified `{status,body}` map payload; removed the literal `X-Cache-Invalidate:false` header.
- `browse/workflow/step/execute.cppm` — rewritten around `core::query::parse/apply/has_restricted_field`, a local authors restricted-field transformer, `name asc`/slug autoOrder, single store read, and `blog::mapping::clean/wrap/pagination`; exports shared `detail` helpers (restriction set, stripper, author wire mapper, pagination-link normalizer).
- `read_by_id/workflow/step/execute.cppm`, `read_by_slug/workflow/step/execute.cppm` — rewritten around `core::query::parse/apply`, explicit id/slug predicates, include validation, 400/404, and `blog::mapping::clean/wrap`; import `:authors_browse_step` for the shared helpers.

Shared abstractions used:
- `core::query` — allow-listed input parsing, NQL filters, order, fields, single-pass pagination, restricted-field guard.
- `core::auth::ghost` + `blog::handlers::middleware::require_ghost_permission` — Ghost-shaped 401/403 and Content API public pass-through (`permissions:true`).
- `blog::mapping::clean/wrap/pagination` — Ghost `clean.author` allow-list and `{authors:[...],meta.pagination}` envelope.
- `blog_context` store — `json_for("authors")`, `total("authors")`, `total("posts")`.
- `core::outbox` and `core::adapters` — deliberately unused: this controller emits no events and makes no external calls; emitting anything would break strict parity.

Accepted residual gaps (keep `PARTIAL`): `authenticatePublic` sub-middleware (content-API key, rate limit, redirects, prettyUrls) and configured CORS `maxAge`; real per-author `count.posts` (store has no author→posts relation, total posts used as approximation); configured `urlService` base (request-derived with `127.0.0.1:2369` fallback); fixture-style IDs/timestamps from `model_author.cppm`; malformed-NQL → `BadRequestError` (no C++ primitive).
