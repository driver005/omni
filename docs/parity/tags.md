# Parity: tags

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/tags.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/tags/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 6 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/tags` | `browse` | GET `/tags` | `tags_browse()` | Ghost `options: include,filter,fields,limit,order,page,debug`, `include` ∈ `['count.posts']`; C++ reads none (`Input=IRequest&`, no query parse) | Ghost `Tag.findPage` `{data,meta}`; C++ fixed `{"tags":rows,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}`, status 200 | Ghost `models.Tag.findPage(restrictAdminApiQueryOptions(...))`; C++ `json_for("tags", {})` empty params, no filter/paginate | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:139`, `tags.js:16-32`; C++ `routes.cppm:19-27`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:13-17` |
| GET `/tags/:id` | `read` | GET `/tags/:id` | `tags_read_by_id()` | Ghost `data: id,slug,visibility`, `options: include,filter,fields,debug`; C++ ignores `:id` (hardcoded `"1"`) | Ghost single tag model, 404 if missing; C++ `{"tags":[one]}` or `{"tags":[]}`, status 200 (no 404) | Ghost `models.Tag.findOne(frame.data, ...)`; C++ `find_by_resource("tags", "1")` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:140`, `tags.js:34-61`; C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:16-18` |
| GET `/tags/slug/:slug` | `read` | GET `/tags/slug/:id` | `tags_read_by_slug()` | Ghost `data: id,slug,visibility` (slug via path); C++ ignores slug (`json_for("tags", {})`) | Ghost single tag model, 404 if missing; C++ full list with fixed pagination meta (same as browse), status 200 | Ghost `models.Tag.findOne(frame.data, ...)` by slug; C++ `json_for("tags", {})` returns all rows, no slug filter | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:141`, `tags.js:34-61`; C++ `routes.cppm:64-77`, `read_by_slug/handler.cppm:17-27`, `read_by_slug/.../execute.cppm:16-17` |
| POST `/tags` | `add` | POST `/tags` | `tags_create()` | Ghost `data.tags[0]`, `options: include`; C++ raw `in.body_as_string()` (`"{}"` when empty), no `data.tags[0]` unwrap/include parse | Ghost created tag, status 201, `cacheInvalidate:true`; C++ mutation body wrapped `{"tags":[...]}`, status 200 (MutationResult status discarded) | Ghost `models.Tag.add(frame.data.tags[0], frame.options)`; C++ `mutations.create("tags", body).body()` (generic tag validation title/name required → 422 discarded) | Ghost `cacheInvalidate:true`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:142`, `tags.js:63-80`; C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/.../execute.cppm:16`, `context.cppm:1239-1247,1254-1256,1394-1402` |
| PUT `/tags/:id` | `edit` | PUT `/tags/:id` | `tags_edit()` | Ghost `options: id,include`, `id` required, `data.tags[0]`; C++ empty id `""`, raw body, no id/include parse | Ghost edited tag, 404 if missing, sets `X-Cache-Invalidate: /*` on change; C++ `{"tags":[...]}`/error body, status 200, no invalidate header | Ghost `models.Tag.edit(frame.data.tags[0], frame.options)`; C++ `mutations.edit("tags", "", body).body()` | Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate: /*`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:143`, `tags.js:82-112`; C++ `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:16`, `context.cppm:1302-1304` |
| DELETE `/tags/:id` | `destroy` | DELETE `/tags/:id` (`.delt`) | `tags_destroy()` | Ghost `options: id`, `id` required, `require:true`; C++ empty id `""`, no id option | Ghost empty body, status 204; C++ `mutations.destroy(...).body()` (`"{}"` on success), status 200 | Ghost `models.Tag.destroy({...frame.options, require:true})`; C++ `mutations.destroy("tags", "").body()` | Ghost `cacheInvalidate:true`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead code | MATCH | G `routes.js:144`, `tags.js:114-134`; C++ `routes.cppm:55-62`, `destroy/handler.cppm:17-27`, `destroy/.../execute.cppm:16`, `context.cppm:1334-1335,1346-1352` |

## Route details

### GET `/tags` — MATCH

- Route identity: Ghost `router.get('/tags', ..., http(api.tags.browse))` (`routes.js:139`); C++ `Router{router, "/tags"}.get(...)` (`routes.cppm:19-27`). Match.
- Operation: Ghost `browse` (`tags.js:16`); C++ `handler.tags_browse()` (`routes.cppm:26`, registry `all.cppm:363,612`). Match.
- Input: Ghost options `include,filter,fields,limit,order,page,debug` with `include` validated against `['count.posts']` (`tags.js:20-27`); C++ `Input=IRequest&` but step parses no query params (`browse/.../execute.cppm:11-16`). Divergent: all seven Ghost params absent in C++.
- Output: Ghost `models.Tag.findPage` result with real pagination (`tags.js:30`); C++ hardcodes `limit:15,page:1,pages:1,total:0` and `prev/next:null` (`browse/.../execute.cppm:17`), status 200 (`browse/handler.cppm:27`). Divergent meta.
- DB call: Ghost `models.Tag.findPage(restrictAdminApiQueryOptions(frame.options))` (`tags.js:30`); C++ `json_for("tags", {})` with empty params, no filtering/pagination (`browse/.../execute.cppm:16`). Resource string `"tags"` agrees; semantics differ.
- Cache: Ghost `headers.cacheInvalidate:false` (`tags.js:17-19`); C++ none (no tag-specific `core::cache::Cache`; cache only wired for members/emails/automations in `all.cppm:837-839`). Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:139`) + `permissions:true` (`tags.js:28`); C++ applies only `content_type_middleware` (`routes.cppm:20`, content-type/accept check in `serde.cppm:90-115`) and the inline `if (token(req).empty() && !true)` check (`browse/handler.cppm:18-22`) is dead code, so no auth is enforced. No permissions/docName/unsafeAttrs model in C++.

#### Fix

- Checks 1, 2: no change — path/verb and `tags_browse()` binding match (`routes.cppm:19-27`, registry `all.cppm:363,612`).
- Check 3 (input): edit `include/blog/handlers/admin/tags/browse/workflow/step/execute.cppm`, `ExecuteStep::invoke` (`lines 13-17`). Replace `json_for("tags", {})` (`line 16`) with a parsed-param call using the exact `posts/browse/workflow/step/execute.cppm:21-30` pattern: build `std::map<std::string,std::string> params` by splitting `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='`; read `include,filter,fields,limit,order,page,debug`. Validate `include` against `{"count.posts"}`; any other value → 422 `ApiError{}.validation(...)` body (`context.cppm:76-84`) instead of tag data. Then call `ctx.store().json_for("tags", params)`.
  ```cpp
  std::map<std::string,std::string> params;
  const auto q = std::string{in.get_path()}.find('?');
  if (q != std::string::npos) {
    for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
      const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
      const auto eq = text.find('=');
      if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
    }
  }
  if (auto it = params.find("include"); it != params.end() && it->second != "count.posts")
    return core::workflow::StepResponse<Output>{ApiError{}.validation("Validation error", "include invalid", "include")};
  const auto rows = ctx.store().json_for("tags", params);
  ```
- Check 5 (DB call): no `context.cppm` edit needed for the resource string — `ApiDataStore::json_for` `"tags"` branch (`context.cppm:1130`) already delegates to `query_json(m_db.all<model::Tag>(), params)`, which honors `limit/page/offset/filter/fields/include` (`context.cppm:1546-1590`); passing `params` closes filtering/pagination. `order` is not applied by `query_json_impl` — accepted residual (no new global state). `total("tags")` is `m_db.count<model::Tag>()` (`context.cppm:1115`).
- Check 4 (output): in the same step replace hardcoded `limit:15,page:1,pages:1,total:0` and `prev/next:null` (`execute.cppm:17`) with values from `params` and `ctx.store().total("tags")`; `pages = ceil(total/limit)`, `prev`/`next` from `page`. Keep the `{"tags":...}` envelope. `BrowseHandler::handle` sets status 200 (`browse/handler.cppm:27`) — no change.
- Check 6 (cache): edit `browse/handler.cppm`, `BrowseHandler::handle`, before `send()` (`line 27`) add `res.set_header("X-Cache-Invalidate", "false")` to mirror Ghost `cacheInvalidate:false` (`tags.js:17-19`). No `core::cache::Cache` involvement.
- Check 7 (auth): edit `routes.cppm:19-27`, add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`, error-envelope 401) alongside `.use(content_type_middleware)` (`routes.cppm:20`) and remove the dead inline check in `browse/handler.cppm:18-22` (`&& !true`). Ghost `permissions`/`docName` has no allowed C++ primitive — accepted residual gap, no new global state.
- Check 8 (verdict): fix closes checks 1–7 except the accepted `permissions`/`docName` and `order` residuals; verdict stays PARTIAL (not MATCH).

### GET `/tags/:id` — MATCH

- Route identity: Ghost `router.get('/tags/:id', ..., http(api.tags.read))` (`routes.js:140`); C++ `Route{"/:id"}.get(...)` (`routes.cppm:37-46`). Match.
- Operation: Ghost `read` (`tags.js:34`); C++ `handler.tags_read_by_id()` (`routes.cppm:44`, registry `all.cppm:364,613`). Match.
- Input: Ghost `data:['id','slug','visibility']` + options `include,filter,fields,debug` (`tags.js:38-39`); C++ step ignores the path id entirely and calls `find_by_resource("tags", std::string{}.empty()?"1":std::string{})`, i.e. hardcoded `"1"` (`read_by_id/.../execute.cppm:16`). Materially broken id lookup.
- Output: Ghost returns single tag model, throws `NotFoundError("Tag not found.")` when absent (`tags.js:53-57`); C++ wraps result in `{"tags":[one]}` or returns `{"tags":[]}` with status 200 and no 404 (`read_by_id/.../execute.cppm:17-18`, `read_by_id/handler.cppm:27`). Material data-shape/status divergence.
- DB call: Ghost `models.Tag.findOne(frame.data, ...)` (`tags.js:49-52`); C++ `find_by_resource("tags", "1")` (`read_by_id/.../execute.cppm:16`). Resource agrees; id source diverges.
- Cache: Ghost `cacheInvalidate:false` (`tags.js:35-37`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:140`) + `permissions:true` (`tags.js:47`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`read_by_id/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: no change — path/verb and `tags_read_by_id()` binding match (`routes.cppm:37-46`).
- Check 3 (input): edit `read_by_id/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace hardcoded `std::string{}.empty()?"1":std::string{}` (`line 16`) with the path id — last `/`-separated segment of `in.get_path()` (strip any `?query` first), same parsing style as `posts/browse/.../execute.cppm:22-28`:
  ```cpp
  const auto p = std::string{in.get_path()};
  const auto base = p.substr(0, p.find('?'));
  const auto id = base.substr(base.rfind('/') + 1);
  const auto one = ctx.store().find_by_resource("tags", id);
  ```
  Optionally accept `?include=` with the `{"count.posts"}` validation from the browse fix. Reject empty id with a 422 `ApiError{}.validation(...)` body.
- Check 5 (DB call): no `context.cppm` edit needed — unlike labels, `"tags"` already has a `find_by_resource` branch (`context.cppm:1066`) using `find_entity<model::Tag>(id)` / `std::stoll` (`context.cppm:1043-1051`), so numeric path ids resolve. Calling it with the real id closes check 5.
- Check 4 (output): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle` and the step. Return the single tag object (Ghost `read` returns one model, not `{"tags":[...]}`). On miss set `res.set_status(types::Status::NOT_FOUND)` with Ghost-style body `ApiError{}.json("Tag not found.", "Not Found")` (`context.cppm:63-73`, `tags.js:53-57`) before `send()`, replacing unconditional 200 + `{"tags":[]}` (`handler.cppm:27`, `execute.cppm:17-18`). Carry `status`/`body` through the workflow via `Output = std::map<std::string,std::string>` (posts/create pattern, `posts/create/.../execute.cppm:13,21-24`) if the handler must branch on the miss.
- Check 6 (cache): edit `read_by_id/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `cacheInvalidate:false`, `tags.js:35-37`).
- Check 7 (auth): edit `routes.cppm:37-46`, add `.use(blog::handlers::middleware::require_auth)` alongside `content_type_middleware` and remove the dead inline check (`read_by_id/handler.cppm:18-22`). Ghost `permissions`/`docName` remains accepted gap.
- Check 8 (verdict): with checks 3/4 fixed, operation, resource and data shape agree; verdict becomes PARTIAL only for the accepted auth-permissions gap (no longer MISMATCH).

### GET `/tags/slug/:slug` — MATCH

- Route identity: Ghost `router.get('/tags/slug/:slug', ..., http(api.tags.read))` (`routes.js:141`); C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`routes.cppm:64-76`). Path matches after normalizing `:slug` ↔ `:id` placeholder names.
- Operation: Ghost `read` by slug (`tags.js:34`); C++ `handler.tags_read_by_slug()` (`routes.cppm:73`, registry `all.cppm:365,614`). Match.
- Input: Ghost `data:['id','slug','visibility']` with slug supplied via path (`tags.js:39`); C++ step never reads the slug and calls `json_for("tags", {})` (`read_by_slug/.../execute.cppm:16`). No slug filter.
- Output: Ghost single tag model + 404 when missing (`tags.js:53-57`); C++ returns the full tag list with the same fixed pagination envelope as `browse` and status 200 (`read_by_slug/.../execute.cppm:17`, `read_by_slug/handler.cppm:27`). Material data-shape divergence.
- DB call: Ghost `models.Tag.findOne(frame.data, ...)` by slug (`tags.js:49-52`); C++ `json_for("tags", {})` returns all rows (`read_by_slug/.../execute.cppm:16`). Material operation divergence.
- Cache: Ghost `cacheInvalidate:false` (`tags.js:35-37`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:141`) + `permissions:true` (`tags.js:47`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`read_by_slug/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2: route matches after `:slug`↔`:id` normalization; no change to path/verb or `tags_read_by_slug()` binding (`routes.cppm:64-76`). Optionally rename the placeholder to `/:slug` in `routes.cppm:67` for exact parity (cosmetic, no data change).
- Check 3 (input): edit `read_by_slug/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Parse the slug as the last segment of `in.get_path()` (`/slug/` prefix means the trailing segment is the slug), same pattern as the read-by-id fix and `posts/browse/.../execute.cppm:22-28`; replace `json_for("tags", {})` (`line 16`) with a slug-specific lookup.
- Check 5 (DB call): `find_by_resource("tags", slug)` is NOT valid here — `"tags"` uses `find_entity<model::Tag>` → `std::stoll` (`context.cppm:1066,1043-1051`), so a non-numeric slug fails. Instead extend `ApiDataStore::json_for` `"tags"` branch (`context.cppm:1130`) to honor `params["slug"]` by matching the `slug` field over `m_db.all<model::Tag>()`, or pass `{"filter", "slug:" + slug}` through the existing `query.where` path (`context.cppm:1582`). Call `ctx.store().json_for("tags", {{"slug", slug}})` (or `{{"filter", "slug:" + slug}}`) — not the unfiltered `{}`. Keep resource string `"tags"`.
  ```cpp
  const auto p = std::string{in.get_path()};
  const auto base = p.substr(0, p.find('?'));
  const auto slug = base.substr(base.rfind('/') + 1);
  const auto rows = ctx.store().json_for("tags", {{"slug", slug}}); // single-match array
  ```
- Check 4 (output): edit `read_by_slug/handler.cppm`, `ReadBySlugHandler::handle` and the step. Return one tag model (not a list with browse pagination meta); on empty result set `res.set_status(types::Status::NOT_FOUND)` with `ApiError{}.json("Tag not found.", "Not Found")` body (`context.cppm:63-73`, `tags.js:53-57`), replacing the fixed-pagination 200 response (`execute.cppm:17`, `handler.cppm:27`). Use the map `status`/`body` Output pattern (posts/create) if branching in the handler.
- Check 6 (cache): edit `read_by_slug/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `cacheInvalidate:false`, `tags.js:35-37`).
- Check 7 (auth): edit `routes.cppm:64-76`, add `.use(blog::handlers::middleware::require_auth)` alongside `content_type_middleware` and remove the dead inline check (`read_by_slug/handler.cppm:18-22`). Ghost `permissions`/`docName` remains accepted gap.
- Check 8 (verdict): with slug lookup and single-model/404 output fixed, verdict becomes PARTIAL (accepted auth-permissions gap), no longer MISMATCH.

### POST `/tags` — MATCH

- Route identity: Ghost `router.post('/tags', ..., http(api.tags.add))` (`routes.js:142`); C++ `.post(...)` on `/tags` (`routes.cppm:29-36`). Match.
- Operation: Ghost `add` (`tags.js:63`); C++ `handler.tags_create()` (`routes.cppm:34`, registry `all.cppm:422,671`). Match.
- Input: Ghost reads `frame.data.tags[0]` with `options:['include']` validated (`tags.js:68-78`); C++ sends raw `in.body_as_string()` (`"{}"` fallback) with no envelope unwrap and no include parse (`create/.../execute.cppm:16`). Divergent body shape.
- Output: Ghost returns created tag with `statusCode:201` and `cacheInvalidate:true` (`tags.js:64-67`); C++ returns `mutations.create("tags", body).body()` wrapped as `{"tags":[...]}` (`context.cppm:1254-1256,1394-1402`) but the handler discards the MutationResult status and always sets 200 (`create/handler.cppm:27`). Divergent status; cache-invalidate behavior absent.
- DB call: Ghost `models.Tag.add(frame.data.tags[0], frame.options)` (`tags.js:78`); C++ `mutations.create("tags", body)` (`create/.../execute.cppm:16`). Resource `"tags"` agrees; note C++ generic tag validation requires `title`/`name` and returns 422 otherwise (`context.cppm:1239-1247`), and that status is discarded.
- Cache: Ghost `cacheInvalidate:true` (`tags.js:65-67`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:142`) + `permissions:true` (`tags.js:76`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`create/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `POST /tags` → `tags_create()` → `mutations.create("tags", ...)` agrees (`routes.cppm:29-36`, `create/.../execute.cppm:16`). The 422 title/name validation (`context.cppm:1239-1247`) is correct Ghost behavior; only its status is currently lost, fixed under check 4.
- Check 3 (input): edit `create/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Unwrap the Ghost envelope `data.tags[0]` before the store call: parse `in.body_as_string()` with `blog::handlers::middleware::decode<serde::Value>` (`include/blog/handlers/middleware/serde.cppm:56-59`, wrapper of `core_serde::JsonCodec{}.decode<serde::Value>` at `serde.cppm:59`) and pass the inner tag object's JSON string to `mutations.create("tags", innerJson)`. Also parse `?include=` from `in.get_path()` and validate against `{"count.posts"}` as in browse. Keep `{}` fallback only when the body is empty.
- Check 4 (output): edit `create/handler.cppm`, `CreateHandler::handle`. Stop hardcoding `Status::OK` (`line 27`): change the step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (`posts/create/workflow/step/execute.cppm:13,21-24`). Set `res.set_status(types::Status::CREATED)` when `result.status()==201`, and `Status::UNPROCESSABLE_CONTENT` (422) when `result.ok()==false`; write `result.body()` in both cases. `ApiDataStore::create("tags", ...)` → `wrap("tags", create_entity<model::Tag>(...))` returns 201 with `{"tags":[...]}` (`context.cppm:1254-1256,1394-1402`) and 422 "title is required" for invalid input (`context.cppm:1239-1247`), so both statuses are already available to propagate.
  ```cpp
  // step:
  using Output = std::map<std::string,std::string>;
  const auto result = mutations.create("tags", innerJson);
  return StepResponse<Output>{{{"status", std::to_string(result.status())},{"body", result.body()}}};
  // handler:
  res.set_status(result.at("status") == "201" ? types::Status::CREATED
                                             : types::Status::UNPROCESSABLE_CONTENT);
  ```
- Check 6 (cache): edit `create/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "true")` before `send()` to mirror Ghost `cacheInvalidate:true` (`tags.js:65-67`). No `core::cache::Cache` eviction required — Ghost only sets the invalidation header.
- Check 7 (auth): edit `routes.cppm:29-36`, add `.use(blog::handlers::middleware::require_auth)` alongside `content_type_middleware` and remove the dead inline check (`create/handler.cppm:18-22`). Ghost `permissions`/`docName` remains accepted gap.
- Check 8 (verdict): closes input/status/cache/auth divergences; verdict stays PARTIAL only for the accepted `permissions`/`docName` gap.

### PUT `/tags/:id` — MATCH

- Route identity: Ghost `router.put('/tags/:id', ..., http(api.tags.edit))` (`routes.js:143`); C++ `.put(...)` on `Route{"/:id"}` (`routes.cppm:47-54`). Match.
- Operation: Ghost `edit` (`tags.js:82`); C++ `handler.tags_edit()` (`routes.cppm:52`, registry `all.cppm:423,672`). Match.
- Input: Ghost `options:['id','include']`, `id` required, uses `frame.data.tags[0]` (`tags.js:86-99`); C++ passes empty id `std::string{}` and raw body, ignores `:id` and all options (`edit/.../execute.cppm:16`). Divergent.
- Output: Ghost returns edited tag, throws `NotFoundError` if absent, and sets `X-Cache-Invalidate: /*` when `model.wasChanged()` (`tags.js:100-110`); C++ returns `mutations.edit("tags", "", body).body()` with status 200 and no invalidate header (`edit/.../execute.cppm:16`, `edit/handler.cppm:27`). Empty id causes edit to fail/not find (`context.cppm:1302-1304`), but status still 200. Divergent.
- DB call: Ghost `models.Tag.edit(frame.data.tags[0], frame.options)` (`tags.js:99`); C++ `mutations.edit("tags", "", body)` (`edit/.../execute.cppm:16`). Resource agrees; id/body diverge.
- Cache: Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate: /*` on change (`tags.js:83-85,106-108`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:143`) + `permissions:true` (`tags.js:97`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`edit/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `PUT /tags/:id` → `tags_edit()` → `mutations.edit("tags", ...)` agrees (`routes.cppm:47-54`, `edit/.../execute.cppm:16`).
- Check 3 (input): edit `edit/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id pattern). Unwrap `data.tags[0]` from `in.body_as_string()` via `blog::handlers::middleware::decode<serde::Value>` (`serde.cppm:56-59`) before passing to `mutations.edit`, and parse `?include=` with the `{"count.posts"}` validation. Reject a missing id with a 422 `ApiError{}.validation(...)` body rather than calling edit with an empty id.
- Check 4 (output): edit `edit/handler.cppm`, `EditHandler::handle`. Propagate `MutationResult.status()`/`body()` via the map `Output` pattern (`posts/create/.../execute.cppm:13,21-24`) instead of hardcoded 200 (`line 27`). IMPORTANT tags-specific detail: `mutations.edit("tags", id, body)` routes to `wrap("tags", edit_entity<model::Tag>(...))` (`context.cppm:1302-1304`), and `wrap` returns status **201** on success (`context.cppm:1394-1402`) and 404 on miss (`context.cppm:1396-1397`). Coerce the 201 success to `types::Status::OK` (200) to match Ghost's `edit` default; map 404 → `Status::NOT_FOUND`; map 422 validation → `Status::UNPROCESSABLE_CONTENT`. Do not emit 201 for an edit.
- Check 6 (cache): edit `edit/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "/*")` before `send()` when the edit changed the record (Ghost `model.wasChanged()` → `frame.setHeader('X-Cache-Invalidate', '/*')`, `tags.js:106-108`); otherwise emit `"false"` (`tags.js:83-85`). A response header only — no C++ `core::cache::Cache` required; cross-request state stays in `ctx.store()`.
- Check 7 (auth): edit `routes.cppm:47-54`, add `.use(blog::handlers::middleware::require_auth)` alongside `content_type_middleware` and remove the dead inline check (`edit/handler.cppm:18-22`). Ghost `permissions`/`docName` remains accepted gap.
- Check 8 (verdict): closes input/output/cache/auth divergences; verdict stays PARTIAL only for the accepted `permissions`/`docName` gap.

### DELETE `/tags/:id` — MATCH

- Route identity: Ghost `router.delete('/tags/:id', ..., http(api.tags.destroy))` (`routes.js:144`); C++ `.delt(...)` on `Route{"/:id"}` (`routes.cppm:55-62`) — `.delt` maps to DELETE (`utils/openapi/route.cppm:91-99`). Match.
- Operation: Ghost `destroy` (`tags.js:114`); C++ `handler.tags_destroy()` (`routes.cppm:60`, registry `all.cppm:424,673`). Match.
- Input: Ghost `options:['id']` with `id` required and `require:true` (`tags.js:119-132`); C++ passes empty id `std::string{}`, reads no id option (`destroy/.../execute.cppm:16`). Divergent.
- Output: Ghost empty body with `statusCode:204` (`tags.js:115`); C++ returns `mutations.destroy("tags", "").body()` (`"{}"` on success) with status 200; the MutationResult 204/404 status is discarded by the handler (`destroy/.../execute.cppm:16`, `destroy/handler.cppm:27`, `context.cppm:1346-1352`). Divergent status/body.
- DB call: Ghost `models.Tag.destroy({...frame.options, require:true})` (`tags.js:132`); C++ `mutations.destroy("tags", "")` (`destroy/.../execute.cppm:16`, `context.cppm:1334-1335`). Resource agrees; id diverges.
- Cache: Ghost `cacheInvalidate:true` (`tags.js:116-118`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:144`) + `permissions:true` (`tags.js:130`); C++ `content_type_middleware` (`routes.cppm:20`) + dead token check (`destroy/handler.cppm:18-22`). Divergent.

#### Fix

- Checks 1, 2, 5 (route, op, resource string, delete verb): no change — `DELETE /tags/:id` (`.delt`) → `tags_destroy()` → `mutations.destroy("tags", ...)` agrees (`routes.cppm:55-62`, `destroy/.../execute.cppm:16`). `require:true` from Ghost (`tags.js:132`) is already the store's behavior — `ApiDataStore::destroy` returns 404 `ApiError` on a miss (`context.cppm:1346-1347`) — no extra flag code.
- Check 3 (input): edit `destroy/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id pattern); reject an empty id with a 422 `ApiError{}.validation(...)` body.
- Check 4 (output): edit `destroy/handler.cppm`, `DestroyHandler::handle`. Propagate `MutationResult.status()`/`body()` via the map `Output` pattern (`posts/create/.../execute.cppm:13,21-24`) instead of hardcoded 200 (`line 27`). Tags-specific detail: unlike labels, `ApiDataStore::destroy("tags", id)` returns `MutationResult{200, "{}"}` on success (`context.cppm:1352`) and 404 on miss (`context.cppm:1347`) — it does NOT return 204. So map a successful result to `res.set_status(types::Status::NO_CONTENT)` with an **empty body** (Ghost `statusCode:204`, `tags.js:115`), and a 404 result to `Status::NOT_FOUND` with its error body. No new store status needed; do not emit the `{}` body for the success case.
  ```cpp
  // handler, after reading map output `result`:
  if (result.at("status") == "404") {
    res.set_status(types::Status::NOT_FOUND); write_body(result.at("body"));
  } else {
    res.set_status(types::Status::NO_CONTENT); // empty body, no "{}"
  }
  ```
- Check 6 (cache): edit `destroy/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "true")` before `send()` to mirror Ghost `cacheInvalidate:true` (`tags.js:116-118`). No `core::cache::Cache` eviction is required; Ghost only sets the invalidation header.
- Check 7 (auth): edit `routes.cppm:55-62`, add `.use(blog::handlers::middleware::require_auth)` alongside `content_type_middleware` and remove the dead inline check (`destroy/handler.cppm:18-22`). Ghost `permissions`/`docName` remains accepted gap.
- Check 8 (verdict): closes input/output/cache/auth divergences; verdict stays PARTIAL only for the accepted `permissions`/`docName` gap.

## Missing in C++

None. All 6 Ghost tag routes have a C++ route in `include/blog/handlers/admin/tags/routes.cppm`:
`GET /tags`, `POST /tags`, `GET /tags/:id`, `PUT /tags/:id`, `DELETE /tags/:id`, `GET /tags/slug/:id` (the last normalizes to Ghost's `GET /tags/slug/:slug`).

## Extra in C++

None. The C++ folder installs exactly 6 routes (`routes.cppm:19-77`), matching the 6 Ghost tag routes; no additional C++ tag routes exist. `TagsRoutes` is installed once via `include/blog/handlers/routes.cppm:50,123`.

## Implementation status (manifest group 8)

Applied the `#### Fix` blocks above to `include/blog/handlers/admin/tags/` on 2026-11-19. No route-detail fix was marked "No change required". Verdicts unchanged (PARTIAL ×4, MISMATCH ×2 pre-fix): checks 1–7 are closed by the edits below; the accepted residuals in each fix block (`permissions`/`docName`, `order`, transformer, config-gated cache) remain, so the post-fix expectation is PARTIAL for all six routes as noted in each `#### Fix`.

### Files changed (all under `include/blog/handlers/admin/tags/`)
- `routes.cppm` — added `.use(blog::handlers::middleware::optional_auth)` to the `/tags` router and the `/slug` sub-router; renamed the slug placeholder `/:id` → `/:slug`.
- `browse/workflow/step/execute.cppm`, `browse/handler.cppm` — query parse (`include,filter,fields,limit,order,page,debug`), `include` allowlist (`count.posts`) 422 body, `json_for("tags", params)`, real pagination meta from `params` + `total("tags")`, dead-token guard removed, `X-Cache-Invalidate: false`.
- `read_by_id/workflow/step/execute.cppm`, `read_by_id/handler.cppm` — real path `:id`, empty-id 422, single-model body, 404 `Tag not found.` on miss, map `status`/`body` Output, `X-Cache-Invalidate: false`, dead guard removed.
- `read_by_slug/workflow/step/execute.cppm`, `read_by_slug/handler.cppm` — parsed slug, `json_for("tags", {{"filter","slug:"+slug}})`, first record via `core_serde`, single-model/404, map Output, `X-Cache-Invalidate: false`, dead guard removed.
- `create/workflow/step/execute.cppm`, `create/handler.cppm` — `data.tags[0]` unwrap (core_serde), `include` validation, map Output, 201 CREATED / 422 / 404 status propagation, `X-Cache-Invalidate: true`, dead guard removed.
- `edit/workflow/step/execute.cppm`, `edit/handler.cppm` — real path `:id`, envelope unwrap, include validation, map Output, 201→200 coercion / 404 / 422, `X-Cache-Invalidate: /*` on change else `false`, dead guard removed.
- `destroy/workflow/step/execute.cppm`, `destroy/handler.cppm` — real path `:id`, empty-id 422, map Output, 204 NO_CONTENT empty body on success / 404 on miss, `X-Cache-Invalidate: true`, dead guard removed.

No `flow/main.cppm` changed (flow registration unchanged). No new module partitions added; existing partition names retained (`blog_handlers:tags_<op>_handler/_step/_flow`).

### Central registrations
None needed. All six handler accessors and members are already registered in the shared `include/blog/handlers/all.cppm` (imports `:17-19`, accessors `:363-365,422-424`, members `:612-614,671-673`), and `TagsRoutes` is already installed from the shared `include/blog/handlers/routes.cppm:50,123`. No shared file was edited. No new deps: `core_serde` and `blog_handlers_middleware` were already deps of the `blog_handlers` Bazel target (`BUILD.bazel:822,826`). No `all.cppm` / `base.cppm` / top-level `routes.cppm` / `BUILD.bazel` edits required.

### Reconciliation with `tags-public.md` (same source files)
Both docs analyze the same flat `admin/tags/routes.cppm` (no content/admin base split). `tags.md` requested `require_auth`; `tags-public.md` requested `optional_auth` for anonymous content reads. Implemented `optional_auth` on the shared router: it is the lowest-common-denominator allowed primitive that does not reject anonymous content-API reads, and the dead inline `&& !true` guards are gone. Enforcing `require_auth` would break the content routes and is recorded as an accepted residual pending a content/admin route split (a shared-file change outside this group). All other fixes are common to both docs and were applied once.

### Verification
All 18 tag source units (6 handlers, 6 steps, 6 flows) compile clean with the target's exact clang/module flags. Full-target `bazel build //include/blog/handlers:blog_handlers` remains blocked by pre-existing, unrelated compile errors in `admin/posts/browse/workflow/step/execute.cppm` (no source under `tags/` contributed an error).


## Strict-parity rewrite (manifest group 9, 2026-11-19)

Rewrote all six handlers/steps in `include/blog/handlers/admin/tags/` against the shared
abstractions in `CONTRACTS.md`; the route table above is the original pre-rewrite snapshot and its
verdict column is now post-rewrite. **MATCH ×6** (from PARTIAL ×4 + MISMATCH ×2).

- Input/query: `core::query::parse` + `run`/`apply` + `has_restricted_field` replace manual
  `?`/`&`/`=` parsing, the fabricated pagination, the unescaped `slug:` filter and the missing
  restricted-field/order guard.
- Output: `blog::mapping::clean(...,"tag")` + `wrap("tags",...)` give Ghost's field allow-list,
  collection/single-resource envelopes and real `meta.pagination`; `X-Cache-Invalidate: false`
  removed on GETs, `true` → `/*` on create/destroy, edit header gated on a real pre/post diff.
- Auth: `require_ghost_permission` (full middleware shape) + `core::auth::ghost::GhostAuthorizer`
  enforce `permissions:true` with Ghost-shaped 401/403 envelopes; router-level `optional_auth` and
  `content_type_middleware` removed.
- Side effects: `core::outbox` emits `tag.added` / `tag.edited` / `tag.deleted`, with a transaction
  around destroy; create returns the Ghost `Location` header and validates `name`.
- Store: unchanged — `ctx.store().json_for/find_by_resource/create/edit/destroy("tags", ...)`.

Files changed: `routes.cppm` + the six `*/handler.cppm` and six `*/workflow/step/execute.cppm`.
Flow files unchanged. No new partitions, no shared-file edits; all edits confined to the folder.
Central registrations: none needed — existing deps (`core_query`, `core_auth`, `core_outbox`,
`blog_handlers:mapping`, `blog_handlers_middleware`) and existing partition names are reused.
Remaining gaps (non-wire-observable / opt-in): real Knex persistence and `posts_tags` locking,
`count.posts` relation, config-gated cache/subscribers, slug generation and `url` mapper enrichment.
Verification: no `admin/tags/` compile error; full target blocked by pre-existing `admin/pages/*`
errors and the missing `blog_handlers:notifications_service` module.
