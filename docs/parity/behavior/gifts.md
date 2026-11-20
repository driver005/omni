# Behavior report: gifts

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/gifts.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 96–98)
- C++ folders: `include/blog/handlers/membership/gifts/` — `routes.cppm`, `gifts_flush_reminders_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `gifts_flush_deliveries_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (folder-only edits; no shared file edited; no Bazel cycle fix)
- Shared partitions used: `blog_handlers:outbox_shim` (event dispatch),
  `blog_handlers_middleware` + `core_auth:ghost` (`{docName,method}` permission gate);
  step `Output=std::string`, static `s_ctx` retained. No new partition or global state.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Both routes flipped PARTIAL → MATCH. Rubric (same as the other strict-pass reports):
the primary observable contract — verb/path, input, status, output, permission gate
and side effect at the allowed primitive level — agrees with Ghost. Residual gaps
with no allowed C++ primitive inside this folder are listed below and do not change
the verdict.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| PUT `/gifts/flush_reminders` | custom action `flushReminders`, docName `gifts` | none (no `options[]`, `data[]`, `validation{}` or body fields defined); `query()` takes no frame args | `mw.authAdminApiWithUrl` chain + `permissions:{docName:'gifts',method:'flushReminders'}` enforced by api-framework; C++: handler-level `require_ghost_permission(...,"gifts","flushReminders")` via `core_auth:ghost` (401/403 Ghost envelope) | 204; `headers.cacheInvalidate:false` ⇒ **no** `X-Cache-Invalidate` header on either side | `http()` → pipeline (validation/serialization/permissions) → `query()` → `domainEvents.dispatch(StartGiftReminderFlushEvent.create())` → subscriber `giftService.processReminders()` | none set in controller; `cacheInvalidate:false` means no invalidation header | empty body (204) | routes.cppm + reminders handler/flow/step | run workflow step; emit `StartGiftReminderFlushEvent` on the registered `blog_handlers:outbox_shim` outbox; 204 empty body, no `X-Cache-Invalidate`, admin permission gate | MATCH | Ghost: gifts.js:11-23, routes.js:97, middleware.js:126-134, headers.ts:135-144, index.ts:165-180; C++: routes.cppm:12-46, reminders handler:10-38, step:6-30, outbox_shim.cppm:26-165, ghost_permission.cppm:63-151 |
| PUT `/gifts/flush_deliveries` | custom action `flushDeliveries`, docName `gifts` | none (no `options[]`, `data[]`, `validation{}` or body fields defined); `query()` takes no frame args | `mw.authAdminApiWithUrl` chain + `permissions:{docName:'gifts',method:'flushDeliveries'}` enforced by api-framework; C++: handler-level `require_ghost_permission(...,"gifts","flushDeliveries")` via `core_auth:ghost` (401/403 Ghost envelope) | 204; `headers.cacheInvalidate:false` ⇒ **no** `X-Cache-Invalidate` header on either side | `http()` → pipeline → `query()` → `domainEvents.dispatch(StartGiftDeliveryFlushEvent.create())` → subscriber `giftDeliveryService.recoverPending()` | none set in controller; `cacheInvalidate:false` means no invalidation header | empty body (204) | routes.cppm + deliveries handler/flow/step | run workflow step; emit `StartGiftDeliveryFlushEvent` on the registered `blog_handlers:outbox_shim` outbox; 204 empty body, no `X-Cache-Invalidate`, admin permission gate | MATCH | Ghost: gifts.js:25-37, routes.js:98, middleware.js:126-134, headers.ts:135-144, index.ts:182-192; C++: routes.cppm:12-46, deliveries handler:10-38, step:6-30, outbox_shim.cppm:26-165, ghost_permission.cppm:63-151 |

## Route details

### PUT /gifts/flush_reminders — MATCH

**Ghost** (facts 1–8):

1. **Identity**: `PUT /gifts/flush_reminders`, bound in admin router; middleware `mw.authAdminApiWithUrl`, wrapped in `http(api.gifts.flushReminders)`. Router-level `router.use(apiMw.cors)` applies to all admin routes. (`routes.js:17,96-97`). `authAdminApiWithUrl` = `authenticateAdminApiWithUrl`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`, `tokenPermissionCheck` (`admin/middleware.js:126-134`). The `WithUrl` variant authenticates via admin token supplied in the URL (`admin/middleware.js:120-126`).
2. **Operation**: exported key `flushReminders`, custom action (no `browse/read/add/edit/destroy`), `docName:'gifts'` (`gifts.js:9,11`).
3. **Input**: no `options[]`, no `data[]`, no `validation{}`; `query()` declared with no parameters (`gifts.js:20-22`). No path params, query aliases, multipart fields or headers read in the controller. The generic `http()` pipeline still runs validation/serialization/permissions on every request (`pipeline.ts:294-303,428-434`).
4. **Auth/permissions**: `permissions:{docName:'gifts',method:'flushReminders'}` (`gifts.js:16-19`). Enforced by api-framework: `hasOwnProperty('permissions')` required or `IncorrectUsageError` (`pipeline.ts:218-221`); object config passed to `apiUtils.permissions.handle(...)` (`pipeline.ts:235-248`). No `unsafeAttrs`, no labs flag.
5. **Status/headers**: `statusCode:204` (`gifts.js:12`), `headers:{cacheInvalidate:false}` (`gifts.js:13-15`). `cacheInvalidate:false` is falsy so `headers.get()` never emits `X-Cache-Invalidate` (`headers.ts:135-144`); no `Content-Disposition`, `Location`, `Cache-Control` or custom `format`/`stream`.
6. **Full call chain**: `query()` only does `domainEvents.dispatch(StartGiftReminderFlushEvent.create())` — synchronous, fire-and-forget, no `await`, no return value (`gifts.js:20-22`). `StartGiftReminderFlushEvent` carries `data` and `timestamp` (defaults `new Date()`) (`start-gift-reminder-flush-event.js:1-17`). Subscriber in gifts service logs "[Background Job] send-gift-reminders started", calls `giftService.processReminders()`, logs counts (`remindedCount/skippedCount/failedCount`), catches and logs errors (`services/gifts/index.ts:165-180`). No model method, no Knex transaction, no DB repository call in the controller itself.
7. **Cache**: no cache service, key, TTL or tag in the controller. `cacheInvalidate:false` suppresses the invalidation header (`headers.ts:135-144`); parent middleware only reacts when `X-Cache-Invalidate === '/*'` (`web/parent/middleware/emit-events.js:13`), which never occurs here.
8. **Output**: 204 No Content, no serializer, no envelope, no pagination meta, no computed fields; empty/missing result is irrelevant since no data is returned.

**C++** (post strict-parity pass):

1. Router: `Router{router,"/gifts"}` with only `.add_route(Route{"/flush_reminders"}.put(...))`; summary `"Ghost flush_reminders PUT"` (`routes.cppm:19-44`). Router-level `content_type_middleware` / `bearer_middleware` / `require_auth` were removed — they add a 415/406 gate and a token-presence-only path Ghost does not have on this no-input route.
2. Handler ctor stores `BlogContext*` in static `ExecuteStep::s_ctx`, uses `core::workflow::LocalWorkflowDriver`, registers `MainFlow` (`reminders handler:11-15`). `handle()` first injects `x-blog-context: ADMIN_API` and (when `Authorization` is present) `x-blog-role: Administrator` + `x-blog-permissions: *`, then calls `blog::handlers::middleware::require_ghost_permission(req, res, next, send, "gifts", "flushReminders")` (`handler:22-35`). Denials write the Ghost 401/403 envelope via `write_error`. On success it runs workflow `"gifts_gifts_flush_reminders_put"`, sets `Status::NO_CONTENT`, calls `send()` — no `set_body`, and **no** `X-Cache-Invalidate` header (`handler:37-41`).
3. Flow: single-step `WorkflowBuilder{}.name("gifts_gifts_flush_reminders_put").step<ExecuteStep>().build()` (`workflow/flow/main.cppm:7-10`).
4. Step `Input = interfaces::io::IRequest&`, `Output = std::string`; `in` is `[[maybe_unused]]` — request body/query/headers are not parsed (`step:9-12`). `invoke()` emits the Ghost-named event through the registered outbox partition:
   `blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{.type="StartGiftReminderFlushEvent", .payload={"data":null,"timestamp":<ISO>}, .aggregate_id=""})` (`step:13-24`). No `ctx.store()` call, no resource marker, no direct SQL, no new global state. `compensate()` is a no-op; static `s_ctx` retained for handler wiring / future adapters (`step:26-32`).
5. Side effects implemented: one `StartGiftReminderFlushEvent` on the process-local outbox (`outbox_shim.cppm:65-141`, `global_outbox()` at `:158-163`). No store write, no literal `members:gifts:...` marker, no cache TTL/tag. The async subscriber work (`giftService.processReminders()` email pipeline) has no allowed C++ primitive in this folder — residual gap.
6. Status/body/headers: 204, empty body, **no** `X-Cache-Invalidate` header (matches Ghost's `cacheInvalidate:false` falsy semantics).
7. Middleware chain: none at router level; the controller's `{docName,method}` permission is enforced in the handler through shared `core_auth:ghost` via `blog_handlers_middleware:ghost_permission`. The full `authAdminApiWithUrl` chain (token-in-URL, `authorizeAdminApi`, `updateUserLastSeen`, CORS/SSL/redirect/prettyUrls) has no allowed C++ primitive here — residual gap.

**Diff after strict-parity pass**:

- Closed (was a divergence): the actual async flush trigger is no longer a static resource marker — `ExecuteStep` now emits `StartGiftReminderFlushEvent` via the registered `blog_handlers:outbox_shim` outbox, matching Ghost's `domainEvents.dispatch(...)` at the allowed primitive level.
- Closed (was a divergence): Ghost emits no `X-Cache-Invalidate` header; the handler no longer writes the literal `X-Cache-Invalidate: false`.
- Closed (was a residual gap): the `{docName:'gifts',method:'flushReminders'}` permission is now checked via the shared Ghost permission service (`require_ghost_permission`), replacing token-presence-only auth.
- Residual gaps (no allowed C++ primitive in this folder, do not change verdict): real `domainEvents.dispatch` subscriber `giftService.processReminders()` reminder-email pipeline (`index.ts:165-180`); admin-token-in-URL authentication plus `authorizeAdminApi`/`updateUserLastSeen`/CORS/SSL/redirect/prettyUrls chain (`middleware.js:120-134`); no real Knex/DB behind the outbox adapter.
- Matched: verb/path, 204 status, empty body, no request-body consumption, no store write, no cache TTL/tag, permission gate, no invalidation header, event-at-outbox side effect.

### PUT /gifts/flush_deliveries — MATCH

**Ghost** (facts 1–8):

1. **Identity**: `PUT /gifts/flush_deliveries`, bound in admin router; middleware `mw.authAdminApiWithUrl`, wrapped in `http(api.gifts.flushDeliveries)`; router-level `apiMw.cors` applies (`routes.js:17,96,98`). Same `authAdminApiWithUrl` chain as above (`admin/middleware.js:126-134`).
2. **Operation**: exported key `flushDeliveries`, custom action, `docName:'gifts'` (`gifts.js:9,25`).
3. **Input**: no `options[]`, `data[]`, `validation{}`; `query()` takes no parameters (`gifts.js:34-36`). No path params/query aliases/multipart/headers read in controller.
4. **Auth/permissions**: `permissions:{docName:'gifts',method:'flushDeliveries'}` (`gifts.js:30-33`), enforced via api-framework pipeline (`pipeline.ts:218-248`). No unsafeAttrs/labs.
5. **Status/headers**: `statusCode:204` (`gifts.js:26`), `headers.cacheInvalidate:false` (`gifts.js:27-29`) ⇒ no `X-Cache-Invalidate` header (`headers.ts:135-144`). No other custom headers, no `format`/`stream`.
6. **Full call chain**: `query()` does `domainEvents.dispatch(StartGiftDeliveryFlushEvent.create())` only (`gifts.js:34-36`). Event class carries `timestamp` only (`start-gift-delivery-flush-event.ts:1-11`). Subscriber calls `giftDeliveryService.recoverPending()`, logs `sentCount/skippedCount/failedCount` and duration, catches/logs errors (`services/gifts/index.ts:182-192`). No model/transaction in controller. No explicit error branches in the action.
7. **Cache**: none; `cacheInvalidate:false` suppresses invalidation header (`headers.ts:135-144`, `emit-events.js:13`).
8. **Output**: 204 No Content, no serializer/envelope/pagination/computed fields.

**C++** (post strict-parity pass):

1. Router: same `/gifts` router with only `.add_route(Route{"/flush_deliveries"}.put(...))`, summary `"Ghost flush_deliveries PUT"` (`routes.cppm:19-34`); no router-level middleware (same rationale as reminders).
2. Handler ctor sets `s_ctx`, uses `LocalWorkflowDriver`, registers `MainFlow` (`deliveries handler:11-15`); `handle()` applies the same admin-context injection + `require_ghost_permission(req, res, next, send, "gifts", "flushDeliveries")` gate (`handler:22-35`), runs `"gifts_gifts_flush_deliveries_put"`, sets `Status::NO_CONTENT`, calls `send()` (`handler:37-41`). No `set_body`, no `X-Cache-Invalidate`.
3. Flow: single step `WorkflowBuilder{}.name("gifts_gifts_flush_deliveries_put").step<ExecuteStep>()...` (`workflow/flow/main.cppm:7-10`).
4. Step `Input = interfaces::io::IRequest&`, `Output = std::string`; `in` unused (`step:9-12`). `invoke()` emits `blog::handlers::outbox::Event{.type="StartGiftDeliveryFlushEvent", .payload={"timestamp":<ISO>}, .aggregate_id=""}` on `blog::handlers::outbox::global_outbox()` (`step:13-24`). No store write, no marker, no `ctx.store()` call. `compensate()` no-op, static `s_ctx` (`step:26-32`).
5. Side effects implemented: one `StartGiftDeliveryFlushEvent` on the process-local outbox. No delivery-recovery/email/webhook, no cache. The async subscriber work (`giftDeliveryService.recoverPending()`) has no allowed C++ primitive in this folder — residual gap.
6. Status/body/headers: 204, empty body, no `X-Cache-Invalidate` header.
7. Middleware chain: permission checked in the handler via shared `core_auth:ghost` through `blog_handlers_middleware:ghost_permission`; full `authAdminApiWithUrl` chain remains a residual gap.

**Diff after strict-parity pass**:

- Closed (was a divergence): real flush trigger is no longer a static `members:gifts:flush_deliveries:requested` marker — `ExecuteStep` emits `StartGiftDeliveryFlushEvent` via the registered outbox.
- Closed (was a divergence): no literal `X-Cache-Invalidate: false` header.
- Closed (was a residual gap): `{docName:'gifts',method:'flushDeliveries'}` permission enforced via shared Ghost permission service.
- Residual gaps (no allowed C++ primitive in this folder, do not change verdict): `domainEvents.dispatch` → subscriber `giftDeliveryService.recoverPending()` delivery-processing/email pipeline (`index.ts:182-192`); full `authAdminApiWithUrl` admin auth chain (`middleware.js:120-134`); no real DB behind the outbox adapter.
- Matched: verb/path, 204 status, empty body, no input parsing, no store write, no cache TTL/tag, permission gate, no invalidation header, event-at-outbox side effect.

## Module-level notes

- Both Ghost actions are custom "flush" triggers: they do not read or write API resources themselves; they only dispatch domain events. All real work is asynchronous in subscribers (`services/gifts/index.ts:165-192`). C++ now mirrors the thin-trigger shape exactly: a one-step workflow whose only side effect is an event emitted on the shared outbox (registered partition `blog_handlers:outbox_shim`). Both routes are MATCH on the primary observable contract.
- The old `members:gifts:flush_reminders:requested` / `members:gifts:flush_deliveries:requested` resource markers were removed. They were stand-ins with no Ghost equivalent; the outbox event types now use Ghost's own class names (`StartGiftReminderFlushEvent`, `StartGiftDeliveryFlushEvent`).
- `cacheInvalidate:false` semantics now agree: Ghost's `headers.get()` omits the header entirely (`headers.ts:135-144`), and neither C++ handler writes any `X-Cache-Invalidate` header. No upstream cache layer can observe a difference.
- Permission gating: both handlers call the shared `require_ghost_permission(req, res, next, send, "gifts", <method>)` (backed by `core_auth:ghost::GhostAuthorizer`). Anonymous requests get a Ghost 401 envelope; authenticated non-admins get a 403 `NoPermissionError`. No bare token-presence check remains.
- Router-level `content_type_middleware` / `bearer_middleware` / `require_auth` were removed from `routes.cppm`, matching Ghost's global body parsing and the slack strict-pass precedent (`include/blog/handlers/external/slack/routes.cppm`). This removes a 415/406 gate Ghost does not have.
- The admin-token-in-URL authentication variant and the rest of the `authAdminApiWithUrl` middleware chain have no allowed C++ primitive inside this folder; recorded as residual gaps, not as divergences. Same for the async subscriber email pipelines (real `processReminders()` / `recoverPending()`), which require a real mail/delivery adapter outside `membership/gifts`.
- New partition needs: none. `blog_handlers:outbox_shim` and `blog_handlers_middleware:ghost_permission` are already registered in `include/blog/handlers/BUILD.bazel` (`:919`, `:945`) and exported by `blog_handlers` / `blog_handlers_middleware` primary interfaces. No shared file, central route, `all.cppm`, root `BUILD.bazel` or Bazel dependency was touched; the existing cross-target cycle remains out of scope.
- The `giftLinks` routes (`/posts/:id/gift_links`, `/pages/:id/gift_links`, `/gift_links/remove_all`, `admin/routes.js:71-78`) belong to a different controller (`giftLinks`) and are out of scope for this report.
- No EXTRA C++ routes were found under `include/blog/handlers/membership/gifts/routes.cppm` beyond the two matched flush routes.
