# Parity: automation-run-history

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-run-history.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (controller wired via `docs/Ghost/ghost/core/server/api/endpoints/index.js:22-24`)
- C++: `include/blog/handlers/membership/automations/routes.cppm` (no dedicated run-history folder; matched by exact route path + verb, handler `automations_get_detail()`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations/:id/runs/:run_id` | `read` (`api.automationRunHistory.read`) | GET `/automations/:id/runs/:id` (placeholder normalized `:run_id`→`:id`) | `automations_get_detail()` | Ghost: required `id`, `run_id` options; C++: none parsed | Ghost: run-history model from steps; C++: `{"automations":[...]}` hardcoded id `1` | Ghost: `repository.exists(automationId)` + `getRunHistory(automationId,runId)`; C++: `find_by_resource("automations","1")` | Ghost: none (`cacheInvalidate:false`); C++: none | Ghost: `mw.authAdminApi` + permissions `docName:'automations', method:'read'`; C++: token-present check only, no permissions | MISMATCH | Ghost `routes.js:315`, `index.js:22-24`, `automation-run-history.ts:8-14`, `automations-api.ts:263-272`, `database-automations-repository.ts:243-245,259-261`; C++ `routes.cppm:106-124`, `all.cppm:139,485,734`, `get_detail/handler.cppm:17-27`, `get_detail/workflow/step/execute.cppm:11-18` |

## Route details

### GET /automations/:id/runs/:run_id — MISMATCH

- Route identity: Ghost `router.get('/automations/:id/runs/:run_id', mw.authAdminApi, http(api.automationRunHistory.read))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:315`; controller exposed as `automationRunHistory` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:22-24`. C++ nested routers `/automations` (`routes.cppm:19`) → `/:id` (`:38`) → `/runs` (`:106`) → `Route{"/:id"}` with `.get` (`:116-122`) bound to `handler.automations_get_detail()` (`:122`). Path+verb match after normalizing `:run_id`/`:id`.
- Operation mapping: Ghost op `read` (`automation-run-history.ts:8`) → `automationsApi.readRunHistory(id, run_id)` (`:14`). C++ handler is `automations_get_detail()` / workflow `automations_get_detail` (`get_detail/handler.cppm:23`, `get_detail/workflow/flow/main.cppm:9`, step `automations_get_detail_execute` at `get_detail/workflow/step/execute.cppm:12`). Despite the name, the C++ step never reads a run or run history — it returns the `automations` resource.
- Input divergence: Ghost declares `options: ['id','run_id']` with both required (`automation-run-history.ts:10-11`). C++ `using Input = interfaces::io::IRequest&` (`get_detail/workflow/step/execute.cppm:11`) performs no query/param parsing at all; neither `id` nor `run_id` is read. `id`/`run_id` present on Ghost side only.
- Output divergence: Ghost service returns the run-history object built from recorded steps (`automations-api.ts:263-272`, shape defined via `getRunHistory`/`loadRunHistory` in `database-automations-repository.ts:259-261,682-715`), with 404 on missing automation or run (`automations-api.ts:264-271`). C++ always sets status 200 (`get_detail/handler.cppm:27`) and returns `{"automations":[<single>]}`, or `{"automations":[]}` if nothing found (`get_detail/workflow/step/execute.cppm:17-18`). Envelope, resource name, status semantics, and data shape all differ; C++ never emits 404.
- DB call divergence: Ghost `repository.exists` → `knex('automations').where({id}).first('id')` (`database-automations-repository.ts:243-245`) and `getRunHistory` → `loadRunHistory(trx, automationId, runId)` filtered by BOTH ids (`:259-261`). C++ `ctx.store().find_by_resource("automations", "1")` — hardcoded `"1"`, no filtering by `id` or `run_id`, no history tables (`get_detail/workflow/step/execute.cppm:14-16`). Resource string differs (`automations` single record vs run-history data).
- Cache: Ghost `headers: { cacheInvalidate: false }` (`automation-run-history.ts:9`) — no invalidation. C++ handler/step perform no cache reads/writes (`get_detail/handler.cppm:17-28`, `get_detail/workflow/step/execute.cppm:13-19`); the only `automations:*` key (`automations:test_emails_sent`) lives in `operations.cppm:65-71` for `email_test`, not this route. Both sides: none.
- Auth divergence: Ghost applies `mw.authAdminApi` (`routes.js:315`) plus controller permissions `{ docName: 'automations', method: 'read' }` (`automation-run-history.ts:12`). C++ route installs `content_type_middleware` (`routes.cppm:20`) and the handler only checks `middleware::token(req).empty()` → `401 {"error":"Unauthorized"}` (`get_detail/handler.cppm:18-21`); no permission/docName/method authorization and no `unsafeAttrs`.
- Verdict rationale: route and verb agree, but operation, DB resource, and data shape are materially different (single hardcoded automation vs run history keyed by two ids) → MISMATCH per checklist rule 6.

## Missing in C++

None. The Ghost route has a C++ counterpart at the same path+verb (`routes.cppm:116-122`), though its operation is materially different (see MISMATCH above).

## Extra in C++

None. The sibling C++ route GET `/automations/:id/runs` → `automations_automations_id_runs_get()` (`routes.cppm:106-114`) corresponds to a different Ghost controller (`api.automationRuns.browse`, `routes.js:314`) and is not counted as an extra relative to `automation-run-history.ts`.
