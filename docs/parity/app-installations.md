# Parity: app-installations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-installations.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 227–238)
- C++: `include/blog/handlers/external/apps/app_installations_{browse,add,read,edit,destroy}/`
  (live bindings in `external/apps/routes.cppm`)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 23)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 1 |

All five Ghost `appInstallations` routes exist and now reproduce their observable contract
through shared abstractions: `core::query`, `core_serde`, `core_auth::ghost` + permission
middleware, the exact `AppInstallationResource` serializer (+ history), revision-guarded
approval, soft-uninstall idempotency, and an outbox-recorded manifest/challenge/icon/action
side-effect log. Verdict stays PARTIAL (not MATCH): the labs `apps` gate, real HTTP/crypto
manifest work, Knex `FOR UPDATE` row locks, token→role resolution and the full
`buildHistory`/`movedBetween` merge have no allowed shared primitive (see the behavior
report's residual gaps).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/apps/installations` | browse | `GET /apps/installations` via `AppsRoutes::install` | `AppInstallationsBrowseHandler` | none | `{app_installations:[...]}` with Ghost field set, 200 | `core::query` over `app_installations` + manifest JOIN | none; no cache header | `require_auth` + `require_ghost_permission("app_installations","browse")` | PARTIAL | C++ `apps/routes.cppm`, browse step + `apps::support::browse` |
| POST `/apps/installations` | add | `POST /apps/installations` | `AppInstallationsAddHandler` | `data.app_installations[0]` `{manifest_url,digest}` | 201 `{app_installations:[Installation]}` | outbox-recorded review + trx insert + action | none; no cache header | `require_auth` + `require_ghost_permission(...,"add")` | PARTIAL | C++ add step + `support::fetch_reviewed/install` |
| GET `/apps/installations/:id` | read | `GET /apps/installations/:id` | `AppInstallationsReadHandler` | path `id`, `include∈{history}` | `{app_installations:[InstallationWithHistory]}`, 200/404 | `core::query` find + history rows | none; no cache header | `require_auth` + `require_ghost_permission(...,"read")` | PARTIAL | C++ read step + `support::read_one/history_for` |
| PUT `/apps/installations/:id` | edit | `PUT /apps/installations/:id` | `AppInstallationsEditHandler` | path `id`, `{manifest_url,digest,revision≥0}` | `{app_installations:[Installation]}`, 200/404/409 | revision-guarded approval write + action + re-read | none; no cache header | `require_auth` + `require_ghost_permission(...,"add")` (deliberate override) | PARTIAL | C++ edit step + `support::approve` |
| DELETE `/apps/installations/:id` | destroy | `DELETE /apps/installations/:id` (`delt`) | `AppInstallationsDestroyHandler` | path `id` | 204 empty / 404 | soft uninstall + action (idempotent) | none; no cache header | `require_auth` + `require_ghost_permission(...,"destroy")` | PARTIAL | C++ destroy step + `support::uninstall` |

## Route details (implemented strict-parity changes)

All five routes share these facts after the pass:
1. **Route**: bindings already existed in `AppsRoutes::install`; only the router middleware
   changed — `content_type_middleware` was removed (Ghost has no 415 gate on `/apps`),
   `require_auth` kept.
2. **Operation**: existing handler/flow/step trios reused; `Output=std::map<string,string>`
   (`status`/`body`), static `s_ctx`, workflow names unchanged.
3. **Input/DB**: each step now performs the Ghost service work (see the behavior report) using
   shared helpers in `blog::handlers::apps::support`, hosted in the already-registered
   `:apps_app_installations_browse_step` partition — no new partition added.
4. **Output**: `{app_installations:[...]}` (browse/read/add/edit) / 204 empty (destroy),
   with the exact Ghost field set; no `data/meta` wrapper, no fabricated constants.
5. **Cache**: `noCacheInvalidation` → no `X-Cache-Invalidate` header; old literal writes
   removed.
6. **Auth**: per-op `require_ghost_permission` via `core::auth::ghost`; edit uses the `add`
   method override; labs gate remains a residual.
7. **Evidence**: the six `.cppm` files per route compile in
   `bazel build //include/blog/handlers:blog_handlers`; the target still fails only in
   unrelated partitions and the known `blog_handlers` ↔ `blog_handlers_routes` cycle, which
   this group did not touch.

### Fix blocks (implemented; supersede the original route-add plan)

- **GET /apps/installations** — browse: `core::query::apply` with `status:!uninstalled` and
  `created_at asc,id asc`, manifest JOIN, exact serializer, 200, permission
  `("app_installations","browse")`, no cache header. Residuals: SQL JOIN/eager decode, labs.
- **POST /apps/installations** — add: reviewed-field validation, outbox-recorded
  challenge/fetch/icon, digest check, transaction insert (`status:'active'`, `revision:0`)
  and `installed` action, full 201 envelope, permission `("app_installations","add")`.
  Residuals: real HTTP/crypto, Knex trx/deadlock retry, ObjectId ids, labs.
- **GET /apps/installations/:id** — read: `path_id`, `include∈{history}`, `core::query` find,
  404 Ghost `NotFoundError`, history from manifest+action rows (`buildHistory` stand-in),
  `app_installations` envelope. Residuals: full `buildHistory`/`movedBetween`, labs.
- **PUT /apps/installations/:id** — edit: revision validation, 404/409 uninstalled,
  `fetch_reviewed`, revision concurrency (409 `APP_INSTALLATION_CHANGED`), approvalWrite
  decision, outbox transaction + `changes_approved` action + re-read, permission **add**
  override. Residuals: signed-manifest fetch/identity, Knex `FOR UPDATE` lock, labs.
- **DELETE /apps/installations/:id** — destroy: `path_id`, find (uninstalled rows included),
  soft transition (`status:'uninstalled'`, `current_app_id:null`, `revision+1`), idempotent
  204, `uninstalled` action. Residuals: Knex UPDATE stand-in, labs.

## Missing in C++

None. All five Ghost routes have a C++ counterpart. The original MISSING verdicts were
closed in an earlier pass when the route/handler/step trios were added; this pass made their
behavior strict-parity.

## Extra in C++

1. **GET `/apps`** — `apps_all_collection` from the external apps-catalog domain. No Ghost
   counterpart in this controller; not a duplicate of any `/apps/installations*` route. Keep
   it installed — do not delete or gate it. No change made.

## Central registrations needed (parent session)

No new partition was added: the shared `blog::handlers::apps::support` helpers live in the
already-registered `:apps_app_installations_browse_step` partition and are imported by the
other five step partitions. All handler/flow/step partitions and the six route bindings were
already registered centrally in `include/blog/handlers/BUILD.bazel` and `all.cppm`; no shared
file was edited by this group. Recorded new-partition need: none. (Optional future cleanup:
a dedicated `blog_handlers:apps_support` partition would be cleaner than hosting helpers in
the browse step, but it would require central BUILD registration and was deliberately avoided.)
