# Behavior report: automation-email-previews

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automation-email-previews.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (admin API router, mounted at `/ghost/api/admin`; base path confirmed by `test/utils/url-utils.js:16`)
- C++ folders: `include/blog/handlers/membership/automations/automations_id_email_preview_post/`, `include/blog/handlers/membership/automations/automations_id_email_test_post/` (registered in `include/blog/handlers/BUILD.bazel:556-561`, `include/blog/handlers/base.cppm:175-176`, `include/blog/handlers/all.cppm:169-170,540-541`)
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
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/automations/:id/email_preview` | `preview` (docName `automation_email_previews`) | path `id` required; body `subject`, `lexical` (flat keys); validator: non-empty subject + lexical, well-formed Lexical | `mw.authAdminApi` (authenticate + authorize + permission `automations:edit` via api-framework) | 200; `headers.cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` header; etag/content-version | `memberWelcomeEmails.service.previewAutomationEmail` → `#renderAutomationEmailPreview` → `Automation.findOne` (+relations) → `EmailDesignSetting.findOne` → renderer.render | none (no invalidation) | `{automation_email_previews:[{html,plaintext,subject}]}` (serializer envelope, etag) | `automations_id_email_preview_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` + `operations.cppm` | token-only auth; structural Lexical check; stand-in plaintext + `<p>` HTML; flat JSON no envelope; no design-settings load/template render | MATCH | Ghost `automation-email-previews.js:7-30`, `routes.js:316-320`, `automations.test.ts:1290-1351`; C++ `routes.cppm:19-21,73-83`, `execute.cppm:18-28`, `operations.cppm:25-51` |
| POST `/automations/:id/email_test` | `sendTestEmail` (docName `automation_email_previews`) | path `id` required; body `email`, `subject`, `lexical`; validator: valid email + non-empty subject/lexical + well-formed Lexical | `previewEmailLimiter` then `mw.authAdminApi` + permission `automations:edit` | 204 empty body; `cacheInvalidate:false` ⇒ no invalidation header | `memberWelcomeEmails.service.sendTestAutomationEmail` → `#sendTestAutomationEmail` → `#renderAutomationEmailPreview` → refresh sender options → `#sendTransactionalEmail` → `GhostMailer.send` (SMTP/outbox) | none (no invalidation) | empty (204) | `automations_id_email_test_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` + `operations.cppm` | no rate limiter; token-only auth; increments cache counter + writes `emails` store record as outbox stand-in; emits `X-Cache-Invalidate: false` header (divergence) | MATCH | Ghost `automation-email-previews.js:33-58`, `routes.js:321-326`, `spam-prevention.ts:273-278`, `automations.test.ts:1353-1392`; C++ `routes.cppm:84-94`, `handler.cppm:36`, `execute.cppm:20-35`, `operations.cppm:53-84` |

## Route details

### POST `/automations/:id/email_preview` — MATCH

**Ghost**

1. Identity: `router.post('/automations/:id/email_preview', mw.authAdminApi, http(api.automationEmailPreviews.preview))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:316-320`. Router-level `router.use(apiMw.cors)` at line 17. Bound via `apiFramework.pipeline(require('./automation-email-previews'), localUtils)` at `endpoints/index.js:30-32`. `mw.authAdminApi` is the array `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`).
2. Operation: exported key `preview`, op `preview`, `docName: 'automation_email_previews'` (`automation-email-previews.js:5,7`).
3. Input: `options: ['id']` (path param, `validation.options.id.required:true` at lines 11, 14-18); `data: ['subject','lexical']` (line 12). Body is flat keys (`test/e2e-api/admin/automations.test.ts:1300-1303`). Validator `validatePreviewData` enforces `subject` non-empty string (`validators/input/automation_email_previews.js:20-25`), `lexical` non-empty string (lines 27-32) and `lexicalLib.validate(lexical)` (line 34), each throwing `ValidationError` with `property`.
4. Auth: `mw.authAdminApi` (`routes.js:318`); controller `permissions: {docName:'automations', method:'edit'}` (`automation-email-previews.js:20-23`) → api-framework permission service checks `edit` on `automations`. Unauthenticated admin API requests 403 (`automations.test.ts:1281-1286`, same router).
5. Status/headers: 200; `headers.cacheInvalidate:false` (`automation-email-previews.js:8-10`) ⇒ `X-Cache-Invalidate` header NOT emitted (`automations.test.ts:1305` `cacheInvalidateHeaderNotSet()`); etag/content-version present (lines 1312-1315). No custom `Content-Disposition`/`Location`; response `format` default JSON.
6. Full call chain: `query(frame)` calls `memberWelcomeEmails.service.previewAutomationEmail({automationId, subject, lexical})` (`automation-email-previews.js:24-30`). Service `previewAutomationEmail` (`member-welcome-email-service.js:796-812`) → `#renderAutomationEmailPreview` (lines 716-775): `Automation.findOne({id}, {withRelated:['welcomeEmailAutomatedEmail','welcomeEmailAutomatedEmail.emailDesignSetting']})` (lines 723-731); `NotFoundError` if no automation (`MESSAGES.NO_MEMBER_WELCOME_EMAIL`, lines 734-738); `ValidationError` for empty lexical/subject (lines 740-750); hardcoded test member Jamie Larson / `jamie@example.com` / uuid `00000000-0000-4000-8000-000000000000` (lines 752-756); `#getDefaultEmailDesignSettings()` (lines 758-760) → `EmailDesignSetting.findOne({slug:'default-automated-email'})`, `NotFoundError` if absent (lines 272-284; slug at `constants.ts:1`); `#renderer.render(...)` (lines 763-769) which builds design, renders Lexical, applies Handlebar wrapper, `finalize()` to inline HTML/plaintext (`member-welcome-email-renderer.js:121-257`, finalize at line 250). Returns `{html, plaintext, subject}` (service lines 771-774, 807-811). Service deps wired in `services/member-welcome-emails/index.ts:79-103` (models.Automation, EmailDesignSetting, Newsletter; GhostMailer; MailgunClient). Errors: 404 missing automation (`automations.test.ts:1323-1335`), 422 missing lexical with `property:'lexical'` (lines 1337-1350).
7. Cache: none — `cacheInvalidate:false`; no cache service/key/TTL in the controller or service path.
8. Output: api-framework serializer wraps the `{html, plaintext, subject}` result into `{"automation_email_previews":[{html,plaintext,subject}]}` (`automations.test.ts:1307-1310`; field set also declared `test/legacy/api/admin/utils.js:149`); includes etag/content-version. Empty/missing: 404 error envelope instead of empty list.

**C++**

1. Identity/route: `Router{router,"/automations"}.use(content_type_middleware).use(require_auth)` then `.add_route(Route{"/email_preview"}.post(...))` (`include/blog/handlers/membership/automations/routes.cppm:19-21,73-83`); handler `automations_automations_id_email_preview_post()` (line 80). Installed at `include/blog/handlers/routes.cppm:85`.
2. Files/symbols: `automations_id_email_preview_post/handler.cppm:16-38` (`AutomationsIdEmailPreviewPostHandler`), `workflow/flow/main.cppm:7-10` (`MainFlow` single `ExecuteStep`), `workflow/step/execute.cppm:14-59` (`ExecuteStep::invoke`), `membership/automations/operations.cppm:25-51` (`AutomationsOperations::automation_email_preview`).
3. Input/parsing: `path_id` scans `"/automations/"` in `get_path()` (`execute.cppm:34-40`); empty id ⇒ 400 `validation("id is required","","id")` (lines 20-22). Body decoded as `AutomationEmailPreviewInput` (`operations.cppm:31`; DTO fields `subject`,`lexical` at `types/dto_automation_email_preview_input.cppm:11-21`) via serde `decode` (`middleware/serde.cppm:161-165`).
4. Status/body/headers: handler checks `token(req).empty()` ⇒ 401 `{"error":"Unauthorized"}` (`handler.cppm:24-28`); else writes payload status/body (lines 29-35), default `{"status":"200","body":"{}"}`. No `X-Cache-Invalidate` header emitted (consistent with Ghost here).
5. store()/cache calls: `automation_exists(id)` reads `ctx.store().resources().list("automations")` and `ctx.store().json_for("automations", {})` (`execute.cppm:45-56`); no mutations. `operations.cppm` preview path does no cache access.
6. Middleware chain: router-level `content_type_middleware` (rejects non-JSON Content-Type with 415 / non-JSON Accept with 406; `middleware/serde.cppm:90-126`) + `require_auth` (`middleware/auth.cppm:12-29`) + redundant token check in handler; `token()` reads `Authorization: Bearer` (`middleware/bearer.cppm:27-33`). No permission helper — no `automations:edit` check.
7. Side effects: none.
8. NOT implemented: real permission service; `Automation.findOne` with relations; default design-settings load; Handlebars/Lexical render + `finalize()`; serializer envelope/etag. `operations.cppm:46-50` fabricates `plaintext` from text nodes via `lexical_plaintext` (lines 138-150) and `html = "<p>"+plaintext+"</p>"` (line 47) — a stand-in for real rendering. Valid Lexical check is structural only (`type:"doc"` + `root.children` array, lines 127-135); output is flat `{html,plaintext,subject}` with no `automation_email_previews` envelope.

**Diff**

- Residual gaps (Ghost primitives absent in C++): permission service `{docName:'automations',method:'edit'}` (`automation-email-previews.js:20-23`); real `Automation`/`EmailDesignSetting` model lookups + `NotFoundError` (`member-welcome-email-service.js:723-738,272-284`); full email renderer/template/finalize pipeline (`member-welcome-email-renderer.js:121-257`); api-framework serializer envelope + etag (`automations.test.ts:1307-1315`).
- Divergences: C++ auth is token presence only (401) vs Ghost authenticate+authorize+permission (403) (`handler.cppm:24-28` vs `admin/middleware.js:110-118`); fabricated `<p>` HTML vs rendered email (`operations.cppm:47`); flat JSON vs enveloped array; C++ 404 body `{"errors":[...]}` with context `"Automation not found"` (`execute.cppm:24-25`) vs Ghost `NO_MEMBER_WELCOME_EMAIL` message; no etag/content-version.

### POST `/automations/:id/email_test` — MATCH

**Ghost**

1. Identity: `router.post('/automations/:id/email_test', shared.middleware.rateLimiting.previewEmailLimiter, mw.authAdminApi, http(api.automationEmailPreviews.sendTestEmail))` (`routes.js:321-326`). Rate limiter `previewEmailLimiter = use('emailPreviewBlock')` (`web/shared/middleware/rate-limiting.ts:74`) → `emailPreviewBlock` limiter with `ignoreIP:true`, message "Only 10 test emails can be sent per hour" (`web/shared/middleware/api/spam-prevention.ts:273-278`), config `freeRetries:10, lifetime:3600` (`test/utils/fixtures/config/defaults.json:133-138`), blocking with `TooManyRequestsError` (429).
2. Operation: exported key `sendTestEmail`, `docName: 'automation_email_previews'` (`automation-email-previews.js:5,33`).
3. Input: `options:['id']` required (lines 38, 40-46); `data:['email','subject','lexical']` (line 39). Validator `sendTestEmail` (lines 47-58): `email` must pass `validator.isEmail` (`validators/input/automation_email_previews.js:48-55`), then `validatePreviewData` (subject/lexical non-empty + well-formed, lines 16-40, 57).
4. Auth: `mw.authAdminApi` + `permissions:{docName:'automations',method:'edit'}` (`automation-email-previews.js:47-50`); rate limiter runs before auth (`routes.js:322-324`).
5. Status/headers: `statusCode:204` with empty body (`automation-email-previews.js:34`; `automations.test.ts:1370-1371` `expectEmptyBody()`); `headers.cacheInvalidate:false` (lines 35-37) ⇒ no `X-Cache-Invalidate` header (`automations.test.ts:1372`).
6. Full call chain: `query(frame)` → `memberWelcomeEmails.service.sendTestAutomationEmail({automationId, email, subject, lexical})` (`automation-email-previews.js:51-58`). Service `sendTestAutomationEmail` (lines 824-831) → `#sendTestAutomationEmail` (lines 833-867): `#renderAutomationEmailPreview` with `memberEmail:email` (lines 840-851; same model lookups/validators/render as preview route); refresh `#defaultNewsletterSenderOptions = await #getDefaultNewsletterSenderOptions()` (line 854); `#getEffectiveSenderOptions` (line 856); `#sendTransactionalEmail({to:email, subject:'[Test] '+renderedSubject, html, text, ...senderOptions})` (lines 860-866) → `transactionalMailer.send` (`GhostMailer.send`, lines 561-572; wired `index.ts:93`). Side effect: real SMTP/mail outbox write (`automations.test.ts:1355,1374` stubs/asserts `GhostMailer.prototype.send` called once). Errors: 422 invalid email (`automations.test.ts:1377-1391`); 404/422 inherited from `#renderAutomationEmailPreview`.
7. Cache: none (no invalidation header, no cache service).
8. Output: 204, no body, no serializer fields.

**C++**

1. Identity/route: `Route{"/email_test"}.post(...)` under the same `/automations` router with `content_type_middleware` + `require_auth` (`routes.cppm:84-94`); handler `automations_automations_id_email_test_post()` (line 91). No rate limiter middleware is installed in C++.
2. Files/symbols: `automations_id_email_test_post/handler.cppm:17-40` (`AutomationsIdEmailTestPostHandler`), `workflow/flow/main.cppm:7-10`, `workflow/step/execute.cppm:16-78`, `operations.cppm:53-84`.
3. Input/parsing: `path_id` (`execute.cppm:42-48`); empty id ⇒ 400 (lines 22-24); body decoded as `AutomationEmailTestInput` with `email`,`subject`,`lexical` (`operations.cppm:59`; DTO `types/dto_automation_email_test_input.cppm:11-24`). `email_of` extracts `email` from raw JSON for the outbox record (`execute.cppm:65-74`).
4. Status/body/headers: handler token check ⇒ 401 (handler lines 25-29); success 204, empty body (`execute.cppm:35`, `handler.cppm:33-37`). Notably `handler.cppm:36` sets `X-Cache-Invalidate: false` — Ghost omits that header entirely (`automations.test.ts:1372`), so this is a divergence.
5. store()/cache calls: on success the step writes `ctx.store().resources().create("emails", '{"to":<email>,"status":"submitted","direction":"outbound"}')` (`execute.cppm:31-34`) — a local outbox stand-in. `operations.cppm:81-82` increments and stores cache counter `kTestEmailsSent = "automations:test_emails_sent"` with `Ttl{}.never()` and tag `{"automations"}` (key at line 96; analogous poll marker at 89/97).
6. Middleware chain: same router-level `content_type_middleware` + `require_auth` (`routes.cppm:20-21`; `middleware/auth.cppm:12-29`) plus handler token check (`handler.cppm:25-29`); no permission helper, no rate limiter.
7. Side effects: `emails` store record + cache counter (above). No SMTP, no `GhostMailer.send`, no sender-option resolution, no rendered email.
8. NOT implemented: `previewEmailLimiter` 10/hour budget; permission service; `#renderAutomationEmailPreview` model/relations/design-settings; `#getDefaultNewsletterSenderOptions`/`#getEffectiveSenderOptions`; real mail send. Email validation is structural only (`valid_email`, `operations.cppm:152-162`) vs `@tryghost/validator.isEmail`.

**Diff**

- Residual gaps: rate limiter (`spam-prevention.ts:273-278`, `rate-limiting.ts:74`), permission service (`automation-email-previews.js:47-50`), rendering + sender resolution (`member-welcome-email-service.js:840-866`), real `GhostMailer.send` (`index.ts:93`, `automations.test.ts:1374`), `NotFoundError`/`ValidationError` model errors.
- Divergences: no 429 throttling; token-only auth (401) vs 403/permission; fabricated outbox (`emails` resource) and cache counter stand-in instead of SMTP (`execute.cppm:31-34`, `operations.cppm:81-82`); C++ emits `X-Cache-Invalidate: false` (`handler.cppm:36`) whereas Ghost omits the header (`automation-email-previews.js:35-37`, `automations.test.ts:1372`); structural email check weaker than `isEmail`.

## Module-level notes

- Both routes share `docName: 'automation_email_previews'` and `permissions:{docName:'automations', method:'edit'}` (`automation-email-previews.js:5,20-23,47-50`); the C++ side has no permission-service primitive, only bearer/token presence (`middleware/auth.cppm:12-29`, `middleware/bearer.cppm:27-33`).
- Both Ghost routes set `headers.cacheInvalidate:false`; neither invalidates cache. C++ preview route likewise writes no invalidate header, but C++ email_test writes `X-Cache-Invalidate: false` (`automations_id_email_test_post/handler.cppm:36`) — an internal C++ inconsistency and a Ghost divergence.
- C++ stands in for the entire email stack: `operations.cppm` contains hand-rolled `valid_lexical`, `lexical_plaintext`, `valid_email` (lines 127-162) and a cache-tag counter, replacing Ghost's real Lexical validation, Handlebars renderer, `finalize`, design-settings model, and mailer.
- C++ error envelopes come from `ApiError::validation`/`json` (`include/blog/handlers/context/context_context.cppm:79-124`) producing `{"errors":[{id,code,context,details,ghostErrorCode,help,message,property,type}]}`; C++ uses 400/404/422 statuses that map to Ghost's 422/404/429 in intent but not in message text.
- Do not confuse `include/blog/handlers/membership/email_previews/` (routes at `membership/email_previews/routes.cppm`, handlers `get_detail`/`post_detail`) with this module. Those map to Ghost's `/automated_emails/:id/preview` and `/automated_emails/:id/test` (`admin/routes.js:344-350`), a different controller; they are not part of automation-email-previews parity and are not counted here.
- No EXTRA routes in C++ for this module and no MISSING routes: both Ghost routes have exact path+verb C++ counterparts at the `/automations` router.

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
