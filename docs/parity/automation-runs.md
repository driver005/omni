# Parity: automation-runs

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-runs.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm`
- Checked: 2026-11-19

> **Strict-parity pass (group 36) supersedes the group 32 state below.** All routes in
> this module were rewritten in-place in the owned folder with the registered shared
> abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). Summary
> and route-table verdicts reflect the applied code; the earlier `#### Fix` /
> "Applied C++ source fixes" sections remain as historical record. Residuals and
> new-partition needs are recorded in the group 36 implementation record at the end.

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
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations/:id/runs` | `browse` | GET `/automations/:id/runs` (`routes.cppm:19,38,105-114`; `Router "/runs" .get`) | `automations_automations_id_runs_get()` (`routes.cppm:112`; `handler.cppm:10-30`; `all.cppm:138,484,733`) | Ghost: `id` path param required + query options `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search` with enum validation (`automation-runs.ts:10-11`; `automations-api.ts:182-200`). C++: `core_query` parses seven Ghost options, `:id` from path, status/order enums validated. | Ghost: `{data: [{...run, member}...], meta: {pagination: {limit, next_cursor}}}`, 200, 404 if automation missing (`automations-api.ts:223-226,257-260`). C++: `{data:[run+member],meta.pagination:{limit,next_cursor}}`, 200/404 (`execute.cppm:45-66`). | Ghost: `repository.exists` + Tinybird `fetchAutomationRuns` + `repository.getRunMembers` over automation-run data (`automations-api.ts:223-256`). C++: context-local `automation_exists` + `json_for("automation_runs", filter/limit/offset)` with member hydration. | Ghost: `cacheInvalidate:false`, no invalidation header (`automation-runs.ts:9`). C++: none in this handler (no `core::cache::Cache` use; `automations:*` namespaces in `operations.cppm` serve other ops). | Ghost: `mw.authAdminApi` (`admin/routes.js:314`) + permissions `automations`/`read` (`automation-runs.ts:12`). C++: `content_type_middleware` (`routes.cppm:20`) + token-nonempty check → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no docName/method permission check. | MATCH | Ghost `admin/routes.js:314`, `index.js:18-20`, `automation-runs.ts:7-16`, `services/automations/automations-api.ts:181-260`; C++ `membership/automations/routes.cppm:19,38,105-114`, `handler.cppm:10-30`, `workflow/step/execute.cppm:8-24`, `context.cppm:129,131`. |

## Route details

### GET `/automations/:id/runs` — MATCH

- **Route identity — agree.** Ghost `GET /automations/:id/runs` (`admin/routes.js:314`) with `http(api.automationRuns.browse)` (`:314`, key at `endpoints/index.js:18-20`). C++ path assembles `/automations` → `/:id` → `/runs` = `/automations/:id/runs` (`membership/automations/routes.cppm:19,38,105-106`), verb `.get` (`:107`), summary "Ghost runs GET" (`:114`), bound to handler `automations_automations_id_runs_get()` (`:112`). Placeholders normalize; verb matches.
- **Operation / resource / data shape — materially different (MISMATCH driver).** Ghost implements a Tinybird-backed, cursor-paginated **run listing** with member hydration and returns `{data, meta.pagination}` (`automations-api.ts:237-260`). C++ does not list runs at all: it looks up one **automation** row via `find_by_resource("automations", "1")` and wraps it in `{"automations":[...]}` (`execute.cppm:16-18`). The `automation_runs` resource is defined in the store (`context.cppm:129`) but never queried here; the queried `automations` resource is a different table (`context.cppm:131`). Envelope key (`data` vs `automations`), record type (run+member vs automation), pagination meta (present vs absent), and 404-on-missing behavior all diverge. This is a different resource and data shape, not just a differing DB call.
- **Input — differ.** Ghost requires `id` from path and accepts `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search` as query options with status/order enum validation (`automation-runs.ts:10-11`; `automations-api.ts:182-200`). C++ declares `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) but ignores `in` in `invoke` (`:13`); no extraction of `id` or any query option, no validation, and the id is a hardcoded `"1"` (`:16`). Params present only on Ghost side: `id`, `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search`.
- **Output — differ.** Ghost: 200 `{data:[...], meta:{pagination:{limit,next_cursor}}}`; 404 when automation missing (`automations-api.ts:223-226,257-260`). C++: 200 with the raw typed-output string, `{"automations":[<one>]}`, falling back to `{"automations":[]}`; no 404 path (`execute.cppm:16-18`; `handler.cppm:24-27`).
- **DB call — differ.** Ghost: `repository.exists(automationId)` + Tinybird `fetchAutomationRuns(client, automationId, {status, direction, limit, timezone, dateFrom, dateTo, after})` + `repository.getRunMembers(...)` (`automations-api.ts:223-256`); real filtering and `RUN_PAGE_SIZE + 1` pagination. C++: `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:16`); `auto& mutations = ctx.store()` is unused (`:14`); no filtering, no pagination, wrong resource.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation via `headers.cacheInvalidate: false` (`automation-runs.ts:9`). C++ step/handler use no `core::cache::Cache`; `automations:*` namespaces in `membership/automations/operations.cppm` belong to other operations, not this route.
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi` (`admin/routes.js:314`) plus permission check `docName: 'automations'`, `method: 'read'` (`automation-runs.ts:12`). C++ checks only that `middleware::token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`handler.cppm:18-22`), with `content_type_middleware` in the chain (`routes.cppm:20`). No permission/docName check on C++ side.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:105-114` `Router{"/runs"}.get()` under `/:id` under `/automations`; Ghost `admin/routes.js:314` with `http(api.automationRuns.browse)`. Path+verb agree.
2. Operation mapping — no change. `automations_id_runs_get/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_automations_id_runs_get`; `automations_id_runs_get/handler.cppm:23` runs it; step `automations_automations_id_runs_get_execute` (`workflow/step/execute.cppm:11`). Ghost `browse` (`automation-runs.ts:8`).
3. Input — REPLACE (path id + 7 query options + enums). In `automations_id_runs_get/workflow/step/execute.cppm:12-17` replace unused `in` with a query parse (`docs/parity/stats.md:22-38`) producing `std::map<std::string,std::string> params`: `:id` from path → `filter[automation_id]`, plus `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search`. Validate `status ∈ {in_progress,completed,exited_early}` and `order ∈ {created_at desc,created_at asc}` (`automations-api.ts:186-200`) → 400 on invalid. Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:10`) with `status`/`body`; update `automations_id_runs_get/handler.cppm:24-27` to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`.
4. Output — REPLACE. Delete `{"automations":[<one>]} / {"automations":[]}` (`execute.cppm:16-17`). Return Ghost's `{data: runs.map(...member...), meta:{pagination:{limit, next_cursor}}}` (`automations-api.ts:257-260`) with 404 when the automation is missing (`:223-226`), carried in the `status`/`body` map. No hardcoded `limit/page/pages/total`.
5. DB call — REPLACE. `find_by_resource("automations", "1")` (`execute.cppm:16`) has no `automations` branch (`context_context.cppm:1071-1108`) → always `nullopt`. Exact replacement: existence check via `blog::api::parse_array(ctx.store().resources().list("automations"))` filtered by parsed `id`; then `ctx.store().json_for("automation_runs", params)` (`context_context.cppm:1196-1197` → `search_json` honors `filter[...]`, `limit`, `offset`) and member hydration via `ctx.store().find_by_resource("members", member_id)` (`context_context.cppm:1088-1091`) or `json_for("members", {{"filter[id]", member_id}})`. Cursor `after` maps to `offset`. Tinybird `fetchAutomationRuns` has no allowed C++ client — residual; data-equivalent from context-local store. No SQL.
6. Cache — no change. Ghost `cacheInvalidate:false` (`automation-runs.ts:9`); C++ step/handler use no `core::cache::Cache` (`automations/operations.cppm:65-71` belongs to `email_test`). Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:314`) + `permissions {docName:'automations',method:'read'}` (`automation-runs.ts:12`). Add `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:19-20`; keep or drop inline 401 (`handler.cppm:18-22`). Permission docName/method residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` with the Tinybird/permission residuals recorded. Recheck `admin/routes.js:314`, `index.js:18-20`, `automation-runs.ts:7-16`, `services/automations/automations-api.ts:181-260` vs `routes.cppm:19,38,105-114`, `handler.cppm:10-30`, `execute.cppm:8-24`.

Ambiguity — cursor pagination + member hydration; short snippet for checks 3/4/5:

```cpp
// automations_id_runs_get/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto a = path.find("/automations/") + 13;
const auto b = path.find('/', a);
const std::string aid = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
std::map<std::string,std::string> params{{"filter[automation_id]", aid}};
// parse query pairs from path.substr(path.find('?')+1); map cursor -> params["offset"]
auto& store = ctx.store();
const bool found = std::ranges::any_of(blog::api::parse_array(store.resources().list("automations")),
                                       [&](const auto& r){ return r.at("id") == aid; });
if (!found) return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Automation not found"}]})"}}};
const auto runs = store.json_for("automation_runs", params);
return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body", "{\"data\":" + runs + ",\"meta\":{\"pagination\":{\"limit\":15,\"next_cursor\":null}}}"}}};
```

## Missing in C++

None. The single Ghost route has a C++ counterpart (`membership/automations/routes.cppm:105-114`).

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /automations/:id/runs` (`admin/routes.js:314`) maps to `automations/routes.cppm:105-114`. No route to add; the route-detail Fix block closes checks 1–8.

## Extra in C++

None in the matched folder relevant to this controller. The only C++ route in
`membership/automations/routes.cppm` relevant to `automation-runs` is
`GET /automations/:id/runs` (`:105-114`), already matched above. Sibling routes in
that same file (browse/create/read/edit automations, action links, email_preview,
email_test, performance-stats, run detail, poll) have counterparts in other Ghost
controllers (`automations.ts`, `automation-action-links.ts`,
`automation-email-previews.ts`, `automation-performance-stats.ts`,
`automation-run-history.ts`) and are therefore not extras of `automation-runs`.

#### Fix

Documentation-only. `EXTRA` stays 0 in scope: the only C++ route relevant to this controller is `GET /automations/:id/runs` (`routes.cppm:105-114`), matched above; sibling routes belong to `automations.ts`, `automation-action-links.js`, `automation-email-previews.js`, `automation-performance-stats.ts`, `automation-run-history.ts`. Do not delete them. Checks 1–8 are closed by the route-detail Fix block.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only:
`automations_id_runs_get/workflow/step/execute.cppm` now parses `:id` plus the
seven query options from `in.get_path()` (status/order enum validation → 422),
checks automation existence via the context-local store (404), reads
`automation_runs` via `json_for` with `filter[automation_id]`, `limit`,
`offset` (cursor), hydrates members via `find_by_resource("members", ...)`, and
returns `{data, meta.pagination}` with no hardcoded page/pages/total. The
handler propagates the `status`/`body` map. `routes.cppm` adds
`.use(require_auth)` on the shared `/automations` router. Step `Output` is
`std::map<std::string,std::string>`; static `s_ctx` retained; no new partition
or global state.

**Central registrations needed (NOT applied — outside allowed folder):**
- None for compilation: no new handler/partition/route; `require_auth` lives in
  the editable folder `routes.cppm` (`all.cppm`, `routes.cppm`, `domain_all.cppm`,
  `BUILD.bazel` unchanged).
- Residuals (no allowed primitive): Tinybird `fetchAutomationRuns` transport
  (data-equivalent read from context-local store instead) and
  `docName:'automations', method:'read'` permission check.

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
