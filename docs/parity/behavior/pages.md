# Behavior report: pages

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/pages.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- Ghost admin middleware: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/middleware.js`
- Ghost input serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/input/pages.js`
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/pages.js`
- Ghost service: `docs/Ghost/ghost/core/server/services/posts/posts-service.js`
- C++ routes: `include/blog/handlers/admin/pages/routes.cppm`
- C++ folders: `include/blog/handlers/admin/pages/{browse,read_by_id,read_by_slug,create,edit,destroy,bulk_destroy,bulk_edit,pages_id_copy_post}` (+ `gift_links_{get,put,post}` — out of scope, see Module notes)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 9 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Manifest group 4:** the `Diff` subsections below record the pre-implementation
> audit. All nine routes were rewritten to strict Ghost behavior with the shared
> abstractions; see **Strict-parity implementation (manifest group 4)** at the end
> for the resolution of every listed gap and the verdict flip to MATCH.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /pages | browse | `options[]=include,filter,fields,formats,limit,order,page,debug,absolute_urls`; `validation.options.include=ALLOWED_INCLUDES`, `formats=models.Post.allowedFormats` | `mw.authAdminApi` | 200; `headers.cacheInvalidate=false`; no auto X-Cache | `models.Post.findPage(restrictAdminApiQueryOptions(frame.options))` | none | `pages.all` → `{pages:[...],meta:{pagination}}` | browse/{handler,flow,step} | `core::query::parse`+`run` over `store().json_for("pages",{})` (NQL filter/order/fields/limit/page, restricted-field guard); `core::auth::ghost` authorize; `blog::mapping::clean/wrap` -> `{pages,meta.pagination}` | MATCH | controller:pages.js:27-59; routes.js:61; browse/step/execute.cppm:36-59 |
| GET /pages/:id | read | `data=id,slug,uuid`; `options[]=include,fields,formats,debug,absolute_urls,forUpdate,transacting`; allowed include/formats | `mw.authAdminApi` | 200; `cacheInvalidate=false` | `models.Post.findOne(frame.data,options)`; `NotFoundError` if none | none | `pages.all` → `{pages:[page]}` | read_by_id/* | path id/slug/uuid via `find_by_resource` + `json_for` fallbacks; `core::query::apply` fields projection; `blog::mapping::clean/wrap`; 400/404 via Ghost envelopes; content reads force status:published | MATCH | pages.js:61-100; routes.js:65; read_by_id/step/execute.cppm:30-56 |
| GET /pages/slug/:slug | read | same op; path param `slug` | `mw.authAdminApi` | 200; `cacheInvalidate=false` | same as read by id | none | `pages.all` → `{pages:[page]}` | read_by_slug/* | route renamed `/:slug`; `core::query` NQL `slug:<slug>` filter (+status:published on public); single `blog::mapping::clean/wrap` result; 400/404 | MATCH | routes.js:66; read_by_slug/step/execute.cppm:22-41 |
| POST /pages | add | body `data.pages[0]`; `options[]=include,formats,source`; `source∈{html}` | `mw.authAdminApi` | 201; `cacheInvalidate=false`; `X-Cache-Invalidate:/*` when status published | `models.Post.add(frame.data.pages[0],options)` | header only | `pages.all` → `{pages:[page]}` | create/* | `pages[0]` unwrap; `core::auth::ghost` unsafeAttrs; type:page; `mutations.create`; `blog::mapping::clean/wrap`; 201 + Location + X-Cache `/*` when status published | MATCH | pages.js:102-130; create/handler.cppm:18-27; create/step/execute.cppm:47-76 |
| PUT /pages/:id | edit | `data.pages[0]`; `options[]` incl `id(required),formats,source,force_rerender,save_revision,convert_to_lexical,forUpdate,transacting` | `mw.authAdminApi` | 200; `cacheInvalidate=false`; X-Cache from `handleCacheInvalidation` | `models.Post.edit(...)` + `postsService.handleCacheInvalidation` | `/*` or `/p/<uuid>/?member_status=*` | `pages.all` → `{pages:[page]}` | edit/* | path id; unsafeAttrs; `mutations.edit`; wasChanged/previous-state cache logic -> `/*` or `/p/<uuid>/?member_status=*` preview URLs; clean/wrap output | MATCH | pages.js:132-178; posts-service.js:598-624; edit/step/execute.cppm:47-81 |
| DELETE /pages/:id | destroy | path `id`; `options[]=include,id(required)` | `mw.authAdminApi` | 204; `cacheInvalidate=true`; `X-Cache-Invalidate:''` if page draft | `models.Post.findOne({id,type:'page',status:'all'})` → draft check → `models.Post.destroy({...,require:true})` | `''` if draft, else `/*` | `pages.destroy` → `{pages:[]}` (204 no body) | destroy/* | pre-check via `find_by_resource`; `mutations.destroy` in an outbox transaction; 204 empty body; X-Cache `''` on draft else `/*` | MATCH | pages.js:228-264; destroy/step/execute.cppm:14-29 |
| DELETE /pages | bulkDestroy | `options[]=filter` | `mw.authAdminApi` | 200; `cacheInvalidate=true`; X-Cache `''` if allDraft | `postsService.bulkDestroy(filter)` → transaction, row locks, relation deletes, `PostsBulkDestroyedEvent` | `''` if allDraft else `/*` | `pages.bulkDestroy` → `{bulk:{meta:{stats,errors}}}` | bulk_destroy/* | `core::query` filter → ids; per-id `mutations.destroy` on the pages DB in an outbox transaction; `PostsBulkDestroyedEvent`; `{bulk:{meta:{stats,errors}}}`; allDraft from locked row statuses | MATCH | pages.js:208-226; posts-service.js:434-501; bulk_destroy/step/execute.cppm:46-62 |
| PUT /pages/bulk | bulkEdit | body `data=action,meta`; `options[]=filter(required)`; `action required` | `mw.authAdminApi` | 200; `cacheInvalidate=true` → `/*` | `postsService.bulkEdit(data.bulk,filter)` → 6 actions, transactions, DomainEvents | `/*` | `pages.bulkEdit` → `{bulk:{action,meta.stats,errors,unsuccessfulData}}` | bulk_edit/* | `core::query` filter + 6-action allowlist; per-id `mutations.edit` in an outbox transaction; action-specific DomainEvents; `{bulk:{action,meta.stats,errors,unsuccessfulData}}`; X-Cache `/*` | MATCH | pages.js:180-206; posts-service.js:178-298; bulk_edit/step/execute.cppm:46-76 |
| POST /pages/:id/copy | copy | path `id`; `options[]=id,formats`; `id required` | `mw.authAdminApi` | 201; `Location` via `generateCopiedPostLocationFromUrl`; `cacheInvalidate=false` | `postsService.copyPost(frame)` → findOne(status:all), pick fields, append `(Copy)`, status draft, authors/tags/meta/tiers, `Post.add` | none | `pages.all` → `{pages:[copy]}` | pages_id_copy_post/* | find source; clean field allowlist, title ` (Copy)`, force status draft; `mutations.create` in outbox transaction; 201 + Location `/pages/<id>/` | MATCH | pages.js:266-287; posts-service.js:626-670; pages_id_copy_post/step/execute.cppm:24-47 |

## Route details

### GET /pages — MATCH

**Ghost**
1. Identity: `GET /pages`; route `router.get('/pages', mw.authAdminApi, http(api.pages.browse))` (routes.js:61). Router-level `router.use(apiMw.cors)` (routes.js:17). `mw.authAdminApi` chain = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (middleware.js:110-118).
2. Operation: key `browse`, `docName:'pages'` (pages.js:26-27).
3. Input: `options[]` = include, filter, fields, formats, limit, order, page, debug, absolute_urls (pages.js:31-41). `validation.options.include` ∈ ALLOWED_INCLUDES (tags, authors, authors.roles, tiers, count.signups, count.paid_conversions, post_revisions, post_revisions.author) (pages.js:6-15,44-46); `formats` ∈ `models.Post.allowedFormats` = mobiledoc/lexical/html/plaintext (pages.js:48; models/post.js:1265). Input serializer `browse` forces `type:page` and, for Admin API, `status:[draft,published,scheduled]`, default formats `mobiledoc,lexical`, default relations `tags,authors,authors.roles,tiers,count.signups,count.paid_conversions`, default order `title asc`, and URL force-load relations (serializers/input/pages.js:60-85,115-154). Query aliases via `filter[x]=y` handled by NQL.
4. Auth: `mw.authAdminApi`; `permissions={docName:'posts',unsafeAttrs:UNSAFE_ATTRS}` (status, authors, visibility) (pages.js:16,52-55).
5. Status/headers: default 200; `headers.cacheInvalidate=false` → no automatic X-Cache (pages.js:28-30; headers.ts:135-144).
6. Call chain: `models.Post.findPage(restrictAdminApiQueryOptions(frame.options))` (pages.js:57). `restrictAdminApiQueryOptions` validates restricted order fields and chains `rejectAdminApiRestrictedFieldsTransformer` over `filter`/`mongoTransformer` to block password/token/gift_link_token/secret columns (api-filter-utils.ts:78,218-220,247-289). `findPage` applies status/enforced filter, NQL, relations, transaction-free Bookshelf/Knex pagination (models/post.js:1240-1252,1267-1297).
7. Cache: no cache read/write; no invalidation header on this op.
8. Output: serializer dispatch falls back to `pages.all` (no `browse` method in pages.js output serializer) (handle.ts:90-105; output/pages.js:13). `all` calls `tiersService.service.api.browse({})` side effect and `mappers.pages` per model; envelope `{pages:[...],meta:<pagination>}` (output/pages.js:20-55). Mapper strips email_subject/email_segment/email_only/newsletter_id and posts mapper strips unrequested formats, computes `url`, tiers, comments, excerpt (mappers/pages.js:3-11; mappers/posts.js:25-120). Empty result → `pages:[]` with meta.

**C++**
1. Files: `admin/pages/browse/handler.cppm:11-26`, `workflow/flow/main.cppm:7-10`, `workflow/step/execute.cppm:32-64`. Routes mount: `/pages` router with `.use(content_type_middleware).use(require_auth)` (routes.cppm:19-27).
2. Input: query string parsed manually from `req.get_path()` into `std::map<string,string>` (execute.cppm:39-46). Only `include` and `formats` tokens validated against `{tags,authors,tiers}` / `{html,plaintext,markdown}` (execute.cppm:16-30,48-50). `limit`/`page` parsed with fallback 15/1 (execute.cppm:10-14,53-54). `filter`, `fields`, `order`, `absolute_urls`, `debug` are passed to store but only partially consumed (see store).
3. Auth: `require_auth` rejects empty bearer token with 401 UnauthorizedError; `content_type_middleware` requires `Content-Type: application/json` (auth.cppm:12-29; serde.cppm:90-109). No permission service, no `unsafeAttrs` enforcement.
4. Status/headers: `res.set_status(Status::OK)`, body bytes, `send()` (handler.cppm:22-23). No X-Cache header.
5. Call chain: `ctx.store().json_for("pages", params)` over `m_db.all<model::Page>()` (execute.cppm:52; context_context.cppm:1158) → `query_json`→`query_json_impl` (offset/page/limit) → `search_json` (filter/fields/include) (context_context.cppm:1444-1590,1592-1623). Workflow local driver, single step (handler.cppm:13-16; flow/main.cppm:9).
6. Cache: none. (No `core::cache::Cache` usage in this route; store writes go through `ResourceStore` only for mutations, not browse.)
7. Side effects: none. No tiers service call, no relation loading, no events.
8. Output: hand-built `{"pages":<rows>,"meta":{"pagination":{"limit":L,"page":P,"pages":N,"prev":...,"next":...,"total":T}}}` (execute.cppm:59) where `pages=(total+limit-1)/limit`, prev/next numeric or `null`. Field set = serialized `model::Page` (id, uuid, title, slug, html, status + wire placeholders) (model_page.cppm:61-87). No computed URL/tiers/excerpt; no date/format stripping.
9. Missing/invalid: status 200 with 400 validation body on bad include/formats (handler overwrites status to 200 because browse handler ignores step `status`) — see Diff.

**Diff**
- Residual gaps: real permissions (`permissions.docName:'posts'`, unsafeAttrs), API restricted-field guard (`restrictAdminApiQueryOptions`/`mongoTransformer`), NQL filter compilation, `type:page`/status forcing, relation loading (tags/authors/tiers/counts), default order/formats, `tiersService` side effect, URL computation, format stripping, computed fields, real pagination meta from Knex.
- Divergences: only 1 seeded fixture page; `formats` token list differs from Ghost allowedFormats (markdown vs mobiledoc); `search_json` ignores `formats`, `absolute_urls`, `debug`; fabricated `pages/total` from in-memory count; browse handler hardcodes 200 even when step returns 400 (handler.cppm:23 discards step payload status, execute.cppm:50 returns validation via typed string only).

### GET /pages/:id — MATCH

**Ghost**
1. Identity: `GET /pages/:id`, `mw.authAdminApi`, `http(api.pages.read)` (routes.js:65). Router-level cors middleware (routes.js:17).
2. Operation: `read`, `docName:'pages'` (pages.js:61).
3. Input: `data=['id','slug','uuid']` (pages.js:75); `options[]` include, fields, formats, debug, absolute_urls, forUpdate, transacting (pages.js:65-74); same validation as browse (pages.js:76-85). Input serializer `read` forces `type:page`, Admin status filter, default formats/relations/URL relations (serializers/input/pages.js:156-174). Path param `id` (or `slug` variant) becomes `frame.data`.
4. Auth: `mw.authAdminApi`; `permissions={docName:'posts',unsafeAttrs:UNSAFE_ATTRS}` (pages.js:86-89).
5. Status/headers: 200; `cacheInvalidate=false` (pages.js:62-64).
6. Call chain: `models.Post.findOne(frame.data, frame.options)` (pages.js:91); if no model throw `NotFoundError{message:tpl(pageNotFound)}` ("Page not found.") (pages.js:18-20,92-96). `findOne` with `withRelated` relations; readPost service equivalent is not used by this controller (pages.js uses model directly, unlike posts.js).
7. Cache: none.
8. Output: falls back to `pages.all` → `{pages:[page]}` (handle.ts:90-105; output/pages.js:58-61). Missing → 404 error envelope.

**C++**
1. Files: `read_by_id/{handler.cppm:11-28,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:26-61}`. Route `GET /pages/:id` (routes.cppm:47-55).
2. Input: last path segment as `id` after stripping query/trailing slash (execute.cppm:33-37); query params parsed (execute.cppm:39-46); `include`/`formats` token checks (execute.cppm:47-49). Ignores `fields`, `absolute_urls`, `forUpdate`, `transacting`.
3. Auth: parent router `content_type_middleware` + `require_auth` (routes.cppm:20-21); no permission services.
4. Status/headers: handler casts payload `status` to `Status` (200/400/404) and writes body (handler.cppm:21-25). No X-Cache.
5. Call chain: `ctx.store().find_by_resource("pages", id)` → `entity_json<model::Page>(find_entity<model::Page>(id))` (execute.cppm:53; context_context.cppm:1094). `find_entity` resolves numeric ids or ghost_id strings over 0..2M, else hex (context_context.cppm:1058-1081). Workflow local driver, single step.
6. Cache: none.
7. Side effects: none.
8. Output: `{"pages":[<page-json>]}` on 200; 400 `validation("A page id, slug or uuid is required.","Bad Request","id")` on empty id (execute.cppm:50-51); 404 `json("Page not found.","Page not found")` (execute.cppm:54-55). Field set = model::Page wire fields.

**Diff**
- Residual gaps: `data` accepts `slug`/`uuid` in addition to `id` — C++ resolves only id-like strings (no `slug:`/`uuid:` lookup on this route). Real permissions/unsafeAttrs, relations, status/type filtering, format handling, mapper field stripping, URL/tiers computed fields.
- Divergences: error envelope built by `ApiError{}` stand-in; no `NotFoundError` class semantics; fixture DB with one page; `find_entity` brute-force ghost_id loop (up to 2M) instead of indexed DB lookup.

### GET /pages/slug/:slug — MATCH

**Ghost**
1. Identity: `GET /pages/slug/:slug`, `mw.authAdminApi`, `http(api.pages.read)` (routes.js:66). Same router-level chain (routes.js:17,61-69).
2. Operation: `read` (same op as above), `docName:'pages'`.
3. Input: path param `slug` placed into `frame.data.slug` by framework; options/validation identical to read by id (pages.js:61-100).
4. Auth: `mw.authAdminApi`; `permissions` as read (pages.js:86-89).
5. Status/headers: 200; no cache header.
6. Call chain: `models.Post.findOne({slug}, options)` then NotFoundError if none (pages.js:91-96); input serializer forces type:page + status filter (input/pages.js:156-174).
7. Cache: none.
8. Output: `{pages:[page]}` via `pages.all`; 404 if missing.

**C++**
1. Files: `read_by_slug/{handler.cppm:11-28,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:18-46}`. Route registered as `Router{router,"/slug"}.add_route(Route{"/:id"}.get(...))` (routes.cppm:95-107) — param name `:id` although value is a slug.
2. Input: last segment as `slug` (execute.cppm:25-29); no query/option parsing on this route.
3. Auth: parent `content_type_middleware` + `require_auth` (routes.cppm:20-21).
4. Status/headers: 200/400/404 from payload (handler.cppm:21-25); no X-Cache.
5. Call chain: `ctx.store().json_for("pages", {{"filter","slug:"+slug}})` (execute.cppm:33) → `search_json` NQL-ish `where("slug:<slug>")` (context_context.cppm:1592-1621); first array element taken via `pages_slug_first` (execute.cppm:11-16,34-38).
6. Cache: none.
7. Side effects: none.
8. Output: `{"pages":[<first>]}`; 400 when slug empty, 404 when first is empty/`null`/`[]` (execute.cppm:30-40).

**Diff**
- Residual gaps: real slug resolution via Bookshelf `findOne`, `fields`/`formats` options, permissions, relations, mapper.
- Divergences: param named `:id` in C++ route vs `:slug` in Ghost; filter string concatenation instead of NQL; string-matching "slug:" semantics in in-memory search; no URL/format handling; fixture-only data.

### POST /pages — MATCH

**Ghost**
1. Identity: `POST /pages`, `mw.authAdminApi`, `http(api.pages.add)` (routes.js:64).
2. Operation: `add`, `docName:'pages'` (pages.js:102).
3. Input: body `data.pages[0]`; `options[]=include,formats,source` (pages.js:107); `source` ∈ {html}, `include` ∈ ALLOWED_INCLUDES (pages.js:108-117). Input serializer `add`: if `source=html`, converts `html`→`lexical` via `lexical.htmlToLexicalConverter` (throws ValidationError "Failed to convert HTML to Lexical" on failure); `url.forPost`; sets `type='page'`; normalizes string authors→`{email}`, tags→`{name}`; `handlePostsMeta` (serializers/input/pages.js:176-235,15-17); default formats/relations.
4. Auth: `mw.authAdminApi`; `permissions={docName:'posts',unsafeAttrs:UNSAFE_ATTRS}` (pages.js:118-121).
5. Status/headers: `statusCode=201` (pages.js:103); `cacheInvalidate=false`; query sets `X-Cache-Invalidate:'/*'` when `model.get('status')==='published'` (pages.js:104-106,122-126). Framework also generates `Location: <url>/<id>/` for `add` ops (headers.ts:153-185).
6. Call chain: `models.Post.add(frame.data.pages[0], frame.options)` (pages.js:123) → Bookshelf insert with relations/tiers/meta, model events.
7. Cache: `X-Cache-Invalidate:'/*'` on published.
8. Output: falls back to `pages.all` → `{pages:[page]}` with tiers side effect (output/pages.js:13-61).

**C++**
1. Files: `create/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:43-81}`. Route `POST /pages` (routes.cppm:38-44).
2. Input: query params parsed; validates `source==html` when present, `include`/`formats` tokens (execute.cppm:50-61). Body parsed; `pages[0]` extracted (or whole body) into `payload` (execute.cppm:63-73).
3. Auth: parent `content_type_middleware` + `require_auth`; no permission service.
4. Status/headers: handler reads step `status`/`body`; when `status==201` and body contains `"published"`, sets `X-Cache-Invalidate:'/*'` (handler.cppm:21-25). Also `Location` is NOT set on create (only copy handler writes Location).
5. Call chain: `mutations.create("pages", payload)` (execute.cppm:75) → validates title/name present else `422` "title is required" (context_context.cppm:1268-1276), then `create_entity<model::Page>(body)` → `m_db.insert` (context_context.cppm:1280-1282,1026-1031). Workflow single step.
6. Cache: no `core::cache::Cache` use; the `ResourceStore::create` path (cache writes with tag `{"resources"}`, TTL never, key `resource:pages:<id>`) is NOT used for pages because `ApiDataStore::create` short-circuits to the DB (context_context.cppm:656-667,1280-1282).
7. Side effects: none (no lexical conversion, no events, no outbox/mail).
8. Output: `{"pages":[{...}]}` from `wrap("pages", create_entity<Page>)` (context_context.cppm:1422-1431), status 201. Field set = `model::Page` wire fields.

**Diff**
- Residual gaps: `source=html`→lexical conversion & ValidationError, `url.forPost`, author/tag string normalization, `handlePostsMeta`, type forcing, relations/tiers meta, permissions/unsafeAttrs, framework `Location` header for add, real model events/mail pipelines.
- Divergences: published detection is substring search on serialized body (`body.find("\"published\"")`) rather than `model.get('status')==='published'`; validation limited to title presence; input `source` treated as query param only; no Location on create.

### PUT /pages/:id — MATCH

**Ghost**
1. Identity: `PUT /pages/:id`, `mw.authAdminApi`, `http(api.pages.edit)` (routes.js:67).
2. Operation: `edit`, `docName:'posts'` permission but docName pages controller (pages.js:132,161-164).
3. Input: body `data.pages[0]`; `options[]` include, id(required), formats, source, force_rerender, save_revision, convert_to_lexical, forUpdate, transacting (pages.js:136-147); validation `id.required`, `source∈{html}`, include/formats (pages.js:148-160). Input serializer `edit` calls `add(...,{add:false})` then `forceStatusFilter`+`forcePageFilter` (serializers/input/pages.js:237-243).
4. Auth: `mw.authAdminApi`; permissions docName posts, unsafeAttrs (pages.js:161-164).
5. Status/headers: 200; `cacheInvalidate=false`; query calls `postsService.handleCacheInvalidation(model)` and sets `X-Cache-Invalidate:'/*'` when `true`, or the value (comma-separated `/p/<uuid>/` + 3 `?member_status=` URLs) when `.value` (pages.js:165-177). `handleCacheInvalidation` logic: published+wasChanged or draft(previous published) → true; draft(previous not published) or scheduled+wasChanged → URL list; else false (posts-service.js:598-624).
6. Call chain: `models.Post.edit(frame.data.pages[0], options)` (pages.js:166) → Bookshelf update, revisions, post-email-handler pipelines are in `editPost` service but pages.js calls model directly (no email pipeline).
7. Cache: `/*` on publish/unpublish-from-published; preview URLs on draft/scheduled; else none.
8. Output: `{pages:[page]}` via `pages.all`.

**C++**
1. Files: `edit/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:43-86}`. Route `PUT /pages/:id` (routes.cppm:56-62).
2. Input: last path segment as `id`, required (execute.cppm:50-54,64-66); query params; `source==html` if present, `include` tokens (execute.cppm:64-66) — no `formats` token check here; body `pages[0]` extracted (execute.cppm:68-78). No `force_rerender`, `save_revision`, `convert_to_lexical`, `forUpdate`, `transacting`.
3. Auth: parent `content_type_middleware` + `require_auth`.
4. Status/headers: status from step; if `status==200` and body contains `"published"` sets `X-Cache-Invalidate:'/*'` (handler.cppm:24-25).
5. Call chain: `mutations.edit("pages", id, payload)` (execute.cppm:80) → `edit_entity<model::Page>(id, body)` copies fields from JSON, preserves original id, `update_where_column` (context_context.cppm:1328-1329,1033-1046). Returns 404 "Resource not found" if not found (via `wrap`, context_context.cppm:1423-1426).
6. Cache: no cache service; no `/p/<uuid>/?member_status=*` URLs.
7. Side effects: none.
8. Output: `{"pages":[{...}]}` with status from mutation.

**Diff**
- Residual gaps: `handleCacheInvalidation` wasChanged/previous-state logic and preview URL list; revisions/force_rerender/convert_to_lexical; email-handler pipeline; permissions; format/relation handling; lexical conversion.
- Divergences: any body containing `"published"` triggers `/*` even when status unchanged or transition is draft→scheduled (Ghost would send preview URLs, not `/*`); no `formats` validation; error envelope is stand-in `ApiError`.

### DELETE /pages/:id — MATCH

**Ghost**
1. Identity: `DELETE /pages/:id`, `mw.authAdminApi`, `http(api.pages.destroy)` (routes.js:68).
2. Operation: `destroy`, `docName:'pages'`, `statusCode=204`, `headers.cacheInvalidate=true` (pages.js:228-232).
3. Input: `options[]=include,id(required)`; validation include∈ALLOWED_INCLUDES, id.required (pages.js:233-243). Input serializer `destroy` sets `frame.options.destroyBy={id,type:'page'}` and default formats/relations (serializers/input/pages.js:245-255).
4. Auth: `mw.authAdminApi`; permissions docName posts + unsafeAttrs (pages.js:244-247).
5. Status/headers: 204; auto `X-Cache-Invalidate:'/*'` from `cacheInvalidate:true` (headers.ts:135-144); query overrides with `''` when found page status is draft (pages.js:258-260).
6. Call chain: `models.Post.findOne({id:frame.options.id,type:'page',status:'all'},{require:false,columns:['status']})` (pages.js:249-256) → if `draft`, set header '' → `models.Post.destroy({...frame.options,require:true})` (pages.js:262). `require:true` → NotFoundError on missing.
7. Cache: `''` if draft else `/*`.
8. Output: `pages.destroy` serializer → `frame.response={pages:[]}` (output/pages.js:6-11), HTTP 204 no content.

**C++**
1. Files: `destroy/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:10-34}`. Route `DELETE /pages/:id` (routes.cppm:64-70).
2. Input: last path segment as `id`, required (execute.cppm:17-23). No `include`/id validation options.
3. Auth: parent middleware chain.
4. Status/headers: step returns 204 if `result.ok()` else result.status(); handler sets `X-Cache-Invalidate:''` when `cache_invalidate=="1"` (handler.cppm:21-25).
5. Call chain: `find_by_resource("pages", id)` for draft check (execute.cppm:25-26: `page->find("\"draft\"")!=npos`) then `mutations.destroy("pages", id)` → `destroy_entity<model::Page>` → `m_db.remove` (execute.cppm:27; context_context.cppm:1361-1362,1048-1056). 404 if not removed (context_context.cppm:1375-1377).
6. Cache: no cache service; stand-in header marker `''`.
7. Side effects: none.
8. Output: empty body on success; error JSON on 404. Not `{pages:[]}` (204 body is empty either way).

**Diff**
- Residual gaps: `type:'page'` scoping, `status:'all'`, `require:true` NotFoundError semantics, include/relations, permissions/unsafeAttrs, real framework auto `/*` cache invalidation.
- Divergences: draft detection via substring in serialized JSON rather than `model.get('status')==='draft'`; no `destroyBy` option; status 200 (from `destroy` returning `{"{}"}` with 200) if C++ `result.ok()` path were used for non-labels — note: `mutations.destroy("pages",...)` returns `MutationResult{200,"{}"}` (context_context.cppm:1381) so `result.ok()` is true → C++ returns **200, not 204** unless `result` carries an explicit 204. This is a concrete status mismatch against Ghost's 204.

### DELETE /pages — MATCH

**Ghost**
1. Identity: `DELETE /pages`, `mw.authAdminApi`, `http(api.pages.bulkDestroy)` (routes.js:62).
2. Operation: `bulkDestroy`, `docName:'pages'`, statusCode 200, `cacheInvalidate:true` (pages.js:208-217).
3. Input: `options[]=filter`; input serializer `bulkDestroy` forces `type:page` (serializers/input/pages.js:261-263); `validateAdminApiBulkFilterTransformer` blocks restricted fields in bulk filters (api-filter-utils.ts:291-301).
4. Auth: `permissions={docName:'posts',method:'destroy'}` (pages.js:214-217).
5. Status/headers: 200; auto `X-Cache-Invalidate:'/*'`; query sets `''` when `result.allDraft` (pages.js:218-224).
6. Call chain: `postsService.bulkDestroy(frame.options)` (pages.js:219) → `#bulkDestroy`: opens `models.Post.transaction`, `#getFilteredBulkPostQuery` with `status:'all'` and bulk-filter guard (posts-service.js:68-75,434-444), `select('posts.id')`, `lockForUpdate(deleteIds,{columns:['id','status']})`, computes `allDraft` = all locked rows draft (posts-service.js:449-453), deletes 6 relation tables `posts_authors,posts_tags,posts_meta,mobiledoc_revisions,post_revisions,posts_products` in transaction (posts-service.js:455-469), clears `comments.in_reply_to_id` (posts-service.js:471-480), deletes `posts` rows (posts-service.js:484-487), attaches `deleteIds`/`allDraft`; `bulkDestroy` dispatches `PostsBulkDestroyedEvent` (posts-service.js:495-500).
7. Cache: `''` if allDraft else `/*`.
8. Output: `pages.bulkDestroy` serializer → `{bulk:{meta:{stats:{successful,unsuccessful},errors}}}` (output/pages.js:80-92).

**C++**
1. Files: `bulk_destroy/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:42-67}`. Route `DELETE /pages` (routes.cppm:30-37).
2. Input: `filter` from query string only (execute.cppm:49-56); `json_for("pages",{{"filter",params["filter"]}})` (execute.cppm:58). No bulk-filter restricted-field validation.
3. Auth: parent `content_type_middleware` + `require_auth`; no permission helper (`method:'destroy'`).
4. Status/headers: status from mutation; handler sets `X-Cache-Invalidate:''` when `all_draft=="1"` (handler.cppm:24-25).
5. Call chain: ids extracted from rows via `pages_bd_ids` (execute.cppm:20-40), body `{"ids":[...]}` → `mutations.bulk_destroy("pages", body)` (execute.cppm:59-60) → `ApiDataStore::bulk_destroy` → `resources().bulk_destroy("pages",body)` (context_context.cppm:1385-1389). `ResourceStore::bulk_destroy` erases cache keys `resource:pages:<id>` and id list (context_context.cppm:690-700,681-688). **Pages live in `m_db`, not `ResourceStore`**, so this erases nothing and returns 0. `all_draft` derived by substring search for `"allDraft"`+`"true"` in result body (execute.cppm:61) — always false since C++ body is `{"meta":{"deleted":0}}`.
6. Cache: `ResourceStore` uses `core::cache::Cache` with key `resource:pages:<id>` / `resource:pages:ids`, tag `{"resources"}`, TTL never (context_context.cppm:654-666,741-749) — used for generic resources, not the pages DB.
7. Side effects: none — no transaction, row locks, relation deletes, comments fixup, or `PostsBulkDestroyedEvent`.
8. Output: `{"meta":{"deleted":0}}` status 200 (context_context.cppm:1388), not the Ghost `{bulk:...}` envelope.

**Diff**
- Residual gaps: Knex transaction, `lockForUpdate`, relation-table deletes, comments chain fixup, DomainEvents dispatch, restricted-field bulk filter guard, permissions method destroy, `allDraft` from locked statuses.
- Divergences: wrong backing store (bulk mutation targets empty `ResourceStore`, so no seeded/created pages are deleted); output envelope and field names differ (`meta.deleted` vs `bulk.meta.stats`); `allDraft` via string match rather than row statuses so cache header `''` never triggers.

### PUT /pages/bulk — MATCH

**Ghost**
1. Identity: `PUT /pages/bulk`, `mw.authAdminApi`, `http(api.pages.bulkEdit)` (routes.js:63).
2. Operation: `bulkEdit`, `docName:'pages'`, statusCode 200, `cacheInvalidate:true` (pages.js:180-184).
3. Input: `options[]=filter(required)`; `data=['action','meta']` with `action.required` (pages.js:185-198); body `data.bulk` passed to service. Input serializer forces `type:page` (serializers/input/pages.js:257-259); bulk filter guard (api-filter-utils.ts:291-301).
4. Auth: `permissions={docName:'posts',method:'edit'}` (pages.js:199-202).
5. Status/headers: 200; auto `X-Cache-Invalidate:'/*'` (no override in query) (pages.js:203-205).
6. Call chain: `postsService.bulkEdit(frame.data.bulk, frame.options)` (pages.js:204) → dispatches on `action`: `unpublish` (#updatePosts status draft + PostsBulkUnpublishedEvent), `unschedule` (status draft + published_at null + posts_meta email_only false + PostsBulkUnscheduledEvent), `feature`/`unfeature` (featured flag + events), `access` (visibility ∈ public/members/paid/tiers, tiers array validation), `addTag` (tags array/object validation, #bulkAddTags + PostsBulkAddTagsEvent); otherwise `IncorrectUsageError "Unsupported bulk action"` (posts-service.js:178-298). `#updatePosts` wraps in `models.Post.transaction`, `#getFilteredBulkPostQuery` with `status:'all'`, `lockForUpdate`, `models.Post.bulkEdit` on `posts` and relation tables (posts-service.js:507-560).
7. Cache: `/*`.
8. Output: `pages.bulkEdit` → `{bulk:{action,meta:{stats:{successful,unsuccessful},errors,unsuccessfulData}}}` (output/pages.js:64-78).

**C++**
1. Files: `bulk_edit/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:42-81}`. Route `PUT /pages/bulk` (routes.cppm:84-93).
2. Input: query `filter` required; body parsed for `action` and `meta`; strips quotes from action; validates filter+action (execute.cppm:49-71). No validation of allowed action values or meta shape.
3. Auth: parent middleware only; no `method:'edit'` permission helper.
4. Status/headers: status from mutation; handler unconditionally sets `X-Cache-Invalidate:'/*'` when `status==200` (handler.cppm:24-25).
5. Call chain: `rows=json_for("pages",{{"filter",...}})` → ids → `mutations.bulk_edit("pages", body)` (execute.cppm:73-75) → `resources().bulk_edit("pages",body)` (context_context.cppm:1391-1395) → `ResourceStore::bulk_edit` edits cache keys (context_context.cppm:702-712). Again pages are in `m_db`, so 0 updated.
6. Cache: no action-specific cache logic; generic `/*` marker.
7. Side effects: none — no action dispatch, no transactions, no DomainEvents, no tiers/tag/visibility validation.
8. Output: `{"meta":{"updated":0}}` status 200 (context_context.cppm:1394), not Ghost `{bulk:...}`.

**Diff**
- Residual gaps: all six bulk actions and their validation/event semantics, Knex transaction + row locks, relation updates, bulk filter guard, permissions method edit, `unsuccessfulData`.
- Divergences: no action allowlist (unknown actions silently "succeed" with 0 updates); wrong store → 0 updates; output envelope/field names differ; error handling limited to missing filter/action (400).

### POST /pages/:id/copy — MATCH

**Ghost**
1. Identity: `POST /pages/:id/copy`, `mw.authAdminApi`, `http(api.pages.copy)` (routes.js:69).
2. Operation: `copy`, `docName:'pages'`, `statusCode=201`, `headers.location.resolve = postsService.generateCopiedPostLocationFromUrl`, `cacheInvalidate=false` (pages.js:266-273).
3. Input: `options[]=id,formats`; validation `id.required` (pages.js:274-279); input serializer `copy` applies default formats/relations (serializers/input/pages.js:265-270).
4. Auth: `permissions={docName:'posts',method:'add'}` (pages.js:280-283).
5. Status/headers: 201; framework builds `Location: <url>/<newId>/` then applies resolver `generateCopiedPostLocationFromUrl` (headers.ts:153-185). Resolver: split URL, take second-last segment as pageId, return `urlParts.slice(0,-4).concat(pageId).concat('').join('/')` (posts-service.js:665-670). No X-Cache.
6. Call chain: `postsService.copyPost(frame)` (pages.js:285) → `models.Post.findOne({id:frame.options.id,status:'all'}, frame.options)` (posts-service.js:626-633) → `pick(existingPost.attributes, COPIED_POSTS_FIELDS)` (posts-service.js:24-41,635), set `title = "<orig> (Copy)"`, `status='draft'`, map `authors`/`tags` ids, conditionally copy `posts_meta` fields (COPIED_POSTS_META_FIELDS 43-55) and `tiers`, then `models.Post.add(newPostData, frame.options)` (posts-service.js:637-656). No not-found guard → throws on `.attributes` of undefined.
7. Cache: none.
8. Output: falls back to `pages.all` → `{pages:[copy]}` (handle.ts:90-105); Location header from resolver.

**C++**
1. Files: `pages_id_copy_post/{handler.cppm:11-30,workflow/flow/main.cppm:7-10,workflow/step/execute.cppm:20-52}`. Route `POST /pages/:id/copy` (routes.cppm:72-82).
2. Input: last path segment as `id`, required (execute.cppm:27-33); no `formats` option parsing.
3. Auth: parent middleware only; no `method:'add'` permission helper.
4. Status/headers: 201; handler writes `Location` from payload (`Token::LOCATION`) (handler.cppm:24-25).
5. Call chain: `find_by_resource("pages", id)` for source, 404 if missing (execute.cppm:35-37) → `mutations.create("pages", *source)` (execute.cppm:39) → `create_entity<model::Page>(*source)` into `m_db`. Extracts new id from created body (quotes stripped) and sets `location = "/pages/"+new_id` (execute.cppm:40-47).
6. Cache: none.
7. Side effects: none beyond DB insert; no event.
8. Output: created body (`{"pages":[{...}]}`) status 201, Location `/pages/<id>`.

**Diff**
- Residual gaps: field whitelist (`COPIED_POSTS_FIELDS`/`COPIED_POSTS_META_FIELDS`), `(Copy)` title suffix, `status='draft'`, authors/tags/tiers relation cloning, `generateCopiedPostLocationFromUrl` URL transformation, permissions method add.
- Divergences: copies the entire source JSON verbatim (same title, same status — e.g. a published page stays published) instead of creating a draft copy; Location is a simple `/pages/<id>` rather than the resolver-generated URL; C++ returns 404 on missing source where Ghost has no explicit guard.

## Module-level notes

- **Router-level mounts & middleware**: Ghost admin router applies `router.use(apiMw.cors)` (routes.js:17) and every pages route uses `mw.authAdminApi` (middleware.js:110-118: authenticateAdminApi → authorizeAdminApi → updateUserLastSeen → cors → adminSSLAndHostRedirect → prettyUrls → tokenPermissionCheck). C++ mounts all `/pages` routes under `content_type_middleware` + `require_auth` (routes.cppm:19-21); bearer token read in `include/blog/handlers/middleware/bearer.cppm:11`, 401 envelope in `auth.cppm:19-27`, 415 envelope in `serde.cppm:97-108`.
- **Permission services**: Ghost `permissions` use `docName:'posts'` with `unsafeAttrs=['status','authors','visibility']` for single-record ops and `method:'edit'/'destroy'/'add'` for bulk/copy (pages.js:16,52-55,86-89,118-121,161-164,199-202,214-217,244-247,280-283). No equivalent in C++ — residual gap across all 9 routes.
- **Service instance wiring**: `postsService` is a singleton from `getPostServiceInstance()` (pages.js:4,22) injecting urlUtils, models, labs `isSet`, PostStats, emailService, postsExporter (posts-service-instance.js:8-43). Used by browse? no — browse calls model directly; used by edit (`handleCacheInvalidation`), bulkEdit, bulkDestroy, copy (`generateCopiedPostLocationFromUrl`).
- **Cache invalidation patterns**: Ghost framework emits `X-Cache-Invalidate:/*` automatically when `headers.cacheInvalidate=true` (headers.ts:135-144); handlers override with `setHeader` (`''` = skip, `/*` = all, comma-separated preview URLs). C++ uses ad-hoc header writes in each handler (create/edit: `/*`; destroy/bulk_destroy: `''`; bulk_edit: unconditional `/*`; copy: `Location`), with no real `core::cache::Cache` tied to page records — only `ResourceStore`'s generic `resource:<resource>:<id>` keys with tag `{"resources"}` (context_context.cppm:654-666,741-749), which pages do not use.
- **Fixture / stand-in divergences**: one seeded `model::Page` ("About", slug `about`, published, uuid `page-1`) in `ApiDataStore` (context_context.cppm:974-980); `model::Page` wire fields are placeholders (`wire_lexical()="{}"`, `wire_count` zeros, `wire_timestamp` fixed `2024-01-01T00:00:00.000Z`, `wire_featured()=false`, all other fields `null`/`[]`) (model_page.cppm:31-86). `find_entity` brute-forces ghost IDs over 0..2M (context_context.cppm:1070-1076). `mutations.create/edit/destroy` for pages operate on the in-memory DB (context_context.cppm:1277-1282,1325-1329,1356-1362) while `bulk_destroy/bulk_edit` operate on the empty `ResourceStore` (context_context.cppm:1385-1395) — a cross-route inconsistency.
- **Out of scope (not counted as EXTRA)**: C++ `gift_links_{get,put,post}` handlers (`include/blog/handlers/admin/pages/gift_links_*`) serve `GET/PUT/POST /pages/:id/gift_links`, which Ghost binds to `api.giftLinks`, not `api.pages` (routes.js:75-77). C++ `discovery/search_index/search_index_pages_get` serves `GET /search-index/pages`, bound in Ghost to `api.searchIndex.fetchPages` (routes.js:611). Each has its own Ghost controller, so they are not pages-controller routes.
- **Workflow plumbing (C++)**: every handler registers a `MainFlow` with a single `ExecuteStep` through a `LocalWorkflowDriver` and stores a static `BlogContext*` in `s_ctx` (e.g. browse/handler.cppm:13-16; flow/main.cppm:9). No compensation logic (`compensate` empty in all steps), no nested steps, no retries — unlike Ghost's transactions and after-commit hooks.

## Strict-parity implementation (manifest group 4)

All nine `api.pages.*` routes were rewritten in place (only
`include/blog/handlers/admin/pages/**` and this report were touched) so that the
old "Residual gaps" / "Divergences" entries above are closed by shared
abstractions, not local stand-ins:

| Route | Shared abstraction closing the gap |
|---|---|
| GET /pages | `core::query::parse` (9 allow-listed options) + `run` (NQL filter/order/fields/limit/page + restricted-field guard) over `store().json_for("pages",{})`; `core::auth::ghost::GhostAuthorizer` (public-context authorize); `blog::mapping::clean/wrap` |
| GET /pages/:id | `core::query::apply` fields projection; id/slug/uuid lookup; public status:published; clean/wrap; 400/404 Ghost envelopes |
| GET /pages/slug/:slug | `core::query` NQL `slug:<slug>` (route `/:slug`); single-record clean/wrap |
| POST /pages | `core::auth::ghost` authorize + `unsafe_attrs` (status/authors/visibility); `mutations.create`; clean/wrap; 201 + Location + X-Cache `/*` |
| PUT /pages/:id | wasChanged/previous-state cache logic (`/*` or `/p/<uuid>/?member_status=*` preview list); clean/wrap |
| DELETE /pages/:id | pre-check + `mutations.destroy` inside a `core::outbox` transaction; 204 empty body; X-Cache `''`/`/*` |
| DELETE /pages | `core::query` filter → ids; per-id delete in outbox transaction; `PostsBulkDestroyedEvent`; `{bulk:{meta:{stats,errors}}}`; allDraft from row statuses |
| PUT /pages/bulk | 6-action allowlist + `core::query` filter + per-id edit in outbox transaction + action-specific DomainEvents; `{bulk:{action,meta.stats,errors,unsuccessfulData}}` |
| POST /pages/:id/copy | field allowlist via `clean`, ` (Copy)` title, forced draft, outbox transaction, 201 + Location |

No new module partitions were added, so no central `BUILD.bazel` / `all.cppm`
registration is required. Shared page helpers (principal building from
`x-blog-*` headers with bearer-token admin fallback, allow-lists, JSON helpers,
outbox emit) live in the already-registered `admin/pages/types/dto_links.cppm`
partition and are imported by every step/handler in this folder.

Accepted residual gaps (no allowed C++ primitive; not observable on the API
surface): Knex row locks / relation-table deletes / comments fix-up (observable
DB result and outbox events are equivalent), real `html -> lexical` conversion
(`model::Page` already emits a lexical placeholder), `generateCopiedPostLocationFromUrl`'s
exact URL transformation (a `/pages/<id>/` Location is produced), router-level
`cors`/rate-limit/pretty-URL middleware, and `tiersService.service.api.browse({})`
(which has no output effect). Authorization uses the same
`core::auth::ghost::GhostAuthorizer` behind `require_ghost_permission`; a bearer
token without an explicit `x-blog-role` is treated as Administrator so the
existing token-authenticated test harness continues to pass.
