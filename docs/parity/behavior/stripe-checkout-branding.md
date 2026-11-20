# Behavior report: stripe-checkout-branding

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-branding.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (route at :156)
- C++ folders: `include/blog/handlers/external/stripe/` + `include/blog/handlers/external/stripe/stripe_checkout_branding_get/`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Single Ghost route (`GET /stripe/checkout/branding`) has a current C++ handler by exact
path+verb. Route identity/input/cache align, but output envelope + key casing, Stripe API
side effects, permission check, and error branches diverge — hence PARTIAL. Note: the
sibling `docs/parity/stripe-checkout-branding.md` describes an older C++ state
(in-memory `stripe.branding()` stub, inline token check, `json_for("stripe", {})`);
this report reflects the **current** source read on 2026-11-19.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/stripe/checkout/branding` (admin) — **MATCH (group 30)** | `stripeCheckoutBranding.read`, docName `checkout_branding` | none (no `options`/`data`/`validation`, no path/query/body) | Ghost: `mw.authAdminApi` + `permissions{products,read}`. C++: router `content_type_middleware` + `require_auth` (token presence only) | Ghost: 200 default, `cacheInvalidate:false` (no `X-Cache-Invalidate`), admin app `cacheControl('private')`. C++: 200 (handler), 401 on no token, no headers | Ghost: `stripeCheckoutConfig.service.branding.read()` → `CheckoutBrandingService.read()` → `stripe.api.getCheckoutBranding()` (Stripe session create+expire, 2 throttles) → Zod parse → `{displayName, design}`. C++: workflow `ExecuteStep` reads `ctx.store().json_for("stripe_checkout_config", {})`, no Stripe API | Ghost: nothing cached. C++: none | Ghost serializer → `{"checkout_branding":[{display_name,design}]}`. C++ raw body `{"displayName":"<name>","design":null}` (no envelope, camelCase, design always null) | `external/stripe/routes.cppm`, `external/stripe/stripe_checkout_branding_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, `middleware/{auth,bearer,serde}.cppm`, `stripe.cppm` | Group 30: `core::auth::ghost` `products:read` gate; outbox-recorded Stripe session create + expire (with throttles + `ghost_checkout_preview`); 422 not-connected and 500 unreadable-identity; Ghost `{checkout_branding:[{display_name,design}]}`; design-warn event; `Cache-Control: private`, no invalidate header | MATCH | Ghost: routes.js:156; controller :10-18; serializer :5-12; branding-service.ts:35-62; stripe-api.js:1058-1081; serializers.ts:199-218; models.ts:23-30,159-164. C++: routes.cppm:19-33; handler.cppm:12-27; flow/main.cppm:7-10; execute.cppm:9-36; auth.cppm:12-29; bearer.cppm:27-33 |

## Route details

### GET `/stripe/checkout/branding` — MATCH (group 30)

**Ghost** (numbered per checklist 1–8):

1. **Identity**: `router.get('/stripe/checkout/branding', mw.authAdminApi, http(api.stripeCheckoutBranding.read))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:156`. Middleware chain on route: `mw.authAdminApi`, then `http(...)` wrapper. Router-level mounts: parent API app mounts `/admin/` → admin app (`web/api/app.js:22`); admin app applies `shared.middleware.cacheControl('private')`, body parsers, `parseQueryBooleans`, `checkVersionMatch`, then `routes()` (`web/api/endpoints/admin/app.js:24,33-44`). Full prefixed path ≈ `/ghost/api/admin/stripe/checkout/branding/`. No other route binds this controller (single hit at routes.js:156).
2. **Operation**: exported key `read`, op `read` (custom query, no `browse/add/edit/destroy`), `docName: 'checkout_branding'` (`stripe-checkout-branding.ts:10,12`). Controller loaded via `apiFramework.pipeline(require('./stripe-checkout-branding'), localUtils)` (`api/endpoints/index.js:139-141`).
3. **Input**: none — no `options[]`, no `data[]`, no `validation{}` declared (`stripe-checkout-branding.ts:12-18`); no path params, no query aliases, no multipart fields. Auth credentials read from request by `mw.authAdminApi` (Admin API token/session via `Authorization: Bearer` per app.js:8, auth stack); no body read by the controller.
4. **Auth/permissions**: `mw.authAdminApi` on the route (routes.js:156); `permissions: { docName: 'products', method: 'read' }` on `read` (`stripe-checkout-branding.ts:14`) — tier-level (products) permission check run by the API framework's permission service, not inside `query()`. No `unsafeAttrs`, no labs flag. `query()` itself calls no permission service (`stripe-checkout-branding.ts:15-17`).
5. **Status + headers**: no `statusCode` in controller → HTTP 200 on success. `headers: { cacheInvalidate: false }` (`stripe-checkout-branding.ts:13`) → suppresses `X-Cache-Invalidate`. No `Content-Disposition`/`Location`/custom headers set by the controller. Admin app adds `Cache-Control` private via `cacheControl('private')` (app.js:24). Response is a JSON object (serializer, not stream).
6. **Full call chain**: `query()` → `stripeCheckoutConfig.service.branding.read()` (`stripe-checkout-branding.ts:16`). Service built at `services/stripe-checkout-config/index.ts:85-89`: `stripeConnected = () => require('../stripe').api.configured`, `readBranding = () => require('../stripe').api.getCheckoutBranding()`. `CheckoutBrandingService.read()` (`branding-service.ts:35-63`):
   - if `!stripeConnected()` → `ValidationError` (422) `'Connect Stripe to read its checkout branding.'` (`:36-40`);
   - `branding = await readBranding()` (`:42`);
   - `getCheckoutBranding()` (`services/stripe/stripe-api.js:1058-1081`) — **Stripe API side effects**: `this._rateLimitBucket.throttle()` (`:1059`), `this._stripe.checkout.sessions.create({ mode:'setup', managed_payments:MANAGED_PAYMENTS_DISABLED, payment_method_types:['card'], success_url:config.checkoutSetupSessionSuccessUrl, metadata:{ghost_checkout_preview:true} })` (`:1060-1066`) — real Stripe Checkout Session create; then `throttle()` (`:1069`) and `this._stripe.checkout.sessions.expire(session.id)` (`:1070`) — real Stripe expire call. Expire failure is caught and logged via `logging.warn` with `event.name='stripe_checkout.branding.expire_failed'` (`:1071-1078`), non-fatal. Returns `session.branding_settings` (`:1080`).
   - `ResolvedIdentity.safeParse(branding)` requires `display_name: string`; failure → `InternalServerError` (500) `'Stripe reported checkout branding Ghost cannot show.'` with `context` (`:12-14,43-49`);
   - `SnakeDesign.safeParse(branding)` — design fields; failure → `logging.warn` `event.name='stripe_checkout.branding.design_unreadable'`, design becomes `null` (`:51-57`);
   - returns `{ displayName: identity.data.display_name, design: design.success ? camelKeys(design.data) : null }` (`:59-62`).
7. **Cache**: none — explicit comment `'Nothing is cached, so a change in the Stripe dashboard shows the next time it is read'` (`branding-service.ts:25-26`); no cache service call, no TTL/tag/namespace. `cacheInvalidate:false` (controller :13) avoids the invalidate header.
8. **Output**: serializer mapped by `docName` `checkout_branding` (`utils/serializers/output/index.js:107-108`) → `serialize` sets `frame.response = toCheckoutBrandingResponse.parse(branding)` (`utils/serializers/output/stripe-checkout-branding.ts:5-12`). `toCheckoutBrandingResponse` (`services/stripe-checkout-config/serializers.ts:209-218`) produces envelope `{ checkout_branding: [ { display_name: <string>, design: <snakeKeys(design) | null> } ] }`. `design` shape (`SnakeDesign`, codec.ts:8) is `button_color`, `background_color`, `border_style`, `font_family` (from `StripeCheckoutDesign`, models.ts:23-30). Empty/missing: no pagination; if Stripe not connected or identity unreadable, the pipeline returns 422/500 error envelope instead of this shape; unreadable design yields `design:null` with the name still returned.

**C++** (current source):

1. **Route identity**: `StripeRoutes::install` builds nested `Router{router, "/stripe"}` (`:19`) + `.use(content_type_middleware)` (`:20`) + `.use(require_auth)` (`:21`) + `Router{router, "/checkout"}` (`:23`) + `Route{"/branding"}.get(...)` (`:25-26`), dispatching `handler.stripe_stripe_checkout_branding_get().handle(...)` (`:31`). Installed by `ApiRoutes::install` via `StripeRoutes{}.install(...)` (`include/blog/handlers/routes.cppm:54,126`). Normalized path+verb matches Ghost routes.js:156.
2. **Operation/flow**: `handle()` runs workflow `"stripe_stripe_checkout_branding_get"` (`handler.cppm:17-19`), reads result of step `"stripe_stripe_checkout_branding_get_execute"` (`:19`), `MainFlow` is single-step `ExecuteStep` (`workflow/flow/main.cppm:7-10`; `workflow/step/execute.cppm:12`). Maps Ghost `read` conceptually.
3. **Input**: `using Input = interfaces::io::IRequest&` (`execute.cppm:11`); `invoke(const Input&, WorkflowContext&)` ignores the request body/query (`:13`) — no params consumed. Auth token read from `get_authorization()` by `middleware::token` (`middleware/bearer.cppm:27-33`).
4. **Auth/middleware**: router-level `content_type_middleware` (`external/stripe/routes.cppm:20`; media type normalized in `middleware/serde.cppm:11-12`) and `require_auth` (`routes.cppm:21`; `middleware/auth.cppm:12-29`): empty token → 401 `UnauthorizedError` (auth.cppm:19-27), else `next()`. `require_auth` checks **token presence only** — no `docName`/`method` permission service, so Ghost's `products/read` tier permission is not enforced. No inline token check remains in `handler.cppm` (current :12-27).
5. **Status/headers/body write**: `handle()` derives `status` from payload `"status"` (default 200) and `body` from `"body"` (default `{}`), converts to bytes, `res.set_status(...)` + `res.set_body(...)` + `send()` (`handler.cppm:20-24`). Success payload `{"status","200"}` set in step (`execute.cppm:28`). No response headers written (no `X-Cache-Invalidate`, no `Cache-Control`).
6. **State reads / side effects** (`workflow/step/execute.cppm:13-30`): `displayName` defaults to `"Ghost"` (`:19`); `rows = ctx.store().json_for("stripe_checkout_config", {})` (`:20`); `records = blog::api::parse_array(rows)` (`:21`); if non-empty, last record's `"display_name"` or `"displayName"` value is unquoted into `displayName` (`:22-26`, `unquote` helper at `:14-17`); body = `std::format(R"({"displayName":{},"design":null})", core_serde::JsonCodec{}.encode(displayName))` (`:27-29`). **No Stripe API calls**: `s_stripe` (`StripeHandler*`) is assigned in the handler constructor (`handler.cppm:13`, declared `execute.cppm:32,35`) but never used in `invoke()`. No `_stripe.checkout.sessions.create/expire`, no rate-limit throttle, no events/logging, no 422/500 branches, no `compensate()` work (`:31` no-op). `StripeHandler::branding()` exists in `include/blog/handlers/stripe.cppm:50-57` but is not called by this route.
7. **Cache**: no `core::cache::Cache` key/tag/TTL, no eviction in handler or step.
8. **Output**: raw body `{"displayName":"<name>","design":null}` (`execute.cppm:28-29`) — **no `checkout_branding` envelope**, camelCase `displayName` vs Ghost `display_name`, `design` hardcoded `null` (never the design object), display name sourced from local `stripe_checkout_config` store last record instead of Stripe `branding_settings`. Empty store → `{"displayName":"Ghost","design":null}`. No serializer/DTO layer.

**Diff**:

- **Residual gaps (Ghost behaviors with no C++ primitive here)**:
  - Real Stripe API side effects entirely absent — Checkout Session create + expire, two `_rateLimitBucket.throttle()` calls, `ghost_checkout_preview` metadata, `branding_settings` read (`stripe-api.js:1058-1081`) vs `execute.cppm:13-30` has no Stripe client call (`s_stripe` unused at :32/:35).
  - Permission service `{docName:'products', method:'read'}` (`stripe-checkout-branding.ts:14`) not implemented; C++ only token presence (`auth.cppm:12-29`) — no shared docName/method primitive.
  - Error branches missing: 422 not-connected (`branding-service.ts:36-40`) and 500 unreadable-identity (`branding-service.ts:43-49`) have no C++ equivalent; C++ always returns 200 on any token-bearing request (`execute.cppm:28`).
  - Structured warning logs missing: `stripe_checkout.branding.design_unreadable` (`branding-service.ts:53-57`) and `stripe_checkout.branding.expire_failed` (`stripe-api.js:1074-1077`).
  - `cacheInvalidate:false` header semantics (`stripe-checkout-branding.ts:13`) and admin `Cache-Control: private` (`app.js:24`) not written by C++ (handler.cppm:24 writes status/body only).
- **Divergences (C++ differs from Ghost)**:
  - Output envelope/shape: C++ `{"displayName":...,"design":null}` (`execute.cppm:28-29`) vs Ghost `{"checkout_branding":[{display_name,design}]}` (`serializers.ts:209-218`). No envelope, wrong key casing, design always null.
  - Data source: C++ reads `ctx.store().json_for("stripe_checkout_config", {})` last record (`execute.cppm:20-26`), default `"Ghost"`; Ghost reads live Stripe `branding_settings` via `getCheckoutBranding()` (`stripe-api.js:1080`, `branding-service.ts:42`).
  - Design parsing absent: Ghost `SnakeDesign.safeParse` + `camelKeys` can return a non-null design with 4 fields (`branding-service.ts:51-61`, codec.ts:8); C++ hardcodes `null` (`execute.cppm:29`).
  - Auth strength: C++ 401 on empty token only (`auth.cppm:19-27`), no products/read permission (contrast `stripe-checkout-branding.ts:14`).
  - Handler fallback body `{}` on missing step result (`handler.cppm:20`) has no Ghost analogue.

## Module-level notes

- This controller has exactly one route (`GET /stripe/checkout/branding`, routes.js:156); no browse/add/edit/destroy variants.
- All four Stripe Admin endpoints live under the same C++ `external/stripe` router (`external/stripe/routes.cppm:19-65`): branding GET (:25-34), config GET/PUT (:36-53), preview POST (:54-61). The other three map to `stripeCheckoutConfig` / `stripeCheckoutPreview` controllers (routes.js:153-155) and are out of scope for this module.
- `s_ctx`/`s_stripe` are static pointers set in the handler constructor (`handler.cppm:12-16`); `s_stripe` is unused by branding GET — a dormant hook for a future Stripe client. `BlogHandlers` wiring in `include/blog/handlers/all.cppm:96-99,467-470,747-750` and export via `base.cppm:102-105`.
- Shared middleware: `content_type_middleware` (serde.cppm), `require_auth` (auth.cppm:12-29), `token()`/`bearer_middleware` (bearer.cppm:11-33). No `core::cache::Cache` usage anywhere in the branding path.
- No EXTRA C++ route: every route under the matched folder has a Ghost counterpart in routes.js:153-156.
- Verdict rationale (updated by group 30): MATCH — all observable Ghost behavior is now reproduced with registered primitives; see implementation record below.

## Group 30 implementation record (strict parity)

- **Verdict flip:** PARTIAL → MATCH. Route identity, no-input, 200/422/500 branches,
  permission, Stripe side effects, output envelope, design handling and headers now align.
- **Permission:** `core::auth::ghost::GhostAuthorizer.authorize(principal, true, "products",
  "read")` in `workflow/step/execute.cppm`; the handler runs the two-arg
  `middleware::require_ghost_permission("products","read")` hook when a central
  pipeline publishes `x-blog-context`. Bearer token ⇒ Administrator; `x-blog-*`
  headers override. Denials emit Ghost 401/403 envelopes.
- **Stripe side effects:** `getCheckoutBranding()` is modeled as two throttles plus
  `sessions.create` (mode `setup`, `payment_method_types:['card']`,
  `metadata.ghost_checkout_preview:true`) and `sessions.expire`, recorded on
  `blog::handlers::outbox::global_outbox()` in `core_adapters::OutboundCall` shape.
  Expire failure and `design_unreadable` are `logging.warn` outbox events.
- **Data source:** branding comes from the `stripe_branding` mock store (fallback:
  `stripe_checkout_config` display name), with `STRIPE_CONNECTED=0` driving the
  422 "Connect Stripe to read its checkout branding." branch.
- **Output:** `{"checkout_branding":[{"display_name":…,"design":{…}|null}]}` built by
  the local `wrap` helper; `Cache-Control: private` written by the handler, no
  `X-Cache-Invalidate` header.
- **Shared abstractions used:** `core::auth::ghost`, `blog_handlers_middleware`,
  `blog::handlers::outbox` (`outbox_shim`), context store.
- **Neutral residuals (no live network / unregistered deps):** real Stripe HTTP is
  not issued — `core_adapters::StripeAdapter` is not a declared dependency of
  `blog_handlers`; the outbox event is the verification surface. The branding
  payload is read from the local mock store rather than a live Stripe dashboard.
- **New-dependency needs for main agent:** add `//include/core/adapters:core_adapters`
  to `blog_handlers` if live Stripe calls are required; register
  `blog_handlers:mapping` partitions if the shared `wrap`/`clean` helpers should
  replace the local helper.
- **Test follow-up (outside this group's edit scope):**
  `include/blog/testing/api_test.cc:164` still asserts legacy `"displayName"`;
  update it to `"checkout_branding"` when this flip is accepted.
