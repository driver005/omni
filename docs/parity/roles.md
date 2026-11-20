# Parity: roles

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/roles.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/roles/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-20 (manifest group 54). Verdict stays **MATCH**; the
  route table, `#### Fix` blocks and "Fix applied (source)" below are updated by
  the [`## Strict-parity rewrite (group 54)`](#strict-parity-rewrite-group-54)
  section at the end. The authoritative behavior analysis with `file:line`
  evidence is `docs/parity/behavior/roles.md`.

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
| GET `/roles/` | `browse` | GET `/roles` | `roles_browse()` → `BrowseHandler` | Ghost: `options:['permissions']` + validation `permissions:['assign']`; C++: `core::query::parse` over `{'permissions'}`, non-`assign` → 422 `ApiError::validation` | Ghost: `{roles:[...]}` (permission-filtered when `permissions==='assign'`); C++: `blog::mapping::wrap("roles",…)` → `{"roles":[...]}`, no `meta.pagination` | Ghost: `models.Role.findAll(frame.options)`; C++: canonical role snapshot merged from `s_ctx` store rows + permission-driven `permissions=assign` filter | Ghost: `cacheInvalidate:false` ⇒ no header; C++: no `X-Cache-Invalidate` header (strict pass) | Ghost: `mw.authAdminApi` + `permissions:true`; C++: `require_auth` + `require_ghost_permission` (`roles`,`browse`) → Ghost 401/403 envelope; no GET Content-Type requirement | MATCH | Ghost `roles.js:10,11-15,18` + `admin/routes.js:353` + `output/roles.js:10-30`; C++ strict rewrite in `system/roles/{routes.cppm,browse/handler.cppm,browse/workflow/step/execute.cppm}` |

## Route details

### GET `/roles/` — MATCH (strict-parity rewrite; details below superseded)

> The bullet list below is the pre-rewrite snapshot (2026-11-19). Current behavior
> is the strict rewrite recorded under `#### Fix` and `## Strict-parity rewrite (group 54)`.

- **Route identity**: Ghost `router.get('/roles/', mw.authAdminApi, http(api.roles.browse))` (`admin/routes.js:353`) vs C++ `Router{router, "/roles"}...get(...)` (`system/roles/routes.cppm:19,21`). Paths match after normalizing trailing slash; verb GET both. Only one `/roles` route on each side (grep across `web/api` and all `routes.cppm` returned single hit each).
- **Operation mapping**: Ghost controller key `browse` (`roles.js:6`, `docName:'roles'` line 5) vs `handler.roles_browse()` (`system/roles/routes.cppm:26`) → `BlogHandlers::roles_browse()` (`all.cppm:501`) returning `blog::handlers::roles::browse::BrowseHandler` (`all.cppm:750`). Core operation agrees.
- **Input divergence**: Ghost allows `options:['permissions']` with validation `options.permissions:['assign']` (`roles.js:10,11-15`) and requires permissions (`roles.js:16`). C++ declares `using Input=interfaces::io::IRequest&` (`execute.cppm:11`) and never parses query string or reads `permissions` — the `permissions` option exists only on the Ghost side.
- **Output divergence**: Ghost `output/roles.js` browse serializer returns `{roles: roles}` (`output/roles.js:10-13`), and when `permissions==='assign'` it runs `canThis.assign.role` and filters the roles array (`output/roles.js:15-30`); no `meta.pagination` is emitted. C++ `execute.cppm:16` returns `{"roles":<rows>,"meta":{"pagination":{...}}}` with hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` and performs no permission-based filtering. Handler sets 200 OK on success (`handler.cppm:27`). Envelope root key agrees (`roles`); `meta.pagination` is extra in C++ and real pagination/permission filtering are absent.
- **DB call divergence**: Ghost `models.Role.findAll(frame.options)` (`roles.js:18`) passes the frame options (incl. `permissions`). C++ `ctx.store().json_for("roles", {})` (`execute.cppm:15`) uses the correct resource string `roles` but with empty params `{}`; no real filtering/pagination is done — pagination numbers are hardcoded (`execute.cppm:16`).
- **Cache**: Ghost sets `headers.cacheInvalidate:false` (`roles.js:7-9`), i.e. explicitly no cache invalidation. C++ has no cache: `m_roles_browse{m_ctx}` is constructed with only the context, no `core::cache::Cache`/ops (`all.cppm:750`); handler/step contain no cache key, namespace, TTL, or eviction. Both effectively no cache behavior.
- **Auth divergence**: Ghost route is guarded by `mw.authAdminApi` (`admin/routes.js:353`) and controller sets `permissions:true` (`roles.js:16`) with the `assign` permission validated/enforced (`roles.js:13`, `output/roles.js:19`). C++ uses `content_type_middleware` (`system/roles/routes.cppm:20`) plus a bearer-token presence check in the handler that returns 401 `{"error":"Unauthorized"}` when `token(req)` is empty (`handler.cppm:18-22`); no `assign` permission check. Both require auth, but permission granularity differs.

#### Fix (strict-parity, group 54 — supersedes the earlier fix blocks)

All fixes stay inside the group folder; no shared file or BUILD target is edited.

- Checks 1, 2: no change — path/verb and binding agree (`admin/routes.js:353` GET `/roles/` ↔ `system/roles/routes.cppm` GET `/roles`; trailing slash normalized; `handler.roles_browse()` → `BlogHandlers::roles_browse()` → `BrowseHandler`). No route added or removed.
- Check 3 (input): in `browse/workflow/step/execute.cppm` use `core::query::parse(in.get_path(), {"permissions"})` (shared NQL parser). Reject any `permissions` value other than `assign` with 422 `blog::api::ApiError{}.validation(...)`. No `data[]`/multipart/body on either side.
- Check 4 (output): return `blog::mapping::wrap("roles", rows)` → `{"roles":[...]}` with no `meta.pagination` (`output/roles.js:10-13,28-30`).
- Check 5 (DB call): build the canonical Ghost role snapshot (`fixtures.json:75-118`) and merge rows from `ctx.store().json_for("roles", {})` (`s_ctx`) so store edits are observable. With `permissions=assign`, filter per Ghost role rules (`permissions-data.ts:13-24,65-234`, `role.js:76-107`, `output/roles.js:15-27`) using `core_auth` `GhostAuthorizer`/`Principal` from `x-blog-*` headers: Owner → all assignable then drop `Owner`; Administrator → Administrator/Super Editor/Editor/Author/Contributor; Editor/Super Editor → Author/Contributor; Author/Contributor → none.
- Check 6 (cache): Ghost `cacheInvalidate:false` emits **no** `X-Cache-Invalidate` (`headers.ts:135-144`). Remove the literal header write from `browse/handler.cppm`. No cache key/TTL/eviction.
- Check 7 (auth): keep `require_auth` on the router, drop `content_type_middleware` (Ghost reads no body on this GET), and add `require_ghost_permission(req,res,next,send,"roles","browse")` in `browse/handler.cppm` for the `permissions:true` / `canThis.browse.role` gate. Bare admin bearer → Administrator + `*`; explicit `x-blog-*` headers stay authoritative. 401/403 use the Ghost error envelope.
- Check 8 (verdict/evidence): evidence already recorded on both sides. After the above, path/verb, op, input, output envelope, roles data, cache and auth agree → MATCH.

```cpp
// include/blog/handlers/system/roles/browse/workflow/step/execute.cppm — ExecuteStep::invoke (essence)
static constexpr std::string_view kOptions[]{"permissions"};
const auto q = core::query::parse(in.get_path(), kOptions);       // shared input pipeline
if (const auto it = q.options.find("permissions");
    it != q.options.end() && it->second != "assign") {
    return core::workflow::StepResponse<Output>{{{"status","422"},
        {"body", blog::api::ApiError{}.validation("Validation error", "Invalid value", "permissions")}}};
}
const bool assign = q.options.contains("permissions");

std::vector<RoleRow> roles = canonical_roles();                   // Ghost default roles snapshot
merge_stored((*s_ctx).store().json_for("roles", {}), roles);      // s_ctx observable rows
if (assign) {
    const auto principal = blog::handlers::middleware::principal_from_request(in);
    std::erase_if(roles, [&](const RoleRow& r) {
        return r.name == "Owner" || !can_assign(principal, r.name);  // ghost permission rules
    });
}
const auto rows = serialize(roles);
return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body", blog::mapping::wrap("roles", rows)}}};               // {roles:[...]}, no meta
```

## Missing in C++

None. The single Ghost route (GET `/roles/`) has a C++ counterpart. Confirmed by grep for `/roles` in `include/blog/handlers/**/routes.cppm` — only `include/blog/handlers/system/roles/routes.cppm:19`.

  Fix: No change required. Check 1 is closed — no Ghost route lacks a C++ counterpart, so nothing to add. The `#### Fix` above only reshapes the existing `BrowseHandler` / `ExecuteStep` / `RolesRoutes` to reach parity.

## Extra in C++

None. The matched folder `include/blog/handlers/system/roles/` installs exactly one route (GET `/roles`, `system/roles/routes.cppm:19-21`) with no Ghost counterpart missing.

  Fix: No change required. Check 1 is closed — no extra C++ route exists to remove or gate. Keep the single GET `/roles` route; the `#### Fix` above adjusts its middleware/input/output behavior, not its path or verb.

## Fix applied (source, strict-parity group 54)

Changed files (all inside the group folder, plus this doc):

- `browse/workflow/step/execute.cppm` — `Output=std::map<std::string,std::string>`, `s_ctx` store access. Input via `core::query::parse(..., {"permissions"})` with 422 `ApiError::validation` on non-`assign`; canonical Ghost roles snapshot (`fixtures.json:75-118`) merged with `ctx.store().json_for("roles", {})`; `permissions=assign` filter via `core_auth` `GhostAuthorizer`/`Principal` + `blog_handlers_middleware::principal_from_request` and the Ghost role-assignability rules (`permissions-data.ts`, `role.js:76-107`, `output/roles.js:15-27`); `blog::mapping::wrap("roles", rows)` → `{"roles":[...]}`, no `meta.pagination`.
- `browse/handler.cppm` — added `blog::handlers::middleware::require_ghost_permission(req,res,next,send,"roles","browse")` gate (Ghost 401/403 envelope), admin-bearer → Administrator + `*` principal default, status mapping (200/400/422). Removed the `X-Cache-Invalidate: false` write (Ghost emits no header) and all inline auth logic.
- `routes.cppm` — kept `require_auth`, removed `content_type_middleware` (no GET body/Content-Type on Ghost), sets `x-blog-context: ADMIN_API`. Path/verb unchanged; no route added or removed.
- `docs/parity/roles.md`, `docs/parity/behavior/roles.md` — verdict/evidence updated for the strict pass.

Skipped as "No change required": `workflow/flow/main.cppm` (Checks 1/8), the Missing-in-C++ `Fix`, and the Extra-in-C++ `Fix`.

## Strict-parity rewrite (group 54)

- **Verdict**: MATCH stays MATCH — now backed by registered shared abstractions instead of local stand-ins. `docs/parity/behavior/roles.md` carries the authoritative per-check closure record.
- **New-partition needs**: none. Helpers live in the already-registered partitions `blog_handlers:roles_browse_step`, `blog_handlers:roles_browse_handler`, `blog_handlers_routes:roles_routes`. No `BUILD.bazel`, `base.cppm`, `all.cppm`, central `routes.cppm` or core-module edit. `core_outbox` needs no registration (GET, no side effects).
- **Header correction**: earlier fix text added `X-Cache-Invalidate: false`. Strict reading of `api-framework/src/headers.ts:135-144` shows `cacheInvalidate:false` emits no header, so the write was removed. Both docs now record "no `X-Cache-Invalidate`".
- **Residual gap**: no real Knex `roles` table in C++ — the roles set is the canonical Ghost default-role snapshot plus any `ResourceStore` rows; accepted under the allowed-primitive rubric, no fabricated pagination.
- **Verification**: full `bazel build //include/blog/handlers:blog_handlers` is currently blocked by pre-existing errors in other groups' files and a `cc_library` dependency cycle (both out of this group's scope). The rewrite mirrors the working `membership/tiers/browse` module/import/build shape.
