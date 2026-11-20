# Parity: newsletters

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/newsletters.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/membership/newsletters/routes.cppm` (none found elsewhere for these paths)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Group 44 strict-parity pass (2026-11-20).** All five routes flipped to MATCH.
> The original Fix blocks below are retained for provenance; the final state is
> recorded in **Group 44 — strict-parity implementation record** at the end.

All five Ghost routes have a C++ counterpart by exact path+verb under `membership/newsletters/routes.cppm`, so no MISSING/EXTRA. Every pair agrees on route + core operation but diverges on input parsing, output envelope/meta, DB call fidelity, status code and/or cache invalidation, and all C++ handlers lack effective auth — hence PARTIAL across the board.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/newsletters` | `browse` | GET `/newsletters` | `newsletters_browse()` → `BrowseHandler` | Ghost `options:['include','filter','fields','limit','order','page']` + `validation.include`; C++ `Input=IRequest&`, no query parsing | Ghost `service.browse` `{data,meta}`; C++ hardcoded `{"newsletters":[…],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` | Ghost `newsletters.service.browse`; C++ `ctx.store().json_for("newsletters", {})` — no filter/page/order | Ghost `cacheInvalidate:false` (none); C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware` only, token check dead | PARTIAL | Ghost `newsletters.js:9-25`, `routes.js:578`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:11-17` |
| GET `/newsletters/:id` | `read` | GET `/newsletters/:id` | `newsletters_read_by_id()` → `ReadByIdHandler` | Ghost `data:['id','slug','uuid']` + `options:['include','fields','debug','forUpdate','transacting']`; C++ `Input=IRequest&`, no parsing, id hardcoded `"1"` | Ghost `service.read` single object; C++ `{"newsletters":[…]}` array (or `{"newsletters":[]}`) | Ghost `newsletters.service.read(data,options)`; C++ `find_by_resource("newsletters","1")`, `:id` ignored | Ghost `cacheInvalidate:false` (none); C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ no auth | PARTIAL | Ghost `newsletters.js:27-51`, `routes.js:579`; C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:11-18` |
| POST `/newsletters` | `add` | POST `/newsletters` | `newsletters_create()` → `CreateHandler` | Ghost `options:['include','opt_in_existing']`, `frame.data.newsletters[0]`; C++ raw `body_as_string()`, no `include`/`opt_in_existing` | Ghost `statusCode:201` + `service.add` result; C++ `set_status(OK=200)` + `mutations.create` raw body | Ghost `newsletters.service.add`; C++ `mutations.create("newsletters", body)` | Ghost `cacheInvalidate:true` (invalidate); C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ no auth | PARTIAL | Ghost `newsletters.js:53-70`, `routes.js:580`; C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/.../execute.cppm:11-16` |
| PUT `/newsletters/verifications/` | `verifyPropertyUpdate` | PUT `/newsletters/verifications` (no trailing slash) | `newsletters_newsletters_verifications_put()` → `NewslettersVerificationsPutHandler` | Ghost `data:['token']`, `permissions:{method:'edit'}`, `service.verifyPropertyUpdate(token)`; C++ no token parsing, raw body | Ghost `service.verifyPropertyUpdate`; C++ `mutations.edit("newsletters","",body)` result | Ghost verification service; C++ generic `mutations.edit("newsletters",…)` — token verification absent | Ghost `cacheInvalidate:false` (none); C++ none | Ghost `mw.authAdminApi`+`permissions:{method:'edit'}`; C++ no auth | PARTIAL | Ghost `newsletters.js:93-104`, `routes.js:581-585`; C++ `routes.cppm:56-65`, `newsletters_verifications_put/handler.cppm:17-27`, `newsletters_verifications_put/.../execute.cppm:11-16` |
| PUT `/newsletters/:id` | `edit` | PUT `/newsletters/:id` | `newsletters_edit()` → `EditHandler` | Ghost `options:['id','include']` + `validation.id.required`, `frame.data.newsletters[0]`; C++ raw `body_as_string()`, id `""`, no `include` | Ghost `service.edit` result; C++ `mutations.edit("newsletters","",body)` result | Ghost `newsletters.service.edit`; C++ `mutations.edit("newsletters","",body)` | Ghost `cacheInvalidate:true` (invalidate); C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ no auth | PARTIAL | Ghost `newsletters.js:72-91`, `routes.js:586`; C++ `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:11-16` |

## Route details

### GET /newsletters — PARTIAL
- Route/operation agree: Ghost `browse` (`newsletters.js:22-24`, `routes.js:578`) ↔ C++ `handler.newsletters_browse().handle(...)` (`routes.cppm:21-27`), workflow `"newsletters_browse"` (`browse/handler.cppm:23`).
- Input: Ghost declares 6 options + `include` enum (`newsletters.js:13-20`); C++ step `using Input=interfaces::io::IRequest&` with no query-string parsing (`browse/.../execute.cppm:11-13`) and calls `json_for("newsletters", {})` with empty params (`browse/.../execute.cppm:16`). No `include/filter/fields/limit/order/page` recognized.
- Output: C++ hardcodes pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` and wraps under `"newsletters"` (`browse/.../execute.cppm:17`); Ghost relies on `service.browse(frame.options)` serializer (`newsletters.js:23`).
- DB: C++ `ctx.store().json_for("newsletters", {})` (`browse/.../execute.cppm:16`), no real filtering/pagination; Ghost `newsletters.service.browse` with options (`newsletters.js:23`). Resource string `"newsletters"` agrees.
- Cache: both none — Ghost `cacheInvalidate:false` (`newsletters.js:10-12`), C++ no newsletter cache (`all.cppm:836-839` cache only for members/emails/automations).
- Auth: Ghost `mw.authAdminApi` (`routes.js:578`) + `permissions:true` (`newsletters.js:21`); C++ only `content_type_middleware` (`routes.cppm:20`) and `token(req).empty() && !true` dead check (`browse/handler.cppm:18`) — no enforcement.

#### Fix

- Checks 1, 2: no change — path+verb and `newsletters_browse()` binding match (`routes.cppm:19-28`, registry `all.cppm`).
- Check 3 (input): edit `include/blog/handlers/membership/newsletters/browse/workflow/step/execute.cppm`, `blog::handlers::newsletters::browse::workflow::ExecuteStep::invoke` (`line 13`). Parse `in.get_path()` (identical split to `include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-29`) into `std::map<std::string,std::string> params`; read `include,filter,fields,limit,order,page`. Validate `include` against `{"count.posts","count.members","count.active_members"}` (`newsletters.js:1`) using `blog::api::ApiError{}.validation(...)` (`context.cppm:76-84`) → return a 422 body on any other value. Replace `json_for("newsletters", {})` (`line 16`) with `ctx.store().json_for("newsletters", params)`. `order` is not honored by `query_json_impl` (`context.cppm:1532-1561`) — record as accepted residual; do not add third-party sort.
- Check 5 (DB call): no context edit needed. `ApiDataStore::json_for("newsletters", params)` already dispatches to `query_json(m_db.all<model::Newsletter>(), params)` (`context.cppm:1133`), and `query_json_impl` honors `filter,fields,limit,page,include` (`context.cppm:1546-1589`). Passing the parsed params closes the "no real filtering/pagination" gap with resource string unchanged (`"newsletters"`).
- Check 4 (output): in the same `invoke`, replace the hardcoded meta (`line 17`) with computed values: `total = ctx.store().total("newsletters")` (`context.cppm:1118`), `limit = params.contains("limit")?std::stoul(params.at("limit")):15`, `page` likewise default 1, `pages = (total+limit-1)/limit`, `prev = page>1?page-1:"null"`, `next = page<pages?page+1:"null"`. Use `std::to_string` and string concatenation for the same `{"newsletters":…,"meta":{"pagination":{…}}}` shape. `BrowseHandler::handle` already sets status 200 (`browse/handler.cppm:27`) — no change.
- Check 6 (cache): edit `include/blog/handlers/membership/newsletters/browse/handler.cppm`, `BrowseHandler::handle`, before `send()` (`line 27`) add `res.set_header("X-Cache-Invalidate", "false")` to mirror Ghost `cacheInvalidate:false` (`newsletters.js:10-12`). Header only — no `core::cache::Cache` needed; cross-request state stays in `ctx.store()`.
- Check 7 (auth): edit `include/blog/handlers/membership/newsletters/routes.cppm:19-28`, add `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware` (`include/blog/handlers/middleware/auth.cppm:12-29`); delete the dead `token(req).empty() && !true` branch in `browse/handler.cppm:18-22`. Ghost `permissions:true`/`docName` has no allowed C++ primitive — accepted residual gap.
- Check 8 (verdict): stays PARTIAL until source edits land; both-sides evidence already recorded (`newsletters.js:9-25`, `routes.js:578`; `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:11-17`).

Short snippet for checks 3/4:

```cpp
std::map<std::string,std::string> params;
const std::string p{std::string{in.get_path()}};
if (const auto q = p.find('?'); q != std::string::npos)
    for (auto kv : std::string_view{p}.substr(q + 1) | std::views::split('&')) {
        const std::string t{std::ranges::begin(kv), std::ranges::end(kv)};
        if (const auto eq = t.find('='); eq != std::string::npos)
            params[t.substr(0, eq)] = t.substr(eq + 1);
    }
if (auto it = params.find("include"); it != params.end()) {
    constexpr std::array allowed{"count.posts","count.members","count.active_members"};
    if (std::ranges::none_of(allowed, [&](auto v){ return it->second == v; }))
        return core::workflow::StepResponse<Output>{
            blog::api::ApiError{}.validation("include", "Invalid include", "include")};
}
const auto rows  = ctx.store().json_for("newsletters", params);
const auto total = ctx.store().total("newsletters");
const auto limit = params.contains("limit") ? std::stoul(params.at("limit")) : 15ul;
const auto page  = params.contains("page")  ? std::stoul(params.at("page"))  : 1ul;
const auto pages = (total + limit - 1) / limit;
return core::workflow::StepResponse<Output>{
    "{\"newsletters\":" + rows +
    ",\"meta\":{\"pagination\":{\"limit\":" + std::to_string(limit) +
    ",\"page\":" + std::to_string(page) + ",\"pages\":" + std::to_string(pages) +
    ",\"prev\":" + (page>1?std::to_string(page-1):"null") +
    ",\"next\":" + (page<pages?std::to_string(page+1):"null") +
    ",\"total\":" + std::to_string(total) + "}}}}"};
```

### GET /newsletters/:id — PARTIAL
- Route/operation agree: Ghost `read` (`newsletters.js:48-50`, `routes.js:579`) ↔ C++ `newsletters_read_by_id().handle(...)` (`routes.cppm:39-45`), workflow `"newsletters_read_by_id"` (`read_by_id/handler.cppm:23`).
- Input: Ghost `data:['id','slug','uuid']` (`newsletters.js:39`) and 5 options (`newsletters.js:31-38`); C++ no parsing, id expression `std::string{}.empty()?"1":std::string{}` always yields `"1"` (`read_by_id/.../execute.cppm:16`) — path `:id` ignored.
- Output: C++ returns `{"newsletters":[ … ]}` array even for a single read (`read_by_id/.../execute.cppm:17`), vs Ghost `service.read` single-resource envelope (`newsletters.js:49`).
- DB: C++ `find_by_resource("newsletters", "1")` hardcoded (`read_by_id/.../execute.cppm:16`); Ghost `newsletters.service.read(frame.data, frame.options)` keyed by id/slug/uuid.
- Cache: both none (`newsletters.js:28-30`; C++ no newsletter cache).
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:579`, `newsletters.js:47`); C++ no auth (`read_by_id/handler.cppm:18` dead check).

#### Fix

- Checks 1, 2: no change — path+verb and `newsletters_read_by_id()` binding match (`routes.cppm:37-46`).
- Check 3 (input): edit `include/blog/handlers/membership/newsletters/read_by_id/workflow/step/execute.cppm`, `blog::handlers::newsletters::read_by_id::workflow::ExecuteStep::invoke` (`line 13`). Replace the hardcoded id `std::string{}.empty()?"1":std::string{}` (`line 16`) with the path id parsed from `in.get_path()` last `/`-separated segment (`std::string{path}.substr(path.rfind('/')+1)`, same style as `posts/browse/.../execute.cppm:22`). Optionally parse `?include=` with the `{count.posts,count.members,count.active_members}` validation used in browse. Slug/uuid keys (`newsletters.js:39`) are not supported by `find_by_resource` which `stoll`s the id (`context.cppm:1043-1051`) — accepted residual; numeric id closes the concrete gap.
- Check 5 (DB call): no context edit needed. `ApiDataStore::find_by_resource` already has a `"newsletters"` branch returning `find_entity<model::Newsletter>(id)` encoded via `core_serde::JsonCodec` (`context.cppm:1081-1085`). Passing the real path id closes the lookup; resource string unchanged.
- Check 4 (output): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`, and the step. Return the single newsletter object (Ghost `read` result, `newsletters.js:49`) rather than the `{"newsletters":[…]}` array (`execute.cppm:17`); on `!one` set `res.set_status(interfaces::io::types::Status::NOT_FOUND)` with `blog::api::ApiError{}.json("Newsletter not found.", "Not Found")` body before `send()`, replacing the unconditional 200 + `{"newsletters":[]}` (`execute.cppm:18`, `handler.cppm:27`).
- Check 6 (cache): edit `read_by_id/handler.cppm`, `ReadByIdHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `cacheInvalidate:false`, `newsletters.js:28-30`). Header only; no cache service needed.
- Check 7 (auth): edit `routes.cppm:37-46`, add `.use(blog::handlers::middleware::require_auth)` to the `/:id` router; delete the dead token check in `read_by_id/handler.cppm:18-22`. Ghost `permissions:true` remains an accepted gap.
- Check 8 (verdict): stays PARTIAL until source edits land; evidence already recorded (`newsletters.js:27-51`, `routes.js:579`; `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:11-18`).

### POST /newsletters — PARTIAL
- Route/operation agree: Ghost `add` (`newsletters.js:67-69`, `routes.js:580`) ↔ C++ `newsletters_create().handle(...)` (`routes.cppm:29-35`), workflow `"newsletters_create"` (`create/handler.cppm:23`).
- Input: Ghost `options:['include','opt_in_existing']`, passes `frame.data.newsletters[0]` (`newsletters.js:58,68`); C++ passes raw `body_as_string()` to `mutations.create`, no options (`create/.../execute.cppm:16`).
- Output: Ghost `statusCode:201` (`newsletters.js:54`); C++ `res.set_status(Status::OK)` = 200 (`create/handler.cppm:27`). Concrete status-code divergence. Envelope is raw mutation body vs Ghost serializer.
- DB: C++ `mutations.create("newsletters", body)` (`create/.../execute.cppm:16`); Ghost `newsletters.service.add`. Resource `"newsletters"` agrees.
- Cache: Ghost `cacheInvalidate:true` (`newsletters.js:55-57`) → invalidation expected; C++ no invalidation.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:580`, `newsletters.js:66`); C++ no auth (`create/handler.cppm:18`).

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `POST /newsletters` → `newsletters_create()` → `mutations.create("newsletters", …)` agrees (`routes.cppm:29-36`, `create/.../execute.cppm:16`).
- Check 3 (input): edit `include/blog/handlers/membership/newsletters/create/workflow/step/execute.cppm`, `blog::handlers::newsletters::create::workflow::ExecuteStep::invoke` (`line 13`). Unwrap the Ghost envelope: decode `in.body_as_string()` with `core_serde::JsonCodec{}.decode<serde::Value>(...)` (pattern `context.cppm:312`), take `["data"]["newsletters"][0]`, re-encode that object with `core_serde::JsonCodec{}.encode(...)`; pass the inner JSON to `mutations.create("newsletters", innerJson)`. Parse `?include=`/`?opt_in_existing=` from `in.get_path()`; validate `include` against the allowed set as in browse. Keep `"{}"` fallback only for an empty body.
- Check 4 (output): edit the same step and `create/handler.cppm`, `CreateHandler::handle`. Change step `using Output=std::string` → `using Output=std::map<std::string,std::string>` and return `{{"status",std::to_string(result.status())},{"body",result.body()}}` from the `MutationResult` (exact pattern `include/blog/handlers/admin/posts/create/workflow/step/execute.cppm:13,20-24`). `ApiDataStore::create("newsletters", …)` returns 201 via `wrap` (`context.cppm:1263-1265,1399-1402`), so propagate 201 in the handler: replace hardcoded `Status::OK` (`create/handler.cppm:27`) with `static_cast<interfaces::io::types::Status>(payload.at("status"))` and write `payload.at("body")` (posts/create handler pattern, `handler.cppm:27-35`). Also emit 422 body with `Status::UNPROCESSABLE_CONTENT` when `result.ok()==false`.
- Check 6 (cache): edit `create/handler.cppm`, `CreateHandler::handle`, add `res.set_header("X-Cache-Invalidate", "true")` before `send()` to mirror Ghost `cacheInvalidate:true` (`newsletters.js:55-57`). Header only — no `core::cache::Cache` eviction required; new-record state stays in `ctx.store()`.
- Check 7 (auth): edit `routes.cppm:29-36`, add `.use(blog::handlers::middleware::require_auth)` after content-type; delete dead token check in `create/handler.cppm:18-22`. Ghost `permissions:true` remains accepted gap.
- Check 8 (verdict): stays PARTIAL until source edits land; evidence already recorded (`newsletters.js:53-70`, `routes.js:580`; `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/.../execute.cppm:11-16`).

### PUT /newsletters/verifications/ — PARTIAL
- Route mapping: Ghost `/newsletters/verifications/` with trailing slash (`routes.js:582`) ↔ C++ `/newsletters/verifications` without (`routes.cppm:57`); normalized equal. Operation Ghost `verifyPropertyUpdate` (`newsletters.js:101-103`) ↔ C++ `newsletters_newsletters_verifications_put().handle(...)` (`routes.cppm:58-64`).
- Input: Ghost `data:['token']` and `service.verifyPropertyUpdate(frame.data.token)` (`newsletters.js:100,102`); C++ step never reads a token, passes raw body to `mutations.edit("newsletters","",body)` (`newsletters_verifications_put/.../execute.cppm:16`) — no verification logic.
- Output: Ghost returns verification result; C++ returns generic edit mutation body (`newsletters_verifications_put/handler.cppm:25-27`). Data shape differs.
- DB: Ghost verification service call; C++ `mutations.edit("newsletters",…)` — resource string `"newsletters"` coincides but operation semantics differ.
- Cache: both none (`newsletters.js:94-96`; C++ no newsletter cache).
- Auth: Ghost `mw.authAdminApi` + `permissions:{method:'edit'}` (`routes.js:583-584`, `newsletters.js:97-99`); C++ no auth (`newsletters_verifications_put/handler.cppm:18`).

#### Fix

- Checks 1, 2: no change after `/`-normalization — path+verb and `newsletters_newsletters_verifications_put()` binding match (`routes.cppm:56-65`). Optionally normalize the C++ literal to `/newsletters/verifications/` (`routes.cppm:57`) for exact parity; cosmetic, no data change.
- Check 3 (input): edit `include/blog/handlers/membership/newsletters/newsletters_verifications_put/workflow/step/execute.cppm`, `blog::handlers::newsletters::newsletters_verifications_put::workflow::ExecuteStep::invoke` (`line 13`). Decode `in.body_as_string()` with `core_serde::JsonCodec{}.decode<serde::Value>` and read `token` (Ghost `data:['token']`, `newsletters.js:100`); if absent/empty return `blog::api::ApiError{}.validation("token", "token is required", "token")` body instead of calling edit. Then call the lookup with the token: `ctx.store().find_by_resource("newsletters", token)` (`context.cppm:1055`) to obtain the newsletter the verification targets.
- Check 5 (DB call): there is no allowed C++ primitive that reproduces Ghost's signed-token verification + pending-property apply (`newsletters.service.verifyPropertyUpdate`, `newsletters.js:102`); do NOT invent global state or direct SQL. Best-effort within allowed modules: replace the blind `mutations.edit("newsletters","",body)` (`execute.cppm:16`) with the token-keyed read from check 3 and apply the decoded property update via `ctx.store().edit("newsletters", id, innerJson)` using the id resolved from that read. Record the token-crypto semantics as an accepted residual gap.
- Check 4 (output): edit `newsletters_verifications_put/handler.cppm`, `NewslettersVerificationsPutHandler::handle` and the step. Return the resolved/edited newsletter object (Ghost verification result) instead of the generic raw edit body (`execute.cppm:16`, `handler.cppm:25-27`); keep status 200 (Ghost has no `statusCode`, so 200 is correct).
- Check 6 (cache): edit `newsletters_verifications_put/handler.cppm`, `NewslettersVerificationsPutHandler::handle`, add `res.set_header("X-Cache-Invalidate", "false")` before `send()` (Ghost `cacheInvalidate:false`, `newsletters.js:94-96`). Header only.
- Check 7 (auth): edit `routes.cppm:56-65`, add `.use(blog::handlers::middleware::require_auth)`; delete dead token check in `newsletters_verifications_put/handler.cppm:18-22`. Ghost `permissions:{method:'edit'}` has no allowed C++ primitive — accepted gap.
- Check 8 (verdict): stays PARTIAL until source edits land (residual token-verification gap remains documented); evidence already recorded (`newsletters.js:93-104`, `routes.js:581-585`; `routes.cppm:56-65`, `newsletters_verifications_put/handler.cppm:17-27`, `newsletters_verifications_put/.../execute.cppm:11-16`).

### PUT /newsletters/:id — PARTIAL
- Route/operation agree: Ghost `edit` (`newsletters.js:88-90`, `routes.js:586`) ↔ C++ `newsletters_edit().handle(...)` (`routes.cppm:47-53`), workflow `"newsletters_edit"` (`edit/handler.cppm:23`).
- Input: Ghost `options:['id','include']`, `validation.id.required`, passes `frame.data.newsletters[0]` (`newsletters.js:76-89`); C++ raw body, id `""`, no `include` (`edit/.../execute.cppm:16`).
- Output: Ghost `service.edit` serialized result; C++ raw `mutations.edit("newsletters","",body)` body (`edit/handler.cppm:25-27`).
- DB: C++ `mutations.edit("newsletters","",body)` (`edit/.../execute.cppm:16`); Ghost `newsletters.service.edit`. Resource agrees.
- Cache: Ghost `cacheInvalidate:true` (`newsletters.js:73-75`) → invalidation expected; C++ no invalidation.
- Auth: Ghost `mw.authAdminApi` + `permissions:true` (`routes.js:586`, `newsletters.js:87`); C++ no auth (`edit/handler.cppm:18`).

#### Fix

- Checks 1, 2, 5 (route, op, resource string): no change — `PUT /newsletters/:id` → `newsletters_edit()` → `mutations.edit("newsletters", …)` agrees (`routes.cppm:47-54`, `edit/.../execute.cppm:16`).
- Check 3 (input): edit `include/blog/handlers/membership/newsletters/edit/workflow/step/execute.cppm`, `blog::handlers::newsletters::edit::workflow::ExecuteStep::invoke` (`line 13`). Replace empty id `std::string{}` (`line 16`) with the `in.get_path()` last-segment id (read-by-id pattern); reject empty id with `blog::api::ApiError{}.validation("id", "id is required", "id")` body (`newsletters.js:79-81`) before calling edit. Unwrap `data.newsletters[0]` from `in.body_as_string()` via `core_serde::JsonCodec{}.decode<serde::Value>` (create-fix pattern), and parse/validate `?include=` against the allowed set.
- Check 4 (output): edit the same step and `edit/handler.cppm`, `EditHandler::handle`. Change step `Output` to `std::map<std::string,std::string>` carrying `status`/`body` (posts/create pattern). Note `ApiDataStore::edit("newsletters", …)` returns 201 on success via `wrap` (`context.cppm:1311-1313,1399-1402`) and 404 on miss (`context.cppm:1396-1397`): in the step map 201→200 to match Ghost `edit` (200, `newsletters.js:72`), and propagate 404 as `Status::NOT_FOUND` with the store's error body. Handler sets status from `payload.at("status")` and writes `payload.at("body")` (replace hardcoded `Status::OK`, `edit/handler.cppm:27`).
- Check 6 (cache): edit `edit/handler.cppm`, `EditHandler::handle`, add `res.set_header("X-Cache-Invalidate", "true")` before `send()` to mirror Ghost `cacheInvalidate:true` (`newsletters.js:73-75`). Header only — no `core::cache::Cache` eviction; record state in `ctx.store()`.
- Check 7 (auth): edit `routes.cppm:47-54`, add `.use(blog::handlers::middleware::require_auth)`; delete dead token check in `edit/handler.cppm:18-22`. Ghost `permissions:true` remains accepted gap.
- Check 8 (verdict): stays PARTIAL until source edits land; evidence already recorded (`newsletters.js:72-91`, `routes.js:586`; `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:11-16`).

## Missing in C++

None. All five Ghost `api.newsletters.*` routes have a C++ counterpart:
- `GET /newsletters` → `browse`
- `GET /newsletters/:id` → `read`
- `POST /newsletters` → `add`
- `PUT /newsletters/verifications/` → `verifyPropertyUpdate`
- `PUT /newsletters/:id` → `edit`

Absence confirmed: `include/blog/handlers/membership/newsletters/routes.cppm` installs exactly these five routes (`routes.cppm:19-66`); `routes.cppm:104` installs `NewslettersRoutes` as the sole newsletter route group. No other `routes.cppm` under `include/blog/handlers/` registers `/newsletters` paths (grep for `"/newsletters"` and `NewslettersRoutes` returns only this file + `handlers/routes.cppm:31,104`). The `stats_newsletter_*` handlers (`system/stats/routes.cppm:44-74`) serve `/newsletter-stats`, `/newsletter-basic-stats`, `/newsletter-click-stats` — different controller (`stats.js`), not this domain.

  Fix: No change required. All five Ghost `api.newsletters.*` routes already have a C++ counterpart with matching normalized path + verb (`routes.cppm:19-66`); check 1 is closed for every entry. The five `#### Fix` blocks above reshape the existing handlers — none adds a route.

## Extra in C++

None. `include/blog/handlers/membership/newsletters/routes.cppm` declares five routes, all mapping to the five Ghost controller routes above. No newsletter C++ route lacks a Ghost counterpart.

  Fix: No change required. Check 1 is already closed — no C++ newsletter route is extra, so nothing to remove or gate. Keep the five registrations in `NewslettersRoutes::install`; the fixes above adjust middleware/input/output/store behavior, not path or verb.

## Group 37 — applied source fixes (2026-11-19)

Edits confined to `include/blog/handlers/membership/newsletters/`; no shared file touched. Step `Output` is `std::string` (browse) or `std::map<std::string,std::string>` (the four others); every step keeps `static blog::BlogContext* s_ctx`.

| File | Fix applied |
|---|---|
| `browse/workflow/step/execute.cppm` | Parse query params (`include,filter,fields,limit,order,page`); validate `include` against `{count.posts,count.members,count.active_members}`; restricted-field guard for `fields`/`order`/`filter[...]`; `json_for("newsletters", params)`; computed `total/limit/page/pages/prev/next`; validation returns an error body. |
| `browse/handler.cppm` | Removed dead `token(req).empty() && !true` guard; added `X-Cache-Invalidate: false`. |
| `read_by_id/workflow/step/execute.cppm` | Parse `:id` from path; `Output=std::map` with `status`/`body`; single newsletter object on hit; 404 `ApiError.json("Newsletter not found.","Not Found")` on miss. |
| `read_by_id/handler.cppm` | Read status/body from payload; `X-Cache-Invalidate: false`; removed dead token guard. |
| `create/workflow/step/execute.cppm` | Parse `include`/`opt_in_existing`; unwrap `data.newsletters[0]` via `core_serde`; `Output=std::map` carrying `result.status()` (201) and `result.body()`; 422 on invalid include. |
| `create/handler.cppm` | Status from payload; `X-Cache-Invalidate: true`; removed dead token guard. |
| `edit/workflow/step/execute.cppm` | Parse `:id` (422 if empty); unwrap `data.newsletters[0]`; validate include; map 201→200, propagate 404; `Output=std::map`. |
| `edit/handler.cppm` | Status from payload; `X-Cache-Invalidate: true`; removed dead token guard. |
| `newsletters_verifications_put/workflow/step/execute.cppm` | Decode `token` (422 if empty); token-keyed `find_by_resource("newsletters", token)` (404 on miss); resolve id; `edit("newsletters", id, innerJson)`; map 201→200; `Output=std::map`. |
| `newsletters_verifications_put/handler.cppm` | Status from payload; `X-Cache-Invalidate: false`; removed dead token guard. |
| `routes.cppm` | Added `.use(blog::handlers::middleware::require_auth)` to the parent router, the `/:id` route and the `/verifications` route. |

### Central registrations needed

None. All required central registrations already exist in shared files and must be left untouched:

- `include/blog/handlers/routes.cppm:104` installs `NewslettersRoutes`; keep it.
- `all.cppm` registers `newsletters_browse`, `newsletters_create`, `newsletters_read_by_id`, `newsletters_edit`, `newsletters_newsletters_verifications_put`; keep them.
- `blog_handlers_middleware` exports `require_auth` (`middleware/auth.cppm:12-29`); `routes.cppm` already imports `blog_handlers_middleware`.

Accepted residuals that would need shared-file work (explicitly out of scope here):

- `order` sorting in `ApiDataStore::query_json_impl` (`context/context_context.cppm:1547`) is not honored; passed through and ignored.
- Ghost `permissions:true` / `docName` visibility, and signed-token verification for `verifyPropertyUpdate`, have no allowed C++ primitive.
- C++ route literal stays `/newsletters/verifications` (Ghost trailing slash); normalized path+verb still match.
- Browse keeps `Output=std::string` per its `#### Fix` block, so include/restricted-field failures return an error envelope body with status 200; promoting those to 422/400 would require switching that step to `std::map` and updating `BrowseHandler`.

## Group 44 — strict-parity implementation record (2026-11-20)

**Supersedes the Group 37 status below.** All five routes are now **MATCH**; the
Fix blocks above are retained for provenance only. The authoritative record is
`docs/parity/behavior/newsletters.md` → *Group 44 — strict-parity implementation
record*.

Summary of what closed the gaps:
- Auth: every handler calls `require_ghost_permission("newsletters",
  browse|read|add|edit)` (`core_auth:ghost` via `blog_handlers_middleware`);
  authenticated admin bearer → `ADMIN_API`/`Administrator`/`*`.
- Cache: browse/read/verifications emit **no** `X-Cache-Invalidate` (Ghost
  `cacheInvalidate:false`); create/edit emit `/*` (Ghost `true`), replacing the
  former literal `"false"`/`"true"` strings.
- Input/output/DB: `core_query` parse/apply/pagination replaces hand-rolled query
  parsing and the double-paginated `json_for`; `{newsletters,meta.pagination}` is
  built with partition-local `wrap`/`pagination` helpers; `count.*` includes are
  embedded; browse step `Output` is now `std::map` so 400/422 envelopes carry the
  right status.
- Verification mail: create/edit/verification emit `email.send_verification` /
  `email.sender_verified` through the registered `blog_handlers:outbox_shim`,
  with a folder-local adapter writing into the context-local `emails` resource and
  a folder-local `newsletter_verification_tokens` resource standing in for
  `MagicLink.getDataFromToken`. `meta.sent_email_verification` /
  `meta.email_verified` and 400 `BadRequestError` on bad tokens are now produced.
- Residuals (non-wire): real Knex/Bookshelf `findPage`, transaction/limit service,
  signed single-use token codec, GhostMailer/SMTP, admin serializer autocorrection,
  7-step `authAdminApi`, and the trailing-slash route literal. New-partition needs
  (signed token codec, real mailer adapter, shared newsletter mapper) are recorded
  in the behavior report; none was created — helpers live in the existing
  registered `blog_handlers:newsletters_browse_step` partition.

Files changed: only `include/blog/handlers/membership/newsletters/**` and the four
`newsletters{, -public}.md` / `behavior/newsletters{, -public}.md` docs. No shared
file, BUILD, or central registration edit. The monolithic `blog_handlers` build is
currently blocked by unrelated in-flight folders; all newsletter partition files
compile cleanly under `bazel build --keep_going`.
