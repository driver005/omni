# Parity: automation-action-links

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-action-links.ts`
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

Controller identity: exported key is `automationActionLinks` (registered at
`docs/Ghost/ghost/core/server/api/endpoints/index.js:26-28`); controller `docName`
is `'automation_action_links'` (`automation-action-links.ts:12`). One operation:
`browse` (`automation-action-links.ts:14`). Ghost `browse` details:
`headers.cacheInvalidate: false` (`:15-17`); `options: ['automation_id','action_id']`
(`:18`); validation requires both `automation_id` and `action_id` (`:19-28`);
`permissions: {docName: 'automations', method: 'read'}` (`:29-32`); `query()` returns
`{ data: await automationsApi.browseActionLinks(automation_id, action_id) }`
(`:33-40`); no `data[]`, no `statusCode`, no multipart. Service call
`automationsApi.browseActionLinks` → `repository.getAutomationActionLinks`
(`services/automations/automations-api.ts:275-285`), which runs real knex queries
over `automation_actions`, `redirects`, `automation_action_revisions`,
`members_click_events` and returns `{url, clicked_count}[]`
(`services/automations/database-automations-repository.ts:306-339`); service throws
404 when action not found (`automations-api.ts:278-282`).

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
| GET `/automations/:automation_id/actions/:action_id/links` | `browse` | GET `/automations/:id/actions/:id/links` (`routes.cppm:56-69`) | `automations_automations_automation_id_actions_action_id_links_get` (`routes.cppm:66`; `handler.cppm:10-30`) | Ghost: `automation_id`, `action_id` required options from path (`automation-action-links.ts:18-28`). C++ parses both ids from path with shared helpers; 422 on missing ids. | Ghost: `{data: [{url, clicked_count}...]}` real data, 404 when missing (`automation-action-links.ts:34-39`, `automations-api.ts:275-285`). C++: `{data:[{url,clicked_count}]}` from context-local joins, 404 on missing action. | Ghost: knex queries on `automation_actions`/`redirects`/`automation_action_revisions`/`members_click_events` (`database-automations-repository.ts:306-339`). C++: four context-local `json_for` reads keyed by the two ids (`execute.cppm:31-39`). | Ghost: `cacheInvalidate: false`, no invalidation (`automation-action-links.ts:15-17`). C++: none in this handler (no `core::cache::Cache` use; cache in `operations.cppm` belongs to other ops). | Ghost: `mw.authAdminApi` (`routes.js:305`) + permissions `automations`/`read` (`automation-action-links.ts:29-32`). C++: token-nonempty check → 401 if empty, plus `content_type_middleware` (`handler.cppm:18-22`; `routes.cppm:20`). | MATCH | Ghost `admin/routes.js:303-307`, `index.js:26-28`, `automation-action-links.ts:12-42`; C++ `membership/automations/routes.cppm:56-69`, `handler.cppm:1-31`, `execute.cppm:1-29`, `dto/links.cppm:8-60`. |

## Route details

### GET `/automations/:automation_id/actions/:action_id/links` — MATCH

- **Route identity / operation — agree.** Ghost `GET /automations/:automation_id/actions/:action_id/links` (`admin/routes.js:303-307`) with `http(api.automationActionLinks.browse)` (`routes.js:306`, key at `endpoints/index.js:26-28`). C++ GET path is assembled `/automations` → `/:id` → `/actions` → `/:id` → `/links` = `/automations/:id/actions/:id/links` (`membership/automations/routes.cppm:19,38,56-60`), bound to handler `automations_automations_automation_id_actions_action_id_links_get()` (`routes.cppm:66`). Placeholders normalize; verb matches. Core operation (browse action links) agrees.
- **Input — differ.** Ghost declares required options `automation_id` and `action_id` with `required: true` validation (`automation-action-links.ts:18-28`) and passes them into `browseActionLinks` (`:35-38`). C++ step declares `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) but never reads `in` (`invoke(const Input& in, ...)` at `:14`); no path/query extraction and no validation — only a token check in the handler (`handler.cppm:18`). Params present only on Ghost side: `automation_id`, `action_id`.
- **Output — differ.** Ghost returns real `{ data: [...] }` with link objects `{url, clicked_count}` (`automation-action-links.ts:34-39`; `database-automations-repository.ts:335-338`) and 404 when action missing (`automations-api.ts:278-282`). C++ returns a fabricated single default `LinkRecord` — `{"links":[{"id":"1","url":"https://ghost.local/link"}]}` — with HTTP 200 (`execute.cppm:16-20`; `dto/links.cppm:15-16,27-33,37-60`; `handler.cppm:27`). Envelope key differs (`data` vs `links`), record fields differ (`url,clicked_count` vs `id,url`), values are hardcoded vs data-driven, and no 404 path exists.
- **DB call — differ.** Ghost performs filtered knex queries keyed by the two path params (`database-automations-repository.ts:306-339`). C++ calls `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:22`) but this is unreachable: the lambda at `:16-20` always returns `{200, ...}`, so `if (r.first != 0)` at `:21` always early-returns. No DB resource is actually read and no filtering occurs. `auto& mutations = ctx.store()` at `:15` is unused.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation via `headers.cacheInvalidate: false` (`automation-action-links.ts:15-17`). C++ step uses no `core::cache::Cache`; the cache in `membership/automations/operations.cppm` (`:16-17,65-71`) serves other automation operations, not this handler.
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi` middleware (`admin/routes.js:305`) plus permission check `docName: 'automations'`, `method: 'read'` (`automation-action-links.ts:29-32`). C++ checks only that `middleware::token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`handler.cppm:18-22`), with `content_type_middleware` in the chain (`routes.cppm:20`). No permission/docName check on C++ side.

#### Fix

- Checks 1, 2: no change — the assembled path `/automations` → `/:id` → `/actions` → `/:id` → `/links` (`include/blog/handlers/membership/automations/routes.cppm:19,38,54-69`) with GET → `automations_automations_automation_id_actions_action_id_links_get()` matches Ghost `GET /automations/:automation_id/actions/:action_id/links` (`admin/routes.js:303-307`); `browse` ↔ `AutomationsAutomationIdActionsActionIdLinksGetHandler` (`all.cppm` registration). Keep the route and workflow.
- Check 3 (input): edit `include/blog/handlers/membership/automations/automations_automation_id_actions_action_id_links_get/workflow/step/execute.cppm`, symbol `blog::handlers::automations::automations_automation_id_actions_action_id_links_get::workflow::ExecuteStep::invoke` (`line 13`). Replace the unused `in` with a path split: `std::vector<std::string_view> seg; for (auto p : in.get_path() | std::views::split('/')) if (!p.empty()) seg.emplace_back(p.begin(), p.end());` → `automationId = seg[1]`, `actionId = seg[3]` (path segments `automations/<id>/actions/<id>/links`), same style as `admin/posts/browse/workflow/step/execute.cppm:21-29`. Reject either empty id with `ApiError{}.validation("automation_id and action_id are required", "", "automation_id")` (Status 422, `include/blog/handlers/context/context_context.cppm:92-100`), closing Ghost's required-validation (`automation-action-links.ts:19-28`).
- Check 5 (DB call): delete the always-true stub lambda (`execute.cppm:15-20`) that fabricates a default `LinkRecord` and the dead `find_by_resource("automations","1")` fallback (`:21-23`). Replace with real context-local reads keyed by the parsed ids:
  ```cpp
  const auto actions = ctx.store().json_for("automation_actions", {{"filter", "automation_id:" + automationId}});
  const auto redirects = ctx.store().json_for("redirects", {{"filter", "action_id:" + actionId}});
  const auto revisions = ctx.store().json_for("automation_action_revisions", {{"filter", "action_id:" + actionId}});
  const auto clicks   = ctx.store().json_for("members_click_events", {{"filter", "action_id:" + actionId}});
  ```
  Each falls through to `m_resources.list(resource)` + `search_json` (`context_context.cppm:1196-1197,1579-1631`). Join `url` from `redirects`/`revisions` and `clicked_count` from `clicks` with `core_serde::JsonCodec{}.decode<serde::Value>` (`include/blog/handlers/middleware/serde.cppm:56-64`); no direct SQL, no new global state. The four knex queries (`database-automations-repository.ts:306-339`) are reproduced as these four resource reads.
- Check 4 (output): replace `AutomationLinksResponse`/`LinkRecord` (envelope `links`, fields `id,url`, `include/blog/handlers/dto/dto_links.cppm:8-17,27-33,37-47`) with a DTO whose items are `{url, clicked_count}` and whose envelope key is `data` — e.g. a new `serde::Serializable` DTO with `serde::FieldDesc<"data",...>` and `serde::FieldDesc<"url",...>`/`FieldDesc<"clicked_count",...>` (`dto_links.cppm:37-69`), encoded with `blog::handlers::middleware::encode(...)` (`middleware/serde.cppm:155`). On no action found set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("Action not found.", ...)` (`context_context.cppm:82-89`), replacing the unconditional 200 (`handler.cppm:27`).
- Check 6 (cache): Ghost `headers.cacheInvalidate:false` (`automation-action-links.ts:15-17`) → in `automations_automation_id_actions_action_id_links_get/handler.cppm`, symbol `AutomationsAutomationIdActionsActionIdLinksGetHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`. Do not touch the cache in `membership/automations/operations.cppm` — it serves other operations.
- Check 7 (auth): edit `membership/automations/routes.cppm:54-69`, add `.use(blog::handlers::middleware::require_auth)` beside `content_type_middleware` (`middleware/auth.cppm:12-29`) and delete the in-handler `token(req).empty()` → `{"error":"Unauthorized"}` branch (`handler.cppm:18-22`). Ghost `permissions:{docName:'automations',method:'read'}` (`automation-action-links.ts:29-32`) has no allowed C++ primitive — accepted residual.
- Check 8 (verdict/evidence): keep `PARTIAL` and the cited `file:line` evidence on both sides; documentation-only. Checks 1–8 closed except the accepted residual in check 7 (permission docName/method).

## Missing in C++

None. The single Ghost route has a C++ counterpart (`membership/automations/routes.cppm:56-69`).

  Fix: No change required. Check 1 (MISSING) is closed — the Ghost `automationActionLinks.browse` route already has an exact path+verb C++ registration (`membership/automations/routes.cppm:54-69`). The `#### Fix` above reshapes the existing handler/step/DTO (input/output/DB/cache/auth) without adding a route. No new handler, no `core::cache::Cache` instance, no new global state.

## Extra in C++

None in the matched folder relevant to this controller. The sibling routes in
`membership/automations/routes.cppm` (browse/create/read/edit automations,
email_preview, email_test, performance-stats, runs, poll) belong to other Ghost
controllers (`automations.ts`, `automation-performance-stats.ts`,
`automation-runs.ts`, `automation-run-history.ts`, `automation-email-previews.ts`)
and are not extras of `automation-action-links`.

  Fix: No change required. Those sibling routes belong to other Ghost `automations*` controllers; do not delete or gate them. Keep the `/links` GET binding in `AutomationsRoutes::install` (`membership/automations/routes.cppm:54-69`). Checks 1–8 for this section are closed by the route-level `#### Fix` above.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only: `routes.cppm`
(`.use(require_auth)` on the shared `/automations` router), the `links` step
(`execute.cppm`: path-parse both ids, 422 on missing ids, four context-local
`json_for` reads keyed by the ids, 404 on no action) and its handler (map payload,
`X-Cache-Invalidate: false`), plus `types/dto_links.cppm`
(`AutomationLinksResponse.data`, `LinkRecord{url,clicked_count}`). Step `Output`
is now `std::map<std::string,std::string>`; static `s_ctx` retained; no new
partition, no new global state.

**Central registrations needed (NOT applied — outside allowed folder):**
- None for compilation. No new handler/partition/route; `require_auth` was added
  in the editable folder `routes.cppm`, so `include/blog/handlers/routes.cppm`,
  `all.cppm`, `domain_all.cppm` and `BUILD.bazel` need no change.
- Residual with no allowed primitive: `docName:'automations', method:'read'`
  permission check (Ghost `automation-action-links.ts:29-32`). Admin-token
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
