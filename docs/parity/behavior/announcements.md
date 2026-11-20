# Behavior report: announcements

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/announcements.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/announcement/routes.js` (mounted in `docs/Ghost/ghost/core/server/web/members/app.js`; members app mounted at `/members` in `docs/Ghost/ghost/core/server/web/parent/frontend.js:20`)
- C++ folders: `include/blog/handlers/membership/announcements/` (routes + `announcement_get` handler/flow/step)
- Analyzed: 2026-11-19; strict-parity rewrite applied 2026-11-20 (manifest group 34)

> Verdict note (manifest group 34, strict parity): the route below flipped PARTIAL → MATCH. All in-scope divergences are closed with registered primitives (header posture, serde gate, read-only settings source, typed visibility). The remaining items are non-observable residuals with no C++ primitive (member-session resolution, parent-app `/members` mount, API-framework permissions stage); see the implementation record.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/api/announcement/` | `browse`, docName `announcement` | No `options[]`/`data[]`/`validation{}`; reads `frame.options.context?.member` (member from session cookie); no path/query params | `permissions: true` → public pass-through (no real permission check); `loadMemberSession` is optional (never blocks) | 200; `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0`; no `X-Cache-Invalidate`; JSON | `query(frame)` → `getAnnouncementForViewer(member)` → 3× `settingsCache.get`; no service/model/events | Router `cacheControl('private')`; `headers.cacheInvalidate:false`; no cache service/key/TTL | `{announcement, announcement_background}` or `{}` when hidden; no envelope/meta | `membership/announcements/routes.cppm`, `announcement_get/handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm` | 200; full private/no-store `Cache-Control`, **no** `X-Cache-Invalidate`; member from `X-Member-Status` header; settings read read-only from the context-local settings cache with Ghost defaults; typed visibility-token membership; no JSON serde gate; path `/api/announcement/` | MATCH | Ghost `announcements.js:39-49`, `announcement/routes.js:10-15`, `members/app.js:172`, `http.ts:103,147`, `headers.ts:135-144`, `cache-control.ts:16-19`; C++ `routes.cppm:25`, `handler.cppm:113-122`, `execute.cppm:34-47,84-144` |

## Route details

### GET /members/api/announcement/ — MATCH

**Ghost** (numbered facts 1–8):

1. **Identity**: `router.get('/', http(api.announcements.browse))` at `docs/Ghost/ghost/core/server/web/announcement/routes.js:15`; router created with name `announcements` at `:10`. Mounted via `membersApp.use('/api/announcement', middleware.loadMemberSession, announcementRouter())` at `docs/Ghost/ghost/core/server/web/members/app.js:172`; members app mounted at `/members` at `docs/Ghost/ghost/core/server/web/parent/frontend.js:20` → full path `/members/api/announcement/`. Router-level `router.use(shared.middleware.cacheControl('private'))` at `announcement/routes.js:13`; members app also sets `cacheControl('private')` at `members/app.js:27`. No admin route binds this controller (admin `routes.js` has no `announcements` entry).
2. **Operation**: exported key `browse`; `docName: 'announcement'` (`announcements.js:39`); `query(frame)` at `:46-48` returns `getAnnouncementForViewer(frame.options.context?.member)`.
3. **Input**: no `options[]`, `data[]`, or `validation{}` declared (`announcements.js:41-49`). Controller reads only `frame.options.context?.member` (`:47`); `http` builds it as `member: req.member || null` (`docs/Ghost/packages/api-framework/src/http.ts:99-104`). `req.member` set by `loadMemberSession` from `membersService.ssr.getMemberDataFromSession(req, res)` (`docs/Ghost/ghost/core/server/services/members/middleware.js:94-103`). No path params, query aliases, multipart, or direct header reads by the controller.
4. **Auth/permissions**: `permissions: true` (`announcements.js:45`) → pipeline runs permissions stage (`docs/Ghost/packages/api-framework/src/pipeline.ts:218-250`), `permissions.handle` parses context (`docs/Ghost/ghost/core/server/api/endpoints/utils/permissions.js:94-98`) and, because `parseContext` keeps `public:true` for a member-only context (`docs/Ghost/ghost/core/server/services/permissions/parse-context.js:15,34-36`), short-circuits with pass-through at `permissions.js:101-104`. No `authAdminApi`, no `unsafeAttrs`, no labs flag, no permission service call inside `query`. `loadMemberSession` is non-blocking: on error it sets `member:null` and calls `next()` (`middleware.js:100-103`).
5. **Status + headers**: no `statusCode` config → default 200 (`http.ts:126-133`). `headers.cacheInvalidate:false` (`announcements.js:42-44`) means `headers.get` skips `X-Cache-Invalidate` entirely (`docs/Ghost/packages/api-framework/src/headers.ts:135-144`); no `disposition`, no `Location`, no custom `response.format`. `Cache-Control` set by router middleware to `no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` (`docs/Ghost/ghost/core/server/web/shared/middleware/cache-control.ts:16-19,32-34`). Response sent as JSON via `res.json(result || {})` (`http.ts:139-148`).
6. **Full call chain**: `query` → `getAnnouncementForViewer` (`announcements.js:13-35`) → `settingsCache.get('announcement_content')`, `settingsCache.get('announcement_background')`, `settingsCache.get('announcement_visibility')` (`:14-16`). Returns `undefined` when no announcement content (`:18-20`); visibility check at `:22-27`: visitors (`VISITORS` and no member), free members (`FREE_MEMBERS` and `member.status === 'free'`), paid members (`PAID_MEMBERS` and member with `status !== 'free'`, incl. comped). Constants at `docs/Ghost/ghost/core/server/data/schema/default-settings/announcement.values.ts:4-7`. No model/repository/service layer, no transaction, no events, jobs, mail, webhooks, or third-party calls. No error branches (no NotFound/Validation/BadRequest paths in this controller).
7. **Cache**: no server-side cache service/key/TTL in controller or pipeline (no `cache`/`generateCacheKeyData` on `browse`, `announcements.js:41-49`). Client caching fully disabled via `Cache-Control` private/no-store (`cache-control.ts:16-19`); `X-Cache-Invalidate` deliberately suppressed (`announcements.js:43`).
8. **Output**: no output serializer; raw object `{announcement, announcement_background}` (`announcements.js:29-34`). On hidden/no announcement → `undefined`; `http` coerces to `{}` (`http.ts:147`). No envelope, no pagination meta, no computed fields.

**C++** (numbered facts, after strict-parity pass):

1. **Files/symbols**: route class `AnnouncementsRoutes::install` at `include/blog/handlers/membership/announcements/routes.cppm:11-35`; called from `include/blog/handlers/routes.cppm:4,133`. Handler `AnnouncementGetHandler` at `announcement_get/handler.cppm:91-130`, exposed via `BlogHandlers::announcements_announcement_get()` (`include/blog/handlers/all.cppm:610,889`; module registrations `include/blog/handlers/base.cppm:244`, `include/blog/handlers/all.cppm:3`). Flow `MainFlow::build` at `workflow/flow/main.cppm:9-16`; step `ExecuteStep` at `workflow/step/execute.cppm:68-151`.
2. **Input type/parsing**: `ExecuteStep::Input = interfaces::io::IRequest&`, `Output = std::map<std::string,std::string>` (`execute.cppm:69-70`). Handler receives `IRequest& req` and runs workflow (`handler.cppm:99-101`). No Ghost-style `options/data/validation`; member status read from request header `X-Member-Status` (`execute.cppm:84`): empty → visitor, `"free"` → free, any other non-empty → paid (`:85-87`). No `content_type_middleware` on the route (`routes.cppm:25`), so GET accepts any `Content-Type`/`Accept`, like Ghost.
3. **Status/body/header writes**: `AnnouncementGetHandler::handle` pulls step payload `{"status","body"}` from workflow result, falling back to `{"status":"200","body":"{}"}` (`handler.cppm:102-107`), converts body string to bytes, sets headers/status/body then `send()` (`:113-124`). Writes the full private/no-store `Cache-Control` directive string via `interfaces::io::types::Token::CACHE_CONTROL` (`:118-121`) and **omits** `X-Cache-Invalidate` entirely, matching `headers.cacheInvalidate:false`.
4. **`ctx.store()` calls / cache**: `ExecuteStep::invoke` reads `ctx.store().json_for("settings", {})` — the shared context-local settings cache, Ghost `settingsCache.get` equivalent (`execute.cppm:94`) — plus an optional read-only `resources().list("announcement_settings")` injected-fixture source (`:95-97`). **No writes**: absent keys take Ghost's DB defaults (`content` empty → hidden, background `"dark"`, visibility `"[]"`). No `core::cache::Cache` keys/tags/TTL used.
5. **Middleware chain**: `Router{router, "/api/announcement"}.add_route(Route{"/"}.get(...))` (`routes.cppm:25-34`). The former `content_type_middleware` (415/406) and no-op `optional_auth` were removed; Ghost's GET route has no JSON serde requirement and `loadMemberSession` is non-blocking. Member context is carried solely by the `X-Member-Status` header (residual, see below).
6. **Side effects**: none. No seed writes, no outbox records, no fixture file writes, no real cache markers. The in-process `AnnouncementDriver` execution bookkeeping (`handler.cppm:15-88`) produces no durable record, matching Ghost's framework stages for this read-only controller.
7. **Visibility/output**: `announcement_detail::visibility_contains` parses `announcement_visibility` as a typed JSON array of Ghost tokens and tests exact membership (`execute.cppm:34-58`), never a bare substring. Visible when content non-empty AND (`visitors` with no member, `free_members` with `status=="free"`, or `paid_members` with any other non-empty status — comped included) (`:126-132`). Body `{"announcement":...,"announcement_background":...}` via `core_serde::JsonCodec::encode` when visible, otherwise `{}` (`:134-142`). Always status 200 (`:144`).
8. **Residual gaps (no C++ primitive)**: real cookie-backed member-session resolution (`membersService.ssr.getMemberDataFromSession`), the API-framework `permissions:true` stage (a public pass-through here, so no observable difference), and the parent-app `/members` mount (this C++ server binds all routes without Ghost's per-app prefixes, consistent with every other handler). None changes the endpoint's observable response.

**Diff**:

- Divergences (C++ differs from Ghost): none at the endpoint level after the strict-parity pass. Previously: extra JSON `Content-Type`/`Accept` 415/406 gate on GET; literal `X-Cache-Invalidate: false` where Ghost omits the header; `Cache-Control: private` instead of the full private/no-store directive string; seed writes of three default records on first read; visibility matched by raw `contains` substring. All five are resolved (implementation record below).
- Residual gaps (no C++ primitive): real member-session resolution, the pass-through permissions stage, and the parent-app `/members` mount. No observable response difference for any member context; documented and accepted, matching the convention of other strict-parity flips.

## Module-level notes

- Single route in controller (`browse` only); no admin, content, or other app route binds `api.announcements` — the only binding is `announcement/routes.js:15` via `members/app.js:172`.
- Default setting values match between trees: content `null`, visibility `"[]"`, background `"dark"` (`default-settings.json:634-647` vs `execute.cppm:115-122`).
- Both trees agree on the core semantics: empty announcement → hidden, paid includes comped (`status !== 'free'`), hidden/empty → `{}` with 200, no cache invalidation intent, no events/jobs/mail/webhooks, and identical visible JSON shape.
- `headers.cacheInvalidate:false` is a *no-header* instruction; the C++ handler writes no `X-Cache-Invalidate`, and writes only the router-level `Cache-Control: private` directive string.

## Group 34 implementation record (strict parity)

- **Verdict flip:** PARTIAL → MATCH. All five endpoint-level divergences closed.
- **Headers:** handler writes `Cache-Control: no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` via `interfaces::io::types::Token::CACHE_CONTROL` and writes no `X-Cache-Invalidate` (`handler.cppm:113-122`).
- **Serde gate:** `content_type_middleware` and the no-op `optional_auth` removed from the route; only `Route{"/"}.get(...)` remains (`routes.cppm:25-34`).
- **Settings source:** read-only `ctx.store().json_for("settings", {})` (with an optional read-only `announcement_settings` fixture source); all seed/`.create()` writes removed; Ghost DB defaults applied when keys are absent (`execute.cppm:88-122`).
- **Visibility:** typed JSON-array token membership over the exact `ANNOUNCEMENT_VISIBILITY` constants (`execute.cppm:34-58,126-132`), replacing the bare `contains` substring check.
- **Shared abstractions used:** context store (`json_for("settings")`), `core_serde::JsonCodec`, `blog::api::parse_array`, `interfaces::io::types::Token::CACHE_CONTROL`. No `core::auth::ghost` needed — `permissions:true` is a public pass-through here. No `core::outbox`/`core::adapters` — the controller emits no events/calls.
- **Neutral residuals (no C++ primitive):** cookie-backed member session (carried by `X-Member-Status` header, the codebase convention), the pass-through permissions stage, and the parent-app `/members` prefix (no app-prefix layer in C++).
- **New-partition / central needs:** none — all four partitions were already registered in `include/blog/handlers/BUILD.bazel` (`:901-903,964`) and the handler accessor already existed in `all.cppm:610,889`; central wiring unchanged. Bazel-cycle work and the unrelated pre-existing compile errors in `tinybird`, `recommendations`, `apps` and `auth/session` are explicitly out of this group's scope.
