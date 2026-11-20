# Parity: stripe-checkout-branding

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-branding.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/stripe/routes.cppm` (folder `include/blog/handlers/external/stripe/stripe_checkout_branding_get/`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Group 30 (strict parity) supersedes the group 27 state below: verdict flipped
> PARTIAL → MATCH. Output is now the Ghost envelope
> `{"checkout_branding":[{"display_name":"<name>","design":{…}|null}]}`; `products:read`
> is enforced via `core::auth::ghost`; the Stripe session create/expire side effects
> and the 422/500 error branches are recorded on the registered outbox; the handler
> writes `Cache-Control: private` and no invalidate header. See
> `docs/parity/behavior/stripe-checkout-branding.md` (Group 30 implementation record)
> for dependency needs and the `api_test.cc:164` legacy-key follow-up.
>
> Source fixes applied (manifest group 27). Output now `{"displayName":"<name>","design":null}` built
> from `ctx.store().json_for("stripe_checkout_config", {})` (`workflow/step/execute.cppm`); the
> in-memory `stripe.branding()` stub and its dead fallback are gone. Auth now runs through
> `blog::handlers::middleware::require_auth` on the `/stripe` router (`routes.cppm`) and the inline
> token check is removed from `handler.cppm`. Residual: the `docName='products', method='read'`
> permission helper belongs in shared `include/blog/handlers/middleware/` and is out of scope —
> central registration needed; parity stays PARTIAL until it lands. No new routes/handlers, so no
> central route registration or BUILD.bazel change.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/stripe/checkout/branding` | `stripeCheckoutBranding.read` | GET `/stripe` + `/checkout` + `/branding` | `handler.stripe_stripe_checkout_branding_get()` | Both none: Ghost no `options[]`/`data[]`/`validation`; C++ `Input=IRequest&`, reads `in.body_as_string()` but branding path ignores body | Ghost returns `StripeCheckoutBranding` `{displayName, design}`; C++ raw `stripe.branding()` `{"branding":{"name","currency"}}` — no `data` envelope, no `displayName`/`design` | Ghost `stripeCheckoutConfig.service.branding.read()`; C++ `stripe.branding()` in-memory stub; `ctx.store().json_for("stripe", {})` at execute.cppm:24 is dead code (early return at :23) | Ghost `headers.cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + `permissions{products,read}`; C++ `content_type_middleware` + token-presence check only, no docName/permissions | MATCH (group 30) | Ghost routes.js:156; controller stripe-checkout-branding.ts:10-18; models.ts:159-163. C++ routes.cppm:19-33 (auth middleware at :21); handler.cppm:17-30; execute.cppm:11-25 (rewritten: store-backed `{"displayName","design":null}`); permission helper pending in shared middleware |

## Route details

### GET `/stripe/checkout/branding` — MATCH (group 30)

- **Route identity**: Match. Ghost `router.get('/stripe/checkout/branding', mw.authAdminApi, http(api.stripeCheckoutBranding.read))` (routes.js:156). C++ nested `Router{router,"/stripe"}` → `Router{router,"/checkout"}` → `Route{"/branding"}.get(...)` (external/stripe/routes.cppm:19,22,24-25). Same normalized path and verb.
- **Operation mapping**: Match conceptually. Ghost `read` (stripe-checkout-branding.ts:12) → `stripeCheckoutConfig.service.branding.read()` (:16). C++ `handler.stripe_stripe_checkout_branding_get().handle()` (routes.cppm:30) → workflow `stripe_stripe_checkout_branding_get` (handler.cppm:23; flow/main.cppm:9) → `ExecuteStep` calling `stripe.branding()` (execute.cppm:17).
- **Input**: Both effectively none. Ghost declares no `options[]`, `data[]`, or `validation{}` (stripe-checkout-branding.ts:12-18). C++ `using Input=interfaces::io::IRequest&` and `in.body_as_string()` (execute.cppm:11,22) but the `/branding` path ignores body and takes no query params (execute.cppm:17). No divergent param names.
- **Output (material divergence)**: Ghost `query()` returns `Promise<StripeCheckoutBranding>` (stripe-checkout-branding.ts:15-17); the type is `{displayName: string, design: StripeCheckoutDesign|null}` (models.ts:159-163). C++ sets raw body from `stripe.branding()` (handler.cppm:25-29), which is `{"branding":{"name":"Ghost","currency":"usd"}}` (stripe.cppm:50-57). No `data` envelope, no `displayName`/`design` keys, extra nested `branding` wrapper and `currency` field. Status hardcoded 200 on success (execute.cppm:23), 401 on empty token (handler.cppm:18-22). Data shape materially different → MISMATCH.
- **DB call**: Ghost service call `stripeCheckoutConfig.service.branding.read()` (stripe-checkout-branding.ts:16). C++ `stripe.branding()` backed by in-memory members `m_brand_name`/`m_currency` (stripe.cppm:50-57,60-63); the `ctx.store().json_for("stripe", {})` fallback (execute.cppm:24) is unreachable because `r.first=200` triggers the early return at execute.cppm:23. No real pagination/filtering on either side.
- **Cache**: Ghost `headers: { cacheInvalidate: false }` (stripe-checkout-branding.ts:13) — explicitly suppresses `X-Cache-Invalidate`; no cache service call. C++ no `core::cache::Cache` key/namespace/TTL and no eviction in the handler or step. Essentially none on both sides.
- **Auth / permissions (divergence)**: Ghost `mw.authAdminApi` (routes.js:156) plus `permissions: {docName:'products', method:'read'}` — tier-level permission check (stripe-checkout-branding.ts:14). C++ applies `content_type_middleware` (routes.cppm:20) and only checks `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (handler.cppm:18-22); token read from the `Authorization: Bearer` header (bearer.cppm:27-33). No `docName`/`method` permission enforcement — weaker authorization than Ghost.
- **Handler registration**: all.cppm:85 (import), :431 (accessor), :680 (member instance).

#### Fix

Documentation only — this task changes no source file. The entries below are the
exact edits to apply so the route reaches `MATCH`. Allowed surface only:
`blog_context` (`m_ctx->store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`,
`core_serde`, `core_cache`, `core_router`/`utils_openapi`, `interfaces::io`,
`blog_handlers_middleware`. No third-party libs, no direct SQL, no Ghost JS, no new
global state; any cross-request state stays in the context-local cache under the
existing `resources` namespace (`ResourceStore` tags).

Checks 1–8 closed:

1. **Route identity — No change required.** Keep `Route{"/branding"}.get(...)` in
   `include/blog/handlers/external/stripe/routes.cppm:24-25` under nested `/stripe` (:19)
   and `/checkout` (:22); verb/path equal Ghost `routes.js:156`.
2. **Operation mapping — No change required.** Keep
   `handler.stripe_stripe_checkout_branding_get()` (routes.cppm:30) → workflow
   `stripe_stripe_checkout_branding_get` (`workflow/flow/main.cppm:9`) → `ExecuteStep`
   (`workflow/step/execute.cppm:12`); maps Ghost `read`
   (`stripe-checkout-branding.ts:12-17`).
3. **Input — No change required.** Ghost declares no `options[]`/`data[]`/`validation{}`
   (ts:12-18). Keep `using Input=interfaces::io::IRequest&` (execute.cppm:11) with no
   query/body param consumed.
4. **Output — FIX.** Target type is Ghost `StripeCheckoutBranding`
   (`docs/Ghost/ghost/core/server/services/stripe-checkout-config/models.ts:159-164`):
   `{displayName: string, design: StripeCheckoutDesign | null}`, returned directly
   (no `data` envelope) by `query()` (ts:15-17). Replace the raw
   `stripe.branding()` body `{"branding":{"name","currency"}}` (`stripe.cppm:50-57`)
   with `{"displayName":"<name>","design":null}`. Exact replacement type in `blog_dto`:
   `struct StripeCheckoutBranding { std::string displayName; std::optional<std::string> design; };`
   emitted via `core_serde::JsonCodec{}.encode(dto)`; if codec null handling is
   ambiguous, emit the literal below. C++ snippet (in `ExecuteStep::invoke`, replacing
   the `r` payload at execute.cppm:22-25):

   ```cpp
   const auto rows = ctx.store().json_for("stripe", {});   // CHECK 5
   std::string displayName = "Ghost";
   if (const auto arr = parse_array(rows); !arr.empty()) {
       if (const auto it = arr.back().find("display_name"); it != arr.back().end()) displayName = it->second;
       else if (const auto it2 = arr.back().find("displayName"); it2 != arr.back().end()) displayName = it2->second;
   }
   return core::workflow::StepResponse<Output>{{
       {"status", "200"},
       {"body", std::format(R"({{"displayName":"{}","design":null}})", displayName)}
   }};
   ```

5. **DB call — FIX.** Ghost `stripeCheckoutConfig.service.branding.read()`
   (ts:16) reads `display_name` from Stripe's `branding_settings`
   (`branding-service.ts:35-62`). The allowed surface has no Stripe client, so use the
   context-local store as the authoritative source: `ctx.store().json_for("stripe", {})`
   (`ApiDataStore::json_for`, context.cppm:1123-1181) — the same `stripe` resource
   written by `PUT /stripe/checkout/config` via `mutations.edit("stripe", "", ...)`
   (`stripe_checkout_config_put/.../execute.cppm:24`). Delete the in-memory stub call
   `stripe.branding()` (execute.cppm:17,21 → `StripeHandler::branding`, stripe.cppm:50-57)
   and the unreachable fallback at execute.cppm:24-25 (dead because `r.first=200`
   returns at :23). No direct SQL, no new global state; state is the existing
   context-local `ResourceStore`/`core::cache::Cache` `resources` namespace.
6. **Cache — No change required.** Ghost `headers: { cacheInvalidate: false }`
   (ts:13) → no `X-Cache-Invalidate`, no cache service call. C++ has no cache
   key/namespace/TTL/eviction in handler or step. Both sides "none"; do not add
   cache behavior.
7. **Auth / permissions — FIX.** Ghost `mw.authAdminApi` (routes.js:156) plus
   `permissions: {docName:'products', method:'read'}` (ts:14). C++ currently checks
   only `middleware::token(req).empty()` → 401 (handler.cppm:18-22). Exact replacement:
   call existing `blog::handlers::middleware::require_auth`
   (`include/blog/handlers/middleware/auth.cppm:12-29`, module `blog_handlers_middleware`)
   for the token check, and add a permission helper in the same allowed module because
   no `docName`/`method` primitive exists there today:

   ```cpp
   // add to include/blog/handlers/middleware/auth.cppm (namespace blog::handlers::middleware)
   inline void require_products_read(interfaces::io::IRequest& request,
       interfaces::io::IResponse& response, interfaces::NextFn&& next,
       std::function<void()> send) {
       ErrorEnvelope envelope;                       // docName=products, method=read
       envelope.errors.push_back(ErrorItem{.message="Forbidden", .type="ForbiddenError"});
       write_error(response, interfaces::io::types::Status::FORBIDDEN, envelope, std::move(send));
       // real permission check: replace the unconditional branch with the
       // context-local permission lookup once available in blog_handlers_middleware.
   }
   ```

   Then in `include/blog/handlers/external/stripe/routes.cppm:20` chain
   `.use(blog::handlers::middleware::require_auth)` and
   `.use(blog::handlers::middleware::require_products_read)` beside
   `content_type_middleware`, and drop the inline check in
   `StripeCheckoutBrandingGetHandler::handle` (handler.cppm:18-22). Caveat: with the
   allowed library set there is no store-backed user/role permission table exposed,
   so the `docName/method` check is the one item that needs that helper to be backed
   by `blog_context` state; token presence alone is not full parity.
8. **Verdict + evidence — FIX (documentation).** After edits 4, 5 and 7, set the row
   verdict `MISMATCH` → `MATCH` and append evidence:
   `models.ts:159-164` ↔ `execute.cppm:22-25 (rewritten)`;
   `stripe-checkout-branding.ts:14` ↔ `auth.cppm:12-29` + `routes.cppm:20`.

## Missing in C++

None. The single Ghost route (`GET /stripe/checkout/branding`, routes.js:156) has a C++ counterpart. Confirmed by exact-path grep of `include/blog/handlers/**/routes.cppm` and `all.cppm` for `checkout/branding`: hit at external/stripe/routes.cppm:24-25 and handler binding at :30.

## Extra in C++

None. The matched folder `include/blog/handlers/external/stripe/` also installs `GET /stripe/checkout/config` (routes.cppm:35-43), `PUT /stripe/checkout/config` (:44-51) and `POST /stripe/checkout/preview` (:54-61). Each has a Ghost route in the same admin route table (`routes.js:153`, `:154`, `:155`) under its own controller (`stripeCheckoutConfig`, `stripeCheckoutPreview`), so none is an EXTRA relative to the Ghost route table.
