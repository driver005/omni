# Behavior report: themes

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/themes.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:359-378`
- Output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/themes.js`
- Ghost service root: `docs/Ghost/ghost/core/server/services/themes/` (`index.js:29-38` exports the `api.*` methods bound below)
- C++ folders: `include/blog/handlers/system/themes/` (`routes.cppm` + 7 handler/flow/step triplets)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Seven Ghost routes, seven C++ routes (no missing/extra). The strict-parity rewrite recorded at the end of this document closed every wire-observable divergence: the inline token gate and spoofable `x-blog-permissions` helper were replaced by the registered `GhostAuthorizer` middleware under `docName='themes'`, and `theme.uploaded` / `site.changed` are now emitted through the registered outbox. The pre-rewrite route details and Diff bullets below are the original analysis and are superseded by the `Strict-parity rewrite` section.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/themes/` | browse | no options/data | `mw.authAdminApi`; `permissions:true` | 200; `cacheInvalidate:false` → no X-Cache-Invalidate | `api.getJSON()` → `toJSON()` → `themeList.getAll()` + `packageJSON.filter` | read, no invalidation | `{themes:[…]}` via serializer (`frame.response=data`) | `system/themes/browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | token check; `store().json_for("themes",{})` → `{"themes":rows}`; 200 | MATCH | Ghost `themes.js:13-21`, `routes.js:360`; C++ `browse/handler.cppm:17-28`, `browse/.../execute.cppm:20-21` |
| GET `/themes/active` | readActive | no path params used; reads `active_theme` from settings cache | `mw.authAdminApi`; `permissions:true` | 200; `cacheInvalidate:false` | `settingsCache.get('active_theme')` → `getThemeErrors(name)` → `getJSON(name, errors)` | read, no invalidation | `{themes:[single theme + warnings/errors]}` | `system/themes/themes_active_get/...` | token check; reads `active_theme` from `settings` resource; finds matching theme record; `{themes:[theme]}`; 200 | MATCH | Ghost `themes.js:23-33`, `validate.js:104-126`; C++ `themes_active_get/.../execute.cppm:32-56` |
| PUT `/themes/:name/activate` | activate | options `name` (path `:name`); validation `name.required` | `mw.authAdminApi`; `permissions:true` | 200; `cacheInvalidate:true` → X-Cache-Invalidate `/*` | limit check → `activate(name)` (list.get → checkSafe → bridge.activateFromAPI) → `models.Settings.edit` → `getJSON(name, errors)` | mutation, invalidates `/*`; finish also emits `site.changed` | `{themes:[single theme]}` | `system/themes/themes_name_activate_put/...` | token check; parses `:name`; edits/creates `active_theme` in `settings` resource; returns `{themes:[theme]}`; sets X-Cache-Invalidate `/*`; 200 | MATCH | Ghost `themes.js:35-66`, `activate.js:53-69`, `activation-bridge.js:16-21`, `emit-events.js:12-15`; C++ `activate/handler.cppm:23-43`, `activate/.../execute.cppm:57-85` |
| POST `/themes/install` | install | options `source` (required, values `['github']`), `ref` (required) | `mw.authAdminApi`; `permissions:{method:'add'}` | 200; `cacheInvalidate:false`; X-Cache-Invalidate `/*` only when theme overridden | limit check → `installFromGithub(ref)` → GitHub zipball HTTP GET → temp zip → `setFromZip` → `events.emit('theme.uploaded')` | override → X-Cache-Invalidate `/*` + `site.changed` | returned `theme` object (serializer `frame.response=data`) | `system/themes/themes_install_post/...` | token check; parses source/ref from query/body/path; **simulated** context-local theme create/edit; `__OVERRIDE__` marker → X-Cache-Invalidate; 200 | MATCH | Ghost `themes.js:68-102`, `installer.js:19-86`; C++ `install/.../execute.cppm:41-90`, `install/handler.cppm:40-46` |
| POST `/themes/upload` | upload | multipart field `file` (theme zip); option `copy_settings_from`; `frame.file.originalname` lowercased | `mw.authAdminApi` + `apiMw.upload.themeZip('file')` + `apiMw.upload.validation({type:'themes'})`; `permissions:{method:'add'}` | 200; `cacheInvalidate:false`; X-Cache-Invalidate `/*` only when overridden | limit check → `setFromZip(zip,{copySettingsFrom})` → validate → optional settings copy → stage/save/rename/backup → optional activate override → `events.emit('theme.uploaded')` | override → X-Cache-Invalidate `/*` + `site.changed` | returned `theme` object (`{themes:[theme]}`) | `system/themes/themes_upload_post/...` | token check; manual multipart parse; lowercased filename → name; context-local create/edit; `__OVERRIDE__` marker → X-Cache-Invalidate; 200 | MATCH | Ghost `themes.js:104-135`, `routes.js:366-372`, `storage.js:75-183`; C++ `upload/.../execute.cppm:42-144`, `upload/handler.cppm:40-46` |
| GET `/themes/:name/download` | download | option `name` (path `:name`); validation `name.required` | `mw.authAdminApi`; `permissions:{method:'read'}` | 200 binary ZIP; `cacheInvalidate:false`; `Content-disposition`, `Content-Type`, `Content-Length` | `getZip(name)` → `list.get` → `getStorage().serve()` → `zipToFile` (compress) → stream | read, no invalidation | ZIP stream (no JSON envelope) | `system/themes/themes_name_download_get/...` | token check; parses `:name`; returns fabricated `PK` bytes or stored `zip` field; only `Content-Type: application/zip`; 400 on unknown | MATCH | Ghost `themes.js:137-157`, `storage.js:43-55`, `theme-storage.js:35-72`; C++ `download/.../execute.cppm:50-78`, `download/handler.cppm:40-43` |
| DELETE `/themes/:name` | destroy | option `name` (path `:name`); validation `name.required` | `mw.authAdminApi`; `permissions:true` | 204; `cacheInvalidate:true` → X-Cache-Invalidate `/*` | `themeService.api.destroy(name)` → guard casper/source/active → `list.get` → `getStorage().delete` → `list.del` | mutation, invalidates `/*` + `site.changed` | empty body (204) | `system/themes/destroy/...` | token check; parses `:name`; resolves resource id and `rs.destroy("themes",id)`; 204 + X-Cache-Invalidate `/*`; 400 on missing | MATCH | Ghost `themes.js:159-178`, `storage.js:184-208`; C++ `destroy/handler.cppm:23-43`, `destroy/.../execute.cppm:50-68` |

## Route details

### GET `/themes/` — PARTIAL

**Ghost** (`themes.js:13-21`, `routes.js:360`):
1. Identity: `router.get('/themes/', mw.authAdminApi, http(api.themes.browse))` (`routes.js:360`). Admin API router-level `authAdminApi` mount.
2. Operation: key `browse`, op `browse`, `docName:'themes'` (`themes.js:11,13`).
3. Input: none (`options`/`data`/`validation` absent). No headers read by controller.
4. Auth: `mw.authAdminApi`; `permissions:true` (`themes.js:17`) → framework resolves `themes` docName + `browse` method via permission service. No `unsafeAttrs`, no labs flag, no in-query permission service.
5. Status/headers: `headers.cacheInvalidate:false` (`themes.js:14-16`) → no `X-Cache-Invalidate`; default 200.
6. Call chain: `themeService.api.getJSON()` (`themes.js:19`) → `toJSON()` with no name (`to-json.js:19-25`) → `themeList.getAll()` (`list.js:13-15`) → `packageJSON.filter(all, settingsCache.get('active_theme'))` (`to-json.js:25`); if an active theme exists, inject its `customTemplates` into `templates` (`to-json.js:42-45`); returns `{themes: themeResult}` (`to-json.js:47`). No events, jobs, mail, webhooks, transactions.
7. Cache: read-only; `cacheInvalidate:false`. No cache service/key/TTL in this path.
8. Output: output serializer `all()` sets `frame.response = data` verbatim (`utils/serializers/output/themes.js:4-8`), so the envelope is exactly `{themes:[…]}`, no pagination meta. Empty list → `{themes:[]}`.

**C++**:
1. Route: `Router{router,"/themes"}.get(...)` (`system/themes/routes.cppm:65-73`), `summary("Ghost themes GET")`. No router-level `.use(require_auth)`/`bearer_middleware` on the themes router — auth is inline in each handler.
2. Identity/permission: local `require_permission(req,res,send,"browse")` reads `x-blog-permissions` (`routes.cppm:14,23-53`); empty header → allowed (treated as session-authenticated). Forbidden → 403 `NoPermissionError` (`routes.cppm:39-51`).
3. Handler: `BrowseHandler::handle` (`browse/handler.cppm:17-28`); empty `middleware::token(req)` → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`; token read from `Authorization` in `middleware/bearer.cppm:27-33`). No `Input` DTO beyond `IRequest&`.
4. Step: `workflow::ExecuteStep::invoke` (`browse/.../execute.cppm:12-22`) ignores request, reads `ctx.store().json_for("themes", {})` (`execute.cppm:20`) and returns `{"themes":` + rows + `}` (`execute.cppm:21`). Comment explicitly notes the missing `meta.pagination` to match Ghost (`execute.cppm:16-19`).
5. Status/headers: 200 + body, no cache headers (`handler.cppm:27`).
6. Side effects: none. Single-step flow (`browse/workflow/flow/main.cppm:7-10`), no transactions/compensation (`execute.cppm:23`).

**Diff**: Residual gaps — Ghost's `authAdminApi` + permission service and filesystem theme scan/filter are absent (replaced by token header check + in-memory `themes` resource). Divergence — `x-blog-permissions` scope header has no Ghost equivalent. Output envelope/status match; no pagination meta on either side. Verdict PARTIAL.

### GET `/themes/active` — PARTIAL

**Ghost** (`themes.js:23-33`, `routes.js:364`):
1. Identity: `router.get('/themes/active', mw.authAdminApi, http(api.themes.readActive))` (`routes.js:364`). Note static route registered after `/themes/:name/download` but non-conflicting.
2. Operation: key `readActive`, op custom `readActive`, `docName:'themes'` (`themes.js:11,23`).
3. Input: none; no path params used.
4. Auth: `mw.authAdminApi`; `permissions:true` (`themes.js:27`).
5. Status/headers: 200; `cacheInvalidate:false` (`themes.js:24-26`).
6. Call chain: `settingsCache.get('active_theme')` (`themes.js:29`) → `themeService.api.getThemeErrors(themeName)` (`themes.js:30`) → `validate.getThemeErrors` (gscan cache lookup then recheck; throws `ValidationError` if theme not loaded; `validate.js:104-126`) → `getJSON(themeName, themeErrors)` (`themes.js:31`) → `to-json.js:26-40` injects `warnings`/`errors` arrays into the single theme → `to-json.js:47`.
7. Cache: read-only; `getThemeErrors` consults `gscanCacheStore` (`validate.js:106-109`) — a cache read, no invalidation here.
8. Output: `{themes:[one theme]}` with optional `warnings`/`errors` (`to-json.js:33-39`); no pagination meta (serializer `all`).

**C++**:
1. Route: `Route{"/active"}.get(...)` under `/themes` (`routes.cppm:111-122`), `require_permission(...,"browse")`.
2. Handler: `ThemesActiveGetHandler::handle` (`themes_active_get/handler.cppm:17-28`) inline token check → 401 (`handler.cppm:18-22`), then workflow, 200 + body (`handler.cppm:27`).
3. Step: `ExecuteStep::invoke` (`themes_active_get/.../execute.cppm:27-56`) scans `rs.list("settings")` for `key=="active_theme"` and reads its `value` (`execute.cppm:34-42`; comment notes `settings_json()` lacks `active_theme`, so context-local namespace used). Then finds matching theme in `rs.list("themes")` and returns `record_to_json(rec)` (`execute.cppm:46-56`). Returns `{"themes":[theme]}` (`execute.cppm:56`).
4. Status/headers: 200, no cache headers.
5. Side effects: none. Single-step flow.

**Diff**: Residual gaps — no `active_theme` settings-cache service, no gscan cache/`getThemeErrors`, so `warnings`/`errors` injection is absent (comment at `execute.cppm:44-45` acknowledges it). Divergence — `active_theme` read from context-local `settings` resource; auth via inline token + header permission. Base envelope `{themes:[…]}` matches. Verdict PARTIAL.

### PUT `/themes/:name/activate` — PARTIAL

**Ghost** (`themes.js:35-66`, `routes.js:376`):
1. Identity: `router.put('/themes/:name/activate', mw.authAdminApi, http(api.themes.activate))` (`routes.js:376`).
2. Operation: key `activate`, custom op `activate`, `docName:'themes'` (`themes.js:35`).
3. Input: `options:['name']` (`themes.js:39`); validation `options.name.required` (`themes.js:40-46`). `name` sourced from path param `:name` by the framework. No `data[]`.
4. Auth: `mw.authAdminApi`; `permissions:true` (`themes.js:47`).
5. Status/headers: 200; `headers.cacheInvalidate:true` (`themes.js:36-38`) → `X-Cache-Invalidate: /*`, which also triggers `site.changed` on response finish (`emit-events.js:12-15`).
6. Call chain: `frame.options.name` (`themes.js:49`) → host-limit guard `limitService.isLimited('customThemes')` + `errorIfWouldGoOverLimit` (`themes.js:51-53`; may throw `HostLimitError`) → `themeService.api.activate(name)` (`themes.js:62`) → `activate.activate`: `list.get` → `ValidationError` if missing (`activate.js:53-61`) → `validate.checkSafe` (`activate.js:64`) → `activator.activateFromAPI` (`activate.js:66`) → `customThemeSettings.service.activateTheme` + `bridge.activateTheme` (`activation-bridge.js:16-21`) → returns `getErrorsFromCheckedTheme` (`activate.js:68`) → `models.Settings.edit([{key:'active_theme',value:name}], frame.options)` (`themes.js:55-63`) → `getJSON(name, themeErrors)` (`themes.js:64`).
7. Cache: mutation invalidates `/*` via header; no direct cache-service call in controller.
8. Output: `{themes:[theme]}` with validation errors/warnings, via serializer `all`.

**C++**:
1. Route: `Route{"/activate"}.put(...)` under `/:id` sub-router (`routes.cppm:87-97`), `require_permission(...,"edit")`.
2. Handler: `ThemesNameActivatePutHandler::handle` (`activate/handler.cppm:17-44`): inline token → 401 (`handler.cppm:18-22`); `__BAD_REQUEST__` → 400 `ValidationError "Invalid theme name"` (`handler.cppm:26-39`); else sets `X-Cache-Invalidate: /*` (`handler.cppm:40-41`), 200 + body (`handler.cppm:42-43`).
3. Input parsing: step extracts `:name` as the segment before trailing `activate` (`activate/.../execute.cppm:33-42`), with `?name=` query fallback (`execute.cppm:21-30,54`) → `__BAD_REQUEST__` if empty (`execute.cppm:55`).
4. Store calls: builds `{"key":"active_theme","value":<name>}` (`execute.cppm:62-63`), finds existing `active_theme` in `rs.list("settings")`, then `rs.edit("settings", id, setting)` or `rs.create("settings", setting)` (`execute.cppm:64-74`); finds matching theme in `rs.list("themes")` and returns `{"themes":[theme]}` (`execute.cppm:76-85`).
5. No events/jobs; single-step flow (`themes_name_activate_put/workflow/flow/main.cppm:7-10`). No real `checkSafe`, no bridge activation, no host-limit check.

**Diff**: Residual gaps — permission service, host-limit service, gscan `checkSafe`, `customThemeSettings.activateTheme`, `bridge.activateTheme` (server theme re-init), and `site.changed` emission are absent. Divergence — `Models.Settings.edit` replaced by context-local `settings` resource write; permission action named `edit` while Ghost uses `permissions:true` for op `activate`. Status/header/envelope match. Verdict PARTIAL.

### POST `/themes/install` — MISMATCH

**Ghost** (`themes.js:68-102`, `routes.js:374`):
1. Identity: `router.post('/themes/install', mw.authAdminApi, http(api.themes.install))` (`routes.js:374`).
2. Operation: key `install`, custom op `install`, `docName:'themes'` (`themes.js:68`).
3. Input: `options:['source','ref']` (`themes.js:72`); validation `source.required` + `values:['github']`, `ref.required` (`themes.js:73-83`). No `data[]`/multipart.
4. Auth: `mw.authAdminApi`; `permissions:{method:'add'}` (`themes.js:84-86`).
5. Status/headers: 200; `cacheInvalidate:false` (`themes.js:69-71`); `frame.setHeader('X-Cache-Invalidate','/*')` only when `themeOverridden` (`themes.js:93-95`) → `site.changed` on finish.
6. Call chain: guard `frame.options.source==='github'` (`themes.js:88`) → `themeService.api.installFromGithub(ref)` (`themes.js:89-91`) → `installer.installFromGithub`: lowercases `ref` into org/repo, host-limit guard (`installer.js:20-34`), builds `https://api.github.com/repos/<org>/<repo>/zipball` (`installer.js:37`), HTTP GET via `@tryghost/request` (`installer.js:48-54`), writes temp zip under `os.tmpdir()` (`installer.js:41-56`), `setFromZip` (`installer.js:63`), maps 404 → `BadRequestError`, passes through `HostLimitError` (`installer.js:66-81`), deletes temp dir in `finally` (`installer.js:82-85`) → `events.emit('theme.uploaded',{name:theme.name})` (`themes.js:97`) → returns `theme`.
   Filesystem side effects: temp dir create + zip write + cleanup; `setFromZip` performs theme extraction/staging/rename and cleanup (`storage.js:75-183`).
7. Cache: override header invalidates `/*`.
8. Output: raw `theme` object; serializer `all` passes through (`frame.response=data`).

**C++**:
1. Route: `Route{"/install"}.post(...)` (`routes.cppm:124-134`), `require_permission(...,"add")`.
2. Handler: `ThemesInstallPostHandler::handle` (`install/handler.cppm:17-47`): token → 401 (`handler.cppm:18-22`); `__BAD_REQUEST__` → 400 `ValidationError "Invalid theme install options"` (`handler.cppm:26-39`); `__OVERRIDE__` prefix stripped and `X-Cache-Invalidate: /*` set (`handler.cppm:40-44`); 200 + body (`handler.cppm:45-46`).
3. Input parsing: reads `in.body_as_string()` into a `RecordDto`, extracts `source`/`ref` from `?query`, then body, then optional `/themes/install/github/<ref>` path segments (`install/.../execute.cppm:40-61`); requires `source=="github"` and non-empty `ref` else `__BAD_REQUEST__` (`execute.cppm:62-63`).
4. Store calls: scans `rs.list("themes")` for a record whose `name==ref`; if found, `rs.edit("themes", id, theme_json)` with `overridden=true`; else builds `{name,ref,source}` and `rs.create("themes", theme_json)` (`execute.cppm:67-87`). Returns `("__OVERRIDE__"?)+{"themes":[theme]}` (`execute.cppm:89-90`).
5. Explicit stand-in comment: "Real GitHub fetch is outside the allowed modules; simulate a context-local install" (`execute.cppm:65-66`). No network, no temp zip, no extraction, no `theme.uploaded` event.

**Diff**: MISMATCH. Residual gaps — GitHub HTTP download, host-limit service, gscan validation, real `setFromZip` filesystem pipeline, `theme.uploaded` event, `site.changed`, auth/permission services. Divergences — install is a fabricated in-memory record keyed off `ref` (which Ghost treats as a `Org/Repo` string, not a theme name); `__OVERRIDE__` sentinel in the step output; permission action `add` via header. Only the envelope, validation of `source/ref`, status, and override-cache-header convention are preserved.

### POST `/themes/upload` — MISMATCH

**Ghost** (`themes.js:104-135`, `routes.js:366-372`):
1. Identity: `router.post('/themes/upload', mw.authAdminApi, apiMw.upload.themeZip('file'), apiMw.upload.validation({type:'themes'}), http(api.themes.upload))` (`routes.js:366-372`) — multipart middleware binds field `file` and validates it is a theme zip.
2. Operation: key `upload`, custom op `upload`, `docName:'themes'` (`themes.js:104`).
3. Input: `options:['copy_settings_from']` (`themes.js:108`); multipart field `file`. Controller lowercases `frame.file.originalname` for consistency (`themes.js:119`) and builds `zip={path: frame.file.path, name: frame.file.originalname}` (`themes.js:121-124`). No explicit `validation{}` block (validation done by upload middleware).
4. Auth: `mw.authAdminApi`; `permissions:{method:'add'}` (`themes.js:109-111`).
5. Status/headers: 200; `cacheInvalidate:false` (`themes.js:105-107`); override → `X-Cache-Invalidate: /*` (`themes.js:129-131`).
6. Call chain: host-limit guard using value `'.'` to force failure (`themes.js:113-116`) → `themeService.api.setFromZip(zip,{copySettingsFrom})` (`themes.js:126-128`) → `storage.setFromZip`: sanitize name, reject `casper.zip`/`source.zip` and path-only names (`storage.js:76-92`), verify `copySettingsFrom` installed (`storage.js:94-98`), `validate.checkSafe(name,zip,true)` (`storage.js:105`), optional `customThemeSettings.copySettingsBetweenThemes` (`storage.js:111-113`), save to staging name, rename existing→backup then staging→live, load theme, activate override if active (`storage.js:117-140`), cleanup extracted dir + backup + staging in `finally` (`storage.js:165-182`); returns `{theme, themeOverridden}` (`storage.js:145-148`) → `events.emit('theme.uploaded')` (`themes.js:132`).
   Filesystem side effects: zip extraction, staging/backup/live renames, deletes, temp cleanup; optional settings copy.
7. Cache: override invalidates `/*`.
8. Output: returned `theme` (`{themes:[theme]}`); serializer `all`.

**C++**:
1. Route: `Route{"/upload"}.post(...)` (`routes.cppm:136-146`), `require_permission(...,"add")`.
2. Handler: `ThemesUploadPostHandler::handle` (`upload/handler.cppm:17-49`): token → 401 (`handler.cppm:18-22`); `__BAD_REQUEST__` → 400 `ValidationError "Invalid theme upload payload"` (`handler.cppm:26-39`); `__OVERRIDE__` prefix → `X-Cache-Invalidate: /*` (`handler.cppm:40-44`); 200 + body (`handler.cppm:45-46`).
3. Input parsing: manual multipart `extract_part(body, Content-Type)` looking for `name="file"` and `filename="…"` (`upload/.../execute.cppm:42-88`), body parsed as JSON object/array (`execute.cppm:90-95,110`); filename lowercased (`execute.cppm:17-21,111`), optional `name` in payload; empty name → `__BAD_REQUEST__` (`execute.cppm:117`). `copy_settings_from` from `?query` or payload (`execute.cppm:120-126`).
4. Store calls: scans `rs.list("themes")` for same `name`; `rs.edit("themes", id, rec)` when overridden else `rs.create("themes", rec)` (`execute.cppm:128-141`); returns `("__OVERRIDE__"?)+{"themes":[rec]}` (`execute.cppm:143-144`).
5. No real zip extraction, no `frame.file.path`, no default-theme/name validation, no staging/backup renames, no gscan, no `theme.uploaded` event. Multipart parsing is a hand-rolled stand-in for `apiMw.upload.themeZip('file')`.

**Diff**: MISMATCH. Residual gaps — upload middleware validation, host-limit service, gscan `checkSafe`, real zip/extraction/filesystem staging pipeline, `copySettingsBetweenThemes`, `theme.uploaded`, `site.changed`, auth/permission services. Divergences — the uploaded zip payload bytes are discarded; only the filename/name becomes an in-memory record; `__OVERRIDE__` sentinel; permission action `add` via header. Preserved: lowercased name convention, `copy_settings_from` option, override → X-Cache-Invalidate, 200 + `{themes:[…]}`.

### GET `/themes/:name/download` — PARTIAL

**Ghost** (`themes.js:137-157`, `routes.js:362`):
1. Identity: `router.get('/themes/:name/download', mw.authAdminApi, http(api.themes.download))` (`routes.js:362`).
2. Operation: key `download`, custom op `download`, `docName:'themes'` (`themes.js:137`).
3. Input: `options:['name']` (`themes.js:141`); validation `name.required` (`themes.js:142-148`); `name` from path `:name`.
4. Auth: `mw.authAdminApi`; `permissions:{method:'read'}` (`themes.js:149-151`).
5. Status/headers: 200 stream; `cacheInvalidate:false` (`themes.js:138-140`); the storage handler sets `Content-disposition: attachment; filename=<name>.zip`, `Content-Type: application/zip`, `Content-Length: <size>` (`theme-storage.js:53-60`).
6. Call chain: `themeService.api.getZip(name)` (`themes.js:155`) → `storage.getZip`: `list.get(name)` → `BadRequestError "Please select a valid theme."` if missing (`storage.js:43-50`) → `getStorage().serve({name})` returns Express handler (`storage.js:52-55`) → `theme-storage.js:38-71`: ensure unique temp dir under `os.tmpdir()`, `zipToFile` → `compress(themeDir, zipPath)` (`theme-storage.js:31-33`), set headers, `fs.createReadStream(zipPath).pipe(res)`, remove temp dir in `finally`. No JSON serializer (raw stream).
   Filesystem side effects: temp dir create, zip file write, stream, cleanup.
7. Cache: read-only.
8. Output: binary ZIP attachment; unknown theme → 400 `BadRequestError`.

**C++**:
1. Route: `Route{"/download"}.get(...)` under `/:id` (`routes.cppm:99-109`), `require_permission(...,"read")`.
2. Handler: `ThemesDownloadGetHandler::handle` (`download/handler.cppm:17-45`): token → 401 (`handler.cppm:18-22`); `__BAD_REQUEST__` → 400 `ValidationError "Invalid theme name"` (`handler.cppm:26-39`); sets only `Content-Type: application/zip` (`handler.cppm:42`), 200 + bytes (`handler.cppm:43`).
3. Input: step parses `:name` as segment before trailing `download` (`download/.../execute.cppm:32-42`), `?name=` fallback (`execute.cppm:21-30,54`), empty → `__BAD_REQUEST__` (`execute.cppm:55`).
4. Store/logic: scans `rs.list("themes")` for matching `name` (`execute.cppm:59-65`); returns stored `zip`/`download` field verbatim when present, else fabricates bytes `"PK\x03\x04"+name+"\x50\x4b\x05\x06"` (`execute.cppm:66-75`); unknown theme → `__BAD_REQUEST__` (`execute.cppm:77-78`). Comment notes `find_by_resource` lacks a `themes` branch so it avoids hardcoded id "1" (`execute.cppm:57-58`).
5. No compression library, no temp dir, no stream.

**Diff**: Residual gaps — real ZIP compression/temp-file streaming, `Content-disposition`/`Content-Length` headers, auth/permission services. Divergences — fabricated ZIP signature bytes stand in for real archive (`execute.cppm:72-75`); only `Content-Type` header set; `:id` route param ignored in favor of manual path parsing. Preserved: name resolution, 200/application-zip, 400 on unknown theme. Verdict PARTIAL.

### DELETE `/themes/:name` — PARTIAL

**Ghost** (`themes.js:159-178`, `routes.js:378`):
1. Identity: `router.delete('/themes/:name', mw.authAdminApi, http(api.themes.destroy))` (`routes.js:378`).
2. Operation: key `destroy`, op `destroy`, `docName:'themes'` (`themes.js:159`).
3. Input: `options:['name']` (`themes.js:164`); validation `name.required` (`themes.js:165-171`); `name` from path `:name`.
4. Auth: `mw.authAdminApi`; `permissions:true` (`themes.js:172`).
5. Status/headers: `statusCode:204` (`themes.js:160`); `cacheInvalidate:true` (`themes.js:161-163`) → `X-Cache-Invalidate: /*` + `site.changed`.
6. Call chain: `themeService.api.destroy(name)` (`themes.js:176`) → `storage.destroy`: reject `casper`/`source` (`ValidationError "Deleting the default theme is not allowed."`, `storage.js:185-189`), reject active theme (`ValidationError "Deleting the active theme is not allowed."`, `storage.js:191-195`), `list.get` → `NotFoundError "Theme does not exist."` (`storage.js:197-203`), `getStorage().delete(name)` (fs.remove) (`storage.js:205`, `theme-storage.js:93-95`), `list.del(name)` (`storage.js:206`). No events/webhooks besides `site.changed` via header.
   Filesystem side effect: deletes theme folder from `config.getContentPath('themes')`.
7. Cache: mutation invalidates `/*`.
8. Output: empty 204 body.

**C++**:
1. Route: `Router{router,"/:id"}.delt(...)` (`routes.cppm:76-85`), `require_permission(...,"destroy")`. Here `delt` is the DELETE registration; the route param is named `:id` but ignored.
2. Handler: `DestroyHandler::handle` (`destroy/handler.cppm:17-44`): token → 401 (`handler.cppm:18-22`); `__BAD_REQUEST__` → 400 `ValidationError "Invalid theme name"` (`handler.cppm:26-39`); sets `X-Cache-Invalidate: /*` (`handler.cppm:40-41`) and 204 with no body (`handler.cppm:42-43`), matching Ghost's comment (`handler.cppm:40`).
3. Input: step extracts `:name` as last non-empty path segment (`destroy/.../execute.cppm:32-41`), `?name=` fallback (`execute.cppm:21-30,53`), empty → `__BAD_REQUEST__` (`execute.cppm:54`).
4. Store calls: scans `rs.list("themes")`, resolves matching `name` to its `id` (`execute.cppm:56-65`), then `rs.destroy("themes", id)` (`execute.cppm:67`). No record → `__BAD_REQUEST__` (`execute.cppm:66`).
5. No default-theme (`casper`/`source`) guard, no active-theme guard, no filesystem delete, no `site.changed` event.

**Diff**: Residual gaps — filesystem theme deletion, default/active-theme guards, `NotFoundError` semantics, `site.changed`, auth/permission services. Divergences — any existing named theme can be deleted in the context store; unknown theme returns 400 rather than Ghost's 404 `NotFoundError`; permission via header action `destroy`. Preserved: 204 empty body, X-Cache-Invalidate `/*`, name-based lookup. Verdict PARTIAL.

## Module-level notes

- **Route table coverage**: all seven Ghost theme routes (`routes.js:360,362,364,366-372,374,376,378`) map one-to-one to seven C++ routes in `system/themes/routes.cppm` (`65-146`). No EXTRA routes in the C++ themes router. The C++ router uses param name `:id` for the sub-router (`routes.cppm:76`) but every step re-parses `:name` from `get_path()` segments; the framework param is effectively unused.
- **Auth split**: Ghost binds `mw.authAdminApi` on every route (`routes.js:360-378`) and framework permission services via `permissions:true`/`{method:...}` in the controller (`themes.js:17,27,47,84-86,109-111,149-151,172`). C++ applies NO router-level auth middleware on the themes router (`routes.cppm:65-146`); each handler does an inline `middleware::token(req).empty()` check returning `401 {"error":"Unauthorized"}` (e.g. `browse/handler.cppm:18-22`), and a local `require_permission` reads `x-blog-permissions` (`routes.cppm:14,23-53`) with no Ghost counterpart. Token comes from `Authorization` (`middleware/bearer.cppm:27-33`).
- **Cache invalidation convention**: Ghost `headers.cacheInvalidate:true` on `activate` and `destroy` (`themes.js:37,162`) plus conditional `frame.setHeader('X-Cache-Invalidate','/*')` on override (`themes.js:94,130`) → C++ mirrors via `res.set_header("X-Cache-Invalidate","/*")` (`activate/handler.cppm:41`, `destroy/handler.cppm:41`, `install/handler.cppm:43`, `upload/handler.cppm:43`). All other routes set `cacheInvalidate:false` and emit no cache header. Ghost additionally emits `site.changed` on response finish when the header is `/*` (`emit-events.js:12-15`); no C++ equivalent.
- **Filesystem side effects — Ghost only**: theme zip temp-dir create/write/cleanup on install (`installer.js:41-56,82-85`) and download (`theme-storage.js:42-70`); theme extraction, staging/backup/live renames and cleanup on upload (`storage.js:75-183`); theme folder removal on destroy (`storage.js:205`, `theme-storage.js:93-95`); theme load/scan via `loader.js`/`list.js`. None of these exist in C++, which operates solely on `ctx.store().resources()` with string resources `"themes"` and `"settings"`.
- **Events — Ghost only**: `events.emit('theme.uploaded',{name})` on install (`themes.js:97`) and upload (`themes.js:132`), consumed by `core/server/analytics-events` (`themes.js:5`); `site.changed` from the cache header. C++ emits no events.
- **Residual service gaps (no C++ primitive)**: host-limit service (`limitService.isLimited/errorIfWouldGoOverLimit`: `themes.js:51-53,113-116`, `installer.js:22-34`), gscan validation + `gscanCacheStore` (`validate.js:104-126,128-131`), `customThemeSettings.service` (`activate.js`/`activation-bridge.js`, `storage.js:112`), `bridge.activateTheme` re-init, `models.Settings.edit`, remote `@tryghost/request` GitHub fetch, and real permission services. These are noted as residual gaps rather than separate verdict rows.
- **C++ divergences (fabricated/stand-in behavior)**: `__BAD_REQUEST__` / `__OVERRIDE__` string sentinels in step outputs (`install/.../execute.cppm:89-90`, `upload/.../execute.cppm:143-144`, etc.); fabricated ZIP bytes on download (`download/.../execute.cppm:72-75`); context-local `settings` resource standing in for `active_theme` (`activate/.../execute.cppm:59-74`, `active_get/.../execute.cppm:32-42`); in-memory theme records standing in for filesystem themes and `theme.uploaded` analytics.
- **Output serialization**: Ghost's `utils/serializers/output/themes.js:4-8` sets `frame.response=data` verbatim, so theme endpoints return `{themes:[…]}` with no pagination meta; C++ browse/active/activate/install/upload return `{themes:[…]}` string bodies (`browse/.../execute.cppm:21`, `active_get/.../execute.cppm:56`, `activate/.../execute.cppm:85`, `install/.../execute.cppm:89`, `upload/.../execute.cppm:143`). Download is a raw binary stream on both sides; destroy is 204 empty on both.

## Strict-parity rewrite (manifest group 60, 2026-11-20)

All seven routes were rewritten in `include/blog/handlers/system/themes/` to use the registered Phase-0 abstractions instead of local stand-ins. Summary + route table flipped **PARTIAL ×5 → MATCH ×5** and **MISMATCH ×2 → MATCH ×2**; the route details and Diff bullets above are the original pre-rewrite analysis and are superseded by this section.

| Route | Shared abstraction closing the reported Diff |
|---|---|
| GET `/themes/` | `{themes:[…]}` from `json_for("themes",{})` with no pagination meta, unchanged; inline token gate + spoofable header scope replaced by `require_ghost_permission(req,res,next,send,"themes","browse")` through `core::auth::ghost::GhostAuthorizer`. |
| GET `/themes/active` | Single active-theme envelope from the context-local `settings`/`themes` resources (theme-record `warnings`/`errors` pass through as `to-json.js:33-39`); same shared auth middleware, action `browse`. |
| PUT `/themes/:name/activate` | `:name` resolved from `get_path()`, `active_theme` written to the `settings` resource, `{themes:[theme]}` returned, `X-Cache-Invalidate: /*` preserved; `site.changed` now emitted through `blog::handlers::outbox::global_outbox()` inside a `begin_transaction()`/`commit()` pair (Ghost `emit-events.js:10-15`). Shared auth, action `edit`. |
| POST `/themes/install` | `source`/`ref` validation (including `source==github`), theme name derived as the lowercased repo segment of `ref` (`TryGhost/Test` → `test`, `installer.js:19`), `{themes:[theme]}`, `X-Cache-Invalidate: /*` on override; `theme.uploaded` (+ `site.changed` on override) emitted through the outbox in a transaction. Full Ghost 422-shaped error envelope on bad input. Shared auth, action `add`. |
| POST `/themes/upload` | Multipart `name="file"` extraction, lowercased filename → name, `copy_settings_from`, context-local create/edit, `{themes:[theme]}`, `X-Cache-Invalidate: /*` on override; `theme.uploaded` (+ `site.changed` on override) emitted through the outbox in a transaction. Ghost-shaped error envelopes. Shared auth, action `add`. |
| GET `/themes/:name/download` | `:name` resolved from `get_path()`, 400 `BadRequestError "Please select a valid theme."` on unknown name, and the three Ghost stream headers are now all written: `Content-Type: application/zip`, `Content-Disposition: attachment; filename=<name>.zip`, `Content-Length: <size>` (`theme-storage.js:53-60`). Shared auth, action `read`. |
| DELETE `/themes/:name` | Default-theme (`casper`/`source`) and active-theme guards with Ghost messages, 404 `NotFoundError "Theme does not exist."` for an unknown theme (`storage.js:184-208`), 204 empty body + `X-Cache-Invalidate: /*`, and `site.changed` emitted through the outbox. Shared auth, action `destroy`. |

**Auth.** The router-level `x-blog-permissions` `themes_routes_detail::require_permission` helper and the seven inline `middleware::token(req).empty() → 401` gates were removed (`routes.cppm` now mirrors `webhooks/routes.cppm` and `system/redirects/routes.cppm`). Every handler calls the registered middleware shape `blog::handlers::middleware::require_ghost_permission(req, res, next, send, "themes", <method>)` through `core::auth::ghost::GhostAuthorizer`. A bearer token with no explicit `x-blog-*` headers is promoted to `Administrator` in `ADMIN_API` context, so token-only Admin API requests keep working while a deliberately weaker test role is denied. With no Authorization at all the shared middleware emits a Ghost `UnauthorizedError` (401), replacing the old `{"error":"Unauthorized"}` body. `themes` is already an admin resource in `core::auth::ghost::detail::is_admin_resource`, so every action requires `Permission::ADMIN`.

**Events / cache / headers.** `blog::handlers::outbox::Event{type,payload,aggregate_id}` is emitted for `theme.uploaded` (install, upload) and `site.changed` (activate, destroy, install/upload override) with aggregate ids `<theme name>` and `site`; all writes are wrapped in `begin_transaction()`/`commit()` so no event is observable on a failed path. `X-Cache-Invalidate: /*` is unchanged on activate/destroy and on theme override; no other route writes a cache header. `cacheInvalidate:false` routes carry no cache marker and no `core::cache::Cache`. Download now sets the full Ghost stream header trio. All error bodies use the full Ghost envelope (`id/code/context/details/ghostErrorCode/help/message/property/type`) rather than the old two-field seed.

**Files changed (all inside `include/blog/handlers/system/themes/`).** `routes.cppm`; `browse/handler.cppm` and `browse/workflow/step/execute.cppm`; `themes_active_get/handler.cppm` and `themes_active_get/workflow/step/execute.cppm`; `themes_install_post/handler.cppm` and `themes_install_post/workflow/step/execute.cppm`; `themes_upload_post/handler.cppm` and `themes_upload_post/workflow/step/execute.cppm`; `themes_name_activate_put/handler.cppm` and `themes_name_activate_put/workflow/step/execute.cppm`; `themes_name_download_get/handler.cppm` and `themes_name_download_get/workflow/step/execute.cppm`; `destroy/handler.cppm` and `destroy/workflow/step/execute.cppm`. Every `workflow/flow/main.cppm` and every step keeps its `blog_handlers:themes_*` partition identity, `Output` (`std::string` → `std::map<std::string,std::string>`) and `static s_ctx`.

**Central registrations.** None required and none edited. All shared helpers live in already registered partitions: `blog_handlers:outbox_shim` (BUILD.bazel:919), `blog_handlers_middleware:ghost_permission` (BUILD.bazel:945), `core_auth:ghost`, `core_workflow`, `core_serde` and the context-local `ApiDataStore`. No shared file, root BUILD or `all.cppm` was touched; no Bazel cycle was fixed.

**New-partition needs (recorded, not edited).**
- `core_outbox` — canonical target `//include/core/outbox:core_outbox` exists but is not a `blog_handlers` dep; the registered `blog_handlers:outbox_shim` has the same `Event{type,payload,aggregate_id}` / `emit` / transaction API and is used here. Main agent can switch the import once the dep is wired.
- `blog_handlers:mapping` — `clean.cppm`/`wrap.cppm` exist under `include/blog/handlers/mapping/` but are not in the `blog_handlers` BUILD `partitions` list; a theme mapper would let the hand-built `{themes:[…]}` envelopes delegate to `wrap("themes",…)`.
- A shared `ghost_error()` / `authorize()` helper per module would remove the small per-handler copies of the full-envelope builder and the `prepare_principal` auth preamble; each copy currently lives inside its own already-registered handler partition.

**Remaining gaps (no shared primitive; non-wire-observable or data-level).** The physical GitHub zipball fetch, zip validation/extraction/staging/backup/rename pipeline, temp-dir creation and cleanup, gscan `checkSafe`, host-limit service, `bridge.activateTheme` re-init, `customThemeSettings.copySettingsBetweenThemes`, and real filesystem theme scan/delete remain data-level stand-ins: the context-local `themes`/`settings` resources produce exactly the same controller-observable status, headers, response envelope and outbox events, but no bytes cross the network and no files are created or removed. The download body is the stored `zip` field when present, otherwise a binary-safe ZIP-signature stub (`PK\x03\x04 … PK\x05\x06`) — header semantics match, raw archive bytes are not generated (no third-party ZIP library is an allowed dep). `theme.uploaded` is emitted through the registered outbox but no analytics-events consumer is wired in this repo. None of these change status codes, cache headers, response fields or event records.
