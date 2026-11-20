# Behavior report: app-sessions

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/app-sessions.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/app_sessions/` (dedicated `AppSessionsRoutes`, unwired) and the live binding in `include/blog/handlers/external/apps/routes.cppm`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (group 22)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

One Ghost route. The C++ step now reproduces `startSession`: `recheck()` with per-id
in-flight dedup, manifest challenge/fetch, identity and other-app checks,
`unavailableUnlessChanged`, silent-apply / pending-suspend writes under a revision check,
and `read()` with the exact `{"app_sessions":[{"installation":...}]}` envelope and Ghost
`AppInstallationResource` field set. Permission uses `core::auth::ghost` through
`require_ghost_permission`; `cacheInvalidate:false` means **no** `X-Cache-Invalidate` header.
Verdict stays `PARTIAL`: the labs `apps` gate, real network/crypto manifest work, the
Knex `whileUnchanged` row lock, token→role resolution and the router-level content-type
gate have no allowed shared primitive (see residual gaps).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST /apps/installations/:id/sessions | `start` (custom action), docName `app_sessions` | Ghost: `options:['id']`, `validation.options.id.required=true`, path param `:id`→`frame.options.id`, no body/multipart. C++: last path segment as `id`, empty→422 `ValidationError` (`execute.cppm:492-503`) | Ghost: `mw.authAdminApi`, labs `apps`, permissions `{app_installations, read}`. C++: router `require_auth` + handler `require_ghost_permission("app_installations","read")` from `core::auth::ghost`; labs gate residual | Ghost: default 200, `cacheInvalidate:false` → no `X-Cache-Invalidate` header. C++: 200/400/404/409/422/500, no `X-Cache-Invalidate` (`handler.cppm:64-72`) | Ghost: `startSession`→`recheck`→`read`. C++: `recheck` (dedup by id, challenge/fetch outbox events, other-app/identity checks, `unavailableUnlessChanged`, silent apply / pending suspend with revision guard) then `read` (`execute.cppm:273-476`) | Ghost: no cache service; `rechecking` Map dedups. C++: no cache service; static guarded `rechecks` map with `condition_variable` shares one result per id (`execute.cppm:252-266,419-459`) | Ghost: `{"app_sessions":[{installation: AppInstallationResource}]}`. C++: same envelope + exact zod field set (`id, app_id, status, revision, manifest_url, manifest, icon_data_url, created_at, updated_at`), no meta (`execute.cppm:179-210,474-475`) | `external/app_sessions/{routes.cppm,sessions_create/handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`; live route also `external/apps/routes.cppm:49-63` | see details | PARTIAL | Ghost `routes.js:227-237`, `app-sessions.ts:6-23`, `service.ts:286-294,525-590`, `serializers.ts:27-29,7-19`; C++ `handler.cppm:29-72`, `execute.cppm:291-476` |

## Route details

### POST /apps/installations/:id/sessions — PARTIAL

**Ghost** (numbered facts 1–8):

1. **Identity / middleware chain.** Exported controller bound by
   `appsRouter.post('/installations/:id/sessions', http(api.appSessions.start))`
   (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:237`). `appsRouter` is created at
   `routes.js:227`, mounted at `router.use('/apps', appsRouter)` (`:229`), guarded by
   `router.all(['/apps','/apps/*'], mw.authAdminApi)` (`:228`) and
   `appsRouter.use(labs.enabledMiddleware('apps'))` (`:230`). API app mounts admin at `/admin/`
   (`web/api/app.js:22`) under base path `ghost/api`
   (`web/api/middleware/api-version-compatibility/legacy-api-path-match.js:4`), so public path is
   `POST /ghost/api/admin/apps/installations/:id/sessions`. `api.appSessions` is wired via
   `apiFramework.pipeline(require('./app-sessions'), localUtils)` (`api/endpoints/index.js:359-361`).
   Chain: `authAdminApi` → labs `apps` → `http(...)` pipeline (validation, permissions, serializer).
2. **Operation.** Export key `start` (`app-sessions.ts:14`), `docName: 'app_sessions'` (`:7`); a custom
   action, not CRUD `browse/read/add/edit/destroy`; resolved as `api.appSessions.start` (routes.js:237).
3. **Input.** `options: ['id']` (`app-sessions.ts:16`); `validation: { options: { id: { required: true } } }`
   (`:17`); frame type `Frame<{ options: { id: string; ... } }>` (`:4`). Path param `:id` from routes.js:237
   becomes `frame.options.id`. No `data[]`, no `data.<resource>[0]` body shape, no multipart fields, no
   query aliases (`filter[x]=y`), no request headers read by the controller.
4. **Auth / permissions.** `mw.authAdminApi` on `/apps/*` (routes.js:228). Labs gate
   `labs.enabledMiddleware('apps')` (routes.js:230) → `NotFoundError` (404) when flag is off
   (`core/shared/labs.js:150-157`). Declarative controller permission
   `permissions: { docName: 'app_installations', method: 'read' }` (`app-sessions.ts:18`) — comment at
   `:9-13` explains this is intentionally the read permission of the installation because the re-check
   fetches only the approved manifest URL and never a URL from the request. Permissions stage resolves
   `permissions.canThis(context).read.app_installation(id)` (`api/endpoints/utils/permissions.js:47`);
   `NoPermissionError` → 403-style Ghost error (`:66-73`). No `unsafeAttrs`, no labs check inside `query`,
   no permission service invoked manually in `query`.
5. **Status + headers.** `headers: { cacheInvalidate: false }` (`app-sessions.ts:15`) → the framework skips the `cacheInvalidate` header branch entirely
   (`@tryghost/api-framework/src/headers.ts:135-144`), so no `X-Cache-Invalidate` header; no
   `statusCode` override, so default 200; no `Content-Disposition`,
   `Cache-Control`, or `Location` headers; no streaming — uses the output serializer in item 8.
6. **Full call chain.** `query(frame)` calls `appInstallations.service.startSession(frame.options.id)`
   (`app-sessions.ts:19-21`).
   - `startSession(id)` (`services/app-installations/service.ts:286-294`): `await this.recheck(id)` (`:287`),
     then `await this.read(id)` (`:288`); if `installation.status === 'uninstalled'` throws
     `notInstalled()` (`:290-292`) = `NotFoundError` "This app is not installed.", code
     `APP_NOT_INSTALLED` (`:76-82`); returns `{ installation }` (`:293`). No session state is stored
     (comment `:281-285`).
   - `recheck(id)` (`:525-533`): dedups concurrent rechecks per installation ID in the `rechecking` Map
     (`:145-146`), running `recheckOnce(id)` once and sharing its promise.
   - `recheckOnce(id)` (`:553-590`): `currentInstallation(id)` (`:554`; implementation `:593-596` uses
     repository `find(id)` and treats `status==='uninstalled'` as absent) → `notInstalled()` if none
     (`:555-557`). `Promise.all([load(before.manifest_url), pinnedIdentityOf(before.app_id, before.identity_key_thumbprint)])`
     (`:562-565`): `load()` (`:345-350`) starts the identity challenge and fetches the manifest over the
     network; `pinnedIdentityOf()` (`:331-339`) reads retired backend keys from the repository. If the
     fetched manifest names a different app → `ValidationError` `APP_MANIFEST_OTHER_APP` (`:567-572`).
     `identity.check(loaded, pinned, started)` (`:573`). `recheckWrite(...)` + `withIconCopy(...)`
     (`:574-576`) plan silent changes vs. pending changes (pending → suspend). `store(before, write)`
     (`:586`; implementation `:490-523`) runs inside `repository.whileUnchanged(current, ...)` with
     `locked.activate` / `addManifest` / `suspend` (`:499-514`), throwing if the installation changed
     mid-flight; `stored.status === 'uninstalled'` → `notInstalled()` (`:587-589`). A `ValidationError`
     routes to `unavailableUnlessChanged()` (`:577-584`, `:603-615`): if revision changed, proceed with
     the current installation; otherwise throw `unavailable()` = `ConflictError` with code
     `APP_UNAVAILABLE` (`:98-104`). Non-Validation errors rethrow (`:580-582`).
   - `read(id)` (`:177-189`): `repository.find(id)`; missing → `notFound()` = `NotFoundError` "App
     installation not found." (`:72-74`); `withHistory` defaults false, so no history is loaded.
   - Side effects: outbound manifest fetch + identity challenge (network, `:345-350`), possible
     manifest row writes / suspension (`:574-576`, `:490-523`), no staff history record for
     recheck-driven changes (comment `:535-539`), no outbox/mail/webhook/third-party calls in this path.
   - Error surface: 404 (`APP_NOT_INSTALLED`, "App installation not found."), 409 (`APP_UNAVAILABLE`,
     `APP_MANIFEST_OTHER_APP`), plus framework 401/403 from auth/permissions and 404 from labs flag.
7. **Cache.** No cache service/key/TTL in this route. `headers.cacheInvalidate:false`
   (`app-sessions.ts:15`) suppresses the invalidate header. The only in-process state is the
   `rechecking: Map<string, Promise<void>>` dedup (service.ts:145-146, :525-533).
8. **Output.** Serializer `endpoints/utils/serializers/output/app-sessions.ts:6-9` sets
   `frame.response = toAppSessionsResponse.parse(session)`. Zod schema
   (`services/app-installations/serializers.ts:27-29,38-41`) yields envelope
   `{"app_sessions":[{"installation": AppInstallationResource}]}`. `AppInstallationResource` fields
   (`serializers.ts:7-19`): `id`, `app_id`, `status`, `revision`, `manifest_url`, `manifest`,
   `icon_data_url`, `created_at`, `updated_at`, optional `history`. No pagination/meta/computed fields.
   Missing/uninstalled installation → 404 (service.ts `:76-82`, `:72-74`); empty array never returned.

**C++** (numbered facts, strict-parity pass):

1. **Identity / routing.** `external/app_sessions/routes.cppm:12-36`: `AppSessionsRoutes::install`
   creates `Router{router,"/apps"}` with `require_auth` only (`:25`); the bodyless action no longer
   carries `content_type_middleware`. The live registration is still `AppsRoutes::install` in
   `external/apps/routes.cppm:49-63` (installed from central `routes.cppm:81`); the dedicated
   installer remains unwired (central edit required, not made here).
2. **Operation / symbols.** Getter `app_sessions_sessions_create()` (`all.cppm:610`), member
   `m_app_sessions_sessions_create{m_ctx}` (`:889`); `AppSessionsCreateHandler`
   (`handler.cppm:14`); workflow `"app_sessions_sessions_create"` (`flow/main.cppm:10`); step
   `"app_sessions_sessions_create_execute"` (`execute.cppm:488`); single-step `MainFlow`; the
   handler sets `ExecuteStep::s_ctx` and uses `LocalWorkflowDriver`. `Output` remains
   `std::map<std::string,std::string>` and `s_ctx` remains the context channel as required.
3. **Input / parsing.** `execute.cppm:492-503`: last non-empty path segment → `id`; empty → 422
   `ApiError{}.validation("id is required", {}, "id")` (framework equivalent of
   `validation.options.id.required`). No body/`data[]`/multipart, no query aliases, no header reads
   by the controller.
4. **Middleware chain.** `require_auth` (router, absence of bearer → 401) plus
   `blog::handlers::middleware::require_ghost_permission(req,res,next,send,
   "app_installations","read")` in `handler.cppm:44-49`, built on `core::auth::ghost::GhostAuthorizer`
   with `ADMIN_API` context and a presented-token→Administrator mapping (`:31-38`). Denials write a
   Ghost `UnauthorizedError`/`NoPermissionError` envelope through `write_error`. Labs `apps` gate is
   still a residual.
5. **Status / headers / body writes.** `handler.cppm:52-74`: workflow result → `status` + `body`;
   maps `400→BAD_REQUEST`, `404→NOT_FOUND`, `409→CONFLICT`, `422→UNPROCESSABLE_CONTENT`, default `OK`;
   no `X-Cache-Invalidate` write — `cacheInvalidate:false` makes Ghost skip the header entirely
   (`api-framework/src/headers.ts:135-144`), so 200/404/409/422 all omit it.
6. **Call chain.** `execute.cppm:291-476`:
   `start_session` = `recheck(id)` then `read(id)`. `recheck_once` reads the current installation
   through `core::query::apply` over `ctx.store().json_for("app_installations",{})` (`:114`), records
   `identity.challenge` + `manifest.fetch` outbox events (`:300-301`), loads the newest stored
   manifest row for the installation, maps a different served app to `APP_MANIFEST_OTHER_APP` and an
   identity mismatch to `APP_UNAVAILABLE` via `unavailable_unless_changed` (`:273-281,311-326`),
   plans silent-apply / pending-suspend from digest + `requires_approval` (`:328-362`), re-reads the
   revision under an outbox transaction (`whileUnchanged` stand-in, `:368-371`), writes the manifest
   row and status/revision patch, then emits `manifest.added` + `installation.activated|suspended`
   (`:363-417`). `recheck` dedups concurrent calls per id with a guarded slot map + condition
   variable (`:252-266,419-459`). `read` distinguishes `notFound()` and `notInstalled()` (`:470-471`).
7. **Cache / side effects.** No `core::cache::Cache` key, tag or TTL; the only in-process state is the
   `rechecking` dedup map. Manifest fetch/challenge/icon and row writes are recorded in
   `blog::handlers::outbox` (`Outbox::begin_transaction/commit`, `global_outbox`) for verification,
   not sent on the wire. No session row is stored, matching Ghost.
8. **Output.** `execute.cppm:179-210,474-475`: the installation is projected to the exact
   `AppInstallationResource` field set (`id`, `app_id`, `status`, `revision`, `manifest_url`,
   `manifest`, `icon_data_url`, `created_at`, `updated_at`; `history` omitted because `read` uses
   `withHistory=false`) and wrapped as `{"app_sessions":[{"installation":...}]}` — no `data/meta`.

**Diff:**

- Residuals (Ghost behavior with no allowed shared primitive):
  - Labs `apps` flag gate returning 404 when off (`routes.js:230`, `shared/labs.js:150-157`).
  - Real HTTP fetch of the approved manifest URL, cryptographic identity challenge/signature check,
    icon download, and `compareManifests` path-level diff. The C++ stand-in reads manifest rows from
    the ResourceStore, records the fetch/challenge/icon in the outbox, compares digests and uses the
    stored `requires_approval` flag; a manifest row with no digest is treated as unchanged.
  - Knex `repository.whileUnchanged` row lock: C++ re-reads `revision` before writing inside an
    outbox transaction instead of taking a DB lock.
  - `authAdminApi` token→user resolution: a presented bearer token maps to Administrator with
    `*` permissions (presence-only); no token store exists in this folder.
  - Router-level `content_type_middleware` in `apps/routes.cppm:20` still applies a 415/406 gate
    to this bodyless action (it is shared with the sibling app-installations routes here); Ghost
    has no such gate on `start`.
  - No new test target was added in this folder (test registration is a shared BUILD edit).
- Divergences resolved by this pass:
  - Output envelope is now `{"app_sessions":[{"installation":...}]}`, not `{"installation":...}`.
  - Field set now matches `AppInstallationResource` (adds `manifest_url`, `manifest`,
    `icon_data_url`, numeric `revision`, nullable `updated_at`).
  - `recheck` is implemented (dedup, fetch/challenge record, other-app/identity failures,
    silent apply / pending suspend, revision guard), replacing the `"status":"uninstalled"`
    substring check; distinct 404/409 envelopes carry `APP_NOT_INSTALLED`, `APP_UNAVAILABLE` and
    `APP_MANIFEST_OTHER_APP`.
  - Permission now routes through `core::auth::ghost` (`app_installations:read`).
  - No `X-Cache-Invalidate` header is emitted on any status; previously it was always `false`.
- Remaining structural gap: the dedicated `AppSessionsRoutes` installer is still unwired; the same
  path is served through `AppsRoutes` (`external/apps/routes.cppm:49-63`, central `routes.cppm:81`).
  Wiring is a central-file edit and is recorded, not performed.

## Strict-parity implementation notes (group 22)

### Shared abstractions used

- `core::query` — `ParsedQuery` with explicit `id` / `installation_id` filters and
  `created_at desc` order over `ctx.store().json_for(...)`; `apply()` does the single
  filtered/ordered pass (no substring matching on serialized JSON).
- `core::auth::ghost` + `blog_handlers_middleware::require_ghost_permission` — the
  full middleware shape enforces `{app_installations, read}`, writing Ghost-style
  401/403 envelopes on denial; the handler supplies the `ADMIN_API` principal.
- `blog::handlers::outbox` (`outbox_shim.cppm`, already registered in `blog_handlers`)
  — transactional record of `identity.challenge`, `manifest.fetch`, `icon.fetch`,
  `manifest.added` and `installation.activated|suspended`, with `begin_transaction`/
  `commit` for the revision-guarded store step.
- `core_serde` — JSON decode/encode for store rows and manifest objects;
  `blog::api::ApiError` — 422 validation envelope.
- `Output=std::map<std::string,std::string>` and `static blog::BlogContext* s_ctx`
  are preserved (no new context channel).

### New-partition / central needs (recorded, not edited)

- `blog_handlers:mapping` is not listed in `include/blog/handlers/BUILD.bazel`'s
  `blog_handlers` partitions. This step uses a local `installation_resource` projection
  instead of importing `clean`/`wrap`; wire the mapping partitions and add an
  `app_installation` field set to `clean.cppm` to drop the local projection.
- `core_adapters` is not a dependency of `blog_handlers`; the manifest/icon fetches
  are recorded through `outbox_shim` rather than `core::adapters::WebmentionAdapter`.
  Add the dep if a real local mock server must be driven.
- Central wiring of `AppSessionsRoutes` (or removal of the duplicate route) remains
  in `all.cppm`, `base.cppm`, `BUILD.bazel` and central `routes.cppm`.

Verdict **PARTIAL**: all controller-observable behavior (input, permission, recheck
failure taxonomy, silent/suspend writes, success envelope and cache-header suppression)
is implemented; the remaining gaps are infrastructure primitives (labs flag, network/
crypto identity, real row lock, router-level content-type gate, central wiring).

## Module-level notes

- Single route in this controller. No `data[]`, multipart, `Location`, `Content-Disposition`,
  pagination meta, or streaming applies to this module on either side.
- The C++ handler is now a recheck + read over the in-memory resource store (`app_installations`
  plus `app_installation_manifests`), shared with the sibling `external/apps/app_installations_*`
  handlers (`context_context.cppm:139,243` declares the resource fields and default `revision=0`).
  It reuses `core::query`, `core::auth::ghost`, `core_serde` and `outbox_shim`; no new shared module
  was created.
- There are two C++ route registrations for `POST /apps/installations/:id/sessions` pointing to the same
  handler (`external/app_sessions/routes.cppm:27-35` and `external/apps/routes.cppm:56-63`); only the
  latter is installed by `ApiRoutes::install` (`include/blog/handlers/routes.cppm:74-138,81`).
- The planning doc `docs/parity/app-sessions.md` was updated by this pass from MISSING to PARTIAL and
  its Fix blocks now describe the implemented strict-parity behavior plus the recorded central
  registration / new-partition needs.
