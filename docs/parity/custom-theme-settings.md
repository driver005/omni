# Parity: custom-theme-settings

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/custom-theme-settings.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/custom_theme_settings/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/custom_theme_settings` | `browse` (`listSettings()`) | GET `/custom_theme_settings` (`routes.cppm:19,21-27`) | `custom_theme_settings_browse()` → `BrowseHandler` (`all.cppm:570`, `browse/handler.cppm:10`) | Ghost: no `options`/`data`/`validation` declared (`custom-theme-settings.js:7-15`). C++: `using Input=IRequest&`, no query parsing (`browse/.../execute.cppm:11`). Agree (none). | DIVERGE: Ghost returns `listSettings()` array of setting objects (`custom-theme-settings.js:13`; service `:89-95`). C++ returns hardcoded `{"custom_theme_settings":[<fixture>]}` with no `meta` (`browse/.../execute.cppm:15-21`); the `meta.pagination` branch at `:22-23` is unreachable. | DIVERGE: Ghost reads in-memory `_activeThemeSettings` populated from repository/BREAD at theme activation (`service.js:89-95,311-333`), no per-request DB. C++ `ctx.store().json_for("custom_theme_settings", {})` (`execute.cppm:22`) is dead code; effective call = none, body is `GhostFixtures::custom_theme_setting()` (`:18`). | DIVERGE: Ghost `cacheInvalidate:false` (`custom-theme-settings.js:8-10`) → no `X-Cache-Invalidate`. C++ none (handler constructed with `m_ctx` only, `all.cppm:819`; no `core::cache::Cache` wired). | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:575`) + `permissions:true`, `docName:'custom_theme_settings'` (`custom-theme-settings.js:5,11`). C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`); no permissions/docName. | PARTIAL | Ghost: `custom-theme-settings.js:5,7-15`; `routes.js:575`; `custom-theme-settings-service.js:89-95,153-172`. C++: `routes.cppm:19-28`; `browse/handler.cppm:10-27`; `browse/.../execute.cppm:11-23`; `all.cppm:570,819`. |
| PUT `/custom_theme_settings` | `edit` (`updateSettings(frame.data.custom_theme_settings)`) | PUT `/custom_theme_settings` (`routes.cppm:19,29-35`) | `custom_theme_settings_bulk_edit()` → `BulkEditHandler` (`all.cppm:571`, `bulk_edit/handler.cppm:10`) | DIVERGE: Ghost reads `frame.data.custom_theme_settings` body wrapper (`custom-theme-settings.js:22-24`). C++ passes raw `in.body_as_string()` with no wrapper extraction (`bulk_edit/.../execute.cppm:11,20`); mutation unreachable. | DIVERGE: Ghost returns full updated settings objects (`custom-theme-settings.js:23`; `service.js:165-171`). C++ returns hardcoded `{"custom_theme_settings":[],"meta":{}}` (`bulk_edit/.../execute.cppm:15-18`) via early return at `:18`; no updated data. | DIVERGE: Ghost `_repository.read` + `settingRecord.save` per setting (`service.js:153-159`). C++ `mutations.bulk_edit("custom_theme_settings", ...)` (`execute.cppm:20`) unreachable → no write; would wrap `{"meta":{"updated":N}}` (`context.cppm:1362-1366`). | DIVERGE: Ghost `cacheInvalidate:true` (`custom-theme-settings.js:18-20`) → `X-Cache-Invalidate`; service also updates internal cache (`service.js:162`) and public cache `_valueCache.populate` (`:168`). C++ none (`all.cppm:820`; no cache wired). | DIVERGE: same as GET — Ghost `mw.authAdminApi` (`routes.js:576`) + `permissions:true` (`custom-theme-settings.js:21`); C++ `content_type_middleware` (`routes.cppm:20`) + token check → 401 (`bulk_edit/handler.cppm:18-22`); no permissions/docName. | PARTIAL | Ghost: `custom-theme-settings.js:5,17-24`; `routes.js:576`; `custom-theme-settings-service.js:100-172`. C++: `routes.cppm:29-36`; `bulk_edit/handler.cppm:10-27`; `bulk_edit/.../execute.cppm:11-20`; `all.cppm:571,820`. |

## Route details

### GET `/custom_theme_settings` — PARTIAL

- Route identity agrees: Ghost `routes.js:575` `GET /custom_theme_settings` with `mw.authAdminApi`; C++ `routes.cppm:19,21-27` `Router{..."/custom_theme_settings"}.get(...)`.
- Operation agrees: Ghost `browse` → `customThemeSettingsService.service.listSettings()` (`custom-theme-settings.js:12-14`); C++ `.get` binds `custom_theme_settings_browse()` (`routes.cppm:26`) which runs workflow `custom_theme_settings_browse` (`browse/handler.cppm:23`; `browse/workflow/flow/main.cppm:9`).
- Input: no divergence — neither side declares query options/data/validation; C++ `using Input=IRequest&` with no parsing (`browse/.../execute.cppm:11`).
- Output diverges: Ghost returns the `listSettings()` array of full setting objects (`custom-theme-settings.js:13`; `service.js:89-95`). C++ returns a single hardcoded fixture object in `{"custom_theme_settings":[...]}` (`browse/.../execute.cppm:16-20`), status 200 with no `meta` — the `meta.pagination` envelope at `:23` is unreachable because `r.first==200` triggers early return at `:21`.
- DB call diverges: Ghost reads the in-memory `_activeThemeSettings` map, populated from the BREAD repository on theme activation (`service.js:89-95,311-333`); no per-request DB read. C++ `ctx.store().json_for("custom_theme_settings", {})` (`execute.cppm:22`) is never executed; the effective data source is `GhostFixtures::custom_theme_setting()` (`:18`). No real filtering/pagination.
- Cache diverges: Ghost `headers.cacheInvalidate:false` (`custom-theme-settings.js:8-10`), so no `X-Cache-Invalidate` emitted. C++ has no cache behavior — handlers are constructed with `m_ctx` only (`all.cppm:819-820`), the `core::cache::Cache` at `all.cppm:836` is not passed to these handlers.
- Auth diverges: Ghost admin API middleware `mw.authAdminApi` (`routes.js:575`) plus framework permissions (`permissions:true`, `docName:'custom_theme_settings'`, `custom-theme-settings.js:5,11`). C++ only applies `content_type_middleware` (`routes.cppm:20`) and an in-handler token-presence check returning 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; token read via `middleware/bearer.cppm:27-33`); no permissions/docName/`unsafeAttrs` model.

### PUT `/custom_theme_settings` — PARTIAL

- Route identity agrees: Ghost `routes.js:576` `PUT /custom_theme_settings` with `mw.authAdminApi`; C++ `routes.cppm:19,29-35` `Router{..."/custom_theme_settings"}.put(...)`.
- Operation is the same domain intent but named differently: Ghost `edit` → `updateSettings(frame.data.custom_theme_settings)` (`custom-theme-settings.js:22-24`); C++ `.put` binds `custom_theme_settings_bulk_edit()` (`routes.cppm:34`) running workflow `custom_theme_settings_bulk_edit` (`bulk_edit/handler.cppm:23`; `bulk_edit/workflow/flow/main.cppm:9`). Both update a set of theme settings.
- Input diverges: Ghost extracts `frame.data.custom_theme_settings` from the request body (`custom-theme-settings.js:23`). C++ passes the entire raw `in.body_as_string()` with no wrapper extraction (`bulk_edit/.../execute.cppm:11,20`); this call is also unreachable.
- Output diverges: Ghost returns full updated setting objects (`custom-theme-settings.js:23`; `service.js:165-171`). C++ returns hardcoded `{"custom_theme_settings":[],"meta":{}}` (`bulk_edit/.../execute.cppm:15-18`) via early return at `:18` — no updated data, no real envelope.
- DB call diverges: Ghost validates keys/values then `_repository.read` + `settingRecord.save` per setting (`service.js:100-159`). C++ `mutations.bulk_edit("custom_theme_settings", ...)` (`execute.cppm:20`) is never executed (early return `:18`), so no write occurs; the wrapped result would be `{"meta":{"updated":N}}` (`context.cppm:1362-1366`), which does not match Ghost's returned settings objects.
- Cache diverges: Ghost `headers.cacheInvalidate:true` (`custom-theme-settings.js:18-20`) emits cache invalidation; service additionally mutates internal cache (`service.js:162`) and repopulates the public `_valueCache` (`:168`). C++ has no cache behavior (`all.cppm:820`, no `core::cache::Cache` wired).
- Auth diverges: same as GET — Ghost `mw.authAdminApi` (`routes.js:576`) + `permissions:true` (`custom-theme-settings.js:21`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check → 401 (`bulk_edit/handler.cppm:18-22`); no permissions/docName.

## Missing in C++

None. Both Ghost admin routes have C++ route/handler counterparts:

- `GET /custom_theme_settings` → `include/blog/handlers/system/custom_theme_settings/routes.cppm:21-27`, `browse/handler.cppm:10`.
- `PUT /custom_theme_settings` → `include/blog/handlers/system/custom_theme_settings/routes.cppm:29-35`, `bulk_edit/handler.cppm:10`.

Confirmed by searching all `routes.cppm` files for `custom_theme_settings`/`customThemeSettings`: only `include/blog/handlers/system/custom_theme_settings/routes.cppm` (plus central install references in `include/blog/handlers/routes.cppm:11,84`).

## Extra in C++

None. The matched folder `include/blog/handlers/system/custom_theme_settings/routes.cppm` installs exactly two routes (GET and PUT at `/custom_theme_settings`, lines 21-35), both with Ghost counterparts. No third C++ route exists in that folder.
