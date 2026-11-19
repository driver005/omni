# Parity: app-sessions

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-sessions.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: none found (no C++ route or handler for `POST /apps/installations/:id/sessions`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 1 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST /apps/installations/:id/sessions | `appSessions.start` (docName `app_sessions`) | none | none | Ghost: `options: ['id']`, `validation.options.id.required=true`, no `data[]`; C++: n/a | Ghost: returns `{ installation }` (AppSession), default 200, `headers.cacheInvalidate=false`; C++: n/a | Ghost: `appInstallations.service.startSession(id)` → `recheck(id)` + `read(id)` + repository; no C++ store call | Ghost: `cacheInvalidate: false` (suppresses X-Cache-Invalidate); no cache service. C++: none | Ghost: `mw.authAdminApi` on `/apps/*`, permissions `{docName:'app_installations', method:'read'}`, labs `apps` middleware; C++: none | MISSING | Ghost `admin/routes.js:237` + `:229`, controller `app-sessions.ts:14-21`, `index.js:359-361`, `service.ts:286-294`; C++ absence: grep `installations`/`app_session`/`startSession` in `include/blog/handlers` = 0 matches; `sessions` = only `members/.../members_id_sessions_delete` |

## Route details

### POST /apps/installations/:id/sessions — MISSING

- **Route identity**: Ghost binds `appsRouter.post('/installations/:id/sessions', http(api.appSessions.start))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:237`, mounted under `/apps` at `:229` (`router.use('/apps', appsRouter)`). Full path: `POST /apps/installations/:id/sessions`. No C++ `Route{...}` with path `/apps/installations/:id/sessions` and verb POST exists in any `routes.cppm` (see search evidence below).
- **Operation mapping**: Ghost key `start` (`app-sessions.ts:14`) on docName `app_sessions` (`:7`), exposed as `api.appSessions` (`.../api/endpoints/index.js:359-361`). No C++ `handler.<method>()` bound to this path/verb.
- **Input parity**: Ghost `options: ['id']` + `validation: { options: { id: { required: true } } }` (`app-sessions.ts:16-17`), no `data[]`. C++: no Input / DTO / query parsing for this route (none exists).
- **Output parity**: Ghost `query()` returns `appInstallations.service.startSession(frame.options.id)` (`app-sessions.ts:19-21`); service returns `{ installation }` where session state is not stored (`.../services/app-installations/service.ts:286-294`). `statusCode` not set (default 200). `headers: { cacheInvalidate: false }` (`:15`). C++: no output envelope, no serde DTO, no `response.set_status`/`set_body` — no counterpart.
- **DB call parity**: Ghost service `startSession` calls `recheck(id)` then `read(id)` via the app-installations repository (`service.ts:287-288`). No C++ `ctx.store().json_for(...)` / `find_by_resource` / `mutations.*` call exists for this route.
- **Cache parity**: Ghost `headers.cacheInvalidate=false` suppresses the invalidate header; no cache service call. C++: none (no `core::cache::Cache` key/namespace, TTL, or `ResourceStore` tag for this route).
- **Auth / permissions**: Ghost `router.all(['/apps','/apps/*'], mw.authAdminApi)` at `admin/routes.js:228`, plus `appsRouter.use(labs.enabledMiddleware('apps'))` at `:230`, and controller permissions `{ docName: 'app_installations', method: 'read' }` (`app-sessions.ts:18`). C++: no middleware/auth chain for this route.
- **Verdict**: MISSING — Ghost route has no C++ route or handler.

## Missing in C++

Every Ghost route in this controller (all 1) has no C++ counterpart:

- `POST /apps/installations/:id/sessions` → `api.appSessions.start` (`admin/routes.js:237`; controller `app-sessions.ts:14-21`).

Search performed to confirm absence (all in `/home/default/cc/omni`):

- grep `installations` in `include/blog/handlers/**` → 0 matches.
- grep `app_session|appSession|startSession|start_session` in `include/blog/handlers/**` → 0 matches.
- grep `sessions` in `include/blog/handlers/**` → only `membership/members/members_id_sessions_delete` (`include/blog/handlers/membership/members/routes.cppm:97-105`, `DELETE /members/:id/sessions`) — a different route (members logout), not app sessions.
- grep `"/apps` in `include/blog/handlers/**` → only `include/blog/handlers/external/apps/routes.cppm:19` (`Router{router, "/apps"}`) with a single `GET /apps` route (`:21-28`) bound to `apps_all_collection`, whose step reads `ctx.store().json_for("apps", {})` (`external/apps/all_collection/workflow/step/execute.cppm:15-16`). This is the external apps catalog list, not `POST /apps/installations/:id/sessions`.
- Central install list `include/blog/handlers/routes.cppm:4,77` installs only `AppsRoutes` (the GET catalog above); no app-session/installation routes are registered.

## Extra in C++

None. No C++ folder matched this domain (no C++ route for app installations or app sessions), so there are no matched-folder C++ routes without a Ghost counterpart.

Note (out of scope, not counted): the C++ `GET /apps` catch-all in `include/blog/handlers/external/apps/routes.cppm:19-28` belongs to the external apps-catalog feature (different Ghost domain), not to app-sessions, and is therefore not recorded as EXTRA here.
