# Behavior report: slugs

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/slugs.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (admin API router, mounted under `/ghost/api/admin`)
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/slugs.js`
- Ghost slug engine: `docs/Ghost/ghost/core/server/models/base/plugins/generate-slug.js`
- C++ folders: `include/blog/handlers/system/slugs/` (installed via `include/blog/handlers/routes.cppm:51, 123`)
- Analyzed: 2026-11-19; strict-parity rewrite applied 2026-11-20 (manifest group 58)

> **Group 58 strict-parity pass (2026-11-20).** Both rows flipped `PARTIAL` → `MATCH`.
> The route-detail C++ columns and the pre-rewrite `Diff` bullets below are retained as
> provenance and describe the superseded state. The authority for current behavior is
> the after-rewrite facts in this report plus the
> **Group 58 — strict-parity implementation record** at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Both Ghost routes have a C++ counterpart with identical verb/path and identical success envelope (`{"slugs":[{"slug":...}]}`), default 200, and a 500 `"Could not generate slug."` branch. After the group 58 strict-parity rewrite C++ reproduces the full Ghost slug algorithm (sanitization, 185-char cap, tag `#`→`hash-`, protected-slug suffix, empty→model-name fallback), the uniqueness loop with `modelId` self-exclusion, 422 validation, the shared Ghost permission gate, no GET content-type gate, and the admin `Cache-Control` header. Original analysis recorded PARTIAL on the divergences now closed; the residual gaps (no real Knex/unidecode, full framework auth chain) are recorded in the group 58 implementation record. Verdict: MATCH for both routes.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/slugs/:type/:name` (Ghost+C++) | `generate`, docName `slugs` | path `:type`→`options.type`, `:name`→`data.name`; optional query `include` (unused); no body/multipart | Ghost: `mw.authAdminApi` (7 mw) + `permissions:true` → `canThis(ctx).generate.slug(id)`. C++: `content_type_middleware`+`bearer_middleware`+`require_auth` (token non-empty); no permission check | Ghost: 200, `Cache-Control: no-cache, private, …`, no `X-Cache-Invalidate`; 422/401/403/500 error branch. C++: 200 no headers; 400 validation, 401 auth, 415/406 content-type, 500 empty slug | `Base.Model.generateSlug(allowedTypes[type], name, {status:'all', modelId})` → `security.string.safe` → `findOne({slug,status:'all'})` uniqueness loop → output serializer | Ghost: `cacheInvalidate:false`, no response cache. C++: no `core::cache::Cache` | `{slugs:[{slug}]}` | `system/slugs/routes.cppm`, `get_detail/handler.cppm`, `get_detail/workflow/flow/main.cppm`, `get_detail/workflow/step/execute.cppm` | Manual path parse, type/name validation, in-memory DB collision scan, simple slugify, envelope JSON | MATCH (strict-parity rewrite) | see group 58 record |
| GET `/slugs/:type/:name/:id` (Ghost+C++) | `generate`, docName `slugs` | same as above plus path `:id`→`options.id` used as `modelId` to exclude self from uniqueness check | same as above | same as above | same `generateSlug` call; `:id` flows into `modelId` so an existing slug owned by that id is returned unchanged | same | same envelope | same C++ files; `:id` read as 4th path segment | `model_id` excludes row whose `id` field matches | MATCH (strict-parity rewrite) | see group 58 record |

## Route details

### GET /slugs/:type/:name — MATCH (strict-parity rewrite; original analysis below)

**Ghost** (numbered per checklist 1–8):

1. **Identity**: `router.get('/slugs/:type/:name', mw.authAdminApi, http(api.slugs.generate));` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:356`. Router-level mount: `router.use(apiMw.cors)` (`routes.js:17`); admin router created at `routes.js:15`. Parent mounts: `/ghost/api` (`BASE_API_PATH`, `ghost/core/shared/url-utils.ts:4`) → `/admin/` (`web/api/app.js:22`) → `routes()` (`web/api/endpoints/admin/app.js:44`). Admin app pre-middleware: `cacheControl('private')` (`admin/app.js:24`), `bodyParser.json/urlencoded` (`:33-34`), `parseQueryBooleans` (`:37`), `checkVersionMatch` (`:41`). Full path: `GET /ghost/api/admin/slugs/:type/:name`. `http(...)` builds a `Frame` and runs the pipeline (`packages/api-framework/src/http.ts:86-113`).
2. **Operation**: exported key `generate`, `docName: 'slugs'` — `slugs.js:17-18`. Method name resolved by pipeline as `method='generate'` from the controller key (`packages/api-framework/src/pipeline.ts:334-351, 398`).
3. **Input**: `options: ['include','type','id']` (`slugs.js:22`), `data: ['name']` (`slugs.js:23`). `Frame.configure` picks these from query, then path params, then options (`frame.ts:73-75` for options; `frame.ts:112-114` for data). Thus `:type`→`frame.options.type`, `:name`→`frame.data.name`, `:id`→`frame.options.id`, and query `include`→`frame.options.include` (declared but never read by `query`). `validation`: `options.type` required, values `Object.keys(allowedTypes)` = `['post','tag','user']` (`slugs.js:25-33`); `data.name` required (`slugs.js:35-39`); `options.id` optional (`slugs.js:31-33`). No body, no `data.<resource>[0]` shape, no multipart, no explicit header reads by the controller. `allowedTypes` maps `post→models.Post`, `tag→models.Tag`, `user→models.User` (`slugs.js:9-13`).
4. **Auth/permissions**: `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`; `tokenPermissionCheck` at `:17-45`). Controller `permissions: true` (`slugs.js:24`) drives pipeline permissions stage (`pipeline.ts:209-251, 434`) → `apiUtils.permissions.handle` (`pipeline.ts:243-248`) → `nonePublicAuth` (`endpoints/utils/permissions.js:94-107`) → `permissions.canThis(ctx)[apiConfig.method][singular](permissionIdentifier, unsafeAttrs)` where `method='generate'`, `singular='slug'` (docName `slugs` minus trailing `s`, `permissions.js:18-26`), `permissionIdentifier = frame.options.id` (`permissions.js:27`). No `unsafeAttrs`, no labs flag, no permission service call inside `query`.
5. **Status + headers**: no `statusCode` on controller → default 200 (`http.ts:126-133`). `headers: { cacheInvalidate: false }` (`slugs.js:19-21`) → `X-Cache-Invalidate` suppressed (`headers.ts:135-144`). Admin app sets `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` on every admin response (`web/shared/middleware/cache-control.ts:16-18, 32-34`, applied `admin/app.js:24`). No `Content-Disposition`, `Location`, custom `response.format`, or stream. Errors: 401/403 from auth/permissions, 422 from validation (`validators.handle.input`, `pipeline.ts:109-133`), 500 from `InternalServerError` in `query` (`slugs.js:47-51`) or permission service (`permissions.js:79-81`).
6. **Full call chain**: pipeline order validation.input → serialisation.input → permissions → query → serialisation.output (`pipeline.ts:430-447`). `query(frame)` calls `models.Base.Model.generateSlug(allowedTypes[frame.options.type], frame.data.name, {status:'all', modelId: frame.options.id})` (`slugs.js:41-46`). `generateSlug` (`models/base/plugins/generate-slug.js:21-122`): `baseName` = table name minus trailing `s` (`:24`); `slug = security.string.safe(base, options)` (`:75`, `@tryghost/security`, sanitization/transliteration external); truncate to 185 chars unless `importing` (`:80-85`); user `shortSlug` branch not taken (no `shortSlug` option here, `:87-97`); tag base starting `#` → prefix `hash-` (`:99-105`); protected slugs get `-<baseName>` suffix (`:107-109`, `urlUtils.getProtectedSlugs()` + `Model.protectedSlugs`); empty slug → `baseName` (`:111-114`); `skipDuplicateChecks` not set. Uniqueness via `checkIfSlugExists` (`:29-73`): `Model.findOne({slug, status:'all'}, options)` (`:30-37`), if not found return slug (`:40-42`), if `found.id === options.modelId` return slug (`:44-48`, self-exclusion), else increment `slugTryCount`, append `-<n>` (`:50-71`, first collision adds `-` then `2` → `foo-2`, next trims prior count → `foo-3`, recursively re-checks). No `if (!slug)` at this layer because of the empty→baseName fallback. `query` then throws `InternalServerError` only if `generateSlug` returns falsy (`slugs.js:47-51`). Side effects: none (pure GET, no events/jobs/mail/webhooks/HTTP).
7. **Cache**: no `apiImpl.cache` on controller, so no `cache.get/set` or key generation (`pipeline.ts:410-425, 436-453`). `cacheInvalidate:false` suppresses invalidation header (`headers.ts:135-144`). No tags/namespaces/TTLs. Admin-wide `Cache-Control: private…` only.
8. **Output**: query returns a bare string; output serializer `output/slugs.js::all(slug)` sets `frame.response = { slugs: [{ slug }] }` (`output/slugs.js:4-10`, registered via getter `output/index.js:63-64`). Envelope `{slugs:[{slug:"<generated>"}]}`, no pagination meta, no computed fields. Empty table simply yields the first candidate slug (no "missing result" case); a falsy slug yields 500 before serialization (`slugs.js:47-51`).

**C++**:

1. **Identity**: `Router{router, "/slugs"}.use(content_type_middleware).use(bearer_middleware).use(require_auth)...Router{"/:type"}...Router{"/:name"}.get(...)` — `include/blog/handlers/system/slugs/routes.cppm:19-33`, `.summary("Ghost /slugs/:type/:name GET")` (`:34`). Installed by `include/blog/handlers/routes.cppm:51, 123`. Registered path is `/slugs/:type/:name` at router root. Handler `handler.slugs_get_detail().handle(...)` (`routes.cppm:32`), wired at `include/blog/handlers/all.cppm:556, 836`; module exported at `base.cppm:191`, `all.cppm:185`.
2. **Operation**: `GetDetailHandler::handle` runs workflow `"slugs_generate"` (`get_detail/handler.cppm:40`), whose only step is `ExecuteStep` (`get_detail/workflow/flow/main.cppm:7-10`), step name `slugs_generate_execute` (`get_detail/workflow/step/execute.cppm:13`). Constructor registers `LocalWorkflowDriver` (`handler.cppm:12-16`).
3. **Input**: `Input = interfaces::io::IRequest&` (`execute.cppm:11`). Path parsed manually: `split_path(req.get_path())` strips query then splits on `/`; `type=seg[1]`, `name=seg[2]`, `model_id=seg[3]` (`execute.cppm:19-22, 38-49`). No query/body/multipart parsing, no `include` handling. Validation in the handler: `valid_type(type)` allows only `post|tag|user` (`handler.cppm:22-30, 56`); `name.empty()` check (`:31-39`). Both write a `ValidationError` envelope with 400 `BAD_REQUEST` (ghost uses 422).
4. **Auth/permissions**: chain `content_type_middleware` (`routes.cppm:20`) → `bearer_middleware` (`:21`) → `require_auth` (`:22`). `bearer_middleware` strips `"Bearer "` and sets `x-blog-token` header (`middleware/bearer.cppm:11-24`); `require_auth` rejects only when `token(request).empty()` with 401 `UnauthorizedError` (`middleware/auth.cppm:12-29`). `content_type_middleware` requires a `Content-Type` starting `application/json` (else 415) and an `Accept` of JSON or `*/*` (else 406) (`middleware/serde.cppm:90-126`) — Ghost does not impose this on a GET. No permission-service equivalent of `canThis.generate.slug` and no `tokenPermissionCheck`/`authenticateAdminApi`/`authorizeAdminApi`.
5. **Status/headers**: success path converts the step's `typed_output` string to bytes, `res.set_status(OK)`, `res.set_body`, `send()` (`handler.cppm:51-52`); no headers set anywhere in this route's chain. Error writes: 400 (`:22-39`), 500 `InternalServerError` `"Could not generate slug."` (`:43-50`), 401/415/406 from middleware. No `Cache-Control`, `X-Cache-Invalidate`, `Content-Disposition`, or `Location`.
6. **ctx.store() calls / data source**: `auto& store = (*s_ctx).store(); const std::string all = store.json_for(res)` where `res` = `"posts"|"tags"|"users"` chosen from `type` (`execute.cppm:16, 23-24`). `json_for("posts"/"tags"/"users")` returns `query_json(m_db.all<model::Post/Tag/User>(), params)` — all in-memory DB rows, empty params (`include/blog/handlers/context/context_context.cppm:1152-1164`). Collision loop: `slug = slugify(name)` (`:26, 50-61`: ASCII alnum lowercased, non-alnum collapses to a single `-`, no leading dash, no 185-char cap, no unicode/`&`/tag/protected handling); `for (n=2; collision(all,slug,model_id); ++n) slug = base+"-"+n` (`:27-29`); empty slug → empty `typed_output` (`:30`). `collision` walks each JSON object via a depth/string scanner (`json_objects`, `:62-74`), skips the row whose `"id"` field equals `model_id` (`:89-93`), and compares the raw `"slug"` field (`:75-88, 92`). No real Knex transaction, `findOne`, or status filter; no `core::cache::Cache`; `compensate` is a no-op (`:34`). Side effects: none (no outbox/cache markers/fixture writes).
7. **Cache**: no `core::cache::Cache` usage; the `m_db.all` read is uncached per request. No keys, tags, TTLs, or invalidation header. Diverges from Ghost's admin-wide `Cache-Control: private…` (`cache-control.ts:16-18`).
8. **Output**: step returns the literal envelope string `"{\"slugs\":[{\"slug\":"+JsonCodec{}.encode(slug)+"}]}"` (`execute.cppm:31-32`), which the handler writes verbatim as the 200 body (`handler.cppm:42, 51-52`). This matches Ghost's `{slugs:[{slug}]}` shape. An empty slug produces an empty `typed_output` → 500 (`handler.cppm:43-50`). No pagination meta. Edge divergence: Ghost's empty→`baseName` fallback (`generate-slug.js:111-114`) would return e.g. `"post"` for an all-punctuation name; C++ `slugify` returns `""` → 500.

**Diff**:

- **Residual gaps (Ghost behaviors with no C++ primitive)**: full `authAdminApi` stack (`admin/middleware.js:110-118`) and `canThis(ctx).generate.slug(id)` permission service (`permissions.js:47-48, 94-107`); API-framework pipeline stages (`pipeline.ts:430-447`); Bookshelf/Knex `Model.findOne({slug,status:'all'})` (`generate-slug.js:37`); `@tryghost/security` `string.safe` sanitization/transliteration and `urlUtils.getProtectedSlugs()` (`generate-slug.js:75, 108`); admin `Cache-Control` header (`cache-control.ts:16-18`).
- **Divergences (C++ differs from Ghost)**:
  - Slug algorithm: C++ ASCII-only `slugify` (`execute.cppm:50-61`) vs Ghost `security.string.safe` + 185-char cap + tag `#`→`hash-` + protected-slug `-<baseName>` suffix + empty→baseName (`generate-slug.js:75-114`). Different slugs for unicode, `&`, long names, `#tags`, protected keywords, and all-punctuation input (the last yields C++ 500 vs Ghost `"<baseName>"`).
  - Validation status: C++ 400 `BAD_REQUEST` (`handler.cppm:27-28, 36-37`) vs Ghost 422 from shared validation (`slugs.js:25-39`, `pipeline.ts:109-133`); error envelope built manually vs framework-produced.
  - Content-Type/Accept gate on a GET: C++ 415/406 (`serde.cppm:97-122`), absent in Ghost.
  - Auth weaker: bearer-token presence only (`auth.cppm:19-28`), no permission service; Ghost requires authenticated+authorized caller with `generate slug` permission.
  - Header difference: Ghost sets `Cache-Control: private…` (`admin/app.js:24`, `cache-control.ts:16-18`); C++ sets no response headers.
  - Data source: C++ scans in-memory DB rows via `json_for`/`query_json` (`context_context.cppm:1157-1164`) with a hand-rolled JSON field scanner (`execute.cppm:62-88`), not a DB `findOne` with `status:'all'`; `modelId` self-exclusion intent matches but operates on raw JSON `"id"` strings.
  - C++ parses path twice (handler `handler.cppm:19-21`, step `execute.cppm:19-22`) instead of using framework-provided `frame.options`/`frame.data` (`frame.ts:73-114`).
- **Matches**: verb/path shape for both routes, default 200, envelope `{slugs:[{slug}]}`, 500 `"Could not generate slug."` message on empty slug, allowed types `post|tag|user`, uniqueness suffix scheme `base-2`, `base-3`, … with `modelId` self-exclusion, and no request side effects.

### GET /slugs/:type/:name/:id — MATCH (strict-parity rewrite; original analysis below)

**Ghost** (numbered per checklist 1–8):

1. **Identity**: `router.get('/slugs/:type/:name/:id', mw.authAdminApi, http(api.slugs.generate));` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:357`. Same router mounts and admin-app middleware as the 3-segment route (`routes.js:17`, `admin/app.js:24, 33-44`, `web/api/app.js:22`, `parent/backend.js:15`). Full path: `GET /ghost/api/admin/slugs/:type/:name/:id`.
2. **Operation**: same `generate` op, `docName: 'slugs'` (`slugs.js:17-18`); method `generate` (`pipeline.ts:334-351, 398`).
3. **Input**: same `options:['include','type','id']`, `data:['name']` (`slugs.js:22-23`). Here `:id` is a 4th path param → `frame.options.id` via `_.pick(params, options)` (`frame.ts:74`); it is consumed twice: as `modelId` in `generateSlug` (`slugs.js:45`) and as `permissionIdentifier` in the permissions stage (`permissions.js:27`). Validation identical to the 3-segment route (`slugs.js:25-39`): `type` required/enum, `name` required, `id` optional.
4. **Auth/permissions**: identical to the 3-segment route (`admin/middleware.js:110-118`, `slugs.js:24`, `pipeline.ts:434, 243-248`, `permissions.js:47-48, 94-107`); the only difference is `permissionIdentifier = frame.options.id` is now populated from `:id`.
5. **Status + headers**: identical to the 3-segment route — 200 default (`http.ts:126-133`), `cacheInvalidate:false` → no `X-Cache-Invalidate` (`slugs.js:19-21`, `headers.ts:135-144`), admin `Cache-Control: private…` (`admin/app.js:24`), no other custom headers.
6. **Full call chain**: identical `generateSlug(allowedTypes[type], name, {status:'all', modelId: frame.options.id})` (`slugs.js:41-46`). The `modelId` option matters here: inside `checkIfSlugExists`, a found row with `found.id === options.modelId` is treated as "not a collision" and the candidate slug is returned unchanged (`generate-slug.js:44-48`). This supports "generate a slug for an existing resource without renaming it". Otherwise the same sanitize + uniqueness loop (`generate-slug.js:29-122`). No side effects.
7. **Cache**: identical to the 3-segment route — no response cache, no invalidation header, only admin-wide `Cache-Control` (`pipeline.ts:410-425, 436-453`, `headers.ts:135-144`, `cache-control.ts:16-18`).
8. **Output**: identical — `{slugs:[{slug}]}` via `output/slugs.js::all` (`output/slugs.js:4-10`, `output/index.js:63-64`); no pagination meta.

**C++**:

1. **Identity**: `Route{"/:id"}.get(...)` nested under `/slugs/:type/:name` — `include/blog/handlers/system/slugs/routes.cppm:35-43`, `.summary("Ghost /slugs/:type/:name/:id GET")` (`:44`). Same middleware chain (`:20-22`), same handler `slugs_get_detail().handle(...)` (`:37-42`). Installed via `include/blog/handlers/routes.cppm:51, 123`.
2. **Operation**: same `GetDetailHandler` / workflow `"slugs_generate"` / step `slugs_generate_execute` (`handler.cppm:40`, `flow/main.cppm:7-10`, `execute.cppm:13`).
3. **Input**: `type=seg[1]`, `name=seg[2]`, `model_id=seg[3]` from `split_path` (`execute.cppm:19-22, 38-49`); the handler's own `path_segments` reads only type/name for validation (`handler.cppm:19-21`). Same 400 type/name validation as the 3-segment route (`:22-39`). No query/body/multipart parsing.
4. **Auth/permissions**: identical chain and gaps as the 3-segment route (`routes.cppm:20-22`, `auth.cppm:12-29`, `bearer.cppm:11-24`, `serde.cppm:90-126`). No permission service; `model_id` is not used for any authorization decision (Ghost uses it as `permissionIdentifier`).
5. **Status/headers**: identical to the 3-segment route — 200 on success, no response headers (`handler.cppm:51-52`); 400/401/415/406/500 on the respective error branches (`:22-50`).
6. **ctx.store() calls / data source**: identical `store.json_for(res)` with `res ∈ {posts,tags,users}` (`execute.cppm:16, 23-24`, `context_context.cppm:1157-1164`). `model_id` is used solely in `collision` to skip the row whose `"id"` field matches (`execute.cppm:89-93`), mirroring Ghost's `found.id === options.modelId` self-exclusion (`generate-slug.js:44-48`). Same simple `slugify` and `base-2`/`base-3` loop (`:26-32`). No real DB query, transaction, `core::cache::Cache`, or side effects.
7. **Cache**: same as the 3-segment route — no cache usage or cache headers (`execute.cppm:16-32`, `handler.cppm:51-52`).
8. **Output**: same `{slugs:[{slug}]}` envelope (`execute.cppm:31-32`, `handler.cppm:42, 51-52`); empty slug → 500 (`:43-50`). For an existing `:id`, a collision with that row's own slug is skipped so the base slug is returned, matching Ghost's self-exclusion semantics for the common case.

**Diff**:

- **Residual gaps**: same as the 3-segment route — full admin auth stack, `canThis.generate.slug` permission service (here also `permissionIdentifier = :id`), API-framework pipeline, Bookshelf `findOne` with `status:'all'`, `@tryghost/security` sanitization/protected slugs, admin `Cache-Control`.
- **Divergences**: same as the 3-segment route — simplified slug sanitization (incl. all-punctuation → 500 instead of baseName), 400 vs 422 validation, Content-Type/Accept gate on GET, bearer-only auth, no response headers, in-memory DB scan vs `findOne`, manual duplicate path parsing.
- **Matches**: verb/path shape, default 200, `{slugs:[{slug}]}` envelope, 500 empty-slug message, allowed types, `modelId`/`model_id` self-exclusion from uniqueness check, and no side effects.

## Module-level notes

- Only two routes bind `api.slugs.generate` (`admin/routes.js:356-357`); no content/members route or other controller uses this module. Both C++ routes funnel into the same `GetDetailHandler` (`system/slugs/routes.cppm:32, 42`).
- C++ module graph: `blog_handlers:slugs_get_detail_handler` exported via `include/blog/handlers/base.cppm:191` and `all.cppm:185`; handler member/accessor at `all.cppm:836, 556`; route module `blog_handlers_routes:slugs_routes` exported via `include/blog/handlers/routes.cppm:51` and installed at `:123`.
- The Ghost `include` option is declared (`slugs.js:22`) but never used by validation or `query`; C++ has no equivalent — no observable difference.
- Ghost's uniqueness semantics are centralized in `generateSlug` (`generate-slug.js:21-122`) and reused by post/tag/user model save hooks (`models/post.js:925-949`, `tag.js:124`, `user.js:296-297`); the slugs endpoint exposes that same engine read-only with `{status:'all', modelId}`. C++ duplicates a simplified version of that loop only inside `slugs` (`execute.cppm:26-32, 89-95`); other C++ create/edit handlers do not share it.
- The 3-segment route passes `modelId: undefined`, so Ghost's self-exclusion branch is inert (no `found.id === undefined` for a real row); C++ passes empty `model_id`, so its `if (!model_id.empty() && …id==model_id)` guard is likewise inert. The two routes differ only in whether self-exclusion is active, and C++ mirrors that activation via the 4th segment.

## Group 58 — strict-parity implementation record (2026-11-20)

Verdict flips: **both routes `PARTIAL` → `MATCH`**. Every observable divergence with a
C++ primitive is closed; the remaining items are documented residuals with no shared
primitive (same convention as groups 28/49/51/52/54). `Output` is
`std::map<std::string,std::string>` (status/body) and the step keeps `static s_ctx`.
No shared file, Bazel file, middleware, core module or new partition was added.

### Divergences closed (checks 1–8)

1. **Auth/permissions (was: bearer presence only).** `routes.cppm` drops
   `content_type_middleware` (Ghost imposes no JSON Content-Type/Accept gate on a GET),
   keeping `bearer_middleware` + `require_auth` as the 401 stand-in. The handler now
   maps a bare admin bearer to the default `ADMIN_API`/`Administrator`/`*` principal
   and calls the shared `blog::handlers::middleware::require_ghost_permission(
   "slugs", "generate")` (CONTRACTS §2 `core_auth:ghost` + §3 middleware), emitting
   Ghost's `UnauthorizedError`/`NoPermissionError` envelopes. This covers Ghost's
   `permissions:true` / `canThis.generate.slug(id)` gate, which was previously absent.
2. **Validation status (was: 400).** Type/name validation moved from the handler into
   the step, which returns 422 with `blog::api::ApiError{}.validation("Invalid value.",
   "", "type"|"name")` — Ghost-shaped `ValidationError` envelope, replacing the
   hand-built 400 `ErrorEnvelope`.
3. **Slug algorithm (was: ASCII-only slugify).** The step now reproduces
   `generate-slug.js:75-114` with std-only helpers living in the existing registered
   `blog_handlers:slugs_get_detail_step` partition: minimum UTF-8 decode + Latin-1 /
   Latin-Extended-A transliteration (e.g. `Café`→`cafe`, matching the admin-x-framework
   fixture), non-alnum → single `-`, trim, 185-char cap, tag `^#` → `hash-` prefix,
   protected-slug suffix (`ghost`/`rss`/`amp`, plus tag `new` → `-<baseName>`) and
   empty → `baseName` fallback. Non-Latin scripts have no transliteration primitive
   (residual below). The admin client slugifies before the request, so this path is
   idempotent for normal callers.
4. **Uniqueness / DB call.** Uses `blog::api::parse_array` over
   `store.json_for(posts|tags|users, {})` (all-status stand-in for `findOne({slug,
   status:'all'})`), compares the JSON-encoded `slug` field and skips the row whose
   `id` equals the `modelId` (4th path segment) — Ghost's `found.id === options.modelId`
   self-exclusion. The `base-2`, `base-3`, … loop is unchanged.
5. **Output / cache / headers.** Success body is built with the shared
   `blog::mapping::wrap("slugs", …)` (CONTRACTS §6) → `{"slugs":[{"slug":…}]}`, no
   pagination meta; 500 `Could not generate slug.` uses the Ghost `InternalServerError`
   envelope. Handler sets the admin-wide `Cache-Control: no-cache, private, no-store,
   must-revalidate, max-stale=0, post-check=0, pre-check=0` and emits no
   `X-Cache-Invalidate` (`cacheInvalidate:false`). No `core::cache::Cache` key added.
6. **Path parsed once.** The handler no longer re-parses the path; the step's
   `split_path` is the single source for `type`, `name`, `model_id`.

### Residuals / new-partition needs (recorded, not added)

- No real Knex/Bookshelf `Model.findOne` or transaction exists in C++; the
  context-local `json_for` read is the accepted stand-in (outbox/cache markers absent,
  no cross-request state).
- Full `unidecode` transliteration for non-Latin scripts (CJK/Cyrillic/etc.) has no
  C++ primitive. **New-partition need:** a shared `core_text:unidecode` partition
  (would require central BUILD registration + Bazel-cycle fix — out of scope here).
- The full 7-stage `authAdminApi` chain (`authenticateAdminApi`, `authorizeAdminApi`,
  `updateUserLastSeen`, `cors`, SSL/host redirect, pretty URLs, `tokenPermissionCheck`)
  and the `permissionIdentifier = :id` nuance are host/framework-owned and not
  observable diffs for the documented fixtures; the shared permission service covers
  the authorization decision.
- `res.set_header("Cache-Control", …)` is written by the handler, so middleware-only
  401 responses (missing bearer) do not carry it; this is host/app-layer behavior in
  Ghost (`web/shared/middleware/cache-control.ts`), not controller-owned.
- Pipeline stage order (`validation.input` before `permissions`, `pipeline.ts:430-447`)
  is not reproduced literally: the C++ permission gate runs in the handler before the
  workflow's validation, so an unauthenticated caller with an invalid `type` receives
  401/403 where Ghost would emit 422. Equivalent to the accepted framework-internal
  residual in the other strict-parity groups; the documented fixtures always send
  valid input with an authorized caller.

### Files changed (folder + docs only)

- `include/blog/handlers/system/slugs/routes.cppm` (middleware chain, comments).
- `include/blog/handlers/system/slugs/get_detail/handler.cppm` (permission gate,
  status/body map, Cache-Control).
- `include/blog/handlers/system/slugs/get_detail/workflow/step/execute.cppm` (full
  slug algorithm, 422/500 envelopes, `blog::mapping::wrap`, `blog::api::parse_array`).
- `docs/parity/behavior/slugs.md` (this record; verdicts flipped).

