# Parity: announcements

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/announcements.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/announcement/routes.js` (mounted by `docs/Ghost/ghost/core/server/web/members/app.js`)
- C++: none found
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
| GET `/api/announcement/` | `api.announcements.browse` (controller `announcements.js`, `docName: 'announcement'`, op `browse`) | none | none | Ghost: no `options[]`/`data[]`/`validation`; reads `frame.options.context?.member` from member session. C++: none | Ghost: raw `{announcement, announcement_background}` or `undefined`, default 200, no serializer/meta. C++: none | Ghost: settings-cache reads only (`announcement_content`, `announcement_background`, `announcement_visibility`); no model/service call. C++: none | Ghost: `headers.cacheInvalidate: false` + route middleware `cacheControl('private')`. C++: none | Ghost: `permissions: true`; member session via `middleware.loadMemberSession`. C++: none | MISSING | Ghost `web/announcement/routes.js:15`, `web/members/app.js:172`, `api/endpoints/announcements.js:38-49`; C++ absence: no `announcements_routes` in `include/blog/handlers/routes.cppm:3-56`, no announcement handler in `include/blog/handlers/all.cppm`, greps below |

## Route details

### GET `/api/announcement/` — MISSING

- Route identity: `router.get('/', http(api.announcements.browse))` at `docs/Ghost/ghost/core/server/web/announcement/routes.js:15`, mounted at `/api/announcement` via `membersApp.use('/api/announcement', middleware.loadMemberSession, announcementRouter())` at `docs/Ghost/ghost/core/server/web/members/app.js:172`. Effective path `GET /api/announcement/`. No C++ `Router/Route` for this path/verb anywhere in `include/blog/handlers/**/routes.cppm`.
- Operation mapping: Ghost key `announcements.browse` (`docs/Ghost/ghost/core/server/web/announcement/routes.js:15`) resolves through getter `get announcements()` → `apiFramework.pipeline(require('./announcements'), localUtils)` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:107-109`, to the `browse` operation of `docs/Ghost/ghost/core/server/api/endpoints/announcements.js:41-49` (`docName: 'announcement'` at line 39). No C++ handler method.
- Input parity: controller declares no `options[]`, `data[]`, or `validation{}`; the only input is `frame.options.context?.member` from the member session (`announcements.js:47`). C++ side has no input parsing for this route.
- Output parity: `query()` returns `getAnnouncementForViewer(...)` (`announcements.js:46-48`), which yields a plain object `{announcement, announcement_background}` or `undefined` (`announcements.js:29-34`); no serializer, no `{data, meta}` envelope, no explicit `statusCode`/`headers` in the controller (default 200). C++ side has no output envelope for this route.
- DB call parity: no `models.*` / service call — data comes from `settingsCache.get('announcement_content' | 'announcement_background' | 'announcement_visibility')` at `announcements.js:14-16`. Visibility uses `ANNOUNCEMENT_VISIBILITY` values `visitors` / `free_members` / `paid_members` (`data/schema/default-settings/announcement.values.ts:4-10`); paid = any non-free status, incl. comped (`announcements.js:8,24-27`). C++ side has no `store().json_for`, `find_by_resource`, or cache read for this route.
- Cache parity: Ghost disables cache invalidation (`headers: { cacheInvalidate: false }` at `announcements.js:42-44`) and marks the response private at the router (`shared.middleware.cacheControl('private')` at `web/announcement/routes.js:13`, with comment at line 12 noting it depends on member context). C++ side: no `core::cache::Cache` key/namespace/TTL and no `ResourceStore` tag for this route.
- Auth / permissions: Ghost `permissions: true` on the operation (`announcements.js:45`) plus member session loading and private cache control in the web layer (`web/members/app.js:172`, `web/announcement/routes.js:13`). C++ side has no auth or middleware chain for this route.
- Absence evidence: `grep api.announcement` and `grep announcement` across `docs/Ghost/ghost/core/server/web/api/endpoints` → no matches (route lives in the dedicated `web/announcement` app). `grep announcement` in `include/blog/handlers` → no matches. Case-insensitive `grep announce` in `include/blog` → 9 matches, all the unrelated member field `enable_updates_and_announcements` (`context/context.cppm:156`, `model/member.cppm:41-42,127,194,224`, `testing/ghost_schema.cppm:200,568`, `dto/fixtures.cppm:151`). `grep '/api/announcement|announcements?_routes|AnnouncementHandler|getAnnouncement'` in `include/blog` → no matches. `include/blog/handlers/routes.cppm:3-56` exports no `announcements_routes` module and `:install()` at lines 76-129 installs no announcement routes. `include/blog/handlers/all.cppm` imports/declares no announcement handler. `glob **/announcement*` in `include/blog/handlers` → no files.

## Missing in C++

- `GET /api/announcement/` — Ghost `api.announcements.browse` (`docs/Ghost/ghost/core/server/web/announcement/routes.js:15`; controller `docs/Ghost/ghost/core/server/api/endpoints/announcements.js:41-49`). No C++ route, handler, cache entry, or middleware chain exists. This is a members-app/content-facing endpoint keyed off member context; its absence in C++ is confirmed by the greps and module-list checks recorded above.

## Extra in C++

None. No C++ route in the scanned handler folders lacks a Ghost counterpart in this controller (there are no announcement C++ routes at all).
