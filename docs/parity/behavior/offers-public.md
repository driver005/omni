# Behavior report: offers-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/offers-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
  (router `express.Router('content api')` at `:12`, installed as the Content API).
  The same relative path in `admin/routes.js:166` binds `api.offers.read`, a different
  controller (`offers.js`), covered by `offers.md`.
- C++ folders: `include/blog/handlers/membership/offers/` — `routes.cppm`,
  `read_by_id/handler.cppm`, `read_by_id/workflow/flow/main.cppm`,
  `read_by_id/workflow/step/execute.cppm`. Counterpart chosen by exact path+verb
  `GET /offers/:id`; one C++ handler serves both the public (`route_catalog.cppm:41`,
  `admin=false`) and admin (`:132`, `admin=true`) catalog intent.
- Analyzed: 2026-11-19; strict-parity rewrite: 2026-11-20 (Group 45)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/offers/:id` | `read`, docName `offers` | `data=['id']` from path param `:id`; no `options[]`/`validation{}`/body | `permissions:true` public pass-through; router `authenticatePublic` (rate limit, content-api auth, cors, redirects, prettyUrls) | 200; no `X-Cache-Invalidate`; 404 `NotFoundError` "Offer not found." | `getOffer` → `getById(id,{transacting,withRelated:['product']})` → `findOne` → `mapToOffer` → `OfferMapper.toDTO`; null → 404 | no cache service/key/TTL; `cacheInvalidate:false` omits header; CORS maxAge from config | `{"offers":[OfferDTO]}` via the shared output serializer (`output/default.js:22-41`); no meta; missing → 404 | `membership/offers/routes.cppm`, `read_by_id/{handler.cppm, workflow/flow/main.cppm, workflow/step/execute.cppm}` | 200/404; real `:id` parsed; id-filtered `core::query::apply` over cleaned store rows makes 404 reachable; `offer_dto` + `wrap` → `{"offers":[OfferDTO]}`; Ghost permission gate via `require_ghost_permission`; no cache header | MATCH | Ghost `offers-public.js:11-31`, `content/routes.js:12,14,42`, `content/middleware.js:18-25`, `output/default.js:22-41`, `headers.ts:135-144`; C++ `routes.cppm:19-21,38-47`, `read_by_id/handler.cppm:17-34`, `read_by_id/execute.cppm:12-34` |

## Route details

### GET /offers/:id — MATCH

**Ghost** (numbered facts 1–8):
1. `router.get('/offers/:id', mw.authenticatePublic, http(api.offersPublic.read))`
   (`content/routes.js:42`); router-level CORS `maxAge` from config (`:14`).
2. Controller key `read` (`offers-public.js:13`), `docName:'offers'` (`:11`), `query` at `:19-30`.
3. Input `data:['id']` from `req.params.id`; no options/validation/body/header reads.
4. Auth: `permissions:true` public pass-through (`:18`); content-API key auth,
   authorization, rate limiting, redirects, prettyUrls from `mw.authenticatePublic`
   (`content/middleware.js:18-25`). No `unsafeAttrs`/labs flags.
5. Status/headers: no `statusCode` → 200; `cacheInvalidate:false` (`:14-16`) omits
   `X-Cache-Invalidate` (`headers.ts:135-144`); no Location/Disposition.
6. Call chain: `getOffer(frame.data)` → transaction → `getById` → `findOne` with
   `withRelated:['product']` → `mapToOffer` → `OfferMapper.toDTO`; null → `NotFoundError`
   "Offer not found." (`:21-25`). No events/jobs/mail/webhooks on this read path.
7. Cache: none in controller/call path; only the suppressed header.
8. Output: controller `{data:[offer]}` (`:27-29`) → shared default serializer →
   `{"offers":[OfferDTO]}` (`output/default.js:22-41`); DTO field set from
   `offer-mapper.ts:5-24,49-71`; missing → 404 error envelope, not an empty array.

**C++**:
1. `OffersRoutes::install` (`routes.cppm:19-21,38-47`) → `offers_read_by_id().handle` →
   `ReadByIdHandler` → workflow `offers_read_by_id` → `offers_read_by_id_execute` step.
2. `Input = IRequest&`, `Output = std::map<std::string,std::string>`; step parses `:id`
   as the last path segment with query stripped (`execute.cppm:12-20`).
3. `core::query::apply` with an `id` filter over `support::clean(json_for("offers",{}))`
   (`execute.cppm:21-28`); the step-local filter supplies the id-awareness the shared
   store's `offers` branch does not have. Miss → 404 `ApiError{}.json("Offer not found.",
   "Not Found")`; hit → `support::offer_dto` + `support::wrap` → `{"offers":[DTO]}`.
4. `ReadByIdHandler::handle` (`handler.cppm:17-34`) calls
   `support::authorize(...,"read")` (Ghost permission gate via `core_auth:ghost`),
   maps code 200/404/422, emits no cache header, sends the body.
5. `offer_dto` normalizes the wire fixture to Ghost's OfferDTO enums and drops
   `updated_at`; currency/currency_restriction/duration_in_months use OfferMapper rules.

**Diff**:
- Residual gaps (no C++ primitive): real Bookshelf/Knex `findOne`, `withRelated:['product']`,
  redemption aggregation, full `OfferMapper.toDTO`, content-API key auth/authorization/
  rate limiting/redirects/prettyUrls/CORS, and the public-vs-admin route catalog split
  (one C++ route serves both). None affect the primary observable contract.
- No divergences remain in verb/path, input id, status (200/404), headers, envelope or
  DTO field shape.

## Module-level notes

- `offers-public.js` exposes one route. The C++ `OffersRoutes` installs four routes;
  the other three correspond to Ghost's admin `offers.js` controller and are out of
  scope for this module's verdict (not `EXTRA` for the public controller).
- The earlier Group 38/implementation-doc claim that the serialized public body is
  `{data:[offer]}` was the controller-stage value; the content API runs the same shared
  output serializers as the admin API (`pipeline.ts:168-190`, `output/default.js:22-41`),
  so the HTTP body is `{"offers":[...]}`, as corrected in Group 45.
- Service init (`services/offers/service.js:11-27`) wires the redirect manager and
  repository; the redirect middleware getter (`:31-33`) is unrelated to this read path.
- Group 45 changes are confined to `include/blog/handlers/membership/offers/`; no
  shared file was edited and the existing Bazel cycle was left untouched.
