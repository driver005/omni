# Parity: identities

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/identities.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/auth/identities/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/identities` | `read` (docName `identities`) | GET `/identities` | `handler.identities_browse()` | Ghost: no query/data params; uses `frame.user` (`email`, `roles`). C++: `using Input = interfaces::io::IRequest&`; step ignores request, reads no params. | Ghost: `{ token }` from `IdentityTokenService.getTokenForUser(email, role)`; `headers.cacheInvalidate=false`. C++: `{"identities":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}`; status 200. | Ghost: service call `IdentityTokenService.instance.getTokenForUser` (no model/store read in controller). C++: `ctx.store().json_for("identities", {})` — real resource read, empty params, no filtering/pagination. | Ghost: `cacheInvalidate:false` (explicit no cache invalidation header). C++: none (no `core::cache::Cache`, no namespace/TTL). | Ghost: `mw.authAdminApi` + `permissions:true` + `docName:'identities'`. C++: `content_type_middleware` + manual `token(req).empty()` → 401 `{"error":"Unauthorized"}`; no permissions system. | MISMATCH | Ghost `routes.js:437`, `identities.js:4-27`. C++ `auth/identities/routes.cppm:19-28`, `browse/handler.cppm:18-27`, `browse/workflow/step/execute.cppm:11-17`. |

## Route details

### GET /identities — MISMATCH

- **Route identity — agree.** Ghost `router.get('/identities', ...)` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:437`. C++ `Router{router, "/identities"}...get(...)` at `include/blog/handlers/auth/identities/routes.cppm:19-21`.
- **Operation mapping — diverges.** Ghost exporter key is `read` (docName `identities`) at `identities.js:5-6`, bound via `http(api.identities.read)` at `routes.js:437`. C++ binds `handler.identities_browse()` at `auth/identities/routes.cppm:26` (handler class `BrowseHandler`, workflow name `"identities_browse"` at `browse/handler.cppm:23` and `browse/workflow/flow/main.cppm:9`). Ghost `read` fetches an identity token for the current user; C++ `browse` fetches a resource list. Materially different operations behind the same URL.
- **Input — diverges.** Ghost controller takes no `options[]`/`data[]`; it reads `frame.user`, loads `['roles']`, and extracts role name (`identities.js:16-17`) plus `frame.user.get('email')` (`identities.js:23`). C++ `using Input = interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:11`); the step ignores `in` entirely — no query-string or body parsing. The authenticated-user identity that Ghost depends on is absent on the C++ side beyond a bearer-token presence check.
- **Output — diverges.** Ghost returns `{ token }` (`identities.js:27`) with `headers.cacheInvalidate:false` (`identities.js:7-9`). C++ returns a hardcoded pagination envelope `{"identities":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` (`browse/workflow/step/execute.cppm:17`) and sets status 200 (`browse/handler.cppm:27`). No `token` field on the C++ side; no `identities` list on the Ghost side. `limit/page/pages/total` are hardcoded, not derived.
- **DB call — diverges.** Ghost calls `IdentityTokenService.instance.getTokenForUser(email, role)` (`identities.js:12,22-25`) — a service call producing a signed token, no direct model/store call in the controller. C++ calls `ctx.store().json_for("identities", {})` (`browse/workflow/step/execute.cppm:16`) against resource string `"identities"` with empty params; no token generation, no filtering or pagination performed.
- **Cache — differs in intent.** Ghost explicitly sets `headers.cacheInvalidate:false` (`identities.js:8`). C++ has no cache behavior — no `core::cache::Cache` use, no namespace, key, TTL, or eviction in `routes.cppm`, `handler.cppm`, or `execute.cppm`.
- **Auth — both gate the route, differently.** Ghost: `mw.authAdminApi` at `routes.js:437` plus `permissions:true` / `docName:'identities'` at `identities.js:5,10`. C++: `content_type_middleware` at `auth/identities/routes.cppm:20`, then a manual bearer-token presence check in `browse/handler.cppm:18-22` that returns 401 `{"error":"Unauthorized"}` when empty; no `permissions.method`/`docName` equivalent.

## Missing in C++

None. The single Ghost route `GET /identities` (`routes.js:437`) has a C++ counterpart at `auth/identities/routes.cppm:19-28`.

## Extra in C++

None. The matched C++ folder `include/blog/handlers/auth/identities/` exposes one route (`GET /identities`, `routes.cppm:19-28`), which maps to a Ghost controller route. No unmatched C++ routes found.
