# Parity: offers-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/offers-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (content API)
- C++: `include/blog/handlers/membership/offers/routes.cppm` (installed via `include/blog/handlers/routes.cppm:34,107`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Superseded by the Group 45 strict-parity rewrite (2026-11-20); see the final section
and `docs/parity/behavior/offers-public.md`, which is the current behavior report.
The three admin routes below are owned by `offers.js` and are not EXTRA for this
public controller.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/offers/:id` | `offersPublic.read` (`docName: 'offers'`) | `GET /offers/:id` (`Router "/offers"` + `Route{"/:id"}`, `.get`) | `offers_read_by_id().handle` → `ReadByIdHandler` → `offers_read_by_id_execute` | Ghost `data: ['id']` passed to `getOffer(frame.data)`; C++ `Input=IRequest&`, **ignores** request `id`, hardcodes `"1"` | Ghost `{data:[offer]}`, 404 `NotFoundError('Offer not found.')` when absent; C++ `{"offers":[...]}` / `{"offers":[]}`, **always 200**, no 404 | Ghost `offersService.api.getOffer(frame.data)`; C++ `ctx.store().find_by_resource("offers", "1")` (hardcoded id, no `:id` lookup) | Ghost `headers.cacheInvalidate: false` (no invalidation, no read); C++ none (no cache key/namespace in step) | Ghost `mw.authenticatePublic` + controller `permissions: true`; C++ `content_type_middleware` only, token check is dead branch (`&& !true`) → auth effectively absent (pre-rewrite); Group 45 adds the Ghost permission gate | MATCH | Ghost: content/routes.js:42; offers-public.js:11,13-20,21-29. C++: offers/routes.cppm:19-20,37-45; read_by_id/handler.cppm:17-27; read_by_id/workflow/step/execute.cppm:11,16-18 |
| — (none) | — | `GET /offers` (root, `.get`) | `offers_browse().handle` | C++ only | — | `json_for("offers", {})` (offers/browse execute.cppm:16) | none | token check dead branch | n/a (admin `offers.js`) | C++: offers/routes.cppm:21-28; all.cppm:90,436,686. No `browse` key in offers-public.js; content/routes.js has no GET `/offers` |
| — (none) | — | `POST /offers` (root, `.post`) | `offers_create().handle` | C++ only (body) | — | `mutations.create("offers", body)` | none | token check dead branch | n/a (admin `offers.js`) | C++: offers/routes.cppm:29-36; all.cppm:91,437,686. No `add` key in offers-public.js; content/routes.js exposes no POST |
| — (none) | — | `PUT /offers/:id` (`Route{"/:id"}`, `.put`) | `offers_edit().handle` | C++ only (body) | — | `mutations.edit("offers", "", body)` | none | token check dead branch | n/a (admin `offers.js`) | C++: offers/routes.cppm:47-54; all.cppm:92,438,687. No `edit` key in offers-public.js; content/routes.js exposes no PUT |

## Route details

### GET `/offers/:id` — MISMATCH

Ghost route: `router.get('/offers/:id', mw.authenticatePublic, http(api.offersPublic.read))`
(`docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:42`). Controller key resolved to
`read` in `docs/Ghost/ghost/core/server/api/endpoints/offers-public.js:13-31`, `docName: 'offers'` (line 11).

C++ route: `Router{router, "/offers"}` + `.add_route(Route{"/:id"}.get(...))`
(`include/blog/handlers/membership/offers/routes.cppm:19,37-45`) → `handler.offers_read_by_id().handle(...)`
(routes.cppm:44) → `ReadByIdHandler::handle` (`read_by_id/handler.cppm:17-28`) → workflow `offers_read_by_id`
(`read_by_id/workflow/flow/main.cppm:9`) → `offers_read_by_id_execute` step
(`read_by_id/workflow/step/execute.cppm:12`). Path + verb and core operation (read-by-id) agree.

Divergences:

- **Input**: Ghost declares `data: ['id']` and calls `offersService.api.getOffer(frame.data)` — the
  request `id` drives the lookup (`offers-public.js:17,20`). C++ step declares
  `using Input=interfaces::io::IRequest&` (`execute.cppm:11`) but never reads the `:id` param; it calls
  `find_by_resource("offers", std::string{}.empty()?"1":std::string{})` — a hardcoded `"1"`
  (`execute.cppm:16`). Any id other than `"1"` returns the wrong offer.
- **Output envelope**: Ghost returns `{data: [offer]}` (`offers-public.js:27-29`). C++ returns
  `{"offers":[<row>]}` or `{"offers":[]}` (`execute.cppm:17-18`) — top-level key `offers` vs `data`.
- **Status / not-found**: Ghost throws `NotFoundError('Offer not found.')` when no offer
  (`offers-public.js:21-25`) → 404. C++ handler always calls
  `res.set_status(Status::OK)` (`handler.cppm:27`) and returns `{"offers":[]}` on miss — never 404.
- **DB call**: Ghost uses service `offersService.api.getOffer(frame.data)` (`offers-public.js:20`),
  not a direct model call. C++ uses `ctx.store().find_by_resource("offers", "1")`
  (`execute.cppm:16`); `mutations` is declared but unused (`execute.cppm:14`). Resource string `offers`
  matches; lookup value and mechanism do not.
- **Cache**: Ghost sets `headers.cacheInvalidate: false` (`offers-public.js:14-16`) — no invalidation,
  no read. C++ step has no `core::cache::Cache` usage; the shared `m_cache` in `all.cppm:836-846`
  is only wired to members/emails/automations operations (`all.cppm:837-839`). Both sides: no cache
  behavior for this route — no divergence of consequence.
- **Auth**: Ghost uses `mw.authenticatePublic` (content API key) on the route
  (content/routes.js:42) plus controller `permissions: true` (`offers-public.js:18`). C++ applies
  only `content_type_middleware` (`offers/routes.cppm:20`) and the guard in `handler.cppm:18`
  is `if (token(req).empty() && !true)` — `&& !true` makes it unreachable, so the 401 branch
  (handler.cppm:19-21) never fires. Bearer/public-key auth is effectively absent on the C++ side.

No differences beyond those listed (workflow name `offers_read_by_id` and `docName: 'offers'` agree;
handler registry `all.cppm:23,369,618` and central install `routes.cppm:34,107` are present).

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the MISMATCH on `GET /offers/:id`
(Ghost `offers-public.js:13-31`, `content/routes.js:42`):

- **Check 1 (route identity) — closed, no change.** `include/blog/handlers/membership/offers/routes.cppm:38-45` already
  binds `Route{"/:id"}.get(...)` under `Router{router, "/offers"}` → `GET /offers/:id`. Keep.
- **Check 2 (operation mapping) — closed, no change.** `routes.cppm:44` binds `handler.offers_read_by_id().handle`;
  Ghost key `offersPublic.read`. Keep both.
- **Check 3 (input parity) — fix.** `read_by_id/workflow/step/execute.cppm:16` ignores `Input=interfaces::io::IRequest&`
  (`execute.cppm:11`) and passes hardcoded `"1"`. Replace lookup value with request `id` extracted via
  `interfaces::io::IRequest::get_path()` (`include/interfaces/io/request.cppm:807`). Exact replacement call:
  `ctx.store().find_by_resource("offers", id)` where `id` = last non-empty path segment. Canonical in-repo pattern:
  `admin/posts/read_by_id/workflow/step/execute.cppm:19-23`.
- **Check 4 (output parity) — fix.** `execute.cppm:17-18` emits `{"offers":[...]}` / `{"offers":[]}`; Ghost returns
  `{data:[offer]}` (`offers-public.js:27-29`). Replace bodies with `{"data":[" + row + "]}` and `{"data":[]}`.
  `handler.cppm:27` hardcodes `Status::OK`; on miss return 404 (`interfaces::io::types::Status::NOT_FOUND`) to match
  `NotFoundError('Offer not found.')` (`offers-public.js:21-25`). Canonical status pattern:
  `admin/posts/read_by_id/handler.cppm:25-36` (returns `{"status","body"}` map, casts to
  `interfaces::io::types::Status`).
- **Check 5 (DB call parity) — fix + caveat.** Resource string `offers` already matches (`execute.cppm:16`). Caveat:
  `ApiDataStore::find_by_resource("offers", id)` (`include/blog/context/context.cppm:1077`) ignores `id` and returns the
  single `encode_offer_wire()` fixture, so any id resolves to the same record. To fully honor id lookup, write per-id
  offer records through the context-local `ResourceStore` (`context.cppm:638-645`, tag/namespace `resources`, via
  `BlogContext::store()`), then read with `ctx.store().find_by_resource("offers", id)`. No direct SQL / no global state.
- **Check 6 (cache parity) — closed, no change.** Ghost `headers.cacheInvalidate:false` (`offers-public.js:14-16`); C++
  step has no `core::cache::Cache` use (`all.cppm:836-839` wires `m_cache` only to members/emails/automations). Both sides:
  no cache behavior.
- **Check 7 (auth) — fix.** `read_by_id/handler.cppm:18` guard is `token(req).empty() && !true` (dead). Replace with
  `blog::handlers::middleware::token(req).empty()` (`include/blog/handlers/middleware/bearer.cppm:27`) so the 401 branch
  (`handler.cppm:19-21`) fires — mirrors `require_auth` (`middleware/auth.cppm:12-29`) and Ghost `mw.authenticatePublic`
  (`content/routes.js:42`).
- **Check 8 (verdict) — closed.** After checks 3/4/5/7: path, verb, input name `id`, output envelope `data`, DB resource
  `offers`, no cache, and token auth agree → MATCH.

Short snippet where the exact replacement is non-trivial (step: id extraction + envelope + not-found):

```cpp
// read_by_id/workflow/step/execute.cppm — replace lines 16-18
auto path = std::string_view{std::string{in.get_path()}}.substr(0, std::string{in.get_path()}.find('?'));
while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
const auto slash = path.find_last_of('/');
const std::string id{slash == std::string_view::npos ? path : path.substr(slash + 1)};
const auto one = ctx.store().find_by_resource("offers", id);
if (!one) return core::workflow::StepResponse<Output>{R"({"data":[]})"};
return core::workflow::StepResponse<Output>{"{\"data\":[" + *one + "]}"};
```

Handler status (replace `handler.cppm:27`): derive status from the step result on miss — emit
`interfaces::io::types::Status::NOT_FOUND` when `one` is absent, else `Status::OK` (pattern:
`admin/posts/read_by_id/handler.cppm:30,34`). Handler auth guard (replace `handler.cppm:18`):
`if (blog::handlers::middleware::token(req).empty())`.

## Missing in C++

None. The sole Ghost route (`GET /offers/:id`, content/routes.js:42) has a C++ path+verb counterpart
(`offers/routes.cppm:37-45`), so this public/content controller is not fully MISSING despite the
general expectation for `*-public` domains.

## Extra in C++

C++ routes in the matched folder `include/blog/handlers/membership/offers/routes.cppm` with no
counterpart in `offers-public.js` / the content routes table:

1. `GET /offers` → `offers_browse().handle` (routes.cppm:21-28). This is an admin-API `browse`
   operation; `offers-public.js` exposes no `browse`.
   - Fix: DOCUMENTATION ONLY. File `include/blog/handlers/membership/offers/routes.cppm`, symbol
     `OffersRoutes::install` (lines 21-28): remove the root `.get(...)` binding that calls
     `handler.offers_browse().handle` from the `Router{router, "/offers"}` chain, or relocate it to the admin offers
     routes. Keep `offers_browse()` registered in `include/blog/handlers/all.cppm:90,436,686` only if admin `offers.js`
     owns it. Checks 1-2: no Ghost route/op in this controller → nothing to align. Checks 3-7: n/a here — its step
     (`browse/workflow/step/execute.cppm:16-17`) uses `json_for("offers", {})`, hardcoded `limit/page/pages/total`, no
     `core::cache::Cache`, dead auth branch; all irrelevant to the public surface. Check 8: EXTRA until removed/relocated.
     Snippet (exact binding to delete from `OffersRoutes::install`):
     ```cpp
     .get([&handler](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
         handler.offers_browse().handle(req, res, std::move(send));
     })
     .summary("Ghost offers GET")
     ```
2. `POST /offers` → `offers_create().handle` (routes.cppm:29-36). Admin-API `add`/create;
   `offers-public.js` exposes no `add`.
   - Fix: DOCUMENTATION ONLY. File `include/blog/handlers/membership/offers/routes.cppm`, symbol
     `OffersRoutes::install` (lines 29-36): remove the root `.post(...)` binding that calls
     `handler.offers_create().handle` from the `Router{router, "/offers"}` chain, or relocate it to the admin offers
     routes. Keep `offers_create()` registered in `include/blog/handlers/all.cppm:91,437,686` only if admin `offers.js`
     owns it. Checks 1-2: no Ghost route/op in this controller → nothing to align. Checks 3-7: n/a here — its step
     (`create/workflow/step/execute.cppm:16`) calls `mutations.create("offers", body)`, no cache, dead auth branch;
     `content/routes.js` exposes no POST. Check 8: EXTRA until removed/relocated.
     Snippet (exact binding to delete from `OffersRoutes::install`):
     ```cpp
     .post([&handler](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
         handler.offers_create().handle(req, res, std::move(send));
     })
     .summary("Ghost offers POST")
     ```
3. `PUT /offers/:id` → `offers_edit().handle` (routes.cppm:47-54). Admin-API `edit`;
   `offers-public.js` exposes no `edit`.
   - Fix: DOCUMENTATION ONLY. File `include/blog/handlers/membership/offers/routes.cppm`, symbol
     `OffersRoutes::install` (lines 47-54): remove the `Route{"/:id"}.put(...)` binding that calls
     `handler.offers_edit().handle`, or relocate it to the admin offers routes. Keep `offers_edit()` registered in
     `include/blog/handlers/all.cppm:92,438,687` only if admin `offers.js` owns it. Checks 1-2: no Ghost route/op in this
     controller → nothing to align. Checks 3-7: n/a here — its step (`edit/workflow/step/execute.cppm:16`) calls
     `mutations.edit("offers", "", body)` (empty id), no cache, dead auth branch; `content/routes.js` exposes no PUT.
     Check 8: EXTRA until removed/relocated.
     Snippet (exact binding to delete from `OffersRoutes::install`):
     ```cpp
     .put([&handler](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
         handler.offers_edit().handle(req, res, std::move(send));
     })
     .summary("Ghost :id PUT")
     ```

Note: these are extra relative to *this* controller. Ghost likely maps them in the admin offers
controller (`offers.js`), which is outside this subagent's scope.

## Group 38 — applied source fixes (2026-11-19)

This controller's own `#### Fix` blocks stay DOCUMENTATION ONLY — no source edit was applied from them. The shared in-scope files were changed by the admin `offers.js` group-38 work (`offers.md`), which exercises the same `Router{router, "/offers"}` block. Edits stayed inside `include/blog/handlers/membership/offers/`; no shared file touched. The three admin-only routes (`GET /offers`, `POST /offers`, `PUT /offers/:id`) remain installed because `offers.js` owns them — removing them is out of scope for this controller.

| File | Fix applied |
|---|---|
| `read_by_id/workflow/step/execute.cppm` | `Output=std::map`; `:id` parsed from `in.get_path()` last segment instead of the hardcoded `"1"`; `{"data":[offer]}` on hit; 404 `ApiError{}.json("Offer not found.","Not Found")` on miss. Closes this doc's checks 3 and 4. |
| `read_by_id/handler.cppm` | Status/body read from payload (404 can now propagate); `X-Cache-Invalidate: false`; dead `token(req).empty() && !true` guard removed. |
| `routes.cppm` | Added `.use(blog::handlers::middleware::require_auth)` after `content_type_middleware`; bearer token is now actually enforced in place of the dead handler guard. |

### Central registrations needed

None. Central registrations already present in shared files:

- `include/blog/handlers/routes.cppm:34,107` install `OffersRoutes` (one unified router; no second content route added).
- `all.cppm:369,436-438,618,685-687` register the four offers handlers; keep them.
- `blog_handlers_middleware` exports `require_auth`/`token`; `routes.cppm` already imports the module.

Residuals needing shared-file work (out of scope):

- `ApiDataStore::find_by_resource("offers", id)` (`context/context_context.cppm:1093`) ignores `id` — the parsed `:id` does not actually narrow the offer lookup, so this doc's check 5 caveat stands.
- Ghost content-API-key validation (`mw.authenticatePublic`, `content/routes.js:42`) is approximated by `require_auth`'s bearer-token check; exact key semantics and `permissions:true`/`docName` visibility have no allowed C++ primitive.

## Group 45 — strict-parity rewrite (2026-11-20)

This section supersedes the older `#### Fix` blocks above (which were documentation
only). Edits were confined to `include/blog/handlers/membership/offers/`; no shared
file, BUILD file or Bazel cycle was touched.

| File | Strict-parity change |
|---|---|
| `read_by_id/workflow/step/execute.cppm` | Real `:id` parse; id-filtered `core::query::apply` over `support::clean(json_for("offers",{}))` makes the 404 branch reachable; `support::offer_dto` + `support::wrap` emit `{"offers":[OfferDTO]}` (the shared output serializer maps controller `data` to the `offers` docName; the older `{data:[offer]}` claim was the controller-stage value). |
| `read_by_id/handler.cppm` | Ghost permission gate via `support::authorize(...,"read")` (`core_auth:ghost`); status from payload; **no** `X-Cache-Invalidate` header. |
| `browse/workflow/step/execute.cppm` | Hosts the shared `blog::handlers::offers::support` helpers (`core_query`, `:outbox_shim`, `blog_handlers_middleware`, local OfferDTO allow-list/envelope) in an already-registered partition; other steps import `:offers_browse_step`. |

Accepted residuals (no C++ primitive): content-API key auth/rate limiting/CORS/
redirects, real Knex `findOne` + redemption aggregation, full `OfferMapper.toDTO`,
and the public-vs-admin catalog split (one C++ route serves both catalogs). The
verdict is MATCH on the primary observable contract: 200/404, no cache header,
`{"offers":[OfferDTO]}` and Ghost-shaped errors. See
`docs/parity/behavior/offers-public.md` for the current evidence.
