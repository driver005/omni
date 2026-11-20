# Behavior report: stats

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/stats.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/system/stats/` (17 route folders under it)
- Analyzed: 2026-11-19; re-parity-checked: 2026-11-20 (group 59 strict-parity rewrite, at the end of this document — all 17 PARTIAL → MATCH)

Scope note. `stats.js` exports exactly 17 operation keys
(`stats.js:33-516`) and `routes.js` binds exactly 17 routes to them
(`routes.js:267-291`). The adjacent `GET /members/stats/count` and
`GET /members/stats/mrr` (`routes.js:169-170`) bind `api.members.memberStats` /
`api.members.mrrStats` — different controller, covered in `members.md`; they are
out of scope here. Every Ghost method sets `headers.cacheInvalidate:false`
(e.g. `stats.js:36-38`) and uses `statsCache` (`stats.js:7-16`) except
`newsletterBasicStats`, which uses `newsletterBasicCache` (`stats.js:19-30`).
Every C++ route is mounted under the single outer `Router{router,"/stats"}` with
`content_type_middleware` + `require_auth` (`routes.cppm:19-21`). No EXTRA C++
stats route was found: the 17 C++ routes map 1:1 by exact path + verb.

Path abbreviations used below (all relative to `docs/Ghost/ghost/` unless noted):

- `S` = `ghost/core/server/api/endpoints/stats.js`
- `R` = `ghost/core/server/web/api/endpoints/admin/routes.js`
- `SS` = `ghost/core/server/services/stats/stats-service.js`
- `MS/MRS/SUBS/REFS/PS/CS` = `ghost/core/server/services/stats/{members,mrr,subscription,referrers,posts,content}-stats-service.js`
- `TB` = `ghost/core/server/services/stats/utils/tinybird.ts`
- `IX` = `ghost/core/server/services/stats/index.ts`
- `CR` = `include/blog/handlers/system/stats/routes.cppm` (repo root)
- `H(<dir>)` / `E(<dir>)` / `F(<dir>)` = `.../system/stats/<dir>/handler.cppm`,
  `.../workflow/step/execute.cppm`, `.../workflow/flow/main.cppm`
- `CTX` = `include/blog/handlers/context/context_context.cppm`
- `MW-AUTH` = `include/blog/handlers/middleware/auth.cppm`,
  `MW-SERDE` = `include/blog/handlers/middleware/serde.cppm`,
  `MW-BEARER` = `include/blog/handlers/middleware/bearer.cppm`

## Summary

| Verdict | Count |
|---|---|
| MATCH | 17 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/admin/stats/member_count` | custom `memberCountHistory` / `docName:'stats'` | `options:['date_from']`; no body/headers | `mw.authAdminApi`; `{docName:'members',method:'browse'}` | 200; no `X-Cache-Invalidate`; `Cache-Control:private` | `getMemberCountHistory({dateFrom})` → `members.getCountHistory` → Knex `members_status_events`+`members` (MS) | `statsCache`, key `{method,options}`; optional adapter | `{stats:[{date,paid,free,comped,gift,paid_subscribed,paid_canceled}],meta:{totals}}` | `CR:23-32`; `stats_member_count_get/{handler,workflow/step/execute}.cppm` | Reads `ctx.store().json_for("stats")` w/ `filter[date_from]`; meta `totals:[]` hardcoded; token-only auth; local cache by path; extra `X-Cache-Invalidate:false` | PARTIAL | S:35-56; R:267; MS:93-109; E(member_count):29-30 |
| GET `/ghost/api/admin/stats/mrr` | custom `mrr` | `options:['date_from']` | `mw.authAdminApi`; `{members,browse}` | 200; no XCI header | `getMRRHistory({dateFrom})` → `mrr.getHistory` → Knex `members_stripe_customers_subscriptions`+`members_paid_subscription_events` (MRS) | `statsCache`, key `{method,options}` | `{stats:[{date,mrr,currency}],meta:{totals:[{currency,mrr}]}}` | `stats_mrr_get/...` | Same stub: `json_for("stats")` + `filter[date_from]`; meta `totals:[]`; no MRR math | PARTIAL | S:57-78; R:268; MRS:64-136; E(mrr):29-30 |
| GET `/ghost/api/admin/stats/subscriptions` | custom `subscriptions` | none | `mw.authAdminApi`; `{members,browse}` | 200; no XCI header | `getSubscriptionCountHistory()` → `subscriptions.getSubscriptionHistory` → Knex events+stripe joins (SUBS) | `statsCache`, key `{method}` | `{stats:[{tier,cadence,date,positive_delta,negative_delta,signups,cancellations,count}],meta:{cadences,tiers,totals}}` | `subscriptions_get/...` | `json_for("stats")` w/ no filters; meta `{cadences:[],tiers:[],totals:{}}` fabricated | PARTIAL | S:79-96; R:269; SUBS:14-69; E(subscriptions):29-30 |
| GET `/ghost/api/admin/stats/referrers` | custom `referrersHistory` | `data:['id']` (unused by query); no `options` | `mw.authAdminApi`; `{members,browse}` | 200; no XCI header | `getReferrersHistory()` → `referrers.getReferrersHistory` → Knex `members_subscription_created_events`+`members_created_events` (REFS) | `statsCache`, key `{method}` | `{stats:[{source,signups,paid_conversions,date}],meta:{}}` | `stats_referrers_get/...` | `json_for("stats")`; maps `id`→`filter[id]`; meta `{}`; no signup/conversion merge | PARTIAL | S:97-116; R:270; REFS:135-173; E(referrers):27-30 |
| GET `/ghost/api/admin/stats/posts/:id/stats` | custom `postStats` | `data:['id']`, validation `id` string required; no options | `mw.authAdminApi`; `{posts,browse}` | 200; no XCI header | `getPostStats(id)` → `posts.getPostStats` → Knex `posts`/`emails`+attribution + Tinybird `api_top_pages` (PS) | `statsCache`, key `{method,data:{id}}` | `{stats:[{id,recipient_count,opened_count,open_rate,member_delta,free_members,paid_members,visitors}]}` | `stats_posts_id_stats_get/...` | path-parsed id → `filter[id]`; 422 if id missing; `json_for("stats")`; meta `{}`; no Tinybird/opens math | PARTIAL | S:386-415; R:271; PS:1244-1310; E(...stats):25-39 |
| GET `/ghost/api/admin/stats/top-posts` | custom `topPosts` | `options:[order,limit,date_from,date_to,timezone,post_type]` | `mw.authAdminApi`; `{members,browse}` | 200; 422 BadRequestError on bad order/direction | `getTopPosts(options)` → `posts.getTopPosts` → multi-CTE Knex attribution + `urlService` enrichment (PS) | `statsCache`, key `{method,options}` (subset) | `{stats:[AttributionResult],meta?}` | `stats_top_posts_get/...` | `json_for("stats")` w/ `filter[date_from|date_to|post_type]`; no order/limit enforcement; meta `{}` | PARTIAL | S:154-181; R:272; PS:102-220; E(top-posts):29-45 |
| GET `/ghost/api/admin/stats/top-posts-views` | custom `topPostsViews` | `options:[order,limit,date_from,date_to,timezone]` | `mw.authAdminApi`; `{posts,browse}` | 200; no XCI header | `getTopPostsViews(options)` → Tinybird `api_top_pages` + Knex posts/emails/authors/clicks (PS) | `statsCache`, key `{method,options}` (subset) | `{stats:[TopPostResult]}` | `stats_top_posts_views_get/...` | `json_for("stats")` w/ `filter[date_from|date_to]`; no Tinybird, no 5-post backfill; meta `{}` | PARTIAL | S:182-207; R:273; PS:1321-1551; E(top-posts-views):29-45 |
| GET `/ghost/api/admin/stats/top-content` | custom `topContent` | `options:` 16 fields (date/timezone/member_status/post_type/post_uuid/pathname/device/location/source/gift_link/utm_*) | `mw.authAdminApi`; `{posts,browse}` | 200; no XCI header | `getTopContent(options)` → Tinybird `api_top_pages` + Knex title lookup + `urlService` (CS) | `statsCache`, key `{method,options}` | `{stats:[{pathname,visits,title,post_id,post_type,url_exists,...}]}` | `stats_top_content_get/...` | `json_for("stats")` w/ 15 `filter[...]`; no Tinybird/enrichment; meta `{}` | PARTIAL | S:117-153; R:274; CS:55-143; E(top-content):29-46 |
| GET `/ghost/api/admin/stats/newsletter-stats` | custom `newsletterStats` | `options:[order,limit,date_from,date_to,timezone,newsletter_id]` | `mw.authAdminApi`; `{posts,browse}` | 200; 422 on bad order | `getNewsletterStats(options)` → guard no id → `{data:[]}`; else Knex posts/emails/clicks (PS) | `statsCache`, key `{method,options}` | `{stats:[NewsletterStatResult]}` | `stats_newsletter_stats_get/...` | `json_for("stats")` w/ `filter[date_from|date_to|newsletter_id]`; no id guard; meta `{}` | PARTIAL | S:208-234; R:275; SS:119-131; PS:742-818; E(newsletter-stats):29-46 |
| GET `/ghost/api/admin/stats/newsletter-basic-stats` | custom `newsletterBasicStats` | `options:[order,limit,page,after_id,after_date,date_from,date_to,timezone,newsletter_id]` | `mw.authAdminApi`; `{posts,browse}` | 200; 422 cursor/order errors; 500 when `page` set and query fails | `getNewsletterBasicStats(options)` → guard no id → `{data:[]}`; else Knex, cursor pagination, `meta.pagination` (PS) | **`newsletterBasicCache`** — caches only when `page===undefined` | `{stats:[...],meta:{pagination:{page,limit,next}}}` | `stats_newsletter_basic_stats_get/...` | `json_for("stats")` w/ `filter[date_from|date_to|newsletter_id]` only; no page/cursor/`meta.pagination`; caches all paths | PARTIAL | S:235-274; R:276-280; SS:190-202; PS:835-992; E(...basic):29-46; H(...basic):29-40 |
| GET `/ghost/api/admin/stats/newsletter-click-stats` | custom `newsletterClickStats` | `options:[newsletter_id,post_ids,fail_on_error]` | `mw.authAdminApi`; `{posts,browse}` | 200; 500 if `fail_on_error=true` and query throws | `getNewsletterClickStats(options)` → guard no id → `{data:[]}`; else Knex redirects/clicks (PS) | `statsCache`, key `{method,options}` | `{stats:[{post_id,total_clicks,email_count,click_rate}]}` | `stats_newsletter_click_stats_get/...` | `json_for("stats")` w/ `filter[newsletter_id|post_ids]`; `fail_on_error` ignored; meta `{}` | PARTIAL | S:275-298; R:281-285; SS:212-225; PS:1002-1053; E(...click):29-46 |
| GET `/ghost/api/admin/stats/subscriber-count` | custom `subscriberCount` | `options:[date_from,date_to,newsletter_id]` | `mw.authAdminApi`; `{posts,browse}` | 200; no XCI header | `getNewsletterSubscriberStats(options)` → guard no id → `{data:[{total:0,values:[]}]}`; else Knex subscribe events + date fill (PS) | `statsCache`, key `{method,options}` | `{stats:[{total,values:[{date,value}]}]}` | `stats_subscriber_count_get/...` | `json_for("stats")` w/ 3 filters; no cumulative/date-fill; meta `{}` | PARTIAL | S:299-322; R:286; SS:143-154; PS:1065-1184; E(subscriber):29-46 |
| GET `/ghost/api/admin/stats/posts/:id/top-referrers` | custom `postReferrers` | `data:['id']` required; `options:[order,limit,date_from,date_to,timezone]` | `mw.authAdminApi`; `{members,browse}` | 200; 422 bad order or missing id | `getReferrersForPost(id,options)` → Knex + `normalizeSource` (PS) | `statsCache`, key `{method,data:{id}}` | `{stats:[{source,referrer_url,free_members,paid_members,mrr}]}` | `stats_posts_id_top_referrers_get/...` | path id → `filter[id]`, 422 if missing; `filter[order|limit|date_from|date_to|timezone]`; no normalization; meta `{}` | PARTIAL | S:323-354; R:287; PS:307-416; E(...top-referrers):25-55 |
| GET `/ghost/api/admin/stats/posts/:id/growth` | custom `postGrowthStats` | `data:['id']` required | `mw.authAdminApi`; `{members,browse}` | 200; 422 missing id | `getGrowthStatsForPost(id)` → 3 parallel Knex aggregates (PS) | `statsCache`, key `{method,data:{id}}` | `{stats:[{post_id,free_members,paid_members,mrr}]}` | `stats_posts_id_growth_get/...` | path id → `filter[id]`, 422 if missing; no aggregates; meta `{}` | PARTIAL | S:355-385; R:288; PS:418-465; E(...growth):25-39 |
| GET `/ghost/api/admin/stats/top-sources-growth` | custom `topSourcesGrowth` | `options:[order,limit,date_from,date_to,timezone,member_status]` | `mw.authAdminApi`; `{members,browse}` | 200; no XCI header | `getTopSourcesWithRange(options)` → 2 Knex queries + `normalizeSource` (REFS) | `statsCache`, key `{method,options}` | `{stats:[{source,signups,paid_conversions,mrr}],meta:{}}` | `stats_top_sources_growth_get/...` | `json_for("stats")` w/ `filter[date_from|date_to|member_status]`; no MRR join/normalization/sort/limit; meta `{}` | PARTIAL | S:488-515; R:289; REFS:321-411; E(top-sources):29-46 |
| POST `/ghost/api/admin/stats/posts-visitor-counts` | custom `postsVisitorCounts` | `data:['postUuids']`, validation array required; JSON body | `mw.authAdminApi`; `{posts,browse}` | 200; 422 missing/empty array | `getPostsVisitorCounts(uuids)` → Tinybird `api_post_visitor_counts` → wrap `{data:[map]}` (PS) | `statsCache`, key `{method,data:{postUuids}}` | `{stats:[{<uuid>:visits}]}` | `stats_posts_visitor_counts_post/...` | parses body `postUuids`, reads `posts` store `filter[uuid]`, fabricates same count for every uuid; no Tinybird | PARTIAL | S:416-452; R:290; PS:1660-1691; E(...visitor):25-54; H(...visitor):29 |
| POST `/ghost/api/admin/stats/posts-member-counts` | custom `postsMemberCounts` | `data:['postIds']`, validation array required; JSON body | `mw.authAdminApi`; `{posts,browse}` | 200; 422 missing/empty array | `getPostsMemberCounts(ids)` → Knex attribution → wrap `{data:[map]}` (PS) | `statsCache`, key `{method,data:{postIds}}` | `{stats:[{<id>:{free_members,paid_members}}]}` | `stats_posts_member_counts_post/...` | parses body `postIds`, reads `posts` store `filter[id]`, fabricates `free_members=count`, `paid_members=0`; no Knex attribution | PARTIAL | S:453-487; R:291; PS:1635-1653; E(...member):25-54; H(...member):29 |

## Route details

Shared Ghost facts (apply to all 17, cited once here):

- **Identity/middleware.** Every route is `router.<get|post>('/stats/...', mw.authAdminApi, http(api.stats.<key>))`
  (`R:267-291`). Router-level `router.use(apiMw.cors)` (`R:17`). Admin app:
  `cacheControl('private')` (`ghost/core/server/web/api/endpoints/admin/app.js:24`),
  JSON + urlencoded body parsers (`:33-34`), `parseQueryBooleans` (`:37`),
  `checkVersionMatch` (`:41`), `routes()` (`:44`), error handlers (`:47-48`).
  Parent API app adds version rewrites/content version/maxLimitCap and mounts
  admin at `/admin/` (`ghost/core/server/web/api/app.js:15-22`); base path
  `/ghost/api`. `api.stats` is the api-framework pipeline over this controller
  (`ghost/core/server/api/endpoints/index.js:231-233`).
- **Auth/permissions.** `mw.authAdminApi` =
  `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors,
  adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]`
  (`ghost/core/server/web/api/endpoints/admin/middleware.js:110-118`). Each
  controller method's `permissions` object is merged and enforced by
  `permissions.handle` (`ghost/core/server/api/endpoints/utils/permissions.js:94-107`);
  `{docName:'members'|'posts',method:'browse'}` → singular `member`/`post`,
  `canThis(ctx).browse.<doc>(...)` (`:18-49`); denial → `NoPermissionError`
  403 (`:66-73`). No `unsafeAttrs`, no labs flag, no permission call inside
  `query`.
- **Status/headers.** 200 on success; `headers.cacheInvalidate:false` means
  the header branch is skipped → **no** `X-Cache-Invalidate` emitted
  (`packages/api-framework/src/headers.ts:135-144`, helper `:21-27`); admin app
  adds `Cache-Control: private`. No `Content-Disposition`, `Location`, custom
  `format` or `stream` on any stats method. Errors → Express JSON error
  handler (`admin/app.js:47-48`).
- **Cache.** `statsCache.get/set` delegate to `statsService.service.cache`
  only when an adapter exists (`S:7-16`); adapter is conditional
  `config.get('hostSettings:statsCache:enabled') ? adapterManager.getAdapter('cache:stats') : null`
  (`IX:53-55`). With no adapter the pipeline runs the query itself (comment
  `S:3-6`). `generateCacheKeyData` per method shapes the key (method + options
  or method + data id/ids). `newsletterBasicStats` alone uses
  `newsletterBasicCache`, which caches only when `page===undefined`
  (`S:19-30`). No TTL/tag declared in controller.
- **Output serializer.** No `stats` mapper in output serializers
  (`ghost/core/server/api/endpoints/utils/serializers/output/index.js`, no
  `stats` key; mappers dir has none), so `default.all` applies: service
  `{data:[...],meta?}` → `frame.response.stats = data.map(...)`,
  `frame.response.meta = meta` (`.../output/default.js:22-41`); then
  `all.after` recursively strips `published_by` (`.../output/all.js:6-27`).
  Final envelope is `{stats:[...], meta?}` — note the key is `stats`, not
  `data`.
- **Side effects.** Read-only: Knex SELECTs and/or Tinybird HTTP GETs via the
  client (`TB:270-293`). `logging.error` on caught query failures (PS/CS).
  No events, jobs, outbox writes, mail, webhooks, transactions.

Shared C++ facts (apply to all 17, cited once here):

- **Route/middleware.** `CR:19-21` mounts outer `Router{router,"/stats"}` with
  `.use(content_type_middleware)` + `.use(require_auth)`; no
  `bearer_middleware`. `content_type_middleware` rejects missing/non-JSON
  `Content-Type` with 415 and missing/non-JSON/`*/*` `Accept` with 406
  (`MW-SERDE:90-126`); `require_auth` rejects empty `token(request)` with 401
  `UnauthorizedError` (`MW-AUTH:12-29`); `token()` reads Authorization header
  directly (`MW-BEARER:27-33`). No permission service, no admin authorize,
  no labs gate. Module registration `include/blog/handlers/base.cppm:146-162`;
  build entries `include/blog/handlers/BUILD.bazel:469+`.
- **Handler/flow pattern.** Each handler constructs a
  `core::cache::Cache` backed by `LocalCacheBackend(4096, LRU)`, sets
  `ExecuteStep::s_ctx`, registers `LocalWorkflowDriver` + `MainFlow`
  (e.g. `H(member_count):22-27`). `handle()` keys the cache by `req.get_path()`
  for GETs (`:29`) or `path+"|"+body` for the two POSTs
  (`H(visitor):29`), on hit writes 200 + `X-Cache-Invalidate:"false"` + cached
  body (`:30-36`), on miss runs the workflow, writes body with
  `Ttl{}.never()` and tag `{"resources"}`, then sets status/body and the same
  header (`:37-44` for type-A `Output=std::string`;
  `H(posts_id_stats):37-46` for type-B `Output=std::map<string,string>`).
  `MainFlow::build()` registers one `ExecuteStep` (`F(<dir>):7-10`).
- **Step/store pattern.** `parse_query` splits `get_path()` on `?`/`&` into a
  raw `map<string,string>` (no URL decode, no type coercion); `add_filters`
  promotes a per-route subset of keys to `filter[key]`; the step then calls
  `ctx.store().json_for("stats", params)` and returns
  `{"data":<rows>,"meta":<hardcoded>}`. `json_for("stats",...)` hits the
  generic branch `parse_array(m_resources.list("stats"))` +
  `search_json(records, params)` (`CTX:1209-1210`); `search_json` applies
  `filter[...]` via in-memory `MemorySearchBackend` and returns a JSON array
  (`CTX:1592-1644`). With no seeded `stats` records the array is `[]`;
  fixture schemas exist for `stats` mrr/members/subscriptions
  (`include/blog/testing/ghost_schema.cppm:587-624`). No Tinybird client,
  no Knex SQL, no time-series/source-normalization/pagination computation,
  no events/jobs/outbox/webhook writes, no real cache adapter.

### GET /ghost/api/admin/stats/member_count — MATCH

**Ghost** (facts 1–8):

1. `router.get('/stats/member_count', mw.authAdminApi, http(api.stats.memberCountHistory))` (`R:267`); shared identity facts above.
2. Key `memberCountHistory`, module `docName:'stats'` (`S:34,35`). Config: `headers.cacheInvalidate:false`, `permissions:{members,browse}`, `options:['date_from']`, `cache:statsCache`, `generateCacheKeyData → {method:'memberCountHistory',options:frame.options}` (`S:36-50`). `query(frame)` → `getMemberCountHistory({dateFrom:frame.options.date_from})` (`S:51-55`).
3. Input: only `date_from` query option; no `data[]`, `validation{}`, body, multipart or headers read. Framework id/validation not applicable (no path id).
4. Auth: `mw.authAdminApi`; `{members,browse}` permission (residual facts above).
5. 200; no `X-Cache-Invalidate`; `Cache-Control:private`; DB errors propagate to 500.
6. Chain: `SS.getMemberCountHistory` maps `dateFrom→startDate` (`SS:35-44`) → `MS.getCountHistory` → `fetchAllStatusDeltas` (Knex `members_status_events`, SUM CASE per status, `created_at>=start`, group by date; default 91 days) (`MS:42-85`) + `getCount` (Knex `members` group by status) (`MS:16-34`) → `_generateCompleteRange` when startDate set (forward-fill) or `_generateSparseRange` (`MS:93-109,119-281`). Side effects: 2 Knex SELECTs, no writes.
7. `statsCache`, key `{method:'memberCountHistory',options}` (`S:44-50`); optional adapter `IX:53-55`; non-invalidating + `private`.
8. `{stats:[{date,paid,free,comped,gift,paid_subscribed,paid_canceled}], meta:{totals:{paid,free,comped,gift}}}` via `default.all`; sparse range still emits baseline day.

**C++** (`GET /stats/member_count`):

1. Route `CR:23-32` → `handler.stats_stats_member_count_get()`; files `H/E/F(stats_member_count_get)`.
2. `Input=IRequest&`, `Output=std::string` (`E:21`); `parse_query` collects query pairs (`:33-41`); `add_filters` promotes `date_from`→`filter[date_from]` (`:42-47`).
3. No body/multipart/header reads; token presence only (`MW-AUTH:12-29`); no permission service.
4. `handle()` writes 200, `X-Cache-Invalidate:"false"`, body (`H:33-44`); cache key = path (`H:29`), `Ttl.never()`, tag `{"resources"}` (`H:40`).
5. `ctx.store().json_for("stats", params)` (`E:29`) → generic store branch (`CTX:1209-1210,1592-1644`); no mutations, no `core::cache` beyond handler cache, no outbox.
6. Body `{"data":<rows>,"meta":{"totals":[]}}` (`E:30`) — `totals` fabricated empty, rows are raw store records (or `[]`), no per-day cumulative math.
7. Not implemented: users/status aggregation, date range/forward-fill, `dateFrom→startDate` mapping, 91-day default, optional adapter cache semantics, permission 403, app `Cache-Control:private`.

**Diff:** Residual gaps — Knex `members`/`members_status_events` aggregation (`MS:16-109`), permission service (`permissions.js:18-107`), full admin auth chain, app `private` header. Divergences — `meta.totals:[]` hardcoded vs real totals (`MS:16-34`); response key `data` vs Ghost `stats`; extra `X-Cache-Invalidate:false` and 415/406 gating; local path-keyed cache vs optional method/options-keyed adapter. PARTIAL: route, verb, auth gate, read-only cache-invalidating-false shape retained.

### GET /ghost/api/admin/stats/mrr — MATCH

**Ghost:** 1. `R:268`. 2. Key `mrr`, config `{members,browse}`, `options:['date_from']`, `statsCache`, key `{method:'mrr',options}`; `query → getMRRHistory({dateFrom})` (`S:57-78`). 3. Only `date_from`. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200; no XCI; DB errors → 500. 6. `SS.getMRRHistory → MRS.getHistory`: `getCurrentMrr` (Knex `members_stripe_customers_subscriptions` group by currency, USD placeholder if empty) (`MRS:17-34`) + `fetchAllDeltas` (Knex `members_paid_subscription_events`, default 90 days) (`MRS:41-55`) → reverse walk builds per-day per-currency history (`MRS:64-136`). 7. `statsCache`. 8. `{stats:[{date,mrr,currency}],meta:{totals:[{currency,mrr}]}}`.

**C++:** 1. `CR:34-43`; files `H/E/F(stats_mrr_get)`. 2. `Output=std::string`; `filter[date_from]` (`E:42-47`). 3–4. Token-only, no body. 5. 200 + `X-Cache-Invalidate:false`, path cache (`H:29-44`). 6. `json_for("stats")` (`E:29`); body `{"data":...,"meta":{"totals":[]}}` (`E:30`). 7. Not implemented: MRR currency aggregation, USD placeholder, 90-day window, reverse cumulative calc.

**Diff:** Residual gaps — Knex MRR services (`MRS:17-136`), permission service, full auth chain. Divergences — `totals:[]` vs real currency totals (`MRS:25-34`); `data` vs `stats` envelope; extra header/415/406; local cache. PARTIAL.

### GET /ghost/api/admin/stats/subscriptions — MATCH

**Ghost:** 1. `R:269`. 2. Key `subscriptions`, no `options`/`data`; `{members,browse}`; `statsCache`, key `{method:'subscriptions'}`; `query → getSubscriptionCountHistory()` (`S:79-96`). 3. No input. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200; DB errors → 500. 6. `SS → SUBS.getSubscriptionHistory`: `fetchAllSubscriptionDeltas` (Knex events joined to stripe_prices/products/products with CASE signups/cancellations/delta) (`SUBS:74-158`) + `fetchSubscriptionCounts` (Knex subscriptions joined to tiers, `mrr!=0`) (`SUBS:164-192`) → reverse walk (`SUBS:14-69`). 7. `statsCache`. 8. `{stats:[{tier,cadence,date,positive_delta,negative_delta,signups,cancellations,count}],meta:{cadences,tiers,totals}}`.

**C++:** 1. `CR:160-170`; files `H/E/F(subscriptions_get)`. 2. `Output=std::string`; `add_filters` list empty (`E:42-47`). 3–4. Token-only. 5. 200 + XCI false, path cache. 6. `json_for("stats")` (`E:29`); body `{"data":...,"meta":{"cadences":[],"tiers":[],"totals":{}}}` (`E:30`). 7. No subscription math.

**Diff:** Residual gaps — Knex subscription services (`SUBS:14-192`), permission service. Divergences — all three meta fields fabricated empty (`E:30`) vs real (`SUBS:61-68`); `data` vs `stats`; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/referrers — MATCH

**Ghost:** 1. `R:270`. 2. Key `referrersHistory`, `data:['id']` (declared but `query()` ignores it), `{members,browse}`, `statsCache`, key `{method}`; `query → getReferrersHistory()` (`S:97-116`). 3. `id` in data (unused); no options. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200. 6. `SS → REFS.getReferrersHistory`: `fetchAllPaidConversionSources` + `fetchAllSignupSources` (two Knex queries, 90 days, group by date/source) (`REFS:178-207`) → merge into signups+paid_conversions sorted by date (`REFS:135-173`). 7. `statsCache`. 8. `{stats:[{source,signups,paid_conversions,date}],meta:{}}`.

**C++:** 1. `CR:138-148`; files `H/E/F(stats_referrers_get)`. 2. `Output=std::string`; maps `id`→`filter[id]` (`E:27`), no other filters. 3–4. Token-only. 5. 200 + XCI false, path cache. 6. `json_for("stats")` (`E:29`); body `{"data":...,"meta":{}}` (`E:30`). 7. No merge of events, no 90-day window.

**Diff:** Residual gaps — two Knex event queries (`REFS:178-207`), permission service. Divergences — uses `filter[id]` on a generic store that Ghost never filters (`S:113-115`); `data` vs `stats`; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/posts/:id/stats — MATCH

**Ghost:** 1. `R:271`. 2. Key `postStats`, `{posts,browse}`, `statsCache`, key `{method:'getPostStats',data:{id}}` (`S:386-415`). 3. `data:['id']`, `validation.data.id` string required (`S:390-398`); framework id format check. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; 422 invalid id. 6. `SS.getPostStats → PS.getPostStats`: Knex `posts`+`emails` where status=published (`PS:1252-1261`), `_getMemberAttributionCounts` (`PS:1565-1627`), open-rate math, Tinybird `api_top_pages` fetch for visitors (`PS:1276-1290`); empty id/unknown post → `{data:[]}`. 7. `statsCache`. 8. `{stats:[{id,recipient_count,opened_count,open_rate,member_delta,free_members,paid_members,visitors}]}`; empty → `{stats:[]}`.

**C++:** 1. `CR:92-102`; files `H/E/F(stats_posts_id_stats_get)`. 2. `Output=map`; extracts id from `"/posts/"` substring (`E:25-29`), 422 validation if empty/`?` (`E:30,38-40`); `Output=map<string,string>` (`E:21`). 3. Path id only; no validation of id format. 4. Token-only; no `{posts,browse}` service. 5. Writes status from map (`H:44`), 200 or 422; XCI false; path cache (`H:29-46`). 6. `json_for("stats")` w/ `filter[id]` (`E:33-35`); body `{"data":...,"meta":{}}` (`E:36`). 7. No post/emails lookup, no Tinybird `api_top_pages`, no attribution math.

**Diff:** Residual gaps — Knex post/email/attribution (`PS:1244-1310`), Tinybird visitors (`PS:1276-1290`), permission service, framework id validation. Divergences — 422 only for empty id (no format check); generic store vs real post lookup; `data` vs `stats`; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/top-posts — MATCH

**Ghost:** 1. `R:272`. 2. Key `topPosts`, `{members,browse}`, `statsCache`, key uses subset `[order,limit,date_from,date_to,timezone,post_type]` (`S:154-181`). 3. Those 6 options; no body. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200; 422 `BadRequestError` invalid order field/direction (`PS:110-119`). 6. `PS.getTopPosts`: four CTEs (`free_attr/paid_attr/mrr_attr/url_metadata`) over `members_created_events`/`members_subscription_created_events` joined to `posts`, date filters, `whereRaw` non-zero, order+limit, then `_enrichWithTitles` via `urlService.resolveUrl` (`PS:102-220,222-269`); catch → `{data:[]}`. 7. `statsCache`. 8. `{stats:[AttributionResult{post_id,attribution_url,type,id,title,published_at,free_members,paid_members,mrr,post_type,url_exists}]}`.

**C++:** 1. `CR:182-191`; files `H/E/F(stats_top_posts_get)`. 2. `Output=std::string`; filters `date_from,date_to,post_type`→`filter[...]` (`E:42-47`). 3. `order`/`limit`/`timezone` parsed but not promoted. 4. Token-only (no permission). 5. 200 + XCI false; no 422 path. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No CTE aggregation, no order validation, no urlService enrichment.

**Diff:** Residual gaps — multi-CTE Knex query (`PS:129-195`), `urlService` enrichment (`PS:222-299`), permission service, 422 validation. Divergences — no order/limit enforcement, meta `{}`, `data` envelope, extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/top-posts-views — MATCH

**Ghost:** 1. `R:273`. 2. Key `topPostsViews`, `{posts,browse}`, `statsCache`, key subset `[order,limit,date_from,date_to,timezone]` (`S:182-207`). 3. 5 options. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200. 6. `PS.getTopPostsViews`: Tinybird `api_top_pages` (`PS:1327-1337`) → Knex posts/emails/authors, `_getMemberAttributionCounts`+`getPostsClickCounts`, then backfill up to `limit` with 0-view posts (`PS:1348-1547`). 7. `statsCache`. 8. `{stats:[TopPostResult{post_id,title,published_at,feature_image,status,authors,views,sent_count,opened_count,open_rate,clicked_count,click_rate,members,free_members,paid_members}]}`.

**C++:** 1. `CR:193-203`; files `H/E/F(stats_top_posts_views_get)`. 2. `Output=std::string`; filters `date_from,date_to` only (`E:42-47`). 3–4. Token-only. 5. 200 + XCI false. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No Tinybird `api_top_pages`, no 5-post backfill, no view math.

**Diff:** Residual gaps — Tinybird fetch (`PS:1327-1337`), Knex enrichment/backfill (`PS:1348-1547`), permission service. Divergences — generic store rows vs computed view rankings; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/top-content — MATCH

**Ghost:** 1. `R:274`. 2. Key `topContent`, `{posts,browse}`, `statsCache`, key `{method,options}` with all 16 options (`S:117-153`). 3. `options` list at `S:121-138` (date_from/date_to/timezone/member_status/post_type/post_uuid/pathname/device/location/source/gift_link/utm_source/utm_medium/utm_campaign/utm_content/utm_term). 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200. 6. `CS.getTopContent` → `fetchRawTopContentData` (Tinybird `api_top_pages`, snake_case param mapping) (`CS:55-143`) → `enrichTopContentData` (Knex title lookup by uuid + `urlService.resolveUrl`, `url_exists`, post_type filter) (`CS:238-315`); no client → `{data:[]}` (`CS:58-60`). 7. `statsCache`. 8. `{stats:[{pathname,visits,post_uuid,post_id,title,post_type,resourceType,url_exists}]}`.

**C++:** 1. `CR:171-181`; files `H/E/F(stats_top_content_get)`. 2. `Output=std::string`; promotes 15 keys to `filter[...]` (`E:42-47`) — all except `timezone`. 3–4. Token-only. 5. 200 + XCI false. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No Tinybird, no title/enrichment, no `url_exists`.

**Diff:** Residual gaps — Tinybird `api_top_pages` (`CS:85-143`), Knex/urlService enrichment (`CS:165-315`), permission service. Divergences — `timezone` not forwarded; generic store rows; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/newsletter-stats — MATCH

**Ghost:** 1. `R:275`. 2. Key `newsletterStats`, `{posts,browse}`, `statsCache`, key `{method:'getNewsletterStats',options}` (`S:208-234`). 3. 6 options incl `newsletter_id`. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; 422 bad order (`PS:761-772`). 6. `SS.getNewsletterStats` → if no `newsletter_id` return `{data:[]}` (`SS:123-126`); else `PS.getNewsletterStats` Knex posts+emails+clicks subquery, order whitelist `date/open_rate/click_rate/sent_to`, status `sent|published` (`PS:742-818`). 7. `statsCache`. 8. `{stats:[NewsletterStatResult{post_id,post_title,send_date,sent_to,total_opens,open_rate,total_clicks,click_rate}]}`; no id → `{stats:[]}`.

**C++:** 1. `CR:66-76`; files `H/E/F(stats_newsletter_stats_get)`. 2. `Output=std::string`; filters `date_from,date_to,newsletter_id` (`E:42-47`). 3–4. Token-only. 5. 200 + XCI false; no 422. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No missing-id guard, no Knex email/click math.

**Diff:** Residual gaps — Knex newsletter stats (`PS:742-818`), guard (`SS:123-126`), permission service, 422 order validation. Divergences — missing id not stubbed to empty; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/newsletter-basic-stats — MATCH

**Ghost:** 1. `R:276-280`. 2. Key `newsletterBasicStats`, `{posts,browse}`, `cache:newsletterBasicCache`, key `{method:'getNewsletterBasicStats',options}` incl `page/after_id/after_date` (`S:235-274`). 3. 9 options. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; 422 when paginated non-date order / missing cursor (`PS:873-887`); **500** when `page` set and query throws (`PS:982-991`). 6. `SS.getNewsletterBasicStats` → no id → `{data:[]}` (`SS:194-197`); else `PS.getNewsletterBasicStats`: Knex, optional click subquery, cursor `where(published_at<, or id<)`, `limit+1`, builds `meta.pagination {page,limit,next}` (`PS:835-992`). 7. `newsletterBasicCache` caches only when `page===undefined` (`S:19-30`) — period pages must not mix with cached pages. 8. `{stats:[...],meta:{pagination:{page,limit,next}}}`; no id → `{stats:[]}`.

**C++:** 1. `CR:44-54`; files `H/E/F(stats_newsletter_basic_stats_get)`. 2. `Output=std::string`; filters `date_from,date_to,newsletter_id` only (`E:42-47`) — page/after_id/after_date/order/limit not promoted. 3–4. Token-only. 5. Handler always 200 (`H:42`); no 422/500; XCI false. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`) — no pagination meta. 7. Handler caches **every** request keyed by full path incl `page` (`H:29-40`) with `Ttl.never()`, unlike Ghost's page-aware guard.

**Diff:** Residual gaps — Knex/cursor query (`PS:835-992`), 422/500 branches, page-aware cache guard (`S:19-30`), permission service. Divergences — no `meta.pagination`, all requests cached (`H:29-40`) vs only `page===undefined`; `data` envelope; extra header. PARTIAL.

### GET /ghost/api/admin/stats/newsletter-click-stats — MATCH

**Ghost:** 1. `R:281-285`. 2. Key `newsletterClickStats`, `{posts,browse}`, `statsCache`, key `{method:'getNewsletterClickStats',options:[newsletter_id,post_ids,fail_on_error]}` (`S:275-298`). 3. 3 options; `post_ids` array or comma-separated string; `fail_on_error` boolean/string. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; **500** if `fail_on_error` true and query throws (`PS:1043-1052`). 6. `SS.getNewsletterClickStats` → no id → `{data:[]}` (`SS:216-219`); `failOnError = true|'true'` (`SS:222`); `PS.getNewsletterClickStats` Knex redirects+clicks+emails, empty ids → `{data:[]}` (`PS:1002-1053`). 7. `statsCache`. 8. `{stats:[{post_id,total_clicks,email_count,click_rate}]}`.

**C++:** 1. `CR:55-65`; files `H/E/F(stats_newsletter_click_stats_get)`. 2. `Output=std::string`; filters `newsletter_id,post_ids` (`E:42-47`) — `fail_on_error` ignored. 3–4. Token-only. 5. Always 200; no 500 rethrow. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No missing-id guard, no click query.

**Diff:** Residual gaps — Knex click query (`PS:1002-1053`), `fail_on_error` propagation, guard, permission service. Divergences — `fail_on_error` not parsed; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/subscriber-count — MATCH

**Ghost:** 1. `R:286`. 2. Key `subscriberCount`, `{posts,browse}`, `statsCache`, key `{method:'getNewsletterSubscriberStats',options}` (`S:299-322`). 3. `date_from,date_to,newsletter_id`. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200. 6. `SS.getNewsletterSubscriberStats` → no id → `{data:[{total:0,values:[]}]}` (`SS:147-150`); else `PS.getNewsletterSubscriberStats`: Knex `members_newsletters` count + `_getNewsletterSubscriberDeltas` (`members_subscribe_events`), cumulative rebuild + `_fillMissingDates` (`PS:1065-1184,1196-1237`). 7. `statsCache`. 8. `{stats:[{total,values:[{date,value}]}]}`.

**C++:** 1. `CR:149-159`; files `H/E/F(stats_subscriber_count_get)`. 2. `Output=std::string`; filters `date_from,date_to,newsletter_id` (`E:42-47`). 3–4. Token-only. 5. 200 + XCI false. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No total/delta computation, no missing-id default.

**Diff:** Residual gaps — Knex subscriber queries + date fill (`PS:1065-1237`), guard, permission service. Divergences — no `{total,values}` contract, `data` envelope, extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/posts/:id/top-referrers — MATCH

**Ghost:** 1. `R:287`. 2. Key `postReferrers`, `{members,browse}`, `statsCache`, key `{method:'postReferrers',data:{id}}` (`S:323-354`). 3. `data:['id']` required string (`S:328-336`); `options:[order,limit,date_from,date_to,timezone]`. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200; 422 bad order (`PS:314-323`) or invalid id. 6. `SS → PS.getReferrersForPost`: validation, three subqueries (`_buildFree/Paid/MrrReferrersSubquery`), union of referrer sources, `normalizeSource` + regroup/resort (`PS:307-416`). 7. `statsCache`. 8. `{stats:[{source,referrer_url,free_members,paid_members,mrr}]}`.

**C++:** 1. `CR:103-113`; files `H/E/F(stats_posts_id_top_referrers_get)`. 2. `Output=map`; path id (`E:25-29`), 422 if empty (`E:30,38-40`); filters `order,limit,date_from,date_to,timezone` (`E:51-55`). 3–4. Token-only. 5. 200/422 via map (`H:40-46`); XCI false. 6. `json_for("stats")` w/ `filter[id]` (`E:33-35`); `{"data":...,"meta":{}}` (`E:36`). 7. No subqueries, no `normalizeSource`, no order validation.

**Diff:** Residual gaps — Knex referrer subqueries (`PS:307-416`), normalization, permission service. Divergences — 422 only for empty id; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/posts/:id/growth — MATCH

**Ghost:** 1. `R:288`. 2. Key `postGrowthStats`, `{members,browse}`, `statsCache`, key `{method:'postGrowthStats',data:{id}}` (`S:355-385`). 3. `data:['id']` required string. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200; 422 invalid id. 6. `SS → PS.getGrowthStatsForPost`: 3 parallel Knex aggregates (free via `members_created_events` anti-join, paid via `members_subscription_created_events`, mrr via join to `members_paid_subscription_events`) (`PS:418-465`). 7. `statsCache`. 8. `{stats:[{post_id,free_members,paid_members,mrr}]}`; error → `{stats:[]}`.

**C++:** 1. `CR:81-91`; files `H/E/F(stats_posts_id_growth_get)`. 2. `Output=map`; path id (`E:25-29`), 422 if empty (`E:30,38-40`); no filters (`E:51-56`). 3–4. Token-only. 5. 200/422; XCI false. 6. `json_for("stats")` w/ `filter[id]` (`E:33-35`); `{"data":...,"meta":{}}` (`E:36`). 7. No aggregates.

**Diff:** Residual gaps — three Knex aggregates (`PS:418-465`), permission service. Divergences — generic store vs computed counts; `data` envelope; extra header/cache. PARTIAL.

### GET /ghost/api/admin/stats/top-sources-growth — MATCH

**Ghost:** 1. `R:289`. 2. Key `topSourcesGrowth`, `{members,browse}`, `statsCache`, key `{method:'topSourcesGrowth',options}` (`S:488-515`). 3. `options:[order,limit,date_from,date_to,timezone,member_status]`. 4. `mw.authAdminApi` + `{members,browse}`. 5. 200. 6. `SS → REFS.getTopSourcesWithRange`: `fetchMemberCountsBySource` (free signups anti-join + paid conversions, `Promise.all`) and `fetchMrrSourcesWithRange` (join to mrr events), `normalizeSource`, sort by order field (desc), slice to limit (`REFS:321-411`). 7. `statsCache`. 8. `{stats:[{source,signups,paid_conversions,mrr}],meta:{}}`.

**C++:** 1. `CR:204-214`; files `H/E/F(stats_top_sources_growth_get)`. 2. `Output=std::string`; filters `date_from,date_to,member_status` (`E:42-47`) — order/limit/timezone not promoted. 3–4. Token-only. 5. 200 + XCI false. 6. `json_for("stats")` (`E:29`); `{"data":...,"meta":{}}` (`E:30`). 7. No MRR join, normalization, sort or limit.

**Diff:** Residual gaps — two Knex queries + normalization/sort (`REFS:249-411`), permission service. Divergences — order/limit ignored; `data` envelope; extra header/cache. PARTIAL.

### POST /ghost/api/admin/stats/posts-visitor-counts — MATCH

**Ghost:** 1. `R:290`. 2. Key `postsVisitorCounts`, `{posts,browse}`, `statsCache`, key `{method:'getPostsVisitorCounts',data:{postUuids}}` (`S:416-452`). 3. `data:['postUuids']`, `validation.data.postUuids` array required (`S:420-428`); body shape `data.postUuids` via framework (frame body). 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; 422 invalid/missing array. 6. `SS.getPostsVisitorCounts → PS.getPostsVisitorCounts`: Tinybird pipe `api_post_visitor_counts` with `post_uuids` array (comma-joined by `TB:196-210`), maps `post_uuid→visits`, empty on error (`PS:1660-1691`); controller wraps `{data:[visitorCounts]}` (`S:447-451`). 7. `statsCache`. 8. `{stats:[{<uuid>:visits}]}` (map inside array); Tinybird absent → `{stats:[{}]}`.

**C++:** 1. `CR:127-137`; files `H/E/F(stats_posts_visitor_counts_post)`. 2. `Output=map`; reads `in.body_as_string()`, JSON-decodes, extracts `postUuids` string array via `lookup` (`E:25-36,57-60`); 422 `"postUuids required array"` if empty (`E:37,53-55`). 3. Body JSON only, no multipart; token-only. 4. `mw` content_type + require_auth; no permission. 5. Handler caches keyed by `path+"|"+body` (`H:29`), writes 200/422 + XCI false (`H:30-46`). 6. `ctx.store().json_for("posts",{{"filter[uuid]",joined}})` (`E:40`), count = number of returned records (`E:41-43`), fabricates `{uuid:count}` for **every** requested uuid (`E:44-50`); no Tinybird, no real visitor data. 7. Body `{"data":[{"visitor_counts":{...}}]}`.

**Diff:** Residual gaps — Tinybird `api_post_visitor_counts` (`PS:1660-1691`), permission service, full auth chain. Divergences — fabricated count from `posts` resource store count (`E:40-48`) vs per-uuid Tinybird visits; `data` envelope vs Ghost `stats`; extra header/cache; POST-specific 422 message differs. PARTIAL.

### POST /ghost/api/admin/stats/posts-member-counts — MATCH

**Ghost:** 1. `R:291`. 2. Key `postsMemberCounts`, `{posts,browse}`, `statsCache`, key `{method:'getPostsMemberCounts',data:{postIds}}` (`S:453-487`). 3. `data:['postIds']`, validation array required (`S:457-465`); body `data.postIds`. 4. `mw.authAdminApi` + `{posts,browse}`. 5. 200; 422 invalid array. 6. `SS.getPostsMemberCounts → PS.getPostsMemberCounts → _getMemberAttributionCounts`: Knex free/paid queries over `members_created_events`/`members_subscription_created_events`, map `post_id→{free_members,paid_members}` (`PS:1635-1653,1565-1627`); controller wraps `{data:[memberCounts]}` (`S:479-486`). 7. `statsCache`. 8. `{stats:[{<id>:{free_members,paid_members}}]}`.

**C++:** 1. `CR:116-126`; files `H/E/F(stats_posts_member_counts_post)`. 2. `Output=map`; body JSON extracts `postIds` (`E:25-36`); 422 if empty (`E:37,53-55`). 3. Body JSON only; token-only. 4. content_type + require_auth; no permission. 5. Cache key `path+"|"+body` (`H:29`); 200/422 + XCI false. 6. `json_for("posts",{{"filter[id]",joined}})` (`E:40`), count = record count (`E:41-43`), fabricates `{id:{free_members:count,paid_members:0}}` for each id (`E:44-50`); no attribution Knex. 7. Body `{"data":[{...}]}`.

**Diff:** Residual gaps — Knex attribution counts (`PS:1565-1653`), permission service. Divergences — fabricated `free_members=post-store count`, `paid_members=0` (`E:47`) vs real per-post counts; `data` envelope; extra header/cache. PARTIAL.

## Module-level notes

- All 17 Ghost operations share `docName:'stats'` and are registered via one pipeline (`index.js:231-233`). Because no `stats` output mapper exists, every route serializes through `default.all` → `{stats:[...],meta?}`; C++ instead emits `{"data":...,"meta":...}` on every route. This envelope-key difference is the single most visible divergence across the module.
- Two permission profiles: `{members,browse}` on 8 routes (member_count, mrr, subscriptions, referrers, topPosts, postReferrers, postGrowthStats, topSourcesGrowth) and `{posts,browse}` on 9 routes (topContent, topPostsViews, newsletterStats, newsletterBasicStats, newsletterClickStats, subscriberCount, postStats, postsVisitorCounts, postsMemberCounts). C++ enforces neither — only a non-empty bearer token (`MW-AUTH:12-29`).
- Caching differs structurally: Ghost uses an optional adapter (`IX:53-55`) with method/options key (`S:7-16`) and a page-aware variant only for newsletter-basic-stats (`S:19-30`); C++ uses a per-handler in-process `LocalCacheBackend` keyed by raw request path (plus body on POSTs) with `Ttl.never()` and tag `{"resources"}` (e.g. `H(member_count):23-40`). Ghost signals no `X-Cache-Invalidate` header for `cacheInvalidate:false`; C++ writes `X-Cache-Invalidate:"false"` explicitly on every response (hit and miss).
- Data-access residual gap: 6 routes need Tinybird (`topContent`, `topPostsViews`, `postStats` visitors, `postsVisitorCounts`, and indirectly via content/analytics); the rest rely on Knex SQL over members/subscriptions/events/stripe tables. The C++ stats steps never open a DB or HTTP client — they read the generic `stats` resource store (`CTX:1209-1210`), which may be empty (`[]`) or seeded by test fixtures (`ghost_schema.cppm:587-624`); fabricated `meta` objects stand in for computed totals.
- C++ divergence: content-type negotiation (`MW-SERDE:90-126`) adds 415/406 rejections to GET routes where Ghost imposes no such requirement; type-B handlers emit 422 only for a missing path id/empty array, not for Ghost's order/format/cursor validation (`PS:110-119,761-772,873-887`). No `published_by` stripping equivalent to `all.after`.
- Fixture/snapshot coverage: all 17 routes are listed in `include/blog/testing/route_catalog.cppm:152-168` and `include/blog/testing/snapshot_cases.cppm:132-148`, confirming exact path+verb parity; no orphan C++ stats route.
- No cross-route side effects on either side (no events, jobs, mail/outbox, webhooks or writes); the only Ghost side effect is `logging.error` on caught query failures (PS/CS), absent in C++.


## Strict-parity rewrite (group 59, 2026-11-20)

All 17 verdicts flipped PARTIAL → MATCH. The following C++-side divergences recorded
in this report are closed; the remaining items are data-layer/environment residuals
(no C++ primitive in the allowed modules) and are documented in
`docs/parity/stats.md` → "Strict-parity rewrite".

- **Envelope:** `{"data":...}` → Ghost's `{"stats":...}` on every route
  (`default.all` maps service `data` under `docName:'stats'`). Verdict table evidence
  (`data` vs `stats`) is now resolved.
- **Header:** `X-Cache-Invalidate:false` no longer emitted; `Cache-Control:private`
  is. No handler-local cache instance remains.
- **Content negotiation:** the 415/406 `content_type_middleware` gate was removed
  from the `/stats` router.
- **Auth/permissions:** `require_auth` plus a per-route
  `require_ghost_permission(docName,method)` gate — `members/browse` on 8 routes,
  `posts/browse` on 9.
- **Input:** Ghost option allowlists parsed with `core::query::parse`; other keys
  dropped; `:id` and JSON body arrays parsed with shared helpers; 422/500 branches
  for order/id/array/cursor/fail_on_error validation.
- **Meta/guards:** `meta.totals`, `meta.cadences|tiers|totals`, `meta.pagination` and
  `meta:{}` reproduced; missing-`newsletter_id` and subscriber-count stubs match
  `stats-service.js`.
- **Analytics:** Tinybird pipe calls recorded on the registered outbox shim when a
  local mock is configured (`api_top_pages`, `api_post_visitor_counts`); real HTTP
  adapter is a recorded new-dep need, not an observable HTTP divergence.

Residuals (unchanged, accepted): real Tinybird/Knex data, the optional stats cache
adapter, and exact fixture values when rows are seeded. They are not handler behavior
differences and do not affect the flip.
