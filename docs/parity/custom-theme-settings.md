# Parity: custom-theme-settings

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/custom-theme-settings.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/custom_theme_settings/routes.cppm`
- Checked: 2026-11-19

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
| GET `/custom_theme_settings` | `browse` (`listSettings()`) | GET `/custom_theme_settings` (`routes.cppm:19,21-27`) | `custom_theme_settings_browse()` → `BrowseHandler` (`all.cppm:570`, `browse/handler.cppm:10`) | Ghost: no `options`/`data`/`validation` declared (`custom-theme-settings.js:7-15`). C++: `using Input=IRequest&`, no query parsing (`browse/.../execute.cppm:11`). Agree (none). | DIVERGE: Ghost returns `listSettings()` array of setting objects (`custom-theme-settings.js:13`; service `:89-95`). C++ returns hardcoded `{"custom_theme_settings":[<fixture>]}` with no `meta` (`browse/.../execute.cppm:15-21`); the `meta.pagination` branch at `:22-23` is unreachable. | DIVERGE: Ghost reads in-memory `_activeThemeSettings` populated from repository/BREAD at theme activation (`service.js:89-95,311-333`), no per-request DB. C++ `ctx.store().json_for("custom_theme_settings", {})` (`execute.cppm:22`) is dead code; effective call = none, body is `GhostFixtures::custom_theme_setting()` (`:18`). | DIVERGE: Ghost `cacheInvalidate:false` (`custom-theme-settings.js:8-10`) → no `X-Cache-Invalidate`. C++ none (handler constructed with `m_ctx` only, `all.cppm:819`; no `core::cache::Cache` wired). | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:575`) + `permissions:true`, `docName:'custom_theme_settings'` (`custom-theme-settings.js:5,11`). C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`); no permissions/docName. | MATCH | Ghost: `custom-theme-settings.js:5,7-15`; `routes.js:575`; `custom-theme-settings-service.js:89-95,153-172`. C++: `routes.cppm:19-28`; `browse/handler.cppm:10-27`; `browse/.../execute.cppm:11-23`; `all.cppm:570,819`. |
| PUT `/custom_theme_settings` | `edit` (`updateSettings(frame.data.custom_theme_settings)`) | PUT `/custom_theme_settings` (`routes.cppm:19,29-35`) | `custom_theme_settings_bulk_edit()` → `BulkEditHandler` (`all.cppm:571`, `bulk_edit/handler.cppm:10`) | DIVERGE: Ghost reads `frame.data.custom_theme_settings` body wrapper (`custom-theme-settings.js:22-24`). C++ passes raw `in.body_as_string()` with no wrapper extraction (`bulk_edit/.../execute.cppm:11,20`); mutation unreachable. | DIVERGE: Ghost returns full updated settings objects (`custom-theme-settings.js:23`; `service.js:165-171`). C++ returns hardcoded `{"custom_theme_settings":[],"meta":{}}` (`bulk_edit/.../execute.cppm:15-18`) via early return at `:18`; no updated data. | DIVERGE: Ghost `_repository.read` + `settingRecord.save` per setting (`service.js:153-159`). C++ `mutations.bulk_edit("custom_theme_settings", ...)` (`execute.cppm:20`) unreachable → no write; would wrap `{"meta":{"updated":N}}` (`context.cppm:1362-1366`). | DIVERGE: Ghost `cacheInvalidate:true` (`custom-theme-settings.js:18-20`) → `X-Cache-Invalidate`; service also updates internal cache (`service.js:162`) and public cache `_valueCache.populate` (`:168`). C++ none (`all.cppm:820`; no cache wired). | DIVERGE: same as GET — Ghost `mw.authAdminApi` (`routes.js:576`) + `permissions:true` (`custom-theme-settings.js:21`); C++ `content_type_middleware` (`routes.cppm:20`) + token check → 401 (`bulk_edit/handler.cppm:18-22`); no permissions/docName. | MATCH | Ghost: `custom-theme-settings.js:5,17-24`; `routes.js:576`; `custom-theme-settings-service.js:100-172`. C++: `routes.cppm:29-36`; `bulk_edit/handler.cppm:10-27`; `bulk_edit/.../execute.cppm:11-20`; `all.cppm:571,820`. |

## Route details

### GET `/custom_theme_settings` — MATCH

- Route identity agrees: Ghost `routes.js:575` `GET /custom_theme_settings` with `mw.authAdminApi`; C++ `routes.cppm:19,21-27` `Router{..."/custom_theme_settings"}.get(...)`.
- Operation agrees: Ghost `browse` → `customThemeSettingsService.service.listSettings()` (`custom-theme-settings.js:12-14`); C++ `.get` binds `custom_theme_settings_browse()` (`routes.cppm:26`) which runs workflow `custom_theme_settings_browse` (`browse/handler.cppm:23`; `browse/workflow/flow/main.cppm:9`).
- Input: no divergence — neither side declares query options/data/validation; C++ `using Input=IRequest&` with no parsing (`browse/.../execute.cppm:11`).
- Output diverges: Ghost returns the `listSettings()` array of full setting objects (`custom-theme-settings.js:13`; `service.js:89-95`). C++ returns a single hardcoded fixture object in `{"custom_theme_settings":[...]}` (`browse/.../execute.cppm:16-20`), status 200 with no `meta` — the `meta.pagination` envelope at `:23` is unreachable because `r.first==200` triggers early return at `:21`.
- DB call diverges: Ghost reads the in-memory `_activeThemeSettings` map, populated from the BREAD repository on theme activation (`service.js:89-95,311-333`); no per-request DB read. C++ `ctx.store().json_for("custom_theme_settings", {})` (`execute.cppm:22`) is never executed; the effective data source is `GhostFixtures::custom_theme_setting()` (`:18`). No real filtering/pagination.
- Cache diverges: Ghost `headers.cacheInvalidate:false` (`custom-theme-settings.js:8-10`), so no `X-Cache-Invalidate` emitted. C++ has no cache behavior — handlers are constructed with `m_ctx` only (`all.cppm:819-820`), the `core::cache::Cache` at `all.cppm:836` is not passed to these handlers.
- Auth diverges: Ghost admin API middleware `mw.authAdminApi` (`routes.js:575`) plus framework permissions (`permissions:true`, `docName:'custom_theme_settings'`, `custom-theme-settings.js:5,11`). C++ only applies `content_type_middleware` (`routes.cppm:20`) and an in-handler token-presence check returning 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; token read via `middleware/bearer.cppm:27-33`); no permissions/docName/`unsafeAttrs` model.

#### Fix

Target: `include/blog/handlers/system/custom_theme_settings/browse/workflow/step/execute.cppm` — `blog::handlers::custom_theme_settings::browse::workflow::ExecuteStep::invoke` (`:11-23`); store branch in `include/blog/handlers/context/context_context.cppm` (`ApiDataStore::json_for`, `context.cppm:1139-1198`); header in `browse/handler.cppm:10-27`; auth in `system/custom_theme_settings/routes.cppm:19-28`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /custom_theme_settings` (`routes.js:575`) ↔ `Router{router,"/custom_theme_settings"}.get(...)` (`routes.cppm:19,21-27`).
- 2 (operation): no change — `browse` → `listSettings()` (`custom-theme-settings.js:12-14`) ↔ `handler.custom_theme_settings_browse()` (`routes.cppm:26`, `all.cppm:570`).
- 3 (input): no change — no params on either side (`custom-theme-settings.js:7-15`; `using Input=IRequest&`, `execute.cppm:11`).
- 4 (output): replace `GhostFixtures::custom_theme_setting()` (`execute.cppm:18`) and the unreachable `meta.pagination` branch (`:22-23`) with the real rows from check 5; emit `{"custom_theme_settings":[...]}` with status 200 and no `meta`, matching Ghost's `listSettings()` array wrapped by docName (`output/` serializer).
- 5 (DB call): add a `"custom_theme_settings"` branch to `ApiDataStore::json_for` returning `search_json(parse_array(m_resources.list("custom_theme_settings")), params)` (pattern `context.cppm:1196-1197`) and replace the dead `ctx.store().json_for("custom_theme_settings", {})` (`execute.cppm:22`) with a live call. Rows are written by the PUT fix through `mutations.bulk_edit` into the same context-local namespace; no per-request DB, no SQL.
- 6 (cache): Ghost `cacheInvalidate:false` (`custom-theme-settings.js:8-10`) → `res.set_header("X-Cache-Invalidate","false")` in `browse/handler.cppm` before `send()`; no `core::cache::Cache` key needed on this read.
- 7 (auth): replace inline token check (`browse/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:19-28`, keeping `content_type_middleware`. `permissions:true`/`docName:'custom_theme_settings'` (`custom-theme-settings.js:5,11`) is controller metadata with no allowed C++ primitive — accepted residual.
- 8 (verdict): with 1–7 closed, re-rate MATCH (accepted permission residual); Summary `PARTIAL` 2→1, `MATCH` 0→1; keep evidence lines.

```cpp
// new context.cppm branch:
if (resource == "custom_theme_settings")
    return search_json(parse_array(m_resources.list("custom_theme_settings")), params);
const auto rows = ctx.store().json_for("custom_theme_settings", {}); // live, no fixture
```

### PUT `/custom_theme_settings` — MATCH

- Route identity agrees: Ghost `routes.js:576` `PUT /custom_theme_settings` with `mw.authAdminApi`; C++ `routes.cppm:19,29-35` `Router{..."/custom_theme_settings"}.put(...)`.
- Operation is the same domain intent but named differently: Ghost `edit` → `updateSettings(frame.data.custom_theme_settings)` (`custom-theme-settings.js:22-24`); C++ `.put` binds `custom_theme_settings_bulk_edit()` (`routes.cppm:34`) running workflow `custom_theme_settings_bulk_edit` (`bulk_edit/handler.cppm:23`; `bulk_edit/workflow/flow/main.cppm:9`). Both update a set of theme settings.
- Input diverges: Ghost extracts `frame.data.custom_theme_settings` from the request body (`custom-theme-settings.js:23`). C++ passes the entire raw `in.body_as_string()` with no wrapper extraction (`bulk_edit/.../execute.cppm:11,20`); this call is also unreachable.
- Output diverges: Ghost returns full updated setting objects (`custom-theme-settings.js:23`; `service.js:165-171`). C++ returns hardcoded `{"custom_theme_settings":[],"meta":{}}` (`bulk_edit/.../execute.cppm:15-18`) via early return at `:18` — no updated data, no real envelope.
- DB call diverges: Ghost validates keys/values then `_repository.read` + `settingRecord.save` per setting (`service.js:100-159`). C++ `mutations.bulk_edit("custom_theme_settings", ...)` (`execute.cppm:20`) is never executed (early return `:18`), so no write occurs; the wrapped result would be `{"meta":{"updated":N}}` (`context.cppm:1362-1366`), which does not match Ghost's returned settings objects.
- Cache diverges: Ghost `headers.cacheInvalidate:true` (`custom-theme-settings.js:18-20`) emits cache invalidation; service additionally mutates internal cache (`service.js:162`) and repopulates the public `_valueCache` (`:168`). C++ has no cache behavior (`all.cppm:820`, no `core::cache::Cache` wired).
- Auth diverges: same as GET — Ghost `mw.authAdminApi` (`routes.js:576`) + `permissions:true` (`custom-theme-settings.js:21`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check → 401 (`bulk_edit/handler.cppm:18-22`); no permissions/docName.

#### Fix

Target: `include/blog/handlers/system/custom_theme_settings/bulk_edit/workflow/step/execute.cppm` — `blog::handlers::custom_theme_settings::bulk_edit::workflow::ExecuteStep::invoke` (`:10-20`); header in `bulk_edit/handler.cppm:10-27`; auth at `routes.cppm:29-36`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `PUT /custom_theme_settings` (`routes.js:576`) ↔ `Router{...}.put(...)` (`routes.cppm:19,29-35`).
- 2 (operation): no change — `edit` → `updateSettings(frame.data.custom_theme_settings)` (`custom-theme-settings.js:22-24`) ↔ `handler.custom_theme_settings_bulk_edit()` (`routes.cppm:34`, `all.cppm:571`).
- 3 (input): replace raw `in.body_as_string()` (`execute.cppm:11,20`) with the unwrapped `data.custom_theme_settings` array decoded via `core_serde::JsonCodec{}.decode<serde::Value>(...)` (pattern `dto/response.cppm:266`); validate setting keys/values as Ghost does (`service.js:100-159`) and reject invalid input with `ApiError{}.validation(...)`.
- 5 (DB call): remove the early return at `execute.cppm:18` so the write executes; call `mutations.bulk_edit("custom_theme_settings", innerJson)` (`context.cppm:1378`) — `ResourceStore::bulk_edit` (`context.cppm:702-710`) stores records in the context-local `"custom_theme_settings"` namespace, which the GET fix reads. No SQL, no new resource.
- 4 (output): after the mutation, replace the hardcoded `{"custom_theme_settings":[],"meta":{}}` (`:15-18`) with `ctx.store().json_for("custom_theme_settings", {})` wrapped as `{"custom_theme_settings":[...]}`, returning the full updated setting objects as Ghost does (`custom-theme-settings.js:23`; `service.js:165-171`); keep status 200.
- 6 (cache): Ghost `cacheInvalidate:true` (`custom-theme-settings.js:18-20`) plus internal/public cache refresh (`service.js:162,168`) → `res.set_header("X-Cache-Invalidate","true")` in `bulk_edit/handler.cppm` before `send()`. The store update is already context-local; if a cache eviction is desired use the existing `core::cache::Cache` accessor (`all.cppm:836`) under the `custom_theme_settings` resource tag — no new global state.
- 7 (auth): replace inline token check (`bulk_edit/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:29-36`, keeping `content_type_middleware`.
- 8 (verdict): with 1–7 closed, re-rate MATCH (accepted permission residual); Summary `PARTIAL` 1→0, `MATCH` 1→2; keep evidence lines.

```cpp
const auto body = in.body_as_string().empty() ? "{}" : in.body_as_string();
const auto inner = unwrap_data_field(body, "custom_theme_settings"); // core_serde decode
const auto result = mutations.bulk_edit("custom_theme_settings", inner);
const auto rows = ctx.store().json_for("custom_theme_settings", {}); // full updated objects
```

## Missing in C++

None. Both Ghost admin routes have C++ route/handler counterparts:

- `GET /custom_theme_settings` → `include/blog/handlers/system/custom_theme_settings/routes.cppm:21-27`, `browse/handler.cppm:10`.
- `PUT /custom_theme_settings` → `include/blog/handlers/system/custom_theme_settings/routes.cppm:29-35`, `bulk_edit/handler.cppm:10`.

Confirmed by searching all `routes.cppm` files for `custom_theme_settings`/`customThemeSettings`: only `include/blog/handlers/system/custom_theme_settings/routes.cppm` (plus central install references in `include/blog/handlers/routes.cppm:11,84`).

#### Fix

No change required. Check 8 (MISSING) is closed: both Ghost admin routes (`routes.js:575,576`) have C++ counterparts (`routes.cppm:21-27,29-35`; `browse/handler.cppm:10`, `bulk_edit/handler.cppm:10`), confirmed by the `custom_theme_settings`/`customThemeSettings` grep over all `routes.cppm` files (only this folder plus central install at `include/blog/handlers/routes.cppm:11,84`). Do not add a route; the two route-detail Fix blocks close checks 1–8. No `MISSING` entry exists here.

## Extra in C++

None. The matched folder `include/blog/handlers/system/custom_theme_settings/routes.cppm` installs exactly two routes (GET and PUT at `/custom_theme_settings`, lines 21-35), both with Ghost counterparts. No third C++ route exists in that folder.

#### Fix

No change required. Check 8 (EXTRA) is closed: `CustomThemeSettingsRoutes::install` (`routes.cppm:19-36`) registers only GET and PUT `/custom_theme_settings`, both mapped to Ghost `routes.js:575,576` with exact file+symbol evidence above; no C++ route in this folder lacks a Ghost counterpart. Do not delete or gate either route; the route-detail Fix blocks close checks 1–8 for both.

## Implementation notes (group 41)

Applied fixes, all inside `include/blog/handlers/system/custom_theme_settings/` (no shared file touched):

- GET execute (`browse/workflow/step/execute.cppm`): dropped fixture `encode_custom_theme_setting_wire()` and the unreachable `meta.pagination` branch; live `ctx.store().json_for("custom_theme_settings", {})` drives `{"custom_theme_settings":[...]}` (status 200, no `meta`). Output `std::string`. The `ApiDataStore::json_for` branch proposed in the Fix block was not added: the generic fallback (`context.cppm:1196-1197`) already returns `search_json(parse_array(m_resources.list(resource)), params)`, so the context-local namespace is read without editing that shared file.
- GET handler (`browse/handler.cppm`): removed inline token check; added `res.set_header("X-Cache-Invalidate","false")`.
- PUT execute (`bulk_edit/workflow/step/execute.cppm`): unwraps `data.custom_theme_settings` with `core_serde::JsonCodec{}.decode<serde::Value>` (local helper, no new module), validates `key`/`value` with `ApiError{}.validation(...)`, runs `mutations.bulk_edit("custom_theme_settings", {"ids":...})` against the context-local store, materialises incoming setting records so the read observes them, then returns the full objects from `json_for` as `{"custom_theme_settings":[...]}`. Output `std::map<std::string,std::string>` (carries 422 status on validation failure).
- PUT handler (`bulk_edit/handler.cppm`): removed inline token check; added `res.set_header("X-Cache-Invalidate","true")` on 2xx (else `"false"`).
- Routes (`routes.cppm`): added `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware` for both verbs.

Accepted residual: Ghost `permissions:true` / `docName:'custom_theme_settings'` controller metadata has no C++ primitive.

Central registrations needed: **none**. Already present and unchanged — `CustomThemeSettingsRoutes` export at `include/blog/handlers/routes.cppm:11` and install at `:83`; handlers declared at `include/blog/handlers/all.cppm:616-617,888-889` and exported at `base.cppm:258-259`; sources listed at `include/blog/handlers/BUILD.bazel:805-810,942`. No edit to `all.cppm`, `base.cppm`, `routes.cppm`, `domain_all.cppm`, `BUILD.bazel`, `context/`, `middleware/`, or `dto/` was made or is required.

## Strict-parity pass (group 47, 2026-11-20)

The Fix blocks above (group 41) closed the coarse divergences but left the behavior
report at PARTIAL 2. Group 47 reworked the same folder to strict Ghost behavior with
the CONTRACTS.md shared abstractions; the behavior report
(`docs/parity/behavior/custom-theme-settings.md`) now records MATCH 2 with the flip
rationale and residuals. Differences from the group-41 notes:

- GET no longer emits a literal `X-Cache-Invalidate:false` header (Ghost emits none);
  it gates on the shared `require_ghost_permission` middleware and emits the
  full-object docName envelope with no `meta`.
- PUT uses Ghost's validation messages and type checks against the stored definition
  rows, updates existing records in place by key (no `bulk_edit`/`create` duplicates),
  accepts JSON boolean values, and emits `X-Cache-Invalidate: /*` only on 2xx.
- The `ApiDataStore::json_for` branch proposed in the GET Fix was still not added:
  the generic fallback already reads the context-local namespace, and shared files
  stay untouched.
- No new partition or central registration was needed. All helpers remain in the
  two existing registered `.../step/execute.cppm` partitions; `blog_handlers:mapping`
  is not registered in BUILD, so the output envelope helper is local to the folder.
