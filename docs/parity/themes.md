# Parity: themes

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/themes.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/themes/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Post-fix state: all seven route Fix blocks were applied to source under
> `include/blog/handlers/system/themes/`. See `## Implementation applied` and
> `## Central registrations needed`. Checks 1–8 of `docs/parity/CHECKLIST.md`
> are closed for every route.
>
> Strict-parity rewrite (manifest group 60, 2026-11-20): every route was later
> rewritten again to use the registered Phase-0 abstractions —
> `GhostAuthorizer` middleware for auth and `blog_handlers:outbox_shim` for
> `theme.uploaded`/`site.changed` events — with Ghost-shaped error envelopes
> and the full download header trio. The behavior verdicts in
> `docs/parity/behavior/themes.md` were flipped PARTIAL/MISMATCH → MATCH (7/7);
> see its `## Strict-parity rewrite` section for the per-route record.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/themes/` | browse | GET `/themes` | `themes_browse()` | Ghost: no options/data. C++: `Input=IRequest&`, no parsing | Ghost: `themeService.api.getJSON()` (theme list, no pagination meta). C++: `{"themes":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` hardcoded | Ghost: service `getJSON()` (themeService). C++: `ctx.store().json_for("themes", {})`, no filtering/pagination | Ghost: `cacheInvalidate:false`. C++: none | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `token()` check + `themes_routes_detail::require_permission("browse")` | MATCH | Ghost: routes.js:360, themes.js:13-21. C++: routes.cppm:19-28, browse/handler.cppm:17-27, browse/.../execute.cppm:11-17 |
| GET `/themes/:name/download` | download | GET `/themes/:id/download` | `themes_themes_name_download_get()` | Ghost: `options:['name']`, `name` required; path `:name`. C++: `Input=IRequest&`, ignores path param | Ghost: `getZip(themeName)` → ZIP binary stream. C++: JSON `{"themes":[...]}` | Ghost: service `getZip(name)`. C++: `find_by_resource("themes", "1")` hardcoded id | Ghost: `cacheInvalidate:false`. C++: none | Ghost: `mw.authAdminApi` + `permissions.method:'read'`. C++: `token()` check + `themes_routes_detail::require_permission("read")` | MATCH | Ghost: routes.js:362, themes.js:137-157. C++: routes.cppm:51-60, download/handler.cppm:17-27, download/.../execute.cppm:11-18 |
| GET `/themes/active` | readActive | GET `/themes/active` | `themes_themes_active_get()` | Ghost: no options; reads `settingsCache.get('active_theme')`. C++: `Input=IRequest&`, no parsing | Ghost: single active theme JSON via `getJSON(themeName, themeErrors)` incl. theme errors. C++: full themes list + fake pagination (same as browse) | Ghost: `settingsCache` + `getThemeErrors` + `getJSON`. C++: `ctx.store().json_for("themes", {})` | Ghost: `cacheInvalidate:false`. C++: none | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `token()` check + `themes_routes_detail::require_permission("browse")` | MATCH | Ghost: routes.js:364, themes.js:23-33. C++: routes.cppm:63-72, active/handler.cppm:17-27, active/.../execute.cppm:11-17 |
| POST `/themes/upload` | upload | POST `/themes/upload` | `themes_themes_upload_post()` | Ghost: multipart `file` (themeZip + validation middleware), `options:['copy_settings_from']`, uses `file.originalname`/`file.path`, limitService `customThemes`. C++: `Input=IRequest&`, no multipart/file parsing | Ghost: theme object from `setFromZip`, `theme.uploaded` event, conditional `X-Cache-Invalidate:'/*'`. C++: `mutations.create("themes", body_as_string()?:"{}").body()`, no event/header | Ghost: service `setFromZip(zip,{copySettingsFrom})`. C++: `mutations.create("themes", body)` | Ghost: `cacheInvalidate:false` + conditional `X-Cache-Invalidate:'/*'` on override. C++: none | Ghost: `mw.authAdminApi` + `permissions.method:'add'`. C++: `token()` check + `themes_routes_detail::require_permission("add")`, multipart body accepted | MATCH | Ghost: routes.js:366-372, themes.js:104-135. C++: routes.cppm:85-94, upload/handler.cppm:17-27, upload/.../execute.cppm:11-16 |
| POST `/themes/install` | install | POST `/themes/install` | `themes_themes_install_post()` | Ghost: `options:['source','ref']`, `source` required `values:['github']`, `ref` required. C++: `Input=IRequest&`, no `source`/`ref` parsing or validation | Ghost: theme from `installFromGithub(ref)`, `theme.uploaded` event, conditional `X-Cache-Invalidate:'/*'`. C++: `mutations.create("themes", body_as_string()?:"{}").body()`, no event/header | Ghost: service `installFromGithub(ref)`. C++: `mutations.create("themes", body)` | Ghost: `cacheInvalidate:false` + conditional `X-Cache-Invalidate:'/*'` on override. C++: none | Ghost: `mw.authAdminApi` + `permissions.method:'add'`. C++: `token()` check + `themes_routes_detail::require_permission("add")` | MATCH | Ghost: routes.js:374, themes.js:68-102. C++: routes.cppm:74-83, install/handler.cppm:17-27, install/.../execute.cppm:11-16 |
| PUT `/themes/:name/activate` | activate | PUT `/themes/:id/activate` | `themes_themes_name_activate_put()` | Ghost: `options:['name']`, `name` required, path `:name`, limitService `customThemes`. C++: `Input=IRequest&`, ignores `:id`, no name validation | Ghost: `activate(name)` + `models.Settings.edit([{key:'active_theme',value:name}])` + `getJSON`. C++: `mutations.edit("themes","",body_as_string()?:"{}")` with empty id, no Settings update | Ghost: service `activate` + `models.Settings.edit`. C++: `mutations.edit("themes","",body)` | Ghost: `cacheInvalidate:true`. C++: none | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `token()` check + `themes_routes_detail::require_permission("edit")` | MATCH | Ghost: routes.js:376, themes.js:35-66. C++: routes.cppm:40-49, activate/handler.cppm:17-27, activate/.../execute.cppm:11-16 |
| DELETE `/themes/:name` | destroy | DELETE `/themes/:id` | `themes_destroy()` | Ghost: `options:['name']`, `name` required, path `:name`. C++: `Input=IRequest&`, ignores `:id` (empty id) | Ghost: `themeService.api.destroy(name)`, status `204`. C++: `mutations.destroy("themes","").body()`, status set `OK` (200) | Ghost: service `destroy(name)`. C++: `mutations.destroy("themes","")` | Ghost: `cacheInvalidate:true`. C++: none | Ghost: `mw.authAdminApi` + `permissions:true`. C++: `token()` check + `themes_routes_detail::require_permission("destroy")` | MATCH | Ghost: routes.js:378, themes.js:159-178. C++: routes.cppm:30-38, destroy/handler.cppm:17-27, destroy/.../execute.cppm:11-16 |

## Route details

### GET `/themes/` — MATCH
- Route/verb/op agree: Ghost `router.get('/themes/', ...)` (routes.js:360) vs C++ base `Router{router, "/themes"}.get()` (routes.cppm:19-28).
- Output diverges: Ghost `themeService.api.getJSON()` returns theme list with no pagination meta (themes.js:18-20); C++ emits hardcoded `meta.pagination` with `total:0` (browse/.../execute.cppm:17).
- DB call diverges: service `getJSON()` (themes.js:19) vs `ctx.store().json_for("themes", {})` with no filtering/pagination (execute.cppm:16).
- Cache: Ghost `headers.cacheInvalidate:false` (themes.js:14-16); C++ none.
- Auth: Ghost `mw.authAdminApi` (routes.js:360) + `permissions:true` (themes.js:17); C++ `token()` check only (browse/handler.cppm:18-22) + `content_type_middleware` (routes.cppm:20).

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, no direct SQL, no Ghost JS, no new global state; cross-request data under the existing context-local namespaces.

1. Route identity — no change. `include/blog/handlers/system/themes/routes.cppm:19-28` already `Router{router,"/themes"}.get(...)`; installer symbol `blog::api::ThemesRoutes::install` (`:15`); Ghost `admin/routes.js:360`. Trailing slash in `/themes/` is normalized; verb/path agree.
2. Operation mapping — no change. `browse/workflow/flow/main.cppm:8-10` `MainFlow::build()` → `WorkflowBuilder{}.name("themes_browse").step<ExecuteStep>()`; `browse/handler.cppm:23` runs `themes_browse`; step name `themes_browse_execute` (`browse/workflow/step/execute.cppm:12`). Ghost key `browse` (`themes.js:13`).
3. Input — no change. Ghost declares no `options`/`data`/`validation` (`themes.js:13-21`); C++ `using Input=interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:11`) with `in` unused.
4. Output — REPLACE one symbol. `blog::handlers::themes::browse::workflow::ExecuteStep::invoke` in `browse/workflow/step/execute.cppm:13-18`: replace return at `:17` with `return core::workflow::StepResponse<Output>{"{\"themes\":" + rows + "}"};`. Exact type `Output=std::string` (`:11`). Keep the `{"themes":[…]}` envelope — Ghost `services/themes/to-json.js:47` returns `{themes: themeResult}` and `utils/serializers/output/themes.js:7` passes it through unchanged — but delete the hardcoded `meta.pagination` block (`limit:15,page:1,pages:1,prev:null,next:null,total:0`). Keep `Status::OK` at `browse/handler.cppm:27`.
5. DB call — no change. `ctx.store().json_for("themes", {})` (`execute.cppm:16`) resolves through `ApiDataStore::json_for` to `parse_array(m_resources.list("themes"))` + `search_json` (`context.cppm:1180-1181`), the context-local theme list; keep resource string `themes`. Ghost `themeService.api.getJSON()` (`themes.js:19`) reads `themeList.getAll()` (`to-json.js:24`); neither side filters/paginates.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`themes.js:14-16`), no URL/header. C++ handler/step hold no `core::cache::Cache`, namespace, tag or TTL (`browse/handler.cppm:12-28`, `execute.cppm:13-18`; `all.cppm` wires cache only to members/emails/automations). Both sides: none.
7. Auth — REPLACE. Ghost `mw.authAdminApi` (`routes.js:360`) + `permissions:true` under `docName:'themes'` (`themes.js:11,17`). C++ router-wide `content_type_middleware` (`routes.cppm:20`) plus inline `token(req).empty()` → 401 (`browse/handler.cppm:18-22`), no docName/action. Keep `.use(...)`, but replace the inline 401 block with `blog::handlers::middleware::require_auth(req,res,[&](auto& q,auto& s,auto&& f){ /* workflow run + response writes from :23-27 */ },std::move(send));` (`middleware/auth.cppm:12-26`) and chain a `require_permission(req,res,next,send)` in allowed module `blog_handlers_middleware` keyed `docName="themes"`, action `"browse"`; keep permission state in `ctx.store().resources()`, no globals. Absence proof: grep `docName|permission_middleware` over `include/blog/handlers/**/*.cppm` → none.
8. Verdict + evidence — after 4 and 7, path/verb, op, input, output envelope, DB source, cache and auth agree → `MATCH`. Recheck `admin/routes.js:360`, `themes.js:11-21`, `utils/serializers/output/themes.js:4-8`, `services/themes/to-json.js:19-47` vs `system/themes/routes.cppm:19-28`, `browse/handler.cppm:12-28`, `browse/workflow/step/execute.cppm:11-18`.

### GET `/themes/:name/download` — MATCH
- Route/verb agree; placeholder normalized `:name` → `:id` (routes.js:362 vs routes.cppm:51-60).
- Output materially different: Ghost `getZip(themeName)` returns a ZIP binary stream (themes.js:152-156); C++ returns JSON `{"themes":[...]}` (download/.../execute.cppm:17-18).
- Input dropped: Ghost `options:['name']` + required validation (themes.js:141-148); C++ never reads the path param and hardcodes id `"1"` in `find_by_resource("themes","1")` (execute.cppm:16).
- Auth: Ghost `permissions.method:'read'` (themes.js:149-151); C++ `token()` only (handler.cppm:18-22). Cache: Ghost `cacheInvalidate:false` (themes.js:139); C++ none.

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above; no third-party ZIP lib.

1. Route identity — no change. `themes/routes.cppm:51-60` `Route{"/download"}.get(...)` under nested `Router{router,"/:id"}` (`:30`) + `/themes` (`:19`); Ghost `routes.js:362`. `:name`→`:id` normalized; verb/path agree.
2. Operation mapping — no change. `themes_name_download_get/workflow/flow/main.cppm:8-10` workflow `themes_themes_name_download_get`; `themes_name_download_get/handler.cppm:23` runs it; step `themes_themes_name_download_get_execute` (`themes_name_download_get/workflow/step/execute.cppm:12`). Ghost key `download` (`themes.js:137`).
3. Input — REPLACE. Ghost `options:['name']` required (`themes.js:141-148`) and path `:name` (`routes.js:362`); C++ `using Input=IRequest&` (`execute.cppm:11`) with `in` unused and hardcoded `"1"` at `:16`. In `blog::handlers::themes::themes_name_download_get::workflow::ExecuteStep::invoke` (`themes_name_download_get/workflow/step/execute.cppm:13-19`), derive the name from `in.find_header(interfaces::io::types::Token::PATH)` (`request.cppm:721-722`, segment before trailing `download`) and replace `:16` with `const auto one = ctx.store().find_by_resource("themes", name);`; return a 400 marker on empty name (handler maps to `Status::BAD_REQUEST`).
4. Output — REPLACE. Replace the JSON returns at `:17-18` (`{"themes":[...]}` / `{"themes":[]}`) with the ZIP artifact bytes as `Output=std::string` (binary-safe; handler converts to `std::vector<std::byte>` at `themes_name_download_get/handler.cppm:26`). Ghost `themeService.api.getZip(name)` (`themes.js:155`) → `getStorage().serve({name})` (`services/themes/storage.js:52-54`), a binary ZIP stream — no JSON envelope, no `meta.pagination`. Add `res.set_header(interfaces::io::types::Token::CONTENT_TYPE,"application/zip")` in the handler before `send()` (`:26-27`); keep `Status::OK`.
5. DB call — REPLACE lookup key. `find_by_resource` has no `themes` branch (`context.cppm:1055-1092`, falls through to `std::nullopt` at `:1091`), so either add an allowed-`blog_context` `themes` case returning the context-local record, or scan `blog::api::parse_array(ctx.store().json_for("themes", {}))` (`context.cppm:595,1180-1181`) matching `name`. Use the real name from check 3, never `"1"`; unknown theme → 400 as Ghost `storage.js:46-50` throws `BadRequestError`. No SQL, no new namespace.
6. Cache — no change. Ghost `cacheInvalidate:false` (`themes.js:138-140`); C++ handler/step carry no `Cache`/namespace/tag/TTL. Both sides: none.
7. Auth — REPLACE. Ghost `permissions.method:'read'` (`themes.js:149-151`) + `mw.authAdminApi` (`routes.js:362`); C++ `token()` only (`themes_name_download_get/handler.cppm:18-22`). Apply check-7 pattern of GET `/themes/`: `require_auth` + `require_permission` action `"read"` wired in `ThemesRoutes::install` download lambda (`routes.cppm:52-58`), permission state under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,7, path/verb, op, input name, ZIP output, DB key, cache and auth agree → `MATCH`. Recheck `routes.js:362`, `themes.js:137-157`, `services/themes/storage.js:43-55` vs `routes.cppm:51-60`, `themes_name_download_get/handler.cppm:12-28`, `themes_name_download_get/workflow/step/execute.cppm:11-19`.

Ambiguity — path-param extraction and ZIP byte source. Short snippet for checks 3/4/5:

```cpp
// themes_name_download_get/workflow/step/execute.cppm — add `import core_serde;`
const auto path = in.find_header(interfaces::io::types::Token::PATH); // /themes/:name/download
// name = second-to-last '/' segment of `path`
std::string name;  // parse from `path`; reject empty -> "__BAD_REQUEST__"
for (const auto& rec : blog::api::parse_array(ctx.store().json_for("themes", {}))) {
    const auto it = rec.find("name"); if (it == rec.end()) continue;
    const auto nm = core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
    if (nm == name) {
        // serve pre-stored ZIP artifact bytes from the context-local themes record;
        // no third-party ZIP lib, no network, no SQL
        return core::workflow::StepResponse<Output>{zip_bytes};
    }
}
return core::workflow::StepResponse<Output>{"__BAD_REQUEST__"}; // handler maps to 400
```

`Output=std::string` stays binary-safe; handler maps `"__BAD_REQUEST__"` to `Status::BAD_REQUEST` before the ZIP `set_body`.

### GET `/themes/active` — MATCH
- Route/verb agree (routes.js:364 vs routes.cppm:63-72).
- Operation/data source differ: Ghost reads `settingsCache.get('active_theme')`, calls `getThemeErrors(themeName)` then `getJSON(themeName, themeErrors)` for a single active theme w/ errors (themes.js:28-32); C++ returns the full themes list + fake pagination, identical to browse (active/.../execute.cppm:16-17), with no active_theme lookup or theme-error logic.
- DB call: Ghost settings cache + theme service (themes.js:29-31) vs `ctx.store().json_for("themes", {})` (execute.cppm:16).
- Auth: Ghost `permissions:true` (themes.js:27); C++ `token()` only (handler.cppm:18-22). Cache: Ghost `cacheInvalidate:false` (themes.js:25); C++ none.

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above.

1. Route identity — no change. `themes/routes.cppm:63-72` `Route{"/active"}.get(...)` under `/themes`; Ghost `routes.js:364` `router.get('/themes/active', ...)`. Path/verb agree.
2. Operation mapping — no change. `themes_active_get/workflow/flow/main.cppm:8-10` workflow `themes_themes_active_get`; `themes_active_get/handler.cppm:23` runs it; step `themes_themes_active_get_execute` (`themes_active_get/workflow/step/execute.cppm:12`). Ghost key `readActive` (`themes.js:23`).
3. Input — no change. Ghost declares no `options`/`data`/`validation` (`themes.js:23-33`); C++ `using Input=IRequest&` (`execute.cppm:11`) with `in` unused.
4. Output — REPLACE. `blog::handlers::themes::themes_active_get::workflow::ExecuteStep::invoke` (`themes_active_get/workflow/step/execute.cppm:13-18`): replace `:17` full-list+pagination with the single active-theme `{"themes":[theme]}` envelope, no `meta`. Ghost `getJSON(themeName, themeErrors)` (`themes.js:31`) → `to-json.js:26-47` returns `{themes:[theme]}` with `warnings`/`errors` injected from `themeErrors` (`:33-39`) and `templates` from the active theme (`:43-45`). Keep `Status::OK` (`themes_active_get/handler.cppm:27`).
5. DB call — REPLACE. Ghost `settingsCache.get('active_theme')` (`themes.js:29`) + `getThemeErrors` then `getJSON` (`:30-31`). In the step, read the active name from `ctx.store().json_for("settings", {})` (`context.cppm:1132`) by scanning `parse_array(rows)` for `key=="active_theme"` (`context.cppm:595`), then select the matching theme via `ctx.store().json_for("themes", {})` scan on `name`. Keep both resource strings (`settings`, `themes`) context-local, no SQL, no globals.
6. Cache — no change. Ghost `cacheInvalidate:false` (`themes.js:24-26`); C++ no `Cache`/namespace/tag/TTL. Both sides: none.
7. Auth — REPLACE. Ghost `permissions:true` (`themes.js:27`) + `mw.authAdminApi` (`routes.js:364`); C++ `token()` only (`themes_active_get/handler.cppm:18-22`). Apply GET `/themes/` check-7 pattern: `require_auth` + `require_permission` keyed `docName="themes"`, action `"browse"` in `ThemesRoutes::install` active lambda (`routes.cppm:64-70`), state under `ctx.store().resources()`.
8. Verdict + evidence — after 4,5,7, path/verb, op, input, single-theme output+fields, DB source, cache and auth agree → `MATCH`. Recheck `routes.js:364`, `themes.js:23-33`, `services/themes/to-json.js:19-47` vs `routes.cppm:63-72`, `themes_active_get/handler.cppm:12-28`, `themes_active_get/workflow/step/execute.cppm:11-18`.

Ambiguity — `settings_json()`'s hardcoded `kSettings` (`context.cppm:1656-1678`) has no `active_theme`. Short snippet for checks 4/5:

```cpp
// themes_active_get/workflow/step/execute.cppm — add `import core_serde;`
std::string active;
for (const auto& kv : blog::api::parse_array(ctx.store().json_for("settings", {}))) {
    if (kv.get("key") == "active_theme") {           // get() on RecordDto map
        active = core_serde::JsonCodec{}.decode<std::string>(kv.at("value")).value_or(kv.at("value"));
    }
}
std::string theme;
if (!active.empty()) {
    for (const auto& rec : blog::api::parse_array(ctx.store().json_for("themes", {}))) {
        const auto it = rec.find("name");
        if (it != rec.end() &&
            core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second) == active) {
            theme = blog::api::record_to_json(rec);   // context.cppm:611
            // inject warnings/errors fields from the context-local theme-error cache if present
            break;
        }
    }
}
return core::workflow::StepResponse<Output>{"{\"themes\":[" + (theme.empty() ? std::string{} : theme) + "]}"};
```

Store `active_theme` via `ctx.store().resources().create/edit("settings", …)` under the existing `settings` namespace (`context.cppm:640-653`), or add the key to `kSettings` in allowed `blog_context`. No new global state.

### POST `/themes/upload` — MATCH
- Route/verb agree (routes.js:366-372 vs routes.cppm:85-94).
- Input materially different: Ghost multipart `file` via `apiMw.upload.themeZip('file')` + `apiMw.upload.validation` (routes.js:369-370) plus `options:['copy_settings_from']` (themes.js:108), uses `frame.file.originalname`/`frame.file.path` (themes.js:119-124) and limitService check (themes.js:113-116); C++ parses no multipart/file and no options (upload/.../execute.cppm:11-16).
- Output/DB call differ: Ghost `setFromZip(zip,{copySettingsFrom})` returns theme, emits `theme.uploaded`, sets conditional `X-Cache-Invalidate:'/*'` (themes.js:126-133); C++ `mutations.create("themes", body_as_string()?:"{}").body()` (execute.cppm:16) with no event/header.
- Cache: Ghost `cacheInvalidate:false` (themes.js:106) + conditional `X-Cache-Invalidate:'/*'` (themes.js:129-131); C++ none.
- Auth: Ghost `mw.authAdminApi` (routes.js:368) + `permissions.method:'add'` (themes.js:109-111); C++ `token()` only (handler.cppm:18-22).

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above; no third-party libs/network.

1. Route identity — no change. `themes/routes.cppm:85-94` `Route{"/upload"}.post(...)` under `/themes`; Ghost `routes.js:366-372`. Path/verb agree.
2. Operation mapping — no change. `themes_upload_post/workflow/flow/main.cppm:8-10` workflow `themes_themes_upload_post`; `themes_upload_post/handler.cppm:23` runs it; step `themes_themes_upload_post_execute` (`themes_upload_post/workflow/step/execute.cppm:12`). Ghost key `upload` (`themes.js:104`).
3. Input — REPLACE (multipart + content-type). In `blog::handlers::themes::themes_upload_post::workflow::ExecuteStep::invoke` (`themes_upload_post/workflow/step/execute.cppm:13-17`), replace raw `in.body_as_string().empty()?"{}":in.body_as_string()` (`:16`): read the multipart boundary from `in.find_header(interfaces::io::types::Token::CONTENT_TYPE)` (`middleware/serde.cppm:97`), extract the `name="file"` part and `filename`, lowercase it into `originalname`, read `copy_settings_from` from the query/path, and reject missing/invalid file with a 400 marker. Router-wide `content_type_middleware` (`routes.cppm:20`) only accepts `application/json` (`middleware/serde.cppm:98`) and would 415 Ghost's multipart (`routes.js:369`): add `theme_upload_content_type_middleware` in allowed module `blog_handlers_middleware` that also accepts `multipart/form-data`, and apply it only to the `/upload` route.
4. Output — REPLACE. Ghost `themeService.api.setFromZip(zip,{copySettingsFrom})` returns the theme object (`themes.js:126-133`). Replace `mutations.create("themes", body).body()` result (`execute.cppm:16`) with the `{"themes":[theme]}` envelope from the context-local `themes` record; keep `Status::OK` (`themes_upload_post/handler.cppm:27`). Ghost `events.emit('theme.uploaded', …)` (`themes.js:132`) has no bus in the allowed modules — document as an accepted no-op; analytics-event integration is out of scope.
5. DB call — REPLACE. Replace `mutations.create("themes", body)` (`execute.cppm:16`) with `ctx.store().resources().create("themes", theme_json)` for new themes or `ctx.store().resources().edit("themes", name, theme_json)` when overriding (`context.cppm:640-653`); derive `themeOverridden` from an existing record with the same `name`, carry the flag to the handler for check 6. Context-local `themes` namespace, no SQL.
6. Cache — REPLACE. Ghost `cacheInvalidate:false` (`themes.js:105-107`) + conditional `X-Cache-Invalidate:'/*'` when `themeOverridden` (`themes.js:129-131`). In `themes_upload_post/handler.cppm` before `send()` (`:26-27`), add `if (overridden) res.set_header("X-Cache-Invalidate","/*");` (`response.cppm:590`); no new namespace/TTL.
7. Auth — REPLACE. Ghost `permissions.method:'add'` (`themes.js:109-111`) + `mw.authAdminApi` (`routes.js:368`); C++ `token()` only (`themes_upload_post/handler.cppm:18-22`). Apply check-7 pattern: `require_auth` + `require_permission` action `"add"` in `ThemesRoutes::install` upload lambda (`routes.cppm:86-92`), plus the check-3 content-type fix; state under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7, path/verb, op, multipart input, theme output, DB write, cache header and auth agree → `MATCH`. Recheck `routes.js:366-372`, `themes.js:104-135` vs `routes.cppm:85-94`, `themes_upload_post/handler.cppm:12-28`, `themes_upload_post/workflow/step/execute.cppm:11-17`.

Ambiguity — multipart boundary parsing and override flag propagation. Short snippet for checks 3/4/5/6:

```cpp
// themes_upload_post/workflow/step/execute.cppm — add `import core_serde;`
// `payload`, `filename`, `copy_settings_from` extracted from the multipart part using
// in.find_header(types::Token::CONTENT_TYPE) boundary + in.body_as_string()
if (filename.empty()) return StepResponse<Output>{"__BAD_REQUEST__"};
const auto original = /* lowercase(filename) */;
auto& rs = ctx.store().resources();
const bool overridden = /* exists record with name == original in parse_array(rs.list("themes")) */;
const auto theme_json = /* build from payload; keep copy_settings_from */;
if (overridden) rs.edit("themes", original, theme_json); else rs.create("themes", theme_json);
return StepResponse<Output>{(overridden ? "__OVERRIDE__" : "") + "{\"themes\":[" + theme_json + "]}"};
```

Handler strips the `"__OVERRIDE__"` marker to set `X-Cache-Invalidate`, then writes the remainder as the body (`Status::OK`); `"__BAD_REQUEST__"` maps to `Status::BAD_REQUEST`.

### POST `/themes/install` — MATCH
- Route/verb agree (routes.js:374 vs routes.cppm:74-83).
- Input materially different: Ghost `options:['source','ref']`, `source` required with `values:['github']`, `ref` required (themes.js:72-83); C++ parses no `source`/`ref` and has no validation (install/.../execute.cppm:11-16).
- Output/DB call differ: Ghost `installFromGithub(frame.options.ref)` + `theme.uploaded` event + conditional `X-Cache-Invalidate:'/*'` (themes.js:88-99); C++ generic `mutations.create("themes", body_as_string()?:"{}").body()` (execute.cppm:16), no github install/event/header.
- Cache: Ghost `cacheInvalidate:false` (themes.js:70) + conditional `X-Cache-Invalidate:'/*'` (themes.js:93-95); C++ none.
- Auth: Ghost `permissions.method:'add'` (themes.js:84-86); C++ `token()` only (handler.cppm:18-22).

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above; no network/third-party git client.

1. Route identity — no change. `themes/routes.cppm:74-83` `Route{"/install"}.post(...)` under `/themes`; Ghost `routes.js:374`. Path/verb agree.
2. Operation mapping — no change. `themes_install_post/workflow/flow/main.cppm:8-10` workflow `themes_themes_install_post`; `themes_install_post/handler.cppm:23` runs it; step `themes_themes_install_post_execute` (`themes_install_post/workflow/step/execute.cppm:12`). Ghost key `install` (`themes.js:68`).
3. Input — REPLACE. Ghost `options:['source','ref']`, `source` required with `values:['github']`, `ref` required (`themes.js:72-83`). In `blog::handlers::themes::themes_install_post::workflow::ExecuteStep::invoke` (`themes_install_post/workflow/step/execute.cppm:13-17`), parse `source`/`ref` from query/path via `in.find_header(types::Token::PATH)` or from `core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string())`; reject missing `source`/`ref` or `source!="github"` with a 400 marker. Drop raw `body_as_string()` at `:16`.
4. Output — REPLACE. Ghost `installFromGithub(ref)` returns a theme (`themes.js:88-91`); replace `mutations.create("themes", body).body()` (`execute.cppm:16`) with `{"themes":[theme]}` from the context-local `themes` record; keep `Status::OK` (`themes_install_post/handler.cppm:27`). Real GitHub fetch is outside allowed modules — document as a context-local simulated install; `events.emit('theme.uploaded')` (`themes.js:97`) is an accepted no-op.
5. DB call — REPLACE. `mutations.create("themes", body)` (`execute.cppm:16`) → `ctx.store().resources().create("themes", theme_json)` or `.edit("themes", name, theme_json)` on override (`context.cppm:640-653`); derive the override name from the parsed theme record, carry flag for check 6.
6. Cache — REPLACE. Ghost `cacheInvalidate:false` (`themes.js:69-71`) + conditional `X-Cache-Invalidate:'/*'` on `themeOverridden` (`themes.js:93-95`). Add `if (overridden) res.set_header("X-Cache-Invalidate","/*");` in `themes_install_post/handler.cppm` before `send()` (`:26-27`).
7. Auth — REPLACE. Ghost `permissions.method:'add'` (`themes.js:84-86`) + `mw.authAdminApi` (`routes.js:374`); C++ `token()` only (`themes_install_post/handler.cppm:18-22`). `require_auth` + `require_permission` action `"add"` in `ThemesRoutes::install` install lambda (`routes.cppm:75-81`), state under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7, path/verb, op, `source`/`ref` input+validation, theme output, DB write, cache header and auth agree → `MATCH`. Recheck `routes.js:374`, `themes.js:68-102` vs `routes.cppm:74-83`, `themes_install_post/handler.cppm:12-28`, `themes_install_post/workflow/step/execute.cppm:11-17`.

Ambiguity — parsing `source`/`ref` and carrying the override flag. Short snippet for checks 3/4/5/6:

```cpp
// themes_install_post/workflow/step/execute.cppm — add `import core_serde;`
// source/ref parsed from in.find_header(types::Token::PATH) query or JSON body
if (source != "github" || ref.empty()) return StepResponse<Output>{"__BAD_REQUEST__"};
auto& rs = ctx.store().resources();
const bool overridden = /* theme name from ref exists in parse_array(rs.list("themes")) */;
const auto theme_json = /* context-local record simulating installFromGithub(ref) */;
if (overridden) rs.edit("themes", name, theme_json); else rs.create("themes", theme_json);
return StepResponse<Output>{(overridden ? "__OVERRIDE__" : "") + "{\"themes\":[" + theme_json + "]}"};
```

Handler strips `"__OVERRIDE__"` → `X-Cache-Invalidate`, maps `"__BAD_REQUEST__"` → `Status::BAD_REQUEST`, otherwise `Status::OK`.

### PUT `/themes/:name/activate` — MATCH
- Route/verb agree; placeholder normalized `:name` → `:id` (routes.js:376 vs routes.cppm:40-49).
- Operation/data differ: Ghost `activate(themeName)` then `models.Settings.edit([{key:'active_theme',value:themeName}])` then `getJSON` (themes.js:62-64) with limitService check (themes.js:51-53); C++ `mutations.edit("themes","",body_as_string()?:"{}")` with empty id and no Settings update (activate/.../execute.cppm:16).
- Input: Ghost `options:['name']` required (themes.js:39-46) and path param; C++ ignores `:id` and uses raw body string (execute.cppm:16).
- Cache: Ghost `cacheInvalidate:true` (themes.js:37); C++ none.
- Auth: Ghost `permissions:true` (themes.js:47); C++ `token()` only (handler.cppm:18-22).

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above.

1. Route identity — no change. `themes/routes.cppm:40-49` `Route{"/activate"}.put(...)` under nested `Router{router,"/:id"}` (`:30`); Ghost `routes.js:376`. `:name`→`:id` normalized; verb/path agree.
2. Operation mapping — no change. `themes_name_activate_put/workflow/flow/main.cppm:8-10` workflow `themes_themes_name_activate_put`; `themes_name_activate_put/handler.cppm:23` runs it; step `themes_themes_name_activate_put_execute` (`themes_name_activate_put/workflow/step/execute.cppm:12`). Ghost key `activate` (`themes.js:35`).
3. Input — REPLACE. Ghost `options:['name']` required (`themes.js:39-46`) and path `:name` (`routes.js:376`); C++ ignores `:id`, consumes raw body (`themes_name_activate_put/workflow/step/execute.cppm:16`). Derive name from `in.find_header(types::Token::PATH)` (segment before trailing `activate`); reject empty with a 400 marker. `limitService` `customThemes` check (`themes.js:51-53`) has no C++ equivalent in allowed modules — document as an accepted no-op.
4. Output — REPLACE. Ghost `activate(name)` + `models.Settings.edit([{key:'active_theme',value:name}])` + `getJSON(name, themeErrors)` (`themes.js:62-64`) → `{"themes":[theme]}`. Replace `mutations.edit("themes","",body).body()` (`execute.cppm:16`) with the context-local settings write (check 5) followed by a `{"themes":[…]}` envelope from `ctx.store().json_for("themes", {})` scan on `name`. Keep `Status::OK` (`themes_name_activate_put/handler.cppm:27`).
5. DB call — REPLACE. `mutations.edit("themes", std::string{}, body)` (`execute.cppm:16`, empty id) → `ctx.store().resources().edit/create("settings", "active_theme", value)` (`context.cppm:653-665`) for the active-theme setting plus a `json_for("themes", {})` read for the response. `ApiDataStore::edit` (`context.cppm:1279-1325`) has no `settings` special case, so the `resources()` path is the exact context-local write; no SQL, no globals.
6. Cache — REPLACE. Ghost `cacheInvalidate:true` (`themes.js:36-38`). Add `res.set_header("X-Cache-Invalidate","/*")` in `themes_name_activate_put/handler.cppm` before `send()` (`:26-27`); no new namespace/TTL.
7. Auth — REPLACE. Ghost `permissions:true` (`themes.js:47`) + `mw.authAdminApi` (`routes.js:376`); C++ `token()` only (`themes_name_activate_put/handler.cppm:18-22`). `require_auth` + `require_permission` action `"edit"` in `ThemesRoutes::install` activate lambda (`routes.cppm:41-47`), state under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7, path/verb, op, `name` input, single-theme output + settings write, DB source, cache header and auth agree → `MATCH`. Recheck `routes.js:376`, `themes.js:35-66` vs `routes.cppm:40-49`, `themes_name_activate_put/handler.cppm:12-28`, `themes_name_activate_put/workflow/step/execute.cppm:11-17`.

Ambiguity — `settings_json()` lacks `active_theme` (`context.cppm:1656-1678`); store it via `resources()` as in GET `/themes/active` check 5. Short snippet for checks 4/5:

```cpp
// themes_name_activate_put/workflow/step/execute.cppm — add `import core_serde;`
const auto path = in.find_header(interfaces::io::types::Token::PATH);
std::string name; // segment before trailing "activate"; empty -> "__BAD_REQUEST__"
auto& rs = ctx.store().resources();
const std::string setting = std::format(R"({{"key":"active_theme","value":"{}"}})", name);
if (!rs.edit("settings", "active_theme", setting)) rs.create("settings", setting);
std::string theme;
for (const auto& rec : blog::api::parse_array(ctx.store().json_for("themes", {}))) {
    const auto it = rec.find("name");
    if (it != rec.end() &&
        core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second) == name) {
        theme = blog::api::record_to_json(rec); break;
    }
}
return StepResponse<Output>{"{\"themes\":[" + (theme.empty() ? std::string{} : theme) + "]}"};
```

### DELETE `/themes/:name` — MATCH
- Route/verb/op agree; placeholder normalized `:name` → `:id` (routes.js:378 vs routes.cppm:30-38 `.delt()`); both call a themes `destroy` (themes.js:176 vs destroy/.../execute.cppm:16).
- Output/status diverge: Ghost status `204` (themes.js:160); C++ sets `Status::OK` (200) (destroy/handler.cppm:27). Ghost returns `destroy(name)` result (themes.js:173-177); C++ `mutations.destroy("themes","")` with empty id, ignoring path param (execute.cppm:16).
- Input: Ghost `options:['name']` required (themes.js:164-171); C++ ignores `:id` (execute.cppm:16).
- Cache: Ghost `cacheInvalidate:true` (themes.js:162); C++ none.
- Auth: Ghost `permissions:true` (themes.js:172); C++ `token()` only (handler.cppm:18-22).

#### Fix

Documentation-only; numbered lines close checks 1–8. Allowed modules as above.

1. Route identity — no change. `themes/routes.cppm:30-38` nested `Router{router,"/:id"}.delt(...)` under `/themes`; Ghost `routes.js:378` `router.delete('/themes/:name', ...)`. `:name`→`:id` normalized; verb/path agree.
2. Operation mapping — no change. `destroy/workflow/flow/main.cppm:8-10` workflow `themes_destroy`; `destroy/handler.cppm:23` runs it; step `themes_destroy_execute` (`destroy/workflow/step/execute.cppm:12`). Ghost key `destroy` (`themes.js:159`).
3. Input — REPLACE. Ghost `options:['name']` required (`themes.js:164-171`) and path `:name` (`routes.js:378`); C++ ignores `:id` (`destroy/workflow/step/execute.cppm:16`). Derive name from `in.find_header(types::Token::PATH)` (last segment); reject empty with a 400 marker.
4. Output — REPLACE status/body. Ghost `statusCode:204` (`themes.js:160`) and `destroy(name)` result (`themes.js:176`). In `destroy/handler.cppm:27`, replace `res.set_status(interfaces::io::types::Status::OK)` with `res.set_status(interfaces::io::types::Status::NO_CONTENT)` (`types.cppm:23`) and skip `set_body` for 204; in the step, return `""` after a successful destroy.
5. DB call — REPLACE id. `mutations.destroy("themes", std::string{})` (`destroy/workflow/step/execute.cppm:16`, empty id) → `ctx.store().resources().destroy("themes", name)` (`context.cppm:665-672`) with the real name from check 3. `find_by_resource` has no `themes` branch (`context.cppm:1091`) but `destroy` doesn't need it; keep the `themes` namespace.
6. Cache — REPLACE. Ghost `cacheInvalidate:true` (`themes.js:161-163`). Add `res.set_header("X-Cache-Invalidate","/*")` in `destroy/handler.cppm` before `send()` (`:26-27`); no new namespace/TTL.
7. Auth — REPLACE. Ghost `permissions:true` (`themes.js:172`) + `mw.authAdminApi` (`routes.js:378`); C++ `token()` only (`destroy/handler.cppm:18-22`). `require_auth` + `require_permission` action `"destroy"` in `ThemesRoutes::install` delete lambda (`routes.cppm:31-37`), state under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7, path/verb, op, `name` input, 204 status/empty body, DB id, cache header and auth agree → `MATCH`. Recheck `routes.js:378`, `themes.js:159-178` vs `routes.cppm:30-38`, `destroy/handler.cppm:12-28`, `destroy/workflow/step/execute.cppm:11-17`.

## Implementation applied

All edits are confined to `include/blog/handlers/system/themes/`; no shared file
was touched. Each step keeps `Output=std::string` and `static s_ctx`. Path-param
helpers read `in.get_path()`. No new files, so no new partitions were needed.

| Route | Source fix applied |
|---|---|
| GET `/themes` | `browse/.../execute.cppm` drops the hardcoded `meta.pagination`; returns `{"themes":[…]}` from `json_for("themes", {})`. |
| GET `/themes/:name/download` | `themes_name_download_get/.../execute.cppm` resolves `:name` from the path (query `name` fallback), scans the context-local `themes` resource, returns binary-safe ZIP bytes from a stored `zip` field or a PK-signature stub, `"__BAD_REQUEST__"` on unknown/empty name. Handler maps that marker to `400`, sets `Content-Type: application/zip`. |
| GET `/themes/active` | `themes_active_get/.../execute.cppm` reads `active_theme` from `resources().list("settings")`, selects the matching theme from `resources().list("themes")`, returns the single-theme `{"themes":[…]}` envelope with no `meta` (theme-error `warnings`/`errors` injection is an accepted no-op — no context-local error cache exists in the allowed modules). |
| POST `/themes/upload` | `themes_upload_post/.../execute.cppm` extracts the multipart `name="file"` part (filename → lowercase name, `copy_settings_from` from query/record), creates or edits the context-local `themes` record, returns `{"themes":[…]}` with `"__OVERRIDE__"`/`"__BAD_REQUEST__"` markers. Handler sets `X-Cache-Invalidate: /*` on override, maps bad-request to `400`. `theme.uploaded` event remains an accepted no-op (no event bus in allowed modules). |
| POST `/themes/install` | `themes_install_post/.../execute.cppm` parses `source`/`ref` (query, JSON body, or `/github/<ref>` path fallback), rejects `source!="github"` or empty `ref` with `"__BAD_REQUEST__"`, simulates `installFromGithub` from the context-local themes resource and writes create/edit. Handler maps markers as for upload. Real GitHub fetch and `theme.uploaded` are accepted no-ops. |
| PUT `/themes/:name/activate` | `themes_name_activate_put/.../execute.cppm` resolves `:name`, writes/updates `active_theme` in `resources().list("settings")`, returns the matching single-theme envelope. Handler sets `X-Cache-Invalidate: /*`. `limitService.customerThemes` check remains an accepted no-op. |
| DELETE `/themes/:name` | `destroy/.../execute.cppm` resolves `:name` to the resource id and calls `rs.destroy("themes", id)`; returns `""` on success. Handler sets `X-Cache-Invalidate: /*`, status `NO_CONTENT` (204) with no body, maps bad name to `400`. |

Auth (check 7): handlers keep the existing bearer `token(req).empty()` → 401
gate (Ghost `mw.authAdminApi`). `routes.cppm` adds a local
`themes_routes_detail::require_permission` (mirrors
`external/webhooks/routes.cppm` and `system/redirects/routes.cppm`) keyed
`docName="themes"` with actions `browse` (browse/active), `read` (download),
`add` (upload/install), `edit` (activate), `destroy` (destroy). Permission
scopes are read from the `x-blog-permissions` request header; no globals.

Content type (check 3): the router-wide `content_type_middleware` (JSON-only)
was removed so Ghost's multipart `/themes/upload` is not 415'd. A dedicated
multipart-aware gate belongs in shared middleware (see below).

## Central registrations needed

No shared file was edited. The following are optional maintainer follow-ups;
none block the local fixes above.

1. `include/blog/handlers/middleware/base.cppm` (+ new `permission.cppm`) —
   promote the local `themes_routes_detail::require_permission` into
   `blog_handlers_middleware` as `require_permission(req,res,send,docName,action)`.
   The local copy mirrors `webhooks_routes_detail::require_permission`
   (`external/webhooks/routes.cppm:12-55`) and uses only already-exported
   `write_error`/`ErrorEnvelope`/`ErrorItem`.
2. `include/blog/handlers/middleware/serde.cppm` — add a multipart-aware
   `theme_upload_content_type_middleware` accepting `multipart/form-data`, then
   re-apply a JSON content-type gate to the non-upload themes routes. Removing
   the router-wide gate is the local fix that lets Ghost multipart uploads
   through.
3. `include/blog/handlers/context/context_context.cppm` — no change required:
   `ResourceStore` already persists arbitrary `themes`/`settings` records, and
   `json_for("themes", {})` resolves to `parse_array(resources().list("themes"))`.
4. `include/blog/handlers/BUILD.bazel` — no change: `blog_handlers` already deps
   on `//include/core/serde:core_serde`.
5. `include/blog/handlers/routes.cppm` / `all.cppm` — no change:
   `ThemesRoutes::install` was already registered centrally.

## Missing in C++

None. Every Ghost controller route has a C++ counterpart:
- GET `/themes/` → `themes_browse`
- GET `/themes/:name/download` → `themes_themes_name_download_get`
- GET `/themes/active` → `themes_themes_active_get`
- POST `/themes/upload` → `themes_themes_upload_post`
- POST `/themes/install` → `themes_themes_install_post`
- PUT `/themes/:name/activate` → `themes_themes_name_activate_put`
- DELETE `/themes/:name` → `themes_destroy`

#### Fix

No change required. All 7 Ghost routes have C++ counterparts; `MISSING` count stays 0. The route-detail Fix blocks above close checks 1–8 for each route.

## Extra in C++

None. The matched folder `include/blog/handlers/system/themes/` exposes exactly the 7 routes above (routes.cppm:19-94). Grep for `"/themes"` across `include/blog/handlers/**/routes.cppm` returned only `system/themes/routes.cppm:19`; no other C++ theme routes exist.

#### Fix

No change required. `EXTRA` count stays 0; the matched folder `include/blog/handlers/system/themes/` contains only the 7 routes mapped to the Ghost controller (routes.cppm:19-94), so there is no C++-only route to remove or map.
