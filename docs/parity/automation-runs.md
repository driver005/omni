# Parity: automation-runs

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-runs.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm`
- Checked: 2026-11-19

Controller identity: exported key is `automationRuns` (registered at
`docs/Ghost/ghost/core/server/api/endpoints/index.js:18-20`); controller `docName`
is `'automation_runs'` (`automation-runs.ts:7`). One operation: `browse`
(`automation-runs.ts:8-16`). Ghost `browse` details: `headers.cacheInvalidate: false`
(`:9`); `options: ['id','status','date_from','date_to','timezone','order','cursor','search']`
(`:10`); validation requires `id` (`:11`); `permissions: {docName: 'automations',
method: 'read'}` (`:12`); `query()` returns `automationsApi.browseRuns(frame.options.id,
frame.options)` (`:13-15`); no `data[]`, no `statusCode`, no multipart. Service
`browseRuns` (`services/automations/automations-api.ts:181-261`) validates `status`
enum `in_progress|completed|exited_early` (`:186-194`) and `order` enum
`created_at desc|created_at asc` (`:195-200`), checks `repository.exists(automationId)`
→ 404 (`:223-226`), fetches runs from Tinybird `fetchAutomationRuns` (`:237-245`) with
cursor pagination, hydrates members via `repository.getRunMembers` (`:253-256`), and
returns `{data: runs.map(...member...), meta: {pagination: {limit, next_cursor}}}`
(`:257-260`).

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
| GET `/automations/:id/runs` | `browse` | GET `/automations/:id/runs` (`routes.cppm:19,38,105-114`; `Router "/runs" .get`) | `automations_automations_id_runs_get()` (`routes.cppm:112`; `handler.cppm:10-30`; `all.cppm:138,484,733`) | Ghost: `id` path param required + query options `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search` with enum validation (`automation-runs.ts:10-11`; `automations-api.ts:182-200`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation, `id` hardcoded to `"1"` (`execute.cppm:11,13-16`). | Ghost: `{data: [{...run, member}...], meta: {pagination: {limit, next_cursor}}}`, 200, 404 if automation missing (`automations-api.ts:223-226,257-260`). C++: `{"automations":[<one row>]}` / `{"automations":[]}`, 200; no runs, no member, no pagination (`execute.cppm:16-18`; `handler.cppm:27`). | Ghost: `repository.exists` + Tinybird `fetchAutomationRuns` + `repository.getRunMembers` over automation-run data (`automations-api.ts:223-256`). C++: `find_by_resource("automations", "1")` — wrong resource (`automation_runs` exists at `context.cppm:129` but is not used); hardcoded id, no filtering/pagination (`execute.cppm:14-16`). | Ghost: `cacheInvalidate:false`, no invalidation header (`automation-runs.ts:9`). C++: none in this handler (no `core::cache::Cache` use; `automations:*` namespaces in `operations.cppm` serve other ops). | Ghost: `mw.authAdminApi` (`admin/routes.js:314`) + permissions `automations`/`read` (`automation-runs.ts:12`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no docName/method permission check. | MISMATCH | Ghost `admin/routes.js:314`, `index.js:18-20`, `automation-runs.ts:7-16`, `services/automations/automations-api.ts:181-260`; C++ `membership/automations/routes.cppm:19,38,105-114`, `handler.cppm:10-30`, `workflow/step/execute.cppm:8-24`, `context.cppm:129,131`. |

## Route details

### GET `/automations/:id/runs` — MISMATCH

- **Route identity — agree.** Ghost `GET /automations/:id/runs` (`admin/routes.js:314`) with `http(api.automationRuns.browse)` (`:314`, key at `endpoints/index.js:18-20`). C++ path assembles `/automations` → `/:id` → `/runs` = `/automations/:id/runs` (`membership/automations/routes.cppm:19,38,105-106`), verb `.get` (`:107`), summary "Ghost runs GET" (`:114`), bound to handler `automations_automations_id_runs_get()` (`:112`). Placeholders normalize; verb matches.
- **Operation / resource / data shape — materially different (MISMATCH driver).** Ghost implements a Tinybird-backed, cursor-paginated **run listing** with member hydration and returns `{data, meta.pagination}` (`automations-api.ts:237-260`). C++ does not list runs at all: it looks up one **automation** row via `find_by_resource("automations", "1")` and wraps it in `{"automations":[...]}` (`execute.cppm:16-18`). The `automation_runs` resource is defined in the store (`context.cppm:129`) but never queried here; the queried `automations` resource is a different table (`context.cppm:131`). Envelope key (`data` vs `automations`), record type (run+member vs automation), pagination meta (present vs absent), and 404-on-missing behavior all diverge. This is a different resource and data shape, not just a differing DB call.
- **Input — differ.** Ghost requires `id` from path and accepts `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search` as query options with status/order enum validation (`automation-runs.ts:10-11`; `automations-api.ts:182-200`). C++ declares `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) but ignores `in` in `invoke` (`:13`); no extraction of `id` or any query option, no validation, and the id is a hardcoded `"1"` (`:16`). Params present only on Ghost side: `id`, `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search`.
- **Output — differ.** Ghost: 200 `{data:[...], meta:{pagination:{limit,next_cursor}}}`; 404 when automation missing (`automations-api.ts:223-226,257-260`). C++: 200 with the raw typed-output string, `{"automations":[<one>]}`, falling back to `{"automations":[]}`; no 404 path (`execute.cppm:16-18`; `handler.cppm:24-27`).
- **DB call — differ.** Ghost: `repository.exists(automationId)` + Tinybird `fetchAutomationRuns(client, automationId, {status, direction, limit, timezone, dateFrom, dateTo, after})` + `repository.getRunMembers(...)` (`automations-api.ts:223-256`); real filtering and `RUN_PAGE_SIZE + 1` pagination. C++: `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:16`); `auto& mutations = ctx.store()` is unused (`:14`); no filtering, no pagination, wrong resource.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation via `headers.cacheInvalidate: false` (`automation-runs.ts:9`). C++ step/handler use no `core::cache::Cache`; `automations:*` namespaces in `membership/automations/operations.cppm` belong to other operations, not this route.
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi` (`admin/routes.js:314`) plus permission check `docName: 'automations'`, `method: 'read'` (`automation-runs.ts:12`). C++ checks only that `middleware::token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`handler.cppm:18-22`), with `content_type_middleware` in the chain (`routes.cppm:20`). No permission/docName check on C++ side.

## Missing in C++

None. The single Ghost route has a C++ counterpart (`membership/automations/routes.cppm:105-114`).

## Extra in C++

None in the matched folder relevant to this controller. The only C++ route in
`membership/automations/routes.cppm` relevant to `automation-runs` is
`GET /automations/:id/runs` (`:105-114`), already matched above. Sibling routes in
that same file (browse/create/read/edit automations, action links, email_preview,
email_test, performance-stats, run detail, poll) have counterparts in other Ghost
controllers (`automations.ts`, `automation-action-links.ts`,
`automation-email-previews.ts`, `automation-performance-stats.ts`,
`automation-run-history.ts`) and are therefore not extras of `automation-runs`.
