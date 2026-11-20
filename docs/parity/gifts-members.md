# Parity: gifts-members

> **Superseded by `docs/parity/behavior/gifts-members.md` (strict-parity rewrite, manifest group 40, 2026-11-20).** The two route rows below were MISSING when first analyzed; the handlers, steps and central registrations have since been added and rewritten for strict Ghost behavior. Both now flip MISSING → MATCH (see the behavior report for the authoritative route-by-route evidence and the residual list). The historical Fix blocks are kept for traceability but their file layout differs from the implemented `gifts_members/redeem_{get,post}` folders.

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/members/app.js` (members app; no separate members `routes.js` — routes registered inline)
- C++: none found (no redeem route/handler anywhere under `include/blog/handlers`)
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
| GET `/api/gifts/:token/redeem` | `getRedeemable` | none | none | `token` (route param `:token`, also `data:['token']`, validated string/required); member status read from `options.context.member.status` (nullable) | single `GiftRedemption` object (`token`, `cadence`, `duration`, `currency`, `amount`, `buyer_name`, `recipient_name`, `recipient_email`, `message`, `expires_at`, `consumes_at`, `tier{id,name,description,benefits}`) via `serializeRedemption`; no `{data,meta}` wrapper; default 200 | `giftRepository.getByToken(token)`; throws `NotFoundError` (`GIFT_NOT_FOUND`) if absent; `assertRedeemable(gift, memberStatus)` | none (`headers.cacheInvalidate: false`) | `permissions: false`; `middleware.loadMemberSession`; member session optional (status only) | MATCH (superseded) | Ghost: `members/app.js:159-163`, `gifts-members.js:7-24`, `gift-controller.ts:25-30`, `gift-service.ts:816-832`, `gift-service.ts:282-300`. C++: no route — grep `redeem` over `include/` hits only schema fields (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`); grep `giftsMembers\|gifts_members\|gifts-members` over `include/` = 0 matches |
| POST `/api/gifts/:token/redeem` | `redeem` | none | none | `token` (route param `:token`, also `data:['token']`, validated string/required); `member.id` required from context else `UnauthorizedError`; `bodyParser.json({limit:'50mb'})` mounted on route | single `GiftRedemption` object via `serializeRedemption`; `statusCode: 200`; no `{data,meta}` wrapper | transactional: `giftRepository.getByToken(forUpdate)`, `memberRepository.get`/`update` (status `gift`, product with `expiry_at`), `giftRepository.update`, `giftDeliveryService.cancelPendingForGift`, `tiersService.api.read`, staff email notify, `giftReminderScheduler.scheduleAt` | none (`headers.cacheInvalidate: false`) | `permissions: false`; `middleware.loadMemberSession`; member auth enforced in service (`!member.id` → `UnauthorizedError`) | MATCH (superseded) | Ghost: `members/app.js:164-169`, `gifts-members.js:26-44`, `gift-controller.ts:32-46`, `gift-service.ts:834-891`, `gift-service.ts:893-935`. C++: no route — grep `redeem` over `include/` hits only schema fields (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`); grep `giftsMembers\|gifts_members\|gifts-members` over `include/` = 0 matches |

## Route details

### GET /api/gifts/:token/redeem — MISSING

- Route + op: registered in members app with `middleware.loadMemberSession` then `http(api.giftsMembers.getRedeemable)` (`docs/Ghost/ghost/core/server/web/members/app.js:159-163`); controller key `getRedeemable` with `docName: 'gifts'` (`docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js:5,7-24`).
- Input: `data:['token']`, `token` validated `{type:'string', required:true}` (`gifts-members.js:11-19`); token supplied as URL param `:token` (`members/app.js:160`). Member status read from `frame.options?.context?.member?.status ?? null` (`gift-controller.ts:26-29`).
- Output: returns `GiftRedemption` (`gift-service.ts:816-832`, interface at `:282-300`); no meta/pagination envelope; default 200.
- DB call: `giftRepository.getByToken(input.token)` then `assertRedeemable` and `serializeRedemption` (`gift-service.ts:820-831`).
- Cache: `headers.cacheInvalidate: false` (`gifts-members.js:8-10`); no other invalidation. C++: none.
- Auth: `permissions: false` (`gifts-members.js:20`); only `loadMemberSession` middleware (`members/app.js:161`); member is optional.
- C++ counterparts: none. `include/blog/handlers/membership/gifts/routes.cppm:19-42` installs only `PUT /gifts/flush_deliveries` and `PUT /gifts/flush_reminders` — no `:token/redeem` path, no GET verb. Grep `redeem` over `include/` returns only data-field names (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`), never a route or handler; grep `giftsMembers|gifts_members|gifts-members` over `include/` returns no matches.

#### Fix

MISSING → add four files + two registrations; use the existing gifts-flush handler as the template
(`include/blog/handlers/membership/gifts/gifts_flush_deliveries_put/handler.cppm:10-30`,
`.../workflow/flow/main.cppm:7-10`, `.../workflow/step/execute.cppm:9-21`).

1. **Check 1 — route.** In `include/blog/handlers/membership/gifts/routes.cppm`, `GiftsRoutes::install`,
   extend the existing `Router{router, "/gifts"}` chain (`:19`) with a GET route (the members-app
   `/api/gifts/:token/redeem` maps to C++ `/gifts/:token/redeem`; the router already drops `/api`,
   same as `admin/routes.js:97-98` → `routes.cppm:22,33`). Do **not** attach `bearer_middleware` —
   Ghost uses only `loadMemberSession` (`members/app.js:161`), member is optional:
   ```cpp
   .add_route(
       Route{"/:token/redeem"}
           .get([&handler](interfaces::io::IRequest& req,
                          interfaces::io::IResponse& res,
                          std::function<void()> send) {
               handler.gifts_gifts_redeemable_get().handle(req, res, std::move(send));
           })
           .summary("Ghost getRedeemable (gifts-members)")
   )
   ```
2. **Check 2 — operation.** In `include/blog/handlers/all.cppm`: add
   `import :gifts_gifts_redeemable_get_handler;` near `:58-59`; add getter
   `gifts_gifts_redeemable_get()` returning
   `blog::handlers::gifts::gifts_redeemable_get::GiftsRedeemableGetHandler&` (pattern `:404-405`);
   add member `m_gifts_gifts_redeemable_get{m_ctx}` (pattern `:681-705`).
3. **New handler/step files** (clone flush template, change names):
   - `include/blog/handlers/membership/gifts/gifts_redeemable_get/handler.cppm` — module
     `blog_handlers:gifts_gifts_redeemable_get_handler`, class `GiftsRedeemableGetHandler`; copy
     `handle()` from flush handler but remove the `token(req).empty()` bearer rejection, run
     workflow `"gifts_gifts_redeemable_get"`, read result `"gifts_gifts_redeemable_get_execute"`,
     set `Status::OK`.
   - `.../workflow/flow/main.cppm` — `WorkflowBuilder{}.name("gifts_gifts_redeemable_get").step<ExecuteStep>().build()`.
   - `.../workflow/step/execute.cppm` — `ExecuteStep` with
     `using Input = interfaces::io::IRequest&; using Output = std::string;`,
     `name{"gifts_gifts_redeemable_get_execute"}`, `static blog::BlogContext* s_ctx`.
4. **Checks 3,4,5,7 — execute step body.** Add new DTO
   `include/blog/handlers/membership/gifts/dto/response.cppm` (module `blog_handlers:gifts_dto`):
   `blog::dto::GiftRedemption` with `token, cadence, duration, currency, amount, buyer_name,
   recipient_name, recipient_email, message, expires_at, consumes_at, tier{id,name,description,benefits}`
   plus `serde::Serializable` specialization (copy `gift_links/dto/response.cppm:97-110`). In
   `ExecuteStep::invoke`: parse `:token` from `in.get_path()` and optional member status from a
   request header (no member-session middleware exists in C++):
   ```cpp
   std::vector<std::string_view> seg;
   for (auto part : in.get_path() | std::views::split('/'))
       if (!part.empty()) seg.emplace_back(part.begin(), part.end());   // {"gifts","<token>","redeem"}
   const std::string token = seg.size() > 1 ? std::string{seg[1]} : std::string{};
   if (token.empty()) return error(400, "token required");             // string/required (check 3)
   const auto statusHdr = in.find_header("X-Member-Status");            // Ghost context.member.status ?? null
   const std::string memberStatus = statusHdr ? std::string{*statusHdr} : std::string{};
   ```
   Look up the gift via `ctx.store().json_for("gifts")` and filter records where `token == token`
   — **do not** use `find_by_resource("gifts", token)`: `ApiDataStore::find_by_resource`
   (`context.cppm:1055-1092`) has no `"gifts"` branch and returns `std::nullopt`, while
   `json_for("gifts")` falls through to `m_resources.list("gifts")` (`context.cppm:1180`).
   If absent return 404 `{"errors":[{"code":"GIFT_NOT_FOUND",...}]}`; else run the inline
   `assertRedeemable(gift, memberStatus)` rules (`gift-service.ts:282-300`). Read tier via
   `ctx.store().find_by_resource("tiers", tierId)` (`context.cppm:1086-1090`), fill the DTO, return
   `blog::handlers::middleware::encode(dto)` (`middleware/serde.cppm:155`) as a single object body
   — no `{data,meta}` wrapper, `Status::OK` (check 4).
5. **Check 6 — cache.** `headers.cacheInvalidate: false` (`gifts-members.js:8-10`) ⇒ emit no
   invalidation header and perform no `cache.put`/erase in the step. No change beyond confirming
   this; do not invent a cache key.
6. **Check 8 — evidence.** The new route/handler/step above supply the missing C++ side for
   `members/app.js:159-163` + `gifts-members.js:7-24`; re-run the greps in the MISSING row to
   confirm a `/gifts/:token/redeem` GET route and a `gifts_gifts_redeemable_get` handler now exist.

### POST /api/gifts/:token/redeem — MISSING

- Route + op: registered with `bodyParser.json({limit:'50mb'})`, `middleware.loadMemberSession`, `http(api.giftsMembers.redeem)` (`docs/Ghost/ghost/core/server/web/members/app.js:164-169`); controller key `redeem`, `statusCode: 200` (`docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js:26-44`).
- Input: `data:['token']`, `token` validated string/required (`gifts-members.js:31-39`); token from URL param `:token` (`members/app.js:165`); `member.id` from context required or `UnauthorizedError('Member authentication required.')` (`gift-controller.ts:32-46`).
- Output: returns `GiftRedemption` from `serializeRedemption` (`gift-service.ts:834-891`, `:282-300`); `statusCode: 200`; no `{data,meta}` wrapper.
- DB call: `giftRepository.transaction(...)` → `giftRepository.getByToken(forUpdate)`, `memberRepository.get`/`update`, `giftRepository.update`, `giftDeliveryService.cancelPendingForGift`, `tiersService.api.read`, staff email, reminder scheduler (`gift-service.ts:840-948`).
- Cache: `headers.cacheInvalidate: false` (`gifts-members.js:28-30`). C++: none.
- Auth: `permissions: false` (`gifts-members.js:40`); `loadMemberSession` middleware (`members/app.js:167`) plus explicit member-id check in service (`gift-controller.ts:36-40`).
- C++ counterparts: none. Same absence evidence as GET row: `include/blog/handlers/membership/gifts/routes.cppm:19-42` has only the two admin flush PUT routes; no POST `redeem` route and no `giftsMembers` handler. No `redeem`-named route/handler found in `include/` (grep hits only schema/dto fields listed above).

#### Fix

MISSING → add one more route + handler/step trio, reusing the DTO from the GET fix.

1. **Check 1 — route.** In `include/blog/handlers/membership/gifts/routes.cppm`,
   `GiftsRoutes::install`, append to the same `Router{router, "/gifts"}` chain a POST route:
   ```cpp
   .add_route(
       Route{"/:token/redeem"}
           .post([&handler](interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send) {
               handler.gifts_gifts_redeem_post().handle(req, res, std::move(send));
           })
           .summary("Ghost redeem (gifts-members)")
   )
   ```
   Ghost mounts `bodyParser.json({limit:'50mb'})` on this route (`members/app.js:166`); the
   domain router's existing `content_type_middleware` (`routes.cppm:20`) is the C++ equivalent.
   No admin bearer gate — `permissions: false` (`gifts-members.js:40`).
2. **Check 2 — operation.** In `include/blog/handlers/all.cppm`: add
   `import :gifts_gifts_redeem_post_handler;`, getter `gifts_gifts_redeem_post()` returning
   `...::gifts_redeem_post::GiftsRedeemPostHandler&`, and member `m_gifts_gifts_redeem_post{m_ctx}`
   (same lines/patterns as the GET fix).
3. **New files** mirroring the flush template:
   `include/blog/handlers/membership/gifts/gifts_redeem_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`
   — modules `blog_handlers:gifts_gifts_redeem_post_handler`, `..._flow`, `..._step`; workflow
   name `"gifts_gifts_redeem_post"`, result `"gifts_gifts_redeem_post_execute"`.
4. **Checks 3,4,5,7 — execute step.** Extract token exactly as in the GET fix. Read member id
   from a request header (`X-Member-Id` — Ghost `frame.options.context.member.id`,
   `gift-controller.ts:34`); if empty return `Status::UNAUTHORIZED` with
   `{"errors":[{"message":"Member authentication required."}]}` — do **not** use the admin
   `blog::handlers::middleware::token()` check; this is member auth (`gift-controller.ts:36-40`).
   Body has no validated fields beyond token; `in.body_as_string()` (empty ⇒ `{}`). Reproduce the
   Ghost transaction (`gift-service.ts:840-948`) as ordered `ctx.store()` mutations —
   `ApiDataStore` has no transaction primitive, so this is the allowed equivalent; all state stays
   in `ResourceStore`/`Cache` (`resources`/`members` tags), no new global state:
   ```cpp
   auto& store = ctx.store();
   // 1. gift = filter(json_for("gifts"), token == token)  (find_by_resource has no "gifts" branch)
   // 2. member = store.find_by_resource("members", memberId)   (context.cppm:1072-1076)
   // 3. store.edit("members", memberId, R"({"status":"gift","products":[{"tier_id":"...","expiry_at":"..."}]})");
   // 4. store.edit("gifts", giftId, R"({"status":"redeemed","redeemer_member_id":"...","redeemed_at":"..."})");
   // 5. store.edit("gift_deliveries", deliveryId, R"({"status":"cancelled"})"); // cancelPendingForGift
   // 6. tier = store.find_by_resource("tiers", tierId);                          // tiersService.api.read
   ```
   Return `blog::handlers::middleware::encode(GiftRedemption{...})` as a single object with
   `Status::OK` (200), no `{data,meta}` wrapper (`gifts-members.js:27`, `gift-service.ts:834-891`).
   Staff email notify and `giftReminderScheduler.scheduleAt` have no transport in the allowed
   modules; if an observable stub is required, write cache entries under `emails:*` /
   `automations:*` tags (never new global state), otherwise omit and document.
5. **Check 6 — cache.** `headers.cacheInvalidate: false` (`gifts-members.js:28-30`) ⇒ no
   invalidation header / no eviction; no change.
6. **Check 8 — evidence.** New route + handler/step cover `members/app.js:164-169` +
   `gifts-members.js:26-44`; re-run the absence greps to confirm a POST `/gifts/:token/redeem`
   route and `gifts_gifts_redeem_post` handler now exist.

## Missing in C++

Both routes in this controller have no C++ counterpart:

- GET `/api/gifts/:token/redeem` → `api.giftsMembers.getRedeemable` (`docs/Ghost/ghost/core/server/web/members/app.js:159-163`).
  Fix: Register `.get(Route{"/:token/redeem"}...)` in `GiftsRoutes::install`
  (`include/blog/handlers/membership/gifts/routes.cppm:19`, no bearer middleware), add
  `gifts_gifts_redeemable_get()` getter + `m_gifts_gifts_redeemable_get{m_ctx}` member in
  `include/blog/handlers/all.cppm`, and add
  `gifts/gifts_redeemable_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`
  plus `gifts/dto/response.cppm` (`GiftRedemption` DTO). Step parses `:token` from `in.get_path()`,
  reads optional member status from a header, looks up via `ctx.store().json_for("gifts")` filtered
  by token (not `find_by_resource` — no `"gifts"` branch, `context.cppm:1055-1092`), returns the
  encoded DTO with `Status::OK` and no `{data,meta}` envelope. Closes checks 1-8; see GET detail fix.
- POST `/api/gifts/:token/redeem` → `api.giftsMembers.redeem` (`docs/Ghost/ghost/core/server/web/members/app.js:164-169`).
  Fix: Register `.post(Route{"/:token/redeem"}...)` in `GiftsRoutes::install` (existing
  `content_type_middleware` covers the JSON body), add `gifts_gifts_redeem_post()` getter +
  `m_gifts_gifts_redeem_post{m_ctx}` member in `include/blog/handlers/all.cppm`, and add
  `gifts/gifts_redeem_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`.
  Step parses `:token`, requires a member id header else `Status::UNAUTHORIZED`, then reproduces the
  Ghost transaction as ordered `ctx.store()` mutations (`members`/`gifts`/`gift_deliveries`/`tiers`)
  and returns the encoded `GiftRedemption` DTO with `Status::OK`, no envelope, no cache invalidation.
  Closes checks 1-8; see POST detail fix.

Absence confirmed by: (1) reading `include/blog/handlers/membership/gifts/routes.cppm:19-42`, the only gifts route installer, which registers `PUT /gifts/flush_deliveries` and `PUT /gifts/flush_reminders` only; (2) grep `redeem` (case-insensitive) across `include/` → only data-field occurrences at `context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`; (3) grep `giftsMembers|gifts_members|gifts-members` across `include/` → 0 matches; (4) `include/blog/testing/route_catalog.cppm:93-94` lists only the two flush PUT gift routes.

## Extra in C++

None for this controller. The two C++ routes in `include/blog/handlers/membership/gifts/routes.cppm:22-41` (`PUT /gifts/flush_deliveries`, `PUT /gifts/flush_reminders`) are not extras — they map to the Admin API `gifts.js` controller (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:97-98`), not to `gifts-members.js`.

## Needs central registration

Both MISSING routes are now implemented as **self-contained new files** under
`include/blog/handlers/membership/gifts_members/`. No shared file was touched. These
modules are not yet reachable — a central registrar must wire them in. Files created:

| File | Module | Purpose |
|---|---|---|
| `routes.cppm` | `blog_handlers_routes:gifts_members_routes` | `blog::api::GiftsMembersRoutes::install(router, handler)` — installs `GET` and `POST /gifts/:token/redeem` |
| `redeem_get/handler.cppm` | `blog_handlers:gifts_members_redeem_get_handler` | `blog::handlers::gifts_members::redeem_get::RedeemGetHandler` |
| `redeem_get/workflow/flow/main.cppm` | `blog_handlers:gifts_members_redeem_get_flow` | workflow `gifts_members_redeem_get` |
| `redeem_get/workflow/step/execute.cppm` | `blog_handlers:gifts_members_redeem_get_step` | `Output=std::map<std::string,std::string>`, static `s_ctx` |
| `redeem_post/handler.cppm` | `blog_handlers:gifts_members_redeem_post_handler` | `blog::handlers::gifts_members::redeem_post::RedeemPostHandler` |
| `redeem_post/workflow/flow/main.cppm` | `blog_handlers:gifts_members_redeem_post_flow` | workflow `gifts_members_redeem_post` |
| `redeem_post/workflow/step/execute.cppm` | `blog_handlers:gifts_members_redeem_post_step` | `Output=std::map<std::string,std::string>`, static `s_ctx` |

Implementation notes: route prefix `/gifts` matches the admin gifts router (router
drops `/api`). No bearer/`require_auth` gate — Ghost uses only `loadMemberSession`,
`permissions:false`; member status/id come from `X-Member-Status` / `X-Member-Id`
headers (no C++ member-session middleware exists). Steps tokenize `:token` from the
last-two path segments, look gifts up via `json_for("gifts")` filtered by token (not
`find_by_resource`, which has no `"gifts"` branch), inline `assertRedeemable`, and
return a bare `GiftRedemption` object with `Status::OK` / no `{data,meta}` envelope.
POST reproduces the transaction as ordered `ctx.store()` mutations over
`members`/`gifts`/`gift_deliveries`/`tiers`; staff email and reminder scheduling are
omitted (no transport in allowed modules). `headers.cacheInvalidate:false` ⇒ no
eviction; only the `X-Cache-Invalidate: false` marker is set, matching existing handlers.

Central registrations required (all in shared files, intentionally NOT done here):

1. `include/blog/handlers/BUILD.bazel`:
   - Add the six handler/step/flow partitions to the `blog_handlers` target's
     `partitions` list, near the existing `membership/gifts/...` entries:
     `membership/gifts_members/redeem_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and the `redeem_post/` equivalents.
   - Add `membership/gifts_members/routes.cppm` to the `blog_handlers_routes`
     target's `partitions` list (near `membership/gifts/routes.cppm`, BUILD.bazel:952).
2. `include/blog/handlers/base.cppm`: add `export import :gifts_members_redeem_get_handler;`
   and `export import :gifts_members_redeem_post_handler;` (near the existing gift
   handler imports at `:77-78`).
3. `include/blog/handlers/all.cppm`: add `import :gifts_members_redeem_get_handler;`
   and `import :gifts_members_redeem_post_handler;`; add getters
   `gifts_members_redeem_get()` → `blog::handlers::gifts_members::redeem_get::RedeemGetHandler&`
   and `gifts_members_redeem_post()` →
   `blog::handlers::gifts_members::redeem_post::RedeemPostHandler&` (pattern `:404-405`);
   add members `m_gifts_members_redeem_get{m_ctx}` and
   `m_gifts_members_redeem_post{m_ctx}` (pattern `:681-705`). These getters are what
   `routes.cppm` calls — without them it will not compile.
4. `include/blog/handlers/routes.cppm`: add `export import :gifts_members_routes;`
   (near `:20`) and call `GiftsMembersRoutes{}.install(router, handlers);` in
   `ApiRoutes::install` (near `:92`). Until this call exists the two routes are
   not registered at runtime.
5. Optionally add a `content_type_middleware` `.use(...)` on the POST route in
   `gifts_members/routes.cppm` if JSON body validation equivalent to Ghost's
   `bodyParser.json({limit:'50mb'})` is required at the router layer; the POST step
   already calls `in.body_as_string()` and has no validated body fields.
