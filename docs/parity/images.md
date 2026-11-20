# Parity: images

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/images.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/images/routes.cppm` (handler: `include/blog/handlers/system/images/images_upload_post/`)
- Checked: 2026-11-19; strict-parity rewrite: 2026-11-20 (group 51)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> The 2026-11-19 fix record below asserted a **bare URL string** response and was incorrect:
> Ghost's pipeline applies the `images.upload` output serializer, so the actual wire body is
> `{images:[{url:<absolute>,ref}]}`. The strict-parity rewrite corrects this. The authoritative
> analysis is `docs/parity/behavior/images.md`.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/images/upload` | `upload` (controller key; `docName: 'images'`) | POST `/images/upload` (`Router{..., "/images"}` + `Route{"/upload"}.post`) | `images_images_upload_post()` → `ImagesUploadPostHandler::handle` | Ghost: multipart `file` field (`upload.single('file')`), validation `{type:'images'}` (extension/MIME allowlists, `_o` trim, SVG sanitize, image-content sniff), optional `ref` form field. C++: boundary-aware multipart parse of `file` (`filename`, part `Content-Type`, bytes) + `ref`; 422 missing-file, 415 invalid-file, 400 transform-failure | Ghost: `statusCode:201`, pipeline serializer `images.upload` → `{images:[{url: forImage(path), ref: frame.data.ref||null}]}`, no disposition/location. C++: step map carries `status`/`body`; handler writes that envelope with an absolute URL and `ref:null` when missing, status 201/422/415/400 | Ghost: `adapterManager.getAdapter('storage:images').save()` (+ optional `imageTransform.resizeFromPath` double save); no model call. C++: outbox `image.transform.resize` / `image.store.save` events drained by a local `ImageStoreAdapter` that assigns `content/images/YYYY/MM/<name>` (+ `_o` original); no model call, no marker write | Ghost: `cacheInvalidate:false` → **no** `X-Cache-Invalidate`; app-level `Cache-Control: private`. C++: no `X-Cache-Invalidate`, sets `Cache-Control: private` | Ghost: `mw.authAdminApi` route-side + upload validation middleware; controller `permissions:false`. C++: `bearer_middleware` → `require_auth` → local `require_image_upload` (no `docName` gate, matching `permissions:false`) | MATCH (residual: real bytes/sharp/storage adapter, full auth chain — new-partition needs) | Ghost: `images.js:11-91`, `routes.js:459-466`, `upload.js:262-331`, `utils/serializers/output/images.js:5-20`; C++: `system/images/routes.cppm`, `images_upload_post/{handler,workflow/step/execute}.cppm` |

## Route details

### POST /images/upload — MATCH (strict parity)

Path, verb and operation agree. After the strict-parity rewrite the multipart input + allowlist
validation, the 201/envelope output, the storage-side-effect outbox chain, the header posture and
the auth chain agree at the observable route-contract level. Ghost's real `store.save()` file
writes, sharp resize, SVG sanitization and magic-byte sniffing remain accepted residuals with no
allowed C++ primitive — recorded as new-partition needs in `docs/parity/behavior/images.md`.

Applied strict-parity rewrite (2026-11-20, group 51), all within
`include/blog/handlers/system/images/` (plus this doc and the behavior report):
- `routes.cppm`: removed `content_type_middleware` (it 415'd the multipart body Ghost requires);
  chain is now `bearer_middleware` → `require_auth` → local `require_image_upload` which emits a
  Ghost 422 `ValidationError` `"Please select an image."` for a missing `file` part.
- `images_upload_post/workflow/step/execute.cppm`: `Output = std::map<std::string,std::string>`
  carrying `status`/`body`; static `s_ctx` retained, model-call marker retired. Parses the
  multipart `file` part (`filename`, part `Content-Type`, bytes) and `ref`; allowlist validation
  (422 missing / 415 invalid); trims `_o`; computes `YYYY/MM` target dir and sanitized-unique
  names; emits outbox events through the registered `blog_handlers:outbox_shim`
  (`image.transform.resize` with `defaultMaxWidth=2000`, `image.svg.sanitize`,
  `image.store.save` ×2 processed + `_o` original or ×1), drained by a local `ImageStoreAdapter`;
  absolutizes the saved path via `x-blog-site-url`/`SITE_URL`; returns the Ghost envelope.
- `images_upload_post/handler.cppm`: reads the step map, maps status 201/422/415/400, writes the
  envelope/error body, sets `Cache-Control: private`, emits **no** `X-Cache-Invalidate`.

Original divergence record (retained as evidence; superseded by the rewrite above):

- **Input**: Ghost took a real multipart file (`routes.js:463` `upload.single('file')`,
  `routes.js:464` `upload.validation({type:'images'})`) and read `frame.file.name/ext/path`;
  C++ declared `using Input=IRequest&` and consumed only `in.body_as_string()` with an
  `image/` content-type check — no multipart/file parsing, no allowlist, no `ref`.
- **Output**: Ghost `statusCode:201` and `{images:[{url,ref}]}` via the pipeline serializer;
  C++ hardcoded 200 and returned a bare `"content/images/<id><ext>"` string.
- **DB call / resource**: Ghost used a storage adapter with optional resize double-save; C++
  called `ctx.store().create("images", body)` → generic cache-backed `resource:images:<id>` marker.
- **Cache**: Ghost explicitly disabled invalidation (`headers.cacheInvalidate:false`); C++ wrote
  `resource:images:<id>` / `resource:images:ids` markers with `Ttl{}.never()` and tag `resources`,
  and emitted a stand-in `X-Cache-Invalidate:false`.
- **Auth**: Ghost route-side `mw.authAdminApi` + `permissions:false`; C++ used
  `content_type_middleware` plus an in-handler token-presence 401 with no permission model.

#### Fix

Target: `include/blog/handlers/system/images/{routes.cppm,images_upload_post/handler.cppm,
images_upload_post/workflow/step/execute.cppm}`. No shared-file edits were needed or permitted.

Checks 1–8 closed:
- **1, 2 (route/op): no change.** `POST /images/upload` agrees; Ghost op `upload` stays bound.
- **3 (input):** multipart `file` parse + presence/allowlist validation + `ref` echo; real
  multer/temp-file lifecycle is a recorded new-partition need (no third-party libs).
- **5 (DB call / storage):** the model-call marker is retired; storage side effects are observable
  as outbox events (`image.store.save`, processed + `_o` original when resizing) drained by a
  local adapter. Real bytes/S3/local writes are a new-partition need. No direct SQL, no new global
  state.
- **4 (output/status):** step `Output` is `std::map<std::string,std::string>` (`status`, `body`);
  handler writes `{images:[{url:<absolute>,ref}]}` for 201 and Ghost-shaped error envelopes for
  422/415/400 — correcting the earlier bare-URL fix.
- **6 (cache header):** no `X-Cache-Invalidate`; `Cache-Control: private` set at app level.
- **7 (auth):** `bearer_middleware` + `require_auth` + local multipart guard; no `docName` gate
  (`permissions:false`). Full `authAdminApi` is a recorded new-partition need.
- **8 (evidence/verdict):** `docs/parity/behavior/images.md` flipped MISMATCH → MATCH with the
  strict-parity resolution; this doc corrected to the envelope output.

## Missing in C++

None. The single Ghost route (`POST /images/upload`, `routes.js:460-466`) has a C++ counterpart
(`system/images/routes.cppm`). No change required; nothing to add.

## Extra in C++

None. `system/images/routes.cppm` installs only the `/upload` POST route, the counterpart of
`api.images.upload` (`routes.js:460-466`). Keep the handler registration in `all.cppm`.

## Central registrations

No shared-file edits were needed (none were permitted). All central registrations already exist:
- `include/blog/handlers/all.cppm`: `import :images_images_upload_post_handler`,
  `images_images_upload_post()` accessor, `m_images_images_upload_post` member — unchanged.
- `include/blog/handlers/base.cppm`: `export import :images_images_upload_post_handler` — unchanged.
- `include/blog/handlers/routes.cppm`: `export import :images_routes` and
  `ImagesRoutes{}.install(router, handlers)` — unchanged.
- `blog_handlers_middleware`: `bearer_middleware` / `require_auth` / error helpers already exported
  by `middleware/base.cppm`; mounted locally in `ImagesRoutes::install`.
- `blog_handlers:outbox_shim`: already a registered partition in `blog_handlers/BUILD.bazel`
  (line 919); imported by the step for the image-store side effects. No BUILD edit.

Step output is `std::map<std::string,std::string>` and the context pointer remains the static
`ExecuteStep::s_ctx`; no new global state. The outbox adapter registers once via a function-local
static. New-partition needs (real image-store adapter, shared absolutizer, full admin-auth chain)
are recorded in `docs/parity/behavior/images.md` and deliberately not added here.
