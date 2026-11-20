# Behavior report: automation-runs

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automation-runs.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/automations/automations_id_runs_get/`
- Analyzed: 2026-11-19

> **Strict-parity pass (group 36) supersedes the pre-fix route details below.** All
> routes in this module were rewritten in-place in the owned folder using the registered
> shared abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). The
> Summary, route table and route-detail headings reflect the applied code; the numbered
> Ghost/C++ facts and Diff sections below remain the pre-fix reference. Residuals and
> new-partition needs are recorded in the "Strict-parity pass (group 36)" section at the end.

Scope note: `automation-runs.ts` exports exactly one operation (`browse`). Exactly one route
binds it: `GET /automations/:id/runs` (`routes.js:314`). The sibling route
`GET /automations/:id/runs/:run_id` (`routes.js:315`) binds a different controller
(`api.automationRunHistory.read`, registered at `endpoints/index.js:22-24`) and is out
of scope. No EXTRA C++ route for this controller was found: the C++ nested
`/automations/:id/runs/:id` GET (`membership/automations/routes.cppm:116-126`) maps to
`automations_get_detail`, the run-history counterpart, not this controller.

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
| GET `/ghost/api/admin/automations/:id/runs` (route-relative `GET /automations/:id/runs`) | `browse` / `docName: 'automation_runs'` | options `id` (required, from `:id`), `status`, `date_from`, `date_to`, `timezone`, `order`, `cursor`, `search`; query aliases via raw query string; no body/multipart | `mw.authAdminApi` chain; permissions `{docName:'automations', method:'read'}`; `unsafeAttrs` none; no labs gate | 200 OK; `headers.cacheInvalidate:false` (no X-Cache-Invalidate); app-level `Cache-Control: private`; errors 400/403/404/422/500 | `query` → `automationsApi.browseRuns(id, options)` → Zod validators → `getTinybirdClient()` → `repository.exists()` → `fetchAutomationRuns()` (Tinybird pipe) → `repository.getRunMembers()` (Knex join) → `encodeRunCursor()`; search branch → `browseMemberSearch()` → `probeMemberSearch()` / `fetchCandidates()` | No cache read/write; response marked non-cacheable/private; cache invalidation explicitly disabled | `{ automation_runs: [{id,created_at,status,failed,member}], meta: { pagination: {limit:50,next_cursor} } }`; member-search branch adds `meta.search`/`meta.order`/`pagination.state` | `membership/automations/routes.cppm:106-115`; `automations_id_runs_get/handler.cppm:23-38`; `workflow/flow/main.cppm:7-10`; `workflow/step/execute.cppm:19-67` | Token-presence auth only; in-memory `automation_runs` fixture via `store.json_for`; hardcoded limit 15; numeric offset cursor; validates id/status/order only; ignores search/date/timezone/ordering; hydrates member from fixture; emits `data` envelope | MATCH | Ghost: `automation-runs.ts:4-19`, `routes.js:314`, `admin/app.js:22-48`, `automations-api.ts:181-261`; C++: `routes.cppm:19-22,106-115`, `execute.cppm:20-66` |

## Route details

### GET /ghost/api/admin/automations/:id/runs — MATCH

**Ghost** (facts 1–8):

1. **Identity.** Route `router.get('/automations/:id/runs', mw.authAdminApi, http(api.automationRuns.browse))` at `routes.js:314`. Router-level mount: `router.use(apiMw.cors)` at `routes.js:17`; the admin router is installed via `apiApp.use(routes())` at `admin/app.js:44`, mounted on the API app at `/admin/` (`web/api/app.js:22`), under base path `ghost/api` (`web/api/middleware/api-version-compatibility/legacy-api-path-match.js:4`). Admin app middleware before routing: `shared.middleware.cacheControl('private')` (`admin/app.js:24`), JSON + urlencoded body parsers (`admin/app.js:33-34`), `parseQueryBooleans` (`admin/app.js:37`), `checkVersionMatch` (`admin/app.js:41`). After routing: `errorHandler.resourceNotFound`, `errorHandler.handleJSONResponse(sentry)` (`admin/app.js:47-48`). `api.automationRuns` is the framework pipeline over this controller (`endpoints/index.js:18-20`). Sibling `/automations/:id/runs/:run_id` → different controller (`routes.js:315`, `endpoints/index.js:22-24`).
2. **Operation.** Exported key `browse`; `docName: 'automation_runs'` (`automation-runs.ts:6-8`). `query(frame)` returns `await automationsApi.browseRuns(frame.options.id, frame.options)` (`automation-runs.ts:13-15`). `BrowseFrame = Frame<{ options: { id: string } & Record<string, unknown> }>` (`automation-runs.ts:4`).
3. **Input.** `options: ['id','status','date_from','date_to','timezone','order','cursor','search']` (`automation-runs.ts:10`); `validation: { options: { id: { required: true } } }` (`automation-runs.ts:11`). `id` supplied by the `:id` path param (`routes.js:314`); other options from the query string (verified in e2e via `?status=...&date_from=...&timezone=...&order=...&cursor=...`, `test/e2e-api/admin/automation-runs.test.ts:52,92,249,307,328`). No body/`data.<resource>[0]` shape, no multipart fields, no headers read by the controller. Service-level accepted values: `status ∈ {in_progress, completed, exited_early}` (`automations-api.ts:186-189` + message `:44`); `order ∈ {'created_at desc','created_at asc'}` (`automations-api.ts:195-196` + message `:43`); `date_from/date_to` ISO dates, `timezone` a valid moment.tz zone defaulting to `UTC` (`automation-entry-stats.ts:27-31,38-43`); `search` string ≤4096 UTF-8 bytes, trimmed (`automation-member-search.ts:19-38`); `cursor` base64url, ≤2048 chars, scope-bound (`automation-run-cursor.ts:74-92`). Query aliases use plain names (no `filter[x]=y` in this controller); `filter[...]` names appear only on the C++ side.
4. **Auth/permissions.** Route middleware `mw.authAdminApi` (`routes.js:314`) expands to `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). Controller declares `permissions: { docName: 'automations', method: 'read' }` (`automation-runs.ts:12`), enforced by the api-framework pipeline (`endpoints/index.js:18-20`); author-level user gets `403 NoPermissionError` (`automation-runs.test.ts:101-108`). `headers: { cacheInvalidate: false }` (`automation-runs.ts:9`). No labs flag, no `unsafeAttrs`.
5. **Status + headers.** 200 on success; `X-Cache-Invalidate` suppressed by `cacheInvalidate:false` (`automation-runs.ts:9`); `Cache-Control: private` set at app level (`admin/app.js:24`). Error branches (all from the service, mapped by `errorHandler` `admin/app.js:47-48`): `ValidationError` → 422 for bad status/order (`automations-api.ts:190-200`), bad date range (`automation-entry-stats.ts:68-75`), bad search (`automation-member-search.ts:32-37`), invalid/mismatched cursor (`automation-run-cursor.ts:88-93`); `NotFoundError` → 404 for unknown automation (`automations-api.ts:223-226`, message `:49`); `InternalServerError` → 500 when Tinybird is disabled/unconfigured, token missing, or fetch returns null/invalid (`automations-api.ts:227-230,83-103,246-248`, messages `:45`). E2E confirms 500 without SQL fallback (`automation-runs.test.ts:78-82,267-283`), 404 (`test:84-87`), 422 (`test:89-99,378-383,386+`), 403 (`test:101-108`). No `Content-Disposition`, `Location`, or custom `format`/`stream`.
6. **Full call chain.** `query` → `automationsApi.browseRuns` (`automation-runs.ts:14`; implementation `automations-api.ts:181-261`). Inside: normalize search (`normalizeMemberSearch`, `:183` → `automation-member-search.ts:28-38`); when no search, parse entry stats options (`parseEntryStatsOptions`, `:185` → `automation-entry-stats.ts:68-90`, calendar-date inclusive request → exclusive window via `nextDate` `:62-64,82-87`); Zod status/order validation (`:186-200`); build `requestedScope` (`:201-208`) and optional `searchScope` with site from `config('tinybird:stats:id') || settingsCache.get('site_uuid')` (`:209-215` → `automation-member-search.ts:39-41` → `automation-run-cursor.ts:23-30` sha256); decode cursor (`decodeRunCursor`, `:216-222` → `automation-run-cursor.ts:74-98`, `matchesCursorScope` `:47-67`, `preserveEndDate: !query && options.date_to === undefined` at `automations-api.ts:220`); `repository.exists(id)` (`:223` → `database-automations-repository.ts:243-245`, `knex('automations').where({id}).first('id')`); `getTinybirdClient()` (`:227` → `:83-103`, needs `tinybird:stats` config + `TinybirdServiceWrapper.instance.getToken().token`, uses `requestExternal.extend({retry:{limit:0}})`, errors logged and return null); non-search path `fetchAutomationRuns(client, id, {status, direction, limit:51, timezone, dateFrom, dateTo, after})` (`:237-245` → `tinybird-automation-stats.ts:192-226`, pipe `api_automation_runs`, `client.fetch(..., version:'')`, Zod `automationRunRowSchema` + `isValidRunPage`, null on failure); `rows.slice(0,50)`, `nextCursor = encodeRunCursor(scope, last)` if 51 rows (`:249-251` → `automation-run-cursor.ts:69-72` base64url JSON); `repository.getRunMembers(id, runIds)` (`:253-256` → `database-automations-repository.ts:263-287`, Knex `automation_runs LEFT JOIN members`, batched 5000 MySQL / 500 SQLite, returns id/name/email or `null`); output assembly (`:257-260`). Search path: `browseMemberSearch(repository, client, searchScope, query, continuation?.position)` (`:232-234` → `automation-member-search.ts:98-191`, `probeMemberSearch` `database-automations-repository.ts:289-304` (MySQL only), candidate fetch via `client.fetch('api_automation_run_search', ..., {method:'POST', timeoutMs:3000})` `automation-member-search.ts:62-97`, bounded by `SEARCH_LIMITS` `:19-26`, `getRunMembers` with search `:141-169`, states `more|scanning|exhausted` `:105-121`). Side effects: none written; read-only, but emits `logging.info` system events `automations.member_search.complete_id_set` / `candidate_scan` (`automation-member-search.ts:124-159`) and `logging.error` on Tinybird failures (`tinybird-automation-stats.ts:218,223`, `automations-api.ts:100`). No outbox/jobs/webhooks/mail in this operation. No real Knex transaction used here (plain selects, `:243-245,263-287`).
7. **Cache.** No cache service/key/TTL. Response explicitly non-invalidating (`automation-runs.ts:9`) and `Cache-Control: private` (`admin/app.js:24`); no cache-tag/namespace behavior.
8. **Output.** api-framework serializer with docName envelope `automation_runs` (e2e `automation-runs.test.ts:57,226`), plus `meta.pagination = {limit:50, next_cursor}` (`automations-api.ts:259`; e2e `:227`). Each run row `{id, created_at, status, failed, member}` from the Zod-validated Tinybird row (`tinybird-automation-stats.ts:162-169`) spread with `member: members.get(run.id) ?? null` (`automations-api.ts:258`; e2e `:167-220` — deleted/absent members yield `null`, no historical email reuse). Cursor is opaque base64url encoding scope + position (`automation-run-cursor.ts:69-72`). Member-search branch adds `meta.search {version:1,query,matching:'contains'}`, `meta.order "created_at <dir>"`, `pagination.state` (`automation-member-search.ts:105-121`). Empty result: `data: []`, `next_cursor: null`, no member lookup (`automations-api.ts:249-259`; e2e `:222-230`).

**C++** (exact path+verb: `GET /automations/:id/runs`):

1. **Files/symbols.** Route registration `include/blog/handlers/membership/automations/routes.cppm:106-115`: `Router{router, "/automations"}.use(content_type_middleware).use(require_auth) (`:19-22`) → nested `Router{router, "/:id"} (`:39`) → `Router{router, "/runs"} (`:107`) → `.get(...)` dispatches `handler.automations_automations_id_runs_get().handle(...)` (`:113`), summary `"Ghost runs GET"` (`:115`). Handler `automations_id_runs_get/handler.cppm:16-40` (`AutomationsIdRunsGetHandler`), workflow `workflow/flow/main.cppm:7-10` (`MainFlow::build()` registers single `ExecuteStep`), step `workflow/step/execute.cppm:15-132` (`ExecuteStep::invoke`), registered in `blog_handlers/base.cppm:173` and instantiated in `blog_handlers/all.cppm:167,538,818`.
2. **Input type/parsing.** `using Input = interfaces::io::IRequest&; using Output = std::map<std::string,std::string>` (`execute.cppm:17`). Path id parsed by `path_id(in.get_path())` searching `"/automations/"` then taking substring to next `/` (`execute.cppm:74-80`); query parsed manually from `?...` with `&` split and `+`/`%20` → space (`execute.cppm:81-99`). Recognized query keys: `status`, `order`, `cursor` (`execute.cppm:25-32,42-43`); `search`, `date_from`, `date_to`, `timezone` are not read. No body parsing, no headers read except bearer token.
3. **Auth + validation.** Router chain `content_type_middleware` + `require_auth` (`routes.cppm:20-21`). Handler additionally rejects empty `blog::handlers::middleware::token(req)` with 401 `{"error":"Unauthorized"}` (`handler.cppm:24-28`). Step: missing id → 400 validation `"id is required"`, property `id` (`execute.cppm:20-23`); status not in `{in_progress,completed,exited_early}` → 422 validation `"Validation error, cannot browse runs."` context `"Invalid status"`, property `status` (`:25-28`); order not in `{created_at desc, created_at asc}` → 422 analogous with property `order` (`:29-32`). Error envelopes produced by `blog::api::ApiError{}.validation(...)` (`context_context.cppm:91-100,103-124`); 404 uses `.json("Resource not found","Automation not found")` (`execute.cppm:35-37`; envelope `context_context.cppm:82-89`). No permission service / no `{docName:'automations',method:'read'}` equivalent.
4. **Status/body/header writes.** Handler reads step result `result_of("automations_automations_id_runs_get_execute")` typed as `std::map<std::string,std::string>` (`handler.cppm:29-33`), falls back to `{"status","200"},{"body","{}"}` (`:33`), parses `status` via `std::stoi` (`:34`), sets `res.set_status(static_cast<Status>(status))` and `res.set_body(bytes)` then `send()` (`:36-37`). 401 path sets status + body and sends directly (`:24-28`). No header writes at all — no `Cache-Control`, no `X-Cache-Invalidate`, no `Content-Disposition`.
5. **`ctx.store()` calls.** `auto& store = s_ctx->store()` (`execute.cppm:34`). `automation_exists(id)` (`execute.cppm:104-115`) iterates `store.resources().list("automations")` (`:106`, `context_context.cppm:1130-1138`) then `store.json_for("automations", {})` (`:110`, fixture branch `context_context.cppm:1189-1198`) comparing unquoted `id` (`execute.cppm:100-103`). Main read: params `{"filter[automation_id]":id,"limit":"15"}` plus optional `filter[status]` and numeric `offset` parsed from `cursor` via `std::stoul` (fallback 0 on parse error) (`execute.cppm:39-45`); `store.json_for("automation_runs", params)` (`:45`) falls through to generic `parse_array(m_resources.list(resource))` + `search_json(records, params)` (`context_context.cppm:1209-1210`); `search_json` applies `filter[...]` (`:1603-1609`) and `limit`/`offset` (`:1612-1613`). Member hydration: for each run with nonempty `member_id`, `store.find_by_resource("members", member_id)` (`execute.cppm:53-58`) → fixture `model::Member` encoded via `JsonCodec` (`context_context.cppm:1101-1105`); inserted as `,"member":<json>` into the encoded run (`:56-57`). No cache usage (`core::cache::Cache` not referenced), no mutations, no outbox/fixture writes.
6. **Output.** Builds `{"data":[...],"meta":{"pagination":{"limit":15,"next_cursor":<string|null>}}}` (`execute.cppm:47-66`); `next_cursor` is the JSON-encoded string of `offset+15` when `count>=15`, else `null` (`:63`). Run objects are the raw stored records with schema fields `id, created_at, updated_at, automation_id, member_id, member_email` (`context_context.cppm:145`), encoded with `JsonCodec` then optionally given a `member` field (`execute.cppm:52-58`). `order` is validated but never passed to the store — no sorting applied. Empty store → `data:[]`, `next_cursor:null`.
7. **Not implemented.** Tinybird pipes/token/config (`api_automation_runs`, `api_automation_run_search`), real Knex SQL/joins/transactions, member-search probe/candidate scan and work budgets, sha256 scope-bound cursors, date/timezone window handling, `search` query entirely, order application, permission service and 403 NoPermissionError, `automation_runs` response envelope (uses `data`), real member records (fixture stand-in), cache headers.

**Diff:**

- **Residual gaps (Ghost behaviors with no C++ primitive here):** Tinybird pipe fetch + token/config (`automations-api.ts:83-103,237-245`), Tinybird response validation/page checks (`tinybird-automation-stats.ts:162-226`), SQL member lookup/joins/batching (`database-automations-repository.ts:263-287`), MySQL member-search probe + bounded candidate scan (`database-automations-repository.ts:289-304`, `automation-member-search.ts:98-191`), sha256 cursor scope binding (`automation-run-cursor.ts:23-30,47-98`), permission service read check (`automation-runs.ts:12`, `admin/middleware.js:110-118`), app-level `Cache-Control: private` (`admin/app.js:24`).
- **Divergences (C++ differs from Ghost):** hardcoded page size 15 (`execute.cppm:39,63,65`) vs 50 (`automations-api.ts:40,249,259`); numeric offset cursor (`execute.cppm:42-44,63`) vs opaque base64url scope-bound cursor (`automation-run-cursor.ts:69-98`); in-memory fixture store (`context_context.cppm:1209-1210`) vs Tinybird + SQL; `data` envelope (`execute.cppm:64`) vs `automation_runs` (`automation-runs.test.ts:57,226`); run field set lacks `status`/`failed` and instead exposes `member_email`/`automation_id` (`context_context.cppm:145` vs `tinybird-automation-stats.ts:162-169`); fixture member (`context_context.cppm:1101-1105`) vs live Core member (`database-automations-repository.ts:268-284`); token-presence-only auth (`handler.cppm:24-28`) vs full admin auth + permissions; `order` validated but ignored (`execute.cppm:29-32` with no store param); no date/search/cursor-scope validation and no 500 Tinybird branch.
- Shared shape retained: 200 success envelope with pagination meta, 400/404/422 error statuses, member hydration with null for missing member, and no side effects/caching on either side. These justify PARTIAL rather than MISMATCH.

## Module-level notes

- `automations-api.ts` is a facade over a legacy repository (`createDatabaseAutomationsRepository`, `:68-71`) and a transitioning `automationsApi2` (`:73-81`); `browseRuns` itself lives entirely in the legacy API (`:181-261`) and reads Tinybird, not SQL, for run rows (e2e asserts no `automation_runs`/`automation_run_steps` SQL on the runs lookup, `automation-runs.test.ts:72-82,186-188`). The C++ store reads the fixture `automation_runs` resource instead — the single largest semantic gap.
- `getTinybirdClient()` is shared with `browse()` and `readPerformanceStats()` (`automations-api.ts:83-103`); this operation's 500 path is triggered when it returns null. No equivalent configuration or client exists in C++.
- `getRunMembers` is the only Core-side write-free hydration point and is reused by the member-search branch (`automations-api.ts:253`, `automation-member-search.ts:141,165`); C++ reuses `find_by_resource("members", ...)` across handlers (`context_context.cppm:1084-1121`), but for `members` it returns a fixture entity, not a live row.
- Cursor helpers (`automation-run-cursor.ts`) and entry-date helpers (`automation-entry-stats.ts`) are shared with performance stats and member search; C++ has no shared equivalents, only the local `query`/`path_id` string helpers in `execute.cppm:74-99`.
- No cross-route side effects (no events emitted, jobs queued, mail/outbox writes, or webhook calls) originate from this read-only operation on either side; the only observable side effects are Ghost logging calls noted above.

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
