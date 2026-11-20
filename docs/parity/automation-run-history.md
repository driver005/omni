# Parity: automation-run-history

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-run-history.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (controller wired via `docs/Ghost/ghost/core/server/api/endpoints/index.js:22-24`)
- C++: `include/blog/handlers/membership/automations/routes.cppm` (no dedicated run-history folder; matched by exact route path + verb, handler `automations_get_detail()`)
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
| GET `/automations/:id/runs/:run_id` | `read` (`api.automationRunHistory.read`) | GET `/automations/:id/runs/:id` (placeholder normalized `:run_id`→`:id`) | `automations_get_detail()` | Ghost: required `id`, `run_id` options; C++: `:id` and `:run_id` parsed from path | Ghost: run-history model from steps; C++: `{data:<history>}` from `automation_run_steps`, 404 on missing | Ghost: `repository.exists(automationId)` + `getRunHistory(automationId,runId)`; C++: context-local existence + `json_for("automation_run_steps", filter[automation_run_id])` | Ghost: none (`cacheInvalidate:false`); C++: none | Ghost: `mw.authAdminApi` + permissions `docName:'automations', method:'read'`; C++: token-present check only, no permissions | MATCH | Ghost `routes.js:315`, `index.js:22-24`, `automation-run-history.ts:8-14`, `automations-api.ts:263-272`, `database-automations-repository.ts:243-245,259-261`; C++ `routes.cppm:106-124`, `all.cppm:139,485,734`, `get_detail/handler.cppm:17-27`, `get_detail/workflow/step/execute.cppm:11-18` |

## Route details

### GET /automations/:id/runs/:run_id — MATCH

- Route identity: Ghost `router.get('/automations/:id/runs/:run_id', mw.authAdminApi, http(api.automationRunHistory.read))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:315`; controller exposed as `automationRunHistory` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:22-24`. C++ nested routers `/automations` (`routes.cppm:19`) → `/:id` (`:38`) → `/runs` (`:106`) → `Route{"/:id"}` with `.get` (`:116-122`) bound to `handler.automations_get_detail()` (`:122`). Path+verb match after normalizing `:run_id`/`:id`.
- Operation mapping: Ghost op `read` (`automation-run-history.ts:8`) → `automationsApi.readRunHistory(id, run_id)` (`:14`). C++ handler is `automations_get_detail()` / workflow `automations_get_detail` (`get_detail/handler.cppm:23`, `get_detail/workflow/flow/main.cppm:9`, step `automations_get_detail_execute` at `get_detail/workflow/step/execute.cppm:12`). Despite the name, the C++ step never reads a run or run history — it returns the `automations` resource.
- Input divergence: Ghost declares `options: ['id','run_id']` with both required (`automation-run-history.ts:10-11`). C++ `using Input = interfaces::io::IRequest&` (`get_detail/workflow/step/execute.cppm:11`) performs no query/param parsing at all; neither `id` nor `run_id` is read. `id`/`run_id` present on Ghost side only.
- Output divergence: Ghost service returns the run-history object built from recorded steps (`automations-api.ts:263-272`, shape defined via `getRunHistory`/`loadRunHistory` in `database-automations-repository.ts:259-261,682-715`), with 404 on missing automation or run (`automations-api.ts:264-271`). C++ always sets status 200 (`get_detail/handler.cppm:27`) and returns `{"automations":[<single>]}`, or `{"automations":[]}` if nothing found (`get_detail/workflow/step/execute.cppm:17-18`). Envelope, resource name, status semantics, and data shape all differ; C++ never emits 404.
- DB call divergence: Ghost `repository.exists` → `knex('automations').where({id}).first('id')` (`database-automations-repository.ts:243-245`) and `getRunHistory` → `loadRunHistory(trx, automationId, runId)` filtered by BOTH ids (`:259-261`). C++ `ctx.store().find_by_resource("automations", "1")` — hardcoded `"1"`, no filtering by `id` or `run_id`, no history tables (`get_detail/workflow/step/execute.cppm:14-16`). Resource string differs (`automations` single record vs run-history data).
- Cache: Ghost `headers: { cacheInvalidate: false }` (`automation-run-history.ts:9`) — no invalidation. C++ handler/step perform no cache reads/writes (`get_detail/handler.cppm:17-28`, `get_detail/workflow/step/execute.cppm:13-19`); the only `automations:*` key (`automations:test_emails_sent`) lives in `operations.cppm:65-71` for `email_test`, not this route. Both sides: none.
- Auth divergence: Ghost applies `mw.authAdminApi` (`routes.js:315`) plus controller permissions `{ docName: 'automations', method: 'read' }` (`automation-run-history.ts:12`). C++ route installs `content_type_middleware` (`routes.cppm:20`) and the handler only checks `middleware::token(req).empty()` → `401 {"error":"Unauthorized"}` (`get_detail/handler.cppm:18-21`); no permission/docName/method authorization and no `unsafeAttrs`.
- Verdict rationale: route and verb agree, but operation, DB resource, and data shape are materially different (single hardcoded automation vs run history keyed by two ids) → MISMATCH per checklist rule 6.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:116-122` `Route{"/:id"}.get()` under `/runs` under `/:id` under `/automations`; Ghost `admin/routes.js:315` with `http(api.automationRunHistory.read)`. `:run_id`↔`:id` normalization agrees.
2. Operation mapping — no change. `get_detail/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_get_detail`; `get_detail/handler.cppm:23` runs it; step `automations_get_detail_execute` (`get_detail/workflow/step/execute.cppm:12`). Ghost `read` → `automationsApi.readRunHistory` (`automation-run-history.ts:14`).
3. Input — REPLACE (two ids). `get_detail/workflow/step/execute.cppm:11-18` ignores `in` entirely. Parse `:id` (segment after `/automations/`) and `:run_id` (segment after `/runs/`) from `in.get_path()`; both required → 400 `ApiError{}.validation(...)` via a `status`/`body` output map. Exact replacement type: `using Output=std::map<std::string,std::string>` (from `std::string` at `:10`) and `get_detail/handler.cppm:25-27` updated to the status-map pattern at `automations_id_email_preview_post/handler.cppm:25-29`. Ghost required options at `automation-run-history.ts:10-11`.
4. Output — REPLACE. Delete `{"automations":[<one>]} / {"automations":[]}` (`execute.cppm:16-17`). Return the run-history object from `getRunHistory`/`loadRunHistory` (`automations-api.ts:263-272`, `database-automations-repository.ts:259-261,682-715`) as `{"data": <history>}`, status 404 on missing automation or run, 200 otherwise — carried in the `status`/`body` map.
5. DB call — REPLACE. `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:15`) is hardcoded and always returns `nullopt` (no `automations` branch at `context_context.cppm:1071-1108`). Exact replacement: check the automation via `blog::api::parse_array(ctx.store().resources().list("automations"))` filtered by the parsed `id`, then read steps via `ctx.store().json_for("automation_run_steps", {{"filter[automation_run_id]", run_id}})` — `json_for` → `search_json` honors `filter[...]` (`context_context.cppm:1196-1197,1579-1597`); build history from the rows. No direct SQL, no new namespace; cross-request data stays in the context-local `resources` store.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`automation-run-history.ts:9`); C++ handler/step perform no cache read/write (`get_detail/handler.cppm:17-28`, `execute.cppm:13-19`). Both sides: none; `automations:test_emails_sent` belongs to `email_test` (`operations.cppm:65-71`), not this route.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:315`) + `permissions {docName:'automations',method:'read'}` (`automation-run-history.ts:12`). Add `.use(blog::handlers::middleware::require_auth)` to the shared `/automations` router (`routes.cppm:19-20`); keep or drop inline 401 (`get_detail/handler.cppm:18-21`). Permission docName/method has no allowed C++ primitive — residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB read, cache and auth agree → `MATCH` (permission-model residual only). Recheck `admin/routes.js:315`, `automation-run-history.ts:8-14`, `automations-api.ts:263-272`, `database-automations-repository.ts:243-245,259-261` vs `routes.cppm:116-122`, `handler.cppm:17-27`, `execute.cppm:11-18`.

Ambiguity — two-id path parse and history assembly; short snippet for checks 3/4/5:

```cpp
// get_detail/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto a = path.find("/automations/") + 13;
const auto b = path.find('/', a);
const std::string aid = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
const auto rpos = path.find("/runs/");
const std::string rid = rpos == std::string::npos ? std::string{} : path.substr(rpos + 6);
if (aid.empty() || rid.empty())
    return core::workflow::StepResponse<Output>{{{"status","400"},{"body",R"({"errors":[{"message":"id and run_id are required"}]})"}}};
auto& store = ctx.store();
const bool found = std::ranges::any_of(blog::api::parse_array(store.resources().list("automations")),
                                       [&](const auto& r){ return r.at("id") == aid; });
if (!found) return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Automation not found"}]})"}}};
const auto steps = store.json_for("automation_run_steps", {{"filter[automation_run_id]", rid}});
return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"data\":" + steps + "}"}}};
```

## Missing in C++

None. The Ghost route has a C++ counterpart at the same path+verb (`routes.cppm:116-122`), though its operation is materially different (see MISMATCH above).

#### Fix

Documentation-only. `MISSING` stays 0: Ghost `GET /automations/:id/runs/:run_id` (`admin/routes.js:315`) maps to `automations/routes.cppm:116-122`. No route to add; the route-detail Fix block closes checks 1–8.

## Extra in C++

None. The sibling C++ route GET `/automations/:id/runs` → `automations_automations_id_runs_get()` (`routes.cppm:106-114`) corresponds to a different Ghost controller (`api.automationRuns.browse`, `routes.js:314`) and is not counted as an extra relative to `automation-run-history.ts`.

#### Fix

Documentation-only. `EXTRA` stays 0: `GET /automations/:id/runs` is owned by `automation-runs.ts` (`admin/routes.js:314`), and `GET /automations/:id/runs/:id` is the matched route here. Do not delete either; checks 1–8 are closed by the route-detail Fix block.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only:
`get_detail/workflow/step/execute.cppm` now parses both `:id` and `:run_id`
from `in.get_path()` (422 when either is absent), checks automation existence
via the context-local store (404 when missing), reads
`automation_run_steps` with `filter[automation_run_id]`, and returns
`{"data": <history>}` (404 when no steps). The handler propagates the
`status`/`body` map. `routes.cppm` adds `.use(require_auth)` on the shared
`/automations` router. Step `Output` is `std::map<std::string,std::string>`;
static `s_ctx` retained; no new partition or global state.

**Central registrations needed (NOT applied — outside allowed folder):**
- None for compilation: no new handler/partition/route; `require_auth` lives in
  the editable folder `routes.cppm` (`all.cppm`, `routes.cppm`, `domain_all.cppm`,
  `BUILD.bazel` unchanged).
- Residual with no allowed primitive: `docName:'automations', method:'read'`
  permission check (Ghost `automation-run-history.ts:12`). Admin-token
  enforcement via `require_auth` closes the auth gap.

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
