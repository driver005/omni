# Parity: app-installation-previews

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/app-installation-previews.ts`
- Ghost routes: `docs/Ghost/ghost/core/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/apps/app_installation_previews_add/` (live binding in
  `external/apps/routes.cppm`)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 23)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 1 |

The route exists and now reproduces the full `AppInstallationPreview` contract through shared
abstractions (`core::query`, `core_serde`, `core_auth::ghost` + permission middleware,
`blog::handlers::outbox`). Verdict stays PARTIAL (not MATCH): the labs `apps` gate and real
HTTP/crypto manifest/icon work have no allowed shared primitive (see residual gaps).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| `POST /apps/installations/preview` | `add`, docName `app_installation_previews` | `POST /apps/installations/preview` via `AppsRoutes::install` (`external/apps/routes.cppm:64-73`) | `blog::handlers::apps::app_installation_previews_add::AppInstallationPreviewsAddHandler` | Ghost: `app_installation_previews[0].manifest_url` (zod string); C++: same via `first_data_item`, 422 on missing | Ghost: `AppInstallationPreview` (6 fields), 200, `cacheInvalidate:false`; C++: same body, 200/400/404/409/422/500, no cache header | Ghost: `service.preview(url)` — no store write; C++: outbox-recorded challenge/fetch/review/icon, no mutation | none on both sides | Ghost: `authAdminApi` + labs `apps` + `{app_installations, add}`; C++: `require_auth` + `require_ghost_permission("app_installations","add")`; labs residual | PARTIAL | Ghost `routes.js:232`, controller `:18-28`, service `:195-198,345-371,378-414`; C++ preview handler/step, `apps::support::preview` |

## Route details

### POST `/apps/installations/preview` — PARTIAL

Ghost facts (1–8) unchanged from `docs/parity/behavior/app-installation-previews.md`. C++ side
after this pass:

1. **Route identity**: `Route{"/installations/preview"}.post(...)` in `AppsRoutes::install`
   (`external/apps/routes.cppm:64-73`). Router keeps `require_auth` only; the former
   `content_type_middleware` 415 gate was removed (Ghost has no content-type guard here).
2. **Operation mapping**: `add`, docName `app_installation_previews`; step
   `"apps_app_installation_previews_add_execute"`, `Output=std::map<string,string>`, static
   `s_ctx`.
3. **Input parity**: `data.app_installation_previews[0].manifest_url`; missing/empty → 422
   Ghost `ValidationError` "Expected the manifest_url to preview." with
   `property:"manifest_url"`.
4. **Output parity**: full `AppInstallationPreview` — `manifest_url`, `manifest`, `digest`,
   `icon_data_url` (string|null), `trusted_origins` (array), `installation` (null or
   `{id,status,revision,manifest_url,manifest,icon_data_url,changes[]}`) — produced by
   `support::preview` from outbox-recorded fixture manifest/icon rows; no `data/meta` wrapper.
5. **DB call parity**: no store mutation (Ghost stores nothing); `load` emits
   `identity.challenge` + `manifest.fetch` and reads the newest fixture manifest row,
   `review` checks other-app/identity, `previewOf` builds the icon/diff.
6. **Cache parity**: `cacheInvalidate:false` → no `X-Cache-Invalidate` header; the old literal
   `false` write was removed.
7. **Auth / permissions**: `require_auth` plus
   `require_ghost_permission("app_installations","add")` via `core::auth::ghost`; labs gate is
   a residual.
8. **Verdict**: PARTIAL — controller behavior reproduced; infrastructure residuals remain.

#### Fix (implemented; supersedes the original route-add plan)

1. **Check 1 — route**: existing binding reused; only the router middleware changed.
2. **Check 2 — operation**: existing handler/flow/step reused; no new symbols or central
   `all.cppm` edits needed.
3. **Checks 3, 5 — input + DB call**: `JsonCodec` + `first_data_item` frame unwrap; 422 on
   bad input; `support::preview` performs the load/review/previewOf chain with outbox events
   and no store mutation.
4. **Check 4 — output**: full 6-field `AppInstallationPreview` with `changes[]` diff and
   nullable installation; no envelope.
5. **Check 6 — cache**: no cache header (cacheInvalidate:false); no cache key/TTL.
6. **Check 7 — auth**: `require_ghost_permission("app_installations","add")`; labs residual.
7. **Check 8 — evidence**: the three `.cppm` files compile in
   `bazel build //include/blog/handlers:blog_handlers`; target still fails only in unrelated
   partitions and the known Bazel cycle, untouched by this group.

## Missing in C++

None. Ghost `POST /apps/installations/preview` has a C++ counterpart. The original MISSING
verdict was closed when the route/handler/step trio was added; this pass made its behavior
strict-parity.

## Extra in C++

`GET /apps` → `apps_all_collection` belongs to the external apps-catalog domain and has no
counterpart in this controller, but is not a duplicate of the preview route. Keep it
installed — do not delete or gate it. No change made.

## Central registrations needed (parent session)

No new partition was added. The shared `blog::handlers::apps::support` helpers used by the
preview step live in the already-registered `:apps_app_installations_browse_step` partition.
The preview handler/flow/step partitions and route binding were already registered centrally
in `include/blog/handlers/BUILD.bazel` and `all.cppm`; no shared file was edited by this
group. Recorded new-partition need: none.
