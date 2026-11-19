# Parity: automation-performance-stats

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-performance-stats.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (installed via `include/blog/handlers/routes.cppm:81`; no folder-name assumption — located by exact path+verb)
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
| GET `/automations/:id/performance-stats` | `read` → `automationsApi.readPerformanceStats` | GET `/automations/:id/performance-stats` (`Router{"/automations"}`→`Router{"/:id"}`→`Route{"/performance-stats"}`) | `automations_automations_id_performance_stats_get().handle` → step `..._get_execute` | Ghost: `data:['id']` + `options:['date_from','date_to','timezone']` w/ zod validation; C++: `Input=IRequest&`, no query/body/id parsing | Ghost: `{automation_performance_stats:[{automation_id,total_run_count,in_progress_run_count,completed_run_count,exited_early_run_count,entries,entry_window}]}`, 404/403/500; C++: always 200 `{"stats":[]}` | Ghost: `repository.exists` + Tinybird pipe `api_automation_performance_stats`; C++: no real call (dead `find_by_resource("automations","1")`) | Ghost: `headers.cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + `permissions{docName:'automations',method:'read'}`; C++: token-present check → 401 only | MISMATCH | Ghost: endpoints/automation-performance-stats.ts:8-15, routes.js:309-313, automations-api.ts:138-179, tinybird-automation-stats.ts:106-152; C++: membership/automations/routes.cppm:94-104, handler.cppm:17-27, step/execute.cppm:12-22, dto/stats_empty_response.cppm:18,31 |

## Route details

### GET /automations/:id/performance-stats — MISMATCH

- **Route identity — MATCH.** Ghost `router.get('/automations/:id/performance-stats', mw.authAdminApi, http(api.automationPerformanceStats.read))` (`.../admin/routes.js:309-313`). C++ `Route{"/performance-stats"}.get(...)` nested under `/:id` under `/automations` (`membership/automations/routes.cppm:19,38,94-104`), installed via `include/blog/handlers/routes.cppm:81`. Paths and verb agree after `:id` normalization.
- **Operation mapping — PARTIAL.** Ghost controller key `read` (`automation-performance-stats.ts:8`) calls `automationsApi.readPerformanceStats` (`:14`). C++ binds `handler.automations_automations_id_performance_stats_get()` (`routes.cppm:101`, accessor `all.cppm:483`, member `all.cppm:732`), running step `automations_automations_id_performance_stats_get_execute` (`workflow/flow/main.cppm:9`, `workflow/step/execute.cppm:13`). Both are "read stats" in name only; logic is a stub (below).
- **Input parity — MISMATCH.** Ghost declares `ReadFrame = Frame<{data:{id:string}}>` (`:4`), `data:['id']` (`:10`), `options:['date_from','date_to','timezone']` (`:11`), with validation in `parseEntryStatsOptions`: ISO dates, IANA timezone default UTC, `date_to` requires `date_from`, `date_from <= date_to`, no future dates (`automation-entry-stats.ts:27-31,38-60,68-90`). C++ `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) — no query-string parsing, no `id` extraction, no `date_from`/`date_to`/`timezone` reads, no body DTO. Param names `id`, `date_from`, `date_to`, `timezone` exist on Ghost side only.
- **Output parity — MISMATCH.** Ghost returns `{automation_id, ...stats, entry_window}` (`automations-api.ts:174-178`) where `stats` = `{total_run_count, in_progress_run_count, completed_run_count, exited_early_run_count, entries:[{date,count}]}` (`tinybird-automation-stats.ts:126-147`) and `entry_window` = `{date_from,date_to,bucket,timezone}` (`automation-entry-stats.ts:97-102`), serialized under the `automation_performance_stats` docName envelope; errors 404 unknown automation (`automations-api.ts:139-142`), 500 Tinybird failure (`:144-161`), 403 permission (route middleware + `permissions`). C++ returns HTTP 200 (`handler.cppm:27`) with `encode(StatsEmptyResponse{})` (`execute.cppm:17`) → `{"stats":[]}` — DTO has single `stats` field (`dto/stats_empty_response.cppm:18,31`). No `automation_performance_stats` key, no counts/entries/window, no error statuses. Data shape materially different.
- **DB call parity — MISMATCH.** Ghost checks `repository.exists(automationId)` against the `automations` table then calls Tinybird pipe `api_automation_performance_stats` with `{version:'', automationId, timezone:'UTC', dateFrom?, dateTo?}` (`tinybird-automation-stats.ts:112-117`); e2e asserts no SQL against `automation_runs`/`automation_run_steps`/`automation_actions` (`test/e2e-api/admin/automation-performance-stats.test.ts:58-62`). C++ performs no real DB or Tinybird call: `r.first` is hardcoded `200` (`execute.cppm:16-18`), so the early return at `:19` always fires and the `find_by_resource("automations", "1")` / `{"automations":[...]}` code at `:20-22` is unreachable. That fallback also ignores the route `:id` (hardcoded `"1"`) and emits the wrong resource key.
- **Cache parity — PARTIAL.** Ghost controller sets `headers: { cacheInvalidate: false }` (`automation-performance-stats.ts:9`) — `X-Cache-Invalidate: false`; no other cache in `readPerformanceStats`. C++ has no cache behavior in this handler/step. The only cache in the automations area is `automations:test_emails_sent` (key `kTestEmailsSent`, namespace tag `automations`, TTL never) used by the email-test operation (`membership/automations/operations.cppm:65-71`), unrelated to this route. C++ side: none for this route.
- **Auth / permissions — PARTIAL.** Ghost applies `mw.authAdminApi` (`routes.js:311`) plus `permissions: {docName:'automations', method:'read'}` (`automation-performance-stats.ts:12`); author role gets 403 (`test/e2e-api/admin/automation-performance-stats.test.ts:70-77`). C++ has `content_type_middleware` on the router (`routes.cppm:20`) and an inline token-presence check in the handler: empty `middleware::token(req)` → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`). No role/permission doc checking; auth present but materially weaker.

## Missing in C++

None. The single Ghost route (`GET /automations/:id/performance-stats`) has a C++ route/handler counterpart (`membership/automations/routes.cppm:94-104`, `all.cppm:483,732`). It is a functional stub, not absent — recorded as MISMATCH above.

## Extra in C++

None. Every C++ route installed in `membership/automations/routes.cppm` maps to a Ghost admin-controller route in the automations family: `/automations` GET/POST (`:21,29` → automations browse/add), `/automations/:id` GET/PUT (`:39,47` → automations read/edit), `/:id/actions/:id/links` GET (`:60-66` → automationActionLinks.browse), `/email_preview` POST (`:73-79` → automationEmailPreviews.preview), `/email_test` POST (`:84-90` → automationEmailPreviews.test), `/performance-stats` GET (`:95-101` → this controller), `/runs` GET/`:id` GET (`:106-122` → automationRuns/automationRunHistory), `/poll` PUT (`:129-135` → automations.poll). No unmatched C++ route found.
