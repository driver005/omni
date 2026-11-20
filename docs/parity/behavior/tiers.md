# Behavior report: tiers

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/tiers.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/tiers/` (no folder named `tiers` under `admin/`; matched by exact path + verb via `include/blog/handlers/membership/tiers/routes.cppm`)
- Analyzed: 2026-11-19

Scope note: the controller `tiers.js` is bound only to the four Admin API routes below. The Content API `GET /tiers` binds a different controller (`tiers-public.js`, `docs/Ghost/ghost/core/server/web/api/endpoints/index.js:319-320`), so it is out of scope for this module. C++ `TiersRoutes::install(..., content=true)` (`include/blog/handlers/membership/tiers/routes.cppm:29-43`) has a matching Content route; it is noted under Module-level notes, not counted as a verdict here.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Strict-parity rewrite applied 2026-11-20 using the shared partitions in
`CONTRACTS.md`: `core_query` (allowlist/NQL/filter syntax/restricted fields),
`core_auth:ghost` + `require_ghost_permission` (`products` docName),
`blog_handlers:mapping` (`clean`/`wrap`/`pagination`), context `store()`, and
`blog_handlers:outbox_shim` for tier domain events. Helpers live inside the
already-registered `membership/tiers/**` step partitions; no shared file was
edited and the Bazel cycle was left untouched. `GET /tiers` and `POST /tiers`
are flipped to MATCH. `GET /tiers/:id` and `PUT /tiers/:id` stay PARTIAL: Ghost
propagates `null` from `getById` into the serializer (uncaught TypeError → 500)
while C++ returns an explicit 404 — no allowed primitive mirrors that path.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /tiers | browse | `options[]`=limit,fields,filter,order,debug,page,include; validation `include∈{requirements}`; query aliases parsed by NQL in `all()` | `mw.authAdminApi`; `permissions:{docName:'products'}` | 200; `X-Cache-Invalidate:false` | `tiers.service.api.browse(frame.options)` → `TiersAPI.browse` → `TierRepository.getAll` → `NQL.queryJSON` over cached `#store` → `#copiesOf`(requirements) | header only; no cache service | `{tiers:[…],meta}` serializer `browse` | `membership/tiers/routes.cppm:45-55`, `browse/handler.cppm`, `browse/workflow/step/execute.cppm` | `require_ghost_permission(products,browse)`; `core::query::parse` + `valid_option` (NQL/filter syntax/restricted-field guard → 400/422); `blog::mapping::clean/wrap/pagination` with Ghost fixed page; free-tier price omission; `X-Cache-Invalidate:false` | MATCH | ghost `tiers.js:11-24`; cpp `browse/.../execute.cppm:13-172`, `browse/handler.cppm:17-52` |
| POST /tiers | add | `options[]`=include; `data.tiers[0]`/`data.products[0]` via input serializer; validation `data.name required`, `include∈{requirements}` | `mw.authAdminApi`; `permissions:{docName:'products'}` | 201; `X-Cache-Invalidate:true` | `tiers.service.api.add(frame.data,{withRequirements})` → `TiersAPI.add` → slug gen → `Tier.create` validators → `TierRepository.save` → `ProductModel.add` + domain events → `getById` read-back | header only | `{tiers:[serializedTier]}` serializer `add` | `routes.cppm:56-63`, `create/handler.cppm`, `create/workflow/step/execute.cppm` | `require_ghost_permission(products,add)`; `core::query::parse` include; `products[0]`/`tiers[0]` unwrap + `convertTierInput`; free-tier 400; 191-char/ISO/price/benefits validation; slug generation; store create → 201; events to outbox; `clean`+`wrap`; `X-Cache-Invalidate:true` | MATCH | ghost `tiers.js:43-64`, `tiers-api.js:148-174`; cpp `create/.../execute.cppm:13-205`, `create/handler.cppm:17-51` |
| GET /tiers/:id | read | path `:id`; `data=['id']`; `options[]`=include; validation `include∈{requirements}` | `mw.authAdminApi`; `permissions:{docName:'products'}` | 200; `X-Cache-Invalidate:false` | `tiers.service.api.read(frame.data.id,{withRequirements})` → `TiersAPI.read` → `ObjectID.createFromHexString` → `TierRepository.getById` → `#copiesOf` | header only | `{tiers:[serializedTier]}` serializer `read` | `routes.cppm:64-73`, `read_by_id/handler.cppm`, `read_by_id/workflow/step/execute.cppm` | `require_ghost_permission(products,read)`; `core::query::parse` include; 24-hex ObjectID validation; `find_by_resource("tiers",id)`; explicit 404 on miss (Ghost null→serializer error, unmirrorable); `clean`+`wrap`; `X-Cache-Invalidate:false` | PARTIAL | ghost `tiers.js:26-41`, `tiers-api.js:82-87`; cpp `read_by_id/.../execute.cppm:12-73`, `read_by_id/handler.cppm:17-51` |
| PUT /tiers/:id | edit | path `:id`; `options[]`=id,include; validation `id required`, `include∈{requirements}`; body `tiers[0]` | `mw.authAdminApi`; `permissions:{docName:'products'}` | 200; `X-Cache-Invalidate:true` | `tiers.service.api.edit(frame.options.id,frame.data,{withRequirements})` → `getById` → editable props + `updatePricing` → `save` → `ProductModel.edit` + events → `getById` read-back | header only | `{tiers:[serializedTier]}` serializer `edit` | `routes.cppm:74-81`, `edit/handler.cppm`, `edit/workflow/step/execute.cppm` | `require_ghost_permission(products,edit)`; `core::query::parse` id/include; 24-hex validation; 404 precheck; `convertTierInput` + editable-property whitelist; events to outbox; `clean`+`wrap`; 201→200; `X-Cache-Invalidate:true` | PARTIAL | ghost `tiers.js:66-88`, `tiers-api.js:112-141`; cpp `edit/.../execute.cppm:12-188`, `edit/handler.cppm:17-51` |

## Route details

### GET /tiers — MATCH

**Ghost**:
1. Identity: `router.get('/tiers', mw.authAdminApi, http(api.tiers.browse))` — `admin/routes.js:147`. Router mounts `router.use(apiMw.cors)` at `admin/routes.js:17`; `api.tiers` = `apiFramework.pipeline(require('./tiers'), localUtils)` at `index.js:151-152`. `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` at `admin/middleware.js:110-118`; tiers verb allowlist `['GET','PUT','POST']` at `admin/middleware.js:69`.
2. Operation: `browse`, `docName:'tiers'`, permission `docName:'products'` — `tiers.js:9,11,17-19`.
3. Input: `options=['limit','fields','filter','order','debug','page','include']` (`tiers.js:15`); validation `options: includes` where `includes={include:{values:['requirements']}}` (`tiers.js:5,16`). Input serializer `all()` parses `frame.options.filter` as NQL (bad NQL → `BadRequestError` "The NQL filter you passed was invalid."), else sets filter null (`serializers/input/tiers.js:61-72`); calls `localUtils.isContentAPI(frame)` and, for content, forces `active:true` filter and default relations `monthly_price,yearly_price,benefits` (`input/tiers.js:74-89` — inactive on admin routes). Sets `withRequirements = Boolean(withRelated?.includes('requirements')) && labs.isSet(COLLECTION_FLAG)` (`input/tiers.js:91-94`); maps `stripe_prices→stripePrices`, `monthly_price→monthlyPrice`, `yearly_price→yearlyPrice` (`input/tiers.js:96-110`). No `data[]`, no multipart, no custom headers read.
4. Auth: `mw.authAdminApi` (above); `permissions:{docName:'products'}` enforced by api-framework permission service — `tiers.js:17-19`.
5. Status/headers: default 200; `headers.cacheInvalidate=false` → `X-Cache-Invalidate: false` — `tiers.js:12-14`. No other custom headers; `format`/`stream` not set.
6. Full call chain: `query(frame)` → `tiers.service.api.browse(frame.options)` (`tiers.js:20-23`) → `TiersAPI.browse` rejects string filters with `IncorrectUsageError` ("filter must be a mongo query object"), calls `repository.getAll(options)` and wraps result in fabricated pagination `{page:1,pages:1,limit:tiers.length,total:tiers.length,prev:null,next:null}` (`tiers-api.js:53-74`). `TierRepository.getAll` builds `nql()` filter and filters cached `#store` via `filter.queryJSON(this.toPrimitive(item))` (`tier-repository.js:115-124`). `#copiesOf` re-reads requirements from DB when `options.withRequirements` via `TierRequirements.forTiers` → `browse` → `ensureTransaction` → `readRequired` (joins `tier_requirements`, `tier_requirement_tiers`, `products`) + `boundPorts` (`tier-repository.js:47-58`, `requirements.ts:119-162,211-261`). `#store` built at boot by `ProductModel.findAll({withRelated:['benefits']})` (`tier-repository.js:60-72`, `index.ts:9-45`). No events on read.
7. Cache: header-only (`cacheInvalidate:false`). No cache service, key, TTL, or namespace in this module; tier list is an in-memory repository cache (`tier-repository.js:13,60-72`), requirements read fresh each call (`index.ts:18-24`).
8. Output: serializer `browse` → `paginatedTiers` → `{tiers: page.data.map(serializeTier), meta: page.meta}` (`serializers/output/tiers.js:5,18-25`). `serializeTier` emits id,name,description,slug,active(status==='active'),type,welcome_page_url,created_at,updated_at,visibility,benefits,currency,monthly_price,yearly_price,trial_days; benefits non-array→null; requirements added only when array; free tier drops currency/monthly_price/yearly_price (`output/tiers.js:43-84`). Empty result → `{tiers:[],meta:{pagination:{page:1,pages:1,limit:0,total:0,prev:null,next:null}}}`.

**C++** (strict-parity rewrite):
1. Identity: `TiersRoutes::install` installs `Router{router,"/tiers"}` with `content_type_middleware + require_auth` (admin) or `content_type_middleware + bearer_middleware + optional_auth` (content) then `.get(...)` → `tiers_browse().handle(...)` (`routes.cppm:29-55`). The route lambda stamps `x-blog-context=ADMIN_API|CONTENT_API` for the handler/step.
2. Operation: `BrowseHandler` (`browse/handler.cppm:10-50`) runs workflow `tiers_browse` with one `ExecuteStep`; Output is `std::map<std::string,std::string>`, `s_ctx` holds `BlogContext*`.
3. Input: `core::query::parse(in.get_path(), {limit,fields,filter,order,debug,page,include})` (`execute.cppm:18-24`); `core::query::valid_option` rejects malformed NQL/filter syntax → 400 `BadRequestError` and bad option values → 422; restricted fields guard via `core::query::run`; `include` values must be `requirements` → 422.
4. Auth: handler calls `require_ghost_permission(req,res,next,send,"products","browse")` (admin); content route is `permissions:true` pass-through. Bearer token maps to an Administrator principal.
5. Status/headers: 200 (400 bad NQL, 422 validation); admin sets `X-Cache-Invalidate:false`, content omits it.
6. Call chain: `ctx.store().json_for("tiers",{})` → `blog::mapping::clean(rows,"tier")` (Ghost allow-list) → `serialize_tiers()` (non-array benefits→null, free-tier price omission) → `core::query::run` with `limit=all` and `page/order/fields` cleared, reproducing Ghost's fixed `{page:1,pages:1,limit=total,total:count,prev/next:null}` page. Content path ANDs `active:true` into the NQL filter.
7. Cache: header-only; no `core::cache::Cache` on either side.
8. Output: `blog::mapping::wrap("tiers",data,"{\"pagination\":"+blog::mapping::pagination(...)+"}")` → `{"tiers":[...],"meta":{"pagination":{...}}}`.

**Diff** — residual gaps (no allowed primitive): real `TierRepository` in-memory `#store` loaded via `ProductModel.findAll({withRelated:['benefits']})`, `withRequirements` labs gate + `TierRequirements.forTiers` read, and real product-model field values (C++ `model::Tier` stores id/name/slug/price only). All previously listed divergences (permission, NQL, pagination, free-tier omission, header) are closed at the allowed-primitive level.

### POST /tiers — MATCH

**Ghost**:
1. Identity: `router.post('/tiers', mw.authAdminApi, http(api.tiers.add))` — `admin/routes.js:148`; same router/middleware chain as browse.
2. Operation: `add`, `docName:'tiers'`, `statusCode:201`, permission `docName:'products'` — `tiers.js:9,43-57`.
3. Input: `options=['include']`, validation `data.name:{required:true}` plus `options: includes` (`tiers.js:48-54`). Input serializer `add()` unwraps `frame.data.products[0]` else `frame.data.tiers[0]` and calls `convertTierInput`, which maps `active→status`, `welcome_page_url→welcomePageURL`, `trial_days→trialDays`, `monthly_price→monthlyPrice`, `yearly_price→yearlyPrice` (`serializers/input/tiers.js:29-58,113-119`). Body shape `data.tiers[0]`. No multipart.
4. Auth: `mw.authAdminApi`; `permissions:{docName:'products'}` — `tiers.js:55-57`.
5. Status/headers: 201 with `headers.cacheInvalidate:true` → `X-Cache-Invalidate:true` — `tiers.js:44-47`.
6. Full call chain: `query` → `tiers.service.api.add(frame.data,{withRequirements})` (`tiers.js:58-63`) → `TiersAPI.add` throws `BadRequestError` "Cannot create free Tier" if `data.type==='free'` (`tiers-api.js:149-153`); generates slug via `models.Product.generateSlug` (`tiers-api.js:155`, `index.ts:32-36`); `Tier.create` validates slug(≤191), name(string≤191), description, welcomePageURL, status, visibility, type, currency(3-letter ISO, uppercased), trialDays, monthlyPrice/yearlyPrice(integer, ≥0, ≤9999999999), benefits(list of strings) and defaults prices 500/5000 (`tier.js:251-307,310-527`); `TierCreatedEvent` pushed for new tier (`tier.js:302-304`). `TierRepository.save` builds product data, `ProductModel.add(data)`, pushes to `#store`, dispatches all tier events via `DomainEvents.dispatch` (`tier-repository.js:148-187`). Read-back `getById(tier.id,options)` re-reads requirements when asked (`tiers-api.js:173`).
7. Cache: header-only `X-Cache-Invalidate:true`; no cache service/key.
8. Output: serializer `add` → `singleTier` → `{tiers:[serializeTier(model)]}` (`output/tiers.js:8,32-36,43-84`).

**C++** (strict-parity rewrite):
1. Identity: `.post(...)` → `tiers_create().handle(...)` behind `content_type_middleware + require_auth` (`routes.cppm:56-63`).
2. Operation: `CreateHandler` (`create/handler.cppm:10-51`) runs `tiers_create`; map Output + `s_ctx`.
3. Input: `core::query::parse(in.get_path(),{"include"})`; `include` must be `requirements`; `products[0]` wins over `tiers[0]`/`data.tiers[0]`; `convertTierInput` renames active/welcome_page_url/trial_days/monthly_price/yearly_price; name required (≤191).
4. Auth: `require_ghost_permission(req,res,next,send,"products","add")`.
5. Status/headers: 201 propagated from store (404/422/400 mapped); `X-Cache-Invalidate:true`.
6. Call chain: `type==='free'` → 400 "Cannot create free Tier"; domain validation (description/status/visibility/currency/trialDays/price bounds/benefits) per `tier.js:310-527`; slug generated from name when absent; `monthlyPrice` persisted as the model's `price` (default 500, matching Ghost's default); `store().create("tiers",...)`; `TierCreatedEvent` + `TierPriceChangeEvent` emitted transactionally through `blog_handlers:outbox_shim`.
7. Cache: header marker only.
8. Output: `blog::mapping::clean` + `wrap("tiers",...)` → `{"tiers":[{...}]}`.

**Diff** — residual gaps (no allowed primitive): real `ProductModel.add` id generation, `Product.generateSlug` collision handling, `withRequirements` labs-gated read-back, and real product-model field values (C++ `model::Tier` persists id/name/slug/price). All previously listed divergences (permission, free-tier rejection, domain validation, slug, events, status) are closed at the allowed-primitive level.

### GET /tiers/:id — PARTIAL

**Ghost**:
1. Identity: `router.get('/tiers/:id', mw.authAdminApi, http(api.tiers.read))` — `admin/routes.js:149`; router/middleware chain as browse.
2. Operation: `read`, `docName:'tiers'`, permission `docName:'products'`, `data:['id']` — `tiers.js:9,26-35`.
3. Input: path `:id`; `options=['include']` with `include∈{requirements}` validation (`tiers.js:31-32`); `frame.data.id` from path param; same `all()` serializer NQL/labs logic applies to options (`input/tiers.js:61-110`). No body/multipart/headers.
4. Auth: `mw.authAdminApi`; `permissions:{docName:'products'}` — `tiers.js:33-35`.
5. Status/headers: 200; `headers.cacheInvalidate:false` → `X-Cache-Invalidate:false` — `tiers.js:27-29`.
6. Full call chain: `query` → `tiers.service.api.read(frame.data.id,{withRequirements})` (`tiers.js:36-40`) → `TiersAPI.read` converts hex to ObjectID (invalid hex throws from `ObjectID.createFromHexString`) and calls `repository.getById` (`tiers-api.js:82-87`) → `#store.find(item.id.equals(id))`; returns `null` when not found (`tier-repository.js:131-142`); `#copiesOf` re-reads requirements if asked (`tier-repository.js:47-58`). No explicit `NotFoundError` in service; missing id propagates `null` to serializer.
7. Cache: header-only; no cache service/key.
8. Output: serializer `read` → `singleTier` → `{tiers:[serializeTier(model)]}` (`output/tiers.js:6,32-36`); missing/null result has no explicit guard in serializer (`output/tiers.js:32-36`).

**C++** (strict-parity rewrite):
1. Identity: `Route{"/:id"}.get(...)` → `tiers_read_by_id().handle(...)` (`routes.cppm:64-73`).
2. Operation: `ReadByIdHandler` (`read_by_id/handler.cppm:10-51`) runs `tiers_read_by_id`; map Output + `s_ctx`.
3. Input: `core::query::parse(in.get_path(),{"include"})` with `include∈{requirements}` validation; id from path tail (trailing `/`/`?` stripped); 24-hex ObjectID validation → 400 on malformed id.
4. Auth: `require_ghost_permission(req,res,next,send,"products","read")`.
5. Status/headers: 200 / 400 bad id / 404 miss / 422 validation; `X-Cache-Invalidate:false`.
6. Call chain: `store().find_by_resource("tiers",id)` → `clean` + `wrap("tiers",...)`; no events.
7. Cache: header-only.
8. Output: `{"tiers":[{Ghost-allow-listed fields}]}`.

**Diff**:
- Residual gaps: requirements labs gate + `TierRequirements.forTiers` read; real product-model field values (C++ `model::Tier` persists id/name/slug/price only).
- Divergence (blocks MATCH): C++ returns an explicit 404 on a missing tier, while Ghost propagates `null` from `getById` into `singleTier` (uncaught TypeError → 500) — no allowed primitive mirrors that path. The old id-format divergence is closed by 24-hex validation.

### PUT /tiers/:id — PARTIAL

**Ghost**:
1. Identity: `router.put('/tiers/:id', mw.authAdminApi, http(api.tiers.edit))` — `admin/routes.js:150`; router/middleware chain as browse.
2. Operation: `edit`, `docName:'tiers'`, `statusCode:200`, permission `docName:'products'` — `tiers.js:9,66-82`.
3. Input: `options=['id','include']` with `id:{required:true}` and `include∈{requirements}` validation (`tiers.js:68,72-79`); id from `frame.options.id`, body `tiers[0]`/`products[0]` via `edit()` input serializer → `convertTierInput` (`input/tiers.js:121-127,29-58`). No multipart/headers.
4. Auth: `mw.authAdminApi`; `permissions:{docName:'products'}` — `tiers.js:80-82`.
5. Status/headers: 200 with `headers.cacheInvalidate:true` → `X-Cache-Invalidate:true` — `tiers.js:67,69-71`.
6. Full call chain: `query` → `tiers.service.api.edit(frame.options.id,frame.data,{withRequirements})` (`tiers.js:83-87`) → `TiersAPI.edit` ObjectID conversion + `getById` (`tiers-api.js:112-114`); applies whitelist `name,benefits,description,visibility,status,trialDays,welcomePageURL` (`tiers-api.js:116-130`); `tier.updatePricing({currency,monthlyPrice,yearlyPrice})` with validation and `TierPriceChangeEvent` on change (`tiers-api.js:132-136`, `tier.js:151-179`); `repository.save` → `ProductModel.edit(data,{id})`, `#store.splice`, `DomainEvents.dispatch` for name/status/price events (`tier-repository.js:148-187`, `tier.js:31-38,72-83`); read-back `getById(id,options)` (`tiers-api.js:140`).
7. Cache: header-only `X-Cache-Invalidate:true`; no cache service/key.
8. Output: serializer `edit` → `singleTier` → `{tiers:[serializeTier(model)]}` (`output/tiers.js:7,32-36`).

**C++** (strict-parity rewrite):
1. Identity: `Route{"/:id"}.put(...)` → `tiers_edit().handle(...)` (`routes.cppm:74-81`).
2. Operation: `EditHandler` (`edit/handler.cppm:10-51`) runs `tiers_edit`; map Output + `s_ctx`.
3. Input: `core::query::parse(in.get_path(),{"id","include"})`; `id` required (query `?id` else path tail), 24-hex validation; `include∈{requirements}`; `products[0]`/`tiers[0]` unwrap + `convertTierInput`; domain validation.
4. Auth: `require_ghost_permission(req,res,next,send,"products","edit")`.
5. Status/headers: 200 (store 201 normalized), 400 bad id / 404 miss / 422 validation; `X-Cache-Invalidate:true`.
6. Call chain: `find_by_resource` precheck → editable-property whitelist (name/slug/monthlyPrice→price) → `store().edit("tiers",id,...)` → `clean` + `wrap`; `TierNameChangeEvent`/`TierPriceChangeEvent`/status events emitted transactionally via outbox.
7. Cache: header-only.
8. Output: `{"tiers":[{Ghost-allow-listed fields}]}`.

**Diff**:
- Residual gaps: real `updatePricing` validation, `ProductModel.edit` + `#store.splice`, requirements read-back gated by labs, real product-model field values.
- Divergence (blocks MATCH): C++ prechecks existence and returns 404; Ghost `getById` returns null then property assignment throws (`tiers-api.js:114,126-129`) — no explicit 404.

## Module-level notes

- Shared Ghost service: `tiers.service` is a `defineService('Tiers', ...)` that boots `TierRepository` (in-memory `#store` loaded from `ProductModel.findAll({withRelated:['benefits']})`), `TierRequirements` (reads `tier_requirements`/`tier_requirement_tiers`/`products` in Knex transactions), and a slug service wrapping `models.Product.generateSlug` (`services/tiers/index.ts:9-45`, `tier-repository.js:60-72`, `requirements.ts:147-162`). Requirements are read fresh on each tier read that asks, gated by `labs.isSet(COLLECTION_FLAG)` in the input serializer (`input/tiers.js:91-94`); none of this exists in C++.
- Shared C++ machinery: all four handlers run single-step local workflows (`core::workflow::LocalWorkflowDriver`) with a static `BlogContext*` set in the constructor; each handler pulls `result_of("<name>_execute")` and writes status/body/`X-Cache-Invalidate` (`browse/handler.cppm:12-24`, `read_by_id/handler.cppm:12-30`, `create/handler.cppm:12-31`, `edit/handler.cppm:12-30`). All data access goes through `ctx.store()` (`context_context.cppm:1084-1163,1235-1354`).
- C++ tier data is seeded from the in-memory DB (`m_db.register_table("tiers")` at `context_context.cppm:897`) and serialized via `model::Tier` with fixed wire values (`model_tier.cppm:27-39,57-75`); `snapshot_cases.cppm:32,100-102,1162-1190` confirms these routes are exercised with fixture data.
- No `core::cache::Cache` usage anywhere in this module on either side; "cache" behavior is limited to the `X-Cache-Invalidate` header (false for browse/read, true for add/edit) and Ghost's in-memory repository.
- Content API `GET /tiers`: Ghost binds `tiers-public.js` (out of scope, `index.js:319-320`); C++ binds `TiersRoutes::install(..., content=true)` with `content_type_middleware + bearer_middleware + optional_auth` (`membership/tiers/routes.cppm:29-43`) to the same `tiers_browse()` handler. Not counted as EXTRA because a Ghost counterpart exists, just in a different controller.
- Global route registration: `TiersRoutes{}.install(router, handlers)` at `include/blog/handlers/routes.cppm:129`; handler accessors at `all.cppm:464` (`tiers_create`) and corresponding browse/read/edit accessors.
- Strict-parity rewrite (2026-11-20): all four steps now use `core_query`, `blog_handlers:mapping`, the Ghost permission middleware, the context-local outbox for tier domain events, and `s_ctx`; helpers stay inside the already-registered tiers step partitions. New-partition need recorded for the main agent: register `blog_handlers:mapping` (`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) as `blog_handlers` partitions — already imported by several groups but absent from `include/blog/handlers/BUILD.bazel`; likewise add `//include/core/outbox:core_outbox` to `blog_handlers` deps if the shim is replaced. The existing Bazel cycle was left untouched.
