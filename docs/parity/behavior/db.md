# Behavior report: db

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/db.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/system/db/` (`routes.cppm`; `browse/`, `create/`, `bulk_destroy/`, `db_backup_post/`, `db_media_inline_post/`)
- Analyzed: 2026-11-19
- Strict-parity recheck: 2026-11-20 (Group 48) — final section is authoritative;
  the per-route C++ facts and Diff below are the pre-strict baseline.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All five Ghost `db` ops have a same-verb/same-path C++ counterpart. Each C++ handler reproduces the verb, token gate and status/header shape but replaces the real service (backup file I/O, exporter, importer/job/email, media-inliner job, Knex transaction + model events) with an in-memory `ctx.store()` stand-in, and drops the permission service. No C++ db route is EXTRA.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/db` (full `/ghost/api/admin/db`) | `exportContent` (docName `db`) | `options:['include','filename']`; `validation.options.include ∈ exporter.BACKUP_TABLES`; `include`→`withRelated` via input serializer; query `filename` selects backup-file mode | `mw.authAdminApi` chain; `permissions:true` → `canThis(ctx).exportContent.db` | 200; `disposition.file` (`Content-Disposition: Attachment; filename="<generated>.json"`); `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate`; JSON | `query`: `?filename` → `dbBackup.readBackup` (404 if absent) else `exporter.doExport`→`DataExportError`→`InternalServerError` | No cache adapter/TTL; invalidation disabled | `{db:[exportData]}` where exportData=`{meta:{exported_on,version},data:{…tables}}` | `system/db/routes.cppm:19-29`; `browse/handler.cppm`; `browse/workflow/{flow/main,step/execute}.cppm` | 200; `Content-Disposition: attachment; filename="<query filename or ghost-export>.json"`; `X-Cache-Invalidate:false`; body = in-memory `db`/posts rows, no `readBackup`, no permissions | PARTIAL | Ghost: `db.js:44-79`, `routes.js:390`, `output/db.js:12-18`, `backup.js:28-43`, `exporter.js:34-70`, `export-filename.js:10-36`, `headers.ts:29-30,93-109`; C++: `routes.cppm:19-29`, `browse/handler.cppm:17-31`, `browse/.../execute.cppm:14-33` |
| POST `/db` | `importContent` (docName `db`) | multipart `importfile` (`upload.single('importfile')` + `upload.validation({type:'db'})`); `frame.file`; `settingsCache.get('timezone')`; `frame.user.get('email')` else `models.User.getOwnerUser()`; importTag `#Import YYYY-MM-DD HH:mm` | `mw.authAdminApi`; `permissions:true` → `canThis(ctx).importContent.db` | `statusCode(result)`: 200 if `result.db||result.problems` else 202 (serializer always sets `db:[]` ⇒ effectively 200); `cacheInvalidate:true` ⇒ `X-Cache-Invalidate: /*` | `query`: `importer.getInstance().importFromFile(frame.file,{user:{email},importTag})` → store upload + dispatch `ContentImportJob` → `processImport` → load/preProcess/doImport/report/cleanup/email | `cacheInvalidate:true` emits `X-Cache-Invalidate: /*`; no cache adapter | `{db:[],problems:[…]}` via `output/db.js.importContent` | `system/db/routes.cppm:30-37`; `create/handler.cppm`; `create/workflow/{flow/main,step/execute}.cppm` | 200/202; `X-Cache-Invalidate:true`; JSON body `importfile`/`file` string decoded, written to `db` resource; no multipart, no job/storage/email, no timezone/owner/importTag | PARTIAL | Ghost: `db.js:81-111`, `routes.js:391-397`, `output/db.js:20-28`, `importer/index.js:17-42`, `import-manager.js:410-445,577-619`, `upload.js:20-23,115-170,270-330`; C++: `routes.cppm:30-37`, `create/handler.cppm:17-29`, `create/.../execute.cppm:14-51` |
| DELETE `/db` | `deleteAllContent` (docName `db`) | none (no `options`/`data`/`validation`) | `mw.authAdminApi`; `permissions:true` → `canThis(ctx).deleteAllContent.db` | `statusCode:204`; `cacheInvalidate:true` ⇒ `X-Cache-Invalidate: /*`; empty body | `query`: `dbBackup.backup()` → `models.Base.transaction` → `Post.findAll({columns:'id'})` + `Post.destroy` via `promisePool(…,100)` → same for `Tag`; errors → `InternalServerError` | `cacheInvalidate:true` emits `X-Cache-Invalidate: /*`; no cache adapter | `{db:[]}` serializer, but 204 suppresses body | `system/db/routes.cppm:38-45`; `bulk_destroy/handler.cppm`; `bulk_destroy/workflow/{flow/main,step/execute}.cppm` | 204; `X-Cache-Invalidate:true`; snapshots posts/tags JSON into `db` resource then `resources().bulk_destroy("posts"/"tags", body)`; in-memory only, body `{}` deletes nothing | PARTIAL | Ghost: `db.js:136-186`, `routes.js:398`, `output/db.js:30-34`, `backup.js:51-70`, `promise-pool`; C++: `routes.cppm:38-45`, `bulk_destroy/handler.cppm:17-29`, `bulk_destroy/.../execute.cppm:13-24`, `context_context.cppm:690-700,1385-1389` |
| POST `/db/backup` | `backupContent` (docName `db`) | `options:['include','filename']`; `validation.options.include ∈ BACKUP_TABLES`; `filename` must be basename (no path separators) → 422 | `mw.authAdminApi`; `permissions:true` → `canThis(ctx).backupContent.db` | 200; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` | `query`: filename guard → set `frame.options.include=withRelated` → `dbBackup.backup(options)` → `exporter.fileName` + `exporter.doExport` → `fs.writeFile(<contentPath>/data/<file>.json)`; `disableJSBackups` → `null` | `cacheInvalidate:false`; no cache adapter | `{db:[{filename}]}` via `output/db.js.backupContent` (filename = written path or null) | `system/db/routes.cppm:46-56`; `db_backup_post/handler.cppm`; `db_backup_post/workflow/{flow/main,step/execute}.cppm` | 200; `X-Cache-Invalidate:false`; includes/filename validated (422 on bad include or path separator); returns snapshot JSON of table rows, not a filename; no file write | PARTIAL | Ghost: `db.js:16-42`, `routes.js:399`, `output/db.js:4-10`, `backup.js:16-23,51-70`, `exporter/index.js:1-8`; C++: `routes.cppm:46-56`, `db_backup_post/handler.cppm:17-29`, `db_backup_post/.../execute.cppm:14-37` |
| POST `/db/media/inline` | `inlineMedia` (docName `db`) | `validation.options.include ∈ ['domains']`; `frame.data.domains` (string list) from request body | `mw.authAdminApi`; `permissions:{method:'importContent'}` → `canThis(ctx).importContent.db` | 200; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` | `query`: `mediaInliner.service.startInlining(frame.data.domains)` → log + `dispatchJob(ExternalMediaInlinerJob{domains||DEFAULT_DOMAINS})` | `cacheInvalidate:false`; no cache adapter | `{db:[{status:'success'}]}` via default `all` serializer (no `inlineMedia` key in `output/db.js`) | `system/db/routes.cppm:57-72`; `db_media_inline_post/handler.cppm`; `db_media_inline_post/workflow/{flow/main,step/execute}.cppm` | 200; `X-Cache-Invalidate:false`; requires non-empty `data.domains` array (422 otherwise), writes it to `db` resource, reads posts with `filter=html:*`; no inliner job/fetch/rewrite | PARTIAL | Ghost: `db.js:113-134`, `routes.js:401`, `media-inliner/index.ts:10-68`, `external-media-inliner.js:676-681`, `serializers/handle.ts:152-169`, `output/default.js:22-41`; C++: `routes.cppm:57-72`, `db_media_inline_post/handler.cppm:17-29`, `db_media_inline_post/.../execute.cppm:14-56` |

## Route details

### GET /db — PARTIAL

**Ghost** (facts 1–8):

1. **Identity.** Admin router: `router.get('/db', mw.authAdminApi, http(api.db.exportContent))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:390`), mounted under `/ghost/api/admin` with router-level `router.use(apiMw.cors)` (`admin/routes.js:15-17`). `mw.authAdminApi` is `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). `http()` builds a `Frame` from `req.query/params/body/file(s)` plus context `{api_key,user,integration,member}` (`packages/api-framework/src/http.ts:86-105`) and freezes/dispatches through the pipeline (`pipeline.ts:334-461`).
2. **Operation.** Controller key `exportContent`, `docName:'db'` (`db.js:13-14,44`). Custom action op (not browse/read/add).
3. **Input.** `options:['include','filename']` (`db.js:45`); `validation.options.include.values = exporter.BACKUP_TABLES` (`db.js:46-52`, table list `data/exporter/table-lists.js:2-91`). Shared `all` validator applies these option rules (`validators/input/all.ts:74-105`); invalid `include` values are **silently filtered**, not rejected (`validators/input/all.ts:89-94`). Input serializer renames `include`→`withRelated` (trimmed/lowercased) and strips internal options (`serializers/input/all.ts:21-24,39-43`). `http()` Frame.configure only picks declared option keys (`frame.ts:66-76`), so `filter[x]=y` is not captured. No `data[]`, no multipart, no path params.
4. **Auth/permissions.** `mw.authAdminApi` chain (above); `permissions:true` (`db.js:60`) drives `STAGES.permissions` (`pipeline.ts:434`) → `utils/permissions.handle` (`utils/permissions.js:94-107`): `permissions.canThis(frame.options.context).exportContent.db` called with `(frame.options.id /* undefined */, {})` (`permissions.js:21-48`). `NoPermissionError` → "You do not have permission to exportContent db" (`permissions.js:7-8,66-73`); other errors → 500 (`permissions.js:75-81`). No labs flag, no `unsafeAttrs`, no in-query permission call.
5. **Status + headers.** No `statusCode` → HTTP 200 (`http.ts:126-133`). `headers:{disposition:{type:'file',value:()=>exporter.fileName()},cacheInvalidate:false}` (`db.js:53-59`): `disposition.file` resolves the async filename and emits `Content-Disposition: Attachment; filename="<value>"` (`headers.ts:29-30,93-109,124-133`); filename from `export-filename.js:10-36` = `path.basename(options.filename)` or `<safe title>.ghost.<YYYY-MM-DD-HH-mm-ss>.json`. `cacheInvalidate:false` is falsy so **no** `X-Cache-Invalidate` header is emitted (`headers.ts:135-144`). Response via `res.json(result)` (`http.ts:139-148`). No `Location`, no stream.
6. **Full call chain.** Pipeline order validation→input serialisation→permissions→query→output serialisation (`pipeline.ts:430-445`). `query(frame)`: if `frame.options.filename` → `dbBackup.readBackup(filename)` (`db.js:63-71`); `readBackup` sanitizes to basename, resolves `<contentPath>/data/<name>`, returns `JSON.parse` or `null` (`data/db/backup.js:28-43`); `null` → `NotFoundError` (`db.js:66-68`, 404). Otherwise `exporter.doExport({include:withRelated})` (`db.js:73-77`): iterates `schema.commands.getTables`, selects only `TABLES_ALLOWLIST` or explicitly included tables, strips `settings.group==='core'` and `SETTING_KEYS_BLOCKLIST`, wraps in `{meta:{exported_on,version},data:{…}}` (`data/exporter/exporter.js:13-70`, `table-lists.js:96-130`); any error → `DataExportError` → `InternalServerError` (`db.js:75-77`).
7. **Cache.** No `cache` adapter, no `generateCacheKeyData`, no TTL/namespace on this op (`db.js:44-79`). `cacheInvalidate:false` only suppresses the invalidation header (`headers.ts:135`).
8. **Output.** `output/db.js.exportContent` sets `frame.response={db:[exportedData]}` (`utils/serializers/output/db.js:12-18`, registered at `utils/serializers/output/index.js:31-32`). Empty/missing result: `readBackup` miss → 404 `NotFoundError`; successful export always has `meta` and `data` keys.

**C++:**

1. **Files/symbols.** `include/blog/handlers/system/db/routes.cppm:12-74` (`DbRoutes::install`); `browse/handler.cppm:9-35` (`BrowseHandler`); `browse/workflow/flow/main.cppm:7-10` (`MainFlow::build`); `browse/workflow/step/execute.cppm:8-78` (`ExecuteStep`). Accessor `db_browse()` at `all.cppm:567,847`; BUILD entries `BUILD.bazel:637-639,970`.
2. **Route + middleware.** `Router{router,"/db"}` with `.use(content_type_middleware)` and `.use(require_auth)` then `.get` → `handler.db_browse().handle(...)` (`routes.cppm:19-29`). `content_type_middleware` 415 on missing/non-JSON Content-Type and 406 on bad Accept (`middleware/serde.cppm:90-126`); `require_auth` only checks bearer token presence, 401 `{errors:[{message:"Unauthorized",type:"UnauthorizedError"}]}` (`middleware/auth.cppm:12-29`). No permission helper, no bearer_middleware, no admin auth chain.
3. **Input parsing.** `Input=IRequest&`, `Output=map<string,string>` (`execute.cppm:10-12`). Query string parsed manually from `get_path()` after `?`, split on `&`/`=` (`execute.cppm:65-75`) — accepts any key. `include` validated against a hardcoded 88-entry table list; invalid → 422 `ApiError{}.validation("Validation error","Invalid include","include")` (`execute.cppm:16-18,38-64`). No `filename` basename validation, no include→withRelated rewrite.
4. **Status/header writes.** Handler reads payload from step result with fallback `{status:"200",body:"[]"}` (`browse/handler.cppm:19-26`), sets status, `Content-Disposition: attachment; filename="<filename>.json"` and `X-Cache-Invalidate: false`, writes bytes and `send()` (`handler.cppm:27-31`). `filename` from query `filename` or `"ghost-export"` (`execute.cppm:31-32`).
5. **Store/body.** `rows=ctx.store().json_for("db",params)` then overwritten by `parse_array(ctx.store().resources().list("db"))` rendered via `record_to_json` when non-empty (`execute.cppm:20-30`). `json_for("db")` returns `[DbWire]` where `DbWire{status:"success"}` (`context_context.cppm:1199-1208`, `model/model_wire_fixtures.cppm:76-79`). No `core::cache::Cache` read/write in the handler.
6. **Side effects.** None (no file write, no export, no DB query).
7. **NOT implemented.** `readBackup` filename mode + 404; real exporter (`meta.exported_on/version`, `data` tables, settings blocklist); permission service; `disableJSBackups` branch; generated filename from settings title/date; silent include filtering; `X-Cache-Invalidate` suppression semantics.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): real permission service (`permissions.js:47` vs token-only `auth.cppm:19-28`); Knex table export + settings secret filtering (`exporter.js:13-70`); `fs` backup-file read (`backup.js:28-43`); Admin API auth chain (`admin/middleware.js:110-118`).
- Divergences:
  - Body shape: Ghost `{db:[{meta,data}]}` (`output/db.js:12-18`) vs C++ `[{"status":"success"}]`/stored rows (`execute.cppm:20-30`, `DbWire:76-79`).
  - `?filename=` Ghost reads a backup file and 404s when missing (`db.js:63-71`); C++ ignores it except as a download filename (`execute.cppm:31-32`).
  - Invalid `include`: Ghost silently filters (`validators/input/all.ts:89-94`), C++ returns 422 (`execute.cppm:16-18`); C++'s hardcoded table list also omits default-export tables like `posts`/`tags` (`execute.cppm:39-62` vs `table-lists.js:96-117`).
  - Ghost emits **no** `X-Cache-Invalidate` (`headers.ts:135` + `db.js:58`); C++ always emits `X-Cache-Invalidate: false` (`handler.cppm:30`).
  - Content-Disposition filename: Ghost generated from settings title/date (`export-filename.js:10-36`); C++ from query param or `ghost-export` (`execute.cppm:32`, `handler.cppm:29`).
  - Extra 415/406 gates on GET (`serde.cppm:90-126`) absent from Ghost route.

### POST /db — PARTIAL

**Ghost** (facts 1–8):

1. **Identity.** `router.post('/db', mw.authAdminApi, apiMw.upload.single('importfile'), apiMw.upload.validation({type:'db'}), http(api.db.importContent))` (`admin/routes.js:391-397`).
2. **Operation.** Controller key `importContent`, `docName:'db'` (`db.js:81,93`).
3. **Input.** Multipart field `importfile` captured by multer single upload (`web/api/middleware/upload.js:115-170`); `validation({type:'db'})` checks file exists, extension/content-type against `config.uploads.db`, errors: `ValidationError` "Please select a database file to import." and `UnsupportedMediaTypeError` "Unsupported file. Please try any of the following formats: {extensions}" (`upload.js:19-23,270-330`). `http()` puts `req.file` into the Frame (`http.ts:86-105`); no declared `options`/`data`/`validation` on the op. Controller reads `settingsCache.get('timezone')` (`db.js:94`) and formats `importTag='#Import '+moment().tz(tz).format('YYYY-MM-DD HH:mm')` (`db.js:95`), resolves email from `frame.user.get('email')` or `(await models.User.getOwnerUser()).get('email')` (`db.js:97-102`).
4. **Auth/permissions.** `mw.authAdminApi` chain; `permissions:true` (`db.js:92`) → `canThis(ctx).importContent.db` (`permissions.js:47`); no `unsafeAttrs`, no labs flag.
5. **Status + headers.** `statusCode(result)` function: 200 when `result && (result.db||result.problems)`, else 202 (`db.js:82-88`); because `output/db.js.importContent` always sets `db:[]` (truthy), HTTP status is effectively always 200 on success (`http.ts:126-133`, `output/db.js:20-28`). `headers.cacheInvalidate:true` (`db.js:89-91`) → `X-Cache-Invalidate: /*` (`headers.ts:9,21-27,135-144`).
6. **Full call chain.** `query` → `importer.getInstance().importFromFile(frame.file,{user:{email},importTag})` (`db.js:104-109`; singleton from `data/importer/index.js:35-42`). `ImportManager.importFromFile`: non-testing env validates file, `storeUpload` (uuid key via `storage:imports` adapter, `import-manager.js:453-479`), constructs `ContentImportJob{uploadKey,fileName,emailRecipient,importTag}` and `jobsService.dispatch(job)` (`import-manager.js:410-432`). Job execution `executeImport`→`processImport`→`loadStoredUpload` + `preProcess` + `doImport` + `generateReport` + cleanup + `mailer.send` completion/failure email (`import-manager.js:525-565,577-619`). Testing env or `runningInJob` runs `loadFile`+`processImport` inline (`import-manager.js:434-445`). Failures are caught in `processImport`, reported in the email, and resolve undefined (`import-manager.js:596-618`).
7. **Cache.** `cacheInvalidate:true` emits `X-Cache-Invalidate: /*`; no cache adapter/key/TTL on this op.
8. **Output.** `output/db.js.importContent` sets `{db:[],problems: response?.data?.problems ?? []}` (`output/db.js:20-28`). No pagination; import details/errors are only in the queued job and notification email.

**C++:**

1. **Files/symbols.** `routes.cppm:30-37`; `create/handler.cppm:9-33`; `create/workflow/flow/main.cppm:7-10`; `create/.../execute.cppm:9-54`. Accessor `db_create()` at `all.cppm:568,848`; BUILD `BUILD.bazel:640-642`.
2. **Route + middleware.** Same `Router{"/db"}` content_type + require_auth chain (`routes.cppm:19-37`); `.post` dispatches `db_create().handle`. No multipart middleware (`upload.single`/`validation`) at all.
3. **Input parsing.** `Input=IRequest&` (`execute.cppm:10`). Reads `body_as_string()` (default `"{}"`) and `decode_import` expects a JSON object with a non-empty string `importfile` or `file` key; optional `timezone`, `ownerEmail`/`email` captured and re-encoded (`execute.cppm:16,35-51`). Missing/empty file → 422 `validation("Validation error","importfile is required","importfile")` (`execute.cppm:18-20`). No timezone computation, no `#Import` tag, no `models.User.getOwnerUser()`.
4. **Store/body.** `mutations.create("db", *decoded)` (`execute.cppm:23`) → default branch of `BlogContext.create` calls `resources().create("db", body)` and returns `{"db":[{"id":"N"}]}` with 201 (`context_context.cppm:1301-1305`, `ResourceStore::create` at `656-667`). `completed` = result body contains `"db"` or `"problems"` → status 200 else 202 (`execute.cppm:24-27`). Body is that mutation result (`execute.cppm:27`).
5. **Status/header writes.** Handler fallback `{status:"202",body:"{}"}` (`create/handler.cppm:21-25`); sets `X-Cache-Invalidate:true`, status, body, `send()` (`handler.cppm:27-29`).
6. **Side effects.** Only an in-memory `db` resource record (`context_context.cppm:656-667`). No upload storage, no job dispatch, no mail/outbox, no fixture import.
7. **NOT implemented.** Multipart upload/validation (`upload.js:115-170,270-330`); `settingsCache`/owner email/importTag (`db.js:94-109`); `ImportManager` job + storage + email chain (`import-manager.js:410-445,577-619`); permission service; real `problems` extraction.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): multer multipart parsing and db upload validation (`upload.js`); `storage:imports` + background job + `GhostMailer` (`import-manager.js:410-445,577-619`); settingsCache timezone + owner-user lookup (`db.js:94-109`); permission service (`permissions.js:47`).
- Divergences:
  - Input channel: Ghost requires a real uploaded file (`req.file`); C++ accepts a JSON string field in the body (`execute.cppm:35-51`).
  - Completion email / job queue / stored upload are absent; C++ writes one in-memory record (`context_context.cppm:1301-1305`).
  - Body: Ghost `{db:[],problems:[…]}` (`output/db.js:20-28`) vs C++ `{"db":[{"id":"N"}]}` with no `problems` key (`execute.cppm:23-27`).
  - Status logic differs in form: Ghost uses `result.db||result.problems` (`db.js:82-88`), C++ string-searches the mutation body (`execute.cppm:24-26`); both land on 200 for the happy path.
  - C++ adds a 415/406 gate absent from Ghost POST route (`serde.cppm:90-126`).

### DELETE /db — PARTIAL

**Ghost** (facts 1–8):

1. **Identity.** `router.delete('/db', mw.authAdminApi, http(api.db.deleteAllContent))` (`admin/routes.js:398`).
2. **Operation.** Controller key `deleteAllContent`, `docName:'db'` (`db.js:136,142`).
3. **Input.** None — no `options`, `data`, `validation`; `async query()` takes no frame (`db.js:142`). No path params, no body, no headers read.
4. **Auth/permissions.** `mw.authAdminApi` chain; `permissions:true` (`db.js:141`) → `canThis(ctx).deleteAllContent.db` (`permissions.js:47`).
5. **Status + headers.** `statusCode:204` (`db.js:140`); `headers.cacheInvalidate:true` (`db.js:137-139`) → `X-Cache-Invalidate: /*` (`headers.ts:21-27,135-144`). 204 → no body sent.
6. **Full call chain.** `query`: `await dbBackup.backup()` (`db.js:143`) → `exporter.fileName`+`doExport`+`fs.writeFile` (`backup.js:51-70`, `16-23`), skipped when `config.disableJSBackups` (`backup.js:52-56`). Then `models.Base.transaction(async transacting => {…})` (`db.js:153`): `queryOpts={columns:'id',context:{internal:true},destroyAll:true,transacting}` (`db.js:154-159`); `Post.findAll(queryOpts)` then `Post.destroy({id,…queryOpts})` through `promisePool(...,100)` (`db.js:161-169`); same for `Tag` (`db.js:171-178`); any error → `InternalServerError` (`db.js:179-183`). Columns limited to `id` deliberately to avoid model-event logic depending on missing fields (`db.js:145-152`).
7. **Cache.** `cacheInvalidate:true` emits `X-Cache-Invalidate: /*`; no cache adapter/TTL.
8. **Output.** `output/db.js.deleteAllContent` sets `{db:[]}` (`output/db.js:30-34`), but 204 status means Express sends no body.

**C++:**

1. **Files/symbols.** `routes.cppm:38-45` (`.delt(...)` is the router's DELETE registration); `bulk_destroy/handler.cppm:9-33`; `bulk_destroy/workflow/flow/main.cppm:7-10`; `bulk_destroy/.../execute.cppm:8-30`. Accessor `db_bulk_destroy()` at `all.cppm:569,849`; BUILD `BUILD.bazel:643-645`.
2. **Route + middleware.** Same `/db` content_type + require_auth chain (`routes.cppm:19-45`). No permission helper.
3. **Input parsing.** Reads `body_as_string()` (default `"{}"`) only to pass to `bulk_destroy` (`execute.cppm:15,22-23`); no validation.
4. **Store/body.** Builds `backup` string `{"posts":<json_for("posts",{})>,"tags":<json_for("tags",{})>}` and calls `mutations.create("db",backup)` (`execute.cppm:17-20`), mirroring `dbBackup.backup()`. Then `resources().bulk_destroy("posts",body)` and `("tags",body)` (`execute.cppm:21-23`). `ResourceStore::bulk_destroy` only removes ids listed under an `"ids"` key (`context_context.cppm:690-700`), so a default `{}` body removes nothing.
5. **Status/header writes.** Step always returns `{status:"204",body:""}` (`execute.cppm:24`); handler fallback same (`bulk_destroy/handler.cppm:21-25`); sets `X-Cache-Invalidate:true`, status 204, empty body, `send()` (`handler.cppm:27-29`).
6. **Side effects.** In-memory snapshot written to `db` resource and attempted in-memory deletes of `posts`/`tags` ids. No real file backup, no transaction, no model events.
7. **NOT implemented.** Real `dbBackup.backup()` file write / `disableJSBackups` (`backup.js:51-70`); Knex transaction and `destroyAll` with `promisePool` concurrency 100 (`db.js:153-184`); model-event side effects; `InternalServerError` branch; permission service.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): Knex transaction + model `destroyAll` with events (`db.js:153-184`); filesystem backup (`backup.js:16-23,51-70`); permission service.
- Divergences:
  - Deletion scope: Ghost removes **all** posts and tags (`destroyAll:true`, `db.js:154-178`); C++ only removes ids in a JSON `"ids"` key and does nothing for the usual empty body (`context_context.cppm:690-700`).
  - Backup is an in-memory JSON blob in the `db` namespace (`execute.cppm:17-20`) instead of a file under `<contentPath>/data` (`backup.js:16-23`).
  - No error path (Ghost 500 on transaction failure, `db.js:179-183`); C++ always 204.
  - Extra 415/406 gate (`serde.cppm:90-126`) on a bodyless DELETE.

### POST /db/backup — PARTIAL

**Ghost** (facts 1–8):

1. **Identity.** `router.post('/db/backup', mw.authAdminApi, http(api.db.backupContent))` (`admin/routes.js:399`).
2. **Operation.** Controller key `backupContent`, `docName:'db'` (`db.js:16,29`).
3. **Input.** `options:['include','filename']` (`db.js:21`); `validation.options.include ∈ exporter.BACKUP_TABLES` (`db.js:22-28`); query `filename` guarded with `path.basename(filename)!==filename → ValidationError "Export filename must not contain path separators"` (`db.js:30-35`). include→withRelated rewrite (`serializers/input/all.ts:21-24`); controller reassigns `frame.options.include=frame.options.withRelated` for the backup service (`db.js:37-38`).
4. **Auth/permissions.** `mw.authAdminApi` chain; `permissions:true` (`db.js:20`) → `canThis(ctx).backupContent.db` (`permissions.js:47`).
5. **Status + headers.** Default 200 (`http.ts:126-133`); `headers.cacheInvalidate:false` (`db.js:17-19`) ⇒ **no** `X-Cache-Invalidate` header (`headers.ts:135-144`). No disposition/location in controller.
6. **Full call chain.** `query` → `dbBackup.backup(frame.options)` (`db.js:40`): `disableJSBackups` → log + `null` (`backup.js:52-56`); else `exporter.fileName(options)` (basename+`.json` or `<title>.ghost.<date>.json`, `export-filename.js:10-36`), `exporter.doExport(options)` (`exporter.js:34-70`), `writeExportFile` → `fs.writeFile(path.resolve(<contentPath>/data/<filename>), JSON.stringify(data))` returning the absolute path (`backup.js:16-23,63-69`).
7. **Cache.** `cacheInvalidate:false`; no cache adapter/key/TTL.
8. **Output.** `output/db.js.backupContent` → `{db:[{filename}]}` (`output/db.js:4-10`); filename is the written path or `null` when backups disabled.

**C++:**

1. **Files/symbols.** `routes.cppm:46-56`; `db_backup_post/handler.cppm:9-33`; `db_backup_post/workflow/flow/main.cppm:7-10`; `db_backup_post/.../execute.cppm:9-82`. Accessor `db_db_backup_post()` at `all.cppm:570,850`; BUILD `BUILD.bazel:646-648`.
2. **Route + middleware.** Sub-route `Route{"/backup"}.post(...)` on the `/db` router with content_type + require_auth chain (`routes.cppm:46-56`). No permission helper.
3. **Input parsing.** Manual query parse (`execute.cppm:16,69-79`); invalid `include` → 422 `Invalid include` (`execute.cppm:17-19`); `filename` containing `/` or `\` → 422 `Export filename must not contain path separators` (`execute.cppm:20-23`); table set = `include` value or default `{"posts","tags","users","settings"}` (`execute.cppm:25-27`).
4. **Store/body.** Builds `snapshot = {"<table>":ctx.store().json_for(table,{}) …}` (`execute.cppm:29-35`), writes it via `mutations.create("db",snapshot)` (`execute.cppm:36`), returns `{status:"200",body:snapshot}` (`execute.cppm:37`).
5. **Status/header writes.** Handler fallback `{status:"200",body:"{}"}` (`db_backup_post/handler.cppm:21-25`); sets `X-Cache-Invalidate:false`, status, body, `send()` (`handler.cppm:27-29`).
6. **Side effects.** One in-memory `db` record; no filesystem write, no outbox.
7. **NOT implemented.** Real backup file write and returned filename (`backup.js:16-23,51-70`); `exporter.fileName`/`doExport` with settings/tables (`export-filename.js:10-36`, `exporter.js:34-70`); `disableJSBackups` branch; permission service. The same hardcoded 88-table list is used for include validation (`execute.cppm:42-68`), which does not contain the C++ default tables (`posts`/`tags`/`users`/`settings`) — an internal inconsistency.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): filesystem backup + `config.disableJSBackups` (`backup.js:16-23,52-69`); real exporter; permission service.
- Divergences:
  - Body: Ghost `{db:[{filename}]}` (`output/db.js:4-10`) vs C++ raw snapshot JSON (`execute.cppm:29-37`).
  - No generated/stored filename; `filename` is only validated, never used to write or name anything (`execute.cppm:20-23`).
  - Invalid `include` is a 422 in C++ (`execute.cppm:17-19`) but silently filtered in Ghost (`validators/input/all.ts:89-94`).
  - Ghost omits `X-Cache-Invalidate` (`headers.ts:135` + `db.js:18`); C++ emits `X-Cache-Invalidate: false` (`handler.cppm:28`).
  - Extra 415/406 gate (`serde.cppm:90-126`).

### POST /db/media/inline — PARTIAL

**Ghost** (facts 1–8):

1. **Identity.** `router.post('/db/media/inline', mw.authAdminApi, http(api.db.inlineMedia))` (`admin/routes.js:401`).
2. **Operation.** Controller key `inlineMedia`, `docName:'db'` (`db.js:113,127`).
3. **Input.** `validation.options.include.values:['domains']` (`db.js:120-126`); `query` reads `frame.data.domains` (`db.js:128`). Body shape `data.domains` (string list). No path params, no multipart, no headers read.
4. **Auth/permissions.** `mw.authAdminApi` chain; **method override** `permissions:{method:'importContent'}` (`db.js:117-119`) → `canThis(ctx).importContent.db` (not `inlineMedia.db`) via `permissions.js:47`.
5. **Status + headers.** Default 200; `headers.cacheInvalidate:false` (`db.js:114-116`) ⇒ no `X-Cache-Invalidate` (`headers.ts:135-144`). No disposition/location.
6. **Full call chain.** `await mediaInliner.service.startInlining(frame.data.domains)` (`db.js:128`); service wired with Post/Tag/User/PostMeta models, storage adapters, `request-external`, urlUtils, file-type/HEIC loaders and `dispatchJob` (`services/media-inliner/index.ts:10-68`). `startInlining` logs `[Background Job] external-media-inliner queued` and dispatches `ExternalMediaInlinerJob({domains: domains?.length ? domains : DEFAULT_DOMAINS})` (`external-media-inliner.js:676-681`). Job execution (`inline`, `external-media-inliner.js:687+`) fetches and rewrites post media into image/media/file storage.
7. **Cache.** `cacheInvalidate:false`; no cache adapter/key/TTL; no cache writes in controller.
8. **Output.** `query` returns `{status:'success'}` (`db.js:129-133`). There is no `inlineMedia` key in `output/db.js` (`utils/serializers/output/db.js:3-35`), so `getBestMatchSerializer` falls back to `default.all` (`serializers/handle.ts:152-169`): no `response.data`/`meta`, hence `frame.response = {db:[{status:'success'}]}` (`utils/serializers/output/default.js:22-41`).

**C++:**

1. **Files/symbols.** Nested `Router{router,"/media"}` + `Route{"/inline"}` in `routes.cppm:57-72`; `db_media_inline_post/handler.cppm:9-33`; `db_media_inline_post/workflow/flow/main.cppm:7-10`; `db_media_inline_post/.../execute.cppm:9-60`. Accessor `db_db_media_inline_post()` at `all.cppm:571,851`; BUILD `BUILD.bazel:649-651,970`.
2. **Route + middleware.** Inner router re-applies `content_type_middleware` + `require_auth` (`routes.cppm:58-61`) before `.post` → `db_db_media_inline_post().handle`. No permission helper / method override.
3. **Input parsing.** `decode_data_domains` requires JSON object with `data.domains` as a **non-empty** array of non-empty strings; otherwise 422 `validation("Validation error","domains must be a non-empty string list","domains")` (`execute.cppm:16-20,31-57`). No `include` option validation; no `include:'domains'` enum.
4. **Store/body.** Writes the domains JSON to `mutations.create("db",*domains)` (`execute.cppm:23`) and calls `ctx.store().json_for("posts",{{"filter","html:*"}})` for the candidate set (`execute.cppm:25`). Returns `{status:"200",body:"{\"status\":\"success\"}"}` (`execute.cppm:26`).
5. **Status/header writes.** Handler fallback same success body/status (`db_media_inline_post/handler.cppm:21-25`); sets `X-Cache-Invalidate:false`, status, body, `send()` (`handler.cppm:27-29`).
6. **Side effects.** One in-memory `db` record; no inliner job, no network fetch, no storage writes, no outbox.
7. **NOT implemented.** `mediaInliner` service/job dispatch and `DEFAULT_DOMAINS` fallback (`external-media-inliner.js:676-681`); permission override to `importContent` (`db.js:117-119`); real image fetch/rewrite/storage (`media-inliner/index.ts:28-68`); include enum validation.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): external HTTP fetch + image storage adapters + background job (`media-inliner/index.ts:10-68`, `external-media-inliner.js:676-681`); permission service with method override (`permissions.js:47`).
- Divergences:
  - Empty/omitted `domains`: Ghost falls back to `DEFAULT_DOMAINS` (`external-media-inliner.js:679`); C++ returns 422 (`execute.cppm:18-20,45`).
  - C++ ignores `include` entirely and validates the body shape instead (`execute.cppm:31-57` vs `db.js:120-126`).
  - "Inlining" only records a `db` record and reads posts (`execute.cppm:23-25`); no job/rewrite.
  - Body envelope: Ghost `{db:[{status:'success'}]}` (`output/default.js:30-36` via `serializers/handle.ts:159-168`) vs C++ raw `{"status":"success"}` (`execute.cppm:26`, `handler.cppm:23-25`).
  - No permission override — token presence only (`auth.cppm:19-28`).
  - Extra 415/406 gates from both routers (`serde.cppm:90-126`, `routes.cppm:59-60`).

## Module-level notes

- Controller registration: `docs/Ghost/ghost/core/server/api/endpoints/index.js:38-39` wraps `./db` with `apiFramework.pipeline`; output serializer registered at `utils/serializers/output/index.js:31-32`; no docName-specific input validator for `db` (`utils/validators/input/index.js` has no `db` key), so only the shared `all` validator + controller `validation.options` run (`validators/handle.ts:46-58`, `pipeline.ts:430-434`).
- All five routes require `mw.authAdminApi` in Ghost (`admin/routes.js:389-401`) vs `content_type_middleware`+`require_auth` in C++ (`system/db/routes.cppm:19-21,58-61`). `require_auth` is token-presence only (`middleware/auth.cppm:19-28`); it does not implement authenticate/authorize, lastSeen, cors, redirects, prettyUrls or tokenPermissionCheck (`admin/middleware.js:110-118`). No permissions service is called anywhere in the C++ db handlers.
- Header divergences are uniform: Ghost `cacheInvalidate:false` means **no** `X-Cache-Invalidate` header (`headers.ts:135`), while all C++ handlers explicitly set `X-Cache-Invalidate:false`; Ghost `cacheInvalidate:true` (`importContent`, `deleteAllContent`) emits `X-Cache-Invalidate: /*` (`headers.ts:9,21-27`), while C++ sets the literal `"true"`.
- No `core::cache::Cache` usage in any db handler. The only cache interaction is indirect through `ctx.store()` (`ResourceStore::create/…` puts records with tag `{"resources"}` and `Ttl{}.never()` at `context_context.cppm:656-667`) and the `db` fixture fallback at `context_context.cppm:1199-1208`.
- Cross-route side-effect chain on the C++ side: `create`, `bulk_destroy` and `db_backup_post` all write to the same in-memory `db` resource, and `browse` reads it back (`execute.cppm` of each; `context_context.cppm:1199-1208`). Ghost instead uses real filesystem backups and DB tables; the only shared cross-route artifact is the backup file consumed by `exportContent?filename=` and created by `backupContent`/`deleteAllContent` (`backup.js:16-43,51-70`).
- Error behavior: Ghost can return 401/403 (auth/permissions), 404 (`readBackup` miss), 422 (filename/include/upload validation), 500 (`InternalServerError` in export/delete) (`db.js:32,66-68,75-77,179-183`; `permissions.js:66-81`; `upload.js:286-303`). C++ returns 401 (empty token), 415/406 (content-type/accept), 422 (local validation) or the happy-path status; it has no 403/404/500 paths for these routes (`middleware/auth.cppm:19-28`, `middleware/serde.cppm:97-125`, each `execute.cppm`).
- The C++ `.delt(...)` call at `routes.cppm:38` is the router's DELETE registration for `/db` (not a typo in this report).
- Verdict rationale: every route exists on both sides with matching verb/path, status and (broadly) header shape, so `MISMATCH` (wrong behavior entirely) and `MISSING`/`EXTRA` do not apply; the real services are replaced by in-memory stand-ins with token-only auth and no permission service, so each is `PARTIAL` (consistent with the convention used in `config.md`, `actions.md`, `comment-replies.md`).

## Strict-parity pass (Group 48, 2026-11-20)

Scope: `include/blog/handlers/system/db/**` and the two db parity reports only.
No shared file or BUILD target was edited; no verdict flipped (all five remain
PARTIAL — full parity needs filesystem/job/network primitives with no allowed
primitive in this folder).

### Current C++ behavior per route (supersedes the baseline facts above)

| Verb + path | Verdict | C++ behavior now | Remaining divergences |
|---|---|---|---|
| GET `/db` | PARTIAL | `system/db/routes.cppm:20-55` mounts a folder-local `require_permission` (shared `blog_handlers_middleware::require_ghost_permission`, docName `db`, method `exportContent`); `content_type_middleware`/`require_auth` removed. `browse/.../execute.cppm:16-42` uses `core_query::parse` for allowlisted `include`/`filename`, silently drops unknown tables, returns `{db:[{meta:{exported_on,version},data:{...}}]}`, applies the settings secret blocklist, reads a stored snapshot for `?filename=` with a 404 miss, and generates `Attachment; filename="<title>.ghost.<ts>.json"` via `browse/handler.cppm:29`. No `X-Cache-Invalidate` header (cacheInvalidate:false). | `exported_on` is epoch ms from wall clock, `version` fixed; no real exporter/ZIP or `fs.readBackup`; simple filename sanitization; shared in-memory authorizer instead of DB-backed `canThis` |
| POST `/db` | PARTIAL | `routes.cppm:20-92` maps method `importContent`; `create/.../execute.cppm:16-43` decodes JSON `importfile`, derives timezone/owner email from settings/users, builds `#Import YYYY-MM-DD HH:mm`, records the job in the `db` resource and returns `{db:[],problems:[]}` at 200; `create/handler.cppm:27` sets `X-Cache-Invalidate: /*`. | JSON string instead of multipart `req.file`; no multer validation, upload storage, ContentImportJob dispatch or notification email; UTC-only timestamps |
| DELETE `/db` | PARTIAL | `routes.cppm` maps method `deleteAllContent`; `bulk_destroy/.../execute.cppm:15-36` snapshots posts+tags under a generated filename into `db`, enumerates every in-memory post/tag id and calls `resources().bulk_destroy`, returns 204 empty body; `bulk_destroy/handler.cppm:27` sets `X-Cache-Invalidate: /*`. | No Knex transaction/`context.internal`/`promisePool(...,100)`, no model events, no `InternalServerError` branch; in-memory rows only |
| POST `/db/backup` | PARTIAL | `db_backup_post/.../execute.cppm:14-50` uses `core_query::parse`, silently drops invalid `include`, keeps the 422 filename path-separator guard, snapshots `TABLES_ALLOWLIST` (or selected tables) with `meta`, stores it keyed by generated filename and returns `{db:[{filename}]}` at 200; `db_backup_post/handler.cppm` emits no cache header (cacheInvalidate:false). | Returns a generated filename, not the absolute written file path; no real `fs.writeFile`/exporter/`disableJSBackups` |
| POST `/db/media/inline` | PARTIAL | Nested `/media` router reuses `require_permission` with method override `importContent` (`routes.cppm:100-110`); `db_media_inline_post/.../execute.cppm:14-33` records an `ExternalMediaInlinerJob` with `data.domains` or Ghost `DEFAULT_DOMAINS` when absent/empty, returns `{db:[{status:'success'}]}` at 200; no cache header. | No real outbox job dispatch, HTTP fetch/rewrite or storage; permission service is the shared in-memory authorizer |

No handler performs its own token check; all five rely on the folder-local
router middleware. Step `Output` remains `std::map<std::string,std::string>`
(`status`/`body`, plus `filename` for GET) and `s_ctx` remains the context handle,
as required by the group contract.

### Divergences closed since the baseline report

- Body envelopes now match Ghost's serializers (`output/db.js` and
  `output/default.js`): `{db:[{meta,data}]}`, `{db:[],problems:[]}`, 204 empty,
  `{db:[{filename}]}`, `{db:[{status:'success'}]}`.
- `X-Cache-Invalidate` semantics: `/*` for `cacheInvalidate:true` (POST/DELETE),
  omitted entirely for `cacheInvalidate:false` (GET/backup/inline).
- `Content-Disposition` is `Attachment; filename="..."` with an exporter-style
  generated name derived from settings title and timestamp.
- `?filename=` now reads a stored context-local snapshot and 404s on a miss.
- Invalid `include` is silently filtered as in Ghost's shared validator;
  default export set is `TABLES_ALLOWLIST`, not the backup table list.
- Settings secret keys are stripped from export data (Ghost's
  `SETTING_KEYS_BLOCKLIST`).
- DELETE enumerates all in-memory post/tag ids so `bulk_destroy` actually clears
  the tables instead of deleting nothing for an empty body.
- Missing/empty `data.domains` falls back to `DEFAULT_DOMAINS` instead of 422.
- Extra 415/406 Content-Type/Accept gates removed from all five routes.

### Residual gaps and new-partition needs

Unchanged residuals from the baseline report still apply (real filesystem
backup/exporter, multer upload + importer job/storage/email, Knex transaction +
model events, media fetch/rewrite, DB-backed permissions). Recorded needs for
the main agent (not created here):

1. Wire `blog_handlers:mapping` (`include/blog/handlers/mapping/{base,clean,wrap}.cppm`)
   into `include/blog/handlers/BUILD.bazel` partitions and `base.cppm` exports so
   folder-local `ghost_wrap` can be replaced by `blog::mapping::wrap`.
2. Add `//include/core/outbox:core_outbox` as a `blog_handlers` dep plus a
   folder-local adapter partition to emit real `ContentImportJob` /
   `ExternalMediaInlinerJob` events.
3. Add shared multipart-upload and filesystem/storage adapter partitions for
   `upload.single('importfile')` and `backup.js` read/write.
4. Add shared HTTP/adapter support for the media fetch/rewrite path.
5. Add a DB-backed role/permission data source for the shared `GhostAuthorizer`.

### Verdicts

No flips. Summary remains MATCH 0 / PARTIAL 5 / MISMATCH 0 / MISSING 0 / EXTRA 0:
every residual is a full-service primitive, so no route reaches MATCH.
