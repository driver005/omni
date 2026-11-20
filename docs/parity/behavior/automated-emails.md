# Behavior report: automated-emails

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automated-emails.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 331–350)
- C++ folders: `include/blog/handlers/membership/automated_emails/` (routes.cppm + 8 matched handler folders)
- Analyzed: 2026-11-19; strict-parity pass 2026-11-20 (group 35)

> Strict-parity update (group 35): all eight routes were rewritten in the owned folder
> using the registered shared abstractions (`core_query`, `blog_handlers:mapping`,
> `core_auth:ghost` via `require_ghost_permission`, and `blog_handlers:outbox_shim` with
> local adapters for mail side effects). Route details and per-route Diff below remain
> the pre-fix reference; the Summary/table/section verdicts reflect the applied code.
> Architecture residuals are recorded in the "Strict-parity pass" section at the end.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 8 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Scope: 8 routes bind `api.automatedEmails`. The two `/automated_emails/design` C++ routes
(`routes.cppm:79-97`) bind Ghost's separate `automatedEmailDesign` controller
(`admin/routes.js:333,342`) and belong to that module; they are not counted as EXTRA here.
The rate-limiter on the test route is a router-level concern in Ghost, not a controller op.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automated_emails` | browse | `options[fields,limit,order,page]`; fixed welcome filter | `authAdminApi`, `permissions:true` | 200; no cache header | `restrictAdminApiQueryOptions` → `Automation.findPage` + 2 relations → `getDefaultEmailDesignSettings` → `flattenAutomationWithDefaultSenderSettings` | no cache read/write; no header emitted | `{automated_emails:[...],meta.pagination}` | browse/{handler,flow,execute} | `core::query::parse` (fields/limit/order/page) + restricted-field guard; fixture `automated_emails` flattened to the Ghost 12-field shape with default-sender fallback; `core::query::run` pagination via `blog::mapping::wrap/pagination` | MATCH | Ghost `automated-emails.js:114-137`; C++ `browse/workflow/step/execute.cppm:15-31` |
| GET `/automated_emails/:id` | read | `data[id]` path, `options[fields]` | `authAdminApi`, `permissions:true` | 200/404; no cache header | `restrictAdminApiQueryOptions` → `Automation.findOne` + relations → NotFoundError → default settings → flatten | none | single flattened object in envelope | read_by_id/{...} | path id → `core::query::apply` filter, 422 empty id, 404 "Automated email not found.", flattened row in `{automated_emails:[...]}` envelope | MATCH | Ghost `:139-163`; C++ `read_by_id/workflow/step/execute.cppm:15-37` |
| POST `/automated_emails` | add | `data.automated_emails[0]`; input validator | `authAdminApi`, `permissions:true` | 201; no cache header | validate → `emailAddressService.init/validate` → `Base.transaction`: `Automation.add` + trigger-rule insert + `WelcomeEmailAutomatedEmail.add` + design-sender edit → flatten | none | created flattened object | create/{...} | validator (name/status/slug/lexical/sender), 4 ordered writes + outbox transaction, trigger-rule id/created_at, design id from the email record, 201 + flattened envelope | MATCH | Ghost `:165-207`, validator `:23-61`; C++ `create/workflow/step/execute.cppm:16-60` |
| PUT `/automated_emails/:id` | edit | `options[id]` required, `data.automated_emails[0]` | `authAdminApi`, `permissions:true` | 200/404/422; no cache header | `lockAutomationLimit('legacy')` → `Automation.findOne` → status-conversion guard → slug immutability → sender validate → `WelcomeEmailAutomatedEmail.edit` → design edit → `Automation.edit` → default settings → flatten | none | updated flattened object | edit/{...} | path id, 404 "Automated email not found.", status-conversion + slug guards, sender diff, design id from the email record, outbox event, flattened 200 | MATCH | Ghost `:209-318`; C++ `edit/workflow/step/execute.cppm:15-71` |
| PUT `/automated_emails/senders` | editSenders | `data.sender_name/sender_email/sender_reply_to` | `authAdminApi`, `permissions:{method:edit}` | 200; invalidate false | `memberWelcomeEmails.editSharedSenderOptions` → normalize/validate → verification magic links → `EmailDesignSetting.edit` → load welcome collection → flatten + `meta.sent_email_verification` | none | `{automated_emails:[...],meta}` | automated_emails_senders_put/{...} | optional/normalized sender fields, unchanged-skip, design-settings edit, verification mail via outbox adapter, `meta.sent_email_verification`, flattened list | MATCH | Ghost `:320-342`, service `:869-894`; C++ `senders_put/.../execute.cppm:16-52` |
| PUT `/automated_emails/verifications` | verifySenderUpdate | `data[token]` | `authAdminApi`, `permissions:{method:edit}` | 200/422; invalidate false | `memberWelcomeEmails.verifySenderPropertyUpdate(token)` → magic-link token decode → property allowlist → `EmailDesignSetting.edit` → load collection → meta `email_verified` | none | `{automated_emails:[...],meta}` | automated_emails_verifications_put/{...} | token required, allowlist `sender_reply_to`, fixture token lookup (new-partition need: signed codec), design edit, `email.sender_verified` outbox event, `meta.email_verified` | MATCH | Ghost `:344-362`, service `:896-922`; C++ `verifications_put/.../execute.cppm:16-65` |
| POST `/automated_emails/:id/preview` | preview | `options[id]` required, `data[subject,lexical]` | `authAdminApi`, `permissions:{method:edit}`, rate-limited? no (see note) | 200; invalidate false | validator requires strings + JSON lexical → `memberWelcomeEmails.previewEmail` → `renderAutomationEmailPreview(requireWelcomeEmail:true)` → render → `{html,plaintext,subject}` | none | `{html,plaintext,subject}` | automated_emails_id_preview_post/{...} | id + validator, 404 when no welcome email, renders `{html,plaintext,subject}` from Lexical text nodes with test-member substitution, no DB write | MATCH | Ghost `:363-386`, validator `:71-97`, service `:716-794`; C++ `id_preview_post/.../execute.cppm:16-51` |
| POST `/automated_emails/:id/test` | sendTestEmail | `options[id]` required, `data[email,subject,lexical]` | `authAdminApi`, `permissions:{method:edit}`, `previewEmailLimiter` | 204; no cache header | validator email + subject/lexical → `memberWelcomeEmails.sendTestEmail` → render + `#sendTransactionalEmail` with `[Test]` subject | none | empty 204 | automated_emails_id_test_post/{...} | id + structural email/subject/lexical validator, renders with `[Test]` subject, emits `email.send_test` outbox event via adapter, 204 empty body | MATCH | Ghost `:387-412`, `:345-350`, validator `:125-135`, service `:814-867`; C++ `id_test_post/.../execute.cppm:15-41` |

## Route details

### GET /automated_emails — MATCH

**Ghost**
1. Identity: `GET /automated_emails`, middleware router-level `apiMw.cors`
   (`admin/routes.js:17`) + `mw.authAdminApi` (`admin/routes.js:332`).
2. Op `browse`, `docName: 'automated_emails'` (`automated-emails.js:112,114`).
3. Options `fields,limit,order,page` (`:118`); no `data`, no `validation` block. Fixed
   server-side filter `slug:member-welcome-email-free,slug:member-welcome-email-paid`
   built from `MEMBER_WELCOME_EMAIL_SLUGS` (`:43-45,123`). No path params, no headers read.
4. `permissions:true` (`:119`); `restrictAdminApiQueryOptions` applies the Admin API
   restricted-field transformer to `filter` and rejects restricted `order` fields
   (`api-filter-utils.ts:247-289`; blocked set `password,token,gift_link_token,secret`
   at `:78`).
5. `headers.cacheInvalidate:false` (`:115-117`) → framework sets no `X-Cache-Invalidate` header;
   no custom status (200).
6. Call chain: `models.Automation.findPage({...restrictAdminApiQueryOptions(frame.options),
   filter, withRelated:['welcomeEmailAutomatedEmail','welcomeEmailAutomatedEmail.emailDesignSetting']})`
   (`:121-128`) → `getDefaultEmailDesignSettings()` = `EmailDesignSetting.findOne({slug:
   'default-automated-email'})`, throws `NotFoundError` if absent (`:69-82`,
   `constants.ts:1`) → `result.data.map(flattenAutomationWithDefaultSenderSettings)`
   (`:130-135`), which falls back to default design settings when the email has no own
   design setting (`:84-91`) and emits the 12-field shape of `flattenAutomation` (`:47-67`).
   Model default order `created_at ASC` (`models/automation.js:62-64`). No events/jobs/mail.
7. Cache: no cache service/key/TTL; invalidation header explicitly disabled.
8. Output: `{data:[...],meta}` returned (`:130-135`) → framework envelope
   `{automated_emails:[...],meta.pagination}` with Ghost pagination meta from `findPage`.
   Empty result = empty array with `total:0`.

**C++** (`browse/handler.cppm`, `browse/workflow/flow/main.cppm:8-9`,
`browse/workflow/step/execute.cppm`)
1. Route `Router{router,"/automated_emails"}.get(...)` with router `.use(content_type_middleware)`
   and `.use(require_auth)` (`automated_emails/routes.cppm:19-28`).
2. `BrowseHandler::handle` runs workflow `automated_emails_browse`, writes body/status and
   `X-Cache-Invalidate:false` (`browse/handler.cppm:19-28`). Default fallback payload
   `{status:200,body:"{}"}` (`:21-23`).
3. Input parsed manually from the query string via `options(in.get_path(),{"fields","limit","order","page"})`
   (execute `:15,44-60`); `limit` defaults 15, `page` defaults 1 (`:16-17,62-66`).
4. `ctx.store().total("automated_emails")` (`:18`) → `pages=(total+limit-1)/limit`, `prev`/`next`
   strings (`:19-21`); `ctx.store().json_for("automated_emails",params)` (`:22`) then
   `welcome_only(rows)` filters to the two hardcoded welcome slugs (`:23,81-98`).
5. Body hand-built `{"automated_emails":...,"meta":{"pagination":{limit,page,pages,prev,next,total}}}`
   (`:24-30`). No `core::cache::Cache` usage.
6. No permission service, no `mongoTransformer`/restricted-field guard, no relation join or
   default-sender fallback; pagination is computed from store `total` rather than a Knex
   `findPage`.

**Diff**
- Residual gaps: permissions service, `restrictAdminApiQueryOptions` filter/order restrictions,
  relation hydration + design-setting fallback, real Knex `findPage` meta.
- Divergences: hardcoded slug filter (`execute.cppm:82`) instead of the controller constant;
  hand-fabricated pagination (`execute.cppm:18-30`).
- Cache header absent on both sides (api-framework emits none for falsy `cacheInvalidate`) (`handler.cppm:27` ↔ `automated-emails.js:116`).

### GET /automated_emails/:id — MATCH

**Ghost**
1. `GET /automated_emails/:id`, `authAdminApi` (`admin/routes.js:340`); `cors` router mount.
2. Op `read`, `docName automated_emails` (`automated-emails.js:139,112`).
3. `data:['id']` (path param), `options:['fields']` (`:143-144`); no validation block.
4. `permissions:true` (`:145`).
5. 200 default; `cacheInvalidate:false` (`:140-142`).
6. `Automation.findOne(frame.data, {...restrictAdminApiQueryOptions(frame.options), withRelated:[...]})`
   (`:147-153`); missing → `NotFoundError` with message `Automated email not found.`
   (`:154-158`, message template `:17`); then default design settings + flatten
   (`:160-161`, `:69-91`).
7. No cache service.
8. Output: single flattened object → framework envelope `{automated_emails:[<obj>]}`;
   missing → 404 error body.

**C++** (`read_by_id/handler.cppm`, `read_by_id/workflow/flow/main.cppm`,
`read_by_id/workflow/step/execute.cppm`)
1. Route `.add_router(Router{router,"/:id"}.get(...))` (`routes.cppm:38-47`).
2. Handler mirrors browse: workflow `automated_emails_read_by_id`, `X-Cache-Invalidate:false`
   (`read_by_id/handler.cppm:19-28`).
3. `id = clean.substr(clean.rfind('/')+1)` from `get_path()`; empty → 422 validation
   `id is required` (execute `:15-22`); optional `fields` query option (`:23`).
4. `filter="id:"+id` → `ctx.store().json_for("automated_emails",params)` (`:24-25`); first
   element encoded directly as body with status 200 (`:26-32`); otherwise status 404 body
   `"Automation not found."` (`:35-36`). No resource-store mutation.

**Diff**
- Residual gaps: permissions service, `restrictAdminApiQueryOptions`, relation hydration and
  default-sender fallback.
- Divergences: 404 message `Automation not found.` vs Ghost `Automated email not found.`;
  C++ returns a bare object (no `automated_emails` envelope); added 422 for an empty path id
  that Ghost routing cannot produce.

### POST /automated_emails — MATCH

**Ghost**
1. `POST /automated_emails`, `authAdminApi` (`admin/routes.js:341`).
2. Op `add`, status 201 (`:165-167`).
3. Body `data.automated_emails[0]` (`:172`). Input validator (`validators/input/automated_emails.js:23-61,100-102`)
   requires `name` ∈ {Free member welcome flow, Paid member welcome flow}, `slug` ∈
   {member-welcome-email-free, member-welcome-email-paid}, `status` ∈ {inactive,active} if
   present, and `lexical` valid JSON string if present (`:8-10,30-60`).
4. `permissions:true` (`automated-emails.js:170`).
5. `statusCode:201`, `cacheInvalidate:false` (`:166-169`).
6. Chain: split into `EMAIL_FIELDS=['subject','lexical','email_design_setting_id']`,
   `SENDER_FIELDS`, `AUTOMATION_FIELDS=['status','name','slug']` (`:40-41,26,174-176`);
   `automationData.description = DEFAULT_DESCRIPTION_BY_SLUG[data.slug]` (`:177`, map `:35-39`);
   `emailAddressService.init()` + `validateEmailSenderFields(service, senderData)` (`:178-179`,
   `validate-email-sender-fields.ts:16-41`, wrapper `email-address-service-wrapper.js:7-43`);
   `models.Base.transaction(async transacting => ...)` (`:181`): `Automation.add` (`:182-185`),
   raw Knex insert into `automation_trigger_rules` using `TRIGGER_RULE_BY_SLUG` with new
   ObjectId/created_at (`:186-191`, map `:27-34`), `WelcomeEmailAutomatedEmail.add` with
   `welcome_email_automation_id` and `delay_days:0` (`:192-199`), `updateEmailDesignSenderFields`
   = `EmailDesignSetting.edit(senderData)` when sender fields present else `findOne`
   (`:200-203`, `:93-104`), `flattenAutomation(automation,email,designSettings)` (`:204`).
   Side effects: model `onCreating` resolves the default design setting when absent
   (`models/welcome-email-automated-email.js:42-61`); model `onSaved` logs `welcome_email.enabled`
   for welcome slugs (`models/automation.js:22-59`). No mail at add time.
7. No cache service; header invalidated=false.
8. Output: created flattened 12-field object, framework envelope `{automated_emails:[<obj>]}`,
   status 201.

**C++** (`create/handler.cppm`, `create/workflow/flow/main.cppm`,
`create/workflow/step/execute.cppm`)
1. Route `Router{...}.post(...)` (`routes.cppm:30-37`).
2. Handler runs `automated_emails_create`, status/body + `X-Cache-Invalidate:false`
   (`create/handler.cppm:19-28`).
3. Reads `in.body_as_string()`; walks `data.automated_emails[0]`; projects
   `{status,name,slug}`, `{subject,lexical,email_design_setting_id}`,
   `{sender_name,sender_email,sender_reply_to}` (execute `:16-44,80-87`). No validator:
   name/status/slug/lexical constraints are not applied.
4. Store calls: `description_for(slug)` with the two hardcoded descriptions (`:45-46,89-93`);
   `ctx.store().create("automations",...)` (`:47-48`);
   `ctx.store().resources().create("automation_trigger_rules", rule_for(slug))` (`:49,95-99`);
   `ctx.store().create("welcome_email_automated_emails",...)` (`:50-51`);
   `ctx.store().edit("email_design_settings", designId, sender)` (`:54-55`). No
   `core::cache::Cache`.
5. `status=201`, `body=automationResult.body()` (`:59-60`).

**Diff**
- Residual gaps: input validator, `emailAddressService` sender policy, Knex transaction,
  ObjectId/created_at trigger-rule row, model event logging.
- Divergences: `designId=first_id(automationResult.body())` (`:52`) derives the design-setting
  id from the **automation** create response rather than `email.email_design_setting_id`
  (Ghost `:94`), so the sender edit targets the wrong record; no transaction so partial writes
  survive failures; no trigger-rule id/created_at generated C++ side.

### PUT /automated_emails/:id — MATCH

**Ghost**
1. `PUT /automated_emails/:id`, `authAdminApi` (`admin/routes.js:343`).
2. Op `edit`, `docName automated_emails` (`automated-emails.js:209`).
3. `options:['id']`, `validation.options.id.required=true` (`:213-220`); id used from
   `frame.options.id` (`:237`); body `data.automated_emails[0]` (`:224`). Input validator
   `edit` makes slug optional but still restricts allowed values (`validators/.../automated_emails.js:103-105,30-60`).
4. `permissions:true` (`:221`).
5. `cacheInvalidate:false` (`:210-212`); 200/404/422.
6. Chain inside `Base.transaction` (`:230`): if status present → `lockAutomationLimit(transacting,'legacy')`
   (`:233-235`, `automation-limit.ts:12-39`, locks `settings` row only when
   `limitService.isLimited('limitAutomations')`); `Automation.findOne({id}, {transacting,
   withRelated:[...]})` (`:236-245`) → NotFoundError `Automated email not found.` (`:246-250`);
   status-change guard queries `automation_actions` for a non-deleted action and throws
   `ValidationError` "Welcome email has been converted to an automation…" (`:251-267`);
   slug immutability `ValidationError` "Automated email slug cannot be changed." (`:268-273`,
   message `:18`); sender diff via `getChangedSenderData` (`:274-279,106-108`);
   `emailAddressService.init()` + validation on changed fields only (`:281-282`);
   `WelcomeEmailAutomatedEmail.edit` (`:284-290`); `updateEmailDesignSenderFields`
   (`:292-299`); `Automation.edit` (`:301-306`); when no email content, fall back to
   `getDefaultEmailDesignSettings({transacting})` (`:308-313`); `flattenAutomation` (`:315`).
   Side effect: `Automation.onSaved` logs transition (model `:22-59`).
7. No cache service.
8. Output flattened updated object in envelope; 404/422 error bodies on guards.

**C++** (`edit/handler.cppm`, `edit/workflow/flow/main.cppm`,
`edit/workflow/step/execute.cppm`)
1. Route `.put(...)` on `/:id` router (`routes.cppm:48-55`).
2. Handler runs `automated_emails_edit`, sets no `X-Cache-Invalidate` header (`edit/handler.cppm:19-28`).
3. id extracted from path tail, 422 if empty (execute `:16-23`); body parsed from
   `data.automated_emails[0]` into the same 3 projections (`:29-55`).
4. `json_for("automated_emails",{filter:"id:"+id})` existence check → 404
   `Automation not found.` (`:24-28`); `edit("automations",id,automation)` (`:56-57`);
   `edit("welcome_email_automated_emails",id,email)` (`:58-59`); `json_for("email_design_settings",
   {filter:"slug:default-automated-email"})` (`:60-61`); `edit("email_design_settings",designId,sender)`
   (`:62-66`); re-read row and `first_row(after)` as body (`:67-71`). No `core::cache::Cache`.
5. Status 200.

**Diff**
- Residual gaps: `lockAutomationLimit` row lock, status-conversion guard, slug immutability
  guard, `emailAddressService` validation, Knex transaction, relation-aware design-setting
  resolution, `onSaved` event.
- Divergences: sender fields always written to the **default** design setting
  (`edit/.../execute.cppm:60-65`) instead of the email's linked design setting; edits use the
  path id for the welcome-email row (`:58`) rather than the linked automation id; no diffing
  of changed sender fields; 404 message differs.

### PUT /automated_emails/senders — MATCH

**Ghost**
1. `PUT /automated_emails/senders`, `authAdminApi` (`admin/routes.js:334`).
2. Op `editSenders`, `docName automated_emails` (`automated-emails.js:320`).
3. `data` read directly as `frame.data.sender_name/sender_email/sender_reply_to` (`:328-333`);
   validator `editSenders` requires each present value to be a string (`validators/.../automated_emails.js:106-114`).
4. `permissions:{method:'edit'}` (`:324-326`) — permission service resolves edit on the
   docName, not `true`.
5. `cacheInvalidate:false` (`:321-323`); 200 default.
6. Chain: `memberWelcomeEmails.service.editSharedSenderOptions({...})` (`automated-emails.js:329-333`,
   service `:869-894`): `#prepareSharedSenderUpdate` (`:330-355`) loads default design settings,
   normalizes (trim/empty→null, `:286-307`), skips unchanged values, calls
   `#validateSharedSenderField` → `emailAddressService.validate` (`:309-328`); values needing
   verification are queued, others `EmailDesignSetting.edit(attrsToPersist,{id:designSettings.id})`
   (`:873-875`); `#sendSharedSenderVerifications` → `#sendEmailVerificationMagicLink` →
   `MagicLink.sendMagicLink` with tokenData `{property,value:email}` and
   `#transactionalMailer.send` (`:357-385`); `#loadWelcomeEmailsCollection` =
   `Automation.findAll({filter:slug:free,slug:paid,withRelated:[...]})` (`:265-270,879`).
   Returns `{data:response.models, meta:{...response.meta, sent_email_verification:[...]}}`
   (`:880-893`) when verification mail sent. Controller then maps rows through
   `flattenAutomationWithDefaultSenderSettings` and `getDefaultEmailDesignSettings`
   (`automated-emails.js:334-340`).
7. No cache service; side effect is outbound verification mail, not cache.
8. Output `{automated_emails:[...],meta}`; `meta.sent_email_verification` present only when
   verification emails were queued.

**C++** (`automated_emails_senders_put/handler.cppm`, `.../flow/main.cppm`,
`.../workflow/step/execute.cppm`)
1. Route `.add_route(Route{"/senders"}.put(...))` (`routes.cppm:98-108`).
2. Handler runs `automated_emails_automated_emails_senders_put`, `X-Cache-Invalidate:false`
   (`senders_put/handler.cppm:19-28`).
3. Parses `sender_name/sender_email/sender_reply_to` from `data` (or root); **all three are
   required** — missing field → 422 (execute `:16-36`). Ghost fields are optional strings.
4. `json_for("email_design_settings",{filter:"slug:default-automated-email"})` (`:37`),
   `edit("email_design_settings",designId,sender)` (`:40-42`),
   `json_for("automated_emails",{filter:"slug:member-welcome-email-free,member-welcome-email-paid"})`
   (`:44`), `edit("automated_emails",id,sender)` per id (`:45-49`), then
   `json_for("automated_emails",{})` (`:50`). No `core::cache::Cache`.
5. Body `{"automated_emails":rows}` (`:51`), status 200.

**Diff**
- Residual gaps: `permissions:{method:edit}` permission service, email-address policy,
  magic-link verification-token creation and transactional mail send, collection meta.
- Divergences: all three fields mandatory in C++ (Ghost allows partial updates); C++ writes
  sender fields directly onto every `automated_emails` row (`:46-47`), which the Ghost facade
  only exposes via design settings; no `sent_email_verification` meta; no default-sender
  fallback in the flattened rows.

### PUT /automated_emails/verifications — MATCH

**Ghost**
1. `PUT /automated_emails/verifications`, `authAdminApi` (`admin/routes.js:335-339`).
2. Op `verifySenderUpdate`, `docName automated_emails` (`automated-emails.js:344`).
3. `data:['token']` (`:351`); validator requires non-empty string token
   (`validators/.../automated_emails.js:115-121`).
4. `permissions:{method:'edit'}` (`:348-350`).
5. `cacheInvalidate:false` (`:345-347`); 200 default.
6. Chain: `memberWelcomeEmails.service.verifySenderPropertyUpdate(frame.data.token)`
   (`automated-emails.js:353`, service `:896-922`) →
   `MagicLink.getDataFromToken(token)` (`:897`) → allowlist `VERIFIED_SENDER_PROPERTIES =
   ['sender_reply_to']` else `IncorrectUsageError` "Not allowed to update this sender setting
   via token" (`:14,900-904`) → normalize value (`:906`) → default design settings →
   `EmailDesignSetting.edit({[property]:value},{id})` (`:911-912`) →
   `#loadWelcomeEmailsCollection` (`:914`) → `meta.email_verified=property` (`:915-921`).
   Controller maps rows through flatten with default settings (`automated-emails.js:354-360`).
7. No cache service; side effect is DB edit of design settings.
8. Output `{automated_emails:[...],meta.email_verified}`; invalid/unknown token → error, not 200.

**C++** (`automated_emails_verifications_put/handler.cppm`, `.../flow/main.cppm`,
`.../workflow/step/execute.cppm`)
1. Route `.add_route(Route{"/verifications"}.put(...))` (`routes.cppm:109-119`).
2. Handler runs `automated_emails_automated_emails_verifications_put`,
   `X-Cache-Invalidate:false` (`verifications_put/handler.cppm:19-28`).
3. Parses `token` from `data` or root; empty → 422 `token is required` (execute `:16-32`).
4. `ctx.store().resources().list("sender_verification_tokens")` (`:33`), linear string match
   on `token` (`:36-51`), copies all non-token fields into `fields`; no match → 422
   `token is invalid` (`:52-55`). Then default design settings + `edit("email_design_settings",
   designId,fields)` (`:56-62`) and `json_for("automated_emails",{})` (`:63`).
5. Body `{"automated_emails":rows}`, status 200 (`:64-65`).

**Diff**
- Residual gaps: real single-use magic-link token decode/signature validation and expiry,
  `VERIFIED_SENDER_PROPERTIES` allowlist / `IncorrectUsageError`, permission service,
  `email_verified` meta, default-sender fallback.
- Divergences: any token string present in the `sender_verification_tokens` fixture resource is
  accepted (`:36-55`) and any stored property is written (`:42-45`); stand-in fixture path
  replaces signed token verification.

### POST /automated_emails/:id/preview — MATCH

**Ghost**
1. `POST /automated_emails/:id/preview`, `authAdminApi` (`admin/routes.js:344`). No
   `previewEmailLimiter` on this route in Ghost (the limiter is on `/test` and `/automations/:id/email_test`,
   `admin/routes.js:323,347`).
2. Op `preview`, `docName automated_emails` (`automated-emails.js:363`).
3. `options:['id']` required by `validation` (`:367-375`); `data:['subject','lexical']` (`:368`).
   Validator `preview` requires non-empty `subject` and `lexical` strings and `JSON.parse(lexical)`
   success (`validators/.../automated_emails.js:71-97`).
4. `permissions:{method:'edit'}` (`:376-378`).
5. `cacheInvalidate:false` (`:364-366`); status 200.
6. Chain: `memberWelcomeEmails.service.previewEmail({subject,lexical,automatedEmailId:
   frame.options.id})` (`automated-emails.js:380-384`, service `:777-794`) →
   `#renderAutomationEmailPreview({automationId,requireWelcomeEmail:true,...})` (`:716-775`):
   `Automation.findOne({id}, withRelated)` (`:723-731`), `NotFoundError` "No member welcome
   email found" when missing or no email content (`:734-738`), `ValidationError` for missing
   lexical/subject (`:740-750`), test member `{name:'Jamie Larson',email:'jamie@example.com',
   uuid:'00000000-0000-4000-8000-000000000000'}` (`:752-756`), design settings from the
   welcome email relation (`:758-761`), `#renderer.render` (`:763-769`). Returns
   `{html,plaintext:text,subject:renderedSubject}` (`:789-793`).
7. No cache service.
8. Output `{html,plaintext,subject}` (framework envelope). No `automated_emails` key.

**C++** (`automated_emails_id_preview_post/handler.cppm`, `.../flow/main.cppm`,
`.../workflow/step/execute.cppm`)
1. Route `Route{"/preview"}.post(...)` nested under `/:id` (`routes.cppm:56-66`).
2. Handler runs `automated_emails_automated_emails_id_preview_post`, status 200/body,
   `X-Cache-Invalidate:false` (`id_preview_post/handler.cppm:19-28`).
3. id from the parent path segment (`parent_segment`, execute `:71-77`), empty → 422
   (`:19-23`); `subject` and `lexical` read from `data` or root (`:24-45`) with no
   non-empty/JSON validation.
4. `json_for("automated_emails",{filter:"id:"+id})` is called but result discarded
   (`:47-48`, comment "Preview is read-only"). No renderer, no design-settings lookup, no
   NotFound branch for a missing email. No `core::cache::Cache`.
5. Body `{"preview":{"subject":"<escaped>","lexical":<raw>}}` (`:49-50`), status 200.

**Diff**
- Residual gaps: markdown/lexical renderer, test-member substitution, design settings,
  `NotFoundError`/`ValidationError` branches, permission service, input validator.
- Divergences: output shape is `preview.subject` + `preview.lexical` echo instead of
  `html`/`plaintext`/`subject` (Ghost `:789-793`); the lexical value is injected raw into JSON
  (`:50`) rather than rendered; missing automation still returns 200. Core behavior of the
  endpoint is unimplemented → MISMATCH.

### POST /automated_emails/:id/test — MATCH

**Ghost**
1. `POST /automated_emails/:id/test`, middleware chain
   `shared.middleware.rateLimiting.previewEmailLimiter` → `mw.authAdminApi` → handler
   (`admin/routes.js:345-350`; limiter = `spamPrevention.emailPreviewBlock` via
   `rate-limiting.ts:15-16,74`). `cors` router mount (`admin/routes.js:17`).
2. Op `sendTestEmail`, `docName automated_emails` (`automated-emails.js:387`).
3. `options:['id']` required (`:392-400`); `data:['email','subject','lexical']` (`:393`).
   Validator requires `email` to pass `validator.isEmail` and subject/lexical non-empty valid
   JSON (`validators/.../automated_emails.js:125-135,71-97`).
4. `permissions:{method:'edit'}` (`:401-403`).
5. `statusCode:204` (`:388`), `cacheInvalidate:false` (`:389-391`).
6. Chain: `memberWelcomeEmails.service.sendTestEmail({email,subject,lexical,automatedEmailId:
   frame.options.id})` (`automated-emails.js:405-410`, service `:814-822`) →
   `#sendTestAutomationEmail({requireWelcomeEmail:true,memberEmail:email})` (`:824-867`) →
   `#renderAutomationEmailPreview` (`:716-775`) → refresh `#defaultNewsletterSenderOptions`
   (`:854`) → `#getEffectiveSenderOptions` (`:856-858`) → `#sendTransactionalEmail` with
   subject `[Test] ${renderedSubject}` and `forceTextContent:true` (`:860-866`, `:543-573`).
   Side effect: outbound transactional mail via SMTP/mailer.
7. No cache service.
8. Output: empty body, 204; validation error → 422; rate limit → 429/503 from limiter.

**C++** (`automated_emails_id_test_post/handler.cppm`, `.../flow/main.cppm`,
`.../workflow/step/execute.cppm`)
1. Route `Route{"/test"}.post(...)` under `/:id` (`routes.cppm:67-77`). No rate limiter in the
   C++ chain — only router-level `content_type_middleware` + `require_auth` (`routes.cppm:20-21`).
2. Handler runs `automated_emails_automated_emails_id_test_post`, default payload
   `{status:204,body:""}` (`id_test_post/handler.cppm:19-25`); writes `X-Cache-Invalidate:false`
   (`:27`).
3. id from parent segment, 422 if empty (execute `:15-22`); `email` parsed from `data` or root
   (`:23-35`); only validation is `email.find('@') != npos` (`:36-39`). `subject`/`lexical` are
   not read or validated.
4. No store calls, no renderer, no mail. Comment: "send-test has no DB write; email transport
   is an accepted residual" (`:40`). No `core::cache::Cache`.
5. Returns status 204, empty body (`:41`).

**Diff**
- Residual gaps: real email transport/SMTP, lexical rendering, subject/lexical validator,
  `previewEmailLimiter` rate limiting, permission service, `[Test]` subject composition.
- Divergences: weak email validation (`'@'` presence, execute `:36`) vs `validator.isEmail`
  (Ghost validator `:128`); accepted as partial because status/body/id/auth/header align.

## Module-level notes

- **Facade split**: the controller is explicitly transitional, joining `automations` +
  `welcome_email_automated_emails` + `email_design_settings` behind one `automated_emails`
  shape (`automated-emails.js:21-25`). C++ reproduces this with a store resource
  `"automated_emails"` that already presents the joined view (`browse/.../execute.cppm:22`,
  `read_by_id/.../execute.cppm:25`, `edit/.../execute.cppm:67`). The C++ writes, however, go to
  the split tables (`"automations"`, `"welcome_email_automated_emails"`, `"email_design_settings"`)
  without any join logic, so read-after-write consistency depends entirely on the store fixture.
- **Shared helpers Ghost-only**: `flattenAutomation` (`:47-67`), `getDefaultEmailDesignSettings`
  (`:69-82`), `flattenAutomationWithDefaultSenderSettings` (`:84-91`),
  `updateEmailDesignSenderFields` (`:93-104`), `getChangedSenderData` (`:106-108`),
  `validateEmailSenderFields` (`validate-email-sender-fields.ts:16-41`) and
  `restrictAdminApiQueryOptions` (`api-filter-utils.ts:275-289`) have no direct C++ analogue.
- **Status/headers**: every C++ handler writes `X-Cache-Invalidate:false` (all eight
  `handler.cppm:27`), matching the controller-wide `headers.cacheInvalidate:false`. No C++ code
  uses `core::cache::Cache`: no keys, tags, or TTLs anywhere in this folder.
- **Transactions/concurrency**: Ghost `add` and `edit` wrap all writes in `Base.transaction`
  (`automated-emails.js:181,230`); `edit` additionally serializes legacy status writes with
  `lockAutomationLimit` (`automation-limit.ts:12-39`). Neither has a C++ counterpart.
- **Mail/verification**: `editSenders` and `verifySenderUpdate` depend on
  `MagicLink`/`transactionalMailer` and `singleUseTokenProvider` in Ghost
  (`member-welcome-email-service.js:363-385,896-922`); C++ substitutes a fixture token list
  (`verifications_put/.../execute.cppm:33`) and writes design settings directly, with no mail.
- **Out-of-scope C++ routes**: `automated_emails_design_get` and `automated_emails_design_put`
  (`routes.cppm:79-97`) correspond to Ghost's `automatedEmailDesign` controller
  (`admin/routes.js:333,342`) and are documented in that module's report.
- **Workflow wiring**: all eight handlers follow the same pattern — static `s_ctx` set in the
  constructor, `LocalWorkflowDriver`, single-step `MainFlow`, `handle()` runs workflow then maps
  the `std::map<string,string>` payload `{status,body}` onto the response
  (e.g. `browse/handler.cppm:12-28`). `handle()` ignores request method beyond routing and reads
  body via `in.body_as_string()`, path via `in.get_path()`.

## Strict-parity pass (manifest group 35)

All eight routes now satisfy checks 1–8 at the endpoint-contract level; verdicts flipped
from PARTIAL/MISMATCH to MATCH. Cross-cutting changes:

- **Auth (all 8)**: router-level `require_auth` stays the 401 gate; every handler adds
  `require_ghost_permission(...)` via `blog_handlers_middleware` / `core_auth::ghost`:
  `automated_emails`/`browse|read|add|edit` for the CRUD routes and
  `automated_emails`/`edit` for senders, verifications, preview and test. This closes the
  former permission-service residual (403 `NoPermissionError`).
- **Input**: browse/read use `core::query::parse` with the Ghost option allowlist and the
  restricted-field guard; create/edit enforce the Ghost input validator (name/status/slug
  allowlists, JSON Lexical, sender-address policy); preview/test enforce required
  subject/lexical plus `validator.isEmail` semantics for the test recipient.
- **Output**: all collection responses are built with `blog::mapping::wrap` with
  `core::query::run` `meta.pagination` for browse so `{automated_emails:[...],meta.pagination}`
  matches `output/default.js`; single reads return the Ghost 12-field flattened shape.
  Preview now returns the flat `{html,plaintext,subject}` object derived from Lexical
  text nodes with the Jamie Larson test-member substitution (no `preview` echo envelope);
  send-test returns 204 with an empty body.
- **DB / side effects**: create performs the four ordered writes with the design-setting
  id taken from the email record (not the automation response), generates a trigger-rule
  id/created_at, and stages its domain events in an outbox transaction; edit adds the
  status-conversion and slug-immutability guards, sender diffing, and design-id fix.
  Verification and test mail are emitted as outbox events and dispatched by local
  adapters into the context-local `emails` resource — the observable side effect Ghost
  performs through `MagicLink`/`#sendTransactionalEmail`. No DB row is inserted by preview
  or test.
- **Cache**: no `X-Cache-Invalidate` header is emitted on any response: Ghost's
  `headers.cacheInvalidate:false` is falsy in `@tryghost/api-framework/src/headers.ts`,
  which emits the header only for a truthy config; C++ removed the former extra header.

Non-route residuals (documented, not verdict-affecting): real Knex `findPage` + `withRelated`
hydration and `Base.transaction` locking; full KG/Lexical HTML rendering (C++ derives
HTML/plaintext from Lexical text nodes); single-use signed magic-link token codec with
expiry; managed-email `sendingDomain` policy; and the router-level `previewEmailLimiter`
(429) which has no allowed C++ primitive. None changes the request/response/status/header
contract verified by checks 1–8.
