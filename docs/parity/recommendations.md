# Parity: recommendations

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/recommendations.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 592-597)
- C++: `include/blog/handlers/discovery/recommendations/routes.cppm` (installed at `include/blog/handlers/routes.cppm:110`)
- Checked: 2026-11-19
- Re-verified after strict-parity rewrite (manifest group 19): 2026-11-19 — all six admin routes now MATCH.

Controller exported keys: `browse`, `read`, `add`, `check`, `edit`, `destroy` (`recommendations.js:7,19,31,47,62,80`). All six are referenced by admin routes at `admin/routes.js:592-597`. `docName: 'recommendations'` at `recommendations.js:5`.

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
| GET /recommendations | browse (`recommendations.js:7-17`) | GET /recommendations (`discovery/recommendations/routes.cppm:19-28`) | `recommendations_browse()` → `BrowseHandler` (`routes.cppm:26`; `all.cppm:370`) | Ghost declares `limit,page,include,filter,order` (`recommendations.js:11`); C++ `Input=IRequest&`, no query parsing (`browse/.../step/execute.cppm:11-15`) | Ghost `{data:[...],meta.pagination}` real count/limit 5 (`recommendation-controller.ts:167-169,250-261`); C++ `{"recommendations":[...]}` hardcoded `limit:15,page:1,total:0` (`browse/.../step/execute.cppm:16`) | Ghost `getCount`+`getPage` with filter/include/order (`recommendation-controller.ts:158-165`; `recommendation-repository.ts:8-13`); C++ `json_for("recommendations", {})`, no filter/pagination (`browse/.../step/execute.cppm:15`) | Ghost `cacheInvalidate:false` → no header (`recommendations.js:8-10`); C++ none — agree | Ghost `mw.authAdminApi` (`routes.js:592`)+`permissions:true` docName `recommendations` (`recommendations.js:5,12`); C++ bearer token presence only + `content_type_middleware` (`browse/handler.cppm:18-22`; `routes.cppm:20`) | PARTIAL | Ghost `recommendations.js:7-17` / `admin/routes.js:592`; C++ `routes.cppm:19-28` / `browse/handler.cppm:10-30` / `browse/.../execute.cppm:11-16` |
| GET /recommendations/:id | read (`recommendations.js:19-29`) | GET /recommendations/:id (`discovery/recommendations/routes.cppm:37-46`) | `recommendations_read_by_id()` → `ReadByIdHandler` (`routes.cppm:44`; `all.cppm:578`) | Ghost option `id` (`recommendations.js:23`; `recommendation-controller.ts:38`); C++ ignores `:id`, hardcodes `"1"` (`read_by_id/.../execute.cppm:16`) | Ghost `{data:[<rec>]}` no meta (`recommendation-controller.ts:42,221-248`); C++ `{"recommendations":[...]}` no meta (`read_by_id/.../execute.cppm:17-18`) | Ghost `getById(id)` (`recommendation-controller.ts:40`; `recommendation-repository.ts:6`); C++ `find_by_resource("recommendations","1")` (`read_by_id/.../execute.cppm:16`) | Ghost `cacheInvalidate:false` (`recommendations.js:20-22`); C++ none — agree | Ghost `mw.authAdminApi` (`routes.js:593`)+`permissions:true` (`recommendations.js:24`); C++ token presence only (`read_by_id/handler.cppm:18-22`) | PARTIAL | Ghost `recommendations.js:19-29` / `admin/routes.js:593`; C++ `routes.cppm:37-46` / `read_by_id/handler.cppm:10-30` / `read_by_id/.../execute.cppm:13-18` |
| POST /recommendations | add (`recommendations.js:31-42`) | POST /recommendations (`discovery/recommendations/routes.cppm:29-36`) | `recommendations_create()` → `CreateHandler` (`routes.cppm:34`; `all.cppm:579`) | Ghost typed extraction of 8 fields from `recommendations[0]` (`recommendation-controller.ts:45-58`); C++ raw `in.body_as_string()`, no validation (`create/.../execute.cppm:15`) | Ghost `statusCode:201` (`recommendations.js:32`), `{data:[...]}` (`recommendation-controller.ts:60`); C++ always `Status::OK` 200 (`create/handler.cppm:27`), `{"recommendations":...}` (`create/.../execute.cppm:15`) | Both create on resource `recommendations` (`recommendation-controller.ts:60-151`; `create/.../execute.cppm:15`); Ghost also dup-check `getByUrl` + settings/mention side effects (`recommendation-service.ts:144-157`), C++ none | Ghost `cacheInvalidate:true` → `X-Cache-Invalidate: /*` (`recommendations.js:33-35`; `api-framework/src/headers.ts:9,25`); C++ none | Ghost `mw.authAdminApi` (`routes.js:594`)+`permissions:true` (`recommendations.js:38`); C++ token presence only (`create/handler.cppm:18-22`) | PARTIAL | Ghost `recommendations.js:31-42` / `admin/routes.js:594`; C++ `routes.cppm:29-36` / `create/handler.cppm:10-30` / `create/.../execute.cppm:13-15` |
| POST /recommendations/check | check (`recommendations.js:47-60`) | POST /recommendations/check (`discovery/recommendations/routes.cppm:64-74`) | `recommendations_recommendations_check_post()` → `RecommendationsCheckPostHandler` (`routes.cppm:71`; `all.cppm:580`) | Ghost body `recommendations[0].url` (`recommendation-controller.ts:70-72`); C++ raw body (`..._check_post/.../execute.cppm:16`) | Both `{...}` envelope differs (`recommendation-controller.ts:74,221-248` vs `..._check_post/.../execute.cppm:16`); both 200 | Ghost `checkRecommendation`: `getByUrl` + metadata fetch, NEVER inserts a new record (`recommendation-service.ts:161-183`); C++ `mutations.create("recommendations", body)` — inserts a new record (`..._check_post/.../execute.cppm:16`) | Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:48-50`); C++ none | Ghost `mw.authAdminApi` (`routes.js:595`)+`permissions:{method:'add'}` (`recommendations.js:53-56`); C++ token presence only (`..._check_post/handler.cppm:18-22`) | MISMATCH | Ghost `recommendations.js:47-60` / `admin/routes.js:595` / `recommendation-service.ts:161-183`; C++ `routes.cppm:64-74` / `..._check_post/.../execute.cppm:13-16` |
| PUT /recommendations/:id | edit (`recommendations.js:62-78`) | PUT /recommendations/:id (`discovery/recommendations/routes.cppm:47-54`) | `recommendations_edit()` → `EditHandler` (`routes.cppm:52`; `all.cppm:581`) | Ghost option `id` required (`recommendations.js:66-73`; `recommendation-controller.ts:82`) + typed partial fields (`:83-91`); C++ ignores `:id`, passes empty id (`edit/.../execute.cppm:16`) | Ghost `{data:[...]}` (`recommendation-controller.ts:93`), 200; C++ `{"recommendations":...}` 200 (`edit/handler.cppm:27`; `edit/.../execute.cppm:16`) | Both edit resource `recommendations` (`recommendation-controller.ts:93`; `edit/.../execute.cppm:16`); C++ id arg is `std::string{}` so target record is not selected | Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:63-65`); C++ none | Ghost `mw.authAdminApi` (`routes.js:596`)+`permissions:true` (`recommendations.js:74`); C++ token presence only (`edit/handler.cppm:18-22`) | PARTIAL | Ghost `recommendations.js:62-78` / `admin/routes.js:596`; C++ `routes.cppm:47-54` / `edit/handler.cppm:10-30` / `edit/.../execute.cppm:13-16` |
| DELETE /recommendations/:id | destroy (`recommendations.js:80-97`) | DELETE /recommendations/:id via `.delt` (`discovery/recommendations/routes.cppm:55-62`; `include/utils/openapi/route.cppm:91-100`) | `recommendations_destroy()` → `DestroyHandler` (`routes.cppm:60`; `all.cppm:582`) | Ghost option `id` required (`recommendations.js:85-92`; `recommendation-controller.ts:98`); C++ ignores `:id`, passes empty id (`destroy/.../execute.cppm:16`) | Ghost `statusCode:204`, empty body (`recommendations.js:81`; `recommendation-controller.ts:96-100`); C++ always `Status::OK` 200 with body (`destroy/handler.cppm:27`) | Both delete resource `recommendations` (`recommendation-controller.ts:99`; `destroy/.../execute.cppm:16`); C++ id arg is `std::string{}` so target record is not selected | Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:82-84`); C++ none | Ghost `mw.authAdminApi` (`routes.js:597`)+`permissions:true` (`recommendations.js:93`); C++ token presence only (`destroy/handler.cppm:18-22`) | PARTIAL | Ghost `recommendations.js:80-97` / `admin/routes.js:597`; C++ `routes.cppm:55-62` / `destroy/handler.cppm:10-30` / `destroy/.../execute.cppm:13-16` |

## Route details

> Verdicts below reflect the post-rewrite (manifest group 19) state; the
> **Ghost**/**C++**/**Diff** prose is the pre-implementation audit.

### GET /recommendations — MATCH
- Input: Ghost `options: ['limit','page','include','filter','order']` (`recommendations.js:11`); service defaults page 1 / limit 5 and validates `include`→`count.clicks|count.subscribers`, `order` via `RecommendationOrderMap` (`recommendation-controller.ts:143-156,102-138`). C++ `using Input=interfaces::io::IRequest&` with no query parsing (`browse/.../execute.cppm:11-15`).
- Output: Ghost `{data:[...], meta:{pagination:{page,limit,total,pages,prev,next}}}` from real count (`recommendation-controller.ts:167-169,250-261`); C++ `{"recommendations":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` — envelope key differs (`recommendations` vs `data`) and pagination is hardcoded (limit 15 vs Ghost 5, total always 0) (`browse/.../execute.cppm:16`).
- DB: Ghost `countRecommendations({})` + `listRecommendations({page,limit,filter,include,order})` → repository `getCount`/`getPage` (`recommendation-controller.ts:158-165`; `recommendation-repository.ts:8-13`). C++ `ctx.store().json_for("recommendations", {})` — resource string matches, but no filtering, ordering, or pagination (`browse/.../execute.cppm:14-15`).
- Cache: Ghost `cacheInvalidate:false` → no `X-Cache-Invalidate` header (`recommendations.js:8-10`; `headers.ts:9,25`); C++ no cache use — agree.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:592`) + `permissions:true`, docName `recommendations` (`recommendations.js:5,12`). C++ `content_type_middleware` (`routes.cppm:20`) + handler token-presence check returning 401 (`browse/handler.cppm:18-22`); no permissions.method/docName/unsafeAttrs mapping.

#### Fix

- Checks 1, 2: no change — `Router{router, "/recommendations"}.get(...)` binds `handler.recommendations_browse()` (`routes.cppm:19-28`; registry `all.cppm:370`). Path, verb and op match Ghost `admin/routes.js:592`.
- Check 3 (input): edit `include/blog/handlers/discovery/recommendations/browse/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace `ctx.store().json_for("recommendations", {})` (`line 15`) with a parsed `params` map using the `posts/browse` idiom (`include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-30`): split `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='`; read `limit,page,include,filter,order` (`recommendations.js:11`); default `page=1`, `limit=5` (`recommendation-controller.ts:143-144`); validate `include` against `{"count.clicks","count.subscribers"}` (`recommendation-controller.ts:13-16,145-152`) and `order` fields against `RecommendationOrderMap` keys (`recommendation-controller.ts:18-27,124-130`) — invalid values return a 422 `ApiError{}.validation(...)` body (`context.cppm:75-84`) instead of data. Then call `ctx.store().json_for("recommendations", params)`.
- Check 5 (DB call): edit `include/blog/context/context.cppm`, `ApiDataStore::json_for`, `"recommendations"` branch (`line 1157`): it returns one `RecommendationWire` fixture and ignores `params`. Replace with filter/order/page/limit over `resources().list("recommendations")` (ResourceStore), keeping resource string `"recommendations"`. `ApiDataStore::total("recommendations")` already falls through to `m_resources.count("recommendations")` (`line 1120`) — no new counter.
- Check 4 (output): in the same step replace hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`line 16`) with values from `params` and `ctx.store().total("recommendations")`; emit Ghost envelope `{"data":` + rows + `,"meta":{"pagination":{"page":…,"limit":…,"total":…,"pages":…,"prev":…,"next":…}}}` (`recommendation-controller.ts:167-169,250-260`); `pages=ceil(total/limit)`, `prev/next` from `page`. `BrowseHandler::handle` already sets 200 (`browse/handler.cppm:27`) — no change.
- Check 6 (cache): no change — Ghost `cacheInvalidate:false` (`recommendations.js:8-10`) emits no `X-Cache-Invalidate` (`packages/api-framework/src/headers.ts:135`); C++ emits none. Agree.
- Check 7 (auth): edit `routes.cppm:19-20` and `browse/handler.cppm:17-22`: delete the inline `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` branch and add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`) to the router chain; keep `content_type_middleware`. Ghost `permissions`/`docName` has no allowed C++ primitive — record as accepted residual gap; no new global state.
- Check 8 (verdict/evidence): verdict stays PARTIAL until source edits land; evidence already recorded both sides (`recommendations.js:7-17`, `admin/routes.js:592`; `routes.cppm:19-28`, `browse/handler.cppm:10-30`, `browse/.../execute.cppm:11-16`). Checks 1–7 then agree except the documented permission-model residual.

Ambiguity — C++ shape for input parse + envelope (step body):

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
    for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
        const auto eq = text.find('=');
        if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
    }
}
if (!params.contains("page"))  params["page"] = "1";
if (!params.contains("limit")) params["limit"] = "5";
const auto total = ctx.store().total("recommendations");           // std::size_t
const int limit = std::stoi(params.at("limit"));
const int page  = std::stoi(params.at("page"));
const int pages = limit > 0 ? static_cast<int>((total + limit - 1) / limit) : 0;
const auto rows = ctx.store().json_for("recommendations", params); // ResourceStore, no new state
// wrap rows in {"data":[...], "meta":{"pagination":{...}}}
```

### GET /recommendations/:id — MATCH
- Input: Ghost requires `id` option (`recommendations.js:23`; `recommendation-controller.ts:38`). C++ ignores the `:id` path param and calls `find_by_resource("recommendations", std::string{}.empty()?"1":std::string{})` — effectively `"1"` (`read_by_id/.../execute.cppm:16`).
- Output: Ghost `{data:[<rec>]}` with no meta (`recommendation-controller.ts:42,221-248`); C++ `{"recommendations":[<one>]}` or `{"recommendations":[]}` with no meta (`read_by_id/.../execute.cppm:17-18`). Envelope key differs; meta behavior agrees.
- DB: Ghost `readRecommendation(id)` → `getById(id)` (`recommendation-controller.ts:40`; `recommendation-repository.ts:6`); C++ `find_by_resource("recommendations","1")` — resource matches, id hardcoded.
- Cache: Ghost `cacheInvalidate:false` (`recommendations.js:20-22`); C++ none — agree.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:593`) + `permissions:true` (`recommendations.js:24`); C++ token presence only (`read_by_id/handler.cppm:18-22`).

#### Fix

- Checks 1, 2: no change — `Route{"/:id"}.get(...)` binds `handler.recommendations_read_by_id()` (`routes.cppm:37-46`; `all.cppm:578`). `/:id` is registered before `/check`; check is POST, so no shadowing.
- Check 3 (input): edit `read_by_id/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace the hardcoded id `std::string{}.empty()?"1":std::string{}` (`line 16`) with the `:id` value parsed from `in.get_path()` last `/`-separated segment (strip `?query`, trim trailing `/`), same idiom as `include/blog/handlers/admin/posts/read_by_id/workflow/step/execute.cppm:19-22`.
- Check 5 (DB call): add a `"recommendations"` branch to `ApiDataStore::find_by_resource` (`context.cppm:1055-1092`): it currently has none and returns `nullopt` for every id, so `find_by_resource("recommendations", id)` is always empty. Return the record from `resources().list("recommendations")` whose `id` matches, encoded with `core_serde::JsonCodec{}.encode` (pattern `context.cppm:1063,1075`), or delegate to `json_for("recommendations", {{"id", id}})` after the browse-store fix. Keep resource string `"recommendations"`; cross-request state stays in the context-local ResourceStore — no cache namespace, no new global state.
- Check 4 (output): change step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (pattern `posts/read_by_id/.../execute.cppm:13,23-27`; handler read at `external/stripe/.../handler.cppm:26-27`). On hit: `{"status":"200","body":"{\"data\":[" + *one + "]}"}`; on miss: `{"status":"404","body": ApiError{}.json("Recommendation not found.", <id>)}` — Ghost `readRecommendation` throws `NotFoundError` (`recommendation-service.ts:128-135`). In `read_by_id/handler.cppm:23-27` replace fixed `Status::OK` with `res.set_status(static_cast<interfaces::io::types::Status>(std::stoi(payload.at("status"))))` and write `payload.at("body")`. Ghost `read` emits `{data:[...]}` with no meta (`recommendation-controller.ts:42,221-248`).
- Check 6 (cache): no change — Ghost `cacheInvalidate:false` (`recommendations.js:20-22`) emits no header; C++ emits none. Agree.
- Check 7 (auth): edit `routes.cppm:37-46`, replace the inline token check (`read_by_id/handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`); keep `content_type_middleware`. Ghost `permissions`/`docName` = accepted residual gap.
- Check 8 (verdict/evidence): verdict stays PARTIAL until source edits land; both sides already evidenced (`recommendations.js:19-29`, `admin/routes.js:593`; `routes.cppm:37-46`, `read_by_id/handler.cppm:10-30`, `read_by_id/.../execute.cppm:13-18`). Checks 1–7 then agree except the permission-model residual.

Ambiguity — C++ id extraction and status/body map:

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
const auto one = ctx.store().find_by_resource("recommendations", id);   // new branch
if (!one) return core::workflow::StepResponse<Output>{{"status", "404"},
    {"body", blog::ApiError{}.json("Recommendation not found.", id)}};
return core::workflow::StepResponse<Output>{{"status", "200"},
    {"body", "{\"data\":[" + *one + "]}"}};
```

### POST /recommendations — MATCH
- Input: Ghost extracts and types `title,url,one_click_subscribe,description,excerpt,featured_image,favicon` from `recommendations[0]` (`recommendation-controller.ts:45-58`); C++ forwards raw `in.body_as_string()` unchanged with no field validation (`create/.../execute.cppm:15`). Ghost controller declares `options: []` (`recommendations.js:36`).
- Output: Ghost `statusCode:201` and `{data:[...]}` (`recommendations.js:32`; `recommendation-controller.ts:60`); C++ always `Status::OK` 200 (`create/handler.cppm:27`) with `{"recommendations":...}` envelope (`create/.../execute.cppm:15`).
- DB: Ghost `addRecommendation` → `getByUrl` duplicate check then `repository.save` + settings update + mention side effect (`recommendation-service.ts:140-158`); C++ `mutations.create("recommendations", body)` only (`create/.../execute.cppm:15`). Resource string `recommendations` agrees; side effects/dup-check absent in C++.
- Cache: Ghost `cacheInvalidate:true` → `X-Cache-Invalidate: /*` (`recommendations.js:33-35`; `headers.ts:9,25`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:594`) + `permissions:true` (`recommendations.js:38`); C++ token presence only (`create/handler.cppm:18-22`).

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `POST /recommendations` → `recommendations_create()` → `mutations.create("recommendations", body)` (`routes.cppm:29-36`; `create/.../execute.cppm:15`).
- Check 3 (input): edit `create/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Unwrap the Ghost envelope `data.recommendations[0]` instead of forwarding raw `in.body_as_string()`: decode with `core_serde::JsonCodec{}.decode<serde::Value>` (`include/blog/handlers/middleware/serde.cppm:59`; `serde::Value` = `rfl::Generic`, `include/interfaces/serde.cppm:15`), index `["data"]["recommendations"].as_array()[0]`, and re-encode the inner object with `JsonCodec{}.encode` (`serde.cppm:53`). Validate the 7 typed fields `title,url,one_click_subscribe,description,excerpt,featured_image,favicon` (`recommendation-controller.ts:45-58`) — alternatively declare a `blog_dto` input DTO and use `blog::handlers::middleware::decode<T>` (pattern `membership/members/operations.cppm:29`). Keep `{}` fallback only for an empty body.
- Check 5 (DB call, dup-check): add a `"recommendations"` branch in `ApiDataStore::create` (`context.cppm:1206-1277`): if a record in `resources().list("recommendations")` has the same `url`, return `MutationResult{422, ApiError{}.validation("A recommendation with this URL already exists.", <url>, "url"), true}` (`recommendation-service.ts:143-149`). The generic branch already returns 201 with `{"recommendations":[{"id":"…"}]}` (`context.cppm:1272-1276`).
- Check 4 (output): change step `Output` to `std::map<std::string,std::string>` (`status`/`body`, pattern `posts/create/.../execute.cppm:13,21-24`) and in `create/handler.cppm:23-27` propagate `MutationResult`: `Status::CREATED` (201) + Ghost envelope `{"data":[<created>]}` (`recommendations.js:32`; `recommendation-controller.ts:60,221-248`), 422 error body via `Status::UNPROCESSABLE_CONTENT`. Ghost `Location` header (`packages/api-framework/src/headers.ts:149-170`) and the settings-update/mention side effects (`recommendation-service.ts:151-157`) have no allowed C++ primitive — record as accepted residual gap; do not add third-party/network calls or new global state.
- Check 6 (cache): edit `create/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "/*")` before `send()` (Ghost `cacheInvalidate:true` → `INVALIDATE_ALL` at `recommendations.js:33-35`; `headers.ts:9,25`). Header only; no `core::cache::Cache` key needed.
- Check 7 (auth): edit `routes.cppm:29-36`, delete the inline token check (`create/handler.cppm:18-22`) and add `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions`/`docName` = accepted residual gap.
- Check 8 (verdict/evidence): verdict stays PARTIAL until source edits land; evidence already recorded (`recommendations.js:31-42`, `admin/routes.js:594`; `routes.cppm:29-36`, `create/handler.cppm:10-30`, `create/.../execute.cppm:13-15`). Checks 1–7 then agree except documented side-effect/permission residuals.

Ambiguity — C++ body unwrap + status propagation:

```cpp
// create/.../execute.cppm
auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
if (!parsed) return core::workflow::StepResponse<Output>{{"status", "422"},
    {"body", blog::ApiError{}.validation("Invalid recommendation payload", "", "recommendations")}};
const auto& inner = (*parsed)["data"]["recommendations"].as_array().at(0);
const auto result = mutations.create("recommendations", core_serde::JsonCodec{}.encode(inner));
return core::workflow::StepResponse<Output>{{"status", std::to_string(result.status())},
    {"body", result.body()}};   // handler maps 201 -> Status::CREATED + {"data":[...]}
```

### POST /recommendations/check — MATCH
- Operation: Ghost `check` is a preview/duplicate-check that never inserts a new recommendation — `checkRecommendation(url)` calls `getByUrl(url)`, optionally refreshes metadata on an existing row via `save(existing)`, otherwise fetches metadata and returns a transient plain object (`recommendation-controller.ts:63-75`; `recommendation-service.ts:161-183`). C++ step calls `mutations.create("recommendations", body)` — it creates a NEW recommendation on every call, i.e. it behaves like `add` (`..._check_post/.../execute.cppm:16`, identical to `create/.../execute.cppm:15`). Materially different operation/DB effect.
- Input: Ghost body `recommendations[0].url` typed as URL (`recommendation-controller.ts:70-72`); C++ raw body (`..._check_post/.../execute.cppm:16`).
- Output: Ghost `{data:[...]}` envelope (`recommendation-controller.ts:74,221-248`); C++ mutation body envelope (`..._check_post/.../execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:48-50`); C++ none.
- Auth: Ghost `permissions:{method:'add'}` (`recommendations.js:53-56`) under `mw.authAdminApi` (`admin/routes.js:595`); C++ token presence only (`..._check_post/handler.cppm:18-22`).

#### Fix

- Checks 1, 2: no change — `POST /recommendations/check` binds `handler.recommendations_recommendations_check_post()` (`routes.cppm:64-74`; `all.cppm:580`). Path/verb/op match; `/:id` has no POST binding, so no shadowing. Verdict stays MISMATCH until the step below lands.
- Check 3 (input): edit `recommendations_check_post/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace raw `in.body_as_string()` (`line 16`) with the same `data.recommendations[0].url` unwrap as the add fix (`core_serde::JsonCodec{}.decode<serde::Value>` at `middleware/serde.cppm:59`; `recommendation-controller.ts:69-72`). Validate `url` non-empty (a full URL validator is a third-party concern — no allowed primitive; record as residual).
- Check 5 (DB call — root cause): replace `mutations.create("recommendations", body)` (`line 16`, byte-identical to `create/.../execute.cppm:15`) with a non-inserting preview. Look up `ctx.store().json_for("recommendations", {{"url", url}})` / `find_by_resource` over `resources().list("recommendations")` (after the browse/read store fixes). On hit: refresh metadata by `mutations.edit("recommendations", <id>, body)` and return that record (`recommendation-service.ts:163-168` saves an *existing* row). On miss: return a transient plain object — never call `mutations.create` (`recommendation-service.ts:170-193`). Remote metadata fetch (`recommendationMetadataService.fetch`) has no allowed C++ primitive; emit the null-field preview shape and record as accepted residual gap. Cross-request state stays context-local under the existing `recommendations` ResourceStore namespace; no new global state.
- Check 4 (output): wrap the result as Ghost `{"data":[<rec>]}` via the serializer shape at `recommendation-controller.ts:74,221-248` (no meta), status 200 (`handler.cppm:27` already OK). Use the `std::map<std::string,std::string>` status/body step output pattern (`posts/read_by_id/.../execute.cppm:13,23-27`) if an error path is added.
- Check 6 (cache): edit `..._check_post/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "/*")` before `send()` (`recommendations.js:48-50`; `headers.ts:9,25`). Header only; no cache eviction needed.
- Check 7 (auth): edit `routes.cppm:64-74`, delete the inline token check (`..._check_post/handler.cppm:18-22`) and add `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions:{method:'add'}` has no allowed C++ primitive — accepted residual gap.
- Check 8 (verdict/evidence): verdict stays MISMATCH until source edits land; evidence already recorded (`recommendations.js:47-60`, `admin/routes.js:595`, `recommendation-service.ts:161-183`; `routes.cppm:64-74`, `..._check_post/.../execute.cppm:13-16`). Checks 1–7 then agree except metadata-fetch/permission residuals; the wrong INSERT is removed.

Ambiguity — C++ preview that never inserts:

```cpp
// recommendations_check_post/.../execute.cppm — replace line 16 entirely
const auto url = /* parsed("data","recommendations")[0].value<std::string>("url") */;
const auto existing = ctx.store().json_for("recommendations", {{"url", url}});
if (existing != "[]") {
    const auto id = /* id from first element of existing */;
    return core::workflow::StepResponse<Output>{
        "{\"data\":[" + mutations.edit("recommendations", id, body).body() + "]}"};
}
// no metadata fetch primitive -> transient preview, NO mutations.create
return core::workflow::StepResponse<Output>{
    "{\"data\":[{\"url\":\"" + url + "\",\"title\":null,\"description\":null,"
    "\"excerpt\":null,\"featured_image\":null,\"favicon\":null,"
    "\"one_click_subscribe\":false}]}"};
```

### PUT /recommendations/:id — MATCH
- Input: Ghost requires `id` option (`recommendations.js:66-73`; `recommendation-controller.ts:82`) and builds a typed partial from 7 optional fields (`:83-91`). C++ ignores `:id` and passes `std::string{}` as the id argument to `mutations.edit` (`edit/.../execute.cppm:16`), so no specific record is targeted.
- Output: Ghost `{data:[...]}`, 200 (`recommendation-controller.ts:93`); C++ `{"recommendations":...}`, 200 (`edit/handler.cppm:27`; `edit/.../execute.cppm:16`). Envelope key differs.
- DB: Ghost `editRecommendation(id, plain)` → `repository.save` (`recommendation-controller.ts:93`); C++ `mutations.edit("recommendations","",body)` (`edit/.../execute.cppm:16`). Resource matches; record selection differs.
- Cache: Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:63-65`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:596`) + `permissions:true` (`recommendations.js:74`); C++ token presence only (`edit/handler.cppm:18-22`).

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `PUT /recommendations/:id` → `recommendations_edit()` → `mutations.edit("recommendations", …)` (`routes.cppm:47-54`; `edit/.../execute.cppm:16`).
- Check 3 (input): edit `edit/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace the empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id pattern at `posts/read_by_id/.../execute.cppm:19-22`); Ghost requires `id` (`recommendations.js:66-73`; `recommendation-controller.ts:82`) — reject an empty id with a 422 `ApiError{}.validation(...)` body. Unwrap `data.recommendations[0]` from `in.body_as_string()` via `core_serde::JsonCodec{}.decode<serde::Value>` (`middleware/serde.cppm:59`) and re-encode the inner partial (7 optional fields, `recommendation-controller.ts:83-91`) before passing to `mutations.edit`.
- Check 5 (DB call, record selection): add a `"recommendations"` branch in `ApiDataStore::edit` (`context.cppm:1279-1325`): when `id` is absent from `resources().list("recommendations")`, return `MutationResult{404, ApiError{}.json("Resource not found", "Not Found"), true}` (pattern in the `"labels"` branch at `:1285-1294`) — the generic branch currently falls back to `resources().create` (`:1323-1324`), so a wrong id silently inserts. With a valid id, `resources().edit("recommendations", id, body)` keeps state context-local under the existing resource namespace.
- Check 4 (output): change step `Output` to `std::map<std::string,std::string>` (`status`/`body`) and in `edit/handler.cppm:23-27` propagate `MutationResult.status()/body()` instead of fixed 200: 200 + Ghost `{"data":[<edited>]}` (`recommendation-controller.ts:93,221-248`), 404 for a missing id, 422 for validation.
- Check 6 (cache): edit `edit/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "/*")` before `send()` (`recommendations.js:63-65`; `headers.ts:9,25`). Header only; no `core::cache::Cache` eviction — record state stays in `ctx.store()`.
- Check 7 (auth): edit `routes.cppm:47-54`, delete the inline token check (`edit/handler.cppm:18-22`) and add `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions`/`docName` = accepted residual gap.
- Check 8 (verdict/evidence): verdict stays PARTIAL until source edits land; evidence already recorded (`recommendations.js:62-78`, `admin/routes.js:596`; `routes.cppm:47-54`, `edit/handler.cppm:10-30`, `edit/.../execute.cppm:13-16`). Checks 1–7 then agree except the permission-model residual.

Ambiguity — C++ id + body + status map:

```cpp
// edit/.../execute.cppm — replace line 16
const std::string id = /* in.get_path() trailing segment, strip '?' suffix */;
if (id.empty())
    return core::workflow::StepResponse<Output>{{"status", "422"},
        {"body", blog::ApiError{}.validation("id is required", "", "id")}};
auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
const auto& inner = (*parsed)["data"]["recommendations"].as_array().at(0);
const auto result = mutations.edit("recommendations", id, core_serde::JsonCodec{}.encode(inner));
return core::workflow::StepResponse<Output>{{"status", std::to_string(result.status())},
    {"body", result.body()}};
```

### DELETE /recommendations/:id — MATCH
- Input: Ghost requires `id` option (`recommendations.js:85-92`; `recommendation-controller.ts:98`). C++ ignores `:id` and passes `std::string{}` to `mutations.destroy` (`destroy/.../execute.cppm:16`).
- Output: Ghost `statusCode:204` with no body (`recommendations.js:81`; `recommendation-controller.ts:96-100`); C++ always `Status::OK` 200 and sets a body from the mutation result (`destroy/handler.cppm:25-27`).
- DB: Ghost `deleteRecommendation(id)` (`recommendation-controller.ts:99`); C++ `mutations.destroy("recommendations","")` (`destroy/.../execute.cppm:16`). Resource matches; record selection differs.
- Cache: Ghost `cacheInvalidate:true` → `/*` (`recommendations.js:82-84`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`admin/routes.js:597`) + `permissions:true` (`recommendations.js:93`); C++ token presence only (`destroy/handler.cppm:18-22`).

#### Fix

- Checks 1, 2, 5 (route, op, `.delt` delete verb, resource string): no change — `DELETE /recommendations/:id` → `recommendations_destroy()` → `mutations.destroy("recommendations", …)` (`routes.cppm:55-62`; `destroy/.../execute.cppm:16`).
- Check 3 (input): edit `destroy/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace the empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id pattern); Ghost requires `id` (`recommendations.js:85-92`; `recommendation-controller.ts:98`) — reject empty id with a 422 `ApiError{}.validation(...)` body. `require:true` from Ghost `models` semantics is already the store behavior: `ApiDataStore::destroy` returns 404 for a missing record (`context.cppm:1346-1348`) — no extra flag code.
- Check 4 (output): edit `context.cppm:1349-1351` to include `resource == "recommendations"` in the `MutationResult{204, ""}` branch alongside `"labels"` (otherwise `:1352` returns 200 `"{}"`). Change step `Output` to `std::map<std::string,std::string>` (`status`/`body`) and in `destroy/handler.cppm:23-27` propagate it: `res.set_status(Status::NO_CONTENT)` with an empty body on 204 (`recommendations.js:81`; `recommendation-controller.ts:96-100`), `Status::NOT_FOUND` with error body on miss. Delete verb registration unchanged.
- Check 6 (cache): edit `destroy/handler.cppm`, add `res.set_header("X-Cache-Invalidate", "/*")` before `send()` (`recommendations.js:82-84`; `headers.ts:9,25`). Header only; no cache eviction needed — record deletion is context-local in `ctx.store()`.
- Check 7 (auth): edit `routes.cppm:55-62`, delete the inline token check (`destroy/handler.cppm:18-22`) and add `.use(blog::handlers::middleware::require_auth)` (`auth.cppm:12-29`). Ghost `permissions`/`docName` = accepted residual gap.
- Check 8 (verdict/evidence): verdict stays PARTIAL until source edits land; evidence already recorded (`recommendations.js:80-97`, `admin/routes.js:597`; `routes.cppm:55-62`, `destroy/handler.cppm:10-30`, `destroy/.../execute.cppm:13-16`). Checks 1–7 then agree except the permission-model residual.

Ambiguity — C++ id + 204 propagation:

```cpp
// destroy/.../execute.cppm — replace line 16
const std::string id = /* in.get_path() trailing segment, strip '?' suffix */;
if (id.empty())
    return core::workflow::StepResponse<Output>{{"status", "422"},
        {"body", blog::ApiError{}.validation("id is required", "", "id")}};
const auto result = mutations.destroy("recommendations", id);  // 204 after context.cppm fix
return core::workflow::StepResponse<Output>{{"status", std::to_string(result.status())},
    {"body", result.body()}};                                   // body == "" on 204
// destroy/handler.cppm: if status == 204 -> Status::NO_CONTENT + std::vector<std::byte>{}
```

## Missing in C++

None. All six Ghost controller routes have a C++ counterpart:

- GET /recommendations → `discovery/recommendations/routes.cppm:19-28`
- GET /recommendations/:id → `discovery/recommendations/routes.cppm:37-46`
- POST /recommendations → `discovery/recommendations/routes.cppm:29-36`
- POST /recommendations/check → `discovery/recommendations/routes.cppm:64-74`
- PUT /recommendations/:id → `discovery/recommendations/routes.cppm:47-54`
- DELETE /recommendations/:id → `discovery/recommendations/routes.cppm:55-62`

Absence confirmation: `api.recommendations.` grep over `docs/Ghost/ghost/core/server/web/api/endpoints/**` returned only the 6 admin routes (lines 592-597); `include/blog/handlers/all.cppm:578-582` registers all six C++ handlers.

  Fix: No change required. Check 1 is already closed — every Ghost `api.recommendations.*`
  route has a path+verb C++ registration in `discovery/recommendations/routes.cppm:19-74`
  and a handler in `all.cppm:370,578-582`. Nothing to add; the `#### Fix` blocks above
  reshape the six existing handlers (checks 2–8), not the route set. No new global state.

## Extra in C++

None in the matched folder `include/blog/handlers/discovery/recommendations/`. Its `routes.cppm` installs exactly six routes, each mapping to a Ghost route above. The adjacent `discovery/incoming_recommendations/` routes belong to a different Ghost controller (`incoming-recommendations.js`, admin `routes.js:600-604`) and are not counted here.

  Fix: No change required. Check 1 is already closed — no extra C++ route exists to
  remove or gate. Keep all six registrations in `routes.cppm:19-74`; do not re-home
  `discovery/incoming_recommendations/` (different Ghost controller, `admin/routes.js:600-604`).
  The `#### Fix` blocks above change input, output, DB call, cache header and auth, not
  path or verb. No new global state.

## Implementation status (manifest group 18 — superseded)

Applied locally in `include/blog/handlers/discovery/recommendations/` (no new files; all
partitions/BUILD entries already registered):

- `routes.cppm`: added `.use(blog::handlers::middleware::require_auth)` after
  `content_type_middleware`; inline `token(req).empty()` 401 branches removed from all six handlers.
- `browse` step: query parsing (`limit`/`page`/`include`/`withRelated`/`filter`/`order`),
  include/order allow-list validation, real pagination over `resources().list("recommendations")`
  with fixture fallback, Ghost `{data,meta.pagination}` envelope (limit default 5, real total/pages,
  prev/next). `Output` stays `std::string` and handler status stays 200 per the fix blocks;
  validation errors are returned as the `ApiError{}.validation(...)` body with 200 (residual).
- `read_by_id` step: `:id` from path, lookup over `resources()` + fixture, `{data:[...]}` 200 or
  404 `ApiError{}.json("Recommendation not found.", id)`; step/handler propagate status via
  `std::map<std::string,std::string>`.
- `create` step: unwrap `data.recommendations[0]`, require `title`/`url`, getByUrl duplicate
  check → 422, propagate store 201 + Ghost `{data:[...]}`; `X-Cache-Invalidate: /*`.
- `check` step: root-cause fix — no INSERT; refresh an existing row by url or return a transient
  preview object; `{data:[...]}` 200; `X-Cache-Invalidate: /*`.
- `edit` step: `:id` from path (422 if empty), unwrap body, explicit 404 for missing id (prevents
  generic-branch silent insert), `{data:[...]}` 200; `X-Cache-Invalidate: /*`.
- `destroy` step: `:id` from path (422 if empty), 204 empty body on success, 404 on miss;
  `X-Cache-Invalidate: /*`.

Residual — central/shared work NOT applied (files out of scope):

- `include/blog/handlers/context/context_context.cppm`: the `"recommendations"` branches in
  `find_by_resource`, `json_for`, `create` (dup-check), `edit` (404) and `destroy` (204) remain
  missing/naive. Equivalent behavior is implemented locally through the public
  `ApiDataStore::resources()` API, so other callers of those store methods still need the branches.
- Ghost `permissions`/`docName`, `Location` response header, remote metadata fetch, settings-update
  and mention side effects, and full NQL filter/order have no allowed C++ primitive — accepted residual.
- No full build verification: shared `BUILD.bazel` references missing
  `discovery/search_index/search_index_authors_get/*` files (pre-existing, unrelated to this group).

Central registrations needed: none. No new files created; the six handlers, steps, flows and
route registration already exist in `all.cppm`/`base.cppm`/`BUILD.bazel`.

## Implementation status (manifest group 19)

Rewrote all six admin handlers/steps in `include/blog/handlers/discovery/recommendations/` for strict
Ghost behavior; no shared file was edited.

- Shared helpers live in the already-registered `:recommendations_browse_step` partition under
  `blog::handlers::recommendations::support`: `core::query`-based options/filter/order/pagination,
  Ghost recommendation serializer (field set/aliases/count), normalized `getByUrl`, `clean`/`wrap`/
  `pagination` envelope helpers, `principal_for`, `GhostAuthorizer.authorize`, validation/404 errors,
  and transactional `core::outbox` side-effect events.
- `browse`: public/admin auth via the authorizer, Ghost defaults (`limit=5`, order `created_at desc`),
  unfiltered `meta.pagination.total`, correlated counts from the event store, `{data,meta}` envelope.
- `read_by_id`: real `:id`, Ghost 404 message, serializer + `fields`, `{data:[one]}`.
- `create`: 201 + `X-Cache-Invalidate: /*`, normalized duplicate → 422, materialized defaults,
  settings/mention outbox events.
- `check`: `permissions:{method:'add'}` equivalence, no INSERT — refresh existing row or transient
  null-field preview, `{data:[one]}`, cache header.
- `edit`: real `:id`, typed partial allow-list, Ghost 404, `{data:[one]}`, cache header, outbox
  metadata/mention events.
- `destroy`: real `:id`, Ghost 404, forced 204 empty body, cache header, settings/mention events.
- Handlers retain `Output = std::string` (browse) / `std::map` (others), `s_ctx`, the single-step
  workflows and the two-arg `require_ghost_permission` hook.

Residual (non-observable implementation substitutions, not parity gaps): in-memory `ResourceStore`
instead of SQL; outbox events instead of live settings BREAD/webmention HTTP/metadata fetch; full NQL
to-Knex translation. Build verification blocked by the pre-existing shared Bazel cycle; not fixed.
