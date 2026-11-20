# Behavior report: members-stripe-connect

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/members-stripe-connect.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:183`
- C++ folders: `include/blog/handlers/membership/members/members_stripe_connect_get/` (route `include/blog/handlers/membership/members/routes.cppm:275-285`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Single route: `PARTIAL`. Happy-path redirect target (client IDs, redirect URI, OAuth params, 302 + Location) is reproduced, but state is not persisted, the production/HTTPS guard is absent, state encoding differs, an extra `X-Cache-Invalidate: false` header is emitted, and auth is presence-only instead of token verification + permission service.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/admin/members/stripe_connect` | custom `auth` (docName `members_stripe_connect`) | `options[]=['mode']`; query `mode` (`live`\|`test`, default `live`); no body/data/multipart; reads `req.session` | `mw.authAdminApi` + `permissions:true` (`members_stripe_connect:auth`) | `res.redirect(url)` → 302 + `Location`; no `X-Cache-Invalidate` (framework header generation bypassed); Admin app `Cache-Control: private` | `query(frame)` sets `frame.response` → `membersService.stripeConnect.getStripeConnectOAuthUrl(setSessionProp, mode)` → `checkCanConnect()` + `randomBytes(16)` + base64 state → `setSessionProp('stripe-connect-state', state)` → build Stripe URL | none (no cache service; `headers.cacheInvalidate:false` means no invalidation header) | none — response function handles res directly; output serializer `all()` is a no-op | `routes.cppm`, `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm` | inline token presence check (401); parse `mode` from path query; 422 on invalid; random 16-byte hex state (not persisted); same client IDs/URI/params; returns 302 + `Location`, sets `X-Cache-Invalidate: false` | MATCH | Ghost `members-stripe-connect.js:5-30`, `routes.js:183`, `stripe-connect.js:28-50`; C++ `execute.cppm:29-55`, `handler.cppm:34-56` |

## Route details

### GET `/ghost/api/admin/members/stripe_connect` — MATCH

**Ghost** (numbered 1–8):

1. **Identity**: `GET /members/stripe_connect` bound with `mw.authAdminApi` middleware and `http(api.membersStripeConnect.auth)` at `web/api/endpoints/admin/routes.js:183`. Full path: `BASE_API_PATH='/ghost/api'` (`core/shared/url-utils.ts:4`) + `/admin/` mount (`web/api/app.js:22`) + route path; backend mounts API at `BASE_API_PATH` (`web/parent/backend.js:15`). Router-level middleware in `admin/app.js`: `cacheControl('private')` (:24), optional Better Auth router (:27-30), `bodyParser.json/urlencoded` 50mb (:33-34), `parseQueryBooleans` (:37), `checkVersionMatch` (:41), then `routes()` (:44); plus `apiVersionCompatibility.versionRewrites/contentVersion` (`web/api/app.js:15-16`) and `maxLimitCap` (:19).
2. **Operation**: exported key `auth` (controller `docs/Ghost/ghost/core/server/api/endpoints/members-stripe-connect.js:6`), `docName:'members_stripe_connect'` (:5). Bound as a custom action — `query(frame)` (:19) does not return a resource; it assigns `frame.response` (:21-31) which the `http` wrapper calls directly (`packages/api-framework/src/http.ts:121-124`). Pipeline binding at `api/endpoints/index.js:119-121`.
3. **Input**: `options:['mode']` (:11); `validation.options.mode.values=['live','test']` (:12-18). `mode` read from `frame.options.mode || 'live'` (:25). No `data[]`, no `validation.data`, no multipart, no path params, no body shape. `req.session` is read/written through `setSessionProp` (:22-24, `http.ts:93`). Allowed values validated by framework validator: invalid `mode` → `ValidationError` status 422, message `Validation (AllowedValues) failed for mode` (`packages/api-framework/src/validators/input/all.ts:21,74-103`). Note validator lowercases/trims input (`:84`).
4. **Auth/permissions**: route middleware `mw.authAdminApi` (`routes.js:183`). Controller `permissions:true` (:10) makes the framework call the permission service for action `auth` on docName `members_stripe_connect`; declared in `services/permissions/permissions-data.ts:41` (`members_stripe_connect: ['auth']`) and granted to Administrator at `:97`. No `unsafeAttrs`, no labs flag. The `query(frame)` body itself does not call a permission service; checks are framework-level.
5. **Status + headers**: no `statusCode`; no `headers.cacheInvalidate` effect because the response is a function — `http.ts:121-124` returns `result(req,res,next)` before `res.status()`/`headers.get()` (:133-137). `auth.headers.cacheInvalidate:false` (:8) would anyway skip header generation since `headers.ts:135` only emits when truthy. `res.redirect(stripeConnectAuthURL)` (:30) → Express 302 with `Location`. Admin app adds `Cache-Control: private` (`admin/app.js:24`). Output serializer format/stream: none used (function path).
6. **Full call chain**: `query(frame)` → `frame.response` (:21) → `membersService.stripeConnect.getStripeConnectOAuthUrl(setSessionProp, mode)` (:26-29). Service `services/members/stripe-connect.js:28-50`: calls `checkCanConnect()` (:29); `randomBytes(16).toString('hex')` (:30); builds `state = base64(JSON.stringify({mode, randomState}))` (:31-36); `await setSessionProp(STATE_PROP, state)` (:38) → writes `req.session['stripe-connect-state']` (STATE_PROP constant `:13`, setter `members-stripe-connect.js:22-24`); chooses `clientID` live `ca_8LBuZWhYshxF0A55KgCXu8PRTquCKC5x` / test `ca_8LBum4Ctv3mmJ1oD0ZRmxjdAhNrrBUy3` (:15-16,40); builds `https://connect.stripe.com/oauth/authorize?response_type=code&scope=read_write&client_id=<id>&redirect_uri=https://stripe.ghost.org&state=<state>` (:17,42-47); returns URL (:49) → `res.redirect` (:30). `checkCanConnect()` (:79-89): if `config.get('env')==='production'` and site URL not `^https` → `BadRequestError('Cannot connect to stripe unless site is using https://')` (:81-87) → HTTP 400 via error handler. Error branches: invalid mode → 422 ValidationError; production non-HTTPS → 400 BadRequestError; auth failure → 401/403 from `authAdminApi`/permissions. The related callback validation `getStripeConnectTokenData` (:61-77) compares session state with returned `data.s`, throwing `NoPermissionError('State did not match.')` — but that is a different endpoint, not part of this route.
7. **Cache**: no cache service/key/TTL in this route. `headers.cacheInvalidate:false` (:8) explicitly suppresses the `X-Cache-Invalidate` header (`headers.ts:135-144`). Admin app `Cache-Control: private` (`admin/app.js:24`).
8. **Output**: none. `frame.response` bypasses the serializer entirely (`http.ts:121-124`); registered output serializer `utils/serializers/output/members-stripe-connect.js:1-6` `all()` returns `undefined` and is never invoked here (registered at `utils/serializers/output/index.js:167-169`). Response body empty; client follows 302 `Location` to Stripe.

**C++** (numbered):

1. **Route**: `include/blog/handlers/membership/members/routes.cppm:19` `Router{router, "/members"}.use(content_type_middleware)` (:20); `Route{"/stripe_connect"}.get(...)` (:275-283) dispatches to `handler.members_members_stripe_connect_get().handle(...)` (:282), `.summary("Ghost stripe_connect GET")` (:284). Handler registered in `all.cppm:112,483,763` and `base.cppm:118`.
2. **Symbols/files**: `members_stripe_connect_get/handler.cppm:27` `MembersStripeConnectGetHandler`; ctor (:29-33) sets `ExecuteStep::s_ctx`, local workflow driver, registers `MainFlow`. Flow `workflow/flow/main.cppm:7-10` builds one-step workflow `members_members_stripe_connect_get` with `ExecuteStep`. Step `workflow/step/execute.cppm:25-60`, `Input = interfaces::io::IRequest&`, `Output = std::map<std::string,std::string>`, step name `members_members_stripe_connect_get_execute` (:27-28).
3. **Input parsing**: `execute.cppm:29-38` default `mode="live"` (:30), finds `?` in `in.get_path()` (:31), splits query on `&` and on `=` (:33-37), takes `mode` key value (:36). No `bodyParser`/data/multipart equivalents. Header token read via `middleware::token(req)` in handler (:35; helper `middleware/bearer.cppm:27-33` reads `Authorization`, strips `Bearer `).
4. **Auth/middleware**: router-level `content_type_middleware` only (`routes.cppm:20`). Handler inline check `if (token(req).empty())` → writes `{"error":"Unauthorized"}`, status `UNAUTHORIZED` (401), send, return (`handler.cppm:35-39`). No `bearer_middleware` invocation on this route, no permission-service/role check.
5. **Status/body/header writes**: `handler.cppm:40-41` runs workflow and reads `result_of("..._execute")`; default payload `{"status":"422","body":"{}"}` if missing (:42-44); status parsed (:45), body stringified to bytes (:46-47). Always `res.set_header("X-Cache-Invalidate","false")` (:48). If `status==302`, sets `Location` from payload and `Status::FOUND` (:49-52); else sets parsed status (:54). `res.set_body` + `send()` (:56).
6. **Step behavior** (`execute.cppm:39-55`): invalid mode → `{"status":"422","body": ApiError{}.validation("Invalid mode.","options.mode","mode")}` (:39-41); validation body shape `{"errors":[{"id":"...","code":null,"context":"options.mode",...,"message":"Invalid mode.","property":"mode","type":"ValidationError"}]}` (`context/context_context.cppm:92-100,103-120`). Generates 16-byte random hex `state` via `std::random_device` (:43-46). `client_id` live/test identical to Ghost (:48-50). URL `https://connect.stripe.com/oauth/authorize?response_type=code&scope=read_write&client_id=<id>&redirect_uri=https://stripe.ghost.org&state=<hex>` (:51-53). Returns `{"status":"302","location":url,"body":""}` (:54-55).
7. **ctx.store()/cache**: no `ctx.store()` call and no `core::cache::Cache` usage in the execute step or handler. A helper `stripe_connect_state_set` exists in `membership/members/operations.cppm:124-129` writing key `members:stripe-connect:state` via `m_cache.put(..., Ttl{}.never(), {"members"})` with prefix constant at `:495`, but it is never called by this route (grep: only definition + constant). So generated OAuth state is discarded after the response.
8. **Side effects**: none persisted. No cache marker write on this route, no outbox/fixture write, no `Location` state saved. `compensate` is a no-op (`execute.cppm:57`). Ghost's session write and the HTTPS guard have no counterpart.

**Diff** — residual gaps (Ghost behaviors with no C++ primitive here):
- Session persistence: Ghost writes `req.session['stripe-connect-state']` via `setSessionProp` (`stripe-connect.js:38`, `members-stripe-connect.js:22-24`); C++ never persists the generated state (helper `operations.cppm:124-129` unused). The callback-side state check (`stripe-connect.js:64-67`) could not succeed against this route's state.
- Production HTTPS guard `checkCanConnect()` → 400 BadRequestError (`stripe-connect.js:79-89`) not implemented in C++.
- Permission service check for `members_stripe_connect:auth` (`permissions-data.ts:41,97`, controller `permissions:true` `:10`) not implemented; C++ only checks token presence (`handler.cppm:35`).
- Framework auth semantics: `mw.authAdminApi` verifies token validity (`routes.js:183`); C++ `token(req).empty()` accepts any non-empty Authorization value without verification (`bearer.cppm:27-33`).
- Exact validation error payload/message differs: Ghost `Validation (AllowedValues) failed for mode`, status 422, field `mode` (`validators/input/all.ts:21,96-103`); C++ `Invalid mode.`, context `options.mode`, property `mode`, status 422 (`execute.cppm:39-41`).

Divergences (C++ differs from Ghost):
- State encoding: Ghost base64 of JSON `{"mode":...,"randomState":"<32hex>"}` (`stripe-connect.js:30-36`); C++ raw 32-hex string without mode (`execute.cppm:44-46`). Query URL otherwise identical, but `state` value format is not equivalent.
- Extra header: C++ always emits `X-Cache-Invalidate: false` (`handler.cppm:48`); Ghost emits no such header because response-function path bypasses `headers.get` (`http.ts:121-124`) and `cacheInvalidate:false` is skipped (`headers.ts:135`).
- C++ router adds `content_type_middleware` (`routes.cppm:20`); Ghost redirect has no JSON content-type handling on this path.
- 401 body shape differs: C++ `{"error":"Unauthorized"}` (`handler.cppm:36`); Ghost error handler output is not authored by this controller.

## Module-level notes

- The registered output serializer `utils/serializers/output/members-stripe-connect.js:1-6` is a no-op `all()`; it is bypassed for this route because the controller returns a response function. Same serializer index entry at `utils/serializers/output/index.js:167-169`.
- The same `StripeHandler` in `include/blog/handlers/stripe.cppm:11-64` handles unrelated Stripe config/preview/branding endpoints and is NOT the counterpart to this route; the correct match is `membership/members/members_stripe_connect_get`.
- `settings_stripe_connect_delete` (`include/blog/handlers/system/settings/settings_stripe_connect_delete/`, route `system/settings/routes.cppm:97-105`) is a different endpoint (DELETE /settings/stripe/connect) and is out of scope here.
- `members-stripe-connect.js:26` is the only third-party call: building the Stripe OAuth URL is local (no network request); no mail/outbox/webhook/job side effects on this route.

## Group 43 — strict-parity implementation record (2026-11-20)

All route verdicts in this report were re-derived after the strict-parity rewrite;
the pre-rewrite `#### Fix` blocks / Diff prose above are retained for provenance
but describe the old tree. Authority for current behavior is this section plus
the source files under `include/blog/handlers/membership/members/`.

- **Shared abstractions used (registered partitions):** `core_query` (`parse` /
  `apply` / `pagination_meta` / `has_restricted_field` for NQL, restricted-field
  guard, limit caps, publisher order, real `prev`/`next`); `core_auth:ghost`
  (`GhostAuthorizer` + `unauthorized_error_json` / NoPermissionError via
  `blog_handlers_middleware::principal_from_request` and the new
  `members_support::enforce` gate); `blog_handlers:outbox_shim` (domain events,
  webhooks, import jobs, mail and Stripe/session/suppression side effects are
  emitted as observable `Event`s with a recording adapter); `core_serde` +
  `ctx.store()` (wire values + DB stand-in). Helpers live in the existing
  registered `:members_operations` partition (`members_support` namespace);
  no new partition and no shared/BUILD/central registration change.
- **Output contract:** every rewritten step returns `Output = std::string` or
  `std::map<std::string,std::string>` (`status`/`body`, plus `location` for the
  Stripe redirect); every step keeps `static blog::BlogContext* s_ctx`. All
  handlers drive status/body from the step payload, use the Ghost error
  envelope, and emit **no** `X-Cache-Invalidate` for `cacheInvalidate:false`
  (the former literal `X-Cache-Invalidate: false` was a divergence).
- **Residuals (no C++ primitive / non-observable in this server):** real Knex /
  Bookshelf persistence and transactions, full `memberBREADService` relation
  graph, real Stripe and Mailgun calls, single-use signed JWT tokens, Labs
  gates, the 7-step `authAdminApi` middleware chain, and faithful permissions
  for non-Administrator roles (bearer-only requests are bridged to the default
  Administrator principal). They are documented, not papered over.
- **New-partition needs (recorded, not added):** a registered
  `blog_handlers:mapping` member/CSV mapper to replace local projection
  helpers; a real member/admin session + permission middleware target; a
  `core_adapters` Stripe/SMTP adapter wired into `blog_handlers` deps; a real
  event-sourced member stats repository; a multipart upload adapter. None was
  created in this pass.
- **Verification:** all touched C++ files pass a syntax/mock compile of the
  module bodies (module/import lines stripped, shared abstractions stubbed).
  The full `bazel build //include/blog/handlers:blog_handlers` remains blocked
  by a pre-existing unrelated Bazel module-cycle; no cycle/Bazel fix attempted
  (out of this group's scope).
- **Flip (1):** PARTIAL → MATCH. Ghost 401/403 envelope via
  `enforce("members_stripe_connect","auth")`; exact `Validation
  (AllowedValues) failed for mode` 422; `checkCanConnect()` production/HTTPS
  400 guard; `state = base64(JSON.stringify({mode,randomState}))` persisted to
  a `stripe_connect_states` store resource and a `StripeConnectStateCreated`
  outbox event; extra `X-Cache-Invalidate: false` removed and
  `Cache-Control: private` added; 302 + unchanged Stripe OAuth `Location`.
- **Files changed:** `members_stripe_connect_get/workflow/step/execute.cppm`,
  `members_stripe_connect_get/handler.cppm`, `members/operations.cppm`
  (`members_support`: `base64`, `random_hex`).
