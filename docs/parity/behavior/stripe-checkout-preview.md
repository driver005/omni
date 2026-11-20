# Behavior report: stripe-checkout-preview

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-preview.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (route at :155)
- C++ folders: `include/blog/handlers/external/stripe/` + `include/blog/handlers/external/stripe/stripe_checkout_preview_post/`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Single Ghost route (`POST /stripe/checkout/preview`) has a current C++ handler by exact
path+verb. Route identity, request-envelope unwrapping, basic `tier_id`/`cadence` validation,
store-backed tier-existence gate, success 200 / validation 422, and the output envelope align,
but the core side effects (real Stripe Checkout Session creation, Stripe-connected check,
paid+active tier check, design/collection validation, labs collection flag, products:edit
permission) are absent — hence PARTIAL. Note: `docs/parity/stripe-checkout-preview.md`
describes an older C++ state (raw `StripeHandler::preview` object, inline token check,
`EXTRA` count 3); this report reflects the **current** source read on 2026-11-19.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST `/stripe/checkout/preview` (admin) | `stripeCheckoutPreview.add`, docName `checkout_preview` | Ghost: `data.checkout_preview[0]` with `tier_id` (24-hex), `cadence` (month\|year, default month), `design` (required), optional `shipping`/`phone`; Zod strict + `refuseCollectionWhileOff`. C++: `data[0]` unwrapped via `JsonBody::parse`, only `tier_id`/`cadence` read | Ghost: `mw.authAdminApi` + `permissions{products,edit}`. C++: router `content_type_middleware` + `require_auth` (token presence only) | Ghost: 200 default, `cacheInvalidate:false` (no `X-Cache-Invalidate`), admin `Cache-Control: private`. C++: 200 (success), 422 (validation), 401 (no token), literal `X-Cache-Invalidate: false` header | Ghost: `previews.add` → `CheckoutPreviewService.add` → `parseRequest(CheckoutPreviewInput)` → `stripeConnected()` → `readTier` → `checkouts.preview` → `config.readForCheckout` → `createPreviewLink` → `members.paymentsService.getPreviewLink` → `stripeAPIService.createPreviewCheckoutSession` → Stripe `checkout.sessions.create` (+ throttle, fallback/retry, refusal→ValidationError). C++: workflow `ExecuteStep`: tier existence via `ctx.store().json_for("tiers",…)`, then fabricates a Stripe URL | Ghost: nothing cached; `cacheInvalidate:false` suppresses invalidate header. C++: none; sets literal header `X-Cache-Invalidate: false` | Ghost serializer → `{"checkout_preview":[{"url":<stripe URL>}]}`. C++ raw body `{"checkout_preview":[{"url":"https://checkout.stripe.com/c/pay/preview_<tier>_<cadence>"}]}` (fabricated URL) | `external/stripe/routes.cppm`, `external/stripe/stripe_checkout_preview_post/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `middleware/{auth,bearer,serde}.cppm`, `context/context_context.cppm`, `stripe.cppm` | Token-gated local-store tier check + fabricated URL; no Stripe API/network, no permissions, no paid/active check, no design/collection validation, no Stripe-connected error | PARTIAL | Ghost: routes.js:155; controller :16-25; serializer :4-11 & serializers.ts:95-105,187-197; preview-service.ts:9,49-82; checkout-service.ts:87-95; service.ts:325-346; index.ts:76-84; payments-service.js:160-171; stripe-api.js:740-764,193-238,88-93; http.ts:126-136; app.js:22, admin/app.js:24-44. C++: routes.cppm:19-64; handler.cppm:12-26; flow/main.cppm:7-10; execute.cppm:11-64; auth.cppm:12-29; bearer.cppm:27-33; serde.cppm:90-126; context_context.cppm:79-124,322-336,1152-1163 |

## Route details

### POST `/stripe/checkout/preview` — PARTIAL

**Ghost** (numbered per checklist 1–8):

1. **Identity**: `router.post('/stripe/checkout/preview', mw.authAdminApi, http(api.stripeCheckoutPreview.add))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:155`. Route middleware chain: `mw.authAdminApi`, then `http(...)` wrapper (`packages/api-framework/src/http.ts:51-63`). Router-level mounts: parent API app mounts `/admin/` → admin app (`web/api/app.js:22`); backend app mounts `BASE_API_PATH` = `/ghost/api` (`web/parent/backend.js:15`, `core/shared/url-utils.ts:4`); admin app applies `shared.middleware.cacheControl('private')`, `bodyParser.json({limit:'50mb'})`, `bodyParser.urlencoded`, `parseQueryBooleans`, `checkVersionMatch`, then `routes()` (`web/api/endpoints/admin/app.js:24,33-44`). Full prefixed path `/ghost/api/admin/stripe/checkout/preview`. No other route binds this controller (single hit at routes.js:155; `api/endpoints/index.js:135-137` registers `apiFramework.pipeline(require('./stripe-checkout-preview'), localUtils)`).
2. **Operation**: exported key `add`, op `add` (custom `query`, no browse/read/edit/destroy), `docName: 'checkout_preview'` (`stripe-checkout-preview.ts:15-16,18-25`). Output serializer mapped by docName: `utils/serializers/output/index.js:103-104` → `require('./stripe-checkout-preview')`.
3. **Input**: no `options[]` declared; `frame.data.checkout_preview` typed as `unknown[]` (`stripe-checkout-preview.ts:8`) and first element destructured at `:22` — body shape `data.checkout_preview[0]`. No path params, query aliases, `filter[x]=y`, or multipart fields. No headers read by the controller itself. Zod validation lives in the service: `CheckoutPreviewInput` (`services/stripe-checkout-config/serializers.ts:95-105`) is a `strictObject` requiring `tier_id` (string matching `^[0-9a-f]{24}$`, message "Choose a tier to preview checkout for."), `cadence` (`z.enum(['month','year']).default('month')`), `design` (required discriminated union on `customize`: `false` → no design; `true` → all `SnakeDesign` fields required, `serializers.ts:42-56`; design fields = `button_color`/`background_color`/`border_style`/`font_family`, `models.ts:23-30`), optional `shipping`/`phone` collected-part settings (`serializers.ts:59-79`, `CHECKOUT_PART_SETTINGS` from `@tryghost/checkout`), then `.transform(camelKeys)`. `parseRequest` converts the first Zod issue into `ValidationError` with `property` = issue path or `resource: 'checkout_preview'` (`core/server/lib/parse-request.ts:8-22`). Controller additionally calls `refuseCollectionWhileOff(input, labs.isSet(COLLECTION_FLAG))` (`controller:23`; helper `serializers.ts:22-34` throws ValidationError when a `shipping`/`phone` section name is present while the `stripeCheckoutCollection` flag (`models.ts:111`) is off, e.g. "Shipping collection is not available yet.").
4. **Auth/permissions**: `mw.authAdminApi` on route (routes.js:155); `permissions: { docName: 'products', method: 'edit' }` on `add` (`controller:20`) — the API framework merges this over `docName:'checkout_preview'` (`pipeline.ts:235-248`) and runs the permission service (`apiUtils.permissions.handle`) after input validation/serialisation (`:430-434`). No `unsafeAttrs`. Labs flag `stripeCheckoutCollection` gates collected parts via `refuseCollectionWhileOff` (`:23`). `query()` runs no permission service itself.
5. **Status + headers**: no `statusCode` in controller → HTTP 200 on success (`http.ts:126-134`). `headers: { cacheInvalidate: false }` (`controller:19`) → headers.get skips `X-Cache-Invalidate` (`headers.ts:135-144`). No `disposition`/`Location` config; `Location` not emitted because the resource `{url}` has no `id` (`headers.ts:146-185`). Admin app adds `Cache-Control: private` (`admin/app.js:24`). Response is JSON (`http.ts:139-168`). Validation errors surface as 422 error envelope; not-connected/paid-tier 422; Stripe refusal 422; no URL from Stripe → 500.
6. **Full call chain**: `query(frame)` → `stripeCheckoutConfig.service.previews.add(input)` (`controller:24`) — service built at `services/stripe-checkout-config/index.ts:76-84` with `stripeConnected = () => require('../stripe').api.configured`, `readTier = id => tiersRoot.default.service.api.read(id)`, `createPreviewLink = params => require('../members').api.paymentsService.getPreviewLink(params)`, `siteUrl = () => urlUtils.urlFor('home', true)`. `CheckoutPreviewService.add` (`preview-service.ts:49-82`):
   - `parseRequest(CheckoutPreviewInput, input, 'checkout_preview')` destructures `{tierId, cadence, ...settings}` (`:50-54`);
   - `if (!stripeConnected()) throw ValidationError('Connect Stripe to preview checkout.')` (`:56-58`);
   - `tier = await readTier(tierId)`; if `!tier || tier.type !== 'paid' || tier.status !== 'active'` → `ValidationError` property `tier_id` "Choose an active paid tier to preview checkout for." (`:60-66`);
   - `checkout = await checkouts.preview(tierId, settings)` (`:68`) → `CheckoutService.preview` (`checkout-service.ts:87-95`): if collection flag on, `config.readForCheckout(unsaved)` (`service.ts:325-346`, transaction-wrapped `readSaved` + `assertTargets` + merge unsaved parts), else `NOTHING_COLLECTED`; then `checkoutForTier({...collection, design}, tierId)` (`checkout-service.ts:30-48`) picks asked parts for the tier.
   - `createPreviewLink({tier, cadence, checkout, returnUrl: siteUrl(), expiresInSeconds: 31*60 = 1860})` (`:69-75`, `PREVIEW_LIFETIME_SECONDS` at `:9`); from `members/payments-service.js:160-171` → `getPriceForTierCadence` + `stripeAPIService.createPreviewCheckoutSession(price.id, {successUrl:returnUrl, cancelUrl:returnUrl, trialDays:tier.trialDays, expiresInSeconds, checkout})`.
   - **Stripe side effects** in `services/stripe/stripe-api.js`: `createPreviewCheckoutSession` (`:740-764`) calls `_rateLimitBucket.throttle()` (`:741`), builds `_subscriptionSessionOptions(...)` with `metadata:{ghost_checkout_preview:true}` and `expires_at: floor(Date.now()/1000)+expiresInSeconds` (`:742-750`), calls `_createWithFallback(...)` (`:752-757`) → `_stripe.checkout.sessions.create(sessionCreateParams(...))` (`:220-223`) with refusal-driven retries (throttle again at `:230`, recursive `:231-236`); if Stripe refuses a preview setting, `reportRefusal` throws `ValidationError` (`:88-93`, used at `:756,759-761`) — previews never downgrade refused settings (unlike buyer checkouts). On no URL, `throw InternalServerError('Stripe returned no checkout page to preview.')` (`preview-service.ts:76-80`); success returns `{url: session.url}` (`:81`).
7. **Cache**: no cache service call, no TTL/tag/namespace anywhere in this path. `headers.cacheInvalidate:false` (`controller:19`) avoids the invalidate response header. No `X-Cache-Invalidate`, no cache marker, no `core::cache::Cache` equivalent.
8. **Output**: serializer `utils/serializers/output/stripe-checkout-preview.ts:4-11` sets `frame.response = toCheckoutPreviewResponse.parse(preview)`. `toCheckoutPreviewResponse` (`services/stripe-checkout-config/serializers.ts:187-197`) transforms `{url}` into envelope `{"checkout_preview":[{"url":<url>}]}` with `url` validated as `z.url()`. No pagination/meta. Empty/missing results: the service throws (422/500) instead of returning an empty envelope, so the pipeline returns an error envelope rather than `checkout_preview`.

**C++** (current source):

1. **Route identity**: `StripeRoutes::install` builds nested `Router{router, "/stripe"}` (`external/stripe/routes.cppm:19`) with `.use(content_type_middleware)` (`:20`) and `.use(require_auth)` (`:21`), then `Router{router, "/checkout"}` (`:23`) and `Route{"/preview"}.post(...)` (`:54-56`) dispatching `handler.stripe_stripe_checkout_preview_post().handle(...)` (`:61`), summary "Ghost preview POST" (`:63`). Installed by `ApiRoutes::install` via `StripeRoutes{}.install(...)` (`include/blog/handlers/routes.cppm:54,126`); test server installs at root with no `/ghost/api/admin` prefix (`blog/testing/cpp_api_server.cc:127`). Normalized path+verb matches Ghost routes.js:155.
2. **Operation/flow**: `handle()` runs workflow `"stripe_stripe_checkout_preview_post"` (`handler.cppm:17-18`), reads result of step `"stripe_stripe_checkout_preview_post_execute"` (`:19`); `MainFlow` is a single-step `ExecuteStep` (`workflow/flow/main.cppm:7-10`; step name `execute.cppm:12`). Accessor/wiring: `all.cppm:98,469,749`, `base.cppm:104`. Maps conceptually to Ghost `add`.
3. **Input**: `using Input = interfaces::io::IRequest&` (`execute.cppm:11`); `invoke(const Input&, WorkflowContext&)` (`:13`) decodes `in.body_as_string()` (`core_serde::JsonCodec`, `:29`), takes `root["data"]` as an array and encodes `arr[0]` into `inner` (`:31-34`), then `blog::api::JsonBody{}.parse(inner)` (`:35`; parser at `context/context_context.cppm:322-336`). Reads only `tier_id` (`:36`) and `cadence` (`:37`), defaulting cadence to `"month"` when empty (`:38`). Helpers: `unquote` (`:14-17`), `is_hex24` requiring exactly 24 lowercase `[0-9a-f]` (`:18-23`), `field` object lookup (`:24-27`). No `design`, `shipping`, `phone` parsing; no labs flag gate.
4. **Auth/middleware**: router-level `content_type_middleware` (`routes.cppm:20`; enforces JSON `Content-Type` and `Accept`, else 415/406 — `middleware/serde.cppm:90-126`) and `require_auth` (`routes.cppm:21`; `middleware/auth.cppm:12-29`): empty token → 401 `UnauthorizedError`, else `next()`. `token()` reads `get_authorization()` directly, stripping `Bearer ` (`middleware/bearer.cppm:27-33`); `bearer_middleware` is not applied on this router. No permission service, no `products:edit` check.
5. **Status/headers/body write**: `handle()` derives `status` from payload `"status"` (default 200) and `body` from `"body"` (default `{}`), converts body chars to bytes, writes `res.set_header("X-Cache-Invalidate","false")` (`handler.cppm:23`), then `res.set_status(...)` + `res.set_body(...)` + `send()` (`:20-25`). Step success returns `{"status","200"}` (`execute.cppm:58`); invalid input returns `{"status","422"}` with `ApiError{}.validation("Invalid checkout preview input.","body", is_hex24(tier_id)?"cadence":"tier_id")` (`:39-43`; error envelope format at `context_context.cppm:79-124`); missing tier returns 422 `ApiError{}.validation("Choose an active paid tier to preview checkout for.","body","tier_id")` (`:51-55`).
6. **State reads / side effects** (`execute.cppm:28-58`): `tiers = ctx.store().json_for("tiers", {{"id",tier_id}})` (`:44`; `context_context.cppm:1152-1163` routes `tiers` to `query_json(m_db.all<model::Tier>(), params)` at `:1464-1472` → `query_json_impl`/`search_json` at `:1561-1636`; note a bare `id` param is not a search predicate — `search_json` only honors `filter`/`filter[x]`/pagination — so all stored tiers are returned). The step loops records and matches `id == tier_id` (`:45-50`) as the existence gate; no type/status check. Body is fabricated: `std::format(R"({"checkout_preview":[{"url":"https://checkout.stripe.com/c/pay/preview_{}_{}"}]})", tier_id, cadence)` (`:56-57`). **No Stripe API calls**: `s_stripe` (`StripeHandler*`) is assigned in the handler constructor (`handler.cppm:13`, declared `execute.cppm:61-64`) but never used in `invoke()`. No `_stripe.checkout.sessions.create`, no throttle, no rate limiting, no `ghost_checkout_preview` metadata, no `expires_at`, no fallback/refusal, no events/logging, no Stripe-connected check, no paid/active check. `compensate()` is a no-op (`:60`). No outbox records, no fixture writes.
7. **Cache**: no `core::cache::Cache` key/tag/TTL, no eviction. The only cache-related write is the literal `X-Cache-Invalidate: false` header (`handler.cppm:23`) — a stand-in that differs from Ghost (Ghost emits no such header).
8. **Output**: raw body `{"checkout_preview":[{"url":"https://checkout.stripe.com/c/pay/preview_<tier_id>_<cadence>"}]}` (`execute.cppm:56-57`) — matches Ghost's envelope shape and key casing (`serializers.ts:187-197`) but the URL is fabricated from tier id + cadence, not a real Stripe session URL. No serializer/DTO layer. `handler.cppm:20` fallback body `{}` on a missing step result has no Ghost analogue. Success/validation statuses (200/422) align with Ghost's `ValidationError` → 422 and default 200 (`http.ts:126-134`).

**Diff**:

- **Residual gaps (Ghost behaviors with no C++ primitive here)**:
  - Real Stripe side effects entirely absent: rate-limit throttle, `checkout.sessions.create` with `metadata:{ghost_checkout_preview:true}` and `expires_at = now+1860s`, refusal retry/fallback and `reportRefusal` ValidationError (`stripe-api.js:740-764,193-238,88-93`) vs `execute.cppm:28-58` with `s_stripe` unused (`:61-64`).
  - Stripe-connected gate `stripeConnected()` → 422 "Connect Stripe to preview checkout." (`preview-service.ts:56-58`, wired `index.ts:78`) has no C++ equivalent; C++ returns 200 for any tier id present in the store.
  - Paid+active tier check (`tier.type !== 'paid' || tier.status !== 'active'` → 422, `preview-service.ts:60-66`): C++ checks existence by id only (`execute.cppm:44-55`).
  - Full input validation: required `design` discriminated union, `strictObject` rejection of unknown keys, optional `shipping`/`phone` Zod transforms (`serializers.ts:42-105`), and collection-flag gate `refuseCollectionWhileOff` (`controller:23`, `serializers.ts:22-34`, flag `models.ts:111`) have no C++ primitive; C++ reads only `tier_id`/`cadence`.
  - Config merge for preview (`checkouts.preview` → `config.readForCheckout(unsaved)` → `checkoutForTier` → Stripe checkout definition, `checkout-service.ts:87-95`, `service.ts:325-346`, `checkout-service.ts:30-48`) has no C++ equivalent; no tier requirement/custom-field validation (`service.ts:527-622`).
  - Permission service `{docName:'products', method:'edit'}` (`controller:20`, merged at `pipeline.ts:235-248`) not implemented; C++ only token presence (`auth.cppm:12-29`).
  - InternalServerError on missing Stripe URL (`preview-service.ts:76-80`) and admin `Cache-Control: private` (`admin/app.js:24`) have no C++ equivalent.
  - `siteUrl()` return URL (Ghost passes `urlFor('home', true)` as Stripe success/cancel URL, `index.ts:83`, `payments-service.js:163-164`) has no C++ equivalent.
- **Divergences (C++ differs from Ghost)**:
  - Fabricated Stripe URL `https://checkout.stripe.com/c/pay/preview_<tier>_<cadence>` (`execute.cppm:56-57`) vs real `session.url` from Stripe (`payments-service.js:170`, `stripe-api.js:220-223`); C++ creates no Checkout Session, so no page actually exists.
  - Cache header: C++ writes literal `X-Cache-Invalidate: false` (`handler.cppm:23`); Ghost's `cacheInvalidate:false` (`controller:19`) suppresses the header entirely (`headers.ts:135-144`).
  - Middleware: C++ adds JSON `Content-Type`/`Accept` enforcement with 415/406 errors (`serde.cppm:90-126`) that Ghost does not apply on this route (only body parsers, `admin/app.js:33-34`).
  - `ctx.store().json_for("tiers", {{"id",tier_id}})` (`execute.cppm:44`) passes an `id` param that `search_json` ignores (`context_context.cppm:1602-1610` — only `filter`/`filter[x]`/pagination are honored), so the store returns all tiers and the step's own loop performs the id match; Ghost `readTier(id)` (`index.ts:79`) reads the one tier directly. No C++ `core::cache::Cache` usage.
  - `handler.cppm:20` fallback `{}` body on missing step result has no Ghost analogue; Ghost errors go through the error handler (`http.ts:169-176`, `web/api/app.js:25-26`).
  - `data[0]` unwrapping (`execute.cppm:31-35`) ignores the resource key, whereas Ghost reads `data.checkout_preview[0]` specifically (`controller:22`).

## Module-level notes

- This controller has exactly one route (`POST /stripe/checkout/preview`, routes.js:155); no browse/read/edit/destroy variants.
- All four Stripe Admin endpoints live under the same C++ `external/stripe` router (`external/stripe/routes.cppm:19-65`): branding GET (:25-34), config GET/PUT (:36-53), preview POST (:54-64). The other three map to `stripeCheckoutConfig` / `stripeCheckoutBranding` controllers (`routes.js:153-154,156`) and are out of scope for this module — they have real Ghost counterparts in the admin route table, so they are not EXTRA under the checklist definition (`EXTRA` = C++ route with no Ghost counterpart). `EXTRA` count is 0; `MISSING` count is 0 (the sole Ghost route has an exact path+verb C++ handler).
- `s_ctx`/`s_stripe` are static pointers set in the handler constructor (`handler.cppm:12-16`); `s_stripe` is unused by preview POST — a dormant hook for a future Stripe client. `BlogHandlers` wiring in `include/blog/handlers/all.cppm:98,469,749` and export via `base.cppm:104`.
- Shared middleware: `content_type_middleware` (`middleware/serde.cppm:90-126`), `require_auth` (`middleware/auth.cppm:12-29`), `token()`/`bearer_middleware` (`middleware/bearer.cppm:11-33`). Helpers `ApiError` (`context_context.cppm:79-124`), `JsonBody` (`:322-336`), `parse_array` (`:611-624`). No `core::cache::Cache` usage anywhere in the preview path.
- No source edits were made; only this report file was written.
- Verdict rationale (group 30): PARTIAL — permissions, full input validation, the
  Stripe-connected and paid+active tier gates, outbox-recorded session creation,
  422/500 branches, Ghost output envelope and header semantics now align; the
  returned `url` is still deterministic rather than a live Stripe session URL and
  the unsaved-config merge is the flag-off default. See implementation record below.

## Group 30 implementation record (strict parity)

- **Verdict:** stays PARTIAL — no flip. All implementable behaviors were closed, but
  `session.url` cannot be a real Stripe URL without a live adapter, and a fake URL
  is a functional divergence, so the row is not promoted to MATCH.
- **Permission:** `core::auth::ghost::GhostAuthorizer` `products:edit` in the step;
  two-arg `require_ghost_permission("products","edit")` hook in the handler.
- **Input:** `data.checkout_preview[0]` unwrapped (with `data[0]` compatibility
  fallback); 24-hex `tier_id`, `cadence∈{month,year}`, required design
  discriminated union, strict unknown-key rejection, and Ghost's
  collection-flag 422 messages for `shipping`/`phone`.
- **Side effects / gates:** `STRIPE_CONNECTED=0` → 422 "Connect Stripe to preview
  checkout."; `STRIPE_REFUSE=1` → Stripe-refusal 422; `STRIPE_NO_URL=1` → 500
  "Stripe returned no checkout page to preview."; two throttles and
  `sessions.create` (metadata `ghost_checkout_preview`, `expires_at=now+1860`,
  success/cancel from `SITE_URL`) recorded on `blog::handlers::outbox`.
- **Tier read:** `core::query::apply(store.json_for("tiers",{}), filter[id=tier_id])`
  then `type=='paid' && status=='active'` → 422 otherwise.
- **Headers/output:** removed the stand-in `X-Cache-Invalidate: false`; writes
  `Cache-Control: private`; `{"checkout_preview":[{url}]}` via local `wrap`.
- **Neutral residuals / new-dependency needs:** `core_adapters::StripeAdapter`
  is not a declared dependency of `blog_handlers`, so no live Stripe call is made;
  add `//include/core/adapters:core_adapters` for real session URLs, a labs
  primitive for `stripeCheckoutCollection`, and register `blog_handlers:mapping`
  to replace the local envelope helper. `blog_handlers:outbox_shim` is already
  registered and was used for every side effect.
- **Test follow-up (outside this group's edit scope):**
  `include/blog/testing/api_test.cc:158-160` POSTs `data:[{tier_id,cadence}]`
  with an unknown tier id and no design; strict Ghost rejects it (422) before
  any URL is produced, so the assertion needs a paid+active tier fixture and a
  complete design object.
- **Edit scope:** only `include/blog/handlers/external/stripe/**` plus the six
  parity docs; no shared file or Bazel target changed.
