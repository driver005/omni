# Parity: slugs

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/slugs.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/system/slugs/routes.cppm` (handler chain `system/slugs/get_detail/`)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (manifest group 58)

> **Group 58 strict-parity pass (2026-11-20).** Both routes remain `MATCH` and were
> rewritten again against the shared abstractions (CONTRACTS.md): content-type gate
> removed, `core_auth:ghost` + `blog_handlers_middleware` permission gate added,
> validation moved to a 422 `ApiError`, full Ghost slug algorithm (transliteration of
> Latin-1, 185-char cap, tag `#`, protected suffix, empty→baseName) implemented with
> helpers inside the existing registered step partition, success body via
> `blog::mapping::wrap`, and the admin `Cache-Control` header written by the handler.
> `Output` is `std::map<std::string,std::string>` and `static s_ctx` is kept. No shared
> file or Bazel change; `unidecode` for non-Latin scripts is recorded as a new-partition
> need. The behavior report `docs/parity/behavior/slugs.md` is authoritative for the
> final state; the C++ columns below describe the pre-strict-parity state.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Both route fixes were applied locally (see "Applied fixes" below). Original MISMATCH evidence is retained in the per-route sections. The C++ columns in the Routes table describe the pre-fix state, kept as evidence; current behavior is in "Applied fixes".

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/slugs/:type/:name` | `generate` | GET `/slugs/:id/:id` (routers at routes.cppm:19,22,24; `.get` at :25) | `slugs_get_detail()` → `GetDetailHandler` (all.cppm:502) | Ghost: options `include`,`type`,`id`; data `name`; type required ∈ {post,tag,user}, name required, id optional (slugs.js:22-40). C++: no parsing — `Input=IRequest&` ignored (execute.cppm:11,13), hardcoded id `"1"` (execute.cppm:16). | Ghost: `{"slugs":[{"slug":…}]}` from returned slug string (slugs.js:52; admin-x-framework/test/unit/api/slugs.test.tsx:6-8). C++: `{"slugs":[]}` always — status OK (handler.cppm:27; execute.cppm:16-18). | Ghost: `Base.Model.generateSlug(allowedTypes[type], name, {status:'all',modelId:id})` (slugs.js:42-46), models Post/Tag/User (slugs.js:9-13). C++: `ctx.store().find_by_resource("slugs","1")` (execute.cppm:16); "slugs" unhandled in `find_by_resource` (context.cppm:1060-1091) → always nullopt. | Ghost: `cacheInvalidate:false` (slugs.js:19-21) — no invalidation header. C++: none (no Cache/TTL/namespace in routes.cppm/handler/step). | Ghost: `mw.authAdminApi` (routes.js:356) + `permissions:true`, docName `slugs` (slugs.js:17,24). C++: `content_type_middleware` (routes.cppm:20) + token presence check → 401 `{"error":"Unauthorized"}` (handler.cppm:18-22); no permission/docName check. | MATCH (residual: permissions/docName) | Ghost: routes.js:356; slugs.js:17-52. C++: routes.cppm:19-31; handler.cppm:17-27; execute.cppm:11-18; context.cppm:1091. C++ install: routes.cppm(handlers):119. |
| GET `/slugs/:type/:name/:id` | `generate` | GET `/slugs/:id/:id/:id` (routers at routes.cppm:19,22,24; nested `Route{"/:id"}` `.get` at :33-41) | `slugs_get_detail()` → `GetDetailHandler` (all.cppm:502) | Same as above. Ghost `:id` → optional `id` used as `modelId` (slugs.js:31-33,45). C++ ignores all three path params; `"1"` hardwired. | Same as above: Ghost generated dedup slug envelope; C++ constant `{"slugs":[]}`. | Same as above. | Same as above. | Same as above, plus Ghost route at routes.js:357. | MATCH (residual: permissions/docName) | Ghost: routes.js:357; slugs.js:42-52. C++: routes.cppm:19-45 (nested `Route{"/:id"}` at :34); handler.cppm:23-27; execute.cppm:16-18. Catalog confirmations: route_catalog.cppm:197-198 (Ghost) and snapshot_cases.cppm:177-178,1343-1346 (C++). |

## Route details

### GET `/slugs/:type/:name` — MATCH (residual: permissions/docName unenforceable)

- Route identity: Ghost `router.get('/slugs/:type/:name', …)` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:356`. C++ composes `/slugs` (`include/blog/handlers/system/slugs/routes.cppm:19`) + `/:id` (`:22`) + `/:id` (`:24`) with `.get()` at `:25` → same two-segment shape; C++ uses generic `:id` for both `type` and `name`. Router installed at `include/blog/handlers/routes.cppm:119`.
- Operation mapping: Ghost `api.slugs.generate` (`routes.js:356`) → `generate` controller action that generates a slug (`docs/Ghost/ghost/core/server/api/endpoints/slugs.js:18,41-53`). C++ binds `handler.slugs_get_detail()` (`routes.cppm:30`; accessor `include/blog/handlers/all.cppm:502`) → a store "get detail" step (`execute.cppm:13-19`). Different operation.
- Input: Ghost declares `options: ['include','type','id']`, `data: ['name']` (`slugs.js:22-23`), validation `type` required with values `Object.keys(allowedTypes)` = `post|tag|user` (`slugs.js:27-29`), `name` required (`slugs.js:36-38`), `id` optional (`slugs.js:31-33`). URL params are merged into the options channel by the api-framework (`docs/Ghost/packages/api-framework/src/parse-request.ts:19-23`). C++ `using Input=interfaces::io::IRequest&` (`execute.cppm:11`) is accepted but never read — `invoke(const Input& in, …)` ignores `in` (`:13`); no query-string/body parsing, no `type`/`name`/`include` reading; id is hardcoded `"1"` via `std::string{}.empty()?"1":std::string{}` (`:16`).
- Output: Ghost controller returns the generated slug string (`slugs.js:52`), wrapped by api-framework as `{"slugs":[{"slug":…}]}` (`apps/admin-x-framework/test/unit/api/slugs.test.tsx:6-8,25`); status 200. C++ handler sets `Status::OK` and the step body (`handler.cppm:27`), which is `"{\"slugs\":[" + *one + "]}"` only if a record is found, else `{"slugs":[]}` (`execute.cppm:17-18`). Because `"slugs"` is not a handled resource (`context.cppm:1060-1091`, falls through to `return std::nullopt` at `:1091`), the response is always `{"slugs":[]}` — a real generated slug is never returned.
- DB call: Ghost `models.Base.Model.generateSlug(allowedTypes[frame.options.type], frame.data.name, { status:'all', modelId: frame.options.id })` (`slugs.js:42-46`) with models `Post`/`Tag`/`User` (`slugs.js:9-13`); throws `InternalServerError` if no slug (`slugs.js:47-51`). C++ `ctx.store().find_by_resource("slugs", "1")` (`execute.cppm:14,16`) — no slug generation, no type-to-model selection, no `status`/`modelId` options, and unhandled resource.
- Cache: Ghost `headers: { cacheInvalidate: false }` (`slugs.js:19-21`) — explicitly suppresses the invalidation header. C++: none — no `core::cache::Cache`, namespace, key, or TTL in `system/slugs/routes.cppm`, `get_detail/handler.cppm`, or `execute.cppm`.
- Auth: Ghost route middleware `mw.authAdminApi` (`routes.js:356`) plus controller `permissions: true` and `docName: 'slugs'` (`slugs.js:17,24`). C++ router installs only `content_type_middleware` (`routes.cppm:20`); the handler performs a bare token-presence check (`handler.cppm:18`) returning 401 `{"error":"Unauthorized"}` with `Status::UNAUTHORIZED` (`:19-22`) — no `require_auth`, no `bearer_middleware`, no permission/docName enforcement, and a non-standard error envelope.
- Verdict: MISMATCH — same verb and normalized path shape, but operation, DB call/resource, and returned data are materially different.

#### Fix

Target files + exact symbols (fixes APPLIED — see "Applied fixes" below):

- `include/blog/handlers/system/slugs/routes.cppm` — `blog::api::SlugsRoutes::install`: `Router{router, "/:id"}` at `:22`, nested `Router{router, "/:id"}` at `:24`, `.get()` at `:25-31`, `.summary(...)` at `:32`.
- `include/blog/handlers/system/slugs/get_detail/handler.cppm` — `blog::handlers::slugs::get_detail::GetDetailHandler::handle` (`:17-28`), workflow keys at `:23-24`.
- `include/blog/handlers/system/slugs/get_detail/workflow/step/execute.cppm` — `blog::handlers::slugs::get_detail::workflow::ExecuteStep::invoke` (`:13-19`), `Input` (`:11`), `name` (`:12`).

Exact replacements — checks 1–8 closed:

1. **Route identity**: rename placeholders in `routes.cppm` — `:22` `"/:id"` → `"/:type"`, `:24` `"/:id"` → `"/:name"`; `:34` stays `Route{"/:id"}`. Result: `GET /slugs/:type/:name` at `:25-31`, verbatim match to `routes.js:356`. Keep verb GET and install at `include/blog/handlers/routes.cppm:119`; update `.summary()` (`:32`) and expectations in `include/blog/testing/route_catalog.cppm:197-198`, `include/blog/testing/snapshot_cases.cppm:177-178`.
2. **Operation mapping**: keep binding `handler.slugs_get_detail().handle` (`routes.cppm:30`, accessor `include/blog/handlers/all.cppm:502`) — no registry change. Change step work from store "get detail" to slug generation: `name{"slugs_generate_execute"}` (`execute.cppm:12`) with matching `m_workflow.run("slugs_generate", …)` / `ex.result_of("slugs_generate_execute")` (`handler.cppm:23-24`).
3. **Input**: keep `using Input = interfaces::io::IRequest&` (`execute.cppm:11`); read path params from `in.get_path()` (`include/interfaces/io/request.cppm:807`) — `type`, `name`, optional `id`; Ghost merges them into `options` (`slugs.js:22-23`; `parse-request.ts:19-23`). Replace hardcoded `"1"` at `execute.cppm:16` (`std::string{}.empty()?"1":std::string{}`) with parsed segments. Validate `type ∈ {post,tag,user}` and non-empty `name`; on failure use `blog::handlers::middleware::write_error` with a 400 `ErrorEnvelope` (`middleware/error.cppm:21-42`).
4. **Output**: return `{"slugs":[{"slug":<encoded>}]}` — encode with `core_serde::JsonCodec{}.encode(slug)` (`include/core/serde/json_codec.cppm:16-19`); keep `Status::OK` (`handler.cppm:27`); delete constant `{"slugs":[]}` branch (`execute.cppm:18`); on empty result write 500 `ErrorEnvelope` message `Could not generate slug.` (`slugs.js:47-51`).
5. **DB call**: replace `ctx.store().find_by_resource("slugs","1")` (`execute.cppm:16`) with `ctx.store().json_for(resource)` where `resource` = `posts`/`tags`/`users` by `type` — exact `ApiDataStore::json_for` at `include/blog/context/context.cppm:1123-1135`. Scan rows for existing `slug` (all statuses), append `-2`, `-3`, … until unique, excluding the row whose `id` matches optional `:id` (`modelId`). Leave `find_by_resource` untouched; no `"slugs"` resource case needed.
6. **Cache**: no change required — Ghost `cacheInvalidate:false` (`slugs.js:19-21`), C++ has no cache in the three files. Do not add a `core::cache::Cache` key/namespace; dedup reads the store directly, so no new global or cross-request state.
7. **Auth**: replace inline bare-token 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`) with allowed middleware chain in `routes.cppm:20`: `.use(content_type_middleware).use(bearer_middleware).use(require_auth)` (`middleware/auth.cppm:12-29`, `middleware/bearer.cppm:11-24`), emitting the Ghost `UnauthorizedError` envelope. Residual: `permissions:true`/`docName:'slugs'` (`slugs.js:17,24`) has no C++ permission API in the allowed set — record as unenforceable, not silently duplicated.
8. **Verdict**: with 1–7 applied, path, verb, input names, output envelope, DB resource, cache (none/none), and auth agree → `MATCH`.

Short snippet where ambiguity remains (segment split + dedup; `std` + allowed `blog_context`/`core_serde` only):

```cpp
// execute.cppm — inside ExecuteStep::invoke
auto& store = (*s_ctx).store();                 // BlogContext::store() -> ApiDataStore
const std::string p{in.get_path()};             // /slugs/<type>/<name>[/<id>]
std::vector<std::string> seg;
for (std::size_t b = 0, e; (e = p.find('/', b)) != std::string::npos; b = e + 1)
    if (e > b) seg.push_back(p.substr(b, e - b));
const std::string type = seg.size() > 1 ? seg[1] : "";
const std::string name = seg.size() > 2 ? seg[2] : "";
const std::string_view res = type == "post" ? "posts" : type == "tag" ? "tags" : "users";
const std::string all = store.json_for(res);   // status:all fixture rows
auto base = slugify(name);                      // local std-only helper: lower, [^a-z0-9]+ -> '-', trim
auto slug = base;
for (int n = 2; all.find("\"slug\":\"" + slug + "\"") != std::string::npos; ++n)
    slug = base + "-" + std::to_string(n);
return core::workflow::StepResponse<Output>{
    "{\"slugs\":[{\"slug\":" + core_serde::JsonCodec{}.encode(slug) + "}]}"};
```

No third-party libs, direct SQL, Ghost JS, or new global state; all calls come from `blog_context`, `core_workflow`, `core_serde`, `core_router/utils_openapi`, `interfaces::io`, `blog_handlers_middleware`.

### GET `/slugs/:type/:name/:id` — MATCH (residual: permissions/docName unenforceable)

- Route identity: Ghost `router.get('/slugs/:type/:name/:id', …)` at `routes.js:357`. C++ same `/slugs` + `/:id` + `/:id` chain (`system/slugs/routes.cppm:19,22,24`) plus nested `Route{"/:id"}.get()` (`:33-41`) → three-segment shape. Confirmed by `include/blog/testing/route_catalog.cppm:197-198` (Ghost) and `include/blog/testing/snapshot_cases.cppm:177-178,1345-1346` (C++ `/slugs/1/1/1`, `/slugs/post/integrations/<id>`).
- Operation mapping: identical to the first row — Ghost `generate` vs C++ `slugs_get_detail()` (`system/slugs/routes.cppm:40`; `all.cppm:502`).
- Input: Ghost's third placeholder `:id` is the optional `id` option used as `modelId` in `generateSlug` (`slugs.js:31-33,45`). C++ reads none of the three path params; hardcoded `"1"` at `execute.cppm:16`. Same missing validation as first row.
- Output: Ghost generated dedup slug in `{"slugs":[{"slug":…}]}`; C++ constant `{"slugs":[]}` (`execute.cppm:16-18`, `handler.cppm:27`) because `find_by_resource` has no `"slugs"` case (`context.cppm:1091`).
- DB call: Ghost `generateSlug` with `modelId: frame.options.id` (`slugs.js:42-46`); C++ `find_by_resource("slugs","1")` (`execute.cppm:16`) — the request's id is ignored.
- Cache: same as first row — Ghost `cacheInvalidate:false` (`slugs.js:20`); C++ none.
- Auth: same as first row — Ghost `mw.authAdminApi` (`routes.js:357`) + permissions/docName; C++ `content_type_middleware` + bare token check (`system/slugs/routes.cppm:20`, `handler.cppm:18-22`).
- Verdict: MISMATCH — path/verb align, but operation, resource, input handling, and data shape differ.

#### Fix

Target files + exact symbols (same three files as the two-segment route; fixes APPLIED):

- `include/blog/handlers/system/slugs/routes.cppm` — `SlugsRoutes::install`, nested `Route{"/:id"}` at `:34` and its `.get()` at `:35-41`, `.summary(...)` at `:42`.
- `include/blog/handlers/system/slugs/get_detail/handler.cppm` — `GetDetailHandler::handle` (`:17-28`), workflow keys (`:23-24`).
- `include/blog/handlers/system/slugs/get_detail/workflow/step/execute.cppm` — `ExecuteStep::invoke` (`:13-19`), `Input`/`name` (`:11-12`).

Exact replacements — checks 1–8 closed:

1. **Route identity**: after `:22` → `/:type` and `:24` → `/:name` (two-segment Fix), existing `Route{"/:id"}` at `:34` already yields `GET /slugs/:type/:name/:id` — verbatim match to `routes.js:357`. Keep verb GET, install at `include/blog/handlers/routes.cppm:119`; update `.summary()` (`:42`) and `include/blog/testing/snapshot_cases.cppm:177-178,1345-1346`.
2. **Operation mapping**: keep `handler.slugs_get_detail()` binding (`routes.cppm:40`; `all.cppm:502`); change step work to `generate` and rename keys to `slugs_generate`/`slugs_generate_execute` (`handler.cppm:23-24`, `execute.cppm:12`) exactly as in the two-segment Fix.
3. **Input**: third placeholder `:id` is Ghost's optional `id` → `modelId` (`slugs.js:31-33,45`). Read it as `seg[3]` from `in.get_path()` (`interfaces/io/request.cppm:807`); no required-validation for it. Remove hardcoded `"1"` (`execute.cppm:16`); validate `type`/`name` as in the two-segment Fix.
4. **Output**: identical envelope `{"slugs":[{"slug":…}]}` via `core_serde::JsonCodec{}.encode(slug)`; never constant `{"slugs":[]}` (`execute.cppm:18`). Keep `Status::OK` (`handler.cppm:27`); 500 `Could not generate slug.` on empty result (`slugs.js:47-51`).
5. **DB call**: `ctx.store().json_for(res)` (`context.cppm:1123-1135`), `res` chosen by `type`; dedup across all statuses while ignoring the row whose `id` equals `seg[3]` (`{status:'all', modelId:id}`). No `find_by_resource("slugs",…)`, no direct SQL.
6. **Cache**: none either side (`slugs.js:20`; no cache in the three C++ files) — no change required; do not add a `core::cache::Cache` key/namespace or cross-request state.
7. **Auth**: chain `.use(bearer_middleware).use(require_auth)` after `content_type_middleware` (`routes.cppm:20`; `middleware/auth.cppm:12-29`) and drop inline `{"error":"Unauthorized"}` (`handler.cppm:18-22`) for the Ghost error envelope. Permission/docName (`slugs.js:17,24`) stays unenforceable in allowed C++ API — record as residual.
8. **Verdict**: with 1–7 applied, both three-segment routes match → `MATCH`.

Short snippet where ambiguity remains (third-segment `modelId` exclusion; `std` + allowed modules only):

```cpp
// execute.cppm — after the path split from the two-segment Fix
const std::string model_id = seg.size() > 3 ? seg[3] : "";  // Ghost options.id
const std::string all = store.json_for(res);                 // status:all rows
auto base = slugify(name), slug = base;
for (int n = 2; collision(all, slug, model_id); ++n)          // skip row with id==model_id
    slug = base + "-" + std::to_string(n);
return core::workflow::StepResponse<Output>{
    "{\"slugs\":[{\"slug\":" + core_serde::JsonCodec{}.encode(slug) + "}]}"};
```

`collision(all, slug, model_id)` is a std-only helper scanning `all` for `"slug":"<slug>"` while ignoring the object whose `"id": "<model_id>"`; exact JSON object-boundary parsing is the only ambiguity. All other checks are fully specified above. Constraints unchanged: allowed modules only, no third-party libs, direct SQL, Ghost JS, or new global state.

## Missing in C++

None. Both Ghost routes have C++ counterparts (by normalized path segment count and verb). Evidence: Ghost `routes.js:356-357`; C++ `include/blog/handlers/system/slugs/routes.cppm:19-45`, handler registered at `include/blog/handlers/routes.cppm:119` and `include/blog/handlers/all.cppm:156,502,751`.

## Extra in C++

None. `system/slugs/routes.cppm` installs exactly two GET routes (`:25-31` → `/slugs/:id/:id`, `:35-41` → `/slugs/:id/:id/:id`), both binding `handler.slugs_get_detail()`. They correspond one-to-one with the two Ghost `slugs.generate` routes; no C++ slug route lacks a Ghost counterpart.

  Fix: No change required. Both routes were kept (only placeholders renamed to `/slugs/:type/:name` and `/slugs/:type/:name/:id`); no route was added or removed.

## Applied fixes (2026-11-19)

All fixes from both route-detail `#### Fix` blocks were applied locally inside `include/blog/handlers/system/slugs/`. No shared file was edited. `Output` is `std::string`; static `s_ctx` is kept.

- `routes.cppm` — renamed the two `Router` placeholders `/:id` → `/:type` and `/:id` → `/:name` (nested `Route{"/:id"}` kept), producing verbatim `GET /slugs/:type/:name` and `GET /slugs/:type/:name/:id`; added `.use(blog::handlers::middleware::bearer_middleware).use(blog::handlers::middleware::require_auth)` after `content_type_middleware`; updated both `.summary()` strings.
- `get_detail/handler.cppm` — removed the inline bare-token `401 {"error":"Unauthorized"}` branch (auth now middleware-enforced). Validates path `type ∈ {post,tag,user}` and non-empty `name` via `middleware::write_error` with a 400 `ErrorEnvelope` (`ValidationError`); runs workflow `slugs_generate` and reads `result_of("slugs_generate_execute")`; keeps `Status::OK` and writes the generated-slug body; an empty result becomes a 500 `ErrorEnvelope` with message `Could not generate slug.`.
- `get_detail/workflow/flow/main.cppm` — workflow name `slugs_get_detail` → `slugs_generate` so `Workflow::run` resolves the registered definition.
- `get_detail/workflow/step/execute.cppm` — step name `slugs_get_detail_execute` → `slugs_generate_execute`; `import core_serde;`. `invoke` parses `/slugs/<type>/<name>[/<id>]` from `in.get_path()` (std-only segment split), maps `type` to resource `posts`/`tags`/`users`, reads `store.json_for(res)` (all statuses), slugifies `name` and appends `-2`, `-3`, … while a collision exists, skipping the row whose `id` equals the optional third segment (`modelId`). Returns `{"slugs":[{"slug":<encoded>}]}` via `core_serde::JsonCodec{}.encode(slug)`; returns an empty string on an empty slug for the handler's 500 branch. No `find_by_resource("slugs", …)`, no direct SQL, no new global state.
- Cache: unchanged — Ghost `cacheInvalidate:false` and no C++ cache in the three files; dedup reads the context-local store directly.
- Residual: `permissions:true` / `docName:'slugs'` (`slugs.js:17,24`) has no C++ permission API in the allowed set; not silently duplicated.

Note: path-split and collision helpers are local, std-only functions in `execute.cppm`/`handler.cppm`; exact JSON object-boundary parsing remains the only ambiguity in the dedup scan (documented residual).

## Central registrations needed

None for the handler/workflow/route registrations; all required central registrations already exist in shared files and were left untouched:

- `include/blog/handlers/all.cppm`: `import :slugs_get_detail_handler`, `slugs_get_detail()` accessor (`:502`), and handler member/registration (`:156,751`) — keep as-is; binding is unchanged (`:30,:40` in `system/slugs/routes.cppm`).
- `include/blog/handlers/base.cppm`: `export import :slugs_get_detail_handler` — keep as-is.
- `include/blog/handlers/routes.cppm`: `export import :slugs_routes` and `SlugsRoutes{}.install(router, handlers)` (`:119`) — keep as-is.
- `blog_handlers_middleware`: `bearer_middleware` / `require_auth` already exported by `middleware/base.cppm` and mounted locally via `.use(...)`; no central registry change.

Out-of-scope follow-ups (shared files, NOT edited by this group):

- `include/blog/testing/route_catalog.cppm:197-198` and `include/blog/testing/snapshot_cases.cppm:177-178,1345-1346` still record the old normalized paths `/slugs/:id/:id` and `/slugs/:id/:id/:id` (`/slugs/1/1/1`, `/slugs/post/integrations/<id>`). Update these expectations to `/slugs/:type/:name` and `/slugs/:type/:name/:id` in the central pass so the route/snapshot tests match the renamed placeholders. No handler accessor or workflow registry change is needed.
