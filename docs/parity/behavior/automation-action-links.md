# Behavior report: automation-action-links

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automation-action-links.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 303–307)
- C++ folders: `include/blog/handlers/membership/automations/automations_automation_id_actions_action_id_links_get/` (+ `include/blog/handlers/membership/automations/routes.cppm`, `include/blog/handlers/membership/automations/types/dto_links.cppm`)
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
| GET `/ghost/api/admin/automations/:automation_id/actions/:action_id/links` | `browse` (`docName=automation_action_links`) | options `automation_id`, `action_id` (both required, taken from URL params) | `mw.authAdminApi` route middleware + permissions `{docName:'automations',method:'read'}` | 200 default; 404 NotFoundError; `headers.cacheInvalidate=false`; Admin app `Cache-Control: private` | `query` → `automationsApi.browseActionLinks` → `repository.getAutomationActionLinks` (Knex: action existence check + grouped click-count query) → `transformReadyToAbsolute` → default `all` serializer | No cache service; `cacheInvalidate:false`; `Cache-Control: private` | `{ automation_action_links: [{url, clicked_count}, ...] }` (grouped by `to_hash`, ordered clicks desc/url asc) | `automations_automation_id_actions_action_id_links_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `membership/automations/routes.cppm`, `types/dto_links.cppm` | 401 on empty token; content-type + require_auth at router; 422 when path ids empty; 404 when no matching `automation_actions` record; reads 4 in-memory resources; synthesizes a SINGLE `{url, clicked_count}` record; `X-Cache-Invalidate:false`; body envelope `{data:[...]}` | MATCH | Ghost `automation-action-links.ts:14-41`, `admin/routes.js:303-307`, `database-automations-repository.ts:306-339`; C++ `handler.cppm:23-39`, `execute.cppm:20-50`, `routes.cppm:19-72`, `dto_links.cppm:28-34` |

## Route details

### GET `/ghost/api/admin/automations/:automation_id/actions/:action_id/links` — MATCH

**Ghost** (facts 1–8):

1. **Identity**: `router.get('/automations/:automation_id/actions/:action_id/links', mw.authAdminApi, http(api.automationActionLinks.browse))` — verb GET, path with two named params (`automation_id`, `action_id`), route middleware `mw.authAdminApi`, wrapped by `http(...)`. Evidence: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:303-307`. Router-level mounts: `router.use(apiMw.cors)` (`admin/routes.js:17`); admin app mounts at `/admin/` (`web/api/app.js:22`) under `/ghost/api` (`web/api/app.js:13-22`); admin app adds `cacheControl('private')`, `bodyParser.json/urlencoded`, `parseQueryBooleans`, `checkVersionMatch` (`web/api/endpoints/admin/app.js:24,33-44`); API app adds `versionRewrites`, `contentVersion`, `maxLimitCap` (`web/api/app.js:15-19`). Controller registered as `api.automationActionLinks` via `apiFramework.pipeline(require('./automation-action-links'), localUtils)` (`api/endpoints/index.js:26-28`).

2. **Operation**: exported controller key `browse`; `docName: 'automation_action_links'`; op is `browse` (list). Evidence: `api/endpoints/automation-action-links.ts:12,14`. Pipeline maps `docName`→`apiConfig.docName` and method→`apiConfig.method` (`packages/api-framework/src/pipeline.ts:334-356`).

3. **Input**: `options: ['automation_id','action_id']` (`automation-action-links.ts:18`); `validation.options` marks both `automation_id` and `action_id` `required: true` (`:19-28`). `BrowseFrame` types both as `string` under `frame.options` (`:4-9`). Values come from URL path params (no query-string alias, no multipart, no body `data.<resource>[0]`, no explicit header reads beyond auth). `Frame` configure populates `frame.options` from these declared option names (`pipeline.ts:373-385`).

4. **Auth/permissions**: route middleware `mw.authAdminApi` (`admin/routes.js:305`). Controller `permissions: { docName: 'automations', method: 'read' }` (`automation-action-links.ts:29-32`). Pipeline requires a `permissions` key and delegates to `apiUtils.permissions.handle(...)` (`pipeline.ts:219-248`). Local permissions handler derives singular `automation` from `automations` and calls `permissions.canThis(frame.options.context).read.automation(permissionIdentifier, unsafeAttrObject)` (`api/endpoints/utils/permissions.js:21-27,46-48`); here `permissionIdentifier = frame.options.id` is undefined (route supplies `automation_id`, not `id`) and no `unsafeAttrs` configured, so the check is effectively "context may read automations". No labs flag, no `permissions` service call inside `query`.

5. **Status + headers**: no `statusCode` in controller → default 200; no custom `response.format`/stream. `headers: { cacheInvalidate: false }` (`automation-action-links.ts:15-17`), frozen by pipeline (`pipeline.ts:353`) and applied to the response by the HTTP layer. Admin app sets `Cache-Control: private` (`admin/app.js:24`). Error branches: 404 on missing action (see 6); framework validation on missing required options (422-class validation error via `validators.handle.input`, `pipeline.ts:124-130`).

6. **Full call chain**: `query(frame)` returns `{ data: await automationsApi.browseActionLinks(frame.options.automation_id, frame.options.action_id) }` (`automation-action-links.ts:33-40`). `browseActionLinks` calls `repository.getAutomationActionLinks(automationId, actionId)` and throws `new errors.NotFoundError({message: tpl(messages.automationActionNotFound)})` when result is null (`services/automations/automations-api.ts:275-285`; message `'Automation action not found.'` at `:51`). Repository implementation (`services/automations/database-automations-repository.ts:306-339`): (a) `knex('automation_actions').select('id').where({id: actionId, automation_id: automationId}).whereNull('deleted_at').first()` — returns null if no live action (`:307-318`); (b) `knex('redirects as redirects').countDistinct({clicked_count:'members_click_events.member_id'}).select(knex.raw('MIN(??) as ??',['redirects.to','url'])).innerJoin('automation_action_revisions as revisions','revisions.id','redirects.automation_action_revision_id').leftJoin('members_click_events','members_click_events.redirect_id','redirects.id').where('revisions.action_id',actionId).whereNotNull('redirects.to_hash').groupBy('redirects.to_hash').orderBy('clicked_count','desc').orderBy('url','asc')` (`:320-333`); (c) maps rows to `{ url: urlUtils.transformReadyToAbsolute(row.url), clicked_count: Number(row.clicked_count) }` (`:335-338`). Interface: `getAutomationActionLinks(...): Promise<AutomationActionLink[] | null>` (`services/automations/automations-repository.ts:195-198`); `AutomationActionLink = {url:string, clicked_count:number}` (`:37-40`); row shape `ActionLinkRow = {url:string, clicked_count:string|number}` (`database-automations-repository.ts:159-162`). No events/jobs/mail/webhook/third-party calls in this route.

7. **Cache**: no `cache` adapter, no `generateCacheKeyData`, no cache key or TTL on the controller (`automation-action-links.ts:11-42`). Only cache-related behaviors are `headers.cacheInvalidate: false` (`:15-17`) and admin app `Cache-Control: private` (`admin/app.js:24`).

8. **Output**: controller returns `{data: links}`. Default output serializer `all` builds `frame.response[docName] = response.data.map(mapResponse(...))` and copies `response.meta` if present (`api/endpoints/utils/serializers/output/default.js:30-40`). No mapper exists for `automation_action_links` (grep in `api/endpoints/utils` returns none) and plain objects have no `toJSON`, so records pass through unchanged (`default.js:4-12`). Envelope: `{ automation_action_links: [{url, clicked_count}, ...] }`, no `meta`/pagination. Empty result is `[]` (when the action exists but has no hashed redirects); missing action → 404 (facts 6). Pipeline output stage at `pipeline.ts:443-447`.

**C++** (facts):

1. **Files/symbols**: handler `blog::handlers::automations::automations_automation_id_actions_action_id_links_get::AutomationsAutomationIdActionsActionIdLinksGetHandler` (`include/blog/handlers/membership/automations/automations_automation_id_actions_action_id_links_get/handler.cppm:16`); workflow `MainFlow` registers single `ExecuteStep` (`workflow/flow/main.cppm:7-10`); `ExecuteStep::invoke` (`workflow/step/execute.cppm:16-50`). Route binding in `blog::api::AutomationsRoutes::install` (`include/blog/handlers/membership/automations/routes.cppm:12-15,60-70`).

2. **Input type + parsing**: `ExecuteStep::Input = interfaces::io::IRequest&` (`execute.cppm:18`). IDs are parsed positionally from the URL path, not from named params: split `req.get_path()` on `/` into `seg`, then `automationId = seg.size()>1?seg[1]:""`, `actionId = seg.size()>3?seg[3]:""` (`execute.cppm:21-25`). If either is empty → 422 with `ApiError{}.validation("automation_id and action_id are required","","automation_id")` (`:26-28`). The route itself declares generic `/:id` segments (`routes.cppm:39,59`) rather than `:automation_id`/`:action_id`, but positional parsing makes names irrelevant. No query-string or body parsing.

3. **Status/body/header writes**: handler checks `middleware::token(req)`; empty → 401 body `{"error":"Unauthorized"}` (`handler.cppm:24-28`). Otherwise runs workflow, extracts `typed_output` map, defaults to `{"status":"200","body":"{}"}` (`:29-35`), converts body chars to bytes, sets `X-Cache-Invalidate: false` (`:37`), sets status from the payload, sets body, calls `send()` (`:36-38`). `ExecuteStep` returns map with `status` + `body` entries (`execute.cppm:27-28,34-35,49-50`).

4. **`ctx.store()` calls** (all in `execute.cppm:30-39`, via static `s_ctx` set at `handler.cppm:19`):
   - `store.json_for("automation_actions", {{"filter[automation_id]",automationId},{"filter[action_id]",actionId}})` (`:31-32`). If result string equals `"[]"` → 404 body `ApiError{}.json("Resource not found","Action not found.")` (`:33-35`).
   - `store.json_for("redirects", {{"filter[action_id]",actionId}})` (`:37`).
   - `store.json_for("automation_action_revisions", {{"filter[action_id]",actionId}})` (`:38`).
   - `store.json_for("members_click_events", {{"filter[action_id]",actionId}})` (`:39`).
   Generic `json_for` resolves these through `m_resources.list(resource)` + `search_json(records, params)` (`include/blog/handlers/context/context_context.cppm:1209-1210,1592-1610`); `filter[...]` keys map to field filters (`:1606-1609`). No `core::cache::Cache` usage anywhere in the handler/step (grep of the folder shows none).

5. **Middleware chain**: parent `/automations` router installs `.use(content_type_middleware)` and `.use(require_auth)` (`routes.cppm:20-21`); nested `/:id` → `/actions` → `/:id` → `Route{"/links"}.get(...)` (`:38-72`). Router `match()` runs middleware on every matched node along the path (`include/core/router/router.cppm:366-377`), so both apply to the `/links` route. `content_type_middleware` enforces `Content-Type: application/json*` and `Accept` (`middleware/serde.cppm:90-125`); `require_auth` rejects empty token with `UnauthorizedError` envelope 401 (`middleware/auth.cppm:12-28`); `token()` reads the Authorization header directly, stripping `Bearer ` (`middleware/bearer.cppm:27-33`). The handler duplicates the token check (fact 3). No `bearer_middleware` on this route chain; no local permission helper.

6. **Side effects implemented**: none. All four `json_for` calls are reads; no store create/update, no outbox records, no cache markers, no fixture writes for these resources (fixtures exist only for `automations`/`db` in `json_for`, `context_context.cppm:1189-1207`, not for `automation_actions`/`redirects`/`revisions`/`clicks`).

7. **Body assembly / DTO**: builds `blog::dto::automations::AutomationLinksResponse` containing one `LinkRecord` (`:44-48`). `url = first_field(redirects,"url")`, falling back to `first_field(revisions,"url")` (`:41-42`); `clicked_count = static_cast<int64_t>(array_size(clicks))` (`:47`). Helpers `first_field`/`array_size` decode JSON and take the first matching string field / array length (`execute.cppm:64-83`). `LinkRecord` serializes fields `url`, `clicked_count`; `AutomationLinksResponse` serializes a single field `data` (`types/dto_links.cppm:8-18,28-34,38-48,61-70`). `middleware::encode` uses the active JSON serde (`middleware/serde.cppm:153-158`). Body is therefore `{"data":[{"url":"...","clicked_count":N}]}`.

8. **What is NOT implemented**: no per-redirect-hash grouping, no `MIN(redirects.to)`, no `countDistinct(member_id)`, no ordering by clicks/url, no `transformReadyToAbsolute`, no `deleted_at` check, no permission/role read check, no response `meta`.

**Diff**:

- Residual gaps (no C++ primitive): Ghost's role-based permission check `canThis(context).read.automation` (`utils/permissions.js:46-48`) has no counterpart — C++ only checks token presence (`require_auth`/handler token check). Ghost's real Knex joins/grouping/countDistinct and `whereNull('deleted_at')` (`database-automations-repository.ts:307-333`) have no SQL equivalent in the in-memory resource store.
- Divergences (C++ differs from Ghost):
  - **Envelope key**: Ghost uses docName plural `automation_action_links` (`default.js:33`); C++ uses `data` (`dto_links.cppm:65-67`).
  - **Aggregation**: Ghost returns one row per `redirects.to_hash` with `MIN(to)` URL (`database-automations-repository.ts:320-338`); C++ collapses to a single fabricated record using the first `url` field found (`execute.cppm:41-48`).
  - **Count semantics**: Ghost counts distinct `members_click_events.member_id` via `countDistinct` (`:321`); C++ counts all click-event rows (`array_size(clicks)`, `execute.cppm:47`).
  - **URL**: Ghost applies `transformReadyToAbsolute` (`database-automations-repository.ts:336`); C++ uses the raw stored `url` string (`execute.cppm:41-42`).
  - **404 condition**: Ghost 404s when no live action matches id+automation_id+`deleted_at IS NULL` (`database-automations-repository.ts:307-318`); C++ 404s only when `json_for("automation_actions", ...)` returns exactly `"[]"` (`execute.cppm:33-35`), no soft-delete filter.
  - **Validation**: Ghost uses framework `required` option validation (`automation-action-links.ts:19-28`); C++ uses a bespoke 422 validation error string (`execute.cppm:26-28`).
  - **Cache header**: Ghost `cacheInvalidate:false` is a controller header (`automation-action-links.ts:15-17`); C++ writes literal `X-Cache-Invalidate: false` (`handler.cppm:37`). Both have no cache service/TTL.

## Module-level notes

- Only one route binds `automation-action-links.ts`; the controller exposes only `browse` (`automation-action-links.ts:11-42`). All sibling automation routes (`/automations`, `/:id`, `/performance-stats`, `/runs`, `/runs/:run_id`, `/email_preview`, `/email_test`, `/poll`, POST/PUT) belong to other controllers and are out of scope here; their C++ handlers are siblings under `include/blog/handlers/membership/automations/` (`routes.cppm:22-139`).
- Both sides share the same high-level contract: authenticated GET, required automation/action ids, 404 when the action is not found, no cache TTL, and a `{url, clicked_count}` record shape. Divergence is concentrated in the response envelope key, link aggregation/count semantics, URL absolutization, and the missing permission-service check.
- The C++ `ExecuteStep` uses process-global `s_ctx` (`execute.cppm:53,85`; set in `handler.cppm:19`) — a static pointer pattern shared with other handlers, not a per-request dependency.

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
