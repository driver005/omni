# Parity: identities

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/identities.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/auth/identities/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-19 (manifest group 12)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

The earlier `MATCH` verdict was premature: the route still diverged on the
response envelope, `X-Cache-Invalidate`, `Cache-Control`, the Content-Type gate
and permissions. The strict-parity pass closed every divergence that has an
in-folder C++ primitive; the verdict is corrected to `PARTIAL` because real
RS256 JWT signing, a `User`/`roles` relation load, real token verification and
the `/ghost/api/admin` mount prefix remain without one. Verdicts flip only when
fully achieved.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/identities` | `read` (docName `identities`) | GET `/identities` | `handler.identities_read()` | Ghost: no query/data params; uses `frame.user` (`email`, `roles`). C++: `using Input = interfaces::io::IRequest&`; `x-blog-identity-email` / `x-blog-identity-role` with authenticated-principal fallback (`middleware::token`, `x-blog-role`). | Ghost: `{ identities: [ { token } ] }` (`default.all`); RS256 JWT, 5m; `Cache-Control: no-cache, private, no-store, ...`; no `X-Cache-Invalidate`. C++: `{"identities":[{"token":"<claims JSON>"}]}`, status 200, full admin `Cache-Control`, no `X-Cache-Invalidate`. | Ghost: `IdentityTokenService.instance.getTokenForUser` (no model/store read in controller). C++: no store read — `core_serde::JsonCodec{}.encode` over Ghost `{sub, role?}` claims; serde-encoded, not signed. | Ghost: `cacheInvalidate:false` → no invalidation. C++: no header, no `core::cache::Cache` key/TTL. | Ghost: `mw.authAdminApi` + `permissions:true` + `docName:'identities'` (`canThis().read.identity`). C++: admin-token→Administrator mapping + shared `core::auth::ghost` `require_ghost_permission(...,"identities","read")` with Ghost 401/403 envelopes. | PARTIAL | Ghost `routes.js:437`, `identities.js:4-27`, `identity-token-service.ts:16-32`, `output/default.js:22-41`. C++ `auth/identities/routes.cppm:23-62`, `browse/handler.cppm:16-27`, `browse/workflow/step/execute.cppm:24-57`. |

## Route details

### GET /identities — PARTIAL

- **Route identity — agree.** Ghost `router.get('/identities', ...)` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:437`. C++ `Router{router, "/identities"}...get(...)` at `include/blog/handlers/auth/identities/routes.cppm:53-62`. Route unchanged; the flat-router bare mount is retained (full Ghost prefix is a central concern, out of scope).
- **Operation mapping — aligned.** Ghost key is `read` (docName `identities`) at `identities.js:5-6`. The binding calls `handler.identities_read()` (`routes.cppm:60`), a read-named alias returning the existing `BrowseHandler` (workflow name `"identities_browse"`). The `identities_browse()` accessor is retained for compatibility (`all.cppm:582-583`); no handler deleted.
- **Input — aligned within allowed primitives.** Ghost reads `frame.user.get('email')` and the first role from `frame.user.load(['roles'])` (`identities.js:16-17,23`). C++ `ExecuteStep::invoke` reads `x-blog-identity-email` / `x-blog-identity-role`, falling back to the authenticated bearer subject (`middleware::token`) and `x-blog-role` (`execute.cppm:31-34`). No User model / role-relation load available — accepted residual gap.
- **Output — aligned.** Ghost returns `{ identities: [ { token } ] }` via `default.all` (`output/default.js:32-36`). C++ now builds exactly `{"identities":[{"token":"<claims JSON>"}]}` with `core_serde` and no `meta` (`execute.cppm:50-57`), status 200 (`handler.cppm:26`). The former flat `{"token":...}` envelope was removed.
- **DB call — aligned in shape.** Ghost calls `IdentityTokenService.instance.getTokenForUser(email, role)` (`identities.js:12,22-25`) — no model/store read in controller. C++ has no `ctx.store()` call; the token claims are built as Ghost `{sub, role?}` and encoded with `core_serde::JsonCodec` (`execute.cppm:38-45`). Serde-encoded, **not cryptographically signed** — accepted residual gap.
- **Cache — aligned.** Ghost `headers.cacheInvalidate:false` emits no `X-Cache-Invalidate` (`identities.js:7-9`, `headers.ts:135-144`) and the admin app sets `Cache-Control: no-cache, private, no-store, ...` (`admin/app.js:24`). C++ writes no `X-Cache-Invalidate` and sets the full admin `Cache-Control` (`handler.cppm:23`). No `core::cache::Cache` key/namespace/TTL/eviction introduced.
- **Auth — both gate the route.** Ghost `mw.authAdminApi` (`routes.js:437`) plus `permissions:true`/`docName:'identities'` (`identities.js:5,10`). C++ `identities_routes_detail::require_permission` (`routes.cppm:23-42`) maps a presented admin token to Administrator/`*`, then calls the shared `require_ghost_permission(...,"identities","read")`; denials produce Ghost 401/403 envelopes. The router-level `content_type_middleware`, `bearer_middleware`, bare `require_auth` and the inline token check were removed. Real token verification has no primitive here — accepted residual gap.

#### Strict-parity pass (2026-11-19)

Target files: `include/blog/handlers/auth/identities/routes.cppm` (`identities_routes_detail::require_permission`, `IdentitiesRoutes::install`), `browse/handler.cppm` (`BrowseHandler::handle`), `browse/workflow/step/execute.cppm` (`ExecuteStep::invoke`). Shared surfaces: `core::auth::ghost`, `blog::handlers::middleware::require_ghost_permission` + header constants + `token()`, `core_serde::JsonCodec` / `serde::Value`. `Output = std::string` and the static `s_ctx` are retained; the flow file is unchanged.

Exact changes (all earlier divergences closed):
- **Envelope:** flat `{"token":...}` replaced with Ghost `{"identities":[{"token":...}]}` (no `meta`).
- **Claims:** C++-named `{email,role,sub}` replaced with Ghost `{sub, role?}`; role omitted when empty.
- **X-Cache-Invalidate:** stand-in `false` marker removed — Ghost emits no header for `cacheInvalidate:false`.
- **Cache-Control:** full admin `no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0` added.
- **Content-Type gate:** router-level `content_type_middleware` removed (Ghost has no such gate on this GET).
- **Auth:** bare bearer/require_auth and inline token check replaced by admin-token→Administrator mapping + shared `GhostAuthorizer` permission check for `identities`/`read`, with Ghost 401/403 envelopes.

Central registrations: none. All four touched files are already registered partitions in `include/blog/handlers/BUILD.bazel` (`:681-683`, `:986`), and `identities_read()` already exists on `blog::api::BlogHandlers` (`all.cppm:583`). No shared file edited.

New-partition needs (recorded, not created): a real identity-token partition (e.g. `core_auth:identity_tokens`) owning an RS256 signer, signing-key provider, admin issuer URL and 5m expiry; and a shared User+roles / admin-token verifier. Until those exist, the claims are serde-encoded and the admin identity is presence-mapped.

Residual gaps (verdict stays PARTIAL): unsigned serde claims instead of a JWT; presence-based token→Administrator mapping (no real token verification / `updateUserLastSeen`); no `User`/`roles` relation load; bare `/identities` mount; shared `permission_message()` normalizes `read`→`browse` in the 403 wording ("…browse identities" vs Ghost's "…read identities"). Verdicts flip only when fully achieved.

### Applied (2026-11-19)

In-scope source edits (folder only):
- `routes.cppm` — `.use(content_type_middleware)`, `.use(bearer_middleware)` and `.use(require_auth)` removed; new `identities_routes_detail::require_permission` middleware maps the admin token to Administrator and calls shared `require_ghost_permission(...,"identities","read")`; binding still calls `handler.identities_read()`.
- `browse/handler.cppm` — no `X-Cache-Invalidate`; full admin `Cache-Control` added; `Status::OK` unchanged; no inline token check.
- `browse/workflow/step/execute.cppm` — no store read; Ghost `{sub, role?}` claims via `serde::Value`/`core_serde`; `{"identities":[{"token":...}]}` envelope; `Output = std::string` and `s_ctx` retained.
- `browse/workflow/flow/main.cppm` — untouched (single-step `"identities_browse"` flow retained).

Bazel verification is blocked by the pre-existing `blog_handlers` ↔ `blog_handlers_routes` dependency cycle (out of scope); code was reviewed against the established strict-pass patterns in `admin/labels` and `admin/notifications`.

## Missing in C++

None. The single Ghost route `GET /identities` (`routes.js:437`) has a C++ counterpart at `auth/identities/routes.cppm:53-62`.

  Fix: No change required. Check 8 (MISSING) closed — route + verb already present. The strict pass only reshapes the existing `BrowseHandler` / `ExecuteStep` / `IdentitiesRoutes` (admin-principal mapping + permission check, Ghost claims/envelope, no store read, cache headers). No new global state.

## Extra in C++

None. The matched C++ folder `include/blog/handlers/auth/identities/` exposes one route (`GET /identities`, `routes.cppm:53-62`), which maps to a Ghost controller route. No unmatched C++ routes found.

  Fix: No change required. Check 8 (EXTRA) closed — `IdentitiesRoutes::install` (`routes.cppm:49-64`) installs only the single `.get(...)` counterpart of `api.identities.read` (`routes.js:437`); nothing to delete or gate. Keep the `all.cppm:583` accessor; the strict pass changes handler/step behavior only. No new global state.
