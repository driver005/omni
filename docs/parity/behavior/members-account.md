# Behavior report: members-account

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/members-account.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/members/account/routes.ts` (mounted at `/api/member` by `docs/Ghost/ghost/core/server/web/members/app.js:61`, members app mounted at `/members` by `docs/Ghost/ghost/core/server/web/parent/frontend.js:20`)
- C++ folders: `include/blog/handlers/membership/members/members_account_{read,update,destroy_suppression}/`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/api/member/` | `read` | none; member identity from session | `permissions:false`; `emptyWhenAnonymous` → 204; session cookie | 200 body / 204 anon; `cacheInvalidate:false`, `Cache-Control: private`, optional access cookies | `account.read` → `memberBREADService.read({id},{metafieldsFor:MEMBERS})` → `formattedMemberResponse` | no data cache; `cacheMembersContent` stamps cookies | bare member object (no envelope) | `membership/members/members_account_read/{handler.cppm,workflow/step/execute.cppm,workflow/flow/main.cppm}` | bearer `token()` + `x-member-id` header; `find_by_resource`, cache `members:account:<id>:fields`, fabricated `account_projection`; 401/204/404/200 | PARTIAL | Ghost `routes.ts:29-42`, `members-account.ts:29-35`; C++ `routes.cppm:306-314`, `execute.cppm:30-48` |
| PUT `/members/api/member/` | `update` | bare JSON body; 6 writable fields + `metafields`; `bodyParser.json` 50mb | `permissions:false`; `rejectWhenAnonymous` → 401 | 200 body; `cacheInvalidate:false`, private | `account.edit` → metafield `planWrite` → `updateWithMetafields` (txn, events/webhooks) → `read` back → serializer | no data cache | bare member object (fresh read) | `membership/members/members_account_update/...` | bearer + `x-member-id`; `pick()` same 6 fields; `store().edit`, cache marker write `members:account:<id>:fields` (never TTL, tag `members`); no metafields/events | PARTIAL | Ghost `routes.ts:44-49`, `account-service.ts:136-160`; C++ `update/execute.cppm:30-73`, `operations.cppm:148-152` |
| DELETE `/members/api/member/suppression` | `destroySuppression` | none; member id+email from session identity | `permissions:false`; `rejectWhenAnonymous` → 401 | 204 No Content; `cacheInvalidate:false`, private | `account.allowEmail(id,email)` → `emailSuppressionList.removeEmail(email)` + `members.update({email_disabled:false})` | none | empty | `membership/members/members_account_destroy_suppression/...` | bearer + `x-member-id`; email read from stored member JSON; cache erase `members:suppression:<email>` + `store().edit({"email_disabled":false})`; 401/204/404/500/204 | PARTIAL | Ghost `routes.ts:64-68`, `account-service.ts:235-238`; C++ `destroy_suppression/execute.cppm:30-49`, `operations.cppm:139-145` |

## Route details

### GET /members/api/member/ — PARTIAL

**Ghost** (facts 1–8):
1. Identity: `GET /` on the account router (`web/members/account/routes.ts:29-42`), chain mounted `/members` (`web/parent/frontend.js:20`) + `/api/member` (`web/members/app.js:61`). Router-level mounts before account: `cacheControl('private')` (app.js:27), CORS (app.js:30), `createSessionFromMagicLink` (app.js:33). Account router `router.use(loadMemberIdentity)` (routes.ts:24). When `cacheMembersContent:enabled`: `loadMemberSession`, `accessInfoSession`, `emptyWhenAnonymous`, `http(read)` (routes.ts:28-39); else `emptyWhenAnonymous`, `http(read)` (routes.ts:41).
2. Operation: exported key `read`, `docName: 'members_account'`, `headers: {cacheInvalidate:false}`, `permissions:false`, `query` → `membersService.api.account.read(memberOf(frame)!.id)` (`api/endpoints/members-account.ts:26-35`, identity helper at :24).
3. Input: no body, path params, or query aliases; member comes from `frame.options.context.member` set by `carryIdentity` from `req.identity` (`services/members/middleware.js:187-190`). `loadMemberIdentity` uses `getMemberIdentityFromSession` (middleware.js:162-179). No `validation{}` on the controller.
4. Auth/permissions: `permissions:false`; session-based. Anonymous → `emptyWhenAnonymous` writes `204` and ends (middleware.js:225-233). DB error other than BadRequestError propagates (middleware.js:167-176).
5. Status/headers: 200 with body; 204 for anonymous; no X-Cache-Invalidate (`cacheInvalidate:false`); `Cache-Control: private` (app.js:27); with `cacheMembersContent` on, `accessInfoSession` registers `Set-Cookie` for `ghost-access` + `ghost-access-hmac` (middleware.js:84-90, 74-81).
6. Call chain: `account.read(memberId)` (`services/members/account-service.ts:129-133`) → `memberBREADService.read({id}, {metafieldsFor: MEMBERS})` → `memberRepository.get` with withRelated `labels, stripeSubscriptions, stripeSubscriptions.customer, stripeSubscriptions.stripePrice, stripeSubscriptions.stripePrice.stripeProduct, ..., currentSubscription, products, newsletters, productEvents` (`member-bread-service.js:402-427`) → `model.toJSON()` → subscription/customer/gift enrichment. Serializer: output `members-account.ts:13-15` calls `formattedMemberResponse` (`services/members/utils.js:28-64`). Service wired at `members-api.js:193-201`; `api/index` exposes it at `api/endpoints/index.js:340`.
7. Cache: no data cache. `cacheInvalidate:false` suppresses invalidation header; `cacheMembersContent` only affects entitlement cookies, keys off subscriptions (routes.ts:31-34). `Cache-Control: private` on the whole members app.
8. Output: bare object (no `data`/envelope) — `uuid, email, name, firstname, expertise, avatar_image, unsubscribe_url, subscribed, subscriptions, status, paid (status !== 'free'), created_at, enable_comment_notifications, enable_updates_and_announcements, can_comment, commenting`, plus optional `newsletters`, `email_suppression`, `metafields` (`utils.js:32-63`). `null` when member is missing (`utils.js:29-31`), though the route normally 204s before that.

**C++**:
1. Route: `Router{router,"/members"}` (`include/blog/handlers/membership/members/routes.cppm:19`) → nested `Router{router,"/api/member"}` with `.get(... members_account_read().handle ...)` (`routes.cppm:305-314`), yielding `GET /members/api/member` (no trailing slash). No dedicated router for this mount beyond the parent; hander is `blog::handlers::members::members_account::read::MembersAccountReadHandler` (`members_account_read/handler.cppm:26-33`).
2. Input type: `interfaces::io::IRequest&`; member id is `req.find_header("x-member-id")` (`read/workflow/step/execute.cppm:46-48`); no body/query parsed.
3. Status/body writes: handler writes directly to `IResponse` — `401 {"error":"Unauthorized"}` when `middleware::token(req).empty()` (`handler.cppm:34-39`); otherwise reads step payload map and sets status/body (`handler.cppm:40-48`), with default fallback `{"status","200"},{"body","{}"}` (`handler.cppm:42-44`).
4. `ctx.store()` calls: `find_by_resource("members", id)` (`execute.cppm:35`). No mutations on read.
5. Middleware chain: parent `/members` router installs `blog::handlers::middleware::content_type_middleware` (`routes.cppm:20`); router dispatch runs every matched node's middleware before the handler (`include/core/router/router.cppm:366-377`), so GET requires `Content-Type: application/json` and acceptable `Accept` (`include/blog/handlers/middleware/serde.cppm:90-119`). Handler-level bearer check `token()` reads `Authorization` (`include/blog/handlers/middleware/bearer.cppm:27-33`). No `require_auth`/`optional_auth` registered on the `/api/member` router itself.
6. Side effects: none.
7. Cache: `s_ops->account_fields_get(id)` (`execute.cppm:38`) reads key `members:account:<id>:fields`, default `"{}"` (`members/operations.cppm:154-159`, key prefix :497-498, key builder :500-509).
8. Output: `MembersOperations::account_projection(*one, overlay)` (`execute.cppm:40`, `operations.cppm:162-202`) — hand-built bare object with hardcoded defaults: `expertise:""`, `avatar_image:null`, `unsubscribe_url:"https://ghost.local/unsubscribe/<id>"`, `subscriptions:[]`, `created_at:"1970-01-01T00:00:00.000Z"` when missing, id fallback `"1"`, `firstname = first token of name`, `paid = status != "free" && status != "comped"`, `enable_comment_notifications:true`, `enable_updates_and_announcements:true`, `can_comment:true`, `commenting:{"disabled":false}`. Step branches: 204 when no id (`execute.cppm:33-34`), 404 `Resource not found / Member not found` on store miss (:35-37), 200 otherwise (:38-40). Flow is a single step (`read/workflow/flow/main.cppm:8-11`).

**Diff**:
- Residual gaps (no C++ primitive): Ghost session-cookie identity (`getMemberIdentityFromSession`), `cacheMembersContent` entitlement cookies, CORS, `Cache-Control: private`, the BREAD relation graph and metafield/newsletter enrichment, real `formattedMemberResponse` field set.
- Divergences: auth model (session + anonymous-204 vs bearer token + `x-member-id` + 401); GET requires JSON content-type/accept in C++; store miss returns 404 in C++ where Ghost would 204/serialize null; `paid` disagrees for `comped` (Ghost treats comped as paid — `utils.js:43`; C++ excludes it — `operations.cppm:193`); many fields hardcoded/fabricated; `members:account:<id>:fields` is a stand-in cache marker with no Ghost counterpart.

### PUT /members/api/member/ — PARTIAL

**Ghost** (facts 1–8):
1. Identity: `PUT /` on account router (`routes.ts:44-49`): `bodyParser.json({limit:'50mb'})` (:46), `rejectWhenAnonymous` (:47), `http(update)` (:48); plus global app middleware and `loadMemberIdentity` as for GET.
2. Operation: exported key `update` (custom verb — `edit` reserved for enveloped Admin API bodies; controller comment :54-56), `docName:'members_account'`, `headers.cacheInvalidate:false`, `permissions:false`, `query` → `membersService.api.account.edit(frame.data, member.id)` (`members-account.ts:57-63`).
3. Input: bare body `frame.data` (not `data.<resource>[0]`); writable fields `name, expertise, subscribed, newsletters, enable_comment_notifications, enable_updates_and_announcements` (`account-service.ts:32-39`); optional `metafields` planned via `planWrite` (:141-147). No controller-level `validation{}`; Zod validation lives in the metafield values service.
4. Auth/permissions: `permissions:false`; `rejectWhenAnonymous` → `401 Unauthorized` with `messages.noMemberSession` (`middleware.js:203-214`).
5. Status/headers: 200 with fresh serialized body; no 204/404 documented; `cacheInvalidate:false`; `Cache-Control: private`.
6. Call chain: `account.edit` (`account-service.ts:136-160`): metafield `planWrite(unwrapWire(data.metafields), MEMBERS)` first so a refused value fails whole request (:141-147); `memberBREADService.updateWithMetafields(_.pick(data, WRITABLE_FIELDS), {id, withRelated: WRITE_RELATIONS}, [{writes, origin:{writtenBy:{type:'member',id},source:'portal'}}])` (:48-53, :150-154) → guarded transaction with `get(..., forUpdate:true)`, `memberRepository.update`, `metafieldValues.applyWrite`, `member.edited` action/event + webhooks (`member-bread-service.js:693-724`); then `return this.read(memberId)` (:159) and the same `formattedMemberResponse` serializer as GET.
7. Cache: no data cache; `cacheInvalidate:false`; `member.edited` events may drive downstream invalidation, not an in-process cache here.
8. Output: bare object from fresh read (see GET item 8), reflecting stored state (e.g. `subscribed` round-trips via newsletter list).

**C++**:
1. Route: `.put(... members_account_update().handle ...)` under `/api/member` (`members/routes.cppm:315-322`); handler `MembersAccountUpdateHandler` (`update/handler.cppm:26-33`).
2. Input: `interfaces::io::IRequest&`; id from `x-member-id` header (`update/execute.cppm:53-55`); raw body `in.body_as_string()` (:38) parsed by `pick()` (:56-73), which filters to the same six writable keys (`name, expertise, subscribed, newsletters, enable_comment_notifications, enable_updates_and_announcements`, :61-63) and re-encodes values. `metafields` is not in the allow-list, so it is silently dropped (residual gap).
3. Status/body: same direct writes as read handler (`update/handler.cppm:34-48`) with default status 200 / body `{}` (:42-44).
4. `ctx.store()` calls: `find_by_resource("members", id)` (:35), `edit("members", id, picked)` (:43), re-read `find_by_resource("members", id).value_or(*one)` (:45). Step branches: 401 (handler :35-39), 204 no id (:33-34), 404 store miss (:35-37), 422 validation when `picked.empty()` (:40-42), otherwise `result.status()` with projection (:46-47).
5. Middleware: parent `content_type_middleware` (`members/routes.cppm:20`, dispatch `router.cppm:366-377`); bearer `token()` check (`update/handler.cppm:35`). No 50mb body-parser, no `rejectWhenAnonymous`.
6. Side effects: `s_ops->account_fields_set(id, raw)` (`execute.cppm:44`) writes the full raw body as a cache marker; no events/webhooks, no Stripe reconciliation, no transaction, no metafield writes.
7. Cache: `account_fields_set` writes `members:account:<id>:fields` = raw body with `Ttl{}.never()` and tag `{"members"}` (`operations.cppm:148-152`); read-back overlay uses raw body (`execute.cppm:47`).
8. Output: `account_projection(after, raw)` (`execute.cppm:47`) — same fabricated projection as GET, with `subscribed` read from the overlay (`operations.cppm:182-190`); no `newsletters`/`metafields`/`email_suppression` fields.

**Diff**:
- Residual gaps: real `updateWithMetafields` transaction with row lock, `planWrite`/`applyWrite`, `member.edited` event/webhook fan-out, Stripe subscription reconciliation, email-suppression recompute on email change, metafield validation (Zod) and 50mb body limit, session-based `rejectWhenAnonymous` 401 message.
- Divergences: writable-field filter coincidentally matches Ghost's `WRITABLE_FIELDS` exactly (C++ `execute.cppm:61-63` vs Ghost `account-service.ts:32-39`) — this is the strongest point of alignment; however `metafields` is dropped, a body with only `metafields` yields 422 instead of a valid update; auth is bearer+header not session; JSON content-type is mandatory; the stand-in cache marker `members:account:<id>:fields` (never TTL, tag `members`) has no Ghost equivalent; output projection is hardcoded and diverges on `paid`/fields; 404 on missing member has no Ghost analogue.

### DELETE /members/api/member/suppression — PARTIAL

**Ghost** (facts 1–8):
1. Identity: `DELETE /suppression` on account router (`routes.ts:64-68`): `rejectWhenAnonymous` (:66), `http(destroySuppression)` (:67).
2. Operation: exported key `destroySuppression`, `statusCode: 204`, `headers.cacheInvalidate:false`, `permissions:false`, `query` → `membersService.api.account.allowEmail(member.id, member.email)` (`members-account.ts:44-52`).
3. Input: no body/path params; email and id from session identity (`memberOf(frame)`, :49-50).
4. Auth/permissions: `permissions:false`; `rejectWhenAnonymous` → 401 (`middleware.js:203-214`).
5. Status/headers: 204 No Content; no X-Cache-Invalidate; `Cache-Control: private`.
6. Call chain: `allowEmail(memberId, email)` (`account-service.ts:235-238`): `emailSuppressionList.removeEmail(email)` (:236, provider-backed list) then `members.update({email_disabled:false}, {id: memberId})` (:237, `memberRepository.update`). Two-record write, no transaction wrapper in the service.
7. Cache: none explicit; `cacheInvalidate:false`.
8. Output: empty body (204).

**C++**:
1. Route: `Route{"/suppression"}.delt(... members_account_destroy_suppression().handle ...)` under `/api/member` (`members/routes.cppm:323-333`); handler `MembersAccountDestroySuppressionHandler` (`destroy_suppression/handler.cppm:26-33`).
2. Input: `interfaces::io::IRequest&`; id from `x-member-id` (`destroy_suppression/execute.cppm:55-57`); email parsed out of the stored member JSON (:38-42) — not from an authenticated identity.
3. Status/body: direct writes (`handler.cppm:34-48`), default status 200 / `{}` (:42-44); step returns 401 (handler :35-39), 204 no id (:33-34), 404 store miss (:35-37), 500 on failed removal (:43-45), propagated `edit` status on failure (:46-48), 204 empty on success (:49).
4. `ctx.store()` calls: `find_by_resource("members", id)` (:35), `edit("members", id, R"({"email_disabled":false})")` (:46).
5. Middleware: parent `content_type_middleware` (`routes.cppm:20`, `router.cppm:366-377`) — so DELETE also requires JSON content-type/accept; bearer `token()` check (`handler.cppm:35`).
6. Side effects: `s_ops->remove_suppression(email)` (`execute.cppm:43`) → cache erase of `members:suppression:<email>`; returns whether the key existed (`operations.cppm:139-145`). No ESP/SMTP call, no events/webhooks.
7. Cache: stand-in suppression marker `members:suppression:<email>` erased (`operations.cppm:141-144`); `remove_suppression` returns false when the marker was absent, which yields 500 (Ghost's `removeEmail` is not conditioned on prior presence).
8. Output: empty 204 on success; `ApiError.json` bodies on 404/500 (`execute.cppm:37,44`).

**Diff**:
- Residual gaps: real email suppression provider removal (third-party/SMTP), session-derived email, `member.edited`-style side effects, Ghost's provider semantics where removing an already-absent suppression is not a 500.
- Divergences: bearer+`x-member-id` auth instead of session; mandatory JSON content-type on DELETE; email sourced from stored fixture rather than identity; stand-in cache marker instead of provider list; `false` return from `remove_suppression` produces 500 where Ghost would still complete both writes.
- Alignment: both produce 204 with `email_disabled:false` written to the member and no body; both touch two records (suppression list + member flag).

## Module-level notes

- Only three routes in `web/members/account/routes.ts` bind `members-account.ts`: `GET /` (:29-42), `PUT /` (:44-49), `DELETE /suppression` (:64-68). The fourth route in that router, `GET /metafields/:namespace` → `api.memberMetafieldsMembers.browse` (routes.ts:57-61), binds `member-metafields-members.ts`, a different controller, and is out of scope for this report.
- Neighboring C++ `DELETE /members/:id/suppression` (`members/routes.cppm:140-150` → `members_members_id_suppression_delete`) is the counterpart of the Admin API members controller, not `members-account.ts`; not counted here.
- All three C++ handlers share the same shape: token check directly in `handle()`, a single-step `LocalWorkflowDriver` workflow, and an `Output = std::map<std::string,std::string>` carrying `"status"`/`"body"` (read handler :40-48, update :40-48, destroy :40-48). The workflow/flow files only wire that one step (`read/workflow/flow/main.cppm:8-11`, `update/...:8-11`, `destroy/...:8-11`). `ExecuteStep` holds ctx/ops in static pointers set in each handler's constructor (read handler :29-33, etc.) — process-global mutable state, unlike Ghost's per-request service injection.
- C++ auth is uniformly bearer token via `Authorization` (`bearer.cppm:27-33`) plus member identity via the `x-member-id` header, with no session/cookie machinery and no `emptyWhenAnonymous` behavior. Every C++ account route also inherits `content_type_middleware` from the parent `/members` router (`routes.cppm:20`), requiring JSON Content-Type/Accept even on GET and DELETE; Ghost's members app has no such requirement.
- Shared cache stand-ins in `members/operations.cppm`: suppression marker `members:suppression:<email>` (:497, :139-145), account field overlay `members:account:<id>:fields` (:498, :148-159), key builder (:500-509). These are `core::cache::Cache` entries with tag `{"members"}` and `Ttl{}.never()` for account fields; no Ghost cache service, key, or TTL corresponds to them. No `core::cache::Cache` usage writes an `X-Cache-Invalidate` header on these routes; Ghost suppresses that header via `headers.cacheInvalidate:false` on all three ops.
- Output projection `account_projection` (`operations.cppm:162-202`) is the single most material divergence: it fabricates fields with fixture values (id `"1"`, epoch timestamp, `ghost.local` unsubscribe URL, `expertise:""`, `subscriptions:[]`, fixed notification flags) and disagrees with Ghost on `paid` for `comped` members (`utils.js:43` vs `operations.cppm:193`). Ghost's real serializer conditionally adds `newsletters`, `email_suppression`, and `metafields` (`utils.js:50-61`); C++ never emits them.
- Verdict rationale: each of the three routes has a real C++ counterpart at the exact path+verb with a recognizable core flow (identity → store lookup/mutation → response), so none is MISSING/MISMATCH; but auth model, content-type enforcement, cache stand-ins, and fabricated output keep all three at PARTIAL rather than MATCH.


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
- **No flips (3 stay PARTIAL).** Wire divergences that are fixable in this
  folder were closed: anonymous read → 204, anonymous write → Ghost
  `UnauthorizedError` 401, `paid` now treats `comped` as paid, the account
  projection reads real member fields (expertise/avatar/notifications/
  subscriptions/commenting), `metafields` is accepted as a writable field,
  fresh-read status is 200, `member.edited` / `SuppressionRemoved` outbox
  events are emitted, and removing an absent suppression no longer yields 500.
  **Remaining PARTIAL cause (central, not fixable here):** the parent
  `/members` router installs `content_type_middleware`, so GET/DELETE on
  `/members/api/member*` require a JSON `Content-Type`/`Accept` whereas the
  Ghost members app imposes none; fixing it requires editing the central
  router (`routes.cppm`) — a shared file — so it is recorded as a central need.
  Real session cookies / `emptyWhenAnonymous` identity remain non-observable.
- **Files changed:** all three `members_account_*/handler.cppm` and
  `workflow/step/execute.cppm`, plus `members/operations.cppm`
  (`account_projection` + `members_support`).
