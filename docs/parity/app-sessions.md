# Parity: app-sessions

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-sessions.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/app_sessions/` (dedicated `AppSessionsRoutes`,
  unwired) and the live binding in `include/blog/handlers/external/apps/routes.cppm`
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 22)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

The single route exists and now reproduces `startSession`'s observable contract through
shared abstractions. Verdict is PARTIAL, not MATCH: the labs `apps` gate, real HTTP/
crypto manifest work, the Knex row lock, presence-only token→role mapping and the
router-level content-type gate have no allowed shared primitive (see the behavior
report's residual gaps).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST /apps/installations/:id/sessions | `appSessions.start`, docName `app_sessions` | `POST /apps/installations/:id/sessions` via `AppsRoutes::install` (`external/apps/routes.cppm:56-63`); duplicate dedicated installer `external/app_sessions/routes.cppm:27-35` (unwired) | `blog::handlers::app_sessions::sessions_create::AppSessionsCreateHandler` (`external/app_sessions/sessions_create/handler.cppm`) | Ghost: `options:['id']`, `validation.options.id.required=true`, no `data[]`; C++: last path segment, empty→422 | Ghost: `{"app_sessions":[{installation: AppInstallationResource}]}`, 200, `cacheInvalidate:false`; C++: same envelope + field set, 200/400/404/409/422, no `X-Cache-Invalidate` | Ghost: `startSession(id)`→`recheck(id)`+`read(id)`; C++: `core::query` over `app_installations` + `app_installation_manifests`, outbox-recorded recheck, revision-guarded writes | Ghost: no cache service, `rechecking` Map dedup; C++: no cache service, guarded per-id dedup map, no stored session | Ghost: `authAdminApi` + labs `apps` + `{app_installations, read}`; C++: `require_auth` + `require_ghost_permission("app_installations","read")`; labs residual | PARTIAL | Ghost `admin/routes.js:237,228,230`, `app-sessions.ts:14-21`, `service.ts:286-294,525-590`, `serializers.ts:27-29`; C++ `apps/routes.cppm:56-63`, `handler.cppm:29-74`, `execute.cppm:265-444` |

## Route details

### POST /apps/installations/:id/sessions — PARTIAL

Ghost facts (1–8) are unchanged from `docs/parity/behavior/app-sessions.md`; the C++ side
after this pass is:

1. **Route identity**: the live registration is `AppsRoutes::install` in
   `include/blog/handlers/external/apps/routes.cppm:56-63`, installed from central
   `include/blog/handlers/routes.cppm:81`. `AppSessionsRoutes::install` still registers the same
   path but is not installed centrally; central wiring remains a recorded need.
2. **Operation mapping**: key `start`, docName `app_sessions`; handler
   `AppSessionsCreateHandler`, workflow/step names `"app_sessions_sessions_create"` /
   `"..._execute"`, `Output=std::map<std::string,std::string>`, static `s_ctx`.
3. **Input parity**: `id` from the last non-empty path segment; empty → 422
   `ApiError{}.validation("id is required", {}, "id")`; no body/multipart/query aliases.
4. **Output parity**: `execute.cppm:474-475` emits
   `{"app_sessions":[{"installation":<resource>}]}` with exactly
   `id, app_id, status, revision, manifest_url, manifest, icon_data_url, created_at,
   updated_at` (`history` omitted; `read` uses `withHistory=false`). No `data/meta` wrapper.
5. **DB call parity**: `core::query::apply` filters the ResourceStore rows by `id` /
   `installation_id`; the step rechecks then reads, writes manifest and installation rows
   only for silent-apply / pending-suspend decisions, guarded by a revision re-read.
6. **Cache parity**: no `core::cache::Cache`; `cacheInvalidate:false` is honored by emitting
   **no** `X-Cache-Invalidate` header on any status. The old `X-Cache-Invalidate: false`
   write was removed (the framework skips the header entirely — `api-framework/src/headers.ts:135-144`).
7. **Auth / permissions**: `require_auth` at the router plus
   `blog::handlers::middleware::require_ghost_permission("app_installations","read")`
   in the handler, backed by `core::auth::ghost::GhostAuthorizer`. Presented bearer maps to
   Administrator/`*` (presence-only). Labs `apps` gate remains a residual.
8. **Verdict**: PARTIAL — controller behavior reproduced; infrastructure residuals above.

#### Fix (implemented by group 22; supersedes the original route-add plan)

1. **Check 1 — route.** No new route needed: `POST /apps/installations/:id/sessions` already
   exists in `AppsRoutes::install` (`external/apps/routes.cppm:56-63`). The dedicated
   `AppSessionsRoutes` installer (`external/app_sessions/routes.cppm:12-36`) now keeps only
   `require_auth` (no content-type gate for this bodyless action). Central wiring of either
   installer is recorded, not performed.
2. **Check 2 — operation.** Reused the existing `app_sessions_sessions_create` getter,
   handler, flow and step; no new symbols or central `all.cppm` edits were required.
3. **Checks 3, 5 — input + DB call.** `id` parsed from the path, 422 on empty. `recheck`
   (`execute.cppm:291-459`) dedups per id, records `identity.challenge`/`manifest.fetch`,
   checks other-app / identity failures via `unavailableUnlessChanged` (409 coded errors),
   plans silent apply / pending suspend from digest + `requires_approval`, re-reads revision
   under an outbox transaction, writes manifest rows and status patches, then `read`
   (`execute.cppm:461-477`) returns the installation. No session state is stored.
4. **Check 4 — output.** Local `installation_resource` projection (`execute.cppm:179-210`)
   produces Ghost's field set and the `{"app_sessions":[{"installation":...}]}` envelope;
   `blog_handlers:mapping` is not yet registered, so a shared `clean`/`wrap` import is
   recorded as a new-partition need instead.
5. **Check 6 — cache.** `cacheInvalidate:false` → no cache header and no cache key/TTL;
   the previous unconditional `X-Cache-Invalidate: false` write was removed.
6. **Check 7 — auth.** `require_ghost_permission("app_installations","read")` enforces the
   controller permission via `core::auth::ghost`; labs gate is a residual.
7. **Check 8 — evidence.** `.cppm` files compile in `bazel build //include/blog/handlers:blog_handlers`
   (the target currently fails only in unrelated partitions and the known `blog_handlers` ↔
   `blog_handlers_routes` cycle, which this group did not touch). Checks 1–8 closed except
   the recorded residuals and central wiring.

## Missing in C++

None. The Ghost route has a C++ route and handler. The 2026-11-19 MISSING verdict was
superseded when the route/handler trio was added; this pass updated the behavioral
implementation rather than adding another route.

## Extra in C++

None. The out-of-scope `GET /apps` in `external/apps/routes.cppm:22-29` belongs to the
external apps-catalog domain and is not counted here.

## Needs central registration / new partitions (recorded, not edited)

Already present in the tree (no central edit needed to compile them):

- `include/blog/handlers/external/app_sessions/routes.cppm` — `AppSessionsRoutes::install`,
  now without the content-type gate; lambda calls `app_sessions_sessions_create().handle(...)`.
- `include/blog/handlers/external/app_sessions/sessions_create/{handler.cppm,
  workflow/flow/main.cppm, workflow/step/execute.cppm}` — handler enforces
  `{app_installations, read}` and maps 400/404/409/422; step implements recheck + read,
  `Output=std::map<std::string,std::string>`, static `s_ctx`.

Central edits still required (shared files — NOT done here):

1. `include/blog/handlers/all.cppm`, `base.cppm`, `BUILD.bazel`, central `routes.cppm` —
   wire `AppSessionsRoutes` (the dedicated installer) or remove its duplicate registration;
   the live route currently comes only from `AppsRoutes` (`routes.cppm:81`).
2. `include/blog/handlers/BUILD.bazel` — register the `blog_handlers:mapping` partitions
   (`mapping/clean.cppm`, `mapping/wrap.cppm`) and add an `app_installation` field set to
   `clean.cppm`; then the local `installation_resource` projection can be replaced by
   `blog::mapping::clean`/`wrap`.
3. `include/blog/handlers/BUILD.bazel` deps — add `//include/core/adapters:core_adapters`
   if the manifest/icon fetch must call a real local mock server instead of being recorded
   through `outbox_shim`.
4. Test target registration for a focused app-sessions unit test (no test BUILD entry added here).

Residual parity gaps unchanged by availability of primitives: labs `apps` flag gate and
`authAdminApi` token→user resolution; real manifest HTTP fetch, identity challenge/signature
check and icon download; Knex `whileUnchanged` row lock; router-level content-type gate on
the bodyless action. `cacheInvalidate:false` is honored by emitting no cache-invalidate header.
