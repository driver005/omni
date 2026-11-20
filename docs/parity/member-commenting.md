# Parity: member-commenting

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/member-commenting.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (the exported key `member_commenting` is registered as `api.memberCommenting` in `docs/Ghost/ghost/core/server/api/endpoints/index.js:143-145`; routes at `admin/routes.js:259-264`. These are Admin API routes, not a separate members web-app `routes.js`.)
- C++: `include/blog/handlers/membership/members/routes.cppm` (folder found by exact path+verb search, not assumed); handlers under `include/blog/handlers/membership/members/members_id_commenting_{disable,enable}_post/`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/members/:id/commenting/disable` | `disable` | POST `/members/:id/commenting/disable` (`Router "/members"` → `"/:id"` → `"/commenting"` → `Route "/disable"`, `.post()`) | `MembersIdCommentingDisablePostHandler` | Ghost: `options:['id']` (from path), `data:['reason','expires_at','hide_comments']`, id required, `expires_at` parsed to Date, `hide_comments` default `false`. C++: raw `IRequest` body string, hardcoded id `"1"`, parses `reason` and `disabled_until` (not `expires_at`), `hide_comments` default `true` unless explicitly `false`; no DTO/validation of id. → differ | Ghost serializer `membersSerializer.edit` → `{members:[full serialized member]}` for requested id, statusCode 200. C++ returns `mutations.create("members", raw_body).body()` → a *newly created* member with generated id, status hardcoded `200`. → envelope shape similar, data differs | Ghost: `memberRepository.get({id})`, `commenting.disable(reason, until)`, `saveCommenting(...,'commenting_disabled')`, optional `commentsService.bulkUpdateStatus(...'hidden')`, then `this.read(...)`. C++: `s_ops->commenting_set("1",body,true)` writes `members:commenting:*` cache keys, then `ctx.store().create("members", raw_body)`. → differ | Ghost: `X-Cache-Invalidate: /members/` response header (`cacheInvalidate`). C++: cache keys `members:commenting:<id>:{disabled,reason,until,hide_comments}`, TTL never, tag `{"members"}`; no invalidate header. → differ | Ghost: `mw.authAdminApi` + permissions `{method:'edit',docName:'member'}`. C++: inline `token(req).empty()` check → `401 {"error":"Unauthorized"}`, no permission/role check. → differ | PARTIAL | Ghost: `admin/routes.js:259-263`, `member-commenting.ts:28-55`, `.../input/member-commenting.ts:18-29`, `.../output/member-commenting.ts:6`, `member-bread-service.js:742-764`. C++: `membership/members/routes.cppm:19,46,71-83`, `.../disable_post/handler.cppm:18-30`, `.../disable_post/workflow/step/execute.cppm:12-18`, `membership/members/operations.cppm:117-162,269,319-326` |
| POST `/members/:id/commenting/enable` | `enable` | POST `/members/:id/commenting/enable` (`Router "/members"` → `"/:id"` → `"/commenting"` → `Route "/enable"`, `.post()`) | `MembersIdCommentingEnablePostHandler` | Ghost: `options:['id']` (from path), id required; no data fields. C++: raw `IRequest` body string, hardcoded id `"1"`, `commenting_set(...,false)` ignores body fields; no DTO/validation of id. → differ | Ghost serializer `membersSerializer.edit` → `{members:[full serialized member]}` for requested id, statusCode 200. C++ returns `mutations.create("members", raw_body).body()` → newly created member with generated id, status hardcoded `200`. → envelope shape similar, data differs | Ghost: `memberRepository.get({id})`, `commenting.enable()`, `saveCommenting(...,'commenting_enabled')`, then `this.read(...)`. C++: `s_ops->commenting_set("1",body,false)` writes `members:commenting:*` cache keys, then `ctx.store().create("members", raw_body)`. → differ | Ghost: `X-Cache-Invalidate: /members/`. C++: same `members:commenting:<id>:*` keys, TTL never, tag `{"members"}`; `commenting_json` returns `disabled:false`; no invalidate header. → differ | Ghost: `mw.authAdminApi` + permissions `{method:'edit',docName:'member'}`. C++: inline token check → `401 {"error":"Unauthorized"}`. → differ | PARTIAL | Ghost: `admin/routes.js:264`, `member-commenting.ts:57-80`, `.../output/member-commenting.ts:7`, `member-bread-service.js:771-786`. C++: `membership/members/routes.cppm:19,46,84-94`, `.../enable_post/handler.cppm:18-30`, `.../enable_post/workflow/step/execute.cppm:12-18`, `membership/members/operations.cppm:117-162,269,319-326` |

## Route details

### POST `/members/:id/commenting/disable` — PARTIAL

- **Route identity / operation — no differences.** Ghost `router.post('/members/:id/commenting/disable', mw.authAdminApi, http(api.memberCommenting.disable))` (`admin/routes.js:259-263`) vs C++ `Router{router,"/members"}` (`:19`) → `Router{router,"/:id"}` (`:46`) → `Router{router,"/commenting"}` (`:72`) → `Route{"/disable"}.post()` (`:74-75`). Both normalize to POST `/members/:id/commenting/disable`.
- **Input differs.** Ghost `options:['id']`, `data:['reason','expires_at','hide_comments']`, `id` required, `expires_at` converted to `Date` (`member-commenting.ts:33-41`; `utils/serializers/input/member-commenting.ts:18-29`). C++ `using Input = interfaces::io::IRequest&` (no DTO), passes hardcoded `"1"` instead of the `:id` path param and reads raw body via string search for `"reason"` and `"disabled_until"` — `expires_at` is never read (`members_id_commenting_disable_post/workflow/step/execute.cppm:12,16`; `membership/members/operations.cppm:128-150`). `hide_comments` defaults to `true` in C++ (`!body.contains("\"hide_comments\":false")`, `operations.cppm:151`) vs `false` in Ghost (`member-commenting.ts:51`). Missing on C++ side: `id` (path param ignored), `expires_at` (renamed `disabled_until`).
- **Output differs.** Ghost `output/member-commenting.ts:6` → `membersSerializer.edit` → `createSerializer('edit', singleMember)` → `{members:[serializeMember(...)]}` (`output/members.js:11,50-54,191-196`), statusCode 200 (`member-commenting.ts:29`). C++ returns `mutations.create("members", raw_body).body()` wrapped as `{"status":"200",...}` (`execute.cppm:17-18`; `handler.cppm:26-30`); the created member has a generated id (`context.cppm:1259-1260,1271-1275`), not the existing member for `:id`, and no commenting overlay is returned on the success path.
- **DB call differs.** Ghost `memberRepository.get({id})` → `commenting.disable(reason, until)` → `saveCommenting(memberId, updated, 'commenting_disabled', context)` → optional `commentsService.service.api.bulkUpdateStatus("member_id:'<id>'+status:published",'hidden')` → `this.read({id}, {metafieldsFor: ADMIN})` (`member-bread-service.js:743-763`). C++ `commenting_set` only writes cache (`operations.cppm:154`) then `ctx.store().create("members", raw_body)` (`execute.cppm:18`; `context.cppm:1205,1259-1260`). No member `get`/`edit` and no comment bulk hide on C++ side.
- **Cache differs.** Ghost emits `cacheInvalidate: {value:'/members/'}` response header (`member-commenting.ts:4,30-32`). C++ stores state under `members:commenting:<id>:{disabled,reason,until,hide_comments}` with `Ttl{}.never()` and tag `{"members"}` (`operations.cppm:269,319-326`); `apply_commenting`/`commenting_json` read them back (`:88-115,328-340`). No `X-Cache-Invalidate` header is set by the C++ handler.
- **Auth differs.** Ghost `mw.authAdminApi` plus permissions `{method:'edit', docName:'member'}` (`admin/routes.js:261`; `member-commenting.ts:42-45`). C++ has no auth middleware on the route (`membership/members/routes.cppm:20` only `content_type_middleware`); the handler checks `middleware::token(req).empty()` and returns `401 {"error":"Unauthorized"}` (`disable_post/handler.cppm:19-23`; `middleware/bearer.cppm:27-33`). No permission/docName enforcement.

#### Fix
Shared ingredients for both commenting routes (define once, used below):

- **blog_context state / allowed modules** — no new global state. `blog::api::members::MembersOperations` already owns `core::cache::Cache& m_cache` (`membership/members/operations.cppm:18-21,517`) and persists commenting state under the existing namespace `members:commenting:<id>:{disabled,reason,until,hide_comments}` via `store_comment` (`:269,273-282,319-326`) with `Ttl{}.never()` and tag `{"members"}` — reuse this; it is context-local cache under an existing namespace. For the real member record use `blog::BlogContext::store()` → `ApiDataStore::find_by_resource("members", id)` (`include/blog/context/context.cppm:1055`), called from the step exactly as `admin/posts/read_by_id/workflow/step/execute.cppm:19-27` does (no direct SQL, no third-party libs).
- **Check 1 (route identity) — no change.** `membership/members/routes.cppm:19,46,71-83` already mounts `POST /members/:id/commenting/disable`, matching `admin/routes.js:259-263`.
- **Check 2 (operation)** — implement Ghost `disable` in `blog::handlers::members::members_id_commenting_disable_post::workflow::ExecuteStep::invoke` (`members_id_commenting_disable_post/workflow/step/execute.cppm:14-19`) by returning `s_ops->commenting_set(id, in.body_as_string(), true)` verbatim; keep the existing handler/flow binding and symbol name.
- **Check 3 (input)** — `IRequest` exposes `get_path()` but no route-param accessor, so parse `:id` from the path, then pass `id` (not the hardcoded `"1"` at `execute.cppm:16`) to `commenting_set`. In `MembersOperations::commenting_set` (`operations.cppm:117-162`): rename body key `"disabled_until"` → `"expires_at"` (`:139-146`) to match Ghost `data:['reason','expires_at','hide_comments']` (`member-commenting.ts:34`); change `state.hide_comments = !body.contains("\"hide_comments\":false")` (`:151`) to `state.hide_comments = body.contains("\"hide_comments\":true")` so the Ghost default `false` applies (`member-commenting.ts:51`); change `CommentState::hide_comments` default (`:289`) and the `load_comment` fallback (`:313-315`) from `true` to `false`. Typed alternative: add `blog::dto::CommentingDisableInput` (serde fields `reason`, `expires_at`, `hide_comments`) under `blog_dto`, decode via `blog::handlers::middleware::decode<...>(in.body_as_string())` (pattern at `operations.cppm:29,53`), then assign `state.until = dto->expires_at` and `state.hide_comments = dto->hide_comments`. Return 404/400 for a missing/empty `id` as Ghost's `validation.options.id.required` does (`member-commenting.ts:36-41`). Reason-required already exists (`operations.cppm:128-131`); align its message/property with `member-commenting.ts:22-26` + `member-commenting.ts:5` reasonRequired if exact error text is required.
  ```cpp
  auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
  const auto a = path.find("/commenting");
  const auto b = a == std::string_view::npos ? std::string_view::npos : path.rfind('/', a - 1);
  const std::string id{b == std::string_view::npos ? std::string{} : path.substr(b + 1, a - b - 1)};
  if (id.empty()) return core::workflow::StepResponse<Output>{{{"status","400"},{"body",R"({"errors":[{"message":"id is required"}]})"}}};
  if (!ctx.store().find_by_resource("members", id))
      return core::workflow::StepResponse<Output>{{{"status","404"},{"body",R"({"errors":[{"message":"Member not found."}]})"}}};
  const auto r = s_ops->commenting_set(id, in.body_as_string(), true);
  return core::workflow::StepResponse<Output>{{{"status", std::to_string(r.first)},{"body", r.second}}};
  ```
- **Check 4 (output)** — `commenting_set` already builds `{200, {"members":[<member + commenting overlay>]}}` (`operations.cppm:154-161`) through `apply_commenting`/`commenting_json` (`:87-115,328-340`), which matches `output/member-commenting.ts:6` → `membersSerializer.edit` → `{members:[...]}`. Return `r.second` on success and delete `mutations.create("members", ...)` (`execute.cppm:18`) — that creates a generated-id member, not the requested `:id`. Replace the sentinel 404 (`id.size()>20 && all 'a'`, `operations.cppm:123-125`) with the real `find_by_resource` miss in the snippet so any absent member → 404 (`member-bread-service.js:745-749`). For `can_comment` parity, derive it from `disabled_until` expiry (`MemberCommenting.canComment`, `services/members/commenting/member-commenting.ts:40-43`); C++ currently uses `!state.disabled` only (`operations.cppm:104-113`). Status 200 already correct (`member-commenting.ts:29`).
- **Check 5 (DB)** — C++ write is the `store_comment` cache put (`operations.cppm:319-326`) plus `find_by_resource("members", id)` existence read; this stands in for Ghost `memberRepository.get({id})` + `saveCommenting(memberId, updated, 'commenting_disabled', context)` (`member-bread-service.js:743,754`). Remove `mutations.create`. Residual (no allowed primitive): `commentsService.service.api.bulkUpdateStatus("member_id:'<id>'+status:published",'hidden')` (`member-bread-service.js:756-760`) and the audit-action label `'commenting_disabled'`.
- **Check 6 (cache)** — keep `members:commenting:<id>:*` keys, `Ttl{}.never()`, tag `{"members"}` (`operations.cppm:269,319-326`). Add Ghost's invalidation header in `MembersIdCommentingDisablePostHandler::handle` (`disable_post/handler.cppm:18-31`): `res.set_header("X-Cache-Invalidate", "/members/");` before `send()` (`IResponse::set_header` variant overload, `include/interfaces/io/response.cppm:590-594`; header from `member-commenting.ts:4,30-32`). Align validation error `metafield_error("Invalid date format.", "disabled_until")` (`operations.cppm:146`) → `metafield_error("expires_at must be a valid ISO 8601 date string", "expires_at")` (input serializer `.../input/member-commenting.ts:6,19-27`); residual: wrapper text "cannot save member metafield." (`operations.cppm:462-464`) differs from Ghost ValidationError text.
- **Check 7 (auth)** — add `.use(blog::handlers::middleware::require_auth)` immediately after `.use(content_type_middleware)` at `membership/members/routes.cppm:20`; signature `IRequest&, IResponse&, NextFn&&, std::function<void()>` matches `middleware/auth.cppm:12-29`. Keep or drop the inline `token(req).empty()` 401 (`disable_post/handler.cppm:19-23`) — `require_auth` emits the same 401. `permissions {method:'edit', docName:'member'}` (`member-commenting.ts:42-45`) has no C++ permission primitive — residual; admin-token enforcement closes.
- **Check 8** — re-evaluate to `MATCH` once 3–7 are applied; residuals (no allowed C++/context-local equivalent) are the comment bulk-hide side effect, the audit-action label, and fine-grained permission scope.

### POST `/members/:id/commenting/enable` — PARTIAL

- **Route identity / operation — no differences.** Ghost `router.post('/members/:id/commenting/enable', mw.authAdminApi, http(api.memberCommenting.enable))` (`admin/routes.js:264`) vs C++ `Route{"/enable"}.post()` under `/members/:id/commenting` (`membership/members/routes.cppm:72,84-92`).
- **Input differs.** Ghost `options:['id']` only, `id` required (`member-commenting.ts:62-69`). C++ `using Input = interfaces::io::IRequest&`, hardcoded id `"1"` instead of `:id`, body unused (`enable_post/workflow/step/execute.cppm:12,16`). No DTO, no id validation.
- **Output differs.** Ghost serializer `membersSerializer.edit` → `{members:[...]}` for the requested id (`output/member-commenting.ts:7`; `output/members.js:11,50-54`), statusCode 200 (`member-commenting.ts:58`). C++ returns `mutations.create("members", raw_body).body()` for a new generated-id member, status hardcoded 200 (`enable_post/workflow/step/execute.cppm:17-18`; `handler.cppm:26-30`).
- **DB call differs.** Ghost `memberRepository.get({id})` → `commenting.enable()` → `saveCommenting(...,'commenting_enabled')` → `this.read(...)` (`member-bread-service.js:772-785`). C++ `commenting_set("1", body, false)` (`operations.cppm:117-162`) sets the `members:commenting:*` cache keys to disabled=false and then creates a new `members` record via `ctx.store().create(...)` (`execute.cppm:16,18`).
- **Cache differs.** Ghost `X-Cache-Invalidate: /members/` header (`member-commenting.ts:4,59-61`). C++ cache keys/TTL/tag as above; `commenting_json` emits `{disabled:false,disabled_reason:null,disabled_until:null}` when cleared (`operations.cppm:331-333`). No invalidation header on C++.
- **Auth differs.** Ghost `mw.authAdminApi` + `{method:'edit', docName:'member'}` (`admin/routes.js:264`; `member-commenting.ts:70-73`) vs C++ inline token-presence check returning `401 {"error":"Unauthorized"}` (`enable_post/handler.cppm:19-23`). No permission check.

#### Fix
Reuses the shared `blog_context`/`MembersOperations` cache and `require_auth` wiring from the disable Fix above; no new state or modules.

- **Check 1 (route identity) — no change.** `membership/members/routes.cppm:19,46,71-72,84-94` already mounts `POST /members/:id/commenting/enable`, matching `admin/routes.js:264`.
- **Check 2 (operation)** — implement Ghost `enable` in `blog::handlers::members::members_id_commenting_enable_post::workflow::ExecuteStep::invoke` (`members_id_commenting_enable_post/workflow/step/execute.cppm:14-19`) by returning `s_ops->commenting_set(id, in.body_as_string(), false)` verbatim; keep the `members_members_id_commenting_enable_post` binding and handler symbol.
- **Check 3 (input)** — Ghost takes only `options:['id']` (`member-commenting.ts:62-69`); body is unused. Parse `:id` from `in.get_path()` with the identical snippet in the disable Fix (replace `true` with `false` in the `commenting_set` call) and pass that `id` instead of hardcoded `"1"` (`enable_post/.../execute.cppm:16`). No DTO needed; no `hide_comments`/`expires_at` fields apply on enable.
- **Check 4 (output)** — `commenting_set(..., false)` already returns `{200, {"members":[...]}}` with cleared overlay — `commenting_json` emits `{"disabled":false,"disabled_reason":null,"disabled_until":null}` (`operations.cppm:331-333`) — matching `output/member-commenting.ts:7` → `membersSerializer.edit` → `{members:[...]}`, status 200 (`member-commenting.ts:58`). Return `r.second` on success; delete `mutations.create("members", ...)` (`execute.cppm:18`). Use the real `find_by_resource("members", id)` miss for 404 (`member-bread-service.js:774-778`) in place of the `'a'`-sentinel (`operations.cppm:123-125`). `can_comment` should become `true` when `disabled=false` (already produced by `apply_commenting`, `operations.cppm:104-113`).
- **Check 5 (DB)** — C++ write is `store_comment` cache put (`operations.cppm:319-326`) with `state.disabled=false` (`:153-154`), standing in for Ghost `memberRepository.get({id})` + `saveCommenting(...,'commenting_enabled',context)` + `this.read(...)` (`member-bread-service.js:772-785`). Remove `mutations.create`. Residual: audit-action label `'commenting_enabled'` has no C++ primitive.
- **Check 6 (cache)** — same keys/`Ttl{}.never()`/tag `{"members"}` as disable (`operations.cppm:269,319-326`). Add `res.set_header("X-Cache-Invalidate", "/members/");` in `MembersIdCommentingEnablePostHandler::handle` (`enable_post/handler.cppm:18-31`) before `send()`; header from `member-commenting.ts:4,59-61`.
- **Check 7 (auth)** — same `.use(require_auth)` addition at `membership/members/routes.cppm:20` (once for the shared `/members` router, covering both routes); keep or drop the inline 401 at `enable_post/handler.cppm:19-23`. Fine-grained `edit.member` scope remains residual.
- **Check 8** — re-evaluate to `MATCH` once 3–6 are applied; residuals are the audit-action label and fine-grained permission scope.

## Missing in C++

None. Both Ghost routes have exact path+verb C++ counterparts.
  - Fix: None — no change required. Check 1 is already closed for both Ghost routes (`admin/routes.js:259-264` ↔ `membership/members/routes.cppm:71-94`); the work is to make the existing handlers real (see the two `#### Fix` blocks above), not to add routes.

## Extra in C++

None for this domain. The two commenting routes in `include/blog/handlers/membership/members/routes.cppm:71-94` map 1:1 to the two Ghost routes. Other routes in that file (browse/read/edit/destroy/subscriptions/signin_urls/metafields, etc.) belong to other Ghost controllers and are out of scope for this controller.
  - Fix: None — no change required. No C++ commenting route is extra; nothing to remove or gate. Check 1 already closed.


## Group 36 implementation status (2026-11-19)

Source fixes applied (all within `include/blog/handlers/membership/members`):
- `operations.cppm`: `commenting_set` now reads `expires_at` (was `disabled_until`), defaults `hide_comments` to `false` (body must contain `"hide_comments":true` to hide), uses Ghost-aligned validation text, derives `can_comment` from expiry, and no longer uses the all-`a` sentinel id.
- `members_id_commenting_disable_post` / `members_id_commenting_enable_post` steps: parse `:id` from `in.get_path()`, return 400 for missing id / 404 via `find_by_resource("members", id)`, return the `commenting_set` result verbatim; removed the spurious `mutations.create("members", ...)` that created a generated-id member.
- Both handlers set `X-Cache-Invalidate: /members/` before `send()`.

Central registrations needed: **none** for this controller — the two handlers/flows/steps already exist and are wired with `{ctx, m_members_ops}` in `include/blog/handlers/all.cppm`, and both routes already exist in `membership/members/routes.cppm`.

Residuals (no allowed primitive / blocked by shared-file constraint):
- Route-level `.use(blog::handlers::middleware::require_auth)` was NOT added to the shared `/members` router: adding it builds but trips the router's `MiddlewareSize > MaxHandlerSize` offerflow limit at startup (verified with `bazel test //include/blog/testing:api_test_test`). Inline `token(req).empty()` 401 checks remain.
- Comment bulk-hide side effect, audit-action labels (`commenting_disabled` / `commenting_enabled`) and fine-grained `edit.member` permission scope remain GAPs with no allowed C++ primitive.


## Group 43 implementation record (strict parity, 2026-11-20)

Verdicts above are superseded by the strict-parity pass; see the authority
implementation record at the end of `docs/parity/behavior/member-commenting.md` for route-by-route
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
