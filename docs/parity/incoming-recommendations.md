# Parity: incoming-recommendations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/incoming-recommendations.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/discovery/incoming_recommendations/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/incoming_recommendations` | `browse` | GET `/incoming_recommendations` | `incoming_recommendations_browse()` | Ghost: `options=['limit','page']` parsed from query (`page` default 1, `limit` default 5). C++: `using Input = interfaces::io::IRequest&`, no query parsing — `limit`/`page` ignored. | Ghost: `{ data: [{id,title,excerpt,featured_image,favicon,url,recommending_back}], meta }` with real pagination meta. C++: `{"incoming_recommendations": <rows>, "meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` — wrong envelope key, hardcoded limit 15 / total 0. Both status 200. | Ghost: `mentionsApi.listMentions({ filter, page, limit })` (mention source with recommendation filter). C++: `ctx.store().json_for("incoming_recommendations", {})` — different resource, empty params, no filter/pagination. | Ghost: `headers.cacheInvalidate: false` (no invalidation header), no cache service. C++: none. Both effectively none. | Ghost: `mw.authAdminApi` + `permissions: true` (docName `recommendations`). C++: `content_type_middleware` + token check in handler (`token(req).empty()` → 401). Auth present both sides, different mechanism. | MISMATCH | Ghost: routes.js:600-604, controller:7-17, incoming-recommendation-controller.ts:21-49, incoming-recommendation-service.ts:156-175. C++: routes.cppm:19-28, handler.cppm:17-27, execute.cppm:11-17, all.cppm:583,832. |

## Route details

### GET `/incoming_recommendations` — MISMATCH

- **Route identity**: MATCH. Ghost `router.get('/incoming_recommendations', mw.authAdminApi, http(api.incomingRecommendations.browse))` (routes.js:600-604) vs C++ `Router{router, "/incoming_recommendations"}...get(...)` (routes.cppm:19-27).
- **Operation mapping**: MATCH. Ghost op key `browse` (controller:7) delegates to service controller `browse` (incoming-recommendation-controller.ts:21); C++ `handler.incoming_recommendations_browse().handle(...)` (routes.cppm:26), registered as `m_incoming_recommendations_browse` (all.cppm:832) exposed via `incoming_recommendations_browse()` (all.cppm:583).
- **Input**: DIFFERENT. Ghost declares `options: ['limit', 'page']` (controller:11) and the service parses `page`/`limit` ints from `frame.options` (incoming-recommendation-controller.ts:22-25). C++ `ExecuteStep::Input` is the raw request ref with no query parsing (execute.cppm:11-14); `limit`/`page` never read.
- **Output**: DIFFERENT. Ghost serializer returns `{ data: [...], meta }` with entity fields id/title/excerpt/featured_image/favicon/url/recommending_back and real pagination meta (incoming-recommendation-controller.ts:32-49; meta originates from `mentions.meta`, incoming-recommendation-service.ts:172-175). C++ returns envelope key `incoming_recommendations` and hardcoded pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` (execute.cppm:17). Both set status 200 (handler.cppm:27; Ghost default). Material data-shape divergence.
- **DB call**: DIFFERENT. Ghost calls `this.#mentionsApi.listMentions({ filter, page, limit })` with a mention-source filter (incoming-recommendation-service.ts:162-164). C++ calls `ctx.store().json_for("incoming_recommendations", {})` (execute.cppm:16) — different resource string ("incoming_recommendations" vs underlying mentions), empty params, no filter and no real pagination.
- **Cache**: Both none. Ghost sets `headers.cacheInvalidate: false` (controller:8-10), no cache service call. C++ has no `core::cache::Cache` usage in the step/handler.
- **Auth**: DIFFERENT mechanism, present on both. Ghost route uses `mw.authAdminApi` (routes.js:602) and controller `permissions: true` with `docName: 'recommendations'` (controller:5,12). C++ route installs `content_type_middleware` (routes.cppm:20) and the handler enforces bearer token via `token(req).empty()` → `401 {"error":"Unauthorized"}` (handler.cppm:18-22). No serde DTO validation on C++ (Input is the raw request).

## Missing in C++

None — the single Ghost route (GET `/incoming_recommendations`) has a C++ counterpart.

## Extra in C++

None — `include/blog/handlers/discovery/incoming_recommendations/` contains only `routes.cppm` and the `browse` operation (handler/flow/execute); no C++ route without a Ghost controller route. Confirmed via glob `include/blog/handlers/discovery/incoming_recommendations/**/*.cppm` and grep of `routes.cppm` (single `.get(...)` install at line 21).
