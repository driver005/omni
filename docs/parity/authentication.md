# Parity: authentication

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/authentication.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/auth/authentication/routes.cppm` (all 8 handlers under `auth/authentication/<op>/`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 8 |
| MISSING | 0 |
| EXTRA | 0 |

All 8 Ghost `authentication` routes have a C++ route with the same verb + path. Every C++ handler is a
generic scaffold: no query-string/body DTO parsing (only raw `IRequest::body_as_string()`), CRUD on a
single generic `"authentication"` store resource, hardcoded pagination envelope on GETs, no cache, and a
dead auth check (`token(req).empty() && !true` is always false) behind only `content_type_middleware`.
Ghost implements real auth flows (setup pipeline, password reset, invites, session rotation, reset).
Resource and data shape differ materially on every route → MISMATCH across the board.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST /authentication/password_reset | generateResetToken | POST /authentication/password_reset | `authentication_authentication_password_reset_post` | Ghost: `options:['email']`, validation `password_reset`; C++: raw body string, no query parse | Ghost: token/notification result; C++: mutation body, status 200 | Ghost: `auth.passwordreset.generateToken/sendResetNotification`; C++: `mutations.create("authentication", body)` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `permissions:true` + rate-limit globalReset/userReset; C++: dead check, content_type only | MISMATCH | `routes.js:440-445`, `authentication.js:140-157` / `routes.cppm:41-48`, `password_reset_post/handler.cppm:17-27`, `password_reset_post/workflow/step/execute.cppm:13-17` |
| PUT /authentication/password_reset | resetPassword | PUT /authentication/password_reset | `authentication_authentication_password_reset_put` | Ghost: `options:['ip']`, validation `password_reset` + required `newPassword`/`ne2Password`; C++: raw body, no params | Ghost: `{}` (session rotation); C++: mutation body, status 200 | Ghost: `extractTokenParts`/`doReset`/session rotate/spamPrevention; C++: `mutations.edit("authentication","",body)` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `permissions:false` + globalBlock rate limit + `auth.session.initSession`; C++: dead check | MISMATCH | `routes.js:446-451`, `authentication.js:159-205` / `routes.cppm:50-56`, `password_reset_put/handler.cppm:17-27`, `password_reset_put/workflow/step/execute.cppm:13-17` |
| POST /authentication/invitation | acceptInvitation | POST /authentication/invitation | `authentication_authentication_invitation_post` | Ghost: validation `invitations`, `frame.data`; C++: raw body | Ghost: `invites.service.accept(data)`; C++: mutation body, status 200 | Ghost: `invites.service.accept`; C++: `mutations.create("authentication", body)` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `permissions:false`; C++: dead check | MISMATCH | `routes.js:452`, `authentication.js:207-219` / `routes.cppm:22-29`, `invitation_post/handler.cppm:17-27`, `invitation_post/workflow/step/execute.cppm:13-17` |
| GET /authentication/invitation | isInvitation | GET /authentication/invitation | `authentication_authentication_invitation_get` | Ghost: `data:['email']`, validation `invitations`; C++: no query parse | Ghost: Invite model; C++: `{"authentication":rows,"meta":{"pagination":{limit:15,page:1,...}}}` | Ghost: `models.Invite.findOne({email,status:'sent'})`; C++: `json_for("authentication",{})` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `permissions:false`; C++: dead check | MISMATCH | `routes.js:453`, `authentication.js:221-235` / `routes.cppm:31-37`, `invitation_get/handler.cppm:17-27`, `invitation_get/workflow/step/execute.cppm:13-17` |
| POST /authentication/setup | setup | POST /authentication/setup | `authentication_authentication_setup_post` | Ghost: validation `setup`, `frame.data.setup[0]` fields; C++: raw body | Ghost: status **201**, user + fixtures/settings; C++: mutation body, status **200** | Ghost: `auth.setup.*` pipeline (setupUser/doFixtures/doProductAndNewsletter/installTheme/doSettings/sendWelcomeEmail); C++: `mutations.create("authentication",body)` | Ghost: `cacheInvalidate:true`; C++: none | Ghost: `permissions:false`; C++: dead check | MISMATCH | `routes.js:454`, `authentication.js:31-86` / `routes.cppm:71-78`, `setup_post/handler.cppm:17-27`, `setup_post/workflow/step/execute.cppm:13-17` |
| PUT /authentication/setup | updateSetup | PUT /authentication/setup | `authentication_authentication_setup_put` | Ghost: validation `setup`, setup details; C++: raw body | Ghost: settings result; C++: mutation body, status 200 | Ghost: `auth.setup.setupUser`/`doSettings`; C++: `mutations.edit("authentication","",body)` | Ghost: `cacheInvalidate:true`; C++: none | Ghost: `mw.authAdminApi` + async owner check (Owner role, throws NoPermissionError); C++: dead check, no owner check | MISMATCH | `routes.js:455`, `authentication.js:88-115` / `routes.cppm:80-86`, `setup_put/handler.cppm:17-27`, `setup_put/workflow/step/execute.cppm:13-17` |
| GET /authentication/setup | isSetup | GET /authentication/setup | `authentication_authentication_setup_get` | Ghost: no input; C++: no query parse | Ghost: `{status:true}` or `{status:false,title,name,email}`; C++: `{"authentication":rows,"meta":{pagination...}}` | Ghost: `auth.setup.checkIsSetup()`; C++: `json_for("authentication",{})` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `permissions:false`; C++: dead check | MISMATCH | `routes.js:456`, `authentication.js:117-138` / `routes.cppm:88-94`, `setup_get/handler.cppm:17-27`, `setup_get/workflow/step/execute.cppm:13-17` |
| POST /authentication/reset | reset | POST /authentication/reset | `authentication_authentication_reset_post` | Ghost: `frame.options`, session; C++: raw body | Ghost: status **200**, `auth.resetAuthentication` result; C++: mutation body, status **200** | Ghost: `auth.resetAuthentication({schedulerAdapter,userService,options})` + `destroyRequestSession`; C++: `mutations.create("authentication",body)` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + `permissions:true`; C++: dead check, content_type only | MISMATCH | `routes.js:457`, `authentication.js:237-258` / `routes.cppm:60-67`, `reset_post/handler.cppm:17-27`, `reset_post/workflow/step/execute.cppm:13-17` |

## Route details

### POST /authentication/password_reset — MISMATCH

- Ghost `permissions:true`, `options:['email']`, validation `password_reset`, `cacheInvalidate:false`, rate-limited (`globalReset`, `userReset`); calls `auth.passwordreset.generateToken(email)` then `sendResetNotification` (`authentication.js:140-157`, `routes.js:440-445`).
- C++ maps verb+path correctly (`routes.cppm:41-48`) but `ExecuteStep` only does `mutations.create("authentication", body_as_string())` (`execute.cppm:13-17`); no `email` param, no password-reset service, no rate limiting, no cache, status forced 200 (`handler.cppm:27`). Auth check dead: `token(req).empty() && !true` (`handler.cppm:18`).

### PUT /authentication/password_reset — MISMATCH

- Ghost `options:['ip']`, validation requires `newPassword`/`ne2Password`, `globalBlock` rate limit + `auth.session.initSession`, full token extract/doReset/session rotation/spam-prevention reset, returns `{}` (`authentication.js:159-205`, `routes.js:446-451`).
- C++ only `mutations.edit("authentication","",body)` (`execute.cppm:13-17`); no IP/body validation, no session logic, status 200, dead auth (`handler.cppm:18-27`).

### POST /authentication/invitation — MISMATCH

- Ghost validation `invitations`, `invites.service.accept(frame.data)`, `permissions:false`, `cacheInvalidate:false` (`authentication.js:207-219`, `routes.js:452`).
- C++ generic `mutations.create("authentication",body)` (`execute.cppm:13-17`); no invite service, status 200, dead auth (`handler.cppm:18-27`).

### GET /authentication/invitation — MISMATCH

- Ghost `data:['email']`, `models.Invite.findOne({email,status:'sent'}, options)` returning an Invite model (`authentication.js:221-235`, `routes.js:453`).
- C++ returns `json_for("authentication",{})` wrapped in a hardcoded pagination envelope `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`execute.cppm:16-17`); no `email` filter, wrong resource/shape, dead auth (`handler.cppm:18-27`).

### POST /authentication/setup — MISMATCH

- Ghost `statusCode:201`, `cacheInvalidate:true`, full setup pipeline via `auth.setup.*` (assertSetupCompleted(false), setupUser, doFixtures, doProductAndNewsletter, installTheme, doSettings, sendWelcomeEmail) (`authentication.js:31-86`, `routes.js:454`).
- C++ generic create on `"authentication"` and forces status 200 (`execute.cppm:13-17`, `handler.cppm:27`); no pipeline, no 201, no `cacheInvalidate`, dead auth (`handler.cppm:18`).

### PUT /authentication/setup — MISMATCH

- Ghost `mw.authAdminApi` middleware plus async permission check: find Owner user, compare `owner.id !== frame.options.context.user`, throw `NoPermissionError` ("You are not the site owner."); `cacheInvalidate:true`, validation `setup` (`authentication.js:88-115`, `routes.js:455`).
- C++ generic edit on `"authentication"`, no owner/permission check, no cache invalidation, dead auth (`execute.cppm:13-17`, `handler.cppm:18-27`).

### GET /authentication/setup — MISMATCH

- Ghost `auth.setup.checkIsSetup()` returns `{status:true}` or `{status:false,title,name,email}` from config (`authentication.js:117-138`, `routes.js:456`).
- C++ returns `json_for("authentication",{})` with hardcoded pagination envelope (`execute.cppm:16-17`); shape and resource differ, dead auth (`handler.cppm:18-27`).

### POST /authentication/reset — MISMATCH

- Ghost `statusCode:200`, `permissions:true`, `mw.authAdminApi`, calls `auth.resetAuthentication({schedulerAdapter,userService,options})` then `destroyRequestSession` to expire the client cookie (`authentication.js:237-258`, `routes.js:457`).
- C++ generic `mutations.create("authentication",body)` (`execute.cppm:13-17`); no reset service, no session teardown, no admin auth (dead check, content_type only at `routes.cppm:20`, `handler.cppm:18-27`).

## Missing in C++

None. All 8 Ghost routes have a C++ route with matching verb + path in `include/blog/handlers/auth/authentication/routes.cppm:19-96`.
Search performed: `grep -rn "authentication" include/blog/handlers` (routes.cppm, all.cppm, BUILD.bazel, base.cppm) and glob `include/blog/handlers/**/routes.cppm`.

## Extra in C++

None. The C++ `auth/authentication` folder installs exactly 8 routes (invitation POST/GET, password_reset POST/PUT, reset POST, setup POST/PUT/GET) — all 8 have a corresponding Ghost controller route. No unmatched C++ route in this folder.
