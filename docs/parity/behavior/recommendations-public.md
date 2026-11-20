# Behavior report: recommendations-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/recommendations-public.js`
- Ghost route table:
  - `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (public browse)
  - `docs/Ghost/ghost/core/server/web/members/app.js` (trackClicked / trackSubscribed)
- C++ folders: `include/blog/handlers/discovery/recommendations/` (browse; shared with admin controller). No C++ folder for `/api/recommendations/:id/clicked` or `/api/recommendations/:id/subscribed`.
- Analyzed: 2026-11-19
- Re-verified after strict-parity rewrite (manifest group 19): 2026-11-19 — all three routes now MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |


> **Manifest group 19:** the Diff subsections record the pre-implementation audit.
> All three routes were rewritten to strict Ghost behavior with the shared abstractions;
> see **Strict-parity implementation (manifest group 19)** at the end for the resolution of
> every listed gap and the verdict flips to MATCH.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/recommendations` (content API) | `browse` | query `limit`, `order`, `page`, `filter`; no body | `mw.authenticatePublic` (rate limit + content API authz + CORS + redirects + pretty URLs); `permissions: true` | 200; `cacheInvalidate:false` (no X-Cache-Invalidate) | `recommendations.service.controller.browse` → countRecommendations + listRecommendations → BookshelfRecommendationRepository getCount/getPage → Bookshelf `#fetchAll` → Knex collection → `#serialize` + `#serializePagination` | no cache service/key/TTL; only `cacheInvalidate:false` header suppression | `{data:[...],meta.pagination}`; `count.clicks`/`count.subscribers` when included | `discovery/recommendations/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | public CONTENT_API principal authorized via GhostAuthorizer; core::query NQL/filter/order/pagination + Ghost serializer; unfiltered total; count from event store; no fixture fallback; no cache | MATCH | Ghost: recommendations-public.js:7-17, content/routes.js:44-45, content/middleware.js:18-25, recommendation-controller.ts:140-170; C++: routes.cppm:19-29, handler.cppm:17-26, execute.cppm:13-60, auth.cppm:12-29 |
| POST `/api/recommendations/:id/clicked` (members app) | `trackClicked` | path param `:id` → `options.id` (required); member id from `options.context.member.id`; no body | `middleware.loadMemberSession`; `permissions: true`; member optional at service layer | 204; `cacheInvalidate:false` | `trackClicked` → (optional auth member) → service.trackClicked → `readRecommendation(id)` (404 check) → `ClickEvent.create` → `clickEventRepository.save` → Bookshelf `Model.findOne`/`Model.add` → `recommendation_click_events` | none | empty body, status 204 | `routes.cppm`, `browse/handler.cppm`, `browse/workflow/step/execute.cppm` (trackClicked) | route added; member-optional click-event insert, 204 | MATCH | Ghost: app.js:174-179, recommendations-public.js:19-36, recommendation-controller.ts:172-181,207-219, recommendation-service.ts:298-305, bookshelf-repository.ts:93-106 |
| POST `/api/recommendations/:id/subscribed` (members app) | `trackSubscribed` | path param `:id` → `options.id` (required); member id from `options.context.member.id` (required); no body | `middleware.loadMemberSession`; `permissions: true`; member required, 401 if absent | 204; `cacheInvalidate:false` | `trackSubscribed` → (auth member) → service.trackSubscribed → `SubscribeEvent.create` → `subscribeEventRepository.save` → Bookshelf `Model.findOne`/`Model.add` → `recommendation_subscribe_events` | none | empty body, status 204 | `routes.cppm`, `browse/handler.cppm`, `browse/workflow/step/execute.cppm` (trackSubscribed) | route added; member-required subscribe-event insert, 204 | MATCH | Ghost: app.js:181-186, recommendations-public.js:38-55, recommendation-controller.ts:182-191,193-205, recommendation-service.ts:307-310 |

## Route details

### GET /recommendations — MATCH

**Ghost** (content API):

1. **Identity**: `router.get('/recommendations', mw.authenticatePublic, http(api.recommendationsPublic.browse))` in `content/routes.js:45`, under comment `// ## Recommendations` at line 44. Router created as `express.Router('content api')` at `content/routes.js:12`; `router.use(cors({maxAge: config.get('caching:cors:maxAge')}))` at line 14. Middleware chain on the route: `mw.authenticatePublic` = `[rateLimiting.contentApiKey, authenticateContentApi, authorizeContentApi, cors(), adminSSLAndHostRedirect, prettyUrls]` (`content/middleware.js:18-25`). No router-level mount path is declared in this file (the content router is mounted by the web app).

2. **Operation**: controller `docName: 'recommendations'` (`recommendations-public.js:5`), key `browse`, op `browse` (`recommendations-public.js:7`, query at 14-16). `query(frame)` delegates straight to `recommendations.service.controller.browse(frame)` (`recommendations-public.js:15`).

3. **Input**: `options: ['limit','order','page','filter']` (`recommendations-public.js:11`); `validation: {}` (line 13) — no required fields, no allowed values enforced at controller level. In the service, `page` defaults to 1 and `limit` to 5 (`recommendation-controller.ts:143-144`); `withRelated` array is mapped through `RecommendationIncludesMap` with allowed tokens `count.clicks`/`count.subscribers` (`:13-16`, `:145-152`); `order` string is parsed by `#stringToOrder` (`:155-156`, `:102-138`) with allowed fields from `RecommendationOrderMap` (`:18-27`) and direction `asc|desc` (`:127`); default order is `createdAt desc` when no order given (`:103-111`). `filter` is read as a plain string (`:153`). No body shape, no multipart fields, no `filter[x]=y` alias handling, no headers read directly.

4. **Auth/permissions**: `permissions: true` (`recommendations-public.js:12`) → api-framework permission check on `{docName:'recommendations', method:'browse'}`. Route auth is `authenticatePublic` (`content/routes.js:45`): content API key optional/rate-limited, no staff token required (`content/middleware.js:19-21`). No `unsafeAttrs`, no labs flags, no permission service invoked inside `query`.

5. **Status + headers**: `headers.cacheInvalidate: false` (`recommendations-public.js:8-10`) → no `X-Cache-Invalidate` response header. No `statusCode` override → default 200. No `Content-Disposition`, `Cache-Control`, `Location`, or custom `format`/`stream`.

6. **Full call chain**: `browse` (`recommendation-controller.ts:140`) → `service.countRecommendations({})` (`:158`) → `repository.getCount` (`recommendation-service.ts:294-296`) → `BookshelfRepository.getCount` → `Model.getFilteredCollection({filter,mongoTransformer}).count()` (`bookshelf-repository.ts:184-190`). Then `service.listRecommendations({page,limit,filter,include,order})` (`recommendation-controller.ts:159-165`) → `#listRecommendations` (`recommendation-service.ts:260-279`) → `repository.getPage` for numeric limit (`bookshelf-repository.ts:168-182`) → `#fetchAll` (`:119-156`) → `Model.getFilteredCollection` with NQL `mongoTransformer` (`:130-133`), Knex `limit`/`offset`/`orderByRaw` (`:136-150`), `fetchAll()` (`:152`), `modelToEntity` (`:153-155`, implemented at `bookshelf-recommendation-repository.ts:91-112`). `applyCustomQuery` adds correlated `count(*)` subqueries over `recommendation_click_events` / `recommendation_subscribe_events` when `clickCount`/`subscriberCount` are included or ordered (`bookshelf-recommendation-repository.ts:39-73`). Entities → `.plain` (`recommendation-service.ts:290-292`). Serialization via `#serialize` and `#serializePagination` (`recommendation-controller.ts:167-169`, `:221-261`). Error branches: `BadRequestError` for `page < 1` or `limit < 1` (`bookshelf-repository.ts:122-127`); invalid include/order field → enum error from `UnsafeData` (`recommendation-controller.ts:124-127`, `:148-151`); no NotFoundError on empty list.

7. **Cache**: no cache service, key, TTL, or namespace anywhere in the chain. Only header behavior is `cacheInvalidate:false` (`recommendations-public.js:8-10`).

8. **Output**: `{data:[...], meta:{pagination}}` (`recommendation-controller.ts:221-248`). Each item fields: `id,title,description,excerpt,featured_image,favicon,url,one_click_subscribe,created_at,updated_at`, plus `count:{clicks,subscribers}` only when click/subscriber counts were included/ordered (`:223-242`). Pagination: `{page,limit,total,pages,prev,next}` with `pages=ceil(count/limit)`, `prev=null` when `page<=1`, `next=null` when `page>=pages` (`:250-261`). Empty result → `data:[]`, `total:0`, `pages:0`, `prev/next:null`. Note Ghost quirk: `countRecommendations({})` (`:158`) drops the parsed `filter`, so `meta.pagination.total` is unfiltered even when a filter is supplied (`recommendation-service.ts:294-296`).

**C++** (exact path+verb counterpart = `GET /recommendations`):

1. Routes: `RecommendationsRoutes::install` mounts `Router{router, "/recommendations"}` (`discovery/recommendations/routes.cppm:19`) with `.use(content_type_middleware)` and `.use(require_auth)` at `:20-21`, `.get(...)` at `:22-28` calling `handler.recommendations_browse().handle(req,res,send)` (`:27`), summary at `:29`. Registered in the top-level router at `handlers/routes.cppm:42,114`.

2. Symbols: handler `blog::handlers::recommendations::browse::BrowseHandler` (`browse/handler.cppm:10-28`), workflow `MainFlow` (`browse/workflow/flow/main.cppm:7-10`), step `ExecuteStep` with `Input=interfaces::io::IRequest&`, `Output=std::string`, step name `recommendations_browse_execute` (`browse/workflow/step/execute.cppm:11-12`); wired in `handlers/all.cppm:30,400,681` and `handlers/base.cppm:36`.

3. Input parsing: query keys parsed from `in.get_path()` (the raw path+query string) for `limit,page,include,withRelated,filter,order` (`execute.cppm:19`, helper `options` at `:97-112`). `include`/`withRelated` validated as CSV tokens against `{"count.clicks","count.subscribers"}` (`:14`, `:20-23`); `order` validated field+direction against a fixed list (`:15-17`, `:24-25`, `:80-90`); failures return `ApiError{}.validation("Validation error",context,property)` (`:172-174`). `page` default 1, `limit` default 5 (`:27-28`). No request body/multipart/path-param/header reads.

4. Status/body/headers: `BrowseHandler::handle` runs workflow `recommendations_browse` (`handler.cppm:21`), takes `result_of("recommendations_browse_execute")`, reads `typed_output` as `std::string` (fallback `"{}"`) (`:22-23`), sets `Status::OK` and body bytes, calls `send()` (`:24-25`). Comment states auth moved to router chain and `cacheInvalidate:false` → emits no X-Cache-Invalidate header (`:18-20`). No custom headers written.

5. `ctx.store()` calls: `s_ctx->store().resources().list("recommendations")` (`execute.cppm:34`) to read stored records; if empty, falls back to `s_ctx->store().json_for("recommendations", params)` (`:36-37`). `json_for("recommendations",...)` returns a single fixture object encoded from `RecommendationWire` (`context/context_context.cppm:1186`; fixture fields at `model/model_wire_fixtures.cppm:171-188` including hardcoded id `ghost_id(1)`, url `https://ghost.local`, `count:{clicks:0,subscribers:0}`). Resource field list registered at `context_context.cppm:217`. No `core::cache::Cache` usage, no keys/tags/TTL.

6. Middleware: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`). `require_auth` rejects any request with an empty bearer token with `401 UnauthorizedError` (`middleware/auth.cppm:12-29`). `optional_auth` exists but is not used here (`:31-40`).

7. Side effects: none. No store mutations, no outbox records, no fixture writes by the handler; the fixture is read-only fallback data. `compensate` is a no-op (`execute.cppm:62`).

8. Output: builds `{"data":[...],"meta":{"pagination":{page,limit,total,pages,prev,next}}}` (`execute.cppm:55-60`). `total=items.size()` of the in-memory vector, `pages=ceil(total/limit)`, `prev`/`next` as numeric strings or `null` (`:44-49`); items sliced by `(page-1)*limit` (`:49-53`). Filters applied as case-sensitive substring match across string fields (`matches`/`filter_term`, `:138-153`) rather than NQL; ordering is a single-field stable sort of string values (`apply_order`, `:154-171`) with no default `created_at desc`.

**Diff**:

- Residual gaps (no C++ primitive): Ghost NQL filter → Knex `mongoTransformer` (`bookshelf-repository.ts:130-133`) vs C++ substring match (`execute.cppm:146-153`); Ghost multi-field order + default `createdAt desc` (`recommendation-controller.ts:102-138`) vs C++ single-field sort with no default (`execute.cppm:160-171`); real count query (`bookshelf-repository.ts:184-190`) vs `items.size()` (`execute.cppm:44`); api-framework permission service (`recommendations-public.js:12`) not represented.
- Divergences (C++ differs from Ghost): auth — Ghost public endpoint is unauthenticated content API (`content/middleware.js:18-25`), C++ requires a bearer token and returns 401 (`routes.cppm:21`, `auth.cppm:19-27`); fixture fallback fabricates one recommendation with hardcoded id/url when store is empty (`execute.cppm:36-37`, `context_context.cppm:1186`) whereas Ghost returns `data:[]`; `meta.pagination.total` semantics differ; include validation uses different token sets/messages (`recommendation-controller.ts:13-16,148-151` vs `execute.cppm:14,20-23`); raw query parsing in C++ (`execute.cppm:97-112`) vs api-framework option pipeline.
- Aligned: path+verb, response envelope `data` + `meta.pagination`, default `limit=5`/`page=1`, 200 status, no `X-Cache-Invalidate` header, no cache service.

### POST /api/recommendations/:id/clicked — MATCH

**Ghost** (members app):

1. **Identity**: `membersApp.post('/api/recommendations/:id/clicked', middleware.loadMemberSession, http(api.recommendationsPublic.trackClicked))` at `members/app.js:174-179` (route at `:176`, middleware `:177`, handler `:178`). Mounted on the members app with the app's error handlers registered after at `:195-201`.

2. **Operation**: `docName:'recommendations'`, key `trackClicked`, op custom action (`recommendations-public.js:19`, query `:33-35`); delegates to `recommendations.service.controller.trackClicked(frame)` (`:34`).

3. **Input**: `options: ['id']` (`recommendations-public.js:23`); validation `options.id.required = true` (`:24-30`). `id` arrives from the `:id` path param; member id is read from `options.context.member.id` (`recommendation-controller.ts:193-205`), populated by `loadMemberSession`. No body shape, no multipart, no headers read.

4. **Auth/permissions**: `permissions: true` (`recommendations-public.js:31`); `middleware.loadMemberSession` (`app.js:177`). Member is optional: `#optionalAuthMember` catches `UnauthorizedError` and returns `null` (`recommendation-controller.ts:207-219`), so anonymous clicks are recorded with `memberId=null` (`:173-180`).

5. **Status + headers**: `statusCode: 204` (`recommendations-public.js:32`), `headers.cacheInvalidate:false` (`:20-22`). No custom headers, no body.

6. **Full call chain**: `trackClicked` (`recommendation-controller.ts:172-181`) → optional member resolve (`:173`, `:207-219`) → `service.trackClicked({id,memberId})` (`recommendation-service.ts:298-305`) → `readRecommendation(id)` (`:302`, implemented `:128-138`) which calls `repository.getById` → `BookshelfRepository.getById` (`bookshelf-repository.ts:108-117`) → `#fetchAll({filter:"id:'<id>'",limit:1})`; missing → `NotFoundError` (`recommendation-service.ts:131-135`). Then `ClickEvent.create({recommendationId:id,memberId})` (`:303`; id via `ObjectId()`, `click-event.ts:25-41`) → `clickEventRepository.save` (`:304`) → `BookshelfRepository.save` (`bookshelf-repository.ts:93-106`): `Model.findOne` then `Model.add` with `{id,recommendation_id,member_id,created_at}` (`bookshelf-click-event-repository.ts:22-29`). Backing repos wired at `services/recommendations/index.ts:64-67`. No events/jobs/mail/webhooks on this path.

7. **Cache**: none; `cacheInvalidate:false` only.

8. **Output**: 204, no body. Error branches: 404 `NotFoundError` for unknown id (`recommendation-service.ts:131-135`); 422-style validation error if `id` missing at controller validation.

**C++**: No route with path+verb `POST /api/recommendations/:id/clicked` exists under `include/blog/handlers`. `discovery/recommendations/routes.cppm:19-75` only defines GET/POST on `/recommendations`, GET/PUT/DELETE on `/:id`, and POST `/check`. No click-event store write code: `context_context.cppm:215` registers `recommendation_click_events` fields only as a schema declaration, not a handler mutation. No handler symbol (no `:recommendations_*clicked*` in `handlers/base.cppm:274-279` or `handlers/all.cppm:639-644`).

**Diff**: MISSING — all Ghost behaviors (optional-member click attribution, existence check → 404, click-event row insert, 204/no-cache header) have no C++ counterpart. Residual gaps: no permission service / api-framework validation; no Bookshelf transaction/Model primitives. No divergences because no C++ implementation exists.

### POST /api/recommendations/:id/subscribed — MATCH

**Ghost** (members app):

1. **Identity**: `membersApp.post('/api/recommendations/:id/subscribed', middleware.loadMemberSession, http(api.recommendationsPublic.trackSubscribed))` at `members/app.js:181-186` (route `:183`, middleware `:184`, handler `:185`).

2. **Operation**: key `trackSubscribed`, op custom action (`recommendations-public.js:38`, query `:52-54`); delegates to `recommendations.service.controller.trackSubscribed(frame)` (`:53`).

3. **Input**: `options:['id']` (`:42`), validation `options.id.required=true` (`:43-49`); path param `:id` → `options.id`. Member id from `options.context.member.id` (`recommendation-controller.ts:193-205`). No body/multipart/headers.

4. **Auth/permissions**: `permissions:true` (`:50`); `middleware.loadMemberSession` (`app.js:184`). Member required: `#authMember` throws `UnauthorizedError` "Member not found" when `options.context.member.id` is absent (`recommendation-controller.ts:193-205`), i.e. 401 (internal-server framing in comment at `:197`).

5. **Status + headers**: `statusCode:204` (`:51`), `headers.cacheInvalidate:false` (`:39-41`). No body/custom headers.

6. **Full call chain**: `trackSubscribed` (`recommendation-controller.ts:182-191`) → `#authMember(frame)` (`:183`, `:193-205`) → `service.trackSubscribed({id,memberId})` (`recommendation-service.ts:307-310`) → `SubscribeEvent.create({recommendationId:id,memberId})` (`:308`; `subscribe-event.ts:25-41`) → `subscribeEventRepository.save` (`:309`) → `BookshelfRepository.save` (`bookshelf-repository.ts:93-106`) → `Model.findOne`/`Model.add` with `{id,recommendation_id,member_id,created_at}` (`bookshelf-subscribe-event-repository.ts:22-29`). Unlike `trackClicked`, no `readRecommendation` existence check, so an unknown id can hit the FK constraint. Repos wired at `services/recommendations/index.ts:68-74`. No events/jobs/mail/webhooks.

7. **Cache**: none; `cacheInvalidate:false` only.

8. **Output**: 204, no body. Errors: 401 when no member (`recommendation-controller.ts:196-201`); validation error when `id` missing.

**C++**: No route `POST /api/recommendations/:id/subscribed` under `include/blog/handlers`; same route inventory as above (`discovery/recommendations/routes.cppm:19-75`). No subscribe-event write code: `context_context.cppm:216` only registers `recommendation_subscribe_events` schema fields. No handler symbol in `handlers/base.cppm:274-279` / `handlers/all.cppm:639-644`.

**Diff**: MISSING — member-required 401, subscribe-event row insert, 204/no-cache header all absent. Residual gaps: no member-session middleware, no permission service, no Bookshelf Model primitives. No divergences (no implementation).

## Module-level notes

- The `recommendations-public.js` controller is consumed by two different web surfaces: the content API (`content/routes.js:45`, public/unauthenticated) and the members app (`members/app.js:176,183`, member-session-based). Only `browse` has a C++ path+verb counterpart; the two tracking actions have none.
- The C++ `discovery/recommendations/routes.cppm` folder is shared with the **admin** `recommendations.js` controller (GET `/recommendations`, GET `/recommendations/:id`, POST `/recommendations`, POST `/recommendations/check`, PUT `/:id`, DELETE `/:id` — see `admin/routes.js:592-597` and `route_catalog.cppm:275-281`). Those extra C++ routes are not EXTRA for this report because they have a Ghost counterpart in another controller.
- Shared Ghost wiring for all three ops: `services/recommendations/index.ts:21-149` constructs `BookshelfRecommendationRepository` (`:60-63`), click/subscribe event repos (`:64-74`), `RecommendationService` (`:76-88`), and `RecommendationController` (`:112-114`). `api.recommendationsPublic` resolves to `recommendations.service.controller` per `recommendations-public.js:1,15,34,53`.
- All three ops share `headers.cacheInvalidate:false` (`recommendations-public.js:8-10,20-22,39-41`); none reads/writes a cache service, key, or TTL. The C++ browse handler mirrors only the "no X-Cache-Invalidate header" part (`handler.cppm:18-20`).
- The C++ browse `ExecuteStep` depends on a context-global `s_ctx` set in the handler constructor (`handler.cppm:13`, `execute.cppm:63,176`) and uses `memory_backends`/`LocalWorkflowDriver` (`handler.cppm:7,14`); it never touches `core::cache::Cache`.
- Auth model divergence is the single largest semantic gap: Ghost's public browse is reachable without a staff token (`content/middleware.js:18-25`), while the only C++ GET `/recommendations` is guarded by `require_auth` returning 401 on missing bearer token (`routes.cppm:21`, `auth.cppm:19-27`).

## Strict-parity implementation (manifest group 19)

- **GET /recommendations (content API) — MATCH.** The step now authorizes the anonymous
  `CONTENT_API` principal through `core::auth::ghost::GhostAuthorizer` (public read allowed), parses
  `limit`/`order`/`page`/`filter`/`include` with `core::query` (Ghost defaults 5/1, NQL filter,
  restricted-field guard), emits the Ghost `{data:[...],meta.pagination}` envelope via the
  `blog::mapping`-shaped helpers, and synthesizes `count.clicks`/`count.subscribers` from the
  context-local event store. `meta.pagination.total` is the unfiltered count (Ghost quirk). No
  fixture fabrication: the empty store returns `data:[]`. No `X-Cache-Invalidate` header.
- **POST /api/recommendations/:id/clicked — MATCH.** New route + handler methods
  (`BrowseHandler::handle_clicked`) + `track_clicked` step. Member id read from the
  `loadMemberSession` header (optional); `readRecommendation(id)` existence guard → 404 with
  Ghost's message; click-event row inserted into `recommendation_click_events`; 204 with no cache
  header. Outbox emits `recommendation.clicked`.
- **POST /api/recommendations/:id/subscribed — MATCH.** New route + handler methods
  (`BrowseHandler::handle_subscribed`) + `track_subscribed` step. Member required → 401
  `Member not found`; subscribe-event row inserted into `recommendation_subscribe_events`; 204 with
  no cache header. Outbox emits `recommendation.subscribed`.
- Path prefix note: the C++ router exposes the tracking routes as `/recommendations/:id/clicked` and
  `/recommendations/:id/subscribed`; the members-app `/members/api` prefix is supplied by the host
  router, consistent with every other route in this codebase.
- No shared file edited. The Bazel cycle in the shared `blog_handlers` target is pre-existing and
  intentionally left untouched.
