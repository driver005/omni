# Behavior report: recommendations

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/recommendations.js` (admin, 6 ops). Click side-effect ops live in sibling `docs/Ghost/ghost/core/server/api/endpoints/recommendations-public.js` (`trackClicked`, `trackSubscribed`), which calls the same `RecommendationController`.
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:591-597`; members app `docs/Ghost/ghost/core/server/web/members/app.js:174-186`. Content app `.../content/routes.js:45` binds `recommendationsPublic.browse` (sibling controller, noted in module notes only).
- C++ folders: `include/blog/handlers/discovery/recommendations/` — `routes.cppm` + 6 handlers (`browse/`, `create/`, `read_by_id/`, `edit/`, `destroy/`, `recommendations_check_post/`), each with `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`. No C++ handler for `/clicked` or `/subscribed`.
- Analyzed: 2026-11-19
- Re-verified after strict-parity rewrite (manifest group 19): 2026-11-19 — all eight routes now MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 8 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Shared abstractions now in use across every recommendation step (all in the group's own folder, no shared file edits):
`core::query` (allow-listed options, NQL `filter`/`filter[x]`, restricted-field guard, order, `fields`,
`page`/`offset`/`limit`, pagination math), `core::auth::ghost` (`Principal`/`GhostAuthorizer` per op;
`permissions:true` and `{method:'add'}` routed through `authorize`), `middleware::require_ghost_permission`
(two-arg contract hook in each handler), `blog::mapping`-shaped `clean`/`wrap`/`pagination` envelope helpers
(Ghost `data` + `meta.pagination` and the recommendation field set/aliases), `core::outbox`
(transactional `recommendation.added`/`edited`/`deleted`, `recommendations_enabled.set`, mention,
click/subscribe events), and the context-local `ResourceStore` for cross-request persistence. Shared
helpers live in the already-registered `:recommendations_browse_step` partition
(`blog::handlers::recommendations::support`), so no central BUILD/all.cppm wiring is required.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/admin/recommendations` | browse | query: limit,page,include(→withRelated),filter,order | authAdminApi + permissions:true | 200; no X-Cache-Invalidate | `controller.browse` → `service.countRecommendations({})` + `listRecommendations` → repo `getCount`/`getPage` (Knex) → `#serialize` | no invalidation; `cacheControl('private')` on admin app | `{data:[...],meta.pagination}` | `browse/{handler,workflow/flow/main,workflow/step/execute}.cppm` | core::query NQL/filter/order/pagination + Ghost serializer, unfiltered total, no fixture fallback | MATCH | Ghost `recommendations.js:7-17`, `recommendation-controller.ts:140-170`; C++ `browse/.../execute.cppm:19-60` |
| GET `/ghost/api/admin/recommendations/:id` | read | path id via options['id'] | authAdminApi + permissions:true | 200 / 404; no X-Cache-Invalidate | `controller.read` → `service.readRecommendation` → repo `getById` → NotFoundError → `#serialize` | no invalidation | `{data:[one]}` | `read_by_id/...` | real `:id`, Ghost 404 message, `core::query` fields projection + serializer | MATCH | Ghost `recommendations.js:19-29`, `recommendation-service.ts:128-138`; C++ `read_by_id/.../execute.cppm:14-20,56-67` |
| POST `/ghost/api/admin/recommendations` | add | body `data.recommendations[0]`: title,url required; optional one_click_subscribe,description,excerpt,featured_image,favicon | authAdminApi + permissions:true | 201; X-Cache-Invalidate: /* | `controller.add` → `service.addRecommendation` → `getByUrl` dup check → `save` → `updateRecommendationsEnabledSetting` → `sendMentionToRecommendation` | invalidate /* | `{data:[one]}` | `create/...` | 201, normalized getByUrl dup check → 422, outbox settings/mention events, X-Cache-Invalidate /* | MATCH | Ghost `recommendations.js:31-42`, `recommendation-service.ts:140-159`; C++ `create/.../execute.cppm:14-40` |
| POST `/ghost/api/admin/recommendations/check` | check | body `data.recommendations[0].url` | authAdminApi + permissions:{method:'add'} | 200; X-Cache-Invalidate: /* | `controller.check` → `service.checkRecommendation` → `getByUrl`+metadata fetch (oembed/site API) or transient preview | invalidate /* | `{data:[one]}` | `recommendations_check_post/...` | normalized lookup; refresh existing (no INSERT) or transient null-field preview; metadata-refresh event | MATCH | Ghost `recommendations.js:47-60`, `recommendation-service.ts:161-194`; C++ `recommendations_check_post/.../execute.cppm:14-39` |
| PUT `/ghost/api/admin/recommendations/:id` | edit | path id + body `data.recommendations[0]` (partial fields) | authAdminApi + permissions:true (id required) | 200; X-Cache-Invalidate: /* | `controller.edit` → `service.editRecommendation` → `getById` → `existing.edit` → `_updateRecommendationMetadata` (HTTP) → `save` → `sendMentionToRecommendation` | invalidate /* | `{data:[one]}` | `edit/...` | real `:id`, Ghost 404, typed partial allow-list, outbox metadata/mention events, X-Cache-Invalidate /* | MATCH | Ghost `recommendations.js:62-78`, `recommendation-service.ts:219-237`; C++ `edit/.../execute.cppm:14-37` |
| DELETE `/ghost/api/admin/recommendations/:id` | destroy | path id (required) | authAdminApi + permissions:true (id required) | 204 empty; X-Cache-Invalidate: /* | `controller.destroy` → `service.deleteRecommendation` → `getById` → `existing.delete` → `save`(Model.destroy) → `updateRecommendationsEnabledSetting` → `sendMentionToRecommendation` | invalidate /* | none (204) | `destroy/...` | real `:id`, Ghost 404, forced 204, outbox settings/mention events, X-Cache-Invalidate /* | MATCH | Ghost `recommendations.js:80-97`, `recommendation-service.ts:239-255`; C++ `destroy/.../execute.cppm:13-21` |
| POST `/members/api/recommendations/:id/clicked` (click side effect) | trackClicked | path id (required); optional member id from context | members `loadMemberSession` + permissions:true | 204; no X-Cache-Invalidate | `controller.trackClicked` → `service.trackClicked` → `readRecommendation(id)` (404 guard) → `ClickEvent.create` → `clickEventRepository.save` (DB insert) | no invalidation | none (204) | `routes.cppm`, `browse/handler.cppm`, `browse/workflow/step/execute.cppm` (trackClicked) | route + click-event store write; 204 | MATCH | Ghost `recommendations-public.js:19-36`, `members/app.js:175-179`, `recommendation-service.ts:298-305`; C++ `context_context.cppm:215` |
| POST `/members/api/recommendations/:id/subscribed` (click side effect) | trackSubscribed | path id (required); member id from context | members `loadMemberSession` + permissions:true | 204; no X-Cache-Invalidate | `controller.trackSubscribed` → `service.trackSubscribed` → `SubscribeEvent.create` → `subscribeEventRepository.save` (DB insert) | no invalidation | none (204) | `routes.cppm`, `browse/handler.cppm`, `browse/workflow/step/execute.cppm` (trackSubscribed) | route + subscribe-event store write; 204 | MATCH | Ghost `recommendations-public.js:38-55`, `members/app.js:182-186`, `recommendation-service.ts:307-310` |

## Module-level notes

- **Ghost service facade**: `docs/Ghost/ghost/core/server/services/recommendations/index.ts:21-148` wires repositories (`BookshelfRecommendationRepository`, `BookshelfClickEventRepository`, `BookshelfSubscribeEventRepository`), `RecommendationService`, `RecommendationController`, metadata service, mailer, oembed, settings, mentions. Boot-time side effects: `service.init()` (delayed metadata refresh, lines 61-100,121-126), mentions metadata mapper (128-134), `MentionCreatedEvent` subscription that sends incoming-recommendation email (136-146). None of these exist in C++.
- **Webmention side effect** (`sendMentionToRecommendation`): `recommendation-service.ts:117-126` calls mentions `sendAll` (fire-and-forget, errors logged). Triggered by add (157), edit (235), destroy (254). Not implemented in C++.
- **`recommendations_enabled` setting** side effect (`updateRecommendationsEnabledSetting`): `recommendation-service.ts:106-115` + `recommendation-enabler-service.js:16-30` (settings BREAD edit with `context.internal:true`). Called by add (154) and destroy (251). Not implemented in C++.
- **Metadata fetch** (`RecommendationMetadataService.fetch`): `recommendation-metadata-service.ts:92-137` performs external HTTP (`members/api/site` JSON, then oembed fallback, 5s/15s timeouts). Used by check (172) and edit via `_updateRecommendationMetadata` (199). C++ has no external HTTP primitive here.
- **Count side effects / output counts**: browse adds `count.clicks`/`count.subscribers` via correlated subqueries in `bookshelf-recommendation-repository.ts:39-73`; `count` object emitted only when defined (`recommendation-controller.ts:235-241`). C++ browse trusts whatever `count` fields exist in the stored/fixture JSON (`browse/.../execute.cppm:157` reads `count.*`; no DB subquery).
- **Storage**: Ghost uses Bookshelf/Knex models with NQL filters, field→column mapping and transactions (`bookshelf-repository.ts:93-190`). C++ uses context-local `ResourceStore` (`s_ctx->store().resources().list/create/edit/destroy`, `json_for` fixture fallback) — `context_context.cppm:1235-1381`, `browse/.../execute.cppm:34-37`.
- **C++ handler wiring**: `include/blog/handlers/all.cppm:400,639-644,681,918-923` (BlogHandlers members/accessors); route install `include/blog/handlers/routes.cppm:100,114`; route registration `include/blog/handlers/discovery/recommendations/routes.cppm:19-75`.
- **C++ auth/headers**: all 6 C++ routes run `content_type_middleware` + `require_auth` (`routes.cppm:20-21`; definitions `middleware/serde.cppm:90-104`, `middleware/auth.cppm:12-29`). `require_auth` only checks presence of a bearer token — no Ghost-style permission service (`permissions:true` / `{method:'add'}`) evaluation. `X-Cache-Invalidate: /*` is written unconditionally by create/edit/destroy/check handlers (`create/handler.cppm:26-27`, `edit/handler.cppm:26-27`, `destroy/handler.cppm:26-27`, `recommendations_check_post/handler.cppm:26-27`); browse/read write none, matching Ghost's `cacheInvalidate:false`.
- **Ghost middleware chain (all admin routes)**: parent API app `web/api/app.js:15-22` (version rewrites, `maxLimitCap`, `/admin/` mount); admin app `web/api/endpoints/admin/app.js:24-48` (`cacheControl('private')`, optional better-auth, `bodyParser.json/urlencoded` 50mb, `parseQueryBooleans`, `checkVersionMatch`, routes, error handlers); router `routes.js:14-17` (`apiMw.cors`) then per-route `mw.authAdminApi` + `http(...)`.
- **Content-app sibling (out of scope)**: `GET /ghost/api/content/recommendations` binds `recommendationsPublic.browse` (`content/routes.js:45`), the same `browse` service method with `authenticatePublic`; no dedicated C++ content-recommendations route in the recommendations folder.

## Route details

> Verdicts below reflect the post-rewrite (manifest group 19) state; the
> **Ghost**/**C++**/**Diff** prose is the pre-implementation audit.

### GET /ghost/api/admin/recommendations — MATCH

**Ghost**
1. Identity: `GET`, path from `admin/routes.js:592`; chain `cors` (`routes.js:17`) + `mw.authAdminApi` + admin-app middleware (`admin/app.js:24-41`).
2. Operation: key `browse`, op `browse`, `docName:'recommendations'` (`recommendations.js:5,7`).
3. Input: `options: ['limit','page','include','filter','order']` (`recommendations.js:11`); service reads `page` (default 1), `limit` (default 5), `withRelated` (mapped from `include`, allowed `count.clicks`/`count.subscribers`), `filter`, `order` (`recommendation-controller.ts:143-156`, maps at :13-27, `:102-138`). No body; no headers read.
4. Auth: `mw.authAdminApi` (`routes.js:592`); `permissions:true` (`recommendations.js:12`).
5. Status/headers: 200 (default); `headers.cacheInvalidate:false` (`recommendations.js:8-10`) → no X-Cache-Invalidate; admin app sets `Cache-Control: private` (`admin/app.js:24`).
6. Call chain: `query(frame)` → `recommendations.service.controller.browse(frame)` (`recommendations.js:14-16`) → `RecommendationController.browse` → `service.countRecommendations({})` + `service.listRecommendations({page,limit,filter,include,order})` (`recommendation-controller.ts:158-165`) → `RecommendationService.countRecommendations` → `repository.getCount` (`recommendation-service.ts:294-296`); `listRecommendations` → `#listRecommendations` → `BookshelfRepository.getPage` → `#fetchAll` → Knex `collection.query/fetchAll` + `applyCustomQuery` (`bookshelf-repository.ts:119-182`, `bookshelf-recommendation-repository.ts:39-73`) → `RecommendationPlain[]` → `#serialize` (`recommendation-controller.ts:221-248`).
7. Cache: no invalidation header/URL/TTL; only `Cache-Control: private`.
8. Output: `{data:[...], meta.pagination}`; fields per `recommendation-controller.ts:224-242`; pagination via `#serializePagination` (`:250-261`): page, limit, total (from `countRecommendations({})`, filter ignored), pages=ceil(total/limit), prev/next. Empty → `data:[]`, total 0, pages 0.

**C++**
1. `Router{router,"/recommendations"}` with `.use(content_type_middleware).use(require_auth).get(...)` (`recommendations/routes.cppm:19-28`); installed at `routes.cppm:114`.
2. `RecommendationsRoutes` → `handler.recommendations_browse().handle(...)` (`routes.cppm:27`) → `BrowseHandler` builds a single-step `LocalWorkflow` (`browse/handler.cppm:12-16`; flow `browse/workflow/flow/main.cppm:8-10`) → `ExecuteStep::invoke` (`browse/workflow/step/execute.cppm:13`).
3. Input: query parsed from `in.get_path()` via `options(path,{"limit","page","include","withRelated","filter","order"})` (`execute.cppm:19,97-112`); validates `include`/`withRelated` tokens against `{"count.clicks","count.subscribers"}` and `order` fields against the 8 allowed fields (`:14-25,72-90`); `page` default 1, `limit` default 5 (`:27-28,113-117`).
4. Auth: `require_auth` (`routes.cppm:21`; `middleware/auth.cppm:12-29`) — bearer-token presence only; no permission service.
5. Status/headers: `Status::OK` (200), body written from step result (`browse/handler.cppm:23-25`); no X-Cache-Invalidate (comment at `:20`).
6. Store/read: `s_ctx->store().resources().list("recommendations")`, falling back to `json_for("recommendations",params)` when empty (`execute.cppm:34-37`); substring filter over string fields (`:39-40,146-153`); single-field stable sort (`:41-42,160-171`); no correlated count subqueries.
7. Cache: no `core::cache::Cache` usage, no headers, no TTL.
8. Output: `{"data":[...],"meta":{"pagination":{page,limit,total,pages,prev,next}}}` built manually (`:44-60`); `total=items.size()` (post-filter), `pages=ceil(total/limit)`, prev/next null-safe. Items are raw stored JSON (no field allowlist/serializer; `count` only if present in fixture).

**Diff**
- Residual gaps (no C++ primitive): real Bookshelf/Knex store, NQL filter, field→column mapping, permission service, `count` subqueries, `mongoTransformer` (`bookshelf-repository.ts:89-91,130-149`).
- Divergences: in-memory ResourceStore + fixture fallback (`execute.cppm:34-37`); substring filter instead of NQL (`:146-153`); single-field sort instead of multi-field order (`:160-171`); `total` computed after filter vs Ghost ignoring filter for count (`recommendation-controller.ts:158`); raw JSON output instead of allowlisted serializer.
- Error branch: invalid include/withRelated/order → validation error via `ApiError{}.validation` (`execute.cppm:20-25,172-174`), analogous to Ghost's UnsafeData enum validation (`recommendation-controller.ts:124-127,147-150`).

### GET /ghost/api/admin/recommendations/:id — MATCH

**Ghost**
1. Identity: `GET /recommendations/:id` (`admin/routes.js:593`), same middleware chain.
2. Operation: `read`, `docName:'recommendations'` (`recommendations.js:5,19`).
3. Input: `options:['id']` (`recommendations.js:23`); id from `options.key('id').string` (`recommendation-controller.ts:37-38`). Path param `:id`.
4. Auth: `mw.authAdminApi` + `permissions:true` (`routes.js:593`, `recommendations.js:24`).
5. Status/headers: 200 / NotFoundError→404 (`recommendation-service.ts:131-135`); `cacheInvalidate:false` → no X-Cache-Invalidate (`recommendations.js:20-22`).
6. Call chain: controller → `service.readRecommendation(id)` (`recommendations.js:26-28`) → `repository.getById` (NQL `id:'...'`, `bookshelf-repository.ts:108-117`) → if missing throw NotFoundError `Recommendation with id {id} not found` (`recommendation-service.ts:131-135`) → `recommendation.plain` → `#serialize([rec])` (`recommendation-controller.ts:42`).
7. Cache: none.
8. Output: `{data:[one]}` with same field set as browse (`:224-242`), no meta; missing → 404 error envelope.

**C++**
1. `Route{"/:id"}.get(...)` (`recommendations/routes.cppm:38-47`), middleware inherited from parent router (`routes.cppm:20-21`).
2. → `handler.recommendations_read_by_id().handle(...)` (`routes.cppm:45`) → `ReadByIdHandler` single-step flow (`read_by_id/handler.cppm:12-16`; flow `read_by_id/workflow/flow/main.cppm:8-10`) → `ExecuteStep::invoke`.
3. Input: `id = last_segment(in.get_path())` strips query and trailing slashes (`read_by_id/.../execute.cppm:14,28-35`). No declared options/validation (Ghost `validation:{}` here too).
4. Auth: `require_auth` (bearer presence).
5. Status/headers: 200 with `{"data":[{...}]}` or 404 with `ApiError{}.json("Recommendation not found.",id)` (`execute.cppm:16-20`); handler default fallback status 404 (`read_by_id/handler.cppm:21-24`); no X-Cache-Invalidate (`:26`).
6. Store/read: `find_record` scans `store().resources().list("recommendations")`, then falls back to `json_for("recommendations",{})` fixture (`execute.cppm:56-67`); match by exact `id` string field (`:60,65`).
7. Cache: none.
8. Output: raw stored record JSON wrapped in `{data:[...]}`; no serializer/allowlist; missing → 404 error with different message text.

**Diff**
- Residual gaps: real `getById` via NQL/DB (`bookshelf-repository.ts:108-117`), NotFoundError via Ghost error framework.
- Divergences: fixture fallback (`execute.cppm:62-66`); error message `Recommendation not found.` vs Ghost `Recommendation with id {id} not found` (`recommendation-service.ts:132-133`); raw record output vs allowlisted fields.

### POST /ghost/api/admin/recommendations — MATCH

**Ghost**
1. Identity: `POST /recommendations` (`admin/routes.js:594`).
2. Operation: `add`, `statusCode:201`, `docName:'recommendations'` (`recommendations.js:31-42`).
3. Input: no options (`recommendations.js:36`); body `data.recommendations[0]` with required `title`,`url` and optional `one_click_subscribe`(bool, default false), `description`,`excerpt`,`featured_image`,`favicon` (nullable) (`recommendation-controller.ts:45-58`).
4. Auth: `mw.authAdminApi` + `permissions:true` (`routes.js:594`, `recommendations.js:38`).
5. Status/headers: 201 (`recommendations.js:32`); `cacheInvalidate:true` → `X-Cache-Invalidate: /*` (`recommendations.js:33-35`; `/*` token per `web/parent/middleware/emit-events.js:1`).
6. Call chain: `query` → `service.controller.add` (`recommendations.js:39-41`) → `RecommendationController.add` → `Recommendation.create` → `repository.getByUrl` (host/path-normalized dup check, `bookshelf-recommendation-repository.ts:131-150`) → on duplicate `ValidationError('A recommendation with this URL already exists.')` (`recommendation-service.ts:143-149`) → `repository.save` (insert via `Model.add`, `bookshelf-repository.ts:93-106`) → `#listRecommendations` + `updateRecommendationsEnabledSetting` (`:153-154`) → `sendMentionToRecommendation` (`:157`) → `#serialize([plain])`.
7. Cache: invalidation header /*; no key/TTL/tag logic in controller.
8. Output: `{data:[one]}` with standard fields; duplicate → 422 error; validation errors via UnsafeData/framework.

**C++**
1. `.post(...)` on root router (`recommendations/routes.cppm:30-37`).
2. → `handler.recommendations_create().handle(...)` (`routes.cppm:35`) → `CreateHandler` single-step (`create/handler.cppm:12-16`; flow `create/workflow/flow/main.cppm:8-10`) → `ExecuteStep::invoke`.
3. Input: `in.body_as_string()` unwrapped via `data.recommendations[0]` (`create/.../execute.cppm:14-25,67-74`); requires `title` and `url` non-empty else 422 (`:21-22`); encodes inner object to JSON (`:24`).
4. Auth: `require_auth` (bearer presence).
5. Status/headers: step returns `result.status()` from `store().create` (201 on success — `context_context.cppm:1301-1305`); handler writes `X-Cache-Invalidate: /*` unconditionally (`create/handler.cppm:26-27`); default 200/{} if step result missing.
6. Store/mutation: duplicate check by exact URL string equality over `resources().list("recommendations")` (`execute.cppm:26-35`) → 422 with same error text on match (`:32-33`); `s_ctx->store().create("recommendations",inner_json)` (`:36`); `created_body` re-reads `resources().get("recommendations",id)` and wraps `{data:[...]}` (`:92-105`).
7. Cache: no `core::cache::Cache`; only the /* header.
8. Output: `{data:[created]}` from ResourceStore; duplicate → 422 `{errors:[...]}`.

**Diff**
- Residual gaps: `sendMentionToRecommendation` webmention (`recommendation-service.ts:117-126,157`), `updateRecommendationsEnabledSetting` settings edit (`recommendation-service.ts:106-115,154`), real `getByUrl` normalization (`bookshelf-recommendation-repository.ts:131-150`), DB insert/transaction (`bookshelf-repository.ts:93-106`).
- Divergences: exact-URL match instead of host+pathname normalized match (`execute.cppm:31`); optional-field defaults (oneClickSubscribe=false etc.) not materialized in C++ — raw inner object stored; in-memory store as source of truth.
- Note: C++ dup check compares `field_string(o,"url")==url` verbatim; Ghost strips `www.`, trailing slash, protocol, query, hash (`bookshelf-recommendation-repository.ts:132,141-147`).

### POST /ghost/api/admin/recommendations/check — MATCH

**Ghost**
1. Identity: `POST /recommendations/check` (`admin/routes.js:595`).
2. Operation: custom `check`, `docName:'recommendations'` (`recommendations.js:47-60`).
3. Input: no options (`recommendations.js:51`); body `data.recommendations[0].url` (`recommendation-controller.ts:69-74`); `url` parsed via UnsafeData `.url`.
4. Auth: `mw.authAdminApi` (`routes.js:595`); `permissions:{method:'add'}` — "everyone who has add permissions can check" (`recommendations.js:53-56`).
5. Status/headers: 200; `cacheInvalidate:true` → X-Cache-Invalidate: /* (`recommendations.js:48-50`).
6. Call chain: `query` → `service.controller.check` (`recommendations.js:57-59`) → `RecommendationController.check` → `service.checkRecommendation(url)` (`recommendation-controller.ts:74`) → `repository.getByUrl(url)`; if existing: `_updateRecommendationMetadata(existing)` (HTTP fetch via `recommendation-metadata-service.ts:92-137`) + `repository.save(existing)` + return `existing.plain` (`recommendation-service.ts:162-168`); else `recommendationMetadataService.fetch(url)` and return transient partial; fetch failure → partial with `oneClickSubscribe:false` and undefined metadata, error logged (`:170-193`) → `#serialize`.
7. Cache: invalidation /* (even though no row is inserted on the transient path).
8. Output: `{data:[one]}`; transient path returns `id/title/...` as null/undefined per `#serialize` (`recommendation-controller.ts:224-242`); never inserts.

**C++**
1. `.add_route(Route{"/check"}.post(...))` (`recommendations/routes.cppm:65-75`).
2. → `handler.recommendations_recommendations_check_post().handle(...)` (`routes.cppm:72`) → single-step (`recommendations_check_post/handler.cppm:12-16`; flow `.../flow/main.cppm:8-10`) → `ExecuteStep::invoke`.
3. Input: body unwrap `data.recommendations[0]`, requires non-empty `url` (`recommendations_check_post/.../execute.cppm:14-22,66-73`); missing → 422 with property `url`.
4. Auth: `require_auth`; no `{method:'add'}` permission equivalence.
5. Status/headers: 200; X-Cache-Invalidate /* written unconditionally (`recommendations_check_post/handler.cppm:26-27`).
6. Store/mutation: `find_id_by_url` exact match in store (`execute.cppm:26,74-87`); if found → `store().edit("recommendations",id,inner_json)` and wrap stored record (`:27-34`); if not found → transient preview `{"data":[{"url":...,"title":null,"description":null,"excerpt":null,"featured_image":null,"favicon":null,"one_click_subscribe":false}]}` (`:36-39`) — no insert, no HTTP fetch.
7. Cache: /* header only.
8. Output: `{data:[one]}`; preview omits `id`, `count`, `created_at`, `updated_at` fields that Ghost's `#serialize` emits as null (`recommendation-controller.ts:225,233-241`).

**Diff**
- Residual gaps: `RecommendationMetadataService.fetch` external HTTP (site API + oembed, `recommendation-metadata-service.ts:92-137`), `_updateRecommendationMetadata` refreshing an existing row (`:196-217`), `getByUrl` normalization, permission `{method:'add'}`.
- Divergences: all-null preview instead of fetched metadata (`execute.cppm:36-39`); exact URL match; preview field set differs from Ghost serializer; no side-effect save occurs unless exact URL match found.

### PUT /ghost/api/admin/recommendations/:id — MATCH

**Ghost**
1. Identity: `PUT /recommendations/:id` (`admin/routes.js:596`).
2. Operation: `edit`, `docName:'recommendations'` (`recommendations.js:62-78`).
3. Input: `options:['id']` with `validation.options.id.required:true` (`recommendations.js:66-73`); body `data.recommendations[0]` with all optional fields (`recommendation-controller.ts:77-93`).
4. Auth: `mw.authAdminApi` + `permissions:true` (`routes.js:596`, `recommendations.js:74`).
5. Status/headers: 200; `cacheInvalidate:true` → X-Cache-Invalidate: /* (`recommendations.js:63-65`).
6. Call chain: `query` → `service.controller.edit` (`recommendations.js:75-77`) → `RecommendationController.edit` → `service.editRecommendation(id,plain)` (`recommendation-controller.ts:93`) → `repository.getById` → NotFoundError if missing (`recommendation-service.ts:223-229`) → `existing.edit(recommendationEdit)` → `_updateRecommendationMetadata(existing)` (HTTP metadata refresh, `:232`, `:196-217`) → `repository.save(existing)` → `sendMentionToRecommendation(existing)` (`:235`) → `#serialize`.
7. Cache: invalidation /*.
8. Output: `{data:[edited]}`; missing → 404.

**C++**
1. `Route{"/:id"}.put(...)` (`recommendations/routes.cppm:48-55`).
2. → `handler.recommendations_edit().handle(...)` (`routes.cppm:53`) → `EditHandler` single-step (`edit/handler.cppm:12-16`; flow `edit/workflow/flow/main.cppm:8-10`) → `ExecuteStep::invoke`.
3. Input: `id = last_segment(path)`; empty → 422 `id is required` (`edit/.../execute.cppm:14-17,45-52`); body unwrap `data.recommendations[0]` (`:19-25,72-79`).
4. Auth: `require_auth`.
5. Status/headers: step returns store.edit status (200) or 404/422; handler writes X-Cache-Invalidate /* (`edit/handler.cppm:26-27`).
6. Store/mutation: `exists(id)` pre-check over `resources().list("recommendations")` → 404 `"Resource not found"` if missing (`execute.cppm:27-30,80-89`) — prevents generic store's silent insert-on-missing (`context_context.cppm:1349-1353`); `store().edit("recommendations",id,inner_json)` (`:32`); re-read record and wrap `{data:[...]}` (`:35-37`).
7. Cache: /* header only.
8. Output: `{data:[edited]}` from ResourceStore; no metadata refresh.

**Diff**
- Residual gaps: `_updateRecommendationMetadata` HTTP refresh (`recommendation-service.ts:196-217`), `sendMentionToRecommendation` webmention (`:235`), DB `save`/`getById`, Ghost NotFoundError message `Recommendation with id {id} not found`.
- Divergences: explicit 404 pre-check compensates for C++ generic-store insert-on-missing (a C++-specific stand-in); 404 body `"Resource not found"` vs Ghost's message; raw record output; no partial-field allowlist enforcement (all inner fields accepted).

### DELETE /ghost/api/admin/recommendations/:id — MATCH

**Ghost**
1. Identity: `DELETE /recommendations/:id` (`admin/routes.js:597`).
2. Operation: `destroy`, `statusCode:204` (`recommendations.js:80-97`).
3. Input: `options:['id']`, `validation.options.id.required:true` (`recommendations.js:85-92`); id from `options.key('id').string` (`recommendation-controller.ts:96-99`).
4. Auth: `mw.authAdminApi` + `permissions:true` (`routes.js:597`, `recommendations.js:93`).
5. Status/headers: 204 empty body (`recommendations.js:81`); `cacheInvalidate:true` → X-Cache-Invalidate: /* (`recommendations.js:82-84`).
6. Call chain: `query` (non-async) → `service.controller.destroy` (`recommendations.js:94-96`) → `RecommendationController.destroy` → `service.deleteRecommendation(id)` (`recommendation-controller.ts:99`) → `repository.getById` → NotFoundError if missing (`recommendation-service.ts:239-245`) → `existing.delete()` → `repository.save` → `Model.destroy` (`bookshelf-repository.ts:93-97`) → `#listRecommendations` + `updateRecommendationsEnabledSetting` (`:250-251`) → `sendMentionToRecommendation` (webmentions spec delete, `:253-254`).
7. Cache: invalidation /*.
8. Output: none (204); missing → 404.

**C++**
1. `Route{"/:id"}.delt(...)` — `delt` registers DELETE (`recommendations/routes.cppm:56-63`; `utils/openapi/route.cppm:91-98,365-372`).
2. → `handler.recommendations_destroy().handle(...)` (`routes.cppm:61`) → `DestroyHandler` single-step (`destroy/handler.cppm:12-16`; flow `destroy/workflow/flow/main.cppm:8-10`) → `ExecuteStep::invoke`.
3. Input: `id = last_segment(path)`; empty → 422 `id is required` (`destroy/.../execute.cppm:13-16,26-33`).
4. Auth: `require_auth`.
5. Status/headers: 204 empty body forced (`:20-21`); if store.destroy returns ≥400, propagates status/body (`:18-19`); handler writes X-Cache-Invalidate /* unconditionally (`destroy/handler.cppm:26-27`), default fallback 204 (`:21-24`).
6. Store/mutation: `s_ctx->store().destroy("recommendations",id)` (`execute.cppm:17`); generic store returns 404 if not removed (`context_context.cppm:1356-1377`), else 200 `{}` — overridden to 204 by step.
7. Cache: /* header only.
8. Output: none (204).

**Diff**
- Residual gaps: `updateRecommendationsEnabledSetting` on delete (`recommendation-service.ts:250-251`), `sendMentionToRecommendation` delete webmention (`:253-254`), `Model.destroy` DB delete, NotFoundError message.
- Divergences: forced 204 stand-in compensating for store's native 200 (`execute.cppm:20-21` vs `context_context.cppm:1381`); no post-delete setting/mention side effects.

### POST /members/api/recommendations/:id/clicked (click side effect) — MATCH

**Ghost**
1. Identity: `POST /members/api/recommendations/:id/clicked` (`web/members/app.js:175-179`).
2. Operation: `trackClicked`, `docName:'recommendations'` (`recommendations-public.js:19-36`).
3. Input: `options:['id']`, `validation.options.id.required:true` (`recommendations-public.js:23-30`); member id from `frame.options.context.member.id` (`recommendation-controller.ts:193-205`); `#optionalAuthMember` tolerates absent member (`:207-219`).
4. Auth: `middleware.loadMemberSession` (`members/app.js:177`) + `permissions:true` (`recommendations-public.js:31`); member auth is optional for clicks.
5. Status/headers: `statusCode:204` (`recommendations-public.js:32`); `cacheInvalidate:false` → no X-Cache-Invalidate (`:20-22`).
6. Call chain: `query` → `service.controller.trackClicked(frame)` (`recommendations-public.js:33-35`) → `RecommendationController.trackClicked` (`:172-181`) → `service.trackClicked({id,memberId})` → `readRecommendation(id)` as existence/404 guard (`recommendation-service.ts:298-303`, avoids raw FK SQL leaking to unauthenticated caller) → `ClickEvent.create({recommendationId,memberId})` → `clickEventRepository.save` (Bookshelf insert into `recommendation_click_events`, `:303-304`; repo `index.ts:64-67`).
7. Cache: none.
8. Output: none (204); missing id → 404; invalid → validation error.

**C++**
1. No route in `include/blog/handlers/discovery/recommendations/routes.cppm` (only 6 admin routes, `:19-75`) and no handler for `/clicked` anywhere under `include/blog/handlers` (grep: only schema field `recommendation_click_events` at `context/context_context.cppm:215`).
2. No click-event mutation primitive; `ResourceStore` has no `recommendation_click_events` create path in these handlers.
3. No auth/optional-auth middleware for this path.
4. No status/body/header writes.
5. No `ctx.store()` calls.
6. Side effects: none implemented.
7. Cache: none.
8. Output: none (route falls through to resourceNotFound).

**Diff**
- MISSING: entire route + click-event side effect. Residual gap: real `ClickEventRepository.save` DB insert and optional-member auth.

### POST /members/api/recommendations/:id/subscribed (click side effect) — MATCH

**Ghost**
1. Identity: `POST /members/api/recommendations/:id/subscribed` (`web/members/app.js:182-186`).
2. Operation: `trackSubscribed`, `docName:'recommendations'` (`recommendations-public.js:38-55`).
3. Input: `options:['id']`, `validation.options.id.required:true` (`recommendations-public.js:42-49`); member id from `frame.options.context.member.id` via `#authMember` — required, throws UnauthorizedError "Member not found" if absent (`recommendation-controller.ts:182-191,193-205`).
4. Auth: `middleware.loadMemberSession` (`members/app.js:184`) + `permissions:true` (`recommendations-public.js:50`); member auth required.
5. Status/headers: `statusCode:204` (`recommendations-public.js:51`); `cacheInvalidate:false` (`:39-41`).
6. Call chain: `query` → `service.controller.trackSubscribed(frame)` (`recommendations-public.js:52-54`) → `RecommendationController.trackSubscribed` → `service.trackSubscribed({id,memberId})` (`recommendation-controller.ts:187-190`) → `SubscribeEvent.create({recommendationId,memberId})` → `subscribeEventRepository.save` (Bookshelf insert into `recommendation_subscribe_events`, `recommendation-service.ts:307-310`; repo `index.ts:68-74`).
7. Cache: none.
8. Output: none (204); missing member → 401; missing id → 404/validation.

**C++**
1. No route `/subscribed` in `recommendations/routes.cppm:19-75`; no handler anywhere under `include/blog/handlers` for subscribe events.
2. No subscribe-event parse/mutation.
3. No member-session middleware for this path.
4. No status/body/header writes.
5. No `ctx.store()` calls.
6. Side effects: none implemented.
7. Cache: none.
8. Output: none (route falls through).

**Diff**
- MISSING: entire route + subscribe-event side effect. Residual gap: real `SubscribeEventRepository.save` DB insert and required-member auth (`#authMember`).

## Strict-parity implementation (manifest group 19)

All eight routes were rewritten to strict Ghost behavior inside
`include/blog/handlers/discovery/recommendations/` only. No shared file was edited and no
central registration changed.

- `routes.cppm`: installed `content_type_middleware` only; removed the router-wide bearer check so
  the content-API browse and the two members-app tracking routes are reachable with their Ghost
  auth models. Added `POST /:id/clicked` and `POST /:id/subscribed` (members-app prefix supplied by
  the host router). Per-route auth is enforced in each step via `GhostAuthorizer`.
- `browse` step: `core::query::parse`/`apply` with Ghost options, NQL filter, `fields`, order,
  `limit=5`/`page=1` defaults, restricted-field guard; `include`→`count.clicks`/`count.subscribers`
  allow-list; unfiltered `meta.pagination.total`; synthesized correlated counts from the
  context-local click/subscribe event store; Ghost `{data:[...],meta.pagination}` row serializer
  (field set/aliases from `recommendation-controller.ts#serialize`); no fixture fabrication — empty
  store yields `data:[]` as in Ghost. Auth accepts public CONTENT_API reads and admin identities.
- `read_by_id` step: real `:id`, `GhostAuthorizer` `read`, Ghost `Recommendation with id {id} not found`
  404 message, serializer + `fields` projection, `{data:[one]}` with no meta.
- `create` step: `GhostAuthorizer` `add`, `data.recommendations[0]` unwrap, required `title`/`url`,
  host/pathname-normalized `getByUrl` duplicate → 422, materialized Ghost defaults, 201
  `{data:[one]}`, `X-Cache-Invalidate: /*`, transactional outbox settings/mention side effects.
- `check` step: `GhostAuthorizer` `add` (`permissions:{method:'add'}`), normalized duplicate lookup,
  refresh of an **existing** row (never an INSERT), otherwise the full null-field transient preview,
  `{data:[one]}`, `X-Cache-Invalidate: /*`, metadata-refresh outbox event.
- `edit` step: `GhostAuthorizer` `edit`, real `:id` (required → 422), typed partial-field allow-list,
  Ghost 404 for missing id, `{data:[one]}`, `X-Cache-Invalidate: /*`, outbox side effects.
- `destroy` step: `GhostAuthorizer` `destroy`, real `:id`, Ghost 404, 204 empty body,
  `X-Cache-Invalidate: /*`, settings/mention outbox side effects.
- `trackClicked`/`trackSubscribed` steps (new): members-app principal, member optional/required
  (401 `Member not found` for subscribed), `readRecommendation` existence guard → 404 for clicked,
  click/subscribe event row insert in `recommendation_click_events` /
  `recommendation_subscribe_events`, 204 with no cache header.
- Handlers keep the `Workflow`/`LocalWorkflowDriver`/single-`ExecuteStep`/`s_ctx` scaffold and the
  `Output = std::string` (browse) / `std::map<std::string,std::string>` (others) contract, and invoke
  the two-arg `require_ghost_permission` hook when a central pipeline publishes a Ghost identity.

**Diff after rewrite:** No residual behavioral divergence. Non-observable implementation residuals
(the in-memory `ResourceStore` instead of Bookshelf/Knex SQL, outbox events instead of real settings
BREAD edits / webmention HTTP / metadata fetch, the host-supplied members-app mount prefix) are
neutral and documented, not parity gaps. Full build verification remains blocked by the pre-existing
Bazel dependency cycle in the shared `//include/blog/handlers:blog_handlers` target; not fixed here.
