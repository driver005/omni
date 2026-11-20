# Parity: previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/previews.js` (docName `previews`, single exported operation `read`)
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (no `api.previews` route) — actual route is the frontend `PreviewRouter`, `docs/Ghost/ghost/core/frontend/services/routing/preview-router.js:15,29` + `docs/Ghost/ghost/core/frontend/services/routing/config.ts:35-38`
- C++: `include/blog/handlers/admin/previews/` (`routes.cppm`, `preview_get/`), registered at `include/blog/handlers/routes.cppm:8,136`, `include/blog/handlers/all.cppm:8,618,897`, `include/blog/handlers/base.cppm:249`, `include/blog/handlers/BUILD.bazel:914-916,970`
- Checked: 2026-11-19 (strict-parity rewrite using shared abstractions)

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
| GET `/p/:uuid/:options?` (frontend PreviewRouter) | `previews.read` | GET `/p/:uuid` + GET `/p/:uuid/:options` | `handler.previews_preview_get()` | `core::query::parse(path,{include,member_status,member_tier})`; uuid from path; Ghost value/shape validation incl. `member_tier` single-slug guard | `blog::mapping::clean`/`wrap` → `{previews:[mappedPost]}` + `type` + legacy `access` fallback; 404 "Post not found." | `core::query::apply(store().json_for("posts",{include,status:'all'}), uuid NQL)`; store tiers for paid shim + `tiers_browse` | none; no `X-Cache-Invalidate` header | public route, no serde/auth middleware; `permissions:true` via `core::auth::ghost` under INTERNAL proxy principal | MATCH | Ghost: `previews.js:16-99`, `controllers/previews.js:19-28`, `output/previews.js:4-35`. C++: `routes.cppm:18-46`, `execute.cppm:26-151` |

## Route details

### GET `/p/:uuid/:options?` — MATCH

Ghost side:

- Route identity: controller exports only `read` with `docName: 'previews'` (`previews.js:55-58`). No web route calls `api.previews.read`; the route is the hardcoded frontend `PreviewRouter` at `/p/:uuid/:options?` (`preview-router.js:15,29`), configured `{controller:'previews', resource:'previews'}` (`config.ts:35-38`), dispatched as `api[res.routerOptions.query.controller].read(params)` (`controllers/previews.js:27-28`).
- Operation mapping: `read` only → frontend controller `controllers.previews` → `api.previews.read`.
- Input parity: `options:['include','member_status','member_tier']`, `data:['uuid']`; `include.values=['authors','tags','tiers']`, `member_status.values=['anonymous','free','paid']`, `uuid.required` (`previews.js:63-79`); hand-written single-slug guard for `member_tier` (`:11,23-30`). Frontend supplies `uuid`, `status:'all'`, `include:'authors,tags,tiers'` and forwards the member query params (`controllers/previews.js:19-25`).
- Output parity: serializer builds `{previews:[mappedPost]}` with `type = model.get('type')` and tiers injected via `tiersService.api.browse({})` (`output/previews.js:5-35`). Frontend renders or 301-redirects (`controllers/previews.js:29-73`).
- DB call parity: `Post.findOne({status:'all',uuid},frame.options)` + `forceUrlRelations(frame,'posts')` (`previews.js:84-89`); `NotFoundError` on miss (`:90-94`); paid shim via `createPaidMemberShim(member_tier)` (`:46-50`).
- Cache parity: `headers.cacheInvalidate=false` (`previews.js:59-61`) ⇒ no header; no cache service/namespace/TTL.
- Auth/permissions: `permissions:true` (`previews.js:62`); public frontend route with no `mw.authAdminApi`; frame forced to `apiType='content'`/`isPreview` with synthetic member context for `free`/`paid` (`previews.js:32-52`).

C++ side (strict-parity rewrite):

- Route identity: two literal GET shapes `/p/:uuid` and `/p/:uuid/:options` both dispatch `handler.previews_preview_get()` (`routes.cppm:18-46`) because `core::router` has no optional-segment syntax. No `content_type_middleware` and no auth middleware — matching the public HTML frontend route; the previous 415/406 serde wall is gone.
- Input: `core::query::parse` with the three allowlisted options; uuid from the stripped path; repeated-key detection; `include`/`member_status` allowlists and the exact `member_tier` single-slug guard; 422 `ApiError.validation` envelopes (`execute.cppm:29-78,274-280`).
- Permissions: `core::auth::ghost::GhostAuthorizer{}.authorize(Principal{INTERNAL}, "previews","read")` implements `permissions:true` under the frontend proxy's trusted context, so anonymous preview links stay public (`execute.cppm:82-95`).
- Member context: `member_status=paid` reads store tiers and selects paid tiers by `member_tier` slug (or all active paid tiers) for the paid-member shim; `free` is a validated synthetic context (`execute.cppm:98-103,216-237`).
- DB: `core::query::apply(store().json_for("posts",{include,status:'all'}), q-with-uuid-predicate, unlimited)` realizes relation embedding and `findOne(status:'all')`; empty → 404 "Post not found." (`execute.cppm:107-121`).
- Output: `blog::mapping::clean(record,"post")` + `tiers_browse` (with visibility-based paid filter) + `blog::mapping::wrap("previews", mapped)`, `type` injected after mapping and legacy `access = !!html` fallback when no `member_status` (`execute.cppm:123-149`). Handler writes status/body only — no cache header (`handler.cppm:18-31`).

#### Fix (applied — all checks closed)

- Check 1 (route identity) — CLOSED: `include/blog/handlers/admin/previews/routes.cppm` (`blog_handlers_routes:previews_routes`) registers both GET shapes to `handler.previews_preview_get()`; no sitemap optional-segment syntax needed. Central registration done by main agent (`routes.cppm:8,136`). `content_type_middleware` deliberately omitted (public HTML route).
- Check 2 (operation) — CLOSED: `blog::handlers::previews::preview_get::PreviewGetHandler` + workflow `previews_preview_get` (`handler.cppm:10-35`, `flow/main.cppm:8-11`); getter/member at `all.cppm:618,897`, import at `:8`.
- Check 3 (input) — CLOSED: `core::query::parse` + Ghost value/shape validation, exact `member_tier` message, 422 envelopes (`execute.cppm:29-78,274-280`).
- Check 4 (output) — CLOSED: `blog::mapping::clean`/`wrap` → `{previews:[mappedPost]}` with `type` and access fallback; 404 on miss (`execute.cppm:117-149`).
- Check 5 (DB call) — CLOSED: store `json_for("posts",{include,status:'all'})` filtered by `core::query::apply` uuid NQL predicate with no pagination cap; tiers read for shim + mapper (`execute.cppm:98-103,216-237`).
- Check 6 (cache) — CLOSED: no `X-Cache-Invalidate` header written (`cacheInvalidate:false` is a no-header instruction); handler writes status/body only (`handler.cppm:18-31`).
- Check 7 (auth) — CLOSED: `permissions:true` via `core::auth::ghost` under the INTERNAL frontend-proxy principal (`execute.cppm:82-95`); no `require_auth` on the public route; paid-member shim store-backed.
- Check 8 (verdict + evidence) — CLOSED: re-grep confirms the two `/p` GET routes, `previews_preview_get()`, and `ExecuteStep`; verdict MATCH because no endpoint-level divergence remains.

## Missing in C++

None. `GET /p/:uuid/:options?` → `previews.read` is implemented: route, handler, workflow, step, store call, input validation, permissions check, member shim and mapped output all present. Verdict: MATCH.

## Extra in C++

None. The two `/p` GET routes are the Ghost frontend preview route (two literal shapes standing in for `:options?`); they are not extras. `email_previews`, `stripe/checkout/preview`, `automations/:id/email_preview` and `automated_emails/:id/preview` belong to other controllers and were not touched.

## Needs central registration

All central registrations are ALREADY DONE by the main agent; no shared file was touched by this rewrite. The shared abstractions used are `core_query` (`//include/core/query:core_query`), `core_auth:ghost` (`//include/core/auth:core_auth`), `blog_handlers:mapping` (`:clean`/`:wrap` partitions), and the existing `ctx.store()`; `core::adapters` has no applicable primitive on this read-only route.

Files in this folder (all partitions unique, all steps `import :domain_all;`, `Output = std::map<std::string,std::string>`, static `s_ctx`):

| File | Module partition | Role |
|---|---|---|
| `routes.cppm` | `blog_handlers_routes:previews_routes` | `blog::api::PreviewsRoutes::install` — GET `/p/:uuid` + `/p/:uuid/:options` → `handler.previews_preview_get()`; no JSON serde/auth middleware |
| `preview_get/handler.cppm` | `blog_handlers:previews_preview_get_handler` | `PreviewGetHandler` (workflow driver, status/body only, no cache headers) |
| `preview_get/workflow/flow/main.cppm` | `blog_handlers:previews_preview_get_flow` | `MainFlow` → workflow `previews_preview_get` |
| `preview_get/workflow/step/execute.cppm` | `blog_handlers:previews_preview_get_step` | `ExecuteStep` — `core::query` input/NQL, `core::auth::ghost` permissions, store-backed member shim + tiers browse, `blog::mapping` output |

Central registrations (present, recorded for the registrar):

1. `include/blog/handlers/all.cppm` — `import :previews_preview_get_handler;` (`:8`), getter `previews_preview_get()` (`:618`), member `m_previews_preview_get{m_ctx}` (`:897`).
2. `include/blog/handlers/routes.cppm` — `export import :previews_routes;` (`:8`), `PreviewsRoutes{}.install(router, handlers);` (`:136`).
3. `include/blog/handlers/base.cppm` — `export import :previews_preview_get_handler;` (`:249`).
4. `include/blog/handlers/BUILD.bazel` — step/flow/handler partitions listed (`:914-916`), `routes.cppm` in `blog_handlers_routes` (`:970`), deps include `core_query`/`core_auth`/`core_adapters`.
5. `include/blog/handlers/domain_all.cppm` — no change needed (no new domain model; the trio imports `:domain_all`).

Residuals (cross-layer, not endpoint divergences): theme HTML rendering (`renderer.renderEntry`), 301/302 redirect helpers, `next()` fallthrough, and site-URL-dependent computed `url` have no counterpart in this API-server codebase; verdict remains MATCH for the `previews.read` endpoint contract.
