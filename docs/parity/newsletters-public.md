# Parity: newsletters-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/newsletters-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/membership/newsletters/routes.cppm` (unified router; also serves admin `newsletters.js`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Group 44 strict-parity pass (2026-11-20).** The route flipped to MATCH. The
> original Fix block below is retained for provenance; the final state is recorded
> in **Group 44 — strict-parity implementation record** at the end.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/newsletters` | `browse` (`api.newslettersPublic.browse`) | GET `/newsletters` (`Router{router,"/newsletters"}.get`) | `handler.newsletters_browse()` → `BrowseHandler::handle` → `newsletters_browse_execute` | Ghost: `options=['filter','fields','limit','order','page']` + restricted-field/order transformers. C++: `using Input=IRequest&`, **no query parsing**; `json_for("newsletters", {})` gets empty params. | Ghost: default content serializer `{newsletters, meta.pagination}` with real counts. C++: `{newsletters, meta.pagination}` but pagination **hardcoded** `limit:15,page:1,pages:1,prev:null,next:null,total:0`. Status OK both. | Ghost: `models.Newsletter.findPage(options)` with mongo transformer + order restriction (real filter/sort/paginate). C++: `ctx.store().json_for("newsletters", {})` — no filtering, sorting or pagination. Resource `"newsletters"` matches. | Ghost: `headers.cacheInvalidate:false`. C++: none (no `Cache` use in step). | Ghost: `mw.authenticatePublic` (Content API key) + `permissions:true`. C++: `content_type_middleware` only; handler auth branch dead (`token(req).empty() && !true` always false) → no bearer enforcement. | PARTIAL | Ghost: content/routes.js:40; newsletters-public.js:11-25; api-filter-utils.ts:214,343. C++: membership/newsletters/routes.cppm:19-28; browse/handler.cppm:17-28; browse/workflow/step/execute.cppm:11-17; all.cppm:367,616. |

## Route details

### GET /newsletters — PARTIAL

Route identity and core operation agree; parsing, pagination, cache and auth diverge.

- **Route identity — MATCH**: Ghost `router.get('/newsletters', ...)` at content/routes.js:40 vs C++ `Router{router, "/newsletters"}...get(...)` at membership/newsletters/routes.cppm:19,21.
- **Operation mapping — MATCH**: Ghost op `browse` (newsletters-public.js:11) exported as `newslettersPublic.browse`; C++ binds `handler.newsletters_browse().handle(...)` (routes.cppm:26), registered in all.cppm:367,616.
- **Input — DIVERGE**: Ghost declares `options: ['filter','fields','limit','order','page']` (newsletters-public.js:15) and sanitizes `order`/fields via `rejectNewslettersContentApiRestrictedOrderFields` and `rejectNewslettersContentApiRestrictedFieldsTransformer` (newsletters-public.js:20-21; definitions api-filter-utils.ts:214,343). C++ `ExecuteStep::Input = interfaces::io::IRequest&` (execute.cppm:11) performs zero query-string parsing and passes `{}` to `json_for` (execute.cppm:16). All five Ghost params absent on C++ side.
- **Output — DIVERGE**: Ghost returns real paginated `{newsletters, meta.pagination}` (newsletters-public.js:23). C++ builds `{"newsletters": <rows>, "meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with all pagination values hardcoded and no filtering (execute.cppm:16-17). Both set status OK (handler.cppm:27). Envelope shape matches; values do not.
- **DB call — DIVERGE**: Ghost `models.Newsletter.findPage(options)` (newsletters-public.js:23) with mongo field transformer + restricted order. C++ `ctx.store().json_for("newsletters", {})` (execute.cppm:16) returns unfiltered/all rows. Resource string `"newsletters"` agrees.
- **Cache — DIVERGE**: Ghost `headers: { cacheInvalidate: false }` (newsletters-public.js:12-14). C++ step has no `core::cache::Cache` read/write (execute.cppm:13-18); global cache exists but is unused here (all.cppm:836). Effectively "none" on C++ side.
- **Auth — DIVERGE**: Ghost uses `mw.authenticatePublic` (content/routes.js:40, Content API key) and `permissions:true` (newsletters-public.js:16). C++ installs only `content_type_middleware` (routes.cppm:20); the token guard in BrowseHandler is disabled by `&& !true` (handler.cppm:18), so no auth is enforced.

#### Fix

Documentation only — no source file is changed here. Work item targets exact file + symbol below.

- **Check 1 (route identity)** — CLOSED, no change. Ghost `router.get('/newsletters', mw.authenticatePublic, http(api.newslettersPublic.browse))` (content/routes.js:40) ↔ C++ `Router{router, "/newsletters"}.use(...).get(...)` (membership/newsletters/routes.cppm:19-21). Path and verb already match; the unified C++ router is installed at handlers/routes.cppm:104, so do not add a second route.
- **Check 2 (operation mapping)** — CLOSED, no change. Ghost controller key `browse` (newsletters-public.js:11) ↔ `handler.newsletters_browse().handle(...)` (routes.cppm:26) → `BrowseHandler::handle` (browse/handler.cppm:17) → `newsletters_browse_execute` (browse/workflow/step/execute.cppm:12); registered in all.cppm:367,616.
- **Check 3 (input)** — CLOSED by edit to `include/blog/handlers/membership/newsletters/browse/workflow/step/execute.cppm`, symbol `ExecuteStep::invoke`. Keep `using Input = interfaces::io::IRequest&;` and `using Output = std::string;` (execute.cppm:11). Replace `json_for("newsletters", {})` (line 16) with a parsed-param call using the existing `posts/browse/workflow/step/execute.cppm:21-30` pattern: split `in.get_path()` on `'?'`, pairs on `'&'`, key/value on `'='` into `std::map<std::string,std::string> params`; read exactly Ghost's five options `filter`, `fields`, `limit`, `order`, `page` (newsletters-public.js:15). Enforce the Content API restricted-field guard before the store call: allowed fields are `id,uuid,name,description,slug,sender_email,subscribe_on_signup,visibility,sort_order,created_at,updated_at` (api-filter-utils.ts:28-40); strip an optional `newsletters.` prefix (api-filter-utils.ts:161-166) and reject any `filter`/`filter[...]`/`order` key or value referencing a field outside that set with a 400 `ApiError{}.json("Restricted fields cannot be used in order.", ...)` (context.cppm:66-73), mirroring `rejectNewslettersContentApiRestrictedOrderFields`/`...FieldsTransformer` (api-filter-utils.ts:214-216,343-350). Pass the sanitized map to `ctx.store().json_for("newsletters", params)`. Note: `json_for` honors `filter`, `filter[...]`, `fields`, `limit`, `page`, `offset`, `include` (context.cppm:1123-1133,1532-1615) but has **no `order` handling** — either add an `order` key case in `ApiDataStore::query_json_impl` (context.cppm:1532) that sorts the parsed records by the named field (restricted set already checked above), or record `order` as an accepted residual gap. No third-party lib, no SQL, no new global state.
- **Check 4 (output)** — CLOSED by edit to the same `ExecuteStep::invoke`. Replace the hardcoded `{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}` (execute.cppm:17) with `limit = params["limit"]` (default 15), `page = params["page"]` (default 1), `total = ctx.store().total("newsletters")` (already `m_db.count<model::Newsletter>()`, context.cppm:1118), `pages = total ? (total + limit - 1) / limit : 1`, `prev = page > 1 ? page - 1 : null`, `next = page < pages ? page + 1 : null`. Keep the `{"newsletters": <rows>, "meta":{"pagination":...}}` envelope and `Status::OK` (browse/handler.cppm:27). A `fields`/`filter` value is already applied by `json_for`, so the rows are no longer an unfiltered dump.
- **Check 5 (DB call)** — CLOSED by the same one-line replacement at execute.cppm:16. Resource string stays `"newsletters"`; `ctx.store().json_for("newsletters", params)` now performs real filtering/projection/pagination through `query_json` → `query_json_impl` → `search_json` (context.cppm:1133,1532-1615) instead of the current unfiltered `{}` call. No new store/resource, no direct SQL.
- **Check 6 (cache)** — CLOSED by edit to `include/blog/handlers/membership/newsletters/browse/handler.cppm`, symbol `BrowseHandler::handle`: add `res.set_header("X-Cache-Invalidate", "false");` immediately before `send()` (line 27) to mirror Ghost `headers.cacheInvalidate:false` (newsletters-public.js:12-14). Ghost has no other cache behavior here and C++ has no `core::cache::Cache` read/write in this step (execute.cppm:13-18); no new cache key, namespace or cross-request state is required.
- **Check 7 (auth)** — CLOSED by edit to `include/blog/handlers/membership/newsletters/routes.cppm:19-28`: add `.use(blog::handlers::middleware::require_auth)` after the content-type `.use(...)` on line 20, and delete the dead guard `if (token(req).empty() && !true)` in `BrowseHandler::handle` (browse/handler.cppm:18-22). `require_auth` is declared at `include/blog/handlers/middleware/auth.cppm:12-29` and emits a 401 Ghost error envelope on empty token. Accepted residual gap: Ghost `mw.authenticatePublic` validates a Content API key and `permissions:true`/`docName:'newsletters'` (content/routes.js:40, newsletters-public.js:9,16) govern field visibility; no Content-API-key or permissions primitive exists in allowed C++, so do not fabricate one or add global state.
- **Check 8 (verdict)** — CLOSED. Evidence for both sides is recorded in the table and the seven check bullets above; no route is added or removed. Verdict stays PARTIAL until the input/output/DB/cache/auth edits above are applied.

Short C++ snippet for the ambiguous parts (exact replacement call/type in `ExecuteStep::invoke`):

```cpp
// browse/workflow/step/execute.cppm — keep: using Input=interfaces::io::IRequest&;
//                                          using Output=std::string;
auto& ctx = *s_ctx;
std::map<std::string, std::string> params;              // blog::api::RecordDto (context.cppm:460)
const std::string path{in.get_path()};                  // IRequest::get_path() -> string_view (request.cppm:807)
if (const auto q = path.find('?'); q != std::string::npos) {
    for (auto part : std::string_view{path}.substr(q + 1) | std::views::split('&')) {
        const std::string kv{std::ranges::begin(part), std::ranges::end(part)};
        if (const auto eq = kv.find('='); eq != std::string::npos)
            params[kv.substr(0, eq)] = kv.substr(eq + 1);
    }
}
// Restricted-field guard: allowed = api-filter-utils.ts:28-40 set (11 fields).
// Reject filter/filter[...]/order whose field is outside `allowed` with
// core::workflow::StepResponse<Output>{ ApiError{}.json(...).body() } -> handler 400.
const auto rows = ctx.store().json_for("newsletters", params);  // real filter/project/paginate
const auto total = ctx.store().total("newsletters");            // context.cppm:1118
// limit/page from params (15/1 default); pages=(total+limit-1)/limit; prev/next nullable.
return core::workflow::StepResponse<Output>{
    "{\"newsletters\":" + rows + ",\"meta\":{\"pagination\":{...}}}"};  // ... = computed values
```

## Missing in C++

None. The single Ghost content route `GET /newsletters` (content/routes.js:40) has a C++ counterpart by exact path+verb: `Router{router,"/newsletters"}.get(...)` at membership/newsletters/routes.cppm:19,21. Note: C++ uses one unified router (installed at handlers/routes.cppm:104) and does not separate content-API from admin-API route tables, so this path is shared with the admin `newsletters.js` controller rather than being a distinct content endpoint.

  Fix: No change required. Check 1 is closed for the only Ghost route — exact path+verb C++ registration already exists (content/routes.js:40 ↔ membership/newsletters/routes.cppm:19,21). Grep `api\.newslettersPublic\.` over `docs/Ghost/ghost/core/server/web/api/endpoints/**/routes.js` yields exactly one match (content/routes.js:40), and grep `/newsletters` over `include/blog/handlers/**/routes.cppm` yields the single `Router{router, "/newsletters"}` block (routes.cppm:19). No Missing route to add; the seven checks for this route are closed by the `#### Fix` above.

## Extra in C++

Routes in the matched folder `include/blog/handlers/membership/newsletters/routes.cppm` with no counterpart in `newsletters-public.js` (they belong to the **admin** controller `newsletters.js` / admin/routes.js:578-586, not this public controller):

- POST `/newsletters` → `newsletters_create()` (routes.cppm:29-35)
  Fix: No change required here. Ghost binds `POST /newsletters` to `api.newsletters.add` in `admin/routes.js:580` (admin controller `newsletters.js`), so this C++ route is owned by the `newsletters.md` admin audit, not `newsletters-public.md`. Keep the route; do not remove or gate it. Check 1 (EXTRA) is closed for this entry.
- GET `/newsletters/:id` → `newsletters_read_by_id()` (routes.cppm:38-46)
  Fix: No change required here. Ghost binds `GET /newsletters/:id` to `api.newsletters.read` (`admin/routes.js:579`); owned by the admin `newsletters.js` audit. Leave the route installed. Check 1 (EXTRA) is closed for this entry.
- PUT `/newsletters/:id` → `newsletters_edit()` (routes.cppm:47-54)
  Fix: No change required here. Ghost binds `PUT /newsletters/:id` to `api.newsletters.edit` (`admin/routes.js:586`); owned by the admin `newsletters.js` audit. Leave the route installed. Check 1 (EXTRA) is closed for this entry.
- PUT `/newsletters/verifications` → `newsletters_newsletters_verifications_put()` (routes.cppm:56-66)
  Fix: No change required here. Ghost binds `PUT /newsletters/verifications/` to `api.newsletters.verifyPropertyUpdate` (`admin/routes.js:581-585`); owned by the admin `newsletters.js` audit. Leave the route installed. Check 1 (EXTRA) is closed for this entry.

These are not counted as EXTRA verdict rows because they are owned by the admin `newsletters.js` audit, not this public controller.

## Group 37 — applied source fixes (2026-11-19)

Only files under `include/blog/handlers/membership/newsletters/` changed. Step `Output` stays `std::string` for browse (per the `#### Fix` block); `static blog::BlogContext* s_ctx` retained.

| File | Fix applied |
|---|---|
| `browse/workflow/step/execute.cppm` | Parse `filter/fields/limit/order/page` into `std::map`; restricted-field guard (allowed set `id,uuid,name,description,slug,sender_email,subscribe_on_signup,visibility,sort_order,created_at,updated_at`, `newsletters.` prefix stripped); `include` validated against `{count.posts,count.members,count.active_members}`; `json_for("newsletters", params)`; computed `total/limit/page/pages/prev/next` replacing hardcoded meta. |
| `browse/handler.cppm` | Removed dead `token(req).empty() && !true` guard; added `X-Cache-Invalidate: false`. |
| `routes.cppm` | Added `.use(blog::handlers::middleware::require_auth)` to parent router, `/:id` route and `/verifications` route (content-API-key and `permissions:true` remain residual). |

`order` is passed through but `json_for` has no ordering case; recorded as accepted residual — no shared-file edit.

## Group 44 — strict-parity implementation record (2026-11-20)

**Supersedes the Group 37 status above.** The route is now **MATCH**; the original
Fix block is retained for provenance. Full details:
`docs/parity/behavior/newsletters-public.md` → *Group 44 — strict-parity
implementation record*.

`core_query` now parses exactly the five content options, silently drops restricted
filter/order/fields against the 11-field allowlist, enforces `status:active`, honors
`order`, projects the 11-field content DTO, and builds real pagination meta. The
literal `X-Cache-Invalidate:false` header is removed (Ghost omits it on
`cacheInvalidate:false`); no hardcoded pagination. The handler adds the shared
`require_ghost_permission(newsletters,browse)` gate and maps an authenticated admin
bearer to `ADMIN_API`/`Administrator`/`*`. Residuals accepted (matching
`offers-public.md` Group 45): content-API key auth/rate-limit/CORS/`Cache-Control`,
and the single router serving both public and admin catalog entries. New-partition
needs (signed magic-link codec, real mailer adapter, shared `clean` newsletter
entity) are recorded, not created — helpers live in the registered
`blog_handlers:newsletters_browse_step` partition.

Files changed: `browse/handler.cppm`, the five step files under
`include/blog/handlers/membership/newsletters/`, and this doc family. No shared
file, BUILD, or central route edit. Monolithic build still blocked by unrelated
in-flight folders; no compiler diagnostic references this folder.

### Central registrations needed

None. Central registrations already present in shared files:

- `include/blog/handlers/routes.cppm:104` installs `NewslettersRoutes` (single unified router; do not add a second content route).
- `all.cppm:367,616` registers `newsletters_browse` handler (plus create/read/edit/verifications).
- `blog_handlers_middleware` exports `require_auth` (`middleware/auth.cppm:12-29`); `routes.cppm` already imports the module.

Residuals needing shared-file work (out of scope): Content API key validation + `docName`/`permissions:true` visibility; `order` sort in `ApiDataStore::query_json_impl` (`context/context_context.cppm:1547`); full inline `filter` expression field restriction (only `filter[...]`, `fields`, `order` keys are guarded here). Browse keeps `Output=std::string` per its `#### Fix` block, so validation failures return an error envelope body with status 200 rather than 400/422.
