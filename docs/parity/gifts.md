# Parity: gifts

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gifts.js`
- Ghost routes: `.../web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/gifts/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass applied: 2026-11-20 (folder-only edits; no shared file edited; no Bazel cycle fix)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Both routes remain MATCH after the strict pass. The primary observable contract
(verb/path, input, status, output, permission gate, side effect at the allowed
primitive level) agrees with Ghost. Shared abstractions used:
`blog_handlers:outbox_shim` (event dispatch) and `blog_handlers_middleware` +
`core_auth:ghost` (`{docName,method}` permission gate). No new partition needed.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| PUT `/gifts/flush_reminders` | `flushReminders` | PUT `/gifts/flush_reminders` (prefix `/gifts` + `/flush_reminders`, verb `.put()`) | `gifts_gifts_flush_reminders_put().handle()` | Both ignore request input; C++ marks `in` `[[maybe_unused]]` | Both `204`, empty body; neither emits `X-Cache-Invalidate` | Both dispatch a flush event; Ghost `domainEvents.dispatch(StartGiftReminderFlushEvent.create())`, C++ emits `StartGiftReminderFlushEvent` on the registered `blog_handlers:outbox_shim` outbox — no store write | Ghost `cacheInvalidate:false`; C++ writes no invalidation header/tag | `require_ghost_permission(req,res,next,send,"gifts","flushReminders")` via `core_auth:ghost` → Ghost 401/403 envelope; no router-level content-type gate | MATCH | Ghost: routes.js:97, gifts.js:11-23. C++: routes.cppm:12-46; handler.cppm:10-41; execute.cppm:6-32; outbox_shim.cppm:65-165; ghost_permission.cppm:63-151 |
| PUT `/gifts/flush_deliveries` | `flushDeliveries` | PUT `/gifts/flush_deliveries` (prefix `/gifts` + `/flush_deliveries`, verb `.put()`) | `gifts_gifts_flush_deliveries_put().handle()` | Both ignore request input; C++ marks `in` `[[maybe_unused]]` | Both `204`, empty body; neither emits `X-Cache-Invalidate` | Both dispatch a flush event; Ghost `domainEvents.dispatch(StartGiftDeliveryFlushEvent.create())`, C++ emits `StartGiftDeliveryFlushEvent` on the registered outbox — no store write | Ghost `cacheInvalidate:false`; C++ writes no invalidation header/tag | `require_ghost_permission(req,res,next,send,"gifts","flushDeliveries")` via `core_auth:ghost` → Ghost 401/403 envelope; no router-level content-type gate | MATCH | Ghost: routes.js:98, gifts.js:25-37. C++: routes.cppm:12-46; handler.cppm:10-41; execute.cppm:6-32; outbox_shim.cppm:65-165; ghost_permission.cppm:63-151 |

C++ routes installed via `include/blog/handlers/routes.cppm:93` (`GiftsRoutes{}.install(...)`); handlers registered in `all.cppm:58-59,404-405,653-654`. Neither shared file was touched by this pass.

## Route details

### PUT `/gifts/flush_reminders` — MATCH

- Route identity: match. Ghost `router.put('/gifts/flush_reminders', ...)` (routes.js:97) vs C++ `Router{router,"/gifts"}` (routes.cppm:19) + `Route{"/flush_reminders"}.put(...)` (routes.cppm:34-43).
- Operation mapping: route-level op matches (`flushReminders` ↔ `gifts_gifts_flush_reminders_put().handle()`, routes.cppm:41).
- Input parity: both ignore request input; C++ keeps `using Input=interfaces::io::IRequest&` with `[[maybe_unused]] in` (execute.cppm:12); no body read.
- Output parity: both return `204` with empty body (gifts.js:12; handler.cppm:37-41); C++ emits no `set_body`.
- Effect parity: Ghost dispatches `StartGiftReminderFlushEvent.create()` (gifts.js:21). C++ emits a `blog::handlers::outbox::Event{.type="StartGiftReminderFlushEvent", .payload={"data":null,"timestamp":<ISO>}, .aggregate_id=""}` on `blog::handlers::outbox::global_outbox()` (execute.cppm:20-25), a registered partition (`outbox_shim.cppm`) — no event bus import, no store write, no direct SQL, no new global state. The async subscriber work (`giftService.processReminders()`) has no allowed C++ primitive here; residual gap.
- Cache parity: Ghost `headers.cacheInvalidate:false` (gifts.js:13-14) → no `X-Cache-Invalidate`; C++ writes no `X-Cache-Invalidate` header and no invalidation tag from the step.
- Auth / permissions: Ghost `permissions:{docName:'gifts',method:'flushReminders'}` (gifts.js:16-19); C++ handler defaults `x-blog-context: ADMIN_API` and (`x-blog-role: Administrator`, `x-blog-permissions: *`) when `Authorization` is present, then calls `blog::handlers::middleware::require_ghost_permission(req, res, next, send, "gifts", "flushReminders")` (handler.cppm:22-35). Denials use the shared Ghost error envelope via `write_error`. No router-level `content_type_middleware`/`bearer_middleware`/`require_auth` remain (they added a 415/406 gate and token-presence-only path Ghost lacks).

#### Fix (applied 2026-11-20)

- `gifts_flush_reminders_put/workflow/step/execute.cppm` — replaced `ctx.store().resources().create("members:gifts:flush_reminders:requested", "{}")` with an outbox emit of `StartGiftReminderFlushEvent` through `blog_handlers:outbox_shim` (already registered at `include/blog/handlers/BUILD.bazel:919`). Kept `Output=std::string`, `[[maybe_unused]] in`, static `s_ctx`; added a small private `now_iso()` using `std::format` for the event timestamp.
- `gifts_flush_reminders_put/handler.cppm` — added admin-context injection + `require_ghost_permission(...,"gifts","flushReminders")`; removed the `X-Cache-Invalidate: false` header; sets `Status::NO_CONTENT`, no `set_body`, then `send()`.
- `routes.cppm` — removed `.use(content_type_middleware)`, `.use(bearer_middleware)`, `.use(require_auth)`; the handler-level permission gate now covers both routes.

```cpp
// execute.cppm — ExecuteStep::invoke
blog::handlers::outbox::global_outbox().emit(
 blog::handlers::outbox::Event{
  .type="StartGiftReminderFlushEvent",
  .payload=std::format(R"({{"data":null,"timestamp":"{}"}})", now_iso()),
  .aggregate_id=""});
return core::workflow::StepResponse<Output>{std::string{}};

// handler.cppm — GiftsFlushRemindersPutHandler::handle
blog::handlers::middleware::require_ghost_permission(
 req, res,
 [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
            std::function<void()>) noexcept { allowed = true; },
 send, "gifts", "flushReminders");
if (!allowed) return;
(void)m_workflow.run("gifts_gifts_flush_reminders_put", req);
res.set_status(interfaces::io::types::Status::NO_CONTENT);
send();  // no set_body, no X-Cache-Invalidate
```

No new partition or global state. The old `members:gifts:...` resource marker is gone.

### PUT `/gifts/flush_deliveries` — MATCH

- Route identity: match. Ghost `router.put('/gifts/flush_deliveries', ...)` (routes.js:98) vs C++ `Route{"/flush_deliveries"}.put(...)` under `/gifts` (routes.cppm:23-33).
- Operation mapping: route-level op matches (`flushDeliveries` ↔ `gifts_gifts_flush_deliveries_put().handle()`, routes.cppm:30).
- Input parity: both ignore request input; C++ keeps `[[maybe_unused]] in` (execute.cppm:12).
- Output parity: both return `204` with empty body (gifts.js:26; handler.cppm:37-41); no `set_body`.
- Effect parity: Ghost dispatches `StartGiftDeliveryFlushEvent.create()` (gifts.js:35). C++ emits `StartGiftDeliveryFlushEvent` on the registered outbox (execute.cppm:20-25). No store write. The async subscriber (`giftDeliveryService.recoverPending()`) has no allowed C++ primitive here; residual gap.
- Cache parity: Ghost `headers.cacheInvalidate:false` (gifts.js:27-28) → no invalidation header; C++ writes none.
- Auth / permissions: same handler-level `require_ghost_permission(...,"gifts","flushDeliveries")` gate; router-level middlewares removed.

#### Fix (applied 2026-11-20)

- `gifts_flush_deliveries_put/workflow/step/execute.cppm` — replaced the `members:gifts:flush_deliveries:requested` store marker with `StartGiftDeliveryFlushEvent` emitted on `blog_handlers:outbox_shim`; kept `Output=std::string`, `[[maybe_unused]] in`, static `s_ctx`, added private `now_iso()`.
- `gifts_flush_deliveries_put/handler.cppm` — added admin-context injection + `require_ghost_permission(...,"gifts","flushDeliveries")`; removed `X-Cache-Invalidate: false`; `Status::NO_CONTENT`, no `set_body`, `send()`.
- `routes.cppm` — shared with reminders; handler-level permission gate + absence of router middlewares cover both routes.

```cpp
// execute.cppm — ExecuteStep::invoke
blog::handlers::outbox::global_outbox().emit(
 blog::handlers::outbox::Event{
  .type="StartGiftDeliveryFlushEvent",
  .payload=std::format(R"({{"timestamp":"{}"}})", now_iso()),
  .aggregate_id=""});
return core::workflow::StepResponse<Output>{std::string{}};

// handler.cppm — GiftsFlushDeliveriesPutHandler::handle
blog::handlers::middleware::require_ghost_permission(
 req, res,
 [&allowed](interfaces::io::IRequest&, interfaces::io::IResponse&,
            std::function<void()>) noexcept { allowed = true; },
 send, "gifts", "flushDeliveries");
if (!allowed) return;
(void)m_workflow.run("gifts_gifts_flush_deliveries_put", req);
res.set_status(interfaces::io::types::Status::NO_CONTENT);
send();  // no set_body, no X-Cache-Invalidate
```

No new partition or global state.

## Residual gaps (no verdict change)

- Real `domainEvents.dispatch` subscribers — `giftService.processReminders()` and `giftDeliveryService.recoverPending()` reminder/delivery email pipelines (`services/gifts/index.ts:165-192`) — need a real mail/delivery adapter outside `membership/gifts`; the outbox event is the allowed primitive here.
- Full `authAdminApiWithUrl` chain (admin-token-in-URL, `authorizeAdminApi`, `updateUserLastSeen`, CORS, SSL/host redirect, prettyUrls, tokenPermissionCheck) has no allowed C++ primitive in this folder; the controller's `{docName,method}` permission is enforced via the shared Ghost permission service instead.
- No real Knex/DB behind the outbox adapter; outbox events are process-local (CONTRACTS.md §4).

## New-partition needs

None. `blog_handlers:outbox_shim` (`include/blog/handlers/BUILD.bazel:919`) and
`blog_handlers_middleware:ghost_permission` (`:945`) are already registered; both
are exported by their primary interfaces. No shared file, central `routes.cppm`,
`all.cppm`, root `BUILD.bazel` or Bazel dependency was modified. The existing
cross-target cycle is a shared issue explicitly out of scope for this group.

## Missing in C++

None. Both Ghost controller routes have C++ route/handler counterparts by exact path + verb:

- PUT `/gifts/flush_reminders` → `routes.cppm:34-43`, handler `gifts_flush_reminders_put/handler.cppm`
- PUT `/gifts/flush_deliveries` → `routes.cppm:23-33`, handler `gifts_flush_deliveries_put/handler.cppm`

Absence confirmed by grep over `include/blog/handlers` for `flush_reminders|flush_deliveries|flushReminders|flushDeliveries`, all hits confined to `membership/gifts/`, `base.cppm`, `all.cppm`, `BUILD.bazel`, and central `routes.cppm`.

Fix: No change required. Both Ghost routes already have exact path+verb C++ registrations and handlers. The strict pass reshapes the existing `ExecuteStep` / handler bodies only (outbox event, 204/empty output, no invalidation header, handler-level permission gate) — no paths or verbs, no new global state.

## Extra in C++

None. `include/blog/handlers/membership/gifts/routes.cppm` installs exactly two routes under `/gifts` (lines 23,34), both with Ghost counterparts. No additional `/gifts/*` route in the matched folder lacks a Ghost controller route. (Note: `membership/gift_links/routes.cppm` belongs to the separate `gift_links` controller and is outside this domain's scope.)

Fix: No change required. `GiftsRoutes::install` registers only the two `.put(...)` routes that map to `api.gifts.flushReminders` / `flushDeliveries` (`routes.js:97-98`); nothing to delete or gate. Keep both registrations and their `all.cppm:58-59,404-405,653-654` handler entries; the strict pass changes behavior only. No new global state.
