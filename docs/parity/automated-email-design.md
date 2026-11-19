# Parity: automated-email-design

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/automated-email-design.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/automated_emails/routes.cppm`
  (design handlers: `.../automated_emails_design_get/`, `.../automated_emails_design_put/`)
- Exposed as `api.automatedEmailDesign` -> `./automated-email-design` (`.../api/endpoints/index.js:115-116`); controller `docName: 'automated_email_design'`.
- Checked: 2026-11-19

## Controller operations

`docName: 'automated_email_design'`; exported operations: `read`, `edit`.

| Op | Ghost `options[]` / `data[]` / validation | Permissions | headers | Exact model/service call in `query()` |
|---|---|---|---|---|
| `read` | none declared; no body; resolves slug `default-automated-email` | `email_design_setting` / `read` | `cacheInvalidate: false` | `models.EmailDesignSetting.findOne({slug: DEFAULT_EMAIL_DESIGN_SETTING_SLUG}, frame.options)` via `resolveDefaultDesign` (`automated-email-design.js:21-36,50-52`); throws `InternalServerError` if no row |
| `edit` | payload read as `frame.data.automated_email_design[0]`; deletes `id`; rejects `slug` (ValidationError); `emailAddressService.init()` + `validateEmailSenderFields` | `email_design_setting` / `edit` | `cacheInvalidate: false` | `resolveDefaultDesign(frame.options)` then `models.EmailDesignSetting.edit(data, {...frame.options, id: defaultDesign.get('id')})` (`automated-email-design.js:63-85`) |

`DEFAULT_EMAIL_DESIGN_SETTING_SLUG = 'default-automated-email'` (`.../services/member-welcome-emails/constants.ts:1`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 2 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automated_emails/design` | `read` | GET `/automated_emails/design` (`Router "/automated_emails"` + `Route "/design"` `.get`) | `automated_emails_automated_emails_design_get::AutomatedEmailsDesignGetHandler` | Ghost: none; C++: `Input = IRequest&`, no query/body parse | Ghost: single default `EmailDesignSetting` model (`findOne` by slug), no pagination; C++: `{"automated_emails":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` hardcoded | Ghost: `models.EmailDesignSetting.findOne({slug:'default-automated-email'})`; C++: `ctx.store().json_for("automated_emails", {})` (wrong resource, no slug filter, no default-design resolution) | Ghost: `cacheInvalidate:false` header, no cache call; C++: none | Ghost: `mw.authAdminApi` + permission `email_design_setting`/`read`; C++: `content_type_middleware` + `middleware::token(req).empty()` -> 401 | **MISMATCH** | Ghost: routes.js:333, controller:42-53; C++: routes.cppm:19-20,79-87, handler.cppm:17-27, execute.cppm:11-16 |
| PUT `/automated_emails/design` | `edit` | PUT `/automated_emails/design` (`Router "/automated_emails"` + `Route "/design"` `.put`) | `automated_emails_automated_emails_design_put::AutomatedEmailsDesignPutHandler` | Ghost: `data.automated_email_design[0]`, strip id, reject slug, sender-field validation; C++: `Input = IRequest&`, raw `body_as_string()` ("{}" if empty), no parsing/validation | Ghost: edited `EmailDesignSetting` model; C++: raw `mutations.edit(...).body()` | Ghost: `models.EmailDesignSetting.edit(data,{id: defaultDesign.id})` after `resolveDefaultDesign`; C++: `mutations.edit("automated_emails", "", body)` (wrong resource, no default resolution, no validation) | Ghost: `cacheInvalidate:false` header, no cache call; C++: none | Ghost: `mw.authAdminApi` + permission `email_design_setting`/`edit`; C++: `content_type_middleware` + `middleware::token(req).empty()` -> 401 | **MISMATCH** | Ghost: routes.js:342, controller:55-85; C++: routes.cppm:19-20,88-95, handler.cppm:17-27, execute.cppm:11-15 |

## Route details

### GET `/automated_emails/design` — MISMATCH

- Route identity/verb: agree. Ghost `router.get('/automated_emails/design', mw.authAdminApi, http(api.automatedEmailDesign.read))` (`admin/routes.js:333`); C++ `Router{router, "/automated_emails"}` (`routes.cppm:19`) + `Route{"/design"}.get(...)` binding `automated_emails_automated_emails_design_get()` (`routes.cppm:79-87`). Operation mapping: Ghost `read` -> C++ `AutomatedEmailsDesignGetHandler` (`handler.cppm:10-23`, registered `all.cppm:146,492,741`).
- DB call RESOURCE MISMATCH: Ghost `models.EmailDesignSetting.findOne({slug: 'default-automated-email'}, options)` with `InternalServerError` on missing row (`automated-email-design.js:21-36,50-52`; slug from `constants.ts:1`); C++ `ctx.store().json_for("automated_emails", {})` (`design_get/.../execute.cppm:15`) — reads the whole `automated_emails` table, no slug filter, no `email_design_settings` resource, no default-design resolution.
- Output MISMATCH: Ghost returns a single resolved `EmailDesignSetting` model (no `meta.pagination`); C++ returns an `automated_emails` array plus fabricated pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`execute.cppm:16`) — wrong resource key and invented meta.
- Input: Ghost declares no options/data; C++ `Input = interfaces::io::IRequest&` and does no query-string or body parsing (`execute.cppm:11-13`). No param-name divergence to list (both effectively no parsed params).
- Cache: Ghost `headers.cacheInvalidate: false` (`automated-email-design.js:43-45`), no cache service call; C++ none (grep for `cache|Cache|invalidate` under `membership/automated_emails` returned no matches). No eviction/TTL on either side.
- Auth: Ghost `mw.authAdminApi` (`routes.js:333`) plus permissions `email_design_setting`/`read` (`automated-email-design.js:46-49`); C++ `content_type_middleware` on the router (`routes.cppm:20`) and only a token-presence check `middleware::token(req).empty()` -> `401 {"error":"Unauthorized"}` (`handler.cppm:18-21`). No docName/method permission check in C++.

### PUT `/automated_emails/design` — MISMATCH

- Route identity/verb: agree. Ghost `router.put('/automated_emails/design', mw.authAdminApi, http(api.automatedEmailDesign.edit))` (`admin/routes.js:342`); C++ same `Route{"/design"}.put(...)` binding `automated_emails_automated_emails_design_put()` (`routes.cppm:88-95`). Operation mapping: Ghost `edit` -> C++ `AutomatedEmailsDesignPutHandler` (`handler.cppm:10-23`, registered `all.cppm:151,497,746`).
- DB call RESOURCE MISMATCH: Ghost `resolveDefaultDesign` then `models.EmailDesignSetting.edit(data, {...frame.options, id: defaultDesign.get('id')})` targeting `email_design_settings` by the resolved default row id (`automated-email-design.js:63-85`); C++ `mutations.edit("automated_emails", std::string{}, in.body_as_string().empty() ? "{}" : in.body_as_string())` (`design_put/.../execute.cppm:15`) — wrong resource, no default-design lookup, no `id` from slug, no WHERE by resolved id.
- Input MISMATCH: Ghost reads `frame.data.automated_email_design[0]`, strips `id`, throws `ValidationError` on any `slug`, and calls `emailAddressService.init()` + `validateEmailSenderFields(...)` (`automated-email-design.js:64-77`); C++ passes the untouched raw body string with none of these transformations/validations (`execute.cppm:11-15`). Ghost-only params/behavior: `id` strip, `slug` immutability check, sender-field validation; C++-only: none.
- Output MISMATCH: Ghost returns the edited `EmailDesignSetting` model; C++ returns whatever `mutations.edit("automated_emails",...).body()` produces, i.e. an `automated_emails` shaped payload, wrapped with `Status::OK` (`handler.cppm:25-27`).
- Cache: Ghost `headers.cacheInvalidate: false` (`automated-email-design.js:56-58`), no cache service call; C++ none. No eviction/TTL on either side.
- Auth: Ghost `mw.authAdminApi` (`routes.js:342`) plus permissions `email_design_setting`/`edit` (`automated-email-design.js:59-62`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence 401 check only (`handler.cppm:18-21`). No docName/method permission check in C++.

## Missing in C++

None. Both Ghost routes have a C++ route/handler installed at the exact path and verb.

## Extra in C++

None within this controller's scope. The matched folder `include/blog/handlers/membership/automated_emails/` also installs C++ routes for `browse`, `create`, `read_by_id`, `edit`, `/:id/preview`, `/:id/test`, `/senders`, `/verifications` (`routes.cppm:21-77,97-118`), but each has a Ghost counterpart in the separate `automated-emails.js` controller (`admin/routes.js:332,334,336-350`). Those belong to the `automatedEmails` domain (another controller/subagent), not to `automated-email-design`, so they are not counted as EXTRA here.
