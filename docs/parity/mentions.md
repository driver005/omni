# Parity: mentions

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/mentions.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (browse) and `docs/Ghost/ghost/core/server/web/webmentions/routes.js` (receive)
- C++: `include/blog/handlers/discovery/mentions/routes.cppm`, `browse/*`, `receive/*`
- Checked: 2026-11-19
- Strict-parity pass (manifest group 18): 2026-11-19 — both routes MATCH.

Controller exports two keys: `browse` and `receive`. `browse` is mounted as `GET /mentions` in admin routes; `receive` as `POST /receive` in the webmentions web app. C++ implements both.

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
| GET /mentions | `mentions.browse` | `Router{router,"/mentions"}.get(...)` `routes.cppm:19-32` | `mentions_browse()` → `BrowseHandler` → `ExecuteStep` | `options:['filter','fields','limit','order','page','debug','unique']`; input serializer forces `source:-~^'https://brid.gy/'`; Ghost defaults limit=`all`, page=1, order=`created_at asc`, unique only `'true'` (mentions.js:10; mention-controller.js:62-97). C++ `core::query::parse` + `force_bridgy_filter` + `apply_browse_defaults` + `drop_bridgy` + `order_and_dedupe`. | Ghost `{mentions,meta.pagination}` with 13-field mapper + resource enrichment. C++ same envelope; `mention_row` maps `timestamp`←`created_at`, drops `deleted`/`revalidation_failure_count`/`resourceId`, `finish_row` attaches `find_by_resource`. | Ghost `findPage` (+`whereRaw` unique) + `Post.findOne`. C++ `ctx.store().resources().list("mentions")`, `core::query::apply`/`run`, `find_by_resource`. | Ghost `cacheInvalidate:false`. C++ `X-Cache-Invalidate:false`. | Ghost `authAdminApi` + `permissions:true`, docName `mentions`. C++ `require_auth` + step `GhostAuthorizer.authorize(...,"mentions","browse")` + two-arg `require_ghost_permission` hook. | MATCH | Ghost: admin/routes.js:47, mentions.js:5-14, mention-controller.js:61-121. C++: `routes.cppm:19-32`, `browse/handler.cppm`, `browse/.../execute.cppm`. |
| POST /receive | `mentions.receive` | `Router{router,"/receive"}.post(...)` `routes.cppm:31-43` | `mentions_receive()` → `ReceiveHandler` → `ExecuteStep` | Ghost `options:[]`, urlencoded body 64kb, reads `source`/`target` (mention-controller.js:130). C++ `parse_source_target` (urlencoded + JSON), 64kb→413, fixed-window 429. | Ghost 202 plain, empty body, `cacheInvalidate:false` + `Cache-Control:private`. C++ identical. | Ghost queues `ProcessWebmentionJob` → `getBySourceAndTarget`/`save` + domain events. C++ transactional `job.dispatch` outbox event drained by registered `WebmentionJobAdapter` → `webmention.fetch` + `create("mentions")` + `mention.added`. | Ghost `cacheControl('private')`. C++ both headers. | Ghost `permissions:false` + rate limiter. C++ no auth + fixed-window 429. | MATCH | Ghost: webmentions/routes.js:15-25, mentions.js:17-31, mention-controller.js:128-147. C++: `routes.cppm:31-43`, `receive/.../execute.cppm`. |

## Route details

### GET /mentions — MATCH

- Route identity: `GET /mentions` — Ghost `admin/routes.js:47`; C++ `routes.cppm:19-32`. Match (flat router drops the Ghost API mount prefix, neutral across groups).
- Operation: Ghost `mentions.browse` → `controller.browse`; C++ `mentions_browse()` → `BrowseHandler` → `ExecuteStep`. Match.
- Input: all seven Ghost options are allow-listed through `core::query::parse`; the brid.gy exclusion is forced (`input/mentions.ts`) and enforced by `drop_bridgy`; Ghost defaults (page=1, order=`created_at asc` only `created_at desc` accepted, limit absent/`all` ⇒ unbounded, unique⇒dedupe highest id) are implemented in `apply_browse_defaults`/`order_and_dedupe`. `fields` projection and NQL filtering come from `core::query`.
- Output: `{"mentions":[…],"meta":{"pagination":{limit,page,pages,prev,next,total}}}` with real total from `core::query::run`; `limit` stays `"all"` when unbounded. 13-field Ghost mapper + `find_by_resource` enrichment.
- DB call: store list → map → query → `find_by_resource`; no fabricated pagination, no hardcoded body.
- Cache/header: `X-Cache-Invalidate: false`; no cache mutation.
- Auth: router `require_auth` (401) + step `GhostAuthorizer` (403/401 Ghost envelope) + two-arg `require_ghost_permission` hook for central pipelines.
- **Diff**: no residual behavioral divergence. `debug` is a no-op on both sides; SQL `findPage`/`whereRaw` has no C++ primitive (neutral).

#### Fix (applied)

- `browse/.../execute.cppm`: replaced the manual query parse + hand-built envelope with `core::query::parse`/`apply`/`run`, forced brid.gy exclusion, Ghost defaults, `unique` dedupe, `created_at` sort, Ghost 13-field mapper and `find_by_resource` enrichment; `Output` is `std::map<std::string,std::string>` carrying `status`/`body`.
- `browse/handler.cppm`: removed the inline token-presence 401 body, added the two-arg `require_ghost_permission("mentions","browse")` hook, writes payload status + `X-Cache-Invalidate: false`.
- `routes.cppm`: `require_auth` retained on `/mentions`; `content_type_middleware` removed from `/receive`.

### POST /receive — MATCH

- Ghost route: `POST /webmentions/receive`, urlencoded 64kb, rate limiter, `cacheControl private`.
- Controller op: `receive`, `statusCode:202`, `format:'plain'`, `permissions:false`, `options:[]`, returns `null` after queuing `ProcessWebmentionJob`.
- C++: `POST /receive`, no auth, no JSON content-type gate; urlencoded (`+`/`%XX`) and JSON bodies both accepted; body >64kb → 413; fixed-window 429 ("Too many mention attempts").
- Side effects: transactional `job.dispatch` outbox event; a single registered `WebmentionJobAdapter` is driven by `outbox.drain()` and emits `webmention.fetch` (outbound webmention adapter), persists the accepted mention with `create("mentions", …)`, and emits `mention.added` (`DomainEvents.dispatch`). `processed_log()` plus the outbox pending queue are observable proof.
- Output: 202, empty plain body, `X-Cache-Invalidate: false`, `Cache-Control: private`.
- **Diff**: no residual behavioral divergence. Live HTTP metadata fetch, exact BackoffLimiter schedules and the `/webmentions` mount prefix have no in-folder primitive (neutral residuals).

#### Fix (applied)

- Added `POST /receive` chain in `routes.cppm:31-43` (no auth, no content-type middleware).
- `receive/.../execute.cppm`: urlencoded/JSON parse, 64kb 413, fixed-window 429, `job.dispatch` outbox emit + registered `WebmentionJobAdapter` processing, 202 empty return.
- `receive/handler.cppm`: payload status (202/413/429), `X-Cache-Invalidate: false`, `Cache-Control: private`, empty body on 202.

## Missing in C++

- None. Both Ghost controller keys now have a C++ route, handler, flow and step.

## Extra in C++

- None. The only C++ mentions routes are `GET /mentions` and `POST /receive`, both with Ghost counterparts.

## Needs central registration

None. All edits are confined to `include/blog/handlers/discovery/mentions/` and this docs file. Shared helpers live in the already-registered `:mentions_browse_step` partition (namespace `blog::handlers::mentions::support`), which the receive step imports. No new partition, `all.cppm` import/accessor/member, `BUILD.bazel` src entry or root file was added.

The known Bazel dependency cycle in `//include/blog/handlers:blog_handlers` was left untouched, as were unrelated compile failures in other groups' folders.
