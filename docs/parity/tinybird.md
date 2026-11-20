# Parity: tinybird

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/tinybird.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/tinybird/routes.cppm` (found)
- Checked: 2026-11-19

Ghost controller exports one key: `token` (`tinybird.js:7-28`). The only admin
route referencing it is `GET /tinybird/token` (`routes.js:411`). The controller
action is `token.query` (`tinybird.js:15-27`), with `docName: 'tinybird'`
(`tinybird.js:5`).

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
| GET `/tinybird/token` | `token` (`query`, `tinybird.js:15-27`) | GET `/tinybird/token` (`routes.cppm:19-37`) | `tinybird_tinybird_token_get` → `TinybirdTokenGetHandler::handle` (`handler.cppm:17-52`) | Ghost: no options/params; implicit config `tinybird` + `settingsCache site_uuid` (`tinybird-service-wrapper.js:12-13`). C++: no HTTP input; env (`tinybird__*`/`TINYBIRD_*`) + context-local config/settings reads (`execute.cppm:390-396`). | Ghost: `{"tinybird":{token,exp ISO}|{token}|null}`, no `meta` (`tinybird.js:17-26`). C++: identical envelope — HS256 JWT path emits `{token,exp ISO}`, local/stats `{token}`, unconfigured `null`; 200, `Cache-Control: private`, no `X-Cache-Invalidate`. | Ghost: no model call — token minted locally by `getToken()` (`tinybird.js:16-17`). C++: `ctx.store().json_for("settings", {})` + `resources().list("config")` only for config; no model/DB token call. | none on either side; token cache is instance-local. | Ghost: `mw.authAdminApi` + `members:browse` (`routes.js:411`, `tinybird.js:11-14`). C++: `require_auth` + shared `require_ghost_permission("members","browse")` (`routes.cppm:24`, `handler.cppm:38-45`), Ghost-shaped 401/403. | PARTIAL | Ghost: `routes.js:411`, `tinybird.js:5-28`, `tinybird-service.js:47-189`; C++: `external/tinybird/routes.cppm:19-37`, `tinybird_token_get/handler.cppm:17-52`, `workflow/step/execute.cppm:1-450`. |

## Route details

### GET `/tinybird/token` — PARTIAL

> Updated by the group-31 strict-parity pass. The `#### Fix` block below is the
> original historical plan (group 28); see **Implementation status (group 31)**
> at the end for the applied state.

- Route identity: MATCH. Ghost `GET /tinybird/token` (`routes.js:411`) vs C++ `Router{router,"/tinybird"}.Route{"/token"}.get(...)` (`external/tinybird/routes.cppm:19-29`); central install at `handlers/routes.cppm:126`, module import at `handlers/routes.cppm:53`.
- Operation: MATCH. Ghost `token.query` mints/returns a Tinybird JWT (or static/null token) via `TinybirdServiceWrapper.instance.getToken()` (`tinybird.js:15-27`). C++ step now mirrors the full service: config from env + context-local stores, HS256 JWT with 29 pipe scopes and 300s refresh buffer, local/stats/null branches, and ISO `exp` (`execute.cppm:281-450`). The workflow/handler symbols are unchanged (`handler.cppm:17`, `workflow/flow/main.cppm:9`, `execute.cppm:12`).
- Input parity: Both sides take no request params. Ghost `query()` has no arguments and no `options[]`/`data[]`/`validation{}` (`tinybird.js:15`); it depends implicitly on config `tinybird` + `settingsCache site_uuid` (`tinybird-service-wrapper.js:7-25`). C++ declares `using Input=IRequest&` with no query/body parsing and no config validation (`execute.cppm:11,13-16`). Params present only on Ghost side: `config tinybird`, `settingsCache site_uuid`.
- Output parity: MATCH. Both return `{"tinybird":{token,exp ISO}|{token}|null}` with no `meta` (`tinybird.js:17-26` vs `execute.cppm:446-450`); 200 with `Cache-Control: private` and no `X-Cache-Invalidate` (`handler.cppm:46-51`). The old paginated-envelope divergence is gone.
- DB call parity: Ghost makes no model call; it signs a token from config/settingsCache (`tinybird.js:16-17`). C++ reads only `ctx.store().json_for("settings", {})` and `ctx.store().resources().list("config")` as config sources; no model/DB token read remains (`execute.cppm:390-396`). No filtering/pagination.
- Cache parity: none on either side. Ghost explicitly disables cache invalidation (`headers.cacheInvalidate:false`, `tinybird.js:8-10`) and regenerates the token on every request (`tinybird.test.js:25-49`). C++ has no `core::cache::Cache` use in handler/step and no tinybird cache namespace.
- Auth/permissions: Ghost uses `mw.authAdminApi` (`routes.js:411`) plus `permissions:{docName:'members',method:'browse'}` (`tinybird.js:11-14`), so anonymous/Editor gets 401/403. C++ uses `require_auth` (`routes.cppm:24`) plus the shared `require_ghost_permission("members","browse")` gate in the handler (`handler.cppm:38-45`) writing the same Ghost-shaped 401/403 envelopes. Full admin-chain middlewares (token signature verification, `updateUserLastSeen`, CORS, redirect, `tokenPermissionCheck`) remain uncovered by registered primitives — the residual gap keeping verdict PARTIAL.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, no direct SQL, no Ghost JS, no new global state; cross-request data stays under the existing `tinybird` context-local store/cache namespace.

1. Route identity — no change required. `include/blog/handlers/external/tinybird/routes.cppm:19-31` already installs `Router{router,"/tinybird"}.Route{"/token"}.get(...)`; installer symbol `blog::api::TinybirdRoutes::install` (`routes.cppm:15`); Ghost `admin/routes.js:411`. Path and verb match verbatim; keep the registration.
2. Operation mapping — no change required. Keep `handler.tinybird_tinybird_token_get().handle` (`routes.cppm:28`), `blog::handlers::tinybird::tinybird_token_get::TinybirdTokenGetHandler::handle` (`handler.cppm:17`), flow `tinybird_tinybird_token_get` (`workflow/flow/main.cppm:9`) and step `tinybird_tinybird_token_get_execute` (`execute.cppm:12`). Ghost key is `token.query` (`tinybird.js:7,15`). Do not add or rename a route/handler; only the step body changes.
3. Input — no change required. Neither side takes HTTP params: Ghost `query()` has no arguments, `options[]`, `data[]` or `validation{}` (`tinybird.js:15`); C++ `using Input=interfaces::io::IRequest&` and the `in` parameter are unused (`execute.cppm:11,13`). Ghost's implicit config dependency (`config tinybird` + `settingsCache site_uuid`, `tinybird-service-wrapper.js:7-25`) is a config read, not an HTTP input — reproduced in check 5.
4. Output — REPLACE the return in `ExecuteStep::invoke` (`execute.cppm:17`). Ghost wraps with the `tinybird` serializer as `{"tinybird": data}` and no `meta` (`utils/serializers/output/tinybird.js:3-10`); controller data is `{token,exp}` with ISO `exp`, `{token}`, or `null` (`tinybird.js:19-26`). Delete `,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}` from `execute.cppm:17` and return exactly `"{\"tinybird\":" + token_json + "}"`, where `token_json` is built with `core_serde::JsonCodec{}.encode` of the `token` string plus (JWT branch only) an ISO `exp` string; `null` when unconfigured. Keep `Status::OK` 200 and the typed-output pass-through at `handler.cppm:25-27`.
5. DB call — REPLACE source and type. Ghost makes no model call: `TinybirdServiceWrapper.instance.getToken()` (`tinybird.js:16-17`) mints/reads a token from config + settingsCache (`tinybird-service.js:107-135`). C++ `ctx.store().json_for("tinybird", {})` (`execute.cppm:16`) is not a known-model branch in `ApiDataStore::json_for` (`context.cppm:1128-1179`) and falls through to `m_resources.list("tinybird")` with empty params (`context.cppm:1180-1181`). Exact replacement: `ctx.store().json_for("settings", {})` — `settings` is special-cased to `settings_json()` (`context.cppm:1132,1653-1687`), a context-local `[{"key":…,"value":…}]` array — and add the missing keys `site_uuid`, `tinybird_workspace_id`, `tinybird_admin_token`, `tinybird_stats_local_token`, `tinybird_stats_token`, plus a stored `tinybird_token`/`tinybird_exp_iso`, to the existing `kSettings` table in `settings_json()` (`context.cppm:1656-1678`) so the implicit config has a source. Decode values with `core_serde::JsonCodec{}.decode<std::string>`. Branch order mirrors `tinybird-service.js:107-135`: JWT when workspaceId+adminToken present, else local token, else stats token, else `null`. Cross-request token reuse, if any, stays in the existing `tinybird` resource namespace via `ctx.store().create("tinybird", json)` / `ctx.store().json_for("tinybird", {})` — no new global state. JWT HS256 signing (`jwt.sign`, `tinybird-service.js:162`) has no primitive in the allowed modules; record as accepted residual gap — use the stored token string, do not add a JWT/crypto library.
6. Cache — no change required. Ghost `headers.cacheInvalidate:false` (`tinybird.js:8-10`) and refreshes the token per request through the service instance; C++ handler/step hold no `core::cache::Cache`, namespace, TTL or eviction (`handler.cppm:17-28`, `execute.cppm:13-22`; `all.cppm` wires cache only to members/emails/automations). Both sides: none. Any token reuse uses the existing `tinybird` store namespace, not a new cache namespace.
7. Auth — REPLACE to align status/permissions. Ghost `mw.authAdminApi` (`routes.js:411`) plus `permissions:{docName:'members',method:'browse'}` (`tinybird.js:11-14`), so anonymous/Editor gets 403; C++ installs only `content_type_middleware` (`routes.cppm:20`) plus an in-handler token-presence check returning 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`, `token()` at `bearer.cppm:27-32`) — no docName/permission check. Exact fix: in `blog::api::TinybirdRoutes::install` (`routes.cppm:19-31`) add `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware` (`include/blog/handlers/middleware/auth.cppm:12-29`, Ghost-style 401 `{"errors":[{"message":"Unauthorized","type":"UnauthorizedError"}]}`) and delete the inline 401 branch (`handler.cppm:18-22`). A `members:browse` permission check has no primitive in `blog_handlers_middleware` (grep `permission|docName` over `include/blog/handlers/middleware/*.cppm` → none); record as accepted residual gap — do not introduce a permission model or new global state.
8. Verdict + evidence — after 4, 5 and 7, path+verb, op, input, output envelope, store source and the expressible part of auth agree; JWT signing and `members:browse` authorization remain uncovered by the allowed modules, so the evidence-based verdict improves from MISMATCH to PARTIAL, not MATCH. Recheck `admin/routes.js:411`, `tinybird.js:5-28`, `tinybird-service-wrapper.js:7-25`, `tinybird-service.js:107-135,162-168`, `utils/serializers/output/tinybird.js:3-10` vs `external/tinybird/routes.cppm:19-31`, `tinybird_token_get/handler.cppm:17-28`, `tinybird_token_get/workflow/step/execute.cppm:11-18`, `workflow/flow/main.cppm:9`, `context.cppm:1123-1181,1653-1687`, `middleware/auth.cppm:12-29`.

Short C++ snippet for the rewritten `ExecuteStep::invoke` (checks 4/5):

```cpp
// tinybird_token_get/workflow/step/execute.cppm — add `import core_serde;`
using Output = std::string; // {"tinybird": <tokenData>}, no meta
const auto rows = ctx.store().json_for("settings", {});      // context.cppm:1132,1653-1687
std::map<std::string, std::string> kv;
for (const auto& rec : blog::api::parse_array(rows)) {        // context.cppm:595
    const auto k = rec.find("key"), v = rec.find("value");
    if (k == rec.end() || v == rec.end()) continue;
    kv[k->second] = core_serde::JsonCodec{}.decode<std::string>(v->second).value_or(v->second);
}
const bool jwt = !kv["tinybird_workspace_id"].empty() && !kv["tinybird_admin_token"].empty();
const std::string token = jwt ? kv["tinybird_token"]
    : !kv["tinybird_stats_local_token"].empty() ? kv["tinybird_stats_local_token"]
    : kv["tinybird_stats_token"];
std::string token_json = "null";
if (!token.empty()) {
    token_json = "{\"token\":" + core_serde::JsonCodec{}.encode(token);
    if (jwt && !kv["tinybird_exp_iso"].empty())
        token_json += ",\"exp\":" + core_serde::JsonCodec{}.encode(kv["tinybird_exp_iso"]);
    token_json += "}";
}
return core::workflow::StepResponse<Output>{"{\"tinybird\":" + token_json + "}"};
```

`parse_array` values retain JSON quotes, so `JsonCodec{}.decode<std::string>` is the exact decode; `settings_json()` keys come from the existing `kSettings` table (no new namespace, no SQL). JWT signing (`jwt.sign`, `tinybird-service.js:162`) is not reproducible with the allowed modules — residual gap, recorded above.

## Missing in C++

None. The single Ghost route referencing the `token` key (`GET /tinybird/token`,
`routes.js:411`) has a C++ route/handler counterpart
(`external/tinybird/routes.cppm:19-29`).

  Fix: No change required. The sole Ghost route (`GET /tinybird/token`, `admin/routes.js:411`) already has an exact path+verb C++ registration (`external/tinybird/routes.cppm:19-31`) and handler (`handler.cppm:17`); `MISSING` count stays 0. Check 1 is closed for the entry — no route needs adding. The `#### Fix` above reshapes that existing handler/step/store call only; it does not add a route.

## Extra in C++

None within the matched folder `include/blog/handlers/external/tinybird/`: it
installs only `GET /tinybird/token` (`routes.cppm:19-31`), matching the Ghost
route. (The `tinybird_syncs` field map at `context.cppm:216` and the
`RouteInfo`/`SnapshotCase` test catalogs are not route installations.)

  Fix: No change required. `EXTRA` count stays 0: `include/blog/handlers/external/tinybird/` installs only `GET /tinybird/token` (`routes.cppm:19-31`), mapping 1:1 to `admin/routes.js:411`. Check 1 is closed — no C++ route lacks a Ghost counterpart, so nothing to remove or gate. Keep that single route; the `#### Fix` above changes middleware/input/output/store behavior, not its path or verb.

## Implementation status (manifest group 28)

Applied in folder files only (`routes.cppm`, `tinybird_token_get/handler.cppm`,
`tinybird_token_get/workflow/step/execute.cppm`):

- Check 4 (Output): `execute.cppm` now returns exactly `{"tinybird": <tokenData>}`
  with no `meta`; `tokenData` is `{token[,exp]}` or `null`, via
  `core_serde::JsonCodec`.
- Check 5 (DB call, read side): `execute.cppm` now reads
  `ctx.store().json_for("settings", {})`, parses the `[{"key",...}]` array with
  `blog::api::parse_array`, decodes values with `JsonCodec{}.decode<std::string>`,
  and applies the JWT → local → stats → null branch order.
- Check 7 (Auth): `routes.cppm` adds `.use(require_auth)` after
  `content_type_middleware`; the inline 401 branch in `handler.cppm` is removed.

Checks 1, 2, 3, 6 were "No change required" and were skipped. Verdict improves
to PARTIAL (not MATCH) per check 8.

Deferred / central registrations needed (outside the editable folder):

- `context.cppm` `settings_json()` `kSettings` (`context/`, shared/off-limits): add
  settings keys `site_uuid`, `tinybird_workspace_id`, `tinybird_admin_token`,
  `tinybird_stats_local_token`, `tinybird_stats_token`, `tinybird_token`,
  `tinybird_exp_iso` so the config read has a source. Until added the step
  returns `{"tinybird":null}` (a valid Ghost "unconfigured" outcome).
- JWT HS256 signing (`jwt.sign`, `tinybird-service.js:162`) has no primitive in
  the allowed modules; accepted residual gap — the stored `tinybird_token` and
  `tinybird_exp_iso` strings are used verbatim.
- A `members:browse` permission check has no primitive in
  `blog_handlers_middleware`; accepted residual gap.
- Route registration at `handlers/routes.cppm:126` (and import at `:53`) is
  already correct; no central route change needed.

## Implementation status (group 31, 2026-11-20) — strict-parity rewrite

Supersedes the group-28 notes above. Edits confined to
`include/blog/handlers/external/tinybird/**` plus the two parity docs.

- `workflow/step/execute.cppm`: full Ghost `TinybirdService` mirror. Config read
  from nconf-style env (`tinybird__*`) / contract env (`TINYBIRD_*`) first, then
  context-local `config`/`settings` stores (`blog::api::parse_array` +
  `find_setting`). Wrapper guard requires tinybird config and `site_uuid`;
  `stats.id` overrides `siteUuid`. JWT branch signs locally with std-only
  SHA-256/HMAC/base64url (same approach as the featurebase group-24 pass):
  HS256, no `iat`, payload `{workspace_id,name,exp,scopes}` with 29
  `TINYBIRD_PIPES` scopes carrying `fixed_params.site_uuid`, `exp = now+180min`
  in epoch seconds; cached token verified by HMAC recompute + 300s buffer.
  Controller converts JWT `exp` to ISO (`new Date(exp*1000).toISOString()` shape,
  `.000Z`); local/stats branches emit `{token}` only; unconfigured emits `null`.
  Output is exactly `{"tinybird": data}` with no `meta`. `Output=std::string`,
  static `s_ctx` retained.
- `handler.cppm`: added the controller permission gate through the shared
  `require_ghost_permission(req,res,next,send,"members","browse")`; admin bearer
  maps to Administrator/`*`, explicit `x-blog-*` headers preserved. Writes the
  admin-app `Cache-Control` value and no `X-Cache-Invalidate`. Typed string body
  pass-through, 200, `send()` unchanged.
- `routes.cppm`: removed `content_type_middleware` (no 415/406 gate on a bodyless
  GET, matching admin/app.js:33-34), kept `require_auth`.

Previously deferred group-28 items now resolved: JWT signing is implemented
locally; `members:browse` uses the registered `core_auth`/middleware abstraction;
config no longer depends on adding keys to the off-limits `settings_json()`
fixture (env + context-local config/settings are the source).

Shared abstractions used: `blog::api::{RecordDto,parse_array}` (`:domain_all`),
`blog_handlers_middleware::{require_auth,require_ghost_permission,kGhost*Header}`,
`core_serde::JsonCodec`, `core_workflow`, `interfaces::io`. No shared file,
central route, BUILD file, `all.cppm` or middleware file edited.

New-needs still open (recorded, not implemented here):

1. Add `//include/core/adapters:core_adapters` to `//include/blog/handlers:blog_handlers`
   deps: `core_adapters::tinybird_from_env()` / `detail::env_first` would replace the
   local env helper, and `core::adapters::TinybirdAdapter` is available for real
   env/local-mock stats calls. Not used on this route because Ghost never fetches
   the token over HTTP — it mints/reads it locally (tinybird-service.js:107-135).
2. A secret-taking `core_auth` HS256 signer/verifier to delete the hand-rolled
   SHA/HMAC/base64url block (same need recorded by the featurebase group-24 pass).
3. A registered admin-API auth partition (`authenticateAdminApi`/`authorizeAdminApi`/
   `tokenPermissionCheck` with signature verification) to replace `require_auth`;
   this is the residual gap that keeps both parity docs at PARTIAL.

Verification: step and handler syntax-checked with `g++ -std=c++23 -fsyntax-only`
against local stubs; generated JWT independently verified (valid HS256 signature,
29 scopes, no `iat`, ISO `exp` matches epoch), and local/stats/null/stats.id
branches exercised. Full `bazel build` is blocked by the pre-existing Bazel cycle
in the shared target; not touched. Verdict: **PARTIAL** (summary updated from the
stale MISMATCH), not MATCH, because the full admin auth chain remains uncovered.
