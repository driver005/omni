# Behavior report: authors-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/authors-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (router `express.Router('content api')` at `:12`, installed as the Content API). Admin table `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` has **no** `authorsPublic` binding (full read of all 616 lines; users at `:125-136`, no authors entry).
- C++ folders: `include/blog/handlers/admin/authors/` (folder name is `admin/` but the routes are installed as the public Content API — see `include/blog/handlers/routes.cppm:11,83`). Subfolders `browse/`, `read_by_id/`, `read_by_slug/` each with `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`.
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19 (shared abstractions wired in; see "Strict-parity implementation" below).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All controller-level Ghost behavior is now implemented through shared abstractions. Verdicts stay `PARTIAL` because router-level `authenticatePublic` sub-middleware (content-API key, rate limit, host/host SSL redirect, prettyUrls) and CORS `maxAge` have no shared C++ abstraction; see residual gaps.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/authors` | `browse`, docName `authors` | `options=['include','filter','fields','limit','order','page']`; validation `include ∈ ['count.posts']`; restricted filter/order clauses stripped; default `order='name asc'`; slug-list `autoOrder` | `permissions:true` public pass-through via `core::auth::ghost` + `require_ghost_permission` on CONTENT_API surface; router CORS `access-control-allow-origin: *` | 200; no `X-Cache-Invalidate`; 400 on invalid include; JSON via `http` | `core::query::parse` → store `json_for("authors")` → `core::query::apply` (one filter/order/page pass) → `blog::mapping::clean/wrap/pagination`; no events | no cache service/key/TTL; header omitted | `{authors:[...], meta:{pagination:{limit,page,pages,total,prev,next}}}`; `clean.author` field allow-list, `url` computed, empty strings → null, `count.posts` embedded | `admin/authors/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | 200/400 via status+body map; `core::query` options/filter/order/pagination/restricted guard; local authors restricted-field transformer; `name asc`/slug autoOrder; single store read; `blog::mapping` output with real prev/next; no `X-Cache-Invalidate`; Ghost permission guard | PARTIAL | Ghost `authors-public.js:17-39`, `content/routes.js:14,27`, `content/middleware.js:18-25`, `api-filter-utils.ts:9-21,44-64,137-141,198-200,303-311`, `input/authors.js:7-27`, `output/default.js:22-41`, `mappers/users.js:11-38`, `clean.js:27-78`, `models/base/plugins/crud.js:118-211`; C++ `routes.cppm:19-32`, `browse/handler.cppm:16-23`, `browse/.../execute.cppm:1-300` |
| GET `/authors/:id` | `read`, docName `authors` | `data=['id','slug']`; `options=['include','filter','fields']`; validation `include ∈ ['count.posts']`; id-only lookup from path | same public guard as above (`require_ghost_permission("authors","read")`) | 200; no `X-Cache-Invalidate`; 400 on missing id/invalid include; 404 on not found | `core::query::parse` → `core::query::apply` with explicit `id` filter AND user filter → `blog::mapping::clean/wrap`; restricted filter clauses stripped | no cache service | `{authors:[model]}` (single-element array, no `meta`) via `blog::mapping::wrap("authors", …)` + `clean.author`; `url` computed; 404 `ApiError.json` | `admin/authors/routes.cppm`, `read_by_id/handler.cppm`, `read_by_id/workflow/flow/main.cppm`, `read_by_id/workflow/step/execute.cppm` | 400/404/200 via status map; id from last path segment (id-only, not slug); `core::query` restricted guard + mapper; no `X-Cache-Invalidate`; Ghost permission guard | PARTIAL | Ghost `authors-public.js:41-77`, `content/routes.js:28`, `models/base/plugins/crud.js:220-248`, `output/default.js:32-36`; C++ `routes.cppm:33-42`, `read_by_id/handler.cppm:16-28`, `read_by_id/.../execute.cppm:1-90` |
| GET `/authors/slug/:slug` | `read`, docName `authors` | `data=['id','slug']`; path param `slug`; `options=['include','filter','fields']`; include validated | same public chain as above | 200; no `X-Cache-Invalidate`; 400/404 as read | same `read` pipeline with explicit `slug` filter | no cache service | `{authors:[model]}` via `blog::mapping::wrap("authors", …)` + `clean.author`; `url` computed; no `meta` | `admin/authors/routes.cppm`, `read_by_slug/handler.cppm`, `read_by_slug/workflow/flow/main.cppm`, `read_by_slug/workflow/step/execute.cppm` | 400/404/200; slug from last path segment (slug-only); `core::query` restricted guard + mapper; no `X-Cache-Invalidate`; Ghost permission guard | PARTIAL | Ghost `authors-public.js:41-77`, `content/routes.js:29`; C++ `routes.cppm:43-54`, `read_by_slug/handler.cppm:16-28`, `read_by_slug/.../execute.cppm:1-90` |

## Route details

### GET /authors — PARTIAL

**Ghost** (numbered facts 1–8):

1. **Identity**: `router.get('/authors', mw.authenticatePublic, http(api.authorsPublic.browse))` at `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:27`; router named `content api` at `:12`; router-level `router.use(cors({maxAge: config.get('caching:cors:maxAge')}))` at `:14`. `api.authorsPublic` resolves via `apiFramework.pipeline(require('./authors-public'), localUtils, 'content')` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:315-317`. No admin binding.
2. **Operation**: exported key `browse`; `docName:'authors'` (`authors-public.js:17`); `query(frame)` at `:32-38` returns `models.Author.findPage(options)` (`:37`).
3. **Input**: `options: ['include','filter','fields','limit','order','page']` (`:23`); `validation.options.include.values = ALLOWED_INCLUDES = ['count.posts']` (`:7,24-30`); no `data[]`, no multipart, no direct header reads. Input serializer `input/authors.js` runs `url.forceUrlColumns(frame,'authors')` (`:21`), then for Content API `rejectAuthorsRestrictedOrderFields(frame.options.order)` (`:24`) and `setDefaultOrder(frame)` (`:25`), which sets `autoOrder` from `slugFilterOrder` when a `filter` is present (`:8-10`) and otherwise `order='name asc'` (`:12-14`).
4. **Auth/permissions**: route middleware `mw.authenticatePublic` (`content/routes.js:27`) = `[contentApiKey rate limit, authenticateContentApi, authorizeContentApi, cors(), adminSSLAndHostRedirect, prettyUrls]` (`content/middleware.js:18-25`). Controller `permissions:true` (`authors-public.js:31`) → public pass-through. No `authAdminApi`, no `unsafeAttrs`, no labs flag.
5. **Status + headers**: no `statusCode` → default 200. `headers.cacheInvalidate:false` (`authors-public.js:20-22`) makes `headers.get` skip `X-Cache-Invalidate` entirely (`docs/Ghost/packages/api-framework/src/headers.ts:135-144`); no disposition/Location/stream. Response JSON via `http` (`http.ts`).
6. **Full call chain**: `query` → `models.Author.findPage(options)`. `findPage` at `docs/Ghost/ghost/core/server/models/base/plugins/crud.js:118-211`: `filterOptions`, `getFilteredCollection`, `parseOrderOption`, `fetchPage`, pagination meta. `mongoTransformer` (`rejectAuthorsContentApiRestrictedFieldsTransformer`, `api-filter-utils.ts:198-200`) rejects `CONTENT_API_RESTRICTED_FIELDS`, restricted relations and `CONTENT_API_USER_RESTRICTED_FIELDS` for bare author fields (`:9-21,44-64,128-141`). No service layer, no events/jobs/mail/webhooks.
7. **Cache**: no cache service/key/TTL in controller or framework for this read; `cacheInvalidate:false` deliberately omits `X-Cache-Invalidate`.
8. **Output**: no resource-specific output serializer → `default.all` → `mappers.authors` → `mappers.users` (`output/utils/url.js:87-92` adds absolute `url`) → `clean.author` (`clean.js:27-78` strips created_at/updated_at/last_seen/status/email/notification flags/accessibility/tour/visibility/locale and nulls empty strings). Envelope `{authors:[...], meta:{pagination}}`. Empty browse → `{authors:[], meta:{pagination:{...}}}`.

**C++** (numbered facts):

1. **Files/symbols**: `AuthorsRoutes::install` at `include/blog/handlers/admin/authors/routes.cppm:14-56`, called from `include/blog/handlers/routes.cppm:11,83`. `Router{router,"/authors"}` installs a single permission middleware then `.get(...)` → `handler.authors_browse().handle(...)` (`routes.cppm:19-31`). Handler `blog::handlers::authors::browse::BrowseHandler` (`browse/handler.cppm:9-25`), exposed via `BlogHandlers::authors_browse()` (`all.cppm:390`, member `:671`). Flow `MainFlow::build` at `browse/workflow/flow/main.cppm:7-10`; step `ExecuteStep` at `browse/workflow/step/execute.cppm`.
2. **Input**: `Input=interfaces::io::IRequest&`, `Output=std::map<std::string,std::string>` (status+body, so invalid include can return 400). `core::query::parse(path, kAllowedBrowseOptions)` allowlists exactly `include,filter,fields,limit,order,page`; `q.include` values must all equal `count.posts` else 400 `ApiError.validation`. `detail::strip_restricted` applies the authors Content API restriction set (mirrors `api-filter-utils.ts`) plus `core::query::has_restricted_field`; restricted `filter[field]` pairs are erased. `detail::slug_filter_order` implements `slugFilterOrder('users', filter)`; otherwise `query.order = "name asc"`.
3. **Status/body/header writes**: `BrowseHandler::handle` runs the workflow, reads `{status,body}`, converts body to bytes, sets status and body, `send()` (`browse/handler.cppm:16-23`). No `X-Cache-Invalidate` write (Ghost `cacheInvalidate:false` omits it). Middleware sets `access-control-allow-origin: *` and delegates authz to `blog::handlers::middleware::require_ghost_permission("authors","browse")`; denial writes a Ghost 401/403 envelope via `write_error`.
4. **`ctx.store()` calls / cache**: one read `ctx.store().json_for("authors")` with no params (`browse/.../execute.cppm`) — all filtering/order/pagination happens once in `core::query::apply`; no double slice. `ctx.store().total("posts")` feeds the `count.posts` approximation. No `core::cache::Cache` usage. No outbox events: Ghost's browse emits none.
5. **Side effects**: none at the store level (read-only); in-process workflow bookkeeping via `LocalWorkflowDriver` only. No outbox records, no fixture writes, no cache markers.
6. **Output**: `detail::author_rows` rebuilds each record with the full `clean.author` allow-list, converts empty `bio` to `null`, computes `url` from request scheme/authority (`/author/<slug>/`), and adds `count:{posts}` when `include=count.posts`; result passes through `blog::mapping::clean(rows,"author")` then `blog::mapping::wrap("authors", rows, {"pagination":meta})`. `meta` uses `blog::mapping::pagination` with numeric prev/next restored by `detail::ghost_pagination_links`.
7. **Implemented Ghost behaviors**: allow-listed options, include validation, restricted-field transformer (filter + order), `name asc` default + slug autoOrder, single-pass pagination with real prev/next, `clean.author` field set, computed `url`, null normalization, `count.posts` embedding, public permission pass-through, omitted cache-invalidate header.
8. **Residual gaps**: content-API key auth + rate limit, `adminSSLAndHostRedirect`, `prettyUrls`, CORS `maxAge` config (only `*` is emitted); `count.posts` value is the total post count because the local store has no author→posts relation; `url` base comes from request scheme/authority with a `127.0.0.1:2369` fallback rather than configured `urlService`; IDs/timestamps remain fixture-style from `model_author.cppm`.

**Diff**:

- **Divergences remaining (C++ differs from Ghost)**: middleware chain (no content-API key/rate-limit/redirect/prettyUrls; CORS `maxAge` absent); `count.posts` uses a store-wide approximation; `url` base is request-derived; fixture IDs/timestamps.
- **Closed divergences from the prior audit**: output field set now matches `clean.author` (no `email`/`created_at`/`updated_at` leak; `bio` empty → `null`; `url` added); `X-Cache-Invalidate` header is no longer emitted; pagination `prev`/`next` are derived from `page`/`pages` instead of hardcoded null; `include=count.posts` is embedded; default order and restricted-field guards are applied; restricted filtering/ordering by withheld columns is blocked; pagination is applied exactly once.

### GET /authors/:id — PARTIAL

**Ghost**:

1. **Identity**: `router.get('/authors/:id', mw.authenticatePublic, http(api.authorsPublic.read))` at `content/routes.js:28`; same router-level chain as browse.
2. **Operation**: key `read`, docName `authors` (`authors-public.js:17,41`); `async query(frame)` at `:55-76`.
3. **Input**: `data: ['id','slug']` (`ALLOWED_READ_FIELDS`, `:8,46`); `options: ['include','filter','fields']` (`:45`); `validation.include ∈ ['count.posts']` (`:47-53`). The controller re-picks `frame.data` with `pick(frame.data, ALLOWED_READ_FIELDS)` (`:58`); if no truthy `id`/`slug`, `BadRequestError('An author id or slug is required.')` (`:59-61`). Path param `:id` supplies `frame.data.id`.
4. **Auth/permissions**: `mw.authenticatePublic`; `permissions:true` public pass-through; no admin auth/labs/unsafeAttrs.
5. **Status + headers**: default 200; no `X-Cache-Invalidate`; 400 for missing identifier; 404 `NotFoundError('Author not found.')` (`:69-73`).
6. **Full call chain**: `models.Author.findOne(data, options)` (`authors-public.js:68`) → `crud.js:220-248` with the same restricted-field transformer. No events/jobs/mail/webhooks.
7. **Cache**: none; `cacheInvalidate:false` omits `X-Cache-Invalidate`.
8. **Output**: single model → `default.all` else branch `frame.response[docName] = [mapResponse(...)]` (`output/default.js:32-36`) → `mappers.authors`/`users` + `clean.author`; no `meta`.

**C++**:

1. **Files/symbols**: route `Route{"/:id"}.get(...)` at `admin/authors/routes.cppm:33-42` → `handler.authors_read_by_id().handle(...)`; handler `ReadByIdHandler` at `read_by_id/handler.cppm:9-30`; flow `MainFlow::build` at `read_by_id/workflow/flow/main.cppm:7-10`; step `ExecuteStep` at `read_by_id/workflow/step/execute.cppm`.
2. **Input**: `Input=IRequest&`, `Output=std::map<std::string,std::string>`; `id` = last non-slash path segment; `core::query::parse(full, kAllowedReadOptions)` allowlists `include,filter,fields`; include validation → 400; empty id → 400 with Ghost message and property `id`. Restricted filter clauses are stripped by the shared `detail::sanitize`.
3. **Status/body/header**: handler reads `{status,body}`, sets status/body, `send()` (`read_by_id/handler.cppm:16-28`). No `X-Cache-Invalidate`. Router-level Ghost permission guard applies.
4. **`ctx.store()`**: `ctx.store().json_for("authors")` once; `core::query::apply(wire, find)` with an explicit `id:<id>` predicate ANDed into every user-filter OR group (`core::query` explicit filters). Empty result → 404 `Author not found.`; otherwise `blog::mapping::wrap("authors", record_to_json(first))` → `{authors:[model]}`, no `meta`.
5. **Side effects**: none (read-only); workflow bookkeeping only; no outbox events.
6. **Output**: `detail::author_rows` applies the `clean.author` allow-list, computed `url`, null normalization and optional `count.posts`; `blog::mapping::clean` is invoked inside `author_rows` and `blog::mapping::wrap` builds the single-element envelope.
7. **Implemented Ghost behaviors**: id-only lookup (no slug fallback on this route), 400/404/200 statuses, `pick`-equivalent field restriction, include validation, restricted-field transformer, `clean.author` wire, computed `url`, `{authors:[model]}` envelope, public permission guard.
8. **Residual gaps**: same middleware (`authenticatePublic` sub-chain / CORS `maxAge`), `count.posts` approximation, request-derived `url` and fixture IDs/timestamps as browse.

**Diff**:

- **Closed divergences**: lookup is now id-only via an explicit `id` predicate (was `id,id+slug` OR); output is now `{authors:[model]}` with `clean.author` fields and `url` (was a raw model with leaked fields); 404/400 are emitted; `X-Cache-Invalidate` is omitted; permission guard added.
- **Residual gaps**: content-API key/rate-limit/redirect/prettyUrls and CORS `maxAge`; `count.posts` value; request-derived `url`; fixture IDs/timestamps. SQL-invalid `order`/`filter` → `BadRequestError` has no C++ equivalent (the shared `core::query` treats malformed NQL as no filter).

### GET /authors/slug/:slug — PARTIAL

**Ghost**:

1. **Identity**: `router.get('/authors/slug/:slug', mw.authenticatePublic, http(api.authorsPublic.read))` at `content/routes.js:29`; same router chain.
2. **Operation**: `read`, docName `authors` (`authors-public.js:17,41`).
3. **Input**: `data=['id','slug']`; path param `:slug`; `pick` at `:58`; missing identifier 400; `options=['include','filter','fields']` with include validation.
4. **Auth/permissions**: `mw.authenticatePublic`; `permissions:true`.
5. **Status/headers**: 200 / 400 / 404 as `/authors/:id`; no `X-Cache-Invalidate`.
6. **Full call chain**: `models.Author.findOne({slug}, options)` → `crud.js:220-248`.
7. **Cache**: none.
8. **Output**: `{authors:[model]}` via default serializer + `mappers.authors`/`users`/`clean.author`; `url` computed; no `meta`.

**C++**:

1. **Files/symbols**: nested `Router{router, "/slug"}` with `.add_route(Route{"/:slug"}.get(...))` → `handler.authors_read_by_slug().handle(...)` at `admin/authors/routes.cppm:43-54`. Handler `ReadBySlugHandler` at `read_by_slug/handler.cppm:9-30`; flow `MainFlow::build` at `read_by_slug/workflow/flow/main.cppm:7-10`; step `ExecuteStep` at `read_by_slug/workflow/step/execute.cppm`.
2. **Input**: slug = last path segment; `core::query::parse(full, kAllowedReadOptions)`; include validation → 400; empty slug → 400 with property `slug`; `detail::sanitize` strips restricted filters.
3. **Status/body/header**: identical to read_by_id handler (status map, no cache-invalidate header, `send()`). Permission guard is inherited from the `/authors` router (single application, not twice).
4. **`ctx.store()`**: `ctx.store().json_for("authors")` once; explicit `slug:<slug>` predicate ANDed into user filters; empty → 404; otherwise `blog::mapping::wrap("authors", record_to_json(first))`.
5. **Side effects**: none.
6. **Output**: first matching record cleaned by `detail::author_rows` → `blog::mapping::clean` → `blog::mapping::wrap("authors", …)`; no `meta`.
7. **Implemented Ghost behaviors**: slug-only lookup, 400/404/200, include validation, restricted-field transformer, `clean.author` wire, computed `url`, single-resource envelope, public permission guard.
8. **Residual gaps**: same as read_by_id.

**Diff**:

- Path parameter now named `:slug` internally (was `:id`), matching Ghost `content/routes.js:29`; observable path unchanged.
- Closed divergences: slug-only `findOne` predicate, single-resource envelope, no `meta`, `clean.author` field set with `url`, 400/404 statuses, omitted cache header, no duplicated `optional_auth`, permission guard added.
- Residual gaps: same middleware / CORS / `count.posts` / `url` / fixture-ID gaps as the other two routes.

## Module-level notes

- `authors-public.js` exposes only `browse` and `read`; there is no `add`/`edit`/`destroy`. All three Content API routes map to these two operations. No admin route binds this controller; the C++ `admin/authors` folder is a naming artifact — `include/blog/handlers/routes.cppm:11,83` installs it once with public-style middleware.
- All three handlers share `LocalWorkflowDriver` + single-step `MainFlow`, a static `BlogContext* s_ctx`, `Output = std::map<std::string,std::string>` with `{status,body}`, and `ApiError`-based validation envelopes. No outbox records, no fixture writes, no `core::cache::Cache` keys/tags/TTL: correct for these read-only Ghost endpoints.
- Shared abstractions now used in this folder:
  - `core::query` (`core_query`): `parse` (allow-listed options), `has_restricted_field` (restricted guard), `apply` (filter/order/fields/single-pass pagination), `ParsedQuery`.
  - `core::auth::ghost` via `blog::handlers::middleware::require_ghost_permission`: Ghost-shaped 401/403 envelope and `permissions:true` public pass-through for the CONTENT_API surface.
  - `blog::mapping` (`clean`, `wrap`, `pagination`): Ghost `clean.author` allow-list, `{authors:[...]}` envelope with optional `meta.pagination`.
  - `core::outbox` / `core_adapters`: intentionally unused — Ghost's authors-public controller emits no events and makes no third-party calls.
  - `blog_context` store: `json_for("authors")`, `total("authors")`, `total("posts")`.
- Ghost's shared behaviors across routes are now covered: public permission pass-through, omitted `X-Cache-Invalidate`, restricted-field transformer on `filter`/`order`, default `name asc`, `url.forceUrlColumns` + computed `url`, and `clean.author` stripping. `include=count.posts` is embedded (value approximated by the store).
- Verdicts remain `PARTIAL` solely for router-level middleware pieces not provided by any shared C++ abstraction (content-API key validation, rate limiting, host/SSL redirects, prettyUrls, configured CORS `maxAge`) and for fixture-data limitations outside this folder (IDs/timestamps, real per-author post counts, configured site URL).
