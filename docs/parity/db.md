# Parity: db

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/db.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (DB block lines 389–401)
- C++: `include/blog/handlers/system/db/routes.cppm` (registered via `include/blog/handlers/routes.cppm:12,85`; handlers in `include/blog/handlers/all.cppm:167–171,513–517,762–766`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All five Ghost `/db*` routes exist in C++ at the same path + verb. After applying the Fix blocks, each handler now performs the Ghost operation (export snapshot, import record, delete-all backup+resource destroy, table backup snapshot, inlining job), validates its Ghost options, emits Ghost's status/body envelope, sets Ghost's `X-Cache-Invalidate` header, and is gated by `require_auth` in the router chain. Residual gaps (export/backup file format, multipart file IO, physical in-memory model row deletion, media network fetch/rewrite, permission metadata) keep all five at PARTIAL.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/db` | `exportContent` | GET `/db` (`routes.cppm:21`) | `db_browse()` (`routes.cppm:26`) | Ghost `options[]` include, filename + validation (db.js:45–52); C++ `Input=IRequest&`, no query parse (execute.cppm:11) | Ghost file disposition + export stream (db.js:53–59,64,74); C++ `{"db":[...],"meta":{"pagination":{limit:15,page:1,pages:1,total:0}}}` hardcoded (execute.cppm:17) | Ghost `dbBackup.readBackup` / `exporter.doExport` (db.js:64,74); C++ `json_for("db",{})` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:58); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:390; db.js:60); C++ `token()` 401 (handler.cppm:18–21) + content_type (routes.cppm:20) | PARTIAL | Ghost routes.js:390, db.js:44–79; C++ routes.cppm:19–28, browse/handler.cppm:17–27, browse/.../execute.cppm:13–17 |
| POST `/db` | `importContent` | POST `/db` (`routes.cppm:29`) | `db_create()` (`routes.cppm:34`) | Ghost multipart `importfile` upload + validation (routes.js:394–395); C++ raw `body_as_string()` (execute.cppm:16) | Ghost conditional `statusCode` 200/202, no envelope (db.js:82–88); C++ `create()` body, 200 (handler.cppm:27) | Ghost `importer.importFromFile` (db.js:104); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:true` (db.js:90); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:393; db.js:92); C++ `token()` 401 (handler.cppm:18–21) | PARTIAL | Ghost routes.js:391–397, db.js:81–111; C++ routes.cppm:29–36, create/handler.cppm:17–27, create/.../execute.cppm:13–16 |
| DELETE `/db` | `deleteAllContent` | DELETE `/db` (`.delt()`, routes.cppm:37) | `db_bulk_destroy()` (`routes.cppm:42`) | Ghost none (no options/data, db.js:142); C++ `Input=IRequest&`, body unused except default `{}` (execute.cppm:11,16) | Ghost `statusCode:204`, no body (db.js:140); C++ 200 + `{"meta":{"deleted":N}}` (handler.cppm:27; context.cppm:1359) | Ghost `dbBackup.backup()` + `models.Post/Tag.findAll/destroy` in transaction (db.js:143,153–184); C++ `mutations.bulk_destroy("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:true` (db.js:138); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:398; db.js:141); C++ `token()` 401 (handler.cppm:18–21) | PARTIAL | Ghost routes.js:398, db.js:136–186; C++ routes.cppm:37–44, bulk_destroy/handler.cppm:17–27, bulk_destroy/.../execute.cppm:13–16 |
| POST `/db/backup` | `backupContent` | POST `/db/backup` (routes.cppm:45–46) | `db_db_backup_post()` (`routes.cppm:52`) | Ghost `options[]` include, filename + validation (db.js:21–28); C++ raw `body_as_string()` (execute.cppm:16) | Ghost backup result, no explicit status/envelope (db.js:40); C++ `create()` body, 200 (handler.cppm:27) | Ghost `dbBackup.backup(frame.options)` (db.js:40); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:18); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:399; db.js:20); C++ `token()` 401 (handler.cppm:18–21) | PARTIAL | Ghost routes.js:399, db.js:16–42; C++ routes.cppm:45–55, db_backup_post/handler.cppm:17–27, db_backup_post/.../execute.cppm:13–16 |
| POST `/db/media/inline` | `inlineMedia` | POST `/db/media/inline` (routes.cppm:56–59) | `db_db_media_inline_post()` (`routes.cppm:65`) | Ghost `frame.data.domains` + validation include['domains'] (db.js:120–128); C++ raw `body_as_string()` (execute.cppm:16) | Ghost `{"status":"success"}` (db.js:130–132); C++ `create()` body, 200 (handler.cppm:27) | Ghost `mediaInliner.service.startInlining(frame.data.domains)` (db.js:128); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:115); C++ none | Ghost `authAdminApi`+`permissions.method:'importContent'` (routes.js:401; db.js:117–119); C++ `token()` 401 (handler.cppm:18–21) | PARTIAL | Ghost routes.js:401, db.js:113–134; C++ routes.cppm:56–69, db_media_inline_post/handler.cppm:17–27, db_media_inline_post/.../execute.cppm:13–16 |

## Route details

### GET /db — PARTIAL

- Operation: Ghost `exportContent` exports the DB as a file (db.js:44–79); C++ `db_browse` lists a fixture resource (browse/.../execute.cppm:16).
- Input: Ghost `options=['include','filename']` with `include` restricted to `exporter.BACKUP_TABLES` (db.js:45–52,24–26); C++ `Input=interfaces::io::IRequest&` with no query/option parsing (browse/.../execute.cppm:11).
- Output: Ghost sets `headers.disposition` file download + `cacheInvalidate:false` and returns export stream/backup (db.js:53–59,64,74); C++ returns hardcoded `{"db":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with status 200 (browse/.../execute.cppm:17; browse/handler.cppm:27).
- DB call: Ghost `dbBackup.readBackup(filename)` or `exporter.doExport({include:withRelated})` (db.js:64,74); C++ `ctx.store().json_for("db",{})` — fixture only, no filtering/pagination (browse/.../execute.cppm:16; context.cppm:1170–1179).
- Cache: Ghost `cacheInvalidate:false` (db.js:58); C++ none (cache is only wired for members/emails/automations in all.cppm:836–839).
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (routes.js:390; db.js:60); C++ bearer `token()` non-empty check → 401 `{"error":"Unauthorized"}` (browse/handler.cppm:18–21) plus `content_type_middleware` (routes.cppm:20). No permission/docName model.

#### Fix

Target: `include/blog/handlers/system/db/browse/workflow/step/execute.cppm` — `blog::handlers::db::browse::workflow::ExecuteStep::invoke` (`:10-17`); header in `browse/handler.cppm:17-27`; store branch `ApiDataStore::json_for("db", ...)` (`context.cppm:1186-1195`); auth at `system/db/routes.cppm:19-28`. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`; no third-party libs, direct SQL, Ghost JS, or new global state — cross-request state stays in the context-local `resources` namespace.

Checks 1–8 closed:
- 1 (route identity): no change — `GET /db` (`admin/routes.js:390`) ↔ `Router{router,"/db"}.get(...)` (`routes.cppm:19-28`).
- 2 (operation): rename the binding to an export-named handler (`db_export_content()`/`DbExportContentHandler`) or keep `db_browse()` with the export semantics below; the route already maps 1:1 to `exportContent` (`db.js:44-79`). No new route.
- 3 (input): replace `ctx.store().json_for("db", {})` (`execute.cppm:16`) with parsed `include`/`filename` from `in.get_path()` (split on `'?'`/`'&'`/`'='`, pattern `posts/browse/.../execute.cppm:22-29`); validate `include` against the BACKUP_TABLES set (`db.js:45-52,24-26`) and reject unknown values with `ApiError{}.validation(...)`.
- 5 (DB call): replace the call with `ctx.store().json_for("db", params)` and extend the `"db"` branch (`context.cppm:1186-1195`) to merge stored context-local snapshots via `parse_array(m_resources.list("db"))`, filtered by `params`, instead of a single `DbWire{}` fixture. No SQL, no new resource model.
- 4 (output): replace the hardcoded paginated envelope (`execute.cppm:17`) with the export body from check 5; add `res.set_header("Content-Disposition", "attachment; filename=\"" + filename + ".json\"")` in `browse/handler.cppm` before `send()`, keep status 200 and add `X-Cache-Invalidate:"false"` (Ghost `cacheInvalidate:false`, `db.js:58`). Real ZIP stream generation has no allowed primitive — JSON snapshot stream under the `db` namespace is the accepted residual.
- 6 (cache): Ghost sets only `cacheInvalidate:false` (`db.js:58`); no `core::cache::Cache` key is needed, the header above closes this check.
- 7 (auth): replace the inline `token(req).empty()` 401 (`browse/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` on `routes.cppm:19-28`, keeping `content_type_middleware`. `permissions:true` is controller metadata with no allowed C++ primitive — accepted residual.
- 8 (verdict): applied — re-rated MISMATCH→PARTIAL (export stream format residual); evidence lines retained.

```cpp
std::map<std::string,std::string> params; // include/filename parsed from in.get_path()
const auto body = ctx.store().json_for("db", params); // real context-local snapshot
res.set_header("Content-Disposition", "attachment; filename=\"" + filename + ".json\"");
```

### POST /db — PARTIAL

- Operation: Ghost `importContent` imports a DB from an uploaded file (db.js:81–111); C++ `db_create` performs a generic create on resource `"db"` (create/.../execute.cppm:16).
- Input: Ghost multipart single file field `importfile` + `upload.validation({type:'db'})` (routes.js:394–395), derives timezone and owner email (db.js:94–101); C++ passes `in.body_as_string()` as JSON (create/.../execute.cppm:16).
- Output: Ghost conditional `statusCode(result)` returns 200 when `result.db||result.problems`, else 202 (db.js:82–88); C++ always returns 200 with the created body (create/handler.cppm:27). No `{db,problems}` shape.
- DB call: Ghost `importer.getInstance().importFromFile(frame.file,...)` (db.js:104); C++ `mutations.create("db",body)` (create/.../execute.cppm:16).
- Cache: Ghost `headers.cacheInvalidate:true` (db.js:90); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:393; db.js:92); C++ `token()` 401 check (create/handler.cppm:18–21).

#### Fix

Target: `include/blog/handlers/system/db/create/workflow/step/execute.cppm` — `blog::handlers::db::create::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `create/handler.cppm:17-27`; multipart at `admin/routes.js:391-397`; auth at `routes.cppm:29-36`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /db` (`admin/routes.js:391-397`, multipart) ↔ `routes.cppm:29-36`.
- 2 (operation): rename binding to `db_import_content()` or keep `db_create()` with import semantics below; route maps to `importContent` (`db.js:81-111`).
- 3 (input): replace raw `in.body_as_string()` (`execute.cppm:16`) with the decoded upload payload via `core_serde::JsonCodec`/`JsonBody` parser (`context.cppm:1203-1213`): read the `importfile` field (multipart parser has no allowed primitive — accept a JSON-wrapped base64/file-name payload as the residual), derive `timezone` and `ownerEmail` (`db.js:94-101`), and reject a missing file with `ApiError{}.validation(...,"importfile")`.
- 5 (DB call): `mutations.create("db", body)` writes the import record into the context-local `db` resource (`ResourceStore`, `context.cppm:1222+`); the imported rows become the snapshot read by the GET fix. No direct SQL.
- 4 (output): change `Output` to `std::map<std::string,std::string>` carrying `status`/`body`; in `create/handler.cppm:27` set `Status::OK` (200) when the result contains `db` or `problems`, else `Status::ACCEPTED` (202) (`db.js:82-88`), with no extra envelope. 
- 6 (cache): Ghost `cacheInvalidate:true` (`db.js:90`) → `res.set_header("X-Cache-Invalidate","true")` in `create/handler.cppm` before `send()`; no cache key needed.
- 7 (auth): replace inline token check (`create/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:29-36`, keeping `content_type_middleware`; `permissions:true` accepted residual.
- 8 (verdict): applied — re-rated MISMATCH→PARTIAL (multipart/file-IO residual); evidence lines retained.

```cpp
using Output = std::map<std::string,std::string>;
const auto result = mutations.create("db", decoded_import_body);
const int status = (result.body().find("\"db\"") != std::string::npos ||
                    result.body().find("\"problems\"") != std::string::npos) ? 200 : 202;
```

### DELETE /db — PARTIAL

- Verb wiring: Ghost `router.delete` (routes.js:398); C++ `.delt()` with summary "Ghost db DELETE" (routes.cppm:37–44) — verb matches.
- Operation: Ghost `deleteAllContent` backs up then deletes all Posts and Tags in a transaction with `columns:id`/`context.internal`/`destroyAll` and a 100-way promise pool (db.js:136–186); C++ `db_bulk_destroy` calls generic `mutations.bulk_destroy("db",body)` (bulk_destroy/.../execute.cppm:16).
- Output: Ghost `statusCode:204`, no body (db.js:140,142); C++ returns 200 + `{"meta":{"deleted":N}}` (bulk_destroy/handler.cppm:27; context.cppm:1359).
- DB call: Ghost `dbBackup.backup()`, `models.Post.findAll/destroy`, `models.Tag.findAll/destroy` (db.js:143,162–178); C++ resource string `"db"` only, no real post/tag deletion (bulk_destroy/.../execute.cppm:16; context.cppm:1356–1359).
- Cache: Ghost `cacheInvalidate:true` (db.js:138); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:398; db.js:141); C++ `token()` 401 check (bulk_destroy/handler.cppm:18–21).

#### Fix

Target: `include/blog/handlers/system/db/bulk_destroy/workflow/step/execute.cppm` — `blog::handlers::db::bulk_destroy::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `bulk_destroy/handler.cppm:17-27`; auth at `routes.cppm:37-44`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `DELETE /db` (`admin/routes.js:398`) ↔ `.delt()` route (`routes.cppm:37-44`).
- 2 (operation): rename to `db_delete_all_content()` or keep `db_bulk_destroy()` with the delete-all semantics below; route maps to `deleteAllContent` (`db.js:136-186`).
- 3 (input): no change — neither side reads params (`db.js:142`; `Input=IRequest&`, body unused).
- 5 (DB call): before deleting, snapshot the current posts/tags via `ctx.store().json_for("posts", {})` and `ctx.store().json_for("tags", {})` into the context-local `db` namespace, then replace `mutations.bulk_destroy("db", body)` (`execute.cppm:16`) with `ctx.store().resources().bulk_destroy("posts", body)` and `("tags", body)` (`context.cppm:690-698`) — this is the allowed `ResourceStore` equivalent of `models.Post/Tag.destroyAll`. Physically deleting the in-memory model tables is impossible without direct SQL — accepted residual.
- 4 (output): change `Output` to a status/body map; in `bulk_destroy/handler.cppm:27` set `Status::NO_CONTENT` (204) with an empty body (`db.js:140,142`) instead of 200 + `{"meta":{"deleted":N}}`.
- 6 (cache): Ghost `cacheInvalidate:true` (`db.js:138`) → `res.set_header("X-Cache-Invalidate","true")` in `bulk_destroy/handler.cppm` before `send()`; no cache key needed.
- 7 (auth): replace inline token check (`bulk_destroy/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:37-44`; `permissions:true` accepted residual.
- 8 (verdict): applied — re-rated MISMATCH→PARTIAL (in-memory model rows residual); evidence lines retained.

```cpp
const auto backup = ctx.store().json_for("posts", {}) + ctx.store().json_for("tags", {});
mutations.create("db", backup); // context-local snapshot, mirrors dbBackup.backup()
ctx.store().resources().bulk_destroy("posts", body);
ctx.store().resources().bulk_destroy("tags", body);
// handler: Status::NO_CONTENT, empty body, X-Cache-Invalidate:true
```

### POST /db/backup — PARTIAL

- Operation: Ghost `backupContent` creates a backup via `dbBackup.backup(frame.options)` (db.js:16–42); C++ `db_db_backup_post` does a generic create on resource `"db"` (db_backup_post/.../execute.cppm:16) — no backup generated.
- Input: Ghost `options=['include','filename']`, `include` validated against `exporter.BACKUP_TABLES`, plus path-separator validation on `filename` (db.js:21–38); C++ raw `body_as_string()` (db_backup_post/.../execute.cppm:16).
- Output: Ghost returns backup result with `cacheInvalidate:false` (db.js:17–19,40); C++ returns `create()` body with 200 (db_backup_post/handler.cppm:27).
- DB call: Ghost `dbBackup.backup(frame.options)` (db.js:40); C++ `mutations.create("db",body)` (db_backup_post/.../execute.cppm:16).
- Cache: Ghost `cacheInvalidate:false` (db.js:18); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:399; db.js:20); C++ `token()` 401 check (db_backup_post/handler.cppm:18–21).

#### Fix

Target: `include/blog/handlers/system/db/db_backup_post/workflow/step/execute.cppm` — `blog::handlers::db::db_backup_post::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `db_backup_post/handler.cppm:17-27`; auth at `routes.cppm:45-55`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /db/backup` (`admin/routes.js:399`) ↔ `routes.cppm:45-55`.
- 2 (operation): rename to `db_backup_content()` or keep `db_db_backup_post()` with backup semantics below; route maps to `backupContent` (`db.js:16-42`).
- 3 (input): replace raw body (`execute.cppm:16`) with parsed `include`/`filename` from `in.get_path()` (same split/validation as GET /db); validate `include` against BACKUP_TABLES and reject path separators in `filename` (`db.js:21-38`) with `ApiError{}.validation(...)`.
- 5 (DB call): replace `mutations.create("db", body)` with a snapshot body built from `ctx.store().json_for(r, {})` for each selected table/`include` value, written via `mutations.create("db", snapshotJson)` into the context-local `db` namespace (`ResourceStore`); no exporter lib, no SQL — real backup-file generation remains an accepted residual.
- 4 (output): return the stored backup snapshot body with status 200 (`db.js:40`) instead of the raw create echo; add `res.set_header("X-Cache-Invalidate","false")` in `db_backup_post/handler.cppm` (`db.js:18`) before `send()`.
- 6 (cache): Ghost `cacheInvalidate:false` only; the header above closes this check — no `core::cache::Cache` key required.
- 7 (auth): replace inline token check (`db_backup_post/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:45-55`; `permissions:true` accepted residual.
- 8 (verdict): applied — re-rated MISMATCH→PARTIAL (backup-format residual); evidence lines retained.

```cpp
std::string snapshot = "["; // concatenate ctx.store().json_for(table, {}) for each include
const auto result = mutations.create("db", snapshot); // context-local backup record
// handler: Status::OK, result.body(), X-Cache-Invalidate:false
```

### POST /db/media/inline — PARTIAL

- Operation: Ghost `inlineMedia` starts the media inliner on `frame.data.domains` (db.js:113–134); C++ `db_db_media_inline_post` does a generic create on resource `"db"` — no inlining (db_media_inline_post/.../execute.cppm:16).
- Input: Ghost `frame.data.domains` with validation `include` values `['domains']` (db.js:120–128); C++ raw `body_as_string()` (db_media_inline_post/.../execute.cppm:16).
- Output: Ghost returns `{"status":"success"}` (db.js:130–132); C++ returns `create()` body with 200 (db_media_inline_post/handler.cppm:27).
- DB call: Ghost `mediaInliner.service.startInlining(frame.data.domains)` (db.js:128); C++ `mutations.create("db",body)` (db_media_inline_post/.../execute.cppm:16).
- Cache: Ghost `cacheInvalidate:false` (db.js:115); C++ none.
- Auth: Ghost `authAdminApi` + `permissions.method:'importContent'` (routes.js:401; db.js:117–119); C++ `token()` 401 check (db_media_inline_post/handler.cppm:18–21).

#### Fix

Target: `include/blog/handlers/system/db/db_media_inline_post/workflow/step/execute.cppm` — `blog::handlers::db::db_media_inline_post::workflow::ExecuteStep::invoke` (`:10-16`); status/header in `db_media_inline_post/handler.cppm:17-27`; auth at `routes.cppm:56-69`. Allowed modules as in the GET Fix; no new global state.

Checks 1–8 closed:
- 1 (route identity): no change — `POST /db/media/inline` (`admin/routes.js:401`) ↔ `routes.cppm:56-69`.
- 2 (operation): rename to `db_inline_media()` or keep `db_db_media_inline_post()` with inlining-request semantics below; route maps to `inlineMedia` (`db.js:113-134`).
- 3 (input): replace raw `in.body_as_string()` (`execute.cppm:16`) with the decoded `data.domains` array via `core_serde::JsonCodec`; validate it is a non-empty string list (`db.js:120-128`) and reject invalid input with `ApiError{}.validation(...,"domains")`.
- 5 (DB call): replace `mutations.create("db", body)` with `mutations.create("db", innerDomainsJson)` (record the inlining job in the context-local `db` namespace) and, if a result is available, `ctx.store().json_for("posts", {{"filter","html:*"}})` to drive the inlining — actual network fetch/rewrite has no allowed primitive — accepted residual.
- 4 (output): set the body to `{"status":"success"}` (`db.js:130-132`) with status 200 in `db_media_inline_post/handler.cppm`, replacing the raw create echo.
- 6 (cache): Ghost `cacheInvalidate:false` (`db.js:115`) → `res.set_header("X-Cache-Invalidate","false")` before `send()`; no cache key needed.
- 7 (auth): replace inline token check (`db_media_inline_post/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` at `routes.cppm:56-69`; Ghost `permissions.method:'importContent'` (`db.js:117-119`) is metadata with no allowed C++ primitive — accepted residual.
- 8 (verdict): applied — re-rated MISMATCH→PARTIAL (media-inliner residual); evidence lines retained.

```cpp
const auto body = in.body_as_string().empty() ? "{}" : in.body_as_string();
const auto domains = decode_data_domains(body); // core_serde
mutations.create("db", domains_json);           // context-local inlining job
// handler: 200 {"status":"success"}, X-Cache-Invalidate:false, require_auth
```

## Applied source fixes

Route-table Input/Output/DB call/Cache columns above describe the pre-fix baseline; applied behavior lives in the edited files below.

| Ghost op | Files changed | Applied behavior | Residual |
|---|---|---|---|
| `exportContent` (GET `/db`) | `browse/workflow/step/execute.cppm`, `browse/handler.cppm` | Parses `include`/`filename` from the query string, validates `include` against `BACKUP_TABLES`, builds the export body from context-local `db` snapshot rows, returns `{status,body,filename}`; handler sets `Content-Disposition` + `X-Cache-Invalidate:false` (200/422) | Real ZIP/export stream; `permissions:true` metadata |
| `importContent` (POST `/db`) | `create/workflow/step/execute.cppm`, `create/handler.cppm` | Decodes JSON-wrapped `importfile` + `timezone`/`ownerEmail` via `core_serde`, rejects missing file with `ApiError{}.validation(...,"importfile")`, writes context-local `db` record, status 200 when result carries `db`/`problems` else 202, `X-Cache-Invalidate:true` | Multipart file IO / importer |
| `deleteAllContent` (DELETE `/db`) | `bulk_destroy/workflow/step/execute.cppm`, `bulk_destroy/handler.cppm` | Snapshots `posts`+`tags` into context-local `db`, `resources().bulk_destroy` on `posts`/`tags`, 204 empty body, `X-Cache-Invalidate:true` | Physical in-memory model row deletion |
| `backupContent` (POST `/db/backup`) | `db_backup_post/workflow/step/execute.cppm`, `db_backup_post/handler.cppm` | Parses/validates `include` (`BACKUP_TABLES`) and `filename` (no path separators), builds a per-table JSON snapshot from `json_for`, stores it in context-local `db`, returns snapshot body at 200 with `X-Cache-Invalidate:false` | Real backup-file generation |
| `inlineMedia` (POST `/db/media/inline`) | `db_media_inline_post/workflow/step/execute.cppm`, `db_media_inline_post/handler.cppm` | Decodes `data.domains` into a validated non-empty string list, records the inlining job in context-local `db`, returns `{"status":"success"}` at 200 with `X-Cache-Invalidate:false` | Media network fetch/rewrite; `permissions.method` metadata |

All five handlers moved bearer-token rejection out of the handler into the router chain; `system/db/routes.cppm` now mounts `require_auth` on the `/db` router (covers GET/POST/DELETE `/db` and `/db/backup`) and on the nested `/media` router (covers `/db/media/inline`), keeping `content_type_middleware`.

## Central registrations

None required. No shared file was edited.

- Handler names/bindings kept unchanged (`db_browse`, `db_create`, `db_bulk_destroy`, `db_db_backup_post`, `db_db_media_inline_post`), so `include/blog/handlers/{all.cppm,base.cppm,routes.cppm}` need no updates.
- `DbRoutes::install` (`system/db/routes.cppm`) still registers the same five route+verb pairs; auth is now folder-local middleware, no central auth registration.
- Cache parity is expressed solely via `X-Cache-Invalidate` response headers; no `core::cache::Cache` key or central cache wiring is needed.
- `ApiDataStore::json_for("db", ...)` in the shared `context/` branch was left untouched (forbidden shared file). The GET fix performs the equivalent merge of context-local snapshots via `resources().list("db")` + `parse_array` + `record_to_json` inside the browse step instead.

## Missing in C++

None. All five Ghost `/db*` routes have a C++ route at the same path + verb.

Absence check: grep for `api.db.` / `db.` in `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` found exactly the five DB routes (lines 390–401); grep `"/db` across `include/blog/handlers/**/routes.cppm` found only `include/blog/handlers/system/db/routes.cppm:19`, whose five verbs cover all five Ghost routes.

#### Fix

No change required. Check 8 (MISSING) is closed: all five Ghost `/db*` routes (`admin/routes.js:390-401`) have C++ routes at the same path+verb (`system/db/routes.cppm:19-69`), confirmed by grepping `api.db.`/`db.` in the admin routes (exactly five matches at lines 390–401) and `"/db` across `include/blog/handlers/**/routes.cppm` (only `system/db/routes.cppm:19`). Do not add routes; the five route-detail Fix blocks close checks 1–8. No `MISSING` entry exists here.

## Extra in C++

None. The C++ `DbRoutes` folder (`include/blog/handlers/system/db/routes.cppm`) installs exactly five routes — GET/POST/DELETE `/db`, POST `/db/backup`, POST `/db/media/inline` — all five corresponding to Ghost controller operations. No additional db routes exist in that folder (verified by reading routes.cppm:19–69).

#### Fix

No change required. Check 8 (EXTRA) is closed: `DbRoutes::install` (`routes.cppm:19-69`) registers exactly five routes, one per Ghost operation (`admin/routes.js:390,393,398,399,401`); no fifth-plus C++ route lacks a Ghost counterpart. Do not delete or gate any db route — the route-detail Fix blocks above close checks 1–8 by changing handler/step behavior, not route registration.

## Strict-parity pass (Group 48)

Re-reviewed against `docs/parity/behavior/db.md` Diff sections and CONTRACTS.md.
The tables above describe the pre-strict baseline; this section is authoritative
for the current folder. Verdicts were **not** flipped: all five routes remain
PARTIAL because the real services (filesystem, importer job/email, Knex
transaction/model events, media fetch/rewrite) still have no allowed primitive.

### Shared abstractions now used

- `blog_handlers_middleware::require_ghost_permission` — mounted via a
  folder-local `db_routes_detail::require_permission` in `system/db/routes.cppm`
  on both the `/db` and `/media` routers. It maps GET→`exportContent`,
  DELETE→`deleteAllContent`, POST→`importContent`, POST `/backup`→`backupContent`
  and POST `/media/inline`→`importContent` (the Ghost method override at
  `db.js:117-119`), resolves a presented admin token to the staff Administrator
  principal, and writes Ghost-shaped 401/403 envelopes. `content_type_middleware`
  and the token-presence `require_auth` were removed: Ghost mounts only
  `mw.authAdminApi` (`admin/routes.js:389-401`), so the extra 415/406 gates are gone.
- `core_query::parse` — allowlisted `include`/`filename` parsing for GET `/db`
  and POST `/db/backup` (URL-decoded, no hand-rolled query splitter).
- `core_serde::JsonCodec` — all JSON quoting/decoding and `data.domains` parsing.
- `blog::api::{parse_array,record_to_json,ApiError}` — context-local snapshot
  reads and Ghost validation envelopes.
- Existing `blog_context` store/ResourceStore — context-local snapshots,
  import/inliner job records and explicit `destroyAll` by enumerated ids.

### Divergences closed

| Route | Before | After |
|---|---|---|
| GET `/db` | Raw stored rows; 422 on bad `include`; `X-Cache-Invalidate:false`; lowercase `attachment`; no 404 | `{db:[{meta:{exported_on,version},data:{...}}]}`; invalid `include` silently dropped; no cache header; `Attachment; filename="<generated>.json"` (title from settings + timestamp); `?filename=` reads a stored snapshot and 404s on a miss; settings secret blocklist applied |
| POST `/db` | Raw `{"db":[{"id":"N"}]}` echo; `X-Cache-Invalidate:true` | `{db:[],problems:[]}` at 200; `X-Cache-Invalidate: /*`; timezone/owner email/`#Import` tag derived from context-local settings/users; job recorded in `db` resource |
| DELETE `/db` | Snapshot + `bulk_destroy` with empty `{}` (deleted nothing); header `"true"` | Snapshot with generated filename first; all in-memory post/tag ids enumerated and destroyed; 204 empty body; `X-Cache-Invalidate: /*` |
| POST `/db/backup` | Raw snapshot body; internal table-list inconsistency; header `"false"` | `{db:[{filename}]}`; `backupContent` filename path-separator guard kept as 422; invalid `include` silently dropped; default export uses `TABLES_ALLOWLIST`; no cache header; snapshot stored under generated filename for `?filename=` reads |
| POST `/db/media/inline` | 422 on missing/empty domains; raw `{"status":"success"}`; header `"false"` | Missing/empty domains fall back to Ghost `DEFAULT_DOMAINS`; `{db:[{status:"success"}]}`; no cache header; job recorded in `db` resource |

All step `Output` types stay `std::map<std::string,std::string>` with
`status`/`body` (+ `filename` for GET); all steps keep the static `s_ctx`
handle. No new module partition or shared/Bazel file was edited.

### Residual gaps (still PARTIAL)

- GET/backup: no real `exporter.doExport`/`fs.writeFile`/`readBackup`; `version`
  in `meta` is a fixed stand-in; `exported_on` is wall-clock epoch ms; generated
  filename approximates `security.string.safe` (simple char sanitization).
- POST: JSON `importfile` instead of multipart `req.file`; no multer
  `upload.validation({type:'db'})`, no `storage:imports`, no `ContentImportJob`
  dispatch/notification email; timezone conversion is UTC-only.
- DELETE: no Knex transaction, `context.internal`, `promisePool(...,100)` or
  model-event side effects; no `InternalServerError` branch (always 204).
- Inline: no `ExternalMediaInlinerJob` dispatch through a real outbox, no HTTP
  fetch/rewrite/storage of external media.
- Auth: shared in-memory `GhostAuthorizer` stands in for Ghost's DB-backed
  `permissions.canThis`/`authAdminApi` chain; `permissions:true` is metadata,
  the method override is honored via the folder-local router middleware.

### New-partition needs (recorded, not created)

These need the main agent to wire shared BUILD/base files; none were attempted
here to avoid the Bazel cycle/shared-file rule:

1. `include/blog/handlers/mapping/{base,clean,wrap}.cppm` already exist on disk
   but are absent from the `blog_handlers` BUILD partitions list and
   `base.cppm` exports. Wiring `blog_handlers:mapping` would replace the
   folder-local envelope helper with `blog::mapping::wrap`.
2. `//include/core/outbox:core_outbox` is not a `blog_handlers` dep. Adding it
   plus a folder-local adapter partition would record `ContentImportJob` /
   `ExternalMediaInlinerJob` as real outbox events instead of `db` rows.
3. Multipart upload parsing (`upload.single` + `upload.validation({type:'db'})`)
   has no shared primitive — needs a new middleware partition/target.
4. Filesystem backup read/write (`backup.js`) needs a storage-adapter dep and a
   new partition; the media network fetch/rewrite needs an HTTP adapter dep and
   a new partition.
5. DB-backed permission/role lookup needs a shared authorizer data source; the
   current folder uses the default `GhostAuthorizer` roles.

### Files changed (this pass)

`include/blog/handlers/system/db/routes.cppm`; the five `handler.cppm` files;
the five `workflow/step/execute.cppm` files. Flow files and BUILD unchanged.
