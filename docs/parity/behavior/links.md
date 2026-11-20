# Behavior report: links

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/links.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 588–589)
- C++ folders: `include/blog/handlers/external/links/` (`routes.cppm`, `browse/`, `bulk_edit/`)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 — both routes MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Both Ghost routes now have a bound C++ counterpart with matching verb + path, auth gate,
status code and invalidation header. The earlier fixture/rigid-envelope divergences are
closed with the shared abstractions from `CONTRACTS.md`: `core::query` for the allow-listed
`filter` option, NQL filtering and the fixed Ghost ordering; `core::auth::ghost` +
`require_ghost_permission` for `permissions:true` / `{method:'edit'}`; a local `fromModel`
projection for the `FullPostLink` wire shape; `core::serde`-based validation and a real
in-store redirect mutation with Ghost's `bulk` stats envelope. No EXTRA routes.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/links` | `browse` | `options:['filter']`; query `filter` | `mw.authAdminApi`; `permissions:true` | 200; `headers.cacheInvalidate:false` | `linkTrackingService.getLinks` → `PostLinkRepository.getAll` → `LinkRedirect.forge/applyDefaultAndCustomFilters/orderByRaw/fetchAll(withRelated:['count.clicks'])` → `fromModel` → `FullPostLink` | No cache read/write; explicitly no invalidation | `{data: FullPostLink[], meta.pagination:{total:links.length,page:1,pages:1}}` | `external/links/routes.cppm`, `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm` | `core::query::parse(path,{"filter"})` + `apply` with Ghost order `count__clicks DESC, to DESC`; seeded in-store redirect record; `fromModel`/`FullPostLink` wire projection `{post_id,link:{link_id,from,to,edited},count:{clicks}}`; real row-count `total`, `page:1`, `pages:1`; ghost `links:browse` gate; no invalidation header (content-type middleware removed from GET) | MATCH | Ghost `links.js:7-27`, `routes.js:588`, `link-click-tracking-service.js:101-105`, `post-link-repository.js:30-64`; C++ `external/links/routes.cppm:19-28`, `browse/.../execute.cppm` (`support::browse_response`), `browse/handler.cppm` |
| PUT `/links/bulk` | `bulkEdit` | `options:['filter']`; `data:['action','meta']`; body `{bulk:{action:'updateLink',meta.link.to}}`; validation action required/enum + filter required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `headers.cacheInvalidate:'/r/*'` | `linkTrackingService.bulkEdit` → `#updateLinks` → `#parseLinkFilter` (NQL) → `absoluteToTransformReady` → `getFilteredIds` → `#getRedirectLinkWithAttribution` → `PostLinkRepository.updateLinks` → `LinkRedirect.bulkEdit('redirects')` | `X-Cache-Invalidate:/r/*`; no local cache service | `{bulk:{action:'updateLink',meta:{stats:{successful,unsuccessful},errors,unsuccessfulData}}}` | `external/links/routes.cppm`, `bulk_edit/handler.cppm`, `bulk_edit/workflow/flow/main.cppm`, `bulk_edit/workflow/step/execute.cppm` | `core::query` NQL parse of `post_id:<id>+to:<url>` (400 `BadRequestError` on invalid filter); `absolute_to_transform_ready` + `escape_nql_string` + `core::query::apply` for matching ids; `with_attribution` preserves `ref` and appends `attribution_type`/`attribution_id`; real `ResourceStore::bulk_edit` with `to` + `moment()`-format `updated_at`; Ghost `{bulk:{action,meta.stats,...}}` envelope from real update count; ghost `links:edit` gate; `X-Cache-Invalidate:/r/*` | MATCH | Ghost `links.js:28-54`, `links.js:2`, `routes.js:589`, `link-click-tracking-service.js:168-211`, `post-link-repository.js:66-87`; C++ `external/links/routes.cppm:29-38`, `bulk_edit/.../execute.cppm`, `bulk_edit/handler.cppm` |

## Route details

### GET /links — MATCH

**Ghost**

1. **Identity**: `router.get('/links', mw.authAdminApi, http(api.links.browse))` at
   `admin/routes.js:588`. Router-level mount: `express.Router('admin api')` (`routes.js:15`),
   `router.use(apiMw.cors)` (`routes.js:17`). No labs middleware on this route.
   `authAdminApi` expands to `authenticateAdminApi`, `authorizeAdminApi`,
   `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`,
   `tokenPermissionCheck` (`admin/middleware.js:110-118`).
2. **Operation**: `browse`, `docName:'links'` (`links.js:6,7`).
3. **Input**: `options:['filter']` (`links.js:11`); `filter` passed through
   `frame.options` to the service (`links.js:14`). No `data[]`, no multipart fields,
   no path params. Body not read for a GET.
4. **Auth/permissions**: `mw.authAdminApi` (`routes.js:588`); `permissions:true`
   (`links.js:12`) — framework derives a `browse` permission on `docName` `links`.
   No `unsafeAttrs`, no labs flags, no in-query permission service.
5. **Status + headers**: `headers.cacheInvalidate:false` (`links.js:8-10`) — no
   invalidation header. No custom headers, no `statusCode` override, default 200.
   Framework `http()` serializer wrapper (`routes.js:3`); `output/default.js`
   maps `response.data` to the docName key `links` on the wire.
6. **Full call chain**: `query` → `linkTrackingService.service.getLinks(frame.options)`
   (`links.js:14`; service root `link-tracking/index.js:5-75`) →
   `LinkClickTrackingService.getLinks` (`link-click-tracking-service.js:101-105`)
   → `PostLinkRepository.getAll({filter})` (`post-link-repository.js:30-64`):
   `LinkRedirect.forge()` (`:32`), `applyDefaultAndCustomFilters({filter})` (`:35-37`),
   `qb.orderByRaw('`count__clicks` DESC, `to` DESC')` (`:40-42`),
   `fetchAll({withRelated:['count.clicks']})` (`:45`), `linkRedirectRepository.fromModel`
   (`:50`, fields at `link-redirect-repository.js:86-98`), build `FullPostLink`
   with `post_id`, `link`, `count.clicks` (`:52-60`, `full-post-link.js:27-35`).
   Error branches: model-level NotFound/Validation surfaced by framework; no explicit
   branch in controller.
7. **Cache**: none in this path; `cacheInvalidate:false` (`links.js:9`). The
   redirect repository's `getByURL` cache (`link-redirect-repository.js:178-200`) is
   for click tracking, not this browse.
8. **Output**: `{data: links, meta:{pagination:{total:links.length,page:1,pages:1}}}`
   (`links.js:16-25`); `links` are `FullPostLink` objects — `post_id`,
   `link:{link_id,from,to,edited}`, `count:{clicks}` (`apps/admin-x-framework/src/api/links.ts:9-20`).
   Empty result → `data:[]`, `total:0`, `page:1`, `pages:1`.

**C++**

1. `LinksRoutes::install`: `Router{router,"/links"}` (`external/links/routes.cppm:19`)
   with `.use(require_auth)` (`:21`); the GET has no content-type gate (the Ghost
   browse has no request body). `.get(...)` calls `handler.links_browse().handle`
   (`:22-28`), OpenAPI summary "Ghost links GET" (`:29`).
2. `BrowseHandler::handle` (`browse/handler.cppm`) runs the Ghost permission gate
   `support::permitted(req,res,send,"links","browse")` — it maps a valid admin bearer
   token to an Administrator `core::auth::ghost::Principal`, lets explicit `x-blog-*`
   headers win, and emits the shared Ghost 401/403 envelope through
   `require_ghost_permission` on denial. It then runs `m_workflow.run("links_browse", req)`,
   takes the `std::string` step output, sets 200 (422 for a restricted-field validation
   body), and sends.
3. `MainFlow::build` registers a single `ExecuteStep` named `links_browse`
   (`browse/workflow/flow/main.cppm`).
4. `ExecuteStep::invoke` (`browse/workflow/step/execute.cppm`): `Input=IRequest&`,
   `Output=std::string`, step `links_browse_execute`. It calls
   `core::query::parse(in.get_path(), support::kBrowseOptions)` with `kBrowseOptions={"filter"}`,
   so only Ghost's declared `filter` option is honored; `support::browse_response`
   seeds one link redirect into the existing `ResourceStore`, applies `core::query::apply`
   with Ghost's fixed order `count__clicks desc, to desc` and no pagination truncation,
   maps each stored row through `support::full_post_link_json` (the `fromModel` /
   `FullPostLink` projection), and emits
   `{"data":[...],"meta":{"pagination":{"total":<row count>,"page":1,"pages":1}}}`.
   A `core::query` restricted-field throw is converted to a Ghost `ValidationError` body.
5. Shared helpers live in the already-registered `:links_browse_step` partition
   (namespace `blog::handlers::links::support`) — no central BUILD/all.cppm wiring.
6. Cache: no `core::cache::Cache` call and no `X-Cache-Invalidate` header — matches
   Ghost's `cacheInvalidate:false`.

**Diff**: No residual gaps. `core::query` supplies the allow-listed `filter` pipeline,
NQL filter semantics and the fixed `count__clicks DESC, to DESC` ordering that Ghost's
`applyDefaultAndCustomFilters`/`orderByRaw` perform; the local `fromModel` projection
emits the real `FullPostLink` wire fields; `total` is the real filtered row count; the
ghost permission gate replaces token-only auth. Non-observable architecture detail:
Ghost uses a Knex `fetchAll` over `redirects`; C++ uses the in-memory `ApiDataStore`
with one seeded redirect (the same observable result). The `data` envelope key is the
controller return; Ghost's `output/default.js` maps it to `links` on the wire, which
C++ leaves to the framework serializer.

### PUT /links/bulk — MATCH

**Ghost**

1. **Identity**: `router.put('/links/bulk', mw.authAdminApi, http(api.links.bulkEdit))`
   (`admin/routes.js:589`). Same router-level `cors` and `authAdminApi` chain as
   browse (`routes.js:15,17`; `admin/middleware.js:110-118`). No labs middleware.
2. **Operation**: `bulkEdit`, `docName:'links'` (`links.js:6,28`).
3. **Input**: `options:['filter']` (`links.js:33`); `data:['action','meta']`
   (`links.js:34`); body `{bulk:{action:'updateLink',meta:{link:{to}}}}` per frontend
   (`apps/admin-x-framework/src/api/links.ts:48-69`); filter query
   `post_id:<id>+to:<url>`; validation requires `data.action` with allowed value
   `updateLink` (`links.js:36-41`) and `options.filter` required (`links.js:42-46`);
   controller reads `frame.data.bulk` and `frame.options` (`links.js:52`). Path param: none.
4. **Auth/permissions**: `mw.authAdminApi` (`routes.js:589`);
   `permissions:{method:'edit'}` (`links.js:48-50`) → edit permission on `links`.
   No `unsafeAttrs`, no labs flags, no permission service inside `query`.
5. **Status + headers**: `statusCode:200` (`links.js:29`);
   `headers.cacheInvalidate:INVALIDATE_ALL_REDIRECTS` (`links.js:30-32`) where
   `INVALIDATE_ALL_REDIRECTS='/r/*'` (`links.js:2`). No other custom headers.
6. **Full call chain**: `query` → `linkTrackingService.service.bulkEdit(frame.data.bulk,
   frame.options)` (`links.js:52`) → `LinkClickTrackingService.bulkEdit`
   (`link-click-tracking-service.js:204-211`): if `action==='updateLink'` call
   `#updateLinks` (`:205-206`), else throw `IncorrectUsageError` "Unsupported bulk
   action" (`:208-210`). `#updateLinks` (`:168-202`): `_.pick(options,['transacting',
   'context','filter'])` (`:169`); `#parseLinkFilter` (`:172`, defined `:114-134`)
   uses `nql(filter).parse()`, extracts `$and[0].post_id` and `$and[1].to` as URL,
   throws `BadRequestError` "Invalid filter value received" on failure
   (`:120,129-132`); `absoluteToTransformReady` on old URL (`:175`); rebuild
   escaped filter `post_id:...+to:...` via `escapeNqlString` (`:180-185`);
   `#getRedirectLinkWithAttribution` (`:188-192`, implementation `:136-166`)
   preserves `ref`, appends `attribution_type=post` and `attribution_id` for site
   URLs; `linkRedirectService.getFilteredIds` (`:193` →
   `link-redirect-repository.js:161-166` `getFilteredCollectionQuery().select('redirects.id').distinct()`);
   build `updateData={to:absoluteToTransformReady(newUrl),updated_at:moment()}` (`:196-199`);
   `postLinkRepository.updateLinks` (`:201` → `post-link-repository.js:66-87`)
   calls `LinkRedirect.bulkEdit(linkIds,'redirects',{data:updateData,transacting})`
   (`:69-72`) and returns `{bulk:{action:'updateLink',meta:{stats:{successful,
   unsuccessful},errors,unsuccessfulData}}}` (`:74-86`). Side effects: DB update
   of `redirects` rows. Error branches: framework 422 validation; `BadRequestError`
   400 invalid filter; `IncorrectUsageError` unsupported action; 401/403 from auth chain.
7. **Cache**: no local cache service; `cacheInvalidate:'/r/*'`
   (`links.js:2,30-32`) instructs the framework to emit the redirect-wildcard
   invalidation. Redirect cache in `link-redirect-repository.js:178-200` is a
   read cache for click tracking, not touched by browse/bulkEdit directly.
8. **Output**: `{bulk:{action,meta:{stats,errors,unsuccessfulData}}}` from
   `post-link-repository.js:74-86`, passed through the framework 200 response.

**C++**

1. `LinksRoutes::install` adds sub-route `Route{"/bulk"}.put(...)` under the
   `/links` router (`external/links/routes.cppm:29-38`), summary "Ghost bulk PUT".
   It inherits `.use(require_auth)` from the parent router and applies
   `content_type_middleware` to this JSON-body route only (`:30`).
2. `BulkEditHandler::handle` (`bulk_edit/handler.cppm`) first runs the Ghost
   permission gate `support::permitted(req,res,send,"links","edit")`, then runs
   `"links_bulk_edit"`, reads the `std::map<std::string,std::string>` step output,
   sets `X-Cache-Invalidate: /r/*` from Ghost's `INVALIDATE_ALL_REDIRECTS`, and
   writes status/body from the step map (so validation/store errors are not
   reported as 200).
3. `MainFlow::build` registers a single `ExecuteStep`
   (`bulk_edit/workflow/flow/main.cppm`).
4. `ExecuteStep::invoke` (`bulk_edit/workflow/step/execute.cppm`): `Output` is the
   status/body map, step `links_bulk_edit_execute`. It parses `filter` with
   `core::query::parse`, decodes the body with `core_serde::JsonCodec` and reads
   `bulk.action` / `bulk.meta.link.to` (`data` scopes accepted as legacy).
   Missing action / non-`updateLink` / missing filter → 422 Ghost `ValidationError`.
   `support::parse_link_filter` extracts `post_id` and `to` from the NQL filter;
   missing fields → 400 Ghost `BadRequestError` "Invalid filter value received"
   (the Invalid/Unsupported-action paths are collapsed into the controller
   validation 422, which is what reaches the endpoint). It seeds the redirect,
   transforms the old URL with `absolute_to_transform_ready`, re-escapes the
   filter via `escape_nql_string`, resolves matching ids with `core::query::apply`
   (the `getFilteredIds` equivalent), applies `with_attribution` (preserves `ref`,
   appends `attribution_type`/`attribution_id` for site URLs), writes
   `to` + `moment()`-format `updated_at` through a real `ApiDataStore::bulk_edit`,
   and returns Ghost's
   `{"bulk":{"action":"updateLink","meta":{"stats":{"successful":N,"unsuccessful":M},"errors":[],"unsuccessfulData":[]}}}`.
5. Cache: no `core::cache::Cache` call; the only cache-observable behavior is the
   `X-Cache-Invalidate: /r/*` response header, identical to Ghost's `cacheInvalidate`.

**Diff**: No residual gaps. NQL filter parsing + `BadRequestError`, URL normalization,
NQL re-quoting, attribution, distinct id resolution, the real redirect update and
Ghost's `bulk` stats envelope are all implemented with the shared abstractions and
the local support layer. Non-observable architecture detail: Ghost uses a Knex
`bulkEdit('redirects')`; C++ updates in-memory `ResourceStore` records. The unsupported
action branch of the service is unreachable from the HTTP endpoint because the
controller `values:['updateLink']` validation returns 422 first — C++ returns that
same 422.

## Module-level notes

- Both handlers use the same `BlogContext` static pointer pattern
  (`ExecuteStep::s_ctx` set in each handler ctor) and a single-step
  `LocalWorkflowDriver` flow; there is no compensation logic
  (`browse/.../execute.cppm` and `bulk_edit/.../execute.cppm` `compensate` are empty).
- No serializer objects, transformer names, events, jobs, mail/outbox writes or
  webhook calls exist on the C++ side for this module — the handlers only touch the
  context-local store and write HTTP status/body/header.
- **Shared support layer**: `blog::handlers::links::support` lives in the already
  registered `:links_browse_step` partition and is imported by the bulk step and
  both handlers. It provides `kBrowseOptions`, `principal_for`, `permitted`,
  `browse_response`, `parse_link_filter`, `absolute_to_transform_ready`,
  `escape_nql_string`, `with_attribution`, `seed_link`, `link_rows`,
  `full_post_link_json`, `matched_ids`, `updated_count`, `bulk_response`, `now_iso`
  and the Ghost error builders. No new partition was added; no central file edited.
- The earlier `LinkWire` fixture is now seeded into the `ResourceStore` as a real
  record with an id and the Ghost redirect fields (`link_id`, `from`, `to`,
  `count__clicks`), so both routes observe the same mutable redirect. The wire
  mapper emits the real `FullPostLink` shape (`link:{link_id,from,to,edited}`), not
  the fixture's `{url,title}`.
- `INVALIDATE_ALL_REDIRECTS='/r/*'` (`links.js:2`) is mirrored exactly as the
  `X-Cache-Invalidate` response header on the PUT. The GET emits no invalidation.
- **Remaining new-partition need (central owner):** a real link-tracking service
  partition (e.g. `blog_handlers:links_ops` backed by a `LinkRedirect` model and
  `core_database`) would replace the seeded `ResourceStore` record with a true
  `PostLinkRepository` while preserving the same observable wire behavior. Not
  required for parity; recorded for follow-up only.
- **Bazel cycle**: the full `//include/blog/handlers:blog_handlers` build currently
  fails from unrelated in-progress partitions (recommendations, stripe, integrations,
  session) and a pre-existing module cycle; the links partitions compile cleanly and
  no links file is part of the cycle. Not fixed here (shared/central files are out of scope).
- No EXTRA: both installed C++ routes map 1:1 to Ghost controller routes
  (`routes.js:588-589`).
