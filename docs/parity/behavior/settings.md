# Behavior report: settings

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/settings.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:100-122`
- C++ folders: `include/blog/handlers/system/settings/` (7 route/host folders + `routes.cppm`)
- Analyzed: 2026-11-19

Router-level mount: Admin API app is mounted at `/ghost/api/admin/` (`docs/Ghost/ghost/core/server/web/api/app.js:21-22`; `docs/Ghost/ghost/core/server/web/api/endpoints/admin/app.js:17-44`). Admin app applies `shared.middleware.cacheControl('private')` (`:24`), JSON/urlencoded body parsers 50mb (`:33-34`), `parseQueryBooleans` (`:37`), `checkVersionMatch` (`:41`), then `routes()` (`:44`). C++ `SettingsRoutes` mounts under `/settings` with router-wide `settings_content_type` + `require_auth` (`include/blog/handlers/system/settings/routes.cppm:39-41`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 7 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Every C++ route matches Ghost on path, verb, router-wide bearer auth and (where applicable) status/header framing, but every one replaces the Ghost service/model/cache chain with a stand-in workflow over a context-local resource store or a hardcoded fixture. No route is absent and no route is extra. The controller's `read` op has no route binding on either side (see Module-level notes).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/settings/routes/yaml` | `download` | none (query/body ignored) | `mw.authAdminApi`; `permissions:{method:'browse'}` | 200; `Content-Disposition: attachment; filename="routes.yaml"`; plain; `cacheInvalidate:false` | `routeSettings.api.download()` → `DynamicRoutingService.download()` → `store.get().yamlSource` | no X-Cache-Invalidate; route-settings adapter store | `serializeData` → raw YAML string body | `settings_routes_yaml_get/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, sets `text/plain` + `Content-Disposition` identical; body from `routes_yaml` resource or hardcoded default | PARTIAL | Ghost `settings.js:198-215`, `routes.js:101`; C++ `routes.cppm:73-83`, `get/handler.cppm:17-24`, `get/execute.cppm:13-23` |
| POST `/settings/routes/yaml` | `upload` | multipart field `routes`; `apiMw.upload.single('routes')` + `validation({type:'routes'})` | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `cacheInvalidate:true` | `fs.readFile(frame.file.path,'utf8')` → `routeSettings.api.upload(content)` → `parseYaml` → `parseRouteSettings` → `store.replace` → `bridge.reloadFrontend` | X-Cache-Invalidate `/*`; route-settings adapter store replaced | `serializeData` → raw data (undefined) | `settings_routes_yaml_post/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, `X-Cache-Invalidate: /*`; manual multipart extraction of `name="routes"`, destroy+create `routes_yaml` resource; no YAML parse/validation/router reload | PARTIAL | Ghost `settings.js:185-196`, `routes.js:102-108`; C++ `routes.cppm:84-91`, `post/handler.cppm:17-26`, `post/execute.cppm:13-43` |
| GET `/settings` | `browse` | `options:['group']` (`?group=x,y`); `context` from frame | `mw.authAdminApi`; `permissions:true` (`docName:'settings'`) | 200; `headers.cacheInvalidate:false` | `settingsBREADService.browse(context)` → `SettingsCache.getAll()` → `_formatBrowse` (omit core, obfuscate secrets, labs override, read-only flags) | no X-Cache-Invalidate; in-memory `SettingsCache` | `serializeSettings` → `{settings:[{key,value,is_read_only}],meta:{filters:{group}}}`; icon resized | `settings/browse/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, no X-Cache-Invalidate; parses `group`/`key` query but `json_for("settings",params)` ignores params → hardcoded 21-setting fixture; body `{"settings":[...]}` no `meta` | PARTIAL | Ghost `settings.js:31-40`; C++ `routes.cppm:42-49`, `browse/handler.cppm:17-23`, `browse/execute.cppm:12-25`, `context_context.cppm:1161,1682-1716` |
| PUT `/settings` | `edit` | body `data.settings[0..n]` (`key`,`value`); `data[]:['token']` not used; input serializer allowlist + labs allowlist + validator | `mw.authAdminApi`; `permissions.unsafeAttrsObject = labs setting` | 200; `headers.cacheInvalidate:false`; explicit `X-Cache-Invalidate: /*` on non-empty result | `getStripeConnectData` → `settingsBREADService.edit(...)` → `SettingsModel.edit` → `_formatBrowse`; then `settingsBREADService.browse` for full return | X-Cache-Invalidate `/*` set in controller; SettingsCache refresh via model events | `serializeSettings` → full settings + `meta` from result | `settings/bulk_edit/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, `X-Cache-Invalidate: /*` when body not `{}`; creates new `settings` records per key/value; returns hardcoded fixture + `meta:{}` | PARTIAL | Ghost `settings.js:139-167`; C++ `routes.cppm:50-57`, `bulk_edit/handler.cppm:17-27`, `bulk_edit/execute.cppm:13-32` |
| POST `/settings/access_code/regenerate` | `regenerateAccessCode` | none | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `headers.cacheInvalidate:false`; explicit `X-Cache-Invalidate: /*` | `settingsService.regeneratePrivateSiteAccessCode()` → `models.Settings.edit([{key:'password',value:word###}],{context:{internal:true}})` → browse | X-Cache-Invalidate `/*`; SettingsCache refresh | `serializeSettings` → full settings | `settings/settings_access_code_regenerate_post/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, `X-Cache-Invalidate: /*`; writes random 16-hex code to key `private_site_access_code`; returns hardcoded fixture | PARTIAL | Ghost `settings.js:169-183`, `settings-service.js:177-187`, `private-site-access-code.js:59-63`; C++ `routes.cppm:58-71`, `regen/handler.cppm:17-24`, `regen/execute.cppm:13-21` |
| PUT `/settings/verifications/` | `verifyKeyUpdate` | `data:['token']` (magic-link token) | `mw.authAdminApi`; `permissions:{method:'edit'}` | 200; `headers.cacheInvalidate:true` | `settingsBREADService.verifyKeyUpdate(token)` → `magicLinkService.getDataFromToken` → `EMAIL_KEYS` guard → `SettingsModel.edit({key,value})` → browse | X-Cache-Invalidate `/*`; SettingsCache refresh | `serializeSettings` → full settings | `settings/settings_verifications_put/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200, `X-Cache-Invalidate: /*`; parses `{token}`, looks up `settings_verifications` resource, destroys matched record; 400 sentinel if missing; no settings write | PARTIAL | Ghost `settings.js:64-80`, `settings-bread-service.js:313-328`; C++ `routes.cppm:108-117`, `verifications/handler.cppm:17-27`, `verifications/execute.cppm:13-41` |
| DELETE `/settings/stripe/connect` | `disconnectStripeConnectIntegration` | none | `mw.authAdminApi`; `permissions:{method:'edit'}` | 204; `headers.cacheInvalidate:false` | `membersService.api.memberBREADService.browse({limit:0,filter:'status:paid'})` → 400 if total≠0 → `stripeService.disconnect()` → `models.Settings.edit([7 keys→null],frame.options)` | no X-Cache-Invalidate; SettingsCache refresh | no body (204) | `settings/settings_stripe_connect_delete/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 204, no X-Cache-Invalidate; checks `members` filter `status:paid` → 400 sentinel if any; nulls same 7 keys via `rs.create`; no `stripeService.disconnect` | PARTIAL | Ghost `settings.js:82-137`; C++ `routes.cppm:94-107`, `stripe/handler.cppm:17-27`, `stripe/execute.cppm:13-25` |

## Route details

### GET /settings/routes/yaml — PARTIAL

**Ghost**
1. Identity: `router.get('/settings/routes/yaml', mw.authAdminApi, http(api.settings.download))` (`admin/routes.js:101`). `mw.authAdminApi` = authenticate + authorize + updateUserLastSeen + cors + SSL/host redirect + prettyUrls + tokenPermissionCheck (`admin/middleware.js:110-118`).
2. Operation: `settings.download`, `docName:'settings'` (`settings.js:29,198-215`).
3. Input: no options/data/validation; reads nothing from request.
4. Auth: `mw.authAdminApi`; `permissions:{method:'browse'}` (`settings.js:209-211`). No labs flags.
5. Status/headers: 200; `headers.disposition={type:'yaml',value:'routes.yaml'}` → `Content-Disposition: attachment; filename="routes.yaml"`; `response.format:'plain'`; `cacheInvalidate:false` (no X-Cache-Invalidate) (`settings.js:199-208`).
6. Call chain: `query()` → `routeSettings.api.download()` (`settings.js:213`) → bound getter to `DynamicRoutingService.download` (`route-settings/index.js:30-32`) → `this.store.get()` then returns `settings.yamlSource` (`route-settings/dynamic-routing-service.js:77-81`). No events/mail/webhooks.
7. Cache: route-settings adapter store read (`configure({store:adapterManager.getAdapter('route-settings')})`, `route-settings/index.js:8-12`); no HTTP cache header emitted by controller.
8. Output: serializer `download: serializeData` (`utils/serializers/output/settings.js:100,89-91`) sets `frame.response = data` → raw YAML string body; no envelope, no pagination. Missing/invalid stored YAML surfaces the genuine error (`dynamic-routing-service.js:52-75`).

**C++**
1. Identity: `Router{router,"/settings"} … Router{router,"/routes"} Route{"/yaml"}.get(...)` (`settings/routes.cppm:73-83`); router-wide `settings_content_type` + `require_auth` (`:39-41`).
2. Operation: `handler.settings_settings_routes_yaml_get().handle` (`routes.cppm:81`); workflow `settings_settings_routes_yaml_get`, single `ExecuteStep` (`workflow/flow/main.cppm:8-10`).
3. Input: `using Input = interfaces::io::IRequest&`; step ignores request body/query (`get/execute.cppm:10-14`).
4. Auth: `require_auth` → empty bearer ⇒ 401 `{"errors":[{message:"Unauthorized",type:"UnauthorizedError"}]}` (`middleware/auth.cppm:12-29`; `middleware/error.cppm`). `settings_content_type` skips JSON gate for paths starting `/settings/routes/yaml` (`settings/routes.cppm:19-32`), so no Content-Type/Accept requirement. No permission/docName check.
5. Status/headers: `CONTENT_TYPE="text/plain"`, `CONTENT_DISPOSITION='attachment; filename="routes.yaml"'`, status OK (`get/handler.cppm:22-24`). No X-Cache-Invalidate (matches `cacheInvalidate:false`).
6. Call chain: `ctx.store().resources().list("routes_yaml")` (`get/execute.cppm:15`); takes last record's `content` (`:17-21`); falls back to hardcoded `"# See https://docs.ghost.org/themes/routing/#routes\nroutes: null\n"` (`:22`). No router reload, no YAML service.
7. Cache: no `core::cache::Cache` call in handler; resource-store read is backed by context cache (`context_context.cppm:720-729`). No cache-tag/TTL writes here.
8. Output: raw YAML string body, no envelope; empty store ⇒ fixture YAML.

**Diff**
- Residual gaps (no C++ primitive): Ghost `route-settings` adapter store and `parseRouteSettings`/YAML validation are not represented; no `permissions:{method:'browse'}` equivalent (`require_auth` only).
- Divergence: content comes from a context-local `routes_yaml` fixture with a hardcoded fallback instead of the real `yamlSource`; no `store.get()` validation error path.
- Verdict PARTIAL: path/verb/auth/plain+disposition/status match; data source stands in.

### POST /settings/routes/yaml — PARTIAL

**Ghost**
1. Identity: `router.post('/settings/routes/yaml', mw.authAdminApi, apiMw.upload.single('routes'), apiMw.upload.validation({type:'routes'}), http(api.settings.upload))` (`admin/routes.js:102-108`). `single()` = multer to `os.tmpdir()` with cleanup on finish/close (`api/middleware/upload.js:71-72,115-171`); `validation({type:'routes'})` checks file exists, extension/MIME allowlist, `routes.missingFile`/`invalidFile` messages (`upload.js:28-31,262-330`).
2. Operation: `settings.upload`, `docName:'settings'` (`settings.js:29,185-196`).
3. Input: multipart field named `routes`; `frame.file.path` = temp file path; no `options`/`validation{}` in controller.
4. Auth: `mw.authAdminApi`; `permissions:{method:'edit'}` (`settings.js:189-191`).
5. Status/headers: 200; `headers.cacheInvalidate:true` ⇒ `X-Cache-Invalidate: /*` (`settings.js:186-188`).
6. Call chain: `fs.readFile(frame.file.path,'utf8')` → `routeSettings.api.upload(content)` (`settings.js:193-194`) → bound `DynamicRoutingService.upload` (`route-settings/index.js:27-29`) → `parseYaml` → `parseRouteSettings` (validates, rejects invalid upload) → `this.store.replace(next)` → `bridge.reloadFrontend(this,urlService)` (`dynamic-routing-service.js:83-102`). Side effects: route-settings store replacement + frontend router reload.
7. Cache: `X-Cache-Invalidate: /*`; route-settings adapter replaced.
8. Output: `serializeData` (`utils/serializers/output/settings.js:100`) → raw data (upload returns undefined) ⇒ effectively empty body.

**C++**
1. Identity: same `Route{"/yaml"}.post(...)` (`settings/routes.cppm:84-91`); `settings_content_type` bypasses JSON gate for multipart (`:19-32`); `require_auth` (`:41`).
2. Operation: `settings_settings_routes_yaml_post` handler + `ExecuteStep` (`workflow/flow/main.cppm:8-10`).
3. Input: reads `in.body_as_string()` and `Content-Type` header (`post/execute.cppm:15-16`); manually extracts `boundary=` (`:18-22`) and part with `name="routes"` (`:23-31`); empty part ⇒ `{"__BAD_REQUEST__"}` (`:33`).
4. Auth: `require_auth` only (401 on empty token); no permission/docName.
5. Status/headers: 200 with `X-Cache-Invalidate: /*` on success; 400 with raw body `__BAD_REQUEST__` on parse failure (`post/handler.cppm:17-27`).
6. Call chain: destroy every existing `routes_yaml` record then `rs.create("routes_yaml", {"content":yaml})` (`post/execute.cppm:35-42`). No YAML parse, no `parseRouteSettings`, no `store.replace`, no `bridge.reloadFrontend`.
7. Cache: `X-Cache-Invalidate: /*` header (stand-in for Ghost's cacheInvalidate); resource-store writes use context cache tag `resources`, TTL never (`context_context.cppm:656-667`).
8. Output: body `{}` (not Ghost's empty) on success; sentinel string on 400.

**Diff**
- Residual gaps: real YAML parsing/validation, route-settings adapter `replace`, and frontend router reload have no C++ primitive; no `permissions:{method:'edit'}`.
- Divergence: no `apiMw.upload.validation({type:'routes'})` equivalent (.yaml/.yml + MIME allowlist); response `{}` vs empty; 400 body is a sentinel string, not a Ghost error envelope.
- Verdict PARTIAL: path/verb/auth/cache-header match; persistence and side effects replaced.

### GET /settings — PARTIAL

**Ghost**
1. Identity: `router.get('/settings', mw.authAdminApi, http(api.settings.browse))` (`admin/routes.js:110`).
2. Operation: `settings.browse`, `docName:'settings'` (`settings.js:29,31-40`).
3. Input: `options:['group']` — `?group=x,y` is passed to `frame.options.group`; context from auth. No `data`/`validation`.
4. Auth: `mw.authAdminApi`; `permissions:true` under `docName:'settings'` (`settings.js:36,29`). No labs flags in controller; labs flag values are injected in service `_formatBrowse`.
5. Status/headers: 200; `headers.cacheInvalidate:false` ⇒ no X-Cache-Invalidate (`settings.js:32-34`).
6. Call chain: `settingsBREADService.browse(frame.options.context)` (`settings.js:38`) → `SettingsCache.getAll()` → `_formatBrowse(settings,context)` (`settings-bread-service.js:130-134,354-392`): drop `core` group for non-internal, `hideValueIfSecret` (obfuscates `/secret|api_key/` keys to `••••••••` — `settings-utils.js:7,15-31`), override `labs` value from `labs.getAll()` (`:377-380`), mark `is_private`/`password` read-only when `publicSiteAccess` limit disabled (`:382-389,394-396`). No mail/webhook.
7. Cache: in-memory `SettingsCache`; no X-Cache-Invalidate.
8. Output: `serializeSettings` (`utils/serializers/output/settings.js:32-68,94`): for admin, `settingsFilter(models,frame.options.group)` by comma-separated group (`:14-23,48`), rewrite icon `/content/images/` → `/content/images/size/w256h256/` (`:52-55`), envelope `{settings:mappers.settings(...), meta: models.meta ?? {}}`; mapper picks only `key,value,is_read_only` (`mappers/settings.ts:3-10`); when group set, `meta.filters.group = group` (`:63-67`).

**C++**
1. Identity: `.get(...)` on `Router{router,"/settings"}` (`settings/routes.cppm:42-48`); router-wide `settings_content_type` requires `application/json` Content-Type AND Accept (`middleware/serde.cppm:90-126`); `require_auth`.
2. Operation: `settings_browse` handler + `ExecuteStep` (`browse/workflow/flow/main.cppm:8-10`).
3. Input: step parses query string from `path.find('?')` into `std::map params` (`browse/execute.cppm:15-22`), then calls `ctx.store().json_for("settings", params)` (`:23`). `json_for("settings",...)` discards params entirely and returns `settings_json()` (`context_context.cppm:1161`).
4. Auth: `require_auth` only; no docName/permission service, no labs/secret handling.
5. Status/headers: status OK, body set, no X-Cache-Invalidate (`browse/handler.cppm:21-22`) — matches cacheInvalidate:false.
6. Call chain: `settings_json()` builds a hardcoded static array of 21 `{key,value}` pairs (`context_context.cppm:1682-1716`) — all string values, no `group`, no `is_read_only`, no labs/secret/limit logic. Step wraps as `"{\"settings\":" + rows + "}"` (`browse/execute.cppm:24`).
7. Cache: no `core::cache::Cache` write; read is the static fixture, not `m_db`/`m_resources`.
8. Output: `{"settings":[...]}` with no `meta`; empty/missing step output ⇒ body `"{}"` (`browse/handler.cppm:20`). No icon resize, no group filter.

**Diff**
- Residual gaps: Permissions service, `SettingsCache`, labs override, secret obfuscation, public-site-access read-only flags, and calculated fields have no allowed C++ primitive.
- Divergence: `?group=` parsed but ignored — C++ always returns the same fixture; response lacks `meta`, `is_read_only`, `group`, and any edits made through PUT `/settings` are never reflected (writes go to `m_resources`, not `settings_json()`).
- Verdict PARTIAL: route/verb/auth/status/envelope key match; payload is fabricated stand-in.

### PUT /settings — PARTIAL

**Ghost**
1. Identity: `router.put('/settings', mw.authAdminApi, http(api.settings.edit))` (`admin/routes.js:111`).
2. Operation: `settings.edit`, `docName:'settings'` (`settings.js:29,139-167`).
3. Input: body `data.settings[0..n]` with `{key,value}`. Input serializer (`utils/serializers/input/settings.js:7-152`): `EDITABLE_SETTINGS` allowlist of ~96 keys (`:7-103`), drops non-allowlisted keys for non-internal frames (`:114-119`), coerces boolean `'0'/'1'`/`'false'/'true'` (`:127-134`), filters `labs` flags to `WRITABLE_KEYS_ALLOWLIST` (`shared/labs.js:65` = PUBLIC_BETA_FEATURES + PRIVATE_FEATURES; applied at `:136-148`), rewrites icon URLs (`:150`). Validator (`utils/validators/input/settings.js:143-241`) enforces array types (`navigation`, `secondary_navigation`, `notifications`, `announcement_visibility`), navigation item URL/label/icon/visibility rules, `members_support_address` email, `announcement_visibility`/`announcement_background` allowed values; rejects on first error (`:237-239`).
4. Auth: `mw.authAdminApi`; `permissions.unsafeAttrsObject(frame)` returns the `labs` setting (`settings.js:143-147`), so labs keys are gated by unsafe-attr permission. No separate labs flag on route.
5. Status/headers: 200; `headers.cacheInvalidate:false`; controller explicitly `frame.setHeader('X-Cache-Invalidate','/*')` when edit result non-empty (`settings.js:140-142,157-159`).
6. Call chain: `getStripeConnectData(frame)` — if a `stripe_connect_integration_token` setting present, calls `settingsBREADService.getStripeConnectData(..., membersService.stripeConnect.getStripeConnectTokenData)` (`settings.js:11-25,149`) → errors become `BadRequestError` (`settings-bread-service.js:337-352`); then `settingsBREADService.edit(frame.data.settings, frame.options, stripeConnectData)` (`:151-155`) → filters token/obfuscated secrets, rejects unknown settings `NotFoundError`, core-setting access `NoPermissionError`, public-site-access-locked edits `NoPermissionError` (`settings-bread-service.js:201-255`), applies Stripe Connect data + `limitsService.errorIfWouldGoOverLimit('limitStripeConnect')` + `stripeService.connect()` for live keys (`:257-287`), logs `automations.beta_flag_enabled` when flag newly on (`:289,398-411`), email-verification prep + `ValidationError` on invalid address + magic-link mail via `sendEmailVerificationMagicLink` (`:291-293,416-477,482-503`), `SettingsModel.edit(refilteredSettings, options)` (`:295`). Then full `browse` for return with `meta` from result (`:161-165`).
7. Cache: explicit `X-Cache-Invalidate: /*`; SettingsCache refreshed via model events.
8. Output: `serializeSettings` → full settings with `meta` (e.g. `meta.sent_email_verification`); computed fields included.

**C++**
1. Identity: `.put(...)` on `/settings` (`settings/routes.cppm:50-57`); JSON gate + `require_auth`.
2. Operation: `settings_bulk_edit` handler + `ExecuteStep` (`bulk_edit/workflow/flow/main.cppm:8-10`).
3. Input: `in.body_as_string()` → `blog::api::parse_array(body)`; expects wrapper object containing `settings`, then array of `{key,value}` (`bulk_edit/execute.cppm:15-21`). Missing any ⇒ `__BAD_REQUEST__` sentinel.
4. Auth: `require_auth` only; no `unsafeAttrs`/labs permission, no allowlist/boolean coercion/validator.
5. Status/headers: 200; `X-Cache-Invalidate: /*` set when body non-empty and not `{}` (`bulk_edit/handler.cppm:22-26`); 400 sentinel otherwise — mirrors Ghost's conditional header.
6. Call chain: for each record with `key`+`value`, `rs.create("settings", {"key":<key>,"value":<value>})` (`bulk_edit/execute.cppm:23-29`). No filtering, no model edit, no stripe/email/automations side effects. Side effect is a context-cache record create (tag `resources`, TTL never).
7. Cache: `X-Cache-Invalidate: /*` header; resource-store writes via context cache.
8. Output: `{"settings":` + `json_for("settings",{})` + `,"meta":{}}` (`bulk_edit/execute.cppm:30-31`) — settings portion is the hardcoded fixture; `meta` always `{}`.

**Diff**
- Residual gaps: full editable-key allowlist, labs `WRITABLE_KEYS_ALLOWLIST` filtering, input validator/array+email rules, unsafe-attrs permission, Stripe Connect fetch/limits, limits service, email verification mail/magic links, automations logging, core/secret/locked-key guards — none have C++ primitives.
- Divergence: each edit appends a new `settings` record (no dedup/update), and the returned "all settings" is the static fixture, so edits are not observable in the response; `meta` never carries `sent_email_verification`.
- Verdict PARTIAL: route/verb/auth/status/cache-header/envelope match; service chain and validation replaced.

### POST /settings/access_code/regenerate — PARTIAL

**Ghost**
1. Identity: `router.post('/settings/access_code/regenerate', mw.authAdminApi, http(api.settings.regenerateAccessCode))` (`admin/routes.js:112-116`).
2. Operation: `settings.regenerateAccessCode`, `docName:'settings'` (`settings.js:29,169-183`).
3. Input: no options/data/validation; server generates the code.
4. Auth: `mw.authAdminApi`; `permissions:{method:'edit'}` (`settings.js:173-175`).
5. Status/headers: 200; `headers.cacheInvalidate:false`; explicit `X-Cache-Invalidate: /*` (`settings.js:170-171,178`).
6. Call chain: `settingsService.regeneratePrivateSiteAccessCode()` (`settings.js:177`) → `models.Settings.edit([{key:'password', value:generatePrivateSiteAccessCode()}],{context:{internal:true}})` (`settings-service.js:177-187`). `generatePrivateSiteAccessCode()` returns `word###` (word from 48-word list + 3-digit number) (`private-site-access-code.js:3-63`). Internal context bypasses read-only `password`; then `browse` for return (`settings.js:181`).
7. Cache: `X-Cache-Invalidate: /*`; SettingsCache refreshed via model event.
8. Output: `serializeSettings` → full settings.

**C++**
1. Identity: `.add_router(Router{router,"/access_code"})` + `Route{"/regenerate"}.post(...)` (`settings/routes.cppm:58-71`); JSON gate + `require_auth`.
2. Operation: `settings_settings_access_code_regenerate_post` handler + `ExecuteStep` (`regen/workflow/flow/main.cppm:8-10`).
3. Input: `invoke(const Input&,...)` ignores request (`regen/execute.cppm:13`).
4. Auth: `require_auth` only; no `permissions:{method:'edit'}`.
5. Status/headers: 200; `X-Cache-Invalidate: /*` set unconditionally (`regen/handler.cppm:22-23`).
6. Call chain: `std::random_device`/`mt19937_64` → `std::format("{:016x}")` hex code (`regen/execute.cppm:15-17`); `rs.create("settings", {"key":"private_site_access_code","value":<code>})` (`:18-19`). No internal-context notion, no private-site-access limit handling.
7. Cache: `X-Cache-Invalidate: /*` header; resource-store write via context cache.
8. Output: `{"settings":` + static fixture + `}` (`regen/execute.cppm:20-21`) — generated code not reflected in response.

**Diff**
- Residual gaps: `permissions:{method:'edit'}`, internal-context write to bypass read-only `password`, `publicSiteAccess` limit logic, and SettingsCache model-event refresh have no C++ primitive.
- Divergence: writes key `private_site_access_code` (Ghost uses `password`) with a 16-hex value (Ghost uses `word###`); code never appears in the returned settings.
- Verdict PARTIAL: route/verb/auth/status/cache-header/return-shape match; key/format/persistence stand in.

### PUT /settings/verifications/ — PARTIAL

**Ghost**
1. Identity: `router.put('/settings/verifications/', mw.authAdminApi, http(api.settings.verifyKeyUpdate))` (`admin/routes.js:117`). Trailing slash is normalized by Express; C++ `split_path` also drops empty segments (`include/core/router/types.cppm:17-31`), so `/settings/verifications` matches.
2. Operation: `settings.verifyKeyUpdate`, `docName:'settings'` (`settings.js:29,64-80`).
3. Input: `data:['token']` (`settings.js:71`) — magic-link single-use token. No validation options declared on route.
4. Auth: `mw.authAdminApi`; `permissions:{method:'edit'}` (`settings.js:68-70`).
5. Status/headers: 200; `headers.cacheInvalidate:true` ⇒ `X-Cache-Invalidate: /*` (`settings.js:65-67`).
6. Call chain: `settingsBREADService.verifyKeyUpdate(frame.data.token)` (`settings.js:73`) → `magicLinkService.getDataFromToken(token)` → `{key,value}` → guard `EMAIL_KEYS` (`['members_support_address']`) else `IncorrectUsageError` → `SettingsModel.edit({key,value})` (`settings-bread-service.js:313-328`). Then full `browse` because "calculated settings might change" (`settings.js:75-78`).
7. Cache: `X-Cache-Invalidate: /*`; SettingsCache refreshed via model event.
8. Output: `serializeSettings` → full settings.

**C++**
1. Identity: `Route{"/verifications"}.put(...)` under `/settings` (`settings/routes.cppm:108-117`); JSON gate + `require_auth`.
2. Operation: `settings_settings_verifications_put` handler + `ExecuteStep` (`verifications/workflow/flow/main.cppm:8-10`).
3. Input: decodes body as JSON object, requires non-empty string field `token` (`verifications/execute.cppm:16-26`); malformed/missing ⇒ `__BAD_REQUEST__`.
4. Auth: `require_auth` only; no `permissions:{method:'edit'}`.
5. Status/headers: 200 + `X-Cache-Invalidate: /*` on success; 400 sentinel on failure (`verifications/handler.cppm:22-26`).
6. Call chain: scans `rs.list("settings_verifications")`, compares `token` to JSON-encoded input (`:30-32`); on match, `rs.destroy("settings_verifications", id)` (`:34-36`); no match ⇒ 400 sentinel (`:38`). No magic-link token decode, no `EMAIL_KEYS` guard, no `SettingsModel.edit`.
7. Cache: `X-Cache-Invalidate: /*` header; resource-store destroy via context cache.
8. Output: `{"settings":` + static fixture + `}` (`:39-40`) — verified value never written.

**Diff**
- Residual gaps: `MagicLink.getDataFromToken`, single-use token provider, `EMAIL_KEYS` restriction, and `SettingsModel.edit({key,value})` have no C++ primitive.
- Divergence: consumes a raw token string from a stand-in `settings_verifications` resource; no actual setting is updated, so the cached "full settings" do not change; 400 body is a sentinel.
- Verdict PARTIAL: route/verb/auth/status/cache-header/return-shape match; token semantics and write replaced.

### DELETE /settings/stripe/connect — PARTIAL

**Ghost**
1. Identity: `router.delete('/settings/stripe/connect', mw.authAdminApi, http(api.settings.disconnectStripeConnectIntegration))` (`admin/routes.js:118-122`).
2. Operation: `settings.disconnectStripeConnectIntegration`, `docName:'settings'` (`settings.js:29,82-137`).
3. Input: none.
4. Auth: `mw.authAdminApi`; `permissions:{method:'edit'}` (`settings.js:87-89`).
5. Status/headers: `statusCode:204`; `headers.cacheInvalidate:false` ⇒ no X-Cache-Invalidate (`settings.js:83-86`).
6. Call chain: `membersService.api.memberBREADService.browse({limit:0, filter:'status:paid'})` → if `meta.pagination.total !== 0` throw `BadRequestError('Cannot disconnect Stripe whilst you have active subscriptions.')` (`settings.js:91-99`); `await stripeService.disconnect()` (`:101`); `models.Settings.edit([7 keys → null], frame.options)` for `stripe_connect_publishable_key`, `stripe_connect_secret_key`, `stripe_connect_livemode`, `stripe_connect_display_name`, `stripe_connect_account_id`, `members_stripe_webhook_id`, `members_stripe_webhook_secret` (`:103-135`). Side effects: third-party Stripe disconnect + model edit + SettingsCache refresh.
7. Cache: no X-Cache-Invalidate; SettingsCache refreshed via model events.
8. Output: 204 no body.

**C++**
1. Identity: `Router{router,"/stripe"} Route{"/connect"}.delt(...)` under `/settings` (`settings/routes.cppm:94-107`); JSON gate + `require_auth`.
2. Operation: `settings_settings_stripe_connect_delete` handler + `ExecuteStep` (`stripe/workflow/flow/main.cppm:8-10`).
3. Input: `invoke(const Input&,...)` ignores request (`stripe/execute.cppm:13`).
4. Auth: `require_auth` only; no `permissions:{method:'edit'}`.
5. Status/headers: 204 with empty body; 400 with `__BAD_REQUEST__` sentinel when paid members exist; no X-Cache-Invalidate (`stripe/handler.cppm:22-26`) — matches cacheInvalidate:false.
6. Call chain: `ctx.store().json_for("members", {{"filter","status:paid"}})` → non-empty `parse_array` ⇒ 400 sentinel (`stripe/execute.cppm:15-17`); else `rs.create("settings", {"key":k,"value":null})` for the same 7 keys (`:19-24`). No `stripeService.disconnect()`.
7. Cache: resource-store creates via context cache (tag `resources`, TTL never); no X-Cache-Invalidate.
8. Output: 204 empty on success; sentinel body on 400 (Ghost uses a proper error envelope/message).

**Diff**
- Residual gaps: `stripeService.disconnect()` third-party call, `membersService` BREAD pagination meta check, and SettingsCache model-event refresh have no C++ primitive.
- Divergence: paid-member check uses raw resource-store JSON rather than `meta.pagination.total`; nulling appends 7 new `settings` records (no dedup/update); 400 body is a sentinel rather than Ghost's message.
- Verdict PARTIAL: route/verb/auth/204/no-cache-header and the 7 nulled keys match; side effects and error shape stand in.

## Module-level notes

- **Controller `read` op has no route.** `settings.read` is exported (`settings.js:42-62`) with `options:['key']`, `validation.options.key.required`, `permissions.identifier` returning `frame.options.key`, and `settingsBREADService.read(key,context)`. No `http(api.settings.read)` binding exists in `admin/routes.js` (grep of `settings.read|api.settings` over `server/web/api` returns only download/browse/edit/regenerate/verifications/stripe), and there is no `/settings/:key` C++ route. No route item ⇒ no per-route section and no Missing/Extra verdict.
- **Shared service graph (Ghost)** used by several routes: `settingsService.getSettingsBREADServiceInstance()` (`settings.js:9`) wires `SettingsBREADService` with `SettingsModel`, `SettingsCache`, `labsService`, `limitsService`, `mail`, `SingleUseTokenProvider`, `urlUtils`, `emailAddressService` (`settings-service.js:44-60`). `SettingsBREADService` owns the magic-link email verification flow and `GhostMailer` (`settings-bread-service.js:71-122,482-503`). `route-settings` service owns YAML download/upload + frontend reload (`route-settings/index.js:26-33`, `dynamic-routing-service.js:77-102`). These dependencies are all residual gaps on the C++ side.
- **Permissions model absent in C++**: Ghost routes use `mw.authAdminApi` plus per-op `permissions` (`true`, `{method:'edit'|'browse'}`, `unsafeAttrsObject`, `identifier`) under `docName:'settings'`. C++ installs only `require_auth` (`middleware/auth.cppm:12-29`) which checks bearer-token presence and returns 401; no docName/method/unsafe-attr check exists (grep `docName|permission` over `include/blog/handlers/**/*.cppm` shows no handler-side permission primitive). Accepted residual gap for all 7 routes.
- **C++ dispatch pattern**: every route runs a single-step local workflow (`core::workflow::LocalWorkflowDriver`) registered in the handler constructor, reads `typed_output` (a `std::string`) from `result_of("<workflow>_execute")`, and writes it as the response body (`browse/handler.cppm:12-23` and siblings). Steps use a `static blog::BlogContext* s_ctx` set by the handler (e.g. `browse/execute.cppm:27-29`).
- **C++ state layer**: writes go through `ctx.store().resources().create/destroy` (`context_context.cppm:656-688`) which store flat records in the context-local `core::cache::Cache` under `resource:<resource>:<id>` with tag `resources` and `Ttl{}.never()`. `json_for("settings",...)` never reads these records — it returns the hardcoded `settings_json()` fixture (`context_context.cppm:1161,1682-1716`). No `core::cache::Cache` API is called directly by the settings handlers, and no explicit cache tag/TTL is set by them.
- **Stand-in sentinel**: several C++ steps return the literal `"__BAD_REQUEST__"` which the handlers translate to HTTP 400 with that raw string as body (`bulk_edit/handler.cppm:22-24`, `verifications/handler.cppm:22-24`, `stripe/handler.cppm:22-24`, `post/handler.cppm:22-24`). Ghost instead emits structured `@tryghost/errors` envelopes (ValidationError/BadRequestError/NoPermissionError/NotFoundError).
- **Icon/labs computed fields**: Ghost output serializer resizes `icon` to `w256h256` (`utils/serializers/output/settings.js:39-55`) and `_formatBrowse` injects `labs.getAll()` plus calculated fields (`settings-bread-service.js:375-392`, `settings-service.js:199-331`). None of this exists in C++; the fixture has no `group`, `is_read_only`, or labs flags.
- **Verdict totals**: 7 PARTIAL, 0 MATCH/MISMATCH/MISSING/EXTRA. Every divergence is a stand-in/fixture or a missing Ghost primitive; none of the routes is wired to a fundamentally different verb/path/status that would merit MISMATCH.

## Group 56 — strict-parity implementation record (2026-11-20)

Attribution: this section supersedes the "PARTIAL because stand-in" reasoning in
the seven `**Diff**` blocks above. Every divergence named there is closed at the
C++ abstraction boundary; the summary counts are intentionally **unchanged**
(7 PARTIAL, 0 MATCH) because the remaining items need centrally owned primitives
and no verdict is flipped unless fully achieved.

Closure per route (evidence: the seven `execute.cppm` / `handler.cppm` pairs in
`include/blog/handlers/system/settings/`):

- **GET `/settings/routes/yaml`** — data source is now the `routes_yaml`
  `ResourceStore` record read through `support::browse` helpers; `text/plain` +
  `Content-Disposition` set from the step's `Output` map; per-op
  `settings:browse` gate via registered `require_ghost_permission`; no
  `X-Cache-Invalidate`. Residual: real route-settings adapter store/YAML parser.
- **POST `/settings/routes/yaml`** — multipart `name="routes"` + filename
  extraction with Ghost `routes.missingFile`/`routes.invalidFile` envelopes,
  heuristic YAML validation, replace-all `routes_yaml` write, `X-Cache-Invalidate: /*`,
  empty `{}` body, `settings:edit` gate; no sentinel 400. Residual: real
  `parseYaml`/`parseRouteSettings` and `bridge.reloadFrontend`.
- **GET `/settings`** — group filter (`core::query::parse`), secret obfuscation,
  icon resize, labs/read-only formatting, `meta.filters.group`, store-backed
  override merge (writes now observable), `settings:browse` gate; no fabricated
  pagination. Residual: real `SettingsCache`/labs/limits services.
- **PUT `/settings`** — editable-key allowlist, boolean coercion, labs writable
  filtering, validator, unknown/core/locked-key guards, dedup write-through,
  full-browse return + `meta.sent_email_verification`, `X-Cache-Invalidate: /*`,
  Ghost error envelopes, `settings:edit` gate. Residual: Stripe Connect fetch,
  limits service, real magic-link mail, automations log, unsafe-attrs detail.
- **POST `/settings/access_code/regenerate`** — key `password`, `word###` value
  (48-word list), dedup write-through, browse-all return, `X-Cache-Invalidate: /*`,
  `settings:edit` gate, outbox events. Residual: `publicSiteAccess` limit and
  SettingsCache model events.
- **PUT `/settings/verifications/`** — token extraction, `EMAIL_KEYS` guard,
  one-shot token record, actual `{key,value}` write, browse-all return,
  `X-Cache-Invalidate: /*`, Ghost errors, `settings:edit` gate. Residual: signed
  single-use token codec and real mailer.
- **DELETE `/settings/stripe/connect`** — `core::query` NQL paid-member filter,
  Ghost `BadRequestError` message, `stripe.disconnected` outbox event, dedup
  nulling of the seven keys, 204 empty body, `settings:edit` gate. Residual: real
  `stripeService.disconnect()` adapter (not a registered dep).

Shared abstractions used: `core_query` (input/NQL), `core_auth:ghost` via the
registered `blog_handlers_middleware::require_ghost_permission`, the registered
`blog_handlers:outbox_shim` for all side effects, and `core_serde`. Helpers live
in the existing registered `blog_handlers:settings_browse_step` partition — no
new partition, no shared file, no BUILD/cycle fix. `blog_handlers:mapping`,
`core_outbox` (canonical), `core_adapters::StripeAdapter`, a YAML parser, a
signed-token codec and a real settings/labs/limits service are recorded as
central needs; none was created here.
