# Behavior report: app_installations

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/app-installations.ts`
- Ghost route table: `docs/Ghost/ghost/core/web/api/endpoints/admin/routes.js` (lines 221-238)
- Ghost service root: `docs/Ghost/ghost/core/server/services/app-installations/` (wired in `index.ts:94-135`)
- C++ folders: `include/blog/handlers/external/apps/app_installations_{browse,add,read,edit,destroy}/` plus `include/blog/handlers/external/apps/routes.cppm`
- Analyzed: 2026-11-19; strict-parity pass: 2026-11-20 (group 23)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All five Ghost routes have a C++ counterpart with the same path + verb + success status.
The strict-parity pass replaces the generic resource-store CRUD stubs with the observable
Ghost service contract: `core::query` NQL filtering/order, `core_auth::ghost` permissions,
the exact `AppInstallationResource` serializer (+ history), revision-guarded approval,
soft-uninstall idempotency, and outbox-recorded manifest/challenge/icon/action side effects.
Verdicts stay **PARTIAL**: the labs `apps` gate, real HTTP/crypto manifest work, the Knex
`SELECT ... FOR UPDATE` row lock, token→role resolution and (on read) the full action/manifest
history merge have no allowed shared primitive (see residual gaps).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /apps/installations | browse | none | `authAdminApi` + `permissions:true` + labs `apps` | 200; no `X-Cache-Invalidate` | `service.browse` → `repo.list` (JOIN manifests, exclude uninstalled, ORDER BY) → `toAppInstallationsResponse` | none | `{app_installations:[Installation...]}` | browse `handler.cppm`, `workflow/step/execute.cppm` (+ shared `apps::support`) | `core::query::apply` over `app_installations` (`status:!uninstalled`, `created_at asc,id asc`), manifest JOIN + exact serializer; permission gate | PARTIAL | Ghost `routes.js:233`, controller:57-63, service.ts:169-171, repo:166-173, serializers.ts:31-36; C++ `routes.cppm:30-39`, browse step, support::browse |
| POST /apps/installations | add | `data.app_installations[0]` = `{manifest_url, digest}` | `authAdminApi` + `permissions:true` + labs `apps` | 201; no `X-Cache-Invalidate` | `service.create(actingContext, reviewed)` → `fetchReviewed`→`load`→identity challenge→`fetchIcon`→`repo.insert` (trx) → `recordAction('installed')` | none | 201 `{app_installations:[Installation]}` | add `handler.cppm`, `workflow/step/execute.cppm` | validates reviewed fields, outbox-recorded challenge/fetch/icon, digest check, trx insert (status active/revision 0) + `action.add` outbox/store write, full serializer | PARTIAL | Ghost `routes.js:234`, controller:32-40,77-87, service.ts:201-221; C++ add step, support::fetch_reviewed/install |
| GET /apps/installations/:id | read | path `id`; query `?include=history` | `authAdminApi` + `permissions:true` + labs `apps` | 200 / 404; no `X-Cache-Invalidate` | `service.read(id,{withHistory})` → `repo.find` → optional `historyOf` → serializer | none | `{app_installations:[InstallationWithHistory]}` | read `handler.cppm`, `workflow/step/execute.cppm` | `path_id`, `include∈{history}`, `core::query` lookup, 404 NotFound, history from manifest+action rows (`buildHistory` stand-in), `app_installations` envelope | PARTIAL | Ghost `routes.js:235`, controller:65-75, service.ts:177-189; C++ read step, support::read_one/history_for |
| PUT /apps/installations/:id | edit | path `id`; `{manifest_url, digest, revision}` | `authAdminApi` + `permissions:{method:'add'}` + labs `apps` | 200 / 404 / 409; no `X-Cache-Invalidate` | `service.approve` → `repo.find`→`fetchReviewed`→revision check→`approvalWrite`→`store`(lock trx) → `recordAction('changes_approved')` → `read(id)` | none | `{app_installations:[Installation]}` | edit `handler.cppm`, `workflow/step/execute.cppm` | parses id, validates revision≥0, 404/409 uninstalled, fetch+digest, revision concurrency, approvalWrite decision, outbox trx write + action + re-read | PARTIAL | Ghost `routes.js:236`, controller:42-50,91-103, service.ts:234-279; C++ edit step, support::approve |
| DELETE /apps/installations/:id | destroy | path `id` | `authAdminApi` + `permissions:true` + labs `apps` | 204 / 404; no `X-Cache-Invalidate` | `service.uninstall` → `read`→`repo.uninstall` (soft) → `recordAction('uninstalled')` | none | 204, empty body | destroy `handler.cppm`, `workflow/step/execute.cppm` | `path_id`, find (uninstalled rows included), soft status transition + `current_app_id:null` + `revision+1`, outbox action, idempotent 204, empty body | PARTIAL | Ghost `routes.js:238`, controller:105-117, service.ts:302-316; C++ destroy step, support::uninstall |

## Route details

Shared C++ facts for all five routes (strict-parity pass, 2026-11-20):
- **Route + middleware.** `Router{router,"/apps"}.use(require_auth)` in
  `external/apps/routes.cppm`; the former `content_type_middleware` 415 gate was removed —
  Ghost uses `bodyParser.json/urlencoded` on `/apps`, no content-type/accept guard.
  `require_auth` preserves the Ghost 401 `UnauthorizedError`. Labs `apps` gate is a residual.
- **Permission gate.** Each handler publishes `x-blog-context:ADMIN_API` and, for bearer
  callers without explicit role headers, `x-blog-role:Administrator` + `x-blog-permissions:*`,
  then calls `blog::handlers::middleware::require_ghost_permission(req,res,next,send,docName,method)`
  backed by `core::auth::ghost::GhostAuthorizer`, emitting a Ghost 403 `NoPermissionError` on
  denial. docName is always `app_installations`; method is `browse`/`add`/`read`/`destroy`
  per op, with **edit deliberately using `add`** (`permissions:{method:'add'}`, controller:95).
- **Cache.** `headers.noCacheInvalidation` (`cacheInvalidate:false`) is honored by emitting
  **no** `X-Cache-Invalidate` header. The old literal `false` writes were removed.
- **Shared support.** Common helpers live in the already-registered
  `:apps_app_installations_browse_step` partition as `blog::handlers::apps::support`, and
  are imported by the other five steps — no new partition was needed. Shared abstractions:
  `core_query` (filter/order), `core_serde`, `core_auth::ghost` + middleware, and
  `blog::handlers::outbox` (`:outbox_shim`) for side effects.

### GET /apps/installations — PARTIAL

**Ghost** (numbered per checklist 1–8): unchanged from the original analysis — browse has
no options/body/validation, `permissions:true`, 200 + `cacheInvalidate:false`,
`service.browse()` → `repository.list()` (JOIN approved manifest, `whereNot status=uninstalled`,
`ORDER BY created_at asc, id asc`), `{app_installations:[AppInstallationResource...]}`.

**C++** (strict-parity pass):
1. Route `Route{"/installations"}.get(...)` → `AppInstallationsBrowseHandler`; permission
   `require_ghost_permission(...,"app_installations","browse")`.
2. Step `Output=std::map<string,string>` (`status`/`body`), static `s_ctx`.
3. Input: none (matches Ghost).
4. Store: `support::browse` calls `core::query::apply` with `raw_filter="status:!uninstalled"`
   and `order="created_at asc,id asc"` over `ctx.store().json_for("app_installations",{})`
   — in-memory stand-in for the Knex JOIN/ORDER BY. Joined manifest columns filled from
   `app_installation_manifests` when absent.
5. Output: every row projected through `installation_resource` (exact zod field set
   `id, app_id, status, revision, manifest_url, manifest, icon_data_url, created_at,
   updated_at`) inside `{"app_installations":[...]}`; no `data/meta` wrapper.
6. Status/headers: 200, no `X-Cache-Invalidate`.
7. Side effects: none (read-only).
8. NOT implemented: real SQL JOIN/eager decode, labs gate, token→role resolution.

**Diff** (residuals, not divergences): store stand-in for Knex JOIN/ORDER; labs/token residuals.
The former raw-row `{"data":[...]}` envelope and in-step uninstalled filter were replaced.

### POST /apps/installations — PARTIAL

**Ghost**: `statusCode:201`, `permissions:true`, `reviewed()` zod on
`data.app_installations[0]` (`manifest_url`+`digest`), `service.create(actingContext(context),
{manifestUrl,digest})` → `fetchReviewed` (load + identity challenge + signed-manifest verify +
digest) → `fetchIcon` → `repo.insert` trx → best-effort `recordAction('installed')`;
`{app_installations:[Installation]}`.

**C++** (strict-parity pass):
1. Route `Route{"/installations"}.post(...)`; permission `("app_installations","add")`.
2. Step parses the frame with `first_data_item(*,"app_installations")`, requires both strings
   with Ghost-style 422 `ValidationError`; `revision` not read (add).
3. `support::fetch_reviewed`: `load` emits `identity.challenge` + `manifest.fetch` outbox
   events, reads the newest fixture manifest row, checks other-app/identity (coded errors
   `APP_MANIFEST_*`/`APP_IDENTITY_MISMATCH`), and rejects a digest mismatch with 409
   `APP_MANIFEST_CHANGED`.
4. `support::install`: `icon.fetch` outbox event, `begin_transaction`, inserts the manifest row
   (`source:'install'`, `requires_approval:false`) and installation row (`status:'active'`,
   `revision:0`, `current_app_id`), emits `installation.installed`, commits, then records
   `installed` via `action.add` (store + outbox, best-effort). Duplicate install → 409
   `APP_ALREADY_INSTALLED`.
5. Output: 201 `{app_installations:[<full Installation>]}`, no cache header.
6. Side effects: all external work is outbox-recorded; no live network/DB.

**Diff** (residuals): real HTTP fetch/signature verification and Knex deadlock-retry/trx are
outbox stand-ins; ObjectId ids replaced by deterministic hex fixtures; labs/token residuals.
Former stub (auto-id + raw request echo) replaced.

### GET /apps/installations/:id — PARTIAL

**Ghost**: `options:['id','include']`, `id` required, `include∈['history']`→`withHistory`,
`service.read(id,{withHistory})` → `repo.find` (404 NotFound if absent) + optional
`historyOf(id)` (manifests + actions → `buildHistory`, newest-first, `moved_to`),
`{app_installations:[InstallationWithHistory]}`.

**C++** (strict-parity pass):
1. Route `Route{"/installations/:id"}.get(...)`; permission `("app_installations","read")`.
2. Step: `path_id` from last segment (422 on empty); query `include` must be empty or
   `history` (422 otherwise), mirroring the framework validation.
3. `support::read_one`: `core::query::apply` filter `id:<id>` over the store; miss → 404
   Ghost `NotFoundError` envelope; hit → manifest JOIN + exact serializer.
4. When `include=history`, `support::history_for` reads `app_installation_manifests` and
   `actions` rows and builds entries `installed/updated/suspended/resumed/changes_approved/
   uninstalled` (actor `{id,name}|null`, `moved_to` for approvals), newest-first by
   `created_at` then id — a stand-in for `buildHistory`.
5. Output: `{app_installations:[...]}` with optional `history`; no `data/meta` wrapper;
   no cache header.

**Diff** (residuals): `buildHistory`'s full action/manifest merge and `movedBetween` host
comparison are approximated by fixture fields; labs/token residuals. Former bare-object
response and raw-manifest history were replaced with the Ghost envelope + entry shape.

### PUT /apps/installations/:id — PARTIAL

**Ghost**: `options:['id']`, `id` required, `reviewedChange()` adds non-negative int
`revision`, `permissions:{method:'add'}`; `service.approve` → `repo.find` (404) → 409
`uninstalled` → `fetchReviewed` (other-app/identity/digest) → `current.revision!==revision` →
409 `APP_INSTALLATION_CHANGED` with new preview → `approvalWrite`/`withIconCopy` → `store`
(`whileUnchanged` row lock trx) → `recordAction('changes_approved')` → `read(id)`;
200 `{app_installations:[Installation]}`.

**C++** (strict-parity pass):
1. Route `Route{"/installations/:id"}.put(...)`; permission **add** override.
2. Step: `path_id`; `first_data_item`; validates `manifest_url`, `digest`, and
   `revision` parsed as a non-negative integer → 422 on failure.
3. `support::approve`: `find_installation` → 404; `status==uninstalled` → 409
   `APP_INSTALLATION_UNINSTALLED`; `fetch_reviewed` → coded 422/409; revision mismatch →
   409 `APP_INSTALLATION_CHANGED`.
4. Plans the write from approved/pending digests (`approvalWrite` stand-in: nothing /
   approve_existing / add_approved), fetches the icon only for added rows, re-reads the
   revision (`whileUnchanged` stand-in), writes under an outbox transaction, records
   `changes_approved` with `from_manifest_id`/`to_manifest_id`, and re-reads via `read_one`.
5. Output: 200 `{app_installations:[...]}`; no cache header. No 409 on success path except
   the coded conflicts above.

**Diff** (residuals): real signed-manifest fetch/identity check and the Knex `FOR UPDATE` lock
are outbox/revision stand-ins; labs/token residuals. Former generic `edit` merge (which
ignored `revision` and allowed mass-assignment) was replaced.

### DELETE /apps/installations/:id — PARTIAL

**Ghost**: `options:['id']`, `id` required, `statusCode:204`, `permissions:true`;
`service.uninstall` → `read(id)` (404 if absent, uninstalled rows still found) →
`repo.uninstall` (soft: `current_app_id=null`, `status='uninstalled'`, `revision+1`,
returns 0 if already uninstalled → silent no-op) → `recordAction('uninstalled')`;
204 empty body.

**C++** (strict-parity pass):
1. Route `Route{"/installations/:id"}.delt(...)`; permission `("app_installations","destroy")`.
2. Step: `path_id` (422 on empty).
3. `support::uninstall`: `find_installation` includes uninstalled rows → 404 only when the
   row is absent; already-uninstalled → immediate 204 (idempotent, no second action).
4. Otherwise outbox transaction patches `status='uninstalled'`, `current_app_id=null`,
   `revision+1`, records `uninstalled` action, commits; row retained as history.
5. Output: 204 with empty body; no cache header.

**Diff** (residuals): Knex soft-delete UPDATE is a `resources().edit` stand-in; best-effort
Action write is outbox-backed; labs/token residuals. Former hard `destroy` (non-idempotent,
second delete → 404) replaced with soft idempotent transition.

## Module-level notes

- **Middleware gap.** Ghost applies `authAdminApi` on `/apps`, the labs `apps` flag, and
  per-op `permissions`; C++ applies `require_auth` at the router and per-op
  `require_ghost_permission` in handlers. Labs has no allowed primitive — residual.
- **Cache/header.** `cacheInvalidate:false` on all five ops → no `X-Cache-Invalidate` header
  (old literal writes removed); no `core::cache::Cache` key/TTL used directly.
- **Output envelopes.** browse/read/add/edit all use `{app_installations:[...]}` with the
  Ghost field set; destroy emits 204 empty. No hand-built `{"data":[...]}` remains.
- **Shared abstractions used**: `core_query`, `core_serde`, `core_auth::ghost` +
  `blog_handlers_middleware:ghost_permission`, `blog::handlers::outbox` (`:outbox_shim`),
  context store. No new partition needed — helpers live in the registered
  `:apps_app_installations_browse_step` partition.
- **Residual gaps (no allowed shared primitive)**: labs `apps` gate; real remote HTTP
  manifest/challenge/icon fetches; cryptographic signature verification + key thumbprints;
  Knex JOIN/`FOR UPDATE` row locks/deadlock retry; token→role resolution; full
  `buildHistory`/`movedBetween` action-manifest merge. These are infrastructure/data-layer
  stand-ins, not handler-behavior divergences; hence verdicts stay PARTIAL.
