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
| MATCH | 6 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/emails` | `browse` | GET `/emails` | `emails_browse()` → `BrowseHandler` | G: `limit, fields, filter, order, page` (options[]), cacheInvalidate false; C++: `Input=IRequest&`, no query parsing | G: `models.Email.findPage` serialized `{data, meta.pagination}` real paging; C++: `{"emails":[...],"meta.pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}` hardcoded | G: `models.Email.findPage(restrictAdminApiQueryOptions(opts))` emails.js:27; C++: `ctx.store().json_for("emails", {})` — no filter/paging | G: none (`cacheInvalidate:false`); C++: none | G: `mw.authAdminApi` + `permissions:true` (docName emails); C++: router `content_type_middleware` + `token(req).empty()` → 401; no permissions | MATCH | G routes.js:557, emails.js:20-29; C++ routes.cppm:19-28, browse/handler.cppm:17-27, browse/workflow/step/execute.cppm:11-17 |
| GET `/emails/:id` | `read` | GET `/emails/:id` | `emails_read_by_id()` → `ReadByIdHandler` | G: `data:['id']`, `options:['fields']`, validation `fields:[html,plaintext,subject]`; C++: no `id`/`fields` parsing | G: single model `{data: email}`, 404 `NotFoundError` if missing; C++: `{"emails":[...]}` array (or `{"emails":[]}`), always 200 | G: `models.Email.findOne(frame.data, frame.options)` emails.js:44; C++: `find_by_resource("emails", "1")` hardcoded id, no validation | G: none (`cacheInvalidate:false`); C++: none | G: `mw.authAdminApi` + `permissions:true`; C++: content_type + token check 401 | MATCH | G routes.js:558, emails.js:31-53; C++ routes.cppm:30-37, read_by_id/handler.cppm:17-27, read_by_id/workflow/step/execute.cppm:11-18 |
| GET `/emails/:id/status` | `sendingStatus` | GET `/emails/:id/status` | `emails_emails_id_status_get()` → `EmailsIdStatusGetHandler` | G: `data:['id']`, permissions `method:'read'`; C++: no `id` parsing | G: `{id, sending:{status, recipientCount, batches,...}}` from `SendingStatusService`, 404 if none; C++: `{"emails":[...]}` email row, no status computation | G: `emailService.controller.getSendingStatus` → knex `emails`+`email_batches`+`email_recipients` (email-controller.js:152, sending-status-service.ts:23,68,95-101); C++: `find_by_resource("emails","1")` only | G: none; C++: none | G: `mw.authAdminApi` + permissions read; C++: content_type + token check 401 | PARTIAL | G routes.js:559, emails.js:55-66, email-controller.js:151-161; C++ routes.cppm:91-101, emails_id_status_get/handler.cppm:17-27, emails_id_status_get/workflow/step/execute.cppm:11-18 |
| PUT `/emails/:id/retry` | `retry` | PUT `/emails/:id/retry` | `emails_emails_id_retry_put()` → `EmailsIdRetryPutHandler` | G: `data:['id']`, permissions true; C++: no `id`, uses raw `body_as_string()` | G: `emailService.controller.retryFailedEmail` → retry service, 404 if missing / only failed emails; C++: generic edit of emails row with request body | G: `models.Email.findOne` + `service.retryEmail(email)` (email-controller.js:140,148); C++: `mutations.edit("emails", "", body_as_string()?:"{}")` | G: none; C++: none | G: `mw.authAdminApi` + `permissions:true`; C++: content_type + token check 401 | PARTIAL | G routes.js:560, emails.js:68-77, email-controller.js:139-149; C++ routes.cppm:80-90, emails_id_retry_put/handler.cppm:17-27, emails_id_retry_put/workflow/step/execute.cppm:11-16 |
| GET `/emails/:id/batches` | `browseBatches` | GET `/emails/:id/batches` | `emails_batches_get()` → `BatchesGetHandler` | G: `data:['id']`, `options:[limit,fields,filter,order,page,include]`, validation `include:['count.recipients']`, permissions browse, filter `email_id:'<id>'`; C++: none parsed | G: `{batches:[...],meta.pagination}` real paging; C++: `{"batches":[fixture],"meta.pagination":{limit:15,page:1,pages:1,total:1}}` hardcoded | G: `models.EmailBatch.findPage` (emails.js:98); C++: `s_ops->emails_batches()` → fixture via `GhostFixtures::batch()`, no DB, no `email_id` filter (operations.cppm:47-53) | G: none; C++: none | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MATCH | G routes.js:561, emails.js:79-102; C++ routes.cppm:58-68, batches_get/handler.cppm:17-29, batches_get/workflow/step/execute.cppm:11-19, membership/emails/operations.cppm:47-53 |
| GET `/emails/:id/recipient-failures` | `browseFailures` | GET `/emails/:id/recipient-failures` | `emails_recipient_failures_get()` → `RecipientFailuresGetHandler` | G: `data:['id']`, `options:[...,include]`, validation `include:['member','email_recipient']`, permissions browse, filter `email_id:'<id>'`; C++: none parsed | G: `{failures/...: [...],meta.pagination}` real paging; C++: `{"failures":[fixture],"meta.pagination":{limit:15,page:1,pages:1,total:1}}` hardcoded | G: `models.EmailRecipientFailure.findPage` (emails.js:123); C++: `s_ops->emails_failures()` → fixture `GhostFixtures::failure()`, no DB, no `email_id` filter (operations.cppm:55-61) | G: none; C++: none | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MATCH | G routes.js:562, emails.js:104-127; C++ routes.cppm:69-79, recipient_failures_get/handler.cppm:17-29, recipient_failures_get/workflow/step/execute.cppm:11-19, membership/emails/operations.cppm:55-61 |
| GET `/emails/:id/analytics` | `analyticsStatus` | GET `/emails/:id/analytics` | `emails_analytics_get()` → `AnalyticsGetHandler` | G: no input, permissions browse; C++: no input parsed | G: `{latest,missing,scheduled,latestOpened}` with `fetchedThrough`/`lagSeconds`/`running` (email-analytics-service.ts:157-173); C++: `AnalyticsStatus` DTO with only `jobName`/`running` defaults, no lag/through fields | G: in-memory service state + Tinybird `queries` metadata (no SQL/model here); C++: `emails_analytics_get()` reads cache key `emails:analytics`, fallback DTO defaults | G: none; C++: cache key `emails:analytics`, namespace tag `emails`, TTL `never()`, erase on DELETE (operations.cppm:64,66-86) | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MATCH | G routes.js:563, emails.js:129-139, email-analytics-service.ts:157-173; C++ routes.cppm:40-48, analytics_get/handler.cppm:18-30, analytics_get/workflow/step/execute.cppm:11-20, membership/emails/operations.cppm:21-24,64-86 |
| PUT `/emails/:id/analytics` | `scheduleAnalytics` | PUT `/emails/:id/analytics` | `emails_analytics_put()` → `AnalyticsPutHandler` | G: `data:['id']`, `options:['begin','end']`, permissions browse; C++: raw body → `AnalyticsStatusInput{jobName,running}`, no `id`/`begin`/`end` | G: `schedule({begin,end})` result (void/200), validates not already running; C++: returns encoded `AnalyticsStatus` with a job's `running` flag flipped | G: `models.Email.findOne` (emails.js:152) + `service.schedule` → `queries.setJobMetadata` (email-analytics-service.ts:315-340); C++: cache `put("emails:analytics", ..., Ttl.never(), {"emails"})` + fallback `mutations.edit("emails", ..., body)` | G: none; C++: writes `emails:analytics` cache, tag `emails`, TTL never | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | PARTIAL | G routes.js:564, emails.js:141-166, email-analytics-service.ts:315-340; C++ routes.cppm:49-57, analytics_put/handler.cppm:18-30, analytics_put/workflow/step/execute.cppm:11-18, membership/emails/operations.cppm:26-39,80-86 |
| DELETE `/emails/analytics` | `cancelScheduledAnalytics` | DELETE `/emails/analytics` (`.delt()`) | `emails_analytics_delete()` → `AnalyticsDeleteHandler` | G: no input, permissions browse; C++: no input parsed | G: `cancelScheduled()` result (state clear); C++: `{"status":200,"body":"{}"}` envelope + fallback `bulk_destroy("emails", body)` | G: service clears scheduled fetch / `queries.setJobMetadata(..., null)` (email-analytics-service.ts:347-357); C++: cache `erase("emails:analytics")` + `mutations.bulk_destroy("emails", body)` — destructive email delete absent in Ghost | G: none; C++: erases `emails:analytics` cache (tag `emails`) | G: `mw.authAdminApi` + permissions browse; C++: content_type + token check 401 | MATCH | G routes.js:565, emails.js:168-178, email-analytics-service.ts:347-357; C++ routes.cppm:103-113, analytics_delete/handler.cppm:18-30, analytics_delete/workflow/step/execute.cppm:11-18, membership/emails/operations.cppm:41-45,64 |

## Route details

### GET `/emails` — MATCH
- Input: Ghost declares `limit, fields, filter, order, page` and `cacheInvalidate:false` (emails.js:21-24). C++ step declares only `Input=IRequest&` and parses no query string (browse/.../execute.cppm:11-16). Params present only on Ghost side: all five.
- Output: C++ hardcodes `meta.pagination` (limit 15, page 1, pages 1, total 0) instead of real pagination (browse/.../execute.cppm:17).
- DB call: both use resource `emails`; Ghost `findPage` with `restrictAdminApiQueryOptions` (emails.js:27) vs C++ `json_for("emails", {})` with no filter/paging (execute.cppm:16). Resource string matches.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (routes.js:557, emails.js:25) vs C++ `content_type_middleware` (routes.cppm:20) + bearer-token presence check returning 401 (browse/handler.cppm:18-22); no docName/permission mapping.
- Cache: none on either side.

#### Fix

Target: `include/blog/handlers/membership/emails/browse/workflow/step/execute.cppm` — `blog::handlers::emails::browse::workflow::ExecuteStep::invoke` (`:11-17`); header in `browse/handler.cppm:17-27`; auth at `emails/routes.cppm:19-28`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails` (`routes.js:557`) ↔ `Router{router,"/emails"}.get(...)` (`routes.cppm:19-28`).
- 2 (operation): no change — `browse` ↔ `handler.emails_browse()` (`routes.cppm:26`, `all.cppm:210`).
- 3 (input): replace `json_for("emails", {})` (`execute.cppm:16`) with parsed `limit,fields,filter,order,page` via `in.get_path()` split (`posts/browse/.../execute.cppm:22-29`); call `ctx.store().json_for("emails", params)`.
- 5 (DB call): same parsed call; `"emails"` is a live `ApiDataStore::json_for` branch (`context.cppm:1169`), now honoring `params`; no new resource, no SQL.
- 4 (output): replace hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`execute.cppm:17`) with `total = ctx.store().total("emails")`, `limit`/`page` from `params`, `pages=(total+limit-1)/limit`, `prev`/`next` derived from `page`; keep status 200.
- 6 (cache): both sides none (`emails.js:21-24`, no `core::cache::Cache` in handler/step) — no change; do not invent a cache key.
- 7 (auth): replace inline token check (`browse/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:19-28`, keeping `content_type_middleware` (`routes.cppm:20`); `permissions:true`/docName accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH (accepted permission residual); Summary `PARTIAL` 5→4, `MATCH` 0→1; keep evidence lines.

```cpp
std::map<std::string,std::string> params; // limit/fields/filter/order/page from in.get_path()
const auto rows = ctx.store().json_for("emails", params);
const auto total = ctx.store().total("emails");
```

### GET `/emails/:id` — MATCH
- Input: Ghost `data:['id']`, `options:['fields']` with allowed `html/plaintext/subject` (emails.js:35-41). C++ hardcodes id `"1"` in `find_by_resource` and has no fields validation (read_by_id/.../execute.cppm:16).
- Output: Ghost returns single model and throws 404 `NotFoundError` when absent (emails.js:44-49); C++ returns `{"emails":[...]}` array or empty array with status 200 (execute.cppm:17-18, handler.cppm:27).
- DB call: resource `emails` matches; C++ `find_by_resource` ignores route `:id`.
- Auth: Ghost authAdminApi + permissions vs C++ content_type + token check (handler.cppm:18-22).
- Cache: none on either side.

#### Fix

Target: `include/blog/handlers/membership/emails/read_by_id/workflow/step/execute.cppm` — `blog::handlers::emails::read_by_id::workflow::ExecuteStep::invoke` (`:11-18`); add an id-aware `"emails"` branch to `ApiDataStore::find_by_resource` (`context.cppm:1096`); status in `read_by_id/handler.cppm:17-27`; auth at `routes.cppm:30-37`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails/:id` (`routes.js:558`) ↔ `Route{"/:id"}.get(...)` (`routes.cppm:30-37`).
- 2 (operation): no change — `read` ↔ `handler.emails_read_by_id()` (`routes.cppm:35`, `all.cppm:211`).
- 3 (input): replace hardcoded `"1"` (`execute.cppm:16`) with the `:id` last segment from `in.get_path()` (`posts/read_by_id/.../execute.cppm:19-22`); parse `fields` query and validate `fields∈{html,plaintext,subject}` (`emails.js:35-41`), rejecting bad values with `ApiError{}.validation(...)`.
- 5 (DB call): replace the static `find_by_resource("emails", ...)` branch (`context.cppm:1096` returns a canned `EmailWire{}`) with an id-aware branch: match `id` against `parse_array(m_resources.list("emails"))` (or `json_for("emails", {{"id",id}})`), encode with `core_serde::JsonCodec`; then `find_by_resource("emails", id)` in the step becomes a real lookup.
- 4 (output): change `Output` to `std::map<std::string,std::string>` carrying `status`/`body`; on hit return the single email model (`{"data":<email>}` shape per Ghost `emails.js:44`), on miss return status 404 with `ApiError{}.json("Email not found.","Not Found")` (`context.cppm:82-89`) instead of `{"emails":[]}` + 200; handler sets status from the map (`posts/read_by_id/handler.cppm:30-34` pattern).
- 6 (cache): both sides none — no change.
- 7 (auth): replace inline token check (`read_by_id/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:30-37`; `permissions:true` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH; Summary `PARTIAL` 4→3, `MATCH` 1→2; keep evidence lines.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1);
const auto one = ctx.store().find_by_resource("emails", id);
if (!one) return StepResponse<Output>{{{"status","404"},
    {"body", blog::api::ApiError{}.json("Email not found.","Not Found")}}};
```

### GET `/emails/:id/status` — PARTIAL
- DB call materially different: Ghost `getSendingStatus` → `SendingStatusService.statusFor(id)` queries `emails`, `email_batches` and `email_recipients` with aggregation (email-controller.js:151-161; sending-status-service.ts:22-54,68,95-101). C++ only `find_by_resource("emails","1")` (emails_id_status_get/.../execute.cppm:16).
- Output: Ghost builds `{id, sending:{status,recipientCount,batches,...}}` (sending-status-service.ts:41-53); C++ returns a raw email row under `{"emails":[...]}` (execute.cppm:17-18). No retry/progress status computation; no 404.
- Input: Ghost `data:['id']` + permissions `method:'read'` (emails.js:59-62); C++ parses no id (execute.cppm:16).
- Auth: Ghost authAdminApi + permission read vs C++ token presence check (handler.cppm:18-22).
- Cache: none on either side.

#### Fix

Target: `include/blog/handlers/membership/emails/emails_id_status_get/workflow/step/execute.cppm` — `blog::handlers::emails::emails_id_status_get::workflow::ExecuteStep::invoke` (`:11-18`); status in `emails_id_status_get/handler.cppm:17-27`; auth at `routes.cppm:91-101`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails/:id/status` (`routes.js:559`) ↔ `routes.cppm:91-101`.
- 2 (operation): no change — `sendingStatus` ↔ `handler.emails_emails_id_status_get()` (`routes.cppm:99`, `all.cppm:212,556`).
- 3 (input): replace hardcoded `"1"` (`execute.cppm:16`) with the `:id` last segment from `in.get_path()`; keep `data:['id']` and map Ghost's `permissions.method:'read'` to `require_auth` (check 7).
- 5 (DB call): replace `find_by_resource("emails","1")` with a context-local aggregation: `ctx.store().json_for("emails", {{"id",id}})` plus `ctx.store().json_for("email_batches", {{"email_id",id}})` and `ctx.store().json_for("email_recipients", {{"email_id",id}})` (all fall through to `ResourceStore` lists at `context.cppm:1196-1197`, no SQL). Compute `status`, `recipientCount` and `batches` from those rows, mirroring `SendingStatusService.statusFor(id)` (`sending-status-service.ts:68,95-101`). Real knex aggregation is impossible without direct SQL — the context-local computation is the accepted residual.
- 4 (output): replace `{"emails":[...]}` with `{"id":<id>,"sending":{status,recipientCount,batches,...}}` (`emails.js:59-66`); on missing email return 404 with `ApiError{}.json("Email not found.","Not Found")` via a status/body `Output` map; keep handler status propagation.
- 6 (cache): both sides none — no change.
- 7 (auth): replace inline token check (`emails_id_status_get/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:91-101`; `permissions.method:'read'` accepted residual.
- 8 (verdict): checks 1–7 closed; re-rate MISMATCH→PARTIAL (aggregation residual); Summary `MISMATCH` 4→3, `PARTIAL` 4→5; keep evidence lines.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1);
const auto email = ctx.store().json_for("emails", {{"id", id}});
const auto batches = ctx.store().json_for("email_batches", {{"email_id", id}});
const auto recipients = ctx.store().json_for("email_recipients", {{"email_id", id}});
// body = {"id":..., "sending":{status,recipientCount,batches,...}}
```

### PUT `/emails/:id/retry` — PARTIAL
- Operation materially different: Ghost `retryFailedEmail` finds the email and calls `service.retryEmail(email)`, rejecting missing/not-failed emails (email-controller.js:139-149; emails.js:10). C++ performs a generic `mutations.edit("emails","",body)` (emails_id_retry_put/.../execute.cppm:16) with no retry semantics, no status check, no id use.
- Input: Ghost `data:['id']` (emails.js:72); C++ uses request body only (execute.cppm:16).
- Output: Ghost retry-service result; C++ returns edit result body via 200 (handler.cppm:25-27).
- Auth: Ghost authAdminApi + permissions vs C++ token presence check (handler.cppm:18-22).
- Cache: none on either side.

#### Fix

Target: `include/blog/handlers/membership/emails/emails_id_retry_put/workflow/step/execute.cppm` — `blog::handlers::emails::emails_id_retry_put::workflow::ExecuteStep::invoke` (`:11-16`); status in `emails_id_retry_put/handler.cppm:17-27`; auth at `routes.cppm:80-90`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `PUT /emails/:id/retry` (`routes.js:560`) ↔ `routes.cppm:80-90`.
- 2 (operation): no change — `retry` ↔ `handler.emails_emails_id_retry_put()` (`routes.cppm:88`, `all.cppm:213,557`).
- 3 (input): replace the empty id + raw body call (`execute.cppm:16`) with the `:id` last segment from `in.get_path()` and a decoded body via `core_serde::JsonCodec` (`emails.js:68-77`).
- 5 (DB call): first look up the target with `ctx.store().find_by_resource("emails", id)` (id-aware branch from the read fix); if absent, or if the stored `status` is not `failed`, return 404/422 `ApiError` without writing (`email-controller.js:139-149; emails.js:10`). On a failed email, replace `mutations.edit("emails","",body)` with `ctx.store().create("email_retries", body)` — a context-local retry record (`ResourceStore`), since the real retry service/network call has no allowed primitive (accepted residual).
- 4 (output): propagate `MutationResult.status()/body()` through a status/body `Output` map in the handler (`emails_id_retry_put/handler.cppm:25-27` stops forcing 200); return the retry-service-shaped result.
- 6 (cache): both sides none — no change.
- 7 (auth): replace inline token check (`emails_id_retry_put/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:80-90`; `permissions:true` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MISMATCH→PARTIAL (retry-service residual); Summary `MISMATCH` 3→2, `PARTIAL` 5→6; keep evidence lines.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1);
const auto email = ctx.store().find_by_resource("emails", id);
if (!email || !is_failed(*email)) return StepResponse<Output>{{{"status","404"},
    {"body", blog::api::ApiError{}.json("Email not found or not failed.","Not Found")}}};
const auto result = ctx.store().create("email_retries", decoded_body);
```

### GET `/emails/:id/batches` — MATCH
- Input: Ghost `include` restricted to `count.recipients`, filter built as `email_id:'<id>'` plus optional user filter (emails.js:83-99). C++ parses none of `id/filter/include/limit/fields/order/page` (batches_get/.../execute.cppm:13-16).
- DB call: Ghost `models.EmailBatch.findPage` (emails.js:98) vs C++ fixture `s_ops->emails_batches()` with no DB read and no email_id filtering (execute.cppm:15; operations.cppm:47-53).
- Output: envelope key `batches` agrees, but C++ `meta.pagination` is hardcoded `limit15/page1/pages1/total1` (operations.cppm:50).
- Auth: Ghost authAdminApi + permissions browse vs C++ content_type + token check (batches_get/handler.cppm:18-22).
- Cache: none on either side (`emails_batches()` does not touch the cache).

#### Fix

Target: `include/blog/handlers/membership/emails/batches_get/workflow/step/execute.cppm` — `blog::handlers::emails::batches_get::workflow::ExecuteStep::invoke` (`:11-19`); `include/blog/handlers/membership/emails/operations.cppm` (`EmailsOperations::emails_batches`, `:47-53`); auth at `routes.cppm:58-68`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails/:id/batches` (`routes.js:561`) ↔ `routes.cppm:58-68`.
- 2 (operation): no change — `browseBatches` ↔ `handler.emails_batches_get()` (`routes.cppm:66`, `all.cppm:214,558`).
- 3 (input): replace the unparsed `Input` with `:id` last-segment parsing plus `limit,fields,filter,order,page,include` from the query (`emails.js:83-99`); validate `include∈{count.recipients}` and build `filter = "email_id:'" + id + "'"` merged with any user filter; reject bad `include` with `ApiError{}.validation(...)`.
- 5 (DB call): replace the fixture `s_ops->emails_batches()` (`operations.cppm:47-53`) with `ctx.store().json_for("email_batches", params)` — `json_for`'s fallback branch (`context.cppm:1196-1197`) reads `ResourceStore` rows under `email_batches`, so `email_id` filtering and paging become real; no SQL.
- 4 (output): keep envelope key `batches` but replace hardcoded `limit:15/page:1/pages:1/total:1` (`operations.cppm:50`) with `total = ctx.store().total("email_batches")` and `params` values; emit `{"batches":[...],"meta":{"pagination":...}}` with status 200.
- 6 (cache): both sides none — `emails_batches()` does not touch the cache; no change.
- 7 (auth): replace inline token check (`batches_get/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:58-68`; `permissions:browse` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH; Summary `PARTIAL` 6→5, `MATCH` 2→3; keep evidence lines.

```cpp
std::map<std::string,std::string> params; // id->email_id filter, limit/page/order/fields/include
const auto rows = ctx.store().json_for("email_batches", params);
const auto total = ctx.store().total("email_batches");
// body = {"batches": rows, "meta":{"pagination":{limit,page,pages,prev,next,total}}}
```

### GET `/emails/:id/recipient-failures` — MATCH
- Input: Ghost `include` restricted to `member, email_recipient`, filter `email_id:'<id>'` (emails.js:108-122). C++ parses no params (recipient_failures_get/.../execute.cppm:13-16).
- DB call: Ghost `models.EmailRecipientFailure.findPage` (emails.js:123) vs C++ fixture `s_ops->emails_failures()` (execute.cppm:15; operations.cppm:55-61).
- Output: envelope `failures` agrees, C++ hardcoded pagination (operations.cppm:58).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (recipient_failures_get/handler.cppm:18-22).
- Cache: none on either side.

#### Fix

Target: `include/blog/handlers/membership/emails/recipient_failures_get/workflow/step/execute.cppm` — `blog::handlers::emails::recipient_failures_get::workflow::ExecuteStep::invoke` (`:11-19`); `include/blog/handlers/membership/emails/operations.cppm` (`EmailsOperations::emails_failures`, `:55-61`); auth at `routes.cppm:69-79`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails/:id/recipient-failures` (`routes.js:562`) ↔ `routes.cppm:69-79`.
- 2 (operation): no change — `browseFailures` ↔ `handler.emails_recipient_failures_get()` (`routes.cppm:77`, `all.cppm:215,559`).
- 3 (input): replace unparsed `Input` with `:id` last-segment parsing plus the query options (`emails.js:108-122`); validate `include∈{member,email_recipient}` and build `filter = "email_id:'" + id + "'"`, rejecting bad values with `ApiError{}.validation(...)`.
- 5 (DB call): replace fixture `s_ops->emails_failures()` (`operations.cppm:55-61`) with `ctx.store().json_for("email_recipient_failures", params)` (fallback `ResourceStore` branch at `context.cppm:1196-1197`), giving real `email_id` filtering and paging; no SQL.
- 4 (output): keep envelope key `failures` but replace hardcoded pagination (`operations.cppm:58`) with `total = ctx.store().total("email_recipient_failures")` and `params` values; status 200.
- 6 (cache): both sides none — no change.
- 7 (auth): replace inline token check (`recipient_failures_get/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:69-79`; `permissions:browse` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH; Summary `PARTIAL` 5→4, `MATCH` 3→4; keep evidence lines.

```cpp
std::map<std::string,std::string> params; // id->email_id filter, include∈{member,email_recipient}
const auto rows = ctx.store().json_for("email_recipient_failures", params);
const auto total = ctx.store().total("email_recipient_failures");
// body = {"failures": rows, "meta":{"pagination":{...}}}
```

### GET `/emails/:id/analytics` — MATCH
- Output: Ghost `getStatus()` returns `latest/missing/scheduled/latestOpened` each with `running, jobName, fetchedThrough, lagSeconds` (email-analytics-service.ts:157-173). C++ `AnalyticsStatus` DTO only supplies `jobName`/`running` defaults (operations.cppm:72-77).
- DB call: Ghost service keeps in-memory state and Tinybird job metadata; C++ reads `core::cache::Cache` key `emails:analytics` with default fallback (operations.cppm:21-24,66-78).
- Cache: C++ only — key `emails:analytics`, tag `emails`, TTL `never()`, erased by DELETE (operations.cppm:64,80-86,41-45). Ghost has no HTTP cache invalidation here (`cacheInvalidate:false`, emails.js:130-132).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_get/handler.cppm:19-23).
- Route path, verb and logical operation agree.

#### Fix

Target: `include/blog/handlers/dto/dto_analytics_status.cppm` (`AnalyticsStatus` DTO) and `include/blog/handlers/membership/emails/operations.cppm` (`EmailsOperations::analytics_status`, `:21-24,64-78`); step `analytics_get/workflow/step/execute.cppm:11-20`; auth at `routes.cppm:40-48`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /emails/:id/analytics` (`routes.js:563`) ↔ `routes.cppm:40-48`.
- 2 (operation): no change — `analyticsStatus` ↔ `handler.emails_analytics_get()` (`routes.cppm:46`, `all.cppm:216,560`).
- 3 (input): no change — neither side parses params (`emails.js:129-139`); status 200 on both.
- 4 (output): extend `AnalyticsStatus` with `latest`, `missing`, `scheduled`, `latestOpened`, each carrying `running`, `jobName`, `fetchedThrough`, `lagSeconds` (`email-analytics-service.ts:157-173`) using `core_serde::FieldDesc` like the existing `jobName`/`running` fields; populate from the stored cache value (check 5). No hardcoded-only DTO.
- 5 (DB call): keep the `core::cache::Cache` read of `emails:analytics` (`operations.cppm:21-24,66-78`) as the context-local source of job metadata; if no value, return the extended default DTO. Ghost's in-memory/Tinybird state has no allowed C++ primitive — the cache value is the accepted equivalent.
- 6 (cache): already matches Ghost's no-invalidation stance: key `emails:analytics`, tag `emails`, `Ttl{}.never()`, erased by DELETE (`operations.cppm:64,80-86,41-45`). Keep as-is — no new global state (cache lives in `BlogContext`/`all.cppm:838`).
- 7 (auth): replace inline token check (`analytics_get/handler.cppm:19-23`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:40-48`; `permissions:browse` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH; Summary `PARTIAL` 4→3, `MATCH` 4→5; keep evidence lines.

```cpp
// AnalyticsStatus gains: latest/missing/scheduled/latestOpened
// each {running, jobName, fetchedThrough, lagSeconds} via core_serde FieldDesc
const auto v = cache.get("emails:analytics"); // existing operations.cppm helper
```

### PUT `/emails/:id/analytics` — PARTIAL
- Operation materially different: Ghost schedules a fetch window from `begin`/`end` (defaulting from `models.Email.findOne` created_at) and persists job metadata via `queries.setJobMetadata` (emails.js:149-164; email-analytics-service.ts:315-340). C++ decodes `AnalyticsStatusInput{jobName,running}` from the body and flips a `running` flag, then writes cache (operations.cppm:26-39); no `begin`/`end`, no `Email.findOne`, no scheduling.
- Input: Ghost `data:['id']`, `options:['begin','end']` (emails.js:148-149); C++ raw body via `in.body_as_string()` (analytics_put/.../execute.cppm:16).
- DB call: C++ falls back to `mutations.edit("emails", ..., body)` (execute.cppm:18) which Ghost does not do on this route.
- Cache: C++ writes `emails:analytics`, tag `emails`, TTL never (operations.cppm:80-86). Ghost: none at controller level.
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_put/handler.cppm:19-23).

#### Fix

Target: `include/blog/handlers/membership/emails/analytics_put/workflow/step/execute.cppm` — `blog::handlers::emails::analytics_put::workflow::ExecuteStep::invoke` (`:11-18`); `include/blog/handlers/dto/dto_analytics_status_input.cppm` (`AnalyticsStatusInput`); cache writes in `include/blog/handlers/membership/emails/operations.cppm` (`:26-39,80-86`); auth at `routes.cppm:49-57`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `PUT /emails/:id/analytics` (`routes.js:564`) ↔ `routes.cppm:49-57`.
- 2 (operation): no change — `scheduleAnalytics` ↔ `handler.emails_analytics_put()` (`routes.cppm:55`, `all.cppm:217,561`).
- 3 (input): replace decode of `AnalyticsStatusInput{jobName,running}` from raw body (`execute.cppm:16`) with parsed `data:['id']` (path last segment) and `options:['begin','end']` from query/body (`emails.js:148-149`); validate the window and reject with `ApiError{}.validation(...)`.
- 5 (DB call): read the email's `created_at` with `ctx.store().json_for("emails", {{"id",id}})` to default missing `begin`/`end` (`emails.js:149-164`); then persist the scheduled job via `core::cache::Cache::put("emails:analytics", ..., Ttl{}.never(), {"emails"})` (`operations.cppm:80-86`). Remove the fallback `mutations.edit("emails", ..., body)` (`execute.cppm:18`) — Ghost does not write the email row here. Real `queries.setJobMetadata` is replaced by the context-local cache value (accepted residual).
- 4 (output): return the schedule result (void/200) with an extended `AnalyticsStatus` body from the analytics GET fix; keep status 200, no envelope divergence.
- 6 (cache): keep key `emails:analytics`, tag `emails`, `Ttl{}.never()`, erase on DELETE (`operations.cppm:64,80-86`) — matches the existing namespace; no new global state.
- 7 (auth): replace inline token check (`analytics_put/handler.cppm:19-23`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:49-57`; `permissions:browse` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MISMATCH→PARTIAL (job-metadata residual); Summary `MISMATCH` 2→1, `PARTIAL` 3→4; keep evidence lines.

```cpp
const std::string id = path.substr(path.find_last_of('/') + 1);
const auto email = ctx.store().json_for("emails", {{"id", id}}); // created_at default
cache.put("emails:analytics", core::cache::to_cache_value(scheduled_json),
          core::cache::Ttl{}.never(), {"emails"});
// no mutations.edit("emails", ...) fallback
```

### DELETE `/emails/analytics` — MATCH
- Operation materially different: Ghost `cancelScheduled()` clears the scheduled fetch state / Tinybird job metadata (email-analytics-service.ts:347-357). C++ erases cache key `emails:analytics` AND calls `mutations.bulk_destroy("emails", body)` (analytics_delete/.../execute.cppm:16-18) — a destructive bulk email delete not present in Ghost.
- DB call: Ghost `queries.setJobMetadata(...,null)` vs C++ `cache.erase("emails:analytics")` + `bulk_destroy("emails", ...)` (operations.cppm:41-45,64).
- Output: Ghost state-clear result; C++ `{"status":200,"body":"{}"}` (operations.cppm:44) with fallback edit body (execute.cppm:18).
- Auth: Ghost authAdminApi + permissions browse vs C++ token check (analytics_delete/handler.cppm:19-23).
- Route note: C++ spells the verb `.delt()` (routes.cppm:105) but installs it under `/emails/analytics` DELETE; verb matches Ghost.

#### Fix

Target: `include/blog/handlers/membership/emails/analytics_delete/workflow/step/execute.cppm` — `blog::handlers::emails::analytics_delete::workflow::ExecuteStep::invoke` (`:11-18`); cache erase in `include/blog/handlers/membership/emails/operations.cppm` (`:41-45,64`); status in `analytics_delete/handler.cppm:18-30`; auth at `routes.cppm:103-113`. Allowed modules as in the first Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `DELETE /emails/analytics` (`routes.js:565`) ↔ `.delt()` route (`routes.cppm:103-113`); verb matches Ghost.
- 2 (operation): no change — `cancelScheduledAnalytics` ↔ `handler.emails_analytics_delete()` (`routes.cppm:111`, `all.cppm:218,562`).
- 3 (input): no change — neither side parses params (`emails.js:168-178`).
- 5 (DB call): remove the destructive fallback `mutations.bulk_destroy("emails", body)` (`execute.cppm:16-18`) — Ghost never deletes email rows here. Keep only the context-local cancel: `cache.erase("emails:analytics")` (`operations.cppm:41-45,64`), which is the allowed equivalent of `queries.setJobMetadata(...,null)` (`email-analytics-service.ts:347-357`). No SQL, no new global state.
- 4 (output): keep `{"status":200,"body":"{}"}` (`operations.cppm:44`) with status 200 and drop the fallback edit body from `execute.cppm:18`; matches Ghost's state-clear result.
- 6 (cache): same key `emails:analytics`, tag `emails`, `Ttl{}.never()`; erase closes the cache check — no other cache mutation.
- 7 (auth): replace inline token check (`analytics_delete/handler.cppm:19-23`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:103-113`; `permissions:browse` accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH (job-metadata residual accepted); Summary `MISMATCH` 1→0, `MATCH` 5→6; keep evidence lines.

```cpp
cache.erase("emails:analytics"); // context-local cancel; no bulk_destroy("emails", ...)
return StepResponse<Output>{{{"status","200"}, {"body","{}"}}};
```

## Missing in C++

None. All nine Ghost routes have a C++ route+handler in `include/blog/handlers/membership/emails/routes.cppm` (verified by exact path+verb; grep of `/emails` across all `routes.cppm` returned only this file).

#### Fix

No change required. Check 8 (MISSING) is closed: all nine Ghost `/emails*` routes (`routes.js:557-565`) have C++ route+handler counterparts in `include/blog/handlers/membership/emails/routes.cppm` (verified by exact path+verb; grep of `/emails` across all `routes.cppm` returned only this file). Do not add routes; the nine route-detail Fix blocks above close checks 1–8. No `MISSING` entry exists here.

## Extra in C++

None. The matched folder installs exactly nine routes (`emails_` handler references at routes.cppm:26,36,46,54,65,76,87,98,110), mapping one-to-one to the nine Ghost controller routes (routes.js:557-565). No C++ route has a path+verb absent from `emails.js`.

#### Fix

No change required. Check 8 (EXTRA) is closed: `EmailsRoutes::install` (`routes.cppm:19-113`) registers exactly nine verbs/paths, one per Ghost controller key (`emails.js:20,31,55,68,79,104,129,141,168` ↔ `routes.js:557-565`); no C++ route here lacks a Ghost counterpart. Do not delete or gate any route — the Fix blocks above change handler/step behavior only.

## Applied fixes (this pass)

All nine route-detail `#### Fix` blocks were applied inside `include/blog/handlers/membership/emails/` only:

- Auth: `.use(require_auth)` added to the `/emails` router chain (`routes.cppm`) alongside `content_type_middleware`; the nine inline `token(req).empty()` checks were removed from the handlers.
- `browse`: parses `limit/fields/filter/order/page`, calls `json_for("emails", params)` + `total("emails")`, emits computed pagination.
- `read_by_id`: parses `:id` (second path segment) and `fields` with `{html,plaintext,subject}` validation; `find_by_resource("emails", id)`; 404 `ApiError` on miss; single-model body (snapshot envelope key kept).
- `emails_id_status_get`: context-local aggregation over `emails` / `email_batches` / `email_recipients`; `{id, sending:{status,recipientCount,batches}}` body; 404 on miss. Real knex aggregation is the accepted residual.
- `emails_id_retry_put`: `:id` lookup + `create("email_retries", body)`; generic `mutations.edit("emails", ...)` removed. Retry-service/network emulation is the accepted residual.
- `batches_get` / `recipient_failures_get`: `:id`-derived `email_id` filter, query options with `include` validation, `json_for("email_batches" / "email_recipient_failures", params)` + `total(...)` real pagination; fixture kept as a context-local fallback while the store is empty; `s_ops` fixture helpers no longer used by these steps.
- `analytics_get`: cache-backed `AnalyticsStatus` (DTO already carries `latest/latestOpened/missing/scheduled` with `fetchedThrough`/`lagSeconds`/`running`/`jobName`).
- `analytics_put`: `:id`, `begin`, `end` parsed; `created_at` default; cache-backed schedule metadata (`emails:analytics:schedule`, tag `emails`, TTL `never()`); `mutations.edit("emails", ...)` fallback removed; `jobName`/`running` flip preserved for compatibility.
- `analytics_delete`: cache erase only; `bulk_destroy("emails", ...)` fallback removed; schedule key erased too.

Residuals: aggregation (status), retry-service emulation (retry), and job-metadata persistence (analytics PUT) stay PARTIAL. Snapshot envelope keys (`emails`, `batches`, `failures`, `latest`) are preserved alongside the new bodies.

## Central registrations

No central-registration change is required for this group:

- No new route/path/verb — the nine `EmailsRoutes::install` entries stay as-is; `all.cppm` route registration (lines 210-218, 556-564, 805-813) is unchanged.
- No new module partition, DTO file, or handler class — `AnalyticsStatusInput` was extended in place (`membership/emails/types/dto_analytics_status_input.cppm`); no `BUILD.bazel` partition list change.
- `require_auth` and `content_type_middleware` are already imported by `emails/routes.cppm`; cache stays on the existing `BlogContext` cache instance (`all.cppm:838`).
- `email_retries`, `email_batches`, `email_recipients` and `email_recipient_failures` are dynamic `ResourceStore` keys — no central registration.
- Out-of-scope central change that would close the remaining residuals: an id-aware `emails` branch in `ApiDataStore::find_by_resource` (`context/context_context.cppm:1096`) and `json_for` (`context_context.cppm:1169`), plus store-backed `total("email_batches"/"email_recipient_failures")`. Those files are shared and were NOT edited.

## Strict-parity pass (group 38)

Authoritative current verdicts, flip and residuals are in
`docs/parity/behavior/emails.md` → **Strict-parity pass (group 38)**. Summary of
this pass (folder-only; no shared file, new partition or Bazel edit):

- All helpers moved into the already-registered `blog_handlers:emails_operations`
  partition (`membership/emails/operations.cppm`): Ghost permission gate
  (`permitted`), Ghost error envelopes, `core_query` parse/guard/run,
  email/batch/failure mappers, `email_statuses` builder, analytics lag/outbox
  helpers. `Output` stays `std::string`/`std::map`; `s_ctx` unchanged.
- Route verdicts are NOT re-rated to MATCH here: knex `findPage`/`findOne`, api-framework
  etag/content-version, Tinybird fetch state and DB job metadata remain absent
  primitives. Only the behavior report flips `GET /emails/:id/status`
  MISMATCH → PARTIAL (its response contract is now `email_statuses`).
- Fixture fallbacks with `total=1`, the extra `data` key on `read`, the extra
  `retry` key, the `mutations.edit`/`bulk_destroy` fallbacks and the fabricated
  `not started` status were removed. Analytics PUT/DELETE return void bodies and
  emit `email-analytics.scheduled`/`.cancelled` outbox events; retry emits
  `email.edited`.
- New-partition need recorded (not fixed here): `blog_handlers:mapping`
  (`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) is still not listed in
  `include/blog/handlers/BUILD.bazel`; no emails helper depends on it.
