# Parity: automation-email-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-email-previews.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (folder found by exact path, not assumed)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 2 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/automations/:id/email_preview` | `preview` (docName `automation_email_previews`) | POST `/automations/:id/email_preview` | `automations_automations_id_email_preview_post()` → `AutomationsIdEmailPreviewPostHandler` | Ghost: option `id` (required) + body `subject`,`lexical`, lexical well-formed. C++: body DTO `subject`,`lexical` only; `id` ignored/hardcoded `"1"`; no lexical-format check. | Ghost: service return `{html, plaintext, subject}` (single object), 200. C++: `{"automation_email_previews":[{subject,html:"<p>Lorem ipsum.</p>",plaintext:"Lorem ipsum."}]}` array envelope, hardcoded content. | Ghost: `memberWelcomeEmails.service.previewAutomationEmail` → real render by `automationId`. C++: `known_automation(id)` string check + hardcoded response; no store read (dead fallback `mutations.create("automations",...)`). | Ghost: `headers.cacheInvalidate:false` (no invalidation). C++: none in preview op. | Ghost: `mw.authAdminApi` + permissions `automations`/`edit` + validator. C++: inline token check (401) only; no docName/method, no rate limiter. | MISMATCH | Ghost `routes.js:316-320`, controller `:7-31`, validator `:16-45`, service `:796-812`; C++ `routes.cppm:72-82`, handler.cppm `:18-29`, execute.cppm `:15-17`, operations.cppm `:21-45` |
| POST `/automations/:id/email_test` | `sendTestEmail` | POST `/automations/:id/email_test` | `automations_automations_id_email_test_post()` → `AutomationsIdEmailTestPostHandler` | Ghost: option `id` (required) + body `email`,`subject`,`lexical`; `email` via `validator.isEmail`, lexical well-formed, subject/lexical required. C++: body DTO `email`,`subject`,`lexical`; `id` ignored/hardcoded `"1"`; email check only contains `@` and `.`; no subject/lexical validation. | Ghost: 204, empty body. C++: 204, empty body (matches). | Ghost: `sendTestAutomationEmail` → renders + sends real transactional email. C++: no send; increments cache counter only. | Ghost: `headers.cacheInvalidate:false`. C++: `automations:test_emails_sent` counter, tag `automations`, TTL never (extra). | Ghost: `previewEmailLimiter` + `mw.authAdminApi` + permissions `automations`/`edit`. C++: inline token check (401) only; no rate limiter, no permissions. | MISMATCH | Ghost `routes.js:321-326`, controller `:33-59`, validator `:47-58`, service `:824-867`; C++ `routes.cppm:83-93`, handler.cppm `:19-30`, execute.cppm `:16-18`, operations.cppm `:47-68` |

## Route details

### POST /automations/:id/email_preview — MISMATCH

- Route identity MATCH: Ghost `routes.js:316-320` POST `/automations/:id/email_preview`; C++ `routes.cppm:72-82` `Route{"/email_preview"}.post()` nested under `Router{router, "/:id"}` (`routes.cppm:38`) under `Router{router, "/automations"}` (`routes.cppm:19`). Placeholders normalize to `:id`.
- Operation mapping MATCH: controller key `preview` (`automation-email-previews.js:7`) → `api.automationEmailPreviews.preview` (`routes.js:319`) ↔ `handler.automations_automations_id_email_preview_post()` (`routes.cppm:79`), accessor `all.cppm:486`, member `all.cppm:735`.
- Input divergence: Ghost declares `options:['id']` with `id.required:true` (`automation-email-previews.js:11,14-18`) and passes `frame.options.id` as `automationId` (`:26`); C++ step calls `automation_email_preview("1", in.body_as_string())` (`execute.cppm:15`) — route `:id` never read, hardcoded `"1"`, then `known_automation("1")` is always true (`operations.cppm:73-76`). No query `id` parsing anywhere.
- Input divergence: Ghost validator requires well-formed Lexical (`utils/validators/input/automation_email_previews.js:34-39`); C++ decodes `AutomationEmailPreviewInput{subject, lexical}` (`operations.cppm:30`, DTO `automation_email_preview_input.cppm:14-20`) and only checks `.empty()` (`operations.cppm:31`), no lexical validation.
- Output MISMATCH: Ghost returns `{html, plaintext, subject}` from `#renderAutomationEmailPreview` (`member-welcome-email-service.js:796-812`); C++ returns `AutomationEmailPreviewResponse` envelope `automation_email_previews: [...]` (`operations.cppm:38-44`, DTO `automation_email_preview_response.cppm:14-17`) with hardcoded `html="<p>Lorem ipsum.</p>"`, `plaintext="Lorem ipsum."` (`operations.cppm:41-42`) and `subject` echoed from input (`:40`). Envelope shape and content both differ.
- DB/service MISMATCH: Ghost calls `memberWelcomeEmails.service.previewAutomationEmail` (`controller:25`), which renders using `automationId`, models and design settings (`service:801-805`). C++ performs no store read; only `known_automation` string check (`operations.cppm:27,73-76`) and hardcoded response. The `mutations.create("automations",...)` branch (`execute.cppm:17`) is unreachable because `automation_email_preview` always returns a non-zero status (200/404/422, `operations.cppm:28,32,44`), so `r.first!=0` always takes the first branch (`execute.cppm:16`).
- Cache: none on either side for this route (Ghost `controller:8-10` `cacheInvalidate:false`; C++ op has no `m_cache` use).
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:318`) + permissions `docName:'automations', method:'edit'` (`controller:20-23`). C++ handler only checks `token(req).empty()` → 401 (`handler.cppm:18-22`; `bearer.cppm:27-33`); no docName/method permission check, only router-level `content_type_middleware` (`routes.cppm:20`).

### POST /automations/:id/email_test — MISMATCH

- Route identity MATCH: Ghost `routes.js:321-326` POST `/automations/:id/email_test`; C++ `routes.cppm:83-93` `Route{"/email_test"}.post()` under `/:id` under `/automations`.
- Operation mapping MATCH: controller key `sendTestEmail` (`automation-email-previews.js:33`) → `api.automationEmailPreviews.sendTestEmail` (`routes.js:325`) ↔ `handler.automations_automations_id_email_test_post()` (`routes.cppm:90`), accessor `all.cppm:487`, member `all.cppm:736`.
- Input divergence: same `id` issue — Ghost passes `frame.options.id` (`controller:53`); C++ hardcodes `"1"` (`execute.cppm:16`). Ghost `email` validated with `validator.isEmail` (`validator:50`); C++ `input->email.find('@') != npos && find('.') != npos` (`operations.cppm:57-58`). Ghost also requires subject/lexical non-empty + well-formed Lexical via `validatePreviewData` (`validator:16-40,57`); C++ `AutomationEmailTestInput` has `email/subject/lexical` fields (`automation_email_test_input.cppm:14-23`) but validates only `email` (`operations.cppm:56-64`).
- Output MATCH: Ghost `statusCode:204` (`controller:34`) with no returned body (`:51-58`); C++ returns `{204, ""}` (`operations.cppm:67`) set on response (`handler.cppm:27-30`).
- DB/service MISMATCH: Ghost calls `sendTestAutomationEmail` (`controller:52`) → `#renderAutomationEmailPreview` + `#sendTransactionalEmail` (`service:824-867`) — a real email send. C++ does no send and no store call; only `known_automation` check, email validation, then a cache counter increment (`operations.cppm:53-67`). Same dead-code fallback note as preview (`execute.cppm:17-18`).
- Cache divergence: Ghost `cacheInvalidate:false` (`controller:35-37`) with no counter. C++ increments `kTestEmailsSent = "automations:test_emails_sent"` via `m_cache.get/put`, TTL `never()`, tag `{"automations"}` (`operations.cppm:65-66,71`). This is extra behavior absent on the Ghost side.
- Auth divergence: Ghost adds `shared.middleware.rateLimiting.previewEmailLimiter` before `mw.authAdminApi` (`routes.js:323-324`) plus permissions `automations`/`edit` (`controller:47-50`). C++ has only inline token check 401 (`handler.cppm:19-23`); no rate limiter, no permissions.

## Missing in C++

None. Both Ghost routes in this controller (`preview`, `sendTestEmail`) have exact path+verb counterparts in `include/blog/handlers/membership/automations/routes.cppm:72-93`.

Confirmation: grep for `email_preview|email_test|automation.*preview|automation.*test` across `include/blog/handlers` returned the two routes in that folder plus unrelated `membership/email_previews/*` routes (different Ghost controller `email_previews.js`, paths `/email_previews/...`) — not MISSING for this controller.

## Extra in C++

None. The matched folder `include/blog/handlers/membership/automations/routes.cppm` also installs other routes (browse/create/read/edit at `:21-54`, `/links` `:60-69`, `/performance-stats` `:95-104`, `/runs` `:106-126`, `/poll` `:129-137`), but each has its own Ghost counterpart in `admin/routes.js` under other controllers (`automations.js`, `automation-action-links.js`, `automation-performance-stats.js`, `automation-runs.js`, etc.). They are not EXTRA relative to this controller. No C++ route in this folder lacks a Ghost route entirely.
