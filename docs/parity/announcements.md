# Parity: announcements

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/announcements.js`
- Ghost routes: `docs/Ghost/ghost/core/web/announcement/routes.js` (mounted by `docs/Ghost/ghost/core/server/web/members/app.js`)
- C++: `include/blog/handlers/membership/announcements/` (`routes.cppm` + `announcement_get/{handler,workflow/flow/main,workflow/step/execute}.cppm`)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (manifest group 34). Supersedes the earlier MISSING state below — implementation has landed and the verdict is MATCH.

> Verdict note (group 34): the route flipped MISSING → (implemented) PARTIAL → MATCH. The `#### Fix` blocks below are retained for provenance but now describe the implemented strict-parity state; see the authority report `docs/parity/behavior/announcements.md` for the full route details + Diff.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/api/announcement/` (C++ binds `/api/announcement/`) | `api.announcements.browse` (controller `announcements.js`, `docName: 'announcement'`, op `browse`) | `AnnouncementsRoutes::install` → `Router{"/api/announcement"}` `Route{"/"}.get` (`routes.cppm:25-34`) | `announcements_announcement_get` (`announcement_get/handler.cppm:91-130`) | Ghost: no `options[]`/`data[]`/`validation`; reads `frame.options.context?.member`. C++: member status from `X-Member-Status` header, missing → visitor; no path/query params | Ghost: raw `{announcement, announcement_background}` or `undefined`, default 200, no serializer/meta. C++: identical shape/`{}`, status 200 | Ghost: settings-cache reads only (`announcement_content`, `announcement_background`, `announcement_visibility`); no model/service call. C++: read-only `ctx.store().json_for("settings")` + optional read-only fixture; no writes | Ghost: `headers.cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` + router `cacheControl('private')`. C++: no `X-Cache-Invalidate`; full private/no-store `Cache-Control` directive | Ghost: `permissions:true` (public pass-through) + `middleware.loadMemberSession`. C++: no serde/auth middleware; header carries status; pass-through needs no primitive | MATCH | Ghost `web/announcement/routes.js:15`, `web/members/app.js:172`, `api/endpoints/announcements.js:38-49`; C++ `membership/announcements/routes.cppm:25`, `announcement_get/handler.cppm:113-122`, `workflow/step/execute.cppm:84-144` |

## Route details

### GET `/api/announcement/` — MATCH

- Route identity: C++ `Router{router, "/api/announcement"}.add_route(Route{"/"}.get(...))` at `routes.cppm:25-34`, dispatching `handler.announcements_announcement_get().handle(...)`. Installed by `AnnouncementsRoutes{}.install(router, handlers)` at `include/blog/handlers/routes.cppm:133` (import at `:4`). Ghost's full path is `/members/api/announcement/`; the parent-app `/members` mount is not replicated by this C++ server (all routes are bound without Ghost per-app prefixes) — residual, no observable response difference.
- Operation mapping: Ghost key `announcements.browse` → `browse` op of `announcements.js:41-49`. C++ `announcements_announcement_get` handler/flow/step trio (`workflow name "announcements_announcement_get"`, step name `"announcements_announcement_get_execute"`).
- Input parity: controller declares no options/data/validation; only input is `frame.options.context?.member` (`announcements.js:47`). C++ reads `X-Member-Status` header (empty→visitor, `free`→free, other→paid incl. comped) at `execute.cppm:84-87`; no JSON `Content-Type`/`Accept` gate on the route.
- Output parity: `{announcement, announcement_background}` or `{}` (`announcements.js:29-34`), status 200, no serializer/envelope. C++ builds the same raw object with `core_serde::JsonCodec::encode` or `{}` (`execute.cppm:134-144`); handler sets status/body only (`handler.cppm:122-124`).
- DB call parity: no model/service call — three `settingsCache.get(...)` reads (`announcements.js:14-16`). C++ reads the shared context-local settings cache via `ctx.store().json_for("settings", {})` (`execute.cppm:94`) plus an optional read-only `announcement_settings` fixture source (`:95-97`); no writes, Ghost DB defaults applied when keys absent (`:115-122`). Visibility uses exact `ANNOUNCEMENT_VISIBILITY` token membership (`execute.cppm:34-58,126-132`); paid = any non-free status, incl. comped.
- Cache parity: Ghost suppresses `X-Cache-Invalidate` (`headers.cacheInvalidate:false`, `announcements.js:42-44`) and sets `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (router `cacheControl('private')`). C++ writes no `X-Cache-Invalidate` and that full directive string via `interfaces::io::types::Token::CACHE_CONTROL` (`handler.cppm:113-121`); no `core::cache::Cache` key/TTL.
- Auth / permissions: Ghost `permissions:true` (`announcements.js:45`) is a public pass-through for a member-only context (`parse-context.js:15,34-36`, `permissions.js:101-104`); `loadMemberSession` only loads optional member state. C++ has no serde/auth middleware; header carries status; the pass-through needs no C++ primitive (residual, no observable difference).
- Absence evidence (updated): `grep` now finds `/api/announcement` GET at `routes.cppm:25` and the `announcements_announcement_get` handler at `all.cppm:610,889`; the four partitions are registered in `BUILD.bazel:901-903,964`. The MISSING row's absence evidence is historical and superseded.

#### Fix (applied — strict parity, group 34)

1. **Route.** `AnnouncementsRoutes::install` binds `Router{router, "/api/announcement"}.add_route(Route{"/"}.get(... handler.announcements_announcement_get().handle(...) ...))` (`routes.cppm:25-34`). Ghost's effective path is `/members/api/announcement/`; the C++ server has no per-app mount layer, so `/api/announcement` is bound verbatim (parent `/members` prefix recorded as residual). `content_type_middleware` and the no-op `optional_auth` were removed — Ghost's GET route has no JSON serde gate and `loadMemberSession` never blocks.
2. **Operation.** Reuses the implemented `announcements_announcement_get` trio (`handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`); no new handler needed.
3. **Input + DB call.** No Ghost options/data/validation. Member status from `X-Member-Status` (`free`/paid incl. comped/empty→visitor). Settings read read-only from `ctx.store().json_for("settings", {})` (with an optional read-only `announcement_settings` fixture source); all seed `.create()` writes removed; Ghost DB defaults (content empty, background `dark`, visibility `[]`) applied when keys absent.
4. **Output.** Exact visibility-token membership (`visitors`/`free_members`/`paid_members`); raw `{"announcement":…,"announcement_background":…}` or `{}`, status 200, no `{data,meta}` envelope.
5. **Cache.** No `X-Cache-Invalidate` header (the `cacheInvalidate:false` no-header instruction); full private/no-store `Cache-Control` directive string. No `core::cache::Cache` key/TTL.
6. **Auth.** `permissions:true` resolves to public pass-through; `loadMemberSession` has no C++ primitive (member status carried by header). Both are non-observable residuals.
7. **Evidence.** Re-run greps: `/api/announcement` GET and `announcements_announcement_get` now exist; all checks 1–8 closed except accepted residuals (member session, permissions stage, parent mount).

## Missing in C++

None. The formerly missing `GET /api/announcement/` is implemented by `AnnouncementsRoutes::install` + the `announcements_announcement_get` trio. Checks 1–8 closed with the residuals documented above.

## Extra in C++

None. The folder installs exactly one route (`GET /api/announcement/`), matching the controller's single `browse` op.

## Needs central registration

None outstanding. Implementation landed entirely in the four owned files, and all shared wiring was already present before this pass:

1. `include/blog/handlers/all.cppm` — import `:announcements_announcement_get_handler` (`:3`), accessor `announcements_announcement_get()` (`:610`), member `m_announcements_announcement_get{m_ctx}` (`:889`). Already present.
2. `include/blog/handlers/base.cppm` — export `:announcements_announcement_get_handler` (`:244`). Already present.
3. `include/blog/handlers/routes.cppm` — `export import :announcements_routes` (`:4`) and `AnnouncementsRoutes{}.install(router, handlers)` (`:133`). Already present.
4. `include/blog/handlers/BUILD.bazel` — partitions `membership/announcements/announcement_get/{workflow/step/execute,workflow/flow/main,handler}.cppm` (`:901-903`) and `membership/announcements/routes.cppm` (`:964`). Already present.

No new partitions or shared-helper registrations are required. Bazel-cycle remediation and the unrelated pre-existing compile errors in `tinybird`, `recommendations`, `apps`, and `auth/session` are explicitly out of this group's scope.
