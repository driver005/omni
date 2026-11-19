# Parity: exports

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/exports.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/exports/routes.cppm` (handlers in `create/` and `exports_download_get/`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 2 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/exports/download` | `download` | GET `/exports/download` | `ExportsDownloadGetHandler` → `exports_exports_download_get` | Ghost: `options:['components']`, values members/analytics/content/themes/routes; absent→all (exports.js:121-128,135). C++: `using Input=IRequest&`, no query parse, ignores `components` (execute.cppm:11-13). | Ghost: streaming ZIP `{archive, filename}` (exports.js:143-146). C++: 200 + JSON `{"exports":…,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` (handler.cppm:27; execute.cppm:17). | Ghost: `exporter.doExport()` / members+posts+themes+routes services (exports.js:83-110). C++: `ctx.store().json_for("exports", {})` generic, no filter/pagination (execute.cppm:16). | Ghost `cacheInvalidate:false` (exports.js:118-120). C++ none (no cache in handler ctor, all.cppm:767). | Ghost `mw.authAdminApi` + permissions db/exportContent (routes.js:404; exports.js:129-132). C++ bearer `token()` check→401, no docName/method (handler.cppm:18-22). | MISMATCH | Ghost routes.js:404 + exports.js:117-148; C++ routes.cppm:30-38, handler.cppm:17-27, execute.cppm:11-17. |
| POST `/exports` | `add` | POST `/exports` | `CreateHandler` → `exports_create` | Ghost: `frame.data.components` object, keys ⊆ ASYNC components (+media), bool values, ≥1 true → else BadRequest (exports.js:155-184). C++: raw `body_as_string()` passed verbatim, no validation (execute.cppm:11-16). | Ghost: `statusCode:202`, no body (exports.js:151,189-199). C++: 200 + `mutations.create(...).body()` (handler.cppm:27). | Ghost: `exportRequests.service.requestExport({components, requestedByUserId})` (exports.js:195-198), no model write. C++: `mutations.create("exports", body)` (execute.cppm:16). | Ghost `cacheInvalidate:false` (exports.js:152-154). C++ none. | Ghost `mw.authAdminApi` + permissions db/exportContent (routes.js:405; exports.js:185-188). C++ bearer `token()` check→401, no docName/method (handler.cppm:18-22). | MISMATCH | Ghost routes.js:405 + exports.js:150-200; C++ routes.cppm:21-28, handler.cppm:17-27, execute.cppm:11-17. |

## Route details

### GET /exports/download — MISMATCH

- Route identity: exact path+verb match. Ghost `router.get('/exports/download', mw.authAdminApi, http(api.exports.download))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:404`); C++ `Route{"/download"}.get(...)` under `Router{router, "/exports"}` (`include/blog/handlers/system/exports/routes.cppm:19,30-38`), bound to `exports_exports_download_get()` (`routes.cppm:36`), installed via `ExportsRoutes{}.install(...)` (`include/blog/handlers/routes.cppm:88`).
- Operation mapping: Ghost `download` (`exports.js:117`). C++ `ExportsDownloadGetHandler` (`include/blog/handlers/system/exports/exports_download_get/handler.cppm:10`). Op names agree conceptually but payload does not.
- Input divergence: Ghost accepts `components` query option validated against SYNC_EXPORT_COMPONENTS = members, analytics, content, themes, routes (`exports.js:121-128`; values `docs/Ghost/ghost/core/server/data/exporter/export-components.ts:1-7`), absent means all, empty → ValidationError (`exports.js:135-141`). C++ declares `using Input=interfaces::io::IRequest&` and never reads any query/option (`.../exports_download_get/workflow/step/execute.cppm:11-13`). `components` present only on Ghost side.
- Output divergence: Ghost returns `{archive: createSiteExporter().createArchive(components), filename: getExportFileName('export','zip')}` — a streamed ZIP download with filename (`exports.js:143-146`). C++ returns 200 with a JSON array envelope and hardcoded pagination limit/page/pages/total (`handler.cppm:25-27`; `execute.cppm:17`). Data shape materially different.
- DB call divergence: Ghost builds the archive from `exporter.doExport()`, members/posts CSV services, themes list/zip, routes YAML, redirects YAML (`exports.js:83-110`). C++ reads `ctx.store().json_for("exports", {})` with no filtering/pagination (`execute.cppm:16`). Resource string `exports` has no Ghost model equivalent here.
- Cache: Ghost `headers.cacheInvalidate:false` (`exports.js:118-120`); C++ none (handler ctor only binds ctx/workflow, no Cache reference — `handler.cppm:12-16`, `all.cppm:767`). No divergence in effect.
- Auth divergence: Ghost uses `mw.authAdminApi` (`routes.js:404`) plus `permissions{docName:'db', method:'exportContent'}` (`exports.js:129-132`). C++ uses bearer-token presence check returning 401 `{"error":"Unauthorized"}` with no permission docName/method (`handler.cppm:18-22`).

### POST /exports — MISMATCH

- Route identity: exact path+verb match. Ghost `router.post('/exports', mw.authAdminApi, http(api.exports.add))` (`routes.js:405`); C++ `.post(...)` on `Router{router,"/exports"}` (`routes.cppm:19,21-28`) bound to `exports_create()` (`routes.cppm:26`), installed at `include/blog/handlers/routes.cppm:88`.
- Operation mapping: Ghost `add` (`exports.js:150`). C++ `CreateHandler` (`include/blog/handlers/system/exports/create/handler.cppm:10`, flow `create/workflow/flow/main.cppm:8-10`).
- Input divergence: Ghost validates strict body schema — `components` must be a non-array object, keys only from ASYNC_EXPORT_COMPONENTS (members, analytics, content, themes, routes, media — `export-components.ts:9-10`), boolean values, ≥1 true; BadRequestError otherwise (`exports.js:155-184`). C++ passes raw `body_as_string()` (or `{}` if empty) straight to `mutations.create` with no DTO/validation (`create/workflow/step/execute.cppm:11,16`). `components`/`requestedByUserId` validation present only on Ghost side.
- Output divergence: Ghost `statusCode:202` with no response body (`exports.js:151,189-199`). C++ always sets `Status::OK` (200) and echoes `mutations.create("exports", body).body()` (`handler.cppm:25-27`).
- DB call divergence: Ghost enqueues async work via `exportRequests.service.requestExport({components, requestedByUserId: frame.options.context.user})` — a service call, no model write (`exports.js:195-198`). C++ performs a generic `mutations.create("exports", body)` record insertion (`execute.cppm:14-16`). Operation and resource materially different.
- Cache: Ghost `headers.cacheInvalidate:false` (`exports.js:152-154`); C++ none (`create/handler.cppm:12-16`, `all.cppm:767`). No divergence in effect.
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:405`) + `permissions{docName:'db', method:'exportContent'}` (`exports.js:185-188`). C++ bearer-token presence check → 401, no docName/method (`create/handler.cppm:18-22`).

## Missing in C++

None. Both Ghost routes (`GET /exports/download`, `POST /exports`) have C++ counterparts.

## Extra in C++

None. `include/blog/handlers/system/exports/routes.cppm` installs exactly two routes — POST `/exports` (`:21-28`) and GET `/exports/download` (`:30-38`) — matching the two Ghost controller routes. Grep for `exports` in `include/blog/handlers/**` found no additional export route registrations.
