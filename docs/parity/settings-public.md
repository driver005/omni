# Parity: settings-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/settings-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++: `include/blog/handlers/system/settings/routes.cppm` (unified router; also serves admin `settings.js`). Browse subtree: `browse/handler.cppm`, `browse/workflow/flow/main.cppm`, `browse/workflow/step/execute.cppm`. Installed via `include/blog/handlers/routes.cppm:43,116`.
- Checked: 2026-11-19

> Premise check: the audit brief anticipates "no C++ counterpart / all MISSING" for `*-public` domains. Disproven by search — a C++ GET route exists at `/settings` bound to `handler.settings_browse()`. Ghost installs one flat `ApiRoutes` router with no content/admin base split (`include/blog/handlers/routes.cppm:67-130`), so this path is shared between the content and admin controllers rather than being a distinct content endpoint. Verdict is therefore `MISMATCH`, not `MISSING`: route/verb agree, but the C++ step implements the **admin** settings-browse semantics (list of all settings + fabricated pagination), not the public flat-config semantics.
>
> `api.publicSettings` resolves to this controller: `docs/Ghost/ghost/core/server/api/endpoints/index.js:307-308` (`pipeline(require('./settings-public'), localUtils, 'content')`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/settings` | `publicSettings.browse` (docName `settings`) | GET `/settings` (`Router{router,"/settings"}.get`) | `handler.settings_browse()` → `blog::handlers::settings::browse::BrowseHandler` → `settings_browse_execute` | Ghost: no `options`/`data`/`validation` declared — `query()` takes no params; `permissions:true`, `headers.cacheInvalidate:false`. C++: `Input=IRequest&`, **no query/path parsing**; `json_for("settings", {})` with empty params. Param names: none on either side (agrees). | Ghost: **flat object** `Object.assign({}, settingsCache.getPublic(), {url, version, labs})` — public settings keys + `url`, `version`, `labs`; no settings list, no `meta.pagination`. C++: **`{"settings": <rows>, "meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}`**, `Status::OK` — admin-style list envelope with hardcoded pagination. Materially different data shape. | Ghost: in-memory `settingsCache.getPublic()` (public keys only) + `urlUtils.urlFor('home', true)` + `ghostVersion.safe` + `labs.getAll()`; **no model/DB call**. C++: `ctx.store().json_for("settings", {})` — reads the full `"settings"` DB store (all keys, including private), no filtering. Materially different resource. | Ghost: `headers.cacheInvalidate:false` (invalidations suppressed; no read-cache configured in this controller). C++: none — no `core::cache::Cache` use; global `m_cache` is wired only to members/emails/automations. | Ghost: `mw.authenticatePublic` (Content API key) + `permissions:true`. C++: `content_type_middleware` only; token guard dead (`token(req).empty() && !true`) → auth effectively absent. | MISMATCH | Ghost: `settings-public.js:8,10-24`, `content/routes.js:37`, `endpoints/index.js:307-308`. C++: `system/settings/routes.cppm:19-28`, `browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:11-17`, `browse/workflow/flow/main.cppm:9`, `all.cppm:366,615,836-839`. |

## Route details

### GET `/settings` — MISMATCH

- **Route identity — MATCH**: Ghost `router.get('/settings', mw.authenticatePublic, http(api.publicSettings.browse))` (`content/routes.js:37`) vs C++ `Router{router, "/settings"}...get(...)` (`system/settings/routes.cppm:19,21`). Path and verb agree verbatim.
- **Operation mapping — name MATCH, semantics MISMATCH**: Ghost op `browse` (`settings-public.js:10`) exposed as `publicSettings.browse` (`endpoints/index.js:307-308`); C++ binds `handler.settings_browse().handle(...)` (`system/settings/routes.cppm:26`), registered in `all.cppm:366,615`. Names agree, but Ghost `query()` returns public config, while C++ `settings_browse_execute` performs the admin settings-list browse.
- **Input parity — MATCH (both effectively none, with caveats)**: Ghost declares no `options`, `data` or `validation`; `query()` reads only module singletons (`settings-public.js:15-23`). C++ `ExecuteStep::Input = interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:11`) parses no query string and passes `{}` to `json_for` (`:16`). No param name exists on one side only.
- **Output parity — MISMATCH (materially different)**: Ghost returns a **flat public object** — `Object.assign({}, settingsCache.getPublic(), {url: urlUtils.urlFor('home', true), version: ghostVersion.safe, labs: labs.getAll()})` (`settings-public.js:18-22`), with no `settings` array and no pagination meta; `headers.cacheInvalidate:false` (`:11-13`). C++ builds `{"settings": <rows>, "meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` with all pagination values hardcoded and `Status::OK` (`browse/workflow/step/execute.cppm:17`, `browse/handler.cppm:27`). Different data shape, fabricated pagination, and an extra `settings` list wrapper.
- **DB call parity — MISMATCH (materially different)**: Ghost never touches a model — it reads `settingsCache.getPublic()` (in-memory cache restricted to public keys, per `docs/Ghost/ghost/core/shared/settings-cache/cache-manager.js:240-250`) plus three singleton getters (`settings-public.js:18-21`). C++ calls `ctx.store().json_for("settings", {})` (`browse/workflow/step/execute.cppm:16`) — a direct read of the full `"settings"` resource including private settings, with no filter/pagination. Resource/mechanism differ and the C++ response risks exposing non-public settings.
- **Cache parity — MISMATCH (divergent)**: Ghost sets `headers.cacheInvalidate:false` (`settings-public.js:11-13`) — no invalidation header, no read cache configured for this controller. C++ step has no `core::cache::Cache` read, key, namespace, TTL or eviction (`browse/workflow/step/execute.cppm:13-18`); the shared `m_cache` (`all.cppm:836-846`) is wired only to `members`/`emails`/`automations` operations (`all.cppm:837-839`). Effectively "no cache" on both sides; Ghost additionally suppresses invalidation.
- **Auth / permissions — MISMATCH (divergent)**: Ghost uses `mw.authenticatePublic` (`content/routes.js:37`, Content API key) and `permissions:true` (`settings-public.js:14`). C++ installs only `content_type_middleware` (`system/settings/routes.cppm:20`); the token guard in `BrowseHandler` is disabled by `&& !true` (`browse/handler.cppm:18`), so no bearer/authentication is enforced.
- Overall **MISMATCH**: path, verb and `browse` name align, but the C++ step returns the admin settings-list shape with fabricated pagination and reads the entire settings store, whereas the Ghost public endpoint returns a flat public-config object built from an in-memory public-only cache plus `url`/`version`/`labs`. Data shape and resource are materially different.

#### Fix

DOCUMENTATION ONLY — do not apply. Exact edits to close the MISMATCH on `GET /settings`
(Ghost `settings-public.js:8-24`, `content/routes.js:37`; C++ `system/settings/routes.cppm:19-28`,
`browse/workflow/step/execute.cppm:11-17`, `browse/handler.cppm:17-28`):

- **Check 1 (route identity) — closed, no change required.** `include/blog/handlers/system/settings/routes.cppm:19,21` already binds `Router{router, "/settings"}...get(...)` → `GET /settings`, exact path and verb of `content/routes.js:37`. Keep.
- **Check 2 (operation mapping) — fix (semantic, resolved by checks 4/5/7).** `routes.cppm:26` binds `handler.settings_browse().handle` (getter `all.cppm:366`, member `all.cppm:615`); Ghost key `publicSettings.browse` (`settings-public.js:10`, `endpoints/index.js:307-308`). Names agree; the handler currently runs **admin** browse semantics. Reshaping the step to public-config output (below) closes the semantic half — no rename of `settings_browse()` needed.
- **Check 3 (input parity) — closed, no change required.** Ghost `query()` takes no params and declares no `options`/`data`/`validation` (`settings-public.js:15-23`); C++ `using Input = interfaces::io::IRequest&` (`execute.cppm:11`) parses no query string/body and passes `{}`. No param name exists on one side only.
- **Check 4 (output parity) — fix.** `browse/workflow/step/execute.cppm:17` emits `{"settings": <rows>, "meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` — admin list envelope + fabricated pagination. Replace with a **flat public object**: `Object.assign({}, settingsCache.getPublic(), {url, version, labs})` (`settings-public.js:18-22`). Exact replacement call: `return core::workflow::StepResponse<Output>{ctx.store().public_settings_json()};` where `public_settings_json()` is a new `ApiDataStore` method (see check 5 snippet) returning the flat object. Keep `res.set_status(Status::OK)` (`handler.cppm:27`) — Ghost default 200; no `meta.pagination`.
- **Check 5 (DB call parity) — fix + caveat.** Ghost makes **no model/DB call** — it reads `settingsCache.getPublic()` (public keys only, `docs/Ghost/ghost/core/shared/settings-cache/cache-manager.js:240-250`, allowlist `.../settings-cache/public.js:5-68`) plus `urlUtils.urlFor('home',true)`, `ghostVersion.safe`, `labs.getAll()` (`settings-public.js:18-21`). C++ `ctx.store().json_for("settings", {})` (`execute.cppm:16`, branch `context.cppm:1132` → `settings_json()` `context.cppm:1653-1687`) returns the full `{"key","value"}` settings array. Fix: add `public_settings_json()` to `ApiDataStore` (`blog_context`, `context.cppm`, next to `settings_json()`) that filters `settings_json()` through the public allowlist and appends `url`/`version`/`labs`; the step calls that instead. This narrows the exposed resource to public keys only — no direct SQL, no new global state, cross-request state (if any) only via the context-local `ResourceStore` under the existing `"resources"` tag (`context.cppm:635-646,704-709`). See snippet below.
- **Check 6 (cache parity) — closed, no change required.** Ghost `headers.cacheInvalidate:false` (`settings-public.js:11-13`) emits no `X-Cache-Invalidate`; C++ step/handler have no `core::cache::Cache` key, namespace, TTL or eviction (`all.cppm:836-839` wires `m_cache` only to members/emails/automations), so StepResponse sets no invalidation header. Both sides: effectively no cache/invalidation. Do not add a cache primitive.
- **Check 7 (auth) — fix.** Ghost uses `mw.authenticatePublic` (Content API key) on the route (`content/routes.js:37`) plus `permissions:true` (`settings-public.js:14`). C++ installs only `content_type_middleware` (`routes.cppm:20`) and the token guard in `BrowseHandler` is dead — `if (token(req).empty() && !true)` (`handler.cppm:18`). Exact replacement: remove `&& !true` so the 401 `{"error":"Unauthorized"}` branch (`handler.cppm:19-21`) fires — `if (blog::handlers::middleware::token(req).empty())` (`include/blog/handlers/middleware/bearer.cppm:27`). Optionally add `.use(blog::handlers::middleware::require_auth)` (`middleware/auth.cppm:12-29`) after `routes.cppm:20` to mirror AuthenticatePublic; caveat: that router is shared with admin settings routes, so prefer enabling the handler guard to avoid collateral. `permissions:true`/`docName:'settings'` has no allowed C++ primitive — accepted residual gap, no new global state.
- **Check 8 (verdict) — closed.** After fixes 2/4/5/7, path, verb, `browse` name, empty input, flat public-config output, public-only resource, no cache, and token auth agree → MATCH (permissions/docName residual only). No MISSING/EXTRA introduced.

Short snippets where the exact replacement is non-trivial:

New `ApiDataStore` public method (`include/blog/context/context.cppm`, add beside `settings_json()` near line 1653):

```cpp
// Allowlist mirrors docs/Ghost/ghost/core/shared/settings-cache/public.js
[[nodiscard]] std::string public_settings_json() const
{
    static constexpr std::string_view kPublic[]{
        "title","description","logo","icon","accent_color","cover_image",
        "facebook","twitter","threads","bluesky","mastodon","tiktok","youtube",
        "instagram","linkedin","locale","timezone","codeinjection_head",
        "codeinjection_foot","navigation","secondary_navigation","meta_title",
        "meta_description","og_image","og_title","og_description","twitter_image",
        "twitter_title","twitter_description","members_support_address",
        "members_enabled","donations_enabled","allow_self_signup",
        "members_invite_only","members_signup_access","paid_members_enabled",
        "portal_name","portal_plans","comments_enabled","recommendations_enabled",
        "default_email_address","support_email_address","site_uuid","labs",
        "transistor_portal_enabled"};
    const auto rows = settings_json();              // [{"key":...,"value":...}, ...]
    std::string out = "{";
    bool first = true;
    for (const auto& record : parse_array(rows)) {  // parse_array: context.cppm:595 (same class)
        const auto kit = record.find("key"), vit = record.find("value");
        if (kit == record.end() || vit == record.end()) continue;
        if (std::ranges::find(kPublic, kit->second) == std::ranges::end(kPublic)) continue;
        out += (first ? "" : ",") + std::format(R"("{}":{})", kit->second, vit->second);
        first = false;
    }
    // Residual: no urlUtils/ghostVersion/labs primitive in the allowed C++ surface.
    out += (first ? "" : ",") +
           R"("url":"http://localhost:2368/","version":"5.0.0","labs":{})";
    return out + "}";
}
```

Step replacement (`browse/workflow/step/execute.cppm`, replace lines 14-17):

```cpp
auto& ctx = *s_ctx;
return core::workflow::StepResponse<Output>{ctx.store().public_settings_json()};
```

Handler auth guard (replace `browse/handler.cppm:18`): `if (blog::handlers::middleware::token(req).empty()) {`.

## Missing in C++

None. The single Ghost content route `GET /settings` (`content/routes.js:37`) has a C++ path+verb counterpart: `Router{router,"/settings"}.get(...)` at `system/settings/routes.cppm:19,21`.

Search performed to confirm:
- `grep -rn "/settings" include/blog/handlers/**/*.cppm` → exactly one route registration: `system/settings/routes.cppm:19`.
- `grep -rn -i "settings" include/blog/handlers/**` → only `system/settings/` and `system/custom_theme_settings/` route/handler trees; `custom_theme_settings` is mounted at `/custom_theme_settings` (`system/custom_theme_settings/routes.cppm:19`) and is not the content settings endpoint.
- `grep -rn "getPublic|publicSettings|public_settings|version|labs|urlFor" include/blog/handlers` → no matches; no dedicated public/settings C++ implementation exists.
- `index.js:307-308` confirms `publicSettings` → `settings-public.js`; `content/routes.js:37` is its only route in the content table.

  Fix: No change required. Check 8 (MISSING) is closed: the lone Ghost content route already has a
  C++ path+verb counterpart (`system/settings/routes.cppm:19,21`, `handler.settings_browse()`), confirmed
  by the greps above. Do not add a route or handler — the MISMATCH is reshaped by the `#### Fix` block
  above (checks 4/5/7), not by registering a new endpoint. No third-party libs, direct SQL, or new
  global state.

## Extra in C++

Routes in the matched folder `include/blog/handlers/system/settings/routes.cppm` with no counterpart in `settings-public.js` (they belong to the **admin** `settings.js` controller / `admin/routes.js`, not this public controller). Following the convention in `newsletters-public.md`, `pages-public.md` and `recommendations-public.md`, these are listed here but **not counted as EXTRA verdict rows** because they map to a separate admin audit and the C++ folder is shared between the admin and content controllers via the unified router:

- PUT `/settings` → `settings_bulk_edit()` (`routes.cppm:29-36`) — admin `edit`/bulk edit.
  - Fix: No change required. Ghost admin `api.settings.edit` (`admin/routes.js:111`, `settings.js:139`) owns this op; out of scope for `settings-public.js`. Keep `settings_bulk_edit()` bound in `SettingsRoutes::install` (`system/settings/routes.cppm:29-36`) and registered in `all.cppm` — do not remove or gate. Check 1 (EXTRA) closed; no source edit here.
- POST `/access_code/regenerate` → `settings_settings_access_code_regenerate_post()` (`routes.cppm:37-50`).
  - Fix: No change required. Ghost admin `api.settings.regenerateAccessCode` (`admin/routes.js:115`, `settings.js:169`) owns this op; out of scope here. Keep the `/access_code/regenerate` `.post(...)` binding (`routes.cppm:37-50`) installed. Check 1 (EXTRA) closed; no source edit here.
- GET `/routes/yaml` → `settings_settings_routes_yaml_get()` (`routes.cppm:51-62`).
  - Fix: No change required. Ghost admin `api.settings.download` (`admin/routes.js:101`, `settings.js:198`) owns this op; out of scope here. Keep the `/routes/yaml` `.get(...)` binding (`routes.cppm:51-62`) installed. Check 1 (EXTRA) closed; no source edit here.
- POST `/routes/yaml` → `settings_settings_routes_yaml_post()` (`routes.cppm:63-70`).
  - Fix: No change required. Ghost admin `api.settings.upload` (`admin/routes.js:107`, `settings.js:185`) owns this op; out of scope here. Keep the `/routes/yaml` `.post(...)` binding (`routes.cppm:63-70`) installed. Check 1 (EXTRA) closed; no source edit here.
- DELETE `/stripe/connect` → `settings_settings_stripe_connect_delete()` (`routes.cppm:73-86`; route registered via `.delt(...)`).
  - Fix: No change required. Ghost admin `api.settings.disconnectStripeConnectIntegration` (`admin/routes.js:121`, `settings.js:82`) owns this op; out of scope here. Keep the `/stripe/connect` `.delt(...)` binding (`routes.cppm:73-86`) installed. Check 1 (EXTRA) closed; no source edit here.
- PUT `/verifications` → `settings_settings_verifications_put()` (`routes.cppm:87-97`).
  - Fix: No change required. Ghost admin `api.settings.verifyKeyUpdate` (`admin/routes.js:117`, `settings.js:64`) owns this op; out of scope here. Keep the `/verifications` `.put(...)` binding (`routes.cppm:87-97`) installed. Check 1 (EXTRA) closed; no source edit here.

No other C++ route in a matched folder lacks a Ghost controller counterpart.

## Source fixes applied (manifest group 50) / central registrations needed

- `browse/handler.cppm`: dead `&& !true` token guard removed; router-level `blog::handlers::middleware::require_auth` is now installed in `system/settings/routes.cppm` for every settings route. This enforces **admin bearer auth**, not Ghost's `authenticatePublic` (Content API key) — residual for the public controller because the router is shared.
- `browse/workflow/step/execute.cppm`: query string parsed into `params`, hardcoded `meta.pagination` removed, `json_for("settings", params)` called. The step returns the admin `{"settings":...}` envelope required by `settings.md`. One shared `GET /settings` route cannot simultaneously return that and this doc's flat public-config object (check 4), so the public output was **not** applied here.
- **Central registration needed** (`include/blog/handlers/context/context_context.cppm`, shared file — not edited): add `ApiDataStore::public_settings_json()` beside `settings_json()` implementing the public allowlist + `url`/`version`/`labs`, then split the content/admin `GET /settings` route or add content negotiation so the flat object is served to public requests. Until that lands this route stays MISMATCH for the public controller.
- `permissions:true` / `docName:'settings'` has no allowed C++ primitive; accepted residual, same as the `require_permission` registration recorded in `settings.md`.

## Group 56 — strict-parity implementation record (2026-11-20)

Supersedes the group-50 note that the public output "was not applied". All edits
are confined to `include/blog/handlers/system/settings/`; no shared file, BUILD
or central registration was touched.

- **Check 4/5 (output + DB) — closed at the C++ boundary.** The browse step now
  content-negotiates on `x-blog-context: CONTENT_API`: `support::public_browse()`
  builds the flat public-config object from the Ghost public allowlist
  (`shared/settings-cache/public.js`) plus `url`, `version`, `labs` — no
  `settings` array, no pagination meta, no private keys. Admin requests keep the
  `{"settings":[...],"meta":{...}}` envelope required by `settings.md`. Both
  projections are backed by the same store-backed settings merge in the
  registered `:settings_browse_step` partition, so public values reflect edits.
- **Check 7 (auth) — closed at the contract boundary.** `support::authorize()`
  maps the Content API key context to the trusted permission principal and runs
  `core_auth:ghost` via `blog_handlers_middleware::require_ghost_permission`,
  reproducing Ghost `permissions:true`. Bearer-token presence is the router 401
  gate; no dead `&& !true` check remains.
- **Check 6 (cache) — closed.** `cacheInvalidate:false` ⇒ no
  `X-Cache-Invalidate` on the public read.
- **Verdict — stays MISMATCH.** The documented material divergence (admin list
  envelope returned for the public route) is fixed for requests that identify
  the public context via `x-blog-context: CONTENT_API`, but the shared router has
  no `authenticatePublic`/Content-API-key resolver, so a public key presented
  without that header is still served the admin projection. Closing that fully
  needs a centrally owned route split or content-negotiation decision in
  `include/blog/handlers/routes.cppm` / `context/context_context.cppm`
  (`public_settings_json()`), neither of which may be edited from this folder.
- **New-partition needs (recorded, not created):** a `blog_handlers:mapping`
  partition for the canonical public serializer, a shared public-settings
  allowlist service, and an `authenticatePublic` middleware/Content API key
  resolver. Helpers instead live in the existing registered
  `blog_handlers:settings_browse_step` partition; no BUILD/cycle fix attempted.
