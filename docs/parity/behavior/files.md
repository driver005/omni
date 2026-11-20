# Behavior report: files

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/files.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/system/files/` (routes + `files_upload_post` handler/flow/step/dto)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

Single route (`POST /files/upload`). C++ binds the exact path+verb and returns 201. A strict
parity pass (2026-11-20, Group 50) closed the wire output contract, extension validation,
multipart gate, URL/path shape, and content-type fidelity inside the group folder. The route
stays **MISMATCH**: real byte storage and the full `authAdminApi` chain remain outside the
folder and need a registered storage-adapter / shared-middleware partition, so strict parity
is not fully achieved and no verdict flip was made.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/files/upload` | `upload` (`docName: 'files'`) | multipart `file` part; optional `ref` form field; no query/options schema | `authAdminApi` (7 middlewares) + multer `single('file')` + `fileValidation({type:'files'})`; controller `permissions:false` | 201; `headers.cacheInvalidate:false` → no `X-Cache-Invalidate`, no disposition/location | `http()` → pipeline → `adapterManager.getAdapter('storage:files').save({name,path,type})` → `LocalFilesStorage`/`LocalStorageBase.save` → output serializer `files.upload` | No server cache; no invalidation header; static serve sets `X-Content-Type-Options: nosniff`, derived Content-Type, 1y max-age | `{files:[{url: getURL(filePath), ref: frame.data.ref || null}]}` | `system/files/routes.cppm`, `files_upload_post/{handler,dto/response,workflow/flow/main,workflow/step/execute}.cppm`, `middleware/{auth,serde,error}.cppm`, `context/context_context.cppm` | 201 + `{"files":[{"url":<siteUrl>/content/files/YYYY/MM/<sanitized-unique-name>,"ref":<ref\|null>}]}` + `Cache-Control: private`; bearer-token presence only; full 104-extension allowlist with Ghost 400/415 envelopes; no JSON content-type gate; Ghost-shaped path/URL; no real bytes written; metadata still a cache-backed marker; full admin-auth chain absent | MISMATCH | Ghost: `.../files.js:6-23`, `.../admin/routes.js:484-491`, `.../utils/serializers/output/files.js:4-28`; C++: `.../routes.cppm:15-33,104-122,136-148`, `.../execute.cppm:133-167,199-220,242-283`, `.../handler.cppm:36-62` |

## Route details

### POST /files/upload — MISMATCH

**Ghost** (facts 1–8):

1. **Identity.** `router.post('/files/upload', mw.authAdminApi, apiMw.upload.single('file'), apiMw.upload.fileValidation({type:'files'}), http(api.files.upload))` —
   `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:484-491`. Router-level mount: `router.use(apiMw.cors)` at `:17`. No other route in any Ghost route table binds `api.files`
   (only occurrence: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:490`; binding getter at
   `docs/Ghost/ghost/core/server/api/endpoints/index.js:171-173`).
2. **Operation.** Exported key `upload`, op custom action `upload`, `docName: 'files'`; `statusCode: 201`,
   `headers.cacheInvalidate: false`, `permissions: false`; `async query(frame)` —
   `docs/Ghost/ghost/core/server/api/endpoints/files.js:5-24`.
3. **Input.** No `options[]`, `data[]`, or `validation{}` declared on the controller.
   `http()` builds a `Frame` with `file: req.file`, `body: req.body`, `query/params/headers` —
   `docs/Ghost/packages/api-framework/src/http.ts:86-105`; `frame.data` is the cloned request body, so the
   output serializer reads multipart form field `ref` from `frame.data.ref` —
   `docs/Ghost/packages/api-framework/src/frame.ts:79-123`,
   `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/files.js:24`.
   Multipart `file` field is written to `os.tmpdir()` by multer `upload.single('file')` —
   `docs/Ghost/ghost/core/server/web/api/middleware/upload.js:72,115-171`.
   `fileValidation` sets `req.file.name/.type/.ext` and checks `checkFileExists` (mimetype && path) — `:438-448`.
4. **Auth/permissions.** Route uses full `mw.authAdminApi` chain: `authenticateAdminApi`,
   `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`,
   `tokenPermissionCheck` — `docs/Ghost/ghost/core/server/web/api/endpoints/admin/middleware.js:110-118`.
   Controller `permissions:false` skips the api-framework permission stage —
   `docs/Ghost/packages/api-framework/src/pipeline.ts:218-233`; `docs/Ghost/ghost/core/server/api/endpoints/files.js:12`.
   `fileValidation` errors: missing file → `ValidationError` (`'Please select a file.'`); disallowed extension →
   `UnsupportedMediaTypeError` (`'The file type you uploaded is not supported. You can zip your file and upload it as a .zip.'`) —
   `docs/Ghost/ghost/core/server/web/api/middleware/upload.js:64-68,442-458`.
   Extension allowlist is `config.uploads.files.extensions` (~110 extensions: `.7z`, `.zip`, `.pdf`, `.png`, `.txt`, `.html`, `.js`, …) —
   `docs/Ghost/ghost/core/shared/config/overrides.json:70-177`; `contentTypes` is empty and deliberately unused for `files` —
   `docs/Ghost/ghost/core/server/web/api/middleware/upload.js:423-431`.
5. **Status + headers.** 201 via `statusCode` — `docs/Ghost/packages/api-framework/src/http.ts:126-133`;
   `headers.cacheInvalidate:false` means `headers.get()` emits no `X-Cache-Invalidate` (only truthy config emits it) —
   `docs/Ghost/packages/api-framework/src/headers.ts:135-144`; no disposition, no `Location` (method is not `add`, no id) —
   `:146-185`. Response sent as JSON via `res.json(result || {})` — `http.ts:139-148`.
6. **Full call chain.** `query(frame)` calls `adapterManager.getAdapter('storage:files').save({name:frame.file.originalname, path:frame.file.path, type:getStorageContentType(frame.file.originalname)})`
   — `docs/Ghost/ghost/core/server/api/endpoints/files.js:13-23`. Adapter manager resolves config on each `getAdapter` —
   `docs/Ghost/ghost/core/server/services/adapter-manager/index.ts:7-19`; default `storage:files` → `LocalFilesStorage` —
   `docs/Ghost/ghost/core/shared/config/defaults.json:78-88`. `getStorageContentType` three-tier resolution
   (override map → browser-renderable allowlist → `application/octet-stream`) —
   `docs/Ghost/ghost/core/server/lib/file-types.ts:4-52`. `LocalFilesStorage` sets path `config.getContentPath('files')` and
   serve headers — `docs/Ghost/ghost/core/server/adapters/storage/LocalFilesStorage.ts:36-65`. `LocalStorageBase.save()`
   computes `getTargetDir(storagePath)` (`YYYY/MM`), `getUniqueFileName` (sanitize + uniqueness loop), rejects paths
   outside target dir, `fs.mkdirs` + `fs.copy(file.path, targetFilename)`, returns a URL joined from
   `/<subdir>/content/files/<relative path>` — `docs/Ghost/ghost/core/server/adapters/storage/LocalStorageBase.ts:152-189`;
   `docs/Ghost/packages/adapters/storage-base/src/base.ts:41-96`. Alternative S3 adapter stores bytes via `PutObjectCommand`
   with `ContentType: detectStorageContentType(body) ?? file.type` and returns `${cdnUrl}/${key}` —
   `docs/Ghost/ghost/core/server/adapters/storage/S3Storage.ts:205-234`. Pipeline order validation → input serialisation →
   permissions → query → output serialisation — `docs/Ghost/packages/api-framework/src/pipeline.ts:29-305,430-455`.
   Side effects: temp upload file written by multer, cleaned on `res.finish`/`close` unless `req.disableUploadClear` —
   `docs/Ghost/ghost/core/server/web/api/middleware/upload.js:150-169`; real file copied into content storage; no events,
   jobs, mail, or webhooks for this route.
7. **Cache.** No API response cache configured on the controller (`cache` absent) —
   `docs/Ghost/ghost/core/server/api/endpoints/files.js:5-24`. `headers.cacheInvalidate:false` suppresses the invalidation
   header — `docs/Ghost/packages/api-framework/src/headers.ts:135-144`. Static serving of stored files uses 1-year `maxAge`,
   `fallthrough:false`, `X-Content-Type-Options: nosniff`, and derived Content-Type —
   `docs/Ghost/ghost/core/server/adapters/storage/LocalStorageBase.ts:272-320`;
   `docs/Ghost/ghost/core/server/adapters/storage/LocalFilesStorage.ts:58-65`.
8. **Output.** `files.upload` output serializer transforms `{filePath}` into
   `{files:[{url: getURL(filePath), ref: frame.data.ref || null}]}`; `getURL` converts a subdir/content-files path into an
   absolute site URL — `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/files.js:4-28`.
   No pagination meta; no computed fields beyond `url`/`ref`. No empty-result path: `save()` either returns a URL or throws
   (`BadRequestError` on ENAMETOOLONG/unexpected target path — `LocalStorageBase.ts:159-175`).

**C++** (facts, after the 2026-11-20 strict-parity pass):

1. **Route install.** `FilesRoutes::install` now builds `Router{router, "/files"}.use(require_auth).use(require_upload_file)` —
   the JSON-only `content_type_middleware` was removed because it 415'd genuine multipart uploads before validation —
   and registers `.add_route(Route{"/upload"}.post(... handler.files_files_upload_post().handle(...)))` —
   `include/blog/handlers/system/files/routes.cppm:130-148`. Aggregated via `export import :files_routes;` and
   `FilesRoutes{}.install(router, handlers)` — `include/blog/handlers/routes.cppm:23,95`. Catalog/snapshot confirm
   `POST /files/upload` — `include/blog/testing/route_catalog.cppm:235`, `include/blog/testing/snapshot_cases.cppm:215`.
2. **Handler + flow.** `FilesUploadPostHandler` runs workflow `files_files_upload_post`, casts the step output to
   `std::map<std::string,std::string>` (`url`/`ref`/`filePath`), and emits the Ghost serializer envelope with 201 —
   `include/blog/handlers/system/files/files_upload_post/handler.cppm:36-60`. Flow is a single `ExecuteStep` —
   `include/blog/handlers/system/files/files_upload_post/workflow/flow/main.cppm:6-10`. Wired into `BlogHandlers` at
   `include/blog/handlers/all.cppm:594,873`.
3. **Upload validation.** `kFilesExtensions` embeds all 104 entries of `config.uploads.files.extensions` —
   `routes.cppm:15-33`; `upload_filename()` extracts the `name="file"` part's `filename` — `:51-68`. `require_upload_file()`
   writes 400 `{errors:[{message:"Please select a file.",type:"ValidationError"}]}` for a missing file and 415
   `{errors:[{message:"The file type you uploaded is not supported. You can zip your file and upload it as a .zip.",type:"UnsupportedMediaTypeError"}]}`
   for a disallowed extension — `:70-122`; error envelope shape from `include/blog/handlers/middleware/error.cppm:10-42`.
   No MIME check, temp-file handling, or size limit.
4. **Auth.** `require_auth` still checks only `token(request).empty()` → 401 `UnauthorizedError` —
   `include/blog/handlers/middleware/auth.cppm:12-29`. Handler comment notes Ghost `permissions:false` so no in-handler check —
   `handler.cppm:36-37`. No `authorizeAdminApi`/role permission stage.
5. **Middleware conflict — closed in-folder.** `content_type_middleware` is no longer mounted on this route, so a real
   `multipart/form-data` upload reaches `require_upload_file` instead of being 415'd (`routes.cppm:136-137`).
6. **Step parsing + path shape.** `ExecuteStep::invoke` parses the multipart body by boundary, extracts the `name="file"`
   part's `filename`/bytes and the `ref` form field — `include/blog/handlers/system/files/files_upload_post/workflow/step/execute.cppm:56-131`.
   `storage_content_type()` mirrors `getStorageContentType` including `.txt`→`text/plain` — `:133-167`. `target_dir()`
   (`YYYY/MM`), `sanitize_filename()` (`[^\w@.]`→`-`), and `unique_filename()` (`-N` loop) build
   `/<subdir>/content/files/YYYY/MM/<name>` — `:169-220`. `absolute_url()` mirrors the output serializer's `getURL`
   (strip subdir, prepend `x-blog-site-url`/`SITE_URL`/default) — `:242-249`. No bytes are written to disk.
7. **Resource-store side effect.** `FileMetaDto{name,path,type}` is encoded and written via
   `ctx.store().create("files", ...)` — `execute.cppm:278-283`; DTO fields at
   `include/blog/handlers/system/files/files_upload_post/dto/response.cppm:8-36`. `create` falls through to
   `ResourceStore::create`, which assigns an incrementing id, writes `resource:files:<id>` into `core::cache::Cache` with
   tag `resources` and `Ttl{}.never()`, and appends to `resource:files:ids` —
   `include/blog/handlers/context/context_context.cppm:1301-1305,656-667,741-749`. No real file, no serve headers, no invalidation.
8. **Output.** `handler.cppm:52-62` emits `{"files":[{"url":<absolute URL>,"ref":<ref|null>}]}` with status 201 and
   `Cache-Control: private` (admin app), no `X-Cache-Invalidate` — matching
   `utils/serializers/output/files.js:18-28` and `headers.ts:135-144`. The `FilePathDto` wire stand-in was removed from
   the DTO partition.

**Diff** (post-pass):

- **CLOSED — output contract.** Ghost wire response `{files:[{url,ref}]}` (`utils/serializers/output/files.js:18-28`) now
  matches `handler.cppm:52-59`; `ref` is the multipart form field `ref` or `null`, and `url` is absolutized.
- **CLOSED — validation.** The full 104-extension allowlist and both Ghost error envelopes/messages are emitted
  (`routes.cppm:15-33,70-122`; `upload.js:433-462`, `overrides.json:70-177`).
- **CLOSED — multipart gate.** The JSON-only middleware is gone from this route (`routes.cppm:136-137`).
- **CLOSED — `getStorageContentType` fidelity.** `.txt`→`text/plain` and the renderable-type table are present
  (`execute.cppm:133-167`; `file-types.ts:4-52`).
- **OPEN — storage side effect.** Ghost copies upload bytes into `content/files/YYYY/MM/<sanitized-unique-name>` and
  returns that URL (`LocalStorageBase.ts:152-189`, `base.ts:41-96`). C++ now computes the same path shape but writes no
  bytes; uniqueness is an in-process `-N` set, not a filesystem probe, and the store write remains a cache-backed marker
  (`execute.cppm:199-220,278-283`, `context_context.cppm:656-667`). A registered `storage:files` adapter is required —
  recorded as a new-partition need in `docs/parity/files.md`.
- **OPEN — auth chain.** Ghost uses full `authAdminApi` (authenticate + authorize + lastSeen + redirects +
  tokenPermission) (`admin/middleware.js:110-118`); C++ is bearer-presence only (`auth.cppm:12-29`). Shared middleware /
  new partition required.
- **OPEN — residual primitives.** No multer/temp-file lifecycle, no S3 write, no serve-time
  Content-Type/X-Content-Type-Options/max-age headers.
- **Match points.** Both bind `POST /files/upload`, require some auth, return 201 with the Ghost `{files:[{url,ref}]}`
  JSON envelope, validate the extension with the same allowlist, and derive the storage content type from the filename.

**Verdict:** MISMATCH retained. The template's route row and detail heading stay MISMATCH because strict parity is not
*fully* achieved: real byte storage and the full admin-auth chain are outside the folder. No flip was made.

## Module-level notes

- `files.js` exposes exactly one route/operation; no browse/read/edit/destroy, no content or members binding
  (`admin/routes.js:484-491`; only other `files` reference is a legacy-service inventory entry).
- No cache service, no API response cache, and no `X-Cache-Invalidate` for this controller (`files.js:5-24`, `headers.ts:135-144`).
  The only cache interaction on the C++ side is the resource-store marker write (`context_context.cppm:656-667`).
- The C++ `files` resource write is a generic fixture/cache stand-in shared with `images`/`media` upload steps; nothing reads it
  back for this endpoint, so it is a non-observable side effect relative to the HTTP response.
- No events/jobs/mail/webhooks are emitted on either side for this endpoint.
