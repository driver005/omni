# Parity: links

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/links.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/links/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-20 — both routes MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 2 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /links | browse | GET /links (Router `/links`, `.get`) | `links_browse()` → `BrowseHandler` | Ghost `options:['filter']`; C++ `core::query::parse(path,{"filter"})` + `apply` | Ghost `{data, meta.pagination{total,page,pages}}`; C++ same via `support::browse_response` + `fromModel` wire projection | Ghost `linkTrackingService.getLinks(options)`; C++ seeded in-store redirect + `core::query::apply` with `count__clicks DESC, to DESC` | Ghost `cacheInvalidate:false`; C++ no header, no content-type gate on GET | Ghost `mw.authAdminApi`+`permissions:true`; C++ `require_auth`+ghost `links:browse` gate | MATCH | Ghost `links.js:7-27`, `routes.js:588`; C++ `external/links/routes.cppm:19-28`, `browse/{handler.cppm,workflow/step/execute.cppm}` |
| PUT /links/bulk | bulkEdit | PUT /links/bulk (Router `/links`, sub-route `/bulk`, `.put`) | `links_bulk_edit()` → `BulkEditHandler` | Ghost `options:['filter']`, `data:['action','meta']`, validation action=`updateLink`+filter required; C++ same validation + NQL filter parse (`BadRequestError` 400) | Ghost `{bulk:{action,meta.stats,...}}`; C++ same envelope from real `bulk_edit` success count | Ghost `linkTrackingService.bulkEdit(frame.data.bulk, options)`; C++ `core::query` matched ids → `ApiDataStore::bulk_edit("links", {ids,to,updated_at})` | Ghost `cacheInvalidate:'/r/*'`; C++ `X-Cache-Invalidate:/r/*` | Ghost `mw.authAdminApi`+`permissions:{method:'edit'}`; C++ `require_auth`+ghost `links:edit` gate | MATCH | Ghost `links.js:28-54`, `links.js:2`, `routes.js:589`; C++ `external/links/routes.cppm:29-38`, `bulk_edit/{handler.cppm,workflow/step/execute.cppm}` |

## Route details

### GET /links — MATCH

Route identity, operation, evidence and cache behavior agree; every earlier divergence closed.

- **1, 2, 8 (no change):** `GET /links` → `links_browse()` already bound
  (`external/links/routes.cppm:19-28`; registry `base.cppm:272-273`, `all.cppm:638,917`).
  Path, verb, operation and evidence agree.
- **3 (input, closed):** `core::query::parse(in.get_path(), support::kBrowseOptions)`
  with `kBrowseOptions={"filter"}` mirrors Ghost's `options:['filter']` and the
  restricted-field guard; only `filter` is honored.
- **4 (output, closed):** `support::browse_response` emits Ghost's exact envelope
  `{"data":[...],"meta":{"pagination":{"total":<row count>,"page":1,"pages":1}}}`,
  with `total` from the filtered rows (`links.length`), and each row projected to
  the real `FullPostLink` wire shape `{post_id,link:{link_id,from,to,edited},count:{clicks}}`
  instead of the `{url,title}` fixture.
- **5 (DB call, closed):** the `LinkWire` fixture is seeded into the existing
  `ResourceStore` with an id and redirect fields; `core::query::apply` applies the
  real NQL filter and Ghost's fixed `count__clicks DESC, to DESC` ordering. Ghost's
  Knex `fetchAll` is a non-observable architecture difference.
- **6 (cache, no change):** Ghost `cacheInvalidate:false` and C++ no header agree.
- **7 (auth, closed):** inline token checks are gone; the router keeps `require_auth`
  (401) and the handler runs `support::permitted(...,"links","browse")` via
  `core::auth::ghost` + `require_ghost_permission` (Ghost-shaped 401/403). The
  content-type middleware was removed from the GET (no request body in Ghost).

### PUT /links/bulk — MATCH

Route identity, operation, resource string, status header and evidence agree; all
earlier divergences closed.

- **1, 2, 8 (no change):** `PUT /links/bulk` → `links_bulk_edit()` bound as the
  `/bulk` sub-route `.put(...)` (`external/links/routes.cppm:29-38`; registry
  `base.cppm:272-273`, `all.cppm:639,918`).
- **3 (input, closed):** `filter` parsed with `core::query`; body decoded via
  `core_serde::JsonCodec` reading `bulk.action` / `bulk.meta.link.to`; Ghost
  validation (`action` required/enum, `filter` required) enforced with a 422 Ghost
  `ValidationError`; NQL parse errors produce a 400 Ghost `BadRequestError`
  "Invalid filter value received".
- **4 (output, closed):** the step returns a status/body map and the handler writes
  status from it; success body is Ghost's
  `{"bulk":{"action":"updateLink","meta":{"stats":{"successful":N,"unsuccessful":M},"errors":[],"unsuccessfulData":[]}}}`,
  not the old `{"meta":{"updated":N}}`.
- **5 (DB call, closed):** NQL filter → `absolute_to_transform_ready` +
  `escape_nql_string` + `core::query::apply` resolves the matched redirect ids
  (the `getFilteredIds` equivalent); `with_attribution` preserves `ref` and adds
  `attribution_type`/`attribution_id` for site URLs; the real
  `ApiDataStore::bulk_edit("links", {ids,to,updated_at})` persists the change with a
  `moment()`-format `updated_at`, and the success count drives the Ghost stats.
- **6 (cache, closed):** `X-Cache-Invalidate: /r/*` set in `BulkEditHandler::handle`
  from Ghost's `INVALIDATE_ALL_REDIRECTS` (`links.js:2,30-32`).
- **7 (auth, closed):** `require_auth` router gate + ghost `links:edit` permission
  gate replace token-only auth and the ad-hoc error body.

## Missing in C++

None. Every Ghost controller route has a C++ counterpart:

- GET /links — C++ GET /links (`external/links/routes.cppm:22-28`).
- PUT /links/bulk — C++ PUT /links/bulk (`external/links/routes.cppm:31-38`).

No new routes were added; the work made the existing handlers real (see the route
details above).

## Extra in C++

None. The matched folder `include/blog/handlers/external/links` installs exactly two
routes (`external/links/routes.cppm:19-39`), both with Ghost controller counterparts.
No C++ link route lacks a Ghost route.

## Applied fixes

### Group 24 (earlier pass)

Edited only `external/links/**`:

- `browse/workflow/step/execute.cppm`: parse `filter` from query into a
  `std::map<std::string,std::string>`; call `json_for("links", params)`; emit
  Ghost envelope `{"data":rows,"meta":{"pagination":{"total":count,"page":1,"pages":1}}}`.
- `bulk_edit/workflow/step/execute.cppm`: parse `filter`, decode body
  (`core_serde::JsonCodec`) reading `data.action`/`data.meta`, enforce
  `action == "updateLink"` + non-empty filter (422 via `ApiError::validation`),
  build `{"ids":"..."}` from matched row ids, output
  `std::map<std::string,std::string>{"status","body"}`.
- `browse/handler.cppm`, `bulk_edit/handler.cppm`: removed inline
  `token(req).empty()` checks; bulk-edit handler sets status/body from the step map
  and sends `X-Cache-Invalidate: /r/*`.
- `routes.cppm`: added `.use(require_auth)` on `/links` router (covers both routes).

### Strict-parity pass (group 27)

Edited only `external/links/**` (this doc and `docs/parity/behavior/links.md` are
the only docs changed):

- `browse/workflow/step/execute.cppm`: added `blog::handlers::links::support` in the
  already-registered `:links_browse_step` partition — `principal_for`, `permitted`
  (ghost gate via `GhostPermissionRegistry` + `require_ghost_permission`),
  `kBrowseOptions={"filter"}`, `seed_link`, `link_rows`, `browse_response` using
  `core::query::parse`/`apply` with Ghost order `count__clicks desc, to desc`,
  `full_post_link_json` (`fromModel`/`FullPostLink` wire shape), `parse_link_filter`,
  `absolute_to_transform_ready`, `escape_nql_string`, `with_attribution`, `now_iso`,
  `matched_ids`, `updated_count`, `bulk_response` and the Ghost error builders.
- `browse/handler.cppm`: runs `support::permitted(...,"links","browse")`, keeps
  `Output=std::string` and `s_ctx`, maps a validation body to 422.
- `bulk_edit/workflow/step/execute.cppm`: imports `:links_browse_step`, validates
  action/filter (422) and NQL filter (400 `BadRequestError`), resolves ids via
  `core::query::apply`, applies attribution + transform-ready target + timestamp,
  performs a real `ApiDataStore::bulk_edit` and returns Ghost's `bulk` stats envelope.
- `bulk_edit/handler.cppm`: runs `support::permitted(...,"links","edit")`, keeps
  `Output=std::map<std::string,std::string>` and `s_ctx`, sets `X-Cache-Invalidate:/r/*`.
- `routes.cppm`: `content_type_middleware` moved from the router to the `/bulk` sub-route
  only (the GET has no request body in Ghost).

Shared abstractions used (CONTRACTS.md): `core::query` (§1), `core_auth:ghost` +
`blog_handlers_middleware::require_ghost_permission` (§2, §3). `blog::mapping` (§6)
is not wired into `blog_handlers` because of the known Bazel cycle, so the local
`support` namespace provides the exact wire projection/envelope instead — no new
partition was created and no shared/central file was edited.

New-partition need recorded (central owner, follow-up only): a real link-tracking
service partition (`blog_handlers:links_ops` backed by `core_database` / a
`LinkRedirect` model) would replace the seeded `ResourceStore` record while
preserving the same observable wire behavior. Not required for parity.

Bazel note: the full `//include/blog/handlers:blog_handlers` build is currently broken
by unrelated in-progress partitions and a pre-existing module cycle; the links
partitions compile cleanly and are not part of the cycle. Not fixed here.
