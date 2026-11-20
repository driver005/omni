# Parity: stripe-checkout-preview

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-preview.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/stripe/routes.cppm` (matched folder `external/stripe`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 3 |

> Group 30 (strict parity) supersedes the group 27 state below: verdict stays
> PARTIAL. `products:edit` is now enforced through `core::auth::ghost`, the input
> is validated with the Ghost discriminated design union and collection-flag 422s,
> the Stripe-connected / paid-active gates and Stripe refusal/no-URL branches are
> modeled, tier lookup runs through `core::query`, Stripe session calls are recorded
> on the registered outbox, and the literal `X-Cache-Invalidate: false` was removed.
> The returned URL is still deterministic — a real `session.url` needs the
> `core_adapters` dependency, which is not registered. See
> `docs/parity/behavior/stripe-checkout-preview.md` (Group 30 implementation record).
>
> Source fixes applied (manifest group 27). `ExecuteStep::invoke` now unwraps
> `data.checkout_preview[0]`, validates `tier_id` (`^[0-9a-f]{24}$`) and `cadence`
> (`month|year`), performs a store-backed tier gate via
> `ctx.store().json_for("tiers", {{"id",tier_id}})`, and returns the Ghost envelope
> `{"checkout_preview":[{"url":"..."}]}`; the unreachable `mutations.create("stripe", …)`
> fallback is gone. `handler.cppm` sets `X-Cache-Invalidate: false` before `send()`, and
> `require_auth` is wired on the `/stripe` router with the inline token check removed.
> Residuals (already accepted in the Fix block): real Stripe URL/network, labs collection flag,
> and the `products:edit` permission helper (shared `include/blog/handlers/middleware/`, out of
> scope — central registration needed). The shared `StripeHandler::preview` stub is unchanged;
> route output is built in the step, so no central edit is required for parity. No new routes →
> no central route registration or BUILD.bazel change.

Ghost controller exports a single `add` operation (`docName: 'checkout_preview'`), wired to exactly one route: `POST /stripe/checkout/preview` (`routes.js:155`). The matched C++ folder `external/stripe` installs four routes; one matches this route (MISMATCH on data shape) and three have no counterpart in *this* controller (EXTRA — they belong to sibling controllers `stripe-checkout-config` and `stripe-checkout-branding`).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/stripe/checkout/preview` | `add` | POST `/stripe` + `/checkout` + `/preview` | `stripe_stripe_checkout_preview_post` (`StripeCheckoutPreviewPostHandler`) | Ghost: `data.checkout_preview[0]` with `tier_id`/`cadence`/`design`/`shipping`/`phone`, Zod strict validation + flag gate. C++: `IRequest&`, raw `body_as_string()`, only `amount` read | Ghost: `{url}` → `{checkout_preview:[{url}]}`. C++: `{"preview":{"amount","currency","mode"}}`, status 200 | Ghost: `readTier(id)` + `checkouts.preview()` + `createPreviewLink()` (no save). C++: none taken (in-memory `StripeHandler`; `mutations.create("stripe",…)` fallback unreachable) | Ghost: `cacheInvalidate: false` (none). C++: none | Ghost: `mw.authAdminApi` + `products.edit`. C++: `require_auth` on `/stripe` router (:21); `products.edit` helper pending in shared middleware | PARTIAL | Ghost: ts:18-25, routes.js:155, preview-service.ts:49-82, serializers.ts:95-105/187-197. C++: routes.cppm:53-63, handler.cppm:17-30 (cache header), execute.cppm:11-25 (rewritten) |
| (none in this controller) | — | GET `/stripe` + `/checkout` + `/branding` | `stripe_stripe_checkout_branding_get` | — | — | — | — | — | EXTRA | C++: routes.cppm:24-31, all.cppm:85/680/431. Ghost: `routes.js:156` maps to `api.stripeCheckoutBranding.read`, not this controller; grep for `stripeCheckoutPreview` in routes.js → only line 155 |
| (none in this controller) | — | GET `/stripe` + `/checkout` + `/config` | `stripe_stripe_checkout_config_get` | — | — | — | — | — | EXTRA | C++: routes.cppm:35-43, all.cppm:82/677/428. Ghost: `routes.js:153` maps to `api.stripeCheckoutConfig.read`, not this controller |
| (none in this controller) | — | PUT `/stripe` + `/checkout` + `/config` | `stripe_stripe_checkout_config_put` | — | — | — | — | — | EXTRA | C++: routes.cppm:44-51, all.cppm:83/678/429. Ghost: `routes.js:154` maps to `api.stripeCheckoutConfig.edit`, not this controller |

## Route details

### POST /stripe/checkout/preview — PARTIAL

- Route identity MATCH: Ghost `router.post('/stripe/checkout/preview', …)` at `routes.js:155`; C++ `Router{router,"/stripe"}` + `Router{router,"/checkout"}` + `Route{"/preview"}.post(...)` at `include/blog/handlers/external/stripe/routes.cppm:19-22,53-63`. Both POST, same normalized path.
- Operation mapping MATCH: Ghost `api.stripeCheckoutPreview.add` (`routes.js:155`) → controller `add` (`stripe-checkout-preview.ts:18-25`); C++ `handler.stripe_stripe_checkout_preview_post()` (`routes.cppm:60`), registered `all.cppm:84,679,430`.
- Input MISMATCH: Ghost `frame.data.checkout_preview[0]` (`ts:22`) validated by `CheckoutPreviewInput` (`serializers.ts:95-105`: `tier_id` 24-hex, `cadence` month/year, `design`, `shipping`, `phone`) plus `refuseCollectionWhileOff(input, labs.isSet(COLLECTION_FLAG))` (`ts:23`, `serializers.ts:22-34`, `models.ts:111`). C++ `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) reads raw `in.body_as_string()` (`execute.cppm:22`); `StripeHandler::preview` parses only `"amount"` (`stripe.cppm:38-41`). No tier_id/cadence/design/shipping/phone, no Zod validation, no collection-flag gate → those names exist on Ghost side only.
- Output MISMATCH: Ghost `previews.add` returns `{url}` (`preview-service.ts:49,81`) serialized to `{checkout_preview:[{url}]}` (`serializers.ts:187-197`). C++ returns status 200 with `{"preview":{"amount":…,"currency":…,"mode":…}}` (`stripe.cppm:42-47`, `execute.cppm:23`, `handler.cppm:25-29`). Materially different data shape: Stripe Checkout URL vs. a static preview object. No `checkout_preview` envelope on C++ side.
- DB call MISMATCH: Ghost `readTier(tierId)`, `checkouts.preview()`, `createPreviewLink()` (`preview-service.ts:60,68-69`; wired `index.ts:79-83`) — saves nothing in Ghost but reads tier and calls Stripe. C++ executes only the in-memory lambda branch `r.first != 0` → `{200, stripe.preview(body)}` (`execute.cppm:16-23`); `mutations.create("stripe", …)` at `execute.cppm:24` is unreachable because the lambda always returns `200`. No tier read, no Stripe call. `mutations = ctx.store()` (`execute.cppm:14`) is unused on the taken path.
- Cache MATCH (both none): Ghost `headers: { cacheInvalidate: false }` (`ts:19`). C++ step uses no `core::cache::Cache`; the shared cache is wired only to members/emails/automations ops (`all.cppm:836-846`).
- Auth MISMATCH: Ghost `mw.authAdminApi` (`routes.js:155`) + `permissions: { docName: 'products', method: 'edit' }` (`ts:20`), `docName: 'checkout_preview'` (`ts:16`). C++ checks only `middleware::token(req).empty()` → `401 {"error":"Unauthorized"}` (`handler.cppm:18-22`); `content_type_middleware` on `/stripe` router (`routes.cppm:20`). No permission/docName enforcement anywhere in the C++ handler or step.

#### Fix

Documentation-only. Exact files + symbols: `include/blog/handlers/external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`; `include/blog/handlers/external/stripe/stripe_checkout_preview_post/workflow/step/execute.cppm` → `blog::handlers::stripe::stripe_checkout_preview_post::workflow::ExecuteStep::invoke`; `include/blog/handlers/external/stripe/stripe.cppm` → `blog::api::StripeHandler::preview`; `include/blog/handlers/external/stripe/stripe_checkout_preview_post/handler.cppm` → `StripeCheckoutPreviewPostHandler::handle`; `include/blog/context/context.cppm` → `blog::api::ApiDataStore::json_for` / `blog::api::JsonBody::parse` / `blog::api::ApiError::validation`.

- Check 1 (route identity) closed — no change. `POST /stripe/checkout/preview` is registered at `routes.cppm:53-63` via nested `Router{router,"/stripe"}` → `Router{router,"/checkout"}` (`:19,22`) + `Route{"/preview"}.post(...)`; normalized path and verb match `routes.js:155`. Do not add/remove routes.
- Check 2 (operation mapping) closed — no change. `handler.stripe_stripe_checkout_preview_post()` (`routes.cppm:60`, registry/accessor/member `all.cppm:84,679,430`) maps to Ghost `add` (`stripe-checkout-preview.ts:18-25`).
- Check 3 (input) closed with this edit. In `ExecuteStep::invoke` keep `using Input = interfaces::io::IRequest&`; replace the raw `in.body_as_string()` forwarded at `execute.cppm:22` with the unwrapped Ghost envelope `data.checkout_preview[0]`: decode with `core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string())`, use `std::get_if<serde::Value::Object>` / `std::get_if<serde::Value::Array>` (the same pattern as `context.cppm:314-317`), and flatten into a local `struct PreviewInput { std::string tier_id, cadence, design, shipping, phone; }` via `blog::api::JsonBody{}.parse(...)` (`context.cppm:309`). Validate `tier_id` against `^[0-9a-f]{24}$` and `cadence` ∈ {`month`,`year`} (default `month`); on failure return `{"status","422"}` with `blog::api::ApiError{}.validation(...)` (`context.cppm:76-84`). Pass the encoded validated object (`core_serde::JsonCodec{}.encode(...)`) to `stripe.preview(inner)` — not the whole envelope. The `refuseCollectionWhileOff(input, labs.isSet(COLLECTION_FLAG))` gate (`ts:23`, `serializers.ts:22-34`, `models.ts:111`) has no labs primitive in the allowed module set — accepted residual; do not invent global state.
- Check 4 (output) closed with this edit. In `StripeHandler::preview` (`stripe.cppm:38-48`) replace the return format string `{"preview":{"amount":…,"currency":…,"mode":…}}` with the Ghost envelope `{"checkout_preview":[{"url":"…"}]}`; derive the URL deterministically from `tier_id`/`cadence` (no third-party client). `StripeCheckoutPreviewPostHandler::handle` (`handler.cppm:17-30`) already propagates `status`/`body` from the step's `std::map<std::string,std::string>` — no change there. The real Stripe `checkout.stripe.com/…` URL from `createPreviewLink()` (`preview-service.ts:69-81`) is an accepted residual (network/Stripe SDK not in allowed set); the data shape is fixed.
- Check 5 (DB call) closed with this edit. Replace the unreachable `mutations.create("stripe", …)` fallback (`execute.cppm:24`) with a real tier read via the allowed store: `const auto tiers = ctx.store().json_for("tiers", {{"id", tier_id}});` (`context.cppm:1123-1134`); reject with 422 `ApiError{}.validation("Choose an active paid tier to preview checkout for.", "…", "tier_id")` when empty, mirroring `preview-service.ts:60-66`. Keep `mutations = ctx.store()` (`execute.cppm:14`) solely as the store handle; no direct SQL. `stripeConnected()`/`checkouts.preview()` (`preview-service.ts:56,68`, wired `index.ts:78-81`) remain accepted residuals — no network facility allowed.
- Check 6 (cache) closed with this edit. Ghost `headers: { cacheInvalidate: false }` (`ts:19`). Add `res.set_header("X-Cache-Invalidate", "false")` in `StripeCheckoutPreviewPostHandler::handle` immediately before `send()` (`handler.cppm:29`); no `core::cache::Cache` key/namespace needed — neither side caches this request. No cross-request state is introduced.
- Check 7 (auth) closed with this edit. Replace the inline `middleware::token(req).empty()` guard (`handler.cppm:18-22`) with `.use(blog::handlers::middleware::require_auth)` on the `/stripe` Router in `StripeRoutes::install` (`routes.cppm:19-20`; symbol `blog::handlers::middleware::require_auth`, `include/blog/handlers/middleware/auth.cppm:12-29`) so 401 uses Ghost's error envelope. Ghost `permissions:{docName:'products',method:'edit'}` (`ts:20`) has no allowed C++ primitive — accepted residual.
- Check 8 (verdict/evidence) closed: with checks 3–7 applied the row is `PARTIAL` (envelope, input names/validation, store-backed tier read, cache header and auth middleware fixed; only real Stripe URL/network, labs flag and `permissions` remain accepted residuals). Keep both sides' `file:line` evidence; do not change Summary counts until the C++ edit lands. No third-party libs, direct SQL, Ghost JS or new global state used.

Short snippet where ambiguity remains (input unwrap + store-backed tier gate, `ExecuteStep::invoke`):

```cpp
const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
const auto* root = doc ? std::get_if<serde::Value::Object>(&doc->get()) : nullptr;
const auto* arr  = (root && root->contains("data"))
    ? std::get_if<serde::Value::Array>(&root->at("data")) : nullptr;
const std::string inner = (arr && !arr->empty())
    ? core_serde::JsonCodec{}.encode(arr->at(0)) : std::string{"{}"};
const auto input = blog::api::JsonBody{}.parse(inner);       // tier_id, cadence, design, shipping, phone
const std::string tier_id = input.contains("tier_id") ? input.at("tier_id") : "";
// validate ^[0-9a-f]{24}$ + cadence ∈ {month,year} → 422 ApiError{}.validation(...)
const auto tiers = ctx.store().json_for("tiers", {{"id", tier_id}});
if (tiers == "[]" || tiers == "{}") {
    return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", blog::api::ApiError{}.validation(
            "Choose an active paid tier to preview checkout for.", "body", "tier_id")}}};
}
const auto r = /* existing lambda */ (*s_stripe, "POST", inner, std::string{in.get_path()});
```

### GET /stripe/checkout/branding — EXTRA

- C++ route `Route{"/branding"}.get(...)` at `routes.cppm:24-31`, handler `stripe_stripe_checkout_branding_get` (`all.cppm:85,680,431`).
- Absence in this controller: `stripe-checkout-preview.ts` exposes only `add` (`ts:15-27`). Grep `stripeCheckoutPreview` in `routes.js` returns only line 155. This route is mapped in `routes.js:156` to `api.stripeCheckoutBranding.read` (sibling controller), so it is EXTRA relative to this controller.

#### Fix

No change required. Exact file + symbol: `include/blog/handlers/external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/branding"}.get(...)` (`routes.cppm:24-31`); handler `stripe_stripe_checkout_branding_get` (`all.cppm:85,680,431`). Check 1 closed — the C++ route has a real Ghost counterpart in the admin route table (`routes.js:156` → `api.stripeCheckoutBranding.read`, sibling controller `stripe-checkout-branding.ts`); it is EXTRA only relative to this controller. Checks 2–7 are n/a for this controller (its sole op is `add`, `ts:15-27`); keep the registration and its shared `/stripe` middleware chain untouched — do not delete or re-home the handler. Check 8 closed — EXTRA classification stands with evidence above (`grep stripeCheckoutPreview routes.js` → only `:155`). No replacement call/type; no new global state; no third-party libs, direct SQL or Ghost JS.

### GET /stripe/checkout/config — EXTRA

- C++ route `Route{"/config"}.get(...)` at `routes.cppm:35-43`, handler `stripe_stripe_checkout_config_get` (`all.cppm:82,677,428`).
- Absence in this controller: not declared in `stripe-checkout-preview.ts`; `routes.js:153` maps it to `api.stripeCheckoutConfig.read` (sibling controller).

#### Fix

No change required. Exact file + symbol: `include/blog/handlers/external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/config"}.get(...)` (`routes.cppm:35-43`); handler `stripe_stripe_checkout_config_get` (`all.cppm:82,677,428`). Check 1 closed — Ghost counterpart exists at `routes.js:153` → `api.stripeCheckoutConfig.read` (sibling controller `stripe-checkout-config.ts`). Checks 2–7 n/a for this controller (exposes only `add`, `ts:15-27`); keep the route and shared `/stripe` middleware. Check 8 closed — EXTRA relative to this controller with evidence above; do not remove the handler. No replacement call/type; no new global state; no third-party libs, direct SQL or Ghost JS.

### PUT /stripe/checkout/config — EXTRA

- C++ route `Route{"/config"}.put(...)` at `routes.cppm:44-51`, handler `stripe_stripe_checkout_config_put` (`all.cppm:83,678,429`).
- Absence in this controller: not declared in `stripe-checkout-preview.ts`; `routes.js:154` maps it to `api.stripeCheckoutConfig.edit` (sibling controller).

#### Fix

No change required. Exact file + symbol: `include/blog/handlers/external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/config"}.put(...)` (`routes.cppm:44-51`); handler `stripe_stripe_checkout_config_put` (`all.cppm:83,678,429`). Check 1 closed — Ghost counterpart exists at `routes.js:154` → `api.stripeCheckoutConfig.edit` (sibling controller `stripe-checkout-config.ts`). Checks 2–7 n/a for this controller (`add` only, `ts:15-27`); keep the route and shared `/stripe` middleware. Check 8 closed — EXTRA relative to this controller with evidence above; do not remove the handler. No replacement call/type; no new global state; no third-party libs, direct SQL or Ghost JS.

## Missing in C++

None. The sole Ghost controller route (`POST /stripe/checkout/preview`, `routes.js:155`) has a C++ route at the same path and verb; the divergence is in data shape/behavior, not route absence.

#### Fix

No change required. Check 1 is closed — the only Ghost route in this controller (`POST /stripe/checkout/preview`, `routes.js:155`) already has an exact path+verb C++ registration (`external/stripe/routes.cppm:53-63`) and handler (`all.cppm:84,679,430`); `MISSING` count stays 0. Nothing to add; the `#### Fix` on the route detail reshapes the existing `ExecuteStep`/`StripeHandler::preview`/handler to close checks 2–8. No new global state; no third-party libs, direct SQL or Ghost JS.

## Extra in C++

All three are in the matched folder `include/blog/handlers/external/stripe/` and have no route in `stripe-checkout-preview.ts` (they belong to sibling Ghost controllers):

1. `GET /stripe/checkout/branding` — `routes.cppm:24-31`, handler `stripe_stripe_checkout_branding_get` (`all.cppm:85,680,431`); Ghost sibling mapping `routes.js:156`.
   - Fix: No change required. Exact file + symbol `external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/branding"}.get(...)`; keep it — `routes.js:156` maps it to `api.stripeCheckoutBranding.read` (sibling controller `stripe-checkout-branding.ts`). Check 1 closed by that mapping; checks 2–8 n/a for this `add`-only controller. No replacement call/type, no router edit, no new global state.
2. `GET /stripe/checkout/config` — `routes.cppm:35-43`, handler `stripe_stripe_checkout_config_get` (`all.cppm:82,677,428`); Ghost sibling mapping `routes.js:153`.
   - Fix: No change required. Exact file + symbol `external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/config"}.get(...)`; keep it — `routes.js:153` maps it to `api.stripeCheckoutConfig.read` (sibling controller `stripe-checkout-config.ts`). Check 1 closed by that mapping; checks 2–8 n/a here. No replacement call/type, no router edit, no new global state.
3. `PUT /stripe/checkout/config` — `routes.cppm:44-51`, handler `stripe_stripe_checkout_config_put` (`all.cppm:83,678,429`); Ghost sibling mapping `routes.js:154`.
   - Fix: No change required. Exact file + symbol `external/stripe/routes.cppm` → `blog::api::StripeRoutes::install`, `Route{"/config"}.put(...)`; keep it — `routes.js:154` maps it to `api.stripeCheckoutConfig.edit` (sibling controller `stripe-checkout-config.ts`). Check 1 closed by that mapping; checks 2–8 n/a here. No replacement call/type, no router edit, no new global state.
