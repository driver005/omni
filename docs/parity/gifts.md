# Parity: gifts

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gifts.js`
- Ghost routes: `.../web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/gifts/routes.cppm`
- Checked: 2026-11-19

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
| PUT `/gifts/flush_reminders` | `flushReminders` | PUT `/gifts/flush_reminders` (prefix `/gifts` + `/flush_reminders`, verb `.put()`) | `gifts_gifts_flush_reminders_put().handle()` | Ghost: no options/data/validation; `query()` ignores input. C++: `Input=IRequest&`, reads raw `body_as_string()` (empty→`"{}"`), no named params | Ghost: `statusCode 204`, no body, `cacheInvalidate:false`. C++: status `200 OK`, body = store.edit result or `"{}"` | Ghost: `domainEvents.dispatch(StartGiftReminderFlushEvent.create())`, no model call. C++: `ctx.store().edit("gifts","",body)` | Ghost: `headers.cacheInvalidate:false` (invalidation disabled). C++: none explicit; `ResourceStore.edit` uses tag `"resources"`, TTL never (internal, likely no-op for empty id) | Ghost: `mw.authAdminApiWithUrl` + permissions docName `gifts`/method `flushReminders`. C++: `content_type_middleware` + bearer-token-present check only; no docName/method permission | MISMATCH | Ghost: routes.js:97, gifts.js:11-23. C++: routes.cppm:19-20,33-42; handler.cppm:17-28; execute.cppm:11,16; context.cppm:653-663 |
| PUT `/gifts/flush_deliveries` | `flushDeliveries` | PUT `/gifts/flush_deliveries` (prefix `/gifts` + `/flush_deliveries`, verb `.put()`) | `gifts_gifts_flush_deliveries_put().handle()` | Ghost: no options/data/validation; `query()` ignores input. C++: `Input=IRequest&`, reads raw `body_as_string()` (empty→`"{}"`), no named params | Ghost: `statusCode 204`, no body, `cacheInvalidate:false`. C++: status `200 OK`, body = store.edit result or `"{}"` | Ghost: `domainEvents.dispatch(StartGiftDeliveryFlushEvent.create())`, no model call. C++: `ctx.store().edit("gifts","",body)` | Ghost: `headers.cacheInvalidate:false` (invalidation disabled). C++: none explicit; `ResourceStore.edit` uses tag `"resources"`, TTL never (internal, likely no-op for empty id) | Ghost: `mw.authAdminApiWithUrl` + permissions docName `gifts`/method `flushDeliveries`. C++: `content_type_middleware` + bearer-token-present check only; no docName/method permission | MISMATCH | Ghost: routes.js:98, gifts.js:25-37. C++: routes.cppm:19-20,22-31; handler.cppm:17-28; execute.cppm:11,16; context.cppm:653-663 |

C++ routes installed via `include/blog/handlers/routes.cppm:93` (`GiftsRoutes{}.install(...)`); handlers registered in `all.cppm:58-59,404-405,653-654`.

## Route details

### PUT `/gifts/flush_reminders` — MISMATCH

- Route identity: match. Ghost `router.put('/gifts/flush_reminders', ...)` (routes.js:97) vs C++ `Router{router,"/gifts"}` (routes.cppm:19) + `Route{"/flush_reminders"}.put(...)` (routes.cppm:33-42). Same normalized path + verb.
- Operation mapping: route-level op matches (`flushReminders` ↔ `gifts_gifts_flush_reminders_put().handle()`, routes.cppm:39). Implementation differs materially (see DB/output below).
- Input parity: Ghost controller defines no `options`, `data`, or `validation` and `query()` takes no arguments (gifts.js:20-22). C++ step declares `using Input=interfaces::io::IRequest&` and reads only `in.body_as_string()`, substituting `"{}"` when empty (execute.cppm:11,16); no query-string or named-param parsing. No named params on either side, but C++ passes the raw body into a store edit while Ghost ignores request input.
- Output parity: Ghost returns `statusCode: 204` with no body and `headers.cacheInvalidate:false` (gifts.js:12-14). C++ handler always `set_status(Status::OK)` = 200 and writes the step's typed_output string (or `"{}"`) as body (handler.cppm:23-27). Divergence: 204 no-content vs 200 with JSON body.
- DB call parity: Ghost dispatches `StartGiftReminderFlushEvent.create()` through `domainEvents.dispatch` (gifts.js:1,20-22) — no model/store call. C++ does `ctx.store().edit("gifts", std::string{}, body)` (execute.cppm:14-16): resource string `"gifts"`, empty id, body as the whole payload. No domain event is emitted. `ResourceStore::edit` (context.cppm:653-663) only updates an existing cache record on `record_key("gifts","")` hit and returns false otherwise, so this is a generic cache edit, not a reminder flush. Materially different operation/resource.
- Cache parity: Ghost explicitly disables invalidation via `headers.cacheInvalidate:false` (gifts.js:13-14). C++ has no handler-level cache/invalidation behavior; the underlying `ResourceStore.edit` writes tag `"resources"` with `Ttl{}.never()` (context.cppm:660-661). No `members:*`/`emails:*`/`automations:*` namespace involved.
- Auth / permissions: Ghost uses `mw.authAdminApiWithUrl` (routes.js:97) plus controller `permissions: {docName:'gifts', method:'flushReminders'}` (gifts.js:16-19). C++ applies `content_type_middleware` (routes.cppm:20) and an in-handler check that only the bearer token is non-empty, returning 401 `{"error":"Unauthorized"}` when absent (handler.cppm:18-22). No per-method/document permission check — weaker and structurally different auth.

### PUT `/gifts/flush_deliveries` — MISMATCH

- Route identity: match. Ghost `router.put('/gifts/flush_deliveries', ...)` (routes.js:98) vs C++ `Route{"/flush_deliveries"}.put(...)` under `/gifts` (routes.cppm:19,22-31). Same normalized path + verb.
- Operation mapping: route-level op matches (`flushDeliveries` ↔ `gifts_gifts_flush_deliveries_put().handle()`, routes.cppm:28). Implementation differs materially (see DB/output below).
- Input parity: Ghost defines no `options`/`data`/`validation` and `query()` ignores input (gifts.js:34-36). C++ `Input=IRequest&`, reads raw `body_as_string()` (empty→`"{}"`), no named params (execute.cppm:11,16).
- Output parity: Ghost `statusCode: 204`, no body, `cacheInvalidate:false` (gifts.js:26-28). C++ `set_status(Status::OK)` = 200 with typed_output string / `"{}"` body (handler.cppm:23-27). Divergence: 204 vs 200-with-body.
- DB call parity: Ghost dispatches `StartGiftDeliveryFlushEvent.create()` (gifts.js:2,4-5,34-36). C++ does `ctx.store().edit("gifts", std::string{}, body)` (execute.cppm:14-16), a generic cache edit on resource `"gifts"` with empty id, no event dispatch (context.cppm:653-663). Materially different operation/resource.
- Cache parity: Ghost `headers.cacheInvalidate:false` (gifts.js:27-28). C++ no explicit cache behavior; `ResourceStore.edit` internal tag `"resources"`, TTL never (context.cppm:660-661). No gift-specific namespace.
- Auth / permissions: Ghost `mw.authAdminApiWithUrl` (routes.js:98) + `permissions: {docName:'gifts', method:'flushDeliveries'}` (gifts.js:30-33). C++ `content_type_middleware` + bearer-token-present check, 401 on empty token (routes.cppm:20; handler.cppm:18-22). No docName/method permission check.

## Missing in C++

None. Both Ghost controller routes have C++ route/handler counterparts by exact path + verb:

- PUT `/gifts/flush_reminders` → `routes.cppm:33-42`, handler `gifts_flush_reminders_put/handler.cppm`
- PUT `/gifts/flush_deliveries` → `routes.cppm:22-31`, handler `gifts_flush_deliveries_put/handler.cppm`

Absence confirmed by grep over `include/blog/handlers` for `flush_reminders|flush_deliveries|flushReminders|flushDeliveries`, all hits confined to `membership/gifts/`, `base.cppm`, `all.cppm`, `BUILD.bazel`, and central `routes.cppm`.

## Extra in C++

None. `include/blog/handlers/membership/gifts/routes.cppm` installs exactly two routes under `/gifts` (lines 22,33), both with Ghost counterparts. No additional `/gifts/*` route in the matched folder lacks a Ghost controller route. (Note: `membership/gift_links/routes.cppm` belongs to the separate `gift_links` controller and is outside this domain's scope.)
