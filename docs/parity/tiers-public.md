# Parity: tiers-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/tiers-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/membership/tiers/routes.cppm` (and `membership/tiers/browse/**`)
- Checked: 2026-11-19

> **Strict-parity rewrite (2026-11-20):** the `#### Fix` blocks below are the
> historical first-pass plan; see `docs/parity/behavior/tiers-public.md` for the
> rewrite. The shared browse step now uses `core_query`, `blog_handlers:mapping`,
> Ghost fixed pagination, `active:true` on the content path (`x-blog-context`),
> free-tier price omission and content header suppression. Verdict stays
> PARTIAL because the content route set is still not installed globally (shared
> `routes.cppm` wiring owned by the main agent) and requirements/labs plus real
> product-model values have no C++ primitive.

Exported keys of the controller: `docName: 'tiers'` and op `browse` (`tiers-public.js:5,7`).
Only one route references them: `GET /tiers -> api.tiersPublic.browse` (`content/routes.js:41`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 3 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| `GET /tiers` | `browse` | `GET /tiers` (`membership/tiers/routes.cppm:19-28`) | `handler.tiers_browse().handle()` (`routes.cppm:26`; `browse/handler.cppm:17-28`) | Ghost: `options[] = limit, fields, filter, order, debug, page, include` + `include.values=['requirements']` (`tiers-public.js:11-15`); input serializer maps `include=requirements` to `withRequirements` gated by Labs flag (`utils/serializers/input/tiers.js:93-94`). C++: `using Input = IRequest&`, no DTO (`execute.cppm:11`); parses only query string for `monthly_price:` / `>` (`execute.cppm:18-26`); none of limit/fields/order/debug/page/include/withRequirements. | Ghost: `{tiers:[SerializedTier…], meta:{pagination:{page:1,pages:1,limit:tiers.length,total:tiers.length,prev:null,next:null}}}` (`output/tiers.js:5,18-25`; `tiers-api.js:61-73`), status 200. C++: `{"tiers":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` (`execute.cppm:28`), status OK (`handler.cppm:27`); raw store rows, not serialized Tier objects; synthetic `[{name,monthly_price}]` for `monthly_price:` filter (`execute.cppm:25`). | Ghost: `tiers.service.api.browse(frame.options)` (`tiers-public.js:18`) -> `TiersAPI.browse` -> `repository.getAll(options)` (`tiers-api.js:53-59`), resource `tiers`. C++: `ctx.store().json_for("tiers", {})` (`execute.cppm:16`), resource `tiers`; no real filtering/pagination, only ad-hoc `monthly_price:` special case. | Ghost: `headers.cacheInvalidate:false` (`tiers-public.js:8-10`) -> no X-Cache-Invalidate at route level (service repository caches internally, `services/tiers/index.ts:19`). C++: none; no Cache use in handler/step (global `core::cache::Cache` exists at `all.cppm:836-848`, untouched by tiers). | Ghost: `mw.authenticatePublic` (`content/routes.js:41`) + `permissions:true` (`tiers-public.js:16`); public content API. C++: `content_type_middleware` only (`routes.cppm:20`); dead auth check `token(req).empty() && !true` (`browse/handler.cppm:18`) -> no auth enforced; no bearer middleware. | PARTIAL | Ghost: `tiers-public.js:7-22`, `content/routes.js:41`. C++: `membership/tiers/routes.cppm:19-28`, `browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:11-28`, `all.cppm:368,617`. |
| — (no `tiers-public` route) | — | `POST /tiers` (`membership/tiers/routes.cppm:29-36`) | `tiers_create` (`routes.cppm:34`; `create/handler.cppm:17-28`) | — | — | — | — | C++ only: `content_type_middleware` (`routes.cppm:20`), token check `create/handler.cppm:18`. | EXTRA | C++: `membership/tiers/routes.cppm:29-36`. Ghost absence: `tiers-public.js:4-23` defines only `browse`; `grep -n "tiersPublic" content/routes.js` -> only line 41 (`browse`). |
| — (no `tiers-public` route) | — | `GET /tiers/:id` (`membership/tiers/routes.cppm:37-45`) | `tiers_read_by_id` (`routes.cppm:44`; `read_by_id/handler.cppm:17-28`) | — | — | — | — | C++ only token check `read_by_id/handler.cppm:18`. | EXTRA | C++: `membership/tiers/routes.cppm:37-46`. Ghost absence: same grep/status as above (no `tiersPublic.read`). |
| — (no `tiers-public` route) | — | `PUT /tiers/:id` (`membership/tiers/routes.cppm:47-54`) | `tiers_edit` (`routes.cppm:52`; `edit/handler.cppm:17-28`) | — | — | — | — | C++ only token check `edit/handler.cppm:18`. | EXTRA | C++: `membership/tiers/routes.cppm:47-55`. Ghost absence: same grep/status as above (no `tiersPublic.edit`). |

## Route details

### GET /tiers — PARTIAL

- Route identity + operation: `GET /tiers` on both sides, op `browse` / `tiers_browse` — agree (`content/routes.js:41`; `membership/tiers/routes.cppm:19-28`; `browse/handler.cppm:23-24`). No difference here.
- Input (DIFFERENT): Ghost allows `limit, fields, filter, order, debug, page, include` with `include` values `['requirements']` (`tiers-public.js:11-15`) and maps `include=requirements` to `withRequirements` under a Labs flag (`utils/serializers/input/tiers.js:93-94`). C++ declares `using Input = IRequest&` (`execute.cppm:11`) and reads only `monthly_price:` (plus `>`) from the raw query string (`execute.cppm:18-26`). No `limit/fields/order/debug/page/include/withRequirements` handling; no body DTO.
- Output (DIFFERENT): Ghost output serializer produces `{tiers:[…full serialized Tier fields…], meta}` (`output/tiers.js:18-25,43-83`) with dynamic `limit/total = tiers.length` (`tiers-api.js:61-73`). C++ hardcodes pagination `limit:15, total:0` (`execute.cppm:28`), returns raw `json_for` rows without field serialization, and for `monthly_price:` returns synthetic 2-field objects (`execute.cppm:25`). Both set 200/OK (`handler.cppm:27`).
- DB call (PARTIAL): same resource string `tiers`; Ghost calls the service `browse -> repository.getAll(options)` with real option semantics (`tiers-public.js:18`, `tiers-api.js:53-59`), while C++ calls `ctx.store().json_for("tiers", {})` with empty params (`execute.cppm:16`) and does no real filtering/pagination.
- Cache (both effectively none at route level): Ghost `headers.cacheInvalidate:false` (`tiers-public.js:8-10`); C++ handler/step never use the `core::cache::Cache` created at `all.cppm:836-848`.
- Auth (DIFFERENT): Ghost uses content-API middleware `mw.authenticatePublic` and `permissions:true` (`content/routes.js:41`, `tiers-public.js:16`). C++ installs only `content_type_middleware` (`routes.cppm:20`, definition `middleware/serde.cppm:90-104`) and its token guard is dead code (`&& !true`, `browse/handler.cppm:18`), so no authorization/permission check runs.

#### Fix

Target: `include/blog/handlers/membership/tiers/browse/workflow/step/execute.cppm` — `blog::handlers::tiers::browse::workflow::ExecuteStep::invoke` (`:13-29`); its handler `include/blog/handlers/membership/tiers/browse/handler.cppm:17-28`; route middleware `include/blog/handlers/membership/tiers/routes.cppm:19-28`; cache accessor on `blog::api::ApiDataStore` / `blog::BlogContext` in `include/blog/context/context.cppm:868-1716`.

Exact replacements (checks 1–8 closed):
- **3 Input**: replace the raw `monthly_price:` branch (`:18-27`) with an allowlist parse of `limit, fields, filter, order, debug, page, include` into `std::map<std::string,std::string> params` — the only keys `query_json_impl`/`search_json` honor (`context.cppm:1546-1550,1574-1589`). Validate `include ∈ {"requirements"}` (`tiers-public.js:15`) and map it to `withRelated`/`withRequirements` gated by Labs `COLLECTION_FLAG` (`input/tiers.js:79-94`); since no C++ Labs flag exists, accept the value but do not embed requirements (documented residual — checkout-only field). Force content-API `active:true` filter (`input/tiers.js:12-27,74-76`) via `params["filter[active]"] = "true"`; reject malformed NQL through `blog::api::ApiError{}.validation(...)` (`context.cppm:63,76-84`) as Ghost does (`input/tiers.js:62-69`).
- **4 Output**: replace hardcoded `meta.pagination` (`:28`) with real values: `total = m_ctx->store().total("tiers")` (`context.cppm:1111-1121`), `limit = parse_size(params,"limit",total)`, `page = parse_size(params,"page",1)`, `pages = (total+limit-1)/limit`, `prev/next` null-aware. Serialize each row to Ghost `SerializedTier` fields (`id,name,description,slug,active=status=='active',type,welcome_page_url,created_at,updated_at,visibility,benefits,currency,monthly_price,yearly_price,trial_days`; drop currency/prices when `type=='free'`) using `core_serde::JsonCodec`/field rewrite (`output/tiers.js:43-83`). Replace the synthetic 2-field rows (`:25`). Keep `Status::OK` (`browse/handler.cppm:27`) — Ghost does not change status.
- **5 DB call**: keep `m_ctx->store().json_for("tiers", params)` (`:16`) but pass parsed `params` instead of `{}`; real filter/limit/page/fields already live in `query_json_impl` (`context.cppm:1532-1561`), and the `active` force above closes the content-API filter gap.
- **6 Cache**: add `[[nodiscard]] core::cache::Cache& cache() noexcept { return m_cache; }` to `blog::api::ApiDataStore` (member `m_cache`, `context.cppm:1697`; place after `resources()`, `:1101-1109`) and forward from `blog::BlogContext` (`:1711`). In the step use key `"tiers:content:browse:" + canonical(params)`, `auto hit = m_ctx->store().cache().get(key)` → `core::cache::value_as<std::string>(hit.value)` on hit; on miss `cache.put(key, core::cache::to_cache_value(body), core::cache::Ttl{}.never(), {"tiers"})` (`cache.cppm:55-61`, `value.cppm:29,52`, `ttl.cppm:19`). Tag `tiers` mirrors `members`/`emails`/`automations` (`all.cppm:836-839`). Ghost `headers.cacheInvalidate:false` (`tiers-public.js:8-10`) already agrees — C++ emits no `X-Cache-Invalidate`. All cross-request state stays context-local in `m_cache` — no new global state.
- **7 Auth**: add `.use(blog::handlers::middleware::bearer_middleware)` and `.use(blog::handlers::middleware::optional_auth)` after `content_type_middleware` at `routes.cppm:20` (signatures `middleware/bearer.cppm:11-24`, `middleware/auth.cppm:32-40`), matching `mw.authenticatePublic` (`content/routes.js:41`). Delete the dead guard `&& !true` at `browse/handler.cppm:18`. `permissions:true`/`docName:'tiers'` is controller metadata with no separate C++ permission layer.
- Checks 1, 2 already closed (`GET /tiers`, `tiers_browse()`); 8 closed by re-running this parity doc. No third-party libs, direct SQL or new global state.

```cpp
static constexpr std::string_view kAllowed[]{"limit","fields","filter","order","debug","page","include"};
// allowlist raw query pairs into params; validate include ∈ {"requirements"}
params["filter[active]"] = "true"; // content API: active tiers only (input/tiers.js:12-27,74-76)
const auto rows = m_ctx->store().json_for("tiers", params);
const auto total = m_ctx->store().total("tiers");
// meta.pagination = {limit, page, pages=(total+limit-1)/limit, prev, next, total}
const std::string key = "tiers:content:browse:" + canonical(params);
const auto hit = m_ctx->store().cache().get(key);
```

### POST /tiers — EXTRA

- C++ route `POST /tiers` -> `tiers_create` (`membership/tiers/routes.cppm:29-36`) has no counterpart in `tiers-public.js` (only `browse`, lines 4-23) and no `api.tiersPublic.*` binding in `content/routes.js` (only line 41). Likely belongs to the admin tiers controller (separate scope); listed here as EXTRA for the matched C++ folder.

  Fix: No change required for tiers-public parity. Check 8 is closed — `tiers_create()`/`CreateHandler` (accessor `all.cppm:425`, member `:674`; `create/workflow/step/execute.cppm:16` `mutations.create("tiers", body)`) is the Ghost admin `add` operation owned by `tiers.js`; do not delete the handler. Keep it out of any content-only `TiersRoutes::install(content=true)` variant (see Missing Fix). No route/verb/input/output/DB/cache/auth change in this controller; no new global state.

### GET /tiers/:id — EXTRA

- C++ route `GET /tiers/:id` -> `tiers_read_by_id` (`membership/tiers/routes.cppm:37-46`) has no `tiersPublic.read` counterpart (same absence evidence). Likely admin-domain scope.

  Fix: No change required for tiers-public parity. Check 8 is closed — `tiers_read_by_id()`/`ReadByIdHandler` (accessor `all.cppm:426`, member `:675`; `read_by_id/workflow/step/execute.cppm:16-18` `find_by_resource("tiers", ...)`) is the Ghost admin `read` operation owned by `tiers.js`. The content API has no `api.tiersPublic.read` route (`content/routes.js:41` only binds `browse`). Keep the handler; exclude this route from any content-only `TiersRoutes::install(content=true)` variant. No new global state.

### PUT /tiers/:id — EXTRA

- C++ route `PUT /tiers/:id` -> `tiers_edit` (`membership/tiers/routes.cppm:47-55`) has no `tiersPublic.edit` counterpart (same absence evidence). Likely admin-domain scope.

  Fix: No change required for tiers-public parity. Check 8 is closed — `tiers_edit()`/`EditHandler` (accessor `all.cppm:427`, member `:676`; `edit/workflow/step/execute.cppm:16` `mutations.edit("tiers", ...)`) is the Ghost admin `edit` operation owned by `tiers.js`. Keep the handler; exclude this route from any content-only `TiersRoutes::install(content=true)` variant. No new global state.

## Missing in C++

None. The one Ghost route (`GET /tiers`, `content/routes.js:41`) has a C++ counterpart (`membership/tiers/routes.cppm:19-28`).

  Fix: No change required. Check 8 (MISSING) closed — route + verb + op already present; no route to add. The single `#### Fix` in Route details reshapes the existing `browse` handler/step/router (allowlisted input, serialized output with real pagination, `active` filter, context-local `core::cache::Cache` under tag `tiers`, `authenticatePublic`-equivalent auth). Optionally add a `bool content = false` parameter to `blog::api::TiersRoutes::install` (`routes.cppm:15-56`) that installs only the GET route with bearer + optional auth and binds the fixed browse step, registered from `ApiRoutes::install` as a second content route set — do not remove the flat admin set. No new global state.

## Extra in C++

C++ routes in the matched folder `include/blog/handlers/membership/tiers/` with no route in this controller:

1. `POST /tiers` -> `tiers_create` — `membership/tiers/routes.cppm:29-36`
   Fix: No change required — Ghost admin `add` (`tiers.js`), not in `tiers-public.js` (`tiers-public.js:4-23`) nor `content/routes.js:41`. Keep accessor `all.cppm:425` / member `:674` for the admin controller; exclude this route from the content-only install variant described above. Check 8 closed.
2. `GET /tiers/:id` -> `tiers_read_by_id` — `membership/tiers/routes.cppm:37-46`
   Fix: No change required — Ghost admin `read` (`tiers.js`), no `api.tiersPublic.read` binding. Keep accessor `all.cppm:426` / member `:675`; exclude from the content-only variant. Check 8 closed.
3. `PUT /tiers/:id` -> `tiers_edit` — `membership/tiers/routes.cppm:47-55`
   Fix: No change required — Ghost admin `edit` (`tiers.js`), no `api.tiersPublic.edit` binding. Keep accessor `all.cppm:427` / member `:676`; exclude from the content-only variant. Check 8 closed.

Absence confirmed via `grep -n "tiersPublic" docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (only `browse` at line 41) and full read of `tiers-public.js` (only `browse`, lines 4-23). These operations exist in the admin tiers controller (`docs/Ghost/ghost/core/server/api/endpoints/tiers.js`), not the public/content controller.
