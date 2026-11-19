# Parity: integrations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/integrations.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 80–91)
- C++: `include/blog/handlers/external/integrations/routes.cppm` (installed at `include/blog/handlers/routes.cppm:24,97`; handlers registered at `include/blog/handlers/all.cppm:51-56,397-402,646-651`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 5 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

All six Ghost `integrations` routes have a 1:1 C++ route by normalized path + verb. The refresh
route is a `MISMATCH`: Ghost binds it to the `edit` operation (`api.integrations.edit`), while the
dedicated C++ handler calls `mutations.create`. The other five are `PARTIAL`: path, verb, operation
and the `"integrations"` DB resource agree, but C++ parses none of the Ghost options/data/validation,
emits a raw non-`{data,meta}` envelope with wrong status codes (200 vs Ghost 201/204), performs no
real filtering/pagination, and uses a weaker token-presence auth check. No cache behavior on either
side (`cacheInvalidate:false` on Ghost; none in C++).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/integrations` | browse | GET `/integrations` | `integrations_browse()` → `blog::handlers::integrations::browse::BrowseHandler` | Ghost: `options:['include','limit']`, `include` values `['api_keys','webhooks']`; C++: none parsed, empty params | Ghost: `{data,meta}` w/ real pagination, 200; C++: encoded `IntegrationsResponse` → single dummy `{"integrations":[{"id":"1","name":"Ghost Record","slug":"ghost-record"}]}`, no `meta`, status 200 | Ghost: `models.Integration.findPage(options)`; C++: `ctx.store().json_for("integrations", {})` (unreachable — lambda short-circuits) | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | PARTIAL | Ghost `admin/routes.js:82`, `integrations.js:13-29`; C++ `routes.cppm:19-28`, `all.cppm:397,646`, `browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:14-24` |
| GET `/integrations/:id` | read | GET `/integrations/:id` | `integrations_read_by_id()` → `blog::handlers::integrations::read_by_id::ReadByIdHandler` | Ghost: `data:['id']` required, `options:['include']` values `['api_keys','webhooks']`; C++: none parsed, `:id` ignored | Ghost: Integration model `{data}`, 200 / 404 on `NotFoundError`; C++: `{"integrations":[<one>]}`, no `meta`, always 200 | Ghost: `models.Integration.findOne(data,{require:true})`; C++: `ctx.store().find_by_resource("integrations","1")` (hardcoded "1") | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | PARTIAL | Ghost `admin/routes.js:83`, `integrations.js:30-60`; C++ `routes.cppm:38-45`, `all.cppm:398,647`, `read_by_id/handler.cppm:17-28`, `read_by_id/workflow/step/execute.cppm:13-19` |
| POST `/integrations` | add | POST `/integrations` | `integrations_create()` → `blog::handlers::integrations::create::CreateHandler` | Ghost: `data:['name','icon_image','description','webhooks']`, `options:['include']`, `name` required, injects `api_keys:[{content},{admin}]`; C++: raw `body_as_string()` no validation/injection | Ghost: Integration `{data}`, **201**; C++: raw `mutations.create(...).body()`, **200** | Ghost: `models.Integration.add(dataWithApiKeys, options)`; C++: `mutations.create("integrations", body)` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | PARTIAL | Ghost `admin/routes.js:84`, `integrations.js:82-111`; C++ `routes.cppm:29-36`, `all.cppm:399,648`, `create/handler.cppm:17-28`, `create/workflow/step/execute.cppm:13-17` |
| POST `/integrations/:id/api_key/:keyid/refresh` | edit | POST `/integrations/:id/api_key/:id/refresh` | `integrations_integrations_id_api_key_keyid_refresh_post()` → `...::IntegrationsIdApiKeyKeyidRefreshPostHandler` | Ghost: path `id`+`keyid`, `options:['id','keyid','include']`, `data` fields, `id` required; C++: raw `body_as_string()`, no `keyid`/`id` parsing | Ghost: Integration via `integrations.service.edit`, 200; C++: raw `mutations.create(...).body()`, 200 | Ghost: `integrations.service.edit(data, options)` (**edit**); C++: `mutations.create("integrations", body)` (**create**) | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | MISMATCH | Ghost `admin/routes.js:85-89`, `integrations.js:61-81`; C++ `routes.cppm:63-79`, `all.cppm:400,649`, `integrations_id_api_key_keyid_refresh_post/handler.cppm:17-28`, `.../workflow/step/execute.cppm:13-17` |
| PUT `/integrations/:id` | edit | PUT `/integrations/:id` | `integrations_edit()` → `blog::handlers::integrations::edit::EditHandler` | Ghost: `options:['id','keyid','include']`, `data:['name','icon_image','description','webhooks']`, `id` required; C++: raw `body_as_string()`, no `:id`/option parsing | Ghost: Integration via `integrations.service.edit`, 200; C++: raw `mutations.edit("integrations","",body).body()`, 200 | Ghost: `integrations.service.edit(data, options)`; C++: `mutations.edit("integrations", "", body)` (empty id) | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | PARTIAL | Ghost `admin/routes.js:90`, `integrations.js:61-81`; C++ `routes.cppm:47-54`, `all.cppm:401,650`, `edit/handler.cppm:17-28`, `edit/workflow/step/execute.cppm:13-17` |
| DELETE `/integrations/:id` | destroy | DELETE `/integrations/:id` (`.delt`) | `integrations_destroy()` → `blog::handlers::integrations::destroy::DestroyHandler` | Ghost: `options:['id']` required; C++: none parsed, `:id` not passed to destroy | Ghost: empty body, **204** / 404 on `NotFoundError`; C++: raw `mutations.destroy(...).body()`, **200** | Ghost: `models.Integration.destroy({require:true})`; C++: `mutations.destroy("integrations", "")` (empty id) | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token check (401) | PARTIAL | Ghost `admin/routes.js:91`, `integrations.js:112-137`; C++ `routes.cppm:55-62` (`utils/openapi/route.cppm:96,370` = DELETE), `all.cppm:402,651`, `destroy/handler.cppm:17-28`, `destroy/workflow/step/execute.cppm:13-17` |

## Route details

### GET `/integrations` — PARTIAL

- **Route identity — MATCH**: Ghost `router.get('/integrations', mw.authAdminApi, http(api.integrations.browse))` at `admin/routes.js:82`; C++ `Router{router, "/integrations"}...get(...)` at `include/blog/handlers/external/integrations/routes.cppm:19-28`. Verbatim path `/integrations`, verb GET.
- **Operation mapping — MATCH**: Ghost controller key `browse` (`integrations.js:13`); C++ binds `handler.integrations_browse()` (`routes.cppm:26`), accessor `blog::handlers::integrations::browse::BrowseHandler` (`all.cppm:397`, member `:646`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['include','limit']` with `include` values `['api_keys','webhooks']` (`integrations.js:18-25`). C++ `using Input = interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:12`) never reads query params; `json_for("integrations", {})` gets empty params (`:22`). Ghost-only params: `include`, `limit`.
- **Output parity — PARTIAL (divergent)**: Ghost `models.Integration.findPage(options)` (`integrations.js:27`) returns `{data,meta}` with real pagination via the serializer, status 200. C++ step `browse/.../execute.cppm:16-23` builds a lambda returning `{200, encode(IntegrationsResponse)}` with one default `RecordDto` (`:17-19`), then `if (r.first != 0)` (`:21`) is **always true** (r.first is 200), so it returns immediately. The `json_for` + hardcoded `meta.pagination {limit:15,page:1,pages:1,prev:null,next:null,total:0}` line (`:22-23`) is unreachable. Actual C++ body: `{"integrations":[{"id":"1","name":"Ghost Record","slug":"ghost-record"}]}` (`RecordDto` defaults at `include/blog/dto/more_resources.cppm:19-21`; `ARRAY_SERDE` key `integrations` at `:122-137`). No `meta` envelope. Handler always sets `Status::OK` (`browse/handler.cppm:27`).
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.findPage(options)` (`integrations.js:27`). C++ `ctx.store().json_for("integrations", {})` (`execute.cppm:22`) — correct resource string (`include/blog/context/context.cppm:150,239`) but unreachable and, if reached, no filtering/pagination.
- **Cache parity — MATCH (none)**: Ghost `headers.cacheInvalidate: false` (`integrations.js:14-16`); no `X-Cache-Invalidate` URL. C++ has no `core::cache::Cache`, namespace, `ResourceStore` tag, TTL or eviction in handler/step (`browse/handler.cppm:17-28`, `execute.cppm:14-24`). Both sides: no cache behavior.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:82`) + `permissions: true`, `docName: 'integrations'` (`integrations.js:12,17`). C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check returning `401 {"error":"Unauthorized"}` when `blog::handlers::middleware::token(req)` is empty (`browse/handler.cppm:18-22`); no permission/docName check.

### GET `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.get('/integrations/:id', ...)` (`admin/routes.js:83`); C++ `Router{router, "/:id"}...get(...)` (`routes.cppm:38-45`). Both `/integrations/:id`, verb GET.
- **Operation mapping — MATCH (renamed)**: Ghost key `read` (`integrations.js:30`); C++ `integrations_read_by_id()` → `ReadByIdHandler` (`routes.cppm:44`, `all.cppm:398,647`). Name differs, function equivalent.
- **Input parity — PARTIAL (divergent)**: Ghost `data: ['id']` required, `options: ['include']` values `['api_keys','webhooks']` (`integrations.js:35-47`), plus `require:true` (`:51`). C++ `read_by_id/.../execute.cppm:11` declares `using Input = IRequest&` but parses nothing; the store lookup uses a hardcoded `"1"` instead of the route `:id` (`:16`). Ghost-only params: `id`, `include`, `require`.
- **Output parity — PARTIAL (divergent)**: Ghost `findOne(data,{require:true})` → Integration `{data}`, 200 / `NotFoundError`→404 (`integrations.js:49-59`). C++ `{"integrations":[<one>]}` or `{"integrations":[]}` (`execute.cppm:17-18`), no `meta` envelope, always status 200 (`read_by_id/handler.cppm:27`); no 404 path.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.findOne` (`integrations.js:51`). C++ `ctx.store().find_by_resource("integrations", std::string{}.empty()?"1":std::string{})` (`execute.cppm:16`) — same resource string, but target id is constant `"1"` and ignores the URL `:id`.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:31-33`); C++ none in handler/step.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:83`) + `permissions:true` (`integrations.js:34`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`read_by_id/handler.cppm:18-22`).

### POST `/integrations` — PARTIAL

- **Route identity — MATCH**: Ghost `router.post('/integrations', ...)` (`admin/routes.js:84`); C++ `.post(...)` on `/integrations` (`routes.cppm:29-36`).
- **Operation mapping — MATCH (renamed)**: Ghost key `add` (`integrations.js:82`); C++ `integrations_create()` → `CreateHandler` (`routes.cppm:34`, `all.cppm:399,648`).
- **Input parity — PARTIAL (divergent)**: Ghost `data: ['name','icon_image','description','webhooks']`, `options: ['include']`, `name` required (`integrations.js:88-100`), and `query()` injects `api_keys: [{type:'content'},{type:'admin'}]` when absent (`:103-108`). C++ `create/.../execute.cppm:16` passes `in.body_as_string()` verbatim (or `"{}"`), no field allowlist, no `name` validation, no `api_keys` injection.
- **Output parity — PARTIAL (divergent)**: Ghost `statusCode: 201` (`integrations.js:83`) with serialized Integration `{data}`. C++ handler always sets `Status::OK` (200) (`create/handler.cppm:27`) and returns raw `mutations.create(...).body()` — no `{data,meta}` envelope, no 201.
- **DB call parity — MATCH-ish resource / PARTIAL call**: Ghost `models.Integration.add(dataWithApiKeys, options)` (`integrations.js:109`). C++ `mutations.create("integrations", body)` (`execute.cppm:16`) — same `"integrations"` resource (`context.cppm:150`) but generic create without the injected api_keys.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:84-86`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:84`) + `permissions:true` (`integrations.js:87`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`create/handler.cppm:18-22`).

### POST `/integrations/:id/api_key/:keyid/refresh` — MISMATCH

- **Route identity — MATCH**: Ghost `router.post('/integrations/:id/api_key/:keyid/refresh', ...)` (`admin/routes.js:85-89`); C++ `.add_router(Router{"/api_key"}) → Router{"/:id"} → Route{"/refresh"}.post(...)` (`routes.cppm:63-79`) yields `/integrations/:id/api_key/:id/refresh`. Placeholder name differs (`:keyid` vs `:id`) but normalizes to the same path; verb POST.
- **Operation mapping — MISMATCH**: Ghost binds this route to `api.integrations.edit` (`admin/routes.js:88`), i.e. controller `edit` (`integrations.js:61`). C++ binds a distinct dedicated handler `integrations_id_api_key_keyid_refresh_post()` (`routes.cppm:74`, `all.cppm:400,649`) whose step calls `mutations.create` (`.../workflow/step/execute.cppm:16`), not `edit`. Core operation materially differs: edit/refresh vs create.
- **Input parity — MISMATCH**: Ghost resolves `id` + `keyid` from the path and passes `options: ['id','keyid','include']` + `data` (`integrations.js:66-67`) to `integrations.service.edit` (`:79`). C++ `.../execute.cppm:11-16` parses no params — `keyid`/`id` never read; body passed raw as a *create* payload.
- **Output parity — PARTIAL (divergent)**: Ghost serialized Integration via `service.edit`, 200. C++ raw `mutations.create(...).body()`, status 200 (`refresh/handler.cppm:27`), no `{data,meta}` envelope.
- **DB call parity — MISMATCH**: Ghost `integrations.service.edit(data, options)` (`integrations.js:79`) edits an existing integration/key. C++ `mutations.create("integrations", body)` (`refresh/.../execute.cppm:16`) creates a new record.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:63-65`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:87`) + `permissions:true` (`integrations.js:65`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`refresh/handler.cppm:18-22`).

### PUT `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.put('/integrations/:id', ...)` (`admin/routes.js:90`); C++ `Router{"/:id"}...put(...)` (`routes.cppm:47-54`). Both `/integrations/:id`, verb PUT.
- **Operation mapping — MATCH**: Ghost key `edit` (`integrations.js:61`); C++ `integrations_edit()` → `EditHandler` (`routes.cppm:52`, `all.cppm:401,650`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['id','keyid','include']`, `data: ['name','icon_image','description','webhooks']`, `id` required (`integrations.js:66-77`). C++ `edit/.../execute.cppm:11-16` parses nothing; calls `mutations.edit("integrations", std::string{}, body)` with an empty id string. Ghost-only params: `id`, `keyid`, `include`, plus the named data fields.
- **Output parity — PARTIAL (divergent)**: Ghost Integration via `integrations.service.edit` (`integrations.js:79`), 200, `{data,meta}`. C++ raw `mutations.edit(...).body()`, status 200 (`edit/handler.cppm:27`), no envelope.
- **DB call parity — PARTIAL (divergent)**: Ghost `integrations.service.edit(data, options)` (`integrations.js:79`). C++ `mutations.edit("integrations", "", body)` (`edit/execute.cppm:16`) — same resource, but empty id means no real target record.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:62-64`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:90`) + `permissions:true` (`integrations.js:65`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`edit/handler.cppm:18-22`).

### DELETE `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.delete('/integrations/:id', ...)` (`admin/routes.js:91`); C++ `Router{"/:id"}...delt(...)` (`routes.cppm:55-62`). `.delt` registers the DELETE handler (`include/utils/openapi/route.cppm:96,370`). Both `/integrations/:id`, verb DELETE.
- **Operation mapping — MATCH**: Ghost key `destroy` (`integrations.js:112`); C++ `integrations_destroy()` → `DestroyHandler` (`routes.cppm:60`, `all.cppm:402,651`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['id']` required (`integrations.js:118-124`) and `require:true` (`:128`). C++ `destroy/.../execute.cppm:11-16` parses nothing; `mutations.destroy("integrations", "")` receives an empty id. Ghost-only params: `id`, `require`.
- **Output parity — PARTIAL (divergent)**: Ghost `statusCode: 204`, empty body (`integrations.js:113`) / 404 on `NotFoundError` (`:130-134`). C++ raw `mutations.destroy(...).body()` with status 200 (`destroy/handler.cppm:27`) — wrong status and non-empty body.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.destroy({require:true})` (`integrations.js:128`). C++ `mutations.destroy("integrations", "")` (`execute.cppm:16`) — same resource, no target id, no require semantics.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:114-116`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:91`) + `permissions:true` (`integrations.js:117`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`destroy/handler.cppm:18-22`).

## Missing in C++

None. Every Ghost `integrations` route in `admin/routes.js:82-91` has a C++ route in
`include/blog/handlers/external/integrations/routes.cppm` by normalized path + verb:

1. GET `/integrations` — `admin/routes.js:82` → browse — `routes.cppm:19-28`
2. GET `/integrations/:id` — `admin/routes.js:83` → read — `routes.cppm:38-45`
3. POST `/integrations` — `admin/routes.js:84` → add — `routes.cppm:29-36`
4. POST `/integrations/:id/api_key/:keyid/refresh` — `admin/routes.js:85-89` → edit — `routes.cppm:63-79`
5. PUT `/integrations/:id` — `admin/routes.js:90` → edit — `routes.cppm:47-54`
6. DELETE `/integrations/:id` — `admin/routes.js:91` → destroy — `routes.cppm:55-62`

## Extra in C++

None. The matched folder `include/blog/handlers/external/integrations/` installs exactly six routes
(`routes.cppm:19-79`) matching the six Ghost controller routes one-to-one. Each C++ handler is
registered in `all.cppm:397-402,646-651` and no additional `/integrations*` route exists in that
folder's `routes.cppm`.
