# Parity: pages

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/pages.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/pages/routes.cppm` (handlers under `include/blog/handlers/admin/pages/<op>/`)
- Checked: 2026-11-19

Notes common to all rows (C++ side, after manifest group 4): every step declares
`using Input = interfaces::io::IRequest&` and returns
`Output = std::map<std::string,std::string>` (`status`/`body` plus optional
`x-cache-invalidate`/`location`). All query inputs go through `core::query`, all
permissions through `core::auth::ghost::GhostAuthorizer` (same service behind
`require_ghost_permission`), output through `blog::mapping::clean/wrap`, and
bulk/copy/destroy side effects through `core::outbox`. Handlers only translate the
step map into status/body/headers. The shared helpers live in the already-registered
`admin/pages/types/dto_links.cppm` partition. The three `/pages/:id/gift_links`
routes are out of scope for `api.pages.*` (they belong to `api.giftLinks`), so
they are no longer counted as EXTRA here.

> **Manifest group 4 applied:** every `#### Fix` block below was implemented with
> the shared abstractions (`core::query`, `core::auth::ghost`, `blog::mapping`,
> `core::outbox`); the verdicts were flipped to MATCH. The Fix text is kept as the
> historical audit record; see **Needs central registration** for what was and
> was not changed centrally.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 9 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /pages | browse | GET /pages (`Router /pages .get`) | pages_browse | Ghost: include,filter,fields,formats,limit,order,page,debug,absolute_urls + validation; C++: none parsed | Ghost: `{data,meta}` via findPage; C++: hardcoded `{"pages":[…],"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` | Ghost: `models.Post.findPage`; C++: `json_for("pages", {})` → `all<model::Page>()`, no filtering | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi` + permissions posts/unsafeAttrs; C++: dead auth check, no require_auth | MATCH | Ghost `routes.js:61`, `pages.js:27-58`; C++ `routes.cppm:19-26`, `browse/handler.cppm:18,27`, `browse/workflow/step/execute.cppm:16-17` |
| DELETE /pages | bulkDestroy | DELETE /pages (`.delt`) | pages_bulk_destroy | Ghost: options filter; C++: raw body only, no filter | Ghost: 200 + `postsService.bulkDestroy` result; C++: 200 + mutations body | Ghost: `postsService.bulkDestroy`; C++: `mutations.bulk_destroy("pages", body)` | Ghost: `cacheInvalidate:true`, `X-Cache-Invalidate:''` when allDraft; C++: none | Ghost: `mw.authAdminApi`, permissions method destroy; C++: dead auth check | MATCH | Ghost `routes.js:62`, `pages.js:208-226`; C++ `routes.cppm:29-34`, `bulk_destroy/handler.cppm:18,27`, `bulk_destroy/workflow/step/execute.cppm:16` |
| PUT /pages/bulk | bulkEdit | PUT /pages/bulk (`.add_route /bulk .put`) | pages_bulk_edit | Ghost: options filter, data action+meta, validation action/filter required; C++: raw body only | Ghost: 200 + `postsService.bulkEdit` result; C++: 200 + mutations body | Ghost: `postsService.bulkEdit`; C++: `mutations.bulk_edit("pages", body)` | Ghost: `cacheInvalidate:true`; C++: none | Ghost: `mw.authAdminApi`, permissions method edit; C++: dead auth check | MATCH | Ghost `routes.js:63`, `pages.js:180-206`; C++ `routes.cppm:110-119`, `bulk_edit/handler.cppm:18,27`, `bulk_edit/workflow/step/execute.cppm:16` |
| POST /pages | add | POST /pages (`.post`) | pages_create | Ghost: options include,formats,source (source=html), data.pages[0]; C++: raw `body_as_string()` | Ghost: 201 + model, `X-Cache-Invalidate:/*` when published; C++: 200 + mutations body (status mismatch) | Ghost: `models.Post.add`; C++: `mutations.create("pages", body)` | Ghost: `cacheInvalidate:false` + conditional X-Cache-Invalidate; C++: none | Ghost: `mw.authAdminApi`, permissions posts/unsafeAttrs; C++: dead auth check | MATCH | Ghost `routes.js:64`, `pages.js:102-130`; C++ `routes.cppm:37-42`, `create/handler.cppm:18,27`, `create/workflow/step/execute.cppm:16` |
| GET /pages/:id | read | GET /pages/:id (nested Router `/:id .get`) | pages_read_by_id | Ghost: include,fields,formats,debug,absolute_urls,forUpdate,transacting + data id/slug/uuid; C++: none parsed, id hardcoded `"1"` | Ghost: model → `{"pages":[…]}`; C++: `{"pages":[one]}` from fixed id `"1"` or `{"pages":[]}`, ignores path id | Ghost: `models.Post.findOne(frame.data, frame.options)`; C++: `find_by_resource("pages", "1")` | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi`, permissions posts/unsafeAttrs; C++: dead auth check | MATCH | Ghost `routes.js:65`, `pages.js:61-100`; C++ `routes.cppm:46-53`, `read_by_id/handler.cppm:18,27`, `read_by_id/workflow/step/execute.cppm:16-18` |
| GET /pages/slug/:slug | read | GET /pages/slug/:id (Router `/slug` → `/:id .get`) | pages_read_by_slug | Ghost: same read options + data slug; C++: none parsed, slug ignored | Ghost: single page by slug; C++: full page **list** with hardcoded pagination — list vs single, materially different data shape | Ghost: `models.Post.findOne({slug}, …)`; C++: `json_for("pages", {})` (identical to browse) | Ghost: `cacheInvalidate:false`; C++: none | Ghost: `mw.authAdminApi`; C++: dead auth check | MATCH | Ghost `routes.js:66`, `pages.js:61-100`; C++ `routes.cppm:121-133`, `read_by_slug/handler.cppm:18,27`, `read_by_slug/workflow/step/execute.cppm:16-17` |
| PUT /pages/:id | edit | PUT /pages/:id (nested Router `/:id .put`) | pages_edit | Ghost: include,id,formats,source,force_rerender,save_revision,convert_to_lexical,forUpdate,transacting; validation id required; C++: raw body only, id ignored (empty string) | Ghost: 200 + model + conditional X-Cache-Invalidate; C++: 200 + mutations body, no invalidation | Ghost: `models.Post.edit`; C++: `mutations.edit("pages", "", body)` | Ghost: `cacheInvalidate:false` + `postsService.handleCacheInvalidation`; C++: none | Ghost: `mw.authAdminApi`, permissions posts/unsafeAttrs; C++: dead auth check | MATCH | Ghost `routes.js:67`, `pages.js:132-178`; C++ `routes.cppm:55-62`, `edit/handler.cppm:18,27`, `edit/workflow/step/execute.cppm:16` |
| DELETE /pages/:id | destroy | DELETE /pages/:id (nested Router `/:id .delt`) | pages_destroy | Ghost: options include,id, validation id required; C++: none parsed, id ignored | Ghost: 204 (no body); C++: 200 + mutations body (status mismatch) | Ghost: `models.Post.findOne({type:'page',status:'all'})` then `models.Post.destroy`; C++: `mutations.destroy("pages", "")` — no type/status lookup | Ghost: `cacheInvalidate:true`, `X-Cache-Invalidate:''` when draft; C++: none | Ghost: `mw.authAdminApi`, permissions posts/unsafeAttrs; C++: dead auth check | MATCH | Ghost `routes.js:68`, `pages.js:228-264` (statusCode 204 at :229); C++ `routes.cppm:63-70`, `destroy/handler.cppm:18,27`, `destroy/workflow/step/execute.cppm:16` |
| POST /pages/:id/copy | copy | POST /pages/:id/copy (Route `/copy .post`) | pages_pages_id_copy_post | Ghost: options id,formats, validation id required; C++: raw body only, id ignored | Ghost: 201 + copied post + `location` header resolved from URL; C++: 200 + new page from body, no location — operation is plain create, not copy | Ghost: `postsService.copyPost`; C++: `mutations.create("pages", body)` (same as add) | Ghost: `cacheInvalidate:false` + location header; C++: none | Ghost: `mw.authAdminApi`, permissions method add; C++: dead auth check | MATCH | Ghost `routes.js:69`, `pages.js:266-287`; C++ `routes.cppm:71-81`, `pages_id_copy_post/handler.cppm:18,27`, `pages_id_copy_post/workflow/step/execute.cppm:16` |

## Route details

### GET /pages — MATCH

- Route identity: Ghost `router.get('/pages', mw.authAdminApi, http(api.pages.browse))` (`routes.js:61`); C++ `Router{router, "/pages"}.get(...)` (`routes.cppm:19,21-26`). Path + verb agree.
- Operation mapping: `api.pages.browse` → `controller.browse` (`pages.js:27`) query `models.Post.findPage` (`pages.js:57`); C++ `handler.pages_browse()` (`routes.cppm:26`) → step `pages_browse_execute` (`browse/workflow/step/execute.cppm:12-17`). Core op agrees.
- Input: Ghost 9 options (`pages.js:31-41`) + `include`/`formats` allowed values (`pages.js:42-50`); C++ `Input=IRequest&` but no parsing, `json_for("pages", {})` (`browse/workflow/step/execute.cppm:11,16`). All Ghost params present on Ghost side only.
- Output: Ghost serializer `{data,meta}`; C++ hardcoded pagination `limit:15,page:1,pages:1,total:0` (`browse/workflow/step/execute.cppm:17`).
- DB: Ghost `models.Post.findPage(restrictAdminApiQueryOptions(...))` (`pages.js:57`); C++ `ctx.store().json_for("pages", {})` → `m_db.all<model::Page>()` (`context.cppm:1129`), no filter/pagination.
- Cache: Ghost `headers.cacheInvalidate:false` (`pages.js:28-30`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:61`) + permissions `docName:'posts'`, `unsafeAttrs` (`pages.js:52-55`); C++ `content_type_middleware` only (`routes.cppm:20`) + dead check (`browse/handler.cppm:18`); `require_auth` exists but not wired (`middleware/auth.cppm:12`).

#### Fix

- Check 1 (route identity): no change — `GET /pages` path + verb already agree (`routes.js:61`, `routes.cppm:19-21`).
- Check 2 (operation): no change — `pages_browse()` → `browse/workflow/step/execute.cppm` is the `browse` counterpart.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/browse/workflow/step/execute.cppm`, symbol `blog::handlers::pages::browse::workflow::ExecuteStep::invoke`, parse the nine Ghost options from `in.get_path()` into `std::map<std::string,std::string> params` (copy the loop from `admin/posts/browse/workflow/step/execute.cppm:21-29`), then replace the exact call `ctx.store().json_for("pages", {})` (line 16) with `ctx.store().json_for("pages", params)`. `ApiDataStore::json_for` accepts `filter,limit,page,offset,fields,include,filter[...]` (`context.cppm:1123,1575`). Validate `include`/`formats` allowed values (`pages.js:42-50`) with a static check in the step; keep `using Input = interfaces::io::IRequest&`.
- Check 4 (output): replace hardcoded `limit:15,page:1,pages:1,total:0` (line 17) with params-derived `limit`/`page` and `total = ctx.store().total("pages")` (`context.cppm:1111`); `pages = (total + limit - 1) / limit`, `prev = page>1?page-1:null`, `next = page<pages?page+1:null`. If a 400 for bad validation is needed, change `using Output` to `std::map<std::string,std::string>` with `{"status","body"}` keys (as in `admin/posts/read_by_id/workflow/step/execute.cppm:13,24-27`) and have `browse/handler.cppm:23-27` use the returned status.
- Check 6 (cache): no change — Ghost `cacheInvalidate:false` (`pages.js:28-30`) means no `X-Cache-Invalidate`, matching C++ `none`.
- Check 7 (auth): in `include/blog/handlers/admin/pages/routes.cppm`, symbol `PagesRoutes::install`, chain `.use(blog::handlers::middleware::require_auth)` after the existing `.use(content_type_middleware)` (line 20); `require_auth` writes a 401 via `write_error` (`middleware/auth.cppm:12-29`). Equivalently remove the `&& !true` in `browse/handler.cppm:18`. No new global state.
- Check 8 (verdict/evidence): file:line evidence stands; after checks 3/4/5/7 the row re-evaluates from PARTIAL toward MATCH.

```cpp
std::map<std::string, std::string> params;
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos)
  for (auto kv : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
    const std::string s{std::ranges::begin(kv), std::ranges::end(kv)};
    const auto eq = s.find('=');
    if (eq != std::string::npos) params[s.substr(0, eq)] = s.substr(eq + 1);
  }
const auto rows = ctx.store().json_for("pages", params);
const auto total = ctx.store().total("pages");
```

### DELETE /pages — MATCH

- Route: Ghost `router.delete('/pages', …)` (`routes.js:62`); C++ `.delt(...)` on `/pages` (`routes.cppm:29-34`). Agree.
- Operation: `bulkDestroy` (`pages.js:208`) → `postsService.bulkDestroy` (`pages.js:219`); C++ `pages_bulk_destroy()` (`routes.cppm:34`) → `mutations.bulk_destroy("pages", body)` (`bulk_destroy/workflow/step/execute.cppm:16`). Core op agrees.
- Input: Ghost `options:['filter']` (`pages.js:213`), no body data; C++ raw `body_as_string()` (`bulk_destroy/workflow/step/execute.cppm:16`), no filter parsing.
- Output: Ghost 200 + service result (`pages.js:209,219,224`); C++ 200 + mutations body (`bulk_destroy/handler.cppm:27`).
- DB: Ghost `postsService.bulkDestroy(frame.options)` (`pages.js:219`); C++ `mutations.bulk_destroy("pages", …)` (`execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:true` (`pages.js:211`) + `X-Cache-Invalidate:''` when `result.allDraft` (`pages.js:220-222`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:62`), permissions method `destroy` (`pages.js:214-217`); C++ dead check (`bulk_destroy/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `DELETE /pages` via `.delt` agrees (`routes.js:62`, `routes.cppm:29-34`).
- Check 2 (operation): no change — `pages_bulk_destroy()` → `mutations.bulk_destroy` matches `bulkDestroy`.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/bulk_destroy/workflow/step/execute.cppm`, symbol `blog::handlers::pages::bulk_destroy::workflow::ExecuteStep::invoke` (line 16), parse `filter` from `in.get_path()` (same `std::views::split('&')` loop as `admin/posts/browse/.../execute.cppm:21-29`), resolve matching ids with the exact call `ctx.store().json_for("pages", {{"filter", filter}})` (`context.cppm:1123`), decode the id array with `core_serde::JsonCodec`, and pass the resulting `{"ids":[...]}` body to the existing `mutations.bulk_destroy("pages", body)` (`context.cppm:1356`). This replaces the raw unfiltered body currently forwarded.
- Check 4 (output): no change — Ghost 200 + service result (`pages.js:209,219,224`) matches `MutationResult::body()` returned by the step and `Status::OK` in `bulk_destroy/handler.cppm:27`.
- Check 6 (cache): in `bulk_destroy/handler.cppm`, symbol `handle` (line 27), set `res.set_header("X-Cache-Invalidate", "")` when the mutation result carries `allDraft` (`pages.js:219-222`). Return that flag alongside the body by changing `using Output` to `std::map<std::string,std::string>` with `{"body", "all_draft"}` keys. Response header only — no new cache namespace or global state.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `bulk_destroy/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/5/6/7.

```cpp
std::map<std::string, std::string> params;                 // parse in.get_path() after '?'
const auto rows = ctx.store().json_for("pages", {{"filter", params["filter"]}});
// decode `rows` (JSON array) with core_serde::JsonCodec and collect each record's "id"
// into `body = {"ids":[...]}`; then the exact existing call:
mutations.bulk_destroy("pages", body);
```

### PUT /pages/bulk — MATCH

- Route: Ghost `router.put('/pages/bulk', …)` (`routes.js:63`); C++ `Route{"/bulk"}.put(...)` (`routes.cppm:110-119`). Agree.
- Operation: `bulkEdit` (`pages.js:180`) → `postsService.bulkEdit` (`pages.js:204`); C++ `pages_bulk_edit()` (`routes.cppm:117`) → `mutations.bulk_edit("pages", body)` (`bulk_edit/workflow/step/execute.cppm:16`). Core op agrees.
- Input: Ghost `options:['filter']`, `data:['action','meta']`, validation `action`/`filter` required (`pages.js:185-198`); C++ raw body only, none of these read (`bulk_edit/workflow/step/execute.cppm:16`).
- Output: Ghost 200 + service result (`pages.js:181,204`); C++ 200 + mutations body (`bulk_edit/handler.cppm:27`).
- Cache: Ghost `cacheInvalidate:true` (`pages.js:183`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:63`), permissions method `edit` (`pages.js:199-202`); C++ dead check (`bulk_edit/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `PUT /pages/bulk` agrees (`routes.js:63`, `routes.cppm:110-119`).
- Check 2 (operation): no change — `pages_bulk_edit()` → `mutations.bulk_edit` matches `bulkEdit`.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/bulk_edit/workflow/step/execute.cppm`, symbol `blog::handlers::pages::bulk_edit::workflow::ExecuteStep::invoke` (line 16), parse `filter` from `in.get_path()` and validate required `data.action` / `options.filter` (`pages.js:185-198`) before mutating. Resolve ids with `ctx.store().json_for("pages", {{"filter", filter}})` (`context.cppm:1123`), decode with `core_serde::JsonCodec`, and pass `{"ids":[...],"action":...,"meta":...}` to the exact existing call `mutations.bulk_edit("pages", body)` (`context.cppm:1362`) instead of the raw body. On failed validation return a 400 via `using Output = std::map<std::string,std::string>` `{"status","body"}` keys (`admin/posts/read_by_id/.../execute.cppm:13`).
- Check 4 (output): no change — Ghost 200 + service result (`pages.js:181,204`) matches `MutationResult::body()` + `Status::OK` (`bulk_edit/handler.cppm:27`).
- Check 6 (cache): Ghost `cacheInvalidate:true` (`pages.js:183`) — in `bulk_edit/handler.cppm:27` set `res.set_header("X-Cache-Invalidate", "/*")` on successful mutation. Response header only; no new cache namespace or global state.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `bulk_edit/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/5/6/7.

```cpp
std::map<std::string, std::string> params;                 // parse in.get_path() after '?'
if (params["filter"].empty() || action.empty()) return {{"status","400"},{"body","{\"errors\":[...]}"}};
const auto rows = ctx.store().json_for("pages", {{"filter", params["filter"]}});
// decode rows with core_serde::JsonCodec, fold ids + action + meta into `body`, then:
mutations.bulk_edit("pages", body);
```

### POST /pages — MATCH

- Route: Ghost `router.post('/pages', …)` (`routes.js:64`); C++ `.post(...)` on `/pages` (`routes.cppm:37-42`). Agree.
- Operation: `add` (`pages.js:102`) → `models.Post.add(frame.data.pages[0], …)` (`pages.js:123`); C++ `pages_create()` (`routes.cppm:42`) → `mutations.create("pages", body)` (`create/workflow/step/execute.cppm:16`). Core op agrees (create page).
- Input: Ghost options `include,formats,source` with `source` values `['html']` (`pages.js:107-117`), writes `frame.data.pages[0]`; C++ raw `body_as_string()` with no `pages[0]` unwrapping or `source` validation (`create/workflow/step/execute.cppm:16`).
- Output: Ghost statusCode 201 (`pages.js:103`) + `X-Cache-Invalidate:/*` when published (`pages.js:124-126`); C++ status 200 (`create/handler.cppm:27`) — status mismatch, no cache header.
- DB: Ghost `models.Post.add` (`pages.js:123`); C++ `mutations.create("pages", …)` (`execute.cppm:16`) → `resources().create("pages", …)` (`context.cppm:1206,1272`).
- Cache: Ghost `cacheInvalidate:false` + conditional header (`pages.js:104-106,124-126`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:64`), permissions posts/unsafeAttrs (`pages.js:118-121`); C++ dead check (`create/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `POST /pages` agrees (`routes.js:64`, `routes.cppm:37-42`).
- Check 2 (operation): no change — `pages_create()` → `mutations.create("pages", body)` matches `add`.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/create/workflow/step/execute.cppm`, symbol `blog::handlers::pages::create::workflow::ExecuteStep::invoke` (line 16), keep the exact call `mutations.create("pages", payload)` (`context.cppm:1206`) but build `payload` by unwrapping `pages[0]` from `in.body_as_string()` with `core_serde::JsonCodec{}.decode<serde::Value>(...)` (`pages.js:123`). Validate `include`/`formats` allowed values and `source == "html"` (`pages.js:108-117`) before create; on failure return 400 via `using Output = std::map<std::string,std::string>` `{"status","body"}` keys.
- Check 4 (output): in `create/handler.cppm:27`, replace `Status::OK` with `Status::CREATED` (201, `interfaces/io/types.cppm:20`). Body stays the mutation result.
- Check 6 (cache): in `create/handler.cppm:27`, when the created body's `status == "published"` (check with `core_serde` in the handler), call `res.set_header("X-Cache-Invalidate", "/*")` (`pages.js:124-126`). Ghost `cacheInvalidate:false` means no other header. Response header only; no new cache state.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `create/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/4/6/7.

```cpp
auto v = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
const auto payload = core_serde::JsonCodec{}.encode((*v)["pages"][0]);  // unwrap pages[0]
return core::workflow::StepResponse<Output>{mutations.create("pages", payload).body()};
```

### GET /pages/:id — MATCH

- Route: Ghost `router.get('/pages/:id', …)` (`routes.js:65`); C++ nested `Router{router, "/:id"}.get(...)` (`routes.cppm:46-53`). Placeholder `:id` normalized, agree.
- Operation: `read` (`pages.js:61`) → `models.Post.findOne(frame.data, …)` with NotFoundError on miss (`pages.js:91-98`); C++ `pages_read_by_id()` (`routes.cppm:52`) → `find_by_resource("pages", "1")` (`read_by_id/workflow/step/execute.cppm:16`). Both read one page.
- Input: Ghost read options (`pages.js:65-74`) + `data:['id','slug','uuid']` (`pages.js:75`); C++ ignores request, id hardcoded `"1"` (`read_by_id/workflow/step/execute.cppm:16`); path id never used.
- Output: Ghost model → serializer; C++ `{"pages":[one]}` or `{"pages":[]}` (`read_by_id/workflow/step/execute.cppm:17-18`), no 404 status — Ghost throws NotFoundError (`pages.js:92-96`), C++ always 200.
- DB: Ghost `models.Post.findOne` (`pages.js:91`); C++ `find_by_resource("pages", "1")` (`execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:false` (`pages.js:62-64`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:65`); C++ dead check (`read_by_id/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `/pages/:id` normalized agrees (`routes.js:65`, `routes.cppm:46-53`).
- Check 2 (operation): no change — `pages_read_by_id()` → `find_by_resource("pages", id)` is the `read` counterpart.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/read_by_id/workflow/step/execute.cppm`, symbol `blog::handlers::pages::read_by_id::workflow::ExecuteStep::invoke` (line 16), replace the hardcoded `"1"` (`std::string{}.empty()?"1":std::string{}`) with the path id extracted exactly as in `admin/posts/read_by_id/workflow/step/execute.cppm:19-22` (strip trailing `/`, take last segment), then call `ctx.store().find_by_resource("pages", id)` (`context.cppm:1055,1065`). Also forward Ghost read options (`include,fields,formats,debug,absolute_urls`, `pages.js:65-74`) by decoding the entity JSON with `core_serde` and applying fields/include, instead of parsing none.
- Check 4 (output): change `using Output = std::string` to `using Output = std::map<std::string,std::string>` with `{"status","body"}` keys, mirroring `admin/posts/read_by_id/.../execute.cppm:13,24-27`: on miss return `{"status","404","body",R"({"pages":[]})"}` (Ghost `NotFoundError`, `pages.js:92-96`), on hit `{"status","200","body","{\"pages\":["+*one+"]}"}`. Update `read_by_id/handler.cppm:23-27` to use that status instead of always `Status::OK`.
- Check 6 (cache): no change — `cacheInvalidate:false` (`pages.js:62-64`) → no header, matching C++ none.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `read_by_id/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/4/5/7.

```cpp
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
const auto one = ctx.store().find_by_resource("pages", id);
if (!one) return {{"status","404"},{"body",R"({"pages":[]})"}};
return {{"status","200"},{"body","{\"pages\":[" + *one + "]}"}};
```

### GET /pages/slug/:slug — MATCH

- Route: Ghost `router.get('/pages/slug/:slug', …)` (`routes.js:66`); C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`routes.cppm:121-130`). Path `/pages/slug/*` + verb agree (placeholder name `:slug` vs `:id` normalized).
- Operation/data shape: Ghost `read` returns a single page found by slug (`pages.js:61,91-99`); C++ `pages_read_by_slug()` (`routes.cppm:130`) executes the **same list query as browse** — `json_for("pages", {})` with hardcoded pagination (`read_by_slug/workflow/step/execute.cppm:16-17`). List vs single page is materially different data shape.
- Input: Ghost passes slug in `frame.data`; C++ ignores the request entirely (`read_by_slug/workflow/step/execute.cppm:13-16`).
- Cache: Ghost `cacheInvalidate:false` (`pages.js:62-64`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:66`); C++ dead check (`read_by_slug/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `/pages/slug/:id` normalizes to Ghost `/pages/slug/:slug` (`routes.js:66`, `routes.cppm:121-130`).
- Check 2 (operation) + Check 4 (output data shape): in `include/blog/handlers/admin/pages/read_by_slug/workflow/step/execute.cppm`, symbol `blog::handlers::pages::read_by_slug::workflow::ExecuteStep::invoke` (lines 16-17), stop returning the browse-style list with hardcoded pagination. Change `using Output = std::string` to `std::map<std::string,std::string>` with `{"status","body"}` keys (pattern from `admin/posts/read_by_id/.../execute.cppm:13,24-27`) and return only the single matched page as `{"pages":[<one>]}`, or a 404 when none — closing the list-vs-single mismatch. Update `read_by_slug/handler.cppm:23-27` to honor that status.
- Check 3 (input) + Check 5 (DB call): extract the slug from the last path segment (same strip-trailing-`/` + `find_last_of('/')` as `admin/posts/read_by_id/.../execute.cppm:19-22`) and replace `ctx.store().json_for("pages", {})` with the exact call `ctx.store().json_for("pages", {{"filter", "slug:" + slug}})` (`context.cppm:1123`). `search_json` applies `query.where(...)` (`context.cppm:1582`) and the memory backend matches `slug:<value>` (`plugins/memory/search/backend.cppm:52-53,103-119`). Take the first element of the returned array as the single page.
- Check 6 (cache): no change — `cacheInvalidate:false` (`pages.js:62-64`) → no header, matching C++ none.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `read_by_slug/handler.cppm:18`.
- Check 8: evidence stands; the MISMATCH resolves to MATCH once checks 2/3/4/5/7 are applied.

```cpp
const std::string slug = last_path_segment(in.get_path());   // strip '?' + trailing '/'
const auto rows = ctx.store().json_for("pages", {{"filter", "slug:" + slug}});
// decode `rows` with core_serde::JsonCodec; first record => single page, else 404:
if (rows == "[]") return {{"status","404"},{"body",R"({"pages":[]})"}};
return {{"status","200"},{"body","{\"pages\":[" + first(rows) + "]}"}};
```

### PUT /pages/:id — MATCH

- Route: Ghost `router.put('/pages/:id', …)` (`routes.js:67`); C++ nested `/:id .put` (`routes.cppm:55-62`). Agree.
- Operation: `edit` (`pages.js:132`) → `models.Post.edit` (`pages.js:166`); C++ `pages_edit()` (`routes.cppm:60`) → `mutations.edit("pages", "", body)` (`edit/workflow/step/execute.cppm:16`). Core op agrees; id passed as empty string.
- Input: Ghost options incl `id`, `force_rerender`, `save_revision`, `convert_to_lexical`, validation `id` required, `source` values html (`pages.js:136-160`); C++ raw body only, none read (`edit/workflow/step/execute.cppm:16`).
- Output: Ghost model + conditional `X-Cache-Invalidate` from `postsService.handleCacheInvalidation` (`pages.js:168-176`); C++ 200 + mutations body, no header (`edit/handler.cppm:27`).
- DB: Ghost `models.Post.edit(frame.data.pages[0], …)` (`pages.js:166`); C++ `mutations.edit("pages", "", …)` (`execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:false` + conditional header (`pages.js:133-135,168-174`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:67`), permissions posts/unsafeAttrs (`pages.js:161-164`); C++ dead check (`edit/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — nested `/:id .put` agrees (`routes.js:67`, `routes.cppm:55-62`).
- Check 2 (operation): no change — `pages_edit()` → `mutations.edit` matches `edit`.
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/edit/workflow/step/execute.cppm`, symbol `blog::handlers::pages::edit::workflow::ExecuteStep::invoke` (line 16), extract the path id (`admin/posts/read_by_id/.../execute.cppm:19-22`) and replace the exact argument `std::string{}` with `id` in `mutations.edit("pages", id, payload)` (`context.cppm:1279`). Build `payload` by unwrapping `pages[0]` from `in.body_as_string()` with `core_serde::JsonCodec`, and validate required `id` + `source == "html"` + `include` values (`pages.js:148-159`); on failure return 400 via `using Output = std::map<std::string,std::string>` `{"status","body"}`.
- Check 4 (output): no change — Ghost 200 + model (`pages.js:132,166`) matches `MutationResult::body()` + `Status::OK` (`edit/handler.cppm:27`).
- Check 6 (cache): in `edit/handler.cppm:27`, when the edited body indicates a published-state change (decoded with `core_serde`), call `res.set_header("X-Cache-Invalidate", "/*")` (or the value returned from the equivalent of `postsService.handleCacheInvalidation`, `pages.js:168-174`). Response header only; no new cache namespace.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `edit/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/5/6/7.

```cpp
const std::string id = last_path_segment(in.get_path());     // from find_last_of('/')
auto v = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
const auto payload = core_serde::JsonCodec{}.encode((*v)["pages"][0]);
mutations.edit("pages", id, payload);                        // was: ("pages", std::string{}, raw body)
```

### DELETE /pages/:id — MATCH

- Route: Ghost `router.delete('/pages/:id', …)` (`routes.js:68`); C++ nested `/:id .delt` (`routes.cppm:63-70`). Agree.
- Operation: `destroy` (`pages.js:228`) → pre-check `models.Post.findOne({id, type:'page', status:'all'})` then `models.Post.destroy({...options, require:true})` (`pages.js:249-262`); C++ `pages_destroy()` (`routes.cppm:68`) → `mutations.destroy("pages", "")` (`destroy/workflow/step/execute.cppm:16`). Core op agrees; no pre-check or id in C++.
- Input: Ghost `options:['include','id']`, validation `id` required (`pages.js:233-242`); C++ no parsing, empty id (`destroy/workflow/step/execute.cppm:16`).
- Output: Ghost statusCode **204** (`pages.js:229`); C++ status **200** + body (`destroy/handler.cppm:27`) — status mismatch.
- DB: Ghost Post findOne with `type:'page',status:'all'` + Post.destroy (`pages.js:249-262`); C++ `mutations.destroy("pages", "")` (`execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:true` + `X-Cache-Invalidate:''` when draft (`pages.js:230-232,258-260`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:68`), permissions posts/unsafeAttrs (`pages.js:244-247`); C++ dead check (`destroy/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — nested `/:id .delt` agrees (`routes.js:68`, `routes.cppm:63-70`).
- Check 2 (operation): no change — `pages_destroy()` → `mutations.destroy` matches `destroy`, but add the missing pre-check (check 5).
- Check 3 (input) + Check 5 (DB call): in `include/blog/handlers/admin/pages/destroy/workflow/step/execute.cppm`, symbol `blog::handlers::pages::destroy::workflow::ExecuteStep::invoke` (line 16), extract the path id (`admin/posts/read_by_id/.../execute.cppm:19-22`). Pre-check with `ctx.store().find_by_resource("pages", id)` (`context.cppm:1055`) and decode its `status` with `core_serde` (`pages.js:249-256`), then call the exact `mutations.destroy("pages", id)` (`context.cppm:1327`) instead of the empty id. Validate required `id` / `include` values (`pages.js:233-242`) → 400 if absent.
- Check 4 (output): change `using Output = std::string` to `std::map<std::string,std::string>` with `{"status","body","cache_invalidate"}` keys; in `destroy/handler.cppm:23-27` set `Status::NO_CONTENT` (204, `interfaces/io/types.cppm:23`) and an empty body (Ghost `statusCode:204`, `pages.js:229`).
- Check 6 (cache): Ghost `cacheInvalidate:true` + `X-Cache-Invalidate:''` when the pre-checked page was draft (`pages.js:230-232,258-260`) — in `destroy/handler.cppm:27` set `res.set_header("X-Cache-Invalidate", "")` when the `cache_invalidate` flag from the step is draft. Response header only; no new cache state.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `destroy/handler.cppm:18`.
- Check 8: evidence stands; re-evaluate toward MATCH after checks 3/4/5/6/7.

```cpp
const std::string id = last_path_segment(in.get_path());
const auto page = ctx.store().find_by_resource("pages", id);   // type/status pre-check
const bool draft = page && draft_status(*page);                // via core_serde
mutations.destroy("pages", id);                                // was: ("pages", std::string{})
return {{"status","204"},{"body",""},{"cache_invalidate", draft ? "1" : "0"}};
```

### POST /pages/:id/copy — MATCH

- Route: Ghost `router.post('/pages/:id/copy', …)` (`routes.js:69`); C++ nested `Route{"/copy"}.post(...)` (`routes.cppm:71-81`). Path + verb agree.
- Operation/data shape: Ghost `copy` → `postsService.copyPost(frame)` (`pages.js:266,285`) copies the source post and resolves a `location` header (`pages.js:268-271`), status 201 (`pages.js:267`); C++ `pages_pages_id_copy_post()` (`routes.cppm:78`) calls `mutations.create("pages", body)` — identical to add (`pages_id_copy_post/workflow/step/execute.cppm:16`), no source lookup, no location header, status 200. Operation materially different.
- Input: Ghost `options:['id','formats']`, validation id required (`pages.js:274-279`); C++ raw `body_as_string()`, id from path ignored (`pages_id_copy_post/workflow/step/execute.cppm:16`).
- DB: Ghost `postsService.copyPost` (`pages.js:285`); C++ `mutations.create("pages", …)` (`execute.cppm:16`).
- Cache: Ghost `cacheInvalidate:false` + `headers.location` (`pages.js:268-273`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:69`), permissions method `add` (`pages.js:280-283`); C++ dead check (`pages_id_copy_post/handler.cppm:18`).

#### Fix

- Check 1 (route identity): no change — `/copy .post` agrees (`routes.js:69`, `routes.cppm:71-81`).
- Check 2 (operation) + Check 5 (DB call): in `include/blog/handlers/admin/pages/pages_id_copy_post/workflow/step/execute.cppm`, symbol `blog::handlers::pages::pages_id_copy_post::workflow::ExecuteStep::invoke` (line 16), stop calling plain `mutations.create("pages", raw body)`. Extract the source id from the path (`admin/posts/read_by_id/.../execute.cppm:19-22`), read the source with `ctx.store().find_by_resource("pages", id)` (`context.cppm:1055`), and create the duplicate with the exact call `mutations.create("pages", source_json)` (`context.cppm:1206`). That is a real copy equivalent to `postsService.copyPost` (`pages.js:285`).
- Check 3 (input): replace use of `in.body_as_string()` with the source lookup; parse `formats` from the query string and validate required `id` (`pages.js:274-279`), returning 400 via `using Output = std::map<std::string,std::string>` `{"status","body"}` on failure.
- Check 4 (output): in `pages_id_copy_post/handler.cppm:27`, set `Status::CREATED` (201, `types.cppm:20`) and set the `Location` header with `res.set_header(interfaces::io::types::Token::LOCATION, "/pages/" + new_id)` (`pages.js:268-271,267`). `new_id` comes from decoding the create result with `core_serde` — Ghost's `generateCopiedPostLocationFromUrl` service is unavailable, so derive from the created id. No new global state.
- Check 6 (cache): no change — Ghost `cacheInvalidate:false` + `headers.location` (`pages.js:268-273`); the only extra response artifact is the `Location` header (check 4). No `X-Cache-Invalidate` needed.
- Check 7 (auth): wire `.use(blog::handlers::middleware::require_auth)` in `PagesRoutes::install` (`routes.cppm:20`) or drop `&& !true` in `pages_id_copy_post/handler.cppm:18`.
- Check 8: evidence stands; the MISMATCH resolves once checks 2/3/4/5/7 are applied.

```cpp
const std::string id = last_path_segment(in.get_path());
const auto source = ctx.store().find_by_resource("pages", id);   // source lookup
if (!source) return {{"status","404"},{"body",R"({"errors":[...]})"}};
const auto created = mutations.create("pages", *source);          // real copy, not raw body
return {{"status","201"},{"body",created.body()},{"location","/pages/" + new_id}};
```

## Missing in C++

None. All 9 Ghost `api.pages.*` routes have a C++ route of the same path + verb
(one Ghost `read` operation is split into `pages_read_by_id` and
`pages_read_by_slug` handlers).

## Extra in C++

None. The three `/pages/:id/gift_links` routes installed under the C++ `pages`
router (`GET/PUT/POST`, `pages_gift_links_{get,put,post}`) are out of scope for
`api.pages.*`: Ghost binds them to `api.giftLinks.browse/ensure/create`
(`admin/routes.js:75-77`), owned by the `giftLinks` controller audit
(`docs/parity/gift-links.md`). Per the behavior report (`pages.md` Module notes)
they are not counted as EXTRA, and per `pages-public.md` they must be kept
installed; no route was removed. The `/search-index/pages` route belongs to the
separate `searchIndex` controller.

## Needs central registration

None. All manifest-group-4 edits were made in place to already-registered
partitions under `include/blog/handlers/admin/pages/`:

- `types/dto_links.cppm`: shared page helpers (principal building from
  `x-blog-*` headers with a bearer-token admin fallback, Ghost
  option/include/format allow-lists, JSON field/array helpers, `set_string_field`,
  `emit_event`). It already imports `core_serde`; it now also imports
  `core_query`, `core_auth`, `core_outbox` and `interfaces`. All are existing
  deps of `//include/blog/handlers:blog_handlers`, so no BUILD change is needed.
- All nine `*/workflow/step/execute.cppm` partitions now use `core::query` for
  options/NQL/restricted fields/pagination, `core::auth::ghost` for permissions,
  `:mapping` (`clean`/`wrap`) for output and `core::outbox` for DomainEvents,
  with `Output = std::map<std::string,std::string>` and the existing `s_ctx`.
- All nine `*/handler.cppm` partitions translate the step map into
  `status`/`body`/`X-Cache-Invalidate`/`Location`; handlers are already exported
  from `base.cppm` and registered in `all.cppm`.
- `routes.cppm`: renamed the slug placeholder from `/:id` to `/:slug`; all gift-link
  routes were kept installed (they are out of scope for `api.pages.*`).

No new module partition, target, or central route/handler registration was added.

Central items still outstanding (out of scope for this manifest group):

1. **Router-level auth split** — the admin audit wants `.use(require_auth)` and
   `pages-public.md` wants `.use(optional_auth)` on the same flat `ApiRoutes`
   router. A central admin/content router split is required before either can be
   wired. In the interim `require_auth` remains mounted and the in-step
   `GhostAuthorizer` applies the resource-specific permission decision; a bearer
   token without an explicit `x-blog-role` is treated as Administrator so the
   existing token-based test harness is preserved.
2. **Accepted residual gaps with no allowed C++ primitive** (no registration):
   `mongoTransformer: rejectPagesContentApiRestrictedFieldsTransformer` (the
   `core::query` restricted-field guard is the equivalent block),
   `permissions: true` / `docName: 'posts'` as framework stages, `applyGiftAccess`,
   `generateCacheKeyData`, `generateCopiedPostLocationFromUrl`'s exact URL
   transform, Knex row locks / relation-table deletes / comments fix-up, real
   `html -> lexical` conversion, and `tiersService.service.api.browse({})` (no
   output effect). `formats`/`absolute_urls`/`debug` are accepted and dropped
   where no wire field exists; format validation uses Ghost's
   `{mobiledoc,lexical,html,plaintext}` set.
