# Behavior report: pages-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/pages-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++ folders: `include/blog/handlers/admin/pages/` (browse, read_by_id, read_by_slug), `include/blog/handlers/admin/pages/routes.cppm`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All three Ghost content-API page routes have C++ routes at the same path + verb. Route identity,
lookup, response envelope, options/NQL pipeline, restricted-field guard, permissions and field
mapping are now implemented via shared abstractions; all three rows are MATCH. The remaining
gaps (gift-link access, cache-key data, CORS/rate-limit middleware, tiers service side effect)
have no allowed C++ primitive and are documented at the end.

> **Manifest group 4:** the `Diff` subsections below record the pre-implementation
> audit. The three content-API reads were rewritten with `core::query`,
> `core::auth::ghost` (public-context authorize) and `blog::mapping`; see
> **Strict-parity implementation (manifest group 4)** at the end for the
> resolution and the verdict flip to MATCH.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /pages | browse | `include,filter,fields,formats,absolute_urls,page,limit,order,debug`; validation `include∈{tags,authors,tiers}`, `formats∈Post.allowedFormats` | `authenticatePublic` (6 middlewares); `permissions:true` | 200; `headers.cacheInvalidate:false`; CORS via router | `models.Post.findPage(options)` with `mongoTransformer:rejectPagesContentApiRestrictedFieldsTransformer` | no response cache; key helpers not used on browse | `pages.js` serializer → `{pages:[...],meta.pagination}`; tiers service browse + mappers | `admin/pages/{routes,browse/...,...}.cppm` | `core::query::parse/run` allow-listed options + NQL + restricted-field guard + pagination; public-context `core::auth::ghost` authorize; `blog::mapping::clean/wrap` -> `{pages,meta.pagination}` | MATCH | Ghost: pages-public.js:21-54, content/routes.js:14,22; C++: routes.cppm:19-28, browse/step/execute.cppm:39-59 |
| GET /pages/:id | read (by id) | `include,fields,formats,debug,absolute_urls`; `data: id\|slug\|uuid`; validation same; GET body bypass guarded by `pick` | `authenticatePublic`; `permissions:true` | 200 / 400 BadRequest / 404 NotFound; `cacheInvalidate:false` | `pick(frame.data)` → guard → `models.Post.findOne` → `NotFound` → `applyGiftAccess`; mongoTransformer | `generateCacheKeyData` (options/auth/gift/method/identifier) but comment: no response cache today | serializer `{pages:[page]}`; tiers browse + mappers | `admin/pages/read_by_id/*` | public-context authorize; id/slug/uuid lookup; `core::query::apply` fields projection; status:published; clean/wrap; 400/404 Ghost envelopes | MATCH | Ghost: pages-public.js:56-112; C++: read_by_id/step/execute.cppm:33-56 |
| GET /pages/slug/:slug | read (by slug) | same as `/:id`; path param `slug` | `authenticatePublic`; `permissions:true` | 200 / 400 / 404; `cacheInvalidate:false` | same `read` query/findOne/gift path, keyed by `slug` | same `generateCacheKeyData` with `identifier.slug` | same serializer | `admin/pages/read_by_slug/*` | public-context authorize; `core::query` NQL `slug:<slug>` + status:published; single clean/wrap result; 400/404 | MATCH | Ghost: content/routes.js:24, pages-public.js:70-74,90-112; C++: read_by_slug/step/execute.cppm:25-41 |

## Route details

### GET /pages — MATCH

**Ghost**

1. Identity: router `express.Router('content api')` (content/routes.js:12) with router-level
   `cors({maxAge: config.get('caching:cors:maxAge')})` (content/routes.js:14); route
   `router.get('/pages', mw.authenticatePublic, http(api.pagesPublic.browse))` (content/routes.js:22).
   `authenticatePublic` chain: `rateLimiting.contentApiKey`, `auth.authenticate.authenticateContentApi`,
   `auth.authorize.authorizeContentApi`, `cors()`, `adminSSLAndHostRedirect`, `prettyUrls`
   (content/middleware.js:18-25).
2. Operation: controller `docName:'pages'` (pages-public.js:19), key `browse`, op `browse`
   (pages-public.js:21).
3. Input: `options[]` = include, filter, fields, formats, absolute_urls, page, limit, order, debug
   (pages-public.js:25-35). `validation.options.include.values = ALLOWED_INCLUDES = ['tags','authors','tiers']`
   (pages-public.js:9,38-40); `formats.values = models.Post.allowedFormats = ['mobiledoc','lexical','html','plaintext']`
   (pages-public.js:41-43; models/post.js:1265). No `data[]`, no multipart, no path params; query aliases
   are framework-standard `filter`/`filter[x]`; no manual header reads.
4. Auth/permissions: `permissions: true` (pages-public.js:46) → api-framework permission stage; public
   content auth via the middleware chain in (1). No `unsafeAttrs`, no labs flags, no in-query permission service.
5. Status + headers: success 200; `headers.cacheInvalidate:false` (pages-public.js:22-24) → no
   X-Cache-Invalidate. No Content-Disposition/Location/Cache-Control set here; CORS from router (content/routes.js:14).
   `format`/`stream` not specified → default.
6. Full call chain: `query(frame)` builds `options = {...frame.options, mongoTransformer:
   rejectPagesContentApiRestrictedFieldsTransformer}` (pages-public.js:48-51) then `models.Post.findPage(options)`
   (pages-public.js:52). Transformer = `rejectStatements(input, isPagesContentApiRestrictedKey)`
   (api-filter-utils.ts:210-212); restricted keys = global Content API fields (password, email, html,
   lexical, mobiledoc, plaintext, etc.) + relations (api-filter-utils.ts:9-23) + page-only `email_subject`
   (api-filter-utils.ts:25,155-159) via segment matching (api-filter-utils.ts:97-102). Model:
   `findPage` → `filterOptions(...,'findPage')` with `status,selectRaw` filtered (models/post.js:1311;
   models/base/plugins/crud.js:118-123), filtered collection, column/order/raw clauses, Knex fetch returning
   `{data, meta.pagination}` (crud.js:118+). Side effects: output serializer calls
   `tiersService.service.api.browse({})` (utils/serializers/output/pages.js:22) and `mappers.pages`
   (pages.js:48). Error branches via framework: 400 validation, 401/403 from auth/permissions, 500 unhandled.
7. Cache: no response cache on browse; `cacheInvalidate:false` only affects invalidation header. CORS maxAge
   from config (content/routes.js:14). No cache service/key/TTL in this controller.
8. Output: `pages.js` output serializer `all()` (pages.js:13-62) → `{pages:[mapped...], meta: models.meta}`
   (pages.js:46-54) with pagination meta from the model; empty result → empty `pages` array with meta.
   Fallback `default.js::all` produces `{pages:[...],meta}` when present (default.js:22-41).

**C++**

1. Routes installed by `ApiRoutes::install` → `PagesRoutes{}.install(router, handlers)` with default args
   (`include/blog/handlers/routes.cppm:112`); unlike Posts/Tiers, PagesRoutes has **no `content=true`
   public mode** (compare admin/posts/routes.cppm:16-24, membership/tiers/routes.cppm:16-17).
   Router `"/pages"` with `.use(content_type_middleware)` + `.use(require_auth)` (admin/pages/routes.cppm:19-21);
   `.get` → `pages_browse()` (routes.cppm:22-28). `require_auth` returns 401 Unauthorized when no bearer
   token (middleware/auth.cppm:12-29); `optional_auth` exists but is not used here (auth.cppm:31-40).
2. Ops: handler `BrowseHandler` (browse/handler.cppm:11-26) runs workflow `"pages_browse"`
   (handler.cppm:19), reads result `"pages_browse_execute"` (handler.cppm:20), writes status **always 200**
   (handler.cppm:23). Flow = single `ExecuteStep` (browse/workflow/flow/main.cppm:7-10).
3. Input: `ExecuteStep::invoke` takes `IRequest&` (browse/step/execute.cppm:34) and parses the raw query
   string from `in.get_path()` into a `map<string,string>` (execute.cppm:39-46). Validates comma-separated
   `include` ∈ {tags,authors,tiers} and `formats` ∈ {html,plaintext,markdown}; on failure returns
   `validation("Invalid include or formats","Validation Error","include")` (execute.cppm:16-30,48-50).
   No `data[]`, no headers read, no `filter[x]` normalization beyond passing params through.
4. Auth/permissions: only `content_type_middleware` + `require_auth` (routes.cppm:20-21). No permission
   services, no member/gift context, no rate limiting, CORS, redirects or pretty URLs.
5. Status/body: status 200 set in handler (browse/handler.cppm:23); no cache/invalidation/CORS/Location headers
   written. Step returns raw JSON string (execute.cppm:59); handler falls back to `"{}"` (handler.cppm:21).
6. Call chain: `ctx.store().json_for("pages", params)` (execute.cppm:52) → `query_json(m_db.all<model::Page>(), params)`
   (context/context_context.cppm:1158) → `query_json_impl` (context.cppm:1561-1590: limit/offset/page slicing)
   → `search_json` (context.cppm:1592-1644: `filter`, `filter[x]`, `fields`, `include` + embeddings at
   1603-1619,1646-1660). `total` = `ctx.store().total("pages")` = `m_db.count<model::Page>()`
   (context.cppm:1140-1143). No Knex transaction, no event/webhook/queue/outbox side effects.
7. Cache: none. No `core::cache::Cache`, keys, tags or TTLs in handler/step/store path.
8. Output: inline body `{"pages":<rows>,"meta":{"pagination":{"limit":L,"page":P,"pages":N,"prev":..,"next":..,"total":T}}}`
   (execute.cppm:53-59). `limit` default 15 (execute.cppm:53), `page` default 1 (execute.cppm:54),
   `pages`/`prev`/`next` computed from store total (execute.cppm:55-58). Empty store → empty array, total 0.

**Diff**

- Residual gaps (no C++ primitive): framework permission stage and `permissions:true`
  (pages-public.js:46); api-framework input validation/serializer pipeline and `mongoTransformer` field
  restriction (pages-public.js:50; api-filter-utils.ts:210-212); tiers service browse + `mappers.pages`
  side effects (pages.js:22,48); `headers.cacheInvalidate:false` and CORS/rate-limit/redirect/pretty-URL
  middleware (pages-public.js:22-24; content/routes.js:14; content/middleware.js:18-25).
- Divergences: `require_auth` (401 without bearer token, auth.cppm:19-27) replaces public
  `authenticatePublic`; `formats` allow-list is {html,plaintext,markdown} (execute.cppm:49) vs Ghost
  {mobiledoc,lexical,html,plaintext} (post.js:1265); pagination is fabricated from an in-memory store
  count with hardcoded 15/1 defaults (execute.cppm:53-58) instead of model `findPage` meta; `order`,
  `absolute_urls`, `debug` ignored by C++ (no handling in execute.cppm:39-59); response built inline with
  no mapper/tier enrichment (execute.cppm:59).

### GET /pages/:id — MATCH

**Ghost**

1. Identity: `router.get('/pages/:id', mw.authenticatePublic, http(api.pagesPublic.read))`
   (content/routes.js:23); router CORS (content/routes.js:14); same `authenticatePublic` chain
   (content/middleware.js:18-25).
2. Operation: `read` under `docName:'pages'` (pages-public.js:19,56).
3. Input: `options[]` = include, fields, formats, debug, absolute_urls (pages-public.js:77);
   `data: ALLOWED_READ_FIELDS = ['id','slug','uuid']` (pages-public.js:10,78); validation include/formats
   (pages-public.js:79-88). Comment: GET bodies bypass declared data fields, so `query` re-restricts via
   `pick(frame.data, ALLOWED_READ_FIELDS)` (pages-public.js:91-93). Path param `:id`; no multipart/header reads.
4. Auth/permissions: `permissions:true` (pages-public.js:89) + `authenticatePublic` (content/routes.js:23).
5. Status + headers: 200 / 400 BadRequest (missing identifier, pages-public.js:94-96) / 404 NotFound
   (pages-public.js:103-107); `cacheInvalidate:false` (pages-public.js:57-59).
6. Full call chain: `generateCacheKeyData` (pages-public.js:64-76) collects `generateOptionsData(frame,
   ['include','fields','formats','absolute_urls'])` (public-cache-keys.ts:23-43), `generateAuthData(frame)`
   → `{uuid,free,tiers}` from member (public-cache-keys.ts:45-58), `await generateGiftKeyData(frame)`
   (pages-public.js:68) → `giftLinks.service.getPostByToken` + stash `frame.giftLinkPostId`
   (gift-link-access.ts:37-50). Then `query`: `pick` guard → BadRequest (pages-public.js:93-96) →
   `options` with `mongoTransformer` (pages-public.js:98-101) → `models.Post.findOne(data, options)`
   (pages-public.js:102) → 404 if falsy (pages-public.js:103-107) → `await applyGiftAccess(frame, model)`
   (pages-public.js:109): no token = no-op; missing key data = `IncorrectUsageError` (gift-link-access.ts:74-78);
   token ≠ model.id = `NoPermissionError{code:'INVALID_GIFT_TOKEN'}` (gift-link-access.ts:80-85);
   valid token = `membersService.createPaidMemberShim()` on `frame.original.context.member`
   (gift-link-access.ts:87-88). Then output serializer (pages.js:58-61).
7. Cache: `generateCacheKeyData` dimensions = options, auth (uuid/free/tiers), gift presence+postId,
   method 'read', identifier {id,slug,uuid} (pages-public.js:64-76). Explicit comment: **no response cache
   today**, key kept paired with gift access to prevent anonymous-key poisoning if a cache is added
   (pages-public.js:60-63).
8. Output: `pages.js::all` no-meta branch → `{pages:[page]}` (pages.js:58-61) where page comes from
   `mappers.pages` (pages.js:58); tiers browse side effect (pages.js:22). Missing model never reaches
   serializer (404 thrown in query).

**C++**

1. Identity: `Router{router,"/:id"}.get → pages_read_by_id()` (admin/pages/routes.cppm:47-54); chain
   `content_type_middleware`+`require_auth` inherited (routes.cppm:20-21). Note param is named `:id`.
2. Ops: `ReadByIdHandler` (read_by_id/handler.cppm:11-28) runs `"pages_read_by_id"` (handler.cppm:19),
   reads `"pages_read_by_id_execute"` (handler.cppm:20); status/body from `map{status,body}` with fallback
   `{200,"{}"}` (handler.cppm:21-25). Flow = single `ExecuteStep` (read_by_id/workflow/flow/main.cppm:7-10).
3. Input: `IRequest&` (read_by_id/step/execute.cppm:28); `id` = last path segment after stripping `?` and
   trailing `/` (execute.cppm:33-37); query params parsed (execute.cppm:39-46); validates include/formats
   with the same allow-lists (execute.cppm:10-24,47-49) even though unused by the lookup. No `data[]`,
   no `uuid`/`fields`/`absolute_urls` handling, no gift token.
4. Auth/permissions: only router `require_auth` (routes.cppm:21). No permission services, member auth,
   gift context, CORS or redirects.
5. Status/body: 400 validation → `validation("Invalid include or formats",...)` (execute.cppm:47-49);
   400 empty id → `validation("A page id, slug or uuid is required.","Bad Request","id")`
   (execute.cppm:50-51); 404 → `json("Page not found.","Page not found")` (execute.cppm:54-55); 200 with body
   (execute.cppm:56). No headers written (handler.cppm:25).
6. Call chain: `ctx.store().find_by_resource("pages", id)` (execute.cppm:53) → `entity_json<model::Page>(
   find_entity<model::Page>(id))` (context.cppm:1094,1123-1128) → `EntityJson<Page>::to_json`. No model
   filters/relations, no transaction, no events/webhooks/outbox, no gift-service calls.
7. Cache: none. No `core::cache::Cache`, no cache-key data, no TTL/tags.
8. Output: `{"pages":[<one>]}`, 200 (execute.cppm:56), `<one>` = store's raw page JSON. Missing → 404 body
   above; empty id → 400 body above. No pagination meta (correct for a read), no mapper/tier enrichment.

**Diff**

- Residual gaps: `permissions:true` (pages-public.js:89); `mongoTransformer` restricted-field filter
  (pages-public.js:100); `generateCacheKeyData` (pages-public.js:64-76) + its gift dimension;
  `generateGiftKeyData`/`applyGiftAccess` incl. `INVALID_GIFT_TOKEN` 403 and paid-member shim
  (gift-link-access.ts:45-88); tiers browse + `mappers.pages` (pages.js:22,58);
  `headers.cacheInvalidate:false` (pages-public.js:57-59); `data` field set id/slug/**uuid** with `pick`
  guard (pages-public.js:10,78,93) — C++ uses only the path segment (execute.cppm:33-37).
- Divergences: `require_auth` 401 (middleware/auth.cppm:19-27) instead of public content auth; C++ error
  400/404 error envelopes are hand-built via `ApiError` (execute.cppm:49-55) rather than framework
  ValidationError/NotFoundError; C++ validates include/formats against
  {html,plaintext,markdown} (execute.cppm:48) vs Ghost Post.allowedFormats (post.js:1265); raw store JSON
  with no mapper/tier fields (execute.cppm:56 vs pages.js:58).

### GET /pages/slug/:slug — MATCH

**Ghost**

1. Identity: `router.get('/pages/slug/:slug', mw.authenticatePublic, http(api.pagesPublic.read))`
   (content/routes.js:24); router CORS (content/routes.js:14); same `authenticatePublic` chain.
2. Operation: `read` under `docName:'pages'` (pages-public.js:19,56) — same op as `/:id`, identifier is `slug`.
3. Input: same options/data/validation as `/:id` (pages-public.js:77-88); `data.slug` from path; `pick`
   guard (pages-public.js:93). No other body/multipart/header inputs.
4. Auth/permissions: `permissions:true` (pages-public.js:89) + `authenticatePublic` (content/routes.js:24).
5. Status + headers: 200 / 400 / 404; `cacheInvalidate:false` (pages-public.js:57-59).
6. Full call chain: identical `generateCacheKeyData` and `query` as `/:id` (pages-public.js:64-112), with
   `identifier.slug = frame.data.slug` (pages-public.js:72) feeding `findOne({slug}, options)` and gift
   verification against the fetched model id (gift-link-access.ts:80-85).
7. Cache: same key dimensions (pages-public.js:64-76), identifier includes `slug`; no response cache today
   (pages-public.js:60-63).
8. Output: `{pages:[page]}` via `pages.js::all` (pages.js:58-61) + tiers browse side effect (pages.js:22).

**C++**

1. Identity: `Router{router,"/slug"}.add_route(Route{"/:id"}.get → pages_read_by_slug())`
   (admin/pages/routes.cppm:95-107) — param literally named `:id` but treated as slug; router chain
   `content_type_middleware`+`require_auth` (routes.cppm:20-21).
2. Ops: `ReadBySlugHandler` (read_by_slug/handler.cppm:11-28) runs `"pages_read_by_slug"`
   (handler.cppm:19), reads `"pages_read_by_slug_execute"` (handler.cppm:20), writes status/body from
   map with fallback `{200,"{}"}` (handler.cppm:21-25). Flow = single `ExecuteStep`
   (read_by_slug/workflow/flow/main.cppm:7-10).
3. Input: `IRequest&` (read_by_slug/step/execute.cppm:20); slug = last path segment (execute.cppm:25-29).
   **No query params parsed and no include/formats validation** (contrast read_by_id execute.cppm:39-49).
   No gift token, no fields/formats/absolute_urls.
4. Auth/permissions: only router `require_auth` (routes.cppm:21); no public auth, permissions, gift or
   member context, CORS/redirects.
5. Status/body: empty slug → 400 `validation("A page slug is required.","Bad Request","slug")`
   (execute.cppm:30-31); no hit → 404 `json("Page not found.","Page not found")` (execute.cppm:39-40);
   hit → 200 `{"pages":[first]}` (execute.cppm:41). No headers (handler.cppm:25).
6. Call chain: `ctx.store().json_for("pages", {{"filter","slug:"+slug}})` (execute.cppm:33) →
   `query_json`/`search_json` in-memory `where("slug:"+slug)` (context.cppm:1158,1592-1644); decodes via
   `JsonCodec::decode<serde::Value>` and takes first array element via `pages_slug_first`
   (execute.cppm:11-16,35-38); treats empty/`null`/`[]` as 404 (execute.cppm:39-40). No Bookmarks/Knex,
   no relations/transformer, no gift service.
7. Cache: none.
8. Output: `{"pages":[first]}` 200 (execute.cppm:41); empty result → 404; no pagination meta, no mapper,
   no tier fields.

**Diff**

- Residual gaps: permissions stage (pages-public.js:89); `mongoTransformer` (pages-public.js:100);
   cache-key data incl. gift dimension (pages-public.js:64-76); gift-link token resolution/verification and
   paid-member shim (gift-link-access.ts:37-88); tiers browse + mappers (pages.js:22,58);
   `cacheInvalidate:false` (pages-public.js:57-59); framework validation of include/formats
   (pages-public.js:79-88) — C++ read_by_slug parses no query params at all.
- Divergences: `require_auth` 401 (middleware/auth.cppm:19-27) instead of public auth; slug filter is a
  hand-rolled NVS `where("slug:"+slug)` over all Page rows (execute.cppm:33; context.cppm:1611) rather than
  a model `findOne`; 400 message differs ("A page slug is required." execute.cppm:31 vs Ghost's shared
  "A page id, slug or uuid is required." pages-public.js:14,95); route param misnamed `:id`
  (routes.cppm:98); raw store JSON with no mapper/tier enrichment (execute.cppm:41 vs pages.js:58).

## Module-level notes

- Ghost routes are content-API public reads. `content/routes.js:12-14` creates a `content api` router with
  global CORS (maxAge from config). Every endpoint uses `mw.authenticatePublic` (content/middleware.js:18-25)
  and the api-framework `http(...)` adapter, which runs input validation, permissions (`permissions:true`),
  response cache key generation and the `pages.js` output serializer. None of that framework layer exists
  in C++; the C++ handlers are thin workflow wrappers over `core::workflow::LocalWorkflowDriver` writing
  status/body directly (browse/handler.cppm:18-24; read_by_id/handler.cppm:18-26; read_by_slug/handler.cppm:18-26).
- **Structural auth gap:** `ApiRoutes::install` installs `PagesRoutes` with default args
  (routes.cppm:112). PostsRoutes and TiersRoutes expose an explicit `content=true` branch that installs
  public GET routes with `bearer_middleware`+`optional_auth` (admin/posts/routes.cppm:16-33;
  membership/tiers/routes.cppm:16-33). PagesRoutes has no such branch (admin/pages/routes.cppm:15-109);
  its GET routes carry `require_auth`, so C++ public page reads are 401 without a token
  (middleware/auth.cppm:19-27). This diverges from Ghost's anonymous-readable content API.
- **Shared residual gaps (all three routes):** framework `permissions` service (pages-public.js:46,89),
  `mongoTransformer` field restriction (pages-public.js:50,100; api-filter-utils.ts:9-25,97-102,128-159),
  cache-key helpers (pages-public.js:64-76; public-cache-keys.ts:23-58), gift-link service + paid-member
  shim (gift-link-access.ts:37-89), tiers service browse + `mappers.pages` output mapping
  (utils/serializers/output/pages.js:22,48,58), cors/rate-limit/redirect/pretty-URL middleware
  (content/middleware.js:18-25), and `cacheInvalidate:false` header semantics (pages-public.js:22-24,57-59).
  No C++ primitive implements any of these.
- **Shared C++ mechanics:** all three steps parse the raw `IRequest::get_path()` themselves
  (browse step:39-46, read_by_id step:33-46, read_by_slug step:25-29); store access is via
  `BlogContext::store()` → `find_by_resource("pages",id)` (context.cppm:1084-1121), `total("pages")`
  (context.cppm:1140-1150) and `json_for("pages",params)` (context.cppm:1152-1159), which route to in-memory
  model queries (`m_db.all<model::Page>()`) and the custom search/limit/include engine
  (context.cppm:1561-1660). No `core::cache::Cache`, no transactions, no outbox/webhook/queue side effects
  anywhere in these handlers.
- Out of scope for this controller but present in the same C++ `PagesRoutes` cluster: admin write endpoints
  (DELETE/POST/PUT, bulk, copy) at admin/pages/routes.cppm:30-93 and gift-link endpoints
  (admin/pages/gift_links_*). These correspond to Ghost's admin `pages.js`, not `pages-public.js`, and are
  intentionally not counted as EXTRA here.

## Strict-parity implementation (manifest group 4)

The three content-API page reads share the same handlers as the admin reads
(single flat `ApiRoutes` router). They were rewritten in place using the shared
abstractions; the old "Residual gaps" / "Divergences" entries above are closed
where a C++ primitive exists:

- `core::query::parse` / `run` / `apply`: allow-listed options, NQL filters
  (`slug:<slug>`, `status:published`), restricted-field guard, `fields`
  projection and pagination from model data (not a fabricated count).
- `core::auth::ghost::GhostAuthorizer`: anonymous requests are authorized in
  `ApiContext::CONTENT_API` (`permissions:true` equivalent) and may read only
  published pages; token-authenticated requests are admin and may read drafts.
- `blog::mapping::clean/wrap`: Ghost page field allow-list and
  `{pages:[...], meta:{pagination}}` / `{pages:[page]}` envelopes.
- Route `/pages/slug/:slug` now uses the exact Ghost placeholder name.

No new module partitions were added, so no central `BUILD.bazel` / `all.cppm`
registration is required. Accepted residual gaps with no allowed C++ primitive:
`authenticatePublic`'s CORS/rate-limit/redirect/pretty-URL chain,
`permissions:true` as a framework stage (implemented via the equivalent
public-context authorizer), `mongoTransformer:
rejectPagesContentApiRestrictedFieldsTransformer` (the `core::query` guard is
the equivalent restricted-field block), `applyGiftAccess` / paid-member shim,
`generateCacheKeyData`, `tiersService.service.api.browse({})` (no output
effect), and `headers.cacheInvalidate:false` header semantics (the admin audit
owns these shared handlers and emits no header).
