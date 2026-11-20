# Behavior report: member-signin-urls

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/member-signin-urls.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/membership/members/signin_urls/` + `include/blog/handlers/membership/members/routes.cppm`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /ghost/api/admin/members/:id/signin_urls | `read` (docName `member_signin_urls`) | `data:['id']` from path param `:id`; no `options[]`, no `validation{}`, no body/multipart, no query aliases, no headers read | `mw.authAdminApi` (7 middleware) + `permissions:true` → `canThis(ctx).read.member_signin_url(id,{})` | 200; `headers.cacheInvalidate:false` ⇒ no `X-Cache-Invalidate`; app-level `Cache-Control: private`; JSON | pipeline → permissions → `membersService.api.members.get(frame.data,frame.options)` → `MemberRepository.get` → `Member.findOne({id})`; if null → `NotFoundError("Member not found.")`; `getMagicLink(email,'signin')` → `MagicLink.getMagicLink` → `tokenProvider.create` → `SingleUseTokenModel.add` (DB write) → signed signin URL | No framework cache adapter; no cache header; one DB token row written per call | `{member_signin_urls:[{member_id,url}]}` via default output serializer (no mapper); `url` is real signed magic link | `membership/members/routes.cppm`; `signin_urls/handler.cppm`; `signin_urls/workflow/flow/main.cppm`; `signin_urls/workflow/step/execute.cppm`; `members/operations.cppm`; `types/dto_signin_urls_response.cppm` | 200; inline `token(req)` presence check → 401 `{error:"Unauthorized"}` on empty; `content_type_middleware` 415 gate; id parsed from path; `find_by_resource("members",id)` + wire-fixture fallback → 404 `ApiError.json`; fabricated URL `https://ghost.local/members/signin/?token=signin-<email>-<counter>`; cache marker counter + token written with `Ttl::never()` tag `{"members"}`; envelope `member_signin_urls` | MATCH | Ghost: member-signin-urls.js:11-34, routes.js:257, admin/middleware.js:110-118, permissions.js:21-48, members-api.js:278-283, magic-link.js:132-137, single-use-token-provider.js:48-54, default.js:30-36. C++: routes.cppm:19-20,107-117, handler.cppm:34-46, execute.cppm:29-69, operations.cppm:87-121,494,500-509, dto_signin_urls_response.cppm:30 |

## Route details

### GET /ghost/api/admin/members/:id/signin_urls — MATCH

**Ghost** (facts 1–8):

1. **Identity.** Admin endpoints router registers `router.get('/members/:id/signin_urls', mw.authAdminApi, http(api.memberSigninUrls.read))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:257`). Full external path: parent backend mounts the API at `BASE_API_PATH` = `/ghost/api` (`web/parent/backend.js:15`; `core/shared/url-utils.ts:4`), parent API mounts `/admin/` (`web/api/app.js:22`), so `/ghost/api/admin/members/:id/signin_urls`. Router-level mounts/middleware: `router.use(apiMw.cors)` = `[corsCaching, corsMiddleware]` (`routes.js:17`; `web/api/middleware/index.js:3-5`); parent API applies `versionRewrites`, `contentVersion`, `maxLimitCap` (`web/api/app.js:15-21`); admin app applies `cacheControl('private')`, optional better-auth router, `bodyParser.json/urlencoded` 50mb, `parseQueryBooleans`, `checkVersionMatch`, then `routes()` (`admin/app.js:24-48`). Route middleware `mw.authAdminApi` = `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]` (`admin/middleware.js:110-118`). Wrapped by `http()` (`packages/api-framework/src/http.ts:51-63,86-105`).
2. **Operation.** Exported controller key `read`, `docName: 'member_signin_urls'` (`member-signin-urls.js:10-13`). Registered as `api.memberSigninUrls` via `apiFramework.pipeline(require('./member-signin-urls.js'), localUtils)` (`api/endpoints/index.js:155-157`). Only one operation bound to this controller.
3. **Input.** `data: ['id']` (`member-signin-urls.js:17`) — the `:id` path param is the sole input; `frame.data = {id}`. No `options[]`, no `validation{}`, no `schema`, no multipart fields, no query aliases (`filter[x]=y` etc.), no request headers read by the controller. `http()` builds the `Frame` from `req.query`/`req.params`/`req.body`/files and context (`http.ts:86-105`), but `Frame.configure` keeps only named `data`/`options` keys, so only `id` survives (`pipeline.ts:379-385`). Global shared validation does not require anything for `read` with an `id` present (`validators/input/all.ts`).
4. **Auth/permissions.** `mw.authAdminApi` chain above (`admin/middleware.js:110-118`). `permissions: true` (`member-signin-urls.js:18`) drives `STAGES.permissions` (`pipeline.ts:209-251,434`) → `utils/permissions.handle` (`api/endpoints/utils/permissions.js:94-107`) which parses context and calls `nonePublicAuth`: singularizes `member_signin_urls` → `member_signin_url` (strip trailing `s`, `permissions.js:21-26`), then `permissions.canThis(frame.options.context).read.member_signin_url(frame.options.id, {})` (`permissions.js:27-48`; action map declares `member_signin_url: ['read']` at `services/permissions/permissions-data.ts:39`). `NoPermissionError` → message "You do not have permission to read member_signin_urls" and 403 (`permissions.js:8,66-73`); other errors become `InternalServerError` 500 (`permissions.js:75-81`). No labs flag, no `unsafeAttrs`, no in-query permission service call.
5. **Status + headers.** No `statusCode` in the controller → HTTP 200 (`http.ts:126-133`). `headers: { cacheInvalidate: false }` (`member-signin-urls.js:14-16`); `headers.get` emits `X-Cache-Invalidate` only when truthy (`headers.ts:135-144`), so with `false` **no** invalidation header is produced (contrast default `/*` at `headers.ts:9,21-27`). No `disposition`/`location`/`response.format`/stream declared; `frame.getHeaders()` empty for this route (`headers.ts:146-189`). Admin app adds `Cache-Control: private` (`admin/app.js:24`). Response sent with `res.json(result || {})` (`http.ts:139-148`).
6. **Full call chain.** Pipeline order validation → input serialisation → permissions → query → output serialisation (`pipeline.ts:430-449`). Query body (`member-signin-urls.js:19-34`):
   - `membersService.api.members.get(frame.data, frame.options)` (`member-signin-urls.js:20`). `membersService.api.members` is `memberRepository` (`services/members/members-api/members-api.js:534` with `const users = memberRepository` at line 243). `MemberRepository.get({id}, options)` → `_Member.findOne({id}, options)` (`repositories/member-repository.js:523-539`). Returns a `Member` model or `null`.
   - `!model` → `throw new errors.NotFoundError({message: tpl(messages.memberNotFound)})` where message = "Member not found." (`member-signin-urls.js:5-7,22-26`), handled as 404 by `errorHandler` (`admin/app.js:47-48`).
   - `membersService.api.getMagicLink(model.get('email'), 'signin')` (`member-signin-urls.js:28`) → `members-api.js:278-283` → `magicLinkService.getMagicLink({tokenData:{email}, type:'signin'})` → `core/server/lib/magic-link/magic-link.js:132-137`: `token = await tokenProvider.create({...tokenData, type})` (line 134) then `getSigninURL(token, type, referrer)` (line 136). `tokenProvider` is a `SingleUseTokenProvider` (`services/members/api.js:73-79`); its `create` does `SingleUseTokenModel.add({data: JSON.stringify(data)})` and returns `model.get('token')` (`single-use-token-provider.js:48-54`) — **a real DB token row is written on every call**. `getSigninURL` builds a signed signin URL (`services/members/members-config-provider.js:57`). No email is sent (`sendMagicLink` is not called), no event/job/webhook/third-party call; the only side effect is the single-use-token DB insert.
   - Returns plain object `{member_id: model.get('id'), url: magicLink}` (`member-signin-urls.js:30-33`).
7. **Cache.** No `cache` adapter / `generateCacheKeyData` / TTL / namespace on this controller (`member-signin-urls.js:10-36`). `cacheInvalidate:false` disables the invalidation header (item 5). The only persistent write is the SingleUseToken row from item 6 — not a cache.
8. **Output.** No docName-specific output serializer and no mapper for `member_signin_urls` (`api/endpoints/utils/serializers/output/mappers/index.js`, no key), so `default.all` runs (`utils/serializers/output/default.js:22-41`, selected by `packages/api-framework/src/serializers/handle.ts:152-169`): `response.data` absent ⇒ `frame.response['member_signin_urls'] = [mapResponse(docName, response, frame)]` → `[{member_id, url}]`, no `meta`. `res.json` emits `{"member_signin_urls":[{"member_id":"<id>","url":"<signed-url>"}]}` (`http.ts:147`). Client unwraps `[0]` (`apps/admin-x-framework/src/api/members.ts:794-803`). Missing member → 404 error envelope, not an empty array.

**C++** (current tree):

1. **Files/symbols.** `include/blog/handlers/membership/members/routes.cppm` (`MembersRoutes::install`, line 15; `/signin_urls` GET at lines 107-117); `signin_urls/handler.cppm` (`SigninUrlsHandler`, lines 26-49); `signin_urls/workflow/flow/main.cppm` (`MainFlow::build`, lines 7-10); `signin_urls/workflow/step/execute.cppm` (`ExecuteStep`, lines 24-77); `members/operations.cppm` (`MembersOperations::member_signin_urls`, lines 87-122; constants at 494,500-509); `members/types/dto_signin_url.cppm` (fields, lines 27-42); `members/types/dto_signin_urls_response.cppm` (envelope key, lines 24-38). Registered in `include/blog/handlers/BUILD.bazel:433-435,36-37`; accessor `members_signin_urls()` at `include/blog/handlers/all.cppm:499,779`; installed at `include/blog/handlers/routes.cppm:106`.
2. **Route + middleware.** `Router{router, "/members"}` with `.use(content_type_middleware)` at `routes.cppm:19-20`; `Route{"/signin_urls"}.get(...)` dispatches `handler.members_signin_urls().handle(req,res,send)` (`routes.cppm:107-117`); nested under `/:id` sibling structure (`routes.cppm:45-46`) so effective path is `/members/:id/signin_urls` GET. **No `require_auth` / `bearer_middleware` / permission helper is installed on this router.** `content_type_middleware` rejects missing/non-JSON `Content-Type` with 415 `UnsupportedMediaTypeError` (`include/blog/handlers/middleware/serde.cppm:90-109`).
3. **Auth check (inline, in handler).** `if (blog::handlers::middleware::token(req).empty())` → 401 `UNAUTHORIZED` with body `{"error":"Unauthorized"}` (`handler.cppm:35-39`). `token()` reads `get_authorization()`, stripping a `Bearer ` prefix (`include/blog/handlers/middleware/bearer.cppm:27-33`); `bearer_middleware` that normalizes into `x-blog-token` (lines 11-24) is not applied here. This is a token-**presence** check only — no signature/scope/role/permission validation.
4. **Input parsing.** `ExecuteStep::Input = IRequest&`, `Output = std::map<std::string,std::string>` (`execute.cppm:27`). `id` is extracted from `req.get_path()` by string ops: strip query, strip trailing `/`, find `/signin_urls`, take the preceding path segment (`execute.cppm:31-38`); fallback splits into segments and takes `segments[size-2]` when `!= "members"` (`lines 39-47`). No body/data/options/query-alias parsing. Empty id → 400 with `ApiError{}.validation("id is required.","member_signin_urls","id")` (`lines 48-50`; error envelope shape from `include/blog/handlers/context/context_context.cppm:92-120`).
5. **Store/body writes.** `ctx.store().find_by_resource("members", id)` (`execute.cppm:51`) → `find_entity<model::Member>` + JSON encode (`context_context.cppm:1101-1105`). Fallback: on miss, iterate `parse_array(ctx.store().json_for("members", {}))`, strip quotes from each record's `id`, match against `id`, and use `record_to_json` (`execute.cppm:52-63`; `json_for("members")` → `query_json(m_db.all<model::Member>(), {})` at `context_context.cppm:1165`; `parse_array`/`record_to_json` at lines 611,627). Still missing → 404 with `ApiError{}.json("Member not found.","member_signin_urls")` (`execute.cppm:64-66`). On found record, calls `s_ops->member_signin_urls(id, *one)` (`line 67`) and forwards its `{status,body}` pair (`lines 68-69`).
6. **Magic-link stand-in + cache side effects.** In `operations.cppm:87-122`: extracts `email` from `member_json` (lines 92-104); increments a context-local counter under cache key `members:signin_urls:next` (`m_cache.get` +1 then `m_cache.put(..., Ttl::never(), {"members"})`, lines 107-111); builds `token = std::format("signin-{}-{:032x}", email, counter)` (line 112); writes it under `key(kSignin, id, "token")` = `members:signin_urls:<id>:token` with `Ttl::never()`, tag `{"members"}` (lines 113-114,494,500-509). Response DTO `SigninUrlsResponse` gets one `SigninUrl` with `member_id = id` and `url = "https://ghost.local/members/signin/?token=" + token` (lines 116-120), serialized via `encode(response)` (line 121). `SigninUrl` serializes fields `member_id`, `url` (`dto_signin_url.cppm:27-42`); `SigninUrlsResponse` serializes the array under envelope key **`member_signin_urls`** (`dto_signin_urls_response.cppm:30`). No DB row, no signed token, no SMTP/email, no event/job/webhook.
7. **Status/header writes in handler.** Workflow runs via `LocalWorkflowDriver` with a single `ExecuteStep` (`handler.cppm:29-33`; `flow/main.cppm:8-10`); result read from `result_of("members_signin_urls_execute")`, fallback `{status:"200",body:"{}"}` if no typed output (`handler.cppm:40-42`). `res.set_status(status)`, `res.set_body(bytes)`, `send()` (`lines 43-46`). No `X-Cache-Invalidate` header written, no `Cache-Control`, no `Location`/`Content-Disposition`. 200 on success; 400/401/404/415 on the error paths above.
8. **NOT implemented.** Real admin authentication/authorization chain (`authenticateAdminApi`/`authorizeAdminApi`/`tokenPermissionCheck`) and the `canThis(...).read.member_signin_url` permission service / 403 path; real `Member.findOne` via Knex/Bookshelf; real magic-link token (`SingleUseTokenModel.add`) DB write; signed JWT signin URL (uses hardcoded `https://ghost.local` + counter-based string); SMTP/email side effects (Ghost does not send email here either, so no gap); framework cache invalidation header behavior; real data (falls back to wire-fixture records).

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): permission service `canThis(ctx).read.member_signin_url(id,{})` and 403 `NoPermissionError` (`utils/permissions.js:27-48,66-73` vs C++ token-presence-only at `handler.cppm:35-39`); real `SingleUseTokenModel.add` DB write and signed JWT token (`single-use-token-provider.js:48-54`, `magic-link.js:132-137` vs cache markers at `operations.cppm:107-114`); Bookshelf `Member.findOne` repository path (`member-repository.js:523-539` vs `find_by_resource`/wire-fixture fallback at `execute.cppm:51-63`).
- Divergences:
  - **Magic-link side effect replaced by cache marker.** Ghost writes a single-use token DB row and returns a signed URL; C++ only increments a cache counter and writes a token string with `Ttl::never()` + tag `{"members"}` under keys `members:signin_urls:next` / `members:signin_urls:<id>:token` (`operations.cppm:107-114`). Token is not signed and URL host is hardcoded `https://ghost.local` (`operations.cppm:119`).
  - **Auth model differs.** Ghost uses full `mw.authAdminApi` + `permissions:true` (`routes.js:257`, `member-signin-urls.js:18`); C++ has no auth middleware on the route and checks only that an Authorization header is non-empty, returning `{"error":"Unauthorized"}` (singular `error`) instead of Ghost's `{"errors":[{...}]}` envelope (`handler.cppm:35-39`, `context_context.cppm:112-120`).
  - **Extra 415 gate.** C++ `content_type_middleware` rejects a GET without JSON `Content-Type` (`serde.cppm:90-109`); Ghost's GET route imposes no such requirement (`routes.js:257`).
  - **Input extraction.** Ghost passes `data:['id']` through the framework `Frame` (`member-signin-urls.js:17`, `pipeline.ts:379-385`); C++ re-parses the raw URL path with string ops and adds an explicit 400 ValidationError for empty id that Ghost cannot hit because the route requires `:id` (`execute.cppm:31-50`).
  - **404 error envelope context.** Ghost `NotFoundError` message "Member not found." with type `NotFoundError` (`member-signin-urls.js:5-7,22-26`); C++ returns `{"errors":[{...,"context":"member_signin_urls","type":null,...}]}` with the same message text (`execute.cppm:64-66`, `context_context.cppm:92-120`).
  - **Member lookup source.** C++ may resolve the member from wire-fixture records (`execute.cppm:53-62`, `model_wire_fixtures.cppm`) rather than a real DB row; Ghost always uses `Member.findOne` (`member-repository.js:523-539`).
  - Envelope key, success status, and field set **do match**: both produce `{"member_signin_urls":[{"member_id","url"}]}` with 200 (`default.js:30-36` vs `dto_signin_urls_response.cppm:30`, `dto_signin_url.cppm:33-35`).

## Module-level notes

- Only one route is bound to this controller: `GET /members/:id/signin_urls` → `read` (`routes.js:257`); C++ binds exactly the same path+verb (`members/routes.cppm:107-117`). No other verb/operation exists on either side.
- The C++ `members` route installer applies only `content_type_middleware` at the `/members` router level (`routes.cppm:19-20`); the signin_urls handler performs its own token check inline (`handler.cppm:35-39`) rather than relying on a shared auth middleware, unlike other handler folders that use `require_auth`.
- Cross-route side effect: the `members:signin_urls:next` counter and per-id token markers live in the same `core::cache::Cache` with tag `{"members"}` and `Ttl::never()`, so they are shared context-local state with other members operations (`operations.cppm:494,500-509`); a cache flush tagged `members` would clear them. Ghost has no analogous cache state — its per-call token is a durable DB row.
- Both sides avoid sending email: Ghost calls `getMagicLink` (token create + URL build) not `sendMagicLink` (`members-api.js:278-283` vs `magic-link.js:74-121`); C++ writes no outbox/email record (`operations.cppm:87-122`).

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
  `enforce("member_signin_url","read")`; 404 `Member not found.` envelope;
  no wire-fixture fallback (real store row only); the single-use token write
  is observable as a `SingleUseTokenCreated` outbox event alongside the
  context-local token marker; no `X-Cache-Invalidate` header.
- **Files changed:** `signin_urls/workflow/step/execute.cppm`,
  `signin_urls/handler.cppm`, `members/operations.cppm` (`members_support`).
