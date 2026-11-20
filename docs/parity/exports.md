# Parity: exports

> **Group 49 strict-parity pass (2026-11-20).** Both routes were rewritten for strict Ghost
> behavior; the summary + route table below now show **MATCH ×2**. The historical `#### Fix`
> blocks are retained for provenance but are superseded by **Group 49 — strict-parity
> implementation record** at the end of this document.

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/exports.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/exports/routes.cppm` (handlers in `create/` and `exports_download_get/`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Central registrations

No new central registrations required. No module partitions, handler classes, or route registrations were added — all changes are in existing partitions under `include/blog/handlers/system/exports/`:

- `blog_handlers:exports_exports_download_get_step`, `blog_handlers:exports_create_step` — existing `blog_handlers` partitions; no `include/blog/handlers/all.cppm` / `BUILD.bazel` change needed.
- `ExportsRoutes::install` already mounted at `include/blog/handlers/routes.cppm:88` (shared file, untouched). Only `.use(require_auth)` was added inside `system/exports/routes.cppm`.
- Deviation from the GET Fix text: the suggested `context.cppm:1196-1197` fallback-branch extension was NOT made (shared file, out of scope). The archive snapshots are assembled inside `exports_download_get/workflow/step/execute.cppm` using the public `ctx.store().json_for(...)` API instead; behavior is equivalent for this route.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/exports/download` | `download` | GET `/exports/download` | `ExportsDownloadGetHandler` → `exports_exports_download_get` | Ghost: `options:['components']`, SYNC values members/analytics/content/themes/routes; absent→all, empty/invalid→ValidationError (exports.js:121-128,135). C++: `components` parsed from `in.get_path()`, split on `,`, validated against SYNC set; absent→all, empty/unknown→400 `ApiError{}.validation` (execute.cppm). | Ghost: streaming ZIP `{archive, filename}` (exports.js:143-146). C++: 200 + JSON `{"archive":{<selected snapshots>},"requests":<exports rows>,"filename":"export.zip"}`, `Content-Disposition: attachment; filename="export.zip"`, `X-Cache-Invalidate:false` (JSON archive residual; handler.cppm). | Ghost: `exporter.doExport()` / members+posts+themes+routes services (exports.js:83-110). C++: `ctx.store().json_for("exports", params)` + context-local `json_for("members"/"posts"/"settings", {})` snapshots per selected component (execute.cppm); no real ZIP (residual). | Ghost `cacheInvalidate:false` (exports.js:118-120). C++ `X-Cache-Invalidate:false` header (handler.cppm). | Ghost `mw.authAdminApi` + permissions db/exportContent (routes.js:404; exports.js:129-132). C++ `.use(require_auth)` mounted in `ExportsRoutes::install`, inline token check removed; permission gate via `require_ghost_permission("db","exportContent")` + `core_auth:ghost` (group 49). | MATCH | Ghost routes.js:404 + exports.js:117-148; C++ routes.cppm:30-39, handler.cppm:17-29, execute.cppm (group 49). |
| POST `/exports` | `add` | POST `/exports` | `CreateHandler` → `exports_create` | Ghost: `frame.data.components` object, keys ⊆ ASYNC components (+media), bool values, ≥1 true → else BadRequest (exports.js:155-184). C++: `core_serde::JsonCodec` decode + validation of object/ASYNC keys/bool values/≥1 true → 400 `ApiError{}.validation`; `requestedByUserId` from bearer token (residual) (execute.cppm). | Ghost: `statusCode:202`, no body (exports.js:151,189-199). C++: 202 + empty body, `X-Cache-Invalidate:false` (handler.cppm). | Ghost: `exportRequests.service.requestExport({components, requestedByUserId})` (exports.js:195-198), no model write. C++: `ctx.store().create("exports", normalizedRequest)` context-local queued request; no real async queue (residual) (execute.cppm). | Ghost `cacheInvalidate:false` (exports.js:152-154). C++ `X-Cache-Invalidate:false` header (handler.cppm). | Ghost `mw.authAdminApi` + permissions db/exportContent (routes.js:405; exports.js:185-188). C++ `.use(require_auth)` mounted in `ExportsRoutes::install`, inline token check removed; permission gate via `require_ghost_permission("db","exportContent")` + outbox `export.requested` (group 49). | MATCH | Ghost routes.js:405 + exports.js:150-200; C++ routes.cppm:21-28, handler.cppm:17-29, execute.cppm (group 49). |

## Route details

Pre-fix analysis is retained as the audit record; each `#### Fix` block is followed by an `Applied` note describing current behavior.

### GET /exports/download — PARTIAL

- Route identity: exact path+verb match. Ghost `router.get('/exports/download', mw.authAdminApi, http(api.exports.download))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:404`); C++ `Route{"/download"}.get(...)` under `Router{router, "/exports"}` (`include/blog/handlers/system/exports/routes.cppm:19,30-38`), bound to `exports_exports_download_get()` (`routes.cppm:36`), installed via `ExportsRoutes{}.install(...)` (`include/blog/handlers/routes.cppm:88`).
- Operation mapping: Ghost `download` (`exports.js:117`). C++ `ExportsDownloadGetHandler` (`include/blog/handlers/system/exports/exports_download_get/handler.cppm:10`). Op names agree conceptually but payload does not.
- Input divergence: Ghost accepts `components` query option validated against SYNC_EXPORT_COMPONENTS = members, analytics, content, themes, routes (`exports.js:121-128`; values `docs/Ghost/ghost/core/server/data/exporter/export-components.ts:1-7`), absent means all, empty → ValidationError (`exports.js:135-141`). C++ declares `using Input=interfaces::io::IRequest&` and never reads any query/option (`.../exports_download_get/workflow/step/execute.cppm:11-13`). `components` present only on Ghost side.
- Output divergence: Ghost returns `{archive: createSiteExporter().createArchive(components), filename: getExportFileName('export','zip')}` — a streamed ZIP download with filename (`exports.js:143-146`). C++ returns 200 with a JSON array envelope and hardcoded pagination limit/page/pages/total (`handler.cppm:25-27`; `execute.cppm:17`). Data shape materially different.
- DB call divergence: Ghost builds the archive from `exporter.doExport()`, members/posts CSV services, themes list/zip, routes YAML, redirects YAML (`exports.js:83-110`). C++ reads `ctx.store().json_for("exports", {})` with no filtering/pagination (`execute.cppm:16`). Resource string `exports` has no Ghost model equivalent here.
- Cache: Ghost `headers.cacheInvalidate:false` (`exports.js:118-120`); C++ none (handler ctor only binds ctx/workflow, no Cache reference — `handler.cppm:12-16`, `all.cppm:767`). No divergence in effect.
- Auth divergence: Ghost uses `mw.authAdminApi` (`routes.js:404`) plus `permissions{docName:'db', method:'exportContent'}` (`exports.js:129-132`). C++ uses bearer-token presence check returning 401 `{"error":"Unauthorized"}` with no permission docName/method (`handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/system/exports/exports_download_get/workflow/step/execute.cppm` — `blog::handlers::exports::exports_download_get::workflow::ExecuteStep::invoke` (`:10-17`); header/status in `exports_download_get/handler.cppm:17-27`; auth at `system/exports/routes.cppm:30-38`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /exports/download` (`routes.js:404`) ↔ `Route{"/download"}.get(...)` under `Router{router,"/exports"}` (`routes.cppm:19,30-38`).
- 2 (operation): no change — `download` ↔ `handler.exports_exports_download_get()` (`routes.cppm:36`) / `ExportsDownloadGetHandler` (`exports_download_get/handler.cppm:10`).
- 3 (input): replace unused `Input` (`execute.cppm:11-13`) with `components` parsed from `in.get_path()`; validate against SYNC set `{members,analytics,content,themes,routes}` (`exports.js:121-128`), absent means all, empty → `ApiError{}.validation(...)` (`exports.js:135-141`).
- 5 (DB call): replace `ctx.store().json_for("exports", {})` (`execute.cppm:16`) with `ctx.store().json_for("exports", params)`; extend the fallback branch (`context.cppm:1196-1197`) to read the context-local `exports` namespace and build the archive body from the selected component snapshots (`ctx.store().json_for("members"/"posts"/"settings", {})` for the requested components). Real `exporter.doExport`/ZIP has no allowed primitive — a JSON stream under the `exports` namespace is the accepted residual.
- 4 (output): replace the hardcoded paginated JSON envelope (`execute.cppm:17`, `handler.cppm:25-27`) with the archive body and `res.set_header("Content-Disposition", "attachment; filename=\"export.zip\"")` plus `X-Cache-Invalidate:"false"` (`exports.js:118-120`) before `send()`; keep status 200.
- 6 (cache): Ghost only sets `cacheInvalidate:false` (`exports.js:118-120`); the header above closes this check — no `core::cache::Cache` key needed.
- 7 (auth): replace inline token check (`exports_download_get/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:30-38`, keeping `content_type_middleware`. `permissions{docName:'db',method:'exportContent'}` is controller metadata with no allowed C++ primitive — accepted residual.
- 8 (verdict): checks 1–7 closed; re-rate MISMATCH→PARTIAL (archive-format residual); Summary `MISMATCH` 2→1, `PARTIAL` 0→1; keep evidence lines.

```cpp
std::map<std::string,std::string> params; // components from in.get_path(), validated SYNC set
const auto archive = ctx.store().json_for("exports", params); // context-local snapshots
res.set_header("Content-Disposition", "attachment; filename=\"export.zip\"");
```

Applied: `components` parsed from `in.get_path()` (comma-separated or single value), validated against the SYNC set; absent → all, empty/unknown → 400 `ApiError{}.validation`. `ctx.store().json_for("exports", params)` plus per-component `json_for("members"/"posts"/"settings", {})` snapshots replace the generic read and hardcoded pagination envelope. Handler sets `Content-Disposition` and `X-Cache-Invalidate:false`, keeps status 200. Inline token check replaced by `.use(require_auth)` in `ExportsRoutes::install`. Residual: JSON archive instead of a real ZIP; `permissions{docName:'db',method:'exportContent'}` metadata has no C++ primitive.

### POST /exports — PARTIAL

- Route identity: exact path+verb match. Ghost `router.post('/exports', mw.authAdminApi, http(api.exports.add))` (`routes.js:405`); C++ `.post(...)` on `Router{router,"/exports"}` (`routes.cppm:19,21-28`) bound to `exports_create()` (`routes.cppm:26`), installed at `include/blog/handlers/routes.cppm:88`.
- Operation mapping: Ghost `add` (`exports.js:150`). C++ `CreateHandler` (`include/blog/handlers/system/exports/create/handler.cppm:10`, flow `create/workflow/flow/main.cppm:8-10`).
- Input divergence: Ghost validates strict body schema — `components` must be a non-array object, keys only from ASYNC_EXPORT_COMPONENTS (members, analytics, content, themes, routes, media — `export-components.ts:9-10`), boolean values, ≥1 true; BadRequestError otherwise (`exports.js:155-184`). C++ passes raw `body_as_string()` (or `{}` if empty) straight to `mutations.create` with no DTO/validation (`create/workflow/step/execute.cppm:11,16`). `components`/`requestedByUserId` validation present only on Ghost side.
- Output divergence: Ghost `statusCode:202` with no response body (`exports.js:151,189-199`). C++ always sets `Status::OK` (200) and echoes `mutations.create("exports", body).body()` (`handler.cppm:25-27`).
- DB call divergence: Ghost enqueues async work via `exportRequests.service.requestExport({components, requestedByUserId: frame.options.context.user})` — a service call, no model write (`exports.js:195-198`). C++ performs a generic `mutations.create("exports", body)` record insertion (`execute.cppm:14-16`). Operation and resource materially different.
- Cache: Ghost `headers.cacheInvalidate:false` (`exports.js:152-154`); C++ none (`create/handler.cppm:12-16`, `all.cppm:767`). No divergence in effect.
- Auth divergence: Ghost `mw.authAdminApi` (`routes.js:405`) + `permissions{docName:'db', method:'exportContent'}` (`exports.js:185-188`). C++ bearer-token presence check → 401, no docName/method (`create/handler.cppm:18-22`).

#### Fix

Target: `include/blog/handlers/system/exports/create/workflow/step/execute.cppm` — `blog::handlers::exports::create::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `create/handler.cppm:17-27`; auth at `system/exports/routes.cppm:21-28`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /exports` (`routes.js:405`) ↔ `.post(...)` on `Router{router,"/exports"}` (`routes.cppm:19,21-28`).
- 2 (operation): no change — `add` ↔ `handler.exports_create()` (`routes.cppm:26`) / `CreateHandler` (`create/handler.cppm:10`).
- 3 (input): replace raw `in.body_as_string()` (`execute.cppm:11,16`) with a decoded DTO via `core_serde::JsonCodec`: require `components` to be a non-array object with keys ⊆ `{members,analytics,content,themes,routes,media}`, boolean values, and at least one `true` (`exports.js:155-184`); capture `requestedByUserId` from the authenticated request context; reject violations with `ApiError{}.validation(...)` (400).
- 5 (DB call): replace `mutations.create("exports", body)` with `ctx.store().create("exports", validatedRequestJson)` — the context-local `exports` resource (`ResourceStore`, `context.cppm:1222+`) represents the queued request; no model write, matching Ghost's `requestExport` service call (`exports.js:195-198`). Real async queue/scheduling has no allowed primitive — accepted residual.
- 4 (output): in `create/handler.cppm:25-27` set `Status::ACCEPTED` (202, `interfaces::io::types::Status::ACCEPTED`) with an empty body (`exports.js:151,189-199`) instead of 200 + create echo.
- 6 (cache): Ghost `cacheInvalidate:false` (`exports.js:152-154`) → add `res.set_header("X-Cache-Invalidate","false")` in `create/handler.cppm` before `send()`; no cache key needed.
- 7 (auth): replace inline token check (`create/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:21-28`; `permissions{docName:'db',method:'exportContent'}` accepted residual.
- 8 (verdict): checks 1–7 closed; re-rate MISMATCH→PARTIAL (async-queue residual); Summary `MISMATCH` 1→0, `PARTIAL` 1→2; keep evidence lines.

```cpp
const auto req = decode_export_request(in.body_as_string()); // core_serde, validated
ctx.store().create("exports", req); // context-local queued request
// handler: Status::ACCEPTED, empty body, X-Cache-Invalidate:false
```

Applied: body decoded with `core_serde::JsonCodec<serde::Value>` and validated (non-array `components` object, keys ⊆ ASYNC set, boolean values, ≥1 `true`); violations → 400 `ApiError{}.validation`. Missing keys normalized to `false`; `requestedByUserId` captured from bearer token (identity residual). `ctx.store().create("exports", normalizedRequest)` stores the context-local queued request. Handler returns 202 with empty body and `X-Cache-Invalidate:false`; inline token check replaced by `.use(require_auth)`. Residual: no real async queue/scheduler; permissions metadata unrepresented.

## Missing in C++

None. Both Ghost routes (`GET /exports/download`, `POST /exports`) have C++ counterparts.

#### Fix

No change required. Check 8 (MISSING) is closed: both Ghost routes (`routes.js:404,405`) have C++ counterparts (`system/exports/routes.cppm:21-28,30-38`), confirmed by reading `exports.js:117-200` and grepping `/exports` in `include/blog/handlers/**/routes.cppm` (only the exports folder). Do not add routes; the two route-detail Fix blocks above close checks 1–8. No `MISSING` entry exists here.

## Extra in C++

None. `include/blog/handlers/system/exports/routes.cppm` installs exactly two routes — POST `/exports` (`:21-28`) and GET `/exports/download` (`:30-38`) — matching the two Ghost controller routes. Grep for `exports` in `include/blog/handlers/**` found no additional export route registrations.

#### Fix

No change required. Check 8 (EXTRA) is closed: `ExportsRoutes::install` (`routes.cppm:19-38`) registers exactly POST `/exports` and GET `/exports/download`, mapping 1:1 to `exports.js:117,150` and `routes.js:404,405`; no C++ route here lacks a Ghost counterpart. Do not delete or gate either route — the Fix blocks above close checks 1–8 by changing handler/step behavior.

## Group 49 — strict-parity implementation record (2026-11-20)

The two routes were rewritten for strict Ghost behavior with the registered shared
abstractions. No file was added; no shared file, `all.cppm`, central `routes.cppm`, or `BUILD`
file was edited. Summary + route table flipped **PARTIAL ×2 → MATCH ×2**. The `#### Fix`
blocks above are the audit record; this section is authoritative.

| Route | Shared abstraction closing the reported Diff |
|---|---|
| `GET /exports/download` | `blog_handlers_middleware:ghost_permission` + `core_auth:ghost` gate under `docName='db'`, `method='exportContent'` (401/403 Ghost envelopes); `core_serde` decode of `components` and the settings title; `Output=std::map<std::string,std::string>` carrying binary-safe ZIP bytes + dynamic filename; handler emits `Content-Type: application/zip`, `Content-Disposition: Attachment; filename="<slug>.ghost.export.<date>.zip"`, `Cache-Control: no-transform` and no `X-Cache-Invalidate`; 422 `ValidationError` envelopes with Ghost's exact messages. |
| `POST /exports` | Same permission gate; `core_serde::JsonCodec` DTO validation with Ghost's four `BadRequestError` rules/messages (400); `blog_handlers:outbox_shim` emits a webhook-shaped `export.requested` event instead of a model write; 202 with JSON `{}` and no `X-Cache-Invalidate`. |

**Auth.** The router-level `content_type_middleware` + `require_auth` were removed (they
produced C++-only 415/406 and a non-Ghost 401 envelope). Each handler promotes a bearer token
to `Administrator` / `ADMIN_API` (`detail::prepare_principal`) and then calls the registered
`require_ghost_permission("db", "exportContent")` via `GhostPermissionRegistry::set_active()`
with a sentinel so denial writes the Ghost 401/403 envelope exactly once. A deliberately weak
`x-blog-role` is still denied; no Authorization produces a Ghost `UnauthorizedError`.

**Input.** Download: `components` is lowercased + comma-split (Ghost `validators/input/all.ts`
semantics); absent → all five, `components=` → 422 `No export components selected`, unknown →
422 `Validation (AllowedValues) failed for components`. Create: the four rules from
`exports.js:155-184` are unchanged but now produce `BadRequestError` envelopes (null
property/context) rather than `ValidationError`; `requestedByUserId` reads `x-blog-user-id`
(the harness carrier for `frame.options.context.user`) with the bearer token as fallback.

**Output / headers.** Download now returns a structurally valid stored (uncompressed) ZIP
(local headers + central directory + EOCD) with Ghost's component entry names
(`export.json`, `members.csv`, `post-analytics.csv`, `themes.zip`, `routes.yaml`,
`redirects.yaml`); the filename uses the slugified settings title plus the UTC date
(`<slug>.ghost.export.<YYYY-MM-DD>.zip`, fallback `ghost.export.<date>.zip`). The JSON archive /
`requests` / hardcoded `export.zip` body is gone. Create returns 202 with `{}`. Neither route
sets `X-Cache-Invalidate` (`headers.cacheInvalidate:false` ⇒ no header).

**Side effects.** Create emits `blog::handlers::outbox::Event{"export.requested",
{"type":"export","siteId":...,"components":...,"requestedByUserId":...}, "exports"}` on the
context-local outbox — the observable, transactional stand-in for Ghost's signed webhook
(`exportRequests.service.requestExport`), and writes no model record. Download is read-only.

**Files changed (all inside `include/blog/handlers/system/exports/`).** `routes.cppm`;
`create/handler.cppm`; `create/workflow/step/execute.cppm`;
`exports_download_get/handler.cppm`; `exports_download_get/workflow/step/execute.cppm`. The two
`workflow/flow/main.cppm` files are unchanged.

**Central registrations.** None required and none edited. All helpers live in existing
registered partitions: `blog_handlers:outbox_shim` (`BUILD.bazel:919`),
`blog_handlers_middleware:ghost_permission` (`BUILD.bazel:945`), `core_auth:ghost`,
`core_workflow`, `core_serde`, `core_query` (unused here), and the context-local `ApiDataStore`
/ `ResourceStore`. Partition identities (`exports_create_step`,
`exports_exports_download_get_step`, …) are unchanged.

**New-partition needs (recorded, not edited).**
- `core_outbox` — canonical target `//include/core/outbox:core_outbox` exists but is not a
  `blog_handlers` dep; the registered `blog_handlers:outbox_shim` (same
  `Event{type,payload,aggregate_id}` / `emit` / transaction API) is used. Main agent can switch
  the import once the dep is wired.
- `core_adapters` — a dedicated signed-webhook adapter (with `STRIPE_BASE_URL`-style
  `EXPORT_WEBHOOK_URL` / secret config) would replace the outbox event with a real signed HTTP
  POST; the target is not a `blog_handlers` dep, so the outbox stands in.
- `blog_handlers:mapping` — exists but is not in the `blog_handlers` BUILD `partitions` list;
  the ZIP/filename helpers remain local to the download step partition.

**Remaining gaps (non-wire-observable / no shared primitive).** The ZIP entry bytes are
context-local snapshots rather than the real Knex `exporter.doExport()`, members/post-analytics
CSV transforms, theme `zipToFile`, and routes/redirects YAML; per-component error logging/skip,
OS temp-dir cleanup and true streaming (`pipeline`) have no allowed primitive. The outbox event
carries a header-derived `siteId` (`x-blog-site-id`) and user id because no host-settings/API-key
context primitive is registered. None of these change status codes, response headers, or the
response body shape (ZIP for GET, `{}` for POST). All 8 checks are closed for both routes.
