# Parity: settings

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/settings.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/settings/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 56) — see
  [`## Group 56 — strict-parity rewrite`](#group-56--strict-parity-rewrite-2026-11-20).
  The `#### Fix` blocks above are retained for provenance; all seven routes are
  now implemented through shared abstractions at the C++ boundary. Verdicts are
  deliberately **not** flipped (residuals below need centrally owned primitives).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 2 |
| MISMATCH | 5 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/settings/routes/yaml` | `download` (docName `settings`) | GET `/settings/routes/yaml` (`routes.cppm:19,51-62`) | `settings_settings_routes_yaml_get()` → `SettingsRoutesYamlGetHandler` (`all.cppm:406`, `handler.cppm:10`) | Agree (none): Ghost `query()` ignores frame (`settings.js:212`); C++ `using Input=IRequest&`, no parse (`yaml_get/.../execute.cppm:11`) | DIVERGE: Ghost `response.format:'plain'` + YAML disposition `routes.yaml` (`settings.js:199-208`), returns YAML text (`settings.js:213`); C++ 200 + JSON `{"settings":<rows>,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` (`execute.cppm:17`, `handler.cppm:27`) | DIVERGE: Ghost `routeSettings.api.download()` reads routes.yaml from disk (`settings.js:213`); C++ `ctx.store().json_for("settings", {})` (`execute.cppm:16`) — wrong resource, no file read | Agree (none): Ghost `cacheInvalidate:false` (`settings.js:204`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:101`) + `permissions:{method:'browse'}` (`settings.js:209-211`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`handler.cppm:18-22`, `&& !true`), no permissions/docName | MISMATCH | Ghost: `settings.js:198-215`, `routes.js:101`. C++: `routes.cppm:19-20,51-62`; `yaml_get/handler.cppm:17-27`; `yaml_get/.../execute.cppm:11-17`; `all.cppm:406,655` |
| POST `/settings/routes/yaml` | `upload` | POST `/settings/routes/yaml` (`routes.cppm:51-70`) | `settings_settings_routes_yaml_post()` → `SettingsRoutesYamlPostHandler` (`all.cppm:407`, `handler.cppm:10`) | DIVERGE: Ghost multipart `upload.single('routes')` + `upload.validation({type:'routes'})` (`routes.js:105-106`), `fs.readFile(frame.file.path,'utf8')` (`settings.js:193`); C++ raw `in.body_as_string()`, no multipart/file (`execute.cppm:16`) | DIVERGE: Ghost 200, no body (`settings.js:192-195`); C++ 200 + raw `mutations.create(...).body()` (`execute.cppm:16`, `handler.cppm:27`) | DIVERGE: Ghost `routeSettings.api.upload(content)` writes routes.yaml (`settings.js:194`); C++ `mutations.create("settings", body)` creates a settings record — wrong resource | DIVERGE: Ghost `cacheInvalidate:true` → `X-Cache-Invalidate` (`settings.js:187`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:104`) + `permissions:{method:'edit'}` (`settings.js:189-191`); C++ `content_type_middleware` + dead token check | MISMATCH | Ghost: `settings.js:185-196`, `routes.js:102-108`. C++: `routes.cppm:63-70`; `yaml_post/handler.cppm:17-27`; `yaml_post/.../execute.cppm:11-16`; `all.cppm:407,656` |
| GET `/settings` | `browse` | GET `/settings` (`routes.cppm:19,21-28`) | `settings_browse()` → `BrowseHandler` (`all.cppm:366`, `handler.cppm:10`) | DIVERGE: Ghost `options:['group']` (`settings.js:35`); C++ `Input=IRequest&`, no query parsing, `group` ignored (`execute.cppm:11`) | DIVERGE: Ghost `settingsBREADService.browse(context)` calculated settings (`settings.js:38`); C++ `{"settings":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` hardcoded (`execute.cppm:17`), 200 (`handler.cppm:27`) | DIVERGE: Ghost `settingsBREADService.browse` (settings model + derived values) (`settings.js:38`); C++ `json_for("settings", {})` (`execute.cppm:16`), no group filter/pagination | Agree (none): Ghost `cacheInvalidate:false` (`settings.js:33`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:110`) + `permissions:true` (`settings.js:36`); C++ `content_type_middleware` + dead token check, no docName/permissions | PARTIAL | Ghost: `settings.js:31-40`, `routes.js:110`. C++: `routes.cppm:19-28`; `browse/handler.cppm:17-27`; `browse/.../execute.cppm:11-17`; `all.cppm:366,615` |
| PUT `/settings` | `edit` | PUT `/settings` (`routes.cppm:19,29-36`) | `settings_bulk_edit()` → `BulkEditHandler` (`all.cppm:408`, `handler.cppm:10`) | DIVERGE: Ghost `frame.data.settings` + `unsafeAttrsObject` for `labs` (`settings.js:143-155`); C++ raw `in.body_as_string()`, no wrapper/validation, mutation unreachable (`execute.cppm:15-19`) | DIVERGE: Ghost returns browse(all) + `meta=result.meta` (`settings.js:161-165`); C++ hardcoded `{"settings":{"title":"Ghost"},"meta":{}}` via early return (`execute.cppm:15-18`), 200 | DIVERGE: Ghost `settingsBREADService.edit(...)` (`settings.js:151-155`); C++ `mutations.bulk_edit("settings", body)` (`execute.cppm:19`) unreachable → no write | DIVERGE: Ghost `headers.cacheInvalidate:false` but query sets `X-Cache-Invalidate:'/*'` when result non-empty (`settings.js:141,157-159`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:111`) + `permissions.unsafeAttrsObject` (`settings.js:143-147`); C++ `content_type_middleware` + dead token check | PARTIAL | Ghost: `settings.js:139-167`, `routes.js:111`. C++: `routes.cppm:29-36`; `bulk_edit/handler.cppm:17-27`; `bulk_edit/.../execute.cppm:11-20`; `all.cppm:408,657` |
| POST `/settings/access_code/regenerate` | `regenerateAccessCode` | POST `/settings/access_code/regenerate` (`routes.cppm:37-50`) | `settings_settings_access_code_regenerate_post()` → `SettingsAccessCodeRegeneratePostHandler` (`all.cppm:409`, `handler.cppm:10`) | DIVERGE: Ghost no `options`/`data` declared; C++ raw body (`execute.cppm:16`) | DIVERGE: Ghost returns browse(all settings) (`settings.js:181`); C++ raw `mutations.create(...).body()`, 200 (`execute.cppm:16`, `handler.cppm:27`) | DIVERGE: Ghost `settingsService.regeneratePrivateSiteAccessCode()` (`settings.js:177`); C++ `mutations.create("settings", body)` — creates a record, does not regenerate access code | DIVERGE: Ghost `X-Cache-Invalidate:'/*'` (`settings.js:178`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:114`) + `permissions:{method:'edit'}` (`settings.js:173-175`); C++ `content_type_middleware` + dead token check | MISMATCH | Ghost: `settings.js:169-183`, `routes.js:112-116`. C++: `routes.cppm:37-50`; `settings_access_code_regenerate_post/handler.cppm:17-27`; `.../execute.cppm:11-16`; `all.cppm:409,658` |
| PUT `/settings/verifications/` | `verifyKeyUpdate` | PUT `/settings/verifications` (no trailing slash) (`routes.cppm:87-97`) | `settings_settings_verifications_put()` → `SettingsVerificationsPutHandler` (`all.cppm:410`, `handler.cppm:10`) | DIVERGE: Ghost `data:['token']`, passes `frame.data.token` (`settings.js:71,73`); C++ raw body, no token extraction (`execute.cppm:16`) | DIVERGE: Ghost returns browse(all settings) (`settings.js:75-78`); C++ raw `mutations.edit(...).body()`, 200 (`execute.cppm:16`, `handler.cppm:27`) | DIVERGE: Ghost `settingsBREADService.verifyKeyUpdate(token)` — verification, no CRUD write (`settings.js:73`); C++ `mutations.edit("settings","",body)` — persisting generic edit | DIVERGE: Ghost `cacheInvalidate:true` → `X-Cache-Invalidate` (`settings.js:66`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:117`) + `permissions:{method:'edit'}` (`settings.js:68-70`); C++ `content_type_middleware` + dead token check | MISMATCH | Ghost: `settings.js:64-80`, `routes.js:117`. C++: `routes.cppm:87-97`; `settings_verifications_put/handler.cppm:17-27`; `.../execute.cppm:11-16`; `all.cppm:410,659` |
| DELETE `/settings/stripe/connect` | `disconnectStripeConnectIntegration` | DELETE `/settings/stripe/connect` (`routes.cppm:73-86`) | `settings_settings_stripe_connect_delete()` → `SettingsStripeConnectDeleteHandler` (`all.cppm:411`, `handler.cppm:10`) | Agree (none): Ghost no `options`/`data`; C++ raw body unused (`execute.cppm:16`) | DIVERGE: Ghost HTTP **204**, no body (`settings.js:83`); C++ HTTP 200 + raw `mutations.bulk_destroy(...).body()` (`execute.cppm:16`, `handler.cppm:27`) | DIVERGE: Ghost paid-member check, `stripeService.disconnect()`, `models.Settings.edit` nulling 7 keys (`settings.js:91-135`); C++ `mutations.bulk_destroy("settings", body)` destroys records | Agree (none): Ghost `cacheInvalidate:false` (`settings.js:85`); C++ none | DIVERGE: Ghost `mw.authAdminApi` (`routes.js:120`) + `permissions:{method:'edit'}` (`settings.js:87-89`); C++ `content_type_middleware` + dead token check | MISMATCH | Ghost: `settings.js:82-137`, `routes.js:118-122`. C++: `routes.cppm:73-86`; `settings_stripe_connect_delete/handler.cppm:17-27`; `.../execute.cppm:11-16`; `all.cppm:411,660` |

## Route details

### GET `/settings/routes/yaml` — MISMATCH

- Route identity agrees: Ghost `routes.js:101` `GET /settings/routes/yaml` → `api.settings.download`; C++ `routes.cppm:19,51-62` `Router{router,"/settings"}` → `/routes` → `Route{"/yaml"}` `.get`.
- Operation mapping agrees at name level: Ghost `download` ↔ C++ `settings_settings_routes_yaml_get()` (`routes.cppm:60`).
- Output materially differs: Ghost streams plain YAML with `Content-Disposition: ... filename="routes.yaml"` (`settings.js:199-208`) from `routeSettings.api.download()` (`settings.js:213`). C++ returns a JSON settings envelope with hardcoded `meta.pagination` (`yaml_get/.../execute.cppm:16-17`) at HTTP 200 (`handler.cppm:27`) — no disposition header, no plain format.
- DB/resource differs: Ghost reads routes.yaml from disk, not the Settings model; C++ reads the `"settings"` store resource (`execute.cppm:16`).
- Cache agrees (none): Ghost `cacheInvalidate:false` (`settings.js:204`); C++ has no cache (handlers built with `m_ctx` only, `all.cppm:655`; global `core::cache::Cache` at `all.cppm:836` wired only to members/emails/automations ops, `:837-839`).
- Auth differs: Ghost admin auth + browse permission; C++ `content_type_middleware` (`routes.cppm:20`) + token check that is dead code because of `&& !true` (`handler.cppm:18-22`) — no administrative authorization, no permission/docName mapping.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request state stays inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `include/blog/handlers/system/settings/routes.cppm:19,51-62` already `Router{router,"/settings"}` + `/routes` + `Route{"/yaml"}.get(...)`; installer `blog::api::SettingsRoutes::install` (`routes.cppm:15`). Ghost `admin/routes.js:101`. Verbatim path and verb match; nothing to add.
2. Operation mapping — no change required. `settings_routes_yaml_get/workflow/flow/main.cppm:9` `MainFlow::build()` registers `ExecuteStep` as workflow `"settings_settings_routes_yaml_get"`; `settings_routes_yaml_get/handler.cppm:23` runs that name; Ghost key `download` (`settings.js:198,212`).
3. Input — no change required. Ghost `query()` ignores the frame (`settings.js:212`); C++ `using Input=interfaces::io::IRequest&` and `in` is unused (`settings_routes_yaml_get/workflow/step/execute.cppm:11,13`). Both sides: no params.
4. Output — REPLACE two symbols. (a) `blog::handlers::settings::settings_routes_yaml_get::workflow::ExecuteStep::invoke` (`settings_routes_yaml_get/workflow/step/execute.cppm:13-18`): replace the return at `:17` — `{"settings":…,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` — with YAML text reconstructed from the stored `routes_yaml` record (see check 5); default to a minimal YAML stub when empty. Add `import core_serde;` for `core_serde::JsonCodec{}.decode<std::string>` (`include/blog/context/context.cppm:1132` uses the same codec family). (b) `settings_routes_yaml_get/handler.cppm:26-27`: before `res.set_body(...)` add `res.set_header(interfaces::io::types::Token::CONTENT_TYPE,"text/plain");` and `res.set_header(interfaces::io::types::Token::CONTENT_DISPOSITION,"attachment; filename=\"routes.yaml\"");` — `set_header` at `include/interfaces/io/response.cppm:589-590`, tokens at `include/interfaces/io/types.cppm:197,508`. Keep `Status::OK` (200). This closes Ghost `response.format:'plain'` (`settings.js:207`) and `headers.disposition` (`settings.js:199-203`). No `X-Cache-Invalidate` (Ghost `cacheInvalidate:false`, `settings.js:204`).
5. DB call — REPLACE (read source + resource). `settings_routes_yaml_get/workflow/step/execute.cppm:16`: `ctx.store().json_for("settings", {})` → `blog::api::parse_array(ctx.store().resources().list("routes_yaml"))` (`blog::api::parse_array` at `context.cppm:595`, `ApiDataStore::resources()` at `:1101-1104`, `ResourceStore::list` at `:704-715`). Resource string becomes `"routes_yaml"` — the file stand-in written by the POST fix (check 5 of the POST block below). Ghost `routeSettings.api.download()` (`settings.js:213`) reads routes.yaml from disk; direct file I/O is disallowed, so the context-local `ResourceStore` record is the exact stand-in. No SQL, no new global state.
6. Cache — no change required. Ghost `cacheInvalidate:false` (`settings.js:204`); C++ handler ctor binds `ctx`/workflow only (`settings_routes_yaml_get/handler.cppm:12-16`), and `all.cppm:836-839` wires the only `core::cache::Cache` to members/emails/automations ops. Both sides: none.
7. Auth — REPLACE. Delete the dead inline check `if (token(req).empty() && !true) {...}` in `settings_routes_yaml_get/handler.cppm:18-22`. In `routes.cppm:21-29` add `.use(blog::handlers::middleware::require_auth)` on this route (signature `interfaces::MiddlewareFn` = `void(IRequest&,IResponse&,NextFn&&,std::function<void()>)`, `include/interfaces/protocol.cppm:138-141`; impl `include/blog/handlers/middleware/auth.cppm:12-29`). Chain `.use(blog::handlers::middleware::require_permission)` with a new helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"browse"` (Ghost `permissions:{method:'browse'}`, `settings.js:209-211`; `mw.authAdminApi`, `routes.js:101`). Also move router-wide `content_type_middleware` (`routes.cppm:20`) off this GET route — its JSON `Accept` requirement (`middleware/serde.cppm:110-121`) conflicts with Ghost's plain-format download. Permission state, if any, lives under `ctx.store().resources()` (`resources` namespace), no new globals. Absence proof: grep `require_permission|docName` over `include/blog/handlers/**/*.cppm` → no permission middleware today.
8. Verdict + evidence — after 4,5,7 → `MATCH`. Recheck `routes.js:101`, `settings.js:198-215` vs `routes.cppm:19,51-62`, `settings_routes_yaml_get/handler.cppm:12-28`, `settings_routes_yaml_get/workflow/step/execute.cppm:11-18`.

Ambiguity — no YAML library allowed; reconstruct plain scalars with the std JSON codec (JSON string literals are valid YAML scalars). Short snippet for checks 4/5:

```cpp
// settings_routes_yaml_get/workflow/step/execute.cppm — add `import core_serde;`
const auto rows = blog::api::parse_array(ctx.store().resources().list("routes_yaml"));
std::string yaml;
if (!rows.empty()) {
    if (const auto it = rows.back().find("content"); it != rows.back().end()) {
        yaml = core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
    }
}
if (yaml.empty()) yaml = "# See https://docs.ghost.org/themes/routing/#routes\nroutes: null\n";
return core::workflow::StepResponse<Output>{yaml};
```

### POST `/settings/routes/yaml` — MISMATCH

- Route identity agrees: Ghost `routes.js:102-108` `POST /settings/routes/yaml` with `upload.single('routes')` + `upload.validation({type:'routes'})`; C++ `routes.cppm:63-70` `.post` on `/routes/yaml`.
- Input differs: Ghost multipart field `routes` read from disk (`settings.js:193`); C++ reads raw `body_as_string()` with no multipart handling (`yaml_post/.../execute.cppm:16`).
- Operation/resource materially differs: Ghost `routeSettings.api.upload(content)` writes routes.yaml (`settings.js:194`); C++ `mutations.create("settings", body)` inserts a settings record (`execute.cppm:16`).
- Output differs: Ghost no body; C++ returns the raw create mutation body at 200 (`handler.cppm:25-27`).
- Cache differs: Ghost `cacheInvalidate:true` (`settings.js:187`); C++ none.
- Auth differs: Ghost admin auth + edit permission (`routes.js:104`, `settings.js:189-191`); C++ content-type + dead token check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; cross-request state stays inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `routes.cppm:19,63-70` `Route{"/yaml"}.post(...)` under `Router{router,"/settings"}` + `/routes`; Ghost `routes.js:102-108`. Verbatim path/verb match.
2. Operation mapping — no change required. `settings_routes_yaml_post/workflow/flow/main.cppm:9` registers `ExecuteStep` as workflow `"settings_settings_routes_yaml_post"`; `settings_routes_yaml_post/handler.cppm:23` runs it; Ghost key `upload` (`settings.js:185,192`).
3. Input — REPLACE (multipart + content-type gate). Symbol `blog::handlers::settings::settings_routes_yaml_post::workflow::ExecuteStep::invoke` (`settings_routes_yaml_post/workflow/step/execute.cppm:13-17`): replace raw `in.body_as_string().empty()?"{}":in.body_as_string()` (`:16`) with extraction of the `name="routes"` part and its `filename="..."` from the request via `in.find_header(interfaces::io::types::Token::CONTENT_TYPE)` (`middleware/serde.cppm:97`) and `in.raw_body()`/`in.body_as_string()` — split on the `boundary=` value, keep the part bytes as `std::string yaml`; reject empty/invalid with a `__BAD_REQUEST__` sentinel mapped by the handler to `Status::BAD_REQUEST` (`include/interfaces/io/types.cppm:37`). Add `import core_serde;`. Ghost reads `frame.file.path` via `fs.readFile` (`settings.js:193`); no file I/O allowed, so the in-memory part bytes stand in. Also change `routes.cppm:19-20`: the router-wide `content_type_middleware` rejects multipart (`middleware/serde.cppm:97-98` checks `starts_with("application/json")`), so either scope it to the JSON routes only or add a multipart-accepting `blog_handlers_middleware` variant in allowed module `blog_handlers_middleware` (`middleware/serde.cppm:90-126`) — no third-party parser.
4. Output — REPLACE. `settings_routes_yaml_post/handler.cppm:23-27`: replace echoed `body` (from `mutations.create("settings",body).body()`) with the fixed acknowledgement `"{}"` — Ghost `upload` returns no body (`settings.js:192-195`); keep `Status::OK` (200). Map the check-3 sentinel to `Status::BAD_REQUEST` before `send()`.
5. DB call — REPLACE (replace-all, not create-one). In `settings_routes_yaml_post/workflow/step/execute.cppm:16`: drop `mutations.create("settings", ...)` entirely. Exact replacement: `auto& rs = ctx.store().resources();` (`context.cppm:1101-1104`); for each `rec` in `blog::api::parse_array(rs.list("routes_yaml"))` (`context.cppm:595,704-715`) decode `id` with `core_serde::JsonCodec{}.decode<std::string>(it->second)` then `rs.destroy("routes_yaml", id)` (`ResourceStore::destroy`, `context.cppm:665-672`); then `rs.create("routes_yaml", R"({"content":)" + core_serde::JsonCodec{}.encode(yaml) + "})");` (`ResourceStore::create`, `context.cppm:640-651`). Same context-local `resource:routes_yaml:<id>` keys (`context.cppm:725-727`), tag `resources`, TTL `never()` (`:645-646`); this is the record the GET fix reads. Ghost `routeSettings.api.upload(content)` writes routes.yaml (`settings.js:194`). No SQL, no globals.
6. Cache — REPLACE. Ghost `cacheInvalidate:true` (`settings.js:187`). Add in `settings_routes_yaml_post/handler.cppm` before `send()` (`:26-27`): `res.set_header("X-Cache-Invalidate","/*");` (`include/interfaces/io/response.cppm:589-590`). The check-5 destroy/create cycle erases stale `resource:routes_yaml:*` keys (`ResourceStore::destroy` → `m_cache.erase`, `context.cppm:667`) and re-puts under tag `resources` (`:645-646`); no new namespace or TTL.
7. Auth — same gap as the GET fix, check 7. Delete the dead inline check (`settings_routes_yaml_post/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` and a `require_permission` helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"edit"`, on the upload route lambda (`routes.cppm:34-40`), ahead of `handler.settings_settings_routes_yaml_post().handle(...)`; apply the check-3 content-type fix too. Ghost: `mw.authAdminApi` (`routes.js:104`) + `permissions:{method:'edit'}` (`settings.js:189-191`). Nonce/token state, if any, stays under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7 → `MATCH`. Recheck `routes.js:102-108`, `settings.js:185-196` vs `routes.cppm:19,63-70`, `settings_routes_yaml_post/handler.cppm:17-28`, `settings_routes_yaml_post/workflow/step/execute.cppm:11-17`.

Ambiguity — multipart boundary extraction without a parser library. Short snippet for checks 3/5:

```cpp
// settings_routes_yaml_post/workflow/step/execute.cppm — add `import core_serde;`
// `part` = bytes between the boundary delimiters following `name="routes"`.
if (part.empty()) return core::workflow::StepResponse<Output>{"__BAD_REQUEST__"};
const std::string yaml{part};
auto& rs = ctx.store().resources();
for (const auto& rec : blog::api::parse_array(rs.list("routes_yaml"))) {
    const auto it = rec.find("id"); if (it == rec.end()) continue;
    const auto id = core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
    rs.destroy("routes_yaml", id);
}
rs.create("routes_yaml", R"({"content":)" + core_serde::JsonCodec{}.encode(yaml) + "})");
return core::workflow::StepResponse<Output>{"{}"};
```

`core_serde::JsonCodec{}.encode(yaml)` produces a quoted JSON string, which `ResourceStore::create` parses via `JsonBody{}.parse_raw` (`context.cppm:642`); the GET fix decodes it back verbatim.

### GET `/settings` — PARTIAL

- Route identity + core operation agree: Ghost `browse` reads all settings (`settings.js:37-39`); C++ `settings_browse()` reads all `"settings"` rows (`browse/handler.cppm:23`, `browse/.../execute.cppm:16`).
- Input differs: Ghost `options:['group']` (`settings.js:35`); C++ `Input=IRequest&` with no query parsing (`execute.cppm:11`) — `group` is ignored.
- Output differs: Ghost returns calculated settings from BREAD (`settings.js:38`); C++ fabricates `meta.pagination` with hardcoded `limit:15/page:1/pages:1/total:0` (`execute.cppm:17`).
- DB call differs: no `group` filter or pagination applied by C++ `json_for("settings", {})` (`execute.cppm:16`).
- Cache agrees (none): `cacheInvalidate:false` (`settings.js:33`) vs C++ none.
- Auth differs: Ghost `mw.authAdminApi` (`routes.js:110`) + `permissions:true` (`settings.js:36`); C++ content-type + dead token check (`browse/handler.cppm:18-22`), no docName/permissions.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; any settings overrides stay inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `routes.cppm:19,21-28` `Route{"/settings"}.get(...)` via `Router{router,"/settings"}`; Ghost `routes.js:110`. Verbatim path/verb match.
2. Operation mapping — no change required. `browse/workflow/flow/main.cppm:9` registers `ExecuteStep` as `"settings_browse"`; `browse/handler.cppm:23` runs it; Ghost key `browse` (`settings.js:31,37`).
3. Input — REPLACE. Symbol `blog::handlers::settings::browse::workflow::ExecuteStep::invoke` (`browse/workflow/step/execute.cppm:13-18`): parse the query string with the established pattern at `include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-29` (split `in.get_path()` on `'?'` then `'&'` then `'='` into `std::map<std::string,std::string> params`), read `group` — the only Ghost option (`settings.js:35`). No `data[]`/multipart/body on either side.
4. Output — REPLACE. `browse/workflow/step/execute.cppm:17`: delete the hardcoded `meta.pagination` envelope; return `"{\"settings\":" + rows + "}"`. Ghost `settingsBREADService.browse()` returns calculated settings with no pagination meta (`settings.js:38`). Keep `Status::OK` in `browse/handler.cppm:27` — no change required there.
5. DB call — REPLACE (honor `group`, merge stored overrides). `browse/workflow/step/execute.cppm:16`: `ctx.store().json_for("settings", {})` → `ctx.store().json_for("settings", params)`. In `include/blog/context/context.cppm`, `blog::api::ApiDataStore::json_for`, the `"settings"` branch (`context.cppm:1132`) currently returns `settings_json()` ignoring params: change the call to `settings_json(params)`. In `ApiDataStore::settings_json()` (`context.cppm:1653-1687`) add a `const std::map<std::string,std::string>& params = {}` parameter, add a static local `key→group` table inside the method (no new global state), drop fixture keys whose group differs from `params["group"]`, and merge any stored overrides from `parse_array(m_resources.list("settings"))` (`context.cppm:595,704-715`) over the fixture. This also supplies the write-through used by the PUT/regenerate/Stripe fixes. Resource string stays `"settings"`; `core_serde::JsonCodec` decodes stored values (`:1132` already uses this codec family).
6. Cache — no change required. Ghost `headers.cacheInvalidate:false` (`settings.js:33`); C++ handler ctor (`browse/handler.cppm:12-16`) binds `ctx`/workflow only and `all.cppm:836-839` wires cache only to members/emails/automations. Both: none.
7. Auth — REPLACE. Delete the dead inline check (`browse/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` on the GET route (`routes.cppm:21-27`, `include/blog/handlers/middleware/auth.cppm:12-29`) plus a `require_permission` helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"browse"` (Ghost `permissions:true`, `settings.js:36`; `mw.authAdminApi`, `routes.js:110`). Scope router-wide `content_type_middleware` (`routes.cppm:20`) off GET routes if it rejects the Ghost `Accept` header (`middleware/serde.cppm:110-121`).
8. Verdict + evidence — after 3,4,5,7 → `MATCH`. Recheck `routes.js:110`, `settings.js:31-40` vs `routes.cppm:19-28`, `browse/handler.cppm:12-28`, `browse/workflow/step/execute.cppm:11-18`.

Ambiguity — Ghost's `group` filter needs a key→group mapping the fixture lacks. Short snippet for checks 3/5:

```cpp
// browse/workflow/step/execute.cppm — query parse (as posts/browse/execute.cppm:21-29)
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
    for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
        if (const auto eq = text.find('='); eq != std::string::npos)
            params[text.substr(0, eq)] = text.substr(eq + 1);
    }
}
const auto rows = ctx.store().json_for("settings", params);
return core::workflow::StepResponse<Output>{"{\"settings\":" + rows + "}"};
```

```cpp
// context.cppm — ApiDataStore::settings_json(params), inside the method
static constexpr std::pair<std::string_view, std::string_view> kGroup[]{
    {"members_track_sources", "members"}, {"title", "core"}, {"locale", "core"},
    /* ...one entry per kSettings key... */
};
const auto wanted = params.contains("group") ? std::optional<std::string_view>{params.at("group")} : std::nullopt;
// when emitting each kSettings pair: if (wanted && group_of(key) != *wanted) continue;
```

### PUT `/settings` — PARTIAL

- Route identity + core operation agree in intent: Ghost `edit` updates settings (`settings.js:148-155`); C++ `.put` binds `settings_bulk_edit()` (`routes.cppm:34`).
- Input differs: Ghost extracts `frame.data.settings` and uses `unsafeAttrsObject` for `labs` (`settings.js:143-155`); C++ passes raw body with no wrapper extraction or permission logic (`bulk_edit/.../execute.cppm:19`).
- Output differs: Ghost returns full recalculated settings plus `meta` (`settings.js:161-165`); C++ returns a fixed `{"settings":{"title":"Ghost"},"meta":{}}` via the early return at `execute.cppm:15-18`, so the mutation at `:19` is unreachable.
- DB call differs: Ghost `settingsBREADService.edit` (`settings.js:151`); C++ effective write = none (unreachable `mutations.bulk_edit("settings", ...)`).
- Cache differs: Ghost sets `X-Cache-Invalidate:'/*'` on non-empty result (`settings.js:157-159`); C++ none.
- Auth differs: Ghost admin auth + `unsafeAttrsObject` permission; C++ content-type + dead token check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; settings writes stay inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `routes.cppm:19,29-36` `.put(...)` on `Router{router,"/settings"}`; Ghost `routes.js:111`. Verbatim path/verb match.
2. Operation mapping — no change required. `bulk_edit/workflow/flow/main.cppm:9` registers `ExecuteStep` as `"settings_bulk_edit"`; `bulk_edit/handler.cppm:23` runs it; Ghost key `edit` (`settings.js:139,148`).
3. Input — REPLACE (wrapper extraction + `labs` gate). Symbol `blog::handlers::settings::bulk_edit::workflow::ExecuteStep::invoke` (`bulk_edit/workflow/step/execute.cppm:13-20`): add `import core_serde;`, decode `in.body_as_string()` with `core_serde::JsonCodec{}.decode<serde::Value>(...)`, and extract the `settings` wrapper array (`frame.data.settings`, `settings.js:152`) into `std::vector<blog::api::RecordDto>` via `blog::api::parse_array` on the encoded sub-value (`context.cppm:595`). Detect a `labs` entry (`settings.js:145`) and route it through the check-7 permission helper; reject malformed bodies with `blog::api::ApiError{}.validation(...)` (`context.cppm:76-84`). `using Input=interfaces::io::IRequest&` stays (`execute.cppm:11`).
4. Output — REPLACE. `bulk_edit/workflow/step/execute.cppm:15-19`: delete the early-return lambda `return {200, R"({"settings":{"title":"Ghost"},"meta":{}})"};` (`:16`) and the `if(r.first!=0) return ...` guard (`:18`) so the code after it is reachable. After applying edits, return the browse-all envelope plus meta: `"{\"settings\":" + ctx.store().json_for("settings", params) + ",\"meta\":" + meta + "}"` where `meta` is the BREAD-edit metadata (emit `result.meta` when available, else `{}`) — Ghost `settings.js:161-165`. Keep `Status::OK` at `bulk_edit/handler.cppm:27` — no change required there.
5. DB call — REPLACE (real write via stored overrides). `bulk_edit/workflow/step/execute.cppm:19`: `mutations.bulk_edit("settings", body)` → for each parsed `{key,value}` record, `auto& rs = ctx.store().resources();` (`context.cppm:1101-1104`) then `rs.create("settings", R"({"key":)" + core_serde::JsonCodec{}.encode(key) + R"(,"value":)" + core_serde::JsonCodec{}.encode(value) + "})");` (`ResourceStore::create`, `context.cppm:640-651`). This is the override set that `ApiDataStore::settings_json()` merges (established in the GET `/settings` fix, check 5) and that the browse-all output of check 4 reads. Ghost `settingsBREADService.edit(...)` (`settings.js:151-155`). No SQL, no globals; cross-request state under the existing `resources` namespace/tag, TTL `never()` (`context.cppm:645-646`).
6. Cache — REPLACE. Ghost `headers.cacheInvalidate:false` but sets `frame.setHeader('X-Cache-Invalidate','/*')` when the edit result is non-empty (`settings.js:141,157-159`). In `bulk_edit/handler.cppm` before `send()` (`:26-27`): `if (!body.empty() && body != "{}") res.set_header("X-Cache-Invalidate","/*");` (`include/interfaces/io/response.cppm:589-590`). No new cache key/namespace/TTL.
7. Auth — REPLACE. Delete the dead inline check (`bulk_edit/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` on the PUT route (`routes.cppm:29-35`) plus a `require_permission` helper in allowed module `blog_handlers_middleware` that inspects the parsed `settings` array for the `labs` key and enforces Ghost's `unsafeAttrsObject` rule (`settings.js:143-147`), keyed `docName="settings"`, action `"edit"`. Wire it ahead of `handler.settings_bulk_edit().handle(...)` (`routes.cppm:34`). Permission state, if any, stays under `ctx.store().resources()`.
8. Verdict + evidence — after 3,4,5,6,7 → `MATCH`. Recheck `routes.js:111`, `settings.js:139-167` vs `routes.cppm:19,29-36`, `bulk_edit/handler.cppm:12-28`, `bulk_edit/workflow/step/execute.cppm:11-20`.

Ambiguity — wrapper/override shape and the unreachable mutation. Short snippet for checks 3/4/5:

```cpp
// bulk_edit/workflow/step/execute.cppm — add `import core_serde;`
const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
std::vector<blog::api::RecordDto> settings;
if (doc) settings = blog::api::parse_array(
    core_serde::JsonCodec{}.encode(serde::Value{doc->at("settings")}));
if (settings.empty()) {
    return core::workflow::StepResponse<Output>{
        blog::api::ApiError{}.validation("Validation error", "settings required", "settings")};
}
auto& rs = ctx.store().resources();
std::string meta = "{}";
for (const auto& rec : settings) {
    const auto k = rec.find("key"), v = rec.find("value");
    if (k == rec.end() || v == rec.end()) continue;
    const auto key = core_serde::JsonCodec{}.decode<std::string>(k->second).value_or(k->second);
    rs.create("settings", R"({"key":)" + core_serde::JsonCodec{}.encode(key)
              + R"(,"value":)" + v->second + "})");
}
return core::workflow::StepResponse<Output>{"{\"settings\":"
    + ctx.store().json_for("settings", {}) + ",\"meta\":" + meta + "}"};
```

### POST `/settings/access_code/regenerate` — MISMATCH

- Route identity agrees: Ghost `routes.js:112-116` ↔ C++ `routes.cppm:37-50`.
- Operation materially differs: Ghost calls `settingsService.regeneratePrivateSiteAccessCode()` (`settings.js:177`) and returns all settings (`settings.js:181`); C++ performs `mutations.create("settings", body)` — a generic insert that does not regenerate any access code (`settings_access_code_regenerate_post/.../execute.cppm:16`).
- Output differs: Ghost returns browse(all settings); C++ returns the raw create body at 200 (`handler.cppm:25-27`).
- Cache differs: Ghost explicitly sets `X-Cache-Invalidate:'/*'` (`settings.js:178`); C++ none.
- Auth differs: Ghost admin auth + edit permission; C++ content-type + dead token check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; the regenerated code stays inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `routes.cppm:37-50` `Route{"/regenerate"}.post(...)` under `/access_code`; Ghost `routes.js:112-116`. Verbatim path/verb match.
2. Operation mapping — no change required. `settings_access_code_regenerate_post/workflow/flow/main.cppm:9` registers `ExecuteStep` as `"settings_settings_access_code_regenerate_post"`; `settings_access_code_regenerate_post/handler.cppm:23` runs it; Ghost key `regenerateAccessCode` (`settings.js:169,176`).
3. Input — no change required, with one removal. Ghost declares no `options`/`data` (`settings.js:169-175`); C++ `using Input=IRequest&` (`settings_access_code_regenerate_post/workflow/step/execute.cppm:11`) — drop the body read at `:16` since the operation takes no payload.
4. Output — REPLACE. `settings_access_code_regenerate_post/workflow/step/execute.cppm:16`: replace `mutations.create("settings", body).body()` with the browse-all envelope `"{\"settings\":" + ctx.store().json_for("settings", {}) + "}"` (Ghost `settings.js:181`). Keep `Status::OK` at `settings_access_code_regenerate_post/handler.cppm:27` — no change required there.
5. DB call — REPLACE. `settings_access_code_regenerate_post/workflow/step/execute.cppm:16`: instead of the generic create, write a fresh `private_site_access_code` override via `ctx.store().resources().create("settings", R"({"key":"private_site_access_code","value":)" + core_serde::JsonCodec{}.encode(code) + "})");` — `ApiDataStore::resources()` at `context.cppm:1101-1104`, `ResourceStore::create` at `:640-651`, codec at `context.cppm:1132`. `code` is generated locally with std-only `<random>` (`std::random_device`/`std::mt19937`), not a third-party lib and not a global — Ghost `settingsService.regeneratePrivateSiteAccessCode()` (`settings.js:177`). `ApiDataStore::settings_json()` merges the override (GET `/settings` fix, check 5), so the browse output of check 4 reflects it. No SQL, no new global state.
6. Cache — REPLACE. Ghost `frame.setHeader('X-Cache-Invalidate','/*')` (`settings.js:178`). Add in `settings_access_code_regenerate_post/handler.cppm` before `send()` (`:26-27`): `res.set_header("X-Cache-Invalidate","/*");` (`include/interfaces/io/response.cppm:589-590`). No new namespace/TTL.
7. Auth — REPLACE. Delete the dead inline check (`settings_access_code_regenerate_post/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` on the POST route (`routes.cppm:41-47`) plus a `require_permission` helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"edit"` (Ghost `permissions:{method:'edit'}`, `settings.js:173-175`; `mw.authAdminApi`, `routes.js:114`), wired ahead of `handler.settings_settings_access_code_regenerate_post().handle(...)`.
8. Verdict + evidence — after 4,5,6,7 → `MATCH`. Recheck `routes.js:112-116`, `settings.js:169-183` vs `routes.cppm:37-50`, `settings_access_code_regenerate_post/handler.cppm:12-28`, `settings_access_code_regenerate_post/workflow/step/execute.cppm:11-17`.

Ambiguity — generating a code with std only and merging it into browse. Short snippet for checks 4/5:

```cpp
// settings_access_code_regenerate_post/workflow/step/execute.cppm — add `import core_serde;`
#include <random>
std::random_device rd;
std::mt19937_64 gen{rd()};
auto code = std::format("{:016x}", gen());
ctx.store().resources().create("settings",
    R"({"key":"private_site_access_code","value":)" + core_serde::JsonCodec{}.encode(code) + "})");
return core::workflow::StepResponse<Output>{"{\"settings\":"
    + ctx.store().json_for("settings", {}) + "}"};
```

### PUT `/settings/verifications/` — MISMATCH

- Route identity: Ghost `/settings/verifications/` with trailing slash (`routes.js:117`) ↔ C++ `/settings/verifications` without (`routes.cppm:88`) — normalization equal.
- Operation materially differs: Ghost verifies a key-update token via `settingsBREADService.verifyKeyUpdate(frame.data.token)` — a verification, not a CRUD write (`settings.js:73`) — then returns browse(all) (`settings.js:75-78`). C++ performs a persisting `mutations.edit("settings","",body)` with no token parsing (`settings_verifications_put/.../execute.cppm:16`).
- Input differs: Ghost `data:['token']` (`settings.js:71`); C++ raw body.
- Output differs: Ghost returns all settings; C++ returns the generic edit body at 200 (`handler.cppm:25-27`).
- Cache differs: Ghost `cacheInvalidate:true` (`settings.js:66`); C++ none.
- Auth differs: Ghost admin auth + edit permission; C++ content-type + dead token check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; the verification token stays inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. Ghost `routes.js:117` `PUT /settings/verifications/` ↔ C++ `routes.cppm:87-97` `PUT /settings/verifications`; trailing slash normalized. Verbatim path/verb match after normalization.
2. Operation mapping — no change required. `settings_verifications_put/workflow/flow/main.cppm:9` registers `ExecuteStep` as `"settings_settings_verifications_put"`; `settings_verifications_put/handler.cppm:23` runs it; Ghost key `verifyKeyUpdate` (`settings.js:64,72`).
3. Input — REPLACE. Symbol `blog::handlers::settings::settings_verifications_put::workflow::ExecuteStep::invoke` (`settings_verifications_put/workflow/step/execute.cppm:13-17`): add `import core_serde;`, decode `in.body_as_string()` and extract the `token` field (Ghost `data:['token']`, `settings.js:71`, passed at `:73`); on missing/empty token return a `__BAD_REQUEST__` sentinel mapped by the handler to `Status::BAD_REQUEST` (`include/interfaces/io/types.cppm:37`), or use `blog::api::ApiError{}.validation(...)` (`context.cppm:76-84`). `using Input=IRequest&` stays (`execute.cppm:11`).
4. Output — REPLACE. `settings_verifications_put/workflow/step/execute.cppm:16`: replace `mutations.edit("settings","",body).body()` with the browse-all envelope `"{\"settings\":" + ctx.store().json_for("settings", {}) + "}"` (Ghost `settings.js:75-78`). Keep `Status::OK` at `settings_verifications_put/handler.cppm:27`, except the check-3 sentinel → `BAD_REQUEST`.
5. DB call — REPLACE (verify, do not persist). `settings_verifications_put/workflow/step/execute.cppm:16`: drop `mutations.edit("settings","",body)` entirely. Exact replacement: `auto& rs = ctx.store().resources();` (`context.cppm:1101-1104`); compare the decoded `token` against stored records from `blog::api::parse_array(rs.list("settings_verifications"))` (`context.cppm:595,704-715`) — decode the `value`/`token` field with `core_serde::JsonCodec{}.decode<std::string>`; on success optionally `rs.destroy("settings_verifications", id)` (`ResourceStore::destroy`, `:665-672`) to mirror one-shot verification. Ghost `settingsBREADService.verifyKeyUpdate(token)` (`settings.js:73`) is a verification, not a CRUD write. No SQL, no globals, no new namespace.
6. Cache — REPLACE. Ghost `cacheInvalidate:true` (`settings.js:66`) → `X-Cache-Invalidate` header. Add in `settings_verifications_put/handler.cppm` before `send()` (`:26-27`): `res.set_header("X-Cache-Invalidate","/*");` (`include/interfaces/io/response.cppm:589-590`). No new key/namespace/TTL.
7. Auth — REPLACE. Delete the dead inline check (`settings_verifications_put/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` on the PUT route (`routes.cppm:89-95`) plus a `require_permission` helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"edit"` (Ghost `permissions:{method:'edit'}`, `settings.js:68-70`; `mw.authAdminApi`, `routes.js:117`), wired ahead of `handler.settings_settings_verifications_put().handle(...)`.
8. Verdict + evidence — after 3,4,5,6,7 → `MATCH`. Recheck `routes.js:117`, `settings.js:64-80` vs `routes.cppm:87-97`, `settings_verifications_put/handler.cppm:12-28`, `settings_verifications_put/workflow/step/execute.cppm:11-17`.

Ambiguity — token extraction and one-shot comparison. Short snippet for checks 3/5:

```cpp
// settings_verifications_put/workflow/step/execute.cppm — add `import core_serde;`
const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
if (!doc || !doc->contains("token"))
    return core::workflow::StepResponse<Output>{"__BAD_REQUEST__"};
const auto token = core_serde::JsonCodec{}.encode(serde::Value{doc->at("token")});
auto& rs = ctx.store().resources();
bool ok = false;
for (const auto& rec : blog::api::parse_array(rs.list("settings_verifications"))) {
    const auto it = rec.find("token");
    if (it != rec.end() && it->second == token) { ok = true; break; }
}
if (!ok) return core::workflow::StepResponse<Output>{"__BAD_REQUEST__"};
return core::workflow::StepResponse<Output>{"{\"settings\":"
    + ctx.store().json_for("settings", {}) + "}"};
```

### DELETE `/settings/stripe/connect` — MISMATCH

- Route identity agrees: Ghost `routes.js:118-122` ↔ C++ `routes.cppm:73-86`.
- Operation materially differs: Ghost checks paid members, calls `stripeService.disconnect()`, then nulls seven Stripe settings keys via `models.Settings.edit` (`settings.js:91-135`); C++ bulk-destroys `"settings"` records (`settings_stripe_connect_delete/.../execute.cppm:16`) — no member check, no Stripe disconnect, no key nulling.
- Output differs: Ghost HTTP **204** with no body (`settings.js:83`); C++ HTTP 200 with the raw destroy body (`handler.cppm:25-27`).
- Cache agrees (none): `cacheInvalidate:false` (`settings.js:85`) vs C++ none.
- Auth differs: Ghost admin auth + edit permission; C++ content-type + dead token check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. No third-party libs, direct SQL, Ghost JS, or new global state; the nulled settings stay inside `blog::api::ResourceStore` under its existing `resources` namespace/tag.

1. Route identity — no change required. `routes.cppm:73-86` `Route{"/connect"}.delt(...)` under `/stripe`; Ghost `routes.js:118-122`. Verbatim path/verb match.
2. Operation mapping — no change required. `settings_stripe_connect_delete/workflow/flow/main.cppm:9` registers `ExecuteStep` as `"settings_settings_stripe_connect_delete"`; `settings_stripe_connect_delete/handler.cppm:23` runs it; Ghost key `disconnectStripeConnectIntegration` (`settings.js:82,90`).
3. Input — no change required. Ghost declares no `options`/`data` (`settings.js:82-89`); C++ `using Input=IRequest&` (`settings_stripe_connect_delete/workflow/step/execute.cppm:11`) reads an unused body at `:16` — drop that read. Both: no params.
4. Output — REPLACE. `settings_stripe_connect_delete/handler.cppm:25-27`: set `res.set_status(interfaces::io::types::Status::NO_CONTENT);` (`include/interfaces/io/types.cppm:23`) and skip `res.set_body(...)` entirely — send empty bytes (Ghost `statusCode:204`, no body, `settings.js:83`). For the check-5 paid-member error, set `Status::BAD_REQUEST` with a Ghost error envelope via `blog::api::ApiError{}.json(...)` (`context.cppm:66-73`) or `write_error` (`middleware/error.cppm`).
5. DB call — REPLACE. `settings_stripe_connect_delete/workflow/step/execute.cppm:16`: drop `mutations.bulk_destroy("settings", body)`. Exact replacement: (a) paid-member check — `const auto members = ctx.store().json_for("members", {{"filter","status:paid"}});` (`context.cppm:1128,1136`, `query_json` helper), parse matching paid records; if any exist return the check-4 error sentinel (Ghost `membersService.api.memberBREADService.browse({limit:0,filter:'status:paid'})`, `settings.js:91-99`). (b) null the seven keys (`stripe_connect_publishable_key`, `stripe_connect_secret_key`, `stripe_connect_livemode`, `stripe_connect_display_name`, `stripe_connect_account_id`, `members_stripe_webhook_id`, `members_stripe_webhook_secret`; `settings.js:103-133`) via `ctx.store().resources().create("settings", R"({"key":...,"value":null})")` per key (`context.cppm:1101-1104,640-651`), which `ApiDataStore::settings_json()` merges (GET `/settings` fix, check 5). Ghost `stripeService.disconnect()` (`settings.js:101`) has no allowed C++ primitive — record as an accepted residual gap (no third-party lib); the observable DB effect (seven nulled keys) is covered. No SQL, no globals.
6. Cache — no change required. Ghost `headers.cacheInvalidate:false` (`settings.js:85`); C++ handler ctor (`settings_stripe_connect_delete/handler.cppm:12-16`) binds `ctx`/workflow only and `all.cppm:836-839` wires cache only to members/emails/automations. Both: none.
7. Auth — REPLACE. Delete the dead inline check (`settings_stripe_connect_delete/handler.cppm:18-22`); add `.use(blog::handlers::middleware::require_auth)` on the DELETE route (`routes.cppm:77-83`) plus a `require_permission` helper in allowed module `blog_handlers_middleware` keyed `docName="settings"`, action `"edit"` (Ghost `permissions:{method:'edit'}`, `settings.js:87-89`; `mw.authAdminApi`, `routes.js:120`), wired ahead of `handler.settings_settings_stripe_connect_delete().handle(...)`.
8. Verdict + evidence — after 4,5,7 → `MATCH`. Recheck `routes.js:118-122`, `settings.js:82-137` vs `routes.cppm:73-86`, `settings_stripe_connect_delete/handler.cppm:12-28`, `settings_stripe_connect_delete/workflow/step/execute.cppm:11-17`.

Ambiguity — paid-member detection + 204 with empty body. Short snippet for checks 4/5:

```cpp
// settings_stripe_connect_delete/workflow/step/execute.cppm — `import core_serde;`
const auto members = ctx.store().json_for("members", {{"filter", "status:paid"}});
if (!blog::api::parse_array(members).empty())
    return core::workflow::StepResponse<Output>{"__BAD_REQUEST__"};
auto& rs = ctx.store().resources();
for (std::string_view key : {"stripe_connect_publishable_key", "stripe_connect_secret_key",
         "stripe_connect_livemode", "stripe_connect_display_name", "stripe_connect_account_id",
         "members_stripe_webhook_id", "members_stripe_webhook_secret"}) {
    rs.create("settings", R"({"key":)" + core_serde::JsonCodec{}.encode(key) + R"(,"value":null})");
}
return core::workflow::StepResponse<Output>{"{}"};
```

```cpp
// settings_stripe_connect_delete/handler.cppm:25-27
if (body == "__BAD_REQUEST__") {
    res.set_status(interfaces::io::types::Status::BAD_REQUEST);
    res.set_body(std::vector<std::byte>{});
} else {
    res.set_status(interfaces::io::types::Status::NO_CONTENT);
    res.set_body(std::vector<std::byte>{});  // Ghost statusCode 204, no body
}
```

## Missing in C++

None. All seven Ghost admin settings routes have C++ route/handler counterparts:

- `GET /settings/routes/yaml` → `include/blog/handlers/system/settings/routes.cppm:51-62`, `settings_routes_yaml_get/handler.cppm:10`.
- `POST /settings/routes/yaml` → `include/blog/handlers/system/settings/routes.cppm:63-70`, `settings_routes_yaml_post/handler.cppm:10`.
- `GET /settings` → `include/blog/handlers/system/settings/routes.cppm:21-28`, `browse/handler.cppm:10`.
- `PUT /settings` → `include/blog/handlers/system/settings/routes.cppm:29-36`, `bulk_edit/handler.cppm:10`.
- `POST /settings/access_code/regenerate` → `include/blog/handlers/system/settings/routes.cppm:37-50`, `settings_access_code_regenerate_post/handler.cppm:10`.
- `PUT /settings/verifications/` → `include/blog/handlers/system/settings/routes.cppm:87-97`, `settings_verifications_put/handler.cppm:10`.
- `DELETE /settings/stripe/connect` → `include/blog/handlers/system/settings/routes.cppm:73-86`, `settings_stripe_connect_delete/handler.cppm:10`.

Confirmed by searching all `routes.cppm` files for `settings`: matches are confined to `include/blog/handlers/system/settings/routes.cppm` and the central install references in `include/blog/handlers/routes.cppm:43,116` (plus the separate `custom_theme_settings` domain, which is a different controller). Note: the Ghost controller also declares a `read` key (`settings.js:42-62`), but `admin/routes.js:100-122` exposes no `/settings/:key` route for it, so it produces no row here.

  Fix: No change required. Check 1 is closed — no Ghost settings route lacks a C++ counterpart, so there is nothing to add; `MISSING` count stays 0. The seven `#### Fix` blocks above reshape the existing handlers/workflow steps/`SettingsRoutes::install` to reach parity for checks 2–8; they do not add routes. The Ghost controller's `read` key (`settings.js:42-62`) has no route in `admin/routes.js:100-122`, so it correctly produces no row.

## Extra in C++

None. The matched folder `include/blog/handlers/system/settings/routes.cppm` installs exactly seven routes (GET/PUT `/settings`, POST `/settings/access_code/regenerate`, GET/POST `/settings/routes/yaml`, DELETE `/settings/stripe/connect`, PUT `/settings/verifications`), each with a Ghost counterpart. No eighth C++ route exists in that folder.

  Fix: No change required. Check 1 is closed — no C++ route in the matched folder lacks a Ghost counterpart, so nothing to remove or gate; `EXTRA` count stays 0. Keep the seven installed routes; the `#### Fix` blocks above change middleware/input/output/DB behavior only, not the path or verb.

## Source fixes applied (manifest group 50)

Applied in `include/blog/handlers/system/settings/` only (no shared files touched, no new files/partitions; every step keeps `Output=std::string` + `static s_ctx`):

- `settings_routes_yaml_get`: step reads `parse_array(resources().list("routes_yaml"))` and returns plain YAML text (stub when empty); handler sets `Content-Type: text/plain` + `Content-Disposition: attachment; filename="routes.yaml"`; dead `&& !true` token check removed.
- `settings_routes_yaml_post`: multipart `name="routes"` part extracted from the `Content-Type` boundary (in-memory bytes stand in for `fs.readFile`); replace-all write to `resources().create("routes_yaml", ...)`; `__BAD_REQUEST__` sentinel → 400; handler sets `X-Cache-Invalidate: /*`; dead token check removed.
- `browse`: query string parsed into `std::map<std::string,std::string>` and passed to `json_for("settings", params)`; hardcoded `meta.pagination` removed, output `{"settings":...}`; dead token check removed.
- `bulk_edit`: `settings` wrapper decoded and written as per-key `resources().create("settings", ...)` overrides; unreachable early return removed; `__BAD_REQUEST__` → 400; `X-Cache-Invalidate: /*` on non-empty result; dead token check removed.
- `settings_access_code_regenerate_post`: std-only `<random>` 16-hex-char code stored as `private_site_access_code` override; browse-all envelope returned; `X-Cache-Invalidate: /*`; dead token check removed.
- `settings_verifications_put`: `token` extracted from JSON body, compared against `resources().list("settings_verifications")` with one-shot `destroy` on match; browse-all envelope returned; `__BAD_REQUEST__` → 400; `X-Cache-Invalidate: /*`; dead token check removed.
- `settings_stripe_connect_delete`: paid-member check via `json_for("members", {{"filter","status:paid"}})`; seven Stripe keys nulled via `resources().create("settings", ...)`; 204 empty body on success / 400 on paid-member error; dead token check removed.
- `routes.cppm`: router-level `.use(require_auth)` added for all seven admin routes; local `settings_content_type` wrapper skips the JSON-only `content_type_middleware` for `/settings/routes/yaml` (plain download + multipart upload), since parent-node middleware runs on every child route.

### Central registrations needed (shared files — NOT edited here)

1. `include/blog/handlers/context/context_context.cppm`: `ApiDataStore::settings_json()` (~line 1669) must take `const std::map<std::string,std::string>& params = {}`, filter fixture keys by `params["group"]` via a static key→group table, and merge stored overrides from `parse_array(m_resources.list("settings"))`. `json_for("settings", params)` (line 1148) must call `settings_json(params)` instead of `settings_json()`. Without this the `group` filter and all `resources().create("settings", ...)` write-throughs (bulk_edit / regenerate / Stripe nulling) are not observable in the browse response.
2. `include/blog/handlers/middleware/`: add a `require_permission(docName, action)` helper to module `blog_handlers_middleware` and wire it on the settings routes — browse/download keyed `settings`/`browse`, edit routes `settings`/`edit`, PUT `/settings` also enforcing Ghost's `unsafeAttrsObject` rule for the `labs` key. No permission primitive exists today (`require_permission|docName` absent from `include/blog/handlers/**/*.cppm`).
3. `include/blog/handlers/context/context_context.cppm`: `public_settings_json()` (see `settings-public.md`) requires a route/content-negotiation decision because `GET /settings` is shared with the public controller.
4. Accepted residual: `stripeService.disconnect()` has no allowed C++ primitive; only its observable DB effect (seven nulled keys) is implemented.

## Group 56 — strict-parity rewrite (2026-11-20)

Supersedes the group-50 stand-in state above. All source edits are confined to
`include/blog/handlers/system/settings/`; no shared file, `all.cppm`, base module,
central `routes.cppm` or BUILD file was edited. No verdict was flipped: every
check is now closed at the C++ abstraction boundary, but MATCH still requires the
centrally owned primitives under **Needs central registration** below.

### What changed (check-by-check, all seven routes)

| Check | Closure in this pass |
|---|---|
| 1 route identity | Verbatim paths/verbs kept. Router-level `content_type_middleware` removed (Ghost uses JSON/urlencoded body parsers, not a Content-Type/Accept gate) and the local `settings_content_type` wrapper deleted; `require_auth` remains the 401 gate. No route added/removed. |
| 2 operation mapping | Handler/workflow names unchanged; steps now share a folder-local `blog::handlers::settings::support` namespace living in the already-registered `:settings_browse_step` partition — no new partition. |
| 3 input | `core::query::parse` for `?group=` (browse) and NQL `filter=status:paid&limit=all` (Stripe delete). PUT `/settings` decodes the `settings` wrapper, applies the ~96-key Ghost `EDITABLE_SETTINGS` allowlist, boolean coercion, labs `WRITABLE_KEYS_ALLOWLIST` filtering, secret/stripe-token filtering, and the array/email/announcement validators. Multipart upload extracts `name="routes"` + filename with extension/MIME checks and Ghost's `routes.missingFile` / `routes.invalidFile` messages. |
| 4 output | All steps now return `Output = std::map<std::string,std::string>` (`status`/`body`/`cache_invalidate`/`content_type`/`content_disposition`); handlers write statuses from the map. `__BAD_REQUEST__` sentinels are gone — every error is a Ghost `@tryghost/errors`-shaped envelope (`ValidationError`, `BadRequestError`, `NotFoundError`, `NoPermissionError`, `UnsupportedMediaTypeError`, `IncorrectUsageError`) with the exact Ghost message. Browse applies group filter, secret obfuscation (`••••••••`), icon `w256h256` resize, labs override and `meta.filters.group`; mapper emits only `key`/`value`/`is_read_only`. GET yaml keeps `text/plain` + `Content-Disposition`; DELETE Stripe keeps 204 with empty body. |
| 5 DB call | All writes go through `support::write_setting()` — dedup `ResourceStore` edit (destroy matching key, then create), so edits are observable in every later browse. Regenerate writes Ghost's key `password` with a `word###` value (48-word list), not `private_site_access_code`/16-hex. Verifications writes the verified `{key,value}` after the `EMAIL_KEYS` guard. Stripe delete checks paid members with `core::query` then nulls the seven keys. `routes_yaml` is replace-all. |
| 6 cache | `X-Cache-Invalidate: /*` emitted only where Ghost does: yaml upload, PUT edit (non-empty result), regenerate, verifications. GET routes and Stripe delete emit none (`cacheInvalidate:false`). A `cache.invalidate` outbox event accompanies each. |
| 7 auth | Every handler calls `support::authorize()` → registered `blog_handlers_middleware::require_ghost_permission` (`core_auth:ghost`), producing Ghost 401 `UnauthorizedError` / 403 `NoPermissionError`. Per-op decisions: browse+download `settings:browse`; edit/upload/verifications/regenerate/stripe `settings:edit`. PUT labs unsafeAttrsObject collapses to the same `settings:edit` gate (Editor has `settings:browse` only → denied). |
| 8 side effects | All observable side effects are emitted through the registered `blog_handlers:outbox_shim` and drained: `settings.edited`, `stripe.connected`, `automations.beta_flag_enabled`, `email.send_verification`, `routes.yaml.uploaded`, `routes.reloaded`, `email.verified`, `settings.access_code_regenerated`, `stripe.disconnected`, `cache.invalidate`. |

### Files changed (folder only)

`routes.cppm`; `browse/{handler.cppm,workflow/step/execute.cppm}`;
`bulk_edit/{handler.cppm,workflow/step/execute.cppm}`;
`settings_access_code_regenerate_post/{handler.cppm,workflow/step/execute.cppm}`;
`settings_routes_yaml_get/{handler.cppm,workflow/step/execute.cppm}`;
`settings_routes_yaml_post/{handler.cppm,workflow/step/execute.cppm}`;
`settings_verifications_put/{handler.cppm,workflow/step/execute.cppm}`;
`settings_stripe_connect_delete/{handler.cppm,workflow/step/execute.cppm}`.
Flow `main.cppm` files unchanged. Every step keeps `static s_ctx` and a
`std::string`/`std::map` `Output`.

### Needs central registration (recorded, not performed)

1. `blog_handlers:mapping` (`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) is on disk but not in `include/blog/handlers/BUILD.bazel` partitions nor re-exported by `base.cppm`. This pass uses the folder-local serializer in `:settings_browse_step`; main agent may wire the canonical mapper.
2. `core_outbox` is a separate Bazel target not in `//include/blog/handlers` deps; this pass uses the registered `blog_handlers:outbox_shim` clone (identical API). No BUILD/cycle fix attempted.
3. `ApiDataStore::settings_json()` in `context/context_context.cppm` still returns the old 21-key fixture and ignores params; the strict fixture/override merge now lives in the settings partition. Central change is optional once main agent owns it.
4. `core_adapters::StripeAdapter` is not a registered dep; `stripeService.disconnect()` / Stripe Connect token fetch are represented by outbox events, not live HTTP. No new partition was created.
5. A real YAML/schema parser (`parseYaml` + `parseRouteSettings`), a signed single-use magic-link token codec, a labs/config/limits service, and a public `authenticatePublic` resolver have no allowed primitive here; local stand-ins cover the observable contract.
6. The monolithic `blog_handlers` Bazel target currently reports `cycle in dependency graph` from concurrent cross-partition edits in other groups; no shared file or BUILD change was made to resolve it. Verification of this folder was done by compiling all 21 settings step/handler/flow partitions directly with the cached dependency BMIs (dependency order: support step → other steps → handlers → flows) from a temp CWD that exposes the Bazel external tree; all 21 compile cleanly. `routes.cppm` belongs to the `blog_handlers_routes` target and was source-reviewed only. The cycle fix stays with the main agent.

### Verdict

Counts stay 7 PARTIAL / 0 MATCH. All divergences named in the seven `**Diff**`
blocks of `docs/parity/behavior/settings.md` are closed at the C++ abstraction
boundary; the remaining items are the infrastructure residuals above, which are
not endpoint-level divergences but are not owned by this folder either.
