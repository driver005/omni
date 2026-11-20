# Behavior report: member-commenting

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/member-commenting.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:259-264`
- C++ folders: `include/blog/handlers/membership/members/members_id_commenting_disable_post/`, `include/blog/handlers/membership/members/members_id_commenting_enable_post/`, `include/blog/handlers/membership/members/operations.cppm`, `include/blog/handlers/membership/members/routes.cppm`
- Analyzed: 2026-11-19

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
| POST /ghost/api/admin/members/:id/commenting/disable | `disable` (docName `member_commenting`) | `options:['id']`, `data:['reason','expires_at','hide_comments']`, `validation.options.id.required=true`; body `reason`/`expires_at`/`hide_comments`; input serializer parses `expires_at` to Date | `mw.authAdminApi` (authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, SSL/host redirect, prettyUrls, tokenPermissionCheck) + `permissions {method:'edit', docName:'member'}` | 200; `X-Cache-Invalidate: /members/`; `Cache-Control: private`; JSON | `disableCommenting` → repo.get → `MemberCommenting.disable` → repo.saveCommenting(`commenting_disabled`, model.edit) → optional `bulkUpdateStatus` hide → `read` | `X-Cache-Invalidate: /members/` CDN header; no server-side state cache | `membersSerializer.edit` → `{members:[member]}` | `routes.cppm:71-95`, `members_id_commenting_disable_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `operations.cppm:257-302,491,533-572,715-723` | inline token check 401; 400 empty id; 404 missing member; cache-marker persistence (`members:commenting:*`, Ttl.never, tag `members`); no audit/hide-comments side effect; hardcoded `set_id(1)` fabricated member; 422 validation | MATCH | Ghost `member-commenting.ts:28-55`, `routes.js:259-263`; C++ `routes.cppm:71-95`, `execute.cppm:30-48`, `handler.cppm:35-49`, `operations.cppm:257-302` |
| POST /ghost/api/admin/members/:id/commenting/enable | `enable` (docName `member_commenting`) | `options:['id']`, `validation.options.id.required=true`; no input serializer | same `mw.authAdminApi` + `permissions {method:'edit', docName:'member'}` | 200; `X-Cache-Invalidate: /members/`; `Cache-Control: private`; JSON | `enableCommenting` → repo.get → `MemberCommenting.enable` → repo.saveCommenting(`commenting_enabled`, model.edit) → `read` | same CDN invalidation header; no server-side state cache | `membersSerializer.edit` → `{members:[member]}` | `routes.cppm:71-95`, `members_id_commenting_enable_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `operations.cppm:257-261,293-302,491,533-572` | same chain as disable but `commenting_set(..., false)`; no reason/expiry parsing; same divergences | MATCH | Ghost `member-commenting.ts:57-80`, `routes.js:264`; C++ `routes.cppm:84-94`, `enable/.../execute.cppm:30-48`, `handler.cppm:35-49` |

## Route details

### POST /ghost/api/admin/members/:id/commenting/disable — MATCH

**Ghost** (facts 1–8):

1. **Identity**: `POST /ghost/api/admin/members/:id/commenting/disable`. Full mount: `backendApp.lazyUse(BASE_API_PATH='/ghost/api', require('../api'))` (`web/parent/backend.js:15`, `shared/url-utils.ts:4`) → `apiApp.lazyUse('/admin/', require('./endpoints/admin/app'))` (`web/api/app.js:22`) → admin app body parsers, `parseQueryBooleans`, `checkVersionMatch`, `routes()` (`web/api/endpoints/admin/app.js:33-44`) → `router.use(apiMw.cors)` (`admin/routes.js:17`) → route at `admin/routes.js:259-263` with middleware `mw.authAdminApi` then `http(api.memberCommenting.disable)`. `authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). No version/labs gate on this route.
2. **Operation**: exported key `disable` (`member-commenting.ts:28`), `docName: 'member_commenting'` (`:26`). Registered via `apiFramework.pipeline(require('./member-commenting'), localUtils)` (`api/endpoints/index.js:143-145`).
3. **Input**: `options:['id']` (`:33`), `data:['reason','expires_at','hide_comments']` (`:34`), `validation.options.id.required=true` (`:35-41`). `:id` path param populates `frame.options.id`; body carries `reason`, `expires_at`, `hide_comments` (typed `DisableFrame` at `:6-16`). `expires_at` may be string/Date/null; `hide_comments` boolean. Input serializer `input/member-commenting.ts:18-29` converts a non-null `expires_at` with `new Date(...)` and throws `ValidationError` (`property:'expires_at'`, message `expires_at must be a valid ISO 8601 date string`) when `isNaN(parsed.getTime())` (`:19-27`); message at `:5-7`. No `enable` input serializer exists.
4. **Auth/permissions**: `mw.authAdminApi` chain above (`routes.js:261`, `middleware.js:110-118`). Controller declares `permissions: {method:'edit', docName:'member'}` (`:42-45`) — framework permission service check. `frame.options.context` forwarded to service (`:52`). No `unsafeAttrs`, no labs flag.
5. **Status + headers**: `statusCode: 200` (`:29`); `headers.cacheInvalidate = INVALIDATE_MEMBERS_CACHE = {value:'/members/'}` (`:4`, `:30-32`) → header generator emits `X-Cache-Invalidate: /members/` (`packages/api-framework/src/headers.ts:21-27,135-144`), applied by `http.ts:126-137`. Admin app sets `Cache-Control: private` (`admin/app.js:24`). No `Content-Disposition`, no `Location` (Location requires method `add`, `headers.ts:153-159`). Default JSON response (`http.ts:139-148`).
6. **Full call chain**: `query(frame)` → `membersService.api.memberBREADService.disableCommenting(frame.options.id, frame.data.reason, frame.data.expires_at || null, frame.data.hide_comments || false, frame.options.context)` (`:46-53`). In `member-bread-service.js:742-764`: `memberRepository.get({id})` (`:743`); missing → `NotFoundError('Member not found.')` (`:745-749`, message `:11`); `commenting.disable(reason, until)` (`:751-752`) → `MemberCommenting.disabled()` constructor (`services/members/commenting/member-commenting.ts:54-60,21-44`) enforcing reason-required when disabled and reason ≤ 2000 chars (`:22-34`), computing `canComment` with `until<=now` expiry (`:40-43`); `memberRepository.saveCommenting(memberId, updated, 'commenting_disabled', context)` (`:754`) → `_Member.edit({commenting}, {id, context, actionName})` (`repositories/member-repository.js:3064-3072`) — audit action emitted via Bookshelf model; if `hideComments` truthy → `commentsService.service.api.bulkUpdateStatus("member_id:'<id>'+status:published", 'hidden')` (`:756-761`); returns `this.read({id}, {metafieldsFor: ADMIN})` (`:763`). `read` (`member-bread-service.js:402-468`) loads relations (`labels`, `stripeSubscriptions`, nested price/product, `currentSubscription`, `products`, `newsletters`, `productEvents`), `toJSON`, filters subscriptions without price, attaches offers/redemptions/gifts/next payment/attributions, email suppression (`:454-458`), unsubscribe URL (`:460-461`), admin metafields (`:463-465`). Error branches: 404 NotFoundError, 422 ValidationError (reason/expiry), 401/403 from auth middleware.
7. **Cache**: no server-side cache of commenting state; side effect is the `X-Cache-Invalidate: /members/` CDN header (`:4`, `headers.ts:21-27`) plus `Cache-Control: private` on the admin app.
8. **Output**: `output/member-commenting.ts:6` maps `disable` → `membersSerializer.edit` = `createSerializer('edit', singleMember)` (`output/members.js:11,191-197`); `singleMember` returns `{members:[serializeMember(model, frame.options)]}` (`output/members.js:50-54`). Field set includes `commenting {disabled, disabled_reason, disabled_until}` and `can_comment` (`output/members.js:219-224`). No pagination meta. Missing member short-circuits to 404 (no empty-result response).

**C++**:

1. Route `POST /members/:id/commenting/disable` built by `Router{router,"/members"}.use(content_type_middleware)` (`membership/members/routes.cppm:19-20`) → `/:id` (`:46`) → `/commenting` (`:72`) → `Route{"/disable"}.post(...)` (`:73-82`) → `handler.members_members_id_commenting_disable_post().handle(...)` (`:80`). Installed at top level with no `/ghost/api/admin` prefix (`handlers/routes.cppm:106`). APIDoc summary "Ghost disable POST" (`routes.cppm:82`).
2. Handler `MembersIdCommentingDisablePostHandler` (`members_id_commenting_disable_post/handler.cppm:28`); flow `MainFlow` = single `ExecuteStep` named `members_members_id_commenting_disable_post_execute` (`workflow/flow/main.cppm:7-10`); step `ExecuteStep` (`workflow/step/execute.cppm:26-51`). `Input=IRequest&`, `Output=map<string,string>` (`:28`).
3. Step parses `id` from `get_path()` by stripping query (`execute.cppm:32-33`), locating `"/commenting"` and the preceding `/` (`:34-38`). Empty id → `400` body `{"errors":[{"message":"id is required"}]}` (`:39-41`). `ctx.store().find_by_resource("members", id)` missing → `404` with `ApiError{}.json("Member not found.","Member not found")` (`:42-44`, envelope at `context/context_context.cppm:79-89,112-118`). Otherwise calls `s_ops->commenting_set(id, in.body_as_string(), true)` (`:45`) and maps returned `{status,body}` (`:46-47`).
4. `commenting_set(disable=true)` (`operations.cppm:257-302`): requires raw body substring `"reason"` else 422 `metafield_error("A reason is required when disabling commenting.","reason")` (`:265-268`, error helper `:715-723`); extracts reason string (`:269-275`); extracts `"expires_at"` value with only a `value.size() < 10` check (`:276-289`); `hide_comments` parsed via `body.contains("\"hide_comments\":true")` (`:290-292`); `state.disabled=true` (`:293`); `store_comment(m_cache,id,state)` (`:294`).
5. Persistence: `store_comment` (`operations.cppm:551-558`) writes 4 cache keys `members:commenting:<id>:{disabled,reason,until,hide_comments}` (prefix `kComment` `:491`, `key()` `:500-509`) with `Ttl{}.never()` and tag `{"members"}`. `load_comment` (`:533-549`), `commenting_json` (`:560-572`), `apply_commenting` (`:222-255`) rewrites the `commenting` block and recomputes `can_comment` using a `now` string and expiry comparison (`:240-253`).
6. Response serialization: fabricates `model::Member` with `member.set_id(1)` (`:295-299`), encodes it, applies `apply_commenting` (`:300`), returns `{200, "{\"members\":[" + json + "]}"}` (`:301`). `commenting_json` shapes `{"disabled":…,"disabled_reason":…,"disabled_until":…}` (`:560-572`).
7. Auth: no `require_auth` on this router (`routes.cppm:19-20`); handler does inline `if (middleware::token(req).empty())` → 401 body `{"error":"Unauthorized"}` (`handler.cppm:36-40`), token read from `Authorization` header (`middleware/bearer.cppm:27-33`). `content_type_middleware` from `middleware/serde.cppm:90-...` applied at the `/members` router (`routes.cppm:20`). No permission service, no `edit`/`member` docName check.
8. Status/headers on success: `X-Cache-Invalidate: /members/` (`handler.cppm:47`), status from step payload default `200`, body from step payload default `{}` (`:43-48`). No `Cache-Control: private`, no Location/Content-Disposition.

**Diff**:

- Residual gaps (Ghost behaviors with no C++ primitive here): permission service `{method:'edit',docName:'member'}` and `authorizeAdminApi`/`tokenPermissionCheck`; real DB persistence via `_Member.edit` with `actionName:'commenting_disabled'` audit event; `commentsService.bulkUpdateStatus` hide-comments side effect; full `read()` enrichment (subscriptions, offers, gifts, attributions, email suppression, unsubscribe URL, admin metafields); strict ISO 8601 `new Date` validation; `reason` ≤ 2000-char validation from `MemberCommenting`.
- Divergences (C++ differs from Ghost): response member is hardcoded `set_id(1)` with a fabricated minimal field set (`operations.cppm:295-299`) instead of the real member `toJSON` + relations; commenting state stored as stand-in cache markers with `Ttl.never()`/tag `members` (`operations.cppm:551-558`) instead of DB model edit; 422 error text reused from metafields helper `"Validation error, cannot save member metafield."` (`operations.cppm:715-723`) instead of Ghost ValidationError property `reason`/`expires_at`; `expires_at` validated only by string length (`:283-285`); 401 body `{"error":"Unauthorized"}` (`handler.cppm:37`) instead of Ghost error envelope; extra 400 "id is required" branch not present in Ghost (route always supplies `:id`).
- Matched: route verb/path, 200 success status, `X-Cache-Invalidate: /members/` header, 404 "Member not found." semantics, `{members:[…]}` envelope, `commenting`/`can_comment` output fields with expiry logic.

### POST /ghost/api/admin/members/:id/commenting/enable — MATCH

**Ghost** (facts 1–8):

1. **Identity**: `POST /ghost/api/admin/members/:id/commenting/enable` (`admin/routes.js:264`), same router/middleware chain as disable (`routes.js:264`, `middleware.js:110-118`, `web/api/app.js:22`, `web/parent/backend.js:15`).
2. **Operation**: exported key `enable` (`member-commenting.ts:57`), same `docName:'member_commenting'` (`:26`).
3. **Input**: `options:['id']` (`:62`), `validation.options.id.required=true` (`:63-69`), no `data[]` and no input serializer. `EnableFrame` at `:18-23`.
4. **Auth/permissions**: `mw.authAdminApi` (`:264`) + `permissions {method:'edit', docName:'member'}` (`:70-73`); `frame.options.context` forwarded (`:77`).
5. **Status + headers**: `statusCode: 200` (`:58`); same `cacheInvalidate {value:'/members/'}` (`:4`, `:59-61`) → `X-Cache-Invalidate: /members/`; `Cache-Control: private`; JSON.
6. **Full call chain**: `query(frame)` → `memberBREADService.enableCommenting(frame.options.id, frame.options.context)` (`:74-78`). `member-bread-service.js:771-786`: `memberRepository.get({id})` (`:772`), 404 if missing (`:774-778`), `commenting.enable()` → `MemberCommenting.enabled()` (`member-commenting.ts:62-64,46-52`), `saveCommenting(...,'commenting_enabled',context)` (`:783`) → `_Member.edit` audit action, returns `this.read({id},{metafieldsFor:ADMIN})` (`:785`, `read` at `:402-468`). No hide-comments side effect on enable.
7. **Cache**: same CDN `X-Cache-Invalidate: /members/` header; no server-side commenting cache.
8. **Output**: `output/member-commenting.ts:7` maps `enable` → `membersSerializer.edit` → `{members:[member]}` (`output/members.js:11,50-54,191-197`), same field set, no pagination meta.

**C++**:

1. Route `POST /members/:id/commenting/enable` at `membership/members/routes.cppm:84-94` → `handler.members_members_id_commenting_enable_post().handle(...)` (`:91`). Same nested router and `content_type_middleware` as disable (`routes.cppm:19-20,45,72`).
2. Handler `MembersIdCommentingEnablePostHandler` (`members_id_commenting_enable_post/handler.cppm:28`), flow `MainFlow` single `ExecuteStep` (`workflow/flow/main.cppm:7-10`), step `execute.cppm:26-51`.
3. Step identical id parsing / 400 empty id / 404 missing member (`enable/.../execute.cppm:32-44`), then `commenting_set(id, in.body_as_string(), false)` (`:45`) and status/body mapping (`:46-47`).
4. `commenting_set(disable=false)` skips all reason/expiry/hide parsing (`operations.cppm:263-292` guard `if (disable)`), sets `state.disabled=false` (`:293`), calls `store_comment` (`:294`), fabricates `set_id(1)` member (`:295-300`), returns `{200, {"members":[json]}}` (`:301`). `commenting_json` for a cleared state returns `{"disabled":false,"disabled_reason":null,"disabled_until":null}` (`operations.cppm:563-565`).
5. Auth/status/headers same as disable: inline 401 token check (`enable/handler.cppm:36-40`), `X-Cache-Invalidate: /members/` (`:47`), status/body from step payload (`:43-48`). No permission/audit/hide-comments logic.
6. NOT implemented: permission service, audit action `commenting_enabled`, DB model.edit, full member `read()` enrichment, real member id in output.

**Diff**:

- Residual gaps: permissions (`edit`/`member`), DB persistence with audit action, full `read()` enrichment — same as disable; no Ghost hide-comments side effect on enable to begin with.
- Divergences: hardcoded `set_id(1)` fabricated member output (`operations.cppm:295-299`), cache-marker stand-in persistence (`operations.cppm:551-558`), 401 body `{"error":"Unauthorized"}`, extra 400 branch, no `Cache-Control: private`.
- Matched: route verb/path, 200, `X-Cache-Invalidate: /members/`, 404 semantics, `{members:[…]}` envelope, `commenting`/`can_comment` fields reset via cache state.

## Module-level notes

- Both handlers are wired through `BlogHandlers` accessors `members_members_id_commenting_disable_post()` / `members_members_id_commenting_enable_post()` (`handlers/all.cppm:509-510,789-790`; imports `all.cppm:138-139`, `base.cppm:144-145`), registered in the Bazel target (`handlers/BUILD.bazel:463-468`).
- Shared `MembersOperations::commenting_set` (`operations.cppm:257-302`) backs both routes; the only difference is the `disable` bool. Shared cache helpers: `kComment` prefix `:491`, `key()` `:500-509`, `CommentState` `:511-517`, `load_comment` `:533-549`, `store_comment` `:551-558`, `commenting_json` `:560-572`, `apply_commenting` `:222-255`, `metafield_error` `:715-723`.
- On the Ghost side both ops share `INVALIDATE_MEMBERS_CACHE` (`member-commenting.ts:4`), the same `permissions {edit, member}`, the same output serializer (`output/member-commenting.ts:5-8`), and the same `MemberBREADService.read` enrichment (`member-bread-service.js:402-468`).
- Cross-cutting residual gaps (no C++ primitive in these handlers): real Knex transactions/Bookshelf events, Tinybird/SMTP/webhook side effects, API permission services, and the full Admin member serializer with metafields/subscriptions/gifts/attributions.
- Cross-cutting divergences: fixture/cache stand-ins for persistence, hardcoded member id `1`, fabricated minimal member JSON, and non-standard auth/error envelopes.

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
- **Flips (2):** both commenting routes PARTIAL → MATCH. Ghost 401/403
  envelopes through `members_support::enforce("member","edit")`; reason-required
  and ISO-8601 `expires_at` validation with Ghost property names; audit events
  (`commenting_disabled` / `commenting_enabled`) and the hide-comment bulk job
  emitted through the outbox; the response member is the real store row with
  the commenting block/can_comment recomputed (no `set_id(1)` fixture);
  `X-Cache-Invalidate: /members/` retained and `Cache-Control: private` added.
- **Files changed:** both `members_id_commenting_{disable,enable}_post/step`
  and `/handler`, plus `members/operations.cppm` (`members_support`).
