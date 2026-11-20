# Behavior report: emails

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/emails.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:556-565` (admin API router, mounted at `/ghost/api/admin`); router-level `router.use(apiMw.cors)` at `admin/routes.js:17`
- C++ folders: `include/blog/handlers/membership/emails/` — installed as `EmailsRoutes{}.install(router, handlers)` at `include/blog/handlers/routes.cppm:91`; declared at `routes.cppm:19`; all nine handlers exported at `base.cppm:252-260`, imported/accessed at `all.cppm:239-247,617-625`, constructed at `all.cppm:896-904`; files listed in `BUILD.bazel:767-793`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (group 38)

> Verdict note (group 38): the per-route Ghost analysis and the `C++`/`Diff` prose below
> are the pre-implementation audit. Current C++ behavior, verdicts and the one flip are
> recorded in **Strict-parity pass (group 38)** after the Summary; that section is
> authoritative where it disagrees with the older prose.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 9 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Strict-parity pass (group 38)

All helpers live in the already-registered `blog_handlers:emails_operations`
partition (`membership/emails/operations.cppm`); no shared file, new partition or
Bazel change was made. Output stays `std::string`/`std::map<std::string,std::string>`
and every step keeps its `s_ctx` pointer.

Shared abstractions now used in-folder:

- `core_query` — `parse` (option allowlist + `filter[x]`/bare `filter`/`order`/
  `fields`/`limit=all`), `has_restricted_field` (password/token/gift_link_token/
  secret guard → Ghost 400), `apply`/`run` (filter, order, field projection,
  real page paging), `pagination_meta` numbers rebuilt in Ghost key order.
- `blog_handlers_middleware` + `core_auth:ghost` — `permitted()` runs
  `require_ghost_permission("emails", method)` with an Administrator/ADMIN_API
  principal derived from the bearer token (explicit `x-blog-*` headers win);
  denial writes the Ghost 403 `NoPermissionError` envelope through `write_error`.
  Method split honoured: `read` (status/read), `browse` (browse, batches,
  failures, analytics GET/PUT/DELETE), `retry` (retry).
- `core_serde` — `JsonCodec`/`Value` for all mapper work, replacing raw string
  substring hacks.
- `core_cache` — analytics job metadata (`emails:analytics`,
  `emails:analytics:schedule`, TTL `never()`, tag `emails`).
- `blog_handlers:outbox_shim` — `email.edited` on retry,
  `email-analytics.scheduled`/`email-analytics.cancelled` on analytics PUT/DELETE.

| Verb + path | New C++ behavior | Verdict | Residual (no in-folder primitive) |
|---|---|---|---|
| GET `/emails` | 5 allowlisted options parsed; restricted-field/order guard → 400 `BadRequestError` body; `mappers/emails.js` equivalent (placeholder replacement, `post` removal, `stripEmailAccounting`); real `core_query` paging over the resource store; `browse` permission | PARTIAL (unchanged) | real `Email.findPage`/knex; api-framework etag/content-version |
| GET `/emails/:id` | `:id` via `core_query` id filter; 404 `Email not found.` on miss; `fields` allowlist 422; single-model `{emails:[…]}` — extra top-level `data` removed; mapper applied; `read` permission | PARTIAL (unchanged) | real `Email.findOne` with DB projection; etag/content-version |
| GET `/emails/:id/status` | `{email_statuses:[{id,sending:{status,progress:{completed,total,estimated_seconds_remaining},failed_during?,retryable?}}]}`; vocabulary `preparing/submitting/submitted/failed`; no fixture row, no `emails` key; `read` permission | **PARTIAL (MISMATCH → PARTIAL, flipped)** | real knex `statusFor` aggregation; multi-sample ETA estimation |
| PUT `/emails/:id/retry` | `Email.findOne` id lookup; 404 missing / 400 `IncorrectUsageError` non-failed; email status flipped `failed→pending`; `email.edited` outbox event; body `{emails:[claimed]}` — extra `retry` key removed; `retry` permission | PARTIAL (unchanged) | transactional status lock, `scheduleEmail` job, eligibility/limit checks |
| GET `/emails/:id/batches` | `email_id:'<id>'` + caller filter via `core_query`; `include⊆{count.recipients}` 422; `mappers/email-batches.ts` field picking; no fixture fallback and no `total=1` fabrication; real paging; `browse` permission | PARTIAL (unchanged) | real `EmailBatch.findPage`; etag/content-version |
| GET `/emails/:id/recipient-failures` | same filter/guard/paging; `include⊆{member,email_recipient}` (+ camel alias); `mappers/email-failures.ts` field picking with `member`/`email_recipient` null fallback; no fixture/total fabrication; `browse` permission | PARTIAL (unchanged) | real `EmailRecipientFailure.findPage`; relation eager-loading; etag |
| GET `/emails/:id/analytics` | raw `{latest,latestOpened,missing,scheduled}` DTO kept; `lagSeconds` computed from `fetchedThrough` (`getStatus()`); cache-backed metadata; `browse` permission | PARTIAL (unchanged) | live Tinybird fetch state / DB job metadata; etag |
| PUT `/emails/:id/analytics` | `begin`/`end` options, `created_at` default, `end = min(now-1h, created_at+7d)`; already-running guard; schedule metadata in cache + `email-analytics.scheduled` outbox event; void body (no analytics status object, no email edit, no 422 branch); `browse` permission | PARTIAL (unchanged) | real `queries.setJobMetadata` DB write; Tinybird fetch pipeline |
| DELETE `/emails/analytics` | cache keys erased + `email-analytics.cancelled` outbox event; void body; `browse` permission | PARTIAL (unchanged) | real DB metadata nulling / Tinybird cancellation |

New-partition needs recorded for the main agent (not fixed here): `blog_handlers:mapping`
(`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) is still absent from
`include/blog/handlers/BUILD.bazel`; no emails helper depends on it — local
`envelope`/`pagination` in `operations.cppm` are used instead. A true
`//include/core/outbox:core_outbox` dependency would replace the in-module
`outbox_shim` if the main agent prefers it.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/emails` | `browse` (docName `emails`) | `options:['limit','fields','filter','order','page']` (`emails.js:24`); no `data`, no `validation` | `mw.authAdminApi` (`admin/routes.js:557`); `permissions:true` (`emails.js:25`) | 200; `cacheInvalidate:false` (`emails.js:21-23`) ⇒ no `X-Cache-Invalidate`; etag/content-version (`emails.test.js:77-80`) | `query` → `models.Email.findPage(restrictAdminApiQueryOptions(frame.options))` (`emails.js:27`); guard adds `mongoTransformer` rejecting `password/token/gift_link_token/secret` and throws `BadRequestError` on restricted `order` (`api-filter-utils.ts:78,218-220,247-289`) | none | `{emails:[…],meta.pagination}`; fields from default serializer + `mappers/emails.js` (replacements, `stripEmailAccounting`); 2 fixture emails (`emails.test.js.snap:488-572`) | `browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; parses 5 allowlisted query keys, `json_for("emails",params)`+`total`, limit/page 15/1, computed `pages/prev/next`; no mongoTransformer/serializer/etag | PARTIAL | G:`emails.js:20-29`, `admin/routes.js:557`, `api-filter-utils.ts:275-289`; C++:`routes.cppm:19-28`, `browse/.../execute.cppm:18-38` |
| GET `/emails/:id` | `read` (docName `emails`) | `data:['id']` path; `options:['fields']`; validation `fields ∈ {html,plaintext,subject}` (`emails.js:35-41`) | `mw.authAdminApi` (`admin/routes.js:558`); `permissions:true` (`emails.js:42`) | 200 / 404; `cacheInvalidate:false` (`emails.js:32-34`) | `models.Email.findOne(frame.data,frame.options)` → `NotFoundError('Email not found.')` if missing (`emails.js:44-49`); serializer `mappers/emails.js:4-36` applies `buildReplacementDefinitions`/`replaceDefinitions` and `stripEmailAccounting` | none | `{emails:[single]}` (`emails.test.js.snap:689-735`); raw `data` member not present | `read_by_id/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; fields validation 422; `find_by_resource("emails",id)`; 404 'Email not found.'; body has BOTH `data` and `emails:[…]` | PARTIAL | G:`emails.js:31-53`, `mappers/emails.js:4-36`; C++:`read_by_id/.../execute.cppm:18-42` |
| GET `/emails/:id/status` | `sendingStatus` (docName `emails`) | `data:['id']` path; no `options`/`validation` (`emails.js:59`) | `mw.authAdminApi` (`admin/routes.js:559`); `permissions:{method:'read'}` (`emails.js:60-62`) | 200 / 404; `cacheInvalidate:false` (`emails.js:56-58`) | `emailService.controller.getSendingStatus(frame)` (`emails.js:64`) → `sendingStatusService.statusFor(id)` (`email-controller.js:151-161`) → knex `emails` row + `#batchesFor` knex aggregation (`sending-status-service.ts:22-54,74-104`) → `buildSendingStatus` (`sending-status.ts:52-112`) → `toEmailStatusesResponse` (`sending-status-serializers.ts:25-40`) | none | `{email_statuses:[{id,sending:{status,progress:{completed,total,estimated_seconds_remaining},failed_during?,retryable?}}]}` (`emails.test.js.snap:777-870`) | `emails_id_status_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; store+fixture aggregation; fabricated status strings `not started/failed/submitted`; body `{id,sending:{status,recipientCount,batches},emails:[…]}` — different envelope and field set | MISMATCH | G:`emails.js:55-66`, `sending-status-service.ts:22-104`, `sending-status-serializers.ts:23-40`; C++:`emails_id_status_get/.../execute.cppm:19-44` |
| PUT `/emails/:id/retry` | `retry` (docName `emails`) | `data:['id']` path; no `options`/`validation` (`emails.js:72`) | `mw.authAdminApi` (`admin/routes.js:560`); `permissions:true` (`emails.js:73`) | 200 / 400 / 404; `cacheInvalidate:false` (`emails.js:69-71`); 400 with type `IncorrectUsageError` (`emails.test.js.snap:1000-1016`) | `emailService.controller.retryFailedEmail(frame)` (`emails.js:75`) → `Email.findOne` 404 if missing (`email-controller.js:139-149`) → `service.retryEmail` (`email-service.js:397-435`): post status check, `checkCanRetryEmail` → `sendingStatusService.retryEligibilityFor` knex (`sending-status-service.ts:57-72`), `checkLimits`, `batchSendingService.updateStatusLock` (failed→pending), `scheduleEmail` job, rollback `save({status:'failed'})` on error | none (no invalidation header) | `{emails:[retried email]}` with status `pending` (`emails.test.js.snap:872-905`); side effect `email.edited` event (`emails.test.js:281-282`) | `emails_id_retry_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; `find_by_resource` 404 'Email not found or not failed.'; `ctx.store().create("email_retries",body)` stand-in; body `{emails:[…],retry:…}`; no eligibility/lock/job/event | PARTIAL | G:`emails.js:68-77`, `email-service.js:397-457`, `email-controller.js:139-149`; C++:`emails_id_retry_put/.../execute.cppm:18-36` |
| GET `/emails/:id/batches` | `browseBatches` (docName `emails`) | `data:['id']`; `options:[limit,fields,filter,order,page,include]`; validation `include ∈ {count.recipients}` (`emails.js:83-98`) | `mw.authAdminApi` (`admin/routes.js:561`); `permissions:{method:'browse'}` (`emails.js:92-94`) (author denied → 403, `emails.test.js:253-264`) | 200; `cacheInvalidate:false` (`emails.js:80-82`) | filter `email_id:'<id>'` + optional caller filter (`emails.js:96-100`) → `models.EmailBatch.findPage(restrictAdminApiQueryOptions(...))`; serializer `browseBatches` (`output/emails.js:12-24`) → `mappers/email-batches.ts:18-31` | none | `{batches:[…],meta.pagination}`; batch fields `id,mailgun_message_id,status,member_segment,created_at,updated_at,error_status_code,error_message,error_data` + optional `count.recipients` (`emails.test.js.snap:88-171`) | `batches_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; include allowlist 422; builds same `email_id:'<id>'` filter; `json_for("email_batches",params)`; fixture `BatchWire` fallback + fabricated `total=1` when empty | PARTIAL | G:`emails.js:79-102`, `output/emails.js:12-24`, `mappers/email-batches.ts:4-31`; C++:`batches_get/.../execute.cppm:19-53` |
| GET `/emails/:id/recipient-failures` | `browseFailures` (docName `emails`) | `data:['id']`; `options:[limit,fields,filter,order,page,include]`; validation `include ∈ {member,email_recipient}` (`emails.js:108-127`) | `mw.authAdminApi` (`admin/routes.js:562`); `permissions:{method:'browse'}` (`emails.js:117-119`); input serializer maps `email_recipient`→`emailRecipient` (`input/emails.ts:6-18`) | 200; `cacheInvalidate:false` (`emails.js:105-107`) | filter `email_id:'<id>'` + caller filter (`emails.js:121-125`) → `models.EmailRecipientFailure.findPage(...)`; serializer `browseFailures` (`output/emails.js:26-38`) → `mappers/email-failures.ts:28-49` | none | `{failures:[…],meta.pagination}`; fields `id,code,enhanced_code,message,severity,failed_at,event_id` + `member`/`email_recipient` or null (`emails.test.js.snap:3-73,214-377`) | `recipient_failures_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token only; include allowlist 422; same filter construction; `json_for("email_recipient_failures",params)`; fixture `FailureWire` fallback + fabricated `total=1` | PARTIAL | G:`emails.js:104-127`, `output/emails.js:26-38`, `mappers/email-failures.ts:4-49`; C++:`recipient_failures_get/.../execute.cppm:19-53` |
| GET `/emails/:id/analytics` | `analyticsStatus` (docName `emails`) | no `data`/`options`/`validation` on controller (`emails.js:129-139`); path `id` present but ignored by query | `mw.authAdminApi` (`admin/routes.js:563`); `permissions:{method:'browse'}` (`emails.js:133-135`) | 200; `cacheInvalidate:false` (`emails.js:130-132`) | `emailAnalytics.getNewsletters().service.getStatus()` (`emails.js:137`) → `EmailAnalyticsService.getStatus()` (`email-analytics-service.ts:157-173`) computing `fetchedThrough/lagSeconds` for `latest/latestOpened/missing` + `scheduled` | in-memory service state seeded by `queries.setJobMetadata` (Tinybird/DB job metadata, `email-analytics-service.ts:154,336-339`) | raw `{latest,latestOpened,missing,scheduled}` object; no resource key (`emails.test.js.snap:737-775`); serializer `analyticsStatus` passes through (`output/emails.js:40-42`) | `analytics_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` + `operations.cppm` | token only; `EmailsOperations::emails_analytics_get` → `load_analytics(cache)` → DTO `{latest,latestOpened,missing,scheduled}` with same job names; cache key `emails:analytics`, tag `emails`, TTL never; fixture default when cache empty | PARTIAL | G:`emails.js:129-139`, `email-analytics-service.ts:157-173`, `index.ts:123-152`; C++:`operations.cppm:27-30,93,101-113`, `analytics_get/.../execute.cppm:19-25` |
| PUT `/emails/:id/analytics` | `scheduleAnalytics` (docName `emails`) | `data:['id']`; `options:['begin','end']` (`emails.js:148-149`); no validation | `mw.authAdminApi` (`admin/routes.js:564`); `permissions:{method:'browse'}` (`emails.js:145-147`) | 200 (service returns void); `cacheInvalidate:false` (`emails.js:142-144`) | `Email.findOne(frame.data, findOptions)` (`emails.js:151-152`) then `begin` = param or `created_at`; `end` = param or `min(now-1h, created_at+7d)` (`emails.js:154-162`) → `service.schedule({begin,end})` (`emails.js:164`); `schedule` throws `ValidationError` if already running, writes DB job metadata via `queries.setJobMetadata` (`email-analytics-service.ts:315-340`) | DB job metadata `email-analytics-scheduled` (`email-analytics-service.ts:336-339`) | void → no documented body | `analytics_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` + `operations.cppm` | token only; `begin` from query or `created_at` parsed out of store JSON; rejects `begin>end` 422; body decoded as `AnalyticsStatusInput`; toggles `running`, caches `emails:analytics:schedule` (TTL never, tag `emails`); returns analytics status object | PARTIAL | G:`emails.js:141-166`, `email-analytics-service.ts:315-340`; C++:`analytics_put/.../execute.cppm:19-42`, `operations.cppm:32-67` |
| DELETE `/emails/analytics` | `cancelScheduledAnalytics` (docName `emails`) | no `data`/`options`/`validation` (`emails.js:168-178`) | `mw.authAdminApi` (`admin/routes.js:565`); `permissions:{method:'browse'}` (`emails.js:172-174`) | 200; `cacheInvalidate:false` (`emails.js:169-171`) | `emailAnalytics.getNewsletters().service.cancelScheduled()` (`emails.js:176`) → `EmailAnalyticsService.cancelScheduled()` sets `canceled=true` if running or `#clearScheduledData()`, clears DB metadata (`email-analytics-service.ts:347-357,150-155`) | clears `email-analytics-scheduled` job metadata in DB | void → no documented body | `analytics_delete/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` + `operations.cppm` | token only; `.delt(...)` DELETE builder (`include/utils/openapi/route.cppm:91-98`); erases `emails:analytics` and `emails:analytics:schedule`; returns `{}` | PARTIAL | G:`emails.js:168-178`, `email-analytics-service.ts:347-357`; C++:`routes.cppm:104-114`, `analytics_delete/.../execute.cppm:19-24`, `operations.cppm:69-74` |

## Route details

### GET `/emails` — PARTIAL

**Ghost**

1. Identity: `router.get('/emails', mw.authAdminApi, http(api.emails.browse))` at `admin/routes.js:557`. Router-level CORS at `admin/routes.js:17`. `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`).
2. Operation: key `browse`, op `browse`, `docName:'emails'` (`emails.js:18,20`).
3. Input: `options:['limit','fields','filter','order','page']` (`emails.js:24`); no `data`, no `validation{}`; no multipart; no headers read by controller. Query aliases come from api-framework (`filter`, `filter[x]`, `order`, `page`, `limit`, `fields`).
4. Auth: `mw.authAdminApi` (`admin/routes.js:557`) + `permissions:true` (`emails.js:25`) → framework derives `{docName:'emails', method:'browse'}`. `restrictAdminApiQueryOptions` adds a `mongoTransformer` rejecting restricted fields `password/token/gift_link_token/secret` in `filter`/`order` and throws `BadRequestError('Restricted fields cannot be used in order.')` (`api-filter-utils.ts:78,218-220,247-289`).
5. Status/headers: 200; `headers.cacheInvalidate:false` (`emails.js:21-23`) ⇒ no `X-Cache-Invalidate`; etag + content-version present (`emails.test.js:77-80`, `emails.test.js.snap:561-572`).
6. Full call chain: `query(frame)` → `models.Email.findPage(restrictAdminApiQueryOptions(frame.options))` (`emails.js:27`) — Bookshelf `findPage` returns `{data, meta}`; no nested service calls, no events/jobs/mail/webhooks. Error branch: 400 `BadRequestError` from the order guard; otherwise no explicit 404 (empty list on no matches).
7. Cache: none — `cacheInvalidate:false`, no cache service/key/TTL in controller or model path.
8. Output: default api-framework serializer for `browse` → `{emails:[…],meta.pagination}`; each email transformed by `mappers/emails.js:4-36` (HTML/plaintext `%%{}%%` replacements via `emailService.renderer.buildReplacementDefinitions` + `getDefaultExampleMember`, delete `post`, `stripEmailAccounting` deleting `preflight_email_count/candidate_count/preparation_excluded_count/prepared_at` — `strip-email-accounting.ts:1-13`); 2 fixture emails with full field set (`emails.test.js.snap:488-559`), pagination `{limit:15,page:1,pages:1,prev:null,next:null,total:2}` (`:548-557`). Empty → `{emails:[],meta...}`.

**C++**

1. Identity/route: `Router{router,"/emails"}.use(content_type_middleware).use(require_auth).get(...)` at `membership/emails/routes.cppm:19-28`; handler `emails_browse()` (`:27`), installed via `EmailsRoutes{}.install` (`include/blog/handlers/routes.cppm:91`). `content_type_middleware` rejects wrong Content-Type/Accept (`middleware/serde.cppm:90-126`); `require_auth` checks bearer presence (`middleware/auth.cppm:12-29`, `middleware/bearer.cppm:27-33`).
2. Files/symbols: `browse/handler.cppm:16-33` (`BrowseHandler`), `browse/workflow/flow/main.cppm:7-10` (`MainFlow`, single `ExecuteStep`), `browse/workflow/step/execute.cppm:14-79` (`ExecuteStep::invoke`), `operations.cppm` not used.
3. Input/parsing: `options(in.get_path(), {"limit","fields","filter","order","page"})` (`execute.cppm:22,51-69`) parses raw query string; `size_option` defaults `limit=15,page=1` (`:25-26,71-76`). No `filter[x]=y` nesting support, no mongoTransformer semantics, no header reads.
4. Status/body/headers: handler always sets 200 and writes body (`browse/handler.cppm:26-30`); no `X-Cache-Invalidate` header emitted (consistent with Ghost here).
5. store()/cache calls: `ctx.store().json_for("emails", params)` and `ctx.store().total("emails")` (`execute.cppm:23-24`); computes `pages`, `prev`, `next` locally (`:27-29`). No `core::cache::Cache` use, no mutations, no side effects.
6. Middleware chain: router-level `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper, no labs gate.
7. Side effects: none.
8. NOT implemented: `restrictAdminApiQueryOptions` restricted-field/order guard (`api-filter-utils.ts:275-289`); real `Email.findPage`/Booksshelf `findPage` relations; `mappers/emails.js` replacements + `stripEmailAccounting`; api-framework etag/content-version. Pagination numbers are computed over the in-memory resource store, not a real DB/page query.

**Diff**

- Residual gaps (Ghost primitives absent in C++): Admin API restricted-field transformer + `BadRequestError` on restricted order (`api-filter-utils.ts:78,218-220,247-289`); real `models.Email.findPage`; serializer `mappers/emails.js:4-36` + `strip-email-accounting.ts:1-13`; etag/content-version (`emails.test.js:77-80`).
- Divergences: token-presence auth only vs authenticate+authorize+permission (`handler.cppm` vs `admin/middleware.js:110-118`); raw query-string parsing vs api-framework option/alias normalization (`execute.cppm:51-69`); no serializer field stripping — internal accounting fields from fixtures can leak.

### GET `/emails/:id` — PARTIAL

**Ghost**

1. Identity: `router.get('/emails/:id', mw.authAdminApi, http(api.emails.read))` (`admin/routes.js:558`).
2. Operation: key `read`, `docName:'emails'` (`emails.js:18,31`).
3. Input: `data:['id']` from path; `options:['fields']` (`emails.js:35`); `validation.options.fields` limited to `{html,plaintext,subject}` (`emails.js:36-40`) — api-framework returns 422 on other values. No body/multipart/headers.
4. Auth: `mw.authAdminApi` + `permissions:true` (`emails.js:42`) → `{docName:'emails', method:'read'}`.
5. Status/headers: 200 on hit, 404 on miss; `cacheInvalidate:false` (`emails.js:32-34`); etag/content-version (`emails.test.js:83-93`).
6. Full call chain: `models.Email.findOne(frame.data, frame.options)` (`emails.js:44`); `if (!model) throw NotFoundError(tpl(messages.emailNotFound))` with message `'Email not found.'` (`emails.js:8-9,45-49`; snapshot error shape `emails.test.js.snap:920-949`). Serializer uses `mappers/emails.js` (replacements + `stripEmailAccounting`, remove `post` relation).
7. Cache: none.
8. Output: `{emails:[one email]}` (`emails.test.js.snap:689-722`); full field set including `html`, `plaintext`, `subject`, `status`, counts; `%%{...}%%` placeholders replaced with example member/unsubscribe URL (`emails.test.js:400-441`, `emails.test.js.snap:951-998`). Missing → 404 `NotFoundError`, not an empty array.

**C++**

1. Identity/route: nested `Router{router,"/:id"}.get(...)` under `/emails` (`routes.cppm:31-38`); handler `emails_read_by_id()` (`:37`).
2. Files/symbols: `read_by_id/handler.cppm:16-34`, `read_by_id/workflow/flow/main.cppm:7-10`, `read_by_id/workflow/step/execute.cppm:14-93`.
3. Input/parsing: `id_from_path` splits path segments and takes `segments[1]` (`execute.cppm:48-57`); `fields` parsed from query and each comma token checked against `{"html","plaintext","subject"}`, invalid → 422 `{"errors":[...]}` (`execute.cppm:24-33,86-90`). No body read.
4. Status/body/headers: handler casts payload `status`/`body` (`read_by_id/handler.cppm:27-31`); 200 on hit, 404 on miss; no `X-Cache-Invalidate`.
5. store()/cache calls: `ctx.store().find_by_resource("emails", id)` (`execute.cppm:35`); 404 body `ApiError{}.json("Email not found.","Not Found")` (`:36-37`). No mutations, no cache.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: none.
8. NOT implemented: real `Email.findOne` with `fields` projection and `mappers/emails.js` replacements/accounting-strip; etag/content-version. Divergence: body is `{"data":<model>,"emails":[<model>]}` (`execute.cppm:41`) — Ghost has no top-level `data` member for a single read (`emails.test.js.snap:689-722`).

**Diff**

- Residual gaps: permission service; real model lookup + field projection; replacement/serializer transformation; etag/content-version.
- Divergences: extra `data` key in response (`execute.cppm:41`); 422 validation error text/type differs from Ghost API-framework messages; token-only auth.

### GET `/emails/:id/status` — MISMATCH (pre-implementation; current PARTIAL)

**Ghost**

1. Identity: `router.get('/emails/:id/status', mw.authAdminApi, http(api.emails.sendingStatus))` (`admin/routes.js:559`).
2. Operation: key `sendingStatus`, `docName:'emails'` (`emails.js:18,55`).
3. Input: `data:['id']` path (`emails.js:59`); no options/validation.
4. Auth: `mw.authAdminApi` + `permissions:{method:'read'}` (`emails.js:60-62`) — note a reader allowed `emails:read` may access status but not batches (403 test at `emails.test.js:253-264`).
5. Status/headers: 200 on hit, 404 `NotFoundError('Email not found.')` when `statusFor` returns null (`email-controller.js:151-161`; `emails.test.js:178-189`); `cacheInvalidate:false` (`emails.js:56-58`); etag/content-version.
6. Full call chain: `emailService.controller.getSendingStatus(frame)` (`emails.js:64`) → `sendingStatusService.statusFor(frame.data.id)` (`email-controller.js:152`) → knex `emails` row `select id,status,email_count,preflight_email_count,updated_at` (`sending-status-service.ts:23-26`) → `#batchesFor` knex aggregation with `accounted_recipient_count` CASE or correlated `email_recipients` count (`sending-status-service.ts:74-104`) → `buildSendingStatus` deriving phase `preparing/submitting`, `progress.completed/total/estimatedSecondsRemaining`, `failedDuring`, `retryable` via `isRetryable` (`sending-status.ts:52-117,138-193`) → serializer `toEmailStatusesResponse` producing `email_statuses` with snake_cased progress (`sending-status-serializers.ts:23-40`). Side effects: none (read-only knex).
7. Cache: none.
8. Output: `{email_statuses:[{id,sending:{status,progress:{completed,total,estimated_seconds_remaining},failed_during?,retryable?}}]}` — snapshots: submitted `{status:'submitted',progress:{completed:6,total:6,estimated_seconds_remaining:0}}` (`emails.test.js.snap:810-839`); submitting `{status:'submitting',progress:{completed:6,total:8,estimated_seconds_remaining:null}}` (`:841-870`); failed `{status:'failed',failed_during:'preparing',retryable:true,progress:{completed:0,total:3,estimated_seconds_remaining:null}}` (`:777-808`). No `emails` key.

**C++**

1. Identity/route: `Route{"/status"}.get(...)` nested under `/:id` (`routes.cppm:92-102`); handler `emails_emails_id_status_get()` (`:99`).
2. Files/symbols: `emails_id_status_get/handler.cppm:16-34`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-68`.
3. Input/parsing: `id_from_path` `segments[1]` (`execute.cppm:50-59`); no other input.
4. Status/body/headers: 200/404 via payload map (`email_id_status_get/handler.cppm:27-31`); 404 body `'Email not found.'` (`execute.cppm:26-27`); no invalidate header.
5. store()/cache calls: `find_by_resource("emails",id)` (`execute.cppm:25`); `json_for("email_batches",{{"email_id",id}})` and `json_for("email_recipients",{{"email_id",id}})` (`:32-33`); empty batches replaced with hardcoded `BatchWire{}` (`:34-35`, fixture at `types/model_wire_fixtures.cppm:82-94`); `count_rows` counts commas in JSON string (`:61-65`); status chosen by substring check `"\"status\":\"failed\""` (`:38-39`). No `core::cache::Cache`, no mutations.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper (`emails:read` vs browse distinction absent).
7. Side effects: none.
8. NOT implemented: knex row/aggregation, `buildSendingStatus`, `toEmailStatusesResponse`, ETA estimation, `failed_during`/`retryable`, `progress` object. Response contract is `{id,sending:{status,recipientCount,batches},emails:[…]}` (`execute.cppm:41-43`) with status vocabulary `not started/failed/submitted` — none of which match Ghost's `email_statuses` envelope and `preparing/submitting/submitted/failed` vocabulary.

**Diff**

- Residual gaps: entire `SendingStatusService`/`buildSendingStatus`/serializer stack (`sending-status-service.ts:22-104`, `sending-status.ts:52-193`, `sending-status-serializers.ts:23-40`); permission `{method:'read'}` (`emails.js:60-62`); etag/content-version.
- Divergences: different top-level key (`sending`+`emails` vs `email_statuses`), different field names (`recipientCount`/`batches` vs `progress`), fabricated `not started` status, fixture `BatchWire` fallback, comma-count recipient count, distinct read-vs-browse permission not enforced. Core response contract differs → MISMATCH.

### PUT `/emails/:id/retry` — PARTIAL

**Ghost**

1. Identity: `router.put('/emails/:id/retry', mw.authAdminApi, http(api.emails.retry))` (`admin/routes.js:560`).
2. Operation: key `retry`, `docName:'emails'` (`emails.js:18,68`).
3. Input: `data:['id']` path; no options/validation; request body ignored by controller.
4. Auth: `mw.authAdminApi` + `permissions:true` (`emails.js:73`) → `{docName:'emails', method:'retry'}`.
5. Status/headers: 200 on success (`emails.test.js:269-279`); 404 `Email not found.` missing email (`email-controller.js:140-146`); 400 with type `IncorrectUsageError` for non-failed/unknown-outcome/other-status (`email-service.js:401-424,442-456`; `emails.test.js.snap:1000-1029`, `emails.test.js:191-220`); `cacheInvalidate:false` (`emails.js:69-71`).
6. Full call chain: `emailService.controller.retryFailedEmail(frame)` (`emails.js:75`) → `models.Email.findOne(frame.data,{require:false})` → `NotFoundError` if missing (`email-controller.js:139-149`) → `service.retryEmail(email)` (`email-service.js:397-435`): `_getLazyRelation('post')` status must be `published`/`sent` else `IncorrectUsageError('Can only retry emails for published posts')`; `checkCanRetryEmail` → `sendingStatusService.retryEligibilityFor` knex `email_batches` distinct statuses → `not-failed`/`unknown-outcome` 400 (`sending-status-service.ts:57-72`); `checkLimits()`; `batchSendingService.updateStatusLock(Email, id,'pending',['failed'],{autoRefresh:true})` (transactional claim, 400 `Only failed emails can be retried` if null) → `scheduleEmail(pendingEmail)` job → on throw `pendingEmail.save({status:'failed'},{patch:true})` rollback; returns pending email. Side effect: `email.edited` domain event asserted by test (`emails.test.js:281-282`).
7. Cache: none.
8. Output: `{emails:[claimed email]}` with `status:'pending'` (`emails.test.js.snap:872-905`); etag/content-version (`:907-918`).

**C++**

1. Identity/route: `Route{"/retry"}.put(...)` under `/:id` (`routes.cppm:81-91`); handler `emails_emails_id_retry_put()` (`:88`).
2. Files/symbols: `emails_id_retry_put/handler.cppm:16-34`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:14-54`.
3. Input/parsing: `id_from_path` (`:42-51`); body read raw `in.body_as_string()` defaulting to `{}` (`:23`) but not decoded/validated.
4. Status/body/headers: status from `result.status()` (`:32-36`), 404 `'Email not found or not failed.'` (`:26-27`); no invalidate header.
5. store()/cache calls: `find_by_resource("emails",id)` (`:25`); on hit `ctx.store().create("email_retries", body)` (`:31`) as stand-in for the retry claim; output `{"emails":["+*email+"],"retry":<result>}` (`:35`). No `core::cache::Cache`.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: `email_retries` resource write (stand-in for status lock + job schedule). No `email.edited` event, no job, no SMTP.
8. NOT implemented: post-status check, `retryEligibilityFor`, `checkLimits`, `updateStatusLock` transaction (failed→pending), `scheduleEmail`, rollback save, `email.edited` event, the 400 error taxonomy. The email's own `status` is not mutated to `pending`; the extra `retry` response key is not in Ghost's output.

**Diff**

- Residual gaps: knex transactional claim + job scheduling + domain event + eligibility/limit checks (`email-service.js:397-457`, `sending-status-service.ts:57-72`); permission service.
- Divergences: fabricated `email_retries` record instead of real retry; email status not changed; extra `retry` key in body (`execute.cppm:35`); error message "Email not found or not failed." conflates 404 and 400 branches.

### GET `/emails/:id/batches` — PARTIAL

**Ghost**

1. Identity: `router.get('/emails/:id/batches', mw.authAdminApi, http(api.emails.browseBatches))` (`admin/routes.js:561`).
2. Operation: key `browseBatches`, `docName:'emails'` (`emails.js:18,79`).
3. Input: `data:['id']`; `options:['limit','fields','filter','order','page','include']` (`emails.js:83`); `validation.options.include.values = ['count.recipients']` (`emails.js:13,85-91`). Query aliases incl. `filter[x]=y` via api-framework.
4. Auth: `mw.authAdminApi` + `permissions:{method:'browse'}` (`emails.js:92-94`) — GET status readers without `browse` get 403 (`emails.test.js:253-264`).
5. Status/headers: 200; `cacheInvalidate:false` (`emails.js:80-82`); etag/content-version (`emails.test.js:298-309`).
6. Full call chain: filter `email_id:'<id>'` + `+(caller filter)` (`emails.js:96-97`) → `models.EmailBatch.findPage(restrictAdminApiQueryOptions({...frame.options, filter}))` (`emails.js:98-100`) — restricted-field guard applies (`api-filter-utils.ts:275-289`); serializer `browseBatches` (`output/emails.js:12-24`) → `mappers/email-batches.ts:18-31` picks batch fields and optional `count.recipients`. No events/jobs.
7. Cache: none.
8. Output: `{batches:[…],meta.pagination}` (`emails.test.js.snap:88-127`); with `include=count.recipients` a `count:{recipients:6}` field appears (`:129-171`); empty → `{batches:[],meta...}`.

**C++**

1. Identity/route: `Route{"/batches"}.get(...)` under `/:id` (`routes.cppm:59-69`); handler `emails_batches_get()` (`:66`).
2. Files/symbols: `batches_get/handler.cppm:16-34`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-111`.
3. Input/parsing: `id_from_path` (`:59-68`); `options(...include)` with `kIncludes={"count.recipients"}` and 422 `Validation error` for other includes (`:24-32,104-108`); filter constructed exactly as `email_id:'<id>'` + `+(user filter)` (`:33-35`); `limit/page` default 15/1 (`:43-44,97-102`).
4. Status/body/headers: 200 always (`:53`); no invalidate header.
5. store()/cache calls: `json_for("email_batches",params)` + `total("email_batches")` (`:37-38`); if empty, substitutes hardcoded `BatchWire{}` and forces `total=1` (`:39-42`, fixture `types/model_wire_fixtures.cppm:82-94`); pagination computed locally (`:43-47`). No cache, no mutations.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: none.
8. NOT implemented: real `EmailBatch.findPage`, restricted-field guard, serializer mapper/etag. Fixture fallback + fabricated `total` is a divergence: an empty Ghost result returns `{batches:[],...total:0}` but C++ returns one synthetic batch.

**Diff**

- Residual gaps: `restrictAdminApiQueryOptions` (`api-filter-utils.ts:275-289`), real model `findPage`, `mappers/email-batches.ts`, etag/content-version.
- Divergences: fixture `BatchWire` fallback and `total=1` fabrication (`execute.cppm:39-42`); token-only auth with no browse-vs-read permission split.

### GET `/emails/:id/recipient-failures` — PARTIAL

**Ghost**

1. Identity: `router.get('/emails/:id/recipient-failures', mw.authAdminApi, http(api.emails.browseFailures))` (`admin/routes.js:562`).
2. Operation: key `browseFailures`, `docName:'emails'` (`emails.js:18,104`).
3. Input: `data:['id']`; `options:['limit','fields','filter','order','page','include']` (`emails.js:108`); `validation.options.include.values = ['member','email_recipient']` (`emails.js:14,110-116`); input serializer maps snake_case `email_recipient` → camelCase `emailRecipient` in `withRelated` (`input/emails.ts:6-18`).
4. Auth: `mw.authAdminApi` + `permissions:{method:'browse'}` (`emails.js:117-119`).
5. Status/headers: 200; `cacheInvalidate:false` (`emails.js:105-107`); etag/content-version (`emails.test.js:325-338`).
6. Full call chain: filter `email_id:'<id>'` + caller filter (`emails.js:121-122`) → `models.EmailRecipientFailure.findPage(restrictAdminApiQueryOptions({...frame.options, filter}))` (`emails.js:123-125`); serializer `browseFailures` (`output/emails.js:26-38`) → `mappers/email-failures.ts:28-49` (picks `failureFields`, maps `member` → `memberFields` or null, `emailRecipient` → `email_recipient` fields or null). Filter examples `severity:permanent`/`severity:temporary`, `order=failed_at DESC` (`emails.test.js:340-368`).
7. Cache: none.
8. Output: `{failures:[…],meta.pagination}`; fields `id,code,enhanced_code,message,severity,failed_at,event_id` + `member`/`email_recipient` or null (`emails.test.js.snap:3-73,214-377`); includes add nested objects (`:214-364`). Empty → `{failures:[],meta...}`.

**C++**

1. Identity/route: `Route{"/recipient-failures"}.get(...)` under `/:id` (`routes.cppm:70-80`); handler `emails_recipient_failures_get()` (`:77`).
2. Files/symbols: `recipient_failures_get/handler.cppm:16-34`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-111`.
3. Input/parsing: `id_from_path` (`:59-68`); `kIncludes={"member","email_recipient"}` with 422 on other values (`:24-32,104-108`); same `email_id:'<id>'`+caller-filter construction (`:33-35`); limit/page 15/1 (`:43-44,97-102`). No input serializer snake→camel mapping.
4. Status/body/headers: 200 always (`:53`); no invalidate header.
5. store()/cache calls: `json_for("email_recipient_failures",params)` + `total(...)` (`:37-38`); fixture `FailureWire{}` substituted when empty with forced `total=1` (`:39-42`, fixture at `types/model_wire_fixtures.cppm:97-111`); pagination local (`:43-47`). No cache/mutations.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: none.
8. NOT implemented: real `EmailRecipientFailure.findPage`, restricted-field guard, `mappers/email-failures.ts` field picking + null relation handling, input serializer, etag/content-version. Relations (`member`, `email_recipient`) are not loaded — fixture has them as `null`.

**Diff**

- Residual gaps: same as batches plus the `email_recipient`→`emailRecipient` input serializer (`input/emails.ts:6-18`).
- Divergences: `FailureWire` fixture fallback and `total=1` fabrication (`execute.cppm:39-42`); no relation loading even when `include=member,email_recipient` is accepted; token-only auth.

### GET `/emails/:id/analytics` — PARTIAL

**Ghost**

1. Identity: `router.get('/emails/:id/analytics', mw.authAdminApi, http(api.emails.analyticsStatus))` (`admin/routes.js:563`). Path `id` is present but unused by the controller query.
2. Operation: key `analyticsStatus`, `docName:'emails'` (`emails.js:18,129`).
3. Input: none declared (no `data`, `options`, `validation` at `emails.js:129-139`).
4. Auth: `mw.authAdminApi` + `permissions:{method:'browse'}` (`emails.js:133-135`).
5. Status/headers: 200; `cacheInvalidate:false` (`emails.js:130-132`); etag/content-version (`emails.test.js:285-296`).
6. Full call chain: `emailAnalytics.getNewsletters().service.getStatus()` (`emails.js:137`) where `getNewsletters()` asserts initialized newsletter wrapper (`email-analytics/index.ts:37-40`, configured at `:123-152` with job names `email-analytics-latest-others/missing/latest-opened/scheduled`). `EmailAnalyticsService.getStatus()` (`email-analytics-service.ts:157-173`) returns `{latest,missing,scheduled,latestOpened}` with `fetchedThrough` (null when unset) and computed `lagSeconds = max(0, floor((now-fetchedThrough)/1000))` for the three lag-bearing pipelines. No DB/network call in `getStatus` itself; state seeded by fetch jobs/`setJobMetadata`.
7. Cache: no api cache; state is in-memory on the service, persisted via `queries.setJobMetadata` in job paths (`email-analytics-service.ts:154,336-339`).
8. Output: raw object with no resource key (`output/emails.js:40-42` passes through): `{latest:{fetchedThrough:null,jobName:'email-analytics-latest-others',lagSeconds:null,running:false},latestOpened:{...},missing:{...},scheduled:{jobName:'email-analytics-scheduled',running:false}}` (`emails.test.js.snap:737-762`). Missing/empty → initial-state object, never 404.

**C++**

1. Identity/route: `Route{"/analytics"}.get(...)` under `/:id` (`routes.cppm:40-49`); handler `emails_analytics_get()` (`:47`).
2. Files/symbols: `analytics_get/handler.cppm:17-35`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-32`, shared `operations.cppm:19-30,101-121`.
3. Input/parsing: none — step ignores request (`execute.cppm:20`).
4. Status/body/headers: status 200 from `r.first` (`execute.cppm:24-25`); handler writes payload map (`:28-32`); no invalidate header.
5. store()/cache calls: `EmailsOperations::emails_analytics_get()` → `load_analytics(m_cache)` reads key `emails:analytics` via `cache.get` (`operations.cppm:27-30,101-113`); on decode failure returns fixture `AnalyticsStatus` with job names identical to Ghost (`latest:'email-analytics-latest-others'`, `latestOpened:'email-analytics-latest-opened'`, `missing:'email-analytics-missing'`, `scheduled:'email-analytics-scheduled'`, `:107-112`). DTO field set `fetchedThrough/jobName/lagSeconds/running` (`types/dto_analytics_job.cppm:12-44`), `scheduled` has only `jobName/running` (`dto_scheduled_analytics_job.cppm:11-41`). Cache TTL never, tag `{"emails"}` on write (`operations.cppm:115-121`). No `ctx.store()` access.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: none on GET (read-only cache get).
8. NOT implemented: real `EmailAnalyticsService.getStatus()` lag computation against live fetch state; Tinybird/Mailgun fetch pipeline; etag/content-version. Divergence: state is a cache DTO rather than service in-memory state; job names match but `fetchedThrough/lagSeconds` are DTO `rfl::Generic` values rather than computed from real fetch timestamps.

**Diff**

- Residual gaps: `getNewsletters()` wrapper/init (`email-analytics/index.ts:37-40,123-152`), live `getStatus()` lag computation (`email-analytics-service.ts:157-173`), etag/content-version.
- Divergences: cache-backed stand-in with TTL never/tag `emails` (`operations.cppm:93,115-121`) instead of service state + DB job metadata; no real Tinybird fetch; token-only auth.

### PUT `/emails/:id/analytics` — PARTIAL

**Ghost**

1. Identity: `router.put('/emails/:id/analytics', mw.authAdminApi, http(api.emails.scheduleAnalytics))` (`admin/routes.js:564`).
2. Operation: key `scheduleAnalytics`, `docName:'emails'` (`emails.js:18,141`).
3. Input: `data:['id']` path; `options:['begin','end']` (`emails.js:148-149`); no `validation{}` — dates parsed with `new Date(...)`, no explicit 422 in controller.
4. Auth: `mw.authAdminApi` + `permissions:{method:'browse'}` (`emails.js:145-147`).
5. Status/headers: 200 (void return); `cacheInvalidate:false` (`emails.js:142-144`).
6. Full call chain: destructure `begin/end` from options then `models.Email.findOne(frame.data, findOptions)` with remaining options (`emails.js:151-152`) — no explicit 404 if missing (would throw on `model.get`); `begin = beginParam ? new Date(beginParam) : model.get('created_at')`; `end = endParam ? new Date(endParam) : new Date(min(Date.now()-3600000, created_at.getTime()+7*86400000))` (`emails.js:154-162`) → `emailAnalytics.getNewsletters().service.schedule({begin,end})` (`emails.js:164`) → `EmailAnalyticsService.schedule` (`email-analytics-service.ts:315-340`): throws `ValidationError('Already fetching scheduled events…')` if `#fetchScheduledData.running`, sets in-memory `{running:false, jobName:'email-analytics-scheduled', schedule:{begin,end}}`, then `await queries.setJobMetadata('email-analytics-scheduled',{begin:ISO,end:ISO})` — a DB write consumed by the Tinybird fetch job.
7. Cache: writes DB job metadata; no api cache key.
8. Output: void (`Promise<void>`), no documented response body.

**C++**

1. Identity/route: `Route{"/analytics"}.put(...)` under `/:id` (`routes.cppm:50-57`); handler `emails_analytics_put()` (`:55`).
2. Files/symbols: `analytics_put/handler.cppm:17-35`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-82`, `operations.cppm:32-67`.
3. Input/parsing: `id_from_path` (`:48-57`); `options(path,{"begin","end"})` (`:23,59-72`); `begin` falls back to `created_at` extracted by string search of `"created_at":"` in `json_for("emails",{{"id",id}})` (`:27-37`); `begin>end` → 422 `Invalid analytics window` (`:38-39`); body decoded as `AnalyticsStatusInput` (`operations.cppm:42`, DTO at `types/dto_analytics_status_input.cppm:11-53`) plus raw `JsonBody`/`jobName`/`running` fallback (`:39-40,62-64`).
4. Status/body/headers: 200 from `r.first`; handler writes payload (`:29-32`); no invalidate header.
5. store()/cache calls: `ctx.store().json_for("emails",{{"id",id}})` (`:27`); `EmailsOperations::emails_analytics_put` loads `emails:analytics`, looks up job via `job_for` (map `latest-opened`→latestOpened, `missing`→missing, else latest, `operations.cppm:123-131`), sets `job->running = input->running || wire_running` (`:44-46`), writes key `emails:analytics:schedule` with `{id,begin,end}` (TTL never, tag `emails`, `:50-60`), stores analytics back to `emails:analytics` (`:65,115-121`). No DB job metadata write.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: two cache writes (schedule + analytics) as stand-ins for `queries.setJobMetadata`. No Tinybird/Mailgun fetch, no "already running" `ValidationError` branch.
8. NOT implemented: real `Email.findOne` with remaining options, live `EmailAnalyticsService.schedule` running guard, DB job metadata, Tinybird fetch pipeline, void/empty response (C++ returns a full analytics status object). Divergence: C++ `end` has no `min(now-1h, created_at+7d)` default (it stays empty when omitted) (`execute.cppm:29` vs `emails.js:155-162`).

**Diff**

- Residual gaps: permission service; `Email.findOne` + date defaulting (`emails.js:151-164`); service `schedule` guard + DB metadata + Tinybird fetch (`email-analytics-service.ts:315-340`).
- Divergences: cache keys `emails:analytics`/`emails:analytics:schedule` with TTL never/tag `emails` replace DB metadata (`operations.cppm:53-60,115-121`); returns analytics status body where Ghost returns void; no 7-day/`now-1h` `end` default; added 422 `begin>end` branch absent in Ghost controller.

### DELETE `/emails/analytics` — PARTIAL

**Ghost**

1. Identity: `router.delete('/emails/analytics', mw.authAdminApi, http(api.emails.cancelScheduledAnalytics))` (`admin/routes.js:565`).
2. Operation: key `cancelScheduledAnalytics`, `docName:'emails'` (`emails.js:18,168`).
3. Input: none (`emails.js:168-178`); no path id used.
4. Auth: `mw.authAdminApi` + `permissions:{method:'browse'}` (`emails.js:172-174`).
5. Status/headers: 200 (void return); `cacheInvalidate:false` (`emails.js:169-171`).
6. Full call chain: `emailAnalytics.getNewsletters().service.cancelScheduled()` (`emails.js:176`) → `EmailAnalyticsService.cancelScheduled()` (`email-analytics-service.ts:347-357`): if `#fetchScheduledData.running` sets `canceled=true` and eagerly `queries.setJobMetadata('email-analytics-scheduled', null)`; else `#clearScheduledData()` which also nulls metadata (`:150-155`). No other side effects.
7. Cache: clears DB job metadata for `email-analytics-scheduled`.
8. Output: void, no documented body; no 404 branch.

**C++**

1. Identity/route: `Route{"/analytics"}.delt(...)` under `/emails` (`routes.cppm:104-114`); `.delt` is the DELETE builder on `ApiRoute` (`include/utils/openapi/route.cppm:91-98`); handler `emails_analytics_delete()` (`:111`).
2. Files/symbols: `analytics_delete/handler.cppm:17-35`, `.../flow/main.cppm:7-10`, `.../step/execute.cppm:15-31`, `operations.cppm:69-74`.
3. Input/parsing: none — step ignores request (`execute.cppm:20`).
4. Status/body/headers: 200 and body `"{}"` (`operations.cppm:73`); handler writes payload (`:28-32`); no invalidate header.
5. store()/cache calls: `m_cache.erase("emails:analytics")` and `m_cache.erase("emails:analytics:schedule")` (`operations.cppm:71-72`; keys at `:93-94`). No `ctx.store()`, no DB write, no `bulk_destroy("emails", …)`.
6. Middleware chain: `content_type_middleware` + `require_auth` (`routes.cppm:20-21`); no permission helper.
7. Side effects: two cache erasures as stand-in for clearing job metadata.
8. NOT implemented: `cancelScheduled()` running/canceled semantics, DB job metadata nulling, Tinybird fetch cancellation, permission check, etag/content-version (Ghost DELETE with void body). Divergence: C++ returns `{}` explicitly where Ghost returns void.

**Diff**

- Residual gaps: `cancelScheduled()` + `queries.setJobMetadata(...,null)` (`email-analytics-service.ts:347-357,150-155`); Tinybird job cancellation; permission service.
- Divergences: cache erasure of two context-local keys stands in for DB metadata (`operations.cppm:71-72`); explicit `{}` body vs void.

## Module-level notes

- All nine Ghost endpoints declare `headers.cacheInvalidate:false` (`emails.js:22,33,57,70,81,106,131,143,170`) ⇒ none emits `X-Cache-Invalidate`; none of the nine C++ handlers in this folder emits that header either — consistent across the module. (Other C++ modules such as `comments` do emit `X-Cache-Invalidate:false`; this folder does not.)
- Shared C++ `EmailsOperations` (`operations.cppm:19-134`) backs only the three analytics routes: `emails_analytics_get/put/delete` (`:27-74`). Keys `emails:analytics` and `emails:analytics:schedule` (`:93-94`), TTL `Ttl{}.never()`, tag `{"emails"}` (`:59,120`). This is a stand-in for Ghost's `queries.setJobMetadata` DB persistence (`email-analytics-service.ts:336-339,352`) and in-memory fetch state (`email-analytics-service.ts:157-173`).
- `batches_get` and `recipient_failures_get` handlers receive `EmailsOperations&` but never use it (`batches_get/handler.cppm:18`, `recipient_failures_get/handler.cppm:18`); their steps instead fabricate a single `BatchWire`/`FailureWire` fixture row and force `total=1` when the store is empty (`batches_get/.../execute.cppm:39-42`, `recipient_failures_get/.../execute.cppm:39-42`).
- All C++ handlers share process-global `static BlogContext* s_ctx` (and `static EmailsOperations* s_ops` for analytics steps) assigned at handler construction (`browse/handler.cppm:19`, `analytics_get/handler.cppm:19-20`, `analytics_put/.../execute.cppm:80-81`, etc.) — no per-request injection.
- Every C++ route in this folder inherits router-level `content_type_middleware` + `require_auth` (`routes.cppm:20-21`). There is no C++ primitive for the api-framework permission service, so Ghost's distinction between `{method:'browse'}` (batches, failures, analytics, schedule, cancel) and `{method:'read'}` (status) and `permissions:true` (browse, read, retry) is not enforced (`emails.js:25,42,60-62,73,92-94,117-119,133-135,145-147,172-174`).
- Ghost side effects with no C++ primitive in this module: knex transactional status lock + `scheduleEmail` job + `email.edited` event on retry (`email-service.js:397-435`, `emails.test.js:281-282`); Tinybird/Mailgun analytics fetch pipeline and DB job metadata (`email-analytics-service.ts:315-357`, `fetch-mailgun-events.ts`); `stripEmailAccounting` internal-column removal and HTML/plaintext `%%{}%%` replacement (`mappers/emails.js:4-36`); api-framework etag/content-version on every response (`emails.test.js.snap` header snapshots).
- Error-status mapping differs: Ghost `NotFoundError`/`BadRequestError`/`ValidationError` serialize to `{"errors":[{code,context,details,ghostErrorCode,help,id,message,property,type}]}` (`emails.test.js.snap:920-936,1000-1016`); C++ uses `ApiError::json`/`ApiError::validation` with 404/422 and its own context strings (`read_by_id/.../execute.cppm:36-37,86-90`, `batches_get/.../execute.cppm:104-108`).
- No `EXTRA` routes: the nine C++ handlers map one-to-one to the nine Ghost routes; no `add`/`edit`/`destroy` email controller actions exist in Ghost (`emails.js` declares only the nine keys at `:20-178`).

Strict-pass additions to the module notes: the nine handlers now call
`blog::api::emails::permitted` before `workflow.run`, so the Ghost method split
(`read`/`browse`/`retry`) is enforced through `core_auth:ghost`; retry and the
two analytics mutations emit observable events through `blog_handlers:outbox_shim`
(`email.edited`, `email-analytics.scheduled`, `email-analytics.cancelled`); the
`BatchWire`/`FailureWire` fixture fallbacks and `total=1` fabrication are gone
(empty store ⇒ empty collection, `total:0`). `s_ctx`/`s_ops` and the
`std::string`/`std::map` `Output` shapes are unchanged.
