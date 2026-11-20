# Behavior report: schedules

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/schedules.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (line 94; router-level `router.use(apiMw.cors)` at line 17)
- C++ folders: `include/blog/handlers/system/schedules/` — `routes.cppm`, `put_detail/handler.cppm`, `put_detail/workflow/flow/main.cppm`, `put_detail/workflow/step/execute.cppm`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 55)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Verdict unchanged: **PARTIAL**. The single route now implements Ghost's
observable HTTP contract end-to-end through shared abstractions — Ghost-shaped
validation before auth, `core_auth:ghost` permission gate, scheduled-read and
publish edit, tolerance/force semantics, status-derived cache invalidation,
Ghost allow-list output, and outbox side effects. Residuals (no real Knex DB /
config service / webhook adapter / token verification, and an unregistered
`blog_handlers:mapping` partition) have no primitive in this folder; they are
recorded in `docs/parity/schedules.md`, so the verdict is not flipped to MATCH.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| PUT `/schedules/:resource/:id` | custom action `publish`, docName `schedules` | `options:['id','resource']`; `data:['force']`; validation id required, resource required in `[posts,pages]`, global id `^(?:[a-f\d]{24}\|me)$`; `force === true` | `mw.authAdminApiWithUrl` + `permissions.canThis(context).publish.post(id)` (NotFoundError swallowed) | 200 default; `X-Cache-Invalidate: /*` on real publish, none on no-op; `headers.cacheInvalidate:false` suppresses only the framework header | `http()` → validation → serialize → permissions → `query()` → `postScheduling.publish(resource,id,force,{status:'scheduled',id,context:{internal:true}})` → `api[resource].read` + `api[resource].edit(status:'published',updated_at)` → `handleCacheInvalidation()` | no cache service in controller; `X-Cache-Invalidate` drives parent `emit-events.js` | serializer `schedules.all`: `{<resource>:[resource]}` or `{<resource>:[]}` | `system/schedules` routes + put_detail handler/flow/step | `preflight()` validation before auth; `?token=` URL-token lift; `posts::support::authorize(resource,"publish")`; in-flight guard via context-local store; `find_by_resource` + scheduled guard + 5-minute tolerance/force; full-field edit to published; `clean_rows` allow-list output; computed `X-Cache-Invalidate`; outbox `post|page.published`/`.edited`/`cache.invalidate` events drained | PARTIAL | Ghost: `routes.js:94`, `schedules.js:5-11,14-87`, `post-scheduling.js:17-130`, `validators/input/all.ts:26-27,46-60`, `can-this.js:60-137,203-207`, `post.js:413-484`; C++: `routes.cppm`, `handler.cppm:29-76`, `step/execute.cppm:40-176` |

## Route details

### PUT /schedules/:resource/:id — PARTIAL

**Ghost** (facts 1–8):

1. **Identity**: `PUT /schedules/:resource/:id`, admin router; `router.use(apiMw.cors)` applies to all admin routes (`routes.js:17,94`). `authAdminApiWithUrl` authenticates an admin token supplied in the URL (scheduler publish endpoint), followed by authorization / last-seen / cors / SSL-host redirect / pretty URLs / token permission checks (`admin/middleware.js:120-134`).
2. **Operation**: exported key `publish` (custom action), `docName:'schedules'` (`schedules.js:14-16`).
3. **Input**: `options:['id','resource']`, `data:['force']`; validation `id.required`, `resource.required` with `values:['posts','pages']` (`schedules.js:20-32`); global id regex `^(?:[a-f\d]{24}|me)$/i` (`validators/input/all.ts:26-27,46-60`). Path params populate `frame.options`; body populates `frame.data.force`.
4. **Auth/permissions**: `permissions(frame)` is a function, called directly by the pipeline; it runs `canThis(context).publish.post(id)` and swallows **only** `NotFoundError` (`schedules.js:40-51`); `NoPermissionError` → 403, validation → 422.
5. **Status + headers**: no declared statusCode → 200; `headers.cacheInvalidate:false` suppresses the framework header; `setCacheInvalidateHeader()` sets `X-Cache-Invalidate: /*` (true) or `/p/<uuid>/` (value) directly on the frame (`schedules.js:5-11,17-19,79-82`).
6. **Full call chain**: `query()` reads `frame.options.resource`, builds `{status:'scheduled',id,context:{internal:true}}`, calls `postScheduling.publish(...)` (`schedules.js:52-67`). `publish()` dedupes with an in-process `Set` keyed `${resourceType}:${id}` (`post-scheduling.js:21,33-47`); `publishNow()` reads `api[resource].read({id},options)` (NotFound → NO_OP), guards non-scheduled status (`71-73`), future-published beyond tolerance → NO_OP (`75-79`), past-published beyond tolerance without `force===true` → `NotFoundError('Use the force flag to publish a post in the past.')` (`8-10,83-88`), then `api[resource].edit({[resource]:[{status:'published',updated_at:moment(pre.updated_at).toISOString(true)}]}, pick(options,...))` (`90-104`). The model emits `published` then `edited` on a status change (`post.js:453-484`), with webhook/cache side effects. `handleCacheInvalidation()` returns true, `{value:'/p/<uuid>/'}`, or false (`112-130`).
7. **Cache**: no cache service/key/TTL in the controller; the `X-Cache-Invalidate` string drives parent `emit-events.js` when equal to `/*`.
8. **Output**: `schedules.all` sets `frame.response = data`; success `{<resource>:[full post/page]}`, no-op `{<resource>:[]}`; no pagination meta.

**C++**:

1. Router: `SchedulesRoutes::install` mounts `/schedules` + `/:resource` + `Route{"/:id"}.put(...)` → `handler.schedules_put_detail().handle(...)`, summary `"Ghost :resource/:id PUT"`. No router-level `content_type_middleware` / `require_auth` — Ghost uses body parsers and its own auth/permission pipeline.
2. Handler: ctor stores `BlogContext*` in `ExecuteStep::s_ctx`, registers `MainFlow` on a `LocalWorkflowDriver`. `handle()` runs `ExecuteStep::preflight()` **before** auth (Ghost pipeline order), lifts `?token=` into `Authorization` when absent, calls the registered `blog::handlers::posts::support::authorize(req,res,send,resource,"publish")` helper (`core_auth:ghost` through `blog_handlers_middleware:ghost_permission`), then runs workflow `"schedules_put_detail"`, reads the typed map output and writes status/body/conditional `X-Cache-Invalidate` (`handler.cppm:29-76`).
3. Flow: single step — `WorkflowBuilder{}.name("schedules_put_detail").step<ExecuteStep>().build()`; unchanged.
4. Step I/O: `Input=IRequest&`, `Output=std::map<std::string,std::string>` (`status`/`body`/`invalidate`); `s_ctx` static pointer retained. `preflight()` strips the query, splits on `/`, and returns Ghost-shaped 422 envelopes for missing id, resource outside `posts`/`pages`, or id failing the Ghost regex; `force` is parsed from the body and only boolean `true` counts (`step/execute.cppm:40-96`).
5. Store calls and publish mutation: context-local in-flight guard through `ctx.store().resources()` (`schedules_inflight:<resource>:<id>`, RAII release) mirrors Ghost's `Set` without new global state. `find_by_resource(resource,id)` → missing/non-`scheduled` → 200 no-op. RFC3339 `published_at` is compared to `system_clock::now()` with the Ghost default 5-minute tolerance: future beyond tolerance → no-op; past beyond tolerance without `force` → 404 `NotFoundError` envelope. The full stored record is decoded with `core_serde::JsonCodec`, `status` set to `published` and `updated_at` to the current UTC ISO-8601 instant, re-encoded, and written with `ctx.store().edit(resource,id,edit_body)`; `!result.ok()` → no-op (`step/execute.cppm:99-145`).
6. Status/body/headers: success → 200, body built from `result.body()` rows cleaned with the registered `posts::support::clean_rows(rows,"post"|"page")` Ghost allow-list mapper and local envelope wrap; `X-Cache-Invalidate` and a `cache.invalidate` event are emitted only when `handleCacheInvalidation("published","scheduled",uuid)` returns `/*`. No-op → 200 `{<resource>:[]}`, no header/events. Validation → 422 Ghost envelope; past-publish without force → 404 `NotFoundError` envelope (`step/execute.cppm:146-176`, `handler.cppm:73-80`).
7. Side effects: registered `blog_handlers:outbox_shim` emits `<post|page>.published`, `<post|page>.edited` (full payload + aggregate id) and `cache.invalidate`, then `drain()` dispatches registered adapters/sink. No direct SQL, no new `core::cache::Cache`, no new global state, no fixture fallback.
8. Middleware chain: none at router level. Permission gate is `core_auth:ghost` via `posts::support::authorize`; missing identity → 401 `UnauthorizedError`, known under-privileged role → 403 `NoPermissionError`. URL token is lifted from `?token=` (not URL-decoded).

**Diff**:

- Residual gaps (no C++ primitive owned by this folder): real admin-token verification / full `authAdminApiWithUrl` chain; `canThis` / `Post.permissible` loader with NotFound-only swallow (coarse gate + read-miss no-op only); shared config service for `publishAPostBySchedulerToleranceInMinutes` (Ghost default 5 used); real webhook/domain-event/cache-bus delivery (outbox records and drains events; adapters are centrally registered); real Knex `posts`/`pages` edit pipeline (`published_by`, revisions, search indexing); canonical `blog_handlers:mapping` partition (not registered — `posts::support::clean_rows` used).
- Divergences: token→role is presence-based (`Administrator`); URL token not URL-decoded; in-flight guard is context-local rather than process-global; id-format and RFC3339 parsing are local helpers, not a shared validator; `updated_at` uses `+00:00` UTC offset form rather than `moment.toISOString(true)` exact string; no real DB/config lookups. All are recorded, none are hidden.
- Matched: verb/path `PUT /schedules/:resource/:id`; validation before auth with Ghost-shaped 422s; `resource` restricted to `posts`/`pages`; global id format; URL-token auth + Ghost permission gate; 200 success and `{<resource>:[]}` no-op with no invalidation; 404 past-publish error with force semantics; 5-minute tolerance; scheduled→published state change via store edit; `X-Cache-Invalidate: /*` derived from status transition; Ghost-shaped output envelope; publish side effects observable on the outbox.

## Module-level notes

- Single-route module. The controller's only action is `publish`; `docName` is `schedules`, but the request mutates a `post` or `page` selected by `options.resource`.
- The `NO_OP` contract is central to both sides: stale/deleted/duplicate scheduler jobs must return 2xx with an empty list and no cache invalidation so the scheduler marks them done without retry. C++ reproduces it for in-flight overlap, missing records, non-scheduled status, future-beyond-tolerance, and failed edits.
- Publish side effects are now recorded as outbox events instead of being silently dropped; real webhook/cache fan-out depends on centrally registered adapters. The `resources`-tagged cache write in the generic `ResourceStore.edit` path is not used by this route (posts/pages go through the model store edit).
- Shared abstractions used: `core_auth:ghost` + `blog_handlers_middleware:ghost_permission` via `posts::support::authorize`; registered `blog_handlers:posts_support` Ghost output mapper; registered `blog_handlers:outbox_shim` for side effects; `core_serde::JsonCodec` for full-field record round-trips; `core_workflow` for the step/driver. Local helpers cover Ghost id format, RFC3339 tolerance math, cache-transition header and the response wrap because no registered shared partition provides them.
- `s_ctx` remains a static `BlogContext*` pointer, the established workflow-driver pattern in this codebase; no new global state was added for in-flight tracking.
- No EXTRA C++ routes exist under `system/schedules/routes.cppm`; no MISSING Ghost route remains.
