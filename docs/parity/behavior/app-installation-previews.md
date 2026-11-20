# Behavior report: app-installation-previews

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/app-installation-previews.ts`
- Ghost route table: `docs/Ghost/ghost/core/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/apps/app_installation_previews_add/` (route in `include/blog/handlers/external/apps/routes.cppm`)
- Analyzed: 2026-11-19; strict-parity pass: 2026-11-20 (group 23)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

One route: Ghost `POST /ghost/api/admin/apps/installations/preview` ↔ C++ `POST /apps/installations/preview`.
The strict-parity pass replaces the `{"manifest_url":...}` echo with the full
`AppInstallationPreview` contract (manifest, digest, icon data URL, trusted origins,
current-installation diff) produced through shared abstractions and outbox-recorded fetches.
Verdict stays **PARTIAL**: the labs `apps` gate and real HTTP/crypto manifest/icon work have
no allowed shared primitive (see residual gaps).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/ghost/api/admin/apps/installations/preview` | `add`, docName `app_installation_previews` | body `data.app_installation_previews[0].manifest_url` (zod string, non-empty array framework-enforced) | `authAdminApi` on `/apps` + labs `apps`; permission `{docName:'app_installations', method:'add'}` | 200; no `X-Cache-Invalidate`; admin `Cache-Control: private` | `appInstallations.service.preview` → `load` (challenge + manifest fetch/verify) → `review` (identity check) → `previewOf` (icon fetch, approval diff) | no cache service; `cacheInvalidate:false` | `AppInstallationPreview`: `manifest_url`, `manifest`, `digest`, `icon_data_url`, `trusted_origins`, nullable `installation{}` | `routes.cppm:64-73`, `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm` (+ shared `apps::support`) | validates required field (Ghost 422), outbox-recorded challenge/fetch/review/icon, full 6-field preview body incl. `changes[]` diff and nullable installation; permission gate; no cache header | PARTIAL | Ghost `routes.js:232`, controller `:18-27`, service `:195-198,345-371,378-414`; C++ preview step, `support::preview/load/review/preview_of` |

## Route details

### POST `/apps/installations/preview` — PARTIAL

**Ghost** (numbered per checklist 1–8): unchanged from the original analysis —
`PreviewRequest = z.object({manifest_url: z.string()})` on element 0 (422 on failure);
`permissions:{docName:'app_installations',method:'add'}` because the handler fetches a URL;
`headers:{cacheInvalidate:false}`; `service.preview(manifestUrl)` → `load` (identity challenge
+ signed-manifest fetch/verify) → `review` (identity/trusted-origin check against current
installation) → `previewOf` (icon fetch; no current → `installation:null`; else pending
manifest + `approvalWrite` + `changesFrom` diff). Stores nothing; 200 with the
`AppInstallationPreview` object directly (no envelope serializer).

**C++** (strict-parity pass):
1. **Route/middleware.** `Router{"/apps"}.use(require_auth)` and
   `Route{"/installations/preview"}.post(...)` (`routes.cppm:64-73`); the former
   `content_type_middleware` 415 gate was removed (Ghost has no content-type guard on this
   route). Permission enforced in the handler via
   `require_ghost_permission(req,res,next,send,"app_installations","add")`.
2. **Files/symbols.** `AppInstallationPreviewsAddHandler`, single-step workflow
   `"apps_app_installation_previews_add"`, `Output=std::map<string,string>` (`status`/`body`),
   static `s_ctx`.
3. **Input parsing.** `core_serde::JsonCodec{}.decode<serde::Value>` + shared
   `first_data_item(*,"app_installation_previews")`; missing/non-string/empty `manifest_url`
   → 422 Ghost `ValidationError` "Expected the manifest_url to preview." with
   `property:"manifest_url"` (mirrors zod `safeParse` failure).
4. **`support::preview`.** `load` emits `identity.challenge` + `manifest.fetch` outbox events
   and reads the newest fixture manifest row for the URL; `review` checks other-app/identity
   key and supplies `trusted_origins` (coded `APP_MANIFEST_UNREACHABLE`/`APP_IDENTITY_MISMATCH`
   errors); `previewOf` fetches the icon (`icon.fetch` outbox event) and builds the 6-field
   preview. With no current installation → `icon_data_url` from the fetch, `installation:null`;
   with one → kept stored icon and `installation:{id,status,revision,manifest_url,manifest,
   icon_data_url,changes[]}` where `changes[]` is a recursive manifest diff
   (`{path,requires_approval}`).
5. **Status/body/header writes.** Success → 200 with the full preview object (no
   `data/meta` wrapper). 400/404/409/422/500 mapped in the handler. No `X-Cache-Invalidate`
   header (`cacheInvalidate:false`); no `Location`/`Cache-Control`/`Content-Disposition`.
6. **`ctx.store()`/cache.** No mutation, no cache key/TTL — Ghost stores nothing. All
   external work is outbox-recorded; no new global state.
7. **Side effects.** Outbox events only (`identity.challenge`, `manifest.fetch`, `icon.fetch`);
   no fixture writes, no events/jobs sent externally.
8. **Error body shape.** Full Ghost envelope with `code`/`context`/`details`/`errorDetails`
   and `ValidationError`/`ConflictError` types via `support::ghost_error`.

**Diff** (residuals, not divergences):
- Real remote signed-manifest fetch, signature verification, RFC 7638 thumbprints, the
  challenge answer, and the real `fetchIcon` type/size rules have no allowed C++ primitive —
  outbox-recorded fixture stand-ins; labs gate and token→role resolution are residuals.
- Former `{"manifest_url":...}` stub echo and literal `X-Cache-Invalidate:false` header were
  replaced with the full preview payload and no cache header.
- Input validation still uses `JsonCodec` walking rather than zod, but produces the same
  Ghost 422 envelope/message/property.

## Module-level notes

- Single route in scope. `GET /apps` (`apps_all_collection`) remains an adjacent C++ route
  from a different Ghost domain; keep it installed.
- The handler runs a `LocalWorkflowDriver` single-step flow; `compensate` is a no-op —
  consistent with Ghost's "stores nothing" preview.
- Both sides agree on the request contract and on a 422 for a missing `manifest_url`, and
  signal no cache invalidation. Response payload and all service-side fields now match the
  Ghost shape through fixture-backed shared abstractions.
- Shared abstractions used: `core_query` (via shared support), `core_serde`,
  `core_auth::ghost` + `blog_handlers_middleware:ghost_permission`, `blog::handlers::outbox`.
  No new partition needed — helpers live in the registered `:apps_app_installations_browse_step`
  partition.
