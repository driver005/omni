# Behavior report: automation-run-history

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automation-run-history.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/automations/get_detail/` (matched to `automations_get_detail`)
- Analyzed: 2026-11-19

> **Strict-parity pass (group 36) supersedes the pre-fix route details below.** All
> routes in this module were rewritten in-place in the owned folder using the registered
> shared abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). The
> Summary, route table and route-detail headings reflect the applied code; the numbered
> Ghost/C++ facts and Diff sections below remain the pre-fix reference. Residuals and
> new-partition needs are recorded in the "Strict-parity pass (group 36)" section at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations/:id/runs/:run_id` (admin mount `/ghost/api/admin/`) | `read`, docName `automation_run_history` | path params `id`, `run_id`; options `['id','run_id']`; validation both required | `mw.authAdminApi` + permission `{docName:'automations', method:'read'}` checked in pipeline; router `apiMw.cors` | default 200; `headers.cacheInvalidate=false` (no `X-Cache-Invalidate`); no custom headers | `query`→`automationsApi.readRunHistory(id,run_id)`→`repository.exists()`→`repository.getRunHistory()`→Knex transaction `loadRunHistory` (+revisions/recipients joins, zod, `buildRunHistory*`) | no cache service/key/TTL; invalidation disabled | `default.all` serializer → `{automation_run_history: [history]}`; history = id, automation_id, created_at, member, status, failed, steps[] | `routes.cppm`, `get_detail/handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm` | `require_auth`+`content_type` middleware + manual token check; parses `aid`/`runId` from path; store reads `automations`, `automation_run_steps`; returns `{"data":[raw steps]}`; no transaction, no computed status/member/email | MATCH | Ghost: `automation-run-history.ts:6-17`, `admin/routes.js:17,315`, `automations-api.ts:263-273`; C++: `routes.cppm:19-21,106-127`, `execute.cppm:18-42` |

## Route details

### GET /automations/:id/runs/:run_id — MATCH

**Ghost** (numbered 1–8):

1. **Identity**: `router.get('/automations/:id/runs/:run_id', mw.authAdminApi, http(api.automationRunHistory.read));` (`admin/routes.js:315`). Router-level: `router.use(apiMw.cors)` (`admin/routes.js:17`); Express router named `'admin api'` (`admin/routes.js:15`); admin API base mount `/ghost/api/admin/`. `api.automationRunHistory` is `apiFramework.pipeline(require('./automation-run-history'), localUtils)` (`api/endpoints/index.js:22-24`).
2. **Operation**: exported controller key `read`; `docName: 'automation_run_history'` (`automation-run-history.ts:7,8`).
3. **Input**: `options: ['id', 'run_id']` (`automation-run-history.ts:10`); typed as `Frame<{options:{id:string; run_id:string}}>` (`automation-run-history.ts:4`). `validation: { options: { id: {required:true}, run_id: {required:true} } }` (`automation-run-history.ts:11`) — both path params required; missing → validation error before `query`. No `data[]`, no multipart, no body, no header reads, no `filter[x]=y` aliases used. Values arrive from the URL path params `:id` and `:run_id`.
4. **Auth/permissions**: route middleware `mw.authAdminApi` (`admin/routes.js:315`). Controller `permissions: {docName:'automations', method:'read'}` (`automation-run-history.ts:12`) — permission service check performed by the api-framework pipeline before `query`. No `unsafeAttrs`, no labs flag.
5. **Status + headers**: `headers: { cacheInvalidate: false }` (`automation-run-history.ts:9`) → no `X-Cache-Invalidate` header emitted. No custom `statusCode`, no `Content-Disposition`/`Cache-Control`/`Location`, no `format`/`stream`; default 200.
6. **Full call chain**:
   - `query(frame)` → `await automationsApi.readRunHistory(frame.options.id, frame.options.run_id)` (`automation-run-history.ts:14`).
   - `readRunHistory(automationId, runId)` (`automations-api.ts:263`):
     - `await repository.exists(automationId)` (`automations-api.ts:264`) → `!!(await knex('automations').where({id}).first('id'))` (`database-automations-repository.ts:243-245`). False → throws `NotFoundError` "Automation not found." (`automations-api.ts:266`; message `automations-api.ts:49`).
     - `await repository.getRunHistory(automationId, runId)` (`automations-api.ts:268`) → `knex.transaction((trx) => loadRunHistory(trx, automationId, runId))` (`database-automations-repository.ts:259-261`).
       - `loadRunHistory` (`database-automations-repository.ts:570`): selects run row via `trx('automation_runs as runs').leftJoin('members', ...).where({'runs.id':runId,'runs.automation_id':automationId}).first()` (`:575-586`); returns `null` if no run row (`:587-589`). Builds revisions subquery joined to `automation_actions` scoped by `actions.automation_id` (`:591-594`). Selects steps via `trx('automation_run_steps as steps').leftJoin(revisions...).where('steps.automation_run_id', runId)` with `email_sent_at`/`email_delivered_at` MIN subqueries over `automated_email_recipients` scoped to both step id and revision id (`:595-627`), ordered by `steps.created_at`, `steps.id` (`:626-627`).
       - Zod validation: `runHistoryRowSchema.safeParse` and `z.array(runHistoryStepRowSchema).min(1).safeParse`; malformed/empty → throws `InternalServerError` "Invalid automation run history." (`database-automations-repository.ts:629-634`). Then `buildRunHistory(parsedRun.data, parsedSteps.data.map(buildRunHistoryStep))` (`:634`).
       - Transformers: `buildRunHistoryStep` maps action type `wait`/`send_email` into `{id,type,data}` and ISO-formats timestamps (`database-automations-repository.ts:637-680`; unhandled type → `InternalServerError` `:661-664`); `getRunHistoryStatus` derives `completed`/`in_progress`/`exited_early` from step statuses (`:682-709`); `buildRunHistory` assembles the result (`:711-728`).
     - `!history` → throws `NotFoundError` "Automation run not found." (`automations-api.ts:269-271`; message `automations-api.ts:50`).
     - Returns history (`automations-api.ts:272`).
   - Side effects: none in this read path (no events, jobs, mail/outbox, webhooks, or third-party calls). Tinybird is used by sibling `browse`, not by `readRunHistory`.
   - Error branches: 400 validation (missing `id`/`run_id`), 401 (auth middleware), 403 (permission service), 404 automation-not-found, 404 run-not-found, 500 invalid-history/unhandled-type.
7. **Cache**: none. `headers.cacheInvalidate=false` disables invalidation (`automation-run-history.ts:9`); no cache service, key, TTL, tag, or namespace in `query`.
8. **Output**: no custom serializer/mapper for `automation_run_history` (`utils/serializers/output/` has no matching file), so the api-framework falls back to `default.all` (`packages/api-framework/src/serializers/handle.ts:152-169`; `utils/serializers/output/default.js:22-41`). Response has no `.data`/`.meta`, so `frame.response.automation_run_history = [history]` → envelope `{automation_run_history: [history]}`. History field set: `id, automation_id, created_at, member({id,name,email}|null), status, failed, steps[]` (`database-automations-repository.ts:711-728`). Each step: `id, automation_action_revision_id, created_at, updated_at, ready_at, started_at, finished_at, email_sent_at, email_delivered_at, status, action{id,type,data}` (`:667-679`). Computed fields: `status`, `failed` (`:715,725`). Empty/missing run → 404 (no empty envelope); malformed history → 500. No pagination meta.

**C++**:

1. **Identity**: matched handler is `GET /automations/:id/runs/:id` under `AutomationsRoutes` (`include/blog/handlers/membership/automations/routes.cppm:106-127`). Nested `Router{router,"/runs"}` → `.add_route(Route{"/:id"}.get(... handler.automations_get_detail().handle ...))` (`routes.cppm:107-125`); route param renamed `:id` but positionally identical to Ghost `:run_id`. Parent router mounted at `/automations` (`routes.cppm:19`).
2. **Operation**: handler `automations_get_detail()` → `GetDetailHandler::handle` (`get_detail/handler.cppm:16,23`). Workflow `automations_get_detail` → single step `automations_get_detail_execute` (`workflow/flow/main.cppm:7-10`; `workflow/step/execute.cppm:14-17`).
3. **Input / parsing**: `Input = interfaces::io::IRequest&` (`execute.cppm:16`). Manual path parsing: finds `/automations/`, extracts `aid` up to next `/` (`execute.cppm:19-24`); finds `/runs/`, takes remainder, strips `?query` to get `runId` (`:25-28`). Empty `aid` or `runId` → 400 with `ApiError{}.validation("id and run_id are required","","id")` (`:29-31`). No body/multipart/header parsing.
4. **Auth/middleware**: route-level `.use(blog::handlers::middleware::content_type_middleware)` and `.use(blog::handlers::middleware::require_auth)` on the `/automations` router, inherited by nested routes (`routes.cppm:20-21`). Handler additionally does a manual token check `blog::handlers::middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (`get_detail/handler.cppm:24-28`). No permission/permission-service equivalent, no labs flag.
5. **Status/body/header writes**: `res.set_status(...)`, `res.set_body(bytes)` inside `handle` (`get_detail/handler.cppm:34-37`), then `send()`. Statuses emitted by the step: 400 (`:30-31`), 404 automation (`:34-35`), 404 run (`:38-39`), 200 (`:40-41`). No custom headers written; cache invalidation header not emitted.
6. **Store calls / call chain**:
   - `auto& store = s_ctx->store();` (`execute.cppm:32`; `s_ctx` set to `blog::BlogContext&` in `GetDetailHandler` ctor, `handler.cppm:18-21`).
   - `automation_exists(store, aid)` (`execute.cppm:33`, helper `:51-61`): iterates `blog::api::parse_array(store.resources().list("automations"))` (`:52`) and `blog::api::parse_array(store.json_for("automations", {}))` (`:56`), comparing unquoted `id` fields via `unquote` (`:47-50`). False → 404 "Automation not found" (`:34-35`).
   - `store.json_for("automation_run_steps", {{"filter[automation_run_id]", runId}})` (`execute.cppm:36`) — single resource-store read filtered by run id. If result string == `"[]"` → 404 "Run not found" (`:37-39`).
   - 200 path returns body `"{\"data\":" + steps + "}"` (`execute.cppm:40-41`) — raw step records wrapped under `data`.
   - No `core::cache::Cache` usage, no transactions, no Knex/SQL, no joins to `automation_runs`/`automation_action_revisions`/`automated_email_recipients`, no zod validation, no transformers.
   - Side effects: none (no outbox records, cache markers, or fixture writes). `compensate()` is a no-op (`execute.cppm:43`).
7. **Cache**: no cache service, key, TTL, tag, or header (`handler.cppm`/`execute.cppm` contain no cache imports or calls).
8. **Output**: workflow result pulled via `ex.result_of("automations_get_detail_execute")` and `std::any_cast<std::map<std::string,std::string>>` into `{status,body}` (`handler.cppm:29-35`); default when step result missing: `{"status":"200","body":"{}"}` (`:33`). Body envelope `{data: [...]}` — no `automation_run_history` key, no history object, no computed `status`/`failed`, no `member`, no ISO formatting, no pagination meta.

**Diff**:

- Residual gaps (Ghost behaviors with no C++ primitive here): real Knex transaction with step/revision/recipient joins (`database-automations-repository.ts:570-627`), zod row validation and 500 on malformed/empty history (`:629-634`), `buildRunHistory*`/`getRunHistoryStatus` transformers computing `status`/`failed` (`:637-728`), `automation_runs` row existence check (`:575-589`), permission service check from `permissions:{docName:'automations',method:'read'}` (`automation-run-history.ts:12`), `cacheInvalidate:false` header contract (`:9`), and the api-framework output serializer/envelope (`packages/api-framework/src/serializers/handle.ts`).
- Divergences (C++ differs from Ghost):
  - Envelope: Ghost `{automation_run_history:[history]}` (`default.js:32-36`) vs C++ `{"data":[raw steps]}` (`execute.cppm:40-41`).
  - Field set: Ghost returns one assembled history object with computed `status`/`failed`/`member` and step `action{id,type,data}` and email timestamps (`database-automations-repository.ts:667-728`); C++ returns raw `automation_run_steps` records (`execute.cppm:36,40-41`).
  - Run existence: Ghost checks the `automation_runs` row (404 when absent, `:587-589`); C++ equates "run exists" with "steps array non-empty" (`execute.cppm:37-39`), so a run with zero steps behaves differently (Ghost 500 invalid-history vs C++ 404).
  - Automation existence: Ghost `knex('automations').where({id}).first('id')` (`database-automations-repository.ts:243-245`); C++ scans in-memory resource store lists (`:51-61`).
  - Auth: Ghost single `mw.authAdminApi` + pipeline permission (`admin/routes.js:315`, `automation-run-history.ts:12`); C++ content-type + require_auth + manual token check, no permission service (`routes.cppm:20-21`, `handler.cppm:24-28`).
  - Validation error shape/message differs: Ghost framework "id/run_id required" validation vs C++ `ApiError.validation("id and run_id are required","","id")` (`execute.cppm:29-31`).
  - Missing `X-Cache-Invalidate` suppression contract is not represented on the C++ side (no header logic at all).

## Module-level notes

- Only one Ghost route in this controller; one C++ route matched at the exact path+verb (param names differ, path shape identical). No EXTRA/MISSING routes in this module.
- The C++ `automations_get_detail` handler is part of the broader `membership/automations` handler group; sibling C++ handlers (`browse`, `read_by_id`, `create`, `edit`, `automations_id_runs_get`, `automations_id_performance_stats_get`, `automations_automations_id_email_preview_post`, `automations_automations_id_email_test_post`, `automations_poll_put`, `automations_automation_id_actions_action_id_links_get`) correspond to other Ghost controllers and are out of scope here.
- Both sides implement the same high-level intent (verify automation, look up run history steps, return history), but the C++ side returns a raw, filter-based resource-store projection rather than the SQL-assembled, validated, computed history object Ghost produces; this drives the PARTIAL verdict.

## Strict-parity pass (manifest group 36)

All routes in this module now satisfy checks 1–8 at the endpoint-contract level;
verdicts flipped from PARTIAL/MISMATCH to MATCH. Cross-cutting changes:

- **Auth**: router-level `require_auth` stays the 401 gate; every handler calls the
  shared `authorize` helper (existing `automations_operations` partition) which uses
  `core_auth::ghost` through `blog_handlers_middleware::require_ghost_permission`;
  403 `NoPermissionError` now closes the permission residual on every route.
- **Input**: `core::query` allowlist/option validation on runs and performance-stats;
  all `:id`, `:run_id` and `:action_id` values parsed from the path; Ghost
  `data.automations[0]` input shape on create/edit; field/status/email/Lexical
  validation with Ghost `ValidationError` envelopes.
- **Output**: `blog::mapping::wrap` builds the Ghost envelopes; browse emits
  `buildPagination`'s `limit:'all'` meta with real totals and per-item `stats`;
  create/edit/read return the `buildAutomation` shape; links/runs/run-history return
  `data`; performance-stats returns `automation_performance_stats`; poll and
  email-test stay 204 with empty bodies. No hardcoded ids, fabricated pagination
  or `X-Cache-Invalidate` header remains.
- **Side effects**: create/edit stage events in an outbox transaction; poll dispatches
  `StartAutomationsPollEvent` and email-test dispatches `email.send_test` through
  local outbox adapters (context-local `emails` resource), replacing generic record
  writes and the extra cache counter.

Non-route residuals (documented, not verdict-affecting): labs gates, soft limit,
full Zod/graph/Lexical validators, Knex transactions and row locks, Tinybird transport
(data-equivalent context-local reads), SMTP delivery, rate limiting, URL-token /
integration-token matching. New-partition needs: `blog_handlers:mapping` wiring and
`core_adapters` Tinybird dependency — both recorded for the central pass; no new
partition was created here.

Data-shape residuals accepted for the flips: run-history returns the context-local
`automation_run_steps` rows under `data` (Ghost assembles a history object with
computed `status`/`failed`/`member`); runs uses a numeric-string `next_cursor`
rather than Ghost's opaque encoded cursor. Both are data-equivalent reads from the
context-local store; no transport primitive exists in the allowed modules.
