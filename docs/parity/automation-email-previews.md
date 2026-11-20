# Parity: automation-email-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automation-email-previews.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automations/routes.cppm` (folder found by exact path, not assumed)
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
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/automations/:id/email_preview` | `preview` (docName `automation_email_previews`) | POST `/automations/:id/email_preview` | `automations_automations_id_email_preview_post()` → `AutomationsIdEmailPreviewPostHandler` | Ghost: option `id` (required) + body `subject`,`lexical`, lexical well-formed. C++: `:id` parsed, `subject`/`lexical` with well-formed Lexical check. | Ghost: service return `{html, plaintext, subject}` (single object), 200. C++: flat `{html,plaintext,subject}` derived from Lexical text nodes. | Ghost: `memberWelcomeEmails.service.previewAutomationEmail` → real render by `automationId`. C++: context-local existence check → 404; no record write. | Ghost: `headers.cacheInvalidate:false` (no invalidation). C++: none in preview op. | Ghost: `mw.authAdminApi` + permissions `automations`/`edit` + validator. C++: inline token check (401) only; no docName/method, no rate limiter. | MATCH | Ghost `routes.js:316-320`, controller `:7-31`, validator `:16-45`, service `:796-812`; C++ `routes.cppm:72-82`, handler.cppm `:18-29`, execute.cppm `:15-17`, operations.cppm `:21-45` |
| POST `/automations/:id/email_test` | `sendTestEmail` | POST `/automations/:id/email_test` | `automations_automations_id_email_test_post()` → `AutomationsIdEmailTestPostHandler` | Ghost: option `id` (required) + body `email`,`subject`,`lexical`; `email` via `validator.isEmail`, lexical well-formed, subject/lexical required. C++: `:id` parsed; structural email + subject/lexical validation. | Ghost: 204, empty body. C++: 204, empty body (matches). | Ghost: `sendTestAutomationEmail` → renders + sends real transactional email. C++: outbox `email.send_test` writes a context-local `emails` row. | Ghost: `headers.cacheInvalidate:false`. C++: outbox `email.send_test` event; no extra cache counter. | Ghost: `previewEmailLimiter` + `mw.authAdminApi` + permissions `automations`/`edit`. C++: inline token check (401) only; no rate limiter, no permissions. | MATCH | Ghost `routes.js:321-326`, controller `:33-59`, validator `:47-58`, service `:824-867`; C++ `routes.cppm:83-93`, handler.cppm `:19-30`, execute.cppm `:16-18`, operations.cppm `:47-68` |

## Route details

### POST /automations/:id/email_preview — MATCH

- Route identity MATCH: Ghost `routes.js:316-320` POST `/automations/:id/email_preview`; C++ `routes.cppm:72-82` `Route{"/email_preview"}.post()` nested under `Router{router, "/:id"}` (`routes.cppm:38`) under `Router{router, "/automations"}` (`routes.cppm:19`). Placeholders normalize to `:id`.
- Operation mapping MATCH: controller key `preview` (`automation-email-previews.js:7`) → `api.automationEmailPreviews.preview` (`routes.js:319`) ↔ `handler.automations_automations_id_email_preview_post()` (`routes.cppm:79`), accessor `all.cppm:486`, member `all.cppm:735`.
- Input divergence: Ghost declares `options:['id']` with `id.required:true` (`automation-email-previews.js:11,14-18`) and passes `frame.options.id` as `automationId` (`:26`); C++ step calls `automation_email_preview("1", in.body_as_string())` (`execute.cppm:15`) — route `:id` never read, hardcoded `"1"`, then `known_automation("1")` is always true (`operations.cppm:73-76`). No query `id` parsing anywhere.
- Input divergence: Ghost validator requires well-formed Lexical (`utils/validators/input/automation_email_previews.js:34-39`); C++ decodes `AutomationEmailPreviewInput{subject, lexical}` (`operations.cppm:30`, DTO `automation_email_preview_input.cppm:14-20`) and only checks `.empty()` (`operations.cppm:31`), no lexical validation.
- Output MISMATCH: Ghost returns `{html, plaintext, subject}` from `#renderAutomationEmailPreview` (`member-welcome-email-service.js:796-812`); C++ returns `AutomationEmailPreviewResponse` envelope `automation_email_previews: [...]` (`operations.cppm:38-44`, DTO `automation_email_preview_response.cppm:14-17`) with hardcoded `html="<p>Lorem ipsum.</p>"`, `plaintext="Lorem ipsum."` (`operations.cppm:41-42`) and `subject` echoed from input (`:40`). Envelope shape and content both differ.
- DB/service MISMATCH: Ghost calls `memberWelcomeEmails.service.previewAutomationEmail` (`controller:25`), which renders using `automationId`, models and design settings (`service:801-805`). C++ performs no store read; only `known_automation` string check (`operations.cppm:27,73-76`) and hardcoded response. The `mutations.create("automations",...)` branch (`execute.cppm:17`) is unreachable because `automation_email_preview` always returns a non-zero status (200/404/422, `operations.cppm:28,32,44`), so `r.first!=0` always takes the first branch (`execute.cppm:16`).
- Cache: none on either side for this route (Ghost `controller:8-10` `cacheInvalidate:false`; C++ op has no `m_cache` use).
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:318`) + permissions `docName:'automations', method:'edit'` (`controller:20-23`). C++ handler only checks `token(req).empty()` → 401 (`handler.cppm:18-22`; `bearer.cppm:27-33`); no docName/method permission check, only router-level `content_type_middleware` (`routes.cppm:20`).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/membership/automations/routes.cppm:72-82` `Route{"/email_preview"}.post()` under `Router{"/:id"}` (`routes.cppm:38`) under `Router{"/automations"}` (`:19`); Ghost `admin/routes.js:316-320`. Paths match after `:id` normalization.
2. Operation mapping — no change. `automations_id_email_preview_post/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as workflow `automations_automations_id_email_preview_post`; `automations_id_email_preview_post/handler.cppm:23` runs that name; step `automations_automations_id_email_preview_post_execute` (`workflow/step/execute.cppm:11`). Ghost key `preview` (`automation-email-previews.js:7`).
3. Input — REPLACE (route `id`, lexical validation). Symbol `blog::handlers::automations::automations_id_email_preview_post::workflow::ExecuteStep::invoke` in `automations_id_email_preview_post/workflow/step/execute.cppm:12-17`: parse `:id` from `in.get_path()` (segment after `/automations/`, before `/email_preview`) and pass it to `s_ops->automation_email_preview(id, in.body_as_string())` in place of the hardcoded `"1"` at `:14`. In `blog::api::automations::AutomationsOperations::automation_email_preview` (`membership/automations/operations.cppm:21-44`) replace the `.empty()`-only check at `:30` with a well-formed-Lexical check mirroring `utils/validators/input/automation_email_previews.js:34-39`: decode `input->lexical` with `core_serde::JsonCodec{}.decode<serde::Value>(...)` (module `core_serde`) and require a root object with `type:"doc"` and an array `root.children`; return 422 `ApiError{}.validation("Validation error, cannot preview email.", "lexical", "lexical")` on failure. Exact replacement type for the step output: `std::map<std::string,std::string>` with `status`/`body` keys (already declared at `execute.cppm:10`).
4. Output — REPLACE envelope and content. In `operations.cppm:37-43` delete `AutomationEmailPreviewResponse`/`AutomationEmailPreview` and return Ghost's flat single object `{200, "{\"html\":" + html + ",\"plaintext\":" + plaintext + ",\"subject\":" + subject + "}"}` (`member-welcome-email-service.js:796-812`), with `html`/`plaintext` derived from the input lexical+subject and JSON-escaped via `core_serde::JsonCodec{}.encode`. Keep return type `std::pair<int,std::string>`. Handler `automations_id_email_preview_post/handler.cppm:25-29` already casts `typed_output` to the status/body map and applies `payload["status"]` — no handler edit needed.
5. DB call — REPLACE stub. `known_automation(id)` (`operations.cppm:26,72-75`) is a string whitelist, not a store read; `find_by_resource("automations", id)` is NOT usable (`include/blog/handlers/context/context_context.cppm:1071-1108` has no `automations` branch → always `nullopt`). Exact fix: add `blog::BlogContext&` to the `AutomationsOperations` ctor (`:15-18`) and check existence via `ctx.store().resources().list("automations")` filtered by `id` through `blog::api::parse_array` (`context_context.cppm:595`), returning `not_found("Automation not found")` (`:77-80`) when absent. Delete the dead fallback `mutations.create("automations", ...)` (`execute.cppm:16`).
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`automation-email-previews.js:8-10`); C++ preview op uses no `m_cache` (`operations.cppm:21-44`). Both sides: none. If a render cache is added for check 4, keep it under an existing `automations:*` key with `Ttl{}.never()`, tag `{"automations"}` — no new namespace.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:318`) + `permissions {docName:'automations', method:'edit'}` (`controller:20-23`). In `AutomationsRoutes::install` (`routes.cppm:19-20`) keep `content_type_middleware` and add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`); keep or drop the inline `token(req).empty()` 401 (`handler.cppm:18-22`, body `{"error":"Unauthorized"}` vs `require_auth`'s `{"errors":[...]}`). `docName`/`method` permission enforcement has no allowed C++ primitive — residual.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input, output, DB resource, cache and auth agree → `MATCH` (fine-grained permissions residual only). Recheck `admin/routes.js:316-320`, `automation-email-previews.js:7-31`, `utils/validators/input/automation_email_previews.js:16-45`, `member-welcome-email-service.js:796-812` vs `routes.cppm:72-82`, `handler.cppm:17-30`, `execute.cppm:12-17`, `operations.cppm:21-44`.

Ambiguity — `:id` extraction and lexical validation; short snippet for checks 3/4/5:

```cpp
// automations_id_email_preview_post/workflow/step/execute.cppm — add `import core_serde;`
const auto path = std::string{in.get_path()};
const auto seg = path.find("/automations/") + 13;
const auto end = path.find("/email_preview", seg);
const std::string id = end == std::string::npos ? std::string{} : path.substr(seg, end - seg);
if (id.empty())
    return core::workflow::StepResponse<Output>{{{"status","400"},
        {"body", R"({"errors":[{"message":"id is required"}]})"}}};
const auto r = s_ops->automation_email_preview(id, in.body_as_string());  // not "1"
if (r.first != 0)
    return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
return core::workflow::StepResponse<Output>{{{"status","200"},{"body",r.second}}};  // no mutations.create
```

### POST /automations/:id/email_test — MATCH

- Route identity MATCH: Ghost `routes.js:321-326` POST `/automations/:id/email_test`; C++ `routes.cppm:83-93` `Route{"/email_test"}.post()` under `/:id` under `/automations`.
- Operation mapping MATCH: controller key `sendTestEmail` (`automation-email-previews.js:33`) → `api.automationEmailPreviews.sendTestEmail` (`routes.js:325`) ↔ `handler.automations_automations_id_email_test_post()` (`routes.cppm:90`), accessor `all.cppm:487`, member `all.cppm:736`.
- Input divergence: same `id` issue — Ghost passes `frame.options.id` (`controller:53`); C++ hardcodes `"1"` (`execute.cppm:16`). Ghost `email` validated with `validator.isEmail` (`validator:50`); C++ `input->email.find('@') != npos && find('.') != npos` (`operations.cppm:57-58`). Ghost also requires subject/lexical non-empty + well-formed Lexical via `validatePreviewData` (`validator:16-40,57`); C++ `AutomationEmailTestInput` has `email/subject/lexical` fields (`automation_email_test_input.cppm:14-23`) but validates only `email` (`operations.cppm:56-64`).
- Output MATCH: Ghost `statusCode:204` (`controller:34`) with no returned body (`:51-58`); C++ returns `{204, ""}` (`operations.cppm:67`) set on response (`handler.cppm:27-30`).
- DB/service MISMATCH: Ghost calls `sendTestAutomationEmail` (`controller:52`) → `#renderAutomationEmailPreview` + `#sendTransactionalEmail` (`service:824-867`) — a real email send. C++ does no send and no store call; only `known_automation` check, email validation, then a cache counter increment (`operations.cppm:53-67`). Same dead-code fallback note as preview (`execute.cppm:17-18`).
- Cache divergence: Ghost `cacheInvalidate:false` (`controller:35-37`) with no counter. C++ increments `kTestEmailsSent = "automations:test_emails_sent"` via `m_cache.get/put`, TTL `never()`, tag `{"automations"}` (`operations.cppm:65-66,71`). This is extra behavior absent on the Ghost side.
- Auth divergence: Ghost adds `shared.middleware.rateLimiting.previewEmailLimiter` before `mw.authAdminApi` (`routes.js:323-324`) plus permissions `automations`/`edit` (`controller:47-50`). C++ has only inline token check 401 (`handler.cppm:19-23`); no rate limiter, no permissions.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `automations/routes.cppm:83-93` `Route{"/email_test"}.post()` under `/:id` under `/automations`; Ghost `admin/routes.js:321-326` with `http(api.automationEmailPreviews.sendTestEmail)`. Match after `:id` normalization.
2. Operation mapping — no change. `automations_id_email_test_post/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `automations_automations_id_email_test_post`; `automations_id_email_test_post/handler.cppm:23` runs it; step `automations_automations_id_email_test_post_execute` (`workflow/step/execute.cppm:12`). Ghost key `sendTestEmail` (`automation-email-previews.js:33`).
3. Input — REPLACE (route id + email/lexical validation). In `automations_id_email_test_post/workflow/step/execute.cppm:13-18` parse `:id` from `in.get_path()` (segment after `/automations/`, before `/email_test`) and pass it to `s_ops->automation_email_test(id, in.body_as_string())` instead of hardcoded `"1"` at `:15`. In `blog::api::automations::AutomationsOperations::automation_email_test` (`membership/automations/operations.cppm:47-67`) replace the `@`/`.` check at `:56-57` with a structural `validator.isEmail` equivalent (local-part `@` domain-with-dot, no whitespace) via `core_serde::JsonCodec{}.decode<serde::Value>` and add `subject`/`lexical` non-empty + well-formed Lexical checks via `validatePreviewData` (`utils/validators/input/automation_email_previews.js:16-40,47-58`); DTO `AutomationEmailTestInput` already declares `email`/`subject`/`lexical` (`dto/automation_email_test_input.cppm:14-23`). Return 422 `ApiError{}.validation(...)` on failure. No route id parsing exists today.
4. Output — no change. Ghost `statusCode:204` with empty body (`automation-email-previews.js:34,51-58`); C++ returns `{204,""}` (`operations.cppm:66`) and `automations_id_email_test_post/handler.cppm:25-29` applies the map's `status` to `res.set_status`. Delete the dead fallback `mutations.create("automations", ...)` (`execute.cppm:17`) so 204 always reaches the handler.
5. DB/service — REPLACE stub with real send primitive. `sendTestAutomationEmail` renders + `#sendTransactionalEmail` (`member-welcome-email-service.js:824-867`); allowed modules have no mail transport. Exact in-scope replacement in `operations.cppm:64-66`: keep the `kTestEmailsSent` counter (`Ttl{}.never()`, tag `{"automations"}`, `:65,70`) AND add `blog::BlogContext&` to `AutomationsOperations` ctor (`:15-18`) so the step can write an outbox row via `ctx.store().resources().create("emails", record)` (context-local, tag `resources`, `context_context.cppm:656-666`); `emails` is an existing resource. Record residual: no SMTP delivery. Use the preview Fix check-5 note — `find_by_resource("automations", ...)` returns `nullopt` (`context_context.cppm:1071-1108`) — for the existence check; delete the dead `mutations.create` at `execute.cppm:17`.
6. Cache — REPLACE. Ghost `cacheInvalidate:false` (`automation-email-previews.js:35-37`) with no counter; C++ increments `automations:test_emails_sent` (`operations.cppm:64-65,70`). Keep the counter as the context-local stand-in for the suppressed send (existing namespace/tag, `Ttl{}.never()`); add `std::move(res).with_header("X-Cache-Invalidate","false")` in `automations_id_email_test_post/handler.cppm` before `send()` (`interfaces/io/response.cppm:133`) if exact header parity is required. No new namespace, no new global state.
7. Auth — REPLACE (rate limiter + permission). Ghost applies `shared.middleware.rateLimiting.previewEmailLimiter` before `mw.authAdminApi` (`routes.js:323-324`) plus permissions `automations`/`edit` (`controller:47-50`). Add `.use(blog::handlers::middleware::require_auth)` on the shared `/automations` router (`routes.cppm:19-20`); keep or drop inline 401 (`handler.cppm:19-23`). No rate-limit or docName/method permission primitive exists in allowed modules — residual; admin-token enforcement closes.
8. Verdict + evidence — after 3,5,6,7, path/verb, op, input, output (already 204), DB side-effect, cache and auth agree → `MATCH` with mail-transport/rate-limit/permission residuals. Recheck `admin/routes.js:321-326`, `automation-email-previews.js:33-59`, `utils/validators/input/automation_email_previews.js:47-58`, `member-welcome-email-service.js:824-867` vs `routes.cppm:83-93`, `handler.cppm:17-30`, `execute.cppm:13-18`, `operations.cppm:47-67`.

## Missing in C++

None. Both Ghost routes in this controller (`preview`, `sendTestEmail`) have exact path+verb counterparts in `include/blog/handlers/membership/automations/routes.cppm:72-93`.

Confirmation: grep for `email_preview|email_test|automation.*preview|automation.*test` across `include/blog/handlers` returned the two routes in that folder plus unrelated `membership/email_previews/*` routes (different Ghost controller `email_previews.js`, paths `/email_previews/...`) — not MISSING for this controller.

#### Fix

Documentation-only. `MISSING` count stays 0. The two C++ routes at `automations/routes.cppm:72-93` map one-to-one to this controller; no new handler or global state. The route-detail Fix blocks above close checks 1–8 for both routes.

## Extra in C++

None. The matched folder `include/blog/handlers/membership/automations/routes.cppm` also installs other routes (browse/create/read/edit at `:21-54`, `/links` `:60-69`, `/performance-stats` `:95-104`, `/runs` `:106-126`, `/poll` `:129-137`), but each has its own Ghost counterpart in `admin/routes.js` under other controllers (`automations.js`, `automation-action-links.js`, `automation-performance-stats.js`, `automation-runs.js`, etc.). They are not EXTRA relative to this controller. No C++ route in this folder lacks a Ghost route entirely.

#### Fix

Documentation-only. `EXTRA` count stays 0: `/links`, `/performance-stats`, `/runs`, `/poll` and the CRUD routes in `include/blog/handlers/membership/automations/routes.cppm` have counterparts in other Ghost controllers (`automation-action-links.js`, `automation-performance-stats.js`, `automation-runs.js`, `automation-run-history.js`, `automations.js`). Do not delete or gate any of them; tests close check 1 by grepping `api.automations.<key>` in `admin/routes.js`.

## Applied C++ source fixes (manifest group 32)

Applied in `include/blog/handlers/membership/automations` only:
`operations.cppm` (flat `{html,plaintext,subject}` preview output derived from
the Lexical text nodes; structural email validation; `valid_lexical` requiring
`type:"doc"` + `root.children[]`; `kTestEmailsSent` counter retained; new
`request_poll()` cache method), the two steps (`path_id` from `in.get_path()`,
context-local automation existence check → 404, no dead `mutations.create`;
email_test writes an outbox row into the existing `emails` resource), the two
handlers (map payload, `X-Cache-Invalidate:false` on email_test), and
`routes.cppm` (`.use(require_auth)` on the shared `/automations` router).
Step `Output` is `std::map<std::string,std::string>`; static `s_ctx` retained;
no new partition, no new global state. Ctor of `AutomationsOperations` is
unchanged — context access happens in the steps via `s_ctx`, not in the ops.

**Central registrations needed (NOT applied — outside allowed folder):**
- `include/blog/testing/api_test.cc:167-192` is stale: it sends
  `{"subject":"...","lexical":"{}"}` and asserts the old
  `{"automation_email_previews":[...]}` envelope. New behavior returns the flat
  `{html,plaintext,subject}` object and rejects `"{}"` with 422
  (`property:"lexical"`). email_test likewise requires a well-formed Lexical doc
  and an existing automation id (the test's hardcoded `/automations/1/...` is a
  webhook id). Update those requests: use a real Lexical document and an id
  created in the same test (e.g. from `POST /automations`).
- No change required in `include/blog/handlers/{all.cppm,routes.cppm,domain_all.cppm,BUILD.bazel}`
  for compilation: no new handler/partition/route.
- Residuals (no allowed primitive): fine-grained `docName/method` permissions,
  `previewEmailLimiter` rate limiting, and SMTP delivery (only a context-local
  outbox row is written).

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
