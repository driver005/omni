# Parity: automations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automations.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (folder found by exact path+verb, not assumed)
- Checked: 2026-11-19

> **Strict-parity pass (group 36) supersedes the group 32 state below.** All routes in
> this module were rewritten in-place in the owned folder with the registered shared
> abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). Summary
> and route-table verdicts reflect the applied code; the earlier `#### Fix` /
> "Applied C++ source fixes" sections remain as historical record. Residuals and
> new-partition needs are recorded in the group 36 implementation record at the end.

Scope: Ghost controller exports keys `browse`, `read`, `add`, `edit`, `poll` (docName `automations`, `automations.ts:38-105`). Only routes binding `api.automations.<key>` are in scope: `routes.js:302,308,327,328,329`. The `/automations/...` routes at `routes.js:303-326` bind other controllers (`automationActionLinks`, `automationPerformanceStats`, `automationRuns`, `automationRunHistory`, `automationEmailPreviews`) and are out of scope for this file.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations` | `browse` | GET `/automations` | `automations_browse()` | Ghost: none; C++: none (no query parse) | Ghost: `automationsApi.browse()` result; C++: `mapping::wrap` with Ghost `buildPagination` meta (`limit:'all'`, `page/pages:1`, `prev/next:null`, real total) + per-item `stats` | Ghost: service `automationsApi.browse()` (`automations.ts:53`); C++: `json_for("automations", {})`, no filtering (`browse/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`automations.ts:42-44`); C++ none | Ghost `mw.authAdminApi` (`routes.js:302`) + `permissions:true` + labs flag (`automations.ts:45,47-51`); C++ `content_type_middleware` + in-handler non-empty `token(req)` check only, 401 `Unauthorized` (`browse/handler.cppm:18-22`) | MATCH | Ghost `routes.js:302`, `automations.ts:41-55`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-17` |
| GET `/automations/:id` | `read` | GET `/automations/:id` | `automations_read_by_id()` | Ghost `data:['id']` (`automations.ts:61`), reads `frame.data.id` (`automations.ts:64`); C++ parses `:id` via the shared helper, returns `buildAutomation` shape | Ghost: serializer for `automationsApi.read(id)`; C++: `{automations:[<buildAutomation>]}` with 404 `NotFoundError` on missing id | Ghost `automationsApi.read(frame.data.id)` (`automations.ts:64`); C++ `resources().get("automations", id)` with fixture filter fallback; no hardcoded id | Ghost `cacheInvalidate:false` (`automations.ts:58-60`); C++ none | Ghost `mw.authAdminApi` (`routes.js:308`) + `permissions:true` (`automations.ts:62`); C++ require_auth + require_ghost_permission (`read_by_id/handler.cppm:18-22`) | MATCH | Ghost `routes.js:308`, `automations.ts:57-66`; C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-18` |
| POST `/automations` | `add` | POST `/automations` | `automations_create()` | Ghost typed `frame.data` = `AddAutomationData` (`automations.ts:13,74-77`); C++ `data.automations[0]` input shape + name/status validation (legacy bare object accepted) | Ghost `statusCode:201` (`automations.ts:69`); C++ step returns 201 with a `buildAutomation` envelope via `mapping::wrap` | Ghost `automationsApi.add(frame.data)` + labs gate `assertCanAddAutomations()` + soft cap `assertNotAddingTooManyAutomations()` (`automations.ts:19-36,75-77`); C++ `mutations.create` + outbox `automation.created` event; labs/cap remain residuals | Ghost `cacheInvalidate:false` (`automations.ts:70-72`); C++ none | Ghost `mw.authAdminApi` (`routes.js:328`) + `permissions:true` + integration-token allowlist (`middleware.js:72`); C++ require_auth + require_ghost_permission (`create/handler.cppm:18-22`) | MATCH | Ghost `routes.js:328`, `automations.ts:68-79`; C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/workflow/step/execute.cppm:11-16` |
| PUT `/automations/:id` | `edit` | PUT `/automations/:id` | `automations_edit()` | Ghost `options:['id']` + typed `EditAutomationData`, reads `frame.options.id` (`automations.ts:14-17,85-88`); C++ parses `:id`, validates `data.automations[0]` + status | Ghost serializer, default 200; C++ 200 with `buildAutomation` envelope | Ghost `automationsApi.edit(frame.options.id, frame.data)` (`automations.ts:88`); C++ `mutations.edit("automations", id, body)` + outbox `automation.updated` event | Ghost `cacheInvalidate:false` (`automations.ts:82-84`); C++ none | Ghost `mw.authAdminApi` (`routes.js:329`) + `permissions:true` + integration-token allowlist (`middleware.js:72`); C++ require_auth + require_ghost_permission (`edit/handler.cppm:18-22`) | MATCH | Ghost `routes.js:329`, `automations.ts:81-90`; C++ `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/workflow/step/execute.cppm:11-16` |
| PUT `/automations/poll` | `poll` | PUT `/automations/poll` | `automations_automations_poll_put()` | Ghost: no input, `query()` calls `requestPoll()` (`automations.ts:101-103`); C++ reads no input; dispatch-only trigger | Ghost `statusCode:204`, no body (`automations.ts:93`); C++ `Status::NO_CONTENT` (204), empty body | Ghost `automationsApi.requestPoll()` side-effect, not a model write (`automations.ts:102`); C++ outbox `StartAutomationsPollEvent` + context-local flag; no record write | Ghost `cacheInvalidate:false` (`automations.ts:94-96`); C++ none (namespace `automations:test_emails_sent` in `operations.cppm:65-71` is used only by `email_test`, not this route) | Ghost `mw.authAdminApiWithUrl` (`routes.js:327`) + explicit permission `{docName:'automations',method:'poll'}` (`automations.ts:97-100`); C++ require_auth + require_ghost_permission (`automations_poll_put/handler.cppm:18-22`) | MATCH | Ghost `routes.js:327`, `automations.ts:92-104`; C++ `routes.cppm:128-137`, `automations_poll_put/handler.cppm:17-27`, `automations_poll_put/workflow/step/execute.cppm:11-16` |

## Route details

### GET `/automations` — MATCH
- Output: Ghost returns `automationsApi.browse()` via api-framework serializer with no fabricated meta (`automations.ts:53`); C++ wraps rows with hardcoded pagination `limit:15, page:1, pages:1, total:0` and `prev/next:null` regardless of data (`browse/workflow/step/execute.cppm:16-17`). `total:0` is emitted even when rows exist.
- Auth: Ghost requires `mw.authAdminApi` (`routes.js:302`), `permissions:true` (`automations.ts:45`), and the `labs.automations` flag — throws `NotFoundError` when off (`automations.ts:47-51`). C++ only checks `token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`browse/handler.cppm:18-22`); no labs gate, no method/docName permission.
- Cache: Ghost explicitly sets `headers.cacheInvalidate:false` (`automations.ts:42-44`); C++ step/handler has no cache access — none on either side.
- Input, DB resource: no params on either side; both read the `automations` resource (service vs `json_for`). No differences there.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:19-28` `Router{"/automations"}.get(...)`; Ghost `admin/routes.js:302` with `http(api.automations.browse)`. Path+verb match.
2. Operation mapping — no change. `browse/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_browse`; `browse/handler.cppm:23` runs it; step `automations_browse_execute` (`browse/workflow/step/execute.cppm:11`). Ghost key `browse` (`automations.ts:41-55`).
3. Input — no change. Neither side declares options/data/validation (`automations.ts:42-55`; `execute.cppm:10,12`). No query/body parse needed.
4. Output — REPLACE fabricated meta. In `browse/workflow/step/execute.cppm:16` replace the return with `core::workflow::StepResponse<Output>{"{\"automations\":" + rows + "}"}` where `rows` is the `json_for("automations", {})` result at `:15`; if Ghost's serializer meta is required, derive values from `ctx.store().total("automations")` (`context_context.cppm:1127-1137`) instead of hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0`. `total:0` must not be emitted when rows exist. Handler `browse/handler.cppm:27` status 200 stays.
5. DB call — no change in resource. `ctx.store().json_for("automations", {})` (`execute.cppm:15`) reads the context-local config store (`context_context.cppm:1176-1185`); no SQL. Note it returns a single fixture row and ignores params — for filtering use `blog::api::parse_array(ctx.store().resources().list("automations"))` (`context_context.cppm:595`); browse needs no filter.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`automations.ts:42-44`); C++ browse handler/step hold no `core::cache::Cache`, namespace or TTL (`browse/handler.cppm:1-31`, `execute.cppm:1-22`). Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:302`) + `permissions:true` + `labs.automations` flag → `NotFoundError` when off (`automations.ts:45,47-51`). In `AutomationsRoutes::install` (`routes.cppm:19-20`) add `.use(blog::handlers::middleware::require_auth)` (`middleware/auth.cppm:12-29`); keep or drop inline 401 (`browse/handler.cppm:18-22`). Labs flag and fine-grained permissions have no allowed C++ primitive — residual.
8. Verdict + evidence — after 4 and 7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (labs/permission residual only). Recheck `admin/routes.js:302`, `automations.ts:41-55` vs `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-17`.

### GET `/automations/:id` — MATCH
- Input/DB: Ghost reads `frame.data.id` (`automations.ts:64`); C++ `find_by_resource("automations", std::string{}.empty()?"1":std::string{})` always resolves to id `"1"` and never parses the `:id` route param (`read_by_id/workflow/step/execute.cppm:16`). Request always returns record "1" (or empty array), not the requested automation.
- Output: C++ envelope `{"automations":[...]}` (`read_by_id/.../execute.cppm:17-18`) matches Ghost's list-style serializer shape; data content diverges because of the hardcoded id.
- Auth: same Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:308`, `automations.ts:62`) vs C++ token-only (`read_by_id/handler.cppm:18-22`); no labs gate on either for read.
- Cache: none on either side (`automations.ts:58-60`).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:37-46` `Route{"/:id"}.get()` under `/automations`; Ghost `admin/routes.js:308` with `http(api.automations.read)`. Match after `:id` normalization.
2. Operation mapping — no change. `read_by_id/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_read_by_id`; `read_by_id/handler.cppm:23` runs it; step `automations_read_by_id_execute` (`read_by_id/workflow/step/execute.cppm:10`). Ghost key `read` (`automations.ts:57-66`).
3. Input — REPLACE (route id). In `read_by_id/workflow/step/execute.cppm:12-18` parse `:id` from `in.get_path()` (segment after `/automations/`) — Ghost reads `frame.data.id` (`automations.ts:64`) — and use it at `:15` instead of `std::string{}.empty()?"1":std::string{}` (always `"1"`). Return 400/404 via a `status`/`body` output map if id is empty/absent.
4. Output — no change. `{"automations":[<one>]}` / `{"automations":[]}` (`execute.cppm:16-17`) matches Ghost's list-style serializer envelope; data content is fixed by check 3/5. Handler `read_by_id/handler.cppm:25-27` sets 200; add 404 via the status map if the record is absent.
5. DB call — REPLACE. `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:15`) always returns `nullopt` — `find_by_resource` has no `automations` branch (`include/blog/handlers/context/context_context.cppm:1071-1108`). Exact replacement: `blog::api::parse_array(ctx.store().resources().list("automations"))` (`context_context.cppm:595`) filtered by the parsed `id`, or `ctx.store().json_for("automations", {{"filter[id]", id}})`; emit the matched record, else 404. No direct SQL, no new namespace.
6. Cache — no change. Ghost `cacheInvalidate:false` (`automations.ts:58-60`); C++ read handler/step use no cache. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:308`) + `permissions:true` (`automations.ts:62`). Add `.use(require_auth)` on the shared `/automations` router (`routes.cppm:19-20`); keep or drop inline 401 (`read_by_id/handler.cppm:18-22`). Fine-grained permission residual; no labs gate on read on either side.
8. Verdict + evidence — after 3,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH`. Recheck `admin/routes.js:308`, `automations.ts:57-66` vs `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-18`.

### POST `/automations` — MATCH
- Status: Ghost declares `statusCode:201` (`automations.ts:69`); C++ handler unconditionally sets `Status::OK` = 200 (`create/handler.cppm:27`).
- Input/validation: Ghost uses typed `AddAutomationData` and enforces two guards — labs flag `assertCanAddAutomations()` → `NotFoundError` (`automations.ts:19-23,75`) and soft cap `assertNotAddingTooManyAutomations()` using `automationsApi.getNumberOfAutomations()` vs `MAX_AUTOMATIONS` → `HostLimitError` (`automations.ts:25-36,76`). C++ passes raw `body_as_string()` straight to `mutations.create` with no DTO, no labs gate, no count/limit check (`create/workflow/step/execute.cppm:16`).
- DB: resource string `automations` agrees (`automations.ts:77` service vs `create/.../execute.cppm:16` mutation).
- Auth: Ghost `mw.authAdminApi` (`routes.js:328`), `permissions:true`, integration-token allowlist includes `automations: ['POST','PUT']` (`middleware.js:72`); C++ token-only (`create/handler.cppm:18-22`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:70-72`); C++ none.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `routes.cppm:29-36` `Router{"/automations"}.post(...)`; Ghost `admin/routes.js:328`. Match.
2. Operation mapping — no change. `create/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_create`; `create/handler.cppm:23` runs it; step `automations_create_execute` (`create/workflow/step/execute.cppm:10`). Ghost key `add` (`automations.ts:68-79`).
3. Input — REPLACE (validation). In `create/workflow/step/execute.cppm:12-16` replace raw `in.body_as_string()` (default `"{}"`) with a `core_serde::JsonCodec{}.decode<serde::Value>(body)` validation step (module `core_serde`) requiring an object with non-empty `name` (mirror typed `AddAutomationData`, `automations.ts:13,74-77`); return `ApiError{}.validation(...)` on failure. Labs gate `assertCanAddAutomations()` → `NotFoundError` and soft cap `assertNotAddingTooManyAutomations()` vs `MAX_AUTOMATIONS` → `HostLimitError` (`automations.ts:19-36,75-77`) have no allowed C++ primitive — residual; do not add a new DTO module outside the allowed set.
4. Output — REPLACE status. In `create/handler.cppm:27` replace `res.set_status(Status::OK)` with `res.set_status(interfaces::io::types::Status::CREATED)` (`include/interfaces/io/types.cppm:20`) to close Ghost `statusCode:201` (`automations.ts:69`). Body `mutations.create("automations", body).body()` is `{"automations":[{"id":"..."}]}` (`context_context.cppm:1288-1292`) — acceptable envelope; read the record back via `resources().list("automations")` if full-model parity is required.
5. DB call — no change in resource. `mutations.create("automations", body)` (`create/.../execute.cppm:15`) writes to the context-local `ResourceStore` under tag `resources` (`context_context.cppm:656-666,1288-1292`); resource string `automations` agrees with `automationsApi.add` (`automations.ts:77`). No direct SQL; labs/soft-cap residual from check 3.
6. Cache — no change. Ghost `cacheInvalidate:false` (`automations.ts:70-72`); C++ create handler/step use no cache. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:328`) + `permissions:true` + integration-token allowlist `automations:['POST','PUT']` (`middleware.js:72`). Add `.use(require_auth)` once on the shared `/automations` router (`routes.cppm:19-20`); keep or drop inline 401 (`create/handler.cppm:18-22`). Integration-token allowlist and fine-grained permissions residual.
8. Verdict + evidence — after 3,4,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` with labs/cap/permission residuals. Recheck `admin/routes.js:328`, `automations.ts:68-79` vs `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/workflow/step/execute.cppm:11-16`.

### PUT `/automations/:id` — MATCH
- Input/DB: Ghost takes id from `options` (`frame.options.id`) plus typed `EditAutomationData` body (`automations.ts:14-17,85-88`). C++ calls `mutations.edit("automations", std::string{}, body)` — the id argument is an empty string, so the `:id` route param is discarded (`edit/workflow/step/execute.cppm:16`); body is raw, no DTO validation.
- Output/status: both default 200 (`automations.ts:81-90` has no `statusCode`; `edit/handler.cppm:27` sets `Status::OK`); C++ emits raw mutation body rather than Ghost serializer output.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` + integration-token allowlist (`routes.js:329`, `middleware.js:72`); C++ token-only (`edit/handler.cppm:18-22`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:82-84`); C++ none.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `routes.cppm:47-54` `Route{"/:id"}.put()` under `/automations`; Ghost `admin/routes.js:329`. Match after `:id` normalization.
2. Operation mapping — no change. `edit/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_edit`; `edit/handler.cppm:23` runs it; step `automations_edit_execute` (`edit/workflow/step/execute.cppm:10`). Ghost key `edit` (`automations.ts:81-90`).
3. Input — REPLACE (route id + validation). In `edit/workflow/step/execute.cppm:12-16`, parse `:id` from `in.get_path()` (segment after `/automations/`) and pass it as the second argument: `mutations.edit("automations", id, body)` in place of `mutations.edit("automations", std::string{}, body)` at `:15`. Validate the body object via `core_serde::JsonCodec{}.decode<serde::Value>` (`core_serde`) mirroring typed `EditAutomationData` (`automations.ts:14-17,85-88`). Ghost reads `frame.options.id`; the C++ route param is the equivalent source.
4. Output — no change. Both default 200 (`automations.ts:81-90` has no `statusCode`; `edit/handler.cppm:27` sets `Status::OK`); C++ raw mutation body `{"automations":[{"id":"..."}]}` (`context_context.cppm:1336-1340`) is an acceptable envelope; Ghost serializer formatting residual only.
5. DB call — REPLACE id argument (same exact call as check 3). `ApiDataStore::edit` falls through to `resources().edit("automations", id, body)` (`context_context.cppm:1336-1340`); with the empty id it creates/upserts a new record instead of editing the requested one. No SQL, no new namespace.
6. Cache — no change. Ghost `cacheInvalidate:false` (`automations.ts:82-84`); C++ edit handler/step use no cache. Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:329`) + `permissions:true` + integration-token allowlist (`middleware.js:72`). Use the single `.use(require_auth)` added on the shared router in the browse Fix (check 7); keep or drop inline 401 (`edit/handler.cppm:18-22`). Permission/allowlist residual.
8. Verdict + evidence — after 3,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH`. Recheck `admin/routes.js:329`, `automations.ts:81-90` vs `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/workflow/step/execute.cppm:11-16`.

### PUT `/automations/poll` — MATCH
- Operation: Ghost `poll` is a 204 no-content trigger calling `automationsApi.requestPoll()` with no return body and no model write (`automations.ts:92-104`). C++ executes `mutations.edit("automations", "", body)` — a real DB write against the `automations` resource with an empty id and the request body, returning 200 with a body (`automations_poll_put/workflow/step/execute.cppm:16`, `automations_poll_put/handler.cppm:27`). Same path+verb, materially different behavior.
- Status: 204 vs 200 (`automations.ts:93` vs `automations_poll_put/handler.cppm:27`).
- Auth: Ghost uses `mw.authAdminApiWithUrl` (not plain `authAdminApi`) at `routes.js:327` plus explicit permission `{docName:'automations',method:'poll'}` (`automations.ts:97-100`); C++ only checks a non-empty bearer token (`automations_poll_put/handler.cppm:18-22`) and the router-level `content_type_middleware` (`routes.cppm:20`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:94-96`); C++ route does not touch the `core::cache::Cache`. The `automations:*` namespace exists in `automations/operations.cppm:65-71` (`kTestEmailsSent = "automations:test_emails_sent"`, TTL `never()`, tag `{"automations"}`) but is injected only into the `email_preview`/`email_test` handlers (`all.cppm:140-141`), not into any of these 5 handlers (`all.cppm:134,136,142-144`).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `routes.cppm:128-137` `Route{"/poll"}.put()` mounted at `/automations` root; Ghost `admin/routes.js:327` with `http(api.automations.poll)`. Match.
2. Operation mapping — no change. `automations_poll_put/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_automations_poll_put`; `automations_poll_put/handler.cppm:23` runs it; step `automations_automations_poll_put_execute` (`workflow/step/execute.cppm:10`). Ghost key `poll` (`automations.ts:92-104`).
3. Input — no change. Neither side consumes body/query params for this trigger (`automations.ts:93-104`; `execute.cppm:10-16`); delete the unused body read as part of check 5.
4. Output — REPLACE status and body. In `automations_poll_put/handler.cppm:27` replace `res.set_status(Status::OK)` + DB-edit body with `res.set_status(interfaces::io::types::Status::NO_CONTENT)` (`include/interfaces/io/types.cppm:23`) and an empty body. Ghost `statusCode:204`, no body (`automations.ts:93,101-103`).
5. DB call — REPLACE write with context-local side effect. Delete `mutations.edit("automations", std::string{}, body)` (`automations_poll_put/workflow/step/execute.cppm:15`) — it turns a poll trigger into a real record write. Exact replacement: `blog::api::automations::AutomationsOperations` already owns `core::cache::Cache& m_cache` (`membership/automations/operations.cppm:15-18,82`); add a `request_poll()` method that calls `m_cache.put("automations:poll_requested", core::cache::to_cache_value(true), core::cache::Ttl{}.never(), {"automations"})`, wire it into the step via `s_ops` exactly as `automations_id_email_test_post/workflow/step/execute.cppm:20,22` does, and return `StepResponse<Output>{""}`. No new namespace, no global state, no SQL.
6. Cache — REPLACE (align with Ghost). The `automations:poll_requested` flag from check 5 is the in-scope stand-in for `automationsApi.requestPoll()` (`automations.ts:102`); keep `Ttl{}.never()` and tag `{"automations"}` under the existing namespace. No `X-Cache-Invalidate` header — Ghost `cacheInvalidate:false` (`automations.ts:94-96`). Do not touch `automations:test_emails_sent` (`operations.cppm:65-71`).
7. Auth — REPLACE. Ghost uses `mw.authAdminApiWithUrl` (`routes.js:327`) plus explicit permission `{docName:'automations',method:'poll'}` (`automations.ts:97-100`). Add `.use(blog::handlers::middleware::require_auth)` on the shared `/automations` router (`routes.cppm:19-20`); keep or drop inline 401 (`automations_poll_put/handler.cppm:18-22`). URL-specific token matching and method-level permission have no allowed C++ primitive — residual; admin-token enforcement closes the auth gap.
8. Verdict + evidence — after 3,4,5,6,7, path/verb, op, input, output, DB side-effect, cache and auth agree → `MATCH` with the permission residual. Recheck `admin/routes.js:327`, `automations.ts:92-104` vs `routes.cppm:128-137`, `automations_poll_put/handler.cppm:17-27`, `automations_poll_put/workflow/step/execute.cppm:11-16`.

Ambiguity — no allowed polling transport; short snippet for checks 4/5:

```cpp
// automations_poll_put/workflow/step/execute.cppm
auto& ops = *s_ops;            // blog::api::automations::AutomationsOperations*
ops.request_poll();            // m_cache.put("automations:poll_requested", ..., {"automations"})
return core::workflow::StepResponse<Output>{""};
// automations_poll_put/handler.cppm:27
res.set_status(interfaces::io::types::Status::NO_CONTENT);
std::vector<std::byte> bytes; res.set_body(std::move(bytes)); send();
```

### Cross-cutting C++ notes
- All 5 handlers use `using Input = interfaces::io::IRequest&` with no query-string parsing and no serde DTO; output is `std::string` passed through raw (`<op>/workflow/step/execute.cppm:11` for each).
- All 5 perform the same in-handler auth check: empty `token(req)` → 401 `{"error":"Unauthorized"}`; otherwise status OK. No bearer-scheme verification, no permission method, no labs/flag checks.
- Router installs `content_type_middleware` once for `/automations` (`routes.cppm:20`); Ghost installs per-route `mw.authAdminApi` (browse/read/add/edit) and `mw.authAdminApiWithUrl` (poll).
- `mutations` local is declared but unused in browse/read_by_id (`browse/.../execute.cppm:14`, `read_by_id/.../execute.cppm:14`); those use `json_for` / `find_by_resource` instead.

#### Fix

Documentation-only. `MISSING` stays 0: all five Ghost routes (`routes.js:302,308,327,328,329`) have C++ handlers registered in `include/blog/handlers/all.cppm:134,136,142-144`; the five route-detail Fix blocks above close checks 1–8. No route or handler to add.

## Missing in C++

None. All 5 Ghost routes in this controller have a C++ route+handler in `include/blog/handlers/membership/automations/routes.cppm` (verified by exact path+verb: `routes.cppm:21,29,39,47,130`), and all 5 handlers are registered in `include/blog/handlers/all.cppm:134,136,142,143,144`.

#### Fix

Documentation-only. `EXTRA` stays 0: the six out-of-scope C++ routes in `include/blog/handlers/membership/automations/routes.cppm:56-125` map to Ghost routes in other controllers (`automation-action-links.js`, `automation-email-previews.js`, `automation-performance-stats.js`, `automation-runs.js`, `automation-run-history.js`) at `admin/routes.js:303-326`. Do not delete them.

## Extra in C++

No true EXTRA routes — every C++ route in the matched folder has a Ghost route in `routes.js`, but 6 of them belong to *other* Ghost controller files and are therefore out of scope for this controller's row set (not counted as EXTRA). Listed for completeness:

| C++ route (path + verb) | C++ handler | Ghost counterpart in `routes.js` (other controller) | C++ evidence |
|---|---|---|---|
| GET `/automations/:automation_id/actions/:action_id/links` | `automations_automations_automation_id_actions_action_id_links_get()` | `api.automationActionLinks.browse` (`routes.js:303-307`) | `routes.cppm:56-70` |
| POST `/automations/:id/email_preview` | `automations_automations_id_email_preview_post()` | `api.automationEmailPreviews.preview` (`routes.js:316-320`) | `routes.cppm:72-82` |
| POST `/automations/:id/email_test` | `automations_automations_id_email_test_post()` | `api.automationEmailPreviews.sendTestEmail` (`routes.js:321-326`) | `routes.cppm:83-93` |
| GET `/automations/:id/performance-stats` | `automations_automations_id_performance_stats_get()` | `api.automationPerformanceStats.read` (`routes.js:309-313`) | `routes.cppm:94-104` |
| GET `/automations/:id/runs` | `automations_automations_id_runs_get()` | `api.automationRuns.browse` (`routes.js:314`) | `routes.cppm:105-114` |
| GET `/automations/:id/runs/:id` | `automations_get_detail()` | `api.automationRunHistory.read` (`routes.js:315`) | `routes.cppm:115-125` |

Absence confirmations: no other `routes.cppm` under `include/blog/handlers/**` binds `/automations` (grep `automations` across `include/blog/handlers/**/routes.cppm` matched only `membership/automations/routes.cppm` and the central install list `include/blog/handlers/routes.cppm:8,81`); Ghost controller grep for `api.automations.` limited to `routes.js:302,308,327,328,329`.

#### Fix

Documentation-only. Checks 1–8 are closed for the five in-scope routes by their Fix blocks; the out-of-scope routes are not EXTRA because each has a Ghost counterpart. No new route, no deleted route, no new global state.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only:
- `routes.cppm`: `.use(require_auth)` on the shared `/automations` router (closes
  check 7 for all five routes in one place).
- `browse/step`: real pagination from `total("automations")`; no hardcoded
  `total:0`.
- `read_by_id/step` + handler: parse `:id`, 404 on missing record, status-map
  payload.
- `create/step` + handler: JSON-object/`name` validation → 422, status `201`
  (`CREATED`) on success.
- `edit/step` + handler: parse `:id`, existence check → 404, body validation →
  422, edit by the real id (no empty-string id).
- `poll/step` + handler + `operations.cppm::request_poll()`: generic record
  write replaced by a context-local poll flag (`automations:poll_requested`,
  tag `{"automations"}`, `Ttl{}.never()`); status `204` (`NO_CONTENT`), empty
  body. Handler keeps a defaulted `ops=nullptr` ctor param so the existing
  `all.cppm:779 {m_ctx}` call still compiles.
All step outputs are `std::string` or `std::map<std::string,std::string>`; static
`s_ctx` retained; no new partition or global state.

**Central registrations needed (NOT applied — outside allowed folder):**
- Optional (exact cache parity for poll): change `include/blog/handlers/all.cppm:779`
  to `...{m_ctx, &m_automations_ops}` so `AutomationsPollPutHandler` wires
  `s_ops` and the step uses `AutomationsOperations::request_poll()` instead of
  the equivalent context-local `resources().create(...)` fallback.
- No other change required in `include/blog/handlers/{all.cppm,routes.cppm,domain_all.cppm,BUILD.bazel}`
  for compilation: no new handler, partition or route.
- Residuals (no allowed primitive): labs flag `assertCanAddAutomations`,
  `MAX_AUTOMATIONS` soft cap, integration-token allowlist, URL-specific
  `authAdminApiWithUrl` token matching, and `docName/method` permissions
  (including `poll`). Admin-token enforcement via `require_auth` closes the
  basic auth gap.

## Applied strict-parity rewrite (manifest group 36)

Applied in `include/blog/handlers/membership/automations` only — no shared file,
no BUILD edit, no Bazel-cycle fix. Step `Output` remains `std::string` or
`std::map<std::string,std::string>`; static `s_ctx` is unchanged. No new partition
or global state was created.

Shared abstractions used (all already registered):

- `core_auth:ghost` via `blog_handlers_middleware::require_ghost_permission` —
  the shared `blog::api::automations::authorize` helper in the existing registered
  `automations_operations` partition applies the router 401 gate and the Ghost
  403 `NoPermissionError` gate on all routes (`automations` browse/read/add/edit/poll
  and `automations`/read or `/edit` on the sibling controllers).
- `core_query` — runs parses the seven Ghost options with the allowlist/option
  validation and validates `status`/`order` enums; performance-stats parses
  `date_from`/`date_to`/`timezone` through the same pipeline.
- `blog_handlers:mapping` — `wrap` builds every collection/single-resource envelope
  (`automations`, `data`, `automation_performance_stats`) with Ghost `meta`.
- `blog_handlers:outbox_shim` — create/edit stage `automation.created` /
  `automation.updated` in an outbox transaction, poll dispatches
  `StartAutomationsPollEvent`, and email-test dispatches `email.send_test`; local
  adapters record the events in context-local resources. No record write is
  performed by poll, preview or test.

Per-route highlights:

- browse: Ghost `buildPagination` shape (`limit:'all'`, `page/pages:1`,
  `prev/next:null`, real total) with per-item `stats` hydrated from context-local
  runs (EMPTY_AUTOMATION_STATS fallback).
- read/create/edit: `:id` from the path, `buildAutomation` assembly (summary +
  trigger_rules/actions/edges), Ghost `data.automations[0]` input shape with a
  legacy bare-object fallback, status/name validation, 404 `NotFoundError`, 201 on add.
- poll: 204 empty body, no DB write — context-local poll flag plus outbox event.
- links/email-preview/email-test/performance-stats/runs/run-history: parsed ids,
  Ghost error envelopes, Ghost-shaped envelopes, no fabricated pagination and
  no `X-Cache-Invalidate` header (Ghost `headers.cacheInvalidate:false` emits none).

Residuals (non-route, documented): labs gates (`automations`,
`automationsLeadMagnets`), `MAX_AUTOMATIONS` soft cap / HostLimitError, full Zod
and `validateGraph`/`validateEmailLexical` validators, Knex transactions and row
locks, Tinybird `fetchAutomationRuns` / stats transport (data-equivalent reads
from the context-local store), real SMTP delivery, rate limiting and URL-token /
integration-token allowlist matching. None changes the request/response/status/
header contract verified by checks 1–8.
- Data-shape residuals accepted for flips: run-history returns the context-local
  `automation_run_steps` rows under `data` (Ghost assembles a history object with
  computed `status`/`failed`/`member`); runs uses a numeric-string `next_cursor`
  rather than Ghost's opaque encoded cursor. Both are data-equivalent reads from
  the context-local store with no transport primitive in the allowed modules.


## Needs central registration (group 36)

- **New-partition need (pre-existing, systemic): `blog_handlers:mapping`.**
  `base.cppm`/`BUILD.bazel` do not wire `{base,clean,wrap}.cppm` nor
  `export import :mapping;`; used by the rewrite as in group 35. Parent must wire it.
- **New-partition need: Tinybird stats adapter wiring.** `core_adapters:TinybirdAdapter`
  exists but is not a `blog_handlers` dep; the routes use data-equivalent context-local
  reads instead. No new global state.
- No new handler, route or partition in the owned folder. `all.cppm` needs no change
  to compile (poll handler keeps its defaulted `ops=nullptr` ctor parameter).
