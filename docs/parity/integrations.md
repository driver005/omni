# Parity: integrations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/integrations.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 80–91)
- C++: `include/blog/handlers/external/integrations/routes.cppm` (installed at `include/blog/handlers/routes.cppm:24,97`; handlers registered at `include/blog/handlers/all.cppm:51-56,397-402,646-651`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 6 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Strict-parity group 26 applied 2026-11-20 (see "Fixes applied (group 26)" below): all six
routes parse their Ghost inputs through `core::query`/local helpers, use real store calls and
status codes, embed the `api_keys`/`webhooks` relations, prefix admin secrets, emit CRUD side
effects through `core_outbox`, and gate auth through the shared `require_ghost_permission`
middleware plus an integration-token (403) check. The refresh route now performs the actual
key rotation by `keyid` through a registered outbox adapter. All six remain PARTIAL only
because real JWT/session auth and the `customIntegrations` limit live in shared modules.

All six Ghost `integrations` routes have a 1:1 C++ route by normalized path + verb. Path, verb,
operation and the `"integrations"` DB resource agree on every row. No cache behavior on either
side (`cacheInvalidate:false` on Ghost; no cache header in C++).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/integrations` | browse | GET `/integrations` | `integrations_browse()` → `BrowseHandler` | Ghost: `options:['include','limit']`, include `['api_keys','webhooks']`, default type filter; C++: `core::query::parse` + injected filter | Ghost: `{integrations,meta}` 200; C++: `blog::mapping::wrap` with `core::query::run` meta, admin secret prefixed | `resources().list("integrations")` + `api_keys` relation rows | none | Ghost `mw.authAdminApi`+`permissions:true`; C++ tokenPermissionCheck + `require_ghost_permission('integrations','browse')` | PARTIAL | Ghost `integrations.js:13-29`; C++ `browse/step/execute.cppm` |
| GET `/integrations/:id` | read | GET `/integrations/:id` | `integrations_read_by_id()` → `ReadByIdHandler` | Ghost: `data:['id']` required, include validated, default filter; C++: real `:id`, include validated | Ghost: 200 / 404 "Integration not found."; C++: same, relations embedded, secret prefixed | `resources().get("integrations", id)` + relations | none | Ghost auth chain; C++ gate `read` | PARTIAL | Ghost `integrations.js:30-60`; C++ `read_by_id/step/execute.cppm` |
| POST `/integrations` | add | POST `/integrations` | `integrations_create()` → `CreateHandler` | Ghost: name required, api_keys injected; C++: allowlisted fields, slug gen, separate api_keys + webhook rows | Ghost: 201 full integration; C++: 201 `wrap("integrations", record)` with prefixed secret | `resources().create("integrations"/"api_keys"/"webhooks")` + `integration.added` outbox event | none | Ghost auth chain; C++ gate `add` | PARTIAL | Ghost `integrations.js:82-111`; C++ `create/step/execute.cppm` |
| POST `/integrations/:id/api_key/:keyid/refresh` | edit | POST `/integrations/:id/api_key/:keyid/refresh` | `integrations_integrations_id_api_key_keyid_refresh_post()` → dedicated handler (rebound in group 26) | Ghost: path `id`+`keyid`, options id/keyid/include; C++: both ids parsed, include validated | Ghost: 200 integration w/ relations, 404 ApiKey / 500; C++: same, secret rotated by outbox adapter, admin secret prefixed | `find_api_key` → `api_key.secret_refreshed` outbox event drained by `ApiKeyRefreshAdapter` → `rotate_api_key` → reload relations | none | Ghost auth chain; C++ gate `edit` | PARTIAL | Ghost `integrations-service.js:14-42`; C++ `integrations_id_api_key_keyid_refresh_post/step/execute.cppm`, `routes.cppm` |
| PUT `/integrations/:id` | edit | PUT `/integrations/:id` | `integrations_edit()` → `EditHandler` | Ghost: options id/keyid/include, data fields; C++: real `:id`, allowlisted fields, slug regen, webhooks replaced | Ghost: 200 edited integration; C++: same with `integration.edited` event + prefixed secret | `resources().edit("integrations", id, ...)` + relation rows + outbox | none | Ghost auth chain; C++ gate `edit` | PARTIAL | Ghost `integrations.js:61-81`; C++ `edit/step/execute.cppm` |
| DELETE `/integrations/:id` | destroy | DELETE `/integrations/:id` | `integrations_destroy()` → `DestroyHandler` | Ghost: options id required, `require:true`; C++: real `:id`, 404 on miss | Ghost: 204 empty / 404; C++: same, cascades key/webhook rows + `integration.deleted` event | `resources().destroy("integrations", id)` + cascade + outbox | none | Ghost auth chain; C++ gate `destroy` | PARTIAL | Ghost `integrations.js:112-137`; C++ `destroy/step/execute.cppm` |

## Route details

### GET `/integrations` — PARTIAL

- **Route identity — MATCH**: Ghost `router.get('/integrations', mw.authAdminApi, http(api.integrations.browse))` at `admin/routes.js:82`; C++ `Router{router, "/integrations"}...get(...)` at `include/blog/handlers/external/integrations/routes.cppm:19-28`. Verbatim path `/integrations`, verb GET.
- **Operation mapping — MATCH**: Ghost controller key `browse` (`integrations.js:13`); C++ binds `handler.integrations_browse()` (`routes.cppm:26`), accessor `blog::handlers::integrations::browse::BrowseHandler` (`all.cppm:397`, member `:646`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['include','limit']` with `include` values `['api_keys','webhooks']` (`integrations.js:18-25`). C++ `using Input = interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:12`) never reads query params; `json_for("integrations", {})` gets empty params (`:22`). Ghost-only params: `include`, `limit`.
- **Output parity — PARTIAL (divergent)**: Ghost `models.Integration.findPage(options)` (`integrations.js:27`) returns `{data,meta}` with real pagination via the serializer, status 200. C++ step `browse/.../execute.cppm:16-23` builds a lambda returning `{200, encode(IntegrationsResponse)}` with one default `RecordDto` (`:17-19`), then `if (r.first != 0)` (`:21`) is **always true** (r.first is 200), so it returns immediately. The `json_for` + hardcoded `meta.pagination {limit:15,page:1,pages:1,prev:null,next:null,total:0}` line (`:22-23`) is unreachable. Actual C++ body: `{"integrations":[{"id":"1","name":"Ghost Record","slug":"ghost-record"}]}` (`RecordDto` defaults at `include/blog/dto/more_resources.cppm:19-21`; `ARRAY_SERDE` key `integrations` at `:122-137`). No `meta` envelope. Handler always sets `Status::OK` (`browse/handler.cppm:27`).
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.findPage(options)` (`integrations.js:27`). C++ `ctx.store().json_for("integrations", {})` (`execute.cppm:22`) — correct resource string (`include/blog/context/context.cppm:150,239`) but unreachable and, if reached, no filtering/pagination.
- **Cache parity — MATCH (none)**: Ghost `headers.cacheInvalidate: false` (`integrations.js:14-16`); no `X-Cache-Invalidate` URL. C++ has no `core::cache::Cache`, namespace, `ResourceStore` tag, TTL or eviction in handler/step (`browse/handler.cppm:17-28`, `execute.cppm:14-24`). Both sides: no cache behavior.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:82`) + `permissions: true`, `docName: 'integrations'` (`integrations.js:12,17`). C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check returning `401 {"error":"Unauthorized"}` when `blog::handlers::middleware::token(req)` is empty (`browse/handler.cppm:18-22`); no permission/docName check.

#### Fix

**File/symbol:** `include/blog/handlers/external/integrations/browse/workflow/step/execute.cppm` → `blog::handlers::integrations::browse::workflow::ExecuteStep::invoke` (plus `browse/handler.cppm` → `BrowseHandler::handle`). **Checks closed: 3, 4, 5, 7.** No change: 1, 2, 6, 8.

- **1, 2, 6, 8 (no change):** `GET /integrations` → `integrations_browse()` already bound at `routes.cppm:21-27` / `all.cppm:397,646`; Ghost `cacheInvalidate:false` (`integrations.js:14-16`) and no C++ cache agree.
- **3 (input):** `execute.cppm:12` declares `using Input = IRequest&` but never reads the query. Parse `include` and `limit` from `in.get_path()` using the `admin/posts/browse/workflow/step/execute.cppm:21-29` `std::views::split('&')` pattern into `std::map<std::string,std::string> params`. Keep only `include`/`limit`; validate `include` against `{api_keys, webhooks}` (`integrations.js:21-23`), drop other values. Ghost-only params today: `include`, `limit`.
- **5 (DB call):** Delete the short-circuit lambda at `execute.cppm:16-21` — `if (r.first != 0)` is always true (`r.first` is 200), so `json_for` at `:22` is dead. Replace the whole block with `const auto rows = ctx.store().json_for("integrations", params);`. The resource string is already correct (`context.cppm:1180-1181` falls through to `parse_array(m_resources.list("integrations"))` + `search_json`, which honors `limit`, `offset` and `filter[field]`), so filtering/pagination become real instead of the single default `RecordDto`.
- **4 (output):** Remove the hardcoded `meta.pagination` (`limit:15,page:1,pages:1,total:0`) at `:23` and the dummy `IntegrationsResponse`/`RecordDto` at `:17-19`. Derive `limit` = parsed `limit` (default 15), `page` = parsed `page` (default 1), `total` = element count of parsed `rows`, `pages = ceil(total/limit)`, `prev`/`next` from `page`. Keep envelope key `integrations` (`ARRAY_SERDE` at `dto/response.cppm:185`); the `data` vs `integrations` name is serializer-only and does not change data. `browse/handler.cppm:27` `Status::OK` is correct for 200.
- **7 (auth):** token 401 (`browse/handler.cppm:18-22`) + `content_type_middleware` (`routes.cppm:20`) already gate the route. Optional closer for `mw.authAdminApi`/`permissions:true`: add `.use(blog::handlers::middleware::require_auth)` on `Router{router, "/integrations"}` at `routes.cppm:19-20`. No `docName`/permission middleware exists in the allowed modules — record as accepted residual gap, add no new global state.

Snippet (`execute.cppm`):

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
    for (auto part : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
        const std::string kv{std::ranges::begin(part), std::ranges::end(part)};
        const auto eq = kv.find('=');
        if (eq == std::string::npos) continue;
        auto key = kv.substr(0, eq), val = kv.substr(eq + 1);
        if (key == "limit" || key == "include") params[key] = val; // include ∈ {api_keys, webhooks}
    }
}
const auto rows = ctx.store().json_for("integrations", params);
// limit/page/pages/total derived from params + parsed rows (no hardcoded literals)
return core::workflow::StepResponse<Output>{"{\"integrations\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":" + limit + ",\"page\":" + page + ",\"pages\":" + pages + ",\"prev\":null,\"next\":null,\"total\":" + total + "}}}"};
```

### GET `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.get('/integrations/:id', ...)` (`admin/routes.js:83`); C++ `Router{router, "/:id"}...get(...)` (`routes.cppm:38-45`). Both `/integrations/:id`, verb GET.
- **Operation mapping — MATCH (renamed)**: Ghost key `read` (`integrations.js:30`); C++ `integrations_read_by_id()` → `ReadByIdHandler` (`routes.cppm:44`, `all.cppm:398,647`). Name differs, function equivalent.
- **Input parity — PARTIAL (divergent)**: Ghost `data: ['id']` required, `options: ['include']` values `['api_keys','webhooks']` (`integrations.js:35-47`), plus `require:true` (`:51`). C++ `read_by_id/.../execute.cppm:11` declares `using Input = IRequest&` but parses nothing; the store lookup uses a hardcoded `"1"` instead of the route `:id` (`:16`). Ghost-only params: `id`, `include`, `require`.
- **Output parity — PARTIAL (divergent)**: Ghost `findOne(data,{require:true})` → Integration `{data}`, 200 / `NotFoundError`→404 (`integrations.js:49-59`). C++ `{"integrations":[<one>]}` or `{"integrations":[]}` (`execute.cppm:17-18`), no `meta` envelope, always status 200 (`read_by_id/handler.cppm:27`); no 404 path.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.findOne` (`integrations.js:51`). C++ `ctx.store().find_by_resource("integrations", std::string{}.empty()?"1":std::string{})` (`execute.cppm:16`) — same resource string, but target id is constant `"1"` and ignores the URL `:id`.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:31-33`); C++ none in handler/step.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:83`) + `permissions:true` (`integrations.js:34`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`read_by_id/handler.cppm:18-22`).

#### Fix

**File/symbol:** `read_by_id/workflow/step/execute.cppm` → `ExecuteStep::invoke` (plus `read_by_id/handler.cppm` → `ReadByIdHandler::handle`). **Checks closed: 3, 4, 5, 7.** No change: 1, 2, 6, 8.

- **1, 2, 6, 8 (no change):** `GET /integrations/:id` → `integrations_read_by_id()` at `routes.cppm:39-46` / `all.cppm:398,647` maps Ghost `read`; no cache on either side.
- **3 (input):** stop ignoring `in`. Extract `:id` from `in.get_path()` with the `admin/posts/read_by_id/workflow/step/execute.cppm:19-22` idiom (strip `?`, trim trailing `/`, take segment after last `/`). Parse `include` from the query and validate against `{api_keys,webhooks}` (`integrations.js:44-46`). Ghost's `require:true` (`integrations.js:51`) has no store flag — enforce it by returning 404 when the lookup misses.
- **5 (DB call):** `ApiDataStore::find_by_resource` has **no** `"integrations"` branch (`context.cppm:1055-1091`), so `find_by_resource("integrations", std::string{}.empty()?"1":std::string{})` (`execute.cppm:16`) always returns `nullopt` — even the hardcoded `"1"` never resolves. Replace with `ctx.store().resources().get("integrations", id)` (`ResourceStore::get` at `context.cppm:698-702`, returns `""` on miss). This uses the existing context-local `resources` namespace — allowed for cross-request state.
- **4 (output):** on `record.empty()` return 404 with Ghost `NotFoundError` body (`integrations.js:53-56`) via `blog::handlers::middleware::write_error`; on hit return `{"integrations":[<record>]}`, 200. Change `Output` from `std::string` to `std::map<std::string,std::string>` carrying `{"status","200"|"404"}` + `{"body", ...}` (same pattern as `posts/read_by_id/.../execute.cppm:13,25-27`), and have `ReadByIdHandler::handle` read those to call `res.set_status(Status::OK / Status::NOT_FOUND)` instead of unconditional `Status::OK` (`read_by_id/handler.cppm:27`).
- **7 (auth):** token 401 + `content_type_middleware` already present (`read_by_id/handler.cppm:18-22`, `routes.cppm:20`); optional `.use(require_auth)` closes the Ghost auth mechanism. Permission/docName check: accepted gap (no allowed middleware).

Snippet (`execute.cppm`):

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
if (id.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", not_found_json}}};
const auto record = ctx.store().resources().get("integrations", id); // "" on miss
if (record.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", not_found_json}}};
return core::workflow::StepResponse<Output>{{{"status","200"},{"body","{\"integrations\":[" + record + "]}"}}};
```

### POST `/integrations` — PARTIAL

- **Route identity — MATCH**: Ghost `router.post('/integrations', ...)` (`admin/routes.js:84`); C++ `.post(...)` on `/integrations` (`routes.cppm:29-36`).
- **Operation mapping — MATCH (renamed)**: Ghost key `add` (`integrations.js:82`); C++ `integrations_create()` → `CreateHandler` (`routes.cppm:34`, `all.cppm:399,648`).
- **Input parity — PARTIAL (divergent)**: Ghost `data: ['name','icon_image','description','webhooks']`, `options: ['include']`, `name` required (`integrations.js:88-100`), and `query()` injects `api_keys: [{type:'content'},{type:'admin'}]` when absent (`:103-108`). C++ `create/.../execute.cppm:16` passes `in.body_as_string()` verbatim (or `"{}"`), no field allowlist, no `name` validation, no `api_keys` injection.
- **Output parity — PARTIAL (divergent)**: Ghost `statusCode: 201` (`integrations.js:83`) with serialized Integration `{data}`. C++ handler always sets `Status::OK` (200) (`create/handler.cppm:27`) and returns raw `mutations.create(...).body()` — no `{data,meta}` envelope, no 201.
- **DB call parity — MATCH-ish resource / PARTIAL call**: Ghost `models.Integration.add(dataWithApiKeys, options)` (`integrations.js:109`). C++ `mutations.create("integrations", body)` (`execute.cppm:16`) — same `"integrations"` resource (`context.cppm:150`) but generic create without the injected api_keys.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:84-86`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:84`) + `permissions:true` (`integrations.js:87`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`create/handler.cppm:18-22`).

#### Fix

**File/symbol:** `create/workflow/step/execute.cppm` → `ExecuteStep::invoke` (plus `create/handler.cppm` → `CreateHandler::handle`). **Checks closed: 3, 4, 5, 7.** No change: 1, 2, 6, 8.

- **1, 2, 6, 8 (no change):** `POST /integrations` → `integrations_create()` at `routes.cppm:29-36` / `all.cppm:399,648` maps Ghost `add`; no cache on either side.
- **3 (input):** `execute.cppm:16` forwards `in.body_as_string()` verbatim. Add `import core_serde;` and `import blog_handlers_middleware;` to the step module (the snippet reuses `blog::handlers::middleware::decode<T>` at `blog/handlers/middleware/serde.cppm:160-165`). Validate the body as a JSON object, allowlist `name, icon_image, description, webhooks` (`integrations.js:88`) and require non-empty `name` (`:92-94`; else 422 via `blog::handlers::middleware::write_error`). Inject `api_keys: [{type:'content'},{type:'admin'}]` when absent (`integrations.js:103-108`) into the payload before passing it to the store. Do not decode nested `webhooks`/`api_keys` into `std::string` map values — inject the `api_keys` literal into the raw body text so it stays a JSON array.
- **5 (DB call):** keep `mutations.create("integrations", payload)` (`execute.cppm:16`) — the generic branch `resources().create` stores the raw payload and returns `{"integrations":[{"id":"..."}]}` with status **201** (`context.cppm:1272-1276`). `"integrations"` is already a registered resource (`context.cppm:150,239`); no new resource string.
- **4 (output):** the step discards the 201 by returning only `MutationResult::body()` (`context.cppm:46-49`). Change `Output` to `std::map<std::string,std::string>` with `{"status", std::to_string(mr.status())}` and `{"body", mr.body()}` (`posts/read_by_id` pattern), and in `CreateHandler::handle` set `Status::CREATED` when `status=="201"`, else `Status::OK` (replace unconditional `Status::OK` at `create/handler.cppm:27`). Resource-key envelope is retained; 201 now matches Ghost (`integrations.js:83`).
- **7 (auth):** token 401 + `content_type_middleware` already present (`create/handler.cppm:18-22`, `routes.cppm:20`); optional `.use(require_auth)` for the Ghost auth mechanism. Permission/docName check: accepted gap.

Snippet (`execute.cppm`):

```cpp
std::string payload = in.body_as_string();
if (payload.empty()) payload = "{}";
const auto fields = blog::handlers::middleware::decode<std::map<std::string, std::string>>(payload);
if (!fields || !fields->contains("name") || fields->at("name").empty()) return /* 422 via write_error */;
if (!payload.contains("\"api_keys\"")) {
    const auto comma = payload != "{}" ? "," : "";
    payload.insert(payload.size() - 1, std::string{comma} + R"("api_keys":[{"type":"content"},{"type":"admin"}])");
}
const auto mr = ctx.store().create("integrations", payload);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(mr.status())}, {"body", mr.body()}}};
```

### POST `/integrations/:id/api_key/:keyid/refresh` — PARTIAL (pre-rewrite analysis below; superseded by group 26 section)

- **Route identity — MATCH**: Ghost `router.post('/integrations/:id/api_key/:keyid/refresh', ...)` (`admin/routes.js:85-89`); C++ `.add_router(Router{"/api_key"}) → Router{"/:id"} → Route{"/refresh"}.post(...)` (`routes.cppm:63-79`) yields `/integrations/:id/api_key/:id/refresh`. Placeholder name differs (`:keyid` vs `:id`) but normalizes to the same path; verb POST.
- **Operation mapping — MISMATCH**: Ghost binds this route to `api.integrations.edit` (`admin/routes.js:88`), i.e. controller `edit` (`integrations.js:61`). C++ binds a distinct dedicated handler `integrations_id_api_key_keyid_refresh_post()` (`routes.cppm:74`, `all.cppm:400,649`) whose step calls `mutations.create` (`.../workflow/step/execute.cppm:16`), not `edit`. Core operation materially differs: edit/refresh vs create.
- **Input parity — MISMATCH**: Ghost resolves `id` + `keyid` from the path and passes `options: ['id','keyid','include']` + `data` (`integrations.js:66-67`) to `integrations.service.edit` (`:79`). C++ `.../execute.cppm:11-16` parses no params — `keyid`/`id` never read; body passed raw as a *create* payload.
- **Output parity — PARTIAL (divergent)**: Ghost serialized Integration via `service.edit`, 200. C++ raw `mutations.create(...).body()`, status 200 (`refresh/handler.cppm:27`), no `{data,meta}` envelope.
- **DB call parity — MISMATCH**: Ghost `integrations.service.edit(data, options)` (`integrations.js:79`) edits an existing integration/key. C++ `mutations.create("integrations", body)` (`refresh/.../execute.cppm:16`) creates a new record.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:63-65`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:87`) + `permissions:true` (`integrations.js:65`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`refresh/handler.cppm:18-22`).

#### Fix

**File/symbol:** primary — `include/blog/handlers/external/integrations/routes.cppm` `IntegrationsRoutes::install()` (`:63-79`) rebinds to the edit handler; then `edit/workflow/step/execute.cppm` → `ExecuteStep::invoke` (see the PUT fix below). Alternative if keeping the dedicated handler — `integrations_id_api_key_keyid_refresh_post/workflow/step/execute.cppm` → `ExecuteStep::invoke`. **Checks closed: 1, 2, 3, 4, 5, 7.** No change: 6, 8.

- **1, 2 (operation, primary):** Ghost binds this path to `api.integrations.edit` (`admin/routes.js:88`, controller `edit` at `integrations.js:61`). C++ binds a separate handler whose step calls `mutations.create` (`routes.cppm:74`, `.../refresh/.../execute.cppm:16`) — materially wrong. Exact change: at `routes.cppm:69-75` replace `handler.integrations_integrations_id_api_key_keyid_refresh_post().handle(req, res, std::move(send));` with `handler.integrations_edit().handle(req, res, std::move(send));`. Remove the now-unused refresh handler/step from `all.cppm:400,649` registration (do not leave a second `/integrations*refresh` route). Alternatively, keep the dedicated handler but make its step call `mutations.edit` as in 5.
- **3 (input):** the refresh path carries two ids: `/integrations/<id>/api_key/<keyid>/refresh`. Parse both from `in.get_path()` — segment after `integrations` is `id`, segment after `api_key` is `keyid` (extend the `posts/read_by_id` last-segment idiom, `execute.cppm:19-22`). Pass `id` to `mutations.edit` and keep `keyid` in the include/options map (`integrations.js:67` `options:['id','keyid','include']`); `id` required (`:70-72`). `include` validated against `{api_keys,webhooks}` (`:73-75`). No C++ parsing exists today (`refresh/.../execute.cppm:11-16`).
- **5 (DB call):** replace `mutations.create("integrations", body)` (`refresh/.../execute.cppm:16`) with `mutations.edit("integrations", id, body)` (`ApiDataStore::edit` at `context.cppm:1279-1325`), matching Ghost `integrations.service.edit(data, options)` (`integrations.js:79`).
- **4 (output):** keep `Status::OK` 200 (`refresh/handler.cppm:27`) since Ghost edit is 200; carry the `MutationResult` status/body via the `std::map` pattern from the PUT fix and set the envelope from `mr.body()` instead of raw `.body()` (no `{data,meta}` today).
- **7 (auth):** token 401 + `content_type_middleware` already present (`refresh/handler.cppm:18-22`, `routes.cppm:20`); optional `.use(require_auth)` on the router. Permission/docName check: accepted gap.

Snippet (path ids + edit call, reusable in both refresh step and edit step):

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
const auto i = path.find("/integrations/");
const auto k = path.find("/api_key/");
const std::string id  = i == std::string_view::npos ? "" : std::string{path.substr(i + 14, path.find('/', i + 14))};
const std::string keyid = k == std::string_view::npos ? "" : std::string{path.substr(k + 9)}; // trim trailing "/refresh"
if (id.empty()) return /* 400/404 via write_error */;
const auto mr = ctx.store().edit("integrations", id, /* validated body */);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(mr.status())}, {"body", mr.body()}}};
```

- **6, 8 (no change):** `cacheInvalidate:false` on both sides; no Extra route after removing the dedicated refresh registration.

### PUT `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.put('/integrations/:id', ...)` (`admin/routes.js:90`); C++ `Router{"/:id"}...put(...)` (`routes.cppm:47-54`). Both `/integrations/:id`, verb PUT.
- **Operation mapping — MATCH**: Ghost key `edit` (`integrations.js:61`); C++ `integrations_edit()` → `EditHandler` (`routes.cppm:52`, `all.cppm:401,650`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['id','keyid','include']`, `data: ['name','icon_image','description','webhooks']`, `id` required (`integrations.js:66-77`). C++ `edit/.../execute.cppm:11-16` parses nothing; calls `mutations.edit("integrations", std::string{}, body)` with an empty id string. Ghost-only params: `id`, `keyid`, `include`, plus the named data fields.
- **Output parity — PARTIAL (divergent)**: Ghost Integration via `integrations.service.edit` (`integrations.js:79`), 200, `{data,meta}`. C++ raw `mutations.edit(...).body()`, status 200 (`edit/handler.cppm:27`), no envelope.
- **DB call parity — PARTIAL (divergent)**: Ghost `integrations.service.edit(data, options)` (`integrations.js:79`). C++ `mutations.edit("integrations", "", body)` (`edit/execute.cppm:16`) — same resource, but empty id means no real target record.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:62-64`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:90`) + `permissions:true` (`integrations.js:65`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`edit/handler.cppm:18-22`).

#### Fix

**File/symbol:** `edit/workflow/step/execute.cppm` → `ExecuteStep::invoke` (plus `edit/handler.cppm` → `EditHandler::handle`). **Checks closed: 3, 4, 5, 7.** No change: 1, 2, 6, 8.

- **1, 2, 6, 8 (no change):** `PUT /integrations/:id` → `integrations_edit()` at `routes.cppm:47-54` / `all.cppm:401,650`; Ghost `edit` (`integrations.js:61`); no cache.
- **3 (input):** `edit/.../execute.cppm:11-16` ignores `in`. Extract `:id` from `in.get_path()` with the `posts/read_by_id` idiom (`execute.cppm:19-22`) and pass it to `mutations.edit` instead of `std::string{}`. Parse `keyid`/`include` from the query (`integrations.js:67`); validate `include` against `{api_keys,webhooks}` (`:73-75`); allowlist body fields `name, icon_image, description, webhooks` (`:66`). Ghost-only params today: `id`, `keyid`, `include` + named data fields.
- **5 (DB call):** replace `mutations.edit("integrations", std::string{}, ...)` (`edit/.../execute.cppm:16`) with `mutations.edit("integrations", id, body)` — the generic branch `resources().edit(resource, id, body)` (`context.cppm:1320-1322`) now has a real target id, matching Ghost `integrations.service.edit(data, options)` (`integrations.js:79`).
- **4 (output):** Ghost edit returns 200 (`edit/handler.cppm:27` already 200) with a serialized Integration. Carry `MutationResult` status/body via `Output = std::map<std::string,std::string>` (`{"status", std::to_string(mr.status())}, {"body", mr.body()}`) and set the response from it. Residual, document only: `ApiDataStore::edit` falls back to `create` when `resources().edit` misses (`context.cppm:1323-1324`), so Ghost's `require`-style 404 is not enforced; guard with `ctx.store().resources().get("integrations", id).empty()` → 404 if strict parity is needed. Do not invent a new store resource.
- **7 (auth):** token 401 + `content_type_middleware` already present (`edit/handler.cppm:18-22`, `routes.cppm:20`); optional `.use(require_auth)`. Permission/docName check: accepted gap.

Snippet (`edit/.../execute.cppm`):

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
if (id.empty()) return /* 404 via write_error */;
const auto mr = ctx.store().edit("integrations", id, in.body_as_string().empty() ? "{}" : in.body_as_string());
return core::workflow::StepResponse<Output>{{{"status", std::to_string(mr.status())}, {"body", mr.body()}}};
```

### DELETE `/integrations/:id` — PARTIAL

- **Route identity — MATCH**: Ghost `router.delete('/integrations/:id', ...)` (`admin/routes.js:91`); C++ `Router{"/:id"}...delt(...)` (`routes.cppm:55-62`). `.delt` registers the DELETE handler (`include/utils/openapi/route.cppm:96,370`). Both `/integrations/:id`, verb DELETE.
- **Operation mapping — MATCH**: Ghost key `destroy` (`integrations.js:112`); C++ `integrations_destroy()` → `DestroyHandler` (`routes.cppm:60`, `all.cppm:402,651`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['id']` required (`integrations.js:118-124`) and `require:true` (`:128`). C++ `destroy/.../execute.cppm:11-16` parses nothing; `mutations.destroy("integrations", "")` receives an empty id. Ghost-only params: `id`, `require`.
- **Output parity — PARTIAL (divergent)**: Ghost `statusCode: 204`, empty body (`integrations.js:113`) / 404 on `NotFoundError` (`:130-134`). C++ raw `mutations.destroy(...).body()` with status 200 (`destroy/handler.cppm:27`) — wrong status and non-empty body.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Integration.destroy({require:true})` (`integrations.js:128`). C++ `mutations.destroy("integrations", "")` (`execute.cppm:16`) — same resource, no target id, no require semantics.
- **Cache parity — MATCH (none)**: Ghost `cacheInvalidate:false` (`integrations.js:114-116`); C++ none.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost `mw.authAdminApi` (`admin/routes.js:91`) + `permissions:true` (`integrations.js:117`). C++ `content_type_middleware` (`routes.cppm:20`) + token check 401 (`destroy/handler.cppm:18-22`).

#### Fix

**File/symbol:** `destroy/workflow/step/execute.cppm` → `ExecuteStep::invoke` and `destroy/handler.cppm` → `DestroyHandler::handle` (optionally `include/blog/context/context.cppm` → `ApiDataStore::destroy` for the 204 status). **Checks closed: 3, 4, 5, 7.** No change: 1, 2, 6, 8.

- **1, 2, 6, 8 (no change):** `DELETE /integrations/:id` → `integrations_destroy()` at `routes.cppm:55-62` / `all.cppm:402,651` (`.delt` = DELETE, `utils/openapi/route.cppm:96,370`); Ghost `destroy` (`integrations.js:112`); no cache.
- **3 (input):** `destroy/.../execute.cppm:11-16` ignores `in`. Extract `:id` from `in.get_path()` with the `posts/read_by_id` idiom (`execute.cppm:19-22`) and pass it instead of `std::string{}`. Ghost requires `options:['id']` (`integrations.js:118-124`) and `require:true` (`:128`); enforce a 404 on miss.
- **5 (DB call):** replace `mutations.destroy("integrations", std::string{})` (`execute.cppm:16`) with `mutations.destroy("integrations", id)` — generic branch `resources().destroy(resource, id)` (`context.cppm:1344`) returns 200 on success, 404 on miss (`context.cppm:1346-1348`). No new resource string.
- **4 (output):** Ghost returns **204, empty body** (`integrations.js:113`) / 404 on `NotFoundError` (`:130-134`); C++ returns raw `.body()` with 200 (`destroy/handler.cppm:27`). Two exact changes: (a) in `ApiDataStore::destroy` add `if (resource == "integrations") return MutationResult{204, ""};` on the success path alongside the `labels` case (`context.cppm:1349-1350`) — editing `blog_context` is allowed; (b) carry `MutationResult::status()`/`body()` via `Output = std::map<std::string,std::string>` and in `DestroyHandler::handle` set `Status::NO_CONTENT` with an empty body when `status=="204"`, `Status::NOT_FOUND` on `mr.ok()==false`, instead of unconditional `Status::OK`. If (a) is deferred, still map any successful destroy to `NO_CONTENT` + empty body in the handler to close the visible divergence.
- **7 (auth):** token 401 + `content_type_middleware` already present (`destroy/handler.cppm:18-22`, `routes.cppm:20`); optional `.use(require_auth)`. Permission/docName check: accepted gap.

Snippet (`destroy/.../execute.cppm` + `destroy/handler.cppm`):

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
if (id.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},{"body", not_found_json}}};
const auto mr = ctx.store().destroy("integrations", id);
return core::workflow::StepResponse<Output>{{{"status", std::to_string(mr.status())}, {"body", mr.body()}}};
// handler.cppm: status=="204" -> res.set_status(Status::NO_CONTENT), empty body; !mr.ok() -> Status::NOT_FOUND
```

## Fixes applied (2026-11-19, superseded by group 26 below)

All source edits confined to `include/blog/handlers/external/integrations/` (no shared files
changed). `Output` is `std::string` for browse and `std::map<std::string,std::string>`
(`status`/`body`) for the other five steps; each keeps `static blog::BlogContext* s_ctx`.

- **browse** (`browse/workflow/step/execute.cppm`): deleted the always-true short-circuit lambda
  and default `RecordDto`/`IntegrationsResponse`. Query `include`/`limit`/`page` parsed via an
  `options()` helper; `include` tokens filtered to `{api_keys, webhooks}`; `page` converted to
  `offset`. Real `json_for("integrations", params)` + `total("integrations")`; `limit`/`page`/
  `pages`/`prev`/`next`/`total` derived (no hardcoded literals). Envelope key `integrations`
  retained; handler still `Status::OK` (200). Handler file unchanged.
- **read_by_id** (`read_by_id/workflow/step/execute.cppm`, `read_by_id/handler.cppm`): `:id`
  extracted from the path; `include` validated. Store lookup switched from
  `find_by_resource` (no `integrations` branch — always missed) to
  `resources().get("integrations", id)`. Miss → 404 with Ghost `ApiError` body; hit → 200
  `{"integrations":[<record>]}`. Step returns `status`/`body` map; handler maps 404/422/200.
- **create** (`create/workflow/step/execute.cppm`, `create/handler.cppm`): body validated for
  required `name` (`decode` with `first_name` fallback → 422 on miss) and `api_keys`
  `[{type:content},{type:admin}]` injected when absent. Generic `create("integrations", payload)`
  kept (201). Step returns `status`/`body` map; handler maps 201 → `CREATED`, 422 →
  `UNPROCESSABLE_CONTENT`.
- **refresh** (`routes.cppm`, `edit/workflow/step/execute.cppm`, `integrations_id_api_key_keyid_refresh_post/*`):
  route rebound from the dedicated refresh handler to `integrations_edit()`, resolving the
  operation MISMATCH (edit, not create). The edit step's `target_id` understands both
  `/integrations/<id>` and `/integrations/<id>/api_key/<keyid>/refresh`, calls
  `edit("integrations", id, body)`, and returns `status`/`body`; 404 on missing record. The
  dedicated refresh step/handler were likewise corrected to call `edit` (defensive; unused by
  the bound route). No second refresh route exists.
- **edit** (`edit/workflow/step/execute.cppm`, `edit/handler.cppm`): real `:id` extracted from
  path (was `std::string{}`), `keyid`/`include` parsed and `include` validated, missing record →
  404 (strict `require` semantics), `edit("integrations", id, body)` returns 200. Handler maps
  status map.
- **destroy** (`destroy/workflow/step/execute.cppm`, `destroy/handler.cppm`): real `:id`,
  missing record → 404, `destroy("integrations", id)`. Handler returns `NO_CONTENT` + empty body
  on success and `NOT_FOUND` on miss. `ApiDataStore::destroy` was NOT edited (shared file): the
  generic branch still returns 200, but the handler maps it to 204, closing the visible
  divergence without touching `blog_context`.

## Fixes applied (group 26, 2026-11-20)

All edits confined to `include/blog/handlers/external/integrations/`. No shared file changed;
the Bazel partition cycle was left untouched. `Output` is `std::map<std::string,std::string>`
(`status`/`body`) for all six steps; each keeps `static blog::BlogContext* s_ctx`.

- **Shared support (all routes):** strict-parity helpers were added to the folder's existing,
  already-registered `types/dto_more_resources.cppm` partition — JSON object/array helpers,
  Ghost error envelopes, `slugify`, `new_secret` (content 26 hex / admin 64 hex), `now_iso`,
  `prefix_admin_secrets` (the output mapper), include validation, path/id/query parsing and
  store-templated relation helpers. **No new partition was added.**
- **Auth (all handlers):** replaced the bare `token()` 401 with `support::authorized`, which
  publishes a principal through the shared `GhostPermissionRegistry` and calls
  `require_ghost_permission("integrations", <op>)`; integration tokens
  (`x-blog-context: INTERNAL|SYSTEM`) get 403 `apiTokenBlocked`. Router-level
  content-type enforcement is now skipped for GET/DELETE (Ghost has no serde gate there).
- **browse:** `core::query::parse` + Ghost's default `type:[custom,builtin,core]` filter;
  `core::query::run` supplies real pagination/meta; `api_keys` relation embedded and admin
  secrets prefixed; a local `support::wrap` builds the envelope (the `blog_handlers:mapping`
  partitions are not registered in BUILD, so they were not imported).
- **read_by_id:** real `:id`, default type filter, 404 "Integration not found.", relations
  embedded, admin secrets prefixed.
- **create:** field allowlist + required `name` (422); slug generation; separate `api_keys`
  and `webhooks` store rows; `integration.added` outbox event; full 201 integration body.
- **refresh:** route rebound to the dedicated handler and `:keyid` slot; both ids parsed;
  `ApiKey` lookup with 404 "ApiKey not found."; `api_key.secret_refreshed` event drained by
  the newly registered `ApiKeyRefreshAdapter`, which rotates the stored secret; integration
  reloaded with `api_keys`+`webhooks`; 500 on adapter failure; admin secret prefixed.
- **edit:** real `:id`; field allowlist; slug regeneration; webhook relation replaced;
  `integration.edited` outbox event; full 200 integration body; 404 "Integration not found.".
- **destroy:** real `:id`; 404 "Integration not found."; cascades `api_keys`/`webhooks` rows;
  `integration.deleted` outbox event; 204 empty body.

**Verdict change:** refresh `MISMATCH → PARTIAL` (core key-rotation operation now implemented;
only shared-auth residual gaps remain). The other five stay PARTIAL.

**Residual gaps (accepted):** real HS256 JWT/session verification, `authorizeAdminApi` and the
hourly `updateUserLastSeen` write live in shared admin middleware, not this folder; the
`customIntegrations` limit needs the shared limits service. `{data,meta}` serializer naming is
not reproduced (resource-key envelope retained). New helpers needed no new partition; the
`blog_handlers:mapping` partitions are imported by the steps (as by many other groups) but are
not listed in `include/blog/handlers/BUILD.bazel` — main agent owns that registration. The
dedicated refresh handler registration previously flagged for optional cleanup is now actively
used by the route and must be kept. No unit test file was added (would require a shared BUILD
edit).

## Missing in C++

None. Every Ghost `integrations` route in `admin/routes.js:82-91` has a C++ route in
`include/blog/handlers/external/integrations/routes.cppm` by normalized path + verb:

1. GET `/integrations` — `admin/routes.js:82` → browse — `routes.cppm:19-28`
2. GET `/integrations/:id` — `admin/routes.js:83` → read — `routes.cppm:38-45`
3. POST `/integrations` — `admin/routes.js:84` → add — `routes.cppm:29-36`
4. POST `/integrations/:id/api_key/:keyid/refresh` — `admin/routes.js:85-89` → edit — `routes.cppm:63-79`
5. PUT `/integrations/:id` — `admin/routes.js:90` → edit — `routes.cppm:47-54`
6. DELETE `/integrations/:id` — `admin/routes.js:91` → destroy — `routes.cppm:55-62`

  Fix: No change required. The "None" verdict above is the checked result — there is no
  missing C++ route to add; checks 1–8 are already closed for route presence.

## Extra in C++

None. The matched folder `include/blog/handlers/external/integrations/` installs exactly six routes
(`routes.cppm:19-79`) matching the six Ghost controller routes one-to-one. Each C++ handler is
registered in `all.cppm:397-402,646-651` and no additional `/integrations*` route exists in that
folder's `routes.cppm`.
