# Parity: featurebase

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/featurebase.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/featurebase/routes.cppm` (found)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 24)

Ghost controller exports one key: `token` (`featurebase.js:32`). Only admin route
referencing it is `GET /featurebase/token` (`routes.js:414`). Note: `routes.js:24`
`GET /config/featurebase` → `api.config.featurebase` belongs to the `config`
controller, not this exported key, so it is out of scope for this file.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/featurebase/token` | `token` (`query`, `featurebase.js:37-52`) | GET `/featurebase/token` (`routes.cppm:19-37`) | `featurebase_featurebase_token_get` → `FeaturebaseTokenGetHandler::handle` (`handler.cppm:16-34`) | Ghost: `frame.user` name/email/profile_image + config `featurebase:enabled`/`featurebase:jwtSecret`; throws `ValidationError('Featurebase is not configured')` if unset (`featurebase.js:38-45`). C++: env `FEATUREBASE__ENABLED`/`FEATUREBASE__JWTSECRET` with context-local config/settings overrides; authenticated subject resolved to a store user row with header fallback (`execute.cppm:190-231`). | Ghost: `{featurebase:{token}}` — HS256 JWT, `expiresIn:'7d'` (`featurebase.js:22-25,49-51`, `output/featurebase.js:7-9`); `headers.cacheInvalidate:false`, admin app `Cache-Control: private`. C++: status 200 body `{"featurebase":{"token":"<jwt>"}}`, `Cache-Control` set, no `X-Cache-Invalidate` (`execute.cppm:270-274`, `handler.cppm:29-33`). | Ghost: none — JWT sign from config/settingsCache/urlUtils. C++: no model/resource mutation; a `users` read resolves the authenticated subject. | none on either side; Ghost `cacheInvalidate:false`, C++ no invalidation header. | Ghost: `mw.authAdminApi`, `permissions:false`. C++: `require_auth` only; no permission gate (permissions:false agrees), but no signature verification or integration-token block. | PARTIAL | Ghost: `routes.js:414`, `featurebase.js:32-53`, `output/featurebase.js:4-10`; C++: `external/featurebase/routes.cppm:19-37`, `featurebase_token_get/handler.cppm:16-34`, `workflow/step/execute.cppm:208-286`. |

## Route details

### GET `/featurebase/token` — PARTIAL

- Route identity: MATCH. Ghost `GET /featurebase/token` (`routes.js:414`) ↔ `Router{router,"/featurebase"}.Route{"/token"}.get(...)` (`external/featurebase/routes.cppm:19-37`, installed at `include/blog/handlers/routes.cppm:21,93`).
- Operation: MATCH. Ghost `token.query` generates a Featurebase SSO JWT via `generateJWTToken` (`featurebase.js:7-26,37-52`); C++ builds an equivalent HS256 JWT in `ExecuteStep` (`execute.cppm:253-274`).
- Input parity: MATCH (behavioral). Ghost consumes authenticated user fields and requires `featurebase:enabled` + `featurebase:jwtSecret`, throwing `ValidationError('Featurebase is not configured')` when absent. C++ reads the nconf-style env keys (context-local overrides for the harness), resolves the user from the authenticated subject's store row, and returns the Ghost 422 message on a missing/falsy guard (`execute.cppm:190-231`). The live `frame.user` model and `settingsCache`/`urlUtils` objects are unavailable in this module — recorded as residual gaps in `docs/parity/behavior/featurebase.md`.
- Output parity: MATCH. Both emit `{"featurebase":{"token":"<jwt>"}}`. C++ no longer emits a bare `{"token":...}` (`execute.cppm:282-286`).
- DB call parity: MATCH for wire behavior. Neither mutates a model. C++ reads `users` only to resolve the authenticated subject; no generic `featurebase` resource read/pagination remains.
- Cache parity: MATCH. Ghost sets `cacheInvalidate:false` and forces app-level `Cache-Control: private`; C++ writes the same `Cache-Control` value and no `X-Cache-Invalidate` (`handler.cppm:29-33`).
- Auth/permissions: PARTIAL. Ghost uses `mw.authAdminApi` with `permissions:false` and blocks integration tokens (`featurebase` not allowlisted). C++ uses `require_auth` only; no permission gate (correct for `permissions:false`), but no signature verification, user lookup from bearer, or integration-token block. Ghost's app accepts a bodyless GET without `Content-Type`/`Accept`; the C++ 415/406 `content_type_middleware` gate was removed.

#### Fix (original checks 1–8)

Applied by group 22 and retained: config validation + 422, `{"token":...}` JWT output (now wrapped in the `featurebase` envelope by group 24), no model/resource call, no cache key, `require_auth` on the router, and verdict MISMATCH→PARTIAL. See **Applied fixes (group 22)** and **Strict-parity rewrite (group 24)** below.

## Missing in C++

None. The single Ghost route referencing the `token` key (`GET /featurebase/token`,
`routes.js:414`) has a C++ route/handler counterpart.

#### Fix

No change required. Check 8 (MISSING) is closed. (`GET /config/featurebase`, `routes.js:24`, belongs to the `config` controller.)

## Extra in C++

None within the matched folder `include/blog/handlers/external/featurebase/`: it installs
only `GET /featurebase/token` (`routes.cppm:19-37`), matching the Ghost route. The C++
`GET /config/featurebase` route (`include/blog/handlers/system/config/routes.cppm:30-38`)
maps to the Ghost `config` controller (`routes.js:24`), so it is not an extra for `featurebase`.

#### Fix

No change required. Check 8 (EXTRA) is closed.

## Applied fixes (group 22)

Edited only `external/featurebase/**` (routes, handler, step; flow unchanged) plus
this doc:

- `featurebase_token_get/workflow/step/execute.cppm`: removed the generic
  `ctx.store().json_for("featurebase", {})` read and the hardcoded pagination envelope.
  Added config validation, user fields, settings/URL fallbacks, and an HS256 JWT
  `{"token": <jwt>}` using `core_serde::JsonCodec` and a std-only SHA-256 /
  HMAC-SHA256 / base64url helper. `Output` is `std::map<std::string,std::string>`.
- `featurebase_token_get/handler.cppm`: status/body now come from the step map (422
  propagates); `X-Cache-Invalidate: false` stand-in added (later removed by group 24).
- `routes.cppm`: added `require_auth` to the `/featurebase` router.
- `workflow/flow/main.cppm`: unchanged — still registers `ExecuteStep`.

Checks closed: 3 (config validation + user fields), 4 (JWT output), 5 (no model/resource call),
6 (cache header), 7 (`require_auth`). Check 8: MISMATCH → PARTIAL.

## Strict-parity rewrite (group 24, 2026-11-20)

Edited only `include/blog/handlers/external/featurebase/**` and the two featurebase
docs. No shared file, new partition, or BUILD dependency was added.

- `workflow/step/execute.cppm`:
  - Output wrapped in Ghost's serializer envelope: `{"featurebase":{"token":...}}`
    (`output/featurebase.js:7-9`).
  - 422 body uses `ApiError{}.validation("Featurebase is not configured", "", "")`
    so the wire `ValidationError` carries Ghost's message and no `property`/`context`
    override.
  - Config read from nconf-style env keys (`FEATUREBASE__ENABLED`,
    `FEATUREBASE__JWTSECRET`) with `config`/`settings` context-local overrides;
    `URL`/config `url` feed `companies[0].website`; `truthy()` mirrors nconf
    `parseValues` (`loader.ts:35-42`).
  - User fields resolved from the authenticated subject (`x-blog-user-id` or admin
    bearer → id `1`, the `admin/users` convention) against `ctx.store().json_for("users", {})`,
    with `x-blog-user-*` headers only as fallback.
  - `Output` stays `std::map<std::string,std::string>` (`status`/`body`); static
    `s_ctx` retained.
- `handler.cppm`: removed `X-Cache-Invalidate: false`; now writes the admin-app
  `Cache-Control` value through `interfaces::io::types::Token::CACHE_CONTROL`. The
  422 status from the step still propagates.
- `routes.cppm`: removed `content_type_middleware` (no 415/406 on a bodyless GET);
  kept `require_auth`. `require_ghost_permission` is intentionally absent because
  Ghost's op has `permissions:false`.
- `workflow/flow/main.cppm`: unchanged.

Shared abstractions used: `core_serde::JsonCodec`, `blog::api::{ApiError,RecordDto,parse_array}`
from the registered `:domain_all` partition, and `blog_handlers_middleware::require_auth`.

New shared needs recorded in `docs/parity/behavior/featurebase.md` (not implemented here):
a secret-taking HS256 signer in `core_auth`, a `//include/core/adapters:core_adapters`
dependency so `env_first`/`from_env` can replace the local env helper, and a registered
admin-API auth partition for `authenticateAdminApi`/`tokenPermissionCheck` with signature
verification.

Verdict: PARTIAL unchanged — the full Ghost admin auth chain is still unavailable, so
no verdict was flipped.

Build verification: `bazel build //include/blog/handlers:blog_handlers` is blocked by a
pre-existing Bazel dependency cycle in that target, unrelated to these files. No shared
BUILD, `all.cppm`, `base.cppm`, central route file or middleware file was modified.
