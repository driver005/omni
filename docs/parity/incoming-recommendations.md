# Parity: incoming-recommendations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/incoming-recommendations.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/discovery/incoming_recommendations/routes.cppm`
- Checked: 2026-11-19
- Fix applied: 2026-11-19 (manifest group 16: initial reshape; group 17: strict-parity rewrite). Source reshaped per the `#### Fix` below; verdict MATCH with non-route residuals documented.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/incoming_recommendations` | `browse` | GET `/incoming_recommendations` | `incoming_recommendations_browse()` | Ghost: `options=['limit','page']` parsed from query (`page` default 1, `limit` default 5), non-integer → 422 `<field> must be an integer`. C++: `core::query::parse` allowlist + `ghost_integer` check, defaults 5/1, 422 `ApiError.validation("<field> must be an integer","","")`. MATCH. | Ghost: `{ data: [{id,title,excerpt,featured_image,favicon,url,recommending_back}], meta:{pagination} }`. C++: same envelope and seven fields via local mapper helpers; `title = source_site_title || source_title`, URL suffix stripped, `recommending_back` computed against stored outgoing recommendations; real `core::query` pagination. Status 200 (422 bad input, 401/403 auth). MATCH. | Ghost: `mentionsApi.listMentions({filter,page,limit})`, filter `source:~$'/.well-known/recommendations.json'`. C++: `ctx.store().json_for("mentions",{})` filtered + sliced by `core::query::apply` with the same NQL filter; total from a second unlimited apply. In-memory store stands in for the mentions repository — accepted non-route residual. MATCH (route behavior). | Ghost: falsy `headers.cacheInvalidate:false` emits no `X-Cache-Invalidate` (`@tryghost/api-framework/src/headers.ts:135`). C++: no header set. MATCH. | Ghost: `mw.authAdminApi` + `permissions: true` (docName `recommendations`, method `browse`). C++: router-level middleware removed; `BrowseHandler::handle` calls `require_ghost_permission(req,res,next,send,"recommendations","browse")` backed by `core::auth::ghost`. MATCH. | MATCH (non-route residuals: mentions external fetch/boot refresh, readRecommendationByUrl service) | Ghost: routes.js:600-604, controller:5-17, incoming-recommendation-controller.ts:21-49, incoming-recommendation-service.ts:103-175, unsafe-data.ts:163-177. C++: routes.cppm:27-35, handler.cppm:21-49, execute.cppm:37-44,72-86,126-160,175-228. |

## Route details

### GET `/incoming_recommendations` — MATCH

- **Route identity**: MATCH. Ghost `router.get('/incoming_recommendations', mw.authAdminApi, http(api.incomingRecommendations.browse))` (routes.js:600-604) vs C++ `Router{router, "/incoming_recommendations"}...get(...)` (routes.cppm:27-34). Router-level `content_type_middleware` / `bearer_middleware` / `require_auth` removed (`#### Fix`, check 7).
- **Operation mapping**: MATCH. Ghost op key `browse` (controller:7) delegates to service controller `browse` (incoming-recommendation-controller.ts:21); C++ `handler.incoming_recommendations_browse().handle(...)` (routes.cppm:33), registered as `m_incoming_recommendations_browse` (all.cppm:924) exposed via `incoming_recommendations_browse()` (all.cppm:645).
- **Input**: MATCH. Ghost declares `options: ['limit','page']` (controller:11) and parses integer `page` (default 1) and `limit` (default 5) via `UnsafeData.integer` (incoming-recommendation-controller.ts:22-25, unsafe-data.ts:163-177). C++ `core::query::parse(in.get_path(), {"limit","page"})` (execute.cppm:178-179) plus `ghost_integer()` (parseInt round-trip, :72-86) over raw query pairs in Ghost order `page`, `limit`; non-integer → 422 `ApiError.validation("<field> must be an integer", "", "")` (:183-192); defaults 5/1 set when absent (:195-196).
- **Output**: MATCH. Ghost serializer returns `{ data: [...], meta }` with fields id/title/excerpt/featured_image/favicon/url/recommending_back (incoming-recommendation-controller.ts:35-49). C++ `to_incoming_recommendations` (execute.cppm:137-160) maps mention fields to the same seven names, applies `sourceSiteTitle || sourceTitle`, strips `/.well-known/recommendations.json` from `source`, drops unparseable URLs, and computes `recommending_back` against stored outgoing recommendation URLs. `local_mapping::pagination` + `wrap("data", ...)` produce `{"data":[...],"meta":{"pagination":{total,limit,page,pages,prev,next}}}` (:219-227). Status 200, 422 on bad input.
- **DB call**: MATCH (route behavior). Ghost `listMentions({filter,page,limit})` with `source:~$'/.well-known/recommendations.json'` (incoming-recommendation-service.ts:162-164). C++ `ctx.store().json_for("mentions",{})` (execute.cppm:200) with `core::query::apply` using the same NQL contains filter (`$` sigil dropped; fixed lowercase marker ⇒ equivalent) and real `page`/`limit` slicing (:202-208); total from a second unlimited apply (:210-215). The in-memory `mentions` store stands in for the external mentions repository — accepted non-route residual; no direct SQL.
- **Cache**: MATCH. Ghost's `headers.cacheInvalidate:false` is falsy, so `headers.get()` emits no `X-Cache-Invalidate` (`@tryghost/api-framework/src/headers.ts:135`). C++ sets no such header (handler.cppm:47-49). Earlier C++ set a literal `X-Cache-Invalidate: false`; removed in group 17.
- **Auth**: MATCH. Ghost route uses `mw.authAdminApi` (routes.js:602) and controller `permissions:true` with `docName:'recommendations'`, method `browse` (controller:5,12). C++ handler infers ADMIN_API context, maps a bearer token to Administrator role/permissions, and calls `require_ghost_permission(req,res,next,send,"recommendations","browse")` (handler.cppm:21-34), backed by `core::auth::ghost::GhostAuthorizer` with Ghost's built-in roles. Denials write Ghost-shaped 401/403 envelopes.

#### Fix

Strict-parity rewrite applied to `include/blog/handlers/discovery/incoming_recommendations/` only (no shared files, no BUILD edits, no Bazel-cycle change). Registered shared abstractions used: `core_query` (module `core_query`), `blog_handlers_middleware` + `core_auth:ghost` (via `require_ghost_permission`). Output stays `std::map<std::string,std::string>` and the step keeps static `s_ctx`.

Exact replacements (checks 1–8 closed):
- **1, 2 (route/op):** keep `GET /incoming_recommendations` and `handler.incoming_recommendations_browse()`; registry `all.cppm:645,924` unchanged. Remove the router-level `.use(content_type_middleware)`, `.use(bearer_middleware)`, `.use(require_auth)` chain (Ghost's admin app uses body parsers + in-pipeline `authAdminApi`/`permissions:true`).
- **3 (input):** replace the local `options()`/`integer()` helpers with `core::query::parse(in.get_path(), {"limit","page"})` and a `ghost_integer()` guard over raw query pairs; defaults `limit=5`/`page=1`; 422 `ApiError.validation("<field> must be an integer","","")` (Ghost `ValidationError` wording, `unsafe-data.ts:163-177`).
- **5 (DB call):** replace `ctx.store().json_for("incoming_recommendations", effective)` with `ctx.store().json_for("mentions",{})` and apply Ghost's `source:~$'/.well-known/recommendations.json'` NQL filter via `core::query::apply` (dropping the no-op `$` sigil), with real `page`/`limit` slicing and a filtered total. Non-route residual: no external mentions fetch/refresh primitive.
- **4 (output):** replace hand-built row picks with `to_incoming_recommendations`, which implements `#mentionToIncomingRecommendation` (`sourceSiteTitle || sourceTitle`, URL suffix strip, unparseable-URL drop, `recommending_back` cross-check against stored outgoing recommendations) and `#serialize`'s seven fields; build `{data, meta.pagination}` with local `pagination`/`wrap` helpers (the shared `blog_handlers:mapping` partitions are not a registered dep of `blog_handlers`). Keep status OK/422/400.
- **6 (cache header):** remove `res.set_header("X-Cache-Invalidate","false")`. Ghost's falsy `cacheInvalidate:false` emits no header (`headers.ts:135`).
- **7 (auth):** add `require_ghost_permission(req,res,next,send,"recommendations","browse")` in `BrowseHandler::handle` (context ADMIN_API, bearer→Administrator); router-level bearer/auth middlewares removed. This closes the `permissions:true`/`docName` gap.
- **8 (verdict):** all eight checks MATCH → verdict stays/flipped to MATCH; residual external-fetch/boot-refresh and readRecommendationByUrl-service facts are non-route and documented under **Diff** in `behavior/incoming-recommendations.md`.

## Missing in C++

None — the single Ghost route (GET `/incoming_recommendations`) has a C++ counterpart.

  Fix: No change required. Check 8 (MISSING) closed — route + verb already present; nothing to add. The `#### Fix` above only reshapes the existing `ExecuteStep` / `BrowseHandler` / `IncomingRecommendationsRoutes`. No new global state; no shared/BUILD files edited.

## Extra in C++

None — `include/blog/handlers/discovery/incoming_recommendations/` contains only `routes.cppm` and the `browse` operation (handler/flow/execute); no C++ route without a Ghost controller route. Confirmed via glob `include/blog/handlers/discovery/incoming_recommendations/**/*.cppm` and grep of `routes.cppm` (single `.get(...)` install at line 28).

  Fix: No change required. Check 8 (EXTRA) closed — `IncomingRecommendationsRoutes::install` (routes.cppm:15-35) registers only the single `.get(...)` counterpart of `api.incomingRecommendations.browse` (routes.js:600-604); nothing to delete or gate. Keep the `all.cppm:645,924` accessor; the Fix changes step/handler behavior only. No new global state.
