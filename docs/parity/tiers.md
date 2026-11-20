# Parity: tiers

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/tiers.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 146-150)
- C++: `include/blog/handlers/membership/tiers/routes.cppm` (installed via `include/blog/handlers/routes.cppm:52,125`; registry `include/blog/handlers/all.cppm:22,79-81,368,425-427,617,674-676`)
- Checked: 2026-11-19

> **Strict-parity rewrite (2026-11-20):** the `#### Fix` blocks below are kept as
> the historical first-pass plan and are superseded by the rewrite recorded in
> `docs/parity/behavior/tiers.md`: `GET /tiers` and `POST /tiers` are now MATCH
> (permission middleware, `core_query`, `blog_handlers:mapping`, free-tier price
> omission, domain validation/slug/events, Ghost fixed pagination). The two
> `:id` routes stay PARTIAL (Ghost propagates `null` to the serializer → 500;
> C++ returns 404). No shared file was edited; the Bazel cycle was untouched.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 4 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All four Ghost routes have a C++ route at the same path + verb. Route and core operation agree in every case; each one diverges on input parsing, output envelope/data, DB call semantics, and/or auth, so all four are PARTIAL.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/tiers` | `browse` | GET `/tiers` | `tiers_browse()` → `BrowseHandler` | Ghost: 7 options `limit/fields/filter/order/debug/page/include` + `include:['requirements']`; C++: no option parsing at all, only ad-hoc query-string sniff for `monthly_price:` | Ghost: `tiers.service.api.browse` page (`{data,meta}` with real pagination); C++: `{"tiers":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` hardcoded | Ghost: `tiers.service.api.browse(frame.options)`; C++: `ctx.store().json_for("tiers", {})` + hardcoded Gold/Free reply on `monthly_price:` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + permissions `docName:'products'`; C++: `content_type_middleware` only, dead token check (`&& !true`) | PARTIAL | Ghost: `routes.js:147`, `tiers.js:5,11-24`; C++: `tiers/routes.cppm:19-28`, `browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:14-28` |
| POST `/tiers` | `add` | POST `/tiers` | `tiers_create()` → `CreateHandler` | Ghost: `options:['include']`, validation `data.name required`, `include:['requirements']`; C++: `in.body_as_string()` raw, no validation | Ghost: status 201, add result `{data,meta}`; C++: status 200, `mutations.create(...).body()` envelope undefined | Ghost: `tiers.service.api.add(frame.data,{withRequirements})`; C++: `mutations.create("tiers", body\|"{}")` | Ghost: `cacheInvalidate:true`; C++: none | Ghost: `mw.authAdminApi` + `products`; C++: `content_type_middleware` only, dead token check | PARTIAL | Ghost: `routes.js:148`, `tiers.js:5,43-64`; C++: `tiers/routes.cppm:29-36`, `create/handler.cppm:17-28`, `create/workflow/step/execute.cppm:13-17` |
| GET `/tiers/:id` | `read` | GET `/tiers/:id` | `tiers_read_by_id()` → `ReadByIdHandler` | Ghost: `data:['id']` from path, `options:['include']`, `withRequirements`; C++: no path/query parsing, no `include` | Ghost: tier model + `withRequirements`; C++: `{"tiers":[<one>]}` / `{"tiers":[]}` | Ghost: `tiers.service.api.read(frame.data.id,{withRequirements})`; C++: `find_by_resource("tiers","1")` — hardcoded id, ignores `:id` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + `products`; C++: `content_type_middleware` only, dead token check | PARTIAL | Ghost: `routes.js:149`, `tiers.js:5,26-41`; C++: `tiers/routes.cppm:38-46`, `read_by_id/handler.cppm:17-28`, `read_by_id/workflow/step/execute.cppm:13-19` |
| PUT `/tiers/:id` | `edit` | PUT `/tiers/:id` | `tiers_edit()` → `EditHandler` | Ghost: `options:['id','include']`, validation `id required`, `withRequirements`; C++: `in.body_as_string()` raw, id hardcoded empty | Ghost: status 200, edit result; C++: status 200, `mutations.edit(...).body()` (status agrees, envelope differs) | Ghost: `tiers.service.api.edit(frame.options.id, frame.data, {withRequirements})`; C++: `mutations.edit("tiers","", body\|"{}")` — id hardcoded empty, ignores `:id` | Ghost: `cacheInvalidate:true`; C++: none | Ghost: `mw.authAdminApi` + `products`; C++: `content_type_middleware` only, dead token check | PARTIAL | Ghost: `routes.js:150`, `tiers.js:5,66-88`; C++: `tiers/routes.cppm:47-54`, `edit/handler.cppm:17-28`, `edit/workflow/step/execute.cppm:13-17` |

## Route details

### GET /tiers — PARTIAL

- **Route/op**: both GET `/tiers`, browse — `routes.js:147`, `tiers/routes.cppm:19-28`.
- **Input**: Ghost declares `limit, fields, filter, order, debug, page, include` and validates `include:['requirements']` (`tiers.js:15-16`, `tiers.js:5`). C++ `Input = IRequest&`, declares no `using Input` DTO, parses none of these; only looks for `monthly_price:` in the raw path query (`browse/workflow/step/execute.cppm:11,18-27`). Params present only on Ghost side: all seven options + `withRequirements`.
- **Output**: C++ emits hardcoded `meta.pagination` values `limit:15,page:1,pages:1,total:0,prev:null,next:null` (`browse/.../execute.cppm:28`) regardless of data; Ghost returns real paginated page from `tiers.service.api.browse`. C++ status 200 (`browse/handler.cppm:27`).
- **DB**: Ghost calls `tiers.service.api.browse(frame.options)` (`tiers.js:21`); C++ reads `ctx.store().json_for("tiers", {})` and, if `monthly_price:` is in the query, replaces data with a hardcoded `[{"name":"Gold","monthly_price":5}]` or `[{"name":"Free","monthly_price":0}]` (`browse/.../execute.cppm:16,21-26`). No real filtering/pagination on Ghost option names.
- **Cache**: Ghost `cacheInvalidate:false` (`tiers.js:12-14`); C++ none.
- **Auth**: Ghost `mw.authAdminApi` (`routes.js:147`) + `permissions.docName:'products'` (`tiers.js:17-19`). C++ installs only `content_type_middleware` (`tiers/routes.cppm:20`); handler token check is dead code because `&& !true` is always false (`browse/handler.cppm:18`). Effective auth absent.

#### Fix

- Checks 1, 2 — No change required: `GET /tiers` → `tiers_browse()` (`tiers/routes.cppm:19-28`; registry `all.cppm:22,368,617`; installer `routes.cppm:125`) matches `router.get('/tiers', ..., http(api.tiers.browse))` (`routes.js:147`; controller `tiers.js:11`).
- Check 3 (input): edit `include/blog/handlers/membership/tiers/browse/workflow/step/execute.cppm`, symbol `blog::handlers::tiers::browse::workflow::ExecuteStep::invoke`. Delete the `monthly_price:` path sniff (lines 18-27) — no Ghost counterpart — and parse the query string with the posts-browse pattern (`include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-29`), keeping only Ghost-declared options `limit, fields, filter, order, debug, page, include` (`tiers.js:15`); validate `include` ∈ `['requirements']` (`tiers.js:5,16`) and return a 422 `ApiError{}.validation(...)` body (`context.cppm:76-84`) on any other value. Pass `params` to `ctx.store().json_for("tiers", params)`. `order`/`debug` are not consumed by the store query — accepted residual (no allowed primitive). Reuse `static blog::BlogContext* s_ctx` (`browse/execute.cppm:31`) — no new global state.
  ```cpp
  std::map<std::string, std::string> params;
  const auto q = std::string{in.get_path()}.find('?');
  if (q != std::string::npos) {
      for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
          const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
          const auto eq = text.find('=');
          if (eq == std::string::npos) continue;
          const auto key = text.substr(0, eq);
          if (key == "limit" || key == "fields" || key == "filter" || key == "order" ||
              key == "debug" || key == "page" || key == "include") params[key] = text.substr(eq + 1);
      }
  }
  const auto rows = ctx.store().json_for("tiers", params);
  ```
- Check 5 (DB call): resource string `"tiers"` already agrees (`tiers.js:21` service vs `browse/execute.cppm:16`). No change required to `context.cppm`: `ApiDataStore::json_for` has a real `"tiers"` branch `query_json(m_db.all<model::Tier>(), params)` (`context.cppm:1134`), and `query_json_impl`/`search_json` already apply `limit/page/offset/filter/fields/include` (`context.cppm:1531-1560,1563-1590`) — params become effective through the step edit. `m_db` is owned by `BlogContext` (context-local, `context.cppm:881`), so no direct SQL and no new global state.
- Check 4 (output): replace the hardcoded pagination string (`browse/execute.cppm:28`) with `{"data":` + rows + `,"meta":{"pagination":{...}}}`; Ghost returns `{data, meta.pagination}` with `page:1,pages:1,limit:tiers.length,total:tiers.length,prev:null,next:null` (`tiers-api.js:61-73`; client type `apps/admin-x-framework/src/api/tiers.ts:25-28`). Derive `total`/`limit` from the decoded rows count (`core_serde::JsonCodec{}.decode<serde::Value>(rows)` → `as_array().size()`) instead of literals 15/0. `BrowseHandler::handle` status 200 (`browse/handler.cppm:27`) — No change required.
- Check 6 (cache): add `res.set_header("X-Cache-Invalidate", "false");` in `BrowseHandler::handle` before `send()` (`browse/handler.cppm:27`; `set_header` at `include/interfaces/io/response.cppm:590`) to mirror `cacheInvalidate:false` (`tiers.js:12-14`). Header only — no `core::cache::Cache` eviction, no new state.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` immediately after `content_type_middleware` (`tiers/routes.cppm:20`; middleware `include/blog/handlers/middleware/auth.cppm:12-29`); delete the dead 401 guard `if (... token(req).empty() && !true)` (`browse/handler.cppm:18-22`) — `&& !true` is always false. Ghost `permissions.docName:'products'` (`tiers.js:17-19`) has no C++ primitive — accepted residual; do not invent new global state.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus `order`/`debug` and `permissions` residuals), re-rate this row `MATCH`; Summary `PARTIAL` 4→3 and `MATCH` 0→1; replace the divergent bullets above with "no differences except accepted residuals". Keep existing `file:line` evidence on both sides.

### POST /tiers — PARTIAL

- **Route/op**: both POST `/tiers`, add — `routes.js:148`, `tiers/routes.cppm:29-36`.
- **Input**: Ghost requires `data.name` and allows `include:['requirements']` (`tiers.js:48-54`). C++ takes raw `in.body_as_string()` with no name validation and no `withRequirements` (`create/.../execute.cppm:16`). `name required` present only on Ghost side.
- **Output/status**: Ghost `statusCode:201` (`tiers.js:44`); C++ always sets 200 (`create/handler.cppm:27`). C++ body is `mutations.create("tiers", ...).body()` — not the `{data,meta}` shape Ghost serializes.
- **DB**: Ghost `tiers.service.api.add(frame.data,{withRequirements})` (`tiers.js:60`); C++ `mutations.create("tiers", in.body_as_string().empty()?"{}":in.body_as_string())` (`create/.../execute.cppm:16`). Resource string `tiers` agrees; service layer does not.
- **Cache**: Ghost `cacheInvalidate:true` (`tiers.js:45-47`); C++ none.
- **Auth**: Ghost `mw.authAdminApi` + `products` (`routes.js:148`, `tiers.js:55-57`); C++ `content_type_middleware` only, dead token check (`create/handler.cppm:18`).

#### Fix

- Checks 1, 2 — No change required: `POST /tiers` → `tiers_create()` (`tiers/routes.cppm:29-36`; registry `all.cppm:79,425,674`) matches `router.post('/tiers', ..., http(api.tiers.add))` (`routes.js:148`; `tiers.js:43`).
- Check 3 (input): edit `create/workflow/step/execute.cppm`, symbol `blog::handlers::tiers::create::workflow::ExecuteStep::invoke`. Replace raw `in.body_as_string().empty()?"{}":in.body_as_string()` (`create/execute.cppm:16`) with the Ghost envelope unwrap: the wire body is `{"tiers":[{...}]}` (`apps/admin-x-framework/src/api/tiers.ts:64`), so decode via `core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string())` (`include/blog/handlers/middleware/serde.cppm:59`), take `["tiers"].as_array().at(0)`, re-encode with `core_serde::JsonCodec{}.encode(...)`, and pass the inner tier to `mutations.create("tiers", innerJson)`. Validate `name` required (`tiers.js:49-54`) → 422 `ApiError{}.validation("Validation error", "", "name")` (`context.cppm:76-84`); parse/validate `?include=` against `['requirements']` (`tiers.js:5,48,53`). Keep `"{}"` only as empty-body fallback. `withRequirements` (`tiers.js:61`) has no store primitive — accepted residual. Reuse `static blog::BlogContext* s_ctx` (`create/execute.cppm:19`) — no new global state.
  ```cpp
  auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
  if (!parsed) return core::workflow::StepResponse<Output>{{"status", "422"},
      {"body", blog::ApiError{}.validation("Validation error", "", "tiers")}};
  const auto& inner = (*parsed)["tiers"].as_array().at(0);
  const auto obj = std::get_if<serde::Value::Object>(&inner); // pattern: context.cppm:314-317
  if (!obj || obj->find("name") == obj->end()) return core::workflow::StepResponse<Output>{{"status", "422"},
      {"body", blog::ApiError{}.validation("Validation error", "", "name")}};
  const auto result = mutations.create("tiers", core_serde::JsonCodec{}.encode(inner));
  return core::workflow::StepResponse<Output>{{"status", std::to_string(result.status())},
      {"body", result.body()}};
  ```
- Check 5 (DB call): No change required to the call — `mutations.create("tiers", innerJson)` matches `tiers.service.api.add(frame.data,{withRequirements})` (`tiers.js:60`) on resource `tiers`; `ApiDataStore::create` persists via `create_entity<model::Tier>(body)` → `m_db.insert` (`context.cppm:1010-1015,1266-1268`), context-local, no direct SQL, no new global state.
- Check 4 (output): `ApiDataStore::wrap` already returns status 201 and `{"tiers":[{...}]}` (`context.cppm:1393-1403`) — the Ghost wire shape for a single tier (`apps/admin-x-framework/src/api/tiers.ts:25-28`). Propagate that status instead of forcing 200: set `using Output=std::map<std::string,std::string>` in the step and return `{{"status", std::to_string(result.status())},{"body", result.body()}}` (snippet above; pattern `include/blog/handlers/admin/posts/create/workflow/step/execute.cppm:13,21-24`). In `CreateHandler::handle` (`create/handler.cppm:17-28`) replace hardcoded `Status::OK` (`line 27`) with `res.set_status(static_cast<interfaces::io::types::Status>(std::stoi(payload.at("status"))))` (pattern `posts/create/handler.cppm:27-35`). This closes the 200→201 divergence.
- Check 6 (cache): add `res.set_header("X-Cache-Invalidate", "true");` before `send()` (`create/handler.cppm:27`; `response.cppm:590`) to mirror `cacheInvalidate:true` (`tiers.js:45-47`). Header only; no cache eviction, no new state.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` after content-type middleware (`tiers/routes.cppm:20`; `auth.cppm:12-29`); delete the dead `&& !true` guard (`create/handler.cppm:18-22`). Ghost `permissions.docName:'products'` (`tiers.js:55-57`) = accepted residual; no C++ primitive.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus `withRequirements`/`permissions` residuals), re-rate this row `MATCH`; Summary `PARTIAL` 3→2 and `MATCH` 1→2; replace the divergent bullets above with "no differences except accepted residuals". Keep both sides' `file:line` evidence.

### GET /tiers/:id — PARTIAL

- **Route/op**: both GET `/tiers/:id`, read — `routes.js:149`, `tiers/routes.cppm:38-46`.
- **Input**: Ghost takes `data:['id']` from the path plus `include`/`withRequirements` (`tiers.js:30-31,37-39`). C++ ignores the path entirely; `find_by_resource("tiers", std::string{}.empty()?"1":std::string{})` always resolves to id `"1"` (`read_by_id/.../execute.cppm:16`). `id` from `:id` and `withRequirements` present only on Ghost side.
- **Output**: C++ hardcodes `{"tiers":[<one>]}` or `{"tiers":[]}` (`read_by_id/.../execute.cppm:17-18`) vs Ghost tier model + requirements. C++ status 200 (`read_by_id/handler.cppm:27`).
- **DB**: Ghost `tiers.service.api.read(frame.data.id, {withRequirements})` (`tiers.js:37`); C++ `find_by_resource("tiers","1")` with hardcoded id — wrong record whenever requested `:id` != `1`.
- **Cache**: Ghost `cacheInvalidate:false` (`tiers.js:27-29`); C++ none.
- **Auth**: Ghost `mw.authAdminApi` + `products` (`routes.js:149`, `tiers.js:33-35`); C++ `content_type_middleware` only, dead token check (`read_by_id/handler.cppm:18`).

#### Fix

- Checks 1, 2 — No change required: `GET /tiers/:id` → `tiers_read_by_id()` (`tiers/routes.cppm:38-46`; registry `all.cppm:80,426,675`) matches `router.get('/tiers/:id', ..., http(api.tiers.read))` (`routes.js:149`; `tiers.js:26`).
- Check 3 (input): edit `read_by_id/workflow/step/execute.cppm`, symbol `blog::handlers::tiers::read_by_id::workflow::ExecuteStep::invoke`. Replace `find_by_resource("tiers", std::string{}.empty()?"1":std::string{})` (always `"1"`, line 16) with a real `:id` path parse (posts read-by-id pattern `include/blog/handlers/admin/posts/read_by_id/workflow/step/execute.cppm:19-22`): strip `?query` and trailing `/`, take last `/` segment. Parse/validate `?include=` against `['requirements']` (`tiers.js:5,31-32`); `withRequirements` (`tiers.js:38`) has no store primitive — accepted residual. Reuse `static blog::BlogContext* s_ctx` (`read_by_id/execute.cppm:21`) — no new global state.
  ```cpp
  auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
  while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
  const auto slash = path.find_last_of('/');
  const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
  const auto one = ctx.store().find_by_resource("tiers", id);
  ```
- Check 5 (DB call): `find_by_resource("tiers", id)` is already id-aware — `find_entity<model::Tier>(id)` → `m_db.find<Tier>` (`context.cppm:1086-1090,1042-1052`); no `context.cppm` change needed (unlike fixture-backed resources). Note ids are numeric (`std::stoll`, `context.cppm:1047`) while Ghost uses 24-hex ObjectID (`tiers-api.js:83`): a non-numeric `:id` yields 404 in C++ vs Ghost's `ObjectID.createFromHexString` — accepted residual, no ObjectID primitive in the allowed modules. No direct SQL, no new global state.
- Check 4 (output): `{"tiers":[<one>]}` / `{"tiers":[]}` (`read_by_id/execute.cppm:17-18`) already matches the Ghost wire envelope `{"tiers":[...]}` (`apps/admin-x-framework/src/api/tiers.ts:25-28`) — No change required to the envelope. Close status parity by changing `using Output=std::string` (`read_by_id/execute.cppm:11`) → `std::map<std::string,std::string>` and returning `{{"status","404"},{"body", ApiError{}.json("Resource not found","Not Found")}}` on miss / `{{"status","200"},{"body","{\"tiers\":[" + *one + "]}"}}` on hit, then setting status from the map in `ReadByIdHandler::handle` (`read_by_id/handler.cppm:17-28`; pattern `posts/read_by_id/handler.cppm:25-36`). Prefer the 404 error body over an unconditional 200 `{"tiers":[]}` (`tiers-api.js:84` repository miss).
- Check 6 (cache): add `res.set_header("X-Cache-Invalidate", "false");` before `send()` (`read_by_id/handler.cppm:27`; `response.cppm:590`) — Ghost `cacheInvalidate:false` (`tiers.js:27-29`). Header only.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` in the `/:id` sub-router (`tiers/routes.cppm:37-46`, after `content_type_middleware` at `:20`; `auth.cppm:12-29`); delete the dead `&& !true` guard (`read_by_id/handler.cppm:18-22`). Ghost `permissions.docName:'products'` (`tiers.js:33-35`) = accepted residual.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus `withRequirements`, id-format and `permissions` residuals), re-rate this row `MATCH`; Summary `PARTIAL` 2→1 and `MATCH` 2→3; replace the divergent bullets above with "no differences except accepted residuals". Keep existing `file:line` evidence on both sides.

### PUT /tiers/:id — PARTIAL

- **Route/op**: both PUT `/tiers/:id`, edit — `routes.js:150`, `tiers/routes.cppm:47-54`.
- **Input**: Ghost `options:['id','include']`, `id required`, `withRequirements` (`tiers.js:68,72-79,84-86`). C++ takes raw body, passes hardcoded empty id `""` to `mutations.edit` and ignores `:id` (`edit/.../execute.cppm:16`). `id` validation/path id and `withRequirements` present only on Ghost side.
- **Output/status**: Ghost `statusCode:200` (`tiers.js:67`); C++ status 200 (`edit/handler.cppm:27`) — status agrees. Body shape `mutations.edit(...).body()` vs Ghost edit result differ.
- **DB**: Ghost `tiers.service.api.edit(frame.options.id, frame.data, {withRequirements})` (`tiers.js:84`); C++ `mutations.edit("tiers","", body\|"{}")` with empty id (`edit/.../execute.cppm:16`). Resource string `tiers` agrees; edit target and service layer do not.
- **Cache**: Ghost `cacheInvalidate:true` (`tiers.js:69-71`); C++ none.
- **Auth**: Ghost `mw.authAdminApi` + `products` (`routes.js:150`, `tiers.js:80-82`); C++ `content_type_middleware` only, dead token check (`edit/handler.cppm:18`).

#### Fix

- Checks 1, 2 — No change required: `PUT /tiers/:id` → `tiers_edit()` (`tiers/routes.cppm:47-54`; registry `all.cppm:81,427,676`) matches `router.put('/tiers/:id', ..., http(api.tiers.edit))` (`routes.js:150`; `tiers.js:66`).
- Check 3 (input): edit `edit/workflow/step/execute.cppm`, symbol `blog::handlers::tiers::edit::workflow::ExecuteStep::invoke`. Replace the empty id `std::string{}` (`edit/execute.cppm:16`) with the `:id` path parse (read-by-id pattern above; strip `?query` and trailing `/`); reject an empty id with a 422 `ApiError{}.validation("Validation error", "id is required", "id")` (`context.cppm:76-84`) instead of editing `""`. Unwrap `tiers[0]` from `in.body_as_string()` (wire `{tiers:[tier]}`, `apps/admin-x-framework/src/api/tiers.ts:72`) via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:59`), re-encode the inner object, and pass it to `mutations.edit("tiers", id, innerJson)`. Ghost edits only the whitelisted fields (`tiers-api.js:116-136`); extra fields are harmless to the store. Parse/validate `?include=` against `['requirements']` (`tiers.js:68,72-79`); `withRequirements` (`tiers.js:85`) = accepted residual. Reuse `static blog::BlogContext* s_ctx` (`edit/execute.cppm:19`) — no new global state.
  ```cpp
  auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
  const auto& inner = parsed ? (*parsed)["tiers"].as_array().at(0) : serde::Value{};
  const auto result = mutations.edit("tiers", id, core_serde::JsonCodec{}.encode(inner));
  const int status = result.status() == 201 ? 200 : result.status(); // Ghost statusCode:200 (tiers.js:67)
  return core::workflow::StepResponse<Output>{{"status", std::to_string(status)},{"body", result.body()}};
  ```
- Check 5 (DB call): resource string `tiers` already agrees; `mutations.edit("tiers", id, body)` → `edit_entity<model::Tier>(id, body)` (`context.cppm:1314-1316,1017-1030`) is id-aware and returns 404 via `wrap` when the id is absent (`context.cppm:1393-1398`). No `context.cppm` change required; no direct SQL, no new global state.
- Check 4 (output): change `using Output=std::string` (`edit/execute.cppm:11`) → `std::map<std::string,std::string>` and propagate status+body (snippet above; pattern `posts/create/.../execute.cppm:13,21-24`). In `EditHandler::handle` (`edit/handler.cppm:17-28`) set status from the map (`posts/create/handler.cppm:27-35`) instead of hardcoded `Status::OK` (`line 27`). `wrap` returns 201 on success (`context.cppm:1399-1402`) but Ghost `edit` sets `statusCode:200` (`tiers.js:67`) — normalize 201→200 in the step, keep 404 on miss. Body `{"tiers":[{...}]}` already matches the Ghost wire envelope (`admin-x-framework/src/api/tiers.ts:25-28`).
- Check 6 (cache): add `res.set_header("X-Cache-Invalidate", "true");` before `send()` (`edit/handler.cppm:27`; `response.cppm:590`) — Ghost `cacheInvalidate:true` (`tiers.js:69-71`). Header only; cross-request state stays in the context-local `m_db`, no new global state.
- Check 7 (auth): add `.use(blog::handlers::middleware::require_auth)` in the `/:id` sub-router (`tiers/routes.cppm:37-46`; `auth.cppm:12-29`); delete the dead `&& !true` guard (`edit/handler.cppm:18-22`). Ghost `permissions.docName:'products'` (`tiers.js:80-82`) = accepted residual.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus `withRequirements`, id-format and `permissions` residuals), re-rate this row `MATCH`; Summary `PARTIAL` 1→0 and `MATCH` 3→4; replace the divergent bullets above with "no differences except accepted residuals". Keep existing `file:line` evidence on both sides.

## Missing in C++

None. All four Ghost routes (`GET /tiers`, `POST /tiers`, `GET /tiers/:id`, `PUT /tiers/:id` — `routes.js:147-150`) have C++ counterparts at the same path + verb in `include/blog/handlers/membership/tiers/routes.cppm:19-55`.

  Fix: No change required. Check 1 is closed for every Ghost route — `routes.js:147-150` maps 1:1 to `tiers/routes.cppm:19-55` (`GET/POST /tiers` at `:21,29`, `GET/PUT /tiers/:id` at `:39,47`) with handlers registered in `all.cppm:22,79-81,368,425-427,617,674-676`. `MISSING` count stays 0; none of the four `#### Fix` blocks above adds a route — they reshape the existing handler/step/store calls only. No new global state, no third-party libs, no direct SQL, no Ghost JS; cross-request state stays in the context-local `m_db` (`context.cppm:881`).

## Extra in C++

None. The matched folder's route file contains exactly four routes (`tiers/routes.cppm:21,29,39,47`), each corresponding to a Ghost route; no additional tiers routes or handlers found.

Absence confirmed by:
- `grep -n "/tiers" include/blog/handlers/**/routes.cppm` → only `include/blog/handlers/membership/tiers/routes.cppm:19`.
- `grep -n "TiersRoutes\|tiers" include/blog/handlers/routes.cppm` → only install/import at lines 52 and 125.
- `grep -n "tiers_browse\|tiers_create\|tiers_read_by_id\|tiers_edit" include/blog/handlers` → 4 handlers + 4 steps + registry/route references only; no fifth operation.

  Fix: No change required. No C++ tier route lacks a Ghost counterpart — `TiersRoutes::install` (`tiers/routes.cppm:15-56`) installs exactly the four routes matching `routes.js:147-150`, and the registry exposes only `tiers_browse()`/`tiers_create()`/`tiers_read_by_id()`/`tiers_edit()` (`all.cppm:368,425-427`). Check 1 (EXTRA) is closed; do not delete or gate any route. The four `#### Fix` blocks above change middleware, input parsing, response status/body and headers on existing handlers, not the path set; no new global state or cross-request cache beyond the context-local `m_db`.
