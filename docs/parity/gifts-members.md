# Parity: gifts-members

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/members/app.js` (members app; no separate members `routes.js` — routes registered inline)
- C++: none found (no redeem route/handler anywhere under `include/blog/handlers`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 2 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/api/gifts/:token/redeem` | `getRedeemable` | none | none | `token` (route param `:token`, also `data:['token']`, validated string/required); member status read from `options.context.member.status` (nullable) | single `GiftRedemption` object (`token`, `cadence`, `duration`, `currency`, `amount`, `buyer_name`, `recipient_name`, `recipient_email`, `message`, `expires_at`, `consumes_at`, `tier{id,name,description,benefits}`) via `serializeRedemption`; no `{data,meta}` wrapper; default 200 | `giftRepository.getByToken(token)`; throws `NotFoundError` (`GIFT_NOT_FOUND`) if absent; `assertRedeemable(gift, memberStatus)` | none (`headers.cacheInvalidate: false`) | `permissions: false`; `middleware.loadMemberSession`; member session optional (status only) | MISSING | Ghost: `members/app.js:159-163`, `gifts-members.js:7-24`, `gift-controller.ts:25-30`, `gift-service.ts:816-832`, `gift-service.ts:282-300`. C++: no route — grep `redeem` over `include/` hits only schema fields (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`); grep `giftsMembers\|gifts_members\|gifts-members` over `include/` = 0 matches |
| POST `/api/gifts/:token/redeem` | `redeem` | none | none | `token` (route param `:token`, also `data:['token']`, validated string/required); `member.id` required from context else `UnauthorizedError`; `bodyParser.json({limit:'50mb'})` mounted on route | single `GiftRedemption` object via `serializeRedemption`; `statusCode: 200`; no `{data,meta}` wrapper | transactional: `giftRepository.getByToken(forUpdate)`, `memberRepository.get`/`update` (status `gift`, product with `expiry_at`), `giftRepository.update`, `giftDeliveryService.cancelPendingForGift`, `tiersService.api.read`, staff email notify, `giftReminderScheduler.scheduleAt` | none (`headers.cacheInvalidate: false`) | `permissions: false`; `middleware.loadMemberSession`; member auth enforced in service (`!member.id` → `UnauthorizedError`) | MISSING | Ghost: `members/app.js:164-169`, `gifts-members.js:26-44`, `gift-controller.ts:32-46`, `gift-service.ts:834-891`, `gift-service.ts:893-935`. C++: no route — grep `redeem` over `include/` hits only schema fields (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`); grep `giftsMembers\|gifts_members\|gifts-members` over `include/` = 0 matches |

## Route details

### GET /api/gifts/:token/redeem — MISSING

- Route + op: registered in members app with `middleware.loadMemberSession` then `http(api.giftsMembers.getRedeemable)` (`docs/Ghost/ghost/core/server/web/members/app.js:159-163`); controller key `getRedeemable` with `docName: 'gifts'` (`docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js:5,7-24`).
- Input: `data:['token']`, `token` validated `{type:'string', required:true}` (`gifts-members.js:11-19`); token supplied as URL param `:token` (`members/app.js:160`). Member status read from `frame.options?.context?.member?.status ?? null` (`gift-controller.ts:26-29`).
- Output: returns `GiftRedemption` (`gift-service.ts:816-832`, interface at `:282-300`); no meta/pagination envelope; default 200.
- DB call: `giftRepository.getByToken(input.token)` then `assertRedeemable` and `serializeRedemption` (`gift-service.ts:820-831`).
- Cache: `headers.cacheInvalidate: false` (`gifts-members.js:8-10`); no other invalidation. C++: none.
- Auth: `permissions: false` (`gifts-members.js:20`); only `loadMemberSession` middleware (`members/app.js:161`); member is optional.
- C++ counterparts: none. `include/blog/handlers/membership/gifts/routes.cppm:19-42` installs only `PUT /gifts/flush_deliveries` and `PUT /gifts/flush_reminders` — no `:token/redeem` path, no GET verb. Grep `redeem` over `include/` returns only data-field names (`context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`), never a route or handler; grep `giftsMembers|gifts_members|gifts-members` over `include/` returns no matches.

### POST /api/gifts/:token/redeem — MISSING

- Route + op: registered with `bodyParser.json({limit:'50mb'})`, `middleware.loadMemberSession`, `http(api.giftsMembers.redeem)` (`docs/Ghost/ghost/core/server/web/members/app.js:164-169`); controller key `redeem`, `statusCode: 200` (`docs/Ghost/ghost/core/server/api/endpoints/gifts-members.js:26-44`).
- Input: `data:['token']`, `token` validated string/required (`gifts-members.js:31-39`); token from URL param `:token` (`members/app.js:165`); `member.id` from context required or `UnauthorizedError('Member authentication required.')` (`gift-controller.ts:32-46`).
- Output: returns `GiftRedemption` from `serializeRedemption` (`gift-service.ts:834-891`, `:282-300`); `statusCode: 200`; no `{data,meta}` wrapper.
- DB call: `giftRepository.transaction(...)` → `giftRepository.getByToken(forUpdate)`, `memberRepository.get`/`update`, `giftRepository.update`, `giftDeliveryService.cancelPendingForGift`, `tiersService.api.read`, staff email, reminder scheduler (`gift-service.ts:840-948`).
- Cache: `headers.cacheInvalidate: false` (`gifts-members.js:28-30`). C++: none.
- Auth: `permissions: false` (`gifts-members.js:40`); `loadMemberSession` middleware (`members/app.js:167`) plus explicit member-id check in service (`gift-controller.ts:36-40`).
- C++ counterparts: none. Same absence evidence as GET row: `include/blog/handlers/membership/gifts/routes.cppm:19-42` has only the two admin flush PUT routes; no POST `redeem` route and no `giftsMembers` handler. No `redeem`-named route/handler found in `include/` (grep hits only schema/dto fields listed above).

## Missing in C++

Both routes in this controller have no C++ counterpart:

- GET `/api/gifts/:token/redeem` → `api.giftsMembers.getRedeemable` (`docs/Ghost/ghost/core/server/web/members/app.js:159-163`).
- POST `/api/gifts/:token/redeem` → `api.giftsMembers.redeem` (`docs/Ghost/ghost/core/server/web/members/app.js:164-169`).

Absence confirmed by: (1) reading `include/blog/handlers/membership/gifts/routes.cppm:19-42`, the only gifts route installer, which registers `PUT /gifts/flush_deliveries` and `PUT /gifts/flush_reminders` only; (2) grep `redeem` (case-insensitive) across `include/` → only data-field occurrences at `context.cppm:149`, `ghost_schema.cppm:342`, `fixtures.cppm:529`; (3) grep `giftsMembers|gifts_members|gifts-members` across `include/` → 0 matches; (4) `include/blog/testing/route_catalog.cppm:93-94` lists only the two flush PUT gift routes.

## Extra in C++

None for this controller. The two C++ routes in `include/blog/handlers/membership/gifts/routes.cppm:22-41` (`PUT /gifts/flush_deliveries`, `PUT /gifts/flush_reminders`) are not extras — they map to the Admin API `gifts.js` controller (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:97-98`), not to `gifts-members.js`.
