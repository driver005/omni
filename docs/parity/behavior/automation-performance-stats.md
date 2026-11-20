# Behavior report: automation-performance-stats

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automation-performance-stats.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/automations/automations_id_performance_stats_get/`
- Analyzed: 2026-11-19

> **Strict-parity pass (group 36) supersedes the pre-fix route details below.** All
> routes in this module were rewritten in-place in the owned folder using the registered
> shared abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). The
> Summary, route table and route-detail headings reflect the applied code; the numbered
> Ghost/C++ facts and Diff sections below remain the pre-fix reference. Residuals and
> new-partition needs are recorded in the "Strict-parity pass (group 36)" section at the end.

Scope note: `automation-performance-stats.ts` exports exactly one operation (`read`,
`automation-performance-stats.ts:8-16`). Exactly one route binds it:
`GET /automations/:id/performance-stats` (`routes.js:309-313`, registered as
`api.automationPerformanceStats` at `endpoints/index.js:14-16`). No sibling route
shares this controller; the adjacent `:id`, `:id/runs`, `:id/email_preview` and
`:id/email_test` routes bind different controllers (`routes.js:308,314-326`).
The C++ route `GET /automations/:id/performance-stats`
(`membership/automations/routes.cppm:95-105`) is the exact path+verb counterpart;
no EXTRA C++ route for this controller was found.

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
| GET `/ghost/api/admin/automations/:id/performance-stats` (route-relative `GET /automations/:id/performance-stats`) | `read` / `docName: 'automation_performance_stats'` | `data: ['id']` from `:id`; `options: ['date_from','date_to','timezone']` from query; no body/multipart; no headers read by controller; framework validates `id` as 24-hex or `me` | `mw.authAdminApi` chain; permissions `{docName:'automations',method:'read'}` (→ `canThis(ctx).read.automation`); no `unsafeAttrs`; no labs gate | 200 OK; `headers.cacheInvalidate:false` (no X-Cache-Invalidate); app-level `Cache-Control: private`; errors 401/403/404/422/500 | `query` → `automationsApi.readPerformanceStats(id, options)` → `repository.exists()` → `getTinybirdClient()` → `parseEntryStatsOptions()` → `fetchAutomationPerformanceStats()` (Tinybird pipe `api_automation_performance_stats`) → `getEntryStatsWindow()` → range check → assemble stats | No cache read/write; non-invalidating + `private`; no cache tags/TTL | `{ automation_performance_stats: [{ automation_id, total_run_count, in_progress_run_count, completed_run_count, exited_early_run_count, entries, entry_window }] }` | `membership/automations/routes.cppm:95-105`; `automations_id_performance_stats_get/handler.cppm:16-40`; `workflow/flow/main.cppm:7-10`; `workflow/step/execute.cppm:16-134` | Token-presence auth only; in-memory `automation_runs` fixture instead of Tinybird; count=1 per run; `date_from`/`date_to` echoed raw, not derived; bucket hardcoded `day`; no 403/500; no cache headers | MATCH | Ghost: `automation-performance-stats.ts:4-19`, `routes.js:309-313`, `automations-api.ts:138-179`, `automation-entry-stats.ts:27-107`, `tinybird-automation-stats.ts:90-152`; C++: `routes.cppm:19-22,39,95-105`, `execute.cppm:20-67` |

## Route details

### GET /ghost/api/admin/automations/:id/performance-stats — MATCH

**Ghost** (facts 1–8):

1. **Identity.** Route
   `router.get('/automations/:id/performance-stats', mw.authAdminApi, http(api.automationPerformanceStats.read))`
   at `routes.js:309-313`. Router-level mount: `router.use(apiMw.cors)` (`routes.js:17`).
   Admin router installed via `apiApp.use(routes())` (`admin/app.js:44`), mounted at
   `/admin/` on the API app (`web/api/app.js:22`), under base path `/ghost/api`
   (`shared/url-utils.ts:4`, `web/parent/backend.js:15`). Admin app middleware before
   routing: `cacheControl('private')` (`admin/app.js:24`), optional `betterAuthRouter`
   (`:27-30`), JSON + urlencoded body parsers (`:33-34`), `parseQueryBooleans` (`:37`),
   `checkVersionMatch` (`:41`). After routing: `errorHandler.resourceNotFound` and
   `errorHandler.handleJSONResponse(sentry)` (`admin/app.js:47-48`). Parent app adds
   version rewrites + `maxLimitCap` (`web/api/app.js:15-19`). `api.automationPerformanceStats`
   is the api-framework pipeline over this controller (`endpoints/index.js:14-16`,
   `packages/api-framework/src/pipeline.ts:324-466`).

2. **Operation.** Exported key `read`; module `docName: 'automation_performance_stats'`
   (`automation-performance-stats.ts:6-8`). `query(frame)` returns
   `await automationsApi.readPerformanceStats(frame.data.id, frame.options)`
   (`:13-15`). Frame type `Frame<{ data: { id: string } }>` (`:4`). Controller config:
   `headers: {cacheInvalidate:false}` (`:9`), `data: ['id']` (`:10`),
   `options: ['date_from','date_to','timezone']` (`:11`),
   `permissions: {docName:'automations', method:'read'}` (`:12`). No `schema`,
   `cache`, `statusCode`, `response.format`, `validation` or custom serializer key.

3. **Input.** `frame.data.id` comes from the `:id` path param: `data: ['id']`
   (`automation-performance-stats.ts:10`) and `Frame.configure` picks `id` from
   `original.params` when no body is present (`packages/api-framework/src/frame.ts:79-115`).
   `frame.options` gets exactly `date_from`, `date_to`, `timezone` picked from
   query/params/options (`automation-performance-stats.ts:11`, `frame.ts:68-77`).
   No `data.<resource>[0]` body shape, no multipart fields, no headers read by the
   controller. Framework validation: no `validation{}` block, so shared validation
   runs (`pipeline.ts:109-133,430-433`); `read` delegates to `validators.browse`
   (`validators/input/all.ts:156-159`) which applies `GLOBAL_VALIDATORS.id =
   /^(?:[a-f\d]{24}|me)$/i` to `frame.data.id` (`validators/input/all.ts:26-27,46-70`)
   → non-ObjectId/non-`me` id rejected as a ValidationError. Service-level accepted
   values: `date_from`/`date_to` ISO `YYYY-MM-DD` (`automation-entry-stats.ts:40-41`),
   `timezone` any `moment.tz` zone, case-normalized, default `UTC`
   (`automation-entry-stats.ts:27-31`); cross-field rules: `date_to` without
   `date_from` rejected, `date_from > date_to` rejected, future dates rejected
   (`automation-entry-stats.ts:44-60`, messages `:19-25`). Plain query names only —
   no `filter[x]=y` aliases in this controller (contrast `automation-runs.ts:10`).
   E2E confirms `?timezone=america%2Fnew_york` normalization
   (`test/e2e-api/admin/automation-performance-stats.test.ts:147-164`) and
   `?date_from=...&date_to=...&timezone=...` (`:209-252`).

4. **Auth/permissions.** Route middleware `mw.authAdminApi`
   (`routes.js:311`) expands to
   `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors,
   adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]`
   (`admin/middleware.js:110-118`). Controller
   `permissions: {docName:'automations', method:'read'}`
   (`automation-performance-stats.ts:12`) is merged into the pipeline's
   permission config (`pipeline.ts:235-250`) and handled by
   `permissions.handle` (`endpoints/utils/permissions.js:94-107`): singular becomes
   `automation`, identifier is `frame.options.id` (undefined here — id lives in
   `frame.data`), check is `permissions.canThis(context).read.automation(undefined,{})`
   (`permissions.js:18-49`); denial → `NoPermissionError` reworded
   "You do not have permission to read automations" (`permissions.js:66-73`,
   messages `:7-9`). E2E: author role → 403 (`test:70-77`); owner/admin staff JWT →
   200 (`test:136-145`). No `unsafeAttrs`, no labs flag, no permission call inside
   `query` itself.

5. **Status + headers.** 200 on success (`http.ts:126-137`; no `statusCode` in
   controller). `headers: {cacheInvalidate:false}`
   (`automation-performance-stats.ts:9`) means the `if (apiConfigHeaders.cacheInvalidate)`
   branch is skipped → no `X-Cache-Invalidate` header (`http.ts:135-144` →
   `headers.ts:21-27,135-144`). Admin app sets `Cache-Control: private`
   (`admin/app.js:24`). JSON response (no `response.format`, `http.ts:139-168`).
   Error branches, all surfaced through the Express error handler
   (`admin/app.js:47-48`, `http.ts:169-176`):
   - 401 unauthenticated — `mw.authAdminApi` (`admin/middleware.js:110-118`).
   - 403 no read permission — `permissions.js:66-73`; E2E `test:70-77`.
   - 404 `NotFoundError` "Automation not found." when `repository.exists(id)` is false
     (`automations-api.ts:139-142`, message `:49`); E2E `test:66-68`.
   - 422 `ValidationError` "Invalid automation entry date range." + joined sub-issue
     context (`automation-entry-stats.ts:68-75`, messages `:19-25`); E2E invalid timezone
     `test:166-172`, future date `test:320-324`, `date_to` without `date_from`
     `test:327-331`. Framework id-format validation also 422 (`validators/input/all.ts:26-27`).
   - 500 `InternalServerError` "Could not load Tinybird automation performance stats."
     when `getTinybirdClient()` returns null (`automations-api.ts:144-149`, client `:83-103`,
     message `:47`) or when `fetchAutomationPerformanceStats` returns null
     (`:157-161`); 500 "Tinybird returned entries outside the requested range."
     (`:166-173`, message `:46`); DB lookup failure also 500 (`test:97-117`).
   No `Content-Disposition`, `Location`, custom `format` or `stream`.

6. **Full call chain.** `query` (`automation-performance-stats.ts:13-15`) →
   `automationsApi.readPerformanceStats(id, options)` (`automations-api.ts:138-179`):
   - `repository.exists(id)` (`:139` → `database-automations-repository.ts:243-245`,
     `knex('automations').where({id}).first('id')`) → 404 if false (`:140-142`).
   - `getTinybirdClient()` (`:144` → `:83-103`): requires `tinybird:stats` config and
     `TinybirdServiceWrapper.instance.getToken().token`; builds client with
     `requestExternal.extend({retry:{limit:0}})`, `settingsCache` and `logging`;
     errors are logged and return null → 500 (`:145-149`).
   - `parseEntryStatsOptions(options)` (`:150` → `automation-entry-stats.ts:68-90`):
     Zod schema + `superRefine` (`:38-60`), timezone transform (`:27-31`),
     inclusive request dates → exclusive upper bound via `nextDate` (`:62-64,79-88`).
   - `fetchAutomationPerformanceStats(client, id, {timezone, dateFrom, dateTo})`
     (`:151-156` → `tinybird-automation-stats.ts:106-152`): `client.fetch(
     'api_automation_performance_stats', {version:'', automationId, timezone:'UTC',
     ...options})` (`:112-117`), Zod `performanceRowSchema` with
     `invalid_run_count === 0` refinement (`:96-104`), `.min(1)` + duplicate-date
     rejection (`:118-125`), per-row sums into `total/in_progress/completed/
     exited_early/entries` (`:126-141`), safe-integer guard (`:142-145`), date sort
     (`:146`), null on any throw (`:148-151`). E2E nock confirms the exact pipe and
     query params (`test:119-134`) and that no `automation_runs`/`automation_run_steps`/
     `automation_actions` SQL is issued (`test:48-63`).
   - `getEntryStatsWindow(stats.entries, timezone)` (`:162` →
     `automation-entry-stats.ts:93-103`): derives `date_from` from first entry,
     exclusive `date_to` from last, `bucket = 'hour'` if dates contain `T` else `'day'`.
     Requested window supplied → reuse its bounds with derived `bucket`
     (`automations-api.ts:163-165`).
   - Range guard: any entry day outside `entry_window` → 500 (`:166-173`; E2E
     `test:333-345`).
   - Output assembly (`:174-178`): `{automation_id, ...stats, entry_window}`.
   Side effects: one Knex existence SELECT, one Tinybird HTTP fetch, optional
   `logging.error` on Tinybird failures (`automations-api.ts:100,149`;
   `tinybird-automation-stats.ts:59,68-75,123,143,149`). No events, jobs, mail,
   webhooks, outbox writes or transactions.

7. **Cache.** No `cache` key on the controller (`automation-performance-stats.ts:6-17`),
   so no cache read/write/key/TTL in the pipeline (`pipeline.ts:410-425,436-454`).
   Response is explicitly non-invalidating (`cacheInvalidate:false`, `:9` →
   `headers.ts:135-144`) and admin app applies `Cache-Control: private`
   (`admin/app.js:24`). No cache tags/namespaces.

8. **Output.** No custom serializer for `automation_performance_stats`
   (`endpoints/utils/serializers/output/index.js:1-186`, mappers dir has no such key),
   so `serializers/handle.output` falls back to `default.all`
   (`packages/api-framework/src/serializers/handle.ts:90-109,143-179`):
   `frame.response[docName] = [response]` with no `meta` (`serializers/output/default.js:22-41`),
   then `all.after` strips `published_by` recursively (`serializers/output/all.js:6-27`).
   Final body: `{ automation_performance_stats: [{ automation_id, total_run_count,
   in_progress_run_count, completed_run_count, exited_early_run_count,
   entries: [{date,count}], entry_window: {date_from,date_to,bucket,timezone} }] }`
   (E2E `test:174-207,235-251`). Empty/missing results: never a 200 with empty data —
   no Tinybird rows or invalid response → 500 (`automations-api.ts:157-161`,
   `tinybird-automation-stats.ts:118-125`); zero-count rows are returned as
   `count:0` entries (E2E `test:254-267`); unknown automation → 404 (`:139-142`).

**C++** (exact path+verb: `GET /automations/:id/performance-stats`):

1. **Files/symbols.** Route registration
   `include/blog/handlers/membership/automations/routes.cppm:95-105`: outer
   `Router{router, "/automations"}` with `.use(content_type_middleware)` and
   `.use(require_auth)` (`:19-22`) → nested `Router{router, "/:id"}` (`:39`) →
   `Route{"/performance-stats"}.get(...)` (`:96-103`) dispatching
   `handler.automations_automations_id_performance_stats_get().handle(req,res,send)`
   (`:102`), summary `"Ghost performance-stats GET"` (`:104`). Handler
   `automations_id_performance_stats_get/handler.cppm:16-40`
   (`AutomationsIdPerformanceStatsGetHandler`, constructor `:18-22` registers a
   `LocalWorkflowDriver` and `MainFlow`). Flow
   `workflow/flow/main.cppm:7-10` (`MainFlow::build()` registers single `ExecuteStep`).
   Step `workflow/step/execute.cppm:16-134` (`ExecuteStep::invoke` `:20-68`).
   Module registration `blog_handlers/base.cppm:172`, `all.cppm:166,537,817`;
   build entries `include/blog/handlers/BUILD.bazel:547-549`.

2. **Input type/parsing.** `using Input = interfaces::io::IRequest&; using Output =
   std::map<std::string,std::string>` (`execute.cppm:18`); step name
   `automations_automations_id_performance_stats_get_execute` (`:19`).
   `path_id(in.get_path())` searches literal `"/automations/"`, takes substring up
   to next `/` (`:75-81`); `query(in.get_path())` splits on `?` then `&`, keeps only
   keys `date_from`, `date_to`, `timezone` (`:82-94`). `get_path()` includes the query
   string (`include/app/http/query.cppm:98-108`), so parsing is self-consistent, but
   values are raw/unencoded (no `%2F` decode, unlike Ghost's moment.tz normalization).
   No body parsing, no multipart, no header reads other than the bearer token
   (`middleware/bearer.cppm:27-33`).

3. **Auth + validation.** Router chain `content_type_middleware` +
   `require_auth` (`routes.cppm:20-21`). `content_type_middleware` rejects missing or
   non-`application/json` Content-Type with 415 `UnsupportedMediaTypeError`
   (`middleware/serde.cppm:90-109`, `kJsonMediaType` `:12`). `require_auth` rejects
   empty `token(request)` with 401 `UnauthorizedError` envelope
   (`middleware/auth.cppm:12-29`; `token()` at `middleware/bearer.cppm:27-33`).
   Handler repeats the token check with a different 401 body
   `{"error":"Unauthorized"}` (`handler.cppm:24-28`). Step: empty id → 400 validation
   `"id is required"`, property `id` (`execute.cppm:21-24`); invalid date format
   (strict `YYYY-MM-DD`, digits only, `valid_date` `:95-100`), `date_to` without
   `date_from`, or `date_from > date_to` → 422 validation
   `"Validation error, cannot read performance stats."`, context `"Invalid date
   window"`, property `date_from` (`:25-32`). No framework id-format check, no
   `{docName:'automations',method:'read'}` permission service, no labs flag, no
   `unsafeAttrs`. Error envelopes from `blog::api::ApiError{}.validation/.json`
   (`context_context.cppm:79-124`). 404 uses `.json("Resource not found",
   "Automation not found")` (`execute.cppm:35-37`).

4. **Status/body/header writes.** `handle()` reads step result
   `result_of("automations_automations_id_performance_stats_get_execute")`
   (`handler.cppm:29-30`), `std::any_cast<std::map<std::string,std::string>>` of
   `typed_output`, fallback `{"status","200"},{"body","{}"}` (`:31-33`), parses status
   with `std::stoi` (`:34`), writes `res.set_status(...)`, `res.set_body(bytes)`,
   `send()` (`:35-37`). 401 path writes directly and returns (`:24-28`).
   No header writes: no `Cache-Control`, no `X-Cache-Invalidate`, no
   `Content-Disposition`, no `Location`; no `Cache-Control: private` equivalent.

5. **`ctx.store()` calls.** `auto& store = s_ctx->store()` (`execute.cppm:34`;
   `context_context.cppm:1130-1138`). `automation_exists(id)` (`:105-116`) iterates
   `store.resources().list("automations")` (`:107`) and
   `store.json_for("automations", {})` (`:111`), comparing unquoted `id`
   (`unquote` `:101-104`). The `json_for("automations",{})` branch returns a
   hardcoded `AutomationWire{}` fixture (`context_context.cppm:1189-1198`) whose id is
   `serde::ghost_id(1)` (`model/model_wire_fixtures.cppm:126-136`), so that one fixed id
   always passes existence; other ids require a stored record with matching id.
   Stats read: `store.json_for("automation_runs", {{"filter[automation_id]",id}})`
   (`execute.cppm:38`) falls through to generic
   `parse_array(m_resources.list("automation_runs"))` + `search_json(records,params)`
   (`context_context.cppm:1209-1210`; `parse_array` `:611-624`; `search_json`
   `:1592-1644` applies `filter[automation_id]` at `:1603-1609`). Loop (`:42-55`)
   increments `total` per object, buckets `status` into
   `running/completed/exited` (`:46-49`), and appends one entry per run with
   `"count":1` using the run's `date` field (`:50-54`). No `core::cache::Cache` usage,
   no mutations, no outbox/fixture writes, no SQL/Tinybird calls.

6. **Output.** Payload built at `execute.cppm:57-67`:
   `{"automation_performance_stats":[{"automation_id":<id>,"total_run_count":<n>,
   "in_progress_run_count":<n>,"completed_run_count":<n>,"exited_early_run_count":<n>,
   "entries":[{date,count}...],"entry_window":{"date_from":<raw from>,"date_to":
   <raw to>,"bucket":"day","timezone":<raw zone|UTC>}}]}`. `date_from`/`date_to`
   are the raw request strings (empty `""` when omitted) — never derived from
   returned entries; `bucket` hardcoded `day`; timezone not normalized. No `meta`.
   Empty store → all counts 0, `entries:[]`, still 200.

7. **Not implemented.** Tinybird config/token/client and pipe
   `api_automation_performance_stats` (`automations-api.ts:83-103,151-156`),
   Zod row validation / duplicate-date / safe-integer / `invalid_run_count` checks
   (`tinybird-automation-stats.ts:96-152`), inclusive→exclusive date-window derivation
   and hour buckets (`automation-entry-stats.ts:62-64,93-103`), out-of-range 500
   (`automations-api.ts:166-173`), 500 when Tinybird unavailable/fails, framework
   id-format validation, permission service / 403 `NoPermissionError`, app-level
   `Cache-Control: private`, response `Cache-Control`/`X-Cache-Invalidate` handling,
   real Knex existence lookup (uses fixture/resource store instead), and real
   per-date aggregation (count fixed at 1 per run; no dedupe of dates).

**Diff:**

- **Residual gaps (Ghost behaviors with no C++ primitive here):** Tinybird client +
  token/config + HTTP pipe fetch (`automations-api.ts:83-103,144-156`;
  `tinybird-automation-stats.ts:106-152`); Zod response validation and integrity
  guards (`tinybird-automation-stats.ts:96-125,142-146`); timezone normalization and
  derived exclusive windows / hour buckets (`automation-entry-stats.ts:27-31,62-64,93-107`);
  permission service check (`automation-performance-stats.ts:12`,
  `endpoints/utils/permissions.js:18-83`); full admin auth middleware chain
  (`admin/middleware.js:110-118`); app-level `Cache-Control: private` (`admin/app.js:24`);
  real Knex `automations` existence SELECT (`database-automations-repository.ts:243-245`).
- **Divergences (C++ differs from Ghost):** stats computed from in-memory
  `automation_runs` fixture/resource store (`execute.cppm:38,42-55` via
  `context_context.cppm:1209-1210`) vs Tinybird aggregation
  (`tinybird-automation-stats.ts:126-147`); `count:1` per run and duplicate dates
  allowed (`execute.cppm:52`) vs per-date summed counts with unique-date enforcement
  (`tinybird-automation-stats.ts:118-140`); `entry_window` echoes raw/inclusive request
  dates or empty strings and hardcodes `bucket:'day'` (`execute.cppm:64-66`) vs derived
  exclusive bounds and hour/day detection (`automation-entry-stats.ts:93-103`,
  `automations-api.ts:162-165`); no timezone decode/normalization (`execute.cppm:28,82-94`)
  vs moment.tz transform (`automation-entry-stats.ts:27-31`); token-presence-only auth
  (`handler.cppm:24-28`, `middleware/auth.cppm:12-29`) vs authenticated+authorized
  Admin API with 403; extra 415 on missing/non-JSON Content-Type
  (`middleware/serde.cppm:90-109`) where Ghost GET has no such requirement
  (`admin/app.js:33-41`); hardcoded fixture id `serde::ghost_id(1)` passes existence
  (`context_context.cppm:1189-1198`, `model_wire_fixtures.cppm:128`) vs any real
  automation id (`automations-api.ts:139`); no 500 Tinybird/out-of-range errors
  (`automations-api.ts:144-161,166-173`).
- **Shared shape retained:** exact route path+verb, `automation_performance_stats`
  array envelope, 200 success body field names, 400/404/422 validation statuses,
  token-presence gate, and no cache writes/side effects. These justify PARTIAL rather
  than MISMATCH.

## Module-level notes

- `automations-api.ts` is a facade over a legacy repository (`createDatabaseAutomationsRepository`,
  `automations-api.ts:68-71`) and a transitioning `automationsApi2` (`:73-81`);
  `readPerformanceStats` lives entirely in the legacy API (`:138-179`). `getTinybirdClient`
  is shared with `browse()` and `browseRuns()` (`:83-103`), so this route's 500 path is
  triggered when that shared client factory returns null. No equivalent config/client
  exists in C++.
- `readPerformanceStats` deliberately issues no run-level SQL; the E2E suite asserts
  no `automation_runs`/`automation_run_steps`/`automation_actions` queries
  (`test/e2e-api/admin/automation-performance-stats.test.ts:48-63`). The only DB work
  is the existence SELECT (`database-automations-repository.ts:243-245`). C++ instead
  reads the `automation_runs` resource directly — the single largest semantic gap.
- Entry helpers (`parseEntryStatsOptions`, `getEntryStatsWindow`, `entryDate`) in
  `automation-entry-stats.ts` and `fetchAutomationPerformanceStats` in
  `tinybird-automation-stats.ts` are shared with run browsing / member search; C++
  has no shared equivalents, only local `path_id`/`query`/`valid_date`/`unquote`
  string helpers in `execute.cppm:75-104`.
- The C++ handler imports `:automations_dto_stats_empty_response`
  (`handler.cppm:8`, `execute.cppm:8`) but never uses it; the 200 body is assembled by
  raw string concatenation (`execute.cppm:57-66`).
- The C++ route inherits `content_type_middleware` + `require_auth` from the outer
  `/automations` router (`routes.cppm:19-22`); `bearer_middleware`
  (`middleware/bearer.cppm:11-24`) is NOT in this chain, so the token is read straight
  from the Authorization header by `token()` (`:27-33`).
- No cross-route side effects (events, jobs, mail/outbox writes, webhook calls,
  fixture writes) originate from this read-only operation on either side; the only
  observable side effects are Ghost `logging.error` calls on Tinybird failures.

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
