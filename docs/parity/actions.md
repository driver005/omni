# Parity: actions

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/actions.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/actions/routes.cppm`
- Checked: 2026-11-20

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
| GET /actions | browse | GET /actions | `actions_browse()` → `blog::handlers::actions::browse::BrowseHandler` | Ghost: `page,limit,fields,include,filter` via `restrictAdminApiQueryOptions`; C++: same five via `core::query::parse` + `valid_option` + restricted-field guard | Ghost: `{actions:[...],meta:{pagination:{...}}}` with clean.action shape, no `X-Cache-Invalidate`; C++: same envelope via `blog::mapping::clean`/`wrap`/`pagination`, no cache header | Ghost: `models.Action.findPage(...)`; C++: `ctx.store().json_for("actions", {})` + `core::query::run` (in-memory store stand-in) | none on both sides | Ghost: `mw.authAdminApi` + `permissions:true`/`docName:'actions'`; C++: `require_auth` (401) + `require_ghost_permission("actions","browse")` via `core::auth::ghost` (403 NoPermissionError) | MATCH | Ghost `admin/routes.js:544`, `actions.js:6,8-16`; C++ `external/actions/routes.cppm:19-27`, `browse/handler.cppm:18-52`, `browse/workflow/step/execute.cppm:15-79` |

## Route details

### GET /actions — MATCH

- **Route identity — MATCH**: Ghost `router.get('/actions', mw.authAdminApi, http(api.actions.browse))` (`admin/routes.js:544`); C++ `Router{router, "/actions"}.get(...)` (`external/actions/routes.cppm:19-27`). Verbatim path/verb.
- **Operation mapping — MATCH**: Ghost `browse` (`actions.js:8`); C++ binds `handler.actions_browse()` → `BrowseHandler` (unchanged central registration).
- **Input parity — MATCH**: C++ `allowlisted_query()` keeps only the five exact keys `page,limit,fields,include,filter` before `core::query::parse` (`execute.cppm:20-23,85-106`) — `filter[x]`, `offset`, `order` and other keys are dropped as Ghost's `Frame.configure` does (`frame.ts:66-76`). `core::query::valid_option` reproduces the `page`/`limit` regexes; invalid values return a Ghost 422 `ValidationError` (`execute.cppm:25-29,129-141`). Restricted fields in `filter`/`order` are rejected by `core::query::run`'s guard with a 400 BadRequestError "Restricted fields cannot be used in order." (`execute.cppm:46-51,144-148`).
- **Output parity — MATCH**: Raw rows from `ctx.store().json_for("actions", {})` are passed through `blog::mapping::clean(rows, "action")` (ACTION_FIELDS + nested `clean.action` actor/resource field picking and image aliasing), then `core::query::run` applies filter/order/fields/page/limit and reports totals. Final body is `blog::mapping::wrap("actions", data, {"pagination":...})` with `blog::mapping::pagination` (`execute.cppm:40-76`). Ghost's empty-result `pages:0` is preserved (`execute.cppm:66-68`). No `X-Cache-Invalidate` header is written (`handler.cppm:50-51`), matching `headers.cacheInvalidate:false`.
- **DB call parity — MATCH (architecture stand-in)**: Ghost `Action.findPage(restrictAdminApiQueryOptions(...))`; C++ store read + `core::query` supplies the same filtering/projection/pagination semantics over the in-memory `actions` resource. No real SQL Bookshelf adapter exists in C++; recorded as accepted data-layer residual, not a handler divergence.
- **Cache parity — MATCH (none)**: no cache adapter/key/TTL on either side; false invalidation header removed from C++.
- **Auth / permissions — MATCH**: route keeps `require_auth` for the 401 `UnauthorizedError` gate and drops the 415 `content_type_middleware` gate absent from Ghost (`routes.cppm:20-21`). `BrowseHandler::handle` publishes `ADMIN_API` + Administrator/`*` headers for bearer callers and delegates to `blog::handlers::middleware::require_ghost_permission(req,res,...,"actions","browse")`, backed by `core::auth::ghost::GhostAuthorizer` (`handler.cppm:22-37`). Denial yields Ghost's 403 `NoPermissionError` with message "You do not have permission to browse actions".
- **Side effects — MATCH**: read-only route; no outbox event emitted on either side. `core_outbox` is intentionally unused (Ghost controller has no event/job side effect). No new outbox partition needed.

Overall **MATCH**: path, verb, operation, five-option input, validation/guard, clean action envelope, pagination, no-cache-header, 401 and 403 permission behavior all agree. Residual data-layer stand-in (empty in-memory fixture, no SQL morphTo eager loading) is outside handler parity.

#### Fix

All eight checks from the prior PARTIAL pass are now applied in the owned folder only:

- Check 1 (route identity): unchanged binding `Router{"/actions"}.get(...)`.
- Check 2 (operation mapping): unchanged `actions_browse()` / `BrowseHandler`.
- Check 3 (input): `execute.cppm` — `allowlisted_query` + `core::query::parse` with exactly the Ghost option list; added `page`/`limit` validation via `core::query::valid_option`.
- Check 4 (output): `execute.cppm` — `blog::mapping::clean(...,"action")`, `core::query::run`, `blog::mapping::pagination`, `blog::mapping::wrap("actions",...)`; removed hand-built envelope and fabricated `limit/page/pages/prev/next` constants.
- Check 5 (DB call): `execute.cppm` — `ctx.store().json_for("actions", {})` (raw) + shared `core::query::run` for filtering/pagination instead of forwarding arbitrary params to `search_json`.
- Check 6 (cache): `handler.cppm` — removed `res.set_header("X-Cache-Invalidate","false")`; no header emitted.
- Check 7 (auth): `routes.cppm` — removed extra `content_type_middleware` 415 gate, kept `require_auth`; `handler.cppm` — replaced inline token-presence branch with `require_ghost_permission("actions","browse")` using `core::auth::ghost`.
- Check 8 (verdict + evidence): flipped both route tables from PARTIAL to MATCH; evidence updated to new file:line locations.

## Missing in C++

None. The sole Ghost route `GET /actions` (`admin/routes.js:544`) has a C++ counterpart.

  Fix: No change required. Check 1 (MISSING) closed in the prior pass.

## Extra in C++

None. The folder registers only `GET /actions`; the `/actions` fragment in `membership/automations/routes.cppm` belongs to a different Ghost endpoint.

  Fix: No change required. No extra route to remove or gate.

## Needs central registration

No new files or partition names were added; all edits were in-place in the four existing files of `include/blog/handlers/external/actions/`. Parent session must verify/close the following shared-file items (none edited here):

- **New-partition need (pre-existing, systemic): `blog_handlers:mapping`.** `include/blog/handlers/mapping/{base,clean,wrap}.cppm` exist but are NOT listed in `include/blog/handlers/BUILD.bazel` partitions and `include/blog/handlers/base.cppm` has no `export import :mapping;`. The step imports `blog_handlers:mapping` (as invites/comments/search_index steps already do). Parent must register those three partitions in order and add the re-export to break the known Bazel module cycle. I did not edit these shared files.
- `blog_handlers:actions_browse_step` / `:actions_browse_handler` / `:actions_browse_flow` — already registered at `include/blog/handlers/BUILD.bazel:760-762`; in-place edits only.
- `blog_handlers_routes:actions_routes` — already registered at `BUILD.bazel:963`; in-place edit only.
- `blog_handlers_middleware:ghost_permission` — already registered at `BUILD.bazel:945` and re-exported by `middleware/base.cppm:8`; `require_ghost_permission` used by the handler needs no new dependency.
- `core_query` (`//include/core/query:core_query`) and `core_auth` (`//include/core/auth:core_auth`) — already deps of `blog_handlers` (`BUILD.bazel:924,927`); no new dep line.
- `core_outbox` — not required for this read-only route; intentionally no import and no registration need.

New imports added by the fixes: `core_query`, `blog_handlers:mapping` (step), `blog_handlers_middleware` was already imported (handler/routes). No new global state: the existing `ExecuteStep::s_ctx` static continues to carry the `BlogContext`; no cache instance, no outbox instance.

Verification note: full Bazel build of `//include/blog/handlers:blog_handlers` is blocked in unrelated shared files (`discovery/recommendations/.../execute.cppm` ambiguous `pagination_meta` / undeclared `now_iso`; `auth/session/create/.../execute.cppm` `nodiscard` warnings) and by the known `blog_handlers` module cycle while `blog_handlers:mapping` remains unregistered. No compile error was attributed to the four actions files; targeted manual module verification was likewise blocked by the missing shared `blog_handlers` primary PCM.
