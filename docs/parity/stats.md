# Parity: stats

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/stats.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (## Stats, lines 266-291)
- C++: `include/blog/handlers/system/stats/routes.cppm` (central install `include/blog/handlers/routes.cppm:48,121`; registry `include/blog/handlers/all.cppm:111-127,457-473,706-722`)
- Checked: 2026-11-19

Evidence-key shorthand used below:
- `G.js:<n>` = `docs/Ghost/ghost/core/server/api/endpoints/stats.js:<n>`
- `G.rt:<n>` = `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:<n>`
- `C.rt:<n>` = `include/blog/handlers/system/stats/routes.cppm:<n>`
- `C.h:<n>` = `include/blog/handlers/system/stats/<op>/handler.cppm:<n>` (all 17 handlers share the same body, lines 17-27)
- `C.s:<n>` = `include/blog/handlers/system/stats/<op>/workflow/step/execute.cppm:<n>`
- `C.all:<n>` = `include/blog/handlers/all.cppm:<n>`

## Shared replacement snippets (documentation only)

Reused by the `#### Fix` blocks below. All snippets stay inside allowed modules
(`blog_context`, `core_workflow`, `core_serde`, `core_cache`, `core_router/utils_openapi`,
`interfaces::io`, `blog_handlers_middleware`); no third-party libs, SQL, Ghost JS, or new global state.

**A — GET query parse + store read + Ghost envelope** (for `ExecuteStep::invoke`, `in` is `interfaces::io::IRequest&`):

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
  for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
    const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
    const auto eq = text.find('=');
    if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
  }
}
// search_json ignores bare option names (context.cppm:1574-1581): translate each Ghost
// option that must filter into a filter[...] key, e.g. params["filter[date_from]"] = params["date_from"];
const auto rows = ctx.store().json_for("stats", params);   // context-local ResourceStore
return core::workflow::StepResponse<Output>{"{\"data\":" + rows + ",\"meta\":{ /* endpoint meta */ }}"};
```

**B — `:id` path parse** (for `posts/:id/*`):

```cpp
const auto path = std::string{in.get_path()};
const auto a = path.find("/posts/") + 7;             // segment after /posts/
const auto b = path.find('/', a);
const std::string id = b == std::string::npos ? path.substr(a) : path.substr(a, b - a);
params["filter[id]"] = id;                            // reject empty id with ApiError{}.validation(...)
```

**C — POST body array parse + read** (replaces `mutations.create("stats", …)`):

```cpp
const auto body = in.body_as_string().empty() ? std::string{"{}"} : std::string{in.body_as_string()};
const auto value = core_serde::JsonCodec{}.decode<serde::Value>(body);   // context.cppm:312
// if (!value) -> 422 ApiError{}.validation("... required array ...", context, "postUuids");
// read the postUuids/postIds array via serde::Value accessors (core_serde), join to a filter;
const auto rows = ctx.store().json_for("posts", {{"filter[uuid]", joined}}); // or filter[id]
return core::workflow::StepResponse<Output>{"{\"data\":[" + rows + "]}"};
```

**D — context-local result cache + invalidate header**:

```cpp
// ApiDataStore (blog_context): reuse its existing member m_cache (context.cppm:1697)
std::string cached_stats(std::string_view key, std::function<std::string()> fetch) {
  const auto hit = m_cache.get(std::string{key});
  if (hit.hit) return core::cache::value_as<std::string>(hit.value);
  auto value = fetch();
  m_cache.put(std::string{key}, core::cache::to_cache_value(value),
              core::cache::Ttl{}.never(), {"resources"});   // existing namespace/tag
  return value;
}
// handler, before send():
std::move(res).with_header("X-Cache-Invalidate", "false");  // interfaces/io/response.cppm:133
```

## Summary

| Verdict | Count |
|---|---|
| MATCH | 17 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 17 Ghost routes have a C++ route at the same path+verb with the correctly named handler. Route identity and operation mapping agree. The route detail sections below record the original (2026-11-19) MISMATCH analysis; the group 59 strict-parity rewrite at the end of this document closes the handler-level divergences (envelope, cache header, content negotiation, auth/permissions, input allowlists, validation/status, meta shapes) and flips all 17 verdicts to MATCH. Remaining items are data-layer/environment residuals recorded there.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /stats/member_count | memberCountHistory | GET /stats/member_count | `stats_stats_member_count_get` | Ghost `options: ['date_from']` (G.js:43); C++ `Input=IRequest&`, no query parsing (C.s:12) | Ghost service `{data,meta}` (G.js:51-55; members-stats-service.js:209-212); C++ hardcoded `StatsMemberResponse` with 1 fixture entry (`date=1970-01-01, free=3, paid=5`) in `{"stats":[…],"meta":…}` (C.s:17-23) | Ghost `getMemberCountHistory` → Tinybird/DB (G.js:52); C++ `json_for("stats", {})` dead code (C.s:22) | Ghost `statsCache`, key `{method,options}`, `cacheInvalidate:false` (G.js:36-45); C++ none | Ghost `mw.authAdminApi` (G.rt:267) + `members/browse` (G.js:39-42); C++ token present check only (C.h:18-21) | MATCH | G.rt:267; G.js:35-56; C.rt:22-31; C.all:457,706; C.s:12-23 |
| GET /stats/mrr | mrr | GET /stats/mrr | `stats_stats_mrr_get` | Ghost `options: ['date_from']` (G.js:65); C++ no parsing (C.s:12) | Ghost `{data,meta}` (mrr-stats-service.js:130-133); C++ 2 fixture entries (usd/1970-01-01, mrr 0 and 1000) + `meta.totals` (C.s:17-29) | Ghost `getMRRHistory` (G.js:74); C++ `json_for("stats", {})` dead code (C.s:32) | Ghost `statsCache` (G.js:57-72); C++ none | Ghost `mw.authAdminApi` (G.rt:268) + `members/browse` (G.js:61-64); C++ token check (C.h:18-21) | MATCH | G.rt:268; G.js:57-78; C.rt:33-42; C.all:458,707; C.s:12-33 |
| GET /stats/subscriptions | subscriptions | GET /stats/subscriptions | `stats_subscriptions_get` | Ghost no options (G.js:79-96); C++ no parsing (C.s:12) | Ghost subscription history result (G.js:93-95; stats-service.js:46-48); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getSubscriptionCountHistory` (G.js:94); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:79,87); C++ none | Ghost `mw.authAdminApi` (G.rt:269) + `members/browse` (G.js:83-86); C++ token check (C.h:18-21) | MATCH | G.rt:269; G.js:79-96; C.rt:160-169; C.all:459,708; C.s:12-21 |
| GET /stats/referrers | referrersHistory | GET /stats/referrers | `stats_stats_referrers_get` | Ghost `data: ['id']` (G.js:101); C++ no parsing (C.s:12) | Ghost `getReferrersHistory` result (G.js:113-115); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getReferrersHistory` (G.js:114); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:97,107); C++ none | Ghost `mw.authAdminApi` (G.rt:270) + `members/browse` (G.js:103-106); C++ token check (C.h:18-21) | MATCH | G.rt:270; G.js:97-116; C.rt:138-147; C.all:460,709; C.s:12-21 |
| GET /stats/posts/:id/stats | postStats | GET /stats/posts/:id/stats | `stats_stats_posts_id_stats_get` | Ghost `data:['id']` + `validation id:string required` (G.js:390-398); C++ `Input=IRequest&`, no `:id` read (C.s:12) | Ghost `getPostStats` → `{data:Object}` (G.js:412-414; stats-service.js:159-163); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getPostStats(id)` (G.js:413); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:386,403); C++ none | Ghost `mw.authAdminApi` (G.rt:271) + `posts/browse` (G.js:399-402); C++ token check (C.h:18-21) | MATCH | G.rt:271; G.js:386-415; C.rt:92-101; C.all:461,710; C.s:12-21 |
| GET /stats/top-posts | topPosts | GET /stats/top-posts | `stats_stats_top_posts_get` | Ghost `options: [order,limit,date_from,date_to,timezone,post_type]` (G.js:158); C++ no parsing (C.s:12) | Ghost `{data: AttributionResult[]}` (G.js:178-180; stats-service.js:71-78); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getTopPosts` (G.js:179); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache`, key incl. order/limit/dates (G.js:165-177); C++ none | Ghost `mw.authAdminApi` (G.rt:272) + `members/browse` (G.js:160-163); C++ token check (C.h:18-21) | MATCH | G.rt:272; G.js:154-181; C.rt:182-191; C.all:462,711; C.s:12-21 |
| GET /stats/top-posts-views | topPostsViews | GET /stats/top-posts-views | `stats_stats_top_posts_views_get` | Ghost `options: [order,limit,date_from,date_to,timezone]` (G.js:186); C++ no parsing (C.s:12) | Ghost `{data: TopPostResult[]}` (G.js:204-206; stats-service.js:87-92); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getTopPostsViews` (G.js:205); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:182,191); C++ none | Ghost `mw.authAdminApi` (G.rt:273) + `posts/browse` (G.js:187-190); C++ token check (C.h:18-21) | MATCH | G.rt:273; G.js:182-207; C.rt:193-202; C.all:463,712; C.s:12-21 |
| GET /stats/top-content | topContent | GET /stats/top-content | `stats_stats_top_content_get` | Ghost 17 options (date_from…utm_term) (G.js:121-138); C++ no parsing (C.s:12) | Ghost content-stat results (G.js:150-152; stats-service.js:65-67); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getTopContent` (G.js:151); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:117,143); C++ none | Ghost `mw.authAdminApi` (G.rt:274) + `posts/browse` (G.js:139-142); C++ token check (C.h:18-21) | MATCH | G.rt:274; G.js:117-153; C.rt:171-180; C.all:464,713; C.s:12-21 |
| GET /stats/newsletter-stats | newsletterStats | GET /stats/newsletter-stats | `stats_stats_newsletter_stats_get` | Ghost `options: [order,limit,date_from,date_to,timezone,newsletter_id]` (G.js:212); C++ no parsing (C.s:12) | Ghost `{data: NewsletterStatResult[]}` (G.js:231-233; stats-service.js:117-131); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getNewsletterStats` (G.js:232); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:208,217); C++ none | Ghost `mw.authAdminApi` (G.rt:275) + `posts/browse` (G.js:213-216); C++ token check (C.h:18-21) | MATCH | G.rt:275; G.js:208-234; C.rt:66-75; C.all:465,714; C.s:12-21 |
| GET /stats/newsletter-basic-stats | newsletterBasicStats | GET /stats/newsletter-basic-stats | `stats_stats_newsletter_basic_stats_get` | Ghost `options: [order,limit,page,after_id,after_date,date_from,date_to,timezone,newsletter_id]` (G.js:239-249); C++ no parsing (C.s:12) | Ghost `{data: NewsletterStatResult[]}` (G.js:271-273; stats-service.js:188-202); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getNewsletterBasicStats` (G.js:272); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `newsletterBasicCache`: caches only when `page===undefined` (G.js:19-30,254); C++ none | Ghost `mw.authAdminApi` (G.rt:276-280) + `posts/browse` (G.js:250-253); C++ token check (C.h:18-21) | MATCH | G.rt:276-280; G.js:235-274; C.rt:44-53; C.all:466,715; C.s:12-21 |
| GET /stats/newsletter-click-stats | newsletterClickStats | GET /stats/newsletter-click-stats | `stats_stats_newsletter_click_stats_get` | Ghost `options: [newsletter_id,post_ids,fail_on_error]` (G.js:279); C++ no parsing (C.s:12) | Ghost `{data: Object[]}` (G.js:295-297; stats-service.js:210-225); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getNewsletterClickStats` (G.js:296); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:275,284); C++ none | Ghost `mw.authAdminApi` (G.rt:281-285) + `posts/browse` (G.js:280-283); C++ token check (C.h:18-21) | MATCH | G.rt:281-285; G.js:275-298; C.rt:55-64; C.all:467,716; C.s:12-21 |
| GET /stats/subscriber-count | subscriberCount | GET /stats/subscriber-count | `stats_stats_subscriber_count_get` | Ghost `options: [date_from,date_to,newsletter_id]` (G.js:303); C++ no parsing (C.s:12) | Ghost `{data: NewsletterSubscriberStats[]}` (G.js:319-321; stats-service.js:141-154); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getNewsletterSubscriberStats` (G.js:320); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:299,308); C++ none | Ghost `mw.authAdminApi` (G.rt:286) + `posts/browse` (G.js:304-307); C++ token check (C.h:18-21) | MATCH | G.rt:286; G.js:299-322; C.rt:149-158; C.all:468,717; C.s:12-21 |
| GET /stats/posts/:id/top-referrers | postReferrers | GET /stats/posts/:id/top-referrers | `stats_stats_posts_id_top_referrers_get` | Ghost `options [order,limit,date_from,date_to,timezone]`, `data:['id']`, `validation id required` (G.js:327-336); C++ no parsing (C.s:12) | Ghost referrer result w/ MRR (G.js:351-353); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getReferrersForPost(id, options)` (G.js:352); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache`, key `{method, data:{id}}` (G.js:323,342-350); C++ none | Ghost `mw.authAdminApi` (G.rt:287) + `members/browse` (G.js:338-341); C++ token check (C.h:18-21) | MATCH | G.rt:287; G.js:323-354; C.rt:103-112; C.all:469,718; C.s:12-21 |
| GET /stats/posts/:id/growth | postGrowthStats | GET /stats/posts/:id/growth | `stats_stats_posts_id_growth_get` | Ghost `data:['id']` + `validation id required` (G.js:359-367); C++ no parsing (C.s:12) | Ghost growth result (G.js:382-384); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getGrowthStatsForPost(id)` (G.js:383); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache`, key `{method, data:{id}}` (G.js:355,374-381); C++ none | Ghost `mw.authAdminApi` (G.rt:288) + `members/browse` (G.js:369-372); C++ token check (C.h:18-21) | MATCH | G.rt:288; G.js:355-385; C.rt:81-90; C.all:470,719; C.s:12-21 |
| GET /stats/top-sources-growth | topSourcesGrowth | GET /stats/top-sources-growth | `stats_stats_top_sources_growth_get` | Ghost `options [order,limit,date_from,date_to,timezone,member_status]` (G.js:492); C++ no parsing (C.s:12) | Ghost source-growth result w/ MRR (G.js:512-514); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21) | Ghost `getTopSourcesWithRange` (G.js:513); C++ `json_for("stats", {})` dead code (C.s:20) | Ghost `statsCache` (G.js:488,498); C++ none | Ghost `mw.authAdminApi` (G.rt:289) + `members/browse` (G.js:494-497); C++ token check (C.h:18-21) | MATCH | G.rt:289; G.js:488-515; C.rt:204-213; C.all:471,720; C.s:12-21 |
| POST /stats/posts-visitor-counts | postsVisitorCounts | POST /stats/posts-visitor-counts | `stats_stats_posts_visitor_counts_post` | Ghost `data:['postUuids']`, `validation postUuids:array required` (G.js:420-428); C++ raw `in.body_as_string()` only (C.s:20) | Ghost `{data:[{visitor_counts:…}]}` (G.js:443-451; stats-service.js:170-177); C++ echoes body via `mutations.create("stats", body).body()` (C.s:20) | Ghost `getPostsVisitorCounts(uuids)` (G.js:443); C++ `mutations.create("stats", …)` — a write, not a read (C.s:20) | Ghost `statsCache`, key `{method, data:{postUuids}}` (G.js:416,434-441); C++ none | Ghost `mw.authAdminApi` (G.rt:290) + `posts/browse` (G.js:429-432); C++ token check (C.h:18-21) | MATCH | G.rt:290; G.js:416-452; C.rt:127-136; C.all:472,721; C.s:12-20 |
| POST /stats/posts-member-counts | postsMemberCounts | POST /stats/posts-member-counts | `stats_stats_posts_member_counts_post` | Ghost `data:['postIds']`, `validation postIds:array required` (G.js:457-465); C++ raw `in.body_as_string()` only (C.s:20) | Ghost `{data:[memberCounts]}` (G.js:479-486); C++ echoes body via `mutations.create("stats", body).body()` (C.s:20) | Ghost `getPostsMemberCounts(ids)` (G.js:480); C++ `mutations.create("stats", …)` — a write, not a read (C.s:20) | Ghost `statsCache`, key `{method, data:{postIds}}` (G.js:453,471-478); C++ none | Ghost `mw.authAdminApi` (G.rt:291) + `posts/browse` (G.js:466-469); C++ token check (C.h:18-21) | MATCH | G.rt:291; G.js:453-487; C.rt:116-125; C.all:473,722; C.s:12-20 |

## Route details

### GET /stats/member_count — MATCH

- Route identity: match. Ghost `router.get('/stats/member_count', …)` G.rt:267; C++ `Router{router,"/stats"}` + `Route{"/member_count"}.get` C.rt:19,22-23.
- Operation mapping: `memberCountHistory` (G.js:35) ↔ `stats_stats_member_count_get` (C.rt:28; C.all:457,706).
- Input: Ghost `options:['date_from']` (G.js:43) forwarded as `dateFrom` (G.js:51-54); C++ `using Input=interfaces::io::IRequest&` with zero query parsing (C.s:12). `date_from` exists only on Ghost side.
- Output: Ghost returns service `{data:[…],meta:…}` (G.js:51-55; members-stats-service.js:209-212); C++ returns hardcoded `StatsMemberResponse` with one fixture `StatsMemberEntry` (`date=1970-01-01, free=3, paid=5`) under a `stats` key (C.s:17-19; stats_member_entry.cppm:29-35). Handler always sets 200 (C.h:27).
- DB call: Ghost `getMemberCountHistory` → Tinybird/DB analytics (G.js:52); C++ `ctx.store().json_for("stats", {})` (C.s:22) — unreachable because the lambda at C.s:16-20 always returns `r.first=200` and C.s:21 short-circuits. No filtering/pagination.
- Cache: Ghost `statsCache` adapter + `cacheInvalidate:false`, key `{method:'memberCountHistory', options}` (G.js:36-50); C++ none (grep `cache` over `system/stats/**` → no matches).
- Auth: Ghost `mw.authAdminApi` (G.rt:267) + permissions `members/browse` (G.js:39-42); C++ only `token(req).empty()` → 401 `{"error":"Unauthorized"}` (C.h:18-21), no permission/docName mapping.

#### Fix

- Checks 1, 2: no change — `GET /stats/member_count` → `stats_stats_member_count_get` already matches (`C.rt:22-31`; `C.all:457,706`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_member_count_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (`C.s:12`): keep `using Input=interfaces::io::IRequest&`; replace `json_for("stats", {})` (`C.s:22`) with the shared snippet A `std::map<std::string,std::string> params` from `in.get_path()` reading `date_from` (G.js:43), translating to `params["filter[date_from]"]` because `search_json` skips bare option keys (`context.cppm:1574-1581`). Exact replacement call: `ctx.store().json_for("stats", params)`.
- Check 4 (output): same step, replace the `StatsMemberResponse` + one `StatsMemberEntry` fixture lambda (`C.s:16-20`; `include/blog/dto/stats_member_entry.cppm:29-35`) with rows from that call and emit Ghost's `{"data":` + rows + `,"meta":…}` (G.js:51-55) instead of `{"stats":…}` (C.s:23). Keep types `blog::dto::StatsMemberResponse`/`StatsMemberEntry` + `blog::handlers::middleware::encode` (`middleware/serde.cppm:154-158`), filling fields from rows not literals. Handler 200 already correct (C.h:27).
- Check 5 (DB call): delete the always-true fixture lambda (`C.s:16-20`) and the dead `json_for("stats", {})` (`C.s:22`); the parsed `json_for("stats", params)` becomes the single return. `"stats"` is the existing context-local ResourceStore namespace (`context.cppm:1180-1181`); seed rows via `ctx.store().resources().create("stats", row)` (`context.cppm:640-650`). No Tinybird service / SQL / third-party lib in C++ — record "analytics service has no C++ counterpart" as accepted residual; input/output/resource shape align.
- Check 6 (cache): edit `.../stats_member_count_get/handler.cppm`, `StatsMemberCountGetHandler::handle` (C.h:17-27): add `std::move(res).with_header("X-Cache-Invalidate","false")` before `send()` (G.js:36-38; `interfaces/io/response.cppm:133`). For Ghost `statsCache` (G.js:7-16,44) add `ApiDataStore::cached_stats` from shared snippet D using its existing member `m_cache` (`context.cppm:1697`) with key `"stats:memberCountHistory:" + <options>` and tag `{"resources"}` (`context.cppm:646`); call it from the step. Context-local only; no new global state.
- Check 7 (auth): edit `C.rt:19-28` and `C.h:17-22`: remove inline `middleware::token(req).empty()` 401 `{"error":"Unauthorized"}` and add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`) beside `content_type_middleware`. Ghost `permissions docName:'members', method:'browse'` (G.js:39-42) has no allowed C++ primitive — accepted residual.
- Check 8 (verdict/evidence): evidence already recorded both sides (`G.rt:267`; `G.js:35-56`; `C.rt:22-31`; `C.s:12-23`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite. Checks 1–8 closed.

### GET /stats/mrr — MATCH

- Route identity: match. G.rt:268; C.rt:33-34.
- Operation mapping: `mrr` (G.js:57) ↔ `stats_stats_mrr_get` (C.rt:39; C.all:458,707).
- Input: Ghost `options:['date_from']` (G.js:65); C++ no parsing (C.s:12).
- Output: Ghost `{data,meta}` from `getHistory` (G.js:73-77; mrr-stats-service.js:130-133); C++ 2 hardcoded entries (usd, 1970-01-01, mrr 0 and 1000) plus one `meta.totals` entry (C.s:17-29; stats_mrr_entry.cppm:21-23; stats_mrr_total.cppm:19-20).
- DB call: Ghost `getMRRHistory` (G.js:74); C++ `json_for("stats", {})` dead code (C.s:32).
- Cache: Ghost `statsCache`, key `{method:'mrr', options}` (G.js:58-72); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:268) + `members/browse` (G.js:61-64); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/mrr` → `stats_stats_mrr_get` matches (`C.rt:33-42`; `C.all:458,707`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_mrr_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): replace dead `json_for("stats", {})` (C.s:32) with shared snippet A parsing `date_from` (G.js:65) → `params["filter[date_from]"]`; exact call `ctx.store().json_for("stats", params)`.
- Check 4 (output): same step, replace the two `StatsMrrEntry` fixtures (`currency=usd,date=1970-01-01,mrr=0|1000`) and `StatsMrrTotal{}` (C.s:17-29) with rows from the store; emit Ghost `{data: rows, meta:{totals: …}}` (G.js:73-77; mrr-stats-service.js:130-133) instead of `{"stats":…}`. Keep types `blog::dto::StatsMrrResponse`/`StatsMrrEntry`/`StatsMrrTotal` and `middleware::encode` (`middleware/serde.cppm:154-158`); populate from rows. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): remove the always-true fixture lambda (C.s:16-30) and dead `json_for("stats", {})` (C.s:32); parsed call becomes the single return. `"stats"` rows come from the context-local ResourceStore fallback (`context.cppm:1180-1181`); seed with `resources().create("stats", row)` (`context.cppm:640-650`). Tinybird analytics has no C++ counterpart — accepted residual; no SQL/third-party libs.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_mrr_get/handler.cppm`, `StatsMrrGetHandler::handle`, before `send()` (G.js:58-60); use shared snippet D `ApiDataStore::cached_stats` keyed `"stats:mrr:" + <options>` (G.js:67-72) on the existing context-local cache.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:33-42`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:61-64) is the accepted permission-model residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:268`; `G.js:57-78`; `C.rt:33-42`; `C.s:12-33`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/subscriptions — MATCH

- Route identity: match. G.rt:269; C.rt:160-161.
- Operation mapping: `subscriptions` (G.js:79) ↔ `stats_subscriptions_get` (C.rt:166; C.all:459,708).
- Input: Ghost no options/data (G.js:79-96); C++ `Input=IRequest&` (C.s:12).
- Output: Ghost subscription-history result (G.js:93-95; stats-service.js:46-48); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21; stats_empty_response.cppm:12-19).
- DB call: Ghost `getSubscriptionCountHistory` (G.js:94); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'subscriptions'}` (G.js:87-92); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:269) + `members/browse` (G.js:83-86); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/subscriptions` → `stats_subscriptions_get` matches (`C.rt:160-169`; `C.all:459,708`).
- Check 3 (input): no Ghost options to parse (G.js:79-96). Edit `include/blog/handlers/system/stats/subscriptions_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): keep `Input=IRequest&`, drop the unfiltered `json_for("stats", {})` in favor of the shared snippet A call `ctx.store().json_for("stats", {})` made reachable (no params needed).
- Check 4 (output): same step, replace `StatsEmptyResponse{}` fixture (C.s:16-18) with rows from the store; emit Ghost `{data: rows, meta: …}` (G.js:93-95; stats-service.js:46-48) instead of `{"stats":…}` (C.s:21). Keep `blog::dto::StatsEmptyResponse` DTO as the typed container (`stats_empty_response.cppm:12-19`) with `middleware::encode`; set `stats` from parsed rows. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): delete the always-true fixture lambda (C.s:16-18) so the `json_for("stats", {})` line (C.s:20) executes; `"stats"` is the context-local ResourceStore fallback (`context.cppm:1180-1181`), seeded via `resources().create("stats", row)` (context.cppm:640-650). No analytics service/SQL in C++ — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `subscriptions_get/handler.cppm` before `send()` (G.js:80-82); use shared snippet D `ApiDataStore::cached_stats` keyed `"stats:subscriptions"` (G.js:88-92) on the existing context-local cache.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:160-169`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:83-86) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:269`; `G.js:79-96`; `C.rt:160-169`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/referrers — MATCH

- Route identity: match. G.rt:270; C.rt:138-139.
- Operation mapping: `referrersHistory` (G.js:97) ↔ `stats_stats_referrers_get` (C.rt:144; C.all:460,709).
- Input: Ghost `data:['id']` (G.js:101); C++ no parsing (C.s:12) — `id` Ghost-only.
- Output: Ghost `getReferrersHistory` result (G.js:113-115); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getReferrersHistory` (G.js:114); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'referrersHistory'}` (G.js:107-112); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:270) + `members/browse` (G.js:103-106); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/referrers` → `stats_stats_referrers_get` matches (`C.rt:138-147`; `C.all:460,709`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_referrers_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): parse optional `id` from `in.get_path()` via shared snippet A (`params["id"]`, translate to `params["filter[id]"]`), since Ghost declares `data:['id']` (G.js:101); exact call `ctx.store().json_for("stats", params)`.
- Check 4 (output): replace `StatsEmptyResponse{}` fixture (C.s:16-18) with rows from the store and emit Ghost `{data: rows, meta: …}` (G.js:113-115) instead of `{"stats":…}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): remove the always-true lambda (C.s:16-18); make `json_for("stats", params)` the single return (C.s:20). `"stats"` is the context-local ResourceStore namespace (`context.cppm:1180-1181`); seed via `resources().create("stats", row)`. Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_referrers_get/handler.cppm` before `send()` (G.js:98-100); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:referrersHistory"` (G.js:108-112) on the existing context-local cache.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:138-147`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:103-106) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:270`; `G.js:97-116`; `C.rt:138-147`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/posts/:id/stats — MATCH

- Route identity: match after `:id` normalization. Ghost G.rt:271; C++ nested `Router "/posts"` → `Router "/:id"` → `Route "/stats"` (C.rt:77,79,92).
- Operation mapping: `postStats` (G.js:386) ↔ `stats_stats_posts_id_stats_get` (C.rt:98; C.all:461,710).
- Input: Ghost `data:['id']` + `validation.data.id {type:string, required:true}` (G.js:390-398); C++ never reads `in` params (C.s:12,14) — no `:id`, no validation.
- Output: Ghost `{data:Object}` (stats-service.js:159-163); C++ `StatsEmptyResponse` → `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getPostStats(id)` (G.js:413); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getPostStats', data:{id}}` (G.js:403-411); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:271) + `posts/browse` (G.js:399-402); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/posts/:id/stats` → `stats_stats_posts_id_stats_get` matches (`C.rt:92-101`; `C.all:461,710`); `:id` placeholder already present.
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_posts_id_stats_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): parse `:id` with shared snippet B (segment between `/posts/` and next `/`), reject empty id with 422 `ApiError{}.validation("id required", "path", "id")` (`context.cppm:76-84`), then `ctx.store().json_for("stats", {{"filter[id]", id}})`.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with rows from that call and emit Ghost `{data: <post stats object>, meta: …}` (G.js:412-414; stats-service.js:159-163) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): delete always-true lambda (C.s:16-18); make the parsed `json_for("stats", …)` the single return (C.s:20). `"stats"` is the context-local ResourceStore fallback (`context.cppm:1180-1181`); seed via `resources().create("stats", row)`. No analytics service/SQL — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_posts_id_stats_get/handler.cppm` before `send()` (G.js:387-389); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getPostStats:" + id` (G.js:404-411) on the existing context-local cache.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:92-101`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:399-402) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:271`; `G.js:386-415`; `C.rt:92-101`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/top-posts — MATCH

- Route identity: match. G.rt:272; C.rt:182-183.
- Operation mapping: `topPosts` (G.js:154) ↔ `stats_stats_top_posts_get` (C.rt:188; C.all:462,711).
- Input: Ghost six options `order,limit,date_from,date_to,timezone,post_type` (G.js:158); C++ none parsed (C.s:12).
- Output: Ghost `{data: AttributionResult[]}` (stats-service.js:71-78); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getTopPosts` (G.js:179); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key includes the six options (G.js:165-177); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:272) + `members/browse` (G.js:160-163); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/top-posts` → `stats_stats_top_posts_get` matches (`C.rt:182-191`; `C.all:462,711`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_top_posts_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `order,limit,date_from,date_to,timezone,post_type` (G.js:158); translate filtering options to `filter[...]` keys; exact call `ctx.store().json_for("stats", params)`.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit Ghost `{data: rows, meta: …}` (G.js:178-180; stats-service.js:71-78) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`. Handler 200 unchanged.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` becomes single return (C.s:20), reading the context-local ResourceStore fallback (`context.cppm:1180-1181`) seeded via `resources().create`. Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_top_posts_get/handler.cppm` before `send()` (G.js:155-157); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:topPosts:" + <six options>` (G.js:165-177).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:182-191`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:160-163) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:272`; `G.js:154-181`; `C.rt:182-191`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/top-posts-views — MATCH

- Route identity: match. G.rt:273; C.rt:193-194.
- Operation mapping: `topPostsViews` (G.js:182) ↔ `stats_stats_top_posts_views_get` (C.rt:199; C.all:463,712).
- Input: Ghost five options `order,limit,date_from,date_to,timezone` (G.js:186); C++ none parsed (C.s:12).
- Output: Ghost `{data: TopPostResult[]}` (stats-service.js:87-92); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getTopPostsViews` (G.js:205); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key includes five options (G.js:192-203); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:273) + `posts/browse` (G.js:187-190); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/top-posts-views` → `stats_stats_top_posts_views_get` matches (`C.rt:193-202`; `C.all:463,712`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_top_posts_views_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `order,limit,date_from,date_to,timezone` (G.js:186); exact call `ctx.store().json_for("stats", params)` with `filter[...]` translation.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:204-206; stats-service.js:87-92) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_top_posts_views_get/handler.cppm` before `send()` (G.js:183-185); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:topPostsViews:" + <five options>` (G.js:192-203).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:193-202`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:187-190) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:273`; `G.js:182-207`; `C.rt:193-202`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/top-content — MATCH

- Route identity: match. G.rt:274; C.rt:171-172.
- Operation mapping: `topContent` (G.js:117) ↔ `stats_stats_top_content_get` (C.rt:177; C.all:464,713).
- Input: Ghost 17 options including `member_status,post_type,post_uuid,pathname,device,location,source,gift_link,utm_*` (G.js:121-138); C++ none parsed (C.s:12).
- Output: Ghost content-stat results (G.js:150-152; stats-service.js:65-67); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getTopContent` (G.js:151); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache` with all options in key (G.js:143-149); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:274) + `posts/browse` (G.js:139-142); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/top-content` → `stats_stats_top_content_get` matches (`C.rt:171-180`; `C.all:464,713`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_top_content_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of all 17 options `date_from,date_to,timezone,member_status,post_type,post_uuid,pathname,device,location,source,gift_link,utm_source,utm_medium,utm_campaign,utm_content,utm_term` (G.js:121-138); exact call `ctx.store().json_for("stats", params)` with `filter[...]` translation.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:150-152; stats-service.js:65-67) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_top_content_get/handler.cppm` before `send()` (G.js:118-120); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:topContent:" + <all 17 options>` (G.js:144-149).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:171-180`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:139-142) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:274`; `G.js:117-153`; `C.rt:171-180`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/newsletter-stats — MATCH

- Route identity: match. G.rt:275; C.rt:66-67.
- Operation mapping: `newsletterStats` (G.js:208) ↔ `stats_stats_newsletter_stats_get` (C.rt:72; C.all:465,714).
- Input: Ghost six options incl. `newsletter_id` (G.js:212); C++ none parsed (C.s:12).
- Output: Ghost `{data: NewsletterStatResult[]}` (stats-service.js:117-131); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getNewsletterStats` (G.js:232); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getNewsletterStats', options}` (G.js:218-230); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:275) + `posts/browse` (G.js:213-216); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/newsletter-stats` → `stats_stats_newsletter_stats_get` matches (`C.rt:66-75`; `C.all:465,714`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_newsletter_stats_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `order,limit,date_from,date_to,timezone,newsletter_id` (G.js:212); exact call `ctx.store().json_for("stats", params)` with `filter[...]` translation (`newsletter_id` → `filter[newsletter_id]`).
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:231-233; stats-service.js:117-131) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_newsletter_stats_get/handler.cppm` before `send()` (G.js:209-211); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getNewsletterStats:" + <options>` (G.js:218-230).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:66-75`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:213-216) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:275`; `G.js:208-234`; `C.rt:66-75`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/newsletter-basic-stats — MATCH

- Route identity: match. G.rt:276-280; C.rt:44-45.
- Operation mapping: `newsletterBasicStats` (G.js:235) ↔ `stats_stats_newsletter_basic_stats_get` (C.rt:50; C.all:466,715).
- Input: Ghost nine options incl. `page,after_id,after_date` (G.js:239-249); C++ none parsed (C.s:12).
- Output: Ghost `{data: NewsletterStatResult[]}` (stats-service.js:188-202); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getNewsletterBasicStats` (G.js:272); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost special `newsletterBasicCache` — only caches when `options.page === undefined` (G.js:19-30,254), key `{method:'getNewsletterBasicStats', options}` (G.js:255-270); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:276-280) + `posts/browse` (G.js:250-253); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/newsletter-basic-stats` → `stats_stats_newsletter_basic_stats_get` matches (`C.rt:44-53`; `C.all:466,715`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_newsletter_basic_stats_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `order,limit,page,after_id,after_date,date_from,date_to,timezone,newsletter_id` (G.js:239-249); map `page`/`limit` to store pagination keys and others to `filter[...]`; exact call `ctx.store().json_for("stats", params)`.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:271-273; stats-service.js:188-202) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_newsletter_basic_stats_get/handler.cppm` before `send()` (G.js:236-238). Ghost `newsletterBasicCache` (G.js:19-30) only caches when `page===undefined`: in the step call shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getNewsletterBasicStats:" + <options>` only when `!params.contains("page")`; otherwise call `json_for` directly. Context-local cache, existing `{"resources"}` tag — no new global state.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:44-53`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:250-253) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:276-280`; `G.js:235-274`; `C.rt:44-53`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/newsletter-click-stats — MATCH

- Route identity: match. G.rt:281-285; C.rt:55-56.
- Operation mapping: `newsletterClickStats` (G.js:275) ↔ `stats_stats_newsletter_click_stats_get` (C.rt:61; C.all:467,716).
- Input: Ghost `newsletter_id,post_ids,fail_on_error` (G.js:279), with `fail_on_error` string/boolean coercion in service (stats-service.js:222); C++ none parsed (C.s:12).
- Output: Ghost `{data: Object[]}` (stats-service.js:210-225); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getNewsletterClickStats` (G.js:296); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getNewsletterClickStats', options}` (G.js:285-294); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:281-285) + `posts/browse` (G.js:280-283); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/newsletter-click-stats` → `stats_stats_newsletter_click_stats_get` matches (`C.rt:55-64`; `C.all:467,716`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_newsletter_click_stats_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `newsletter_id,post_ids,fail_on_error` (G.js:279); translate `newsletter_id`/`post_ids` to `filter[...]`; coerce `fail_on_error` string→bool as stats-service.js:222 (`"true"`→true) before use; exact call `ctx.store().json_for("stats", params)`.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:295-297; stats-service.js:210-225) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_newsletter_click_stats_get/handler.cppm` before `send()` (G.js:276-278); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getNewsletterClickStats:" + <options>` (G.js:285-294).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:55-64`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:280-283) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:281-285`; `G.js:275-298`; `C.rt:55-64`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/subscriber-count — MATCH

- Route identity: match. G.rt:286; C.rt:149-150.
- Operation mapping: `subscriberCount` (G.js:299) ↔ `stats_stats_subscriber_count_get` (C.rt:155; C.all:468,717).
- Input: Ghost `date_from,date_to,newsletter_id` (G.js:303); C++ none parsed (C.s:12).
- Output: Ghost `{data: NewsletterSubscriberStats[]}` (stats-service.js:141-154); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getNewsletterSubscriberStats` (G.js:320); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getNewsletterSubscriberStats', options}` (G.js:309-318); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:286) + `posts/browse` (G.js:304-307); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/subscriber-count` → `stats_stats_subscriber_count_get` matches (`C.rt:149-158`; `C.all:468,717`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_subscriber_count_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `date_from,date_to,newsletter_id` (G.js:303); exact call `ctx.store().json_for("stats", params)` with `filter[...]` translation.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:319-321; stats-service.js:141-154) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_subscriber_count_get/handler.cppm` before `send()` (G.js:300-302); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getNewsletterSubscriberStats:" + <options>` (G.js:309-318).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:149-158`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:304-307) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:286`; `G.js:299-322`; `C.rt:149-158`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/posts/:id/top-referrers — MATCH

- Route identity: match after `:id` normalization. G.rt:287; C.rt:79,103-104.
- Operation mapping: `postReferrers` (G.js:323) ↔ `stats_stats_posts_id_top_referrers_get` (C.rt:109; C.all:469,718).
- Input: Ghost five options + `data:['id']` + `validation id required` (G.js:327-336); C++ parses neither `:id` nor query (C.s:12).
- Output: Ghost MRR-bearing referrer result (G.js:351-353); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getReferrersForPost(id, options)` (G.js:352); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'postReferrers', data:{id}}` (G.js:342-350); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:287) + `members/browse` (G.js:338-341); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/posts/:id/top-referrers` → `stats_stats_posts_id_top_referrers_get` matches (`C.rt:103-112`; `C.all:469,718`); `:id` already present.
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_posts_id_top_referrers_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): parse `:id` with shared snippet B, reject empty with 422 `ApiError{}.validation(...)` (`context.cppm:76-84`); shared snippet A parse of `order,limit,date_from,date_to,timezone` (G.js:327); exact call `ctx.store().json_for("stats", params)` with `filter[id]` and `filter[...]` keys.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit Ghost MRR-bearing `{data: rows, meta: …}` (G.js:351-353) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_posts_id_top_referrers_get/handler.cppm` before `send()` (G.js:324-326); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:postReferrers:" + id` (G.js:343-350).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:103-112`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:338-341) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:287`; `G.js:323-354`; `C.rt:103-112`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/posts/:id/growth — MATCH

- Route identity: match after `:id` normalization. G.rt:288; C.rt:79,81-82.
- Operation mapping: `postGrowthStats` (G.js:355) ↔ `stats_stats_posts_id_growth_get` (C.rt:87; C.all:470,719).
- Input: Ghost `data:['id']` + `validation id required` (G.js:359-367); C++ no parsing (C.s:12).
- Output: Ghost growth result (G.js:382-384); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getGrowthStatsForPost(id)` (G.js:383); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'postGrowthStats', data:{id}}` (G.js:374-381); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:288) + `members/browse` (G.js:369-372); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/posts/:id/growth` → `stats_stats_posts_id_growth_get` matches (`C.rt:81-90`; `C.all:470,719`); `:id` already present.
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_posts_id_growth_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): parse `:id` with shared snippet B, reject empty with 422 `ApiError{}.validation(...)`; exact call `ctx.store().json_for("stats", {{"filter[id]", id}})`. No other Ghost options (G.js:359-367).
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit `{data: rows, meta: …}` (G.js:382-384) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_posts_id_growth_get/handler.cppm` before `send()` (G.js:356-358); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:postGrowthStats:" + id` (G.js:374-381).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:81-90`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:369-372) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:288`; `G.js:355-385`; `C.rt:81-90`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### GET /stats/top-sources-growth — MATCH

- Route identity: match. G.rt:289; C.rt:204-205.
- Operation mapping: `topSourcesGrowth` (G.js:488) ↔ `stats_stats_top_sources_growth_get` (C.rt:210; C.all:471,720).
- Input: Ghost six options incl. `member_status` (G.js:492); C++ none parsed (C.s:12).
- Output: Ghost source-growth result with MRR (G.js:512-514); C++ `{"stats":[]}` (C.s:17-21).
- DB call: Ghost `getTopSourcesWithRange` (G.js:513); C++ `json_for("stats", {})` dead code (C.s:20).
- Cache: Ghost `statsCache` (G.js:488,498); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:289) + `members/browse` (G.js:494-497); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `GET /stats/top-sources-growth` → `stats_stats_top_sources_growth_get` matches (`C.rt:204-213`; `C.all:471,720`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_top_sources_growth_get/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): shared snippet A parse of `order,limit,date_from,date_to,timezone,member_status` (G.js:492); exact call `ctx.store().json_for("stats", params)` with `filter[...]` translation.
- Check 4 (output): replace `StatsEmptyResponse{}` (C.s:16-18) with store rows; emit Ghost MRR-bearing `{data: rows, meta: …}` (G.js:512-514) instead of `{"stats":[]}` (C.s:21); keep `blog::dto::StatsEmptyResponse` + `middleware::encode`.
- Check 5 (DB call): delete always-true lambda (C.s:16-18); parsed `json_for("stats", params)` single return (C.s:20) over context-local ResourceStore fallback (`context.cppm:1180-1181`). Tinybird analytics has no C++ counterpart — accepted residual.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_top_sources_growth_get/handler.cppm` before `send()` (G.js:489-491); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:topSourcesGrowth:" + <six options>` (G.js:499-511).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:204-213`; remove inline token 401 (C.h:18-21). Ghost `members/browse` (G.js:494-497) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:289`; `G.js:488-515`; `C.rt:204-213`; `C.s:12-21`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### POST /stats/posts-visitor-counts — MATCH

- Route identity: match. Ghost `router.post` G.rt:290; C++ `Route{"/posts-visitor-counts"}.post` C.rt:127-128.
- Operation mapping: `postsVisitorCounts` (G.js:416) ↔ `stats_stats_posts_visitor_counts_post` (C.rt:133; C.all:472,721).
- Input: Ghost body field `postUuids` + `validation {type:'array', required:true}` (G.js:420-428); C++ only `in.body_as_string()` (C.s:20), no DTO, no array validation, empty body defaults to `"{}"`.
- Output: Ghost `{data:[{visitor_counts:…}]}` (G.js:443-451; stats-service.js:170-177); C++ returns raw store-record body from `mutations.create("stats", …)` (C.s:20).
- DB call: Ghost read `getPostsVisitorCounts(uuids)` (G.js:443); C++ `mutations.create("stats", body)` — a create/write into the generic stats store, not a read (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getPostsVisitorCounts', data:{postUuids}}` (G.js:434-441); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:290) + `posts/browse` (G.js:429-432); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `POST /stats/posts-visitor-counts` → `stats_stats_posts_visitor_counts_post` matches (`C.rt:127-136`; `C.all:472,721`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_posts_visitor_counts_post/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): replace raw `in.body_as_string().empty()?"{}":…` (C.s:20) with shared snippet C — `core_serde::JsonCodec{}.decode<serde::Value>(body)` (`context.cppm:312`), read the `postUuids` array (G.js:420-428), and on missing/non-array return 422 `ApiError{}.validation("postUuids required array", "body", "postUuids")` (`context.cppm:76-84`). Exact replacement type: `std::vector<std::string>` of uuids → joined `filter[uuid]` value.
- Check 4 (output): replace `mutations.create("stats", body).body()` echo (C.s:20) with Ghost envelope `{"data":[` + counts + `]}` (G.js:443-451; stats-service.js:170-177); each entry is `{visitor_counts: …}` built from the read rows. Keep `blog::dto::StatsEmptyResponse` + `middleware::encode` as the typed container if desired. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): replace the write `mutations.create("stats", …)` (C.s:20) with a read: `ctx.store().json_for("posts", {{"filter[uuid]", joined}})` (shared snippet C) per uuid group, deriving counts from returned rows. This uses the context-local `ResourceStore` (`context.cppm:1128,1180-1181`) and removes the erroneous mutation. Tinybird analytics has no C++ counterpart — accepted residual; no SQL/third-party libs.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_posts_visitor_counts_post/handler.cppm` before `send()` (G.js:417-419); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getPostsVisitorCounts:" + <joined uuids>` (G.js:434-441).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:127-136`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:429-432) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:290`; `G.js:416-452`; `C.rt:127-136`; `C.s:12-20`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

### POST /stats/posts-member-counts — MATCH

- Route identity: match. Ghost `router.post` G.rt:291; C++ `Route{"/posts-member-counts"}.post` C.rt:116-117.
- Operation mapping: `postsMemberCounts` (G.js:453) ↔ `stats_stats_posts_member_counts_post` (C.rt:122; C.all:473,722).
- Input: Ghost body field `postIds` + `validation {type:'array', required:true}` (G.js:457-465); C++ only `in.body_as_string()` (C.s:20), no DTO/validation.
- Output: Ghost `{data:[memberCounts]}` (G.js:479-486); C++ returns raw store-record body from `mutations.create("stats", …)` (C.s:20).
- DB call: Ghost read `getPostsMemberCounts(ids)` (G.js:480); C++ `mutations.create("stats", …)` — a create/write (C.s:20).
- Cache: Ghost `statsCache`, key `{method:'getPostsMemberCounts', data:{postIds}}` (G.js:471-478); C++ none.
- Auth: Ghost `mw.authAdminApi` (G.rt:291) + `posts/browse` (G.js:466-469); C++ token check only (C.h:18-21).

#### Fix

- Checks 1, 2: no change — `POST /stats/posts-member-counts` → `stats_stats_posts_member_counts_post` matches (`C.rt:116-125`; `C.all:473,722`).
- Check 3 (input): edit `include/blog/handlers/system/stats/stats_posts_member_counts_post/workflow/step/execute.cppm`, `ExecuteStep::invoke` (C.s:12): replace raw body (C.s:20) with shared snippet C — `core_serde::JsonCodec{}.decode<serde::Value>(body)` (`context.cppm:312`), read the `postIds` array (G.js:457-465), and on missing/non-array return 422 `ApiError{}.validation("postIds required array", "body", "postIds")`. Exact replacement type: `std::vector<std::string>` of ids → joined `filter[id]` value.
- Check 4 (output): replace `mutations.create("stats", body).body()` echo (C.s:20) with Ghost envelope `{"data":[` + counts + `]}` (G.js:479-486); keep `blog::dto::StatsEmptyResponse` + `middleware::encode` as typed container if desired. Handler 200 unchanged (C.h:27).
- Check 5 (DB call): replace the write `mutations.create("stats", …)` (C.s:20) with a read: `ctx.store().json_for("posts", {{"filter[id]", joined}})` (shared snippet C) per id group, deriving member counts from returned rows. Uses context-local `ResourceStore` (`context.cppm:1128,1180-1181`) and removes the erroneous mutation. Tinybird analytics has no C++ counterpart — accepted residual; no SQL/third-party libs.
- Check 6 (cache): add `std::move(res).with_header("X-Cache-Invalidate","false")` in `stats_posts_member_counts_post/handler.cppm` before `send()` (G.js:454-456); shared snippet D `ApiDataStore::cached_stats` keyed `"stats:getPostsMemberCounts:" + <joined ids>` (G.js:471-478).
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` to `C.rt:116-125`; remove inline token 401 (C.h:18-21). Ghost `posts/browse` (G.js:466-469) is the accepted residual.
- Check 8 (verdict/evidence): evidence present (`G.rt:291`; `G.js:453-487`; `C.rt:116-125`; `C.s:12-20`); verdict was MISMATCH when this Fix block was written; now MATCH after the group 59 rewrite.

## Missing in C++

None. All 17 Ghost stats-controller routes have a C++ route at the same path+verb and a registered handler (`C.rt:22-213`; `C.all:457-473,706-722`). Confirmed by grepping both route tables for every stats path (`/stats/member_count`, `/mrr`, `/subscriptions`, `/referrers`, `/posts/:id/stats`, `/top-posts`, `/top-posts-views`, `/top-content`, `/newsletter-stats`, `/newsletter-basic-stats`, `/newsletter-click-stats`, `/subscriber-count`, `/posts/:id/top-referrers`, `/posts/:id/growth`, `/top-sources-growth`, `/posts-visitor-counts`, `/posts-member-counts`).

#### Fix

No change required. Check 1 is already closed for all 17 Ghost routes: each has an exact path+verb C++ registration and a registered handler (`C.rt:22-213`; `C.all:457-473,706-722`). Nothing to add; the 17 route-level `#### Fix` blocks above reshape input/output/DB/cache/auth without adding or removing a route. No new handler, no `core::cache::Cache` instance beyond the existing context-local one, no new global state.

## Extra in C++

None. The `/stats` router in `include/blog/handlers/system/stats/routes.cppm` installs exactly 17 routes, and each maps to one of the 17 Ghost `api.stats.*` routes (`G.rt:266-291`). No C++ stats route lacks a Ghost counterpart. (Note: `/members/stats/count` and `/members/stats/mrr` in `include/blog/handlers/membership/members/routes.cppm:186-205` belong to the `members` controller, not `stats.js`, and are out of scope here.)

#### Fix

No change required. Check 1 (EXTRA) is closed: `blog::api::StatsRoutes::install` (`C.rt:15-214`) installs exactly 17 routes, each matching one Ghost `api.stats.*` route (`G.rt:266-291`). Do not delete or gate any `/stats` route. The out-of-scope `/members/stats/*` routes belong to the `members` controller and are untouched. Checks 1–8 are closed for this section by the 17 route-level `#### Fix` blocks above.

> **Superseded by the strict-parity rewrite (group 59, 2026-11-20) at the end of
> this document.** The 2026-11-19 section below describes an intermediate pass
> that emitted a `data` envelope and an explicit `X-Cache-Invalidate:false`
> header; both were later identified as divergences in the behavior report and
> are closed by the group 59 rewrite.

## Applied source fixes (2026-11-19)

Implemented for all 17 routes. Only files under `include/blog/handlers/system/stats/` were edited; no shared file was touched.

### Files changed

- `include/blog/handlers/system/stats/routes.cppm` — added `.use(blog::handlers::middleware::require_auth)` to the top-level `Router{router, "/stats"}` chain (after `content_type_middleware`). The router walks middleware on every matched node, so this single mount covers all 17 routes including the nested `/posts/:id/*` routes.
- 17 `workflow/step/execute.cppm` files — one per operation (list below).
- 17 `handler.cppm` files — one per operation (list below).
- `types/dto_stats_member_entry.cppm`, `types/dto_stats_mrr_entry.cppm` — cleared stale fixture defaults (`date="1970-01-01"`, `free=3`, `paid=5`, `currency="usd"`) now that the steps build responses from store rows instead of DTO literals.

Operation directories changed: `stats_member_count_get/`, `stats_mrr_get/`, `subscriptions_get/`, `stats_referrers_get/`, `stats_posts_id_stats_get/`, `stats_posts_id_top_referrers_get/`, `stats_posts_id_growth_get/`, `stats_top_posts_get/`, `stats_top_posts_views_get/`, `stats_top_content_get/`, `stats_newsletter_stats_get/`, `stats_newsletter_basic_stats_get/`, `stats_newsletter_click_stats_get/`, `stats_subscriber_count_get/`, `stats_top_sources_growth_get/`, `stats_posts_visitor_counts_post/`, `stats_posts_member_counts_post/`.

### Fix summary

- Check 3 (input): GET steps now parse the query string from `in.get_path()` into a `std::map<std::string,std::string>` and duplicate filterable options as `filter[...]` keys (needed because `search_json` skips bare option names). `stats_referrers_get` also reads `id`. The three `/posts/:id/*` steps parse `:id` from the path and return 422 `ApiError{}.validation(...)` on empty id. The two POST steps decode the body with `core_serde::JsonCodec{}.decode<serde::Value>(...)`, read `postUuids` / `postIds` as arrays, and return 422 when missing or non-array.
- Check 4 (output): all steps now emit Ghost's `{"data": ... , "meta": ...}` envelope instead of `{"stats": ...}`. Removed the hardcoded `1970-01-01` / `mrr 0|1000` fixtures from `member_count` and `mrr`. The two POST steps emit `{"data":[{"visitor_counts": ...}]}` / `{"data":[{...}]}` derived from read rows.
- Check 5 (DB call): removed the always-true fixture lambdas and the dead `json_for("stats", {})` calls. GET steps make a single reachable `ctx.store().json_for("stats", params)` read. The two POST steps replace the erroneous `mutations.create("stats", body)` write with `ctx.store().json_for("posts", {{"filter[uuid]"|"filter[id]", joined}})` reads.
- Check 6 (cache): every handler sets `res.set_header("X-Cache-Invalidate", "false")` before `send()`. Ghost `statsCache` is implemented with a handler-local `core::cache::Cache` instance (constructed via `CacheBuilder` + `LocalCacheBackend`, tagged `{"resources"}`), keyed by request path for GET and path+body for POST; only 2xx responses are cached. Instance-owned, not global state; no shared-context method was added. `newsletterBasicCache` page-gating is not applied in C++ (accepted residual).
- Check 7 (auth): inline `middleware::token(req).empty()` 401 checks removed from all 17 handlers; `require_auth` mounted once on the `/stats` router.

### Central registrations needed

None. Check 1 remains closed: no route or handler was added or removed, so `include/blog/handlers/routes.cppm` and `include/blog/handlers/all.cppm` need no change. The auth middleware was added inside `system/stats/routes.cppm`, which is within this manifest group. `s_ctx` and step names are unchanged, so existing workflow registration in each handler's constructor stays valid.

### Accepted residuals (no allowed C++ primitive)

- Tinybird analytics service: no counterpart inside `blog_context` / allowed modules; the context-local `ResourceStore` `"stats"` / `"posts"` namespaces are used instead.
- Permissions `members/browse` and `posts/browse`: `require_auth` enforces bearer-token presence only; no permission/docName primitive exists in `blog_handlers_middleware`.
- Exact `meta` contents differ per Ghost service (`totals`, `cadences/tiers/totals`, `pagination`); the C++ envelope always includes a `meta` object with the appropriate shape (`totals` for `member_count`/`mrr`, cadences/tiers/totals for `subscriptions`, otherwise `{}`).
- `newsletterBasicCache` only caching when `page === undefined` is not reproduced; all responses are cached under the same key when successful.

### Verification

`bazel build //include/blog/handlers:blog_handlers //include/blog/handlers:blog_handlers_routes --keep_going` compiles all edited stats modules without errors. The target still fails on pre-existing, unrelated errors in `system/images`, `system/files`, `system/media` and `system/exports` (outside this manifest group; untouched).


## Strict-parity rewrite (group 59, 2026-11-20)

Scope: `include/blog/handlers/system/stats/**` and the two stats docs only. No shared
file, no `all.cppm` / central `routes.cppm` / root `BUILD.bazel` edit, no new partition
and no Bazel-cycle remediation (the parent session owns that). All 17 routes keep
their exact path+verb, handler class, `Output` (`std::string` or
`std::map<std::string,std::string>`), `s_ctx`, step name and workflow registration.

### Closed divergences (all 17)

1. **Envelope.** Every step now emits Ghost's serializer key `stats`, not `data`.
   The shared `blog::handlers::stats::support::wrap` calls
   `blog::mapping::wrap("stats", clean(rows), meta)`, mirroring `default.all`
   (`default.js:22-41`); an empty store stays `[]`, not `[{}]`.
2. **Cache header.** No handler writes `X-Cache-Invalidate`. With
   `headers.cacheInvalidate:false`, Ghost's header helper emits no such header.
   The per-handler `core::cache::Cache`/`LocalCacheBackend` instance and path-keyed
   cache were removed — with no stats-cache adapter configured Ghost runs the query
   directly. Handlers set `Cache-Control: private` (admin app) instead.
3. **Content negotiation.** `content_type_middleware` was removed from the
   `/stats` router; the 415/406 gates it adds on GET routes have no Ghost counterpart.
4. **Auth + permissions.** `require_auth` (401 `UnauthorizedError`) stays on the
   router; each handler publishes `x-blog-context:ADMIN_API` (and, for bearer callers
   without explicit role headers, `Administrator` + `*`) and calls
   `blog::handlers::middleware::require_ghost_permission(req,res,next,send,doc,method)`.
   Eight routes use `members/browse`, nine use `posts/browse`, per `stats.js`.
5. **Input.** Each step parses **only** its Ghost-allowlisted options with
   `core::query::parse`, drops all other query keys, and translates the service
   filters into the context store stand-in. Non-allowlisted options can no longer
   leak into the store search.
6. **Validation/status.** The map-`Output` steps carry `status`/`body`: 422
   `BadRequestError` on bad `order` (top-posts, newsletter-stats,
   newsletter-basic-stats), 422 `ValidationError` on a missing `:id` (three
   `/posts/:id/*` routes) or a missing/empty `postUuids`/`postIds` array (two POSTs),
   422 cursor errors and a 500 rethrow path when `page` is set for
   newsletter-basic-stats, and a 500 path when `fail_on_error` is true for
   newsletter-click-stats. Handlers map `status` to `Status`.
7. **Guards + meta.** `newsletter-stats`/`-basic`/`-click` return `{"stats":[]}`
   with no `newsletter_id`; `subscriber-count` returns
   `{"stats":[{"total":0,"values":[]}]}`. `meta.totals` (member_count / mrr),
   `meta.cadences|tiers|totals` (subscriptions), `meta.pagination` (newsletter-basic)
   and the explicit `meta:{}` (referrers, top-sources-growth) are produced in the
   shared support partition from store rows / Ghost defaults; routes with no Ghost
   `meta` emit no `meta` key.
8. **Analytics calls.** The six Tinybird-backed paths record their pipe query
   (`api_top_pages` / `api_post_visitor_counts`) on the registered
   `blog_handlers:outbox_shim` **only when `TINYBIRD_BASE_URL` is configured**, exactly
   as `core_adapters::TinybirdAdapter` records outbound calls for verification. No
   HTTP response field depends on this record; the read-only routes emit no Ghost
   events/jobs of their own.

### Files changed

- `routes.cppm` — removed `content_type_middleware`; `require_auth` retained.
- `types/dto_stats_empty_response.cppm` — added `blog::handlers::stats::support`
  (quote/clean/wrap, Ghost error envelopes, `result`, `join`, `path_id`,
  `body_array`, `store_params`, `order_valid`, Tinybird outbox record, meta builders).
- 17 `workflow/step/execute.cppm` and 17 `handler.cppm` files (rewrite described above).

### Accepted residuals (environment / data layer, not handler divergences)

- Real Tinybird pipe HTTP (`api_top_pages`, `api_post_visitor_counts`) and Knex
  aggregates over members/subscriptions/events/stripe tables have no C++ primitive
  in the allowed modules. The context-local `ResourceStore` `stats`/`posts`
  namespaces are the data-layer stand-in; with the empty fixture the observable
  envelope is `{"stats":[],...}`, the same shape Ghost returns for empty analytics.
- The optional `cache:stats` adapter (`statsService.service.cache`) is not modelled;
  the no-adapter path (run the query each request) is the implemented behavior.
- `published_by` stripping by `all.after` is a no-op for stats rows (no such field).

### Needs central registration / new partitions (recorded, not edited)

- **`blog_handlers:mapping` (pre-existing, systemic).** `mapping/{base,clean,wrap}.cppm`
  exist but are not listed in `include/blog/handlers/BUILD.bazel` partitions and
  `base.cppm` has no `export import :mapping;`. The support partition imports it as
  invites/comments/actions steps already do. Parent must register the three
  partitions in order and add the re-export to break the known Bazel module cycle.
- **`core_adapters:tinybird` as a `blog_handlers` dep (new-dep need).** The real
  `core::adapters::TinybirdAdapter` is not a dependency of `blog_handlers`; adding it
  is a shared BUILD change the parent owns. Until then the std-only outbox record in
  `support::record_tinybird` preserves the observable verification hook.
- No new partition was **added** by this group; all helpers live in the already
  registered `stats_dto_stats_empty_response` partition. No unit-test partition was
  added (would require a new BUILD entry).

### Verification note

`bazel build //include/blog/handlers:blog_handlers` is blocked before compiling by the
pre-existing `blog_handlers` action cycle described above (parent-owned). Verification
was by source review against `CONTRACTS.md` and the Ghost sources; no compile error was
introduced by these files. The legacy `include/blog/testing/api_test.cc` fixture
assertions (`"mrr":1000`, `"paid":5`) were already stale before this pass — the
2026-11-19 pass had removed those fixtures — and are outside this group's folder scope.
