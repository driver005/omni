# Parity: images

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/images.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/images/routes.cppm` (handler: `include/blog/handlers/system/images/images_upload_post/`)
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
| POST `/images/upload` | `upload` (controller key; `docName: 'images'`) | POST `/images/upload` (`Router{..., "/images"}` + `Route{"/upload"}.post`) | `images_images_upload_post()` → `ImagesUploadPostHandler::handle` | Ghost: multipart `file` field (`upload.single('file')`), validation `{type:'images'}`, reads `frame.file.{name,ext,path}`; no query options. C++: `using Input = interfaces::io::IRequest&`, raw `in.body_as_string()` (empty→`"{}"`); no multipart/file parsing, no `file` field | Ghost: `statusCode: 201`, returns plain URL string from `store.save()` (or `processedImageUrl`), no JSON envelope. C++: handler always `set_status(OK)` (200), body `{"images":[{"id":"<n>"}]}` — id envelope, not a URL; ignores `MutationResult.status()` (201) | Ghost: storage adapter `adapterManager.getAdapter('storage:images')`, `store.save(frame.file)` (+ optional `imageTransform.resizeFromPath`, two saves for processed/original); no DB model. C++: `ctx.store().create("images", body)` → generic `ResourceStore::create` cache-backed record, resource string `"images"`; no file storage, no image processing | Ghost: `headers.cacheInvalidate: false` (explicitly suppresses invalidation). C++: writes cache keys `resource:images:<id>` + `resource:images:ids`, `Ttl{}.never()`, tag `resources` (no `images:*` namespace, no explicit eviction) | Ghost: `mw.authAdminApi` middleware route-side; controller `permissions:false` (no permission/docName check) + upload validation middleware. C++: route `content_type_middleware`; handler manual `token(req).empty()` → 401 `{"error":"Unauthorized"}`; no permission model/serde validation | MISMATCH | Ghost: `images.js:11-19`, `images.js:25-89`, `routes.js:460-466`. C++: `system/images/routes.cppm:19-28`, `images_upload_post/handler.cppm:17-27`, `images_upload_post/workflow/step/execute.cppm:11-16`, `context/context.cppm:1206,1272-1276,640-650,725-732` |

## Route details

### POST /images/upload — MISMATCH

Route path and verb agree; operation key (`upload`) maps to the single bound C++ handler. Everything else diverges materially:

- **Input**: Ghost takes a real uploaded multipart file (`routes.js:463` `upload.single('file')`, `routes.js:464` `upload.validation({type:'images'})`) and reads `frame.file.name/ext/path` (`images.js:25,29,32,39,57,77`). C++ declares `using Input = interfaces::io::IRequest&` and only consumes `in.body_as_string()` (empty→`"{}"`) at `execute.cppm:11,16` — no multipart parse, no `file` field, no image-type validation.
- **Output**: Ghost `statusCode: 201` (`images.js:13`) and returns a bare image URL string (`images.js:86,89`). C++ handler hardcodes `res.set_status(Status::OK)` = 200 (`handler.cppm:27`) and returns the `MutationResult` body `{"images":[{"id":"<n>"}]}` (`context.cppm:1273-1276`), discarding the `MutationResult.status()` of 201 (`context.cppm:1274`). Shape and status differ.
- **DB call / resource**: Ghost writes to a storage service via `adapterManager.getAdapter('storage:images')` and `store.save(...)` (`images.js:19,57,77,89`), with optional `imageTransform.resizeFromPath` and original/processed double save (`images.js:32-84`). C++ performs no file I/O: `ctx.store().create("images", body)` (`execute.cppm:16`) routes through `ApiDataStore::create` (`context.cppm:1206`) to the generic `ResourceStore::create` (`context.cppm:640-650`). Resource string `"images"` is not among the special-cased entities (`context.cppm:1210-1271`), so it falls through to a flat cache record. Materially different operation and data shape.
- **Cache**: Ghost explicitly disables invalidation via `headers.cacheInvalidate: false` (`images.js:14-16`). C++ instead writes persisted cache entries `resource:images:<id>` and `resource:images:ids` with `Ttl{}.never()` and tag `resources` (`context.cppm:645-646,725-732,754-755`). No matching invalidation semantics.
- **Auth**: Ghost route-side `mw.authAdminApi` (`routes.js:462`), controller `permissions: false` (`images.js:17`) with no `docName` permission check. C++ uses `content_type_middleware` (`routes.cppm:20`) plus an in-handler token presence check returning 401 (`handler.cppm:18-22`); no permission system or serde validation on either the file payload or body.

## Missing in C++

None. The single Ghost route (`POST /images/upload`, `routes.js:460-466`) has a C++ counterpart (`system/images/routes.cppm:19-28`).

## Extra in C++

None. `system/images/routes.cppm` installs only the `/upload` POST route (`routes.cppm:22-23`), which has a Ghost counterpart.
