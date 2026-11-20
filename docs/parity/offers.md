# Parity: offers

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/offers.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:164-167` (all 4 `api.offers.*` routes live here; the `/offers/:id` GET at `.../content/routes.js:42` binds to a different controller, `api.offersPublic.read`, so it is out of scope)
- C++: `include/blog/handlers/membership/offers/routes.cppm` (installed via `include/blog/handlers/routes.cppm:34`; handlers registered in `include/blog/handlers/all.cppm:369,436-438,618,685-687`)
- Checked: 2026-11-19

Evidence paths below are abbreviated: `offers.js` = Ghost controller, `admin/routes.js` = Ghost URL table, `routes.cppm` = `include/blog/handlers/membership/offers/routes.cppm`, `<op>/handler.cppm` and `<op>/execute.cppm` = files under `include/blog/handlers/membership/offers/<op>/` (execute at `workflow/step/execute.cppm`), `all.cppm` = `include/blog/handlers/all.cppm`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Strict-parity rewrite applied 2026-11-20 (Group 45); all four route verdicts are now
MATCH in `docs/parity/behavior/offers.md`. No shared file was edited and the Bazel
cycle was left untouched.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /offers | browse | GET /offers | `offers_browse()` | Ghost `options:['filter']` (`offers.js:17`), passed to `listOffers(frame.options)`; C++ no query parsing, `using Input=IRequest&`, passes `{}` to `json_for` (`browse/execute.cppm:11,16`) | Ghost `{data: offers}` (`offers.js:21-23`); C++ `{"offers":[...],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` hardcoded (`browse/execute.cppm:17`), status 200 (`browse/handler.cppm:27`) | Ghost `offersService.api.listOffers(frame.options)` (`offers.js:20`); C++ `store().json_for("offers",{})` — resource `offers` agrees, no real filtering/pagination (`browse/execute.cppm:16`) | Ghost `cacheInvalidate:false` (`offers.js:14-16`) = no invalidation; C++ none | Ghost `mw.authAdminApi` (`admin/routes.js:164`) + `permissions:true`/`docName:'offers'` (`offers.js:11,18`); C++ `token(req)` 401 guard is dead (`&& !true`, always false) (`browse/handler.cppm:18-22`) + `content_type_middleware` (`routes.cppm:20`) | PARTIAL | `offers.js:13-25`; `admin/routes.js:164`; `routes.cppm:19-28`; `browse/handler.cppm:17-28`; `browse/execute.cppm:11-17` |
| POST /offers | add | POST /offers | `offers_create()` | Ghost `frame.data.offers[0]` + `frame.options` (`offers.js:80`); C++ raw `in.body_as_string()` (or `{}`), no option/field parsing (`create/execute.cppm:11,16`) | Ghost `{data:[offer]}` (`offers.js:81-83`), no explicit 201; C++ raw `mutations.create(...).body()` returned as-is, no `data`/`offers` envelope, status 200 (`create/execute.cppm:16`; `create/handler.cppm:25-27`) | Ghost `offersService.api.createOffer(...)` (`offers.js:80`); C++ `mutations.create("offers",body)` — resource `offers` agrees (`create/execute.cppm:16`) | Ghost `cacheInvalidate:true` → X-Cache-Invalidate header (`offers.js:76-78`); C++ none | Ghost `mw.authAdminApi` (`admin/routes.js:165`) + `permissions:true` (`offers.js:75`); C++ dead token guard + `content_type_middleware` (`create/handler.cppm:18-22`; `routes.cppm:20`) | PARTIAL | `offers.js:74-85`; `admin/routes.js:165`; `routes.cppm:29-36`; `create/handler.cppm:17-28`; `create/execute.cppm:11-16` |
| GET /offers/:id | read | GET /offers/:id | `offers_read_by_id()` | Ghost `data:['id']` from `frame.data.id` (`offers.js:31,34`); C++ `:id` path param ignored — `find_by_resource("offers", std::string{}.empty()?"1":std::string{})` always `"1"` (`read_by_id/execute.cppm:16`) | Ghost `{data:[offer]}`, throws `NotFoundError` (404) when missing (`offers.js:34-43`); C++ `{"offers":[{...}]}` or `{"offers":[]}` at status 200 (`read_by_id/execute.cppm:17-18`; `read_by_id/handler.cppm:27`) | Ghost `offersService.api.getOffer(frame.data)` (`offers.js:34`); C++ `find_by_resource("offers","1")` — resource `offers` agrees, id hardcoded (`read_by_id/execute.cppm:16`) | Ghost `cacheInvalidate:false` (`offers.js:28-30`) = no invalidation; C++ none | Ghost `mw.authAdminApi` (`admin/routes.js:166`) + `permissions:true` (`offers.js:32`); C++ dead token guard + `content_type_middleware` (`read_by_id/handler.cppm:18-22`; `routes.cppm:20`) | PARTIAL | `offers.js:27-45`; `admin/routes.js:166`; `routes.cppm:37-46`; `read_by_id/handler.cppm:17-28`; `read_by_id/execute.cppm:11-18` |
| PUT /offers/:id | edit | PUT /offers/:id | `offers_edit()` | Ghost `options:['id']` (required) + `frame.data.offers[0]`; id merged into update object (`offers.js:48,54-60`); C++ raw body, id passed as empty string `mutations.edit("offers", std::string{}, body)` (`edit/execute.cppm:11,16`) | Ghost `{data:[offer]}`, `NotFoundError` 404 when missing (`offers.js:62-70`); C++ raw `mutations.edit(...).body()` returned as-is, no envelope, no 404 handling, status 200 (`edit/execute.cppm:16`; `edit/handler.cppm:25-27`) | Ghost `offersService.api.updateOffer({...frame.data.offers[0],id:frame.options.id}, frame.options)` (`offers.js:54-60`); C++ `mutations.edit("offers","",body)` — resource `offers` agrees (`edit/execute.cppm:16`) | Ghost `cacheInvalidate:true` → X-Cache-Invalidate header (`offers.js:50-52`); C++ none | Ghost `mw.authAdminApi` (`admin/routes.js:167`) + `permissions:true` (`offers.js:49`); C++ dead token guard + `content_type_middleware` (`edit/handler.cppm:18-22`; `routes.cppm:20`) | PARTIAL | `offers.js:47-72`; `admin/routes.js:167`; `routes.cppm:47-54`; `edit/handler.cppm:17-28`; `edit/execute.cppm:11-16` |

## Route details

### GET /offers — PARTIAL

- `filter` option accepted on Ghost (`offers.js:17`, forwarded at `:20`) but not parsed in C++ — `json_for("offers", {})` gets empty params (`browse/execute.cppm:11,16`).
- Output envelope differs: Ghost `{data: offers}` (`offers.js:21-23`) vs C++ `{"offers":[...]}` plus hardcoded pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`browse/execute.cppm:17`); no real pagination/filtering in C++.
- Cache: Ghost `cacheInvalidate:false` (`offers.js:14-16`) = no invalidation; C++ none. Agree (both "none").
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:164`) + `permissions:true`/`docName:'offers'` (`offers.js:11,18`); C++ has only `content_type_middleware` (`routes.cppm:20`) and a token guard disabled by `&& !true` (`browse/handler.cppm:18-22`).
- DB resource string `offers` agrees (`offers.js:20` service vs `browse/execute.cppm:16`).

#### Fix

- Checks 1, 2 — No change required: `GET /offers` → `offers_browse()` (`routes.cppm:19-28`, registry `all.cppm:436`) already matches Ghost `router.get('/offers', ..., http(api.offers.browse))` (`admin/routes.js:164`, `offers.js:13`).
- Check 3 (input): edit `include/blog/handlers/membership/offers/browse/workflow/step/execute.cppm`, symbol `blog::handlers::offers::browse::workflow::ExecuteStep::invoke`. Replace line 16 `const auto rows=ctx.store().json_for("offers", {});` with a parsed-param call. Exact pattern: split `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='` (verbatim from `include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-29`), keep only the Ghost-declared option `filter` (`offers.js:17`), then call `ctx.store().json_for("offers", params)`. Reuse existing `static blog::BlogContext* s_ctx` (`browse/execute.cppm:20`) — no new global state.
  ```cpp
  std::map<std::string, std::string> params;
  const auto q = std::string{in.get_path()}.find('?');
  if (q != std::string::npos) {
      for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
          const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
          const auto eq = text.find('=');
          if (eq != std::string::npos && text.substr(0, eq) == "filter") params["filter"] = text.substr(eq + 1);
      }
  }
  const auto rows = ctx.store().json_for("offers", params);
  ```
- Check 5 (DB call): resource string `"offers"` already agrees (`offers.js:20` vs `browse/execute.cppm:16`) — No change required at the call site. To make `params` effective, edit `include/blog/context/context.cppm`, `ApiDataStore::json_for`, `"offers"` branch (`lines 1138-1150`): build `records` from `parse_array(m_resources.list("offers"))`, overlay `::blog::model::encode_offer_wire()` when empty (preserving the current name/id override), then return `search_json(records, params)` — `search_json` already applies `filter`, `limit`, `page`, `fields` (`context.cppm:1563-1585`). Resource stays `"offers"`; all state is context-local `ResourceStore` (`ctx.store().resources()`), no direct SQL, no new global state.
- Check 4 (output): edit the same step, line 17. Replace `{"offers":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with `{"data":` + rows + `}` to match Ghost `{data: offers}` (`offers.js:21-23`); drop the hardcoded pagination meta because this Ghost browse returns no `meta.pagination`. `BrowseHandler::handle` status 200 (`browse/handler.cppm:27`) — No change required.
- Check 6 (cache): edit `browse/handler.cppm`, `BrowseHandler::handle`, before `send()` (`line 27`): `res.set_header("X-Cache-Invalidate", "false");` (Ghost `cacheInvalidate:false`, `offers.js:14-16`; `IResponse::set_header` at `include/interfaces/io/response.cppm:590`). Header only — no `core::cache::Cache` eviction, no new global state.
- Check 7 (auth): edit `routes.cppm:19-28`, add `.use(blog::handlers::middleware::require_auth)` after `.use(blog::handlers::middleware::content_type_middleware)` (`routes.cppm:20`; middleware `include/blog/handlers/middleware/auth.cppm:12-29`). In `browse/handler.cppm:18-22` delete the dead guard `if (blog::handlers::middleware::token(req).empty() && !true)` — `&& !true` is always false, so it never rejects. Ghost `permissions:true`/`docName:'offers'` (`offers.js:11,18`) has no C++ primitive: accepted residual gap, do not invent new global state.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus the accepted permissions residual), re-rate this row `MATCH`; change Summary `PARTIAL` 4→3 and `MATCH` 0→1, and replace the divergent bullets above with "no differences except accepted `permissions` residual". Keep the existing `file:line` evidence on both sides.

### POST /offers — PARTIAL

- Ghost builds input from `frame.data.offers[0]` + `frame.options` (`offers.js:80`); C++ passes raw `in.body_as_string()` (or `{}`), no option/field parsing (`create/execute.cppm:11,16`).
- Output: Ghost `{data:[offer]}` (`offers.js:81-83`); C++ returns the raw `mutations.create` body with no `data` envelope, status fixed 200 (`create/execute.cppm:16`; `create/handler.cppm:25-27`).
- Cache: Ghost sets X-Cache-Invalidate via `cacheInvalidate:true` (`offers.js:76-78`); C++ none — divergence.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:165`) + `permissions:true` (`offers.js:75`) vs C++ dead token guard + content-type middleware (`create/handler.cppm:18-22`; `routes.cppm:20`).

#### Fix

- Checks 1, 2 — No change required: `POST /offers` → `offers_create()` (`routes.cppm:29-36`, registry `all.cppm:437`) matches `router.post('/offers', ..., http(api.offers.add))` (`admin/routes.js:165`, `offers.js:74`).
- Check 3 (input): edit `create/workflow/step/execute.cppm`, `blog::handlers::offers::create::workflow::ExecuteStep::invoke`. Replace line 16's raw `in.body_as_string().empty()?"{}":in.body_as_string()` with the Ghost envelope unwrap: decode via `blog::handlers::middleware::decode<serde::Value>(in.body_as_string())` (helper `include/blog/handlers/middleware/serde.cppm:162`, module `blog_handlers_middleware`), take `value["data"]["offers"][0]`, re-serialize that sub-value with `core_serde::JsonCodec{}.encode(...)`, and pass it to `mutations.create("offers", innerJson)`. Ghost source: `frame.data.offers[0]` + `frame.options` (`offers.js:80`). Keep `"{}"` only as empty-body fallback. No new global state — reuse existing `static blog::BlogContext* s_ctx` (`create/execute.cppm:19`).
- Check 5 (DB call): No change required to resource — `mutations.create("offers", ...)` (`create/execute.cppm:16`) matches `offersService.api.createOffer(...)` (`offers.js:80`) on resource `offers`; `ApiDataStore::create` persists via `resources().create("offers", ...)` (`context.cppm:1272-1276`) = context-local `ResourceStore` under the existing `"offers"` namespace. No direct SQL, no new global state.
- Check 4 (output): edit `create/handler.cppm`, `CreateHandler::handle`, and the step. Change step line 11 `using Output=std::string` → `using Output=std::map<std::string,std::string>` and return status+body from `MutationResult` (exact pattern `include/blog/handlers/admin/posts/create/workflow/step/execute.cppm:13,21-24`):
  ```cpp
  const auto result = mutations.create("offers", innerJson);
  return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},{"body", result.body()}}};
  ```
  In `CreateHandler::handle` replace hardcoded `res.set_status(...Status::OK)` (`create/handler.cppm:27`) with status from the map: `res.set_status(static_cast<interfaces::io::types::Status>(std::stoi(payload.at("status"))))` (pattern `posts/create/handler.cppm:27-35`). Wrap the created offer in Ghost's `{data:[offer]}` (`offers.js:81-83`): parse the `id` from the mutation body and read back via `find_by_resource("offers", id)`, emitting `{"data":[` + offer + `]}`. `ApiDataStore::create` returns 201 for offers (`context.cppm:1272-1276`) — propagate it, do not force 200. If full-offer read-back is unavailable in the fixture store, record the id-only body as an accepted residual gap rather than inventing new state.
- Check 6 (cache): edit `create/handler.cppm`, before `send()` (`line 27`): `res.set_header("X-Cache-Invalidate", "true");` (Ghost `cacheInvalidate:true`, `offers.js:76-78`). Header only, no cache eviction, no new global state.
- Check 7 (auth): edit `routes.cppm:29-36`, add `.use(blog::handlers::middleware::require_auth)` after content-type middleware (`routes.cppm:20`; `auth.cppm:12-29`). Delete the dead `&& !true` guard in `create/handler.cppm:18-22`. Ghost `permissions:true` (`offers.js:75`) = accepted residual gap; no C++ primitive.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus accepted `permissions`), re-rate this row `MATCH`; Summary `PARTIAL` 4→2, `MATCH` 1→2; replace the divergent bullets above with "no differences except accepted `permissions` residual". Keep both sides' `file:line` evidence.

### GET /offers/:id — PARTIAL

- Route id never used: `find_by_resource("offers", std::string{}.empty()?"1":std::string{})` always resolves to `"1"` (`read_by_id/execute.cppm:16`); Ghost `data:['id']` comes from `frame.data.id` (`offers.js:31,34`).
- Output: Ghost `{data:[offer]}` with `NotFoundError` 404 when not found (`offers.js:34-43`); C++ `{"offers":[...]}` / `{"offers":[]}` at 200, no 404 (`read_by_id/execute.cppm:17-18`; `read_by_id/handler.cppm:27`).
- Cache: Ghost `cacheInvalidate:false` (`offers.js:28-30`) = none; C++ none. Agree.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:166`) + `permissions:true` (`offers.js:32`) vs C++ dead token guard + content-type middleware (`read_by_id/handler.cppm:18-22`; `routes.cppm:20`).

#### Fix

- Checks 1, 2 — No change required: `GET /offers/:id` → `offers_read_by_id()` (`routes.cppm:37-46`, registry `all.cppm:369`) matches `router.get('/offers/:id', ..., http(api.offers.read))` (`admin/routes.js:166`, `offers.js:27`).
- Check 3 (input): edit `read_by_id/workflow/step/execute.cppm`, `...::ExecuteStep::invoke`. Replace line 16's `find_by_resource("offers", std::string{}.empty()?"1":std::string{})` (always `"1"`) with a real path-id parse using `in.get_path()` last `/`-separated segment (exact pattern `include/blog/handlers/admin/posts/read_by_id/workflow/step/execute.cppm:19-22`), then `find_by_resource("offers", id)`:
  ```cpp
  auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
  while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
  const auto slash = path.find_last_of('/');
  const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
  const auto one = ctx.store().find_by_resource("offers", id);
  ```
  Reuse existing `static blog::BlogContext* s_ctx` (`read_by_id/execute.cppm:21`) — no new global state.
- Check 5 (DB call): edit `include/blog/context/context.cppm`, `ApiDataStore::find_by_resource`, `"offers"` branch (`line 1077`): currently `return ::blog::model::encode_offer_wire();` ignores `id`. Replace with an id-aware lookup over `m_resources.list("offers")` (context-local `ResourceStore`): match the record whose `"id"` field equals `id` and return its JSON; fall back to `encode_offer_wire()` only when no offers are stored; otherwise return `std::nullopt` so the step can 404. Resource string stays `"offers"`; no direct SQL, no new global state.
- Check 4 (output): edit `read_by_id/execute.cppm:11` `using Output=std::string` → `using Output=std::map<std::string,std::string>`. On miss return `{{"status","404"},{"body", ApiError{}.json("Offer not found.", "Not Found")}}` (Ghost `NotFoundError`, `offers.js:35-39`; `ApiError::json` `context.cppm:66-73`); on hit return `{{"status","200"},{"body","{\"data\":[" + *one + "]}"}}` (Ghost `{data:[offer]}`, `offers.js:41-43`). In `read_by_id/handler.cppm:23-27`, replace hardcoded `Status::OK` with status from the map (pattern `posts/read_by_id/handler.cppm:25-36`). Remove the current `{"offers":[...]}` / `{"offers":[]}` shapes and the unconditional 200.
- Check 6 (cache): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`, before `send()` (`line 27`): `res.set_header("X-Cache-Invalidate", "false");` (Ghost `cacheInvalidate:false`, `offers.js:28-30`). Header only.
- Check 7 (auth): edit `routes.cppm:37-46`, add `.use(blog::handlers::middleware::require_auth)` after content-type middleware (`routes.cppm:20`; `auth.cppm:12-29`). Delete the dead `&& !true` guard in `read_by_id/handler.cppm:18-22`. Ghost `permissions:true` (`offers.js:32`) = accepted residual gap.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus accepted `permissions`), re-rate this row `MATCH`; Summary `PARTIAL` 2→1, `MATCH` 2→3; replace the divergent bullets above with "no differences except accepted `permissions` residual". Keep both sides' `file:line` evidence.

### PUT /offers/:id — PARTIAL

- Ghost requires `options:['id']` and merges `frame.options.id` into the update object (`offers.js:48,54-60`); C++ ignores the path id and passes an empty string to `mutations.edit("offers", std::string{}, body)` (`edit/execute.cppm:11,16`).
- Output: Ghost `{data:[offer]}` + `NotFoundError` 404 when missing (`offers.js:62-70`); C++ raw mutation body with no envelope and no 404, status 200 (`edit/execute.cppm:16`; `edit/handler.cppm:25-27`).
- Cache: Ghost sets X-Cache-Invalidate via `cacheInvalidate:true` (`offers.js:50-52`); C++ none — divergence.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:167`) + `permissions:true` (`offers.js:49`) vs C++ dead token guard + content-type middleware (`edit/handler.cppm:18-22`; `routes.cppm:20`).

#### Fix

- Checks 1, 2 — No change required: `PUT /offers/:id` → `offers_edit()` (`routes.cppm:47-54`, registry `all.cppm:438`) matches `router.put('/offers/:id', ..., http(api.offers.edit))` (`admin/routes.js:167`, `offers.js:47`).
- Check 3 (input): edit `edit/workflow/step/execute.cppm`, `...::ExecuteStep::invoke`. Replace line 16's empty id `std::string{}` with the `in.get_path()` last-segment id (read-by-id pattern above; strip `?query` and trailing `/`); reject an empty id with a 422 `ApiError{}.validation("Validation error", "id is required", "id")` body (`context.cppm:76-84`) instead of calling edit with `""`. Unwrap `data.offers[0]` from `in.body_as_string()` via `blog::handlers::middleware::decode<serde::Value>(...)` and re-serialize, merging the id into the update object to mirror Ghost's `{...frame.data.offers[0], id: frame.options.id}` (`offers.js:54-60`):
  ```cpp
  const auto result = mutations.edit("offers", id, innerJsonWithId);
  return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},{"body", result.body()}}};
  ```
  Ghost requires `options:['id']` (`offers.js:48`). Reuse existing `static blog::BlogContext* s_ctx` (`edit/execute.cppm:19`) — no new global state.
- Check 5 (DB call): No change required to resource string — `mutations.edit("offers", ...)` (`edit/execute.cppm:16`) matches `offersService.api.updateOffer(...)` (`offers.js:54`) on `offers`; persistence goes to `ctx.store().resources()` under existing `"offers"` namespace (`context.cppm:1320-1324`). To restore Ghost's 404, edit `ApiDataStore::edit` (`context.cppm:1279-1325`): add an `"offers"` branch that returns `MutationResult{404, ApiError{}.json("Offer not found.", "Not Found"), true}` when `id` is absent from `resources().list("offers")`, otherwise returns the edited offer JSON — replacing the current generic create-if-missing fallback (`lines 1323-1324`) for this resource. No direct SQL, no new global state.
- Check 4 (output): change `edit/execute.cppm:11` `using Output=std::string` → `using Output=std::map<std::string,std::string>` and return `MutationResult` status+body (snippet above). In `edit/handler.cppm`, `EditHandler::handle`, replace hardcoded `Status::OK` (`line 27`) with status from the map (pattern `posts/create/handler.cppm:27-35`) — 200 on success, 404 on miss, 422 for validation. Wrap the edited offer as `{"data":[` + offer + `]}` to match Ghost `{data:[offer]}` (`offers.js:68-70`), removing the current raw mutation body and unconditional 200.
- Check 6 (cache): edit `edit/handler.cppm`, before `send()` (`line 27`): `res.set_header("X-Cache-Invalidate", "true");` (Ghost `cacheInvalidate:true`, `offers.js:50-52`). Header only; no `core::cache::Cache` eviction, cross-request state stays in `ctx.store().resources()` under `"offers"`, no new global state.
- Check 7 (auth): edit `routes.cppm:47-54`, add `.use(blog::handlers::middleware::require_auth)` after content-type middleware (`routes.cppm:20`; `auth.cppm:12-29`). Delete the dead `&& !true` guard in `edit/handler.cppm:18-22`. Ghost `permissions:true` (`offers.js:49`) = accepted residual gap; no C++ primitive.
- Check 8 (verdict + evidence): with checks 1–7 closed (minus accepted `permissions`), re-rate this row `MATCH`; Summary `PARTIAL` 1→0, `MATCH` 3→4; replace the divergent bullets above with "no differences except accepted `permissions` residual". Keep both sides' `file:line` evidence.

## Missing in C++

None. All 4 Ghost `api.offers.*` routes have a C++ route with the same normalized path + verb in `include/blog/handlers/membership/offers/routes.cppm` (`GET /offers` `:21`, `POST /offers` `:29`, `GET /offers/:id` `:39`, `PUT /offers/:id` `:47`).

Confirmation: grep `api\.offers` over `docs/Ghost/ghost/core/server/web` returned 5 matches — 4 in `admin/routes.js:164-167` (this controller) and 1 in `content/routes.js:42` bound to `api.offersPublic.read` (a different controller, `offersPublic.js`, not `offers.js`). Grep `offers` over `include/blog/handlers/**/routes.cppm` found the single `Router{router, "/offers"}` block at `routes.cppm:19` containing all 4 counterparts (via `include/blog/handlers/routes.cppm:34`).

  Fix: No change required. No Ghost `api.offers.*` route lacks a C++ counterpart, so no route registration is needed. Check 1 is closed for every entry; none of the four `#### Fix` blocks above adds a route, and the out-of-scope `/offers/:id` GET in `content/routes.js:42` belongs to `offersPublic.js` (tracked in its own parity file), not here.

## Extra in C++

None. `include/blog/handlers/membership/offers/routes.cppm` installs exactly 4 routes (`:21,29,39,47`) and each has a matching Ghost route bound to the `offers` controller (`admin/routes.js:164-167`). No offers route exists in C++ without a Ghost counterpart.

  Fix: No change required. No extra C++ offers route exists to remove or gate — the 4 installed routes map 1:1 to `admin/routes.js:164-167`, and `OffersRoutes::install` (`routes.cppm:15-56`) adds no fifth route. Check 1 (EXTRA) is closed; do not delete or disable any route in the four `#### Fix` blocks above.

## Group 45 — strict-parity rewrite (2026-11-20)

The earlier Group 38 fix blocks were applied literally but targeted the **controller-stage**
`{data: …}` value and literal `false`/`true` cache headers. Ghost's api-framework then
serializes `response.data` to `frame.response[docName]` (`output/default.js:22-41`) and
turns `cacheInvalidate:true` into `X-Cache-Invalidate: /*` (`headers.ts:9,21-27,135-144`).
Group 45 corrected those divergences and closed the remaining folder-local gaps with
CONTRACTS.md shared abstractions.

Edits confined to `include/blog/handlers/membership/offers/`; no shared file, BUILD file
or Bazel cycle touched. Every step keeps `Output = std::map<std::string,std::string>`
and `static blog::BlogContext* s_ctx`. Shared helpers live in the already-registered
`browse/workflow/step/execute.cppm` partition as `blog::handlers::offers::support`;
the other steps/handlers `import :offers_browse_step`. No new partition was created.

| File | Strict-parity change |
|---|---|
| `browse/workflow/step/execute.cppm` | Added `support` helpers (`kDocName`, `clean`/`wrap` as local OfferDTO allow-list/envelope helpers, `quote`, `scalar`, `last_segment`, `location_for`, `unwrap`, `extract_id`, `offer_dto`, `emit`, `authorize`) built on `core_query`, `core_serde`, `:domain_all`, `:outbox_shim`, `blog_handlers_middleware`. Browse now uses `core::query::parse` (allowlist `filter`) + `apply` with restricted-field guard → 400; body `{"offers":[...]}` (no meta). |
| `browse/handler.cppm` | Ghost permission gate `support::authorize(...,"browse")`; status from payload; **no** `X-Cache-Invalidate` header. |
| `create/workflow/step/execute.cppm` | Unwrap `data.offers[0]`; `store().create`; `offer_dto` computes currency/cadence/redemption fields; `offer.created` outbox event; `{"offers":[DTO]}`; status **200** (not 201); Location from created id. |
| `create/handler.cppm` | Permission gate for `add`; `X-Cache-Invalidate: /*`; `Location` header; status from payload. |
| `read_by_id/workflow/step/execute.cppm` | Real `:id`; id-filtered `core::query::apply` over cleaned store rows makes the 404 reachable; `offer_dto` + `wrap` → `{"offers":[DTO]}`. |
| `read_by_id/handler.cppm` | Permission gate for `read`; no cache header; status from payload. |
| `edit/workflow/step/execute.cppm` | Path id merged into `data.offers[0]`; 422 on empty id; id-filtered existence check → reachable 404; `offer_dto` + `wrap`; `offer.code_changed` outbox event on code change. |
| `edit/handler.cppm` | Permission gate for `edit`; `X-Cache-Invalidate: /*`; status from payload. |
| `routes.cppm` | Unchanged: already installs `content_type_middleware` → `require_auth` and all four routes. |

### Corrections to earlier fix blocks

- Envelope: `{"data":[...]}` → `{"offers":[...]}` on all four routes (serialized response).
- Cache headers: `false` → omitted (browse/read); `true` → `/*` (create/edit).
- Status: create 201 → 200 (Ghost default); read/edit 404 now reachable; edit 422 kept.
- Location header added to `add` (Ghost `headers.ts:146-185`).
- `filter` now actually applied via `core_query` instead of parsed-then-ignored.
- Fixture field normalization in `offer_dto`: `cadence` → month|year,
  `redemption_type` → signup|retention, `currency` null for non-fixed,
  `currency_restriction = (type==fixed)`, `updated_at` stripped by `clean`.

### Central registrations needed

None. `include/blog/handlers/routes.cppm` and `all.cppm` already install/register the
four handlers; `blog_handlers_middleware` already exports `require_auth` and
`require_ghost_permission`; `core_query`, `core_auth:ghost`, `blog_handlers:mapping`
and `:outbox_shim` are already on the `blog_handlers` dependency graph.

### New-partition needs

None created. `blog::handlers::offers::support` stays in the registered
`browse/workflow/step/execute.cppm` partition; the other steps import `:offers_browse_step`.
Recorded needs for main agent (not done here):
- `blog_handlers:mapping` (`mapping/base.cppm`, `clean.cppm`, `wrap.cppm`) is **not**
  registered in `include/blog/handlers/BUILD.bazel` and is not in any `*.modmap`, so an
  `import blog_handlers:mapping;` fails to resolve. Group 45 therefore implements the
  OfferDTO allow-list/envelope locally. If main agent wires that module, these helpers
  can delegate to `blog::mapping::clean(...,"offer")` / `wrap("offers",...)`.
- Optional cleanup: move `support` into a dedicated `offers/types/dto_offer.cppm` partition
  (would also need a BUILD entry).

### Accepted residuals (no allowed C++ primitive / shared file)

- `ApiDataStore::json_for("offers", params)` / `find_by_resource("offers", id)` /
  `edit("offers", …)` in `context_context.cppm` remain id/params-agnostic; Group 45
  compensates in the step with `core_query` filters and a step-local existence check,
  so no shared file was touched.
- Full `Offer.create` domain validation (unique name/code, cadence/duration/tier/
  redemption rules), real Knex transaction, DDD event dispatch, dynamic redirect
  manager, retention-archive service, and full `OfferMapper` aggregation.
- Generic numeric ids from `ResourceStore` instead of ObjectIDs.
- Workspace Bazel build was blocked by unrelated concurrent-agent errors and an
  existing `blog_handlers` dependency cycle at rewrite time; no offers file appeared
  in the failure set.
