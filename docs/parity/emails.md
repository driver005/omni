# Parity: emails

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/emails.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 556-565)
- C++: `include/blog/handlers/membership/emails/routes.cppm` (folder found by exact path+verb, not assumed)
- C++ registry: `include/blog/handlers/all.cppm` (lines 210-218, 556-564, 805-813, 838)
- C++ cache: `include/blog/handlers/membership/emails/operations.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 4 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/emails` | `browse` | GET `/emails` | `emails_browse()` → `BrowseHandler` | G: `limit, fields, filter, order, page` (options[]), cacheInvalidate false; C++: `Input=IRequest&`, no query parsing | G: `models.Email.findPage` serialized `{data, meta.pagination}` real paging; C++: `{"emails":[...],"meta.pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}` hardcoded | G: `models.Email.findPage(restrictAdminApiQueryOptions(opts))` emails.js:27; C++: `ctx.store().json_for("emails", {})` — no filter/paging | G: none (`cacheInvalidate:false`); C++: none | G: `mw.authAdminApi` + `permissions:true` (docName emails); C++: router `content_type_middleware` + `token(req).empty()` → 401; no permissions | PARTIAL | G routes.js:557, emails.js:20-29; C++ routes.cppm:19-28, browse/handler.cppm:17-27, browse/workflow/step/execute.cppm:11-17 |
| GET `/emails/:id` | `read` | GET `/emails/:id` | `emails_read_by_id()` → `ReadByIdHandler` | G: `data:['id']`, `options:['fields']`, validation `fields:[html,plaintext,subject]`; C++: no `id`/`fields` parsing | G: single model `{data: email}`, 404 `NotFoundError` if missing; C++: `{"emails":[...]}` array (or `{"emails":[]}`), always 200 | G: `models.Email.findOne(frame.data, frame.options)` emails.js:44; C++: `find_by_resource("emails", "1")` hardcoded id, no validation | G: none (`cacheInvalidate:false`); C++: none | G: `mw.authAdminApi` + `permissions:true`; C++: content_type + token check 401 | PARTIAL | G routes.js:558, emails.js:31-53; C++ routes.cppm:30-37, read_by_id/handler.cppm:17-27, read_by_id/workflow/step/execute.cppm:11-18 |
| GET `/emails/:id/status` | `sendingStatus` | GET `/emails/:id/status` | `emails_emails_id_status_get()` → `EmailsIdStatusGetHandler` | G: `data:['id']`, permissions `method:'read'`; C++: no `id` parsing | G: `{id, sending:{status, recipientCount, batches,...}}` from `SendingStatusService`, 404 if none; C++: `{"emails":[...]}` email row, no status computation | G: `emailService.controller.getSendingStatus` → knex `emails`+`email_batches`+`email_recipients` (email-controller.js:152, sending-status-service.ts:23,68,95-101); C++: `find_by_resource("emails","1")` only | G: none; C++: none | G: `mw.authAdminApi` + permissions read; C++: content_type + token check 401 | MISMATCH | G routes.js:559, emails.js:55-66, email-controller.js:151-161; C++ routes.cppm:91-101, emails_id_status_get/handler.cppm:17-27, emails_id_status_get/workflow/step/execute.cppm:11-18 |
| PUT `/emails/:id/retry` | `retry` | PUT `/emails/:id/retry` | `emails_emails_id_retry_put()` → `EmailsIdRetryPutHandler` | G: `data:['id']`, permissions true; C++: no `id`, uses raw `body_as_string()` | G: `emailService.controller.retryFailedEmail` → retry service, 404 if missing / only failed emails; C++: generic edit of emails row with request body | G: `models.Email.findOne` + `service.retryEmail(email)` (email-controller.js:140,148); C++: `mutations.edit("emails", "", body_as_string()?:"{}")` | G: none; C++: none | G: `mw.authAdminApi` + `permissions:true`; C++: content_type + token check 401 | MISMATCH | G routes.js:560, emails.js:68-77, email-controller.js:139-149; C++ routes.cppm:80-90, emails_id_retry_put/handler.cppm:17-27, emails_id_retry_put/workflow/step/execute.cppm:11-16 |
| GET `/emails/:id/batches` | `browseBatches` | GET `/emails/:id/batches` | `emails_batches_get()` → `BatchesGetHandler` | G: `data:['id']`, `options:[limit,fields,filter,order,page,include]`, validation `include:['count.recipients']`, permissions browse, filter `email_id:'<id>'`; C++: none parsed | G: `{batches:[...],meta.pagination}` real paging; C++: `{"batches":[fixture],"meta.pagination":{limit:15,page:1,pages:1,total:1}}` hardcoded | G: `models.EmailBatch.findPage` (emails.js:98); C++: `s_ops->emails_batches()` → fixture via `GhostFixtures::batch()`, no DB, no `email_id` filter (operations.cppm:47-53) | G: none; C++: none | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | PARTIAL | G routes.js:561, emails.js:79-102; C++ routes.cppm:58-68, batches_get/handler.cppm:17-29, batches_get/workflow/step/execute.cppm:11-19, membership/emails/operations.cppm:47-53 |
| GET `/emails/:id/recipient-failures` | `browseFailures` | GET `/emails/:id/recipient-failures` | `emails_recipient_failures_get()` → `RecipientFailuresGetHandler` | G: `data:['id']`, `options:[...,include]`, validation `include:['member','email_recipient']`, permissions browse, filter `email_id:'<id>'`; C++: none parsed | G: `{failures/...: [...],meta.pagination}` real paging; C++: `{"failures":[fixture],"meta.pagination":{limit:15,page:1,pages:1,total:1}}` hardcoded | G: `models.EmailRecipientFailure.findPage` (emails.js:123); C++: `s_ops->emails_failures()` → fixture `GhostFixtures::failure()`, no DB, no `email_id` filter (operations.cppm:55-61) | G: none; C++: none | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | PARTIAL | G routes.js:562, emails.js:104-127; C++ routes.cppm:69-79, recipient_failures_get/handler.cppm:17-29, recipient_failures_get/workflow/step/execute.cppm:11-19, membership/emails/operations.cppm:55-61 |
| GET `/emails/:id/analytics` | `analyticsStatus` | GET `/emails/:id/analytics` | `emails_analytics_get()` → `AnalyticsGetHandler` | G: no input, permissions browse; C++: no input parsed | G: `{latest,missing,scheduled,latestOpened}` with `fetchedThrough`/`lagSeconds`/`running` (email-analytics-service.ts:157-173); C++: `AnalyticsStatus` DTO with only `jobName`/`running` defaults, no lag/through fields | G: in-memory service state + Tinybird `queries` metadata (no SQL/model here); C++: `emails_analytics_get()` reads cache key `emails:analytics`, fallback DTO defaults | G: none; C++: cache key `emails:analytics`, namespace tag `emails`, TTL `never()`, erase on DELETE (operations.cppm:64,66-86) | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | PARTIAL | G routes.js:563, emails.js:129-139, email-analytics-service.ts:157-173; C++ routes.cppm:40-48, analytics_get/handler.cppm:18-30, analytics_get/workflow/step/execute.cppm:11-20, membership/emails/operations.cppm:21-24,64-86 |
| PUT `/emails/:id/analytics` | `scheduleAnalytics` | PUT `/emails/:id/analytics` | `emails_analytics_put()` → `AnalyticsPutHandler` | G: `data:['id']`, `options:['begin','end']`, permissions browse; C++: raw body → `AnalyticsStatusInput{jobName,running}`, no `id`/`begin`/`end` | G: `schedule({begin,end})` result (void/200), validates not already running; C++: returns encoded `AnalyticsStatus` with a job's `running` flag flipped | G: `models.Email.findOne` (emails.js:152) + `service.schedule` → `queries.setJobMetadata` (email-analytics-service.ts:315-340); C++: cache `put("emails:analytics", ..., Ttl.never(), {"emails"})` + fallback `mutations.edit("emails", ..., body)` | G: none; C++: writes `emails:analytics` cache, tag `emails`, TTL never | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MISMATCH | G routes.js:564, emails.js:141-166, email-analytics-service.ts:315-340; C++ routes.cppm:49-57, analytics_put/handler.cppm:18-30, analytics_put/workflow/step/execute.cppm:11-18, membership/emails/operations.cppm:26-39,80-86 |
| DELETE `/emails/analytics` | `cancelScheduledAnalytics` | DELETE `/emails/analytics` (`.delt()`) | `emails_analytics_delete()` → `AnalyticsDeleteHandler` | G: no input, permissions browse; C++: no input parsed | G: `cancelScheduled()` result (state clear); C++: `{"status":200,"body":"{}"}` envelope + fallback `bulk_destroy("emails", body)` | G: service clears scheduled fetch / `queries.setJobMetadata(..., null)` (email-analytics-service.ts:347-357); C++: cache `erase("emails:analytics")` + `mutations.bulk_destroy("emails", body)` — destructive email delete absent in Ghost | G: none; C++: erases `emails:analytics` cache (tag `emails`) | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MISMATCH | G routes.js:565, emails.js:168-178, email-analytics-service.ts:347-357; C++ routes.cppm:103-113, analytics_delete/handler.cppm:18-30, analytics_delete/workflow/step/execute.cppm:11-18, membership/emails/operations.cppm:41-45,64 |

## Route details

### GET `/emails` — PARTIAL
- Input: Ghost declares `limit, fields, filter, order, page` and `cacheInvalidate:false` (emails.js:21-24). C++ step declares only `Input=IRequest&` and parses no query string (browse/.../execute.cppm:11-16). Params present only on Ghost side: all five.
- Output: C++ hardcodes `meta.pagination` (limit 15, page 1, pages 1, total 0) instead of real pagination (browse/.../execute.cppm:17).
- DB call: both use resource `emails`; Ghost `findPage` with `restrictAdminApiQueryOptions` (emails.js:27) vs C++ `json_for("emails", {})` with no filter/paging (execute.cppm:16). Resource string matches.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (routes.js:557, emails.js:25) vs C++ `content_type_middleware` (routes.cppm:20) + bearer-token presence check returning 401 (browse/handler.cppm:18-22); no docName/permission mapping.
- Cache: none on either side.

### GET `/emails/:id` — PARTIAL
- Input: Ghost `data:['id']`, `options:['fields']` with allowed `html/plaintext/subject` (emails.js:35-41). C++ hardcodes id `"1"` in `find_by_resource` and has no fields validation (read_by_id/.../execute.cppm:16).
- Output: Ghost returns single model and throws 404 `NotFoundError` when absent (emails.js:44-49); C++ returns `{"emails":[...]}` array or empty array with status 200 (execute.cppm:17-18, handler.cppm:27).
- DB call: resource `emails` matches; C++ `find_by_resource` ignores route `:id`.
- Auth: Ghost authAdminApi + permissions vs C++ content_type + token check (handler.cppm:18-22).
- Cache: none on either side.

### GET `/emails/:id/status` — MISMATCH
- DB call materially different: Ghost `getSendingStatus` → `SendingStatusService.statusFor(id)` queries `emails`, `email_batches` and `email_recipients` with aggregation (email-controller.js:151-161; sending-status-service.ts:22-54,68,95-101). C++ only `find_by_resource("emails","1")` (emails_id_status_get/.../execute.cppm:16).
- Output: Ghost builds `{id, sending:{status,recipientCount,batches,...}}` (sending-status-service.ts:41-53); C++ returns a raw email row under `{"emails":[...]}` (execute.cppm:17-18). No retry/progress status computation; no 404.
- Input: Ghost `data:['id']` + permissions `method:'read'` (emails.js:59-62); C++ parses no id (execute.cppm:16).
- Auth: Ghost authAdminApi + permission read vs C++ token presence check (handler.cppm:18-22).
- Cache: none on either side.

### PUT `/emails/:id/retry` — MISMATCH
- Operation materially different: Ghost `retryFailedEmail` finds the email and calls `service.retryEmail(email)`, rejecting missing/not-failed emails (email-controller.js:139-149; emails.js:10). C++ performs a generic `mutations.edit("emails","",body)` (emails_id_retry_put/.../execute.cppm:16) with no retry semantics, no status check, no id use.
- Input: Ghost `data:['id']` (emails.js:72); C++ uses request body only (execute.cppm:16).
- Output: Ghost retry-service result; C++ returns edit result body via 200 (handler.cppm:25-27).
- Auth: Ghost authAdminApi + permissions vs C++ token presence check (handler.cppm:18-22).
- Cache: none on either side.

### GET `/emails/:id/batches` — PARTIAL
- Input: Ghost `include` restricted to `count.recipients`, filter built as `email_id:'<id>'` plus optional user filter (emails.js:83-99). C++ parses none of `id/filter/include/limit/fields/order/page` (batches_get/.../execute.cppm:13-16).
- DB call: Ghost `models.EmailBatch.findPage` (emails.js:98) vs C++ fixture `s_ops->emails_batches()` with no DB read and no email_id filtering (execute.cppm:15; operations.cppm:47-53).
- Output: envelope key `batches` agrees, but C++ `meta.pagination` is hardcoded `limit15/page1/pages1/total1` (operations.cppm:50).
- Auth: Ghost authAdminApi + permissions browse vs C++ content_type + token check (batches_get/handler.cppm:18-22).
- Cache: none on either side (`emails_batches()` does not touch the cache).

### GET `/emails/:id/recipient-failures` — PARTIAL
- Input: Ghost `include` restricted to `member, email_recipient`, filter `email_id:'<id>'` (emails.js:108-122). C++ parses no params (recipient_failures_get/.../execute.cppm:13-16).
- DB call: Ghost `models.EmailRecipientFailure.findPage` (emails.js:123) vs C++ fixture `s_ops->emails_failures()` (execute.cppm:15; operations.cppm:55-61).
- Output: envelope `failures` agrees, C++ hardcoded pagination (operations.cppm:58).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (recipient_failures_get/handler.cppm:18-22).
- Cache: none on either side.

### GET `/emails/:id/analytics` — PARTIAL
- Output: Ghost `getStatus()` returns `latest/missing/scheduled/latestOpened` each with `running, jobName, fetchedThrough, lagSeconds` (email-analytics-service.ts:157-173). C++ `AnalyticsStatus` DTO only supplies `jobName`/`running` defaults (operations.cppm:72-77).
- DB call: Ghost service keeps in-memory state and Tinybird job metadata; C++ reads `core::cache::Cache` key `emails:analytics` with default fallback (operations.cppm:21-24,66-78).
- Cache: C++ only — key `emails:analytics`, tag `emails`, TTL `never()`, erased by DELETE (operations.cppm:64,80-86,41-45). Ghost has no HTTP cache invalidation here (`cacheInvalidate:false`, emails.js:130-132).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_get/handler.cppm:19-23).
- Route path, verb and logical operation agree.

### PUT `/emails/:id/analytics` — MISMATCH
- Operation materially different: Ghost schedules a fetch window from `begin`/`end` (defaulting from `models.Email.findOne` created_at) and persists job metadata via `queries.setJobMetadata` (emails.js:149-164; email-analytics-service.ts:315-340). C++ decodes `AnalyticsStatusInput{jobName,running}` from the body and flips a `running` flag, then writes cache (operations.cppm:26-39); no `begin`/`end`, no `Email.findOne`, no scheduling.
- Input: Ghost `data:['id']`, `options:['begin','end']` (emails.js:148-149); C++ raw body via `in.body_as_string()` (analytics_put/.../execute.cppm:16).
- DB call: C++ falls back to `mutations.edit("emails", ..., body)` (execute.cppm:18) which Ghost does not do on this route.
- Cache: C++ writes `emails:analytics`, tag `emails`, TTL never (operations.cppm:80-86). Ghost: none at controller level.
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_put/handler.cppm:19-23).

### DELETE `/emails/analytics` — MISMATCH
- Operation materially different: Ghost `cancelScheduled()` clears the scheduled fetch state / Tinybird job metadata (email-analytics-service.ts:347-357). C++ erases cache key `emails:analytics` AND calls `mutations.bulk_destroy("emails", body)` (analytics_delete/.../execute.cppm:16-18) — a destructive bulk email delete not present in Ghost.
- DB call: Ghost `queries.setJobMetadata(...,null)` vs C++ `cache.erase("emails:analytics")` + `bulk_destroy("emails", ...)` (operations.cppm:41-45,64).
- Output: Ghost state-clear result; C++ `{"status":200,"body":"{}"}` (operations.cppm:44) with fallback edit body (execute.cppm:18).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_delete/handler.cppm:19-23).
- Route note: C++ spells the verb `.delt()` (routes.cppm:105) but installs it under `/emails/analytics` DELETE; verb matches Ghost.

## Missing in C++

None. All nine Ghost routes have a C++ route+handler in `include/blog/handlers/membership/emails/routes.cppm` (verified by exact path+verb; grep of `/emails` across all `routes.cppm` returned only this file).

## Extra in C++

None. The matched folder installs exactly nine routes (`emails_` handler references at routes.cppm:26,36,46,54,65,76,87,98,110), mapping one-to-one to the nine Ghost controller routes (routes.js:557-565). No C++ route has a path+verb absent from `emails.js`.
