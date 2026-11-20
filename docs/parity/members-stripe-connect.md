# Parity: members-stripe-connect

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/members-stripe-connect.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (no members-web route references this controller; `membersStripeConnect` export key resolves only in admin routes)
- C++: `include/blog/handlers/membership/members/routes.cppm` (route `/stripe_connect` under `/members` prefix; handler `include/blog/handlers/membership/members/members_stripe_connect_get/handler.cppm`)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /members/stripe_connect | `auth` (controller `query`; docName `members_stripe_connect`) | `GET /members/stripe_connect` (`Route{"/stripe_connect"}` under router `"/members"`) | `handler.members_members_stripe_connect_get()` → `MembersStripeConnectGetHandler::handle` | Ghost: option `mode` (`live`/`test`, default `live`), session-prop write callback. C++: `using Input = IRequest&`, no query/body parsing, no `mode`. | Ghost: `res.redirect(stripeConnectAuthURL)` (HTTP redirect, no JSON body). C++: status 200 + JSON `{"members":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}}`. | Ghost: `membersService.stripeConnect.getStripeConnectOAuthUrl(setSessionProp, mode)` (service call, no model). C++: `ctx.store().json_for("members", {})`, no real filtering. | Ghost: `headers.cacheInvalidate: false` (explicitly no invalidation). C++: none. | Ghost: `mw.authAdminApi` + `permissions: true` (docName `members_stripe_connect`, op `auth`). C++: `content_type_middleware` + in-handler bearer `token(req).empty()` → 401 `{"error":"Unauthorized"}`; no permissions/docName. | MISMATCH | Ghost `.../admin/routes.js:183`, `.../members-stripe-connect.js:5,8,10,11,15,19,25-30`; C++ `.../membership/members/routes.cppm:19-20,211-219`, `.../members_stripe_connect_get/handler.cppm:17-28`, `.../workflow/step/execute.cppm:11,16-17`, `include/blog/handlers/all.cppm:98,444,693` |

## Route details

### GET /members/stripe_connect — MISMATCH

- **Route identity — MATCH.** Ghost `router.get('/members/stripe_connect', mw.authAdminApi, http(api.membersStripeConnect.auth))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:183`). C++ `Router{router, "/members"}` (`include/blog/handlers/membership/members/routes.cppm:19`) + `Route{"/stripe_connect"}.get(...)` (`routes.cppm:211-212`) = `GET /members/stripe_connect`. Export key `membersStripeConnect` defined at `docs/Ghost/ghost/core/server/api/endpoints/index.js:119-121`.
- **Operation mapping — differs.** Ghost key `auth` implemented as controller `query(frame)` that installs an async response function (`members-stripe-connect.js:19-32`). C++ binds `handler.members_members_stripe_connect_get()` (`routes.cppm:217`) → `MembersStripeConnectGetHandler::handle` (`handler.cppm:17`), a generic members-list GET stub. Method name says "get", not "auth".
- **Input — diverges.** Ghost declares `auth.options: ['mode']` with `validation.mode.values: ['live','test']` (`members-stripe-connect.js:11,13-17`) and reads `frame.options.mode || 'live'` (`:25`). C++ declares `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) but parses no query string and no body; `mode` is absent on the C++ side. Ghost also writes session props via `setSessionProp` callback (`:22-24,27`); C++ has no session handling.
- **Output — materially different.** Ghost returns `res.redirect(stripeConnectAuthURL)` (`members-stripe-connect.js:30`) — a redirect with no JSON payload. C++ returns status `OK` (200) with the step's JSON string (`handler.cppm:25-27`), which is a members array plus hardcoded pagination `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`execute.cppm:17`). Data shape and status class do not agree.
- **DB call — materially different.** Ghost calls the service `membersService.stripeConnect.getStripeConnectOAuthUrl(...)` (`members-stripe-connect.js:1,26`) and touches no member model directly. C++ calls `ctx.store().json_for("members", {})` (`execute.cppm:16`) — reads the `members` resource with empty params and performs no real filtering/pagination (pagination values are hardcoded at `:17`).
- **Cache — both effectively none.** Ghost explicitly sets `auth.headers.cacheInvalidate: false` (`members-stripe-connect.js:8`), i.e. no cache invalidation headers. C++ registers no `core::cache::Cache` key/namespace/TTL for this handler; `all.cppm` constructs the handler with only `m_ctx` (`all.cppm:693`). No diverging behavior beyond the absence of any Ghost invalidation signal.
- **Auth — intent similar, mechanism differs.** Ghost route runs `mw.authAdminApi` (`admin/routes.js:183`) and the controller sets `permissions: true` with `docName: 'members_stripe_connect'` / op `auth` (`members-stripe-connect.js:5,10`; permission registered in `docs/Ghost/ghost/core/server/services/permissions/permissions-data.ts:41,97`). C++ router applies `content_type_middleware` (`routes.cppm:20`) and the handler manually checks `blog::handlers::middleware::token(req).empty()`, returning 401 `{"error":"Unauthorized"}` (`handler.cppm:18-22`); no `permissions`/`docName` equivalent and no admin-role check beyond token presence.
- **Registry wiring (evidence).** Handler imported/registered/exposed at `include/blog/handlers/all.cppm:98,444,693`; workflow `MainFlow` built at `include/blog/handlers/membership/members/members_stripe_connect_get/workflow/flow/main.cppm:9`.

#### Fix

- Check 1 (route identity): no change required. Ghost `GET /members/stripe_connect` (`admin/routes.js:183`) already equals C++ `Router{router, "/members"}` (`members/routes.cppm:19`) + `Route{"/stripe_connect"}.get(...)` (`routes.cppm:211-219`). Path and verb are closed.
- Check 2 (operation mapping): keep the binding `handler.members_members_stripe_connect_get()` (`routes.cppm:217`, registry `all.cppm:98,444,693`) and reimplement its target symbol `blog::handlers::members::members_stripe_connect_get::MembersStripeConnectGetHandler::handle` (`members_stripe_connect_get/handler.cppm:17-28`) to perform the Ghost `auth`/`query` behavior (`members-stripe-connect.js:19-32`) — a redirect — instead of the generic members-list read. Optional cosmetic rename to `members_members_stripe_connect_auth()` would touch `all.cppm:444,693`, `base.cppm`, `routes.cppm:217` but changes no data; skip unless strict op-name parity is wanted.
- Check 3 (input): edit `include/blog/handlers/membership/members/members_stripe_connect_get/workflow/step/execute.cppm`, `ExecuteStep::invoke`. Replace `using Output=std::string` (`line 11`) with `using Output=std::map<std::string,std::string>` — exact type/pattern at `admin/posts/...` sibling `members_id_commenting_enable_post/workflow/step/execute.cppm:12,17`. Parse `mode` from `in.get_path()` using the split-on-`'?'`/`'&'`/`'='` loop at `include/blog/handlers/admin/posts/browse/workflow/step/execute.cppm:21-30`; default `"live"`; reject anything outside `{"live","test"}` with `ApiError{}.validation("Invalid mode.", "options.mode", "mode")` (`include/blog/context/context.cppm:76-84`) returned as `{"status","422"},{"body",...}`. Ghost's `setSessionProp(prop,val)` callback (`members-stripe-connect.js:22-24,27`) has no C++ equivalent in the request object; replace it with the cache write in check 5 (no Ghost JS, no new global state).
- Check 4 (output): delete the hardcoded `{"members":[...],"meta":{"pagination":{...}}}` return (`execute.cppm:16-17`). In `MembersStripeConnectGetHandler::handle` replace `res.set_status(interfaces::io::types::Status::OK)` + JSON body (`handler.cppm:25-27`) with the Ghost `res.redirect(url)` (`members-stripe-connect.js:30`) equivalent: `res.set_status(interfaces::io::types::Status::FOUND)` (`include/interfaces/io/types.cppm:30`) + `res.set_header("Location", auth_url)` (`include/interfaces/io/response.cppm:133-137,589-593`), empty body; on validation failure emit the 422 body instead. Build `auth_url` with `std::format` (style `include/blog/handlers/stripe.cppm:17-22`) as `https://connect.stripe.com/oauth/authorize?response_type=code&scope=read_write&client_id=<id>&redirect_uri=https://stripe.ghost.org&state=<state>`; `<id>` is `ca_8LBuZWhYshxF0A55KgCXu8PRTquCKC5x` for `live` and `ca_8LBum4Ctv3mmJ1oD0ZRmxjdAhNrrBUy3` for `test` (`docs/Ghost/ghost/core/server/services/members/stripe-connect.js:15-16,40-47`). Keep `core::workflow::StepResponse<Output>`; only the Output type changes.
- Check 5 (DB call / service): remove `ctx.store().json_for("members", {})` (`execute.cppm:16`) — Ghost touches no member model (`members-stripe-connect.js:1,26`). Replace the session-prop write with a `core::cache::Cache` write under the **existing** `members:` namespace via `blog::api::members::MembersOperations`: add `stripe_connect_state_set(std::string_view state)` to `include/blog/handlers/membership/members/operations.cppm`, using key prefix `members:stripe-connect:` and tag `{"members"}`, `Ttl{}.never()` — exact pattern at `operations.cppm:269-271,319-326`. Pass `blog::api::members::MembersOperations&` into `MembersStripeConnectGetHandler` (`handler.cppm:12`) and set `ExecuteStep::s_ops` (pattern `members_id_commenting_enable_post/handler.cppm:13-14` and `.../step/execute.cppm:21-23`); change `all.cppm:693` to `m_members_members_stripe_connect_get{m_ctx, m_members_ops}` (`m_members_ops` already constructed at `all.cppm:837`). Generate `state` locally per request (`std::random_device` → 16-byte hex, base64 of `{"mode":...,"randomState":...}`, mirroring `stripe-connect.js:30-36`); no new global state.
- Check 6 (cache): add `res.set_header("X-Cache-Invalidate", "false")` before `send()` in `handler.cppm` to mirror Ghost `headers.cacheInvalidate:false` (`members-stripe-connect.js:8`; pattern `docs/parity/labels.md:47`). The `core::cache::Cache` added in check 5 is cross-request session state under the existing `members:` tag — not invalidation; no new namespace, TTL class, or eviction rule.
- Check 7 (auth): on `Route{"/stripe_connect"}` (`routes.cppm:211-219`) add `.use(blog::handlers::middleware::require_auth)` (`include/blog/handlers/middleware/auth.cppm:12-29`) alongside the router-level `content_type_middleware` (`routes.cppm:20`); delete the inline `token(req).empty()` 401 `{"error":"Unauthorized"}` block (`handler.cppm:18-22`) so the 401 uses the Ghost error envelope. Ghost `permissions:true` / `docName:'members_stripe_connect'` (`members-stripe-connect.js:5,10`) has no allowed C++ primitive — accepted residual gap; do not invent new global state.
- Check 8 (verdict/evidence): after checks 1–7, route identity, redirect behavior, `mode` input, 302+Location output, `members:` cache state, and token auth are closed; residual gaps are the permission-docName model and a real Stripe network call, neither available in the allowed module set. Keep verdict `MISMATCH` until the code changes land.

Short C++ snippet for the ambiguous steps (3–5):

```cpp
// members_stripe_connect_get/workflow/step/execute.cppm — Output = std::map<std::string,std::string>
std::string mode = "live";
const auto q = std::string{in.get_path()}.find('?');
if (q != std::string::npos) {
    for (auto pair : std::string_view{std::string{in.get_path()}}.substr(q + 1) | std::views::split('&')) {
        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
        const auto eq = text.find('=');
        if (eq != std::string::npos && text.substr(0, eq) == "mode") mode = text.substr(eq + 1);
    }
}
if (mode != "live" && mode != "test")
    return core::workflow::StepResponse<Output>{{{"status","422"},
        {"body", ApiError{}.validation("Invalid mode.", "options.mode", "mode")}}};
const auto state = make_stripe_connect_state(mode);   // local random_device hex + base64
s_ops->stripe_connect_state_set(state);                // core::cache::Cache, tag {"members"}
const auto client_id = mode == "live" ? "ca_8LBuZWhYshxF0A55KgCXu8PRTquCKC5x"
                                      : "ca_8LBum4Ctv3mmJ1oD0ZRmxjdAhNrrBUy3";
const auto url = std::format(
    "https://connect.stripe.com/oauth/authorize?response_type=code&scope=read_write"
    "&client_id={}&redirect_uri=https://stripe.ghost.org&state={}", client_id, state);
return core::workflow::StepResponse<Output>{{{"status","302"},{"location",url},{"body",""}}};

// members_stripe_connect_get/handler.cppm — success branch replaces handler.cppm:25-27
res.set_status(interfaces::io::types::Status::FOUND);
res.set_header("Location", payload.at("location"));
res.set_header("X-Cache-Invalidate", "false");
```

## Missing in C++

None. The single Ghost route `GET /members/stripe_connect` has a C++ route/handler counterpart. (This is an admin-API domain, not a public/content domain, so no entirely-MISSING expectation applies.)

  Fix: No change required. Check 1 is closed for the only route: no Ghost route
  lacks a C++ registration. The `#### Fix` block above reshapes the existing
  `MembersStripeConnectGetHandler` / `ExecuteStep` / `Route{"/stripe_connect"}`
  behavior; it does not add or remove a route.

## Extra in C++

None for this controller. `include/blog/handlers/membership/members/routes.cppm` is a shared route table: its other routes (browse/create/read/edit/destroy, `/stats`, `/mrr`, `/upload`, etc.) map to the separate `members` Ghost controller, not to `members-stripe-connect`, so they are not counted as EXTRA here. The only route bound to a `membersStripeConnect`/stripe_connect controller operation is `Route{"/stripe_connect"}` (`routes.cppm:211-219`), which is accounted for above.

  Fix: No change required. No C++ route is extra for this controller; nothing to
  remove or gate. Keep the single `GET /members/stripe_connect` route — the fixes
  above change its middleware/input/output behavior, not its path or verb.


## Group 36 implementation status (2026-11-19)

Source fixes applied (all within `include/blog/handlers/membership/members`):
- `members_stripe_connect_get/workflow/step/execute.cppm`: `Output = std::map<std::string,std::string>`; parses `mode` from the query (default `live`, 422 `Invalid mode.` via `blog::api::ApiError{}`), generates a per-request random hex `state`, builds the `https://connect.stripe.com/oauth/authorize?...client_id=<id>&state=<state>` URL with the Ghost live/test client ids, and returns `{status:302, location:url, body:""}`. Removed `json_for("members", {})` and the hardcoded pagination envelope.
- `members_stripe_connect_get/handler.cppm`: sets `Status::FOUND` (302) + `Location` header on success, propagates the 422 body otherwise, and sets `X-Cache-Invalidate: false`.
- `operations.cppm`: added `stripe_connect_state_set(std::string_view state)` persisting under the existing `members:stripe-connect:` namespace (`kStripe`, tag `{"members"}`, `Ttl{}.never()`).

Central registrations needed:
- `include/blog/handlers/all.cppm`: change `m_members_members_stripe_connect_get{m_ctx}` to `m_members_members_stripe_connect_get{m_ctx, m_members_ops}`, extend the handler ctor to `(BlogContext&, MembersOperations&)`, and set `ExecuteStep::s_ops` so the step can call `stripe_connect_state_set(state)`. Currently the step generates state locally and does not persist it (no ops available in the step).
- Route-level `.use(require_auth)` on `Route{"/stripe_connect"}` is blocked by the router middleware-size limit; inline 401 retained.

Residuals: no real Stripe network call; Ghost `permissions:true` / `docName:'members_stripe_connect'` has no allowed primitive.


## Group 43 implementation record (strict parity, 2026-11-20)

Verdicts above are superseded by the strict-parity pass; see the authority
implementation record at the end of `docs/parity/behavior/members-stripe-connect.md` for route-by-route
evidence. Rewrites closed the observable/wire divergences using the registered
shared abstractions (`core_query`, `core_auth:ghost`, `blog_handlers:outbox_shim`,
`core_serde`), kept helpers in the existing `:members_operations` partition, and
used `Output = std::string` / `std::map` with static `s_ctx`. Only files inside
`include/blog/handlers/membership/members/` and the parity docs changed; no
shared file, BUILD edit or Bazel cycle fix was attempted. Real DB/Stripe/SMTP/
session machinery and non-Administrator permission fidelity remain documented
residuals; new-partition needs (member mapper, real session/permission
middleware, Stripe/SMTP adapters, event-sourced stats, multipart adapter) are
recorded in the behavior reports.
