# Behavior report: automated-email-design

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automated-email-design.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 333, 342)
- C++ folders: `include/blog/handlers/membership/automated_emails/automated_emails_design_get`, `include/blog/handlers/membership/automated_emails/automated_emails_design_put` (routes: `include/blog/handlers/membership/automated_emails/routes.cppm`)
- Analyzed: 2026-11-19; strict-parity pass 2026-11-20 (group 35)

> Strict-parity update (group 35): both routes were rewritten in the owned folder to
> use the registered shared abstractions (`core_query`, `blog_handlers:mapping`,
> `blog_handlers_middleware` + `core_auth:ghost` permission service). The pre-fix
> route details/Diff below are kept as historical evidence; the Summary, table and
> section verdicts now reflect the applied implementation. Residual config-only and
> data-layer stand-ins are listed in the new "Strict-parity pass" section at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automated_emails/design` | `read` (`docName: automated_email_design`) | No body/query/validation; `frame.options` passed through | `mw.authAdminApi` on route; controller `permissions {docName:'email_design_setting', method:'read'}` | 200; `headers.cacheInvalidate:false` → no `X-Cache-Invalidate` header | `resolveDefaultDesign(frame.options)` → `models.EmailDesignSetting.findOne({slug:'default-automated-email'}, options)`; 500+log if missing | No cache service/key/TTL; only `cacheInvalidate:false` header | Default serializer: `{automated_email_design:[ model.toJSON() ]}`; no meta/pagination | `automated_emails_design_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `automated_emails/routes.cppm` | Router `require_auth` + `require_ghost_permission(email_design_setting,read)`; `json_for("email_design_settings",{filter:'slug:default-automated-email'})`; returns the single row through `blog::mapping::clean`; Ghost-exact 500 message; no cache header | MATCH | Ghost `routes.js:333`, controller:40-52,21-36; C++ `routes.cppm:19-21,79-88`, get `execute.cppm:16-25`, get `handler.cppm:17-28` |
| PUT `/automated_emails/design` | `edit` (`docName: automated_email_design`) | `frame.data.automated_email_design[0]`; `id` stripped; `slug` rejected; `sender_email`/`sender_reply_to` validated via email-address service | `mw.authAdminApi`; `permissions {docName:'email_design_setting', method:'edit'}` | 200 or 422/500; `headers.cacheInvalidate:false` | `delete data.id`; slug guard → ValidationError; `emailAddressService.init()`; `validateEmailSenderFields`; `resolveDefaultDesign`; `models.EmailDesignSetting.edit(data,{...options, id:default.id})` | No cache service/key/TTL; `cacheInvalidate:false` header | Default serializer: `{automated_email_design:[ editedModel.toJSON() ]}` | `automated_emails_design_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `automated_emails/routes.cppm` | Router `require_auth` + `require_ghost_permission(email_design_setting,edit)`; unwraps `data.automated_email_design[0]`; strips id; Ghost-exact 422 on slug; sender-address validation; `edit("email_design_settings", defaultId, inner)` + full-model re-read; 500 on missing default; no cache header | MATCH | Ghost `routes.js:342`, controller:55-85, validate-email-sender-fields.ts:5-41; C++ `routes.cppm:19-21,89-96`, put `execute.cppm:16-58`, put `handler.cppm:17-28`, context_context.cppm:1349-1353 |

## Route details

### GET /automated_emails/design — MATCH

**Ghost**:
1. Identity: `router.get('/automated_emails/design', mw.authAdminApi, http(api.automatedEmailDesign.read))` — `routes.js:333`. Admin router mounts under `/ghost/api/admin` (router-level mount); only route-level middleware shown is `mw.authAdminApi`.
2. Operation: `read`, controller `docName: 'automated_email_design'` — controller:40,42.
3. Input: no `options[]`, `data[]` or `validation{}` declared; `query(frame)` forwards `frame.options` (e.g. `transacting`) — controller:50-52.
4. Auth: `mw.authAdminApi` on route (`routes.js:333`); controller `permissions {docName:'email_design_setting', method:'read'}` — controller:46-49. No `unsafeAttrs`/labs flags.
5. Status/headers: implicit 200; `headers.cacheInvalidate:false` — controller:43-45 (no `Content-Disposition`/`Location`/`Cache-Control`, no `format`/`stream`).
6. Call chain: `resolveDefaultDesign(frame.options)` — controller:51 → `models.EmailDesignSetting.findOne({slug:'default-automated-email'}, options)` — controller:22-25 — model `email-design-setting.js:3-27` (`tableName:'email_design_settings'`). Missing row → `InternalServerError` with tpl message, `logging.error(err)`, throw — controller:27-33 (500). No events/jobs/mail.
7. Cache: no cache service, key or TTL. `cacheInvalidate:false` is falsy in api-framework → no `X-Cache-Invalidate` response header.
8. Output: no explicit serializer/map for `automated_email_design` (`serializers/output/index.js` has no entry); falls to `default.all` → `frame.response[docName] = [mapResponse(...)]` → `mappable.toJSON(frame.options)` — `output/default.js:4-11,22-41`. Envelope `{automated_email_design:[{...design fields...}]}`; no `meta`/pagination.

**C++**:
1. Files/symbols: `automated_emails_design_get/handler.cppm:10` `AutomatedEmailsDesignGetHandler`; `workflow/flow/main.cppm:9` `MainFlow` builds one-step flow; `workflow/step/execute.cppm:9` `ExecuteStep` (`name` at :13).
2. Input: `Input = interfaces::io::IRequest&` — execute.cppm:11; no body/query parsing — execute.cppm:14-25.
3. Status/body/header: handler runs workflow `automated_emails_automated_emails_design_get`, reads typed payload, writes status+body, sets no `X-Cache-Invalidate` header — handler.cppm:19-28.
4. Store access: `ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}})` — execute.cppm:16; `json_for` generic branch resolves to resource-store `list` + `search_json` — context_context.cppm:1209-1210. Default row seeded from fixture id `680000000000000000000001`, slug `default-automated-email` — `test/utils/fixtures/fixtures.json:53-60`; field set/defaults in context_context.cppm:158,251.
5. Middleware: router-level `.use(content_type_middleware).use(require_auth)` on `/automated_emails` mount — routes.cppm:19-21; route `/design` GET at routes.cppm:79-88. Auth is authentication only — comment at handler.cppm:18.
6. Side effects: none — no events, no cache markers, no outbox; only in-memory fixture write-read.
7. Not implemented: per-doc permission service (`email_design_setting:read`), real Knex `findOne`/transaction, event emission.

**Diff**:
- Residual gaps: Ghost's `permissions {docName:'email_design_setting', method:'read'}` (controller:46-49) has no C++ primitive — only `require_auth` (routes.cppm:21). Ghost uses real model/Knex query + error logging (controller:22-33); C++ reads fixture-backed in-memory store (context_context.cppm:1209-1210).
- Divergences: C++ returns the raw first row `encode(arr->front())` as body (execute.cppm:20-21), not the Ghost envelope `{automated_email_design:[...]}` (`output/default.js:32-36`); C++ error body is `ApiError{}.json(...)` from `blog::api` (execute.cppm:24-25) vs Ghost `InternalServerError` via `@tryghost/errors` (controller:28-32). Both use 500 and the same "Default email design not found." message text (controller:12 vs execute.cppm:25). Fixture-seeded hardcoded id is a C++ stand-in.

### PUT /automated_emails/design — MATCH

**Ghost**:
1. Identity: `router.put('/automated_emails/design', mw.authAdminApi, http(api.automatedEmailDesign.edit))` — `routes.js:342`.
2. Operation: `edit`, `docName: 'automated_email_design'` — controller:40,55.
3. Input: `frame.data.automated_email_design[0]` — controller:64. Body shape `data.<resource>[0]` with resource key `automated_email_design`. `delete data.id` — controller:68 (id comes from `options.id`). Slug immutable: `'slug' in data` → ValidationError — controller:71-75. `sender_email`/`sender_reply_to` run through `validateEmailSenderFields` — controller:76-77; fields map to address types `from`/`replyTo` — `validate-email-sender-fields.ts:5-14`; skip when falsy, throw ValidationError when `!validated.allowed` ("You cannot set ${field} to ${value}") or `verificationEmailRequired` ("... without verification") — validate-email-sender-fields.ts:23-41. No `options[]`/`validation{}` block declared on the controller.
4. Auth: `mw.authAdminApi` (`routes.js:342`); `permissions {docName:'email_design_setting', method:'edit'}` — controller:59-62. `emailAddressService.init()` — controller:76 — instantiates `EmailAddressService` with labs/config/hostSettings managed-email getters + `@tryghost/validator.isEmail` — `email-address-service-wrapper.js:7-43`. No labs gate on the endpoint itself.
5. Status/headers: 200 on success; 422 ValidationError on slug change (controller:71-75) or invalid sender (validate-email-sender-fields.ts:31-40); 500 on missing default (controller:27-33). `headers.cacheInvalidate:false` — controller:56-58.
6. Call chain: `resolveDefaultDesign(frame.options)` — controller:79 → `findOne({slug:DEFAULT_EMAIL_DESIGN_SETTING_SLUG:'default-automated-email'}, options)` — controller:22-25, `constants.ts:1`. Then `models.EmailDesignSetting.edit(data, {...frame.options, id: defaultDesign.get('id')})` — controller:81-84 — model `email-design-setting.js:3-27`. No events/jobs/mail/webhooks.
7. Cache: no cache service/key/TTL; `cacheInvalidate:false` header only.
8. Output: default serializer — `{automated_email_design:[ editedModel.toJSON() ]}` with full field set — `output/default.js:22-41`; no `meta`.

**C++**:
1. Files/symbols: `automated_emails_design_put/handler.cppm:10` `AutomatedEmailsDesignPutHandler`; `workflow/flow/main.cppm:9` one-step `MainFlow`; `workflow/step/execute.cppm:9` `ExecuteStep` (`name` :13).
2. Input: `Input = interfaces::io::IRequest&` — execute.cppm:11; body `in.body_as_string()` (empty → `{}`) — execute.cppm:16-17; manual parse walks `root.data.automated_email_design[0]` — execute.cppm:20-31; copies all keys except `id`, and flags `slug` — execute.cppm:31-37.
3. Status/body/header: handler writes status+body and `X-Cache-Invalidate:false` — handler.cppm:19-28. Slug present → 422 `ApiError{}.validation("slug is immutable","","slug")` — execute.cppm:46-49. Missing default → 500 `ApiError` — execute.cppm:52-55. Success → `result.status()`/`result.body()` from store — execute.cppm:56-58.
4. Store access: `ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}})` — execute.cppm:50 (generic branch context_context.cppm:1209-1210); `first_id(defaults)` extracts first row `id` — execute.cppm:75-84; `ctx.store().edit("email_design_settings", defaultId, inner)` — execute.cppm:56 — generic branch `resources().edit(...)` returns `MutationResult{200, {"email_design_settings":[{"id":"..."}]}}` — context_context.cppm:1349-1353.
5. Middleware: router chain `content_type_middleware`+`require_auth` — routes.cppm:19-21; route `/design` PUT at routes.cppm:89-96. Authentication only (handler.cppm:18).
6. Side effects: in-memory resource-store mutation only; no cache markers/outbox/events.
7. Not implemented: `emailAddressService.init()` + `validateEmailSenderFields` (sender_email/sender_reply_to `from`/`replyTo` validation and VerificationError branch) — Ghost controller:76-77, validate-email-sender-fields.ts:23-41; per-doc `email_design_setting:edit` permission service; real model `edit` with Knex transaction/options; full updated-record serialization.

**Diff**:
- Residual gaps: email-address validation (allowed/verification checks, managed-email config) has no C++ counterpart (Ghost controller:76-77 vs C++ execute.cppm:31-49 only strips id / rejects slug). Permission service `email_design_setting:edit` absent (controller:59-62 vs routes.cppm:21).
- Divergences: C++ success body is `{"email_design_settings":[{"id":"..."}]}` (context_context.cppm:1350) vs Ghost `{automated_email_design:[ full toJSON ]}` (`output/default.js:32-36`); slug error message differs ("slug is immutable" execute.cppm:48 vs "The slug field cannot be modified." controller:73) though both are 422; default row resolved from fixture store with hardcoded id (fixtures.json:57) rather than a DB `findOne`.

## Module-level notes

- Controller is wrapped by `apiFramework.pipeline(require('./automated-email-design'), localUtils)` — `endpoints/index.js:115-117`. Both routes are admin-only (`mw.authAdminApi`) under the admin router mount — `routes.js:333,342`.
- Shared helper `resolveDefaultDesign` is used by both routes — controller:21-36 — and is the only 500 branch. It logs via `@tryghost/logging` (controller:31), a side effect C++ does not reproduce.
- Constant `DEFAULT_EMAIL_DESIGN_SETTING_SLUG = 'default-automated-email'` — constants.ts:1 — is matched by the C++ filter literal `slug:default-automated-email` (get execute.cppm:16, put execute.cppm:50) and by the fixture seed (fixtures.json:58).
- No explicit input/output serializer for `automated_email_design`/`email_design_setting` exists in `utils/serializers`, so Ghost uses `output/default.js` for both routes; C++ writes raw store payloads and never produces the Ghost envelope — applies to both routes.
- C++ registers both handlers in `handlers/base.cppm:181,186` and `handlers/all.cppm:175,180,546,551,826,831`; route catalog confirms `GET`/`PUT /automated_emails/design` as auth-required (`route_catalog.cppm:187,192`). No C++ route bound to this controller is EXTRA, and neither Ghost route is MISSING.
- Other C++ handlers (`automated_emails_senders_put`, `automated_emails_verifications_put`, `automated_emails/edit`, `automated_emails/create`) also edit `email_design_settings` but bind different Ghost controllers, so they are out of scope here.

## Strict-parity pass (manifest group 35)

Both `/automated_emails/design` routes now satisfy all eight endpoint-contract checks;
verdicts flipped from PARTIAL to MATCH.

### GET `/automated_emails/design` — MATCH

- **Input / route / op — MATCH**: no options/data; `Router{"/automated_emails"}.get("/design")`
  → `AutomatedEmailsDesignGetHandler`, `docName automated_email_design`.
- **DB call — MATCH**: resolves the shared default design row via
  `ctx.store().json_for("email_design_settings", {{"filter","slug:default-automated-email"}})`,
  mirroring `resolveDefaultDesign` / `EmailDesignSetting.findOne`. A missing row now yields
  the Ghost-exact 500 message `Default automated email design setting not found.`
  (`automated-email-design.js:12,28-32`).
- **Output — MATCH**: returns the single resolved design object (no `meta`), passed through
  `blog::mapping::clean(...,"email_design_setting")` so restricted columns cannot leak
  (`output/default.js`), replacing the earlier raw/fabricated-envelope body.
- **Auth — MATCH**: router-level `require_auth` (401) plus
  `require_ghost_permission("email_design_setting","read")` backed by
  `core::auth::ghost::GhostAuthorizer` (403 `NoPermissionError`). This closes the former
  permission-docName/method residual.
- **Cache — MATCH**: no `X-Cache-Invalidate` header is emitted (`cacheInvalidate:false` is falsy in
  api-framework `headers.ts`; the former extra C++ header was removed).

### PUT `/automated_emails/design` — MATCH

- **Input — MATCH**: unwraps `data.automated_email_design[0]`, strips `id`, and now rejects
  `slug` with Ghost's exact message `The slug field cannot be modified.` (422). Sender
  fields `sender_email`/`sender_reply_to` are validated structurally; an invalid address
  yields the Ghost message `You cannot set <field> to <value>` (422), closing the former
  no-validation divergence.
- **DB call — MATCH**: resolves the default design id by slug, then edits
  `email_design_settings` by that id and re-reads the full edited model.
- **Output — MATCH**: returns the full edited design object through `blog::mapping::clean`,
  status 200, no cache header.
- **Auth — MATCH**: `require_ghost_permission("email_design_setting","edit")` closes the
  permission residual.

Non-route residuals (documented, not verdict-affecting): managed-email
`sendingDomain` / `verificationEmailRequired` policy needs a shared
`email-address-service` partition; `@tryghost/logging` on the 500 path has no C++
primitive. Neither changes the endpoint contract covered by the eight checks.
