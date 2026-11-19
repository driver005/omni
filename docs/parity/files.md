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

## Missing in C++

None. The sole Ghost route for this controller (`POST /files/upload`) has a C++ counterpart. Confirmed by grep of `api.files` across `docs/Ghost/ghost/core/server/web` (only hit: `admin/routes.js:490`) and grep of `files` in `include/blog/handlers/all.cppm` (registry entries at lines 193, 539, 788).

## Extra in C++

None. `include/blog/handlers/system/files/routes.cppm` installs exactly one route — `Route{"/upload"}.post(...)` at line 22 — matching the Ghost route. Full-file read confirms no other routes in that file; `routes.cppm:18` exports only `:files_routes`.
