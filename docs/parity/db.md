# Parity: db

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/db.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (DB block lines 389–401)
- C++: `include/blog/handlers/system/db/routes.cppm` (registered via `include/blog/handlers/routes.cppm:12,85`; handlers in `include/blog/handlers/all.cppm:167–171,513–517,762–766`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 5 |
| MISSING | 0 |
| EXTRA | 0 |

All five Ghost `/db*` routes exist in C++ at the same path + verb, but every C++ handler implements generic CRUD on a fixture resource `"db"` (`create`/`json_for`/`bulk_destroy`) rather than Ghost's real export/import/backup/inline/delete-all operations. Operation, DB call, input, output envelope and cache all diverge → MISMATCH across the board.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/db` | `exportContent` | GET `/db` (`routes.cppm:21`) | `db_browse()` (`routes.cppm:26`) | Ghost `options[]` include, filename + validation (db.js:45–52); C++ `Input=IRequest&`, no query parse (execute.cppm:11) | Ghost file disposition + export stream (db.js:53–59,64,74); C++ `{"db":[...],"meta":{"pagination":{limit:15,page:1,pages:1,total:0}}}` hardcoded (execute.cppm:17) | Ghost `dbBackup.readBackup` / `exporter.doExport` (db.js:64,74); C++ `json_for("db",{})` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:58); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:390; db.js:60); C++ `token()` 401 (handler.cppm:18–21) + content_type (routes.cppm:20) | MISMATCH | Ghost routes.js:390, db.js:44–79; C++ routes.cppm:19–28, browse/handler.cppm:17–27, browse/.../execute.cppm:13–17 |
| POST `/db` | `importContent` | POST `/db` (`routes.cppm:29`) | `db_create()` (`routes.cppm:34`) | Ghost multipart `importfile` upload + validation (routes.js:394–395); C++ raw `body_as_string()` (execute.cppm:16) | Ghost conditional `statusCode` 200/202, no envelope (db.js:82–88); C++ `create()` body, 200 (handler.cppm:27) | Ghost `importer.importFromFile` (db.js:104); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:true` (db.js:90); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:393; db.js:92); C++ `token()` 401 (handler.cppm:18–21) | MISMATCH | Ghost routes.js:391–397, db.js:81–111; C++ routes.cppm:29–36, create/handler.cppm:17–27, create/.../execute.cppm:13–16 |
| DELETE `/db` | `deleteAllContent` | DELETE `/db` (`.delt()`, routes.cppm:37) | `db_bulk_destroy()` (`routes.cppm:42`) | Ghost none (no options/data, db.js:142); C++ `Input=IRequest&`, body unused except default `{}` (execute.cppm:11,16) | Ghost `statusCode:204`, no body (db.js:140); C++ 200 + `{"meta":{"deleted":N}}` (handler.cppm:27; context.cppm:1359) | Ghost `dbBackup.backup()` + `models.Post/Tag.findAll/destroy` in transaction (db.js:143,153–184); C++ `mutations.bulk_destroy("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:true` (db.js:138); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:398; db.js:141); C++ `token()` 401 (handler.cppm:18–21) | MISMATCH | Ghost routes.js:398, db.js:136–186; C++ routes.cppm:37–44, bulk_destroy/handler.cppm:17–27, bulk_destroy/.../execute.cppm:13–16 |
| POST `/db/backup` | `backupContent` | POST `/db/backup` (routes.cppm:45–46) | `db_db_backup_post()` (`routes.cppm:52`) | Ghost `options[]` include, filename + validation (db.js:21–28); C++ raw `body_as_string()` (execute.cppm:16) | Ghost backup result, no explicit status/envelope (db.js:40); C++ `create()` body, 200 (handler.cppm:27) | Ghost `dbBackup.backup(frame.options)` (db.js:40); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:18); C++ none | Ghost `authAdminApi`+`permissions:true` (routes.js:399; db.js:20); C++ `token()` 401 (handler.cppm:18–21) | MISMATCH | Ghost routes.js:399, db.js:16–42; C++ routes.cppm:45–55, db_backup_post/handler.cppm:17–27, db_backup_post/.../execute.cppm:13–16 |
| POST `/db/media/inline` | `inlineMedia` | POST `/db/media/inline` (routes.cppm:56–59) | `db_db_media_inline_post()` (`routes.cppm:65`) | Ghost `frame.data.domains` + validation include['domains'] (db.js:120–128); C++ raw `body_as_string()` (execute.cppm:16) | Ghost `{"status":"success"}` (db.js:130–132); C++ `create()` body, 200 (handler.cppm:27) | Ghost `mediaInliner.service.startInlining(frame.data.domains)` (db.js:128); C++ `mutations.create("db",body)` (execute.cppm:16) | Ghost `cacheInvalidate:false` (db.js:115); C++ none | Ghost `authAdminApi`+`permissions.method:'importContent'` (routes.js:401; db.js:117–119); C++ `token()` 401 (handler.cppm:18–21) | MISMATCH | Ghost routes.js:401, db.js:113–134; C++ routes.cppm:56–69, db_media_inline_post/handler.cppm:17–27, db_media_inline_post/.../execute.cppm:13–16 |

## Route details

### GET /db — MISMATCH

- Operation: Ghost `exportContent` exports the DB as a file (db.js:44–79); C++ `db_browse` lists a fixture resource (browse/.../execute.cppm:16).
- Input: Ghost `options=['include','filename']` with `include` restricted to `exporter.BACKUP_TABLES` (db.js:45–52,24–26); C++ `Input=interfaces::io::IRequest&` with no query/option parsing (browse/.../execute.cppm:11).
- Output: Ghost sets `headers.disposition` file download + `cacheInvalidate:false` and returns export stream/backup (db.js:53–59,64,74); C++ returns hardcoded `{"db":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with status 200 (browse/.../execute.cppm:17; browse/handler.cppm:27).
- DB call: Ghost `dbBackup.readBackup(filename)` or `exporter.doExport({include:withRelated})` (db.js:64,74); C++ `ctx.store().json_for("db",{})` — fixture only, no filtering/pagination (browse/.../execute.cppm:16; context.cppm:1170–1179).
- Cache: Ghost `cacheInvalidate:false` (db.js:58); C++ none (cache is only wired for members/emails/automations in all.cppm:836–839).
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (routes.js:390; db.js:60); C++ bearer `token()` non-empty check → 401 `{"error":"Unauthorized"}` (browse/handler.cppm:18–21) plus `content_type_middleware` (routes.cppm:20). No permission/docName model.

### POST /db — MISMATCH

- Operation: Ghost `importContent` imports a DB from an uploaded file (db.js:81–111); C++ `db_create` performs a generic create on resource `"db"` (create/.../execute.cppm:16).
- Input: Ghost multipart single file field `importfile` + `upload.validation({type:'db'})` (routes.js:394–395), derives timezone and owner email (db.js:94–101); C++ passes `in.body_as_string()` as JSON (create/.../execute.cppm:16).
- Output: Ghost conditional `statusCode(result)` returns 200 when `result.db||result.problems`, else 202 (db.js:82–88); C++ always returns 200 with the created body (create/handler.cppm:27). No `{db,problems}` shape.
- DB call: Ghost `importer.getInstance().importFromFile(frame.file,...)` (db.js:104); C++ `mutations.create("db",body)` (create/.../execute.cppm:16).
- Cache: Ghost `headers.cacheInvalidate:true` (db.js:90); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:393; db.js:92); C++ `token()` 401 check (create/handler.cppm:18–21).

### DELETE /db — MISMATCH

- Verb wiring: Ghost `router.delete` (routes.js:398); C++ `.delt()` with summary "Ghost db DELETE" (routes.cppm:37–44) — verb matches.
- Operation: Ghost `deleteAllContent` backs up then deletes all Posts and Tags in a transaction with `columns:id`/`context.internal`/`destroyAll` and a 100-way promise pool (db.js:136–186); C++ `db_bulk_destroy` calls generic `mutations.bulk_destroy("db",body)` (bulk_destroy/.../execute.cppm:16).
- Output: Ghost `statusCode:204`, no body (db.js:140,142); C++ returns 200 + `{"meta":{"deleted":N}}` (bulk_destroy/handler.cppm:27; context.cppm:1359).
- DB call: Ghost `dbBackup.backup()`, `models.Post.findAll/destroy`, `models.Tag.findAll/destroy` (db.js:143,162–178); C++ resource string `"db"` only, no real post/tag deletion (bulk_destroy/.../execute.cppm:16; context.cppm:1356–1359).
- Cache: Ghost `cacheInvalidate:true` (db.js:138); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:398; db.js:141); C++ `token()` 401 check (bulk_destroy/handler.cppm:18–21).

### POST /db/backup — MISMATCH

- Operation: Ghost `backupContent` creates a backup via `dbBackup.backup(frame.options)` (db.js:16–42); C++ `db_db_backup_post` does a generic create on resource `"db"` (db_backup_post/.../execute.cppm:16) — no backup generated.
- Input: Ghost `options=['include','filename']`, `include` validated against `exporter.BACKUP_TABLES`, plus path-separator validation on `filename` (db.js:21–38); C++ raw `body_as_string()` (db_backup_post/.../execute.cppm:16).
- Output: Ghost returns backup result with `cacheInvalidate:false` (db.js:17–19,40); C++ returns `create()` body with 200 (db_backup_post/handler.cppm:27).
- DB call: Ghost `dbBackup.backup(frame.options)` (db.js:40); C++ `mutations.create("db",body)` (db_backup_post/.../execute.cppm:16).
- Cache: Ghost `cacheInvalidate:false` (db.js:18); C++ none.
- Auth: Ghost `authAdminApi` + `permissions:true` (routes.js:399; db.js:20); C++ `token()` 401 check (db_backup_post/handler.cppm:18–21).

### POST /db/media/inline — MISMATCH

- Operation: Ghost `inlineMedia` starts the media inliner on `frame.data.domains` (db.js:113–134); C++ `db_db_media_inline_post` does a generic create on resource `"db"` — no inlining (db_media_inline_post/.../execute.cppm:16).
- Input: Ghost `frame.data.domains` with validation `include` values `['domains']` (db.js:120–128); C++ raw `body_as_string()` (db_media_inline_post/.../execute.cppm:16).
- Output: Ghost returns `{"status":"success"}` (db.js:130–132); C++ returns `create()` body with 200 (db_media_inline_post/handler.cppm:27).
- DB call: Ghost `mediaInliner.service.startInlining(frame.data.domains)` (db.js:128); C++ `mutations.create("db",body)` (db_media_inline_post/.../execute.cppm:16).
- Cache: Ghost `cacheInvalidate:false` (db.js:115); C++ none.
- Auth: Ghost `authAdminApi` + `permissions.method:'importContent'` (routes.js:401; db.js:117–119); C++ `token()` 401 check (db_media_inline_post/handler.cppm:18–21).

## Missing in C++

None. All five Ghost `/db*` routes have a C++ route at the same path + verb.

Absence check: grep for `api.db.` / `db.` in `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` found exactly the five DB routes (lines 390–401); grep `"/db` across `include/blog/handlers/**/routes.cppm` found only `include/blog/handlers/system/db/routes.cppm:19`, whose five verbs cover all five Ghost routes.

## Extra in C++

None. The C++ `DbRoutes` folder (`include/blog/handlers/system/db/routes.cppm`) installs exactly five routes — GET/POST/DELETE `/db`, POST `/db/backup`, POST `/db/media/inline` — all five corresponding to Ghost controller operations. No additional db routes exist in that folder (verified by reading routes.cppm:19–69).
