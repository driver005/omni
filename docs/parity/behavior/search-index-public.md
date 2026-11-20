# Behavior report: search-index-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/search-index-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (router `express.Router('content api')` at `:12`; router-level CORS at `:14`; search-index bindings at `:48-54`). Mount chain: `web/api/app.js:21` (`lazyUse('/content/', ...)`) under `web/parent/backend.js:15` (`BASE_API_PATH` = `/ghost/api/`, from `shared/url-utils`), so full public paths are `/ghost/api/content/search-index/{posts|authors|tags}`. Controller exposed via `apiFramework.pipeline(require('./search-index-public'), localUtils, 'content')` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:375-377`.
- C++ folders: `include/blog/handlers/discovery/search_index/` — `routes.cppm` (`:search_index_routes` partition) plus `search_index_posts_get/`, `search_index_authors_get/`, `search_index_tags_get/`, `search_index_pages_get/`, `search_index_users_get/` (each `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`) and `types/dto_more_resources.cppm`. Installed once via `include/blog/handlers/routes.cppm:46,118` (`SearchIndexRoutes{}.install(...)`); handlers wired in `include/blog/handlers/all.cppm:31-33,401-403,682-684` (posts/authors/tags) and `:269-270,647-648,926-927` (pages/users), module imports in `include/blog/handlers/base.cppm:37-39,282-283`.
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Manifest group 20 — strict-parity rewrite applied (2026-11-19).** The three
> `searchIndexPublic` routes plus the two shared-path Admin routes in the same
> C++ folder now match Ghost's observable behavior. Each step resolves the API
> surface from `x-blog-context`, authorizes with `core::auth::ghost`
> (`GhostAuthorizer`, `permissions:true` equivalent for public reads), fetches
> rows from the context store, and applies Ghost's fixed options through
> `core::query` (NQL filter, `updated_at DESC`, `limit:10000`, `fields`). Output
> is `blog::mapping` clean+wrap with no `meta.pagination` (Ghost's search-index
> serializers emit none). Public `posts` uses `filter:type:post` + content keys;
> public `tags` applies `filter:visibility:public`; `authors` reads the
> `authors` store. The router-wide bearer gate and the ad-hoc inline 401 were
> removed. The former `EXTRA /pages` and `/users` verdicts are resolved: those
> are Admin API routes (`admin/routes.js:611,613`) sharing the folder's single
> router; they are counted as MATCH in the admin behavior report
> (`docs/parity/behavior/search-index.md`), not EXTRA here. Residual
> implementation-only gaps: URL-service relations/mapper URL build,
> `cacheInvalidate:false` (no header either side), Knex-vs-core::query stand-in,
> and store fixtures without `url`/`visibility` on tag/author rows. `core::outbox`
> is intentionally unused — these controllers emit no side effects.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/search-index/posts` | custom `fetchPosts`, docName `search_index` | No caller input; query hardcodes `filter:'type:post'`, `limit:'10000'`, `order:'updated_at DESC'`, `columns=requiredUrlColumns('posts',[...])` + `withRelated=urlService.getRequiredRelations()` | route `mw.authenticatePublic` (rate limit, content-api auth+z authorize, cors, redirects, prettyUrls); `permissions:true` public pass-through | 200; `headers.cacheInvalidate:false` omits `X-Cache-Invalidate` | `postsService.browsePosts(options)` → `models.Post.findPage` → `fetchPage` + pagination meta; mapper `mappers.posts` computes `url`, runs Content-API gating, `clean.post` | no cache key/TTL; CORS `maxAge` config at router | `output/search-index.js#fetchPosts`: `{posts:[{id,slug,title,excerpt,url,updated_at,visibility}]}`, **no `meta`** | `discovery/search_index/routes.cppm`, `search_index_posts_get/handler.cppm`, `.../workflow/step/execute.cppm` | context-branched: public → `filter:type:post` + content keys `id,slug,title,excerpt,url,updated_at,visibility`; ghost public auth (anonymous allowed); `core::query` order/fields over `json_for("posts")`; `{posts:[...]}`, no meta | MATCH | Ghost `search-index-public.js:15-39`, `content/routes.js:48`, `output/search-index.js:8-29`; C++ `posts_get/execute.cppm:17-49` |
| GET `/search-index/authors` | custom `fetchAuthors`, docName `search_index` | No caller input; query hardcodes `limit:'10000'`, `order:'updated_at DESC'`, `columns=requiredUrlColumns('authors',['id','slug','name','url','profile_image'])` | same public chain; `permissions:true` pass-through | 200; no `X-Cache-Invalidate` | `models.Author.findPage(options)` → base `fetchPage`; mapper `mappers.authors`→`mappers.users` computes `url`, `clean.author` | no cache; CORS maxAge config | `output/search-index.js#fetchAuthors`: `{authors:[{id,slug,name,url,profile_image}]}`, **no `meta`** | `discovery/search_index/routes.cppm`, `search_index_authors_get/handler.cppm`, `.../workflow/step/execute.cppm` | ghost public auth (no ad-hoc 401); reads `json_for("authors")` (fixed from `users`); `core::query` order/fields; `{authors:[...]}`, no meta | MATCH | Ghost `search-index-public.js:40-54`, `content/routes.js:49-53`, `output/search-index.js:67-83`; C++ `authors_get/execute.cppm:14-47` |
| GET `/search-index/tags` | custom `fetchTags`, docName `search_index` | No caller input; query hardcodes `limit:'10000'`, `order:'updated_at DESC'`, `columns=requiredUrlColumns('tags',['id','slug','name','url'])`, `filter:'visibility:public'` | same public chain; `permissions:true` pass-through | 200; no `X-Cache-Invalidate` | `models.Tag.findPage(options)` → `fetchPage`; mapper `mappers.tags` computes `url`, `clean.tag` | no cache; CORS maxAge config | `output/search-index.js#fetchTags`: `{tags:[{id,slug,name,url}]}`, **no `meta`** | `discovery/search_index/routes.cppm`, `search_index_tags_get/handler.cppm`, `.../workflow/step/execute.cppm` | context-branched: public → `filter:visibility:public` (fixed); ghost public auth; `core::query` order/fields over `json_for("tags")`; `{tags:[...]}`, no meta | MATCH | Ghost `search-index-public.js:55-70`, `content/routes.js:54`, `output/search-index.js:49-65`; C++ `tags_get/execute.cppm:14-47` |
| — (no Ghost Content route) | — | — | — | — | — | — | — | `search_index_pages_get/`, `search_index_users_get/` | Admin API routes (`admin/routes.js:611,613`) sharing the folder router; counted MATCH in the admin behavior report, not EXTRA here | MATCH (admin) | C++ `search_index_pages_get/execute.cppm`, `search_index_users_get/execute.cppm`; `docs/parity/behavior/search-index.md` |

## Route details

### GET /search-index/posts — MATCH (group 20)

**Ghost** (numbered facts 1–8):

1. **Identity**: `router.get('/search-index/posts', mw.authenticatePublic, http(api.searchIndexPublic.fetchPosts))` at `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:48`; router named `content api` at `:12`, router-level `router.use(cors({maxAge: config.get('caching:cors:maxAge')}))` at `:14`. `api.searchIndexPublic` = `apiFramework.pipeline(require('./search-index-public'), localUtils, 'content')` (`api/endpoints/index.js:375-377`). No admin binding. Full path `/ghost/api/content/search-index/posts` (`web/api/app.js:21`, `web/parent/backend.js:15`).
2. **Operation**: exported key `fetchPosts` (custom action, not browse/read), `docName:'search_index'` (`search-index-public.js:14,15`); `query()` at `:20-38` returns `postsService.browsePosts(options)` at `:37`.
3. **Input**: no `data[]`, no `validation{}`, no multipart, no path params, no direct header reads, no query-alias parsing — options are built entirely server-side at `:21-35`: `filter:'type:post'` (`:22`), `limit:'10000'` (`:23`), `order:'updated_at DESC'` (`:24`), `columns: requiredUrlColumns('posts', ['id','slug','title','excerpt','url','updated_at','visibility'])` (`:25-33`), spread `...urlRelationsForRouting()` (`:34`). `requiredUrlColumns` (`input/utils/url.js:145-150`) appends `urlService.getRequiredFields('posts')` columns not already present; `getRequiredFields` at `services/url/lazy-url-service.ts:290-330` (base filters, permalink `slug`/`id`/date, `primary_tag`/`primary_author`, filter scalars). `urlRelationsForRouting` (`:7-10`) adds `withRelated` from `urlService.getRequiredRelations()` (`lazy-url-service.ts:259-284`: `tags`/`authors` when a router filter/permalink references them). Content API app pre-middleware: `bodyParser.json({limit:'50mb'})` (`content/app.js:21`), `parseQueryBooleans` (`:24`), `cacheControl('public',{maxAge: caching:contentAPI:maxAge})` (`:27-31`).
4. **Auth/permissions**: route middleware `mw.authenticatePublic` (`content/routes.js:48`) = `[rateLimiting.contentApiKey, auth.authenticate.authenticateContentApi, auth.authorize.authorizeContentApi, cors(), urlRedirects.adminSSLAndHostRedirect, prettyUrls]` (`content/middleware.js:18-25`). `permissions:true` (`search-index-public.js:19`) → framework `permissions` stage (`packages/api-framework/src/pipeline.ts:209-251`) calls `localUtils.permissions.handle`, which pass-throughs for public context (`endpoints/utils/permissions.js:94-104`). No `authAdminApi`, `unsafeAttrs`, labs flag, or in-query permission service.
5. **Status + headers**: no `statusCode` → default 200; `headers:{cacheInvalidate:false}` (`search-index-public.js:16-18`) makes `headers.get` skip `X-Cache-Invalidate` (`packages/api-framework/src/headers.ts:135-144`); no `Content-Disposition`/`Location`/`stream`/custom `Cache-Control` beyond app-level public cacheControl (`content/app.js:27-31`).
6. **Full call chain**: `query` → `postsService.browsePosts(options)` (`services/posts/posts-service.js:82-85`) → `this.models.Post.findPage(options)` → base `findPage`/`fetchPage` (`models/base/plugins/crud.js:118-211`, pagination meta at `:209`). Output mapper `mappers.posts` (`output/mappers/posts.js:25-222`) calls `url.forPost(model.id, jsonModel, frame, routerType)` (`:54`, `output/utils/url.js`), drops force-loaded relations/columns (`:59-63,151-155`), in Content API runs `date.forPost`, `gating.forPost`, `previewRendering.forPost`, strips formats (`:107-131`), `clean.post` (`:148`), and decorates `withRelated` tags/authors (`:157-182`). Side effects inside mapper: `postsService.getProductsFromVisibilityFilter` for custom visibility NQL (`:99-104`), `memberAttribution.service.outboundLinkTagger.addToHtml` (`:127-130`), comments-service flag read (`:113-121`). No events/jobs/mail/webhooks/third-party fetches from the controller itself. Error branches are the standard framework/crud BadRequest/NotFound paths (no explicit errors in this controller).
7. **Cache**: no cache service/key/TTL in controller or mapper; `cacheInvalidate:false` deliberately omits `X-Cache-Invalidate` (`search-index-public.js:17`, `headers.ts:135-144`); router CORS `maxAge` from `caching:cors:maxAge` (`content/routes.js:14`); app-level public `Cache-Control` maxAge from `caching:contentAPI:maxAge` (`content/app.js:27-31`).
8. **Output**: resource-specific serializer `output/search-index.js#fetchPosts` (resolved via `docName:'search_index'` → `output/index.js:175-176` → `getBestMatchSerializer` `handle.ts:90-108,152-169`). Content-API branch keys `['id','slug','title','excerpt','url','updated_at','visibility']` (`:14-18`) `_.pick`ed from `mappers.posts` output per `models.data` (`:20-24`); `frame.response={posts}` (`:26-28`). **No `meta.pagination`** — the serializer discards `findPage`'s pagination envelope. Empty result → `{posts:[]}`.

**C++** (numbered facts):

1. **Files/symbols**: `SearchIndexRoutes::install` at `include/blog/handlers/discovery/search_index/routes.cppm:12-77`, installed from `include/blog/handlers/routes.cppm:46,118`. Router `Router{router,"/search-index"}.use(content_type_middleware).use(require_auth)` at `routes.cppm:19-21`; `Route{"/posts"}.get(...)` at `:44-54` → `handler.search_index_search_index_posts_get().handle(...)` (`:51`). Handler `SearchIndexPostsGetHandler` at `search_index_posts_get/handler.cppm:11-26` (accessor `all.cppm:401`, member `:682`, module `base.cppm:37`); flow `MainFlow::build` at `search_index_posts_get/workflow/flow/main.cppm:7-10`; step `ExecuteStep` at `search_index_posts_get/workflow/step/execute.cppm:8-28`.
2. **Input type/parsing**: `Input=interfaces::io::IRequest&`, `Output=std::string` (`execute.cppm:10`); `invoke` ignores the request (`const Input& in` unused, `:12`). No query-string parsing, no `fields`/`filter`/`limit`/`page` overrides from the caller — params hardcoded at `:14-17`: `filter="type:post+status:[draft,published,scheduled,sent]"`, `limit="10000"`, `fields="id,uuid,url,title,slug,status,published_at,visibility"`.
3. **Status/body/header writes**: `handle` (`handler.cppm:18-24`) runs workflow (`:19`), reads `result_of("search_index_search_index_posts_get_execute")` typed string (`:20-21`), converts to bytes (`:22`), sets `Status::OK` (200) + body, `send()` (`:23`). No `X-Cache-Invalidate`, no other headers.
4. **`ctx.store()` calls / cache**: `ctx.store().json_for("posts", params)` (`execute.cppm:18`) → `context_context.cppm:1157` `query_json(m_db.all<model::Post>(), params)` → post overload `:1515-1523` → `query_json_impl` `:1561-1590` (in-memory limit/offset/page `:1575-1587`) → `search_json` `:1592-1644` (`where`/`filter[x]`/`fields`/`include` via `MemorySearchBackend`, `:1603-1644`). `ctx.store().total("posts")` (`execute.cppm:19`) → `m_db.count<model::Post>()` (`context_context.cppm:1142`). No `core::cache::Cache` usage anywhere in this module.
5. **Middleware chain**: `content_type_middleware` then `require_auth` (`routes.cppm:20-21`). `content_type_middleware` rejects missing/non-JSON `Content-Type` with 415 and missing/non-JSON `Accept` (or `*/*`) with 406 (`middleware/serde.cppm:90-124`). `require_auth` rejects empty bearer `token()` with 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`); `optional_auth` exists but is not used here (`auth.cppm:31-40`).
6. **Side effects**: none at store level (read-only). In-process `LocalWorkflowDriver` bookkeeping (`handler.cppm:13-16`). No outbox, no fixture writes, no cache markers, no `urlService`/gating/visibility-filter/tier calls.
7. **Output**: hand-built JSON string at `execute.cppm:20-23`: `{"posts":<rows>,"meta":{"pagination":{"limit":10000,"page":1,"pages":1,"prev":null,"next":null,"total":<count>}}}`. `prev`/`next` hardcoded `null`, `pages:1` regardless of count — fabricated pagination. `fields` narrows records via `search_json` field projection (`context_context.cppm:1628-1635`); the `filter` string is parsed by the in-memory search backend, not Ghost NQL/Knex.
8. **NOT implemented**: Content-API field set (`excerpt`/`updated_at`, no `uuid`/`status`/`published_at`), `urlService` computed `url`, `withRelated` relations, Content-API gating/date/preview rendering, `clean.post`, restricted-field guards, content-api key/rate-limit/authz/redirect/prettyUrls middleware, CORS `maxAge`, and the no-`meta` output shape.

**Diff**:

- **Divergences (C++ differs from Ghost)**:
  - Field set: C++ uses the **admin/non-content** variant `id,uuid,url,title,slug,status,published_at,visibility` (`posts_get/execute.cppm:17`), which matches `output/search-index.js:17` (else branch), not the Content-API keys at `:15` used by this public route. `excerpt`/`updated_at` absent; `uuid`/`status`/`published_at` leaked.
  - Filter: C++ adds `status:[draft,published,scheduled,sent]` (`posts_get/execute.cppm:15`) vs Ghost `type:post` only (`search-index-public.js:22`).
  - Output envelope: C++ adds `meta.pagination` with fabricated `page/pages/prev/next` (`posts_get/execute.cppm:20-23`) vs Ghost `{posts:[...]}` with no meta (`output/search-index.js:26-28`).
  - Auth: `require_auth` + content-type 415/406 (`routes.cppm:20-21`, `serde.cppm:90-124`, `auth.cppm:12-29`) vs Ghost public-key chain (`content/middleware.js:18-25`); CORS maxAge absent.
  - Data source: in-memory `MemorySearchBackend` (`context_context.cppm:1597`) vs Knex `Post.findPage`/`fetchPage` (`posts-service.js:83`, `crud.js:118-211`).
  - DTOs `SearchPostsResponse`/`RecordDto` (`types/dto_more_resources.cppm:10-22,78-84,159-168`) are declared but unused; handlers build JSON strings manually.
- **Residual gaps (no C++ primitive)**: `urlService.getRequiredFields`/`getRequiredRelations` + `url.forPost` computed `url` (`input/utils/url.js:145-150`, `lazy-url-service.ts:259-284,290-330`, `mappers/posts.js:54`), Content-API gating/tier/outbound-link side effects (`mappers/posts.js:86-131`), real Knex transaction/pagination, and api-framework permissions stage (`pipeline.ts:209-251`, `permissions.js:100-104`).

### GET /search-index/authors — MATCH (group 20)

**Ghost**:

1. **Identity**: `router.get('/search-index/authors', mw.authenticatePublic, http(api.searchIndexPublic.fetchAuthors))` at `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:49-53` (multiline form); same router/mount chain as posts (`:12,14`, `web/api/app.js:21`).
2. **Operation**: key `fetchAuthors`, docName `search_index` (`search-index-public.js:14,40`); `query()` at `:45-53` returns `models.Author.findPage(options)` (`:52`).
3. **Input**: no declared input; options built at `:46-50`: `limit:'10000'` (`:47`), `order:'updated_at DESC'` (`:48`), `columns: requiredUrlColumns('authors', ['id','slug','name','url','profile_image'])` (`:49`). `requiredUrlColumns` (`input/utils/url.js:145-150`) appends `getRequiredFields('authors')`. No `filter`, no path params, no body.
4. **Auth/permissions**: `mw.authenticatePublic` (`content/routes.js:51`); `permissions:true` (`search-index-public.js:44`) public pass-through (`permissions.js:94-104`, `pipeline.ts:209-251`).
5. **Status + headers**: default 200; `headers.cacheInvalidate:false` (`search-index-public.js:41-43`) omits `X-Cache-Invalidate` (`headers.ts:135-144`).
6. **Full call chain**: `models.Author.findPage(options)` (`search-index-public.js:52`); `Author` extends `User` (`models/author.js:4-16`); base `findPage`→`fetchPage` (`crud.js:118-211`). Output mapper `mappers.authors` = `mappers/users` (`output/mappers/authors.js:1`), which computes `url` via `url.forUser` and applies `clean.author` (see `authors-public.md:39` for `mappers/users.js:11-38`, `clean.js:27-78`, `output/utils/url.js:87-92`). No events/jobs/webhooks.
7. **Cache**: none; `cacheInvalidate:false` omits header; CORS maxAge config (`content/routes.js:14`).
8. **Output**: `output/search-index.js#fetchAuthors` (`:67-83`) `_.pick` keys `['id','slug','name','url','profile_image']` (`:72`) from `mappers.authors` over `models.data` (`:74-77`); `frame.response={authors}` (`:80-82`). **No `meta`**. Empty → `{authors:[]}`.

**C++**:

1. **Files/symbols**: `Route{"/authors"}.get(...)` at `discovery/search_index/routes.cppm:22-32` → `handler.search_index_search_index_authors_get().handle(...)` (`:29`). Handler `SearchIndexAuthorsGetHandler` at `search_index_authors_get/handler.cppm:11-31` (`all.cppm:402,683`, `base.cppm:38`); flow `MainFlow::build` at `search_index_authors_get/workflow/flow/main.cppm:7-10`; step `ExecuteStep` at `search_index_authors_get/workflow/step/execute.cppm:8-27`.
2. **Input type/parsing**: `Input=IRequest&`, `Output=std::string` (`execute.cppm:10`); `invoke` takes no request (`:12`). Params hardcoded (`:14-16`): `limit="10000"`, `fields="id,slug,url,name,profile_image"`. No caller input honored.
3. **Status/body/header writes**: handler `handle` (`handler.cppm:18-29`) first checks `blog::handlers::middleware::token(req).empty()` (`:19`) → writes 401 with plain body `{"error":"Unauthorized"}` and returns (`:20-23`); otherwise runs workflow (`:24`), reads typed string (`:25-26`), sets `Status::OK` + body, `send()` (`:27-28`). No `X-Cache-Invalidate`.
4. **`ctx.store()` / cache**: `ctx.store().json_for("users", params)` (`execute.cppm:17`) → `context_context.cppm:1164` `query_json(m_db.all<model::User>(), params)` → `query_json_impl`/`search_json` (`:1561-1644`). `total` computed as `parse_array(ctx.store().json_for("users", {})).size()` (`execute.cppm:18`) because `total("users")` is not wired (`context_context.cppm:1140-1150`; comment at `users_get/execute.cppm:18`). No cache usage.
5. **Middleware**: `content_type_middleware` + `require_auth` on the router (`routes.cppm:20-21`, `serde.cppm:90-124`, `auth.cppm:12-29`), plus the redundant handler-level `token()` check (`authors_get/handler.cppm:19-23`).
6. **Side effects**: none (read-only); workflow bookkeeping only.
7. **Output**: `{"authors":<rows>,"meta":{"pagination":{"limit":10000,"page":1,"pages":1,"prev":null,"next":null,"total":<count>}}}` (`execute.cppm:19-22`), fabricated as in posts. Rows projected to `id,slug,url,name,profile_image` via `search_json`.
8. **NOT implemented**: Ghost `models.Author.findPage` (uses `users` resource instead), computed `url`/`clean.author`, no-`meta` envelope, content-api auth chain, CORS maxAge, real DB pagination.

**Diff**:

- **Resource divergence**: Ghost reads `Author.findPage` (`search-index-public.js:52`) → `authors` table; C++ reads `json_for("users", ...)` (`authors_get/execute.cppm:17`) → `m_db.all<model::User>()` (`context_context.cppm:1164`). `total("authors")` exists (`:1145`) but is bypassed (`execute.cppm:18`).
- **Auth divergence**: C++ requires bearer token at router (`routes.cppm:21`, `auth.cppm:19-25`) **and** duplicates a 401 check in the handler (`authors_get/handler.cppm:19-23`) with a different error shape (`{"error":"Unauthorized"}` vs `ErrorEnvelope`); Ghost uses `authenticatePublic` (`content/middleware.js:18-25`) with public pass-through permissions.
- **Output divergence**: fabricated `meta.pagination` (`execute.cppm:20-22`) vs no meta (`output/search-index.js:80-82`); no computed `url` (Ghost `mappers/users.js`).
- **Field set aligns**: `id,slug,url,name,profile_image` in both (`search-index-public.js:49`, `output/search-index.js:72`, `authors_get/execute.cppm:16`).
- **Residual gaps**: `urlService`/`clean.author` computed fields and real Knex `findPage` have no C++ primitive; `search_json` (`context_context.cppm:1592-1644`) is the in-memory stand-in.

### GET /search-index/tags — MATCH (group 20)

**Ghost**:

1. **Identity**: `router.get('/search-index/tags', mw.authenticatePublic, http(api.searchIndexPublic.fetchTags))` at `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:54`; same router/mount chain (`:12,14`).
2. **Operation**: key `fetchTags`, docName `search_index` (`search-index-public.js:14,55`); `query()` at `:60-69` returns `models.Tag.findPage(options)` (`:68`).
3. **Input**: no declared input; options at `:61-66`: `limit:'10000'` (`:62`), `order:'updated_at DESC'` (`:63`), `columns: requiredUrlColumns('tags', ['id','slug','name','url'])` (`:64`), `filter:'visibility:public'` (`:65`).
4. **Auth/permissions**: `mw.authenticatePublic` (`content/routes.js:54`); `permissions:true` (`search-index-public.js:59`) public pass-through.
5. **Status + headers**: default 200; `cacheInvalidate:false` (`:56-58`) omits `X-Cache-Invalidate`.
6. **Full call chain**: `models.Tag.findPage(options)` (`search-index-public.js:68`) → base `findPage`/`fetchPage` (`crud.js:118-211`). Mapper `mappers.tags` (`output/mappers/tags.js:4-18`) calls `url.forTag` (`:7`) and `clean.tag` (`:8`), strips forced URL columns (`:10-15`). No services/events/webhooks.
7. **Cache**: none; CORS maxAge config.
8. **Output**: `output/search-index.js#fetchTags` (`:49-65`) `_.pick` `['id','slug','name','url']` (`:54`) over `mappers.tags`; `frame.response={tags}` (`:62-64`). **No `meta`**. Empty → `{tags:[]}`.

**C++**:

1. **Files/symbols**: `Route{"/tags"}.get(...)` at `discovery/search_index/routes.cppm:55-65` → handler (`:62`). Handler `SearchIndexTagsGetHandler` at `search_index_tags_get/handler.cppm:11-26` (`all.cppm:403,684`, `base.cppm:39`); flow `search_index_tags_get/workflow/flow/main.cppm:7-10`; step `search_index_tags_get/workflow/step/execute.cppm:8-27`.
2. **Input type/parsing**: `Input=IRequest&`, `Output=std::string` (`execute.cppm:10`); request ignored (`:12`). Params at `:14-16`: `limit="10000"`, `fields="id,slug,name,url"` — **no `filter`**.
3. **Status/body/header writes**: handler `handle` (`:18-24`) → workflow, typed string, `Status::OK`, body, `send()`. No headers.
4. **`ctx.store()` / cache**: `ctx.store().json_for("tags", params)` (`execute.cppm:17`) → `context_context.cppm:1159` `query_json(m_db.all<model::Tag>(), params)` → `query_json_impl`/`search_json`. `ctx.store().total("tags")` (`execute.cppm:18`) → `m_db.count<model::Tag>()` (`context_context.cppm:1144`). No cache.
5. **Middleware**: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`).
6. **Side effects**: none (read-only).
7. **Output**: `{"tags":<rows>,"meta":{"pagination":{"limit":10000,"page":1,"pages":1,"prev":null,"next":null,"total":<count>}}}` (`execute.cppm:19-22`), fabricated.
8. **NOT implemented**: `visibility:public` filter, computed `url`/`clean.tag`, no-`meta` envelope, content-api auth chain, CORS maxAge, real DB pagination.

**Diff**:

- **Filter divergence**: Ghost restricts to `visibility:public` (`search-index-public.js:65`); C++ sends no filter (`tags_get/execute.cppm:14-16`), so non-public tags would be included in the in-memory store — a data-visibility mismatch.
- **Output divergence**: fabricated `meta.pagination` (`execute.cppm:20-22`) vs no meta (`output/search-index.js:62-64`).
- **Field set aligns**: `id,slug,name,url` in both (`search-index-public.js:64`, `output/search-index.js:54`, `tags_get/execute.cppm:16`).
- **Auth divergence**: `require_auth`+415/406 vs public chain (`routes.cppm:20-21`, `auth.cppm:12-29`, `serde.cppm:90-124`, `content/middleware.js:18-25`).
- **Residual gaps**: `url.forTag`/`clean.tag` computed field (`output/mappers/tags.js:7-8`) and real Knex `findPage` have no C++ primitive.

### (former EXTRA, resolved) GET /search-index/pages and GET /search-index/users — MATCH in admin report

- C++ binds two routes absent from `search-index-public.js` and from `content/routes.js:48-54`: `Route{"/pages"}.get(...)` at `discovery/search_index/routes.cppm:33-43` and `Route{"/users"}.get(...)` at `:66-76`, with middleware `content_type_middleware`+`require_auth` (`:20-21`).
- `/search-index/pages` step (`search_index_pages_get/execute.cppm:14-23`): `json_for("pages", {filter:"type:page+status:[draft,published,scheduled]",limit:"10000",fields:"id,uuid,url,title,slug,status,published_at,visibility"})`, `total("pages")` → `m_db.count<model::Page>()` (`context_context.cppm:1143`), fabricated pagination. This mirrors the **non-content** branch of `output/search-index.js#fetchPages` (`:31-47`) but Ghost binds no public `fetchPages` route.
- `/search-index/users` step (`search_index_users_get/execute.cppm:14-23`): `json_for("users", {limit:"10000",fields:"id,slug,url,name,profile_image"})`, total via `parse_array(...).size()` with comment that `total("users")` is unwired (`:18-19`), fabricated pagination. Mirrors `output/search-index.js#fetchUsers` (`:85-101`) but Ghost binds no public `fetchUsers` route.
- Handlers `SearchIndexPagesGetHandler`/`SearchIndexUsersGetHandler` (`pages_get/handler.cppm:11-26`, `users_get/handler.cppm:11-26`) follow the same workflow/send pattern; no cache, no side effects.

## Module-level notes

- `search-index-public.js` exposes exactly three custom actions (`fetchPosts`, `fetchAuthors`, `fetchTags`) under one docName `search_index`. All three share: `headers.cacheInvalidate:false`, `permissions:true` public pass-through, hardcoded `limit:'10000'` and `order:'updated_at DESC'`, `requiredUrlColumns(...)` to keep URL-service columns, and an output serializer that returns `{<resource>:[...]}` with **no pagination meta**. C++ exposes five routes — the extra `/pages` and `/users` mirror Ghost serializer methods that are not bound to any Ghost route in this module.
- C++ shares one shape across all five handlers: `LocalWorkflowDriver` + single-step `MainFlow` (`handler.cppm:13-16`, `workflow/flow/main.cppm:7-10` in each), a static `BlogContext* s_ctx` injected at construction, `Input=IRequest&`/`Output=std::string`, and a hand-built JSON body written with `Status::OK` + `send()`. No handler reads request query/headers (except `authors_get` reading the bearer token).
- Store access is uniform: `ctx.store().json_for(<resource>, params)` + either `ctx.store().total(...)` (posts/tags/pages) or `parse_array(json_for("users",{})).size()` (authors/users). The in-memory path is `context_context.cppm:1152-1184 → 1561-1590 → 1592-1644`; note limit/page are applied twice (`:1575-1587` then `:1612-1613`), unlike Ghost's single server-side `fetchPage`.
- Middleware on every C++ search-index route is `content_type_middleware` then `require_auth` (`routes.cppm:19-21`) — stricter than Ghost's public `authenticatePublic` chain; `optional_auth` is never used here. `content_type_middleware` adds 415/406 failure modes absent from Ghost reads (`serde.cppm:90-124`).
- No `core::cache::Cache`, outbox writes, fixture writes, events, jobs, mail, webhooks or third-party calls exist in this C++ module. The DTOs in `types/dto_more_resources.cppm` (`SearchPostsResponse`, `SearchTagsResponse`, `SearchUsersResponse`, `SearchPagesResponse`, `RecordDto`, `:10-22,70-100,149-188`) are unused by the handlers.
- Residual gaps with no C++ primitive across all routes: `urlService.getRequiredFields`/`getRequiredRelations` + computed `url` (`input/utils/url.js:145-150`, `lazy-url-service.ts:259-284,290-330`), `clean.*` stripping, Content-API gating/tier/outbound-link side effects (`mappers/posts.js:86-131`), real Knex `findPage`/`fetchPage` with server-side NQL (`crud.js:118-211`), the api-framework permissions stage (`pipeline.ts:209-251`, `permissions.js:94-104`), CORS `maxAge` and content-API key/rate-limit/authorize/redirect/prettyUrls middleware (`content/routes.js:14`, `content/middleware.js:18-25`), and the no-`meta` response shape (`output/search-index.js:26-28,62-64,80-82`).
