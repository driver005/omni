# Parity: files

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/files.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/files/routes.cppm` (single route file)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/files/upload` | `files.upload` | POST `/files/upload` (`Router{router, "/files"}` + `Route{"/upload"}.post(...)`) | `handler.files_files_upload_post().handle()` → `FilesUploadPostHandler` → `ExecuteStep` | Ghost: multipart `apiMw.upload.single('file')` + `fileValidation({type:'files'})`, reads `frame.file.originalname` / `frame.file.path`, derives type via `getStorageContentType`. C++: only `content_type_middleware`; no multipart parsing, no file field, no file validation; step reads raw `in.body_as_string()` (or `"{}"`). Divergent. | Ghost: `statusCode: 201`, returns `{filePath}`, `headers.cacheInvalidate: false`. C++: `res.set_status(Status::OK)` (200), body = raw `.body()` of generic store create (created record, not `{filePath}`). Divergent. | Ghost: no DB model — `adapterManager.getAdapter('storage:files').save({name,path,type})` (storage adapter). C++: `ctx.store().create("files", body)` — generic store create into resource string `files`. Divergent. | Ghost: `headers.cacheInvalidate: false` (explicit no-invalidate header). C++: none (no `Cache`, namespace, TTL, or eviction in `system/files/`). Both effectively no caching; Ghost sets an explicit header. | Ghost: `mw.authAdminApi` middleware; controller `permissions: false`. C++: `content_type_middleware` + in-handler `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}`; no permissions/role check. Both require a token and neither checks granular permissions; mechanism differs. | MISMATCH | Ghost: `files.js:7-24`, `routes.js:485-491`. C++: `routes.cppm:19-31`, `handler.cppm:17-28`, `execute.cppm:11-16`. |

## Route details

### POST `/files/upload` — MISMATCH

Route path and verb match exactly; operation is conceptually "upload file" on both sides. Data shape, resource, and input are materially different, so verdict is MISMATCH.

- **Route identity — MATCH**: Ghost `router.post('/files/upload', ...)` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:485-486`; C++ `Router{router, "/files"}` + `Route{"/upload"}.post(...)` at `include/blog/handlers/system/files/routes.cppm:19-23` → same `POST /files/upload`.
- **Operation mapping — MATCH**: Ghost `http(api.files.upload)` (`routes.js:490`) → controller key `upload` (`files.js:7`). C++ binds `handler.files_files_upload_post().handle(...)` (`routes.cppm:28`), handler `FilesUploadPostHandler` (`handler.cppm:10-17`), step `ExecuteStep` (`execute.cppm:9`, flow `main.cppm:9`).
- **Input — MISMATCH**: Ghost uses multipart — `apiMw.upload.single('file')` and `apiMw.upload.fileValidation({ type: 'files' })` (`routes.js:488-489`) and consumes `frame.file.originalname`, `frame.file.path`, with type from `getStorageContentType(frame.file.originalname)` (`files.js:1,15-17`). C++ installs only `blog::handlers::middleware::content_type_middleware` (`routes.cppm:20`); no multipart/`single('file')`/file-validation. Step declares `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) and uses raw `in.body_as_string()` with `"{}"` fallback (`execute.cppm:16`). Param names present only on Ghost side: `file` (multipart field), `type: 'files'` validation. No C++ file-name/content-type extraction exists.
- **Output — MISMATCH**: Ghost `statusCode: 201` (`files.js:8`) and returns `{ filePath }` (`files.js:20-22`). C++ sets `Status::OK` (200) at `handler.cppm:27` and returns the raw string body from `mutations.create(...).body()` passed through at `handler.cppm:24-26`; no `{filePath}` key, no 201.
- **DB call — MISMATCH**: Ghost performs no DB model call; it invokes the storage adapter `adapterManager.getAdapter('storage:files').save({...})` (`files.js:2,14-18`). C++ performs `ctx.store().create("files", in.body_as_string().empty() ? "{}" : in.body_as_string())` (`execute.cppm:14-16`) — a generic store create into resource string `files`; no storage-adapter save, no name/path/type params, no real upload.
- **Cache — effectively MATCH (both none)**: Ghost sets `headers: { cacheInvalidate: false }` (`files.js:9-11`), i.e. explicitly no invalidation. C++ `system/files/` has no `core::cache::Cache`, namespace, TTL, or eviction (grep over `include/blog/handlers/system/files/**` for `cache|Cache|namespace|ttl|TTL` found only C++ `namespace` declarations). No cache reads/writes on either side.
- **Auth — PARTIAL**: Ghost middleware chain includes `mw.authAdminApi` (`routes.js:487`) with controller `permissions: false` (`files.js:12`). C++ uses `content_type_middleware` (`routes.cppm:20`) plus an in-handler token check `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`). Both require a token and neither enforces granular permissions (consistent with `permissions: false`), but Ghost uses full admin-API auth middleware while C++ uses only a non-empty-token check.

#### Fix

Checks 1 (route identity), 2 (operation mapping) and 6 (cache) already match — no change. Checks 3, 4, 5, 7 are addressed by the source edits below, implemented 2026-11-19 under `include/blog/handlers/system/files/`.

**A. Input + validation — closes check 3.**
- File `include/blog/handlers/system/files/files_upload_post/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke` / `using Input`: replace the raw `in.body_as_string()` dump (`execute.cppm:16`) with multipart parsing using only `IRequest` + std:
  - read `in.get_content_type()` (`include/interfaces/io/request.cppm:847`), require `multipart/form-data`, extract the `boundary=` parameter;
  - split `in.body_as_string()` (`request.cppm:907`) on `--<boundary>`; for the part whose `Content-Disposition` contains `name="file"`, capture `filename="..."` → Ghost `frame.file.originalname` and the part bytes → `frame.file.path`;
  - add local helper `storage_content_type(filename)` mirroring Ghost `getStorageContentType` at `docs/Ghost/ghost/core/server/lib/file-types.ts:38-52`: lowercase the extension; `.html/.htm/.js/.css/.xml` → `text/plain`; a small renderable-extension → MIME table (`.jpg/.jpeg`→`image/jpeg`, `.png`, `.gif`, `.webp`, `.pdf`, `.json`, `.mp3`, `.wav`, `.m4a`, `.mp4`, `.mov`, `.otf`, `.woff`, `.woff2`); everything else → `application/octet-stream`. No third-party lib (no Ghost `mime-types`, no `file-type`).
- Validation half of check 3: add `require_upload_file` to `blog_handlers_middleware` (pattern: `include/blog/handlers/middleware/auth.cppm:12-29` with `write_error` from `middleware/error.cppm`) and install it in `routes.cppm` after `content_type_middleware`. It writes a 400 `{"errors":[...]}` when no `file` part or an empty filename, replacing Ghost `apiMw.upload.single('file')` + `fileValidation({type:'files'})` (`routes.js:488-489`).

```cpp
// step/execute.cppm
using Input = interfaces::io::IRequest&;
using Output = std::string; // saved file path, NOT raw store body
// in invoke(): parse boundary from in.get_content_type(); split in.body_as_string();
// filename = part's filename="..."; bytes = part body; type = storage_content_type(filename);
```

**B. Output — closes check 4.**
- File `include/blog/handlers/system/files/files_upload_post/handler.cppm`, symbol `FilesUploadPostHandler::handle`, line 27: `Status::OK` → `interfaces::io::types::Status::CREATED` (201; factory at `include/interfaces/io/response.cppm:70-75`).
- Same function, lines 24–26: stop passing `mutations.create(...).body()` through. Take the path returned by `ExecuteStep` and encode it with `core_serde::JsonCodec` into a DTO whose sole serde field is `"filePath"` → `{"filePath":"..."}`, matching Ghost `files.js:20-22`. Add `import core_serde;` and define `FilePathDto` in a new `include/blog/handlers/system/files/files_upload_post/dto/response.cppm` using the existing `serde::Serializable` / `serde::FieldDesc<...>` pattern (`include/blog/handlers/external/oembed/dto/response.cppm:30-58`; encode call style at `include/blog/handlers/middleware/error.cppm:34`).

```cpp
FilePathDto dto; dto.set_filePath(path);
const std::string body = core_serde::JsonCodec{}.encode(dto); // {"filePath":"..."}
res.set_status(interfaces::io::types::Status::CREATED);        // 201
```

**C. DB / storage call — closes check 5 (within allowed modules).**
- File `include/blog/handlers/system/files/files_upload_post/workflow/step/execute.cppm`, lines 14–16: replace `mutations.create("files", in.body_as_string().empty()?"{}":in.body_as_string())` with a create carrying explicit `{name, path, type}` metadata via `core_serde::JsonCodec`, then return `storedPath` as the step `Output` so B can emit `filePath`. Ghost writes no DB model; it calls `adapterManager.getAdapter('storage:files').save({name,path,type})` (`files.js:2,14-18`). No storage-adapter abstraction exists in the allowed modules, so `ctx.store()` resource string `files` is the metadata stand-in. Residual gap (real byte storage and the adapter's returned path) cannot be closed with the allowed modules — recorded, not invented.

```cpp
const std::string json = core_serde::JsonCodec{}.encode(FileMetaDto{filename, storedPath, type});
ctx.store().create("files", json);  // resource string "files", explicit name/path/type
return storedPath;                  // -> handler wraps as {"filePath": storedPath}
```

**D. Auth — closes check 7.**
- File `include/blog/handlers/system/files/routes.cppm`, lines 19–21: add `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware` (defined at `include/blog/handlers/middleware/auth.cppm:12-29`; emits Ghost-style 401 `{"errors":[{"message":"Unauthorized","type":"UnauthorizedError"}]}`).
- File `handler.cppm`, lines 18–22: delete the in-handler `middleware::token(req).empty()` → `{"error":"Unauthorized"}` branch — different 401 body and redundant once `require_auth` is installed. No permission/role check is needed: Ghost sets `permissions: false` (`files.js:12`).

```cpp
Router{router, "/files"}
    .use(blog::handlers::middleware::content_type_middleware)
    .use(blog::handlers::middleware::require_auth)
    .add_route(/* unchanged Route{"/upload"}.post(...) */);
```

## Missing in C++

None. The sole Ghost route for this controller (`POST /files/upload`) has a C++ counterpart. Confirmed by grep of `api.files` across `docs/Ghost/ghost/core/server/web` (only hit: `admin/routes.js:490`) and grep of `files` in `include/blog/handlers/all.cppm` (registry entries at lines 193, 539, 788).

  Fix: No change required. No Ghost route lacks a C++ counterpart, so there is
  nothing to add. The fixes above only reshape the existing
  `FilesUploadPostHandler` / `ExecuteStep` / `FilesRoutes` to reach parity.

## Extra in C++

None. `include/blog/handlers/system/files/routes.cppm` installs exactly one route — `Route{"/upload"}.post(...)` at line 22 — matching the Ghost route. Full-file read confirms no other routes in that file; `routes.cppm:18` exports only `:files_routes`.

  Fix: No change required. No extra C++ route exists to remove or gate. Keep the
  single `POST /files/upload` route; the fixes above adjust its middleware/input/
  output behavior, not its path or verb.

## Implementation status (2026-11-19)

> Superseded by the "Strict-parity pass (Group 50, 2026-11-20)" section below. Kept as the
> historical record of the first Fix-block pass.

Source edits applied inside `include/blog/handlers/system/files/` only:

- `files_upload_post/workflow/step/execute.cppm` — `Output` stays `std::string`; parses
  `multipart/form-data` via `IRequest::get_content_type()` / `body_as_string()` (boundary
  extraction, `name="file"` part, `filename` → `originalname`), maps the extension through
  a local `storage_content_type` mirroring Ghost `getStorageContentType`, writes
  `{name, path, type}` to `ctx.store().create("files", ...)` as a `FileMetaDto` encoded with
  `core_serde::JsonCodec`, and returns the stand-in `storedPath`. `static s_ctx` retained.
- `files_upload_post/dto/response.cppm` (new) — `FilePathDto` (`filePath` only) and
  `FileMetaDto` (`name`/`path`/`type`), with `serde::Serializable` / `serde::FieldDesc`
  specializations.
- `files_upload_post/handler.cppm` — removed the in-handler `token()`/`{"error":...}` branch,
  encodes the step path via `FilePathDto` into `{"filePath":"..."}`, sets
  `Status::CREATED` (201). Imports are limited to `:domain_all`, the new DTO partition,
  `std`, `interfaces`, `core_workflow`, `core_serde`, and the flow partition (the unused
  `memory_backends` import was dropped).
- `routes.cppm` — added `blog::handlers::middleware::require_auth` after
  `content_type_middleware` and a local `require_upload_file` validation step that emits a
  400 Ghost-style `{"errors":[...]}` envelope when the `file` part or its filename is absent.
  Route identity (`POST /files/upload`) unchanged.

### Central registrations — status after the strict-parity pass

1. DONE (already wired by central pass): `include/blog/handlers/BUILD.bazel` lists
   `system/files/files_upload_post/dto/response.cppm`, and `include/blog/handlers/base.cppm`
   exports `:files_files_upload_post_dto`. No further change needed.
2. NEW PARTITION NEED — `include/blog/handlers/middleware/`: promote
   `system/files/routes.cppm`'s local `require_upload_file` + `upload_filename` +
   `kFilesExtensions` into a shared `blog_handlers_middleware` partition, re-export from
   `middleware/base.cppm`, and switch upload routes (files/images/media/themes/redirects)
   to it. Cannot be done from the group folder (shared path).
3. RESOLVED IN-FOLDER — the JSON-only `content_type_middleware` was removed from this
   route's chain (it 415'd genuine multipart before validation). No shared edit needed.
4. NEW PARTITION NEED — a `storage:files` adapter (local filesystem / S3) is absent from
   `core_adapters` and is not a `blog_handlers` dep. Real byte writes, filesystem-based
   uniqueness, and serve-time Content-Type/nosniff/max-age headers need that partition.
5. DONE — `include/blog/handlers/all.cppm` (`files_files_upload_post` member/accessor) and
   `include/blog/handlers/routes.cppm` (`:files_routes` export) already register this
   handler and route.

### Residual gaps (no allowed module closes them)

- Real byte storage and the storage adapter's returned path do not exist in the allowed
  modules; `store().create("files", {name, path, type})` plus a deterministic
  `/content/files/<filename>` stand-in is the metadata-only substitute for
  `adapterManager.getAdapter('storage:files').save(...)`.
- Granular permissions are intentionally absent (Ghost `permissions: false`).
- No explicit `cacheInvalidate: false` header is emitted; caching remains a no-op on both
  sides.

## Strict-parity pass (Group 50, 2026-11-20)

Source rewritten inside `include/blog/handlers/system/files/` only (folder + docs; no
shared files, no BUILD cycle fix). Helpers live in already-registered partitions:
`routes.cppm`, `handler.cppm`, `workflow/step/execute.cppm`, and the registered DTO
partition. Step `Output` is `std::map<std::string,std::string>` (`url`/`ref`/`filePath`);
`static s_ctx` is retained.

Closed in-folder:

- **Wire output.** `handler.cppm` now emits Ghost's actual serializer shape
  `{"files":[{"url":<absolute URL>,"ref":<ref|null>}]}` with 201 — not `{filePath}`.
  The `ref` value is the multipart form field `ref` (`frame.data.ref`), `null` when absent.
- **URL absolutization.** `execute.cppm` mirrors `utils/serializers/output/files.js:getURL`
  and `LocalStorageBase.save`: `/<subdir>/content/files/YYYY/MM/<name>` is stripped of
  `<subdir>` and prefixed by the site URL (`x-blog-site-url` header, else `SITE_URL`,
  else `http://localhost:2368`).
- **Storage path shape.** `YYYY/MM` target dir, Ghost `getSanitizedFileName`
  (`[^\w@.]` → `-`), and a `-N` uniqueness loop replace the raw-filename path.
- **Extension validation.** `routes.cppm` embeds all 104 entries of
  `config.uploads.files.extensions` and emits Ghost's exact envelopes: 400
  `ValidationError` "Please select a file." and 415 `UnsupportedMediaTypeError`
  "The file type you uploaded is not supported. You can zip your file and upload it as a .zip."
- **Multipart gate.** `content_type_middleware` (JSON-only, 415) was removed from this
  route, so a genuine `multipart/form-data` upload reaches `require_upload_file` instead
  of being rejected.
- **Content-type fidelity.** `storage_content_type` now covers the renderable types in
  `mime-types` vocabulary, including `.txt` → `text/plain` (was `application/octet-stream`).

Still open (recorded, not invented):

- No real file bytes, no filesystem-backed collision probe, no serve headers
  (`Content-Type` / `X-Content-Type-Options: nosniff` / 1-year `max-age`) — need a
  registered storage adapter (new partition, item 4 above).
- No full `authAdminApi` chain (authenticate/authorize/lastSeen/redirect/tokenPermission);
  the C++ chain is bearer-presence only. Shared middleware / new partition needed.
- Caching remains a no-op on both sides (Ghost `headers.cacheInvalidate:false`).

**Verdict unchanged: MISMATCH.** The observable response contract, validation, and path
shape now match, but strict parity is not fully achieved while real storage and the full
admin-auth chain remain outside the folder; per the rule, no verdict flip was made.
