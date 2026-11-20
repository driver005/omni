# Parity: schedules

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/schedules.js`
- Ghost routes: `.../web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/schedules/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 55)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

One Ghost controller route; one C++ counterpart. Verdict stays **PARTIAL** —
every Ghost observable contract point is now implemented through shared
abstractions (validation, `core_auth:ghost` permission gate, outbox side
effects, Ghost-shaped output), but infrastructure residuals below have no
primitive inside this folder and are recorded, not hidden. No verdict flip is
claimed: MATCH requires those residuals to be closed by centrally owned work.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| PUT `/schedules/:resource/:id` | `publish` | PUT `/schedules/:resource/:id` (prefix `/schedules` + router `/:resource` + route `/:id`, `.put()`) | `schedules_put_detail().handle()` | Ghost: `options:['id','resource']`, `data:['force']`, validation `id` required, `resource ∈ [posts,pages]`, global id regex `^(?:[a-f\d]{24}\|me)$`; `force === true`. C++: `ExecuteStep::preflight()` parses path segments, emits Ghost-shaped 422 for missing id / bad resource / bad id format; `force` parsed from body as boolean `true` | Ghost: `{[resource]:[scheduledResource]}` or `{[resource]:[]}` (200 no-op). C++: same envelopes; rows passed through registered post/page Ghost allow-list mapper | Ghost: `api[resource].read({id},{status:'scheduled',context:{internal:true}})` then `api[resource].edit(... status:'published', updated_at ...)`. C++: `find_by_resource` → status guard → tolerance/force guard → `ctx.store().edit(resource,id,...)` | Ghost: `X-Cache-Invalidate: /*` on real publish, none on no-op; header computed by `handleCacheInvalidation`. C++: same, computed from status transition; cache.invalidate outbox event emitted | Ghost: `authAdminApiWithUrl` + `permissions.canThis(ctx).publish.post(id)` (NotFoundError swallowed). C++: URL-token injection + `posts::support::authorize(resource,"publish")` via `core_auth:ghost`; NotFound swallowed by the no-op branch | PARTIAL | G `routes.js:94`, `schedules.js:5-11,14-87`, `post-scheduling.js:17-130`; C++ `routes.cppm`, `put_detail/handler.cppm`, `put_detail/workflow/step/execute.cppm` |

C++ routes installed via `include/blog/handlers/routes.cppm` (`SchedulesRoutes{}.install(...)`); handler registered in `all.cppm` / `base.cppm`. No central file was edited in this pass.

## Route details

### PUT `/schedules/:resource/:id` — PARTIAL

- Route identity: matched. Ghost `router.put('/schedules/:resource/:id', mw.authAdminApiWithUrl, http(api.schedules.publish))` (routes.js:94). C++ mounts `/schedules` + `/:resource` + `/:id` with `.put()` — `/schedules/:resource/:id`, verb and both placeholders aligned.
- Operation mapping: matched. Ghost key `publish` → `postScheduling.publish(resourceType,id,force,options)`. C++ binds `handler.schedules_put_detail().handle()` (getter name is not wire-visible) → workflow `schedules_put_detail` → `ExecuteStep::invoke` implementing read-scheduled → guards → edit-to-published. `MainFlow` / `LocalWorkflowDriver` unchanged.
- Input parity: matched at the observable contract. Validation runs in `preflight()` **before** auth (Ghost pipeline order): `id` required → 422 `Validation (FieldIsRequired) failed for id`; `resource` required and restricted to `posts`/`pages` → `Validation (AllowedValues) failed for resource`; global id format `^[a-f\d]{24}|me$` (case-insensitive) → `Validation (matches) failed for id`. `force` is read from body and only boolean `true` counts, matching Ghost's `force === true`.
- DB call parity: matched at the C++ store abstraction. `ctx.store().find_by_resource(resource,id)` resolves the posts/pages model entity; missing/non-`scheduled` → no-op. Published timestamp tolerance uses the Ghost default of 5 minutes (`shared/config/overrides.json:245`): future beyond tolerance → no-op; past beyond tolerance without `force` → 404 `NotFoundError` with Ghost's message. The full stored record is decoded with `core_serde::JsonCodec`, `status` set to `published`, `updated_at` set to the current UTC ISO-8601 instant (Ghost `moment(...).toISOString(true)` shape), and written with `ctx.store().edit(resource,id,edit_body)`. No `ResourceStore::edit("schedules","",...)`/`create` fallthrough remains.
- Output parity: matched at the observable contract. `result.body()` rows are cleaned through the registered `blog::handlers::posts::support::clean_rows(rows,"post"|"page")` Ghost allow-list mapper and wrapped as `{"posts":[...]}` / `{"pages":[...]}`. No-op yields `{"posts":[]}` / `{"pages":[]}` at 200; validation yields the Ghost-shaped 422 envelope; no fabricated numeric-id `schedules` record.
- Cache parity: matched at the observable contract. `handleCacheInvalidation(next,prev,uuid)` reproduces Ghost's branches: scheduled→published → `/*` header + `cache.invalidate` outbox event + `X-Cache-Invalidate`; draft/scheduled branch would produce `/p/<uuid>/`; no-op emits no header. `headers.cacheInvalidate:false` is honored by suppressing the framework header.
- Auth / permissions: matched at the contract level. `apply_url_token()` lifts `?token=...` from the request path into `Authorization` when no header is present (Ghost `authAdminApiWithUrl`). Router-level `content_type_middleware`/`require_auth` were removed (Ghost uses body parsers, not a Content-Type gate). `posts::support::authorize(req,res,send,resource,"publish")` runs `core_auth:ghost` via the registered middleware, producing Ghost `UnauthorizedError` (401) / `NoPermissionError` (403) envelopes. A presented admin token is mapped to `Administrator` as in the other strict-parity groups; the fine-grained `Post.permissible` NotFound-only swallow is reproduced by the step's read-miss no-op.
- Side effects: publish is now observable through the registered `blog_handlers:outbox_shim` partition: `<post|page>.published`, `<post|page>.edited`, and `cache.invalidate` events are emitted with the full serialized payload and resource id, then `drain()`ed to registered adapters/sink. In-flight overlap uses context-local `ResourceStore` state (`schedules_inflight:<resource>:<id>`) with an RAII release, mirroring Ghost's process-global `Set` without new global state.

### Fix blocks (all closed by source edits this pass)

- Check 1 (route identity) — CLOSED. `/:id` child router renamed `/:resource`; registered path `/schedules/:resource/:id`. No route added or removed.
- Check 2 (operation mapping) — CLOSED. Binding kept; generic `schedules` edit/create replaced by the publish pipeline in the step. Flow/registration unchanged.
- Check 3 (input parity) — CLOSED. `preflight()` adds the Ghost validation messages, global id-format check and strict boolean `force`; validation runs before auth, matching the Ghost pipeline.
- Check 4 (output parity) — CLOSED. `Output` stays `std::map<std::string,std::string>` with `status`/`body`/`invalidate`; handler writes a 200/404/422 status, conditional `X-Cache-Invalidate`, and the cleaned Ghost envelope. `s_ctx` static pointer retained as required by the workflow driver pattern.
- Check 5 (DB call parity) — CLOSED. `find_by_resource` + `edit(resource,id,...)` with full-field JSON round-trip; status guard, tolerance/force guard, not-found and edit-failure no-ops; status/updated_at transition only.
- Check 6 (cache parity) — CLOSED. Invalidation computed from the status transition (not hardcoded); emitted through the header and the outbox; no-op emits neither.
- Check 7 (auth / permissions) — CLOSED. URL-token injection + `core_auth:ghost` permission gate through the registered `posts::support::authorize` helper; router-level Content-Type/token-presence checks removed.
- Check 8 (side effects) — CLOSED. Publish side effects are emitted through the registered outbox partition and drained; no new global state introduced.
- Check 9 (verdict) — NOT flipped. All documented divergences are closed at the C++ abstraction boundary; remaining items are infrastructure residuals (real DB/config/webhook/token store and an unregistered `:mapping` partition) owned by the main agent. Verdict remains PARTIAL.

## Missing in C++

None. The single Ghost controller route has a C++ counterpart by exact verb and `/schedules` prefix:

- PUT `/schedules/:resource/:id` → `system/schedules/routes.cppm`, handler `system/schedules/put_detail/handler.cppm`

No new handler, no route addition and no `core::cache::Cache` instance were introduced.

## Extra in C++

None. `SchedulesRoutes::install` registers exactly one `.put(...)` route, the counterpart of Ghost's `PUT /schedules/:resource/:id`.

## Source edits applied

All edits confined to `include/blog/handlers/system/schedules/` plus this doc. No shared file, `all.cppm`, base module, central `routes.cppm` or BUILD file was edited.

- `routes.cppm`: removed `.use(content_type_middleware)` and `.use(require_auth)`; auth/permissions now run inside the handler. Kept `/schedules/:resource/:id` mount and the single `.put(...)` binding.
- `put_detail/handler.cppm`: added registered-partition imports (`:posts_support`, `:outbox_shim`, `:schedules_put_detail_step`); Ghost validation preflight before auth; `?token=` lifting; `posts::support::authorize(resource,"publish")`; status/body/header writer. `Output` remains a `std::map<std::string,std::string>`; `s_ctx` remains the static context pointer.
- `put_detail/workflow/step/execute.cppm`: full rewrite — validation helpers, strict `force`, context-local in-flight guard, scheduled read + tolerance/force guards, full-field publish edit, Ghost allow-list output through `posts::support::clean_rows`, status-transition cache header, outbox events and drain, local RFC3339/Ghost-id/cache helpers.
- `put_detail/workflow/flow/main.cppm`: unchanged (workflow name/step unchanged).

## Needs central registration (recorded, not performed)

- `blog_handlers:mapping` (`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) exists on disk but is **not** in `include/blog/handlers/BUILD.bazel` partitions and not re-exported by `base.cppm`. This pass therefore uses the already-registered `blog_handlers:posts_support` mapper (`clean_rows`) and a local wrap. Main agent: add the three mapping partitions + `export import :mapping;` if the canonical mapper should replace the local wrap.
- `core_outbox` is a separate Bazel target not listed in `//include/blog/handlers` deps. This pass uses the registered `blog_handlers:outbox_shim` clone (same API). Main agent: add `//include/core/outbox:core_outbox` to deps and repoint the handler if canonical outbox ownership is required. No BUILD/cycle fix was attempted here.
- A shared Ghost global-value validator (`id` regex, tolerance config) and a webhook delivery adapter are not registered in this folder; local helpers cover the observable contract, central wiring remains owned by the main agent.
- The working tree currently reports Bazel `cycle in dependency graph` errors from concurrent cross-partition edits in other groups. No shared file or BUILD change was made to resolve them; verification of this folder was done by compiling its four module units directly with the cached BMIs (all succeeded except `routes.cppm`, which needs the not-yet-built `base.pcm`). Cycle resolution stays with the main agent.

## Remaining gaps (residual, no primitive in this folder)

- Real Knex `posts`/`pages` table and full `api[resource].edit` model pipeline (domain events, `published_by`, revisions, webhooks, search indexing) — the C++ store mutates the in-memory model entity and records the same events on the outbox; webhook HTTP delivery depends on a registered adapter.
- `times.publishAPostBySchedulerToleranceInMinutes` is read from the Ghost default (5 minutes), not from a shared config service; config overrides cannot be honored here.
- Token→role mapping is presence-based (`Administrator`); real admin-token verification/allowlisting and the full `authAdminApiWithUrl` middleware chain remain centrally owned. URL token is not URL-decoded.
- Fine-grained `Post.permissible(modelId,'publish')` and `canThis` loader are represented by the coarse `core_auth:ghost` decision plus the read-miss no-op.
- No real process-wide in-flight registry: the guard is context-local `ResourceStore` state, equivalent for the shared `BlogContext` but not across contexts.
- `core::query::valid_option` does not enforce Ghost's global id regex; the check is local until a shared validator partition is registered.
