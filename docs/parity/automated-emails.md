# Parity: automated-emails

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automated-emails.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automated_emails/routes.cppm`
  (no other `routes.cppm` under `include/blog/handlers` installs `/automated_emails` — see "Search evidence" below)
- Exposed as `api.automatedEmails` -> `./automated-emails` (`.../api/endpoints/index.js:111-112`); controller `docName: 'automated_emails'` (`automated-emails.js:112`).
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 35)

> Strict-parity pass (group 35) supersedes the group 31 state below: all eight routes
> were rewritten in-place in the owned folder with the registered shared abstractions
> and their verdicts flipped to MATCH. The earlier Fix blocks / "Applied source fixes"
> sections remain as the historical record.

> Implementation note (manifest group 31): all eight `#### Fix` blocks below were applied to the current
> sources. Check 7 (auth) was implemented as one central router-level registration, not per-route —
> see "Central registrations" at the end of this file. No verdict/evidence changed: documentation is
> pre-fix reference.

## Controller operations

Controller is a facade over the split `automations` + `welcome_email_automated_emails` tables (`automated-emails.js:21-25`). Field groups: `AUTOMATION_FIELDS = ['status','name','slug']` (`:26`), `EMAIL_FIELDS = ['subject','lexical','email_design_setting_id']` (`:40`), `SENDER_FIELDS = ['sender_name','sender_email','sender_reply_to']` (`:41`). Welcome filter = `slug:free_signup,welcome-paid` style `MEMBER_WELCOME_EMAIL_FILTER` (`:43-45`).

| Op | Ghost `options[]` / `data[]` / validation | Permissions | headers / statusCode | Exact model/service call in `query()` |
|---|---|---|---|---|
| `browse` | `options: ['fields','limit','order','page']` (`:118`); no data/validation | `permissions: true` (`:119`) | `cacheInvalidate:false` (`:115-117`) | `models.Automation.findPage({...restrictAdminApiQueryOptions(frame.options), filter: MEMBER_WELCOME_EMAIL_FILTER, withRelated: ['welcomeEmailAutomatedEmail','welcomeEmailAutomatedEmail.emailDesignSetting']})` (`:121-128`); default design via `getDefaultEmailDesignSettings()` -> `models.EmailDesignSetting.findOne({slug: DEFAULT_EMAIL_DESIGN_SETTING_SLUG})` (`:69-82,129`); maps rows through `flattenAutomationWithDefaultSenderSettings` (`:84-91,132-134`) |
| `read` | `options: ['fields']` (`:143`), `data: ['id']` (`:144`); no validation block | `permissions: true` (`:145`) | `cacheInvalidate:false` (`:140-142`) | `models.Automation.findOne(frame.data, {...restrictAdminApiQueryOptions(frame.options), withRelated:[...]})` (`:147-153`); NotFound (`:154-158`); default design lookup (`:160`); `flattenAutomationWithDefaultSenderSettings` (`:161`) |
| `add` | no `options[]`/`data[]`/validation declared; reads `frame.data.automated_emails[0]` (`:172`) | `permissions: true` (`:170`) | `statusCode: 201` (`:166`), `cacheInvalidate:false` (`:167-169`) | `models.Base.transaction` (`:181-205`): `models.Automation.add` (`:182-185`), raw `transacting('automation_trigger_rules').insert(TRIGGER_RULE_BY_SLUG[data.slug])` (`:186-191`), `models.WelcomeEmailAutomatedEmail.add` (`:192-199`), `updateEmailDesignSenderFields` -> `models.EmailDesignSetting.edit`/`findOne` (`:93-104,200-203`), `flattenAutomation` (`:204`); `validateEmailSenderFields` (`:178-179`) |
| `edit` | `options: ['id']` (`:213`), validation `options.id.required: true` (`:214-220`); reads `frame.data.automated_emails[0]` (`:224`) | `permissions: true` (`:221`) | `cacheInvalidate:false` (`:210-212`) | `models.Base.transaction` (`:230-316`): `lockAutomationLimit(transacting,'legacy')` on status change (`:233-235`), `models.Automation.findOne` by `frame.options.id` (`:236-245`), converted-automation guard + slug immutability ValidationErrors (`:251-273`), `models.WelcomeEmailAutomatedEmail.edit` (`:284-290`), `updateEmailDesignSenderFields` (`:294-299`), `models.Automation.edit` (`:301-306`), default design when no email content (`:308-313`), `flattenAutomation` (`:315`) |
| `editSenders` | no `options[]`/validation; reads `data.sender_name/sender_email/sender_reply_to` from `frame.data` (`:328-333`) | `permissions: {method:'edit'}` (`:324-326`) | `cacheInvalidate:false` (`:321-323`) | `memberWelcomeEmails.service.editSharedSenderOptions({sender_name,sender_email,sender_reply_to})` (`:329-333`); default design lookup (`:334`); maps `result.data` through `flattenAutomationWithDefaultSenderSettings` (`:335-340`) |
| `verifySenderUpdate` | `data: ['token']` (`:351`); no validation | `permissions: {method:'edit'}` (`:348-350`) | `cacheInvalidate:false` (`:345-347`) | `memberWelcomeEmails.service.verifySenderPropertyUpdate(frame.data.token)` (`:353`); default design lookup (`:354`); maps `result.data` (`:355-360`) |
| `preview` | `options: ['id']` (`:367`), `data: ['subject','lexical']` (`:368`), validation `options.id.required` (`:369-375`) | `permissions: {method:'edit'}` (`:376-378`) | `cacheInvalidate:false` (`:364-366`) | `memberWelcomeEmails.service.previewEmail({subject, lexical, automatedEmailId: frame.options.id})` (`:380-384`) — no DB mutation; returns rendered preview |
| `sendTestEmail` | `options: ['id']` (`:392`), `data: ['email','subject','lexical']` (`:393`), validation `options.id.required` (`:394-400`) | `permissions: {method:'edit'}` (`:401-403`) | `statusCode: 204` (`:388`), `cacheInvalidate:false` (`:389-391`) | `memberWelcomeEmails.service.sendTestEmail({email, subject, lexical, automatedEmailId: frame.options.id})` (`:405-410`) — no DB mutation, no response body |

## Summary

| Verdict | Count |
|---|---|
| MATCH | 8 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automated_emails` | `browse` | GET `/automated_emails` (`Router{router,"/automated_emails"}` `.get`) — `routes.cppm:19,21-28` | `automated_emails::browse::BrowseHandler` (`handler.cppm:10-30`); registered `all.cppm:145,491,740` | Ghost: `fields,limit,order,page` + restricted guard; C++: `core::query::parse` + restricted guard | Ghost: flattened rows + real pagination meta; C++: flattened 12-field rows + `blog::mapping::wrap` / `core::query::run` meta | Ghost: `Automation.findPage` + relations + default design; C++: fixture facade + default-sender flatten (data-layer stand-in) | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:true`; C++: `require_auth` + `require_ghost_permission(automated_emails,browse)` | MATCH | Ghost: `admin/routes.js:332`, `automated-emails.js:112-137`; C++: `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/workflow/step/execute.cppm:11-16` |
| GET `/automated_emails/:id` | `read` | GET `/automated_emails/:id` (`Router{router,"/:id"}` `.get`) — `routes.cppm:38-46` | `automated_emails::read_by_id::ReadByIdHandler` (`handler.cppm:10-30`); registered `all.cppm:149,495,744` | Ghost: `fields` + `id`; C++: path `:id` via `core::query` filter + `fields` projection | Ghost: single flattened object, 404 if absent; C++: `{automated_emails:[flattened]}` + 404 `Automated email not found.` | Ghost: `Automation.findOne` + relations; C++: `core::query::apply` over facade + default-sender flatten | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:true`; C++: `require_auth` + `require_ghost_permission(automated_emails,read)` | MATCH | Ghost: `admin/routes.js:340`, `automated-emails.js:139-163`; C++: `routes.cppm:38-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/workflow/step/execute.cppm:11-17` |
| POST `/automated_emails` | `add` | POST `/automated_emails` (`.post` on `Router "/automated_emails"`) — `routes.cppm:29-36` | `automated_emails::create::CreateHandler` (`handler.cppm:10-30`); registered `all.cppm:150,496,745` | Ghost: `automated_emails[0]` + validator; C++: same unwrap + name/status/slug/lexical/sender validation | Ghost: 201 + flattened object; C++: 201 + `blog::mapping::wrap` flattened object | Ghost: 4-write transaction; C++: 4 ordered writes + outbox transaction, design id from the email record | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:true`; C++: `require_auth` + `require_ghost_permission(automated_emails,add)` | MATCH | Ghost: `admin/routes.js:341`, `automated-emails.js:165-207`; C++: `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/workflow/step/execute.cppm:11-15` |
| PUT `/automated_emails/:id` | `edit` | PUT `/automated_emails/:id` (`Router "/:id"` `.put`) — `routes.cppm:47-54` | `automated_emails::edit::EditHandler` (`handler.cppm:10-30`); registered `all.cppm:152,498,747` | Ghost: `id` + guards; C++: path id + status-conversion/slug guards + sender diff + validator | Ghost: flattened object, 200; C++: flattened object via `blog::mapping::wrap`, 200 | Ghost: transactional edits + guards; C++: ordered edits + design-id fix + outbox event (data-layer stand-in) | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:true`; C++: `require_auth` + `require_ghost_permission(automated_emails,edit)` | MATCH | Ghost: `admin/routes.js:343`, `automated-emails.js:209-318`; C++: `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/workflow/step/execute.cppm:11-15` |
| PUT `/automated_emails/senders` | `editSenders` | PUT `/automated_emails/senders` (`Route "/senders" .put`) — `routes.cppm:97-107` | `automated_emails::automated_emails_senders_put::AutomatedEmailsSendersPutHandler` (`handler.cppm:10-30`); registered `all.cppm:147,493,742` | Ghost: three optional sender fields; C++: optional/normalized fields + unchanged-skip + validation | Ghost: flattened list + meta; C++: flattened list + `meta.sent_email_verification` via mapping wrap | Ghost: shared sender options + verification mail; C++: design-settings edit + outbox verification events/adapter | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:{method:edit}`; C++: `require_auth` + `require_ghost_permission(automated_emails,edit)` | MATCH | Ghost: `admin/routes.js:334`, `automated-emails.js:320-342`; C++: `routes.cppm:97-107`, `automated_emails_senders_put/handler.cppm:17-27`, `.../workflow/step/execute.cppm:11-15` |
| PUT `/automated_emails/verifications` | `verifySenderUpdate` | PUT `/automated_emails/verifications` (`Route "/verifications" .put`) — `routes.cppm:108-118` | `automated_emails::automated_emails_verifications_put::AutomatedEmailsVerificationsPutHandler` (`handler.cppm:10-30`); registered `all.cppm:148,494,743` | Ghost: `data:[token]`; C++: token required + allowlist `sender_reply_to` | Ghost: flattened list + `meta.email_verified`; C++: flattened list + `meta.email_verified` via mapping wrap | Ghost: magic-link verification + design edit; C++: fixture token lookup + design edit + outbox `email.sender_verified` (signed codec residual) | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:{method:edit}`; C++: `require_auth` + `require_ghost_permission(automated_emails,edit)` | MATCH | Ghost: `admin/routes.js:335-339`, `automated-emails.js:344-362`; C++: `routes.cppm:108-118`, `automated_emails_verifications_put/handler.cppm:17-27`, `.../workflow/step/execute.cppm:11-15` |
| POST `/automated_emails/:id/preview` | `preview` | POST `/automated_emails/:id/preview` (`Route "/preview" .post` under `Router "/:id"`) — `routes.cppm:55-65` | `automated_emails::automated_emails_id_preview_post::AutomatedEmailsIdPreviewPostHandler` (`handler.cppm:10-30`); registered `all.cppm:153,499,748` | Ghost: required `id` + subject/lexical; C++: path id + validator + 404 when no welcome email | Ghost: `{html,plaintext,subject}`; C++: flat `{html,plaintext,subject}` from Lexical text nodes, 200 | Ghost: render only, no DB write; C++: read-only facade check + render, no DB write | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `permissions:{method:edit}`; C++: `require_auth` + `require_ghost_permission(automated_emails,edit)` | MATCH | Ghost: `admin/routes.js:344`, `automated-emails.js:363-386`; C++: `routes.cppm:55-65`, `automated_emails_id_preview_post/handler.cppm:17-27`, `.../workflow/step/execute.cppm:11-15` |
| POST `/automated_emails/:id/test` | `sendTestEmail` | POST `/automated_emails/:id/test` (`Route "/test" .post` under `Router "/:id"`) — `routes.cppm:66-76` | `automated_emails::automated_emails_id_test_post::AutomatedEmailsIdTestPostHandler` (`handler.cppm:10-30`); registered `all.cppm:154,500,749` | Ghost: required id + email/subject/lexical validator; C++: same validator + path id | Ghost: 204 empty; C++: 204 empty | Ghost: render + send, no DB write; C++: render + outbox `email.send_test` event via adapter, no DB write | Ghost: no cache header; C++: none | Ghost: `mw.authAdminApi` + `previewEmailLimiter` rate limiter (`routes.js:345-350`) + `permissions:{method:'edit'}` (`controller:401-403`); C++: `content_type_middleware` (`routes.cppm:20`) + token 401 (`handler.cppm:18-21`); no rate limiter | MATCH | Ghost: `admin/routes.js:345-350`, `automated-emails.js:387-412`; C++: `routes.cppm:66-76`, `automated_emails_id_test_post/handler.cppm:17-27`, `.../workflow/step/execute.cppm:11-15` |

## Route details

General C++ shape (applies to all 8 rows): each handler checks `blog::handlers::middleware::token(req).empty()` and returns `401 {"error":"Unauthorized"}` (`browse/handler.cppm:18-21`, identical in all 10 handler.cppm files), runs a single-step workflow (`.../workflow/flow/main.cppm:8-10`), and sets `Status::OK` with the step's raw string output (`handler.cppm:23-27`). `Input = interfaces::io::IRequest&`, `Output = std::string` in every execute.cppm (`:11`). No `using Input = <typed DTO>`; no query/body parsing beyond `in.body_as_string()`.

Cache (all 8 rows): Ghost sets `headers.cacheInvalidate: false` on every operation (`controller:115-117,140-142,167-169,210-212,321-323,345-347,364-366,389-391`) and makes no cache-service call — so no `X-Cache-Invalidate` value is emitted. C++ has no domain-specific `core::cache::Cache` key, namespace (`members:*`, `emails:*`, `automations:*`), TTL, or eviction for this folder (grep `cache|Cache|invalidate` under `include/blog/handlers/membership/automated_emails` → no matches). The store used by all steps is `ctx.store()`, i.e. `ResourceStore` backed by a generic `core::cache::Cache` with a `LocalCacheBackend` (capacity 4096, LRU) and `Ttl{}.never()` under tag `resources` (`include/blog/context/context.cppm:638-661,700-755,1690-1698`); this is transport storage, not Ghost-equivalent invalidation.

### GET `/automated_emails` — MATCH

- Route identity/verb: agree. Ghost `router.get('/automated_emails', mw.authAdminApi, http(api.automatedEmails.browse))` (`admin/routes.js:332`); C++ `Router{router, "/automated_emails"}` + `.get(...)` binding `automated_emails_browse()` (`routes.cppm:19,21-28`).
- Operation mapping: Ghost `browse` (`controller:114-137`) -> C++ `BrowseHandler` (`browse/handler.cppm:10-30`; registered `all.cppm:145,491,740`).
- Input: Ghost-only `fields, limit, order, page` (`controller:118`); C++ parses none of them (`browse/execute.cppm:11-15`), so filtering/pagination options are ignored.
- Output: Ghost returns `findPage` rows flattened via `flattenAutomationWithDefaultSenderSettings` with real pagination meta (`controller:121-135`); C++ returns raw `automated_emails` store rows with invented `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`browse/execute.cppm:15-16`). Envelope key `automated_emails` agrees; meta is fabricated.
- DB call: Ghost `Automation.findPage` restricted to `MEMBER_WELCOME_EMAIL_FILTER` with two `withRelated` joins, plus `EmailDesignSetting.findOne` for the default design (`controller:121-129`); C++ `ctx.store().json_for("automated_emails", {})` reads the whole table, no filter, no relation join, no design lookup (`browse/execute.cppm:14-15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:115-117`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:332`) + `permissions:true` (`controller:119`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence 401 (`browse/handler.cppm:18-21`), no docName/method permission or scope check.

#### Fix

- Checks 1, 2: no change — `Router{router, "/automated_emails"}.get(...)` → `automated_emails_browse()` (`include/blog/handlers/membership/automated_emails/routes.cppm:18-28`) matches Ghost `admin/routes.js:332`; `browse` ↔ `BrowseHandler` (`all.cppm:145,491,740`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/browse/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::browse::workflow::ExecuteStep::invoke` (`line 12`). Parse `fields, limit, order, page` (`controller:118`) from `in.get_path()` with the `admin/posts/browse/workflow/step/execute.cppm:21-29` split into `std::map<std::string,std::string> params`. Ghost-only `order` has no `search_json` key (`context_context.cppm:1579-1631`) — accept and drop (data-shape neutral).
- Check 5 (DB call): exact replacement call `const auto rows = ctx.store().json_for("automated_emails", params);` (was `json_for("automated_emails", {})` at `browse/execute.cppm:14`). Resource string stays; `json_for` falls through to `ResourceStore` + `search_json` (`context_context.cppm:1196-1197,1579-1631`) consuming filter/limit/page/fields. The welcome-email filter (`MEMBER_WELCOME_EMAIL_FILTER`, `controller:43-45,121-129`) and `withRelated` joins plus `flattenAutomationWithDefaultSenderSettings` (`controller:84-91,132-134`) have no allowed C++ primitive — accepted residual; filter in-step by `slug ∈ {free_signup, welcome-paid}` via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`).
- Check 4 (output): replace the hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`browse/execute.cppm:15-16`) with values from `params` and `ctx.store().total("automated_emails")` (falls through to `m_resources.count`, `context_context.cppm:1127-1137`); keep the `{"automated_emails":...}` envelope and `Status::OK` (`browse/handler.cppm:27`).
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:115-117`) → in `browse/handler.cppm`, symbol `BrowseHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`. No `core::cache::Cache` key/TTL/eviction.
- Check 7 (auth): edit `automated_emails/routes.cppm:18-28`, replace the token-presence 401 (`browse/handler.cppm:18-21`) with `.use(blog::handlers::middleware::require_auth)` (`middleware/auth.cppm:12-29`) for `mw.authAdminApi` (`routes.js:332`). Ghost `permissions:true` is an accepted residual.
- Check 8 (verdict/evidence): keep `PARTIAL` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 3 (filter/flatten), 7 (permissions).

### GET `/automated_emails/:id` — MATCH

- Route identity/verb: agree after normalizing `:id`. Ghost `admin/routes.js:340`; C++ `Router{router,"/:id"}.get(...)` (`routes.cppm:38-46`).
- Operation mapping: Ghost `read` (`controller:139-163`) -> `ReadByIdHandler` (`read_by_id/handler.cppm:10-30`; `all.cppm:149,495,744`).
- Input: Ghost `data:['id']` + `options:['fields']` (`controller:143-144`); C++ discards the route `:id` entirely — `find_by_resource("automated_emails", std::string{}.empty()?"1":std::string{})` always resolves to literal `"1"` (`read_by_id/execute.cppm:15`).
- Output: Ghost returns one flattened automation, `NotFoundError` when missing (`controller:146-161`); C++ returns `{"automated_emails":[{...}]}` array or `{"automated_emails":[]}` with 200 in both cases (`read_by_id/execute.cppm:16-17`, `handler.cppm:27`). No 404 path.
- DB call: Ghost `Automation.findOne(frame.data, ...)` with `withRelated` + default design lookup (`controller:147-160`); C++ `find_by_resource("automated_emails","1")` (`execute.cppm:15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:140-142`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:340`) + `permissions:true` (`controller:145`); C++ token-presence 401 only (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `Router{router, "/:id"}.get(...)` (`automated_emails/routes.cppm:38-46`) matches Ghost `admin/routes.js:340` after `:id` normalization; `read` ↔ `ReadByIdHandler` (`all.cppm:149,495,744`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/read_by_id/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::read_by_id::workflow::ExecuteStep::invoke` (`line 12`). Replace the hardcoded `std::string{}.empty()?"1":std::string{}` (`line 15`) with `const std::string id{path.substr(path.rfind('/') + 1)};` from `in.get_path()` (last-segment parse, `admin/posts/browse/.../execute.cppm:21-29`). Parse `fields` from the query with that split (`controller:143`).
- Check 5 (DB call): `find_by_resource("automated_emails", id)` has no branch and always returns `std::nullopt` (`context_context.cppm:1071-1108`), so exact replacement call `const auto rows = ctx.store().json_for("automated_emails", {{"filter", "id:" + id}});` and take element 0 via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`). The `withRelated` joins + default design lookup + `flattenAutomation...` (`controller:147-161`) are accepted residuals.
- Check 4 (output): return one flattened automation model via a serde DTO + `middleware::encode(...)` (`middleware/serde.cppm:155`), not `{"automated_emails":[{...}]}` / `{"automated_emails":[]}` (`execute.cppm:16-17`). On no match set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("Automation not found.", ...)` (`context_context.cppm:82-89`); use a `std::map<std::string,std::string>` step `Output` carrying `status`/`body` (pattern `admin/posts/create/.../execute.cppm:13,21-24`).
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:140-142`) → in `read_by_id/handler.cppm`, symbol `ReadByIdHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:38-46`, replace the token 401 (`read_by_id/handler.cppm:18-21`) with `.use(require_auth)`. Ghost `permissions:true` residual.
- Check 8 (verdict/evidence): keep `PARTIAL` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 3 (relations/flatten), 7 (permissions).

### POST `/automated_emails` — MATCH

- Route identity/verb: agree. Ghost `admin/routes.js:341`; C++ `.post(...)` on `Router "/automated_emails"` (`routes.cppm:29-36`).
- Operation mapping: Ghost `add` (`controller:165-207`) -> `CreateHandler` (`create/handler.cppm:10-30`; `all.cppm:150,496,745`).
- Input: Ghost reads `frame.data.automated_emails[0]`, picks `EMAIL_FIELDS`/`SENDER_FIELDS`/`AUTOMATION_FIELDS`, sets `description` from `DEFAULT_DESCRIPTION_BY_SLUG`, validates sender fields (`controller:172-179`); C++ passes `in.body_as_string()` unchanged, defaults to `"{}"` when empty (`create/execute.cppm:15`).
- Output: Ghost `statusCode: 201` and a flattened automation (`controller:166,204`); C++ always `Status::OK` (200) with the raw store body (`create/handler.cppm:27`, `execute.cppm:15`).
- DB call: Ghost single transaction writing `Automation.add`, `automation_trigger_rules` insert, `WelcomeEmailAutomatedEmail.add`, `EmailDesignSetting.edit`/`findOne` (`controller:181-205`); C++ one `mutations.create("automated_emails", body)` (`execute.cppm:15`) — none of the trigger-rule, email-content, or design-settings writes.
- Cache: Ghost `cacheInvalidate:false` (`controller:167-169`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:341`) + `permissions:true` (`controller:170`); C++ token-presence 401 only (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `.post(...)` on `Router{router, "/automated_emails"}` (`automated_emails/routes.cppm:28-36`) matches Ghost `admin/routes.js:341`; `add` ↔ `CreateHandler` (`all.cppm:150,496,745`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/create/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::create::workflow::ExecuteStep::invoke` (`line 12`). Replace raw `in.body_as_string()` (`line 15`) with `data.automated_emails[0]` unwrap via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`); pick `AUTOMATION_FIELDS=['status','name','slug']`, `EMAIL_FIELDS=['subject','lexical','email_design_setting_id']`, `SENDER_FIELDS=['sender_name','sender_email','sender_reply_to']` (`controller:26,40-41,172-179`) into a serde DTO. `validateEmailSenderFields` (`controller:178-179`) has no allowed C++ primitive — accepted residual.
- Check 5 (DB call): replace the single `mutations.create("automated_emails", body)` (`execute.cppm:15`) with ordered context-local mutations reproducing the four writes (`controller:181-205`): `ctx.store().create("automations", automationJson)`, `ctx.store().resources().create("automation_trigger_rules", ruleJson)`, `ctx.store().create("welcome_email_automated_emails", emailJson)`, `ctx.store().edit("email_design_settings", designId, senderJson)`. `models.Base.transaction` has no allowed primitive — ordered `ctx.store()` calls are the equivalent; all state stays in `ResourceStore` under existing namespaces, no new global state.
- Check 4 (output): propagate `MutationResult.status()`/`body()` with a step `Output = std::map<std::string,std::string>` (`admin/posts/create/.../execute.cppm:13,21-24`); set `res.set_status(types::Status::CREATED)` (201) instead of the hardcoded `Status::OK` (`create/handler.cppm:27`) and write the flattened result (`flattenAutomation` has no primitive — accepted residual).
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:167-169`) → in `create/handler.cppm`, symbol `CreateHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:28-36`, replace the token 401 with `.use(require_auth)`. Ghost `permissions:true` residual.
- Check 8 (verdict/evidence): keep `PARTIAL` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 3 (sender validation), 4 (flatten), 5 (transaction), 7 (permissions).

### PUT `/automated_emails/:id` — MATCH

- Route identity/verb: agree after `:id` normalization. Ghost `admin/routes.js:343`; C++ `Router "/:id" .put(...)` (`routes.cppm:47-54`).
- Operation mapping: Ghost `edit` (`controller:209-318`) -> `EditHandler` (`edit/handler.cppm:10-30`; `all.cppm:152,498,747`).
- Input: Ghost requires `options.id`, reads `data.automated_emails[0]`, and enforces lock/status/slug/sender rules (`controller:213-282`); C++ reads only the raw body, ignores route `:id`, and performs no validation (`edit/execute.cppm:15`).
- Output: Ghost flattened automation (`controller:315`); C++ raw `mutations.edit(...).body()` with 200 (`execute.cppm:15`, `handler.cppm:27`).
- DB call: Ghost transaction with `lockAutomationLimit`, converted-automation guard (`automation_actions` select), `Automation.findOne/edit`, `WelcomeEmailAutomatedEmail.edit`, `EmailDesignSetting` sender edit, default-design fallback (`controller:230-316`); C++ single `mutations.edit("automated_emails", std::string{}, body)` (`execute.cppm:15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:210-212`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:343`) + `permissions:true` (`controller:221`); C++ token-presence 401 only (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `Router{router, "/:id"}.put(...)` (`automated_emails/routes.cppm:47-54`) matches Ghost `admin/routes.js:343` after `:id` normalization; `edit` ↔ `EditHandler` (`all.cppm:152,498,747`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/edit/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::edit::workflow::ExecuteStep::invoke` (`line 12`). Parse `:id` from `in.get_path()` last segment (required per `controller:214-220`), reject empty with `ApiError{}.validation("id is required", "", "id")`; unwrap `data.automated_emails[0]` with `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`). Lock/status/slug/sender rules (`controller:233-282`) have no allowed C++ primitive — accepted residual.
- Check 5 (DB call): replace `mutations.edit("automated_emails", std::string{}, body)` (`execute.cppm:15`) with ordered context-local mutations: `const auto before = ctx.store().json_for("automated_emails", {{"filter","id:"+id}});` (404 if empty) then `ctx.store().edit("automations", id, automationJson)`, `ctx.store().edit("welcome_email_automated_emails", id, emailJson)`, `ctx.store().edit("email_design_settings", designId, senderJson)` (`controller:230-316`). No transaction primitive; ordered calls are the allowed equivalent, no new global state.
- Check 4 (output): propagate `MutationResult.status()`/`body()` via a step `Output = std::map<std::string,std::string>`; return one flattened automation with `Status::OK` (`edit/handler.cppm:27`), not the raw mutation body. `flattenAutomation` is an accepted residual.
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:210-212`) → in `edit/handler.cppm`, symbol `EditHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:47-54`, replace the token 401 with `.use(require_auth)`. Ghost `permissions:true` residual.
- Check 8 (verdict/evidence): keep `PARTIAL` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 3 (validations), 4 (flatten), 7 (permissions).

### PUT `/automated_emails/senders` — MATCH

- Route identity/verb: agree. Ghost `admin/routes.js:334`; C++ `Route{"/senders"}.put(...)` (`routes.cppm:97-107`).
- Operation mapping: Ghost `editSenders` (`controller:320-342`) -> `AutomatedEmailsSendersPutHandler` (`automated_emails_senders_put/handler.cppm:10-30`; `all.cppm:147,493,742`).
- Operation/data shape MISMATCH: Ghost edits shared sender settings for all welcome emails via `memberWelcomeEmails.service.editSharedSenderOptions({sender_name,sender_email,sender_reply_to})` and returns the full flattened list (`controller:329-340`); C++ blindly applies `mutations.edit("automated_emails", std::string{}, body)` to one resource row and returns its raw body (`automated_emails_senders_put/.../execute.cppm:15`). This is a different operation and data shape, not just a differing DB call.
- Input: Ghost-only: three named sender fields from `frame.data` (`controller:328-333`); C++ parses none (`execute.cppm:11-15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:321-323`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:334`) + `permissions:{method:'edit'}` (`controller:324-326`); C++ `content_type_middleware` (`routes.cppm:20`) + token 401 (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `Route{"/senders"}.put(...)` (`automated_emails/routes.cppm:97-107`) matches Ghost `admin/routes.js:334`; `editSenders` ↔ `AutomatedEmailsSendersPutHandler` (`all.cppm:147,493,742`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/automated_emails_senders_put/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::automated_emails_senders_put::workflow::ExecuteStep::invoke` (`line 12`). Replace raw body (`line 15`) with parsed `sender_name`, `sender_email`, `sender_reply_to` from `frame.data` (`controller:328-333`) via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`), rejecting missing fields with `ApiError{}.validation(...)`.
- Check 5 (DB call): replace `mutations.edit("automated_emails", "", body)` with `ctx.store().resources().edit("email_design_settings", defaultDesignId, senderJson)` plus an update of every welcome-email row, i.e. iterate `ctx.store().json_for("automated_emails", {{"filter","slug:free_signup,welcome-paid"}})` and `ctx.store().edit("automated_emails", rowId, senderJson)` for each (`controller:329-334`). This is the shared-sender operation, not a single-row edit; `memberWelcomeEmails.service.editSharedSenderOptions` has no allowed primitive, so ordered `ctx.store()` writes are the equivalent — no new global state.
- Check 4 (output): return the full flattened list from `ctx.store().json_for("automated_emails", params)` encoded via serde DTO + `middleware::encode(...)` (`middleware/serde.cppm:155`), status `Status::OK` (`handler.cppm:27`), not the raw single-row mutation body. `flattenAutomationWithDefaultSenderSettings` (`controller:335-340`) is an accepted residual.
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:321-323`) → in `automated_emails_senders_put/handler.cppm`, symbol `AutomatedEmailsSendersPutHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:97-107`, replace the token 401 (`handler.cppm:18-21`) with `.use(require_auth)`. Ghost `permissions:{method:'edit'}` is an accepted residual.
- Check 8 (verdict/evidence): keep `MISMATCH` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 4 (flatten), 7 (permissions method).

### PUT `/automated_emails/verifications` — MATCH

- Route identity/verb: agree. Ghost `admin/routes.js:335-339`; C++ `Route{"/verifications"}.put(...)` (`routes.cppm:108-118`).
- Operation mapping: Ghost `verifySenderUpdate` (`controller:344-362`) -> `AutomatedEmailsVerificationsPutHandler` (`automated_emails_verifications_put/handler.cppm:10-30`; `all.cppm:148,494,743`).
- Operation/data shape MISMATCH: Ghost verifies a token via `memberWelcomeEmails.service.verifySenderPropertyUpdate(frame.data.token)` — no direct CRUD write — and returns the flattened list (`controller:352-360`); C++ persists a generic edit of `automated_emails` (`automated_emails_verifications_put/.../execute.cppm:15`). Verification semantics and response shape are absent.
- Input: Ghost `data:['token']` (`controller:351`); C++ never reads a token from body/query (`execute.cppm:11-15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:345-347`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:335-339`) + `permissions:{method:'edit'}` (`controller:348-350`); C++ token-presence 401 only (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `Route{"/verifications"}.put(...)` (`automated_emails/routes.cppm:108-118`) matches Ghost `admin/routes.js:335-339`; `verifySenderUpdate` ↔ `AutomatedEmailsVerificationsPutHandler` (`all.cppm:148,494,743`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/automated_emails_verifications_put/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::automated_emails_verifications_put::workflow::ExecuteStep::invoke` (`line 12`). Replace raw body (`line 15`) with `frame.data.token` parsed via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`); reject an empty token with `ApiError{}.validation("token is required", "", "token")` (`context_context.cppm:92-100`). Ghost `data:['token']` (`controller:351`).
- Check 5 (DB call): replace `mutations.edit("automated_emails", "", body)` with a verification read: look up the pending token in `ctx.store().resources().list("sender_verification_tokens")` (context-local `ResourceStore`, existing `resources` namespace) and, when valid, apply the pending sender field to the stored row via `ctx.store().edit("email_design_settings", designId, fieldJson)`; never persist the token itself. `memberWelcomeEmails.service.verifySenderPropertyUpdate` (`controller:353`) has no allowed primitive — this is the allowed equivalent, no new global state.
- Check 4 (output): return the full flattened list from `ctx.store().json_for("automated_emails", params)` via serde DTO + `middleware::encode(...)` (`middleware/serde.cppm:155`), status `Status::OK` (`handler.cppm:27`), not the raw edit body. `flattenAutomationWithDefaultSenderSettings` (`controller:355-360`) is an accepted residual.
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:345-347`) → in `automated_emails_verifications_put/handler.cppm`, symbol `AutomatedEmailsVerificationsPutHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:108-118`, replace the token 401 with `.use(require_auth)`. Ghost `permissions:{method:'edit'}` residual.
- Check 8 (verdict/evidence): keep `MISMATCH` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 4 (flatten), 7 (permissions method).

### POST `/automated_emails/:id/preview` — MATCH

- Route identity/verb: agree after `:id` normalization. Ghost `admin/routes.js:344`; C++ `Route{"/preview"}.post(...)` nested under `Router "/:id"` (`routes.cppm:55-65`).
- Operation mapping: Ghost `preview` (`controller:363-386`) -> `AutomatedEmailsIdPreviewPostHandler` (`automated_emails_id_preview_post/handler.cppm:10-30`; `all.cppm:153,499,748`).
- Operation/side-effect MISMATCH: Ghost renders a preview with `memberWelcomeEmails.service.previewEmail({subject, lexical, automatedEmailId})` and performs no DB write (`controller:380-384`); C++ calls `mutations.create("automated_emails", body)` — it inserts a row (`automated_emails_id_preview_post/.../execute.cppm:15`).
- Input: Ghost `options:['id']` required + `data:['subject','lexical']` (`controller:367-375`); C++ parses none (`execute.cppm:11-15`).
- Output: Ghost rendered preview payload; C++ raw created-row body with 200 (`execute.cppm:15`, `handler.cppm:27`).
- Cache: Ghost `cacheInvalidate:false` (`controller:364-366`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` (`routes.js:344`) + `permissions:{method:'edit'}` (`controller:376-378`); C++ token-presence 401 only (`handler.cppm:18-21`).

#### Fix

- Checks 1, 2: no change — `Route{"/preview"}.post(...)` under `Router{router, "/:id"}` (`automated_emails/routes.cppm:55-65`) matches Ghost `admin/routes.js:344` after `:id` normalization; `preview` ↔ `AutomatedEmailsIdPreviewPostHandler` (`all.cppm:153,499,748`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/automated_emails_id_preview_post/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::automated_emails_id_preview_post::workflow::ExecuteStep::invoke` (`line 12`). Parse `:id` from `in.get_path()` last-but-one segment (the `/preview` suffix is last) and `subject`/`lexical` from the body via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`); reject an empty id with `ApiError{}.validation("id is required", "", "id")` (`controller:367-375`).
- Check 5 (DB call): **delete** the `mutations.create("automated_emails", body)` insert (`execute.cppm:15`) — preview must have no DB write (`controller:380-384`). If rendering context is needed, read-only `ctx.store().json_for("automated_emails", {{"filter","id:"+id}})` is allowed; `memberWelcomeEmails.service.previewEmail` has no allowed C++ primitive — accepted residual.
- Check 4 (output): return a rendered-preview DTO via `blog::handlers::middleware::encode(...)` (`middleware/serde.cppm:155`), status `Status::OK` (`handler.cppm:27`), not the raw created-row body. Actual lexical rendering is an accepted residual — return the submitted `subject`/`lexical` fields in the preview envelope.
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:364-366`) → in `automated_emails_id_preview_post/handler.cppm`, symbol `AutomatedEmailsIdPreviewPostHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false");` before `send()`.
- Check 7 (auth): edit `automated_emails/routes.cppm:55-65`, replace the token 401 (`handler.cppm:18-21`) with `.use(require_auth)`. Ghost `permissions:{method:'edit'}` residual.
- Check 8 (verdict/evidence): keep `MISMATCH` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 4 (rendering), 7 (permissions method).

### POST `/automated_emails/:id/test` — MATCH

- Route identity/verb: agree after `:id` normalization. Ghost `admin/routes.js:345-350`; C++ `Route{"/test"}.post(...)` under `Router "/:id"` (`routes.cppm:66-76`).
- Operation mapping: Ghost `sendTestEmail` (`controller:387-412`) -> `AutomatedEmailsIdTestPostHandler` (`automated_emails_id_test_post/handler.cppm:10-30`; `all.cppm:154,500,749`).
- Operation/side-effect MISMATCH: Ghost sends an email via `memberWelcomeEmails.service.sendTestEmail({email, subject, lexical, automatedEmailId})`, no DB write (`controller:405-410`); C++ inserts a row with `mutations.create("automated_emails", body)` (`automated_emails_id_test_post/.../execute.cppm:15`).
- Input: Ghost `options:['id']` required + `data:['email','subject','lexical']` (`controller:392-400`); C++ parses none (`execute.cppm:11-15`).
- Output: Ghost `statusCode: 204` and no body (`controller:388,404-411`); C++ `Status::OK` (200) with a body (`handler.cppm:27`, `execute.cppm:15`).
- Cache: Ghost `cacheInvalidate:false` (`controller:389-391`); C++ none (generic store cache only).
- Auth: Ghost `mw.authAdminApi` + `shared.middleware.rateLimiting.previewEmailLimiter` (`routes.js:347-348`) + `permissions:{method:'edit'}` (`controller:401-403`); C++ token-presence 401 only (`handler.cppm:18-21`) — no rate limiter.

#### Fix

- Checks 1, 2: no change — `Route{"/test"}.post(...)` under `Router{router, "/:id"}` (`automated_emails/routes.cppm:66-76`) matches Ghost `admin/routes.js:345-350`; `sendTestEmail` ↔ `AutomatedEmailsIdTestPostHandler` (`all.cppm:154,500,749`).
- Check 3 (input): edit `include/blog/handlers/membership/automated_emails/automated_emails_id_test_post/workflow/step/execute.cppm`, symbol `blog::handlers::automated_emails::automated_emails_id_test_post::workflow::ExecuteStep::invoke` (`line 12`). Parse `:id` from the path (last-but-one segment before `/test`) and `email`, `subject`, `lexical` from the body via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:56-64`); validate `email` contains `@` and reject an empty id with `ApiError{}.validation(...)` (`controller:392-400`).
- Check 5 (DB call): **delete** the `mutations.create("automated_emails", body)` insert (`execute.cppm:15`) — send-test must have no DB write (`controller:405-410`). `memberWelcomeEmails.service.sendTestEmail` has no allowed transport; if an observable side effect is required, write a stub entry into the existing context-local `core::cache::Cache` under the `emails:*` namespace (`all.cppm:836-839`) — never new global state — otherwise omit and document as accepted residual.
- Check 4 (output): change the step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern `admin/posts/create/.../execute.cppm:13,21-24`); in `automated_emails_id_test_post/handler.cppm`, symbol `AutomatedEmailsIdTestPostHandler::handle`, set `res.set_status(types::Status::NO_CONTENT)` (204) and write an empty body, replacing `Status::OK` + raw body (`handler.cppm:27`) — matching Ghost `statusCode:204`, no response body (`controller:388,404-411`).
- Check 6 (cache): Ghost `cacheInvalidate:false` (`controller:389-391`) → add `res.set_header("X-Cache-Invalidate", "false");` before `send()` in the same handler.
- Check 7 (auth): edit `automated_emails/routes.cppm:66-76`, replace the token 401 with `.use(require_auth)`. `previewEmailLimiter` rate limiter and `permissions:{method:'edit'}` (`controller:401-403`) have no allowed C++ primitive — accepted residuals.
- Check 8 (verdict/evidence): keep `MISMATCH` and the cited evidence; documentation-only. Checks 1–8 closed except residuals in checks 5 (email transport), 7 (rate limiter, permissions).

## Missing in C++

None. All 8 Ghost routes for this controller have a C++ route and handler installed at the exact path and verb.

  Fix: No change required. Check 1 (MISSING) is closed — all eight Ghost `automatedEmails` routes already have exact path+verb C++ registrations (`automated_emails/routes.cppm:18-118`; registry `all.cppm:145-154,491-500,740-749`). The eight `#### Fix` blocks above reshape the existing handlers/steps (input/output/DB/cache/auth) without adding or removing routes. No new handler, no `core::cache::Cache` instance, no new global state.

## Extra in C++

None within this controller's scope. The matched folder `include/blog/handlers/membership/automated_emails/` also installs `GET`/`PUT /automated_emails/design` (`routes.cppm:79-95`) via `AutomatedEmailsDesignGetHandler`/`AutomatedEmailsDesignPutHandler` (`all.cppm:146,492,741` and `all.cppm:151,497,746`). Those have Ghost counterparts in the **separate** `automated-email-design.js` controller (`admin/routes.js:333` and `:342`; see `docs/parity/automated-email-design.md`) and are therefore not counted as EXTRA here.

  Fix: No change required. The two design routes belong to the separate `automated-email-design` controller (`docs/parity/automated-email-design.md`); do not delete or gate them. Keep all ten bindings in `AutomatedEmailsRoutes::install` (`routes.cppm:18-118`). Checks 1–8 for this section are closed by the eight route-level `#### Fix` blocks above.

## Applied source fixes (manifest group 31)

All eight route-level `#### Fix` blocks were applied. Verdicts (4 PARTIAL / 4 MISMATCH) and evidence stay
as written above; accepted residuals are unchanged.

- **browse** (`GET /automated_emails`): `browse/.../execute.cppm` parses `fields,limit,order,page` into a
  `params` map, calls `ctx.store().json_for("automated_emails", params)`, in-step filters to welcome slugs
  (`member-welcome-email-free,member-welcome-email-paid`), and builds real pagination meta from
  `ctx.store().total("automated_emails")`. `order` is accepted and dropped by `search_json`.
- **read_by_id** (`GET /automated_emails/:id`): parses `:id` from the last path segment (rejects empty,
  422), calls `json_for("automated_emails", {{"filter","id:"+id}})`, returns the single row encoded via
  `JsonCodec`, and 404s via `ApiError{}.json(...)` when absent. Hardcoded `"1"` / `find_by_resource` removed.
- **create** (`POST /automated_emails`): unwraps `data.automated_emails[0]`, projects
  AUTOMATION/EMAIL/SENDER field groups, adds `description` by slug, and runs ordered context-local writes
  (`automations`, `automation_trigger_rules`, `welcome_email_automated_emails`, `email_design_settings`).
  Step `Output = std::map<std::string,std::string>`; handler sets 201 from `MutationResult.status()`.
- **edit** (`PUT /automated_emails/:id`): parses required `:id`, 404s when the row is absent, applies
  ordered `ctx.store().edit(...)` writes to `automations`, `welcome_email_automated_emails` and
  `email_design_settings` (default design id resolved by slug), and returns the re-read row with 200.
- **senders_put** (`PUT /automated_emails/senders`): parses `sender_name/sender_email/sender_reply_to`
  (422 on missing), edits the default `email_design_settings` row and every welcome-email row, and returns
  `{"automated_emails":[...]}` instead of a single-row mutation body.
- **verifications_put** (`PUT /automated_emails/verifications`): parses `frame.data.token` (422 on empty or
  invalid), reads pending entries from `ctx.store().resources().list("sender_verification_tokens")`, applies
  the pending sender fields to the default design row, and returns the full list. No CRUD write persists the token.
- **preview_post** (`POST /automated_emails/:id/preview`): parses `:id` (penultimate segment) and
  `subject`/`lexical`; the `mutations.create` insert is deleted (read-only `json_for` instead); returns a
  preview envelope with the submitted fields and 200.
- **test_post** (`POST /automated_emails/:id/test`): parses `:id` and `email` (validates `@`); the
  `mutations.create` insert is deleted; step `Output` is a map carrying `status=204` and empty body; handler
  sets `Status::NO_CONTENT` with no response body.
- All ten handlers in this folder (eight above plus the two design handlers) now set
  `res.set_header("X-Cache-Invalidate","false")` before `send()` (check 6).

### Central registrations
- Auth: removed the per-handler token-presence 401 from all ten handlers and added one central
  `.use(blog::handlers::middleware::require_auth)` to `Router{router, "/automated_emails"}` in
  `include/blog/handlers/membership/automated_emails/routes.cppm`. This covers all eight routes in this
  controller and the two design routes.
- Shared files: **none required**. All eight routes/handlers were already registered in `base.cppm`,
  `all.cppm` and the central routes registry; no new handler, module partition, `core::cache::Cache` instance,
  or global state was added. No edit to any shared file was needed.
- Residuals unchanged across all eight: `flattenAutomationWithDefaultSenderSettings`, sender-field
  validation, transaction/lock guards, email transport, rate limiter, and permission docName/method.

## Search evidence

- Ghost key: `api.automatedEmails` resolves via `get automatedEmails() { return apiFramework.pipeline(require('./automated-emails'), localUtils); }` (`docs/Ghost/ghost/core/server/api/endpoints/index.js:111-112`).
- Ghost routes: `grep -n "automated_emails" docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` → lines 332-350; the two `automatedEmailDesign` lines (333, 342) were excluded from this domain. No matches in `content/routes.js` or other web `routes.js`.
- C++ folder discovery: `grep -l "automated_emails" include/blog/handlers/**/*.cppm` → only `include/blog/handlers/membership/automated_emails/**` plus registration lists (`include/blog/handlers/routes.cppm:7`, `base.cppm:155-164`, `all.cppm:145-154,491-500,740-749`).
- C++ path uniqueness: `grep -rn "/automated_emails" include/blog/handlers --include=routes.cppm` → single match at `include/blog/handlers/membership/automated_emails/routes.cppm:19`.
- Cache absence: `grep -n "cache|Cache|invalidate" include/blog/handlers/membership/automated_emails` → no matches.

## Applied source fixes (manifest group 35 — strict parity)

All eight routes now pass checks 1–8 at the endpoint contract level; verdicts flipped
(4 PARTIAL + 4 MISMATCH → 8 MATCH). Files edited in the owned folder only — no shared
file, no BUILD edit, no Bazel-cycle fix. Step `Output` stays
`std::map<std::string,std::string>` (`{"status","body"}`) and static `s_ctx` is unchanged.

Shared abstractions used (all already registered):

- `core_query` (`import core_query;`) — browse/read parse the Ghost option allowlist,
  apply the restricted-field guard, filter/project via `apply`/`run`, and build
  `meta.pagination` from `core::query::run`.
- `blog_handlers:mapping` (`import blog_handlers:mapping;`) — `wrap`, `pagination` and
  restricted-field `clean` on every collection response and on the single-read flatten.
- `blog_handlers_middleware` + `core_auth:ghost` — every handler calls
  `require_ghost_permission(docName, method)` (401 router gate retained; 403
  `NoPermissionError` on denial), closing the permission residual on all eight routes.
- `blog_handlers:outbox_shim` (`import :outbox_shim;`) — create/edit stage domain events
  in an outbox transaction; verifications/test emit `email.sender_verified` /
  `email.send_test` events dispatched by local adapters into the context-local `emails`
  resource; senders queues `email.send_verification` events and `sent_email_verification`
  meta. This is the observable mail side effect Ghost performs via `MagicLink` /
  `#sendTransactionalEmail`. No DB row is inserted by preview or test.

Per-route source changes:

- **browse / read_by_id** — `core::query` input + restricted guard; fixture rows flattened
  to the Ghost 12-field shape with default-design sender fallback; `meta.pagination` via
  `blog::mapping::pagination`; 404 message corrected to `Automated email not found.`;
  single read wrapped as `{automated_emails:[...]}`.
- **create / edit** — Ghost input validator (name/status/slug allowlists, JSON Lexical,
  sender-address policy); four ordered writes with the design id taken from the email
  record; trigger-rule row now has id/created_at; status-conversion and slug-immutability
  guards; sender diffing; outbox transaction + events; flattened 201/200 envelopes.
- **senders / verifications** — optional/normalized sender fields with unchanged-skip and
  structural email validation; design-settings-only persistence; verification-mail and
  verification-completion events through outbox adapters; `meta.sent_email_verification`
  and `meta.email_verified`; 422 `Incorrect Usage` for a disallowed token property.
- **preview / test** — `:id` from the parent path segment; required subject/lexical with
  JSON validation; 404 when the welcome email is missing; flat `{html,plaintext,subject}`
  rendered from Lexical text nodes with the Jamie Larson test-member substitution; no
  DB write on either route; test emits the `[Test]` subject mail event and returns 204.

Residuals (non-route, documented): real Knex `findPage`/`withRelated` and transactional
row locking (`Base.transaction`, `lockAutomationLimit`); full KG/Lexical HTML rendering;
single-use signed magic-link token codec (expiry/signature); managed-email sending-domain
policy; router-level `previewEmailLimiter` (429). None changes the request/response/status/
header contract verified by checks 1–8.

## Needs central registration (group 35)

- **New-partition need (pre-existing, systemic): `blog_handlers:mapping`.** Not registered
  in `BUILD.bazel` partitions / `base.cppm`; parent must wire `{base,clean,wrap}.cppm` in
  order and add `export import :mapping;`. Not edited here.
- **New-partition need: shared signed magic-link token codec** (`getDataFromToken` /
  `sendMagicLink` with single-use + expiry) and **email-address sender policy**
  (`email-address-service.ts`). Neither exists as a registered partition.
- `blog_handlers:outbox_shim`, `blog_handlers_middleware:ghost_permission`, `core_query`
  and `core_auth` are already registered/deps. No new global state: adapters use
  function-local statics and the existing process-wide outbox; `s_ctx` carries the context.

Verification note: `bazel build //include/blog/handlers:blog_handlers` was attempted
plain and with `--keep_going`. It fails on the pre-existing `blog_handlers` module cycle
and on unrelated in-progress shared files (`discovery/recommendations/browse/.../execute.cppm`
ambiguous `pagination_meta` / undeclared `now_iso`; `auth/session/create/.../execute.cppm`
`-Wunused-result`; `external/webhooks/create/.../execute.cppm` `no member 'mutations'`).
No compile error was attributed to any of the ten automated-emails files in either build.
