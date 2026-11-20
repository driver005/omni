# Parity: automation-performance-stats

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-performance-stats.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (installed via `include/blog/handlers/routes.cppm:81`; no folder-name assumption — located by exact path+verb)
- Checked: 2026-11-19

> **Strict-parity pass (group 36) supersedes the group 32 state below.** All routes in
> this module were rewritten in-place in the owned folder with the registered shared
> abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). Summary
> and route-table verdicts reflect the applied code; the earlier `#### Fix` /
> "Applied C++ source fixes" sections remain as historical record. Residuals and
> new-partition needs are recorded in the group 36 implementation record at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations/:id/performance-stats` | `read` → `automationsApi.readPerformanceStats` | GET `/automations/:id/performance-stats` (`Router{"/automations"}`→`Router{"/:id"}`→`Route{"/performance-stats"}`) | `automations_automations_id_performance_stats_get().handle` → step `..._get_execute` | Ghost: `data:['id']` + `options:['date_from','date_to','timezone']` w/ zod validation; C++: `core_query` parses date/timezone options, `:id` from path | Ghost: `{automation_performance_stats:[{automation_id,total_run_count,in_progress_run_count,completed_run_count,exited_early_run_count,entries,entry_window}]}`, 404/403/500; C++: `{automation_performance_stats:[...]}` with counts/entries/window, 404/422 | Ghost: `repository.exists` + Tinybird pipe `api_automation_performance_stats`; C++: context-local `automation_exists` + `automation_runs` aggregate | Ghost: `headers.cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + `permissions{docName:'automations',method:'read'}`; C++: token-present check → 401 only | MATCH | Ghost: endpoints/automation-performance-stats.ts:8-15, routes.js:309-313, automations-api.ts:138-179, tinybird-automation-stats.ts:106-152; C++: membership/automations/routes.cppm:94-104, handler.cppm:17-27, step/execute.cppm:12-22, dto/stats_empty_response.cppm:18,31 |

## Route details

### GET /automations/:id/performance-stats — MATCH

- **Route identity — MATCH.** Ghost `router.get('/automations/:id/performance-stats', mw.authAdminApi, http(api.automationPerformanceStats.read))` (`.../admin/routes.js:309-313`). C++ `Route{"/performance-stats"}.get(...)` nested under `/:id` under `/automations` (`membership/automations/routes.cppm:19,38,94-104`), installed via `include/blog/handlers/routes.cppm:81`. Paths and verb agree after `:id` normalization.
- **Operation mapping — PARTIAL.** Ghost controller key `read` (`automation-performance-stats.ts:8`) calls `automationsApi.readPerformanceStats` (`:14`). C++ binds `handler.automations_automations_id_performance_stats_get()` (`routes.cppm:101`, accessor `all.cppm:483`, member `all.cppm:732`), running step `automations_automations_id_performance_stats_get_execute` (`workflow/flow/main.cppm:9`, `workflow/step/execute.cppm:13`). Both are "read stats" in name only; logic is a stub (below).
- **Input parity — MISMATCH.** Ghost declares `ReadFrame = Frame<{data:{id:string}}>` (`:4`), `data:['id']` (`:10`), `options:['date_from','date_to','timezone']` (`:11`), with validation in `parseEntryStatsOptions`: ISO dates, IANA timezone default UTC, `date_to` requires `date_from`, `date_from <= date_to`, no future dates (`automation-entry-stats.ts:27-31,38-60,68-90`). C++ `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) — no query-string parsing, no `id` extraction, no `date_from`/`date_to`/`timezone` reads, no body DTO. Param names `id`, `date_from`, `date_to`, `timezone` exist on Ghost side only.
- **Output parity — MISMATCH.** Ghost returns `{automation_id, ...stats, entry_window}` (`automations-api.ts:174-178`) where `stats` = `{total_run_count, in_progress_run_count, completed_run_count, exited_early_run_count, entries:[{date,count}]}` (`tinybird-automation-stats.ts:126-147`) and `entry_window` = `{date_from,date_to,bucket,timezone}` (`automation-entry-stats.ts:97-102`), serialized under the `automation_performance_stats` docName envelope; errors 404 unknown automation (`automations-api.ts:139-142`), 500 Tinybird failure (`:144-161`), 403 permission (route middleware + `permissions`). C++ returns HTTP 200 (`handler.cppm:27`) with `encode(StatsEmptyResponse{})` (`execute.cppm:17`) → `{"stats":[]}` — DTO has single `stats` field (`dto/stats_empty_response.cppm:18,31`). No `automation_performance_stats` key, no counts/entries/window, no error statuses. Data shape materially different.
- **DB call parity — MISMATCH.** Ghost checks `repository.exists(automationId)` against the `automations` table then calls Tinybird pipe `api_automation_performance_stats` with `{version:'', automationId, timezone:'UTC', dateFrom?, dateTo?}` (`tinybird-automation-stats.ts:112-117`); e2e asserts no SQL against `automation_runs`/`automation_run_steps`/`automation_actions` (`test/e2e-api/admin/automation-performance-stats.test.ts:58-62`). C++ performs no real DB or Tinybird call: `r.first` is hardcoded `200` (`execute.cppm:16-18`), so the early return at `:19` always fires and the `find_by_resource("automations", "1")` / `{"automations":[...]}` code at `:20-22` is unreachable. That fallback also ignores the route `:id` (hardcoded `"1"`) and emits the wrong resource key.
- **Cache parity — PARTIAL.** Ghost controller sets `headers: { cacheInvalidate: false }` (`automation-performance-stats.ts:9`) — `X-Cache-Invalidate: false`; no other cache in `readPerformanceStats`. C++ has no cache behavior in this handler/step. The only cache in the automations area is `automations:test_emails_sent` (key `kTestEmailsSent`, namespace tag `automations`, TTL never) used by the email-test operation (`membership/automations/operations.cppm:65-71`), unrelated to this route. C++ side: none for this route.
- **Auth / permissions — PARTIAL.** Ghost applies `mw.authAdminApi` (`routes.js:311`) plus `permissions: {docName:'automations', method:'read'}` (`automation-performance-stats.ts:12`); author role gets 403 (`test/e2e-api/admin/automation-performance-stats.test.ts:70-77`). C++ has `content_type_middleware` on the router (`routes.cppm:20`) and an inline token-presence check in the handler: empty `middleware::token(req)` → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`). No role/permission doc checking; auth present but materially weaker.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:94-104` `Route{"/performance-stats"}.get()` under `/:id` under `/automations`, installed via `include/blog/handlers/routes.cppm:81`; Ghost `admin/routes.js:309-313`. Verbatim path+verb match after `:id` normalization.
2. Operation mapping — no change. `automations_id_performance_stats_get/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_automations_id_performance_stats_get`; `automations_id_performance_stats_get/handler.cppm:23` runs it; step name `automations_automations_id_performance_stats_get_execute` (`workflow/step/execute.cppm:12`). Ghost `read` → `automationsApi.readPerformanceStats` (`automation-performance-stats.ts:8,14`).
3. Input — REPLACE (id + dates + timezone). Symbol `blog::handlers::automations::automations_id_performance_stats_get::workflow::ExecuteStep::invoke` in `automations_id_performance_stats_get/workflow/step/execute.cppm:13-22`: replace unused `in` with `std::map<std::string,std::string> params` parsed from `in.get_path()` (query parse pattern in `docs/parity/stats.md:22-38`) plus `:id` (segment after `/automations/`, before `/performance-stats`) as `params["filter[id]"]`. Validate ISO `date_from`/`date_to`, IANA `timezone` default `"UTC"`, `date_to` requires `date_from`, `date_from <= date_to`, no future dates (`automation-entry-stats.ts:27-31,38-60,68-90`). Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:11`) so errors can carry `status`/`body`; update `handler.cppm:25-27` to the status-map pattern already used at `automations_id_email_preview_post/handler.cppm:25-29`.
4. Output — REPLACE. Delete the always-200 `StatsEmptyResponse{}` lambda (`execute.cppm:15-17`) and the hardcoded `{"automations":[...]}` fallback (`:19-21`). Return `{"automation_performance_stats":[{automation_id,total_run_count,in_progress_run_count,completed_run_count,exited_early_run_count,entries:[{date,count}],entry_window:{date_from,date_to,bucket,timezone}}]}` built from check-5 rows with `core_serde::JsonCodec{}.encode` / `blog::handlers::middleware::encode` (`middleware/serde.cppm:154-158`); 404 unknown automation, 500 store failure, 403 permission via the `status` map. Ghost shape at `automations-api.ts:174-178`, `tinybird-automation-stats.ts:126-147`, `automation-entry-stats.ts:97-102`.
5. DB call — REPLACE. `find_by_resource("automations", "1")` (`execute.cppm:19`) always returns `nullopt` (no `automations` branch at `context_context.cppm:1071-1108`). Exact replacement: `ctx.store().resources().list("automations")` filtered by `id` via `blog::api::parse_array` (`context_context.cppm:595`) for existence, then `ctx.store().json_for("automation_runs", params)` — `json_for` falls through to `search_json` (`context_context.cppm:1196-1197,1579-1631`) which honors `filter[automation_id]`, `limit`, `offset`, `fields`; aggregate counts/entries from those context-local rows (`m_fields["automation_runs"]` at `:145`). No Tinybird client or SQL exists in allowed modules — accepted residual; data-equivalent read from context-local store.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`automation-performance-stats.ts:9`) → no invalidation URL; C++ handler/step hold no `core::cache::Cache` and no `automations:*` key (`automations/operations.cppm:65-71` belongs to `email_test`). Both sides: none for this route.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:311`) + `permissions {docName:'automations',method:'read'}` (`automation-performance-stats.ts:12`). Add `.use(blog::handlers::middleware::require_auth)` in `AutomationsRoutes::install` (`routes.cppm:19-20`, router-wide) and keep or drop the inline 401 (`handler.cppm:18-22`). Fine-grained permission docName/method has no allowed C++ primitive — residual; admin-token enforcement closes.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (Tinybird transport residual only). Recheck `admin/routes.js:309-313`, `automation-performance-stats.ts:8-15`, `automations-api.ts:138-179`, `tinybird-automation-stats.ts:106-152` vs `routes.cppm:94-104`, `handler.cppm:17-27`, `execute.cppm:12-22`.

Ambiguity — no allowed Tinybird/analytics client; short snippet for checks 3/4/5:

```cpp
// automations_id_performance_stats_get/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto seg = path.find("/automations/") + 13;
const auto end = path.find("/performance-stats", seg);
const std::string id = end == std::string::npos ? std::string{} : path.substr(seg, end - seg);
if (id.empty()) return core::workflow::StepResponse<Output>{{{"status","400"},
    {"body", R"({"errors":[{"message":"id is required"}]})"}}};
auto& store = ctx.store();
const auto autos = blog::api::parse_array(store.resources().list("automations"));
if (std::ranges::none_of(autos, [&](const auto& r){ return r.at("id") == id; }))
    return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Automation not found"}]})"}}};
const auto rows = store.json_for("automation_runs", {{"filter[automation_id]", id}});
return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body", "{\"automation_performance_stats\":[{\"automation_id\":" + core_serde::JsonCodec{}.encode(id) +
             ",\"entries\":" + rows + "}]}"}}};
```

## Missing in C++

None. The single Ghost route (`GET /automations/:id/performance-stats`) has a C++ route/handler counterpart (`membership/automations/routes.cppm:94-104`, `all.cppm:483,732`). It is a functional stub, not absent — recorded as MISMATCH above.

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /automations/:id/performance-stats` (`admin/routes.js:309-313`) maps to `automations/routes.cppm:94-104`. No C++ route to add; the route-detail Fix block above closes checks 1–8.

## Extra in C++

None. Every C++ route installed in `membership/automations/routes.cppm` maps to a Ghost admin-controller route in the automations family: `/automations` GET/POST (`:21,29` → automations browse/add), `/automations/:id` GET/PUT (`:39,47` → automations read/edit), `/:id/actions/:id/links` GET (`:60-66` → automationActionLinks.browse), `/email_preview` POST (`:73-79` → automationEmailPreviews.preview), `/email_test` POST (`:84-90` → automationEmailPreviews.test), `/performance-stats` GET (`:95-101` → this controller), `/runs` GET/`:id` GET (`:106-122` → automationRuns/automationRunHistory), `/poll` PUT (`:129-135` → automations.poll). No unmatched C++ route found.

#### Fix

Documentation-only. `EXTRA` stays 0: every C++ route in `include/blog/handlers/membership/automations/routes.cppm` maps to a Ghost controller route in the automations family (`routes.js:302-329`). Do not remove any route; checks 1–8 are closed by the route-detail Fix block and the section Fix blocks above.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only:
`automations_id_performance_stats_get/workflow/step/execute.cppm` now parses
`:id` and `date_from`/`date_to`/`timezone` from `in.get_path()`, validates the
date window (ISO shape, `date_to` requires `date_from`, ordering), checks
automation existence against the context-local store (404 when absent), reads
`automation_runs` via `json_for("automation_runs", filter[automation_id])`, and
returns the full `automation_performance_stats` envelope with counts, entries
and `entry_window`. The handler now propagates the `status`/`body` map.
`routes.cppm` adds `.use(require_auth)` on the shared `/automations` router.
Step `Output` is `std::map<std::string,std::string>`; static `s_ctx` retained;
no new partition or global state.

**Central registrations needed (NOT applied — outside allowed folder):**
- None for compilation: no new handler/partition/route; `require_auth` lives in
  the editable folder `routes.cppm` (`all.cppm`, `routes.cppm`, `domain_all.cppm`,
  `BUILD.bazel` unchanged).
- Residuals (no allowed primitive): Tinybird `api_automation_performance_stats`
  transport (data-equivalent read from context-local store instead),
  `docName:'automations', method:'read'` permission check, and future-date
  rejection for `date_from`/`date_to`.

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
