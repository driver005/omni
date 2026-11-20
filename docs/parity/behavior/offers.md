# Behavior report: offers (Admin API)

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/offers.js:1-88`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:164-167`
- C++ folders: `include/blog/handlers/membership/offers/` (`routes.cppm` + `browse/`,
  `read_by_id/`, `create/`, `edit/` handler + flow + step sets); installer
  `include/blog/handlers/routes.cppm:39,111`, exports `base.cppm:35,110-112`,
  `all.cppm:29,104-106,399,475-477,680,755-757`
- Analyzed: 2026-11-19; strict-parity rewrite: 2026-11-20 (Group 45)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Verdict rubric (same as sibling reports): `MATCH` = primary observable contract
(verb/path, input parsing, status codes, headers, output envelope/shape, permission
gate and side effects at the allowed primitive level) agrees. Residual gaps with no
allowed C++ primitive (real Knex/DDD services, full domain validation, redirect
manager) are documented per route and do not affect the primary observable contract.

Group 45 strict-parity rewrite applied 2026-11-20 using CONTRACTS.md shared
abstractions: `core_query` (filter/NQL/restricted guard), `core_auth:ghost` via
`blog_handlers_middleware::require_ghost_permission`, context `store()` and
`blog_handlers:outbox_shim` (offer events). Ghost DTO allow-list/envelope helpers
live in the already-registered `browse/workflow/step/execute.cppm` partition; the
unwired `blog_handlers:mapping` module is recorded as a new-partition need and
was deliberately not imported.
No shared file was edited and the Bazel cycle was left untouched.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/offers` | `browse`, docName `offers` | `options:['filter']`; no `data[]`, no body/multipart | `mw.authAdminApi` + `permissions:true` → `canThis.browse.offer` | 200; no `X-Cache-Invalidate` (`cacheInvalidate:false`) | `listOffers` → `getAll` → `mapToOffer` → `OfferMapper.toDTO` | no controller cache; header suppressed | `{"offers":[OfferDTO…]}`, no meta | `membership/offers/browse/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200/400; `core::query::parse` (allowlist `filter`) + `apply` with restricted-field guard; `clean`+`wrap` → `{"offers":[...]}`; `require_ghost_permission(offers,browse)`; no cache header | MATCH | Ghost `offers.js:13-25`, `output/default.js:22-41`; C++ `browse/.../execute.cppm:101-130`, `browse/handler.cppm:17-35` |
| POST `/offers` | `add`, docName `offers` | body `data.offers[0]`; no declared options/validation | `mw.authAdminApi` + `permissions:true` | **200**; `X-Cache-Invalidate: /*`; `Location` from first resource id | `createOffer` → `Offer.create` → `OfferModel.add` → `OfferCreatedEvent`; retention archive | no controller cache; redirect via event | `{"offers":[OfferDTO]}` + Location | `membership/offers/create/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200; unwrap `data.offers[0]`; `store().create("offers", …)`; `offer_dto` computes currency/cadence/redemption fields; `offer.created` outbox event; `{"offers":[DTO]}`, `X-Cache-Invalidate: /*`, `Location`; permission gate | MATCH | Ghost `offers.js:74-85`, `headers.ts:135-185`; C++ `create/.../execute.cppm:13-40`, `create/handler.cppm:17-36` |
| GET `/offers/:id` | `read`, docName `offers` | `data:['id']` from path param | `mw.authAdminApi` + `permissions:true` | 200 or **404** `Offer not found.`; no `X-Cache-Invalidate` | `getOffer` → `getById` → `findOne` → `mapToOffer` → `OfferMapper.toDTO`; null → 404 | none | `{"offers":[OfferDTO]}`; missing → 404 envelope | `membership/offers/read_by_id/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200/404; real path `:id`; id-filtered `core::query::apply` against store rows (reachable 404); `offer_dto` + `wrap`; permission gate; no cache header | MATCH | Ghost `offers.js:27-45`, `offers-api.js:61-77`; C++ `read_by_id/.../execute.cppm:12-34`, `read_by_id/handler.cppm:17-34` |
| PUT `/offers/:id` | `edit`, docName `offers` | `options:['id']`; body `data.offers[0]`, path id merged over body id | `mw.authAdminApi` + `permissions:true` | 200; `X-Cache-Invalidate: /*`; missing → 404 | `updateOffer` → `getById` → `updateName/updateCode` → `save` → `OfferCodeChangeEvent`; retention archive | no controller cache; redirect rewritten on code change | `{"offers":[OfferDTO]}`; missing → 404 | `membership/offers/edit/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200/404/422; path id parsed + merged into payload; 422 on empty id, 404 when id not found; `offer_dto` + `wrap`; `offer.code_changed` outbox event; `X-Cache-Invalidate: /*`; permission gate | MATCH | Ghost `offers.js:47-72`, `offers-api.js:113-157`; C++ `edit/.../execute.cppm:13-63`, `edit/handler.cppm:17-35` |

## Route details

### GET /offers — MATCH

**Ghost**
1. `GET /offers` (`admin/routes.js:164`); `docName:'offers'`, op `browse` (`offers.js:11,13`).
2. Input: `options:['filter']` (`offers.js:17`); no `data[]`, no `validation{}`, no body.
3. Auth: `mw.authAdminApi` + `permissions:true` → `canThis.browse.offer` (`offers.js:18`).
4. Status/headers: default 200; `cacheInvalidate:false` → no `X-Cache-Invalidate`
   (`offers.js:14-16`, `headers.ts:135-144`).
5. Call chain: `listOffers(frame.options)` → `getAll({filter,transacting})` →
   `OfferModel.findAll` + `mapToOffer` → `OfferMapper.toDTO` (`offers-api.js:164-172`).
6. Cache: none; header suppressed.
7. Output: controller `{data: offers}` → default serializer → `{"offers":[...]}` with
   no `meta.pagination` (`output/default.js:22-41`); empty → `{"offers":[]}`.

**C++**
1. Route installed by `OffersRoutes::install` (`routes.cppm:19-28`), middleware
   `content_type_middleware` → `require_auth`; `BrowseHandler::handle`
   (`browse/handler.cppm:17-35`) calls `support::authorize(...,"browse")` — Ghost
   permission gate via `core_auth:ghost`.
2. Step `offers_browse_execute` (`browse/.../execute.cppm:101-130`): `core::query::parse`
   allowlists only `filter`, `core::query::apply` filters/sorts the cleaned store rows;
   restricted fields raise 400 `BadRequestError`.
3. `support::clean` + `support::wrap` use `blog_handlers:mapping` → `{"offers":[...]}`,
   no pagination meta; status 200; no cache header written.

**Diff** — residual gaps (no C++ primitive): real Knex `findAll`/`mongoTransformer`,
redemption-count/last-redeemed relations, redirect-manager middleware, full
`OfferMapper` computed aggregation, full Ghost permission model. No divergences in
primary observable contract.

### POST /offers — MATCH

**Ghost**
1. `POST /offers` (`admin/routes.js:165`), op `add` (`offers.js:74`).
2. Input: `frame.data.offers[0]` + `frame.options` (`offers.js:80`).
3. Auth: `mw.authAdminApi` + `permissions:true` (`offers.js:75`).
4. Status/headers: no `statusCode` → **200**; `cacheInvalidate:true` →
   `X-Cache-Invalidate: /*`; `Location` generated because method `add` and result has
   `offers[0].id` (`headers.ts:135-185`).
5. Call chain: `createOffer` → `Offer.create` (uniqueness/cadence/duration/tier rules)
   → `OfferModel.add` → `OfferCreatedEvent` → redirect
   (`offers-module.js:36-42`); retention+active → archive others.
6. Output: `{data:[offer]}` → default serializer → `{"offers":[OfferDTO]}` + Location.

**C++**
1. Route `routes.cppm:30-37`; `CreateHandler` (`create/handler.cppm:17-36`) gates with
   `support::authorize(...,"add")`, writes `X-Cache-Invalidate: /*` and `Location`.
2. Step `offers_create_execute` (`create/.../execute.cppm:13-40`): `support::unwrap`
   takes `data.offers[0]`; `ctx.store().create("offers", inner)` persists in the
   context-local ResourceStore; `support::offer_dto` applies the OfferMapper computed
   fields, `support::wrap` emits `{"offers":[DTO]}`.
3. `support::emit("offer.created", …)` records the redirect side effect in the
   context outbox; status 200 (Ghost default), not 201. Location from the created id.

**Diff** — residual gaps: full `Offer.create` domain validation (unique name/code,
duration/cadence/tier/redemption rules), real Knex transaction + DDD event dispatch,
real dynamic redirect manager, ObjectID-shaped store ids. No divergences in primary
observable contract.

### GET /offers/:id — MATCH

**Ghost**
1. `GET /offers/:id` (`admin/routes.js:166`), op `read` (`offers.js:27`), `data:['id']`.
2. Auth: `mw.authAdminApi` + `permissions:true` (`offers.js:32`).
3. Status/headers: 200 or 404 `Offer not found.` (`offers.js:35-39`); no cache header.
4. Call chain: `getOffer` → `getById(id,{transacting,withRelated:['product']})` →
   `mapToOffer` → `OfferMapper.toDTO`; null → `NotFoundError`
   (`offers-api.js:61-77`, `offer-bookshelf-repository.js:164-178`).
5. Output: `{"offers":[OfferDTO]}`; missing → 404 error envelope.

**C++**
1. Route `routes.cppm:38-47`; `ReadByIdHandler` (`read_by_id/handler.cppm:17-34`) gates
   with `support::authorize(...,"read")`, no cache header, maps code 200/404.
2. Step (`read_by_id/.../execute.cppm:12-34`): real path `:id` via `last_segment`;
   `core::query::apply` with an `id` filter over cleaned store rows — the store's
   `offers` branch is id-agnostic, so the id is applied here and a miss now reaches
   the 404 `ApiError{}.json("Offer not found.","Not Found")` branch.
3. Hit → `offer_dto` + `wrap` → `{"offers":[OfferDTO]}`.

**Diff** — residual gaps: real `findOne`/`withRelated:['product']`/redemption stats,
full `OfferMapper.toDTO`, content-API chain for the shared public route (tracked in
`offers-public.md`). No divergences in primary observable contract.

### PUT /offers/:id — MATCH

**Ghost**
1. `PUT /offers/:id` (`admin/routes.js:167`), op `edit` (`offers.js:47`).
2. Input: `options:['id']` required; `frame.data.offers[0]` with path id merged over
   body id (`offers.js:54-60`).
3. Auth: `mw.authAdminApi` + `permissions:true` (`offers.js:49`).
4. Status/headers: 200; `cacheInvalidate:true` → `X-Cache-Invalidate: /*`; missing →
   404 `Offer not found.` (`offers.js:62-66`).
5. Call chain: `updateOffer` → `getById` → `updateName/updateCode` (uniqueness,
   code-change-once) → `save` → `OfferCodeChangeEvent` → redirect rewrite; retention archive.
6. Output: `{"offers":[OfferDTO]}`; missing → 404.

**C++**
1. Route `routes.cppm:48-55`; `EditHandler` (`edit/handler.cppm:17-35`) gates with
   `support::authorize(...,"edit")`, writes `X-Cache-Invalidate: /*`, maps 200/404/422.
2. Step (`edit/.../execute.cppm:13-63`): path id parsed (`last_segment`), empty → 422
   `validation("Validation error","id is required","id")`; `support::unwrap` +
   `with_id` merge the path id into `data.offers[0]`.
3. Existence check via `core::query::apply` id filter → reachable 404 `Offer not found.`;
   `store().edit("offers", id, inner)` persists; `offer_dto` + `wrap` emit
   `{"offers":[OfferDTO]}`; `offer.code_changed` outbox event on code change.

**Diff** — residual gaps: `getById`/uniqueness/code-change-once rules,
retention-archive service, real redirect manager, full domain validation, ObjectID
ids. No divergences in primary observable contract.

## Module-level notes

- All four routes share `mw.authAdminApi` + `permissions:true` in Ghost; C++ now
  performs a permission gate in every handler via `require_ghost_permission`
  (`core_auth:ghost`) with a default ADMIN_API/Administrator bearer passthrough,
  matching the invites/tags precedent. No `unsafeAttrs` on any offers route.
- Headers: browse/read omit `X-Cache-Invalidate` (Ghost `cacheInvalidate:false`);
  create/edit emit `X-Cache-Invalidate: /*` (Ghost `cacheInvalidate:true`); `add`
  also emits `Location: <request path>/<id>/` (`headers.ts:159-184`).
- Status codes: all four success paths are 200 — `offers.js` declares no `statusCode`,
  so `http.ts:126-133` defaults to 200. The store's generic create 201 is remapped
  in the create step.
- Envelope: every route emits `{"offers":[...]}`; there is no `meta` on offers
  responses (`output/default.js:22-41` has no `offers` mapper and the controller
  supplies no `meta`). The previous `{"data":[...]}` bodies were the controller-stage
  value, not the serialized HTTP response, and were corrected in Group 45.
- Shared abstractions used: `core_query` (browse filter/restricted guard, id filters
  for read/edit), `core_auth:ghost` + `blog_handlers_middleware` (permission gates),
  `blog_handlers:outbox_shim` (`offer.created`, `offer.code_changed` events), and the
  context `store()` (context-local ResourceStore). Ghost DTO allow-list/envelope
  helpers are local to the registered `offers_browse_step` partition; `blog_handlers:mapping`
  is not registered in `blog_handlers` BUILD, so it was not imported.
- Helpers live in the existing registered `browse/workflow/step/execute.cppm`
  partition as `blog::handlers::offers::support`; the other steps import
  `:offers_browse_step`. No new partition was created. Optional future cleanup: move
  these helpers to a dedicated `offers/types/dto_offer.cppm` partition (would require
  main agent to add it to `blog_handlers` BUILD — recorded, not done here).
- No `destroy` route binds `api.offers` in Ghost (`admin/routes.js:164-167`) or C++
  (`routes.cppm:19-56`); offer deletion is out of scope.
- Verification note: at the time of this rewrite the workspace Bazel build was
  blocked by unrelated concurrent-agent errors and an existing `blog_handlers`
  dependency cycle; no offers file appeared in the failure set.
