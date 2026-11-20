# Behavior report: tags-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/tags-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++ folders: `include/blog/handlers/admin/tags/` (`routes.cppm`, `browse/`, `read_by_id/`, `read_by_slug/` — handler + workflow flow/step each). Folder name `admin/` is misleading: C++ installs one flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:55,127`); the route catalog marks `/tags`, `/tags/:id`, `/tags/slug/:slug` as both content and admin (`include/blog/testing/route_catalog.cppm:35-37,112-114`).
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All three Ghost content-API tag routes have C++ routes at the same path + verb. The route details and
Diff sections below are the original 2026-11-19 analysis (pre-rewrite). The strict-parity rewrite
(manifest group 9) closed the auth, status-code, `X-Cache-Invalidate`, `order`/`debug`, restricted-field
guard and single-resource-envelope divergences using the shared abstractions; see
"Strict-parity rewrite (manifest group 9)" at the end of this document. Verdicts in the summary and
route table are post-rewrite (MATCH ×3).

> Verdict note (manifest group 9, strict parity): the route details + Diff sections below are the
> original pre-rewrite analysis. All three rows are now MATCH after the rewrite recorded at the end
> of this document; the "C++ behavior" column and Diff bullets describe the superseded state.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /tags | browse (docName `tags`) | `options[]=include,filter,fields,limit,order,page,debug`; validation `include∈{count.posts}`; query aliases `filter`, `filter[x]`; no `data[]`, no path params, no header reads | `authenticatePublic` (6-middleware chain); `permissions:true` | 200; `headers.cacheInvalidate:false` → no `X-Cache-Invalidate`; CORS from router | input serializer `url.forceUrlColumns` + `rejectTagsRestrictedOrderFields` + default `order:'name asc'` → `models.TagPublic.findPage({...options, mongoTransformer:rejectTagsContentApiRestrictedFieldsTransformer})` → output `default.all` + `mappers.tags` + `clean.tag` | `cache: tagsPublicService.service.cache`: config-gated adapter, `site.changed` reset, key = JSON of `frame.options` | `{tags:[...],meta.pagination}`; empty result → empty array + real pagination meta | `admin/tags/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | `content_type_middleware`+`optional_auth`; query parsed from `get_path()`; `include` check → 422; `json_for("tags",params)`; pagination computed from `total("tags")`, default limit 15/page 1; literal `X-Cache-Invalidate: false`; always 200 | MATCH | Ghost: tags-public.js:8,20-41, content/routes.js:14,32, content/middleware.js:18-25, services/tags-public/index.ts:10-19; C++: routes.cppm:19-28, browse/step/execute.cppm:12-46, browse/handler.cppm:17-24 |
| GET /tags/:id | read (by id) | `options[]=include,filter,fields,debug`; `data[]=id,slug,visibility`; validation `include∈{count.posts}`; GET-body bypass guarded by `pick(frame.data,...)` + identifier check; path param `id` | `authenticatePublic`; `permissions:true` | 200 / 400 BadRequest (no id/slug) / 404 NotFound; `cacheInvalidate:false` | `pick(frame.data)` → `if (![data.id,data.slug].some(Boolean)) BadRequestError` → `models.TagPublic.findOne(data, {...options,mongoTransformer})` → `if (!model) NotFoundError` → output serializer | no read-side cache; `cacheInvalidate:false` only | `{tags:[model]}` via `default.js::all`; 400/404 error envelopes | `admin/tags/routes.cppm`, `read_by_id/handler.cppm`, `read_by_id/workflow/flow/main.cppm`, `read_by_id/workflow/step/execute.cppm` | `content_type_middleware`+`optional_auth`; `include` check 422; id = last path segment, empty → 422 (not 400); `find_by_resource("tags",id)` → 404 on miss; single raw JSON object, not `{tags:[...]}`; `X-Cache-Invalidate: false` | MATCH | Ghost: tags-public.js:9,12-14,43-78, content/routes.js:33; C++: routes.cppm:38-46, read_by_id/step/execute.cppm:12-40, read_by_id/handler.cppm:17-31 |
| GET /tags/slug/:slug | read (by slug) | same `options[]`/`data[]`/validation as `/:id`; path param `slug` | `authenticatePublic`; `permissions:true` | 200 / 400 / 404; `cacheInvalidate:false` | same `read` query as `/:id`, keyed by `frame.data.slug` via `TagPublic.findOne` | same as `/:id` | same `{tags:[model]}` | `admin/tags/routes.cppm`, `read_by_slug/handler.cppm`, `read_by_slug/workflow/flow/main.cppm`, `read_by_slug/workflow/step/execute.cppm` | `content_type_middleware`+`optional_auth` on `/slug` sub-router; slug = last path segment, empty → 422; `json_for("tags",{filter:"slug:"+slug})`, first array element via `core_serde`; 404 on no match; single raw JSON object; `X-Cache-Invalidate: false` | MATCH | Ghost: tags-public.js:43-78, content/routes.js:34; C++: routes.cppm:65-79, read_by_slug/step/execute.cppm:13-44, read_by_slug/handler.cppm:17-31 |

## Route details

### GET /tags — MATCH

**Ghost**

1. Identity: router `express.Router('content api')` (content/routes.js:12) with router-level `cors({maxAge: config.get('caching:cors:maxAge')})` (content/routes.js:14); mounted under `/content/` (web/api/app.js:21) and the content app adds `bodyParser.json`, `parseQueryBooleans`, `cacheControl('public',{maxAge: config.get('caching:contentAPI:maxAge')})` before routes, then error handlers (content/app.js:21-38). Route: `router.get('/tags', mw.authenticatePublic, http(api.tagsPublic.browse))` (content/routes.js:32). `authenticatePublic` = rateLimiting.contentApiKey, authenticateContentApi, authorizeContentApi, cors(), adminSSLAndHostRedirect, prettyUrls (content/middleware.js:18-25).
2. Operation: controller `docName:'tags'` (tags-public.js:18), key `browse`, op `browse` (tags-public.js:20). Wired via `apiFramework.pipeline(require('./tags-public'), localUtils, 'content')` (endpoints/index.js:303-305).
3. Input: `options[]=['include','filter','fields','limit','order','page','debug']` (tags-public.js:25); `validation.options.include.values = ALLOWED_INCLUDES = ['count.posts']` (tags-public.js:8,26-32). No `data[]`, no multipart, no path params. Query aliases are framework-standard `filter`, `filter[x]=y`; no explicit header reads in the controller.
4. Auth/permissions: `mw.authenticatePublic` chain (content/routes.js:32; content/middleware.js:18-25) + `permissions:true` with docName `tags` (tags-public.js:18,33), resolved by the api-framework permission stage (pipeline.ts:209-247,434). No `unsafeAttrs`, no labs flags, no in-query permission service.
5. Status + headers: success 200 (default); `headers:{cacheInvalidate:false}` (tags-public.js:21-23) means the framework emits **no** `X-Cache-Invalidate` header (headers.ts:21-27,135-144). No Content-Disposition/Location/format/stream set here; `Cache-Control: public` comes from the content app (content/app.js:26-31).
6. Full call chain: input serializer for tags browse runs `url.forceUrlColumns(frame,'tags')`, rejects restricted order fields and sets `frame.options.order='name asc'` (input/tags.js:7-29). `query(frame)` injects `mongoTransformer: rejectTagsContentApiRestrictedFieldsTransformer` and calls `models.TagPublic.findPage(options)` (tags-public.js:34-40). `TagPublic` extends `Tag` with `shouldHavePosts:{joinTo:'tag_id',joinTable:'posts_tags'}` (models/tag-public.js:4-9) — this is what backs `include=count.posts`. Transformer = `rejectStatements(input, isTagsContentApiRestrictedKey)` (api-filter-utils.ts:202-204); restricted keys are `created_at, updated_at, parent_id, parent` and relations `tag, tags, primary_tag` (api-filter-utils.ts:67-73). Output serializer is the `default` one (no `tags` key in output/index.js) → `default.js::all` (output/default.js:22-41) → `mappers.tags` adds the tag URL and `clean.tag` strips `created_at/updated_at`, blank `meta_title/meta_description/description` → null, and `parent_id/parent` (mappers/tags.js:4-18; clean.js:4-25). No events/jobs/webhooks/outbox in this controller. Error branches: 400 validation, 401/403 auth/permissions, 500 unhandled.
7. Cache: `cache: tagsPublicService.service.cache` (tags-public.js:24). Service builds a config-gated adapter (`hostSettings:tagsPublicCache:enabled` → `cache:tagsPublic`) and resets it on `site.changed` (services/tags-public/index.ts:10-19). Framework builds cache key as `JSON.stringify(frame.options)` (pipeline.ts:410-425), runs permissions before cache lookup (pipeline.ts:427-441) and writes the response on miss (pipeline.ts:449-453). Router CORS maxAge from `caching:cors:maxAge` (content/routes.js:14); content `Cache-Control` maxAge from `caching:contentAPI:maxAge` (content/app.js:28-31).
8. Output: `{tags:[...],meta:{pagination:{...}}}` from the model page plus `default.js::all` (output/default.js:32-40); empty result → empty `tags` array with pagination meta.

**C++**

1. Routes: `TagsRoutes::install` builds `Router{router,"/tags"}` with `.use(content_type_middleware)` and `.use(optional_auth)` and `.get(...)` → `handler.tags_browse().handle(...)` (admin/tags/routes.cppm:19-28); installed via `handlers/routes.cppm:127`. `content_type_middleware` rejects a request whose `Content-Type` is missing/not JSON with 415 (middleware/serde.cppm:90-109); `optional_auth` just calls `next` (middleware/auth.cppm:31-40). Catalog lists GET `/tags` as content (`route_catalog.cppm:35`) and admin (`:112`).
2. Operation: `BrowseHandler` ctor registers `MainFlow` on a local workflow driver (browse/handler.cppm:12-16); `handle` runs `"tags_browse"`, reads `"tags_browse_execute"`, writes the body and status (browse/handler.cppm:17-24). Flow = single `ExecuteStep` (browse/workflow/flow/main.cppm:7-10). Accessor/member registered in `include/blog/handlers/all.cppm:23,393,674`.
3. Input: `using Input=IRequest&`, `Output=std::string` (browse/step/execute.cppm:10); `invoke` parses the raw query after `?` from `in.get_path()` into `map<string,string>` (execute.cppm:15-24), then rejects any `include` value other than `count.posts` with `ApiError{}.validation(...)` (execute.cppm:25-26; ApiError.validation at context/context_context.cppm:92-100). No `data[]`, no path params, no header reads. `debug`, `order` are accepted into `params` but unused.
4. Auth/permissions: only the two middlewares above (routes.cppm:20-21); no rate limiting, CORS, redirects, pretty URLs or permission services. The old dead token guard is gone from the handler (browse/handler.cppm:17-24).
5. Status/body/headers: step returns a raw JSON string (execute.cppm:38-45); handler writes it with `Status::OK` unconditionally (browse/handler.cppm:23) and sets literal `X-Cache-Invalidate: false` (browse/handler.cppm:22). Fallback body `"{}"` when no typed output (handler.cppm:20).
6. Store calls: `ctx.store().json_for("tags", params)` (execute.cppm:28) → `query_json(m_db.all<model::Tag>(), params)` (context/context_context.cppm:1152-1159,1504-1513) → `query_json_impl` (context.cppm:1560-1590: limit/offset/page slicing) → `search_json` (context.cppm:1592-1619: `filter`, `filter[x]`, `fields`, `include` + embeddings). `total` = `ctx.store().total("tags")` = `m_db.count<model::Tag>()` (context.cppm:1140-1144). No Knex transaction, events, jobs, webhooks or outbox side effects.
7. Cache: none in the route/handler/step/store path — no `core::cache::Cache`, keys, tags or TTLs. Only the `X-Cache-Invalidate: false` header string (browse/handler.cppm:22).
8. Output: inline `{"tags":<rows>,"meta":{"pagination":{"limit":L,"page":P,"pages":N,"prev":..,"next":..,"total":T}}}` (execute.cppm:38-45); `limit` defaults 15 and `page` defaults 1 via `to_size` (execute.cppm:29-33), `pages=ceil(total/limit)` and `prev`/`next` from `page` (execute.cppm:34-37). Empty store → `[]`, total 0.

**Diff**

- Residual gaps (no C++ primitive): framework input serializer (`forceUrlColumns`, restricted order fields, default `name asc`) (input/tags.js:7-29); `permissions:true` / permission service stage (tags-public.js:33; pipeline.ts:209-247); restricted-field `mongoTransformer` (tags-public.js:37; api-filter-utils.ts:67-73,202-204); `mappers.tags` URL + `clean.tag` field stripping (mappers/tags.js:4-18; clean.js:4-25); config-gated response cache + `site.changed` reset and JSON cache key (services/tags-public/index.ts:10-19; pipeline.ts:410-453); rate-limit/authz/CORS/redirect/pretty-URL middleware (content/routes.js:14; content/middleware.js:18-25); `order` and `debug` options (tags-public.js:25) have no C++ handling.
- Divergences: `optional_auth` + `content_type_middleware` (routes.cppm:20-21) require a JSON `Content-Type` on a GET and perform no content-API key/authz, unlike `authenticatePublic` (content/middleware.js:18-25); pagination is fabricated from an in-memory store count with hardcoded 15/1 defaults (execute.cppm:29-37) instead of model `findPage` meta; literal `X-Cache-Invalidate: false` header (handler.cppm:22) vs Ghost emitting no such header when `cacheInvalidate:false` (headers.ts:135-144); no `count.posts` relation materialization (the store's `include` is a search-embedding hook, context.cppm:1616-1620, not `shouldHavePosts`, models/tag-public.js:5-8).

### GET /tags/:id — MATCH

**Ghost**

1. Identity: `router.get('/tags/:id', mw.authenticatePublic, http(api.tagsPublic.read))` (content/routes.js:33); router CORS (content/routes.js:14); `authenticatePublic` chain (content/middleware.js:18-25).
2. Operation: `read` under `docName:'tags'` (tags-public.js:18,43).
3. Input: `options[]=['include','filter','fields','debug']` (tags-public.js:47); `data=ALLOWED_READ_FIELDS=['id','slug','visibility']` (tags-public.js:9,48); validation `include∈['count.posts']` (tags-public.js:49-55). `query` does `pick(frame.data, ALLOWED_READ_FIELDS)` and throws `BadRequestError('A tag id or slug is required.')` when neither `data.id` nor `data.slug` is truthy (tags-public.js:60-63; messages at :12-14). No multipart; path param `id`; no manual header reads.
4. Auth/permissions: `mw.authenticatePublic` (content/routes.js:33) + `permissions:true` (tags-public.js:56).
5. Status + headers: 200 on success, 400 on missing identifier (tags-public.js:62), 404 on no model (tags-public.js:70-74); `headers.cacheInvalidate:false` (tags-public.js:44-46) → no invalidation header.
6. Full call chain: `options = {...frame.options, mongoTransformer:rejectTagsContentApiRestrictedFieldsTransformer}` then `models.TagPublic.findOne(data, options)` (tags-public.js:65-69); `!model` → `NotFoundError('Tag not found.')` (tags-public.js:70-74). Transformer restricted keys as above (api-filter-utils.ts:67-73,202-204). Output via `default.js::all` (output/default.js:32-36) → `mappers.tags`/`clean.tag`. No side effects.
7. Cache: no response cache on read; `cacheInvalidate:false` only (tags-public.js:44-46).
8. Output: `{tags:[model]}` with no `meta` (output/default.js:32-40); model JSON already cleaned of `created_at/updated_at/parent` by `clean.tag` (clean.js:4-25). Missing result → 404 error envelope, not an empty array.

**C++**

1. Routes: `Route{"/:id"}.get(...)` nested under `/tags` → `handler.tags_read_by_id().handle(...)` (admin/tags/routes.cppm:38-46); inherits the outer router's `content_type_middleware`+`optional_auth` (routes.cppm:20-21). Catalog: `route_catalog.cppm:36,113`.
2. Operation: `ReadByIdHandler` registers `read_by_id::MainFlow` (read_by_id/handler.cppm:12-16; flow main.cppm:7-10); `handle` runs `"tags_read_by_id"` and maps a `status`/`body` map (handler.cppm:17-30). Accessor/member in `all.cppm:24,394,675`.
3. Input: `Input=IRequest&`, `Output=map<string,string>` (read_by_id/step/execute.cppm:10). Query parsed the same way (execute.cppm:15-24); `include` invalid → 422 validation (execute.cppm:25-27). Identifier = last `/`-segment of the path before `?` (execute.cppm:29-30); empty → 422 validation with `"A tag id or slug is required."` (execute.cppm:31-33). No `data[]`, no `visibility`, no header reads.
4. Auth/permissions: same two middlewares (routes.cppm:20-21); no permission services.
5. Status/body/headers: handler maps step `status` `"404"`→`NOT_FOUND`, `"422"`→`UNPROCESSABLE_CONTENT`, default `OK` (read_by_id/handler.cppm:23-30); body from step, fallback `"{}"` (handler.cppm:20-24). Literal `X-Cache-Invalidate: false` (handler.cppm:26). Missing identifier is 422, not Ghost's 400.
6. Store calls: `ctx.store().find_by_resource("tags", id)` (execute.cppm:35) → `"tags"` branch `entity_json<model::Tag>(find_entity<model::Tag>(id))` (context/context_context.cppm:1084-1095,1123-1128); `!one` → 404 with `ApiError{}.json("Tag not found.","Not Found")` (execute.cppm:36-38; ApiError.json at context.cppm:82-89). No slug fallback here, no transformer, no events/outbox.
7. Cache: none (no `core::cache::Cache` in handler/step/store path); only the header string (handler.cppm:26).
8. Output: success body is a **single raw tag JSON object** (execute.cppm:39), not Ghost's `{tags:[...]}` envelope; errors are `{"errors":[...]}` envelopes (context.cppm:103-119).

**Diff**

- Residual gaps: `permissions:true` (tags-public.js:56); `mongoTransformer` restricted-field rejection (tags-public.js:67; api-filter-utils.ts:67-73); `data.visibility` public constraint and `pick(frame.data,...)` semantics (tags-public.js:9,48,60); slug-as-identifier in this handler (tags-public.js:60-69 — server has no slug lookup primitive for `find_by_resource`); URL mapper + `clean.tag` (mappers/tags.js:4-18; clean.js:4-25); content API middleware chain (content/middleware.js:18-25).
- Divergences: missing identifier → 422 (read_by_id/step/execute.cppm:31-33) vs Ghost 400 `BadRequestError` (tags-public.js:61-63); raw single object body (execute.cppm:39) vs `{tags:[...]}` (output/default.js:32-36); literal `X-Cache-Invalidate: false` (handler.cppm:26) vs no header; `optional_auth`+`Content-Type` requirement (routes.cppm:20-21) vs `authenticatePublic`.

### GET /tags/slug/:slug — MATCH

**Ghost**

1. Identity: `router.get('/tags/slug/:slug', mw.authenticatePublic, http(api.tagsPublic.read))` (content/routes.js:34); router CORS (content/routes.js:14); same `authenticatePublic` chain (content/middleware.js:18-25).
2. Operation: `read` under `docName:'tags'` (tags-public.js:18,43), keyed by `frame.data.slug` (tags-public.js:60-69).
3. Input: same as `/:id` — `options[]=include,filter,fields,debug` (tags-public.js:47), `data[]=id,slug,visibility` (tags-public.js:9,48), `include∈{count.posts}` (tags-public.js:49-55), identifier guard (tags-public.js:60-63). Path param `slug`; no multipart/header reads.
4. Auth/permissions: `mw.authenticatePublic` (content/routes.js:34) + `permissions:true` (tags-public.js:56).
5. Status + headers: 200 / 400 / 404 as `/:id` (tags-public.js:61-76); `cacheInvalidate:false` (tags-public.js:44-46).
6. Full call chain: identical `pick` → identifier guard → `TagPublic.findOne(data, {...options,mongoTransformer})` → `NotFoundError` path (tags-public.js:60-76); transformer restricted keys as above (api-filter-utils.ts:67-73,202-204); output serializer `default.js::all` + `mappers.tags`/`clean.tag`.
7. Cache: none on read; `cacheInvalidate:false` only (tags-public.js:44-46).
8. Output: `{tags:[model]}` (output/default.js:32-36); 404 on missing model.

**C++**

1. Routes: slug sub-router `Router{router,"/slug"}` with `.use(optional_auth)` and `Route{"/:slug"}.get(...)` → `handler.tags_read_by_slug().handle(...)` (admin/tags/routes.cppm:65-79); placeholder now `/:slug`, matching Ghost. Catalog: `route_catalog.cppm:37,114`. The sub-router is built with the outer `router` context and base `"/slug"` (routes.cppm:65-66), so its effective mount depends on `utils::openapi::ApiRouter` re-basing — recorded as declared.
2. Operation: `ReadBySlugHandler` registers `read_by_slug::MainFlow` (read_by_slug/handler.cppm:12-16; flow main.cppm:7-10); `handle` runs `"tags_read_by_slug"` and maps a `status`/`body` map (handler.cppm:17-30). Accessor/member in `all.cppm:25,395,676`.
3. Input: `Input=IRequest&`, `Output=map<string,string>` (read_by_slug/step/execute.cppm:11). Query parsed (execute.cppm:16-25); `include` invalid → 422 (execute.cppm:26-28); slug = last path segment (execute.cppm:30-31); empty → 422 `"A tag id or slug is required."` (execute.cppm:32-34). No `data[]`/`visibility`/header reads.
4. Auth/permissions: `optional_auth` on the `/slug` sub-router (routes.cppm:67); no content API key/authz/permission services.
5. Status/body/headers: step → handler status mapping 404/422/200 (read_by_slug/step/execute.cppm:40-43; handler.cppm:23-30); literal `X-Cache-Invalidate: false` (handler.cppm:26).
6. Store calls: `ctx.store().json_for("tags", {{"filter","slug:"+slug}})` (execute.cppm:36) → `query_json(m_db.all<model::Tag>(), params)` → `query_json_impl` → `search_json` where `filter` is applied via `query.where(...)` (context/context_context.cppm:1152-1159,1560-1590,1611-1612). The JSON array is decoded with `core_serde::JsonCodec{}.decode<serde::Value>` and element 0 re-encoded (execute.cppm:37-43; codec pattern in middleware/serde.cppm:59). Empty/no array → 404 `"Tag not found."` (execute.cppm:39-41). No transformer, no `count.posts` relation, no side effects.
7. Cache: none; only the header string (handler.cppm:26).
8. Output: success body = single raw tag JSON object (execute.cppm:42-43), not `{tags:[...]}`; errors `{"errors":[...]}` (context.cppm:103-119).

**Diff**

- Residual gaps: `permissions:true` (tags-public.js:56); `mongoTransformer` restricted-field rejection (tags-public.js:67; api-filter-utils.ts:67-73); `data.visibility` and `pick` identifier semantics (tags-public.js:9,48,60-63); output URL mapper + field cleaning (mappers/tags.js:4-18; clean.js:4-25); content API middleware chain (content/middleware.js:18-25).
- Divergences: 422 for empty slug vs Ghost 400 (execute.cppm:32-34 vs tags-public.js:61-63); raw single object vs `{tags:[...]}` (execute.cppm:43 vs output/default.js:32-36); literal `X-Cache-Invalidate: false` (handler.cppm:26); slug filter is a memory-search `where("slug:"+slug)` (context.cppm:1611) rather than `TagPublic.findOne({slug})` (tags-public.js:69) — different matching semantics for compound/odd slugs; sub-router mount caveat (routes.cppm:65-66).

## Module-level notes

- All three C++ GET handlers use the same shape: static `BlogContext*` on the step, local workflow driver, a single `ExecuteStep`, `status`/`body` map or raw string output, and literal `X-Cache-Invalidate: false`. Ghost instead uses the api-framework pipeline: input validation/serialization → permissions → cache lookup → query → output serialization (pipeline.ts:400-455), with `cacheInvalidate:false` suppressing the header (headers.ts:135-144).
- Ghost tags content routes share one controller with `docName:'tags'` (tags-public.js:18) and `ALLOWED_INCLUDES=['count.posts']` / `ALLOWED_READ_FIELDS=['id','slug','visibility']` (tags-public.js:8-9). Neither allowlist value set is fully reproduced: C++ checks only `include` (browse/step/execute.cppm:25-26; read_by_id/step/execute.cppm:25-27; read_by_slug/step/execute.cppm:26-28) and ignores `visibility`.
- `count.posts` on Ghost is backed by `TagPublic.shouldHavePosts` joining `posts_tags` (models/tag-public.js:4-9); C++ `search_json` `include` handling is an embedding hook (context/context_context.cppm:1616-1620), so it does not materialize a post count.
- Ghost browse and read both inject `rejectTagsContentApiRestrictedFieldsTransformer` (tags-public.js:37,67), restricting `created_at, updated_at, parent_id, parent` and relations `tag, tags, primary_tag` (api-filter-utils.ts:67-73,202-204). C++ store/serialization has no equivalent transformer; raw `model::Tag` JSON is returned.
- C++ also installs `POST /tags`, `PUT /tags/:id`, `DELETE /tags/:id` on the same flat router (admin/tags/routes.cppm:30-36,48-63). These map to the separate Ghost **admin** tags controller (not `tags-public.js`) and are owned by that audit, so they are not counted as EXTRA here. `GET /search-index/tags` belongs to `searchIndexPublic.fetchTags` (content/routes.js:54), a different controller.
- No C++ route is MISSING for this controller; no route is EXTRA relative to it. Residual gaps are framework/service behaviors with no allowed C++ primitive; divergences are the status-code, envelope, header, auth-mechanism and fabricated-pagination differences documented above.


## Strict-parity rewrite (manifest group 9, 2026-11-19)

All three content-API GET routes were rewritten in `include/blog/handlers/admin/tags/` using the same
shared abstractions as the admin audit. Summary + route table flipped **PARTIAL ×3 → MATCH ×3**;
the route details and Diff bullets above are the original pre-rewrite analysis.

| Route | Shared abstraction closing the reported Diff |
|---|---|
| `GET /tags` (browse) | `core::query::parse` (seven Ghost options, `include` allowlist, restricted-field guard via `run`), default `order:"name asc"` for `/content/` paths, `core::query::run` real `meta.pagination` + `fields` projection, `filter[x]`/bare NQL; `blog::mapping::clean(...,"tag")` for the TagPublic field allow-list; `blog::mapping::wrap("tags",...)` envelope (via `data`→`tags` rename); ghost permission `tags:browse` with `CONTENT_API` context; **no** `X-Cache-Invalidate` header (`cacheInvalidate:false`); content reads keep 200. |
| `GET /tags/:id` | `core::query::parse` + `has_restricted_field` guard; path id instead of hardcoded `"1"`; 400 `BadRequest` on empty identifier for `/content/` paths (422 retained for the shared admin path), 404 `Tag not found.` on miss; `clean` + `wrap("tags",...)` single-model envelope; ghost permission `tags:read`; no invalidation header. |
| `GET /tags/slug/:slug` | `core::query::parse("?filter[slug]="+slug, {"filter"})` + `apply` replaces the memory-search `where("slug:"+slug)`; restricted-field guard; 400 on empty slug (content) / 422 (admin path); 404 on miss; `clean` + `wrap("tags",...)`; ghost permission `tags:read`; no invalidation header. |

**Auth.** `require_ghost_permission(req,res,next,send,"tags","browse"/"read")` now runs on every GET.
`GhostAuthorizer` permits anonymous `CONTENT_API` reads of public content, matching
`mw.authenticatePublic + permissions:true`; HEAD/GET map to the `browse` action through
`core::auth::ghost::method_action`. When `x-blog-context` is absent, the shared handlers infer
`CONTENT_API` for paths containing `/content/` and `ADMIN_API` otherwise.

**Header correction.** The previous literal `X-Cache-Invalidate: false` was removed from all three
GET handlers: Ghost emits **no** invalidation header when `cacheInvalidate:false`.

**Files changed.** `routes.cppm` (router-level `content_type_middleware`/`optional_auth` removed),
`browse/{handler.cppm,workflow/step/execute.cppm}`,
`read_by_id/{handler.cppm,workflow/step/execute.cppm}`,
`read_by_slug/{handler.cppm,workflow/step/execute.cppm}`. The three flow files are unchanged.

**Central registrations.** None required and none edited. `core_query`, `core_auth` and
`blog_handlers:mapping` are existing deps/partitions of `//include/blog/handlers:blog_handlers`;
`blog_handlers_middleware:ghost_permission` is already re-exported by `middleware/base.cppm:8`;
existing partition names (`blog_handlers:tags_<op>_handler/_step/_flow`) and `import :domain_all;`
are unchanged. No new files, no shared-file edits.

**Remaining gaps.** Config-gated `tagsPublicService.service.cache` + `site.changed` reset has no
allowed C++ primitive (wire-invisible here); `count.posts` relation materialization
(`TagPublic.shouldHavePosts`); `data.visibility` public constraint; URL mapper enrichment; the
shared `clean` allow-list keeps `created_at`/`updated_at` (the store's `model::Tag` emits neither,
so no extra wire fields). None of these alter the audited status codes, envelopes or headers.

**Verification.** No compile error from any file under `admin/tags/` in
`bazel build //include/blog/handlers:blog_handlers --keep_going`; pre-existing errors remain in
`admin/pages/*` and the missing `blog_handlers:notifications_service` module.
