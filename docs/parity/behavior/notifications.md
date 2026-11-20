# Behavior report: notifications

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/notifications.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 380–387); router-level `router.use(apiMw.cors)` at `routes.js:17`, router named `'admin api'` at `routes.js:15`
- Ghost service: `docs/Ghost/ghost/core/server/services/notifications/notifications.js`
- Ghost sanitizer: `docs/Ghost/ghost/core/server/services/notifications/sanitize-notification-html.ts`
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/notifications.js`
- C++ folder: `include/blog/handlers/admin/notifications/` (routes.cppm + browse/, create/, destroy/, service/)
- Shared abstractions: `core::query`, `core::auth::ghost` via `blog_handlers_middleware::require_ghost_permission`, `blog::mapping::wrap`, `core::outbox`, `core::adapters`, existing `ApiDataStore`/`ResourceStore`
- Analyzed: 2026-11-19; strict-parity rewrite: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain ↔ C++ strict implementation | Cache | Output | C++ files | Verdict |
|---|---|---|---|---|---|---|---|---|---|
| GET `/notifications` | `browse`, docName `notifications` | Ghost: no `options`/`data`; passes `frame.user.id`. C++: `core::query::parse` over the raw path (allowlisted `limit,page,offset,order,fields,filter`), `user_id` from `blog::handlers::middleware::token`. | Ghost: `mw.authAdminApi` + `permissions:true`. C++: router-level `require_auth` + `require_ghost_permission("notifications","browse")` (full middleware shape, Ghost 403/401 envelope). | Ghost: `cacheInvalidate:false`, default 200. C++: 200 + `X-Cache-Invalidate: false`. | Ghost: semver/seen filter, `_.orderBy addedAt desc`, `sanitizeMessage` on read. C++: `NotificationService::browse` reads the settings row from the existing `ResourceStore`, sanitizes HTML, orders by `addedAt desc`, applies version + legacy release/security regex + `wasSeen` filters, then `core::query::apply`. | Ghost `cacheInvalidate:false`; C++ header only, no eviction. | Ghost serializer strips `seen/seenBy/addedAt/createdAtVersion`, wraps `{notifications:[...]}`, **no pagination meta**. C++ identical via `blog::mapping::wrap("notifications", rows)`. | `routes.cppm`, `browse/{handler,workflow/step/execute}.cppm`, `service/notifications_service.cppm` | MATCH |
| POST `/notifications` | `add`, docName `notifications` | Ghost: `frame.data.notifications`. C++: `JsonBody::parse` field `"notifications"` via `core::query`-compatible raw JSON, `blog::api::parse_array`. | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `require_auth` + `require_ghost_permission("notifications","add")`. | Ghost: 201 if `notifications.length` else 200; `cacheInvalidate:false`. C++: status from `NotificationService::add` (201/200/422), `X-Cache-Invalidate: false`. | Ghost service `add`: defaults, overrides, dedup, non-string message `ValidationError` 422, `sanitizeMessage`, release removal/reorder; settings BREAD edit. C++ mirrors all of it, persists via `ResourceStore` and records the settings call through `core::adapters::record_outbound` + `core::outbox` (`settings.edit`). | header only. | Ghost: full added objects post-serializer. C++: full objects (defaults applied, internal fields stripped) via `blog::mapping::wrap`. | `routes.cppm`, `create/{handler,workflow/step/execute}.cppm`, `service/notifications_service.cppm` | MATCH |
| DELETE `/notifications/:notification_id` | `destroy`, docName `notifications` | Ghost: `options:['notification_id']`, `validation.required`, `frame.options.notification_id` + `frame.user.id`. C++: trailing path segment from `IRequest::get_path()` (empty → 422 validation), `user_id` from token. | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `require_auth` + `require_ghost_permission("notifications","destroy")`. | Ghost: 204 empty; `cacheInvalidate:false`. C++: 204 empty on success, 403/404 Ghost error envelopes, `X-Cache-Invalidate: false`. | Ghost service `destroy`: dismissible `NoPermissionError` 403, missing `NotFoundError` 404, `seen=true` + push `seenBy`; settings BREAD edit. C++ identical semantics + outbox side effect. | header only. | 204 empty body (errors → Ghost envelope). | `routes.cppm`, `destroy/{handler,workflow/step/execute}.cppm`, `service/notifications_service.cppm` | MATCH |

## Route details

### GET /notifications — MATCH

**Ghost** (numbered 1–8): as previously documented (`notifications.js:10-22`, `service:39-61,85-138`, `output/notifications.js:4-28`): `authAdminApi` + `permissions:true`; `cacheInvalidate:false`; service fetches settings notifications, sanitizes on read, orders by `addedAt desc`, filters by `createdAtVersion`/semver and legacy release/security regexes plus `wasSeen`; serializer strips `seen/seenBy/addedAt/createdAtVersion` and emits `{notifications:[...]}` with no pagination meta.

**C++** (numbered):
1. Identity: `Router{router,"/notifications"}.get(...)`; sub-route installed via `NotificationsRoutes{}.install` (`handlers/routes.cppm:37,109`). Placeholder for DELETE renamed to `/:notification_id` (`routes.cppm`).
2. Operation: `notifications_browse()` → `BrowseHandler::handle` → workflow `notifications_browse` → step `notifications_browse_execute`.
3. Input: `core::query::parse(path, {limit,page,offset,order,fields,filter})`; `params["user_id"]` no longer fabricated — user scope from `blog::handlers::middleware::token(in)` passed to the service.
4. Auth/permissions: router-level `content_type_middleware` + `require_auth` + `require_ghost_permission("notifications","browse")` via the shared `GhostAuthorizer` (401/403 Ghost envelope on denial).
5. Status/headers: 200, `X-Cache-Invalidate: false`.
6. Full call chain: `NotificationService::browse` → `fetch_all` reads the `notification_settings` settings row from the existing `ResourceStore` (no static fixture), `sanitize_html` on every message, `stable_sort` by `addedAt desc`, `visible()` applies Ghost's version/legacy/`wasSeen` rules, `core::query::apply` handles explicit NQL options, `serialize_clean` + `blog::mapping::wrap`.
7. Cache: response header only; no `core::cache::Cache` eviction (none on Ghost side either).
8. Output: `{"notifications":[...]}` — no fabricated `meta.pagination`.

**Diff**: Resolved. The former static fixture, discarded params, fabricated pagination, raw-token-as-user-id and missing permission service are gone. `core::query` owns NQL parsing; `core::auth::ghost` owns permission decisions; `blog::mapping::wrap` owns the envelope.

### POST /notifications — MATCH

**Ghost** (numbered 1–8): `authAdminApi` + `permissions:true`; `statusCode` 201/200; `cacheInvalidate:false`; `notifications.add` defaults/overrides/dedup/validation/sanitize/release handling; settings BREAD edit with `internalContext`; serializer returns full added objects.

**C++** (numbered):
1. `.post(...)` on the notifications router with the same middleware chain plus `require_ghost_permission("notifications","add")`.
2. `notifications_create()` → `CreateHandler::handle` → workflow `notifications_create` → step `notifications_create_execute`.
3. Input: `blog::api::JsonBody{}.parse(in.body_as_string())`, field `"notifications"`, `blog::api::parse_array`; empty → `{"notifications":[]}` 200.
4. Auth/permissions: as GET with method `add`.
5. Status/headers: step returns `{status,body}`; handler maps status (201/200/422) and sets `X-Cache-Invalidate: false`.
6. Full call chain: `NotificationService::add` applies `{dismissible:true, location:'bottom', status:'alert', id:ObjectId, createdAtVersion:6.78.0-rc.0}` defaults, `{seen:false, addedAt:now}` overrides, dedup by id, `ValidationError` 422 for non-string message, `sanitize_html`, removal of existing release notifications when a non-custom one arrives, and newest-release reorder; persists the concatenated array to `ResourceStore`; records `settings.edit` via `core::adapters::record_outbound` and drains `core::outbox::global_outbox()`.
7. Cache: header only.
8. Output: full added objects (internal fields stripped) wrapped by `blog::mapping::wrap("notifications", ...)`.

**Diff**: Resolved. Full objects replace id-only records; defaults, validation, sanitization, dedup, release logic and settings-side-effect persistence are present.

### DELETE /notifications/:notification_id — MATCH

**Ghost** (numbered 1–8): `options:['notification_id']` required; `authAdminApi` + `permissions:true`; 204 no body; `cacheInvalidate:false`; dismissible check → 403 `NoPermissionError`, missing → 404 `NotFoundError`, otherwise `seen=true` + push `seenBy`, settings BREAD edit.

**C++** (numbered):
1. `Route{"/:notification_id"}.delt(...)` — placeholder name now exact.
2. `notifications_destroy()` → `DestroyHandler::handle` → workflow `notifications_destroy` → step `notifications_destroy_execute`.
3. Input: trailing path segment parsed from `IRequest::get_path()` with query/trailing-slash stripping; empty id → 422 `ApiError.validation` on `notification_id`; user from token.
4. Auth/permissions: as GET with method `destroy`.
5. Status/headers: 204 empty on success; 403/404 envelopes from the service; `X-Cache-Invalidate: false`.
6. Full call chain: `NotificationService::destroy` → `fetch_all`, find by id, dismissible 403, missing 404, `wasSeen` short-circuit otherwise `seen=true` and `seenBy.push(user_id)`, persist + outbox side effect.
7. Cache: header only.
8. Output: 204 empty body on success; Ghost-shaped error envelope otherwise.

**Diff**: Resolved. The placeholder name, permission guard, dismissible/not-found branches, seen/seenBy marking and settings-side-effect persistence now match Ghost. `resource:notification_settings:*` is the existing-store namespace used for the persisted notifications setting; no new global state is introduced.

## Module-level notes

- Three HTTP-exposed Ghost routes, all present and matched. `destroyAll` is `@private Not exposed over HTTP` — correctly not a route.
- Strict parity is implemented in one new partition, `blog_handlers:notifications_service` (`service/notifications_service.cppm`), reused by all three steps. It is the single source of notification domain logic.
- Shared abstractions used: `core::query` (NQL parse/filter/projection), `core::auth::ghost` + `require_ghost_permission` (per-route permissions), `blog::mapping::wrap` (Ghost output envelope), `core::outbox` + `core::adapters` (settings-edit side effect), existing `ApiDataStore::resources()` (persistence).
- All step outputs are `std::string` (browse) or `std::map<std::string,std::string>` (create/destroy), with `static blog::BlogContext* s_ctx` set by the handler constructor — matching the manifest contract.
- Accepted residual differences that are not observable on the wire: Ghost's literal `settings` BREAD row is represented as an equivalent single `notification_settings` record plus an outbox `settings.edit` event (no settings DTO primitive exists); `frame.user.id` is the bearer-token identity primitive (no token→user model), as documented in CONTRACTS.md.
- No MISSING or EXTRA routes. No shared file was edited.

## Central registration record (deferred to central pass)

- Add `admin/notifications/service/notifications_service.cppm` to the `partitions` list of `congelado_module_library(name = "blog_handlers")` in `include/blog/handlers/BUILD.bazel`. No `export import` in `base.cppm` is required — only `notifications_*_step` imports `:notifications_service` directly. Optional if the central pass prefers a public re-export: `export import :notifications_service;`.
- Existing deps already cover this group: `//include/blog/handlers:blog_handlers` already depends on `core_query`, `core_outbox`, `core_adapters`, `core_auth`, `blog_handlers_middleware`; `blog_handlers_routes` already depends on `blog_handlers_middleware`. No root BUILD.bazel change.
- `all.cppm`, central `routes.cppm`, `base.cppm`, `middleware/` and `context_context.cppm` are untouched by this group.

## Files changed

- `include/blog/handlers/admin/notifications/routes.cppm` — per-verb Ghost permission middleware; DELETE placeholder renamed to `/:notification_id`.
- `include/blog/handlers/admin/notifications/browse/workflow/step/execute.cppm` — delegates to `NotificationService::browse` with `core::query`-parsed path and token user id.
- `include/blog/handlers/admin/notifications/create/workflow/step/execute.cppm` — delegates to `NotificationService::add`.
- `include/blog/handlers/admin/notifications/destroy/workflow/step/execute.cppm` — path-id validation + delegates to `NotificationService::destroy`.
- `include/blog/handlers/admin/notifications/service/notifications_service.cppm` (new) — strict Ghost domain logic with shared abstractions.
- `docs/parity/behavior/notifications.md`, `docs/parity/notifications.md` — verdicts and evidence updated.
