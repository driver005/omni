# Behavior report: media

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/media.js:1-50`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:468-482`
- C++ folders: `include/blog/handlers/system/media/` (`routes.cppm` + `media_upload_post/`,
  `media_thumbnail_upload_put/` — each handler + workflow/flow + workflow/step). Installer
  `include/blog/handlers/routes.cppm:33,105`. Shared store/cache
  `include/blog/handlers/context/context_context.cppm`. Outbox
  `include/blog/handlers/outbox_shim.cppm`. Middleware
  `include/blog/handlers/middleware/{auth,serde,bearer}.cppm`.
- Analyzed: 2026-11-19. Strict-parity rewrite: 2026-11-20 (Group 52).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Rubric (consistent with `docs/parity/behavior/invites.md:21-24`): `PARTIAL` = same verb/path/op,
primary success status agrees, but side effects / output shape / internals diverge or are absent.
Both media routes have a C++ counterpart (no `MISSING`); C++ adds no media route with no Ghost
counterpart (no `EXTRA`). The separate C++ `system/db/db_media_inline_post` route maps to Ghost's
`db.inlineMedia` controller (`admin/routes.js:401`), a different controller, so it is out of scope
here (see Module-level notes).

**Verdict flips in the strict-parity pass: none.** Both rows remain `PARTIAL`: the observable
contract now matches Ghost (output envelope, statuses/messages, multipart acceptance, headers,
`ref`, thumbnail naming, `getURL`), but file bytes are still not persisted by a real
`storage:media` adapter. That residual gap needs a new shared partition (see "New-partition
needs"); flipping to `MATCH` would overclaim.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/ghost/api/admin/media/upload` | `upload`, docName `media` | multipart fields `file` (max 1) + `thumbnail` (max 1) + form field `ref`; controller reads `frame.files.file[0]`, `frame.files.thumbnail[0]`; no `options[]`/`data[]` on controller | `mw.authAdminApi` (7-stage chain) + `apiMw.upload.media` + `apiMw.upload.mediaValidation({type:'media'})`; `permissions:false` | **201**; **no** `X-Cache-Invalidate`; `Cache-Control: no-cache, private, no-store, …` from admin app | `adapterManager.getAdapter('storage:media')` → `storage.save(thumbnail)` then `storage.save(file)` → local `LocalStorageBase.save` / S3 `save`; limit check in input validator | no api-framework `cache`; no cache key/TTL | `{"media":[{url,thumbnail_url,ref}]}` via `utils/serializers/output/media.js` `upload` + `getURL` | `system/media/media_upload_post/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 201; multipart parse, Ghost `uploads.media`/`uploads.thumbnails` MIME+ext allowlists, SVG sanitize + image sniff, `<main>_thumb<ext>` name, `/content/media/YYYY/MM/` paths, metadata `ctx.store().create("media",…)` + `media.saved` outbox events, `{"media":[{url,thumbnail_url,ref}]}` with `getURL`, no `X-Cache-Invalidate`, private `Cache-Control`, `require_auth` only | PARTIAL | Ghost `media.js:8-29`, `admin/routes.js:469-475`, `upload.js:339-421`, `output/media.js:4-29`; C++ `media_upload_post/.../execute.cppm:167-506`, `handler.cppm:27-32`, `system/media/routes.cppm:20-26` |
| PUT `/ghost/api/admin/media/thumbnail/upload` | `uploadThumbnail`, docName `media` | `data:['url','ref']` (multipart form fields / query / params); single multipart `file`; input serializer renames `file.name` to `<base>_thumb<ext>` | `mw.authAdminApi` + `apiMw.upload.single('file')` + `apiMw.upload.validation({type:'images'})`; `permissions:false` | **200** (default); **no** `X-Cache-Invalidate`; admin-app private `Cache-Control` | `mediaStorage.urlToPath(frame.data.url)` → `path.dirname` → `mediaStorage.save(frame.file, targetDir)`; limit check in input validator | no api-framework `cache`; no key/TTL | `{"media":[{url,ref}]}` via output serializer `uploadThumbnail` + `getURL` | `system/media/media_thumbnail_upload_put/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200; `url`/`ref` from multipart fields with query fallback, `urlToPath`/dirname media-prefix validation, Ghost `uploads.images` MIME+ext allowlists, SVG sanitize + image sniff, `<parent>_thumb<ext>` name with collision-safe uniqueness, metadata `ctx.store().create("media",…)` + `media.thumbnail_saved` outbox event, `{"media":[{url,ref}]}` with `getURL`, private `Cache-Control`, `require_auth` only | PARTIAL | Ghost `media.js:31-47`, `admin/routes.js:476-482`, `input/media.ts:7-10`, `output/media.js:31-40`; C++ `media_thumbnail_upload_put/.../execute.cppm:152-500`, `handler.cppm:27-32`, `system/media/routes.cppm:27-37` |

## Route details

### POST /ghost/api/admin/media/upload — PARTIAL

**Ghost**
1. Identity: `POST /media/upload` (`admin/routes.js:469-475`), mounted under
   `/ghost/api/admin` (`parent/backend.js:15`, `web/api/app.js:22`, `admin/app.js:24` installs
   `cacheControl('private')`). Route chain: `mw.authAdminApi` → `apiMw.upload.media('file','thumbnail')`
   → `apiMw.upload.mediaValidation({type:'media'})` → `http(api.media.upload)`.
2. Operation: `upload`, docName `media` (`media.js:7,8`), `permissions:false` (`:13`).
3. Input: multipart `file` + optional `thumbnail`, `maxCount:1` (`upload.js:175-186`); form field
   `ref` reaches `frame.data.ref` and the output serializer. `mediaValidation`
   (`upload.js:339-421`) requires a file (422 "Please select a media file."), checks MIME **and**
   extension against `config.uploads.media` (`overrides.json:53-69`) → 415 "Please select a valid
   media file.", validates the optional thumbnail against `config.uploads.thumbnails`
   (`overrides.json:179-190`) with SVG sanitize/content sniff → 415 "Please select a valid
   thumbnail." / "Please select a valid SVG image". Broken body → 400
   "The request could not be understood." (`e2e-api/admin/media.test.js`).
4. Auth/permissions: `authAdminApi` 7-stage chain; `permissions:false` ⇒ no `canThis`.
5. Status/headers: 201 (`media.js:9`); `headers.cacheInvalidate:false` (`:10-12`) ⇒ no
   `X-Cache-Invalidate`; admin app `Cache-Control: private` (`admin/app.js:24`).
6. Call chain: `adapterManager.getAdapter('storage:media')` → `storage.save(thumbnail)` first,
   then `storage.save(file)` (`media.js:15-22`) → local `LocalStorageBase.save` (mkdirs + copy,
   `/content/media/YYYY/MM/<name>`, unique name on collision) or S3 `save`. `limitService`
   upload-limit check in `validators/input/media.ts:7-9`. `save()` generates a unique filename so
   an upload never replaces a file (`media.js:20-21`).
7. Cache: none; invalidation suppressed.
8. Output: `{"media":[{url:getURL(filePath), thumbnail_url:getURL(thumbnailPath), ref:frame.data.ref||null}]}`
   (`output/media.js:19-29`); `getURL` (`:4-16`) prefixes site URL for paths under
   `<subdir>/content/media`, passes CDN/absolute URLs through; null thumbnail → null URL.

**C++** (strict-parity pass)
1. Route: `Router{router,"/media"}.use(require_auth)` (`system/media/routes.cppm:25-26`).
   `content_type_middleware` was **removed**: it rejected every genuine `multipart/form-data`
   upload with 415 before parsing. Ghost has no JSON-only gate on these routes. `require_auth`
   mirrors `authAdminApi`'s authentication role; `permissions:false` means no permission check
   belongs here.
2. Operation: `MediaUploadPostHandler` (`handler.cppm:12-32`) runs workflow
   `media_media_upload_post` with `ExecuteStep`; `Output` is
   `std::map<std::string,std::string>` (`status`/`body`), `static s_ctx` retained.
3. Input (`execute.cppm:167-506`): multipart parser splits on the `boundary=` from
   `in.get_content_type()`, captures `name`/`filename`/`Content-Type` and non-file fields
   (`ref`); a body without a boundary or closing `--boundary--` → 400
   "The request could not be understood." Missing `file` → 422 "Please select a media file.";
   disallowed MIME **or** extension → 415 "Please select a valid media file." (allowlists mirror
   `overrides.json:53-69`). Optional thumbnail: 422 "Please select a thumbnail." when present but
   empty, 415 "Please select a valid thumbnail." / "Please select a valid SVG image" on
   validation failure; `sanitize_svg` (`:320`) strips `<script>`/event handlers/`javascript:` and
   rejects a missing `<svg>` root; `allowed_image_content` (`:383`) checks image magic bytes.
4. Status/headers: step returns `{"status":"201", "body":…}` (`execute.cppm:506`); handler writes
   it verbatim plus the admin-app private `Cache-Control` (`handler.cppm:30-32`). No
   `X-Cache-Invalidate` (matches Ghost suppression).
5. Call chain / side effects: paths `/content/media/<YYYY>/<MM>/<unique>` (`current_year_month`
   `:412`, `unique_name` keeps the original name and adds `-n` on collision). Thumbnail saved
   before the main file (`:487-498`), each as `ctx.store().create("media",{name,path,type})`
   **and** a `media.saved` event on the shared `blog_handlers:outbox_shim` (`record_save` `:428`).
   The outbox event is the observable stand-in for Ghost's `storage.save()` side effect; no file
   bytes are written.
6. Output (`execute.cppm:502-506`): `{"media":[{"url":…,"thumbnail_url":…|null,"ref":…|null}]}`.
   `get_url` (`:397`) prefixes `scheme://authority` from the request for `/content/media/…` paths
   and passes CDN/absolute paths through, mirroring `getURL`.
7. Cache: none; no cache key/invalidation (matches Ghost).

**Diff** — closed: output envelope + `getURL` + `ref`; 201 status; 422/415/400 statuses and Ghost
messages; MIME **and** extension allowlists; SVG sanitize + image content sniff; `<main>_thumb<ext>`
thumbnail naming; `YYYY/MM` paths; multipart acceptance (no JSON-only gate); private
`Cache-Control`; authentication-only middleware; observable outbox side effects. Residual gaps:
real `storage:media` adapter with byte writes and adapter-generated unique names, multer temp-file
lifecycle, config-driven allowlists (defaults are hard-coded from `overrides.json`),
`limitService` upload limit, full XML SVG sanitizer, and `authAdminApi`'s 7-stage chain. See
"New-partition needs".

### PUT /ghost/api/admin/media/thumbnail/upload — PARTIAL

**Ghost**
1. Identity: `PUT /media/thumbnail/upload` (`admin/routes.js:476-482`); chain `mw.authAdminApi` →
   `apiMw.upload.single('file')` → `apiMw.upload.validation({type:'images'})` →
   `http(api.media.uploadThumbnail)`; `permissions:false` (`media.js:35`).
2. Operation: `uploadThumbnail`, docName `media` (`media.js:31,7`).
3. Input: `data:['url','ref']` (`media.js:36`) from query/params/body; single multipart `file`.
   `validation({type:'images'})` (`upload.js:262-331`) → 422 "Please select an image." /
   415 "Please select a valid image."; SVG sanitize / content sniff in the same path. Input
   serializer `media.uploadThumbnail` (`serializers/input/media.ts:6-10`) renames the file to
   `<basename(url)>_thumb<ext>`. `limitService` check in `validators/input/media.ts:11-13`.
4. Auth/permissions: `authAdminApi`; `permissions:false` ⇒ no `canThis`.
5. Status/headers: default 200; `cacheInvalidate:false` (`media.js:32-34`) ⇒ no
   `X-Cache-Invalidate`; admin-app private `Cache-Control`.
6. Call chain: `mediaStorage = adapterManager.getAdapter('storage:media')`;
   `targetDir = path.dirname(mediaStorage.urlToPath(frame.data.url))` (`media.js:39`; a URL
   outside the media prefix throws `IncorrectUsageError`), then `mediaStorage.save(frame.file,
   targetDir)` (`:45`); `save()` generates a unique filename (`:41-44`).
7. Cache: none; invalidation suppressed.
8. Output: `{"media":[{url:getURL(path), ref:frame.data.ref||null}]}` (`output/media.js:31-40`).

**C++** (strict-parity pass)
1. Route: nested `Router{router,"/thumbnail"}.use(require_auth).add_route(Route{"/upload"}.put(...))`
   under `/media` (`system/media/routes.cppm:27-37`); no JSON-only content-type gate.
2. Operation: `MediaThumbnailUploadPutHandler` (`handler.cppm:12-32`) runs
   `media_media_thumbnail_upload_put` with `ExecuteStep`; `Output` map, `static s_ctx`.
3. Input (`execute.cppm:152-500`): multipart parse (same rules as upload); `url`/`ref` read from
   multipart fields first, then query params (`query_params` `:237`). Missing `url` → 422 "url is
   required". `media_target_dir` (`:274`) strips origin/query and requires `/content/media/…`,
   returning `dirname` or a 400 `IncorrectUsageError` with Ghost's message. `file` required →
   422 "Please select an image."; MIME+ext against `uploads.images` → 415; SVG sanitize /
   content sniff as on upload.
4. Status/headers: step returns `{"status":"200","body":…}` (`execute.cppm:500`); handler writes
   it plus private `Cache-Control`, no `X-Cache-Invalidate` (`handler.cppm:30-32`).
5. Call chain / side effects: `thumbName = stem(url) + "_thumb" + ext(file.filename)`
   (`execute.cppm:493`, mirroring `input/media.ts:6-10`); `storedPath = targetDir + "/" +
   unique_name(thumbName)` with a process-local used-name set so repeated uploads do not collide
   (`:300-309`, Ghost `media.js:41-44`). Metadata `ctx.store().create("media",{name,path,type})`
   plus a `media.thumbnail_saved` outbox event (`record_save` `:420`); no byte write.
6. Output (`execute.cppm:498-500`): `{"media":[{"url":…,"ref":…|null}]}` with `get_url`
   (`:401`) prefixing the request origin for media paths.

**Diff** — closed: output envelope + `getURL` + `ref`; 200 status; `url`/`ref` from multipart
fields; `urlToPath`-style prefix validation with Ghost's error; image MIME+ext allowlist, SVG
sanitize + content sniff; `<parent>_thumb<ext>` naming with collision-safe uniqueness; multipart
acceptance; private `Cache-Control`; authentication-only middleware; observable outbox side
effect. Residual gaps: real adapter `save`/`urlToPath` and byte persistence, config-driven
allowlists, `limitService`, full XML SVG sanitizer, `authAdminApi` chain, and exact
`config.getSiteUrl()`/`getSubdir()` (site URL is derived from the request origin).

## Module-level notes

- Both Ghost routes carry `permissions:false` (`media.js:13,35`): `authAdminApi` authenticates
  but no `canThis` stage runs. C++ uses `require_auth` only (`system/media/routes.cppm:26,29`).
- The two media routers no longer install `content_type_middleware` (`middleware/serde.cppm:90-126`),
  which is JSON-only and contradicted the multipart parsers. No other route is affected.
- Every save is represented twice: an in-memory `media` metadata record (the existing
  `ResourceStore::create` stand-in) and an outbox event via the already-registered
  `blog_handlers:outbox_shim` partition (`record_save`). No shared file or BUILD edit was needed.
  Thumbnail-first ordering is preserved on upload (`execute.cppm:487-498`).
- Step `Output` remains `std::map<std::string,std::string>` (`status`/`body`) and `static s_ctx`
  remains in both steps; no new module partition or DTO was introduced.
- Both handlers write the full admin-app private `Cache-Control` value
  (`web/shared/middleware/cache-control.ts:16-19`) via `Token::CACHE_CONTROL` and write no
  `X-Cache-Invalidate` (`media.js:10-12,32-34`).
- Shared side effects with no C++ primitive: multer temp-file lifecycle (`upload.js:72,206-223`),
  `adapter-manager` resolution and real local/S3 writes (`LocalStorageBase.ts:152-189`,
  `S3Storage.ts:205`), `limitService.errorIfIsOverLimit('uploads', …)`
  (`validators/input/media.ts:7-13`), and the full `sanitizeSvgFile`/`isAllowedImageContent`
  implementations.
- `system/db/db_media_inline_post/` maps to Ghost `api.db.inlineMedia` (`admin/routes.js:401`),
  not `media.js`, and is intentionally excluded from this module's verdicts.

## New-partition needs (recorded, not created)

These would be required to close the residual storage/config gaps; none was added because the
task forbids shared-file/Bazel edits and new partition registration:

1. `core_adapters:storage_media` — a new partition under `include/core/adapters/` exposing
   `save(file, targetDir)`, `urlToPath(url)`, `getUniqueFileName(file, dir)` and real byte writes
   (local fs + S3). Needs a new file plus a `partitions = [...]` entry in `core_adapters/BUILD`
   and main-agent wiring into `blog_handlers`. Would replace `record_save`'s in-memory/outbox
   stand-in.
2. A shared config accessor exposing `getSubdir()`, `STATIC_MEDIA_URL_PREFIX`, `getSiteUrl()` and
   `uploads.{media,images,thumbnails}.{extensions,contentTypes}` (new partition in
   `include/core/config/`). Would replace request-origin `getURL` and the hard-coded allowlists.
3. Optional `blog_handlers_middleware:media_upload` partition for multipart parse/validation so
   the two steps no longer duplicate private helpers. Current helpers live in the already-registered
   step partitions (`media_upload_post/.../execute.cppm`,
   `media_thumbnail_upload_put/.../execute.cppm`), so no registration is needed today.
