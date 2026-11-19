# Parity: config

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/config.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/config/routes.cppm` (found)
- Checked: 2026-11-19

Ghost controller exports one operation key: `read` (`config.js:7-15`; `docName: 'config'`
at `config.js:5`). The only route referencing that key is `GET /config` in the
admin routes (`routes.js:23`). `GET /config/featurebase` (`routes.js:24`) references
`api.config.featurebase`, which is **not** exported by this controller (it is
served by the separate `featurebase` controller / `api.featurebase`), so it is out
of scope for this file — see *Extra in C++* for a note on its C++ route.

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
| GET `/config` | `read` (`query` returns `publicConfig.config`, `config.js:12-14`) | GET `/config` (`system/config/routes.cppm:19-27`) | `config_browse` → `BrowseHandler::handle` (`all.cppm:375,624`; `browse/handler.cppm:17-28`) | Both none. Ghost `read` takes no `options`/`data`/params (`config.js:7-15`). C++ `using Input=IRequest&`, no query/body parsing (`browse/.../execute.cppm:12-14`). | Ghost: `{"config": <19 picked keys>}`, no `meta`, `cacheInvalidate:false` (`output/config.js:8-33`, `config.js:8-10`). C++: status 200 body `encode(ConfigEnvelope{})` = `{"config":{...}}` with 16 hardcoded fields, no `meta` on reachable path (`execute.cppm:16-19`; `dto/config_response.cppm:48-64,110-124`; `handler.cppm:27`). Missing vs Ghost: `hostSettings`, `pintura`, `stats`, `featurebase`; values hardcoded (`version:"5.0.0"`, `database:"mysql8"`, `environment:"testing-mysql"`). | Ghost: no model/service call — reads config/settingsCache/labs in memory (`utils/public-config/config.js:51-92`). C++: effective path uses DTO defaults only; `ctx.store().json_for("config", {})` exists but is dead code (`execute.cppm:20`), and `"config"` is not a known resource → falls through to `m_resources.list("config")` (`context.cppm:1128-1181`). | none (Ghost sets `headers.cacheInvalidate:false`, `config.js:8-10`; no `core::cache::Cache` use in C++ handler/step) | Ghost: `mw.authAdminApi` (`routes.js:23`), `permissions:false` (`config.js:11`). C++: `content_type_middleware` (`routes.cppm:20`) + manual non-empty token check → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; `bearer.cppm:27-32`). No real token validation. | PARTIAL | Ghost: `config.js:5-16`, `admin/routes.js:23`, `output/config.js:4-35`, `public-config/config.js:51-92`. C++: `system/config/routes.cppm:19-27`, `handlers/routes.cppm:10,83`, `all.cppm:29,375,624`, `browse/handler.cppm:12-28`, `browse/.../execute.cppm:12-21`, `browse/.../flow/main.cppm:8-10`, `dto/config_response.cppm:12-124`. |

## Route details

### GET `/config` — PARTIAL

- Route identity: MATCH. Ghost `router.get('/config', mw.authAdminApi, http(api.config.read))`
  (`admin/routes.js:23`) vs C++ `Router{router, "/config"}.get(...)` with `content_type_middleware`
  (`system/config/routes.cppm:19-27`); central install at `include/blog/handlers/routes.cppm:10,83`.
  Path and verb normalize identically (`/config`, GET).
- Operation mapping: PARTIAL (naming only). Ghost operation key is `read` (`config.js:7`);
  C++ binds `handler.config_browse()` (`routes.cppm:26`) — registry accessor
  `config_browse()` at `all.cppm:375`, member `m_config_browse{m_ctx}` at `all.cppm:624`.
  Both perform a single read of the full config object; no functional divergence, but
  the method name is `browse`, not `read`.
- Input parity: no differences. Neither side parses params. Ghost `read.query()` takes
  no arguments (`config.js:12-14`); C++ declares `using Input = interfaces::io::IRequest&`
  and ignores it (`browse/.../execute.cppm:12-14`). No `options`, `data`, `validation`,
  `include`, `formats`, `source`, `order`, `statusCode`, or multipart fields on either side.
- Output parity: PARTIAL. Envelope matches — both produce `{"config": {...}}` with no
  `meta` on the reachable path (`output/config.js:31-33`; `dto/config_response.cppm:110-124`,
  `handler.cppm:27`). Divergences: (1) Ghost picks 19 keys including `hostSettings`,
  `pintura`, `stats`, `featurebase`, `exploreTestimonialsUrl` (`output/config.js:8-29`);
  C++ `ConfigResponse` serializes 16 fields and lacks `hostSettings`, `pintura`, `stats`,
  `featurebase` (`dto/config_response.cppm:48-64,84-101`). (2) C++ values are hardcoded
  fixture defaults — `version "5.0.0"`, `database "mysql8"`, `environment "testing-mysql"`,
  `exploreTestimonialsUrl "https://testing.com/..."` (`dto/config_response.cppm:50-60`) —
  whereas Ghost derives live values from config/settingsCache/labs/utils
  (`public-config/config.js:51-92`). (3) An unreachable branch would append
  `meta.pagination` with literal `limit:15,page:1,pages:1,total:0` (`execute.cppm:20-21`);
  it never runs because the lambda always returns `{200, ...}` so `r.first != 0` is
  always true (`execute.cppm:16-19`).
- DB call parity: PARTIAL. Ghost makes no model/service call — `publicConfig.config`
  is computed from `config.get(...)`, `settingsCache`, `labs` and `databaseInfo`
  (`public-config/config.js:2-6,51-92`). C++ has no reachable DB call; the only store
  access is the dead `ctx.store().json_for("config", {})` (`execute.cppm:20`), and
  even if reached, `"config"` is not in the known-resource list and falls through to
  `m_resources.list("config")` with empty params (`context.cppm:1128-1182`). No real
  filtering or pagination on either side.
- Cache parity: none on either side. Ghost explicitly disables invalidation via
  `headers.cacheInvalidate = false` (`config.js:8-10`). C++ handler/step contain no
  `core::cache::Cache` use and no `ResourceStore` cache namespace/TTL/eviction
  (`browse/handler.cppm:12-28`, `browse/.../execute.cppm:12-21`).
- Auth / permissions: PARTIAL. Ghost uses `mw.authAdminApi` (`admin/routes.js:23`) with
  `permissions: false` (`config.js:11`); no `unsafeAttrs`. C++ installs only
  `content_type_middleware` (`routes.cppm:20`) and performs a manual check that returns
  401 `{"error":"Unauthorized"}` when `middleware::token(req)` is empty
  (`browse/handler.cppm:18-22`), where `token()` reads the `Authorization` /
  `x-blog-token` bearer value (`bearer.cppm:8,27-32`). Both skip permission checks,
  but C++ only checks token presence — no signature/key validation — so auth is
  materially weaker.

## Missing in C++

None. The only Ghost route referencing an exported key of this controller
(`GET /config`, `admin/routes.js:23`) has a C++ route and handler counterpart
(`system/config/routes.cppm:19-27`, `browse/handler.cppm:17-28`).

## Extra in C++

None counted. The matched folder `include/blog/handlers/system/config/` installs two
routes: `GET /config` (`routes.cppm:21-27`, matched above) and `GET /config/featurebase`
(`routes.cppm:29-38` → `config_config_featurebase_get`, `all.cppm:376,625`,
`config_featurebase_get/handler.cppm`). The latter has a Ghost route at the same
path (`admin/routes.js:24`) but that route references `api.config.featurebase`, a key
not exported by `config.js`; it belongs to the separate `featurebase` controller and
is documented in `docs/parity/featurebase.md`, so it is not reported here as EXTRA.
