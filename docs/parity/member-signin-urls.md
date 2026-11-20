# Parity: member-signin-urls

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/member-signin-urls.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/members/signin_urls/handler.cppm` (route installed in `include/blog/handlers/membership/members/routes.cppm`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/:id/signin_urls` | `read` (`api.memberSigninUrls.read`) | GET `/members/:id/signin_urls` (`Router "/members"` → `Router "/:id"` → `Route "/signin_urls"`, `.get`) | `members_signin_urls()` → `blog::handlers::members::signin_urls::SigninUrlsHandler` (`handler.cppm:10`) | Ghost `data:['id']`, passes `frame.data` + `frame.options` to `members.get`; C++ `using Input=IRequest&` (`execute.cppm:11`), parses no params, calls hardcoded `member_signin_urls("1")` (`execute.cppm:15`). Ghost-only param: `id` (route placeholder ignored by C++). | Ghost `{member_signin_urls:[{member_id, url}]}` via framework serializer, real magic-link URL, 404 on missing member (`member-signin-urls.js:22-26`); C++ `{"signin_urls":[{"member_id":"1","url":"https://ghost.local/signin/1"}]}`, always 200, no 404 (`operations.cppm:67-75`; `dto/signin_urls_response.cppm:30`; test `api_test.cc:253`). Envelope key differs; inner fields same. | Ghost `membersService.api.members.get(frame.data, frame.options)` + `getMagicLink(model.get('email'), 'signin')` (`member-signin-urls.js:20,28`); C++ no real DB/service call — `member_signin_urls("1")` fabricates a DTO (`operations.cppm:69-74`); `find_by_resource("members", …)` at `execute.cppm:17` is unreachable because `r.first=200` returns early at `:16`. | Ghost `headers.cacheInvalidate:false` (`member-signin-urls.js:14-16`); C++ none (no `core::cache::Cache` use in signin_urls handler/step/flow; `members:*` namespaces in `operations.cppm:269-271` serve other operations). Effectively no cache behavior on either side. | Ghost `mw.authAdminApi` (`admin/routes.js:257`) + `permissions:true`, docName `member_signin_urls` (`member-signin-urls.js:11,18`); C++ `content_type_middleware` (`members/routes.cppm:20`) + handler `token(req).empty()` → 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`), no docName/permission check. | PARTIAL | Ghost `admin/routes.js:257`, `member-signin-urls.js:9-36`, `admin-x-framework/test/unit/api/members.test.tsx:799-805` ↔ C++ `membership/members/routes.cppm:107-117`, `signin_urls/handler.cppm:17-30`, `signin_urls/workflow/step/execute.cppm:11-19`, `signin_urls/workflow/flow/main.cppm:9`, `membership/members/operations.cppm:66-75`, `dto/signin_urls_response.cppm:10-37`, `dto/signin_url.cppm:10-41`, `all.cppm:108,454,703,837` |

## Route details

### GET /members/:id/signin_urls — PARTIAL

Route identity and core operation agree (both return a sign-in URL for a member); divergences below.

- **Operation mapping**: Ghost binds the route to `api.memberSigninUrls.read` → controller `read` (`admin/routes.js:257`; `member-signin-urls.js:13`). C++ binds a dedicated `members_signin_urls()` handler (`members/routes.cppm:114`; `all.cppm:454,703`, registered via `base.cppm:118`), which runs the single-step workflow `members_signin_urls` (`workflow/flow/main.cppm:9`).

- **Input — differ**: Ghost declares `data: ['id']` and passes `frame.data` (containing the route `id`) and `frame.options` to `membersService.api.members.get` (`member-signin-urls.js:17,20`). C++ step declares `using Input = interfaces::io::IRequest&` (`signin_urls/workflow/step/execute.cppm:11`) but never reads the request; it calls `s_ops->member_signin_urls("1")` with a hardcoded id (`execute.cppm:15`). The `:id` path placeholder is never extracted. Param present only on Ghost side: `id`.

- **Output — differ**: Ghost's api-framework serializer wraps the controller's `{member_id, url}` object into `{member_signin_urls: [{member_id, url}]}` (confirmed at `apps/admin-x-framework/test/unit/api/members.test.tsx:799-805`); `url` is a real magic link from `getMagicLink(email, 'signin')` (`member-signin-urls.js:28,30-33`), and a missing member yields HTTP 404 `NotFoundError` "Member not found." (`:22-26`). C++ `member_signin_urls` builds a `SigninUrlsResponse` DTO with `signin_urls:[{member_id:"1", url:"https://ghost.local/signin/1"}]` (`membership/members/operations.cppm:69-74`; DTO fields `signin_urls` at `dto/signin_urls_response.cppm:17,30` and `member_id`/`url` at `dto/signin_url.cppm:19-20,33-34`), so the envelope key is `signin_urls` (not `member_signin_urls`) and values are fabricated — confirmed by C++ test assertion `body().contains(R"("signin_urls":[)")` (`include/blog/testing/api_test.cc:250-253`). The handler always sets `Status::OK` (200) from the step payload (`signin_urls/handler.cppm:26-29`), so no 404 path exists. Inner object field names (`member_id`, `url`) do agree.

- **DB call — differ**: Ghost performs a real member lookup `membersService.api.members.get(...)` and then `membersService.api.getMagicLink(model.get('email'), 'signin')` (`member-signin-urls.js:20,28`). C++ performs no real DB/service read: `member_signin_urls("1")` only synthesizes a DTO (`operations.cppm:67-75`). The `ctx.store().find_by_resource("members", …)` line in the step (`signin_urls/workflow/step/execute.cppm:17`) is unreachable — `member_signin_urls` returns `{200, body}`, so `if (r.first != 0)` at `:16` always returns first.

- **Cache**: Ghost sets `headers.cacheInvalidate: false`, explicitly disabling invalidation (`member-signin-urls.js:14-16`). C++: none — the signin_urls handler, flow, and step contain no `core::cache::Cache` reference; the `members:commenting:` / `members:metafields:` / `members:subscriptions:` namespaces and TTLs in `membership/members/operations.cppm:269-271,301-500` belong to other member operations and are not touched here. Effectively no cache behavior on either side.

- **Auth — differ**: Ghost route uses `mw.authAdminApi` (`admin/routes.js:257`) and controller `permissions: true` under docName `member_signin_urls` (`member-signin-urls.js:11,18`). C++ applies `content_type_middleware` (`members/routes.cppm:20`) and the handler performs only a `token(req).empty()` presence check, returning `401 {"error":"Unauthorized"}` when absent (`signin_urls/handler.cppm:18-22`); there is no docName/permission or admin-role check.

#### Fix

- **Check 1 (route identity) — no change required.** Ghost `router.get('/members/:id/signin_urls', mw.authAdminApi, http(api.memberSigninUrls.read))` (`admin/routes.js:257`) vs C++ `Router{router, "/members"}` (`membership/members/routes.cppm:19`) → `Router{router, "/:id"}` (`:46`) → `Route{"/signin_urls"}.get(...)` (`:107-117`). Path + verb already match after `:id` normalization. Do not add or rename a route.
- **Check 2 (operation) — no change required.** Ghost `read` (`member-signin-urls.js:13`) vs C++ `handler.members_signin_urls()` (`members/routes.cppm:114`; registry `all.cppm:454,703`, constructed at `:703`) running the single-step workflow `members_signin_urls` (`signin_urls/workflow/flow/main.cppm:9`). Keep the existing binding/symbol names; only change the step's behavior below.
- **Check 3 (input) — edit `include/blog/handlers/membership/members/signin_urls/workflow/step/execute.cppm`, `blog::handlers::members::signin_urls::workflow::ExecuteStep::invoke`.** Replace the hardcoded `s_ops->member_signin_urls("1")` (`line 15`) with the `:id` parsed from `in.get_path()` — the trailing segment is `signin_urls`, so take the segment immediately before it, using the `admin/posts/read_by_id/workflow/step/execute.cppm:19-22` pattern. Exact replacement snippet:
  ```cpp
  auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
  while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
  const auto marker = path.find("/signin_urls");
  const auto slash = marker == std::string_view::npos ? std::string_view::npos : path.rfind('/', marker - 1);
  const std::string id{slash == std::string_view::npos ? std::string{} : std::string{path.substr(slash + 1, marker - slash - 1)}};
  if (id.empty()) return core::workflow::StepResponse<Output>{{{"status","400"},{"body",ApiError{}.validation("id is required.", "member_signin_urls", "id")}}};
  ```
  This closes Ghost's `data:['id']` (`member-signin-urls.js:17`); `Input` stays `interfaces::io::IRequest&` (`execute.cppm:11`), no new DTO/type needed.
- **Check 5 (DB/service call) — same step + `include/blog/handlers/membership/members/operations.cppm`, `blog::api::members::MembersOperations::member_signin_urls`.** Make the already-present `ctx.store().find_by_resource("members", id)` (`execute.cppm:17`) reachable by moving it ahead of the fabricated DTO call and feeding it the parsed `id`; on `std::nullopt` return `{404, ApiError{}.json("Member not found.", "member_signin_urls")}` (`ApiError::json` at `context.cppm:66-73`) to mirror Ghost's `NotFoundError` (`member-signin-urls.js:22-26`). Then change the op signature from `std::pair<int, std::string> member_signin_urls(std::string_view id)` (`operations.cppm:67`) to `std::pair<int, std::string> member_signin_urls(std::string_view id, std::string_view member_json)` and call it as `s_ops->member_signin_urls(id, *one)`. Inside the op, replace the fixed `"https://ghost.local/signin/" + std::string{id}` URL (`operations.cppm:72`) with a token read from the existing context-local `core::cache::Cache` (`m_cache`): add a `members:signin_urls:` key alongside `kComment`/`kMeta`/`kSub` in the existing namespace block (`operations.cppm:269-271`) and mint the token with the same counter pattern as `operations.cppm:31-34` (`m_cache.get/put(..., core::cache::Ttl{}.never(), {"members"})`) — no new global state, no third-party crypto. Derive the email from the member JSON via `core_serde::JsonCodec{}.decode<blog::model::Member>(member_json)` (`core/serde/json_codec.cppm:22`; `email` is a serde field at `blog/model/member.cppm:187`, accessor `get_email()` at `:28`), falling back to a `serde::Value` string lookup as in `blog_handlers_middleware/serde.cppm:59` if model decode is unavailable. Ghost's `getMagicLink(email, 'signin')` calls `magicLinkService.getMagicLink({tokenData:{email}, type:'signin'})` (`members-api.js:278-283`) — the signed-token algorithm has no allowed C++ primitive; format an absolute token-bearing URL (e.g. `https://ghost.local/members/signin/?token=<token>`) and record the exact signature as an accepted residual.
- **Check 4 (output) — edit `include/blog/dto/signin_urls_response.cppm:30` and `include/blog/testing/api_test.cc:253`.** Replace `serde::FieldDesc<"signin_urls", &blog::dto::SigninUrlsResponse::get_signin_urls, &blog::dto::SigninUrlsResponse::set_signin_urls, serde::FieldOptions{}>{}` with the same descriptor using the field name `"member_signin_urls"` so the envelope becomes Ghost's `{"member_signin_urls":[{...}]}` (verified `apps/admin-x-framework/src/api/members.ts:794-803`, test `apps/admin-x-framework/test/unit/api/members.test.tsx:799-805`). Keep `blog::dto::SigninUrl` fields `member_id`/`url` (`dto/signin_url.cppm:33-34`) — inner names already match. Update the C++ assertion from `R"("signin_urls":[)"` to `R"("member_signin_urls":[)"` (`api_test.cc:253`). No handler change needed for status: `SigninUrlsHandler::handle` already reads `status`/`body` from the workflow result and calls `res.set_status` (`signin_urls/handler.cppm:25-29`); once the step returns 400/404 those statuses propagate.
- **Check 6 (cache) — no change required.** Ghost `headers.cacheInvalidate:false` (`member-signin-urls.js:14-16`) suppresses `X-Cache-Invalidate`; C++ emits no such header. Effective behavior already agrees — do not add a response header. The only C++ cache work is the context-local `members:signin_urls:*` token from check 5, which reuses the existing `core::cache::Cache` / `{"members"}` tag, not a new store.
- **Check 7 (auth) — edit `include/blog/handlers/membership/members/routes.cppm:19-21`.** Add `.use(blog::handlers::middleware::require_auth)` immediately after `.use(blog::handlers::middleware::content_type_middleware)`; `require_auth` (`include/blog/handlers/middleware/auth.cppm:12-29`) returns Ghost-style 401 `UnauthorizedError` via `write_error`. Delete the redundant inline `token(req).empty()` branch in `signin_urls/handler.cppm:18-22`. Ghost `permissions:true` + `docName:'member_signin_urls'` (`member-signin-urls.js:11,18`) has no allowed C++ primitive — record as an accepted residual, do not invent new global state.
- **Check 8 (verdict) — re-evaluate to `MATCH` after checks 3, 4, 5 and 7 are applied**: route+verb (1), operation (2), input `id` (3), output envelope/status (4), real member lookup + email (5), cache (6) and admin-token auth (7) then agree; formatting/serializer differences are acceptable. Residuals that do not change envelope or status: the magic-link signed-token value and Ghost's `permissions`/`docName` model.

## Missing in C++

None. The single Ghost controller route has a C++ counterpart:

- GET `/members/:id/signin_urls` — C++ GET `/members/:id/signin_urls` (`include/blog/handlers/membership/members/routes.cppm:107-117`).

  Fix: No change required. Check 1 is already closed — the sole Ghost route has an exact path+verb C++ registration (`members/routes.cppm:107-117`); grep for `signin_urls|signinUrls|memberSigninUrls` across `include/blog/handlers/**` found only `members/routes.cppm:108-116`, `signin_urls/handler.cppm`, `signin_urls/workflow/{flow,step}`, and `members/operations.cppm:66-75`. No route needs adding. Checks 2–8 are closed by the single `#### Fix` block above.

## Extra in C++

None. The matched C++ folder `include/blog/handlers/membership/members/signin_urls` installs exactly one route (`signin_urls/handler.cppm`, one `Route{"/signin_urls"}.get` at `membership/members/routes.cppm:108-116`), which has a Ghost controller counterpart. A grep for `signin_urls|signinUrls|magic_link|member_signin` across `include/blog/handlers/**` found no other route registration for this path+verb.

  Fix: No change required. Check 1 is already closed — no C++ route lacks a Ghost counterpart, so nothing to remove or gate; keep the single `Route{"/signin_urls"}.get` at `membership/members/routes.cppm:108-116`. Checks 2–8 are closed by the `#### Fix` block above; the work is to make that existing handler real, not to add or delete routes.


## Group 36 implementation status (2026-11-19)

Source fixes applied (all within `include/blog/handlers/membership/members`):
- `signin_urls/workflow/step/execute.cppm`: `Input = IRequest&`, `Output = std::map<std::string,std::string>`; parses `:id` from the path, returns 400 for a missing id and 404 via `find_by_resource("members", id)` BEFORE building the DTO, then calls `s_ops->member_signin_urls(id, *one)`. Removed the unreachable `find_by_resource` and the hardcoded `"1"`.
- `operations.cppm`: `member_signin_urls(id, member_json)` now decodes the member email, mints a context-local token under the existing `members:signin_urls:` cache namespace (`kSignin`, tag `{"members"}`, `Ttl{}.never()`), and returns `https://ghost.local/members/signin/?token=<token>`.
- `types/dto_signin_urls_response.cppm`: serde field renamed `signin_urls` -> `member_signin_urls` to match Ghost's serializer envelope.

Central registrations / external edits needed:
- `include/blog/testing/api_test.cc:253`: update the assertion from `R"("signin_urls":[)"` to `R"("member_signin_urls":[)"` (test file is outside the allowed folder).
- No `all.cppm` / route changes needed: the handler already receives `{ctx, m_members_ops}` and the route already exists.

Residuals: route-level `require_auth` is blocked by the router middleware-size limit (same as other member routes); inline 401 retained. Ghost's `permissions:true` / `docName:'member_signin_urls'` and the signed-token algorithm have no allowed primitive.


## Group 43 implementation record (strict parity, 2026-11-20)

Verdicts above are superseded by the strict-parity pass; see the authority
implementation record at the end of `docs/parity/behavior/member-signin-urls.md` for route-by-route
evidence. Rewrites closed the observable/wire divergences using the registered
shared abstractions (`core_query`, `core_auth:ghost`, `blog_handlers:outbox_shim`,
`core_serde`), kept helpers in the existing `:members_operations` partition, and
used `Output = std::string` / `std::map` with static `s_ctx`. Only files inside
`include/blog/handlers/membership/members/` and the parity docs changed; no
shared file, BUILD edit or Bazel cycle fix was attempted. Real DB/Stripe/SMTP/
session machinery and non-Administrator permission fidelity remain documented
residuals; new-partition needs (member mapper, real session/permission
middleware, Stripe/SMTP adapters, event-sourced stats, multipart adapter) are
recorded in the behavior reports.
