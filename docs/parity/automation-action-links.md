# Parity: automation-action-links

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-action-links.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm`
- Checked: 2026-11-19

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
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations/:automation_id/actions/:action_id/links` | `browse` | GET `/automations/:id/actions/:id/links` (`routes.cppm:56-69`) | `automations_automations_automation_id_actions_action_id_links_get` (`routes.cppm:66`; `handler.cppm:10-30`) | Ghost: `automation_id`, `action_id` required options from path (`automation-action-links.ts:18-28`). C++: `using Input=IRequest&` but `in` unused; no path/query parsing, no validation (`execute.cppm:12-14`). | Ghost: `{data: [{url, clicked_count}...]}` real data, 404 when missing (`automation-action-links.ts:34-39`, `automations-api.ts:275-285`). C++: hardcoded `{"links":[{"id":"1","url":"https://ghost.local/link"}]}`, status 200 (`execute.cppm:16-20`; `dto/links.cppm:8-17,27-33,37-60`). | Ghost: knex queries on `automation_actions`/`redirects`/`automation_action_revisions`/`members_click_events` (`database-automations-repository.ts:306-339`). C++: `find_by_resource("automations","1")` is dead code (unreachable because lambda always returns 200) (`execute.cppm:16-24`); no filtering. | Ghost: `cacheInvalidate: false`, no invalidation (`automation-action-links.ts:15-17`). C++: none in this handler (no `core::cache::Cache` use; cache in `operations.cppm` belongs to other ops). | Ghost: `mw.authAdminApi` (`routes.js:305`) + permissions `automations`/`read` (`automation-action-links.ts:29-32`). C++: token-nonempty check → 401 if empty, plus `content_type_middleware` (`handler.cppm:18-22`; `routes.cppm:20`). | PARTIAL | Ghost `admin/routes.js:303-307`, `index.js:26-28`, `automation-action-links.ts:12-42`; C++ `membership/automations/routes.cppm:56-69`, `handler.cppm:1-31`, `execute.cppm:1-29`, `dto/links.cppm:8-60`. |

## Route details

### GET `/automations/:automation_id/actions/:action_id/links` — PARTIAL

- **Route identity / operation — agree.** Ghost `GET /automations/:automation_id/actions/:action_id/links` (`admin/routes.js:303-307`) with `http(api.automationActionLinks.browse)` (`routes.js:306`, key at `endpoints/index.js:26-28`). C++ GET path is assembled `/automations` → `/:id` → `/actions` → `/:id` → `/links` = `/automations/:id/actions/:id/links` (`membership/automations/routes.cppm:19,38,56-60`), bound to handler `automations_automations_automation_id_actions_action_id_links_get()` (`routes.cppm:66`). Placeholders normalize; verb matches. Core operation (browse action links) agrees.
- **Input — differ.** Ghost declares required options `automation_id` and `action_id` with `required: true` validation (`automation-action-links.ts:18-28`) and passes them into `browseActionLinks` (`:35-38`). C++ step declares `using Input = interfaces::io::IRequest&` (`execute.cppm:12`) but never reads `in` (`invoke(const Input& in, ...)` at `:14`); no path/query extraction and no validation — only a token check in the handler (`handler.cppm:18`). Params present only on Ghost side: `automation_id`, `action_id`.
- **Output — differ.** Ghost returns real `{ data: [...] }` with link objects `{url, clicked_count}` (`automation-action-links.ts:34-39`; `database-automations-repository.ts:335-338`) and 404 when action missing (`automations-api.ts:278-282`). C++ returns a fabricated single default `LinkRecord` — `{"links":[{"id":"1","url":"https://ghost.local/link"}]}` — with HTTP 200 (`execute.cppm:16-20`; `dto/links.cppm:15-16,27-33,37-60`; `handler.cppm:27`). Envelope key differs (`data` vs `links`), record fields differ (`url,clicked_count` vs `id,url`), values are hardcoded vs data-driven, and no 404 path exists.
- **DB call — differ.** Ghost performs filtered knex queries keyed by the two path params (`database-automations-repository.ts:306-339`). C++ calls `ctx.store().find_by_resource("automations", "1")` (`execute.cppm:22`) but this is unreachable: the lambda at `:16-20` always returns `{200, ...}`, so `if (r.first != 0)` at `:21` always early-returns. No DB resource is actually read and no filtering occurs. `auto& mutations = ctx.store()` at `:15` is unused.
- **Cache — effectively agree (none).** Ghost explicitly disables invalidation via `headers.cacheInvalidate: false` (`automation-action-links.ts:15-17`). C++ step uses no `core::cache::Cache`; the cache in `membership/automations/operations.cppm` (`:16-17,65-71`) serves other automation operations, not this handler.
- **Auth — both present, mechanism differs.** Ghost uses `mw.authAdminApi` middleware (`admin/routes.js:305`) plus permission check `docName: 'automations'`, `method: 'read'` (`automation-action-links.ts:29-32`). C++ checks only that `middleware::token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`handler.cppm:18-22`), with `content_type_middleware` in the chain (`routes.cppm:20`). No permission/docName check on C++ side.

## Missing in C++

None. The single Ghost route has a C++ counterpart (`membership/automations/routes.cppm:56-69`).

## Extra in C++

None in the matched folder relevant to this controller. The sibling routes in
`membership/automations/routes.cppm` (browse/create/read/edit automations,
email_preview, email_test, performance-stats, runs, poll) belong to other Ghost
controllers (`automations.ts`, `automation-performance-stats.ts`,
`automation-runs.ts`, `automation-run-history.ts`, `automation-email-previews.ts`)
and are not extras of `automation-action-links`.
