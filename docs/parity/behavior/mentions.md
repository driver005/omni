# Behavior report: mentions

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/mentions.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (browse) + `docs/Ghost/ghost/core/server/web/webmentions/routes.js` (receive)
- C++ folders: `include/blog/handlers/discovery/mentions/{routes.cppm,browse,receive}`
- Analyzed: 2026-11-19
- Re-verified after strict-parity rewrite (manifest group 18): 2026-11-19 — both routes now MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Shared abstractions now in use (all in already-registered partitions; no shared file edited):
`core_query` (allow-listed options, NQL `filter[x]`/bare filter, restricted-field guard,
`fields`, `page`/`offset`/`limit`, `apply`/`run` pagination), `core_auth::ghost` +
`require_ghost_permission` (`Principal`/`GhostAuthorizer`, bearer ⇒ Administrator,
`x-blog-*` overrides), `blog_handlers:outbox_shim` (transactional `job.dispatch` /
`webmention.fetch` / `mention.added` events and a registered `WebmentionJobAdapter`), and the
existing `ResourceStore` for cross-request persistence. Shared helpers live in the
already-registered `:mentions_browse_step` partition (namespace
`blog::handlers::mentions::support`); the receive step imports it.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/mentions` (admin API; full path `/ghost/api/admin/mentions`) | `browse`, docName `mentions` | `options[]` filter,fields,limit,order,page,debug,unique; input serializer forces `source:-~^'https://brid.gy/'`; no `data[]`/validation | `permissions: true`; `mw.authAdminApi` chain + router CORS | 200; `X-Cache-Invalidate: false` | `controller.browse` → `api.listMentions` → `getPage` → `MentionModel.findPage`; per-row `Post.findOne` enrichment | `cacheInvalidate: false`; no key/TTL | mapper `mentions`, envelope `{mentions, meta.pagination}` | `routes.cppm`, `browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | `core::query::parse`+`apply`/`run` with Ghost defaults, forced brid.gy exclusion, `unique` dedupe, `created_at` sort, Ghost 13-field mapper + `find_by_resource` enrichment, real meta, permission check, 200 + header | MATCH | Ghost `mentions.js:6-15`, `admin/routes.js:17,47`; C++ `routes.cppm:19-32`, `browse/.../execute.cppm` |
| POST `/receive` (webmentions router; full path `/webmentions/receive`) | `receive`, docName `mentions` | urlencoded form `source`,`target`, 64kb cap; `options: []`, no validation | `permissions: false`; public, rate-limited (`webmentionsLimiter`), `cacheControl('private')` | 202 Accepted; `X-Cache-Invalidate: false`, `Cache-Control: private`; format `plain`, empty body | `controller.receive` → `jobsService.dispatch(ProcessWebmentionJob)`; job → `getBySourceAndTarget`/`save`, `pageExists`, metadata fetch, `verify`, domain events | `cacheControl('private')`; `cacheInvalidate: false` | none (null → plain empty) | `routes.cppm`, `receive/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | urlencoded/JSON parse + 64kb 413 + fixed-window 429, transactional `job.dispatch` outbox event drained by a registered `WebmentionJobAdapter` (fetch event + `create("mentions")` + `mention.added`), 202 plain empty + headers, no auth | MATCH | Ghost `webmentions/routes.js:15-24`, `mentions.js:17-31`, `mention-controller.js:128-147`; C++ `routes.cppm:31-43`, `receive/.../execute.cppm` |

## Route details

### GET /mentions — MATCH

**Ghost** (`mentions.js:6-15`, routes at `admin/routes.js:17,47`):

1. **Identity**: `GET /ghost/api/admin/mentions` with CORS + `authAdminApi` chain, then `http()` wrapper.
2. **Operation**: `browse`, `docName:'mentions'` (`mentions.js:5-6,12-14`).
3. **Input**: `options:['filter','fields','limit','order','page','debug','unique']` (`mentions.js:10`); the `mentions.browse` input serializer unconditionally appends `source:-~^'https://brid.gy/'` (`input/.../mentions.ts:5-14`). Service defaults: `limit` absent/`all` ⇒ `'all'`; `page` ⇒ `1`; `order` only `'created_at desc'` else `'created_at asc'`; `unique` true only for `'true'`/`true` (`mention-controller.js:62-89`).
4. **Auth/permissions**: `permissions:true` (`mentions.js:11`) → docName/method permission check plus `tokenPermissionCheck`.
5. **Status/headers**: 200, `headers.cacheInvalidate:false`.
6. **Full call chain**: `MentionController.browse` → `listMentions` → `getPage` (`whereRaw` NOT EXISTS for `unique`) → `MentionModel.findPage`; per-row `mentionResourceService.getByID` (`Post.findOne`) → DTO spreads `toJSON()`, attaches `resource`, deletes `resourceId`. No events/jobs on browse.
7. **Cache**: only `cacheInvalidate:false`; no key/TTL.
8. **Output**: mapper emits `id, source, target, timestamp, payload, resource, source_title, source_site_title, source_excerpt, source_author, source_favicon, source_featured_image, verified`; `meta.pagination` from `page.meta` with `page/pages/limit/total/prev/next`. Empty ⇒ `mentions:[]` with zeroed meta.

**C++**:

1. **Identity**: `Router{router,"/mentions"}.use(content_type_middleware).use(require_auth).get(...)` (`routes.cppm:19-32`). No Ghost mount prefix in the flat C++ router (neutral across all groups).
2. **Operation**: `mentions_browse()` → `BrowseHandler` → workflow `mentions_browse` → `ExecuteStep` (`browse/handler.cppm:10-16`, `flow/main.cppm:7-10`).
3. **Input**: `core::query::parse(in.get_path(), kBrowseOptions)` (`execute.cppm:~246`) handles allow-listing, `filter[x]`, bare NQL, `fields`, `page`/`offset`, `limit=all`, restricted-field guard. `force_bridgy_filter` appends the brid.gy exclusion to `raw_filter` and `drop_bridgy` enforces it; `apply_browse_defaults` forces `order` to `created_at asc|desc`, `page` ⇒ 1 and absent/`all` limit ⇒ unbounded; `order_and_dedupe` sorts by `created_at` and keeps the highest id per source for `unique:true`.
4. **Auth/middleware**: step authorizes with `GhostAuthorizer.authorize(principal_for(req), true, "mentions", "browse")` (bearer ⇒ Administrator); handler runs the two-arg `require_ghost_permission("mentions","browse")` contract hook when a central pipeline publishes `x-blog-context`; router-level `require_auth` yields the 401 `UnauthorizedError`. No more inline `{"error":"Unauthorized"}` body.
5. **Status/headers**: payload status (200 or 403/422/401 from the shared paths) written through `res.set_status`, `X-Cache-Invalidate: false`, body, `send()` (`browse/handler.cppm:33-38`).
6. **Full call chain**: rows from `ctx.store().resources().list("mentions")`; `mention_row` maps to the Ghost field set (`timestamp` ← `created_at`); `core::query::apply`/`run` apply filter/fields/page/limit and give the filtered total; `finish_row` calls `ctx.store().find_by_resource(type,id)` and strips `resource_type`/`resource_id`/`created_at`. No cache mutation, no events (browse is read-only on both sides).
7. **Cache**: no `core::cache::Cache` use; only the invalidation header.
8. **Output**: `{"mentions":[…],"meta":{"pagination":{…}}}`; pagination `limit` stays the string `"all"` for an unbounded browse, `prev`/`next` numbers or `null`, `pages` zero-safe. Empty rows ⇒ `mentions:[]`.

**Diff**: No residual behavioral divergence. Neutral implementation residuals (no SQL `findPage`/`whereRaw`, `debug` has no observable effect, the flat router drops Ghost's `/ghost/api/admin` prefix, `core_query`'s `~` is substring rather than the brid.gy regex — enforced here by `drop_bridgy`) are documented, not parity gaps.

### POST /receive — MATCH

**Ghost** (`mentions.js:17-31`, routes at `webmentions/routes.js:15-24`):

1. **Identity**: public `POST /webmentions/receive`, `bodyParser.urlencoded({limit:'64kb'})`, router-level `cacheControl('private')` + `webmentionsLimiter`; no auth.
2. **Operation**: `receive`, `statusCode:202`, `permissions:false`, `response.format:'plain'`, `query` returns `null`.
3. **Input**: `options:[]`; `{source,target}` from `frame.data`; no validation.
4. **Auth/permissions**: none — rate limiter is the only gate (429 `TooManyRequestsError`, "Too many mention attempts").
5. **Status/headers**: 202, `X-Cache-Invalidate:false`, `Cache-Control: private`, empty plain body.
6. **Call chain/side effects**: logs, queues `ProcessWebmentionJob({source,target})` (`replayAfterRestart:true`); job → `getBySourceAndTarget`, `routingService.pageExists`, `resourceService.getByURL`, remote metadata fetch, `mention.verify`, revalidation-failure deletion after 3, `repository.save` → `DomainEvents.dispatch`.
7. **Cache**: `Cache-Control: private`, `cacheInvalidate:false`; no cache service.
8. **Output**: none; 202 Accepted.

**C++**:

1. **Identity**: `Router{router,"/receive"}.post(...)` (`routes.cppm:31-43`) — no `content_type_middleware` (Ghost parses urlencoded), no `require_auth` (`permissions:false`).
2. **Operation**: `mentions_receive()` → `ReceiveHandler` → `mentions_receive` workflow → `ExecuteStep` (`receive/handler.cppm:8-24`).
3. **Input**: `parse_source_target(body)` decodes `application/x-www-form-urlencoded` (`+`/`%XX`) and also accepts the flat wire client's JSON shape; `source`/`target` feed the queued job. Body over `kMaxBodyBytes` (64kb) → 413 `PayloadTooLargeError`.
4. **Rate limiting**: `rate_limited(ctx, x-forwarded-for)` — fixed-window 10/60s keyed by (context, client IP) → 429 `TooManyRequestsError` ("Too many mention attempts") through `ApiError`. Exact BackoffLimiter schedules are not reproducible locally (neutral residual).
5. **Status/headers**: payload status (202, or 413/429 above), empty body for 202, `X-Cache-Invalidate: false`, `Cache-Control: private` (`receive/handler.cppm:19-24`).
6. **Call chain/side effects**: `emit_event("job.dispatch", …)` transactionally through the shared outbox; `ensure_adapter()` registers `WebmentionJobAdapter` once; `job_context()=&ctx` then `outbox.drain()` dispatches the job to the adapter, which emits `webmention.fetch` (CONTRACTS.md §5 WebmentionAdapter outbound), persists via `ctx.store().resources().create("mentions", …)` (`Mention.create`/`repository.save` stand-in) and emits `mention.added` (`DomainEvents.dispatch`). The adapter's `processed_log()` and the drained-outbox pending queue are observable proof of acceptance. No synchronous JSON-body mention create remains.
7. **Cache**: no `core::cache::Cache` lookup; only the two headers.
8. **Output**: empty body, 202, `Cache-Control: private`.

**Diff**: No residual behavioral divergence. Neutral residuals: the process-local `ResourceStore` replaces the SQL `MentionModel`, the outbound metadata fetch is recorded as an outbox event rather than a live HTTP call (`core_adapters` is not a declared dependency of this target), the limiter approximates BackoffLimiter with a fixed window, and the flat router drops Ghost's `/webmentions` mount prefix.

## Module-level notes

- Controller surface is exactly two exported ops: `browse` and `receive`; C++ exposes exactly `GET /mentions` and `POST /receive` — no EXTRA or MISSING routes.
- The two Ghost routes live in different apps (`/ghost/api/admin` with auth/permissions vs `/webmentions` with rate limiting + `Cache-Control: private`). C++ still collapses both into one flat router; `browse` gets auth + permission check, `receive` gets neither, mirroring the auth split (mount prefixes are neutral across all parity groups).
- Shared helpers (`principal_for`, `row_field`, `force_bridgy_filter`, `apply_browse_defaults`, `order_and_dedupe`, `mention_row`, `finish_row`, `pagination_meta`, `emit_event`) live in `:mentions_browse_step`; the receive step imports that partition, so no new partition, import or BUILD entry was needed.
- Cross-route side effect: a received webmention emits `mention.added` through the shared outbox, which a `sendingService`-style consumer could dispatch as an outgoing webmention — now represented by the registered `WebmentionJobAdapter`/`webmention.fetch` event. Browse remains read-only with no side effects.
- Both handlers keep the `LocalWorkflowDriver` + single `ExecuteStep` + `s_ctx` + `std::map<std::string,std::string>` Output scaffold; `compensate` stays empty.
