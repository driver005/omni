# Parity: automations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automations.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (folder found by exact path+verb, not assumed)
- Checked: 2026-11-19

Scope: Ghost controller exports keys `browse`, `read`, `add`, `edit`, `poll` (docName `automations`, `automations.ts:38-105`). Only routes binding `api.automations.<key>` are in scope: `routes.js:302,308,327,328,329`. The `/automations/...` routes at `routes.js:303-326` bind other controllers (`automationActionLinks`, `automationPerformanceStats`, `automationRuns`, `automationRunHistory`, `automationEmailPreviews`) and are out of scope for this file.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 4 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations` | `browse` | GET `/automations` | `automations_browse()` | Ghost: none; C++: none (no query parse) | Ghost: `automationsApi.browse()` result; C++: `{"automations":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` — hardcoded meta, total always 0 | Ghost: service `automationsApi.browse()` (`automations.ts:53`); C++: `json_for("automations", {})`, no filtering (`browse/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`automations.ts:42-44`); C++ none | Ghost `mw.authAdminApi` (`routes.js:302`) + `permissions:true` + labs flag (`automations.ts:45,47-51`); C++ `content_type_middleware` + in-handler non-empty `token(req)` check only, 401 `Unauthorized` (`browse/handler.cppm:18-22`) | PARTIAL | Ghost `routes.js:302`, `automations.ts:41-55`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-17` |
| GET `/automations/:id` | `read` | GET `/automations/:id` | `automations_read_by_id()` | Ghost `data:['id']` (`automations.ts:61`), reads `frame.data.id` (`automations.ts:64`); C++ ignores route param, `Input=IRequest&`, no param parse | Ghost: serializer for `automationsApi.read(id)`; C++: `{"automations":[<one>]}` or `{"automations":[]}` — envelope shape agrees, but record lookup is wrong | Ghost `automationsApi.read(frame.data.id)` (`automations.ts:64`); C++ `find_by_resource("automations", "1")` — id HARDCODED to `"1"` (`read_by_id/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`automations.ts:58-60`); C++ none | Ghost `mw.authAdminApi` (`routes.js:308`) + `permissions:true` (`automations.ts:62`); C++ token-only check (`read_by_id/handler.cppm:18-22`) | PARTIAL | Ghost `routes.js:308`, `automations.ts:57-66`; C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-18` |
| POST `/automations` | `add` | POST `/automations` | `automations_create()` | Ghost typed `frame.data` = `AddAutomationData` (`automations.ts:13,74-77`); C++ raw `in.body_as_string()` (defaults `"{}"`), no DTO/validation (`create/.../execute.cppm:16`) | Ghost `statusCode:201` (`automations.ts:69`); C++ handler sets `Status::OK` (200) (`create/handler.cppm:27`). C++ returns raw `mutations.create(...).body()` | Ghost `automationsApi.add(frame.data)` + labs gate `assertCanAddAutomations()` + soft cap `assertNotAddingTooManyAutomations()` (`automations.ts:19-36,75-77`); C++ `mutations.create("automations", body)` — resource agrees; no labs gate, no MAX_AUTOMATIONS/HostLimitError check | Ghost `cacheInvalidate:false` (`automations.ts:70-72`); C++ none | Ghost `mw.authAdminApi` (`routes.js:328`) + `permissions:true` + integration-token allowlist (`middleware.js:72`); C++ token-only check (`create/handler.cppm:18-22`) | PARTIAL | Ghost `routes.js:328`, `automations.ts:68-79`; C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/workflow/step/execute.cppm:11-16` |
| PUT `/automations/:id` | `edit` | PUT `/automations/:id` | `automations_edit()` | Ghost `options:['id']` + typed `EditAutomationData`, reads `frame.options.id` (`automations.ts:14-17,85-88`); C++ reads raw body only, passes EMPTY string as id (`edit/.../execute.cppm:16`) — `:id` route param ignored | Ghost serializer, default 200; C++ `Status::OK` (200), raw `mutations.edit(...).body()` | Ghost `automationsApi.edit(frame.options.id, frame.data)` (`automations.ts:88`); C++ `mutations.edit("automations", "", body)` — resource agrees, id empty (`edit/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`automations.ts:82-84`); C++ none | Ghost `mw.authAdminApi` (`routes.js:329`) + `permissions:true` + integration-token allowlist (`middleware.js:72`); C++ token-only check (`edit/handler.cppm:18-22`) | PARTIAL | Ghost `routes.js:329`, `automations.ts:81-90`; C++ `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/workflow/step/execute.cppm:11-16` |
| PUT `/automations/poll` | `poll` | PUT `/automations/poll` | `automations_automations_poll_put()` | Ghost: no input, `query()` calls `requestPoll()` (`automations.ts:101-103`); C++ reads raw body (defaults `"{}"`) (`automations_poll_put/.../execute.cppm:16`) | Ghost `statusCode:204`, no body (`automations.ts:93`); C++ `Status::OK` (200) with DB-edit body (`automations_poll_put/handler.cppm:27`) — materially different | Ghost `automationsApi.requestPoll()` side-effect, not a model write (`automations.ts:102`); C++ `mutations.edit("automations", "", body)` — turns poll into an edit with empty id (`automations_poll_put/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`automations.ts:94-96`); C++ none (namespace `automations:test_emails_sent` in `operations.cppm:65-71` is used only by `email_test`, not this route) | Ghost `mw.authAdminApiWithUrl` (`routes.js:327`) + explicit permission `{docName:'automations',method:'poll'}` (`automations.ts:97-100`); C++ token-only check (`automations_poll_put/handler.cppm:18-22`) | MISMATCH | Ghost `routes.js:327`, `automations.ts:92-104`; C++ `routes.cppm:128-137`, `automations_poll_put/handler.cppm:17-27`, `automations_poll_put/workflow/step/execute.cppm:11-16` |

## Route details

### GET `/automations` — PARTIAL
- Output: Ghost returns `automationsApi.browse()` via api-framework serializer with no fabricated meta (`automations.ts:53`); C++ wraps rows with hardcoded pagination `limit:15, page:1, pages:1, total:0` and `prev/next:null` regardless of data (`browse/workflow/step/execute.cppm:16-17`). `total:0` is emitted even when rows exist.
- Auth: Ghost requires `mw.authAdminApi` (`routes.js:302`), `permissions:true` (`automations.ts:45`), and the `labs.automations` flag — throws `NotFoundError` when off (`automations.ts:47-51`). C++ only checks `token(req)` is non-empty and returns 401 `{"error":"Unauthorized"}` otherwise (`browse/handler.cppm:18-22`); no labs gate, no method/docName permission.
- Cache: Ghost explicitly sets `headers.cacheInvalidate:false` (`automations.ts:42-44`); C++ step/handler has no cache access — none on either side.
- Input, DB resource: no params on either side; both read the `automations` resource (service vs `json_for`). No differences there.

### GET `/automations/:id` — PARTIAL
- Input/DB: Ghost reads `frame.data.id` (`automations.ts:64`); C++ `find_by_resource("automations", std::string{}.empty()?"1":std::string{})` always resolves to id `"1"` and never parses the `:id` route param (`read_by_id/workflow/step/execute.cppm:16`). Request always returns record "1" (or empty array), not the requested automation.
- Output: C++ envelope `{"automations":[...]}` (`read_by_id/.../execute.cppm:17-18`) matches Ghost's list-style serializer shape; data content diverges because of the hardcoded id.
- Auth: same Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:308`, `automations.ts:62`) vs C++ token-only (`read_by_id/handler.cppm:18-22`); no labs gate on either for read.
- Cache: none on either side (`automations.ts:58-60`).

### POST `/automations` — PARTIAL
- Status: Ghost declares `statusCode:201` (`automations.ts:69`); C++ handler unconditionally sets `Status::OK` = 200 (`create/handler.cppm:27`).
- Input/validation: Ghost uses typed `AddAutomationData` and enforces two guards — labs flag `assertCanAddAutomations()` → `NotFoundError` (`automations.ts:19-23,75`) and soft cap `assertNotAddingTooManyAutomations()` using `automationsApi.getNumberOfAutomations()` vs `MAX_AUTOMATIONS` → `HostLimitError` (`automations.ts:25-36,76`). C++ passes raw `body_as_string()` straight to `mutations.create` with no DTO, no labs gate, no count/limit check (`create/workflow/step/execute.cppm:16`).
- DB: resource string `automations` agrees (`automations.ts:77` service vs `create/.../execute.cppm:16` mutation).
- Auth: Ghost `mw.authAdminApi` (`routes.js:328`), `permissions:true`, integration-token allowlist includes `automations: ['POST','PUT']` (`middleware.js:72`); C++ token-only (`create/handler.cppm:18-22`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:70-72`); C++ none.

### PUT `/automations/:id` — PARTIAL
- Input/DB: Ghost takes id from `options` (`frame.options.id`) plus typed `EditAutomationData` body (`automations.ts:14-17,85-88`). C++ calls `mutations.edit("automations", std::string{}, body)` — the id argument is an empty string, so the `:id` route param is discarded (`edit/workflow/step/execute.cppm:16`); body is raw, no DTO validation.
- Output/status: both default 200 (`automations.ts:81-90` has no `statusCode`; `edit/handler.cppm:27` sets `Status::OK`); C++ emits raw mutation body rather than Ghost serializer output.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` + integration-token allowlist (`routes.js:329`, `middleware.js:72`); C++ token-only (`edit/handler.cppm:18-22`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:82-84`); C++ none.

### PUT `/automations/poll` — MISMATCH
- Operation: Ghost `poll` is a 204 no-content trigger calling `automationsApi.requestPoll()` with no return body and no model write (`automations.ts:92-104`). C++ executes `mutations.edit("automations", "", body)` — a real DB write against the `automations` resource with an empty id and the request body, returning 200 with a body (`automations_poll_put/workflow/step/execute.cppm:16`, `automations_poll_put/handler.cppm:27`). Same path+verb, materially different behavior.
- Status: 204 vs 200 (`automations.ts:93` vs `automations_poll_put/handler.cppm:27`).
- Auth: Ghost uses `mw.authAdminApiWithUrl` (not plain `authAdminApi`) at `routes.js:327` plus explicit permission `{docName:'automations',method:'poll'}` (`automations.ts:97-100`); C++ only checks a non-empty bearer token (`automations_poll_put/handler.cppm:18-22`) and the router-level `content_type_middleware` (`routes.cppm:20`).
- Cache: Ghost `cacheInvalidate:false` (`automations.ts:94-96`); C++ route does not touch the `core::cache::Cache`. The `automations:*` namespace exists in `automations/operations.cppm:65-71` (`kTestEmailsSent = "automations:test_emails_sent"`, TTL `never()`, tag `{"automations"}`) but is injected only into the `email_preview`/`email_test` handlers (`all.cppm:140-141`), not into any of these 5 handlers (`all.cppm:134,136,142-144`).

### Cross-cutting C++ notes
- All 5 handlers use `using Input = interfaces::io::IRequest&` with no query-string parsing and no serde DTO; output is `std::string` passed through raw (`<op>/workflow/step/execute.cppm:11` for each).
- All 5 perform the same in-handler auth check: empty `token(req)` → 401 `{"error":"Unauthorized"}`; otherwise status OK. No bearer-scheme verification, no permission method, no labs/flag checks.
- Router installs `content_type_middleware` once for `/automations` (`routes.cppm:20`); Ghost installs per-route `mw.authAdminApi` (browse/read/add/edit) and `mw.authAdminApiWithUrl` (poll).
- `mutations` local is declared but unused in browse/read_by_id (`browse/.../execute.cppm:14`, `read_by_id/.../execute.cppm:14`); those use `json_for` / `find_by_resource` instead.

## Missing in C++

None. All 5 Ghost routes in this controller have a C++ route+handler in `include/blog/handlers/membership/automations/routes.cppm` (verified by exact path+verb: `routes.cppm:21,29,39,47,130`), and all 5 handlers are registered in `include/blog/handlers/all.cppm:134,136,142,143,144`.

## Extra in C++

No true EXTRA routes — every C++ route in the matched folder has a Ghost route in `routes.js`, but 6 of them belong to *other* Ghost controller files and are therefore out of scope for this controller's row set (not counted as EXTRA). Listed for completeness:

| C++ route (path + verb) | C++ handler | Ghost counterpart in `routes.js` (other controller) | C++ evidence |
|---|---|---|---|
| GET `/automations/:automation_id/actions/:action_id/links` | `automations_automations_automation_id_actions_action_id_links_get()` | `api.automationActionLinks.browse` (`routes.js:303-307`) | `routes.cppm:56-70` |
| POST `/automations/:id/email_preview` | `automations_automations_id_email_preview_post()` | `api.automationEmailPreviews.preview` (`routes.js:316-320`) | `routes.cppm:72-82` |
| POST `/automations/:id/email_test` | `automations_automations_id_email_test_post()` | `api.automationEmailPreviews.sendTestEmail` (`routes.js:321-326`) | `routes.cppm:83-93` |
| GET `/automations/:id/performance-stats` | `automations_automations_id_performance_stats_get()` | `api.automationPerformanceStats.read` (`routes.js:309-313`) | `routes.cppm:94-104` |
| GET `/automations/:id/runs` | `automations_automations_id_runs_get()` | `api.automationRuns.browse` (`routes.js:314`) | `routes.cppm:105-114` |
| GET `/automations/:id/runs/:id` | `automations_get_detail()` | `api.automationRunHistory.read` (`routes.js:315`) | `routes.cppm:115-125` |

Absence confirmations: no other `routes.cppm` under `include/blog/handlers/**` binds `/automations` (grep `automations` across `include/blog/handlers/**/routes.cppm` matched only `membership/automations/routes.cppm` and the central install list `include/blog/handlers/routes.cppm:8,81`); Ghost controller grep for `api.automations.` limited to `routes.js:302,308,327,328,329`.
