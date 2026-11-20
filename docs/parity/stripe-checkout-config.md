# Parity: stripe-checkout-config

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-config.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 153-154)
- C++: `include/blog/handlers/external/stripe/routes.cppm` (shared Stripe router); handlers `.../stripe_checkout_config_get/` and `.../stripe_checkout_config_put/`
- Checked: 2026-11-19

Notes on key mapping: Ghost exports the controller under key `stripeCheckoutConfig` in `docs/Ghost/ghost/core/server/api/endpoints/index.js:131-132` (`require('./stripe-checkout-config')`). The controller itself declares `docName: 'checkout_config'` (stripe-checkout-config.ts:20) and exposes ops `read` + `edit` only.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Group 30 (strict parity) supersedes the group 27 state below: GET flipped
> PARTIAL → MATCH, PUT stays PARTIAL. GET now filters `slug=default` through
> `core::query`, validates the SnakeDesign shape and emits
> `{"checkout_config":[{design}]}` under `products:read`; PUT now has strict
> `checkout_config[0]` validation, a full-row upsert, a structured staff action
> and outbox transaction, and conditional `X-Cache-Invalidate: /*` under
> `products:edit`. Tier-requirement/metafield-binding tables and the labs flag
> remain unmodeled. See `docs/parity/behavior/stripe-checkout-config.md`
> (Group 30 implementation record).
>
> Source fixes applied (manifest group 27). GET: response is now
> `{"checkout_config":[{"design":{"customize":...}}]}` read from
> `ctx.store().json_for("stripe_checkout_config", {})` with `slug=="default"` filtering; the
> in-memory `stripe.config()` stub and dead fallback are gone. PUT: `data.checkout_config[0]` is
> decoded/validated, the row is persisted via `store.edit/create("stripe_checkout_config", ...)`
> (with an `actions` record), an action is recorded, the response uses the GET shape, and
> `X-Cache-Invalidate: /*` is emitted when `changed` is reported. Auth runs through
> `require_auth` on the `/stripe` router; inline token checks removed from both handlers.
> Residual: `docName='products', method={read|edit}` permission helpers live in shared
> `include/blog/handlers/middleware/` (out of scope) — central registration needed; both rows
> stay PARTIAL. No MISSING route → no new handler/BUILD.bazel registration.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/stripe/checkout/config` | `read` | GET `/stripe/checkout/config` (routes.cppm:35-42) | `stripe_stripe_checkout_config_get` (routes.cppm:41; handler.cppm:10) | Ghost: no `options[]`/`data[]`; query takes no frame (controller:25). C++: `Input=IRequest&`, no query parse; only `in.body_as_string()` passed into lambda (get execute.cppm:11,22). | Ghost: serializer builds `{checkout_config:[{design,shipping?,phone?}]}` (output serializer:8-18; serializers.ts:166-185). C++: `{config:{session_mode,currency,payment_methods}}` from in-memory `StripeHandler` (stripe.cppm:15-23; get execute.cppm:18,23). | Ghost: `config.read()` → `readSaved()`, `stripe_checkout_config` table + bindings join, `slug='default'` (service.ts:312-314,352-364; schema.ts:5,8). C++: no DB read in reachable path — lambda returns 200 so `json_for("stripe")` (get execute.cppm:24-25) is dead. | Ghost: `headers.cacheInvalidate:false` (controller:23); no invalidation on read. C++: none. | Ghost: `mw.authAdminApi` (routes.js:153) + permissions `products:read` (controller:24). C++ group 30: `core::auth::ghost` `products:read` gate + two-arg permission hook; `core::query` slug filter; strict design output. | MATCH | Ghost routes.js:153 + controller:22-28; C++ routes.cppm:35-43 (auth at :21) + get handler.cppm:10-30 + get execute.cppm:11-25 (rewritten) |
| PUT `/stripe/checkout/config` | `edit` | PUT `/stripe/checkout/config` (routes.cppm:35,44-51) | `stripe_stripe_checkout_config_put` (routes.cppm:49; handler.cppm:10) | Ghost: `frame.data.checkout_config[0]` (controller:9,34) → `refuseCollectionWhileOff` (controller:35; serializers.ts:22-34) + `CheckoutConfigInput` strictObject `{design,shipping?,phone?}` (serializers.ts:82-88); `actingContext(frame.options.context)` (controller:38; actions.ts:25-31). C++: raw `in.body_as_string()`, no DTO/schema/validation; `configure()` reads flat keys `session_mode/currency/payment_methods` (put execute.cppm:11,19,22,24; stripe.cppm:25-36). | Ghost: returns `config.read()` → `{checkout_config:[...]}` (controller:44; output serializer:8-18); sets `X-Cache-Invalidate: /*` when `changedRequirements` (controller:41-43). C++: `{config:{session_mode,currency,payment_methods}}`, status 200, no cache header (stripe.cppm:15-23; put handler.cppm:26-29). | Ghost: `config.edit(...)` writes `stripe_checkout_config` table (insert/merge on slug) + bindings, records Action, computes `changedRequirements` (service.ts:405-447,421-445). C++: `stripe.configure(body)` mutates in-memory fields only; `mutations.edit("stripe",...)` (put execute.cppm:24) is dead code because lambda always returns 200 (line 19,23). | Ghost: `headers.cacheInvalidate:false` (controller:31) but explicit `X-Cache-Invalidate: /*` on requirement change (controller:42). C++: none. | Ghost: `mw.authAdminApi` (routes.js:154) + permissions `products:edit` (controller:32) + staff-only `actingContext` (actions.ts:25-31). C++: token-presence only, 401 on empty (put handler.cppm:18-22); no `products:edit` helper in shared middleware. | PARTIAL | Ghost routes.js:154 + controller:30-46; C++ routes.cppm:44-51 (auth at :21) + put handler.cppm:10-30 (cache header) + put execute.cppm:11-25 (rewritten) |

## Route details

### GET /stripe/checkout/config — MATCH (group 30)
- Route identity: both GET `/stripe/checkout/config` (Ghost routes.js:153; C++ routes.cppm:35-42 under `/stripe` + `/checkout` routers, routes.cppm:19,22).
- Operation mapping: Ghost `read` (controller:22-28) vs C++ `stripe_stripe_checkout_config_get().handle` (routes.cppm:41).
- Input: no params on Ghost side (controller:25); C++ declares `Input=interfaces::io::IRequest&` and parses no query/body fields, only forwards `in.body_as_string()` (get execute.cppm:11,22). No divergent param names.
- Output: Ghost serializer wraps in `{checkout_config:[{design,shipping?,phone?}]}` with flag-gated collected parts (output serializer:8-18; serializers.ts:166-185). C++ returns `{"config":{"session_mode":"payment","currency":"usd","payment_methods":["card"]}}` (stripe.cppm:15-23,60-62) — different envelope and field set. Material data-shape divergence.
- DB call: Ghost `config.read()` → `readSaved()` (service.ts:312-314) queries `stripe_checkout_config as config` LEFT JOIN `stripe_checkout_config_bindings`, `where config.slug='default'` (service.ts:352-364; schema.ts:5,8; config join tables service.ts:357-361). C++ never reaches `ctx.store().json_for("stripe", {})` (get execute.cppm:24) because the lambda always returns status 200 (line 18,23) and the step returns early. No real DB read or filtering.
- Cache: Ghost read has `cacheInvalidate:false`, no `X-Cache-Invalidate` set (controller:23). C++ none (no cache import in get handler/step).
- Auth: Ghost `mw.authAdminApi` + `permissions:{docName:'products',method:'read'}` (routes.js:153; controller:24). C++ only checks token non-empty → 401 `{"error":"Unauthorized"}` (get handler.cppm:18-22); bearer token via middleware/bearer.cppm:27, `content_type_middleware` applied at routes.cppm:20. No method/permission check.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later.

1. Route identity — no change. `include/blog/handlers/external/stripe/routes.cppm:19,22,35-43` already `Router{router,"/stripe"}` → `Router{router,"/checkout"}` → `Route{"/config"}.get(...)`; installer symbol `blog::api::StripeRoutes::install` (`:15`); Ghost `routes.js:153`. Verbatim path and verb match.
2. Operation mapping — no change. `stripe_checkout_config_get/workflow/flow/main.cppm:8-10` `MainFlow::build()` registers `ExecuteStep` as workflow `stripe_stripe_checkout_config_get`; `stripe_checkout_config_get/handler.cppm:23` runs that name. Ghost key `read` (`controller:22-28`). Op names agree conceptually; payload does not.
3. Input — no change. Ghost declares no `options`/`data`/`validation` (`controller:25`); `stripe_checkout_config_get/workflow/step/execute.cppm:11` `using Input=interfaces::io::IRequest&`, no query parse, only forwards `in.body_as_string()` (`:22`) into the dispatch lambda. GET bodies are empty, so no divergent param names.
4. Output — REPLACE. Symbols: (a) `blog::api::StripeHandler::config` in `include/blog/handlers/stripe.cppm:15-23` emits `{"config":{"session_mode","currency","payment_methods"}}` — wrong envelope/fields; keep the method but stop using it for this route. (b) `blog::handlers::stripe::stripe_checkout_config_get::workflow::ExecuteStep::invoke` in `stripe_checkout_config_get/workflow/step/execute.cppm:13-25`: replace the GET branch at `:18` (`if (method=="GET") return {200, stripe.config()};`) with a body built from the stored row — `{"checkout_config":[{"design":<design>}]}` — where `<design>` is `{"customize":false}` when the `design` column is null and `{"customize":true,...snakeKeys}` otherwise, mirroring Ghost's `toCheckoutConfigResponse` (`serializers.ts:174-185`; output serializer `:8-13`). Delete the dead fallback at `:24-25` (`json_for("stripe",...)` + hardcoded pagination). Add `import core_serde;` and decode the stored `design` string with `core_serde::JsonCodec{}` (core_serde). Keep status 200. No new third-party lib.
5. DB call — REPLACE. Ghost `config.read()` → `readSaved()` (`service.ts:312-314,352-368`) queries `stripe_checkout_config as config` LEFT JOIN `stripe_checkout_config_bindings`, `where config.slug='default'` (`schema.ts:5,8`). C++ never reads the config: in-memory `stripe.config()` (`stripe.cppm:15-23,60-63`) plus dead `ctx.store().json_for("stripe", {})` (`get execute.cppm:24` — unreachable because the lambda returns at `:21-23`). Replace with `ctx.store().json_for("stripe_checkout_config", {})` (resource string `"stripe_checkout_config"`; schema entry at `context.cppm:208`, fields `{id,slug,design,created_at,updated_at}`; `json_for` fallthrough to `m_resources.list` at `context.cppm:1123-1126,1180`), then `blog::api::parse_array` (`context.cppm:595`) and filter `slug=="default"` in C++ to mirror the Ghost `where`. Context-local store only — no SQL, no new globals.
6. Cache — no change. Ghost read `cacheInvalidate:false`, no `X-Cache-Invalidate` (`controller:23`). C++ none (no `core::cache::Cache` reference in get handler/step; `all.cppm` wires cache only to members/emails/automations). Both sides: none. Note `ResourceStore` writes use `Ttl{}.never()` under tag `resources` (`context.cppm:645-646`) — existing namespace, no new state.
7. Auth — REPLACE to close docName/method gap. Ghost `mw.authAdminApi` + `permissions:{docName:'products',method:'read'}` (`routes.js:153`; `controller:24`). Keep bearer `token(req).empty()` → 401 (`get handler.cppm:18-22`; `middleware/bearer.cppm:27`), but add a permission check: in allowed module `blog_handlers_middleware` (new `include/blog/handlers/middleware/permission.cppm`, exported via `blog_handlers_middleware`) add `require_permission(req,res,next,send)` keyed `docName="products"`, action `"read"`; wire it in `StripeRoutes::install` GET lambda (`routes.cppm:36-42`) ahead of `handler.stripe_stripe_checkout_config_get().handle(...)`. Exact replacement for the inline check: wrap the existing body with `blog::handlers::middleware::require_auth(req,res,...)` (`middleware/auth.cppm:12-29`) then `require_permission`. Permission state under `ctx.store().resources()` (`resources` namespace), no new globals. Absence proof: grep `require_permission|permission_middleware|docName` over `include/blog/handlers/**/*.cppm` → no matches.
8. Verdict + evidence — after 4,5,7 path/verb, op, input, output, DB resource, cache and auth agree → `MATCH`. Recheck `routes.js:153`, `controller:22-28`, `output/stripe-checkout-config.ts:8-13`, `serializers.ts:166-185`, `service.ts:312-314,352-368` vs `routes.cppm:19,22,35-43`, `get/handler.cppm:17-30`, `get/.../execute.cppm:11-25`, `stripe.cppm:15-23`.

Ambiguity — collected-part flag gating: Ghost hides `shipping`/`phone` unless `COLLECTION_FLAG` is set (`output serializer:9-12`); C++ has no labs-flag module. Short snippet for check 4 (replaces the GET branch; `import core_serde;`):

```cpp
// stripe_checkout_config_get/workflow/step/execute.cppm
const auto rows = ctx.store().json_for("stripe_checkout_config", {});
std::string body = R"({"checkout_config":[{"design":{"customize":false}}]})";
for (const auto& rec : blog::api::parse_array(rows)) {
    const auto slug = rec.contains("slug") ? rec.at("slug") : std::string{};
    if (slug != R"("default")") continue;            // where config.slug='default'
    const auto design = rec.contains("design") ? rec.at("design") : std::string{"null"};
    if (design != "null" && design != "") {
        body = std::format(R"({{"checkout_config":[{{"design":{{"customize":true,{}}}}} ]}})",
                           design.substr(1, design.size() > 1 ? design.size() - 2 : 0));
    }
    break;                                            // single config row
}
return {200, body};
```

`design` is stored as snake_case JSON text (`service.ts:413`, `codec.ts`); stripping its braces and prefixing `"customize":true` reproduces `snakeKeys(settings.design)` (`serializers.ts:178-180`). `shipping`/`phone` are omitted (flag off) — correct for the default C++ path; add them only when a stored binding exists and the flag is modeled.

### PUT /stripe/checkout/config — PARTIAL
- Route identity: both PUT `/stripe/checkout/config` (Ghost routes.js:154; C++ routes.cppm:35,44-51).
- Operation mapping: Ghost `edit` (controller:30-46) vs C++ `stripe_stripe_checkout_config_put().handle` (routes.cppm:49).
- Input: Ghost typed body `frame.data.checkout_config[0]` (controller:9,34), validated by `refuseCollectionWhileOff` (controller:35; serializers.ts:22-34) and `CheckoutConfigInput = strictObject({design, shipping?, phone?})` (serializers.ts:82-88), with acting user context (controller:38; actions.ts:25-31). C++ uses raw `in.body_as_string()` with no serde DTO/validation; `configure()` consumes flat keys `session_mode`, `currency`, `payment_methods` (put execute.cppm:11,19,22,24; stripe.cppm:25-36) — not the `checkout_config` tuple/design/shipping/phone schema. Input names present only on Ghost side: `checkout_config`, `design`, `shipping`, `phone`; only on C++ side: `session_mode`, `currency`, `payment_methods`.
- Output: Ghost returns `config.read()` with `{checkout_config:[...]}` envelope (controller:44; output serializer:8-18) and conditionally sets `X-Cache-Invalidate: /*` (controller:41-43). C++ returns `{"config":{...}}` status 200 (stripe.cppm:15-23; put handler.cppm:26-29), no cache header.
- DB call: Ghost `config.edit(context,input)` (controller:37-40) parses `CheckoutConfigInput` (service.ts:410), inserts/merges `stripe_checkout_config` row (`slug='default'`), writes target bindings, records an Action, and returns `changedRequirements` (service.ts:417-446). C++ calls `stripe.configure(body)` (in-memory only) and the `mutations.edit("stripe", ...)` write (put execute.cppm:24) is unreachable because the lambda returns 200 (line 19,23). No DB write.
- Cache: Ghost `cacheInvalidate:false` header default (controller:31) but explicit `X-Cache-Invalidate: /*` on changed requirements (controller:42). C++ none.
- Auth: Ghost `mw.authAdminApi` + `permissions:{docName:'products',method:'edit'}` (routes.js:154; controller:32), staff-only `actingContext` (actions.ts:25-31). C++ token-presence only → 401 (put handler.cppm:18-22); no `products:edit` permission.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later.

1. Route identity — no change. `routes.cppm:19,22,35,44-51` already `Route{"/config"}.put(...)` under `Router{router,"/checkout"}` / `/stripe`; Ghost `routes.js:154`. Verbatim path and verb match.
2. Operation mapping — no change. `stripe_checkout_config_put/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as `stripe_stripe_checkout_config_put`; `stripe_checkout_config_put/handler.cppm:23` runs it. Ghost key `edit` (`controller:30-46`).
3. Input — REPLACE (DTO, top-level unwrap, validation, acting context). Symbol `blog::handlers::stripe::stripe_checkout_config_put::workflow::ExecuteStep::invoke` in `stripe_checkout_config_put/workflow/step/execute.cppm:13-24`: replace raw `in.body_as_string()` (`:22`, `:24`) with `core_serde::JsonCodec{}.decode<CheckoutConfigDto>(body)` (core_serde). Define `CheckoutConfigDto` in allowed module `blog_dto` (`import blog_dto;` already at `:7`) with `design` (string) + optional `shipping`, `phone`, and require a top-level `checkout_config` array with ≥1 object (Ghost `frame.data.checkout_config[0]`, `controller:9,34`). Add input names `checkout_config`, `design`, `shipping`, `phone`; drop flat `session_mode`, `currency`, `payment_methods` from this path (they belong to `configure()` at `stripe.cppm:25-36`, not checkout config). Mirror `refuseCollectionWhileOff` (`controller:35`; `serializers.ts:22-34`): reject `shipping`/`phone` when the flag is off (C++ has no labs flag → default reject; 400). `actingContext(frame.options.context)` (`controller:38`; `actions.ts:25-31`) has no staff/integration split in C++ — record actor only if a bearer identity is present, else null (see ambiguity).
4. Output — REPLACE. Ghost returns `config.read()` → `{checkout_config:[...]}` (`controller:44`; output serializer `:8-13`) and conditionally sets `X-Cache-Invalidate: /*` (`controller:41-43`). C++ returns `{"config":{...}}` (`stripe.cppm:15-23`; `put handler.cppm:26-29`). Replace the put step's body at `put .../execute.cppm:24` with the same shaped body as GET check 4 (reuse a shared helper reading `ctx.store().json_for("stripe_checkout_config", {})`); keep status 200. Add the cache header in check 6.
5. DB call — REPLACE operation (persist config row, not in-memory mutate). Ghost `config.edit(...)` (`controller:37-40`; `service.ts:405-446`) inserts/merges the `stripe_checkout_config` row (`slug='default'`), writes bindings, records an Action, returns `changedRequirements`. C++ calls `stripe.configure(body)` (in-memory only, `stripe.cppm:25-36`) and the `mutations.edit("stripe", ...)` write (`put .../execute.cppm:24`) is unreachable because the lambda returns 200 (`:19,23`). Remove `stripe.configure(body)` from this path. Exact replacement: `ctx.store().edit("stripe_checkout_config", "default", body_json)` (resource string `"stripe_checkout_config"`, id `CONFIG_SLUG`=`"default"` from `schema.ts:8`; `ApiDataStore::edit` at `context.cppm:1279-1284`), falling back to `ctx.store().create("stripe_checkout_config", body_json)` (`context.cppm:1206,1272`) when `edit` returns false. `ResourceStore::edit/create` are context-local (`context.cppm:640-672`); no SQL, no new globals. Record the staff Action via the existing `actions` resource (`context.cppm:120`) with `ctx.store().create("actions", ...)` when actor is non-null (`actions.ts:56-97`); compute `changedRequirements` by comparing `json_for("stripe_checkout_config", {})` before/after (no bindings table in C++ — see ambiguity).
6. Cache — REPLACE. Ghost `cacheInvalidate:false` default (`controller:31`) but explicit `X-Cache-Invalidate: /*` on `changedRequirements` (`controller:41-43`). C++ none. In `stripe_checkout_config_put/handler.cppm` before `send()` (`:29`), add `res.set_header("X-Cache-Invalidate","/*")` when the step reports a change. Keep step `Output=std::map<std::string,std::string>` and extend the payload with `{"changed","1"}` (type unchanged, `put .../execute.cppm:24`); read it at `put handler.cppm:25-27` and set the header (`set_header` at `include/interfaces/io/response.cppm:590`). `ResourceStore` writes already refresh `resources`-tagged keys with `Ttl{}.never()` (`context.cppm:645-646,660-661`) — no new namespace or TTL.
7. Auth — same gap as GET check 7 but method `'edit'` (`controller:32`; `routes.js:154`) plus staff-only `actingContext` (`actions.ts:25-31`). Keep bearer `token(req).empty()` → 401 (`put handler.cppm:18-22`) via `require_auth` (`middleware/auth.cppm:12-29`), then add `require_permission(req,res,next,send)` in allowed module `blog_handlers_middleware` keyed `docName="products"`, action `"edit"`, wired in `StripeRoutes::install` PUT lambda (`routes.cppm:44-50`) ahead of `handler.stripe_stripe_checkout_config_put().handle(...)`. Absence proof: grep `require_permission|permission_middleware|docName` over `include/blog/handlers/**/*.cppm` → no matches.
8. Verdict + evidence — after 3,4,5,6,7 → `MATCH`. Recheck `routes.js:154`, `controller:9,30-46`, `serializers.ts:22-34,82-88`, `service.ts:405-446`, `actions.ts:25-31,56-97` vs `routes.cppm:19,22,35,44-51`, `put/handler.cppm:17-30`, `put/.../execute.cppm:11-25`, `stripe.cppm:25-36`.

Ambiguity — DTO decode + action record + requirement diff. Short snippet for checks 3/5 (add `import core_serde;`):

```cpp
// stripe_checkout_config_put/workflow/step/execute.cppm
const auto body = in.body_as_string();
const auto env = core_serde::JsonCodec{}.decode<core_serde::Value>(body); // parse checkout_config[0]
// reject shipping/phone when flag off (default in C++), and unknown keys (strictObject)
const std::string row_json = /* validated design/shipping/phone keys + "slug":"default" */;
auto& store = ctx.store();
const bool changed = store.json_for("stripe_checkout_config", {}) != /* projected-after JSON */;
if (!store.edit("stripe_checkout_config", "default", row_json)) {
    store.create("stripe_checkout_config", row_json);  // insert on first save (onConflict merge)
}
if (actor) store.create("actions", R"({"event":"edited","resource_type":"stripe_checkout_config"})");
return core::workflow::StepResponse<Output>{{{"status","200"},{"changed", changed ? "1" : "0"},
                                              {"body", store.json_for("stripe_checkout_config", {})}}};
```

`core_serde::Value` is the codec's DOM type (core_serde, allowed); no third-party libs. The bindings join and `requirements().browse` diff (`service.ts:357-361,377,419,444`) have no C++ table equivalent — approximate `changedRequirements` by comparing the stored design/parts before and after; document this as the residual fidelity gap rather than adding SQL.

## Missing in C++

None. Both Ghost routes in this controller have C++ route/handler counterparts (confirmed by exact path+verb match in `include/blog/handlers/external/stripe/routes.cppm:35-51`; no Ghost `read`/`edit` route lacks a C++ handler).

#### Fix

No change required. Both Ghost routes have C++ counterparts; `MISSING` count stays 0. The route-detail Fix blocks above close checks 1–8 for each route.

## Extra in C++

None in the matched folder(s). The shared Stripe router also installs GET `/stripe/checkout/branding` (routes.cppm:24-25) and POST `/stripe/checkout/preview` (routes.cppm:54-55); these are served by sibling controllers `stripe-checkout-branding.ts` and `stripe-checkout-preview.ts` (Ghost routes.js:155-156) and are therefore not counted as EXTRA for this controller.

#### Fix

No change required. `EXTRA` count stays 0; no C++-only route to remove or map. The branding/preview routes belong to their own controllers, not this one.
