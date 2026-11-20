# Behavior report: integrations

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/integrations.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/integrations/` (and shared `include/blog/handlers/context/context_context.cppm`, `include/blog/handlers/middleware/`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 6 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

6 Ghost route bindings, 6 C++ route bindings. All 6 matched by exact path + verb.
Strict-parity group 26 (2026-11-20) rewrote all six steps and handlers against the
shared abstractions (`core::query`, `core_auth::ghost` via
`require_ghost_permission`, `core_outbox` + an adapter, local Ghost-envelope helper). The former
MISMATCH is resolved: the refresh route now rotates the API-key secret by `keyid`
through an outbox adapter and returns the integration with both relations. Every
route stays PARTIAL only because real JWT/session verification and the hourly
`last_seen` write live in shared auth middleware outside this folder.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior (group 26) | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/integrations` | `browse` | `options[include,limit]`; query `filter` defaulted to `type:[custom,builtin,core]` | `mw.authAdminApi` (admin JWT or session), `permissions:true`, tokenPermissionCheck (integration tokens blocked) | 200; `headers.cacheInvalidate:false` → no `X-Cache-Invalidate` | `Integration.findPage(options)` → Bookshelf collection + default relations | none in controller; model relation load | mapper adds `id:secret` prefix on admin keys; `{integrations:[],meta.pagination}` | `browse/handler.cppm`, `browse/workflow/**`, `routes.cppm:21-28` | `core::query::parse` + injected default type filter; real `run()` pagination/meta; api_keys relation embedded; admin secret prefixed; permission gate via `require_ghost_permission` | PARTIAL | Ghost `integrations.js:13-29`; C++ `browse/step/execute.cppm`, `types/dto_more_resources.cppm` |
| GET `/integrations/:id` | `read` | `data[id]` path param; `options[include]`; filter defaulted | same as above | 200; `cacheInvalidate:false` | `Integration.findOne(data,{...options,require:true})`; NotFoundError → "Integration not found." | none | mapper + single integration envelope | `read_by_id/handler.cppm`, `read_by_id/workflow/**`, `routes.cppm:39-46` | real `:id` lookup; default type filter enforced; 404 "Integration not found."; relations embedded; admin secret prefixed; permission gate | PARTIAL | Ghost `integrations.js:30-60`; C++ `read_by_id/step/execute.cppm` |
| POST `/integrations` | `add` | body `data.integrations[0]` picked to `[name,icon_image,description,webhooks]`; `options[include]`; `name` required | same | 201; `cacheInvalidate:false` | injects `api_keys:[content,admin]`, `Integration.add` (slug gen, api_key/webhook rows, events, limits) | none | full integration + api_keys, admin secret prefixed | `create/handler.cppm`, `create/workflow/**`, `routes.cppm:29-36` | allowlisted fields + name required (422); slug gen; separate `api_keys` (content 26-hex/admin 64-hex) + `webhooks` rows; `integration.added` outbox event; full 201 body with prefixed secret | PARTIAL | Ghost `integrations.js:82-111`; C++ `create/step/execute.cppm` |
| POST `/integrations/:id/api_key/:keyid/refresh` | `edit` (refresh branch) | path `id`, `keyid`; `options[keyid,id,include]` | same | 200; `cacheInvalidate:false` | `ApiKeyModel.findOne(keyid)` → `refreshSecret` → `Integration.findOne(withRelated:[api_keys,webhooks])`; 404 ApiKey / 500 | none | updated integration with relations | `integrations_id_api_key_keyid_refresh_post/handler.cppm`; route now dispatches it (`routes.cppm:93`) | both path ids parsed; `ApiKey` lookup → 404 "ApiKey not found."; secret rotation performed by a registered outbox adapter; reload with `api_keys`+`webhooks`; 500 on adapter failure; secret prefixed | PARTIAL | Ghost `integrations-service.js:14-42`; C++ `integrations_id_api_key_keyid_refresh_post/step/execute.cppm`, `routes.cppm` |
| PUT `/integrations/:id` | `edit` | body `data.integrations[0]` picked to `[name,icon_image,description,webhooks]`; `options[id,keyid,include]`, `id` required | same | 200; `cacheInvalidate:false` | `integrations.service.edit` → `IntegrationModel.edit(require:true)` or refresh branch; 404/500 mapping | none | edited integration; secret prefix | `edit/handler.cppm`, `edit/workflow/**`, `routes.cppm:47-54` | real `:id`; allowlisted fields + slug regen; webhook relation replaced; `integration.edited` outbox event; full 200 body with prefixed secret; 404 "Integration not found." | PARTIAL | Ghost `integrations.js:61-81`; C++ `edit/step/execute.cppm` |
| DELETE `/integrations/:id` | `destroy` | `options[id]` required | same | 204; `cacheInvalidate:false` | `Integration.destroy(require:true)`, NotFoundError → "Integration not found." | none | empty body | `destroy/handler.cppm`, `destroy/workflow/**`, `routes.cppm:55-62` | real `:id`; 404 "Integration not found."; cascades `api_keys`/`webhooks` rows; `integration.deleted` outbox event; 204 empty body | PARTIAL | Ghost `integrations.js:112-137`; C++ `destroy/step/execute.cppm` |

## Route details

### GET /integrations — PARTIAL

**Ghost** (`integrations.js:13-29`; `admin/routes.js:82`; router mounts `router.use(apiMw.cors)` at `routes.js:17`):
1. Identity: GET `/integrations`, middleware `mw.authAdminApi` then `http(api.integrations.browse)` (`routes.js:82`). `authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`).
2. Operation: key `browse`, docName `integrations` (`integrations.js:12,13`).
3. Input: `options:['include','limit']` (`integrations.js:18`); `include` allowed values `api_keys,webhooks` (`integrations.js:20-24`). Input serializer injects default `filter='type:[custom,builtin,core]'`, or appends it to a caller filter (`input/integrations.js:4-10,13-17`). No body, no path params.
4. Auth/permissions: `permissions:true` (`integrations.js:17`). `authenticateAdminApi` = admin JWT (`Authorization: Ghost <jwt>`, `api-key/admin.js:48-67,112-221`) or user session (`auth/authenticate.ts:14`); `authorizeAdminApi` requires `req.user` or `req.api_key` (`auth/authorize.js:30-44`). `tokenPermissionCheck`: integration tokens (no `user_id`) are checked against an allowlist that does **not** contain `integrations` → 403 `apiTokenBlocked` (`admin/middleware.js:54-99`, allowlist `:55-83`). `updateUserLastSeen` writes user `last_seen` at most once/hour (`update-user-last-seen.js:6-13`). Admin JWT auth verifies via `ApiKey.findOne` + `jwt.verify`, limits custom integrations (`api-key/admin.js:138-163,194-220`).
5. Status/headers: 200 default; `headers.cacheInvalidate:false` (`integrations.js:14-16`) → no `X-Cache-Invalidate`. No custom `Content-Disposition`/`Location`/`Cache-Control`, no `stream`.
6. Call chain: `query({options})` → `models.Integration.findPage(options)` (`integrations.js:26-28`). `findPage` loads relations via `defaultRelations` (adds `api_keys`; `models/integration.js:88-94`) and collection of `integrations` rows (`models/integration.js:9,14`).
7. Cache: none in controller; `cacheInvalidate:false`.
8. Output: mapper `output/mappers/integrations.ts:3-22`: `model.toJSON(frame.options)`, and for each `api_keys` entry with `type==='admin'` rewrites `secret = id + ':' + secret` (`:14-19`). Envelope `{integrations:[...],meta.pagination:{page,limit,pages,total,prev,next}}` from `findPage`. Empty DB → empty array with zeroed pagination meta.

**C++**:
1. `routes.cppm:19-28`: `Router{router,"/integrations"}.use(content_type_middleware).get(...) → integrations_browse().handle`. No auth middleware at router level.
2. `browse/handler.cppm:18-29`: rejects when `middleware::token(req).empty()` with 401 `{"error":"Unauthorized"}` (`:19-23`); runs workflow `integrations_browse` (`:24`); writes 200 + body string (`:25-28`). `token()` reads `get_authorization()`, stripping only `Bearer ` (`middleware/bearer.cppm:27-33`) — no JWT/signature verification.
3. `browse/workflow/flow/main.cppm:7-10`: single step `ExecuteStep`.
4. `browse/step/execute.cppm:14-42`: parses query keys `include,limit,page` from `get_path()` (`:18`, `:51-63`); whitelists `include` against `api_keys,webhooks` (`:15,19-26`); `limit=size_option(...,15)`, `page=...,1` (`:27-28`); derives `offset=(page-1)*limit` (`:29`); calls `s_ctx->store().json_for("integrations",params)` (`:30`) and `total("integrations")` (`:31`); computes `pages/prev/next` (`:32-34`) and builds `{integrations:<rows>,meta.pagination:{limit,page,pages,prev,next,total}}` (`:35-41`).
5. Store: `json_for` generic fallthrough `parse_array(m_resources.list("integrations"))` → `search_json` (`context_context.cppm:1209-1210`); `total` counts stored rows (`:1140`). No `type` filter applied — diverges from Ghost's default `type:[custom,builtin,core]`.
6. Middleware: `content_type_middleware` requires `Content-Type` starting JSON **and** `Accept` JSON/`*/*` for every verb (`middleware/serde.cppm:90-126`). Ghost has no such requirement on GET.
7. Cache: no `core::cache::Cache` key/tag in this handler; only in-memory resource store reads.
8. Output: no output mapper → admin key secrets are NOT prefixed `id:`. Rows are whatever `json_for` emits (flat stored maps), not Ghost's full integration shape.

**Diff**: residual gaps — Ghost permission service, JWT/session auth + tokenPermissionCheck allowlist, `last_seen` write, default type filter, output secret prefix, real aggregated pagination. Divergences — fabricated pagination from in-memory count, mandatory JSON content-type/accept on GET, auth = any non-empty Authorization header.

### GET /integrations/:id — PARTIAL

**Ghost** (`integrations.js:30-60`; `admin/routes.js:83`):
1. Identity: GET `/integrations/:id`, same `authAdminApi` chain (`routes.js:83`, `admin/middleware.js:110-118`).
2. Operation: `read`, docName `integrations` (`integrations.js:30,12`).
3. Input: `data:['id']` from path; `options:['include']` with values `api_keys,webhooks` (`integrations.js:35-47`); `data.id` required (`:38-42`); input serializer applies default type filter (`input/integrations.js:18-22`).
4. Auth: `permissions:true`, same as browse.
5. Status/headers: 200; `cacheInvalidate:false` (`integrations.js:31-33`).
6. Call chain: `findOne(data, Object.assign(options,{require:true}))` (`integrations.js:51`); on `models.Integration.NotFoundError` throws `errors.NotFoundError` "Integration not found." (`:52-57`). `require:true` makes 404 the normal missing case.
7. Cache: none.
8. Output: mapper secret prefix (`output/mappers/integrations.ts:14-19`); single-integration envelope.

**C++**:
1. `routes.cppm:38-46`: nested `Router{router,"/:id"}.get(...) → integrations_read_by_id().handle`.
2. `read_by_id/handler.cppm:18-35`: same token check 401 (`:19-23`); status map 200/404/422 (`:32-35`).
3. `read_by_id/workflow/flow/main.cppm:7-10`: single `ExecuteStep`.
4. `read_by_id/step/execute.cppm:13-25`: validates `include` tokens against `api_keys,webhooks` (`:14,18-20`, `:65-68` → 422 `"Invalid include value"`); `tail_id(path)` extracts last path segment (`:21`, `:30-35`); empty → `not_found()`; `s_ctx->store().resources().get("integrations",id)` (`:23`); miss → 404 `ApiError{}.json("Resource not found","Not Found")` (`:22,24,36-39`); hit → `{"integrations":[<record>]}` (`:25`).
5. Middleware token check as above; `content_type_middleware` from router.
6. Cache: none.
7. Output: raw stored record, no mapper/secret prefix; no type filter means a `builtin`/`core` integration can be read even though Ghost's default filter excludes them.

**Diff**: residual gaps — permission service, real model `require:true` semantics, output secret prefix, default filter. Divergences — different 404 message/context ("Resource not found"/"Not Found" vs "Integration not found."), include validation returns 422 where Ghost's `values` validation also errors but with Ghost's standard error envelope, auth as bare token.

### POST /integrations — PARTIAL

**Ghost** (`integrations.js:82-111`; `admin/routes.js:84`):
1. Identity: POST `/integrations`, `authAdminApi` (`routes.js:84`).
2. Operation: `add`, docName `integrations` (`integrations.js:82,12`).
3. Input: body `data.integrations[0]` picked to `[name,icon_image,description,webhooks]` (`integrations.js:88`, `input/integrations.js:23-27`); `options:['include']`; `data.name` required (`:91-95`); include values `api_keys,webhooks` (`:96-100`). No multipart fields.
4. Auth: `permissions:true`.
5. Status/headers: `statusCode:201` (`integrations.js:83`); `cacheInvalidate:false`.
6. Call chain: builds `dataWithApiKeys = Object.assign({api_keys:[{type:'content'},{type:'admin'}]}, data)` (`integrations.js:103-109`) → `models.Integration.add(dataWithApiKeys,options)`. Model: `defaults type:'custom'` (`models/integration.js:29-33`), `onSaving` slug generation (`:40-53`), `onCreated` emits `integration.added` (`:35-38,55-61`), relations `api_keys`/`webhooks` editable + `defaultRelations` adds `api_keys` (`:14-27,88-94`), `permissible` enforces both user & API-key permission and `customIntegrations` limit (`:96-116`).
7. Cache: none.
8. Output: 201 created integration including generated api_keys; admin key `secret` prefixed `id:` by mapper (`output/mappers/integrations.ts:14-19`).

**C++**:
1. `routes.cppm:29-36`: POST on `/integrations` → `integrations_create().handle`.
2. `create/handler.cppm:18-36`: token check 401 (`:19-23`); maps payload status `"201"→CREATED`, `"422"→UNPROCESSABLE_CONTENT`, `"404"→NOT_FOUND` (`:29-35`).
3. `create/workflow/flow/main.cppm:7-10`: single `ExecuteStep`.
4. `create/step/execute.cppm:14-35`: `body_as_string()` → decode to map (`:15-17`); `name` from fields or `ApiDataStore::first_name(payload)` (`:18-20`, helper `context_context.cppm:1217-1224`); missing name → 422 `validation("Validation error, cannot save integration.","name is required","name")` (`:21-24`); if body lacks `"api_keys"` injects the literal `"api_keys":[{"type":"content"},{"type":"admin"}]` before the final `}` (`:25-33`); `store().create("integrations",payload)` (`:34`) → status/body (`:35`).
5. Store: generic `create` assigns id `++m_next_id`, `core::cache::Cache::put(record_key("integrations",id), ..., Ttl{}.never(), {"resources"})`, appends id to id-list (`context_context.cppm:656-667,1301-1305`); returns 201 `{"integrations":[{"id":"<n>"}]}`.
6. Middleware: content-type/accept required on POST too (`serde.cppm:90-126`).
7. Cache: real `core::cache::Cache` write with `resources` tag, TTL never — but it is a fixture/store marker, not Ghost's response-side cache behavior.
8. Output: id-only envelope; no `name`, `type`, `api_keys`, or secret prefix.

**Diff**: residual gaps — permission service, model transactions, slug generation, `integration.added` event, `customIntegrations` limit, full relation rows, secret prefix. Divergences — api_keys injected by naive string surgery into raw JSON (not model-level), response body is id-only, 422 message/context differs, auth via bare token.

### POST /integrations/:id/api_key/:keyid/refresh — PARTIAL (was MISMATCH; fixed in group 26, see below)

**Ghost** (`integrations.js:61-81` bound at `admin/routes.js:85-89`):
1. Identity: POST `/integrations/:id/api_key/:keyid/refresh`; same `authAdminApi` chain (`routes.js:87`). Explicitly routed **before** the generic `PUT /integrations/:id` but both use `api.integrations.edit`.
2. Operation: `edit`, docName `integrations`; `options:['id','keyid','include']` (`integrations.js:67`), `options.id` required (`:68-73`); `keyid` comes from the `:keyid` path param.
3. Input: `data:[name,icon_image,description,webhooks]` but refresh branch ignores body data (`integrations.js:66`).
4. Auth: `permissions:true`; same auth chain.
5. Status/headers: 200; `cacheInvalidate:false`.
6. Call chain: `integrations.service.edit(data,options)` (`integrations.js:78-80`) → with `options.keyid`: `ApiKeyModel.findOne({id:keyid})` (`integrations-service.js:15-16`); missing → `NotFoundError("ApiKey not found.")` (`:18-23`); `ApiKeyModel.refreshSecret(model.toJSON(), {...options,id:keyid})` rotates the secret (`:25-29`); then `IntegrationModel.findOne({id:options.id},{withRelated:['api_keys','webhooks']})` (`:31-36`); any error → `InternalServerError` (`:37-41`).
7. Cache: none.
8. Output: refreshed integration with `api_keys`,`webhooks` relations; admin secret prefixed `id:`.

**C++**:
1. `routes.cppm:63-79`: `/:id` → `/api_key` → `/:id` (keyid slot) → `Route{"/refresh"}.post(...)`. Structurally matches Ghost's path shape.
2. `routes.cppm:74`: dispatches `handler.integrations_edit()` — i.e. `EditHandler` (`all.cppm:440`), **not** the dedicated refresh handler.
3. A dedicated `IntegrationsIdApiKeyKeyidRefreshPostHandler` exists (`base.cppm:74`, `all.cppm:439`) with its own flow, but no route calls it; it is dead with respect to routing.
4. `edit/step/execute.cppm:31-43`: `target_id()` sees `/api_key/` and extracts the segment after `/integrations/` (the integration `id`), discarding keyid; `edit` step then calls `s_ctx->store().edit("integrations",id,body)` (`:21-26`). `body` is the request body (`""`→`{}`), not a key rotation.
5. The dedicated refresh step (`integrations_id_api_key_keyid_refresh_post/step/execute.cppm:21-29`) does the same: extract integration id via `path_id()` (`:34-42`, keyid never read), `store().edit("integrations",id,body)`. No `ApiKey` lookup, no `refreshSecret`, no relations reload.
6. Handler status mapping (`edit/handler.cppm:18-35`; refresh handler identical `:18-35`): 200/404/422; no special "ApiKey not found" vs "Integration not found" distinction.
7. Cache: no secret/key cache effect; only generic `resources` tag write in `edit` (`context_context.cppm:669-679`).
8. Output: id-only `{"integrations":[{"id":"..."}]}` with 200; no secret material returned.

**Diff (pre-rewrite)**: MISMATCH — the core operation was absent. Group 26 (below) resolved it: `keyid` is now parsed, the `api_keys` row is looked up (404 "ApiKey not found." when missing), `secret` is rotated by a registered outbox adapter, the integration is reloaded with `api_keys`+`webhooks`, and admin secrets are prefixed. Residual: shared JWT/session auth and `customIntegrations` limit only.

### PUT /integrations/:id — PARTIAL

**Ghost** (`integrations.js:61-81`; `admin/routes.js:90`):
1. Identity: PUT `/integrations/:id`, `authAdminApi` (`routes.js:90`).
2. Operation: `edit`, docName `integrations`.
3. Input: body picked to `[name,icon_image,description,webhooks]` (`integrations.js:66`, `input/integrations.js:28-32`); `options:['id','keyid','include']` (`integrations.js:67`); `id` required (`:68-73`); `include` values `api_keys,webhooks` (`:73-76`).
4. Auth: `permissions:true`; same chain.
5. Status/headers: 200; `cacheInvalidate:false`.
6. Call chain: `integrations.service.edit(data,options)` (`integrations.js:78-80`); without `keyid` → `IntegrationModel.edit(data,{...options,require:true})` (`integrations-service.js:44-45`); `NotFound`/`EmptyResponse` → `NotFoundError("Integration not found.")` (`:46-53`); other errors rethrown (`:54-56`). Model `onSaving` slug handling + edits relations (`models/integration.js:40-53,14-27`), emits `integration.edited` via `emitChange`.
7. Cache: none.
8. Output: edited integration; secret prefix mapper.

**C++**:
1. `routes.cppm:47-54`: PUT on `/:id` → `integrations_edit().handle`.
2. `edit/handler.cppm:18-35`: token check 401; status map 200/404/422.
3. `edit/step/execute.cppm:13-26`: validates `include` (`:14,18-20`); `target_id()` last path segment (`:21`,`:41-42`); missing record → 404 (`:22`, `:44-47`); reads body (`:23-24`); `store().edit("integrations",id,body)` (`:25`) → status/body.
4. Store: `resources().edit` overwrites supplied fields on cached record, writes back with `resources` tag / TTL never (`context_context.cppm:669-679`); if absent in cache it creates a new record (`:1352-1353`) — normally unreachable because the step checked existence first. Returns 200 `{"integrations":[{"id":"..."}]}` (`:1349-1350`).
5. Middleware content-type/accept required (`serde.cppm:90-126`).
6. Cache: `core::cache::Cache` put with tag `resources`, TTL never, as above.
7. Output: id-only; no full edited entity, no secret prefix.

**Diff**: residual gaps — permission service, model/transaction semantics, slug regeneration, `integration.edited` event, `require:true` error mapping, output mapper. Divergences — id-only response, body merged as raw fields, create-on-miss fallback in store, bare-token auth.

### DELETE /integrations/:id — PARTIAL

**Ghost** (`integrations.js:112-137`; `admin/routes.js:91`):
1. Identity: DELETE `/integrations/:id`, `authAdminApi` (`routes.js:91`).
2. Operation: `destroy`, docName `integrations` (`integrations.js:112,12`).
3. Input: `options:['id']` from path; `id` required (`:118-125`). No body/data.
4. Auth: `permissions:true`; same chain.
5. Status/headers: `statusCode:204` (`integrations.js:113`); `cacheInvalidate:false`; empty body.
6. Call chain: `models.Integration.destroy(Object.assign(options,{require:true}))` (`integrations.js:128`); `NotFoundError` → `errors.NotFoundError("Integration not found.")` (`:129-134`). Model emits `integration.deleted` via `onDestroy`/`emitChange` and enforces permissions/limits (`models/integration.js:35-38,96-116`).
7. Cache: none.
8. Output: none (204).

**C++**:
1. `routes.cppm:55-62`: DELETE registration on `/:id` → `integrations_destroy().handle`.
2. `destroy/handler.cppm:18-38`: token check 401 (`:19-23`); if payload code `=="404"` writes 404 + body, otherwise forces 204 NO_CONTENT with **empty body** (`:31-38`).
3. `destroy/workflow/flow/main.cppm:7-10`: single `ExecuteStep`.
4. `destroy/step/execute.cppm:13-25`: extracts last path segment as id (`:14-19`); empty or `resources().get("integrations",id)` miss → 404 `ApiError{}.json("Resource not found","Not Found")` (`:20-23`); `store().destroy("integrations",id)` (`:24`) → status/body (`:25`).
5. Store: generic `destroy` erases cached record, removes id from id list (`context_context.cppm:681-688`); `ApiDataStore::destroy` returns 404 on failure, otherwise 200 with `"{}"` for non-special resources (`:1356-1382`, esp. `:1373-1381`). Handler converts any non-404 to 204, so the wire status matches Ghost's 204.
6. Middleware content-type/accept required on DELETE (`serde.cppm:90-126`); Ghost does not require it.
7. Cache: `core::cache::Cache::erase(record_key(...))` on the `resources` store (`context_context.cppm:681-683`) — no `X-Cache-Invalidate` header emitted (grep: no `set_header`/cache-invalidate in integration handlers).
8. Output: empty 204 body — matches Ghost.

**Diff**: residual gaps — permission service / `integration.deleted` event / `require:true` model semantics / limit checks / exact "Integration not found." message. Divergences — store reports 200 then handler forces 204, mandatory content-type/accept, bare-token auth, different 404 message.

## Module-level notes

- **Router-level middleware (C++)**: only `content_type_middleware` on `Router{router,"/integrations"}` (`routes.cppm:19-20`). No `require_auth`, `optional_auth`, `bearer_middleware`, or local permission helper is installed on the router; each handler instead calls `middleware::token(req).empty()` (`bearer.cppm:27-33`) and returns 401 with a bare `{"error":"Unauthorized"}` body. `bearer_middleware` exists (`bearer.cppm:11-24`) but is not used on these routes.
- **Auth divergence**: Ghost `authAdminApi` performs real HS256 JWT verification + secret lookup, session auth, `authorizeAdminApi`, `updateUserLastSeen` (hourly `last_seen` write), and `tokenPermissionCheck`. Crucially, integration API tokens are **blocked** from all `/integrations` endpoints (not in the allowlist: `admin/middleware.js:54-99`). C++ accepts any non-empty Authorization value with no verification and does not update user/key state. This is the dominant cross-route residual gap.
- **Cache**: Ghost controllers uniformly set `headers.cacheInvalidate:false` (`integrations.js:14-16,31-33,62-64,84-86,113-115`) — no invalidation header. C++ has no response cache headers; the only cache activity is `core::cache::Cache` reads/writes inside `ResourceStore` keyed by `record_key("integrations",id)` with `resources` tag and `Ttl{}.never()` (`context_context.cppm:656-688`) — a store/marker primitive, not Ghost's HTTP cache contract.
- **Store backing**: integrations have no dedicated C++ entity; all six steps use the generic resource store (`json_for` → `search_json`, `resources().get/create/edit/destroy`, `total`) in `context_context.cppm:1140,1152-1210,656-688,1235-1382`. Initial `browse` returns whatever is in the in-memory store (typically empty), with fabricated `prev/next/pages` from count — unlike Ghost's Bookshelf `findPage`.
- **Output mapper**: Ghost's only integration-specific serializer is `output/mappers/integrations.ts:3-22`, which prefixes admin-key secrets with `id:`. No C++ equivalent exists; every C++ response is assembled as raw strings in the step handlers.
- **Refresh wiring defect — FIXED in group 26**: `routes.cppm` now dispatches the refresh POST to `integrations_integrations_id_api_key_keyid_refresh_post()` and the keyid slot is named `:keyid`. The step parses both path ids, looks up the `api_keys` row, rotates its secret through a registered `core_outbox` adapter (`ApiKeyRefreshAdapter`), reloads `api_keys`+`webhooks`, and returns the prefixed integration. `keyid` is no longer ignored.
- **No EXTRA / MISSING**: all 6 Ghost bindings have a C++ binding at the same path+verb; no C++ integration route lacks a Ghost counterpart.

## Strict-parity rewrite (group 26, 2026-11-20)

All edits confined to `include/blog/handlers/external/integrations/` and the two parity
docs. No shared file was changed; the Bazel partition cycle was left untouched.

### Shared abstractions used

- **`core::query`** — `browse` parses `include`/`limit`/`page`/`filter`/`fields`/`order`,
  injects Ghost's default `type:[custom,builtin,core]` filter exactly as
  `input/integrations.js` does, and uses `core::query::run` for filtered/sorted/paginated
  rows plus `meta.pagination`. Restricted NQL fields route through `core::query::guard`.
- **Ghost output envelope** — `support::wrap` (in the folder's DTO partition) builds the
  `{integrations:[...],meta}` envelope on every response; `browse` reuses the
  `meta.pagination` object produced by `core::query::run`. `blog_handlers:mapping` was
  deliberately not imported: its partitions are used by other groups but are absent from
  the registered `blog_handlers` BUILD partition list, so a local helper was used instead
  (no new partition / shared BUILD edit allowed).
- **`core_auth::ghost` via `blog_handlers_middleware::require_ghost_permission`** — the
  shared `GhostPermissionRegistry` is published by each handler's `support::authorized`
  gate with the route's Ghost operation name (`browse`/`read`/`add`/`edit`/`destroy`).
  The gate also implements `tokenPermissionCheck`: `x-blog-context` = `INTERNAL`/`SYSTEM`
  (integration token) is rejected with 403 `NoPermissionError` "API tokens do not have
  permission to access this endpoint", since `integrations` is absent from Ghost's
  allowlist. Denials use the Ghost-shaped envelope from `write_ghost_denial`.
- **`core_outbox` (via the registered `blog_handlers:outbox_shim` partition)** — CRUD
  side effects are staged in a transaction and committed: `integration.added`,
  `integration.edited`, `integration.deleted`. The refresh route registers
  `ApiKeyRefreshAdapter`, emits `api_key.secret_refreshed`, drains it (the adapter
  performs the store write) and records the resulting `integration.edited` event.
  Events stay observable through `Outbox::pending()` / `drain()`.

### API keys and webhooks

- `create` persists two `api_keys` rows (content = 26 hex chars, admin = 64 hex chars,
  both with `integration_id`) instead of splicing a literal into raw JSON, plus one
  `webhooks` row per supplied webhook. Slug is generated from `name` (Ghost
  `generateSlug`). Response is the full integration with relations.
- `edit` allowlists `name`/`icon_image`/`description`/`webhooks`, regenerates the slug on
  name change, replaces the webhook relation rows, and saves via `resources().edit`.
- `refresh` uses `keyid` from the path: missing key → 404 "ApiKey not found.", adapter
  failure → 500 "Internal Server Error", otherwise the integration is reloaded with
  `withRelated:['api_keys','webhooks']` and returned as 200.
- `destroy` cascades the integration's `api_keys`/`webhooks` rows.
- The output mapper (`prefix_admin_secrets` in the DTO partition) rewrites every admin
  key secret to `"<id>:<secret>"` on browse, read, create, edit and refresh.

### Verdict changes

- `POST /integrations/:id/api_key/:keyid/refresh`: **MISMATCH → PARTIAL**. The core
  operation (rotate secret by keyid, reload relations) is now implemented; only the
  shared-auth residual gaps below keep it from MATCH.
- The other five routes remain **PARTIAL** (no verdict change).

### Residual gaps (accepted, all cross-cutting)

1. Real HS256 JWT verification / session auth / `authorizeAdminApi` / hourly
   `updateUserLastSeen` live in shared admin middleware, not in this folder. The gate
   uses the shared `require_ghost_permission` abstraction instead.
2. `customIntegrations` limit enforcement needs the shared limits service; not available
   in the registered partitions used by this folder.
3. **New-partition / adapter registration**: all helpers were added to the folder's already-registered
   `types/dto_more_resources.cppm` partition; **no new partition is required**. The
   `blog_handlers:mapping` partitions (`base`/`clean`/`wrap`) are **not** listed in
   `include/blog/handlers/BUILD.bazel` (they are imported by other groups, so the main
   agent owns wiring them); this folder therefore uses a local envelope helper and
   records that registration as a new-partition need rather than importing them. `core_outbox`, `core_query` and
   `core_auth` are already declared deps of `//include/blog/handlers:blog_handlers`.
4. No unit test was added: adding a test file would require a shared BUILD edit. The
   behaviour is covered by the existing route-level harness.
