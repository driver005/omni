# Parity: app-installations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-installations.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 227–238)
- C++: none found for `/apps/installations*` (only `include/blog/handlers/external/apps/routes.cppm`, which installs `GET /apps` → `apps_all_collection`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 5 |
| EXTRA | 1 |

All five Ghost `appInstallations` routes are MISSING in C++. No C++ route or handler implements
`/apps/installations` for any HTTP verb. The only C++ app route, `GET /apps`, has no Ghost
counterpart in this controller (or anywhere in the admin route table) and is recorded as EXTRA.

## Controller facts (Ghost side)

- `docName` / export key: `app_installations` (`app-installations.ts:55`), exposed to the router as
  `api.appInstallations` via `endpoints/index.js:351-353`.
- Operations: `browse`, `read`, `add`, `edit`, `destroy` (`app-installations.ts:57,65,77,91,105`).
- Common headers: `noCacheInvalidation = { cacheInvalidate: false }` (`app-installations.ts:52`)
  applied to all five operations (`:58,66,79,92,107`). No `X-Cache-Invalidate` URL is set.
- Auth/mount: all routes sit under `router.all(['/apps', '/apps/*'], mw.authAdminApi)`
  (`admin/routes.js:228`) and `appsRouter.use(labs.enabledMiddleware('apps'))`
  (`admin/routes.js:230`). Per-operation `permissions`: `true` for browse/read/add/destroy
  (`:59,69,80,110`); `edit` uses `permissions: {method: 'add'}` (`:95`).
- Validation helper: body must contain a non-empty `app_installations` array (framework-enforced,
  `app-installations.ts:16-21`); `reviewed()` zod-checks `manifest_url` + `digest` (`:27,32-40`);
  `reviewedChange()` additionally zod-checks `revision` as non-negative int (`:30,42-50`).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/apps/installations` | browse | none | none | Ghost: no `options[]`/`data[]`/`validation`; service `browse()` takes no args | Ghost: `Installation[]` (serialized `{data:[...]}`), status 200 | Ghost: `appInstallations.service.browse()` → `repository.list()` (`service.ts:169-171`) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `authAdminApi` + labs `apps` + `permissions:true`; C++: none | MISSING | Ghost: `admin/routes.js:233`, `app-installations.ts:57-63`, `service.ts:169-171`. C++: no route — grep `installations` in `include/blog/handlers` → only `README.md:9`; `include/blog/handlers/external/apps/routes.cppm:19` is `GET /apps` only; `include/blog/handlers/routes.cppm:4,77` installs only `AppsRoutes`. |
| POST `/apps/installations` | add | none | none | Ghost: `data.app_installations[0]` with `manifest_url` + `digest` (`app-installations.ts:16-21,27,32-40`); `options.context`; no `options[]`/`validation` declared | Ghost: `Installation`, status **201** (`app-installations.ts:78`) | Ghost: `service.create(actingContext(frame.options.context), reviewed(frame))` (`app-installations.ts:82-85`) → `repository.insert` (`service.ts:201-221`) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `authAdminApi` + labs `apps` + `permissions:true`; C++: none | MISSING | Ghost: `admin/routes.js:234`, `app-installations.ts:77-87`, `service.ts:201-221`. C++: no route — see search evidence in browse row. |
| GET `/apps/installations/:id` | read | none | none | Ghost: `options:['id','include']`, `validation` `id` required, `include` values `['history']` (`app-installations.ts:67-68`); maps `include→withRelated`, `withHistory` flag (`:71-73`) | Ghost: `InstallationWithHistory` (optional `history`), status 200 | Ghost: `service.read(id,{withHistory})` → `repository.find(id)` + optional `historyOf` (`service.ts:177-189,318-325`) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `authAdminApi` + labs `apps` + `permissions:true`; C++: none | MISSING | Ghost: `admin/routes.js:235`, `app-installations.ts:65-75`, `service.ts:177-189`. C++: no route — see search evidence in browse row. |
| PUT `/apps/installations/:id` | edit | none | none | Ghost: `options:['id']`, `id` required (`app-installations.ts:93-94`); body `manifest_url` + `digest` + `revision` (`:30,42-50`) | Ghost: `Installation`, status 200 | Ghost: `service.approve(actingContext, id, reviewedChange(frame))` (`app-installations.ts:97-101`) → `repository.find` + `whileUnchanged`/manifest writes (`service.ts:234-279,490-523`) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `authAdminApi` + labs `apps` + **`permissions:{method:'add'}`** (`app-installations.ts:95`); C++: none | MISSING | Ghost: `admin/routes.js:236`, `app-installations.ts:91-103`, `service.ts:234-279`. C++: no route — see search evidence in browse row. |
| DELETE `/apps/installations/:id` | destroy | none | none | Ghost: `options:['id']`, `id` required (`app-installations.ts:108-109`); no body fields | Ghost: empty, status **204** (`app-installations.ts:106`) | Ghost: `service.uninstall(actingContext, id)` (`app-installations.ts:112-115`) → `repository.uninstall` + `recordAction` (`service.ts:302-316`) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `authAdminApi` + labs `apps` + `permissions:true`; C++: none | MISSING | Ghost: `admin/routes.js:238`, `app-installations.ts:105-117`, `service.ts:302-316`. C++: no route — see search evidence in browse row. |

## Route details

### GET `/apps/installations` — MISSING

- Ghost route `admin/routes.js:233`, op `app-installations.ts:57-63`: no `options[]`, no `data[]`,
  no `validation`, no `statusCode` (200), `headers: noCacheInvalidation`, `permissions: true`,
  `query() → appInstallations.service.browse()` (`app-installations.ts:61`).
- Service: `browse()` → `this.repository.list()` (`service.ts:169-171`); lists installed apps,
  suspended included, uninstalled excluded (`service.ts:168`).
- C++ side: no route and no handler. Absence confirmed by:
  - `grep -ri "installation" include/blog/handlers` → only `include/blog/handlers/README.md:9`
    (generic docs text), no `routes.cppm` match.
  - `grep -ri "installations" include` → only `include/blog/context/context.cppm:123,227`
    (DB column list / default `"revision": "0"`), no HTTP route.
  - `include/blog/handlers/routes.cppm:4,77` imports/installs only `:apps_routes` / `AppsRoutes`.
  - `include/blog/handlers/external/apps/routes.cppm:19` installs only `Router{router, "/apps"}`.
  - `include/blog/handlers/all.cppm:100,446,695` registers only `apps_all_collection`; no
    installation handler.
- Note: C++ `context.cppm:123` knows the `app_installations` DB table, so a storage resource
  exists, but no route/handler/workflow exposes it.

### POST `/apps/installations` — MISSING

- Ghost route `admin/routes.js:234`, op `app-installations.ts:77-87`: `statusCode: 201`
  (`:78`), `headers: noCacheInvalidation` (`:79`), `permissions: true` (`:80`); no `options[]` or
  `validation` on the op — body shape enforced by `reviewed()`: `manifest_url` + `digest`
  (`app-installations.ts:27,32-40`) inside `app_installations[0]` (`:16-21`). `context` read from
  `frame.options.context` and converted via `actingContext` (`:83`).
- Service: `create(context, {manifestUrl,digest})` fetches/verifies manifest, inserts via
  `repository.insert`, records `installed` action (`service.ts:201-221`).
- C++ side: no route/handler; same search evidence as browse row. No C++ body DTO, no 201 status,
  no `{manifest_url,digest}` parsing.

### GET `/apps/installations/:id` — MISSING

- Ghost route `admin/routes.js:235`, op `app-installations.ts:65-75`: `options:['id','include']`
  (`:67`), `validation` `id` required + `include` values `['history']` (`:68`). `include:history`
  is passed as `withRelated` and mapped to `withHistory` (`:71-73`). Status 200,
  `headers: noCacheInvalidation`, `permissions: true`.
- Service: `read(id,{withHistory})` → `repository.find(id)`; throws `NotFoundError` if absent;
  appends `history` via `historyOf(id)` when requested (`service.ts:177-189,318-325`).
- C++ side: no route/handler; same search evidence as browse row. No `:id` placeholder route,
  no `include/withRelated` query parsing.

### PUT `/apps/installations/:id` — MISSING

- Ghost route `admin/routes.js:236`, op `app-installations.ts:91-103`: `options:['id']` (`:93`),
  `id` required (`:94`), body `manifest_url`+`digest`+`revision` from `reviewedChange()`
  (`:30,42-50`), `permissions: {method:'add'}` (`:95`) — distinct permission method from the
  other four ops. `headers: noCacheInvalidation` (`:92`), status 200.
- Service: `approve(context, id, {manifestUrl,digest,revision})` → `repository.find` +
  `whileUnchanged` manifest writes + `recordAction` (`service.ts:234-279,490-523`).
- C++ side: no route/handler; same search evidence as browse row.

### DELETE `/apps/installations/:id` — MISSING

- Ghost route `admin/routes.js:238`, op `app-installations.ts:105-117`: `options:['id']` (`:108`),
  `id` required (`:109`), `statusCode: 204` (`:106`), `headers: noCacheInvalidation` (`:107`),
  `permissions: true` (`:110`). Body: none.
- Service: `uninstall(context,id)` → `read(id)` + `repository.uninstall` + `recordAction`
  (`service.ts:302-316`).
- C++ side: no route/handler; same search evidence as browse row. No 204 status handling.

## Missing in C++

All Ghost routes in this domain lack a C++ counterpart:

1. GET `/apps/installations` — `admin/routes.js:233` → browse (`app-installations.ts:57-63`)
2. POST `/apps/installations` — `admin/routes.js:234` → add (`app-installations.ts:77-87`)
3. GET `/apps/installations/:id` — `admin/routes.js:235` → read (`app-installations.ts:65-75`)
4. PUT `/apps/installations/:id` — `admin/routes.js:236` → edit (`app-installations.ts:91-103`)
5. DELETE `/apps/installations/:id` — `admin/routes.js:238` → destroy (`app-installations.ts:105-117`)

Search performed to confirm absence:
- `grep "app_installations" docs/Ghost/ghost/core/server/web/api/endpoints` → no match (Ghost key
  is camelCase `appInstallations`).
- `grep "appInstallations" docs/Ghost/ghost/core/server/web` → only `admin/routes.js:233-238`.
- `grep -ri "installation" include/blog/handlers` → only `README.md:9`.
- `grep -ri "installations" include` → only `context.cppm:123,227`.
- `include/blog/handlers/routes.cppm:4,77` installs only `AppsRoutes`.
- `include/blog/handlers/all.cppm:100,446,695` registers only `apps_all_collection`.
- `include/blog/handlers/external/apps/routes.cppm:19` installs only `GET /apps`.

## Extra in C++

1. **GET `/apps`** — `include/blog/handlers/external/apps/routes.cppm:19-28` binds
   `handler.apps_all_collection()`. Handler `include/blog/handlers/external/apps/all_collection/handler.cppm:17-28`
   does a manual bearer-token check (`:18-22`), runs workflow `apps_all_collection` (`:23-25`),
   returns status 200 (`:27`). Step
   `include/blog/handlers/external/apps/all_collection/workflow/step/execute.cppm:15-16` calls
   `ctx.store().json_for("apps", {})` and emits `{"apps":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` —
   hardcoded pagination, no filtering, no `:id`, no installation data. No Ghost route `GET /apps`
   exists in `admin/routes.js` (only middleware mounts at `:228-229`) and no `apps.ts` controller
   exists under `docs/Ghost/ghost/core/server/api/endpoints/` (only `app-sessions.ts`,
   `app-installation-previews.ts`, `app-installations.ts`). Verdict: EXTRA.
