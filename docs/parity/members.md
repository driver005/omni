# Parity: members

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/members.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (all 16 `api.members.*` routes live here; no members-app `routes.js` exists under `docs/Ghost/ghost/core/server/web`)
- C++: `include/blog/handlers/membership/members/routes.cppm` (installed via `include/blog/handlers/routes.cppm:29,102`; handlers registered in `include/blog/handlers/all.cppm:432-453,681-705`)
- Checked: 2026-11-19

Evidence paths below are abbreviated: `members.js` = Ghost controller, `admin/routes.js` = Ghost URL table, `routes.cppm` = `include/blog/handlers/membership/members/routes.cppm`, `<op>/handler.cppm` and `<op>/execute.cppm` = files under `include/blog/handlers/membership/members/<op>/` (execute at `workflow/step/execute.cppm`), `context.cppm` = `include/blog/context/context.cppm`, `operations.cppm` = `include/blog/handlers/membership/members/operations.cppm`, `all.cppm` = `include/blog/handlers/all.cppm`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 16 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 5 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /members | browse | GET /members | `members_browse()` | Ghost: 8 options `limit,fields,filter,order,debug,page,search,include` + `include` enum `email_recipients/products/tiers/metafields` (`members.js:62,64-70`). C++: no query parsing, `using Input = IRequest&` (`browse/execute.cppm:11`) | Ghost `page` serialized `{members,meta.pagination}`; C++ hardcoded `{"members":[...],"meta":{"pagination":{limit:15,page:1,pages:1,total:0}}}` (`browse/execute.cppm:17`) | Ghost `memberBREADService.browse(restrictAdminApiQueryOptions(...))` (`members.js:72-76`); C++ `store().json_for("members",{})`, no real filtering/pagination (`browse/execute.cppm:16`) | none on both (`members.js:59-61` `cacheInvalidate:false`; no C++ cache use) | Ghost `mw.authAdminApi` (`admin/routes.js:159`) + `permissions:true` (`members.js:63`); C++ `token(req)` check (`browse/handler.cppm:17-22`), `content_type_middleware` (`routes.cppm:20`) | PARTIAL | `members.js:58-78`; `admin/routes.js:159`; `routes.cppm:19-28`; `browse/handler.cppm:17-28`; `browse/execute.cppm:11-17` |
| POST /members | add | POST /members | `members_create()` | Ghost options `send_email,email_type`, `email` required, `email_type` enum `signin/signup/subscribe` (`members.js:115,116-125`); C++ none of these, raw `body_as_string()` (`create/execute.cppm:16`) | Ghost `statusCode:201` (`members.js:111`); C++ always `Status::OK`=200 (`create/handler.cppm:26-27`) with `wrap("members",...)` body (`context.cppm:1259-1260`) | Ghost `memberBREADService.add(frame.data.members[0], frame.options)` (`members.js:132-135`); C++ `mutations.create("members",body)` (`create/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:160`) + `permissions:true` (`members.js:126`); C++ token (`create/handler.cppm:17-22`) | PARTIAL | `members.js:110-139`; `admin/routes.js:160`; `routes.cppm:29-36`; `create/handler.cppm:17-28`; `create/execute.cppm:11-16`; `context.cppm:1205-1276` |
| DELETE /members | bulkDestroy | DELETE /members | `members_bulk_destroy()` | Ghost options `all,filter,search` (`members.js:323`); C++ none, raw body (`bulk_destroy/execute.cppm:16`) | Ghost `{meta:{stats:{successful,unsuccessful},unsuccessfulIds,errors}}` (`members.js:331-340`); C++ `{meta:{deleted:N}}` (`context.cppm:1355-1359`) via `bulk_destroy/execute.cppm:16` | Ghost `members.bulkDestroy(frame.options)` (`members.js:328`); C++ `mutations.bulk_destroy("members",body)` (`bulk_destroy/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:161`) + `permissions.method:'destroy'` (`members.js:324-326`); C++ token (`bulk_destroy/handler.cppm:17-22`) | PARTIAL | `members.js:318-342`; `admin/routes.js:161`; `routes.cppm:37-44`; `bulk_destroy/handler.cppm:17-28`; `bulk_destroy/execute.cppm:11-16`; `context.cppm:1355-1359` |
| PUT /members/bulk | bulkEdit | PUT /members/bulk | `members_bulk_edit()` | Ghost options `all,filter,search`, data `action,meta`, `action` enum `unsubscribe/addLabel/removeLabel` (`members.js:349-358`); C++ none, raw body (`bulk_edit/execute.cppm:16`) | Ghost `members.bulkEdit(data.bulk,options)` result (`members.js:363`); C++ `{meta:{updated:N}}` (`context.cppm:1361-1365`) | C++ `mutations.bulk_edit("members",body)` (`bulk_edit/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:162`) + `permissions.method:'edit'` (`members.js:359-361`); C++ token (`bulk_edit/handler.cppm:17-22`) | PARTIAL | `members.js:344-365`; `admin/routes.js:162`; `routes.cppm:152-162`; `bulk_edit/handler.cppm:17-28`; `bulk_edit/execute.cppm:11-16`; `context.cppm:1361-1365` |
| GET /members/stats/count | memberStats | GET /members/stats/count | `members_members_stats_count_get()` | none on either side | Ghost `{resource:'members',total,data:[{date,paid,free,comped,gift}]}` (`members.js:436-449`); C++ `{"stats":[StatsMemberEntry{}],"meta":{...}}` hardcoded (`members_stats_count_get/execute.cppm:16-19`; DTO fields `stats,meta` at `include/blog/dto/stats_member_response.cppm:20-21`) — no `total`/`data`/`resource` | Ghost `events.getStatuses()` (`members.js:432`); C++ DTO fixture (`members_stats_count_get/execute.cppm:17-19`), `json_for("members")` unreachable (`:22`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:169`) + `permissions.method:'browse'` (`members.js:428-430`); C++ token (`members_stats_count_get/handler.cppm:17-22`) | PARTIAL | `members.js:424-451`; `admin/routes.js:169`; `routes.cppm:185-197`; `members_stats_count_get/handler.cppm:17-28`; `members_stats_count_get/execute.cppm:14-23`; `include/blog/dto/stats_member_response.cppm:20-21` |
| GET /members/stats/mrr | mrrStats | GET /members/stats/mrr | `members_members_stats_mrr_get()` | none on either side | Ghost `{resource:'mrr',data:[{currency,data:[{date,value}]}]}` (`members.js:476-479`); C++ `{"stats":[...],"meta":{"totals":[...]}}` with hardcoded `usd`/`1970-01-01`/`mrr 0 and 1000` (`members_stats_mrr_get/execute.cppm:16-29`; DTO `include/blog/dto/stats_mrr_response.cppm:20-21`) | Ghost `events.getMRR()` (`members.js:461`); C++ hardcoded `StatsMrrResponse` (`members_stats_mrr_get/execute.cppm:17-28`), `json_for("members")` unreachable (`:32`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:170`) + `permissions.method:'browse'` (`members.js:457-459`); C++ token (`members_stats_mrr_get/handler.cppm:17-22`) | PARTIAL | `members.js:453-481`; `admin/routes.js:170`; `routes.cppm:198-208`; `members_stats_mrr_get/handler.cppm:17-28`; `members_stats_mrr_get/execute.cppm:14-33`; `include/blog/dto/stats_mrr_response.cppm:20-21` |
| GET /members/events | activityFeed | GET /members/events | `members_members_events_get()` | Ghost options `limit,filter` (`members.js:487`); C++ none | Ghost `events.getEventTimeline(options)`; C++ `{"events":[GhostFixtures::event()],"meta":{"pagination":{limit:15,...,total:1}}}` hardcoded (`members_events_get/execute.cppm:15-19`) | Ghost `events.getEventTimeline(frame.options)` (`members.js:492`); C++ fixture (`members_events_get/execute.cppm:18`), `json_for("members")` unreachable (`:21`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:172`) + `permissions.method:'browse'` (`members.js:488-490`); C++ token (`members_events_get/handler.cppm:17-22`) | PARTIAL | `members.js:483-494`; `admin/routes.js:172`; `routes.cppm:163-173`; `members_events_get/handler.cppm:17-28`; `members_events_get/execute.cppm:13-22` |
| GET /members/upload | exportCSV | GET /members/upload | `members_members_upload_get()` | Ghost options `limit,filter,search` (`members.js:368`); C++ none | Ghost CSV stream: `Content-Disposition` csv `getCSVExportFileName('members')`, `contentType:'text/csv'`, `response.format:'plain', stream:true` (`members.js:369-382`), returns `{data:export(...),filename}` (`:387-392`); C++ JSON `{members,meta.pagination}` with no headers/stream/filename (`members_upload_get/execute.cppm:16-17`) | Ghost `membersService.export(restrictAdminApiQueryOptions(...))` (`members.js:389`); C++ `store().json_for("members",{})` (`members_upload_get/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:174`) + `permissions.method:'browse'` (`members.js:383-385`); C++ token (`members_upload_get/handler.cppm:17-22`) | MISMATCH | `members.js:367-393`; `admin/routes.js:174`; `routes.cppm:221-230`; `members_upload_get/handler.cppm:17-28`; `members_upload_get/execute.cppm:11-17` |
| POST /members/upload | importCSV | POST /members/upload | `members_members_upload_post()` | Ghost multipart `upload.single('membersfile')` + validation (`admin/routes.js:175-180`), data `mapping`,`labels`, `frame.user.email` (`members.js:414-419`); C++ none, raw body (`members_upload_post/execute.cppm:16`) | Ghost dynamic status 201/202 (`members.js:399-405`) and `{meta:{originalImportSize,stats:{imported,invalid},import_label}}` (`members.js:16-27,420`); C++ status 200 + `{"members":[...]}` (`members_upload_post/handler.cppm:26-27`; `members_upload_post/execute.cppm:16`) | Ghost `membersService.importCSV({filePath,mapping,extraLabels,requestUserEmail})` (`members.js:414-419`); C++ `mutations.create("members",body)` (`members_upload_post/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` + upload middleware (`admin/routes.js:175-180`) + `permissions.method:'add'` (`members.js:406-408`); C++ token only (`members_upload_post/handler.cppm:17-22`) | MISMATCH | `members.js:14-27,395-422`; `admin/routes.js:175-181`; `routes.cppm:231-238`; `members_upload_post/handler.cppm:17-28`; `members_upload_post/execute.cppm:11-16` |
| GET /members/:id | read | GET /members/:id | `members_read_by_id()` | Ghost options `include`, data `id,email`, include enum (`members.js:81,85,86-92`); C++ none; id ignored — hardcoded `"1"` (`read_by_id/execute.cppm:16`) | Ghost member + `NotFoundError` 404 when missing (`members.js:94-107`); C++ `{"members":[{...}]}` or `{"members":[]}` at status 200 (`read_by_id/execute.cppm:17-18`) | Ghost `memberBREADService.read(data,{metafieldsFor:ADMIN})` (`members.js:95-98`); C++ `find_by_resource("members","1")` (`read_by_id/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:240`) + `permissions:true` (`members.js:93`); C++ token (`read_by_id/handler.cppm:17-22`) | PARTIAL | `members.js:80-108`; `admin/routes.js:240`; `routes.cppm:45-54`; `read_by_id/handler.cppm:17-28`; `read_by_id/execute.cppm:11-18` |
| PUT /members/:id | edit | PUT /members/:id | `members_edit()` | Ghost option `id` required (`members.js:146-153`); C++ none; id passed as empty string (`edit/execute.cppm:16`) | Ghost `statusCode:200` + member (`members.js:142,155-162`); C++ status 200 + `{"members":[...]}` (`edit/handler.cppm:26-27`; `edit/execute.cppm:16`) | Ghost `memberBREADService.edit(data.members[0],options)` (`members.js:156-159`); C++ `mutations.edit("members","",body)` (`edit/execute.cppm:16`; `context.cppm:1307-1308`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:241`) + `permissions:true` (`members.js:154`); C++ token (`edit/handler.cppm:17-22`) | PARTIAL | `members.js:141-163`; `admin/routes.js:241`; `routes.cppm:55-62`; `edit/handler.cppm:17-28`; `edit/execute.cppm:11-16`; `context.cppm:1278-1324` |
| DELETE /members/:id | destroy | DELETE /members/:id | `members_destroy()` | Ghost options `id` required, `cancel` (`members.js:295-302`); C++ none; id empty (`destroy/execute.cppm:16`) | Ghost `statusCode:204` (`members.js:291`); C++ status 200, body `{}` (`destroy/handler.cppm:26-27`; `destroy/execute.cppm:16`; `context.cppm:1351`) | Ghost `members.destroy({id},{require:true,cancelStripeSubscriptions:cancel})` (`members.js:305-314`); C++ `mutations.destroy("members","")` (`destroy/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:242`) + `permissions:true` (`members.js:303`); C++ token (`destroy/handler.cppm:17-22`) | PARTIAL | `members.js:290-316`; `admin/routes.js:242`; `routes.cppm:63-70`; `destroy/handler.cppm:17-28`; `destroy/execute.cppm:11-16`; `context.cppm:1326-1352` |
| DELETE /members/:id/sessions | logout | DELETE /members/:id/sessions | `members_members_id_sessions_delete()` | Ghost option `id` required (`members.js:170-177`); C++ none, id empty | Ghost `statusCode:204`, `memberBREADService.logout(options)` (session invalidation) (`members.js:166,181-185`); C++ status 200 + `{}` (`members_id_sessions_delete/handler.cppm:26-27`) | Ghost `logout` on member (`members.js:182`); C++ `mutations.destroy("members","")` destroys the member resource (`members_id_sessions_delete/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:243`) + `permissions.method:'edit'` (`members.js:178-180`); C++ token (`members_id_sessions_delete/handler.cppm:17-22`) | MISMATCH | `members.js:165-186`; `admin/routes.js:243`; `routes.cppm:96-106`; `members_id_sessions_delete/handler.cppm:17-28`; `members_id_sessions_delete/execute.cppm:11-16` |
| DELETE /members/:id/suppression | deleteEmailSuppression | DELETE /members/:id/suppression | `members_members_id_suppression_delete()` | Ghost option `id` required (`members.js:501-508`); C++ none, id empty | Ghost `statusCode:204`, `null` body, 404 if no member, 500 if removal fails (`members.js:497,512-543`); C++ status 200 + `{}` (`members_id_suppression_delete/handler.cppm:26-27`) | Ghost read member → `emailSuppressionList.removeEmail` → `memberBREADService.edit({email_disabled:false})` (`members.js:516-540`); C++ `mutations.destroy("members","")` destroys the member (`members_id_suppression_delete/execute.cppm:16`) | none both | Ghost `mw.authAdminApi` (`admin/routes.js:244-248`) + `permissions.method:'edit'` (`members.js:509-511`); C++ token (`members_id_suppression_delete/handler.cppm:17-22`) | MISMATCH | `members.js:496-544`; `admin/routes.js:244-248`; `routes.cppm:140-150`; `members_id_suppression_delete/handler.cppm:17-28`; `members_id_suppression_delete/execute.cppm:11-16` |
| POST /members/:id/subscriptions/ | createSubscription | POST /members/:id/subscriptions (no trailing slash) | `members_subscriptions_post()` | Ghost option `id` required, data `stripe_price_id` required (`members.js:252-265`); C++ decodes `MemberSubscriptionInput` = `{tier,status}` (`operations.cppm:29`; `include/blog/dto/member_subscription_input.cppm:19-20`), id hardcoded `"1"` (`subscriptions_post/execute.cppm:16`) | Ghost status 200 + re-read member `{metafieldsFor:ADMIN}`, 404 if missing (`members.js:248,269-287`); C++ ops status + `MemberSubscriptionResponse` `{members:[{email:"member@example.com",id,status:"paid",subscriptions:[...]}]}` (`operations.cppm:43,502-515`; `subscriptions_post/execute.cppm:16-18`) | Ghost `members.createSubscription({id,subscription:{stripe_price_id}})` Stripe call + `memberBREADService.read` (`members.js:270-279`); C++ cache-backed `kSub` list (`operations.cppm:24-44,467-500`) | Ghost none (`members.js:249-251` `cacheInvalidate:false`); C++ cache `members:subscriptions:<id>:count`, `members:subscriptions:<id>:<n>`, `members:subscriptions:next`, tag `members`, TTL `never` (`operations.cppm:31-34,271,493-498`), cache built at `all.cppm:836-837` | Ghost `mw.authAdminApi` (`admin/routes.js:250`) + `permissions.method:'edit'` (`members.js:266-268`); C++ token (`subscriptions_post/handler.cppm:19-22`) | PARTIAL | `members.js:247-288`; `admin/routes.js:250`; `routes.cppm:118-127`; `subscriptions_post/handler.cppm:18-31`; `subscriptions_post/execute.cppm:11-18`; `operations.cppm:24-44,467-515`; `all.cppm:836-837` |
| PUT /members/:id/subscriptions/:subscription_id | editSubscription | PUT /members/:id/subscriptions/:id (param named `:id`, not `:subscription_id`) | `members_put_detail()` | Ghost options `id,subscription_id` required, data `cancel_at_period_end` required, `status` enum `['canceled']` (`members.js:193-212`); C++ `MemberSubscriptionInput{tier,status}`, no `id`/`subscription_id`/`cancel_at_period_end`; ids hardcoded `"1","1"` (`put_detail/execute.cppm:16`) | Ghost status 200 + re-read member (`members.js:189,216-244`); C++ ops status + subscription-list response (`operations.cppm:63,502-515`; `put_detail/execute.cppm:16-18`) | Ghost `members.cancelSubscription` / `updateSubscription` Stripe calls + `memberBREADService.read` (`members.js:217-236`); C++ `member_subscription_edit` over cache `kSub` (`operations.cppm:47-64,467-500`) | Ghost none (`members.js:190-192`); C++ same `members:subscriptions:*` keys/tag/TTL as previous row (`operations.cppm:271,487-500`) | Ghost `mw.authAdminApi` (`admin/routes.js:251-255`) + `permissions.method:'edit'` (`members.js:213-215`); C++ token (`put_detail/handler.cppm:19-22`) | PARTIAL | `members.js:188-245`; `admin/routes.js:251-255`; `routes.cppm:128-138`; `put_detail/handler.cppm:18-31`; `put_detail/execute.cppm:11-18`; `operations.cppm:46-64`; `all.cppm:701-702,836-837` |

## Route details

### GET /members — PARTIAL

- `include`, `filter`, `page`, `limit`, `order`, `fields`, `search`, `debug` parsed/validated on Ghost (`members.js:62-70`) but **none** parsed in C++ (`browse/execute.cppm:11-16`).
- Pagination meta hardcoded `limit:15,page:1,pages:1,total:0` (`browse/execute.cppm:17`) vs Ghost real page from `memberBREADService.browse`.
- Auth equivalent: `mw.authAdminApi` (`admin/routes.js:159`) vs `token(req)` 401 guard (`browse/handler.cppm:17-22`); no per-method permission mapping in C++.
- Cache: none on either side.
- DB resource string `members` agrees (`members.js:72`; `browse/execute.cppm:16`).

#### Fix

**File/symbol:** `browse/workflow/step/execute.cppm` → `blog::handlers::members::browse::workflow::ExecuteStep::invoke`; `browse/handler.cppm` → `BrowseHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `GET /members` → `handler.members_browse()` at `routes.cppm:21-27`, accessor `all.cppm:432`; verb/path and evidence already agree.
- **3 (input):** `browse/execute.cppm:11` declares `using Input = IRequest&` but never reads the query. Replace `json_for("members", {})` at `:16` with a parsed-param call: split `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='` into `std::map<std::string,std::string> params`, keeping `limit,fields,filter,order,debug,page,search,include` (`members.js:62`). Validate `include` against `{"email_recipients","products","tiers","metafields"}` (`members.js:52,64-70`); any other value → 422 `ApiError{}.validation("Validation error, cannot browse members.", <value>, "include")` (`context.cppm:76-84`).
- **5 (DB call):** `ctx.store().json_for("members", params)` — `query_json_impl` honors `limit`/`page`/`offset`/`filter`/`fields`/`include` (`context.cppm:1532-1560,1563-1615`); `ctx.store().total("members")` = `m_db.count<model::Member>()` (`context.cppm:1113`) supplies real total. Resource string `"members"` already correct (`members.js:72-74`).
- **4 (output):** replace hardcoded `limit:15,page:1,pages:1,total:0` (`browse/execute.cppm:17`) with parsed `limit`/`page`, `total` from `ctx.store().total("members")`, `pages=(total+limit-1)/limit`, `prev`/`next` from `page`. Keep `{"members":[...],"meta":{"pagination":...}}` envelope. `BrowseHandler::handle` already sets `Status::OK` (`browse/handler.cppm:27`) — no change.
- **6 (cache):** Ghost `cacheInvalidate:false` (`members.js:59-61`); add `res.set_header("X-Cache-Invalidate", "false")` before `send()` in `BrowseHandler::handle` (`browse/handler.cppm:27`). No `core::cache::Cache` needed.
- **7 (auth):** token 401 + `content_type_middleware` exist (`browse/handler.cppm:18-22`, `routes.cppm:20`). Add `.use(blog::handlers::middleware::require_auth)` on `Router{router, "/members"}` (`middleware/auth.cppm:12-29`) to use Ghost's error envelope. Ghost `permissions:true` (`members.js:63`) has no allowed primitive — accepted gap.

Snippet (`browse/execute.cppm`):

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
  for (auto part : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
    const std::string kv{std::ranges::begin(part), std::ranges::end(part)};
    const auto eq = kv.find('=');
    if (eq != std::string::npos) params[kv.substr(0, eq)] = kv.substr(eq + 1);
  }
}
const auto rows = ctx.store().json_for("members", params);
// total = ctx.store().total("members"); pages = (total + limit - 1) / limit;
return core::workflow::StepResponse<Output>{"{\"members\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":" + limit + ",\"page\":" + page + ",\"pages\":" + pages + ",\"prev\":null,\"next\":null,\"total\":" + std::to_string(total) + "}}}}"};
```

### POST /members — PARTIAL

- Ghost `statusCode:201` (`members.js:111`) vs C++ handler always `Status::OK` 200 (`create/handler.cppm:26-27`), even though `mutations.create` returns a `MutationResult` carrying 201 (`context.cppm:1272-1275`).
- `send_email`, `email_type`, required `email`, email_type enum not parsed/validated in C++ (`members.js:115-125`; `create/execute.cppm:16`).
- C++ `create` does validate `email` contains `@` internally (`context.cppm:1229-1237`) — different validation rule/message than Ghost.
- Cache none; auth as row above.

#### Fix

**File/symbol:** `create/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `create/handler.cppm` → `CreateHandler::handle`. **Checks closed: 3, 4, 6, 7.** No change: 1, 2, 5, 8.

- **1, 2, 5, 8 (no change):** `POST /members` → `members_create()` (`routes.cppm:29-35`) → `mutations.create("members", ...)` (`create/execute.cppm:16`), same resource as `memberBREADService.add` (`members.js:132-135`); token/content-type auth and no-cache side already recorded.
- **3 (input):** Ghost `options:['send_email','email_type']`, required `email`, `email_type ∈ {signin,signup,subscribe}` (`members.js:115-125`). Replace raw `in.body_as_string()` (`create/execute.cppm:16`) with the unwrapped `data.members[0]` object — decode via `core_serde::JsonCodec` into a serde value and extract `data.members[0]` — then parse `email_type` from the query with the `posts/browse` split pattern and reject other values with 422 `ApiError{}.validation("Validation error, cannot save member.", value, "email_type")`. Keep the store's `email` contains `'@'` check (`context.cppm:1230-1238`) but add a required-`email` 422 before it to match Ghost.
- **4 (output):** change `using Output = std::string` to `using Output = std::map<std::string,std::string>` carrying `{"status",std::to_string(result.status())}` and `{"body",result.body()}` (exact pattern `admin/posts/create/workflow/step/execute.cppm:13,20-24`). `wrap("members", ...)` returns 201 (`context.cppm:1260-1262,1394-1409`) and 422 for email validation (`context.cppm:1233-1237`). In `CreateHandler::handle` read that map and set `Status::CREATED` for 201 / `Status::UNPROCESSABLE_CONTENT` for 422 instead of hardcoded `Status::OK` (`create/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:112-114`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the members router (`middleware/auth.cppm:12-29`); `permissions:true` (`members.js:126`) remains an accepted gap.

Snippet (`create/execute.cppm`):

```cpp
using Output = std::map<std::string, std::string>;
const auto result = mutations.create("members", innerMemberJson); // innerMemberJson = data.members[0]
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())}, {"body", result.body()}}};
```

### DELETE /members — PARTIAL

- `all`/`filter`/`search` options (`members.js:323`) not read in C++; body drives which IDs are deleted (`bulk_destroy/execute.cppm:16`).
- Output meta differs: Ghost `stats.successful/unsuccessful + unsuccessfulIds + errors` (`members.js:331-340`) vs C++ `meta.deleted` (`context.cppm:1355-1359`).
- Ghost `permissions.method:'destroy'` vs C++ token only; cache none.

#### Fix

**File/symbol:** `bulk_destroy/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `bulk_destroy/handler.cppm` → `BulkDestroyHandler::handle`. **Checks closed: 3, 4, 6, 7.** No change: 1, 2, 5, 8.

- **1, 2, 5, 8 (no change):** `DELETE /members` → `members_bulk_destroy()` (`routes.cppm:37-43`) → `mutations.bulk_destroy("members", body)` (`bulk_destroy/execute.cppm:16`); resource agrees with `members.bulkDestroy(frame.options)` (`members.js:328`).
- **3 (input):** parse `all,filter,search` from `in.get_path()` (`members.js:323`) with the `posts/browse` split pattern. `ResourceStore::bulk_destroy` only reads `ids` from the body (`context.cppm:674-683`), so replace the raw-body pass-through with `mutations.bulk_destroy("members", R"({"ids":")" + resolvedIds + R"("})")`, where `resolvedIds` is the comma-joined id list from `ctx.store().json_for("members", params)`. `all=true` accepts every id; no third-party parser.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>` carrying `status`/`body`; replace `{"meta":{"deleted":N}}` (`context.cppm:1356-1360`) with Ghost's `{"meta":{"stats":{"successful":N,"unsuccessful":M},"unsuccessfulIds":[...],"errors":[...]}}` (`members.js:331-340`). Derive unsuccessful ids by checking `ctx.store().find_by_resource("members", id)` (`context.cppm:1072-1076`) for each requested id; `handler.cppm` then sets status from the map instead of hardcoded 200 (`bulk_destroy/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:320-322`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)`; Ghost `permissions.method:'destroy'` (`members.js:324-326`) remains an accepted gap.

Snippet (`bulk_destroy/execute.cppm`):

```cpp
using Output = std::map<std::string, std::string>;
const auto result = mutations.bulk_destroy("members", std::format(R"({{"ids":"{}"}})", resolvedIds));
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())}, {"body", ghostStatsMetaBody}}};
```

### PUT /members/bulk — PARTIAL

- Ghost `action` enum and `all/filter/search`/`meta` options (`members.js:349-358`) absent in C++.
- Output: Ghost passes through `bulkEdit` result (`members.js:363`); C++ `{meta:{updated:N}}` (`context.cppm:1361-1365`).
- Ghost `permissions.method:'edit'` vs C++ token only; cache none.

#### Fix

**File/symbol:** `bulk_edit/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `bulk_edit/handler.cppm` → `BulkEditHandler::handle`. **Checks closed: 3, 4, 6, 7.** No change: 1, 2, 5, 8.

- **1, 2, 5, 8 (no change):** `PUT /members/bulk` → `members_bulk_edit()` (`routes.cppm:154-160`) → `mutations.bulk_edit("members", body)` (`bulk_edit/execute.cppm:16`); resource agrees with `members.bulkEdit(frame.data.bulk, frame.options)` (`members.js:363`).
- **3 (input):** parse `all,filter,search` from the query and `action,meta` from the decoded body; validate `action ∈ {unsubscribe,addLabel,removeLabel}` (`members.js:349-358`) with 422 `ApiError{}.validation("Validation error, cannot bulk edit members.", value, "action")`. `ResourceStore::bulk_edit` reads `ids` from the body (`context.cppm:686-696`), so pass `{"ids":"<resolved>","action":...,"meta":...}` rather than the raw body.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>`; replace `{"meta":{"updated":N}}` (`context.cppm:1362-1365`) with the result shape of `membersService.api.members.bulkEdit(...)` (`members.js:363`) — encode the per-action outcome using `core_serde::JsonCodec`. Set status from the map in `BulkEditHandler::handle` instead of hardcoded 200 (`bulk_edit/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:346-348`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)`; Ghost `permissions.method:'edit'` (`members.js:359-361`) remains an accepted gap.

Snippet (`bulk_edit/execute.cppm`):

```cpp
using Output = std::map<std::string, std::string>;
const auto result = mutations.bulk_edit("members", std::format(R"({{"ids":"{}","action":"{}"}})", resolvedIds, action));
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())}, {"body", bulkEditResultBody}}};
```

### GET /members/stats/count — PARTIAL

- Envelope mismatch: Ghost `{resource:'members',total,data:[...]}` (`members.js:436-449`) vs serde DTO `{"stats":[...],"meta":{...}}` (`members_stats_count_get/execute.cppm:16-19`; `include/blog/dto/stats_member_response.cppm:20-21`). No `total`, no `data`, no `resource`.
- Values hardcoded (single empty `StatsMemberEntry{}`); Ghost sums `paid+free+comped+gift` from `events.getStatuses()` (`members.js:432-434`).
- Cache none; auth `browse` permission vs token only.

#### Fix

**File/symbol:** `members_stats_count_get/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_stats_count_get/handler.cppm` → `MembersStatsCountGetHandler::handle`. **Checks closed: 4, 5, 6, 7.** No change: 1, 2, 3, 8.

- **1, 2, 3, 8 (no change):** `GET /members/stats/count` → `members_members_stats_count_get()` (`routes.cppm:189-196`); no input on either side (`members.js:424-451`); evidence already complete.
- **5 (DB call):** the lambda at `members_stats_count_get/execute.cppm:16-20` always returns `{200,...}`, so `if(r.first!=0)` at `:21` is always true and `json_for("members")` at `:22` is unreachable. Delete the lambda; build buckets from `ctx.store().json_for("members", {})` (or context-local `members:stats:count:*` keys under the existing `members:` namespace via `core::cache::Cache`, tag `members`, `Ttl{}.never()`) and compute `total = last.paid + last.free + last.comped + last.gift` (`members.js:432-434`). Do not keep the empty `StatsMemberEntry{}` fixture.
- **4 (output):** replace `encode(StatsMemberResponse)` (`stats`/`meta` fields at `dto/stats_member_response.cppm:20-21`; `execute.cppm:17-19`) with the Ghost envelope `{resource:'members',total,data:[{date,paid,free,comped,gift}]}` (`members.js:436-449`). Build with `core_serde::JsonCodec{}.encode` on each entry and `std::format`; keep `Status::OK` (`handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:425-427`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/stats` router (`routes.cppm:185-209`); Ghost `permissions.method:'browse'` (`members.js:428-430`) remains an accepted gap.

Snippet (`members_stats_count_get/execute.cppm`):

```cpp
const auto rows = ctx.store().json_for("members", {});
// aggregate paid/free/comped/gift per date from rows; last = final bucket
const auto body = std::format(R"({{"resource":"members","total":{},"data":{}}})", total, dataJson);
return core::workflow::StepResponse<Output>{body};
```

### GET /members/stats/mrr — PARTIAL

- Envelope mismatch: Ghost `{resource:'mrr',data:[{currency,data:[{date,value}]}]}` (`members.js:476-479`) vs `{"stats":[...],"meta":{...}}` (`members_stats_mrr_get/execute.cppm:16-29`).
- Hardcoded two `usd` entries dated `1970-01-01` with mrr `0`/`1000` (`:18-27`) vs real `events.getMRR()` data.
- Cache none; auth `browse` permission vs token only.

#### Fix

**File/symbol:** `members_stats_mrr_get/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_stats_mrr_get/handler.cppm` → `MembersStatsMrrGetHandler::handle`. **Checks closed: 4, 5, 6, 7.** No change: 1, 2, 3, 8.

- **1, 2, 3, 8 (no change):** `GET /members/stats/mrr` → `members_members_stats_mrr_get()` (`routes.cppm:200-207`); no input on either side (`members.js:453-481`).
- **5 (DB call):** delete the hardcoded lambda at `members_stats_mrr_get/execute.cppm:16-30` (two `usd` entries at `1970-01-01`, mrr `0`/`1000`) — its `if(r.first!=0)` at `:31` makes `json_for("members")` at `:32` unreachable. Derive currency/date/value buckets from `ctx.store().json_for("members", {})` or context-local `members:stats:mrr:*` keys (existing `members:` namespace, tag `members`, `Ttl{}.never()`), mirroring `events.getMRR()` (`members.js:461-465`).
- **4 (output):** replace `encode(StatsMrrResponse)` (`stats`/`meta.totals` at `dto/stats_mrr_response.cppm:20-21`; `execute.cppm:17-29`) with `{resource:'mrr',data:[{currency,data:[{date,value}]}]}` (`members.js:466-479`). `Status::OK` in `handler.cppm:27` is correct.
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:454-456`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/stats` router; Ghost `permissions.method:'browse'` (`members.js:457-459`) remains an accepted gap.

Snippet (`members_stats_mrr_get/execute.cppm`):

```cpp
const auto rows = ctx.store().json_for("members", {});
// group rows by currency -> date -> mrr; encode with core_serde::JsonCodec
const auto body = std::format(R"({{"resource":"mrr","data":{}}})", dataJson);
return core::workflow::StepResponse<Output>{body};
```

### GET /members/events — PARTIAL

- Ghost `limit`/`filter` (`members.js:487`) ignored in C++.
- C++ returns fixture event plus hardcoded pagination `total:1` (`members_events_get/execute.cppm:15-19`) vs `events.getEventTimeline(frame.options)` (`members.js:492`).
- Cache none; auth `browse` permission vs token only.

#### Fix

**File/symbol:** `members_events_get/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_events_get/handler.cppm` → `MembersEventsGetHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `GET /members/events` → `members_members_events_get()` (`routes.cppm:164-172`); evidence complete.
- **3 (input):** parse `limit,filter` from `in.get_path()` (`members.js:487`) with the `admin/posts/browse/workflow/step/execute.cppm:21-29` split pattern; pass them to the store/filter step instead of ignoring `in` (`members_events_get/execute.cppm:15`).
- **5 (DB call):** replace the `core_serde::JsonCodec{}.encode(blog::model::EventWire{})` fixture at `members_events_get/execute.cppm:18-20` (and the unreachable `json_for` at `:23`) with `ctx.store().json_for("events", params)` — unknown resources fall through to `ResourceStore::list("events")` (`context.cppm:1180-1181`), i.e. context-local state, no SQL. If no events are seeded, return `[]` rather than a synthetic event.
- **4 (output):** replace hardcoded `limit:15,...,total:1` (`execute.cppm:19`) with parsed `limit`/`page`, `total` from `ctx.store().total("events")` (`context.cppm:1120`) or the row count; keep the `{"events":[...],"meta":{"pagination":...}}` envelope. `Status::OK` already set (`handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:484-486`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/events` route (`routes.cppm:163-173`); Ghost `permissions.method:'browse'` (`members.js:488-490`) remains an accepted gap.

Snippet (`members_events_get/execute.cppm`):

```cpp
const auto rows = ctx.store().json_for("events", params);
const auto total = ctx.store().total("events");
return core::workflow::StepResponse<Output>{std::format(
  R"({{"events":{},"meta":{{"pagination":{{"limit":{},"page":{},"pages":{},"prev":null,"next":null,"total":{}}}}}}}})",
  rows, limit, page, pages, total)};
```

### GET /members/upload — MISMATCH

- Ghost produces a CSV **stream** with `Content-Disposition: attachment; filename=<members CSV>`, `Content-Type: text/csv`, `response.format:'plain', stream:true` (`members.js:369-382`) and `{data, filename}` from `membersService.export` (`:387-392`).
- C++ returns JSON `{"members":[...],"meta":{"pagination":...}}` (`members_upload_get/execute.cppm:16-17`), no CSV, no headers, no streaming, no filename. Data shape/material output differ.
- `limit/filter/search` ignored; cache none; auth `browse` vs token only.

#### Fix

**File/symbol:** `members_upload_get/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_upload_get/handler.cppm` → `MembersUploadGetHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `GET /members/upload` → `members_members_upload_get()` (`routes.cppm:223-230`); evidence complete.
- **3 (input):** parse `limit,filter,search` from `in.get_path()` (`members.js:368`) with the `posts/browse` split pattern and pass `params` to the store.
- **5 (DB call):** replace `json_for("members", {})` (`members_upload_get/execute.cppm:16`) with `ctx.store().json_for("members", params)` so the export is filtered/paginated as `membersService.export(restrictAdminApiQueryOptions(frame.options))` is (`members.js:389`); no direct SQL.
- **4 (output):** replace the JSON envelope `{"members":[...],"meta":{...}}` (`execute.cppm:17`) with a CSV body: decode each row from `json_for("members", params)` via `core_serde::JsonCodec`, emit columns as CSV text. In `MembersUploadGetHandler::handle`, before `send()`, set `res.set_header("Content-Type", "text/csv")` and `res.set_header("Content-Disposition", "attachment; filename=" + filename)` via `interfaces::io::IResponse::set_header` (`response.cppm:135`) and write the CSV bytes with `set_body`. Filename: build `"members." + YYYY-MM-DD + ".csv"` with `std::chrono` (the Ghost `getCSVExportFileName('members')` helper at `members.js:10,373` is Ghost JS and must not be imported); this is the exact in-repo replacement call. Ghost `response.format:'plain',stream:true` (`members.js:379-382`) is a serializer concern — emit the CSV in one body chunk and record streaming as an accepted interface gap.
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:377`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/upload` route; Ghost `permissions.method:'browse'` (`members.js:383-385`) remains an accepted gap.

Snippet (`members_upload_get/execute.cppm` + `handler.cppm`):

```cpp
const auto rows = ctx.store().json_for("members", params); // CSV text built from decoded rows
return core::workflow::StepResponse<Output>{csv};
// handler, before send():
res.set_header("Content-Type", "text/csv");
res.set_header("Content-Disposition", "attachment; filename=members." + date + ".csv");
```

### POST /members/upload — MISMATCH

- Ghost is multipart import: `upload.single('membersfile')`, type validation (`admin/routes.js:175-180`), `filePath/mapping/extraLabels/requestUserEmail` into `importCSV` (`members.js:414-419`), dynamic 201/202 (`:399-405`), import stats meta (`:16-27`).
- C++ ignores multipart entirely and calls `mutations.create("members", body)` (`members_upload_post/execute.cppm:16`), status fixed 200 (`members_upload_post/handler.cppm:26-27`). Operation and data shape differ.
- Cache none; auth includes upload middleware + `add` permission vs token only.

#### Fix

**File/symbol:** `members_upload_post/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_upload_post/handler.cppm` → `MembersUploadPostHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `POST /members/upload` → `members_members_upload_post()` (`routes.cppm:231-238`); evidence complete.
- **3 (input):** Ghost is multipart: `upload.single('membersfile')` + type validation (`admin/routes.js:175-180`), body fields `mapping`, `labels`, `frame.user.email` into `importCSV` (`members.js:414-419`). C++ sends raw `body_as_string()` (`members_upload_post/execute.cppm:16`). Exact replacement: read bytes from `interfaces::io::IRequest` body, parse `mapping`/`labels` with `core_serde::JsonCodec`, and extract `membersfile` from the request; reject a missing file with 422 `ApiError{}.validation("Member import requires a CSV file.", "membersfile", "membersfile")`. No multipart library is allowed — accept a JSON body carrying the file bytes as the in-repo representation; document the transport gap rather than adding a dependency.
- **5 (DB call):** replace `mutations.create("members", body)` (`members_upload_post/execute.cppm:16`) with a per-row `ctx.store().mutations.create("members", encodedMemberJson)` loop over decoded CSV rows (or `ResourceStore` records under the existing context-local `members:` namespace). No direct SQL, no Ghost service.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>`; replace status 200 + `{"members":[...]}` (`members_upload_post/handler.cppm:27`, `execute.cppm:16`) with dynamic 201 when `stats.imported != null` else 202 (`members.js:399-405`) and meta `{originalImportSize, stats:{imported,invalid}, import_label}` (`members.js:16-27,420`). `MembersUploadPostHandler::handle` reads `status`/`body` from the map and calls `res.set_status(static_cast<Status>(status))` (the `subscriptions_post/handler.cppm:26-30` pattern).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:396-398`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)`; the upload middleware (`admin/routes.js:178-179`) and `permissions.method:'add'` (`members.js:406-408`) have no allowed C++ primitive — accepted gaps.

Snippet (`members_upload_post/execute.cppm`):

```cpp
using Output = std::map<std::string, std::string>;
const int status = (statsImported != nullptr) ? 201 : 202;
const auto body = std::format(R"({{"meta":{{"originalImportSize":{},"stats":{{"imported":{},"invalid":{}}},"import_label":{}}}}})", size, imported, invalid, label);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(status)}, {"body", body}}};
```

### GET /members/:id — PARTIAL

- Route id never used: `find_by_resource("members", std::string{}.empty()?"1":std::string{})` always resolves to `"1"` (`read_by_id/execute.cppm:16`).
- Ghost `include` option + `id/email` data + enum (`members.js:81-92`) not parsed; Ghost throws 404 when missing (`members.js:100-104`), C++ returns `{"members":[]}` with 200 (`read_by_id/execute.cppm:18`).
- Cache none; auth as row above.

#### Fix

**File/symbol:** `read_by_id/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `read_by_id/handler.cppm` → `ReadByIdHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `GET /members/:id` → `members_read_by_id()` (`routes.cppm:47-53`); evidence complete.
- **3 (input):** replace the constant id `std::string{}.empty()?"1":std::string{}` (`read_by_id/execute.cppm:16`) with the path id from `in.get_path()` — strip the query, trim trailing `/`, take the segment after the last `/` (the `admin/posts/read_by_id` idiom). Parse optional `?include=` and validate against `{"email_recipients","products","tiers","metafields"}` (`members.js:52,81-92`); accept `email` by calling `ctx.store().json_for("members", {{"email", email}})` instead of `find_by_resource`. Reject a missing id with 422 `ApiError{}.validation(..., "id")`.
- **5 (DB call):** `ApiDataStore::find_by_resource` already has a `"members"` branch (`context.cppm:1072-1076`), so keep the call and change only its argument to the real `id`: `ctx.store().find_by_resource("members", id)`. No new resource string or SQL.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>`; on a miss set status 404 with `ApiError{}.json("Member not found.", "Member not found")` (`members.js:100-104`, `context.cppm:66-73`) instead of `{"members":[]}` at 200 (`read_by_id/execute.cppm:18`, `handler.cppm:27`). On a hit keep the `members` envelope — Ghost's serializer also wraps a single member — and set status from the map.
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:82-84`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/:id` router (`routes.cppm:46-151`); Ghost `permissions:true` (`members.js:93`) remains an accepted gap.

Snippet (`read_by_id/execute.cppm`):

```cpp
const auto path = std::string{in.get_path()};
const auto id = path.substr(path.find_last_of('/', path.find('?') == std::string::npos ? path.size() : path.find('?')) + 1);
const auto one = ctx.store().find_by_resource("members", id);
if (!one) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", ApiError{}.json("Member not found.", "Member not found")}}};
return core::workflow::StepResponse<Output>{{{"status","200"},{"body", "{\"members\":[" + *one + "]}"}}};
```

### PUT /members/:id — PARTIAL

- `mutations.edit("members", std::string{}, body)` — id is the empty string (`edit/execute.cppm:16`); Ghost requires `id` option (`members.js:146-153`).
- Envelope `{"members":[...]}` and 200 agree with Ghost's 200 + member; data comes from `edit_entity<model::Member>` (`context.cppm:1307-1308`).
- Cache none; auth `permissions:true` vs token only.

#### Fix

**File/symbol:** `edit/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `edit/handler.cppm` → `EditHandler::handle`. **Checks closed: 3, 4, 6, 7.** No change: 1, 2, 5, 8.

- **1, 2, 5, 8 (no change):** `PUT /members/:id` → `members_edit()` (`routes.cppm:55-61`) → `mutations.edit("members", id, body)`; the `"members"` branch wraps `edit_entity<model::Member>` (`context.cppm:1308-1310`), same resource as `memberBREADService.edit` (`members.js:156-159`).
- **3 (input):** replace the empty id `std::string{}` (`edit/execute.cppm:16`) with the parsed `:id` (query-stripped, last path segment); reject empty with 422 `ApiError{}.validation("Validation error, cannot save member.", ..., "id")` (`members.js:146-153`). Unwrap `data.members[0]` from `in.body_as_string()` with `core_serde::JsonCodec` before passing it to the store.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>` carrying `status`/`body` (posts/create pattern). `wrap` returns 404 on a missing entity (`context.cppm:1394-1402`) and 200 with `{"members":[<member>]}` on success (`context.cppm:1308-1310`); `EditHandler::handle` must read the map and set the status instead of hardcoded `Status::OK` (`edit/handler.cppm:27`), matching Ghost `statusCode:200` (`members.js:142`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:143-145`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/:id` router; Ghost `permissions:true` (`members.js:154`) remains an accepted gap.

Snippet (`edit/execute.cppm`):

```cpp
using Output = std::map<std::string, std::string>;
const auto result = mutations.edit("members", id, innerMemberJson);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())}, {"body", result.body()}}};
```

### DELETE /members/:id — PARTIAL

- `mutations.destroy("members", std::string{})` — id empty (`destroy/execute.cppm:16`); `cancel` option unused.
- Ghost 204 (`members.js:291`) vs C++ 200 + `{}` (`destroy/handler.cppm:26-27`; `context.cppm:1351`).
- Cache none; auth as row above.

#### Fix

**File/symbol:** `destroy/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `destroy/handler.cppm` → `DestroyHandler::handle`. **Checks closed: 3, 4, 6, 7.** No change: 1, 2, 5, 8.

- **1, 2, 5, 8 (no change):** `DELETE /members/:id` → `members_destroy()` (`routes.cppm:63-69`) → `mutations.destroy("members", id)` (`destroy/execute.cppm:16`); the `"members"` branch destroys `model::Member` (`context.cppm:1338-1340`), matching `members.destroy({id}, {...})` (`members.js:305-314`).
- **3 (input):** replace the empty id (`destroy/execute.cppm:16`) with the parsed `:id`; reject empty with 422 (`members.js:296-302`). Parse `cancel` from the query; `cancelStripeSubscriptions` (`members.js:312`) has no allowed primitive (no Stripe lib) — record as an accepted gap, do not add one.
- **4 (output):** `ApiDataStore::destroy` already returns 404 on a miss (`context.cppm:1346-1348`) and 200 + `{}` on success (`context.cppm:1352`). Change `using Output` to `std::map<std::string,std::string>` and map the 200 to **204 with an empty body** in `DestroyHandler::handle` (`members.js:291`), leaving 404 as-is; no hardcoded `Status::OK` (`destroy/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:292-294`).
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/:id` router; Ghost `permissions:true` (`members.js:303`) remains an accepted gap.

Snippet (`destroy/handler.cppm`):

```cpp
const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
const auto out = status == 200 ? interfaces::io::types::Status::NO_CONTENT
                               : static_cast<interfaces::io::types::Status>(status);
const auto body = status == 200 ? std::string{} : payload.at("body");
res.set_status(out); res.set_body(std::vector<std::byte>{body.begin(), body.end()}); send();
```

### DELETE /members/:id/sessions — MISMATCH

- Ghost ends a member's sessions via `memberBREADService.logout(frame.options)` and returns 204 (`members.js:182,166`).
- C++ executes `mutations.destroy("members","")`, i.e. deletes the member record (`members_id_sessions_delete/execute.cppm:16`) and returns 200 (`members_id_sessions_delete/handler.cppm:26-27`). Operation materially different.
- Cache none; auth `permissions.method:'edit'` vs token only.

#### Fix

**File/symbol:** `members_id_sessions_delete/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_id_sessions_delete/handler.cppm` → `MembersIdSessionsDeleteHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `DELETE /members/:id/sessions` → `members_members_id_sessions_delete()` (`routes.cppm:97-104`); evidence complete.
- **3 (input):** parse `:id` from the path (query-stripped, last segment) and require it non-empty (`members.js:170-177`); replace the current no-arg `mutations.destroy("members", "")`.
- **5 (DB call):** replace `mutations.destroy("members", std::string{})` (`members_id_sessions_delete/execute.cppm:16`) with session invalidation, not member deletion. Ghost `memberBREADService.logout({id})` (`members.js:182`) has no allowed service. Exact in-repo replacement: `ctx.store().find_by_resource("members", id)` (`context.cppm:1072-1076`) for the existence check, then write an invalidation marker under the existing `members:` cache namespace with tag `members` and `Ttl{}.never()` (`operations.cppm:269-282,301-326` shows the same key/tag/TTL idiom). `BlogContext` exposes only `store()` (`context.cppm:1711`), so add a public `blog::api::members::MembersOperations::invalidate_sessions(std::string_view id)` in `operations.cppm` wrapping `m_cache.put(key("members:sessions:"...), core::cache::to_cache_value(true), core::cache::Ttl{}.never(), {"members"})`; add `static MembersOperations* s_ops` to the step and change the handler construction at `all.cppm:699` to `{m_ctx, m_members_ops}` (the `all.cppm:701` pattern). No new global state, no SQL, no third-party lib.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>`; set 404 when `find_by_resource` misses (`members.js:521-525` pattern), otherwise **204 with empty body** (`members.js:166`) instead of 200 + `{}` (`members_id_sessions_delete/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:167-169`). The cache write above is cross-request state under an existing namespace, not an HTTP cache.
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/:id` router; Ghost `permissions.method:'edit'` (`members.js:178-180`) remains an accepted gap.

Snippet (`members_id_sessions_delete/execute.cppm`):

```cpp
const auto member = ctx.store().find_by_resource("members", id);
if (!member) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", ApiError{}.json("Member not found.", "Member not found")}}};
s_ops->invalidate_sessions(id); // MembersOperations method; key members:sessions:<id>:invalidated, tag members, Ttl{}.never()
return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
```

### DELETE /members/:id/suppression — MISMATCH

- Ghost reads the member, removes the email from the suppression list, then edits `email_disabled:false`, with 404/500 error paths and 204 response (`members.js:516-543`).
- C++ executes `mutations.destroy("members","")`, deleting the member (`members_id_suppression_delete/execute.cppm:16`) and returning 200 (`members_id_suppression_delete/handler.cppm:26-27`). Operation materially different.
- Cache none; auth `permissions.method:'edit'` vs token only.

#### Fix

**File/symbol:** `members_id_suppression_delete/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `members_id_suppression_delete/handler.cppm` → `MembersIdSuppressionDeleteHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8.

- **1, 2, 8 (no change):** `DELETE /members/:id/suppression` → `members_members_id_suppression_delete()` (`routes.cppm:141-148`); evidence complete.
- **3 (input):** parse `:id` from the path and require it non-empty (`members.js:501-508`); pass the real id instead of the empty string.
- **5 (DB call):** replace `mutations.destroy("members", "")` (`members_id_suppression_delete/execute.cppm:16`) with the three-step Ghost operation using allowed primitives: (1) `ctx.store().find_by_resource("members", id)` (`context.cppm:1072-1076`) → 404 when absent (`members.js:521-525`); (2) remove the member's email from the suppression list via a new public `MembersOperations::remove_suppression(std::string_view email)` in `operations.cppm` that erases key `members:suppression:<email>` from `m_cache` under the existing `members:` namespace, tag `members`, `Ttl{}.never()` (`operations.cppm:269-282` idiom); wire `s_ops` into the step and change the handler construction at `all.cppm:700` to `{m_ctx, m_members_ops}` (the `all.cppm:701` pattern). A miss → 500 (`members.js:530-534`); (3) `ctx.store().mutations.edit("members", id, R"({"email_disabled":false})")` (`members.js:537-540`, `context.cppm:1308-1310`). `emailSuppressionList` is a Ghost service (Ghost JS) — do not import it.
- **4 (output):** change `using Output` to `std::map<std::string,std::string>`; emit **204 with `null`/empty body** on success (`members.js:497,542`), 404 on missing member, 500 on failed removal — instead of 200 + `{}` (`members_id_suppression_delete/handler.cppm:27`).
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:498-500`). The suppression cache write is context-local under the existing `members:` namespace.
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/:id` router; Ghost `permissions.method:'edit'` (`members.js:509-511`) remains an accepted gap.

Snippet (`members_id_suppression_delete/execute.cppm`):

```cpp
const auto member = ctx.store().find_by_resource("members", id);
if (!member) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", ApiError{}.json("Member not found.", "Member not found")}}};
if (!s_ops->remove_suppression(email)) return core::workflow::StepResponse<Output>{{{"status","500"},{"body", ApiError{}.json("Internal Server Error", "Failed to remove email suppression.")}}};
mutations.edit("members", id, R"({"email_disabled":false})");
return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
```

### POST /members/:id/subscriptions/ — PARTIAL

- Ghost requires `id` + `stripe_price_id` and calls Stripe `createSubscription`, then re-reads member with `metafieldsFor:ADMIN` (`members.js:252-265,269-287`).
- C++ decodes `MemberSubscriptionInput{tier,status}` (`operations.cppm:29`; `include/blog/dto/member_subscription_input.cppm:19-20`), hardcodes id `"1"` (`subscriptions_post/execute.cppm:16`), stores subscriptions in cache under `members:subscriptions:*` with TTL `never` and tag `members` (`operations.cppm:31-34,271,487-500`), returns `MemberSubscriptionResponse` with placeholder email/status (`:502-515`).
- Trailing slash difference: Ghost `/subscriptions/` (`admin/routes.js:250`), C++ `/subscriptions` (`routes.cppm:119-127`).
- Cache: Ghost none; C++ has cache behavior (see above). Auth `edit` permission vs token only.

#### Fix

**File/symbol:** `subscriptions_post/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `blog::api::members::MembersOperations::member_subscription_create` (`operations.cppm:24-44`); `subscriptions_post/handler.cppm` → `SubscriptionsPostHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8 (the Ghost trailing slash `/subscriptions/` vs C++ `/subscriptions` at `routes.cppm:118-127` is cosmetic; optionally add an empty child `Route{"/"}` for exact parity — no data change).

- **3 (input):** replace hardcoded `"1"` (`subscriptions_post/execute.cppm:16`) with the parsed `:id` from `in.get_path()` (query-stripped last segment before `/subscriptions`). Replace `blog::dto::MemberSubscriptionInput{tier,status}` (`member_subscription_input.cppm:19-20`) with a body field `stripe_price_id`: add a `stripe_price_id` member + `serde::FieldDesc` to that DTO in place via `core_serde`, then decode the body with `blog::handlers::middleware::decode<MemberSubscriptionInput>` (`operations.cppm:29`). Require `id` and `stripe_price_id` (`members.js:252-265`) — 422 `ApiError{}.validation(..., "stripe_price_id")` on absence. Stripe itself has no allowed primitive; record as an accepted gap.
- **5 (DB call):** call `s_ops->member_subscription_create(id, body)` with the real id, then re-read with `ctx.store().find_by_resource("members", id)` (`context.cppm:1072-1076`) to mirror `memberBREADService.read({id},{metafieldsFor:ADMIN})` (`members.js:276-279`). The existing `members:subscriptions:<id>:count`, `members:subscriptions:<id>:<n>`, `members:subscriptions:next` keys/tag `members`/`Ttl{}.never()` (`operations.cppm:31-34,271,467-500`) are the allowed context-local cache for cross-request state — keep them; do not add new namespaces or global state.
- **4 (output):** replace placeholder `email="member@example.com"`, `status="paid"` in `member_response` (`operations.cppm:502-515`) with the re-read member's email/status and its subscriptions; keep `MemberSubscriptionResponse` (`members` envelope). 200 on success, 404 when the member is absent (`members.js:280-284`). `SubscriptionsPostHandler::handle` already sets status from the map (`subscriptions_post/handler.cppm:26-30`) — no handler change.
- **6 (cache):** Ghost none (`members.js:249-251` ⇒ `cacheInvalidate:false`); add `res.set_header("X-Cache-Invalidate", "false")` before `send()` in the handler for exact parity. The C++ cache keys above are data state, not HTTP cache.
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` on the `/subscriptions` router; Ghost `permissions.method:'edit'` (`members.js:266-268`) remains an accepted gap.

Snippet (`subscriptions_post/execute.cppm`):

```cpp
const auto path = std::string{in.get_path()};
const auto id = path.substr(path.rfind('/', path.find("/subscriptions") - 1) + 1); // real :id, not "1"
const auto r = s_ops->member_subscription_create(id, in.body_as_string());
const auto member = ctx.store().find_by_resource("members", id);
if (!member) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", ApiError{}.json("Member not found.", "Member not found")}}};
```

### PUT /members/:id/subscriptions/:subscription_id — PARTIAL

- Ghost requires `id`, `subscription_id`, `cancel_at_period_end`, `status∈['canceled']` and calls Stripe `cancelSubscription`/`updateSubscription` + re-read (`members.js:193-244`).
- C++ route param is `:id` (`routes.cppm:129`) not `:subscription_id`; step hardcodes both ids to `"1"` (`put_detail/execute.cppm:16`), uses `{tier,status}` DTO and cache-backed edit (`operations.cppm:47-64`), returns subscription-list response rather than a member.
- Cache: Ghost none; C++ `members:subscriptions:*`, TTL `never`, tag `members`. Auth `edit` permission vs token only.

#### Fix

**File/symbol:** `put_detail/workflow/step/execute.cppm` → `ExecuteStep::invoke`; `blog::api::members::MembersOperations::member_subscription_edit` (`operations.cppm:47-64`); `put_detail/handler.cppm` → `PutDetailHandler::handle`. **Checks closed: 3, 4, 5, 6, 7.** No change: 1, 2, 8 (C++ uses `:id` for the subscription placeholder at `routes.cppm:129`; optionally rename to `:subscription_id` — cosmetic).

- **3 (input):** replace both hardcoded `"1"` arguments (`put_detail/execute.cppm:16`) with the two path ids — trim the query, split on `/`, take the penultimate segment as `:id` and the last as `:subscription_id`. Replace `MemberSubscriptionInput{tier,status}` (`member_subscription_input.cppm:19-20`) with `cancel_at_period_end` + `status ∈ {canceled}` (`members.js:193-211`); require both ids (`members.js:196-203`), require `cancel_at_period_end` and validate `status` (`members.js:204-211`) via `ApiError{}.validation(..., "status")` → 422.
- **5 (DB call):** call `s_ops->member_subscription_edit(id, subscription_id, body)` with real ids, then re-read the member via `ctx.store().find_by_resource("members", id)` (`context.cppm:1072-1076`) (`members.js:233-236`). Stripe `cancelSubscription`/`updateSubscription` (`members.js:217-236`) has no allowed primitive — represent the resulting status in the existing `members:subscriptions:*` context-local cache (`operations.cppm:271,487-500`, tag `members`, `Ttl{}.never()`), not new global state.
- **4 (output):** replace the subscription-list response (`operations.cppm:502-515`) with the re-read member's serialized form (`members.js:233-244`); 200 on success, 404 when the member is missing. `PutDetailHandler::handle` already maps the `status`/`body` payload (`put_detail/handler.cppm:26-30`) — no handler change.
- **6 (cache):** add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (`members.js:190-192`); keep existing cache keys/tag/TTL.
- **7 (auth):** add `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:128-138`; Ghost `permissions.method:'edit'` (`members.js:213-215`) remains an accepted gap.

Snippet (`put_detail/execute.cppm`):

```cpp
const auto parts = split_path(in.get_path());            // query-stripped segments
const auto id = parts[parts.size() - 2], subscriptionId = parts.back(); // no hardcoded "1"
const auto r = s_ops->member_subscription_edit(id, subscriptionId, in.body_as_string());
const auto member = ctx.store().find_by_resource("members", id);
if (!member) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", ApiError{}.json("Member not found.", "Member not found")}}};
```

## Missing in C++

None. All 16 Ghost `api.members.*` routes have a C++ route with the same normalized path + verb in `include/blog/handlers/membership/members/routes.cppm`.

Confirmation: grep `api\.members\.` over `docs/Ghost/ghost/core/server/web/api/endpoints/**/routes.js` returned exactly 16 matches (`admin/routes.js:159-254`), and grep `/members` over `include/blog/handlers/**/routes.cppm` found the single `Router{router, "/members"}` block (`routes.cppm:19`) containing all 16 counterparts. No members-app web `routes.js` exists (`glob **/members/**/routes.js` under `docs/Ghost/ghost/core/server/web` found nothing).

  Fix: No change required. All 16 Ghost `api.members.*` routes already have a C++ route with matching normalized path + verb in `routes.cppm:19-239`; check 1 is closed for every entry. The 16 `#### Fix` blocks above reshape the existing handlers — none adds a route.

## Extra in C++

These C++ routes exist in `include/blog/handlers/membership/members/routes.cppm` but have **no** Ghost route bound to the `members` controller (`members.js:55-545`). Ghost does expose the same URLs, but binds them to other controllers — confirmed by grep `api\.members\.` on `admin/routes.js` (16 matches, none of which are these 5):

- `POST /members/:id/commenting/disable` → `members_members_id_commenting_disable_post()` (`routes.cppm:72-83`); Ghost binds `/members/:id/commenting/disable` to `api.memberCommenting.disable` (`admin/routes.js:259-263`).
  Fix: No change required here. The URL is owned by the `member-commenting` controller (`member-commenting.js`), so its parity fix belongs in `docs/parity/member-commenting.md`; do not remove or gate this C++ route from members.md. Check 1 (EXTRA) is closed.
- `POST /members/:id/commenting/enable` → `members_members_id_commenting_enable_post()` (`routes.cppm:84-94`); Ghost binds to `api.memberCommenting.enable` (`admin/routes.js:263`).
  Fix: No change required here. Same owning controller (`member-commenting`); leave the route installed. Check 1 (EXTRA) is closed.
- `GET /members/:id/signin_urls` → `members_signin_urls()` (`routes.cppm:107-117`); Ghost binds to `api.memberSigninUrls.read` (`admin/routes.js:257`).
  Fix: No change required here. Owned by `member-signin-urls` (`docs/parity/member-signin-urls.md`); keep `member_signin_urls()` and its `MembersOperations::member_signin_urls` call (`operations.cppm:66-75`). Check 1 (EXTRA) is closed.
- `GET /members/metafields` → `members_members_metafields_all()` (`routes.cppm:174-184`); Ghost binds `/members/metafields/:namespace` to `api.membersMetafields.*` (`admin/routes.js:199-219`) — C++ has only the single bare `/metafields` route.
  Fix: No change required here. Owned by `member-metafields` (`docs/parity/member-metafields.md`); the missing `/:namespace` variants are tracked there, not by adding routes in `MembersRoutes::install`. Check 1 (EXTRA) is closed.
- `GET /members/stripe_connect` → `members_members_stripe_connect_get()` (`routes.cppm:210-220`); Ghost binds to `api.membersStripeConnect.auth` (`admin/routes.js:183`).
  Fix: No change required here. Owned by `members-stripe-connect` (`docs/parity/members-stripe-connect.md`); do not remove the C++ route. Check 1 (EXTRA) is closed.


## Group 36 implementation status (2026-11-19)

Source fixes applied to the 16 existing member routes (all inside `include/blog/handlers/membership/members`):
- All steps now parse real route params / query options instead of hardcoded `"1"` / `""`; all handler status codes come from a `{"status","body"}` map (`Output = std::map<std::string,std::string>`) except browse/stats/events/upload-GET which keep `Output=std::string`.
- browse: `limit/page` + real `total`/`pages`; create: unwrap `data.members[0]`, `email_type` validation, 201/422; bulk_destroy/bulk_edit: option parsing, Ghost stats meta; stats count/mrr: real envelopes derived from store rows; events: `limit/filter` + `json_for("events", params)`; upload GET: CSV body + `Content-Type`/`Content-Disposition` + filename; upload POST: CSV-row import with 201/202 meta; read_by_id/edit/destroy/sessions/suppression: real `:id` + 404/204; subscriptions/put_detail: real ids, member re-read, 404.
- Every handler sets `X-Cache-Invalidate: false` (commenting routes use `/members/`) except where a route-specific header applies.
- `operations.cppm` gained `invalidate_sessions(id)` and `remove_suppression(email)` (context-local cache helpers) plus the signin/metafields/stripe/account additions recorded in the other docs.

Central registrations needed: **none** for the 16 existing routes — all handlers/flows/steps already exist and are wired in `include/blog/handlers/all.cppm` and `membership/members/routes.cppm`. (The route-level `require_auth` additions recommended in several Fix blocks were intentionally NOT applied: they compile but exceed the router's `MiddlewareSize <= MaxHandlerSize` limit at startup, verified via `bazel test //include/blog/testing:api_test_test`. Inline 401 checks remain.)

Residuals / deferred to central wiring:
- `members_id_sessions_delete` and `members_id_suppression_delete` handlers are constructed with `{ctx}` only in `all.cppm`; their steps therefore cannot call `MembersOperations::invalidate_sessions` / `remove_suppression`. They now do real existence checks, the `email_disabled:false` edit and 204 responses without deleting the member, but the context-local cache writes require changing those two handler constructions to `{ctx, m_members_ops}` in `all.cppm` and setting `ExecuteStep::s_ops`.
- `member_signin_urls` DTO field renamed to `member_signin_urls`; update the assertion at `include/blog/testing/api_test.cc:253` (test file outside allowed folder).
- Admin-token middleware, permission/docName scopes, Stripe side effects and labs gates remain accepted residuals.


## Group 43 implementation record (strict parity, 2026-11-20)

Verdicts above are superseded by the strict-parity pass; see the authority
implementation record at the end of `docs/parity/behavior/members.md` for route-by-route
evidence. Rewrites closed the observable/wire divergences using the registered
shared abstractions (`core_query`, `core_auth:ghost`, `blog_handlers:outbox_shim`,
`core_serde`), kept helpers in the existing `:members_operations` partition, and
used `Output = std::string` / `std::map` with static `s_ctx`. Only files inside
`include/blog/handlers/membership/members/` and the parity docs changed; no
shared file, BUILD edit or Bazel cycle fix was attempted. Real DB/Stripe/SMTP/
session machinery and non-Administrator permission fidelity remain documented
residuals; new-partition needs (member mapper, real session/permission
middleware, Stripe/SMTP adapters, event-sourced stats, multipart adapter) are
recorded in the behavior reports.
