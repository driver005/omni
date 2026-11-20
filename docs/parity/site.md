# Parity: site

> **Authoritative state: `docs/parity/behavior/site.md`.** The strict-parity rewrite
> (manifest group 57, 2026-11-20) is recorded there. Current verdicts: GET `/site`
> PARTIAL, GET `/members/api/site` MISSING. The Summary/Routes table below is the
> pre-strict-parity analysis and is retained for provenance only.

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/site.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (content variant `.../content/routes.js` has no `site` route)
- C++: `include/blog/handlers/system/site/routes.cppm` (or "none found")
- Checked: 2026-11-19

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
| GET `/site` | `site.read` (`docName: 'site'`) | GET `/site` (`system/site/routes.cppm:19-28`) | `site_browse().handle` → `BrowseHandler` (`browse/handler.cppm:18`) | Both none: Ghost no `options[]`/`data[]`; C++ `using Input = IRequest&`, no query/body parse (`execute.cppm:13`) | Aligned after fix: C++ returns `{"site": …}` with no `meta` (`execute.cppm:34`) from a populated `SiteResponse` (`types/dto_site_response.cppm:13-77`) vs Ghost `{"site": publicConfig.site}` (`utils/serializers/output/site.js:8-25`) | Aligned after fix: C++ `ctx.store().json_for("settings", {})` → `settings_json()` (`execute.cppm:21`), no model; Ghost `settingsCache`/`config` (`utils/public-config/site.js:2-24`) | none both sides: Ghost `headers.cacheInvalidate:false` (`site.js:8-10`); C++ no cache key/TTL | Aligned after fix: public route — `optional_auth` + no token gate (`routes.cppm:20`, `browse/handler.cppm:18`) vs Ghost `permissions:false` + `publicAdminApi` (`site.js:11`, `admin/routes.js:20`) | MATCH | Ghost `admin/routes.js:20`, `site.js:5-15`, `utils/public-config/site.js:6-39`, `utils/serializers/output/site.js:8-25`; C++ `system/site/routes.cppm:19-28`, `browse/handler.cppm:18-27`, `browse/workflow/step/execute.cppm:12-35`, `types/dto_site_response.cppm:13-80` |

## Route details

### GET /site — MATCH (source fix applied)

The `Divergent` notes in the per-check bullets below describe the pre-fix state. Checks 4 (output), 5 (DB call) and 7 (auth) were applied in C++; see "Applied source fixes — manifest group 51" below.

- **Route identity / operation**: Ghost `GET /site` → `http(api.site.read)` (`admin/routes.js:20`) with `docName: 'site'` (`site.js:5`). C++ `GET /site` installed via `Router{router, "/site"}.get(...)` (`system/site/routes.cppm:19-21`), bound to `handler.site_browse().handle` (`system/site/routes.cppm:26`). Registered as `site_browse()` / `BrowseHandler` (`all.cppm:28,374,623`); flow name `site_browse`, step `site_browse_execute` (`browse/workflow/flow/main.cppm:9`, `execute.cppm:13`). Op name `browse` vs Ghost `read` — same endpoint, different op label.
- **Input**: no divergence — neither side declares params. Ghost `read` has no `options`/`data`/`validation` (`site.js:7-15`). C++ `using Input = interfaces::io::IRequest&`, no query-string or body parsing (`execute.cppm:12-14`).
- **Output (material divergence)**: Ghost `query()` returns `publicConfig.site` (`site.js:12-14`), a plain object built from live settings: `title, description, logo, icon, cover_image, accent_color, locale, timezone, url, version, allow_external_signup, site_uuid`, plus conditional `sentry_dsn`/`sentry_env` (`utils/public-config/site.js:6-39`). Ghost's output serializer then wraps those 13 fields as `{"site":…}` with **no** `meta` (`utils/serializers/output/site.js:8-25`, registered as `site` at `utils/serializers/output/index.js:151-152`). C++ step computes a lambda `r` that unconditionally returns `{200, encode(SiteResponse{})}` (`execute.cppm:16-18`); line 19 `if (r.first != 0) return ...` is always true, so the step always returns the serde-encoded default DTO — flat JSON with hardcoded placeholder values (`title:"Ghost"`, `description:"Thoughts, stories and ideas"`, `site_uuid:"d5432f87-…"`, `version:"5.0.0"`, etc.; `dto/site_response.cppm:38-49`) with no `site` wrapper at all. The `{site,meta.pagination}` body (`limit:15,page:1,pages:1,prev:null,next:null,total:0`) at `execute.cppm:21` is unreachable. Handler sets status 200 and body from that string (`browse/handler.cppm:25-27`). Ghost has no hardcoded `limit/page/pages/total`; C++ pagination values are hardcoded but dead. C++ DTO also lacks Ghost's conditional `sentry_dsn`/`sentry_env`.
- **DB call**: Ghost reads `settingsCache.get(...)` and `config.get(...)` via `urlUtils`/`ghostVersion`, no `models.X` call (`utils/public-config/site.js:2-24`). C++ declares `ctx.store().json_for("site", {})` (`execute.cppm:20`) but it is unreachable due to the early return at line 19; no real filtering/pagination performed.
- **Cache**: no differences. Ghost explicitly disables invalidation (`headers.cacheInvalidate: false`, `site.js:8-10`), no cache service/invalidation URL. C++ handler/step contain no `core::cache::Cache`, namespace, tag, or TTL usage (`browse/handler.cppm:1-31`, `execute.cppm:1-27`). None on both sides.
- **Auth (material divergence)**: Ghost marks `permissions: false` (`site.js:11`) and uses `mw.publicAdminApi` (`admin/routes.js:20`), which is `tokenPermissionCheck` that calls `next()` when there is no `api_key` — i.e. public, unauthenticated access (`admin/middleware.js:17-21,141-146`). C++ router installs only `content_type_middleware` (`system/site/routes.cppm:20`, `middleware/serde.cppm:90-126`), no auth middleware, but the handler itself returns `401 Unauthorized` when `token(req).empty()` (`browse/handler.cppm:18-22`); `token()` reads the `Authorization`/Bearer header (`middleware/bearer.cppm:27-32`). Anonymous GET `/site` succeeds on Ghost and is rejected on C++.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, no direct SQL, no Ghost JS, no new global state; cross-request data stays under existing context-local store/cache namespaces.

1. Route identity — no change. `include/blog/handlers/system/site/routes.cppm:19-28` already `Router{router, "/site"}` + `.get(...)`; installer symbol `blog::api::SiteRoutes::install` (`:15`); Ghost `admin/routes.js:20`. Verbatim path and verb match. Op label differs (`browse` vs Ghost `read`) but both are the single site endpoint — label-only, not a divergence.
2. Operation mapping — no change. `browse/workflow/flow/main.cppm:8-10` `MainFlow::build()` registers `ExecuteStep` as workflow `site_browse`; `browse/handler.cppm:23` runs that name; step name `site_browse_execute` (`execute.cppm:13`). Ghost key `read` (`site.js:7`).
3. Input — no change. Ghost declares no `options`/`data`/`validation` (`site.js:7-15`); `execute.cppm:12,14` `using Input = interfaces::io::IRequest&` and the `in`/`WorkflowContext&` params are unused. No query/body parse needed.
4. Output — REPLACE two symbols. (a) `blog::handlers::site::browse::workflow::ExecuteStep::invoke` in `browse/workflow/step/execute.cppm:14-22`: delete the dead lambda `:16-18` and the always-true early return `:19`; replace the return at `:21` with `return core::workflow::StepResponse<Output>{"{\"site\":" + blog::handlers::middleware::encode(site) + "}"};` where `site` is a populated `blog::dto::SiteResponse`. Exact replacement type: `Output = std::string` (`execute.cppm:12`); `encode` from `blog_handlers_middleware` (`middleware/serde.cppm:155-158`). No `meta.pagination` — drop the hardcoded `limit/page/pages/total` that only exists in the unreachable line 21. Keep `Status::OK` 200 at `browse/handler.cppm:27`. (b) `include/blog/dto/site_response.cppm`: add two `std::string` fields `sentry_dsn` and `sentry_env` with setters/getters (mirror `:26-28`) and `serde::FieldDesc<"sentry_dsn", &SiteResponse::get_sentry_dsn, &SiteResponse::set_sentry_dsn, serde::FieldOptions{}>` / the `sentry_env` equivalent in `Serializable<SiteResponse>::fields()` (`:59-71`), matching Ghost's 13-field pick (`utils/serializers/output/site.js:8-25`); the DTO currently has 12 fields.
5. DB call — REPLACE source and type. Ghost `publicConfig.site` reads `settingsCache.get(...)`/`config.get(...)`, no model (`utils/public-config/site.js:2-24`). In `browse/workflow/step/execute.cppm`, replace the unreachable `ctx.store().json_for("site", {})` (`execute.cppm:20`) with `ctx.store().json_for("settings", {})`: `ApiDataStore::json_for` special-cases `settings` → `settings_json()` (`context.cppm:1132,1653-1687`), a context-local `[{"key":…,"value":…}]` array. Decode values with `core_serde::JsonCodec{}.decode<std::string>` (module `core_serde`, `include/core/serde/json_codec.cppm:12-29`; module import already needed for check 4's `encode`). No `models.X`, no SQL, no first-party lib outside the allowed set. `json_for("site", …)` falls through to `ResourceStore::list("site")` (`context.cppm:1180-1181`) which is empty — delete it rather than rely on it.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` (`site.js:8-10`) — no invalidation URL or `X-Cache-Invalidate`. C++ handler/step hold no `core::cache::Cache`, namespace, tag or TTL (`browse/handler.cppm:1-31`, `execute.cppm:1-27`; `all.cppm` wires cache only to members/emails/automations). Both sides: none.
7. Auth — REPLACE (make public). Ghost `permissions:false` (`site.js:11`) + `mw.publicAdminApi` (`admin/routes.js:20`) whose `tokenPermissionCheck` calls `next()` when `!req.api_key` (`admin/middleware.js:17-21,141-146`). C++ currently (a) router-wide `content_type_middleware` (`routes.cppm:20`; `middleware/serde.cppm:90-126`) rejects a bodyless GET with no JSON `Content-Type`/`Accept` (415/406), and (b) `BrowseHandler::handle` returns 401 on empty `token(req)` (`browse/handler.cppm:18-22`; `middleware/bearer.cppm:27-32`). Exact fix in `SiteRoutes::install` (`routes.cppm:19-28`): remove `.use(blog::handlers::middleware::content_type_middleware)` (`:20`) and add `.use(blog::handlers::middleware::optional_auth)` — module `blog_handlers_middleware:auth`, `include/blog/handlers/middleware/auth.cppm:32-40`, documented "Content resources stay readable without a token, as in Ghost". In `BrowseHandler::handle`, delete the `if (blog::handlers::middleware::token(req).empty()) { … }` block (`handler.cppm:18-22`). No permission docName needed (`permissions:false`); no new globals. Absence proof for permission/docName in C++: grep `docName|permission_middleware` over `include/blog/handlers/**/*.cppm` → none.
8. Verdict + evidence — after 4, 5 and 7, path/verb, op, input, output envelope+fields, DB source, cache and auth agree → `MATCH`. Recheck `admin/routes.js:20`, `site.js:5-15`, `utils/public-config/site.js:6-39`, `utils/serializers/output/site.js:8-25` vs `system/site/routes.cppm:19-28`, `browse/handler.cppm:12-28`, `browse/workflow/step/execute.cppm:12-22`, `include/blog/dto/site_response.cppm:10-75`.

Ambiguity — six Ghost fields are **not** in `settings_json()`'s `kSettings` (`site_uuid`, `version`, `url`, `allow_external_signup`, `sentry_dsn`, `sentry_env`; `context.cppm:1656-1678`): keep the existing `SiteResponse` defaults for `site_uuid`/`version`, derive `url` from the request `Host` header (`interfaces::io::types::Token::HOST`, `include/interfaces/io/types.cppm:210`), leave `sentry_*` at defaults. Short snippet for checks 4/5:

```cpp
// browse/workflow/step/execute.cppm — add `import core_serde;`
// encode = blog::handlers::middleware::encode (middleware/serde.cppm:155-158)
auto site = blog::dto::SiteResponse{};                       // fallback defaults (site_response.cppm:38-49)
const auto rows = ctx.store().json_for("settings", {});      // context.cppm:1132,1653-1687
for (const auto& rec : blog::api::parse_array(rows)) {        // context.cppm:595; value strings keep quotes
    const auto k = rec.find("key"), v = rec.find("value");
    if (k == rec.end() || v == rec.end()) continue;
    const auto val = core_serde::JsonCodec{}.decode<std::string>(v->second).value_or(v->second);
    if      (k->second == "title")       site.set_title(val);
    else if (k->second == "description") site.set_description(val);
    else if (k->second == "accent_color")site.set_accent_color(val);
    else if (k->second == "locale")      site.set_locale(val);
    else if (k->second == "timezone")    site.set_timezone(val);
    else if (k->second == "cover_image") site.set_cover_image(val);
    // icon/logo are rfl::Generic (site_response.cppm:33-36): site.set_logo(rfl::Generic{val});
}
site.set_url(std::string{in.find_header(interfaces::io::types::Token::HOST)});
return core::workflow::StepResponse<Output>{"{\"site\":" + blog::handlers::middleware::encode(site) + "}"};
```

`parse_array` values retain JSON quotes, so `JsonCodec{}.decode<std::string>` is the exact decode; `settings_json()` already emits the eight settings Ghost reads from `settingsCache`. Cross-request state remains in the existing `settings` resource / context-local cache — no new namespace.

## Missing in C++

None. The sole Ghost route (`GET /site`) has a C++ counterpart.

#### Fix

No change required. Ghost `GET /site` (`admin/routes.js:20`) maps to C++ `GET /site` (`system/site/routes.cppm:19-28`); `MISSING` count stays 0. The Fix block above closes checks 1–8 for that route.

## Extra in C++

None. The matched folder `include/blog/handlers/system/site/` contains only `routes.cppm` + the `browse` handler/flow/step for `GET /site` (verified via `find include/blog/handlers/system/site -type f`), all mapped to the Ghost route; no unmatched C++ route in that folder.

#### Fix

No change required. `EXTRA` count stays 0; the single C++ route in the matched folder (`system/site/routes.cppm:19-28` → `GET /site`) maps to the Ghost controller route (`admin/routes.js:20`), so there is no C++-only route to remove or map.

## Applied source fixes — manifest group 51 (2026-11-19)

Applied checks 4, 5 and 7 from the first `#### Fix` block. Only files under `include/blog/handlers/system/site/` were edited.

| File | Fix applied |
|---|---|
| `include/blog/handlers/system/site/routes.cppm` | Check 7 — swapped `.use(content_type_middleware)` for `.use(blog::handlers::middleware::optional_auth)` on `Router{router, "/site"}`. Bodyless GET is no longer rejected with 415/406; route is public. |
| `include/blog/handlers/system/site/browse/handler.cppm` | Check 7 — deleted the `token(req).empty()` → 401 early-return block. `BrowseHandler::handle` runs the workflow unconditionally; still sets status 200 and writes body bytes. |
| `include/blog/handlers/system/site/browse/workflow/step/execute.cppm` | Checks 4(a) + 5 — added `import core_serde;`; removed the dead `encode(SiteResponse{})` lambda and always-true `r.first != 0` early return; replaced unreachable `json_for("site", {})` and the dead `{site,meta.pagination}` body with `ctx.store().json_for("settings", {})` → `blog::api::parse_array` → `core_serde::JsonCodec{}.decode<std::string>` populating a `SiteResponse` (title, description, accent_color, locale, timezone, cover_image); `set_url` from request `Host`; returns `{"site":` + `blog::handlers::middleware::encode(site)` + `}`. No `meta.pagination`; no new namespace or global. `Output = std::string` and `static s_ctx` unchanged. |
| `include/blog/handlers/system/site/types/dto_site_response.cppm` | Check 4(b) — added `std::string` members `sentry_dsn`/`sentry_env` with setters/getters and matching `serde::FieldDesc` entries in `Serializable<SiteResponse>::fields()` (12 → 14 serialized fields). |

The Fix block points at `include/blog/dto/site_response.cppm`; the real file in this repo is `include/blog/handlers/system/site/types/dto_site_response.cppm` (module `blog_handlers:site_dto_site_response`). Edit was applied there. Per the block's ambiguity note: `site_uuid`/`version` keep their defaults, `url` comes from the `Host` header, `sentry_*` stay at empty defaults, and `icon`/`logo` remain `rfl::Generic` defaults. Cache needed no change (checks 6, MISSING and EXTRA blocks: "No change required").

## Central registrations needed

None. Existing central wiring was verified and left untouched:

- `include/blog/handlers/routes.cppm:116` — `SiteRoutes{}.install(router, handlers);` already installs `GET /site`.
- `include/blog/handlers/all.cppm:27,396,670` — `site_browse()` accessor and `BrowseHandler m_site_browse` already exist.
- `include/blog/handlers/base.cppm:39` — `export import :site_browse_handler;` already exports the handler partition.
- `include/blog/handlers/middleware/base.cppm:5` — `export import :auth;` already exports `optional_auth` through module `blog_handlers_middleware`.
- `include/blog/handlers/BUILD.bazel:52,142-144,899,903` — DTO/handler/flow/step sources plus `core_serde` and `:blog_handlers_middleware` deps are already listed for target `blog_handlers`.

Shared files were not edited: `include/blog/handlers/{all.cppm,base.cppm,routes.cppm,domain_all.cppm,BUILD.bazel,context/,middleware/,dto/}`.

## Strict-parity rewrite (group 57, 2026-11-20)

Supersedes the group 51 "Applied source fixes" record above. Edits confined to
`include/blog/handlers/system/site/**` plus the two docs; no shared file
(`all.cppm`, `base.cppm`, central `routes.cppm`, `BUILD.bazel`, middleware files)
and no Bazel-cycle fix.

| File | Change |
|---|---|
| `browse/workflow/step/execute.cppm` | Rebuilt the step around `serde::Value::Object` + `core_serde::JsonCodec`: all 14 `output/site.js` keys sourced from context-local `config`/`settings` with `URL`/`GHOST_BUILD_VERSION`/`PRO_ENV` env + DTO fallbacks. `url` from configured home URL (not `Host`); `version` from `GHOST_BUILD_VERSION`; `site_uuid` from config/settings; `allow_external_signup` computed from portal settings; `icon`/`logo` populated; `sentry_dsn`/`sentry_env` conditionally emitted from `client_sentry`. Exports `build_site_body(ctx, in)`. `Output = std::string`, `static s_ctx` retained. |
| `browse/handler.cppm` | Writes admin-app `Cache-Control: private` via `Token::CACHE_CONTROL`; no `X-Cache-Invalidate`. Still 200 + string body from the workflow. |
| `routes.cppm`, `browse/workflow/flow/main.cppm`, `types/dto_site_response.cppm` | Unchanged — public `optional_auth` route, single-step flow and DTO (now fallback-only) already matched. |

Shared abstractions used (all in already-registered partitions): `core_serde::JsonCodec`
/ `serde::Value`, `blog::api::{parse_array,RecordDto}` (`:domain_all`), the
`SiteResponse` defaults (`:site_dto_site_response`), and
`interfaces::io::types::Token::CACHE_CONTROL` / `IRequest::{get_scheme,get_authority}`.
No new partition or BUILD entry was introduced.

New shared needs recorded (not implemented here): (1) a real config/settings service
primitive or `core_adapters::Config` dep to replace the context-local store + env
fallbacks; (2) a registered admin-API auth partition with `tokenPermissionCheck`
`site:['GET']` to close the admin auth residual; (3) a members-app router/mount with
the dynamic CORS delegate plus shared handler registration for `GET /members/api/site`
(the exported `build_site_body` can supply the body).

Verdicts: **unchanged** — GET `/site` PARTIAL (real config source + admin auth chain
residual), GET `/members/api/site` MISSING (needs shared wiring). No MATCH flip claimed.
Isolated module compiles pass; the full `bazel build //include/blog/handlers:blog_handlers`
stays blocked by pre-existing errors/cycles in other groups' files.
