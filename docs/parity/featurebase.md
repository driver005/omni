# Parity: featurebase

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/featurebase.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/featurebase/routes.cppm` (found)
- Checked: 2026-11-19

Ghost controller exports one key: `token` (`featurebase.js:32`). Only admin route
referencing it is `GET /featurebase/token` (`routes.js:414`). Note: `routes.js:24`
`GET /config/featurebase` → `api.config.featurebase` belongs to the `config`
controller, not this exported key, so it is out of scope for this file.

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
| GET `/featurebase/token` | `token` (`query`, `featurebase.js:37-52`) | GET `/featurebase/token` (`routes.cppm:19-29`) | `featurebase_featurebase_token_get` → `FeaturebaseTokenGetHandler::handle` (`handler.cppm:17-28`) | Ghost: `frame.user` name/email/profile_image + `config featurebase:enabled`/`featurebase:jwtSecret`; throws `ValidationError` if unset (`featurebase.js:38-45`, `generateJWTToken:7-26`). C++: no input parsing (`using Input=IRequest&`, `execute.cppm:11`), no enabled/secret check. | Ghost: `{token}` — HS256 JWT, `expiresIn:'7d'` (`featurebase.js:22-25,49-51`); `headers.cacheInvalidate:false` (`featurebase.js:33-35`). C++: status 200 body `{"featurebase":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` (`execute.cppm:17`, `handler.cppm:27`). | Ghost: none — JWT sign from config/settingsCache/urlUtils (`featurebase.js:15-25`). C++: `ctx.store().json_for("featurebase", {})` generic resource read, no filtering (`execute.cppm:14,16`); falls through to `m_resources.list("featurebase")` (`context.cppm:1180-1181`) — `"featurebase"` is not a known model. | none (Ghost sets `cacheInvalidate:false`; no cache in C++ step/handler) | Ghost: `mw.authAdminApi` (`routes.js:414`), `permissions:false` (`featurebase.js:36`). C++: `content_type_middleware` (`routes.cppm:20`) + manual bearer check, 401 `{"error":"Unauthorized"}` if token empty (`handler.cppm:18-22`, `bearer.cppm:27-32`). | MISMATCH | Ghost: `routes.js:414`, `featurebase.js:32-53`. C++: `external/featurebase/routes.cppm:19-29`, `featurebase_token_get/handler.cppm:17-28`, `workflow/step/execute.cppm:13-17`, registry `all.cppm:522,771`, install `routes.cppm:89`. |

## Route details

### GET `/featurebase/token` — MISMATCH

- Route identity: MATCH. Ghost `GET /featurebase/token` (`routes.js:414`) vs C++ `Router{router,"/featurebase"}.Route{"/token"}.get(...)` (`external/featurebase/routes.cppm:19-29`); central install at `handlers/routes.cppm:89`.
- Operation: MISMATCH. Ghost `token.query` generates a Featurebase SSO JWT via `generateJWTToken` (`featurebase.js:7-26,37-52`). C++ runs a generic workflow `featurebase_featurebase_token_get` that just reads a resource (`handler.cppm:23-27`, `workflow/step/execute.cppm:13-17`) — no JWT generation anywhere.
- Input parity: Ghost consumes authenticated user fields and requires config `featurebase:enabled` + `featurebase:jwtSecret`, throwing `ValidationError('Featurebase is not configured')` when absent (`featurebase.js:38-45`). C++ parses no query/body and performs no config validation (`execute.cppm:11,13-16`). Params present only on Ghost side: `user`, `featurebase:enabled`, `featurebase:jwtSecret`.
- Output parity: MISMATCH. Ghost returns a bare `{token}` JWT with `cacheInvalidate:false` (`featurebase.js:22-25,33-35,49-51`). C++ returns a hardcoded paginated list envelope `{"featurebase":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` with literal values (`execute.cppm:17`). No token field; data shape materially different.
- DB call parity: Ghost makes no model/service call — it signs a token from config, settingsCache `site_uuid`/`title` and urlUtils home URL (`featurebase.js:9-25`). C++ calls `ctx.store().json_for("featurebase", {})` (`execute.cppm:16`); `"featurebase"` is not handled by any known-model branch in `ApiDataStore::json_for` (`context.cppm:1128-1179`) and falls through to `m_resources.list("featurebase")` with empty params (`context.cppm:1180-1181`). No real filtering/pagination.
- Cache parity: none on either side. Ghost explicitly disables cache invalidation (`cacheInvalidate:false`, `featurebase.js:34`); C++ has no `core::cache::Cache` use in handler/step and no featurebase cache namespace.
- Auth/permissions: Ghost uses `mw.authAdminApi` with `permissions:false` (`routes.js:414`, `featurebase.js:36`). C++ uses `content_type_middleware` (`routes.cppm:20`) plus a manual check that returns 401 `{"error":"Unauthorized"}` when the bearer token is empty (`handler.cppm:18-22`, `token()` at `bearer.cppm:27-32`). Auth mechanism differs (admin-API middleware vs raw bearer-presence check).

## Missing in C++

None. The single Ghost route referencing the `token` key (`GET /featurebase/token`,
`routes.js:414`) has a C++ route/handler counterpart.

## Extra in C++

None within the matched folder `include/blog/handlers/external/featurebase/`: it installs
only `GET /featurebase/token` (`routes.cppm:19-31`), matching the Ghost route. The C++
`GET /config/featurebase` route (`include/blog/handlers/system/config/routes.cppm:30-38`)
maps to the Ghost `config` controller (`routes.js:24`), not this controller, so it is not
an extra for `featurebase`.
