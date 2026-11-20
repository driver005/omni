# Parity: notifications

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/notifications.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- Ghost service/sanitizer: `docs/Ghost/ghost/core/server/services/notifications/notifications.js`, `sanitize-notification-html.ts`
- C++: `include/blog/handlers/admin/notifications/`
- Checked: 2026-11-19; strict-parity rewrite: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/notifications` | `browse` | `.get` on `Router{router,"/notifications"}` | `notifications_browse()` | Ghost: passes `frame.user.id` (`notifications.js:15-20`). C++: `core::query::parse` of the path + token user id into `NotificationService::browse`. | Ghost: `{notifications:[...]}`, no meta (`output/notifications.js:4-28`). C++: `blog::mapping::wrap("notifications", rows)` — same. | Ghost `notifications.browse` + settings cache; C++ `NotificationService::browse` reads persisted setting from `ResourceStore`, version/seen filters, addedAt desc, HTML sanitize. | Ghost `cacheInvalidate:false`; C++ `X-Cache-Invalidate: false`. | Ghost `mw.authAdminApi` + `permissions:true`; C++ `require_auth` + `require_ghost_permission("notifications","browse")`. | MATCH | Ghost `notifications.js:10-22`, `routes.js:381`, service `:39-61,85-138`, `output/notifications.js:4-28`; C++ `routes.cppm`, `browse/.../execute.cppm`, `service/notifications_service.cppm` |
| POST `/notifications` | `add` | `.post` on `Router{router,"/notifications"}` | `notifications_create()` | Ghost: `frame.data.notifications`. C++: `JsonBody::parse` → `"notifications"` → `parse_array` → `NotificationService::add`. | Ghost: 201 if added else 200, full objects; C++: same via `blog::mapping::wrap`. | Ghost: `notifications.add` (defaults/overrides/dedup/validate/sanitize/release) + settings BREAD edit; C++: identical logic, persists to `ResourceStore` + `core::outbox`/`core::adapters` side effect. | header only. | Ghost `permissions:true`; C++ `require_ghost_permission("notifications","add")`. | MATCH | Ghost `notifications.js:24-55`, service `:140-206`; C++ `create/.../execute.cppm`, `service/notifications_service.cppm` |
| DELETE `/notifications/:notification_id` | `destroy` | `.delt` on `Route{"/:notification_id"}` | `notifications_destroy()` | Ghost: `options:['notification_id']`, required, `frame.options.notification_id` + user id. C++: trailing path id with 422 validation + token user id → `NotificationService::destroy`. | Ghost: 204 empty; C++: 204 empty / 403/404 Ghost envelope. | Ghost: dismissible 403, missing 404, seen+seenBy, settings BREAD edit; C++: identical + outbox side effect. | header only. | Ghost `permissions:true`; C++ `require_ghost_permission("notifications","destroy")`. | MATCH | Ghost `notifications.js:57-89`, service `:217-258`; C++ `destroy/.../execute.cppm`, `service/notifications_service.cppm` |

## Route details

### GET /notifications — MATCH

- Verbatim paths match: Ghost `/notifications` (`routes.js:381`) ↔ `Router{router,"/notifications"}.get(...)`.
- Op mapping: `notifications.browse` → `notifications_browse()` → workflow `notifications_browse` → step `notifications_browse_execute`. Agrees.
- Input: `core::query::parse(path, {limit,page,offset,order,fields,filter})`; user scope from `blog::handlers::middleware::token(in)` (Ghost `frame.user.id`). No fabricated `user_id` parameter.
- Output: `NotificationService::browse` returns `blog::mapping::wrap("notifications", rows)` with the serializer field strip (`seen/seenBy/addedAt/createdAtVersion`) and **no pagination meta** — matching `output/notifications.js`. The previously fabricated `meta.pagination` is removed.
- DB call: reads the persisted notifications setting via the existing `ResourceStore` rather than the static `NotificationWire` fixture; applies Ghost's addedAt-desc ordering, semver/version filter, legacy release/security regex rules and `wasSeen`.
- Cache: `X-Cache-Invalidate: false`.
- Auth: `content_type_middleware` + `require_auth` + shared Ghost permission service for `{notifications,browse}` — closes the former permission gap.

#### Fix — APPLIED

- Checks 1–2: routes unchanged (no add/move); DELETE placeholder renamed to `/:notification_id` for exactness.
- Check 3: browse step parses the path with `core::query::parse` and passes `user_id` from `token(in)` to `NotificationService::browse`.
- Check 5: static fixture removed from this group's code path; service reads/writes the settings row via `ResourceStore` (`resource:notification_settings:*`). Settings persistence is represented as an equivalent single row — no shared `context_context.cppm` edit needed.
- Check 4: output built by `blog::mapping::wrap`, no fabricated pagination.
- Check 6: `X-Cache-Invalidate: false` retained.
- Check 7: router-level `require_ghost_permission("notifications","browse")` added; Ghost 401/403 envelopes emitted by the shared middleware.

### POST /notifications — MATCH

- Verbatim paths and op mapping agree (`add` ↔ `create` handler name; workflow `notifications_create`).
- Input: structured `data.notifications` via `JsonBody::parse` + `parse_array`; empty → 200 `{"notifications":[]}`.
- Output/status: `NotificationService::add` returns 201 with full added objects post-serializer, 200 for empty/duplicate-only; 422 Ghost `ValidationError` for non-string message.
- DB call: defaults `{dismissible:true, location:'bottom', status:'alert', id:ObjectId, createdAtVersion:6.78.0-rc.0}`, overrides `{seen:false, addedAt:now}`, dedup by id, `sanitize_html`, release removal/reorder, then persist to `ResourceStore` and record `settings.edit` to `core::outbox` via `core::adapters::record_outbound`.
- Cache/auth: `X-Cache-Invalidate: false`; `require_ghost_permission("notifications","add")`.

#### Fix — APPLIED

- Checks 1–2: no route change; `add` → `create` mapping retained.
- Check 3: envelope unwrapped and each entry passed to `NotificationService::add`.
- Check 5: settings-side-effect persistence implemented through the existing store + outbox/adapters; id-only records replaced by full objects.
- Check 4: handler maps step status/body; 201/200/422 as Ghost.
- Check 6/7: cache header and Ghost permission guard applied.

### DELETE /notifications/:notification_id — MATCH

- Verbatim paths: Ghost `/notifications/:notification_id` ↔ `Route{"/:notification_id"}.delt(...)`. Placeholder name now exact.
- Op mapping: `notifications.destroy` → `notifications_destroy()` → workflow `notifications_destroy` → step `notifications_destroy_execute`.
- Input: trailing path id via `IRequest::get_path()` with query/trailing-slash stripping; empty → 422 Ghost `ValidationError` on `notification_id`; user from token.
- Output/status: 204 empty on success; 403 `NoPermissionError` for non-dismissible, 404 `NotFoundError` for missing — both Ghost envelopes.
- DB call: `NotificationService::destroy` marks `seen=true`, pushes `user.id` into `seenBy`, persists the array and emits the `settings.edit` side effect.
- Cache/auth: `X-Cache-Invalidate: false`; `require_ghost_permission("notifications","destroy")`.

#### Fix — APPLIED

- Checks 1–2: route retained; placeholder renamed for exactness.
- Check 3: path id + validation implemented; no empty-id store call.
- Check 5: seen/seenBy marking and settings-side-effect persistence implemented via the existing store + outbox.
- Check 4: handler maps step status; 204 + empty body on success, error envelope otherwise.
- Check 6/7: cache header and Ghost permission guard applied.

## Missing in C++

None. Ghost `destroyAll` is `@private Not exposed over HTTP` and is not an HTTP route; correctly not counted as MISSING.

## Extra in C++

None. The folder installs exactly the three Ghost-matched routes.

## Needs central registration

Applied in this group (all files under `include/blog/handlers/admin/notifications/`):

- `routes.cppm` — router-level `require_ghost_permission` dispatch by verb for `{notifications, browse|add|destroy}`; DELETE placeholder renamed to `/:notification_id`.
- `browse/workflow/step/execute.cppm`, `create/workflow/step/execute.cppm`, `destroy/workflow/step/execute.cppm` — delegate domain logic to `NotificationService`; Output is `std::string` (browse) or `std::map<std::string,std::string>` (create/destroy); each keeps `static blog::BlogContext* s_ctx`.
- `service/notifications_service.cppm` (new partition `blog_handlers:notifications_service`) — strict Ghost domain logic: settings fetch/persist on the existing `ResourceStore`, HTML sanitization, addedAt-desc ordering, semver + legacy release/security filters, `wasSeen`, add defaults/overrides/dedup/validation/release reorder, destroy dismissible/seen/seenBy, Ghost error envelopes, `core::query` NQL pipeline, `blog::mapping::wrap` output, and settings-edit side effects through `core::adapters::record_outbound` + `core::outbox::global_outbox()`.

Deferred to the central pass (shared files untouched by this group):

1. Add `admin/notifications/service/notifications_service.cppm` to the `partitions` list of `congelado_module_library(name = "blog_handlers")` in `include/blog/handlers/BUILD.bazel`. No `export import :notifications_service;` in `base.cppm` is required (only the three steps import the partition directly); add one only if the central pass wants a public re-export.
2. No new deps: `blog_handlers` already lists `core_query`, `core_outbox`, `core_adapters`, `core_auth` and `blog_handlers_middleware`; `blog_handlers_routes` already lists `blog_handlers_middleware`. No root `BUILD.bazel` change.

Already registered — no action: `all.cppm` imports the three `notifications_*_handler` partitions and owns `notifications_browse()/create()/destroy()` plus their members; `handlers/routes.cppm:37,109` re-exports and installs `NotificationsRoutes`; `BUILD.bazel:628-636,999` lists the pre-existing ten notification `.cppm` files.

Accepted residual (not wire-observable): Ghost's literal `settings` BREAD row is modeled as an equivalent single `notification_settings` record plus a `settings.edit` outbox event (no settings DTO primitive exists), and `frame.user.id` is the bearer-token identity primitive (no token→user model) — both consistent with `CONTRACTS.md`.
