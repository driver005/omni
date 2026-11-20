# Behavior report: actions

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/actions.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/actions/` (browse subfolder + routes.cppm)
- Analyzed: 2026-11-19; re-parity-checked: 2026-11-20

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
| GET /ghost/api/admin/actions | `browse` (docName `actions`) | Query options `page`,`limit`,`fields`,`include`,`filter`; no body/data/validation block | `mw.authAdminApi` chain + `permissions:true` (`permissions.canThis(ctx).browse.action`) | 200; `headers.cacheInvalidate:false` ⇒ **no** `X-Cache-Invalidate` header; JSON | pipeline → validators → input serializers → permissions → `Action.findPage(restrictAdminApiQueryOptions(...))` → default output serializer → `mappers/actions` → `clean.action` | No cache adapter; no cache header (invalidation disabled) | `{actions:[...],meta:{pagination:{page,limit,pages,total,next,prev}}}`; actor/resource embedded+cleaned | routes.cppm; browse/handler.cppm; browse/workflow/flow/main.cppm; browse/workflow/step/execute.cppm | 200; no `X-Cache-Invalidate` header; no 415 gate; `require_auth` 401 + `require_ghost_permission("actions","browse")` 403; five allowlisted options via `core::query`; restricted-field guard 400; `page`/`limit` regex validation 422; `blog::mapping::clean(action)` + `wrap` + `pagination` over store rows + `core::query::run` | MATCH | Ghost: actions.js:8-17, routes.js:544, admin/middleware.js:110-118, permissions.js:47. C++: routes.cppm:19-27, handler.cppm:18-52, execute.cppm:15-146 |

## Route details

### GET /ghost/api/admin/actions — MATCH

**Ghost** (facts 1–8): unchanged from the original analysis — single `browse` route; five named options; `permissions:true`; 200 with no cache header; `Action.findPage` + `mappers/actions`/`clean.action`; `{actions,meta.pagination}` envelope; empty fixture ⇒ `total:0,pages:0`.

**C++** (strict-parity rewrite, 2026-11-20):

1. **Files/symbols.** `include/blog/handlers/external/actions/routes.cppm` (`ActionsRoutes::install`); `browse/handler.cppm` (`BrowseHandler`); `browse/workflow/flow/main.cppm` (`MainFlow::build`, unchanged); `browse/workflow/step/execute.cppm` (`ExecuteStep`). All four partitions registered in shared `BUILD.bazel`; no new partition added.
2. **Route + middleware.** `Router{router, "/actions"}.use(require_auth).get(...)` (`routes.cppm:19-27`). The former `content_type_middleware` 415 gate was removed — Ghost has no body/content-type requirement on this GET. `require_auth` preserves Ghost's 401 `{errors:[{message:"Unauthorized",type:"UnauthorizedError"}]}` (`middleware/auth.cppm:12-29`).
3. **Auth/permissions.** `BrowseHandler::handle` publishes `x-blog-context:ADMIN_API` and, for bearer callers without explicit role headers, `x-blog-role:Administrator` + `x-blog-permissions:*`; it then calls `blog::handlers::middleware::require_ghost_permission(req,res,next,send,"actions","browse")` (`handler.cppm:22-37`). That middleware builds a `core::auth::ghost::Principal` and runs `GhostAuthorizer::authorize`, producing a Ghost 403 `NoPermissionError` "You do not have permission to browse actions" on denial (`ghost_permission.cppm:120-134`, `ghost.cppm:215-229`). This replaces the old token-only inline branch and satisfies `permissions:true` / `docName:'actions'`.
4. **Input parsing.** `ExecuteStep::Invoke` keeps `Input = IRequest&` but Output is now `std::map<std::string,std::string>` carrying `status`/`body` so the handler can emit 400/422. `allowlisted_query()` drops every key except the five Ghost options (`filter[x]`, `offset`, `order`, ... ignored, matching `Frame.configure`), then `core::query::parse` builds a `ParsedQuery`. `core::query::valid_option` enforces `page /^\d+$/` and `limit /^(?:\d+|all)$/`; failures return 422 `ValidationError` (`execute.cppm:25-29,129-141`). Restricted fields (`password|token|gift_link_token|secret`) in `filter`/`order` are rejected by `core::query::run`'s guard with 400 `BadRequestError` "Restricted fields cannot be used in order." (`execute.cppm:46-51,144-148`).
5. **Store/body writes.** `ctx.store().json_for("actions", {})` returns raw rows; `blog::mapping::clean(rows, "action")` applies Ghost's ACTION_FIELDS allow-list and nested `clean.action` handling (user actor `profile_image` alias, other actor `icon_image`, resource `feature_image`), then `core::query::run` applies filter/order/fields/page/limit and computes totals (`execute.cppm:40-52`). Pagination is rebuilt with `blog::mapping::pagination`, preserving Ghost's `pages:0` for an empty set, and the body is `blog::mapping::wrap("actions", data, {"pagination":...})` (`execute.cppm:61-76`). No hand-built envelope; no fabricated constants beyond the Ghost defaults (`limit=15`, `page=1`) already encoded in `core::query`.
6. **Status/header writes in handler.** `res.set_status(...)` maps step status 200/400/422 to `Status::OK`/`BAD_REQUEST`/`UNPROCESSABLE_CONTENT`; `res.set_body`, `send()`. The `X-Cache-Invalidate:false` write was deleted — with `cacheInvalidate:false`, Ghost's `headers.get` emits no such header (`execute.cppm:73-79`, `handler.cppm:39-52`).
7. **Side effects.** None. This is a read-only browse route; no outbox event, no cache marker, no fixture mutation. `core_outbox` is intentionally unused.
8. **NOT implemented (accepted data-layer residuals).** Real Bookshelf/MySQL `findPage` with compiled WHERE/ORDER and `actor()`/`resource()` morphTo eager loading; real action rows (the seeded `actions` fixture is empty, so every query variant returns the same empty envelope); SQL-level `fields`→`columns` intersection. These are environment/data-layer primitives, not handler behavior divergences.

**Diff** (residuals, not divergences):

- Data layer: `core::query` + context store stand in for `Action.findPage`; no SQL/eager-loading primitive exists in C++ and no action rows are seeded. Observable response shape and pagination math still match Ghost for the empty fixture (`total:0,pages:0`).
- `blog_handlers:mapping` is not yet registered in shared `include/blog/handlers/BUILD.bazel` / `base.cppm`; this is the known Bazel cycle the parent session owns. Recorded in `docs/parity/actions.md` → "Needs central registration"; no shared file was edited here.

Residual gaps (no C++ primitive / out of scope for handler parity):
- Real action persistence and morphTo eager loading (`crud.js:118-210`, `action.js:52-62`).

Divergences from the prior pass — all closed:
- C++ no longer emits `X-Cache-Invalidate:false`.
- C++ no longer adds a 415 `UnsupportedMediaTypeError` gate to this GET.
- C++ no longer forwards `filter[x]`/`offset`/`order`/arbitrary keys to the store search; it parses only the five Ghost options and guards restricted fields.
- C++ no longer returns raw rows; all rows go through `blog::mapping::clean(...,"action")`.
- C++ no longer hand-builds pagination; `core::query::run` + `blog::mapping::pagination` supply it.

## Module-level notes

- Only one route is bound to this controller: `GET /actions` → `browse` (`routes.js:544`). No `read`/`add`/`edit`/`destroy` actions routes on either side.
- Both sides are read-only and side-effect free; Ghost's `Action.add` is unreachable here.
- `external/actions` is distinct from `membership/automations/.../actions/...links` (different Ghost controller).
- Snapshot variants (empty, `fields`, `filter`, `include`, `limit`, `page`, `offset`, `order`, ...) continue to reduce to the same empty envelope because the `actions` fixture is empty; input handling now differs correctly per variant (restricted/order keys dropped, invalid `page`/`limit` rejected) even though the data is empty.
- Shared abstractions used: `core_query` (parse/valid_option/run), `core_auth` + `blog_handlers_middleware:ghost_permission` (permissions), `blog_handlers:mapping` (clean/wrap/pagination), context store (rows), no outbox. New-partition need: register `blog_handlers:mapping` centrally (parent-owned).
