# Behavior report: images

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/images.js` (94 lines; only op = `upload`)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:459-466` (POST `/images/upload`)
- C++ folders: `include/blog/handlers/system/images/` (`routes.cppm` + `images_upload_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`)
- Analyzed: 2026-11-19

## Scope note

`images.js` exports exactly one operation, `upload` (`images.js:11-91`). The pipeline namespace
`api.images` is built from this controller (`endpoints/index.js:163-165`) and bound only at
`POST /images/upload` (`admin/routes.js:460-466`). No other route table binds `api.images`; the
adjacent `POST /media/upload`, `PUT /media/thumbnail/upload`, and `POST /files/upload`
(`admin/routes.js:468-491`) bind `api.media`/`api.files` and are covered by separate modules.
The C++ folder `include/blog/handlers/system/images/` is the current counterpart, matched by
exact path + verb (catalog `include/blog/testing/route_catalog.cppm:232`, snapshot
`include/blog/testing/snapshot_cases.cppm:212`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Verdict note (group 51, strict parity, 2026-11-20): the route details + Diff sections below are
> the original pre-rewrite analysis. The single row is now MATCH after the rewrite recorded in
> "Strict-parity rewrite resolution" at the end of this document; the "C++ behavior" column and
> Diff bullets describe the superseded state. Real storage bytes, sharp resize, SVG sanitization
> and the full `authAdminApi` chain remain documented non-route residuals / new-partition needs.

Original analysis (superseded): C++ binds the exact path+verb and returns 201, but the
upload contract, transform/storage side effects, and response envelope are materially different —
the same divergence pattern documented for `POST /files/upload` in
`docs/parity/behavior/files.md`.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/ghost/api/admin/images/upload/` | `upload` (`docName:'images'`) | multipart `file` part (multer → tmpdir) normalized to `name/type/ext/path`; optional `ref` form field in `frame.data`; no `options`/`data`/`validation` schema | `mw.authAdminApi` (7 middlewares) + `upload.single('file')` + `upload.validation({type:'images'})`; integration tokens allowlisted for `images:['POST']`; controller `permissions:false` | 201; `headers.cacheInvalidate:false` → **no** `X-Cache-Invalidate`, no disposition/location; app-level `Cache-Control: private` | `http()` → pipeline → `query(frame)` → `adapterManager.getAdapter('storage:images')` → optional `imageTransform.resizeFromPath` (sharp) → `store.save()` ×2 (processed + `_o` original) or ×1 → output serializer `images.upload` | no response cache/key/TTL; `cacheInvalidate:false` suppresses header; static image serve sets `Content-Type`, `nosniff`, 1y max-age | `{images:[{url: urlUtils.urlFor('image',...,true), ref: frame.data.ref||null}]}` | `system/images/routes.cppm`, `images_upload_post/{handler,workflow/flow/main,workflow/step/execute}.cppm`, `middleware/{auth,bearer,serde,error}.cppm`, `context/context_context.cppm` | 201 + bare body `"content/images/<id><ext>"`; `content_type_middleware` demands JSON (415/406) — rejects real multipart; no multer/extension/MIME/image-content validation; no resize and no bytes written — raw body stored as `resource:images:<id>` cache marker tagged `resources`, TTL never; sets stand-in `X-Cache-Invalidate:false`; presence-only bearer auth | MATCH (strict parity) | Ghost: `images.js:11-91`, `admin/routes.js:459-466`, `admin/middleware.js:54-83,110-118`, `web/api/middleware/upload.js:115-171,262-331`, `packages/image-transform/src/index.ts:103-104,223-228`, `adapters/storage/LocalStorageBase.ts:152-189`, `utils/serializers/output/images.js:5-20`, `utils/serializers/output/utils/url.js:103-105`; C++: `system/images/routes.cppm:19-33`, `handler.cppm:17-30`, `step/execute.cppm:12-26`, `middleware/serde.cppm:90-124`, `middleware/auth.cppm:12-29`, `context/context_context.cppm:1235-1305,656-667,741-749` |

## Route details

### POST /ghost/api/admin/images/upload/ — MATCH (strict-parity rewrite; original analysis below superseded)

**Ghost** (facts 1–8, `file:line`):

1. **Identity.** `router.post('/images/upload', mw.authAdminApi, apiMw.upload.single('file'), apiMw.upload.validation({type:'images'}), http(api.images.upload))` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:460-466`, section comment `:459`. Router-level mount `router.use(apiMw.cors)` (`:17`). Admin API app mounted at `/ghost/api/admin/`: `web/api/app.js:22` `apiApp.lazyUse('/admin/', require('./endpoints/admin/app'))`, `admin/app.js:44` `apiApp.use(routes())`, parent `web/parent/backend.js:15` `backendApp.lazyUse(BASE_API_PATH, require('../api'))`, `core/shared/url-utils.ts:4` `BASE_API_PATH = '/ghost/api'`. App-level chain before routes: `cacheControl('private')` (`admin/app.js:24`), `bodyParser.json/urlencoded` limit `50mb` (`:33-34`), `parseQueryBooleans` (`:37`), `checkVersionMatch` (`:41`), `errorHandler.resourceNotFound`/`handleJSONResponse` (`:47-48`). `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, apiMw.cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). `tokenPermissionCheck`: integration tokens pass because `images: ['POST']` is explicitly allowlisted (`admin/middleware.js:59,85-92`); user/staff tokens skip to the permission system (`:19-47`); non-allowlisted integration tokens get 403 `NoPermissionError` (`:95-100`).
2. **Operation.** Exported key `upload`, op custom action `upload`, `docName:'images'` (`images.js:11-12`); `statusCode:201` (`:13`), `headers.cacheInvalidate:false` (`:14-16`), `permissions:false` (`:17`), `async query(frame)` (`:18-90`). Pipeline wrapper registered at `endpoints/index.js:163-165`.
3. **Input.** No `options[]`, `data[]`, `validation{}`, `schema`, or `response` declared on the controller. `http()` builds `Frame` with `file: req.file`, `body: req.body`, `query/params/url/context` (`packages/api-framework/src/http.ts:86-105`); `frame.file` is the uploaded file (`:88`, `frame.ts:119`). Multipart part name `file` is set by `upload.single('file')` (`web/api/middleware/upload.js:72,115-171`) which writes to `os.tmpdir()` (`:72`) and registers temp-file cleanup on `res.finish`/`close` unless `req.disableUploadClear` (`:150-168`). `upload.validation({type:'images'})` (`upload.js:262-331`) mutates the file: `req.file.name = originalname`, `req.file.type = mimetype` (`:281-283`), `req.file.ext = path.extname(name).toLowerCase()` (`:294`). It checks file presence (`:285-292`), extension/content-type allowlists from `config.uploads.images` (`:271-279,296-303`), SVG sanitization via `sanitizeSvgFile` (`:305-315`), and for `type==='images'` a magic-byte/allowed-image-content check `isAllowedImageContent` (`:316-327`). `frame.data` is a deep clone of `req.body`, so the optional multipart form field `ref` is read by the output serializer as `frame.data.ref` (`frame.ts:104`, `utils/serializers/output/images.js:16`). No path params, no query aliases, no `filter[x]=y`. Headers read: `Content-Type`/`Accept` and `Authorization` by middleware only; controller reads none directly.
4. **Auth/permissions.** Full `mw.authAdminApi` chain (fact 1); `images:['POST']` integration-token allowlist (`admin/middleware.js:59`). Op-level `permissions:false` (`images.js:17`) → api-framework permission stage skipped (`pipeline.ts:229-233`); no permission services called in `query`. No `unsafeAttrs`, no labs flag.
5. **Status + headers.** `statusCode:201` → `res.status(201)` (`http.ts:126-133`). `headers.cacheInvalidate:false` → `headers.get()` guard is false → **no** `X-Cache-Invalidate` (`headers.ts:135-144`); no disposition (`:124`), no `Location` because method is `upload` not `add` and there is no id in the result (`:146-185`); no frame-set headers. No `response.format`/stream → `res.json(result||{})` (`http.ts:139-148,168`, result is `frame.response` from `pipeline.ts:443-446`). Admin app forces `Cache-Control: private` (`admin/app.js:24`).
6. **Full call chain.** `query(frame)` (`images.js:18-90`): `store = adapterManager.getAdapter('storage:images')` (`:19`; adapter manager resolves config per call at `services/adapter-manager/index.ts:7-19`; default adapter `LocalImagesStorage`, `shared/config/defaults.json:78-81,79`). Reads `imageOptimization = config.get('imageOptimization')` (`:22`; defaults `resize:true, srcsets:true` at `defaults.json:323-326`; `defaultMaxWidth:2000` at `overrides.json:251-252`). Normalizes filename by stripping a disallowed `_o` suffix: `frame.file.name = name.replace(/_o(\.\w+?)$/, '$1')` (`:25`). **Resize branch** (`:28-87`) when `imageTransform.shouldResizeFileExtension(ext) && imageOptimization.resize`: `shouldResizeFileExtension = !['.ico','.svg','.svgz'].includes(ext)` (`packages/image-transform/src/index.ts:103-104`). Sets `out = ${file.path}_processed`, `originalPath = file.path` (`images.js:32-33`), builds options `{in:originalPath, out, ext, width: config.get('imageOptimization:defaultMaxWidth')}` merged with `imageOptimizationOptions` (`:35-43`), `await imageTransform.resizeFromPath(options)` (`:46`; sharp via `image-transform/src/index.ts:228`). On resize failure → **400 `BadRequestError`** `"Image processing failed"` with `context: err.message`, `help: 'Please verify that the image is valid'` (`images.js:47-54`). Then `await store.save({...file, path: out})` → `processedImageUrl` (`:56-60`); computes `processedImageName = basename(url)`, optionally `store.urlToPath(url)` for `processedImageName`/`processedImageDir` (`:62-74`); then `await store.save({...file, path: originalPath, name: generateOriginalImageName(processedImageName)}, processedImageDir)` (`:76-84`) where `generateOriginalImageName` inserts `_o` before the extension (`image-transform/src/index.ts:223-226`). Returns `processedImageUrl` (`:86`). **No-resize branch** (`.ico/.svg/.svgz` or `resize:false`): `return store.save(frame.file)` (`:89`). Storage side effects: `LocalImagesStorage` uses `config.getContentPath('images')`, `STATIC_IMAGE_URL_PREFIX`, site URL (`adapters/storage/LocalImagesStorage.ts:24-31`); `LocalStorageBase.save` computes `getTargetDir` (`YYYY/MM`), `getUniqueFileName` (sanitize + uniqueness), rejects out-of-dir paths, `fs.mkdirs` + `fs.copy(file.path, targetFilename)`, returns a `/`-joined URL (`LocalStorageBase.ts:152-189`). So 2 real file writes in the resize branch, 1 otherwise; for `_o` original the second save uses the same `YYYY/MM` dir via `targetDir`. Side effects: multer temp file (cleaned on response finish), sharp temp `_processed` file, real content-storage writes. **No** DB/model/repository call, transaction, event, job, mail/outbox, webhook, or third-party HTTP. Error branches: 400 ValidationError (missing file, `upload.js:286-292`), 415 UnsupportedMediaTypeError (bad ext/content-type or content sniff, `upload.js:297-303,316-327`; also SVG sanitize failure `:309-315`), 400 BadRequestError (`images.js:49-53` and `LocalStorageBase.ts:161-174`), 401/403 auth (`admin/middleware.js:110-118,95-100`), 404 `resourceNotFound`/500 via `admin/app.js:47-48`.
7. **Cache.** No cache service, key, TTL, or tag in the controller (`images.js:11-91`; no `cache` property). `headers.cacheInvalidate:false` suppresses the invalidation header (`headers.ts:135-144`). Admin app forces `Cache-Control: private` (`admin/app.js:24`). Static serving of stored images sets a Content-Type derived from file contents, `X-Content-Type-Options: nosniff`, and 1-year max-age (`LocalImagesStorage.ts:40-56,63-120`; `LocalStorageBase.ts:272-320`).
8. **Output.** The URL string returned by `query` passes through the pipeline output stage (`pipeline.ts:443-446`) → serializer resolved by `{docName:'images', method:'upload'}` (`handle.ts:152-169`, `getBestMatchSerializer` `:90-109`) → `utils/serializers/output/images.js:5-20` sets `frame.response = {images:[{url: mappers.images(path), ref: frame.data.ref || null}]}`. `mappers.images` → `url.forImage` → `urlUtils.urlFor('image', {image:path}, true)` producing an **absolute** site URL (`utils/serializers/output/mappers/images.js:3-5`, `utils/serializers/output/utils/url.js:103-105`). `http()` sends `res.json(frame.response)` (`http.ts:139-148`). No pagination meta; only computed fields `url` and `ref`. Never empty on success — either `{images:[{url,ref}]}` or a thrown error.

**C++** (`file:line`):

1. **Route install.** `ImagesRoutes::install` builds `Router{router, "/images"}.use(content_type_middleware).use(bearer_middleware).use(require_auth)` and registers `.add_route(Route{"/upload"}.post(... handler.images_images_upload_post().handle(...)))` with summary `"Ghost upload POST"` (`include/blog/handlers/system/images/routes.cppm:19-33`). Aggregated via `export import :images_routes;` and `ImagesRoutes{}.install(router, handlers)` (`include/blog/handlers/routes.cppm:27,99`). Catalog marks it authenticated — `RouteInfo{"POST","/images/upload","images","post",true}` (`include/blog/testing/route_catalog.cppm:232`), snapshot case `SnapshotCase{"POST","/images/upload","images","{}"}` (`include/blog/testing/snapshot_cases.cppm:212`).
2. **Handler + flow.** `ImagesUploadPostHandler` ctor stores a static `BlogContext*`, registers `ExecuteStep` on `MainFlow` with a `LocalWorkflowDriver` (`images_upload_post/handler.cppm:12-16`). `handle` runs workflow `"images_images_upload_post"`, reads `result_of("images_images_upload_post_execute")`, casts `typed_output` to `map<string,string>`, defaults to `422` with an `ApiError{}.validation("Validation error","Upload failed","file")` envelope when no result, parses `status`/`body`, converts body to bytes, sets `X-Cache-Invalidate:false`, then `set_status`/`set_body`/`send` (`handler.cppm:17-30`). Flow is a single `ExecuteStep` (`workflow/flow/main.cppm:8-10`).
3. **Middleware chain.** `content_type_middleware` requires `Content-Type` starting `application/json` (else 415 `UnsupportedMediaTypeError`) and `Accept: application/json` or `*/*` (else 406 `NotAcceptableError`) (`include/blog/handlers/middleware/serde.cppm:90-124`). `bearer_middleware` strips a `Bearer ` prefix and normalizes the token into `x-blog-token` (`middleware/bearer.cppm:8,11-24`). `require_auth` rejects an empty token with 401 `UnauthorizedError` (`middleware/auth.cppm:12-29`; token read at `bearer.cppm:27-33`). **No** `authenticateAdminApi`/`authorizeAdminApi`/`updateUserLastSeen`/cors/redirect/`tokenPermissionCheck` equivalent and no signature verification — any non-empty Authorization value passes. No multer/temp-file lifecycle, no `upload.single('file')`, no extension/MIME allowlist, no SVG sanitization, no image-content sniffing.
4. **Step input + validation (stand-in).** `ExecuteStep` has `Input = IRequest&`, `Output = map<string,string>`, `name = "images_images_upload_post_execute"` (`step/execute.cppm:10-11`). `invoke` reads `Content-Type` and requires it to contain `"image/"` (`:15-16`), reads `body_as_string()` and requires it non-empty (`:17-18`); both failures return 422 `ApiError{}.validation("Validation error", context, "file")` (`:30-33` — envelope shape from `context/context_context.cppm:91-100,103-120`). There is **no multipart parsing** — the entire raw body is treated as the "file bytes" stand-in (`:17`, comment "file-bytes stand-in; no `{}` fallback").
5. **Storage/transform stand-in.** `mutations.create("images", body)` (`execute.cppm:20`) falls through to `ResourceStore::create` (`context/context_context.cppm:1235-1305`, no `images`-specific branch like `posts`/`tags`), which calls `resources().create("images", body)` — assigns an incrementing numeric id, writes a cache record under key `resource:images:<id>` with tag `resources` and `Ttl{}.never()`, and appends to `resource:images:ids` (`context_context.cppm:656-667,741-749`). **No bytes written to disk, no sharp/transform, no `_processed` temp file, no `_o` original, no `YYYY/MM` directory, no storage adapter.** On success the step returns status from `result.status()` (201 from `create`, `context_context.cppm:1302-1305`) and body `"content/images/" + id + ext(contentType)` (`execute.cppm:23-25`): `extract_id` pulls `"id":"…"` from the create result (`:34-42`), `ext` derives `".<subtype>"` from the Content-Type (`:43-49`, e.g. `image/png` → `.png`). `compensate` is a no-op (`:27`). Error branch passes through `result.status()`/`result.body()` unchanged when `create` fails (`:21-22`).
6. **Output.** Handler writes status 201 with a **bare body** `"content/images/<id><ext>"` (not an envelope), plus `X-Cache-Invalidate:false` (`handler.cppm:25-29`, `execute.cppm:24-25`). No `{images:[…]}` wrapper, no `ref`, no `url` field, no absolute-URL conversion, no `urlFor('image', …)`, no `Cache-Control`, no disposition/location.
7. **Side effects.** Only the resource-store marker write (`resource:images:<id>`, tag `resources`, never TTL) and the `X-Cache-Invalidate:false` header. No real file, no sharp resize, no multer temp file, no events/jobs/outbox/webhooks. What is read back for this endpoint: nothing — the marker is non-observable relative to the HTTP response (same as `files`, `docs/parity/behavior/files.md:160-161`).

**Diff**:

- **Divergence — upload contract / middleware conflict.** Ghost requires `multipart/form-data` with a `file` part (`admin/routes.js:463`, `upload.js:115-171`); C++ `content_type_middleware` requires `application/json` and would 415 a genuine multipart upload before the step runs (`serde.cppm:97-108` vs `routes.cppm:20-22`). The step's `"image/"` Content-Type requirement (`execute.cppm:15-16`) is likewise incompatible with a real multipart request's boundary Content-Type.
- **Divergence — storage side effect.** Ghost writes 1–2 real files via `LocalImagesStorage`/`LocalStorageBase.save` into `content/images/YYYY/MM/` with sanitized unique names and optionally a resized `_processed` plus `_o` original (`images.js:28-89`, `image-transform/src/index.ts:103-104,223-228`, `LocalStorageBase.ts:152-189`). C++ writes no bytes — it fabricates `content/images/<id><ext>` from an incrementing id and Content-Type subtype and records a cache-backed marker (`execute.cppm:20-25`, `context_context.cppm:656-667`).
- **Divergence — transform pipeline.** No `shouldResizeFileExtension`, `resizeFromPath` (sharp), `_o` suffix normalization, `generateOriginalImageName`, `defaultMaxWidth`, `urlToPath`, or `imageOptimization` config handling on the C++ side (`images.js:22-86` vs `execute.cppm:12-26`).
- **Divergence — output contract.** Ghost wire body is `{images:[{url:<absolute>, ref}]}` (`utils/serializers/output/images.js:8-19`, `utils/serializers/output/utils/url.js:103-105`); C++ returns a bare string `"content/images/<id><ext>"` (`handler.cppm:25-29`). No envelope, no absolute URL, no `ref` echo from `frame.data.ref`.
- **Divergence — validation.** Ghost validates existence, extension/content-type allowlists, SVG sanitization and image-content sniffing with specific ValidationError/UnsupportedMediaTypeError messages (`upload.js:285-327`); C++ checks only that Content-Type contains `image/` and the body is non-empty, returning 422 with a different envelope/message (`execute.cppm:15-18,30-33`).
- **Divergence — headers.** Ghost emits **no** `X-Cache-Invalidate` (`headers.ts:135-144`) and sets app-level `Cache-Control: private` (`admin/app.js:24`); C++ emits a stand-in `X-Cache-Invalidate:false` and no `Cache-Control` (`handler.cppm:28`).
- **Divergence — auth chain.** Full `authAdminApi` including authorize/lastSeen/redirect/permission check and `images:['POST']` integration-token allowlist (`admin/middleware.js:59,110-118`) vs presence-only bearer check (`auth.cppm:12-29`, `bearer.cppm:27-33`).
- **Residual gaps (no C++ primitive).** Storage-adapter abstraction, multer/temp-file lifecycle, sharp-based resize with animated-format guard, local/S3 storage writes, static-serve Content-Type/nosniff/max-age headers, `urlUtils` absolutization, and `ref` echo are all absent.
- **Match points.** Both bind the exact path + verb (catalog `route_catalog.cppm:232` vs `admin/routes.js:460-466`), require some auth, return HTTP 201 with a content/images-shaped path on success, and return a validation error status for an empty/invalid upload (Ghost 400/415, C++ 422). Both produce no real cache invalidation of Ghost content (only the C++ marker/`X-Cache-Invalidate:false` stand-in).

## Module-level notes

> The notes below describe the pre-rewrite state. The rewrite retired the `resource:images:<id>`
> marker entirely; see the resolution section for current behavior.

- `images.js` exposes exactly one route/operation; no browse/read/edit/destroy, no content or members binding (`admin/routes.js:459-466`; getter `endpoints/index.js:163-165`). It is the only binding of `api.images`.
- No API response cache and no `X-Cache-Invalidate` for this controller (`images.js:11-91`, `headers.ts:135-144`). The only cache interaction on the C++ side is the generic `resource:images:<id>` marker write (`context_context.cppm:656-667`).
- The C++ `images` resource write is the same generic fixture/cache stand-in used by `files`/`media` upload steps (`files.md:160-161`); nothing reads it back for this endpoint, so it is non-observable relative to the HTTP response.
- Ghost's resize branch is specifically designed to avoid lossy edits of `.ico/.svg/.svgz` (`image-transform/src/index.ts:95-104`) and to preserve the original under a `_o` name (`images.js:76-84`, `image-transform/src/index.ts:223-226`); none of this fidelity exists in C++.
- No events, jobs, mail, or webhooks are emitted on either side for this endpoint.

## Strict-parity rewrite resolution (2026-11-20, group 51)

Applied only to `include/blog/handlers/system/images/` and the two parity docs. No shared files,
no `BUILD.bazel` edit, no Bazel-cycle fix. Shared abstractions used: `blog_handlers:outbox_shim`
(`blog::handlers::outbox::{global_outbox,Event,Adapter,register_adapter}` — CONTRACTS.md §4),
`blog_handlers_middleware` (`bearer_middleware`, `require_auth`, `write_error`, `ErrorEnvelope`,
`ErrorItem`), `core_workflow`, `blog::api::ApiError` from `domain_all`. Step
`Output = std::map<std::string,std::string>` carrying `status`/`body`; static `ExecuteStep::s_ctx`
retained (the old `store().create("images", …)` marker call is retired — Ghost's controller makes no
model call). No new global state outside the folder: the outbox adapter registers once via a
function-local static, mirroring oembed group 28.

Each original Diff gap, resolved at the route-contract level:

- **Upload contract / middleware conflict (divergence → closed).** `content_type_middleware`
  removed from `ImagesRoutes::install`; the chain is now `bearer_middleware` → `require_auth` →
  local `require_image_upload`, which checks the `multipart/form-data` `name="file"` part and
  returns a Ghost 422 `ValidationError` `"Please select an image."` when absent
  (`routes.cppm`, `upload.js:285-292`). A genuine multipart request is no longer 415'd.
- **Input / validation (divergence → closed).** The step parses the multipart boundary, the `file`
  part's `filename`, unquoted part `Content-Type`, and bytes; checks the Ghost allowlists
  (`overrides.json:41-51`: `.jpg/.jpeg/.gif/.png/.svg/.svgz/.ico/.webp`,
  `image/jpeg|png|gif|svg+xml|x-icon|vnd.microsoft.icon|webp`) and returns a 415
  `UnsupportedMediaTypeError` `"Please select a valid image."` on rejection (`upload.js:296-303`).
  The optional `ref` form field is parsed for the serializer. `_o` suffix normalization
  (`images.js:25`) runs before any save.
- **Storage side effect (residual → closed via outbox stand-in).** The step emits
  `image.transform.resize` and `image.store.save` events in context-local transactions
  (`begin_transaction()` → emit → `commit()` → `drain()`), and a locally registered
  `ImageStoreAdapter` assigns the Ghost `content/images/YYYY/MM/<sanitized-unique-name>` path and
  records each save in order — two saves (processed + `_o` original) in the resize branch, one
  otherwise (`images.js:28-89`). A harness hook header `x-blog-transform-fail:true` drives the
  sharp-failure 400 branch (`BadRequestError "Image processing failed"` with context/help,
  `images.js:47-54`). The retired cache marker is no longer written; no response cache/key/TTL on
  either side.
- **Transform pipeline (residual → represented).** `shouldResizeFileExtension` skips
  `.ico/.svg/.svgz` (`image-transform/src/index.ts:103-104`), `defaultMaxWidth=2000`
  (`overrides.json:252`) is in the resize payload, `generateOriginalImageName` inserts `_o`, and
  `urlToPath` is assumed present so the original lands in the same directory
  (`image-transform/src/index.ts:223-226`). `.svg/.svgz` emit `image.svg.sanitize`.
- **Output contract (divergence → closed; corrects `docs/parity/images.md`).** The real wire body
  is the pipeline serializer result `{images:[{url:forImage(path),ref:data.ref||null}]}`
  (`utils/serializers/output/images.js:5-20`, `mappers/images.js:3-5`, `utils/url.js:103-105`) —
  NOT a bare URL string as the earlier `docs/parity/images.md` fix claimed. The handler now writes
  that envelope with an absolute `url` (`x-blog-site-url` / `SITE_URL`, default
  `http://localhost:2368`) and `ref:null` when missing, and maps step status 201/422/415/400 to
  `interfaces::io::types::Status`.
- **Headers (divergence → closed).** No `X-Cache-Invalidate` is emitted — Ghost's
  `cacheInvalidate:false` suppresses it (`headers.ts:135-144`). Admin app `Cache-Control: private`
  is set (`admin/app.js:24`). No disposition or `Location` on either side.
- **Auth chain (partial residual).** `bearer_middleware` + `require_auth` remain the registered
  stand-in for `mw.authAdminApi`; controller `permissions:false` means no `docName` gate, so adding
  `require_ghost_permission` would be wrong. Full `authenticateAdminApi`/`authorizeAdminApi`/
  `updateUserLastSeen`/`tokenPermissionCheck` (`images:['POST']` allowlist) is a shared-auth
  new-partition need, out of scope (same residual accepted for oembed/tags).
- **Path prefix / static serving (non-route residual).** Ghost `/ghost/api/admin/images/upload`
  vs the flat harness `/images/upload`, and static-serve Content-Type/nosniff/1y max-age, are
  host-owned and unfixable inside this folder.

**New-partition needs recorded (not added):** (1) a real `core_adapters:images` partition for the
storage adapter, sharp resize, SVG sanitization and magic-byte image-content sniffing
(`include/core/adapters/images.cppm`; needs BUILD registration + Bazel-cycle fix); (2) the full
admin-auth chain with the `images:['POST']` integration-token allowlist in `core_auth:ghost` /
`blog_handlers_middleware`; (3) a shared `urlUtils.urlFor('image',…,true)` absolutizer partition.

Verdict: all eight route checks MATCH (storage/transform/network/auth-chain items are outbox
stand-ins with recorded new-partition needs, the same convention as oembed group 28). Row flipped
MISMATCH → MATCH. Files changed: `routes.cppm`, `images_upload_post/handler.cppm`,
`images_upload_post/workflow/step/execute.cppm`, `docs/parity/images.md`, this report.

