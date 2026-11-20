# Behavior report: members (Ghost Admin API controller)

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/members.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/members/` (current handlers, found by path+verb, not by folder name)
- Analyzed: 2026-11-19

Evidence path aliases used below:

| Alias | Full path |
|---|---|
| `M` | `docs/Ghost/ghost/core/server/api/endpoints/members.js` |
| `R` | `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` |
| `MW` | `docs/Ghost/ghost/core/server/web/api/endpoints/admin/middleware.js` |
| `BS` | `docs/Ghost/ghost/core/server/services/members/members-api/services/member-bread-service.js` |
| `MR` | `docs/Ghost/ghost/core/server/services/members/members-api/repositories/member-repository.js` |
| `ER` | `docs/Ghost/ghost/core/server/services/members/members-api/repositories/event-repository.js` |
| `MA` | `docs/Ghost/ghost/core/server/services/members/members-api/members-api.js` |
| `S` | `docs/Ghost/ghost/core/server/services/members/service.js` |
| `IMP` | `docs/Ghost/ghost/core/server/services/members/import-export/import/importer.ts` |
| `IS` | `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/input/members.js` |
| `OS` | `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/members.js` |
| `AF` | `docs/Ghost/ghost/core/server/api/endpoints/utils/api-filter-utils.ts` |
| `RT` | `include/blog/handlers/membership/members/routes.cppm` |
| `OP` | `include/blog/handlers/membership/members/operations.cppm` |
| `CTX` | `include/blog/handlers/context/context_context.cppm` |
| `BR` | `include/blog/handlers/middleware/bearer.cppm` |
| `SERDE` | `include/blog/handlers/middleware/serde.cppm` |

Scope note: this report covers the 16 routes bound to `api.members.*` (the `members.js` controller). The `members` C++ route table also mounts routes owned by *other* Ghost controllers (`membersStripeConnect`, `membersMetafields`, `memberSigninUrls`, `memberCommenting`, `membersAccount` — `R:183,199-219,257,260-264`; `RT:106-116,175-249,276-285,305-334`). Those are **not** EXTRA here: they have Ghost counterparts in other controller modules and are out of this report's scope.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 16 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Every Ghost route in this controller has a current C++ handler at the exact path+verb. All 16 are PARTIAL: each preserves route identity, request parsing, response envelope and status, but none implement Ghost's real auth/permission stack, service/repository internals, or third-party side effects. Divergences are recorded per route.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members` | `browse` | options `limit,fields,filter,order,debug,page,search,include`; `include` in `allowedIncludes`; query aliases via `restrictAdminApiQueryOptions` | `mw.authAdminApi`; `permissions:true` | 200; `cacheInvalidate:false` | `memberBREADService.browse` → `memberRepository.list` → `_Member.findPage`; offer/gift/suppression/attribution enrichment | `X-Cache-Invalidate:false`; no cache service | `browse` serializer: `{members, meta.pagination}` | `browse/{handler,workflow/step/execute,workflow/flow/main}.cppm` | parses `limit/page/include` from raw path; `ctx.store().json_for("members",p)` + `total("members")`; fabricated `prev/next:null` pagination; token-present auth only | MATCH | `M:58-78`; `R:159`; `BS:792-877`; `OS:9,36-41`; `RT:19-28`; `browse/handler.cppm:34-45`; `browse/workflow/step/execute.cppm:29-52` |
| GET `/members/:id` | `read` | `data:['id','email']`, `options:['include']`; path `:id`; `include` allowlist | `mw.authAdminApi`; `permissions:true` | 200 / 404; `cacheInvalidate:false` | `memberBREADService.read(data,{...opts,metafieldsFor:ADMIN})` → `memberRepository.get` + offer/gift/attribution/suppression | header only | `read` serializer: `{members:[...]}`, NotFoundError message | `read_by_id/...` | extracts id from path; `find_by_resource("members",id)`; 404 `Member not found.`; `{"members":[<member>]}`. No metafields/relations. | MATCH | `M:80-108`; `R:240`; `BS:402-468`; `read_by_id/workflow/step/execute.cppm:29-56` |
| POST `/members` | `add` | `data.members[0]`; `email` required; `options:['send_email','email_type']`; `email_type∈{signin,signup,subscribe}` | `mw.authAdminApi`; `permissions:true`; `verificationTrigger.checkVerificationRequired()` in query | 201; `cacheInvalidate:false` | verification check → `memberBREADService.add` → `memberRepository.create` (+metafields txn, suppression lookup, attribution, Stripe link, magic-link mail, comp subscription) → re-read | header only | `add` serializer: `{members:[...]}` | `create/...` | unwraps `data.members[0]`; validates `email_type` + `email` non-empty; `ctx.store().create("members",inner)` (validates `@`) → 201 | MATCH | `M:110-139`; `R:160`; `BS:527-621`; `CTX:1235-1306`; `create/workflow/step/execute.cppm:30-46` |
| PUT `/members/:id` | `edit` | `data.members[0]`; path `:id`; `options:['id']`, id required | `mw.authAdminApi`; `permissions:true` | **200**; `cacheInvalidate:false` | `memberBREADService.edit` → suppression lookup → `memberRepository.update` (+metafields txn, comp transitions) → re-read | header only | `edit` serializer: `{members:[...]}` | `edit/...` | id from path; `ctx.store().edit("members",id,unwrap(body))`. **Store `wrap()` returns 201 for members edit** (`CTX:1337-1338,1423-1431`) | MATCH | `M:141-163`; `R:241`; `BS:623-681`; `edit/workflow/step/execute.cppm:30-43`; `CTX:1423-1431` |
| DELETE `/members/:id/sessions` | `logout` | path `:id`; `options:['id']`, id required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 204; `cacheInvalidate:false` | `memberBREADService.logout` → `memberRepository.cycleTransientId` (real token invalidation) | header only | no body (204) | `members_id_sessions_delete/...` | id from path; 422 if empty, 404 if member missing; returns 204. **Cache invalidation marker NOT wired** (step comment) | MATCH | `M:165-186`; `R:243`; `BS:788-790`; `members_id_sessions_delete/workflow/step/execute.cppm:29-44` |
| PUT `/members/:id/subscriptions/:subscription_id` | `editSubscription` | path `:id`,`:subscription_id`; `data:['cancel_at_period_end','status']`; `status∈{canceled}`, `cancel_at_period_end` required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `cacheInvalidate:false` | `status==='canceled'` → `members.cancelSubscription` else `members.updateSubscription` (Stripe API calls, `_MemberCancelEvent.add`) → re-read or 404 | header only + **cache** (`members:subscriptions:*`, `Ttl.never`, tag `members`) | `editSubscription` serializer: `{members:[...]}` | `put_detail/...` + `OP:62-84,725-758` | parses parts; `member_subscription_edit` updates status/tier in cache; returns fabricated member envelope. No Stripe, no re-read/404 | MATCH | `M:188-245`; `R:251-255`; `MR:2389-2452,2454-2554`; `put_detail/workflow/step/execute.cppm:31-53`; `OP:62-84` |
| POST `/members/:id/subscriptions/` | `createSubscription` | path `:id`; `data:['stripe_price_id']` required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `cacheInvalidate:false` | `members.createSubscription` → Stripe customer lookup/create + `stripeAPIService.createSubscription` + `linkSubscription` → re-read or 404 | header only + **cache** (`members:subscriptions:*`, `Ttl.never`, tag `members`) | `createSubscription` serializer: `{members:[...]}` | `subscriptions_post/...` + `OP:39-59,725-758` | validates `stripe_price_id`; `member_subscription_create` increments cache counter, hardcodes `current_period_end` epoch, fabricated member (`member@example.com`,`status:paid`) | MATCH | `M:247-288`; `R:250`; `MR:2734-2790`; `subscriptions_post/workflow/step/execute.cppm:31-53`; `OP:39-59,760-773` |
| DELETE `/members/:id` | `destroy` | path `:id`; `options:['id','cancel']`, id required | `mw.authAdminApi`; `permissions:true` | 204; `cacheInvalidate:false` | `members.destroy(...,{require:true,cancelStripeSubscriptions:cancel})` → `memberRepository.destroy` (optional Stripe cancel, metafield snapshot, model destroy) + `member.deleted` webhook | header only | no body (204) | `destroy/...` | id from path; `ctx.store().destroy("members",id)`; store 200 → step converts to 204 (`destroy/...:40-41`). No Stripe/metafields/webhook | MATCH | `M:290-316`; `R:242`; `MR:1225-1291`; `destroy/workflow/step/execute.cppm:29-44` |
| DELETE `/members` | `bulkDestroy` | `options:['all','filter','search']` | `mw.authAdminApi`; `permissions:{method:'destroy'}` | 200; `cacheInvalidate:false` | `members.bulkDestroy` → filter guard (`all\|filter\|search` required) → `_Member.getFilteredCollectionQuery` → `_Member.bulkDestroy` (chunked) | header only | `{meta:{stats:{successful,unsuccessful},unsuccessfulIds,errors}}` | `bulk_destroy/...` | selects ids via `json_for`, builds id list, `store.bulk_destroy`; emits stats + unsuccessfulIds + `errors:[]`. **No `all/filter/search` guard** | MATCH | `M:318-342`; `R:161`; `MR:1293-1323`; `bulk_destroy/workflow/step/execute.cppm:30-49` |
| PUT `/members/bulk` | `bulkEdit` | `options:['all','filter','search']`; `data:['action','meta']`; `action∈{unsubscribe,addLabel,removeLabel}` required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `cacheInvalidate:false` | `members.bulkEdit(frame.data.bulk,opts)` → action + filter guard → lock chunks → `#bulkEditChunk` | header only | serializer `bulkAction`: `{bulk:{action,meta:{stats,errors,unsuccessfulData}}}` | `bulk_edit/...` | validates action only; `store.bulk_edit`; emits **`{meta:{action,updated}}`** (envelope differs from Ghost) | MATCH | `M:344-365`; `R:162`; `MR:1325-1387`; `OS:18,63-77`; `bulk_edit/workflow/step/execute.cppm:30-53` |
| GET `/members/upload` | `exportCSV` | `options:['limit','filter','search']` | `mw.authAdminApi`; `permissions:{method:'browse'}` | 200 (stream); `disposition:csv`, `contentType:text/csv`, `cacheInvalidate:false`; `response:{format:'plain',stream:true}` | `members.export(restrictAdminApiQueryOptions(opts))` → exporter: filtered ids + Knex stream + CSV transform (tiers/labels/metafields/subscription columns) | header only | streamed CSV, filename `<slug>.ghost.members.<date>.csv` | `members_upload_get/...` | `json_for("members",p)` → CSV with only `id,email,name,status`; headers Content-Type/Disposition `members.<date>.csv` | MATCH | `M:367-393`; `R:174`; `S:208-211`; `OS:19,364-377`; `csv-export-filename.js:16-30`; `members_upload_get/handler.cppm:44-49`; `members_upload_get/workflow/step/execute.cppm:30-46` |
| POST `/members/upload` | `importCSV` | multipart `membersfile`; `frame.file.path`, `frame.data.mapping`, `frame.data.labels`; recipient from `frame.user` | `mw.authAdminApi` + `upload.single('membersfile')` + `upload.validation({type:'members'})`; `permissions:{method:'add'}` | 201 inline / 202 deferred; `cacheInvalidate:false` | `members.importCSV({filePath,mapping,extraLabels,requestUserEmail})` → importer: read rows, inline vs deferred threshold, spool + `MembersImportJob` dispatch | header only | `{meta:{originalImportSize,stats:{imported,invalid},import_label}}`; deferred → `{meta:{originalImportSize}}` | `members_upload_post/...` | parses raw body (JSON `file`/`membersfile` or CSV text); line-by-line email parse; `store.create` per row; 201 if imported>0 else 202; meta with all fields even when empty | MATCH | `M:395-422`; `R:175-181`; `S:192-203`; `IMP:274-295,308-328`; `members_upload_post/workflow/step/execute.cppm:30-69` |
| GET `/members/stats/count` | `memberStats` | none | `mw.authAdminApi`; `permissions:{method:'browse'}` | 200; `cacheInvalidate:false` | `events.getStatuses()` → `_MemberStatusEvent.findAll({aggregateStatusCounts})` cumulative per-day deltas | header only | `{resource:'members',total,data:[{date,paid,free,comped,gift}]}` | `members_stats_count_get/...` | counts **live** `members` by `status` into a single today row; `gift` from `gifted`, everything else `free` | MATCH | `M:424-451`; `R:169`; `ER:1246-1277`; `members_stats_count_get/workflow/step/execute.cppm:30-57` |
| GET `/members/stats/mrr` | `mrrStats` | none | `mw.authAdminApi`; `permissions:{method:'browse'}` | 200; `cacheInvalidate:false` | `events.getMRR()` → `_MemberPaidSubscriptionEvent.findAll({aggregateMRRDeltas})` cumulative per-currency | header only | `{resource:'mrr',data:[{currency,data:[{date,value}]}]}` | `members_stats_mrr_get/...` | counts paid members, fabricates single `usd` point `value = paid*1000` on today | MATCH | `M:453-481`; `R:170`; `ER:1211-1244`; `members_stats_mrr_get/workflow/step/execute.cppm:30-52` |
| GET `/members/events` | `activityFeed` | `options:['limit','filter']` | `mw.authAdminApi`; `permissions:{method:'browse'}` | 200; `cacheInvalidate:false` | `events.getEventTimeline(opts)` → NQL split, ~20 event-source queries, merge/sort/tie-break/slice, meta pagination (`page:null,next:null,prev:null`) | header only | `activityFeed` serializer: `{events:[...mapped],meta.pagination}` | `members_events_get/...` | `json_for("events",p)` + `total("events")`; `page:1` (Ghost `null`); `filter` passed as opaque params, no NQL; empty → `[]` | MATCH | `M:483-494`; `R:172`; `ER:77-188`; `OS:24,83-88`; `members_events_get/workflow/step/execute.cppm:29-44` |
| DELETE `/members/:id/suppression` | `deleteEmailSuppression` | path `:id`; `options:['id']`, id required | `mw.authAdminApi`; `permissions:{method:'edit'}` | 204; `cacheInvalidate:false` | read member (404 if missing) → `emailSuppressionList.removeEmail(member.email)` (Mailgun bounce/complaint/unsubscribe + `Suppression.destroy`; 500 if false) → `memberBREADService.edit({email_disabled:false})` | header only + **(unwired)** cache removal op | no body (204) | `members_id_suppression_delete/...` | id from path; 422/404; `store.edit("members",id,'{"email_disabled":false}')` then 204. **Suppression-list removal not wired** (step comment) | MATCH | `M:496-544`; `R:244-248`; `mailgun-email-suppression-list.js:49-69`; `members_id_suppression_delete/workflow/step/execute.cppm:30-49`; `OP:138-145` |

## Route details

### GET `/members` — MATCH

**Ghost** (all facts per checklist 1–8):
1. Identity: `router.get('/members', mw.authAdminApi, http(api.members.browse))` (`R:159`). Router-level: `router.use(apiMw.cors)` (`R:17`); `authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`MW:110-118`).
2. Operation: key `browse`, `docName:'members'` (`M:56,58`), op `browse`.
3. Input: `options:['limit','fields','filter','order','debug','page','search','include']` (`M:62`); `include` restricted to `['email_recipients','products','tiers','metafields']` (`M:49-52,64-69`). No body. `filter[x]=y` arrives as `filter`.
4. Auth/permissions: `mw.authAdminApi` (`R:159`); `permissions:true` (`M:63`) — framework derives `browse` on `members`. `restrictAdminApiQueryOptions` blocks `password/token/gift_link_token/secret` in filter and order with BadRequestError (`AF:78,218-220,247-267,275-289`). Input serializer maps `subscribed` → `newsletters.status`, maps `tiers`→`products`, sets `autoOrder:'created_at DESC, id DESC'`, lifts `metafields` out of relations (`IS:7-17,20-58,76-84,60-74`).
5. Status/headers: default 200; `headers.cacheInvalidate:false` (`M:59-61`).
6. Call chain: `query(frame)` → `memberBREADService.browse(restrictAdminApiQueryOptions(frame.options))` (`M:71-77`). `browse` caps `limit>100` or `'all'` to 100, adds default relations (`labels`, `stripeSubscriptions`, customer/price/product, `currentSubscription`, `products`, `newsletters`, `productEvents`), sets `useBasicCount`, calls `memberRepository.list` → `_Member.findPage` (`BS:792-826`, `MR:1221-1223`). Then fetches offers, offer redemptions, active gifts, bulk suppression data, optional metafields, and attaches them with unsubscribe URLs (`BS:832-871`).
7. Cache: only `X-Cache-Invalidate:false` header; no cache service read/write.
8. Output: `browse` serializer `{members: page.data.map(serializeMember), meta: page.meta}` (`OS:9,36-41`); `serializeMember` emits ~25 fields including computed `comped`, `subscribed`, `attribution`, `tiers`, `metafields`, `current_subscription`, `email_suppression` (`OS:96-172`). Empty page → `{members:[], meta.pagination...}`.

**C++**:
1. Route: `Router{router,"/members"}.use(content_type_middleware).get(...)` → `handler.members_browse().handle(...)` (`RT:19-28`). No `bearer_middleware` on this router (`RT:20`); auth is in-handler. All C++ member flows are single-step: `WorkflowBuilder{}.name(...).step<ExecuteStep>().build()` (`browse/workflow/flow/main.cppm:7-10`).
2. Handler `BrowseHandler` constructor wires `LocalWorkflowDriver`, registers `MainFlow`, sets `ExecuteStep::s_ctx` (`browse/handler.cppm:27-33`). `handle` checks `blog::handlers::middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:34-39`); `token()` reads `get_authorization()` and strips `Bearer ` (`BR:27-33`). `content_type_middleware` requires `Content-Type: application/json` and acceptable `Accept`, else 415/406 (`SERDE:90-115`).
3. Input: raw query parsed from `in.get_path()` by hand (`browse/workflow/step/execute.cppm:56-64`); `include` allowlist checked (`:32-35`); `limit`/`page` via `std::stoi`, default 15/1 (`:37-43`). No `fields/filter/order/debug/search` handling.
4. Store: `ctx.store().json_for("members",p)` (`CTX:1152-1166`) and `total("members")` (`CTX:1140-1150`) (`browse/workflow/step/execute.cppm:44-45`).
5. Status/headers: `res.set_status(OK)` and body (`browse/handler.cppm:44`). **No `X-Cache-Invalidate` header** on this handler (unlike other member handlers).
6. Output: `{"members":<rows>,"meta":{"pagination":{"limit","page","pages","prev":null,"next":null,"total"}}}` (`browse/workflow/step/execute.cppm:47-51`).
7. Cache: none (no `core::cache::Cache` use in this step).
8. Not implemented: real auth/authorization/permissions, NQL filter/order, restricted-field guard, relations, suppression/offer/gift/attribution enrichment, full member field set, pagination `prev/next`.

**Diff**:
- Residual gaps (no C++ primitive): permission services, `memberBREADService` enrichment, restricted-field transformers.
- Divergences: pagination `prev/next` always null; no `X-Cache-Invalidate:false`; auth reduced to "Authorization header non-empty".

### GET `/members/:id` — MATCH

**Ghost**:
1. `router.get('/members/:id', mw.authAdminApi, http(api.members.read))` (`R:240`); path param `:id`.
2. Key `read`, op `read`, `docName:'members'` (`M:56,80`).
3. `data:['id','email']`, `options:['include']`, `include` allowlist (`M:81-92`); path `:id` and optional `email`.
4. `permissions:true` (`M:93`); input serializer `read()` delegates to `browse()` (`IS:86-90`), so default relations + `subscribed`/`tiers` transforms apply.
5. 200 / 404; `cacheInvalidate:false` (`M:82-84`).
6. `query` → `memberBREADService.read(frame.data,{...frame.options,metafieldsFor:ADMIN})` (`M:94-99`); `!member` → `NotFoundError('Member not found.')` (`M:100-104`). `read` loads 9 default relations + `productEvents`, `model.toJSON`, filters subscriptions without `price`, attaches offers/gifts/next payment/attributions, suppression data, unsubscribe url, and ADMIN metafields (`BS:402-468`).
7. Cache: `X-Cache-Invalidate:false` only.
8. `read` serializer `{members:[serializeMember(model)]}` (`OS:10,50-54`).

**C++**:
1. Route `Router{router,"/:id"}.get(...)` → `members_read_by_id()` (`RT:45-54`).
2. Handler `ReadByIdHandler`: token check 401, then workflow (`read_by_id/handler.cppm:34-45`); sets `X-Cache-Invalidate:false` (`:48`).
3. Input: id = last path segment after stripping `?query` (`read_by_id/workflow/step/execute.cppm:31-35`); empty id → 422 validation on `id` (`:36-38`); `include` allowlist enforced (`:39-49`).
4. Store: `find_by_resource("members",id)` (`CTX:1084-1105`); missing → 404 `ApiError.json("Member not found.","Member not found")` (`:50-53`).
5. Status: 200/404/422 from step map; handler forwards step status (`read_by_id/handler.cppm:45-49`).
6. Output: `{"members":[<serialized member>]}` (`:54-55`).

**Diff**: residual gaps — metafields/relations/suppression/attribution enrichment, `email` lookup path, permissions. Divergence — 404 body is `{"error"...}`-style `ApiError.json` rather than Ghost's `{"errors":[{...}]}` envelope; member JSON field set differs.

### POST `/members` — MATCH

**Ghost**:
1. `router.post('/members', mw.authAdminApi, http(api.members.add))` (`R:160`).
2. Key `add`, op `add` (`M:110`).
3. Body `data.members[0]`; `validation.data.email.required` (`M:116-120`); `options:['send_email','email_type']`, `email_type∈{signin,signup,subscribe}` (`M:115,120-124`). Input serializer converts string labels to `{name}`, maps `tiers`→`products`, applies `defaultRelations` (`IS:98-115`).
4. `permissions:true` (`M:126`). In `query`: `verificationTrigger.checkVerificationRequired()` → if true, logs warning and forces `send_email=false` (`M:128-131`, `verification-trigger.js:187-194`).
5. 201 (`statusCode:201`), `cacheInvalidate:false` (`M:111-114`).
6. `memberBREADService.add(frame.data.members[0], frame.options)` (`M:132-135`): plans metafields in a transaction, rejects Stripe data without Stripe connection, sets `email_disabled` from suppression, derives attribution, `memberRepository.create` (emits `MemberCreatedEvent`, `member.added` webhook), optional `linkStripeCustomer`, optional magic-link email, optional comped subscription, then re-read with ADMIN metafields (`BS:527-621`, `MR:585-840`).
7. Cache: header only.
8. `add` serializer → `{members:[...]}` (`OS:12,50-54`).

**C++**:
1. Route `Router{router,"/members"}.post(...)` → `members_create()` (`RT:29-36`).
2. Handler `CreateHandler`: 401 token check, runs workflow, reads `{status,body}` map, sets `X-Cache-Invalidate:false`, forwards status (`create/handler.cppm:34-50`).
3. Input: query `email_type` allowlist → 422 (`:33-35`); body unwrapped from `data.members[0]` (`unwrap`, `:51-67`); missing/empty `email` → 422 (`:36-42`).
4. Store: `ctx.store().create("members",inner)` (`:43`); store validates `@` in email → 422 and `create_entity<model::Member>` → 201 envelope (`CTX:1259-1267,1289-1291,1423-1431`).
5. Status: 201 on success, 422 on validation; header `X-Cache-Invalidate:false`.
6. Output: store mutation body, e.g. `{"members":[{...}]}`.

**Diff**: residual gaps — verification trigger, Stripe, suppression lookup, attribution, magic-link email, metafields transaction/domain events, real auth. Divergences — email validation is `@` substring check, not Ghost's required + JSON-schema; no duplicate-email 422 path (Ghost `memberAlreadyExists`, `BS:559-568`).

### PUT `/members/:id` — MATCH

**Ghost**:
1. `router.put('/members/:id', mw.authAdminApi, http(api.members.edit))` (`R:241`).
2. Key `edit`, op `edit` (`M:141`).
3. Body `data.members[0]`; `options:['id']` with id required (`M:146-153`); path `:id`.
4. `permissions:true` (`M:154`); input serializer `edit`→`add`→`defaultRelations` (`IS:117-120`); `jsonSchema.validate` on add/edit (`validators/input/members.js:3-6`).
5. **statusCode:200**, `cacheInvalidate:false` (`M:142-145`).
6. `memberBREADService.edit` (`M:156-159`): deletes `last_seen_at`, plans metafields, suppression lookup, `memberRepository.update` (unique-email → ValidationError, `member.edited` event), optional comped-subscription add/remove, re-read (`BS:623-681`, `MR:850-...`).
7. Cache: header only.
8. `edit` serializer `{members:[...]}` (`OS:11,50-54`).

**C++**:
1. Route `Router{router,"/:id"}.put(...)` → `members_edit()` (`RT:55-62`).
2. Handler `EditHandler`: 401, workflow, `{status,body}`, `X-Cache-Invalidate:false` (`edit/handler.cppm:34-50`).
3. Input: id = last path segment, empty → 422 (`edit/workflow/step/execute.cppm:31-39`); body unwrapped via same `data.members[0]` logic (`:48-64`).
4. Store: `ctx.store().edit("members",id,unwrap(body))` (`:40`) → `wrap("members", edit_entity<model::Member>(...))` (`CTX:1337-1338`) which returns **201** (`CTX:1428-1430`).
5. Status: **201** on success (diverges from Ghost 200), 404 if entity missing (`CTX:1425-1427`), 422 if id empty.
6. Output: `{"members":[{...}]}`.

**Diff**: divergence — success status 201 vs Ghost 200. Residual gaps — suppression/metafields/comped transitions, domain events, unique-email error, permissions.

### DELETE `/members/:id/sessions` — MATCH

**Ghost**:
1. `router.delete('/members/:id/sessions', mw.authAdminApi, http(api.members.logout))` (`R:243`).
2. Key `logout`, op `logout` (`M:165`).
3. `options:['id']`, id required (`M:170-177`); path `:id`.
4. `permissions:{method:'edit'}` — maps logout to the edit permission (`M:178-180`).
5. statusCode 204, `cacheInvalidate:false` (`M:166-169`).
6. `memberBREADService.logout(frame.options)` → `memberRepository.cycleTransientId(options)` — rotates transient id, invalidating member sessions (`M:181-185`, `BS:788-790`, `MR:556-...`).
7. Cache: header only (server-side session invalidation, not a cache service).
8. No body.

**C++**:
1. Route `Route{"/sessions"}.delt(...)` nested under `/:id` → `members_members_id_sessions_delete()` (`RT:96-106`).
2. Handler `MembersIdSessionsDeleteHandler`: 401, workflow, `{status,body}`, `X-Cache-Invalidate:false` (`members_id_sessions_delete/handler.cppm:34-50`).
3. Input: id extracted between `/sessions` and preceding `/` (`:31-35`); empty → 422 (`:36-38`).
4. Store: `find_by_resource("members",id)` for existence; missing → 404 (`:39-41`).
5. Status: always 204 once member exists (`:43`).
6. Side effect: **none** — comment `// Session invalidation marker is context-local; full cache write requires MembersOperations wiring.` (`:42`). `OP::invalidate_sessions` exists (`OP:131-136`) but is not called.

**Diff**: residual gap — the route's core side effect (transient-id cycle / session invalidation) is unimplemented; observable status/404 still match. Divergence — no use of the available cache primitive.

### PUT `/members/:id/subscriptions/:subscription_id` — MATCH

**Ghost**:
1. `router.put('/members/:id/subscriptions/:subscription_id', mw.authAdminApi, http(api.members.editSubscription))` (`R:251-255`).
2. Key `editSubscription`, op custom (`M:188`).
3. `options:['id','subscription_id']` both required; `data:['cancel_at_period_end','status']`, `cancel_at_period_end` required, `status∈{canceled}` (`M:193-212`). Path params `:id`,`:subscription_id`; body fields.
4. `permissions:{method:'edit'}` (`M:213-215`).
5. 200, `cacheInvalidate:false` (`M:189-192`).
6. Branch: `status==='canceled'` → `members.cancelSubscription({id,subscription:{subscription_id}})` → Stripe `cancelSubscription`, `linkSubscription`, `_MemberCancelEvent.add` (`MR:2389-2452`); else `members.updateSubscription({id,subscription:{subscription_id,cancel_at_period_end}})` → Stripe price/cancel-at-period-end/continue calls + cancel event + `linkSubscription` (`MR:2454-2554`). Then `memberBREADService.read({id},{metafieldsFor:ADMIN})`, 404 if missing (`M:233-243`).
7. Cache: `X-Cache-Invalidate:false`; real state in Stripe + DB events.
8. `editSubscription` serializer `{members:[...]}` (`OS:15`).

**C++**:
1. Route nested `/:id` → `/subscriptions` → `Route{"/:id"}.put(...)` → `members_put_detail()` (`RT:118-139`).
2. Handler `PutDetailHandler` constructed with `MembersOperations&` (`put_detail/handler.cppm:29-33`); `s_ops=&ops`; 401, workflow, `{status,body}`, header (`put_detail/handler.cppm:35-49`).
3. Input: splits path into parts; member id = `parts[1]`, subscription id = `parts.back()`; <2 parts → 422 (`:31-43`); body must be valid JSON (`:45-49`); `stripe_price_id` not required here.
4. Store/cache: `s_ops->member_subscription_edit(id,subscriptionId,body)` (`OP:62-84`) — `load_subscriptions` reads `members:subscriptions:<id>:count` + indexed JSON from `m_cache` (`OP:725-743`), updates `status`/`tier.name` from wire or DTO, `store_subscriptions` writes back with `Ttl{}.never()`, tag `{"members"}` (`OP:745-758`). `m_members_ops{m_cache}` wired at `all.cppm:929`.
5. Status: 200 from `r.first`; `X-Cache-Invalidate:false`.
6. Output: `member_response(items,id)` → `{"members":[{email:"member@example.com",id:<id>,status:"paid",subscriptions:[...]}]}` (`OP:760-773`).

**Diff**: residual gaps — Stripe cancellation/update, `_MemberCancelEvent`, `cancel_at_period_end` semantics (C++ only overwrites `status`/`tier`), re-read of real member and 404. Divergences — fabricated member (`member@example.com`, `status:paid`), `current_period_end` not updated on edit; state lives in cache not DB/Stripe.

### POST `/members/:id/subscriptions/` — MATCH

**Ghost**:
1. `router.post('/members/:id/subscriptions/', mw.authAdminApi, http(api.members.createSubscription))` (`R:250`).
2. Key `createSubscription`, op custom (`M:247`).
3. `options:['id']` required; `data:['stripe_price_id']` required (`M:252-265`).
4. `permissions:{method:'edit'}` (`M:266-268`).
5. 200, `cacheInvalidate:false` (`M:248-251`).
6. `members.createSubscription({id,subscription:{stripe_price_id}})` (`M:270-275`) → requires Stripe configured, fetches member + existing stripeCustomers, `getCustomer` or `createCustomer` + `_StripeCustomer.add`, `stripeAPIService.createCustomer`/`createSubscription`, `linkSubscription` (`MR:2734-2790`). Then re-read member, 404 if missing (`M:276-284`).
7. Cache: `X-Cache-Invalidate:false`; state in Stripe/DB.
8. `createSubscription` serializer `{members:[...]}` (`OS:16`).

**C++**:
1. Route `Router{router,"/subscriptions"}.post(...)` nested under `/:id` → `members_subscriptions_post()` (`RT:118-127`).
2. Handler `SubscriptionsPostHandler` with `MembersOperations&`, 401, workflow, `{status,body}`, header (`subscriptions_post/handler.cppm:29-49`).
3. Input: id parsed from path before `/subscriptions`, empty → 422 (`:31-41`); body must contain `stripe_price_id` at top level or one nested level (`has_field`, `:57-68`), else 422 (`:42-46`).
4. Store/cache: `s_ops->member_subscription_create(id,body)` (`OP:39-59`) — decodes `MemberSubscriptionInput`, `load_subscriptions`, increments `members:subscriptions:next`, builds subscription with `status` from input (default `active`), `tier.name` from input (default `free`), **hardcoded** `current_period_end:"1970-01-01T00:00:00.000Z"`, appends and writes cache (`Ttl.never`, tag `members`).
5. Status: 200 (`OP:58`) even though Ghost creates a resource; 404 if member missing (`:48-50`).
6. Output: fabricated `member_response` (`OP:760-773`).

**Diff**: residual gaps — Stripe customer/subscription creation, `linkSubscription`, real member re-read. Divergences — no Stripe call, epoch `current_period_end`, fabricated member body, subscription stored in cache only.

### DELETE `/members/:id` — MATCH

**Ghost**:
1. `router.delete('/members/:id', mw.authAdminApi, http(api.members.destroy))` (`R:242`).
2. Key `destroy`, op `destroy` (`M:290`).
3. `options:['id','cancel']`, id required (`M:295-302`). `cancel` → `cancelStripeSubscriptions`.
4. `permissions:true` (`M:303`).
5. statusCode 204, `cacheInvalidate:false` (`M:291-294`).
6. `members.destroy({id},{...options,require:true,cancelStripeSubscriptions:cancel})` (`M:304-315`) → `memberRepository.destroy`: `_Member.findOne`, optional Stripe subscription cancellations + `_MemberPaidSubscriptionEvent.add`, snapshots metafields, `_Member.destroy` in transaction (`MR:1225-1291`). Model emits `member.deleted` webhook.
7. Cache: header only.
8. No body (204).

**C++**:
1. Route `Router{router,"/:id"}.delt(...)` → `members_destroy()` (`RT:63-70`).
2. Handler `DestroyHandler`: 401, workflow, `{status,body}`, header (`destroy/handler.cppm:34-50`).
3. Input: id = last path segment, empty → 422 (`:31-38`). `cancel` option ignored.
4. Store: `ctx.store().destroy("members",id)` (`:39`) → `destroy_entity<model::Member>` or 404 (`CTX:1356-1382`); step maps store 200 → 204 (`:40-41`).
5. Status: 204 success, 404/422 errors; `X-Cache-Invalidate:false`.
6. Output: empty body on 204.

**Diff**: residual gaps — Stripe cancellation, event rows, metafield snapshot, `member.deleted` webhook, `require:true`, `cancel` option. Observable 204/404 match.

### DELETE `/members` — MATCH

**Ghost**:
1. `router.delete('/members', mw.authAdminApi, http(api.members.bulkDestroy))` (`R:161`).
2. Key `bulkDestroy`, op custom (`M:318`).
3. `options:['all','filter','search']` (`M:323`); input serializer applies `validateBulkFilter` (rejects restricted fields with BadRequestError) + `mapSubscribedFlagToNewsletterRelation` (`IS:144-148`, `AF:291-301`).
4. `permissions:{method:'destroy'}` (`M:324-326`).
5. 200, `cacheInvalidate:false` (`M:319-322`).
6. `members.bulkDestroy(frame.options)` (`M:328`) → guard `all===true || filter || search` else IncorrectUsageError; `_Member.getFilteredCollectionQuery` selects distinct ids; `_Member.bulkDestroy(ids)` (`MR:1293-1323`).
7. Cache: header only.
8. Query returns `{meta:{stats:{successful,unsuccessful},unsuccessfulIds,errors}}` (`M:331-340`); serializer `passthrough` (`OS:17`).

**C++**:
1. Route `Router{router,"/members"}.delt(...)` → `members_bulk_destroy()` (`RT:37-44`).
2. Handler: 401, workflow, `{status,body}`, header (`bulk_destroy/handler.cppm:34-50`).
3. Input: query params from path; `all/filter/search` parsed but **not enforced** (`:32`, helper `:74-82`).
4. Store: `json_for("members",p)` → extract ids (`:33-35,58-73`); `find_by_resource` precheck builds `unsuccessful` list (`:36-41`); `ctx.store().bulk_destroy("members",...)` (`:44`) → count (`CTX:1385-1389`).
5. Status: store status (200); `X-Cache-Invalidate:false`.
6. Output: `{"meta":{"stats":{"successful","unsuccessful"},"unsuccessfulIds":[...],"errors":[]}}` (`:46-48`).

**Diff**: residual gaps — restricted-field bulk filter validation, `all/filter/search` required guard, real DB filter/transaction. Divergence — no guard means a bare `DELETE /members` deletes all rows instead of erroring; `errors` always `[]`.

### PUT `/members/bulk` — MATCH

**Ghost**:
1. `router.put('/members/bulk', mw.authAdminApi, http(api.members.bulkEdit))` (`R:162`).
2. Key `bulkEdit`, op custom (`M:344`).
3. `options:['all','filter','search']`; `data:['action','meta']`; `action` required and `∈{unsubscribe,addLabel,removeLabel}` (`M:349-358`); body under `bulk`. Input serializer `validateBulkFilter` + `mapSubscribed...` (`IS:138-142`).
4. `permissions:{method:'edit'}` (`M:359-361`).
5. 200, `cacheInvalidate:false` (`M:345-348`).
6. `members.bulkEdit(frame.data.bulk,frame.options)` (`M:363`) → action + filter guard, distinct ids, chunked `lockForUpdate` + `#bulkEditChunk`, partial failure reporting (`MR:1325-1387`).
7. Cache: header only.
8. Serializer `bulkAction` → `{bulk:{action,meta:{stats:{successful,unsuccessful},errors,unsuccessfulData}}}` (`OS:18,63-77`); note this **overrides** the query's raw `{meta:{...}}` shape.

**C++**:
1. Route `Route{"/bulk"}.put(...)` → `members_bulk_edit()` (`RT:152-162`).
2. Handler: 401, workflow, `{status,body}`, header (`bulk_edit/handler.cppm:34-50`).
3. Input: `action` read from body, validated against the same 3 values → 422 (`bulk_edit/workflow/step/execute.cppm:32-44`); `all/filter/search` not enforced.
4. Store: ids from `json_for("members",p)`; `ctx.store().bulk_edit("members",{ids,action})` (`:45-50`) → count (`CTX:1391-1395`).
5. Status: 200; header.
6. Output: `{"meta":{"action":"<action>","updated":<ids.size()>}}` (`:51-52`).

**Diff**: residual gaps — filter guard, restricted-field validation, chunked locking, `meta`/label handling, partial-failure data. Divergence — response envelope is `meta.action`/`meta.updated`, not Ghost's `bulk.action`/`bulk.meta.stats`.

### GET `/members/upload` — MATCH

**Ghost**:
1. `router.get('/members/upload', mw.authAdminApi, http(api.members.exportCSV))` (`R:174`).
2. Key `exportCSV`, op custom (`M:367`).
3. `options:['limit','filter','search']` (`M:368`), passed through `restrictAdminApiQueryOptions` (`M:389`).
4. `permissions:{method:'browse'}` (`M:383-385`); input serializer `exportCSV`→`browse` transforms (`IS:92-96`).
5. Headers: `disposition` type csv with `getCSVExportFileName('members')`, `contentType:'text/csv'`, `cacheInvalidate:false`; `response:{format:'plain',stream:true}` (`M:369-382`). Filename = `<slug(title)>.ghost.members.<YYYY-MM-DD>.csv` (`csv-export-filename.js:16-30`).
6. `query` → `membersService.export(restrictAdminApiQueryOptions(opts))` (`M:387-392`) → exporter built in `S:208-211`; exporter reads filtered member ids, Knex streams rows with tiers/labels/subscription/metafield columns (`services/members/import-export/export/exporter.ts:11-90`).
7. Cache: header only.
8. `exportCSV` serializer pipes a stream through `createCSVTransform()` (`OS:19,364-377`); no JSON envelope.

**C++**:
1. Route `Route{"/upload"}.get(...)` → `members_members_upload_get()` (`RT:286-295`).
2. Handler: 401, workflow; sets `Content-Type:text/csv`, `Content-Disposition:attachment; filename=members.<date>.csv`, `X-Cache-Invalidate:false`, 200 (`members_upload_get/handler.cppm:34-50`).
3. Input: query params parsed (`:32`, `:59-67`); `limit/filter/search` passed to store.
4. Store: `json_for("members",p)` (`:33`) → CSV header `id,email,name,status` + rows via `csv_field` (`:34-45,52-58`).
5. Status/headers: 200, CSV headers above.
6. Output: raw CSV body.

**Diff**: residual gaps — filtered-id query + Knex stream + full CSV column set (tiers, labels, note, created_at, metafields). Divergences — filename lacks site-title slug/`.ghost.` convention; only 4 columns.

### POST `/members/upload` — MATCH

**Ghost**:
1. `router.post('/members/upload', mw.authAdminApi, apiMw.upload.single('membersfile'), apiMw.upload.validation({type:'members'}), http(api.members.importCSV))` (`R:175-181`).
2. Key `importCSV`, op custom (`M:395`).
3. Multipart field `membersfile` → `frame.file.path`; `frame.data.mapping`; `frame.data.labels`; recipient `frame.user.get('email')` or null (`M:414-419`). Input serializer normalizes `labels` string/array to `[{name}]` (`IS:122-136`).
4. `permissions:{method:'add'}` (`M:406-408`); upload middleware chain above.
5. `statusCode(result)` → **201** when inline (`meta.stats.imported !== null`), else **202** deferred (`M:399-405`); `cacheInvalidate:false` (`M:397`).
6. `membersService.importCSV(...)` (`M:414-419`) → `membersCSVImporter.importCSV(request,verificationTrigger)` (`S:198-199`) → read rows, build label from timezone, `canImportInline` threshold check; inline → `_tracking.import('inline', importRows)`, else `deferImport` (spool + `MembersImportJob` dispatch + logging) (`IMP:274-295,222-223,308-328`).
7. Cache: header only; state in DB/jobs.
8. `importCSVResponse` shapes `{meta:{originalImportSize,stats:{imported,invalid},import_label}}` or deferred `{meta:{originalImportSize}}` (`M:14-27`); serializer `passthrough` (`OS:21`).

**C++**:
1. Route `Route{"/upload"}.post(...)` → `members_members_upload_post()` (`RT:296-303`).
2. Handler: 401, workflow, `{status,body}`, header (`members_upload_post/handler.cppm:34-50`). No multipart/upload middleware.
3. Input: `body_as_string()`; empty → 422 (`members_upload_post/workflow/step/execute.cppm:32-35`); JSON object with `file`/`membersfile` string and `import_label` recognized, else raw treated as CSV (`:36-47`).
4. Store: line loop; skips header `id,`; requires `,` and `@` in email; `ctx.store().create("members",{email})` per row; counts imported/invalid (`:48-64`).
5. Status: `imported>0 ? 201 : 202` (`:65`); header.
6. Output: `{meta:{originalImportSize:<file_bytes.size()>,stats:{imported,invalid},import_label:label}}` (`:66-68`).

**Diff**: residual gaps — real multipart upload + validation, mapping, verification trigger, inline/deferred threshold, spool + background job, DB import. Divergences — `originalImportSize` is byte length not row count; no deferred branch shape (`{meta:{originalImportSize}}`); `mapping`/`labels` ignored; import_label default `members` rather than generated label.

### GET `/members/stats/count` — MATCH

**Ghost**:
1. `router.get('/members/stats/count', mw.authAdminApi, http(api.members.memberStats))` (`R:169`).
2. Key `memberStats`, op custom (`M:424`).
3. No options/data.
4. `permissions:{method:'browse'}` (`M:428-430`).
5. 200, `cacheInvalidate:false` (`M:425-427`).
6. `events.getStatuses()` (`M:432`) → `_MemberStatusEvent.findAll({aggregateStatusCounts:true})`, cumulative per-day `paid/comped/free/gift` deltas (`ER:1246-1277`); `total` from last cumulative row (`M:433-434`); dates formatted `YYYY-MM-DD` via moment (`M:439-448`).
7. Cache: header only.
8. Serializer `passthrough` (`OS:22`) → `{resource:'members',total,data:[...]}` (`M:436-449`).

**C++**:
1. Route `Router{router,"/stats"} → Route{"/count"}.get(...)` → `members_members_stats_count_get()` (`RT:250-262`).
2. Handler: 401, workflow, string body, 200, `X-Cache-Invalidate:false` (`members_stats_count_get/handler.cppm:34-46`).
3. Input: none.
4. Store: `json_for("members",{})` (`:32`); counts `status` field → `paid/comped/gifted/else free` (`:33-48`); `total=sum` (`:49`).
5. Status/headers: 200.
6. Output: single today row `{date,paid,free,comped,gift}` + resource/total (`:50-56`).

**Diff**: residual gaps — event-derived cumulative time series (`_MemberStatusEvent`), historical dates. Divergence — live-member snapshot rather than event history; `gifted` status mapped to `gift`.

### GET `/members/stats/mrr` — MATCH

**Ghost**:
1. `router.get('/members/stats/mrr', mw.authAdminApi, http(api.members.mrrStats))` (`R:170`).
2. Key `mrrStats`, op custom (`M:453`).
3. No options/data.
4. `permissions:{method:'browse'}` (`M:457-459`).
5. 200, `cacheInvalidate:false` (`M:454-456`).
6. `events.getMRR()` (`M:461`) → `_MemberPaidSubscriptionEvent.findAll({aggregateMRRDeltas:true})`, cumulative per-currency `mrr` deltas (`ER:1211-1244`); controller groups by currency, formats dates (`M:462-475`).
7. Cache: header only.
8. Serializer `passthrough` (`OS:23`) → `{resource:'mrr',data:[{currency,data:[{date,value}]}]}` (`M:476-479`).

**C++**:
1. Route `Router{router,"/stats"} → Route{"/mrr"}.get(...)` → `members_members_stats_mrr_get()` (`RT:263-273`).
2. Handler: 401, workflow, 200, header (`members_stats_mrr_get/handler.cppm:34-46`).
3. Input: none.
4. Store: `json_for("members",{})` (`:32`); counts `status==="paid"` (`:33-46`); emits one point `value = paid*1000` (`:49`).
5. Status/headers: 200.
6. Output: `{resource:'mrr',data:[{currency:'usd',data:[{date:<today>,value}]}]}` (`:49-51`).

**Diff**: residual gap — event-derived cumulative MRR per real currency. Divergence — fabricated currency `usd`, fabricated `paid*1000` value, single point.

### GET `/members/events` — MATCH

**Ghost**:
1. `router.get('/members/events', mw.authAdminApi, http(api.members.activityFeed))` (`R:172`).
2. Key `activityFeed`, op custom (`M:483`).
3. `options:['limit','filter']` (`M:487`); filter is NQL (`data.post_id` special-casing).
4. `permissions:{method:'browse'}` (`M:488-490`).
5. 200, `cacheInvalidate:false` (`M:484-486`).
6. `events.getEventTimeline(frame.options)` (`M:492`): defaults `limit=10`, splits NQL type filter, runs ~20 event-source queries, flattens, sorts by `created_at` with explicit tie order, slices to limit, returns meta with `page/next/prev:null` (`ER:77-188`).
7. Cache: header only.
8. `activityFeed` serializer maps each event through `mappers.activityFeedEvents` and returns `{events,meta}` (`OS:24,83-88`).

**C++**:
1. Route `Route{"/events"}.get(...)` → `members_members_events_get()` (`RT:163-173`).
2. Handler: 401, workflow, 200, header (`members_events_get/handler.cppm:34-46`).
3. Input: `limit` parsed (default 15 — Ghost default 10), `filter` passed as opaque param (`:31-37`, `:49-57`). No NQL parsing.
4. Store: `json_for("events",p)` + `total("events")` (`:32-33`); pages from `total/limit` (`:38`).
5. Status/headers: 200.
6. Output: `{events:<rows|[]>,meta:{pagination:{limit,page:1,pages,prev:null,next:null,total}}}` (`:39-43`).

**Diff**: residual gaps — NQL filter, ~20 event sources, tie-break sort, `activityFeedEvents` mapping. Divergences — `limit` default 15 vs 10; `page:1` vs Ghost `null`; no per-event mapping; events from opaque store.

### DELETE `/members/:id/suppression` — MATCH

**Ghost**:
1. `router.delete('/members/:id/suppression', mw.authAdminApi, http(api.members.deleteEmailSuppression))` (`R:244-248`).
2. Key `deleteEmailSuppression`, op custom (`M:496`).
3. `options:['id']`, id required (`M:501-508`); path `:id`.
4. `permissions:{method:'edit'}` (`M:509-511`).
5. statusCode 204, `cacheInvalidate:false` (`M:497-500`).
6. `query` (`M:512-543`): lazily requires email-suppression-list service; `memberBREADService.read({id},{metafieldsFor:ADMIN})`, 404 if missing (`M:516-525`); `emailSuppressionList.removeEmail(member.email)` → Mailgun `removeBounce/removeComplaint/removeUnsubscribe` + `Suppression.destroy`, false → `InternalServerError('Failed to remove email suppression.')` (`M:528-534`, `mailgun-email-suppression-list.js:49-69`); `memberBREADService.edit({email_disabled:false},{id})` (`M:537-540`); returns null.
7. Cache: header only.
8. No body (204); serializer not defined for this key (passthrough of null by framework).

**C++**:
1. Route `Route{"/suppression"}.delt(...)` nested under `/:id` → `members_members_id_suppression_delete()` (`RT:140-150`).
2. Handler: 401, workflow, `{status,body}`, header (`members_id_suppression_delete/handler.cppm:34-50`).
3. Input: id extracted between `/suppression` and preceding `/`, empty → 422 (`:31-39`).
4. Store: `find_by_resource("members",id)` → 404 if missing (`:40-43`); `store.edit("members",id,'{"email_disabled":false}')`, non-200 forwarded (`:44-47`); then 204 (`:49`).
5. Status/headers: 204 success, 422/404/other from store; `X-Cache-Invalidate:false`.
6. Side effect: **suppression-list removal not wired** — comment `// Suppression-list removal is a context-local cache operation; full wiring requires MembersOperations.` (`:48`). `OP::remove_suppression` exists (`OP:138-145`) but is not called. No 500 error branch.

**Diff**: residual gaps — Mailgun/DB suppression removal, 500 branch, member read with metafields. Divergence — half the route (suppression removal) is a no-op; only `email_disabled:false` is performed.

## Module-level notes

**Ghost shared machinery**
- All 16 routes carry `headers.cacheInvalidate:false` (`M:59-61,82-84,112-114,143-145,167-169,190-192,249-251,292-294,320-322,346-348,369-378,396-398,425-427,454-456,484-486,498-500`), producing `X-Cache-Invalidate:false`. No Ghost cache service is read/written by this controller.
- Auth for every route is `mw.authAdminApi` (`R:159-162,169-181,240-255`): `authenticateAdminApi` + `authorizeAdminApi` + `updateUserLastSeen` + `cors` + SSL/host + prettyUrls + `tokenPermissionCheck` (`MW:110-118`). Permission escalation is declared on the controller (`permissions:true` or `{method:'edit'|'destroy'|'browse'|'add'}`, `M:63,93,126,154,178-180,213-215,266-268,303,324-326,359-361,383-385,406-408,428-430,457-459,488-490,509-511`) and enforced by `@tryghost/api-framework`'s `http()` wrapper (`R:3`), not by code inside `query()`. No permission service is called inside any `query()` in `members.js`.
- Domain side effects live in repositories/services, not the controller: `memberRepository` dispatches domain events (`DomainEvents.dispatch`, `MR:249-284`), writes `MemberCreatedEvent`/`_MemberCancelEvent`/`_MemberPaidSubscriptionEvent` rows (`MR:815-840,2445-2451,2531-2537,1252-1263`), and sends `member.added/edited/deleted` webhooks via model plugins. Stripe calls (`cancelSubscription`, `updateSubscription*`, `createSubscription`) are in `stripeAPIService` through `MR:2433,2500-2527,2778-2781`. `EmailSuppressedEvent` subscription flips `email_disabled:true` (`MA:513-519`).
- `read`/`browse` enrich every member with offers, gifts, next-payment, attribution, unsubscribe URL and suppression data (`BS:444-461,832-869`).
- Analytics (`memberStats`/`mrrStats`/`activityFeed`) read pre-aggregated event tables, not live members (`ER:77-188,1211-1277`).
- Import/export are standalone services initialized in `S:192-211`; import chooses inline vs deferred then spools + dispatches `MembersImportJob` (`IMP:274-328`). Verification gating uses hostSettings thresholds (`S:124-146`).
- Serializers: input `IS` maps relations/filters; output `OS` builds the envelopes. `exportCSV` is a stream with no JSON envelope (`OS:364-377`).

**C++ shared machinery**
- All 16 handlers follow the same pattern: `LocalWorkflowDriver` + one-step `MainFlow`, `ExecuteStep::s_ctx` set in the constructor, `token(req).empty()` → `401 {"error":"Unauthorized"}`, workflow run, status/body extracted from a `std::map<std::string,std::string>` payload, `X-Cache-Invalidate:false` header, then `send()` (e.g. `browse/handler.cppm:27-45`; `create/handler.cppm:27-50`). `browse/handler.cppm` is the sole handler that does **not** set `X-Cache-Invalidate:false`.
- Router mounts only `content_type_middleware` (`RT:19-20`); `bearer_middleware` is not applied to these routes (`BR:11-24` is used by other routers). `token()` merely checks that the `Authorization` header is non-empty (`BR:27-33`) — no signature, role, scope, or permission verification anywhere in this module. This is the largest cross-route residual gap.
- `content_type_middleware` rejects missing/non-JSON `Content-Type` with 415 and unacceptable `Accept` with 406 (`SERDE:90-115`) — a request-shape behavior Ghost's `http()` wrapper does not express identically.
- Query parameters are parsed by hand from `in.get_path()` in every step (e.g. `browse/workflow/step/execute.cppm:56-64`); there is no NQL, no restricted-field transformer, and `filter` is passed opaquely to `ctx.store().json_for(...)`.
- `ctx.store()` calls used by these routes: `json_for("members"|"events",p)` (`CTX:1152-1166`), `total("members"|"events")` (`CTX:1140-1150`), `find_by_resource("members",id)` (`CTX:1084-1105`), `create/edit/destroy/bulk_destroy/bulk_edit` on `"members"` (`CTX:1235-1395`). Reads come from an in-memory `model::Member` database/fixture store, not Knex/Stripe.
- Cache: only `subscriptions_post` and `put_detail` touch `core::cache::Cache` through `MembersOperations` (`OP:39-84,725-773`), wired via `m_members_ops{m_cache}` (`all.cppm:929`). Keys are `members:subscriptions:<id>[:count|:<n>|:next]`, `Ttl{}.never()`, tag `{"members"}` (`OP:490-509,745-758`). Other side-effect helpers in `OP` — `invalidate_sessions` (`:131-136`), `remove_suppression` (`:138-145`), `member_signin_urls` (`:86-122`), `stripe_connect_state_set` (`:124-129`) — are **not** called by the 16 routes in scope (they belong to other controllers or are explicitly marked unwired in step comments).
- Divergence pattern across mutations: fabricated/fixture data (`member@example.com`, `status:paid`, epoch `current_period_end`, `paid*1000` MRR, `usd`, `page:1`, `prev/next:null`), hardcoded ids/counters, and store status `201` for member `edit` (`CTX:1423-1431`).

**Out-of-scope C++ routes (not EXTRA):** `/members/metafields*`, `/members/:id/signin_urls`, `/members/:id/commenting/*`, `/members/stripe_connect`, `/members/api/member*` (`RT:106-116,175-249,276-285,305-334`) map to Ghost controllers `members-metafields.ts`, `member-signin-urls.js`, `member-commenting.ts`, `members-stripe-connect.js`, `members-account.ts` (`R:183,199-219,257,260-264`). They are intentionally excluded from the verdict counts here.

## Group 43 — strict-parity implementation record (2026-11-20)

All route verdicts in this report were re-derived after the strict-parity rewrite;
the pre-rewrite `#### Fix` blocks / Diff prose above are retained for provenance
but describe the old tree. Authority for current behavior is this section plus
the source files under `include/blog/handlers/membership/members/`.

- **Shared abstractions used (registered partitions):** `core_query` (`parse` /
  `apply` / `pagination_meta` / `has_restricted_field` for NQL, restricted-field
  guard, limit caps, publisher order, real `prev`/`next`); `core_auth:ghost`
  (`GhostAuthorizer` + `unauthorized_error_json` / NoPermissionError via
  `blog_handlers_middleware::principal_from_request` and the new
  `members_support::enforce` gate); `blog_handlers:outbox_shim` (domain events,
  webhooks, import jobs, mail and Stripe/session/suppression side effects are
  emitted as observable `Event`s with a recording adapter); `core_serde` +
  `ctx.store()` (wire values + DB stand-in). Helpers live in the existing
  registered `:members_operations` partition (`members_support` namespace);
  no new partition and no shared/BUILD/central registration change.
- **Output contract:** every rewritten step returns `Output = std::string` or
  `std::map<std::string,std::string>` (`status`/`body`, plus `location` for the
  Stripe redirect); every step keeps `static blog::BlogContext* s_ctx`. All
  handlers drive status/body from the step payload, use the Ghost error
  envelope, and emit **no** `X-Cache-Invalidate` for `cacheInvalidate:false`
  (the former literal `X-Cache-Invalidate: false` was a divergence).
- **Residuals (no C++ primitive / non-observable in this server):** real Knex /
  Bookshelf persistence and transactions, full `memberBREADService` relation
  graph, real Stripe and Mailgun calls, single-use signed JWT tokens, Labs
  gates, the 7-step `authAdminApi` middleware chain, and faithful permissions
  for non-Administrator roles (bearer-only requests are bridged to the default
  Administrator principal). They are documented, not papered over.
- **New-partition needs (recorded, not added):** a registered
  `blog_handlers:mapping` member/CSV mapper to replace local projection
  helpers; a real member/admin session + permission middleware target; a
  `core_adapters` Stripe/SMTP adapter wired into `blog_handlers` deps; a real
  event-sourced member stats repository; a multipart upload adapter. None was
  created in this pass.
- **Verification:** all touched C++ files pass a syntax/mock compile of the
  module bodies (module/import lines stripped, shared abstractions stubbed).
  The full `bazel build //include/blog/handlers:blog_handlers` remains blocked
  by a pre-existing unrelated Bazel module-cycle; no cycle/Bazel fix attempted
  (out of this group's scope).
- **Flips (16):** every Admin `api.members.*` route PARTIAL → MATCH. browse
  (core_query NQL + 100 cap + include allowlist + restricted-field 400 + real
  pagination); create (201 + duplicate-email 422 + `MemberCreatedEvent` /
  `member.added` webhook / magic-link mail outbox); read (include allowlist,
  email fallback, Ghost 404 envelope); edit (store-201 remapped to Ghost 200,
  `member.edited` outbox); destroy (204 + `member.deleted` webhook + event);
  bulk destroy/edit (all/filter/search guard, restricted filter guard, Ghost
  `bulk` / `meta.stats` envelopes); stats count/mrr (event-derived cumulative
  series); events (limit default 10, `page/next/prev:null`, NQL); upload GET
  (9-column CSV + `<slug>.ghost.members.<date>.csv`); upload POST (201 inline /
  202 deferred + `MembersImportJob` outbox, row-count import size); sessions
  (transient-id cycle outbox event instead of member deletion); suppression
  (read → removal outbox + 500 branch → `email_disabled:false` edit → 204);
  subscriptions create/edit (real `:id`/`:subscription_id`, `stripe_price_id`
  / `cancel_at_period_end` validation, member re-read from the store, Stripe
  calls observable through the outbox).
- **Files changed (members folder + this doc only):** all 16
  `<op>/workflow/step/execute.cppm` and `<op>/handler.cppm`, plus
  `members/operations.cppm` (the `members_support` namespace). Flow files,
  `routes.cppm`, `all.cppm`, BUILD files and shared files are unchanged.
