# Parity: media

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/media.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/media/routes.cppm`
- Checked: 2026-11-19. Strict-parity rewrite: 2026-11-20 (Group 52).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

**Verdict flips: none.** Both rows stay `PARTIAL`. The rewrite closes the observable divergences
(output envelope/`getURL`/`ref`, status and error statuses/messages, multipart acceptance,
`Cache-Control`, thumbnail naming, outbox side effects) but cannot write real file bytes without a
new shared `storage:media` adapter partition. Flipping to `MATCH` would overclaim.

## Applied strict-parity source fixes (2026-11-20)

Folder-and-docs-only pass. No shared file, BUILD file, central route/DTO, or middleware file was
touched. Helpers live in the two existing registered step partitions; no new partition was added.
Every step keeps `Output = std::map<std::string,std::string>` (`status`/`body`) and `static s_ctx`.

### POST /media/upload
- `routes.cppm`: removed `.use(content_type_middleware)` from the `/media` router. That middleware
  is `application/json`-only (`middleware/serde.cppm:97-122`) and rejected every real
  `multipart/form-data` upload with 415 before the step ran. Ghost has no such gate
  (`admin/routes.js:469-475`). `require_auth` remains; Ghost sets `permissions:false`
  (`media.js:13`), so no `canThis` stage belongs on this route.
- `workflow/step/execute.cppm`: multipart parse (boundary, `name`/`filename`/`Content-Type`,
  non-file `ref` field), 400 for a broken body, 422 "Please select a media file." / 415
  "Please select a valid media file." using Ghost's `uploads.media` MIME **and** extension
  allowlists (`shared/config/overrides.json:53-69`); optional thumbnail validated against
  `uploads.thumbnails` (`overrides.json:179-190`) with std-only SVG sanitize + image magic-byte
  sniff; thumbnail stored as `<main base>_thumb<ext>`; `/content/media/<YYYY>/<MM>/` paths with
  original name kept and `-n` on collision; metadata `ctx.store().create("media",{name,path,type})`
  plus a `media.saved` event on `blog_handlers:outbox_shim`; output
  `{"media":[{url,thumbnail_url,ref}]}` with `getURL` (site origin from request), status 201.
- `handler.cppm`: emits step `status`/`body` verbatim and writes the admin-app private
  `Cache-Control` (`web/shared/middleware/cache-control.ts:16-19`); no `X-Cache-Invalidate`.

### PUT /media/thumbnail/upload
- `routes.cppm`: nested `/thumbnail` router keeps `require_auth` and drops the JSON-only gate.
- `workflow/step/execute.cppm`: `url`/`ref` from multipart fields with query fallback; `url`
  required (422); `urlToPath`-style prefix validation of `/content/media/…` with Ghost's
  `IncorrectUsageError` message; single `file` with Ghost `uploads.images` MIME+ext allowlist, SVG
  sanitize + image sniff; name `<parent base>_thumb<ext>` from `url` (input serializer
  `serializers/input/media.ts:6-10`) with collision-safe uniqueness; metadata store create plus a
  `media.thumbnail_saved` outbox event; output `{"media":[{url,ref}]}` with `getURL`, status 200.
- `handler.cppm`: emits step `status`/`body` verbatim and writes private `Cache-Control`.

### Reused shared abstractions
- `blog_handlers:outbox_shim` (already a registered partition, no BUILD change) — observable
  stand-in for `storage.save()` side effects. `blog_handlers_middleware::require_auth` — the
  authentication half of `authAdminApi`; `permissions:false` makes a permission service
  unnecessary here. `blog::api::ApiError::validation` — Ghost error envelope with exact messages
  and `ValidationError`/`UnsupportedMediaTypeError`/`BadRequestError`/`IncorrectUsageError` types.
- No `core_outbox`, `core_adapters` or `blog_handlers:mapping` import was added: those are separate
  Bazel targets with no dependency edge from `blog_handlers`, and the task forbids wiring/Bazel
  cycle fixes.

### Residual gaps (documented, not invented)
- No real byte storage: metadata + outbox stand in for `storage.save`; filenames are original + `-n`
  rather than adapter-generated; no multer temp-file lifecycle.
- Allowlists mirror Ghost defaults from `overrides.json` but are not read from live config.
- `limitService.errorIfIsOverLimit('uploads', …)` has no C++ primitive.
- `getURL` uses request `scheme://authority` for the site URL; `config.getSubdir()` and
  `STATIC_MEDIA_URL_PREFIX` are not exposed to handlers.
- SVG sanitizer is a std-only subset (script blocks, event handlers, `javascript:` URLs, malformed
  root) rather than Ghost's XML sanitizer.
- Full `authAdminApi` 7-stage chain is not reproduced; only authentication is (permissions are
  false in Ghost).

## New-partition needs (recorded, not created)

1. `core_adapters:storage_media` — `save`/`urlToPath`/`getUniqueFileName` with real local/S3 byte
   writes; needs a new `include/core/adapters/storage_media.cppm`, a BUILD partition entry, and
   main-agent wiring. Would replace `record_save`.
2. Shared config accessor partition (`include/core/config/`) for `getSubdir()`,
   `STATIC_MEDIA_URL_PREFIX`, `getSiteUrl()` and `uploads.*` allowlists. Would replace the
   request-origin `getURL` and hard-coded allowlists.
3. Optional `blog_handlers_middleware:media_upload` partition to de-duplicate multipart helpers;
   today the helpers are private statics inside the two already-registered step partitions.

## Central registrations

No new central registration is required.
- No new module partition / source file was added, so nothing to register in
  `include/blog/handlers/{all.cppm,domain_all.cppm,BUILD.bazel}`.
- `blog::handlers::middleware::require_auth` is already centrally exported by
  `blog_handlers_middleware` (`middleware/auth.cppm`) and imported by `routes.cppm`.
- Both step/handler/flow modules keep their existing `export module` declarations, `MainFlow`
  builders and `Workflow` registration names.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST /media/upload | `upload` | POST /media/upload | `media_media_upload_post()` | Ghost: multipart `file`, `thumbnail`, `ref` + `mediaValidation({type:'media'})`; C++: multipart parse, media MIME+ext allowlists, SVG sanitize + sniff, 400/422/415 Ghost messages | Ghost: `{media:[{url,thumbnail_url,ref}]}`, 201; C++: same envelope + `getURL`, 201 | Ghost: `storage:media` adapter `save` (thumbnail then file); C++: `ctx.store().create("media",{name,path,type})` + `media.saved` outbox event (residual: no byte storage) | Ghost: none (`cacheInvalidate:false`); C++: none, private `Cache-Control` only | Ghost: `mw.authAdminApi`, `permissions:false`; C++: `require_auth` (401), no JSON-only gate, no in-handler token check | PARTIAL | Ghost `routes.js:469-475`, `media.js:8-29`; C++ `routes.cppm:25-26`, `handler.cppm:27-32`, `execute.cppm:167-506` |
| PUT /media/thumbnail/upload | `uploadThumbnail` | PUT /media/thumbnail/upload | `media_media_thumbnail_upload_put()` | Ghost: `data:['url','ref']` + single multipart `file` + `validation({type:'images'})` + input serializer name; C++: multipart/query `url`/`ref`, media-prefix `urlToPath` stand-in, images MIME+ext allowlists, `<parent>_thumb<ext>` name | Ghost: `storage.save` result → `{media:[{url,ref}]}`, 200; C++: same envelope + `getURL`, 200 | Ghost: `urlToPath`/`save` adapter; C++: `ctx.store().create("media",…)` + `media.thumbnail_saved` outbox event (residual: no byte storage) | Ghost: none (`cacheInvalidate:false`); C++: none, private `Cache-Control` only | Ghost: `mw.authAdminApi`, `permissions:false`; C++: `require_auth` (401), no JSON-only gate, no in-handler token check | PARTIAL | Ghost `routes.js:476-482`, `media.js:31-47`; C++ `routes.cppm:27-37`, `handler.cppm:27-32`, `execute.cppm:152-500` |

## Route details

### POST /media/upload — PARTIAL
- Route identity: MATCH — `router.post('/media/upload')` (`routes.js:469-475`) ↔
  `Route{"/upload"}.post(...)` under `Router{router,"/media"}` (`routes.cppm:25,42-49`).
- Operation: Ghost `api.media.upload` saves files via `storage:media` and returns paths
  (`media.js:14-28`); C++ saves metadata + outbox events and returns the serialized envelope.
  Difference is the storage implementation, not the observable contract.
- Input: Ghost consumes `frame.files.file[0]` / `thumbnail[0]` and `frame.data.ref`
  (`media.js:18-22`, `output/media.js:25`); C++ parses the same multipart fields and `ref`
  (`execute.cppm:167-220,500-505`).
- Output: both `{media:[{url,thumbnail_url,ref}]}` with 201; C++ `getURL` derives the site origin
  from the request (`execute.cppm:397-409`).
- DB/storage: Ghost adapter `save` (thumbnail first); C++ metadata `create` + `media.saved`
  outbox event per save (`execute.cppm:428-434,487-498`). Residual: no file bytes.
- Cache: none on both sides; C++ writes only the admin-app private `Cache-Control`.
- Auth: Ghost `authAdminApi` with `permissions:false`; C++ `require_auth` only (`routes.cppm:26`);
  the previous in-handler token check was already removed.

### PUT /media/thumbnail/upload — PARTIAL
- Route identity: MATCH — `router.put('/media/thumbnail/upload')` (`routes.js:476-482`) ↔ nested
  `Router{router,"/thumbnail"}.add_route(Route{"/upload"}.put(...))` (`routes.cppm:27-37`).
- Operation: Ghost computes `path.dirname(mediaStorage.urlToPath(url))` and saves there
  (`media.js:37-46`); C++ uses `media_target_dir` + `unique_name` (`execute.cppm:274-309`) and the
  same `<parent>_thumb<ext>` name from the input serializer.
- Input: `url`/`ref` from multipart fields with query fallback, single `file`, images allowlist,
  SVG sanitize + content sniff (`execute.cppm:445-489`).
- Output: both `{media:[{url,ref}]}` with 200; C++ `getURL` prefixes the request origin.
- DB/storage: C++ metadata `create` + `media.thumbnail_saved` outbox event
  (`execute.cppm:420-426,495`); no byte write.
- Cache: none on both sides; private `Cache-Control` only on C++.
- Auth: `require_auth` only, matching Ghost's `permissions:false`.

## Missing in C++

None. Both Ghost controller routes have a C++ route in `include/blog/handlers/system/media/routes.cppm`
(POST `/media/upload` at `routes.js:469-475`; PUT `/media/thumbnail/upload` at `routes.js:476-482`).

  Fix: No change required. Check 1 is already closed for every entry — no Ghost route lacks a C++
  registration, so nothing to add. The work is to make the two existing handlers exact, done by the
  Fix blocks above, not to add routes.

## Extra in C++

None. The matched folder installs exactly two routes (`routes.cppm:25-49`), both corresponding to a
Ghost controller route. The related `db_db_media_inline_post` step
(`include/blog/handlers/system/db/db_media_inline_post/`) maps to Ghost `api.db.inlineMedia`
(`routes.js:401`), not to this controller.

  Fix: No change required. Check 1 is already closed — no extra C++ route exists to remove or gate.
