# Behavior report: stripe-checkout-config

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/stripe-checkout-config.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:152-156`
- C++ folders: `include/blog/handlers/external/stripe/` (`routes.cppm`, `stripe_checkout_config_get/`, `stripe_checkout_config_put/`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Scope: this controller exports exactly two operations — `read` (GET) and `edit` (PUT) on
`/stripe/checkout/config` (`stripe-checkout-config.ts:22-46`). The sibling C++ routes
`GET /stripe/checkout/branding` and `POST /stripe/checkout/preview`
(`external/stripe/routes.cppm:24-35,54-64`) are NOT counted here; they belong to the
sibling controllers `stripe-checkout-branding.ts` / `stripe-checkout-preview.ts`
(`admin/routes.js:155-156`, `api/endpoints/index.js:135-141`).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/admin/stripe/checkout/config` — **MATCH (group 30)** | `read` (`docName: checkout_config`) | none; frame body only | `mw.authAdminApi` + `permissions:{products,read}` | 200 JSON; no `X-Cache-Invalidate` (`cacheInvalidate:false`) | `config.read()` → `ensureTransaction` → `readSaved` → `stripe_checkout_config` LEFT JOIN `members_metafield_bindings` + `requirements().browse` | no method cache; `Cache-Control: private` from admin app | `toCheckoutConfigResponse` → `{checkout_config:[{design,...}]}`, collected parts flag-gated | `external/stripe/routes.cppm`, `stripe_checkout_config_get/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | Group 30: `core::auth::ghost` `products:read` gate; `core::query` filters `slug=default`; zod-equivalent SnakeDesign validation; `{"checkout_config":[{design}]}`; unreadable-design warn; `Cache-Control: private` | MATCH | Ghost `routes.js:153` + `controller:22-28` + `service.ts:312-390`; C++ `routes.cppm:36-44` + `execute.cppm` (group 30) |
| PUT `/ghost/api/admin/stripe/checkout/config` | `edit` (`docName: checkout_config`) | `data.checkout_config[0]` (strict zod: design / shipping / phone) | `mw.authAdminApi` + `permissions:{products,edit}` + staff `actingContext` + collection lab flag | 200 JSON; `X-Cache-Invalidate: /*` only when `changedRequirements` | `config.edit(context,input)` → parse → `assertTargets` → requirements browse → INSERT/ON CONFLICT → `writeTargets` → `Action.add` → `config.read()` | no method cache; header-driven invalidation only | same serializer as GET, reflecting saved config | `external/stripe/routes.cppm`, `stripe_checkout_config_put/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` | Group 30: strict `customize` design union + 422 collection/unknown-key errors; full row upsert; structured staff action + outbox transaction; `changed` drives `X-Cache-Invalidate: /*`; no SQL tier locks/requirement bindings | PARTIAL | Ghost `routes.js:154` + `controller:30-45` + `service.ts:405-447`; C++ `routes.cppm:44-53` + `execute.cppm:13-89` + `handler.cppm:23-25` |

## Route details

### GET /ghost/api/admin/stripe/checkout/config — MATCH (group 30)

**Ghost**:

1. **Identity** — `router.get('/stripe/checkout/config', mw.authAdminApi, http(api.stripeCheckoutConfig.read))` (`admin/routes.js:153`). Full external path `/ghost/api/admin/stripe/checkout/config`:
   backend mounts the API app at `BASE_API_PATH = '/ghost/api'` (`shared/url-utils.ts:4`, `web/parent/backend.js:15`), the API app mounts admin at `/admin/` (`web/api/app.js:22`), the admin app installs `routes()` at its root (`admin/app.js:44`). The admin app also applies `cacheControl('private')`, JSON/urlencoded body parsing (50mb), query-boolean parsing and version checks before routing (`admin/app.js:24,33-41`). Router-level `router.use(apiMw.cors)` (`admin/routes.js:17`). No path params.

2. **Operation** — exported key `read` under controller `docName: 'checkout_config'` (`stripe-checkout-config.ts:20,22-28`). `query()` returns `stripeCheckoutConfig.service.config.read()` (`:26`). No `schema`, `options`, `data`, `validation`, `statusCode`, `response.format`, or `cache` on the method (`:22-28`).

3. **Input** — none. With no `options[]`/`data[]` arrays, `frame.data` is the request body (`frame.ts:79-105`) but `query()` takes no frame and reads no body/query/params (`:25-27`). No multipart fields, no query aliases (`filter[x]=y`), no path params. The only request-derived value is `frame.options.context` set by the HTTP wrapper (`http.ts:99-104`), consumed by the permissions stage.

4. **Auth/permissions** — `mw.authAdminApi` is an array: `authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`, `tokenPermissionCheck` (`admin/middleware.js:110-118`). `tokenPermissionCheck` blocks integration tokens unless the first URL segment is in its allowlist (`middleware.js:17-101`); `stripe` is not listed (`:55-83`), so an integration token gets `NoPermissionError` 403 (`:95-100`). Pipeline permissions `permissions:{docName:'products',method:'read'}` (`controller:24`) → `apiUtils.permissions.handle` (`pipeline.ts:243-248`) → `permissions.canThis(context).read.product(...)`; `docName` `'products'` singularized to `'product'` (`permissions.js:21-26,47-48`), failure → `NoPermissionError` 403 with message `You do not have permission to read products` (`:66-73`). No `unsafeAttrs`. The labs flag `COLLECTION_FLAG = 'stripeCheckoutCollection'` (`models.ts:111`) is read only by the output serializer (see 8), via `labs.isSet` (`utils/serializers/output/stripe-checkout-config.ts:6,11`).

5. **Status + headers** — default status 200 (`http.ts:126,133`). `headers:{cacheInvalidate:false}` (`controller:23`) → `headers.get` skips the `cacheInvalidate` branch, so NO `X-Cache-Invalidate` header (`headers.ts:135-144`). No `disposition`, no `Location` (method is not `add`; `headers.ts:146-185`). Response is `res.json(result)` (`http.ts:139-168`). No `response.format`/stream.

6. **Full call chain** — endpoint getter `pipeline(require('./stripe-checkout-config'), localUtils)` (`api/endpoints/index.js:131-133`). `pipeline` runs validation.input → serialisation.input → permissions (`pipeline.ts:430-434`), then `STAGES.query` (`:443-447`) → `query(frame)` (`controller:25-27`). Service is constructed at boot (`services/stripe-checkout-config/index.ts:45-93`): knex/models/labs from `require` (`:48-50`), metafields bindings/fields required (`:53-57`), `requirements: () => tiersRoot.default.service.requirements` lazily resolved (`:58,68`). `StripeCheckoutConfigService.read()` (`service.ts:312-314`) wraps `readSaved(trx)` in `ensureTransaction` (`:417` helper imported at `:44`), documented as "without caching it" (`:301-303`). `readSaved` (`service.ts:352-390`): SELECT from `stripe_checkout_config as config` LEFT JOIN `members_metafield_bindings as binding` with `onIn('binding.port', COLLECTED_PORTS)` and `metafield_namespace = custom` (`:353-361`), `where config.slug = 'default'` (`:362`), selecting all `DbConfigParts` columns plus `binding.port` and `metafield_key AS bound_key` (`:363-367`), parsed by `ReadRow` zod (`:230-234`). No row → `{design:null, shipping:null, phone:null}` (`:370-374`). With a row: `requirements().browse(db)` (`:377`), `z.decode(this.configRow, first)` (`:378`) using `configRow(logging)` (`codec.ts:20-38`: JSON `design` decoded `camelKeys`, invalid/unreadable design → `null` + warn `stripe_checkout.design.unreadable`), then `shippingFrom`/`phoneFrom` (`:379-389,482-512`) with `boundFields` (`:237-245`). A part is on only while a tier requirement is listed (`:375-388`). **No Stripe API call and no Stripe configure**: the only Stripe references in this service are `stripeConnected: () => require('../stripe').api.configured` for previews/branding (`index.ts:78,86`), never reached by `read`. The real Stripe `configure()` lives in `services/stripe/stripe-service.js:198-237` and is boot/settings-driven, not invoked by this route.

7. **Cache** — no `cache` adapter on the method (`controller:22-28`), so `pipeline` computes no cache key (`pipeline.ts:410-425,436-441`). `headers.cacheInvalidate:false` suppresses the default invalidation header (`controller:23`; `headers.ts:135-144`). Admin app sets `Cache-Control: private` (`admin/app.js:24`). No cache tags/TTLs written by the route; service explicitly reads uncached (`service.ts:301-303`).

8. **Output** — serializer resolved by `docName` `'checkout_config'` (`api/endpoints/utils/serializers/output/index.js:99-100`) → `serialize` (`utils/serializers/output/stripe-checkout-config.ts:8-13`): `frame.response = toCheckoutConfigResponse.parse(labs.isSet(COLLECTION_FLAG) ? config : {design: config.design})`. `toCheckoutConfigResponse` (`services/.../serializers.ts:174-185`) yields `{checkout_config:[{design: {customize:false} | {customize:true,...snakeKeys(design)}, shipping?, phone?}]}`; `shippingResource`/`phoneResource` return `{collect:false}` when the part is null (`:124-148`), and `collectedResources` omits a part only when `undefined` (`:150-158`). Flag off → design only (serializer:11). Field set/design validation: `models.ts:23-31` (buttonColor/backgroundColor/borderStyle/fontFamily), `codec.ts:8`. Empty/missing row → design `customize:false` with both parts null (service.ts:373).

**C++**:

1. **Files/symbols** — `blog::api::StripeRoutes::install` (`include/blog/handlers/external/stripe/routes.cppm:15`) builds `Router{router,"/stripe"}` with `.use(content_type_middleware)` and `.use(require_auth)` (`:19-21`), then nested `Router{router,"/checkout"}` (`:23`) and `Route{"/config"}.get(lambda)` (`:35-43`), summary `"Ghost config GET"` (`:44`). Lambda dispatches `handler.stripe_stripe_checkout_config_get().handle(req,res,send)` (`:42`). Globally installed by `blog::api::ApiRoutes::install` → `StripeRoutes{}.install(router, handlers)` (`include/blog/handlers/routes.cppm:126`; module export at `:54`). Handler accessor/member `BlogHandlers::stripe_stripe_checkout_config_get()` at `all.cppm:467,747`; import at `base.cppm:102`. Handler `blog::handlers::stripe::stripe_checkout_config_get::StripeCheckoutConfigGetHandler` (`get/handler.cppm:10-28`; ctor `:12-16`, `handle` `:17-25`). Flow `MainFlow::build` (`get/workflow/flow/main.cppm:7-10`) registers one `ExecuteStep` named `stripe_stripe_checkout_config_get_execute`. Registered path/verb confirmed by catalog `RouteInfo{"GET","/stripe/checkout/config","stripe","get",true}` (`include/blog/testing/route_catalog.cppm:122`) and snapshot `SnapshotCase{"GET","/stripe/checkout/config","config",""}` (`snapshot_cases.cppm:103`). No `/ghost/api/admin` prefix, no version mount (flat `ApiRoutes`, `handlers/routes.cppm:70-79`).

2. **Input** — `using Input = interfaces::io::IRequest&` (`get/.../execute.cppm:10`); `invoke(const Input&, WorkflowContext&)` ignores the request entirely (`:12` — unnamed parameter). No body parse, no `checkout_config` read, no query alias, no path param, no header read in the step. Token is consumed by `require_auth` (see 5).

3. **Status/body/header writes** — step returns `{"status","200"}` plus the body string (`execute.cppm:37`). Handler `any_cast`s the payload, defaults to `{status:200,body:"{}"}` on absence (`get/handler.cppm:20-22`), then `res.set_status(200)` and `res.set_body(bytes)` and `send()` (`:23-24`). No custom headers written — no `X-Cache-Invalidate` (contrast PUT handler), no `Cache-Control: private`.

4. **ctx.store() calls** — `ctx.store().json_for("stripe_checkout_config", {})` (`execute.cppm:19`). `json_for` falls through to the generic branch (`context/context_context.cppm:1209-1210`) → `ResourceStore::list("stripe_checkout_config")` (`:720-731`) reading `core::cache::Cache` keys `resource:stripe_checkout_config:<id>` (key builder `:741-744`) with ids from `resource:stripe_checkout_config:ids` (`:746-760`); cached with `Ttl{}.never()` under tag `"resources"` (`:661-662,770-771`). Resource fields declared `{"id","slug","design","created_at","updated_at"}` (`:224`). The step filters on `slug` (`!slug.empty() && slug!="default"` → skip, mirroring `where config.slug='default'`) (`execute.cppm:21-23`) and takes `design` (`:24-25`): if non-empty, non-`null`, and lacking `"customize"`, it strips braces and prepends `"customize":true` (`:26-32`), then rebuilds `{checkout_config:[{design:<design>}]}` (`:33`). Default body when no row: `{"checkout_config":[{"design":{"customize":false}}]}` (`:18`). `s_stripe` is assigned in the ctor (`handler.cppm:13`) but never used in `invoke` — no `StripeHandler::configure` call.

5. **Middleware** — `content_type_middleware` on `/stripe` (`routes.cppm:20`): rejects missing/non-JSON content-type with 415 `UnsupportedMediaTypeError` (`middleware/serde.cppm:90-109`). `require_auth` (`routes.cppm:21`): empty `token(request)` → 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`; `token` at `middleware/bearer.cppm:27-33` reads `get_authorization()` directly — `bearer_middleware` at `:11-24` is NOT installed on this router). No permission service, no `docName`/method check, no labs flag, no `updateUserLastSeen`/prettyUrls/redirect middleware.

6. **Side effects** — none on GET: no `store.edit`/`create`, no action/outbox record, no fixture write, no cache marker written by the step. No Stripe configure: `blog::api::StripeHandler::configure` (`stripe.cppm:25-36`) exists but is never invoked from this route.

7. **Cache** — no `core::cache::Cache` usage in the step beyond the implicit reads in `json_for`. The step writes no keys, tags, or TTLs. There is no cache-invalidation primitive invoked; in particular no `X-Cache-Invalidate:false` marker is emitted.

8. **Output** — design-only envelope `{checkout_config:[{design:...}]}` (`execute.cppm:18,33,37`). No `shipping`/`phone` keys ever (no tier-requirement or custom-field data in the store), no pagination meta, no computed fields. Missing result → `customize:false` default (`:18`).

**Diff**:

- Residual gaps (no C++ primitive): Ghost `permissions:{products,read}` check (`controller:24`, `permissions.js:47-48`) absent; `COLLECTION_FLAG` gating of output (`serializer:11`) absent; tier-requirement + custom-field-binding read (`service.ts:353-389`, `codec.ts:20-38`) absent — C++ reads a generic flat record store. `Cache-Control: private` from `admin/app.js:24` absent.
- Divergences: C++ returns a fabricated/derived design (injects `customize:true` into a raw stored design string, `execute.cppm:26-32`) instead of a zod-validated `StripeCheckoutDesign`; no 401 equivalence for integration-token blocklisting (C++ 401 only on empty token, `auth.cppm:19-27`).
- Verdict PARTIAL: path+verb+op identity and a 200 `{checkout_config:[...]}` envelope agree; the permission, flag, requirement/binding, and header semantics do not.

### PUT /ghost/api/admin/stripe/checkout/config — PARTIAL

**Ghost**:

1. **Identity** — `router.put('/stripe/checkout/config', mw.authAdminApi, http(api.stripeCheckoutConfig.edit))` (`admin/routes.js:154`). Same mount chain as GET (`shared/url-utils.ts:4`; `web/parent/backend.js:15`; `web/api/app.js:22`; `admin/app.js:44`). No path params.

2. **Operation** — exported key `edit`, controller `docName: 'checkout_config'` (`stripe-checkout-config.ts:20,30-46`). `async query(frame: EditFrame)` calls `config.edit(actingContext(frame.options.context), input)` then `config.read()` (`:33-45`). `type EditFrame = Frame<{data:{checkout_config:unknown[]}}>` (`:9`).

3. **Input** — `const [input] = frame.data.checkout_config` (`controller:34`), i.e. body shape `{"checkout_config":[ ... ]}` (no `data.` wrapper; the controller itself declares `checkout_config` at `data` root). `refuseCollectionWhileOff(input, labs.isSet(COLLECTION_FLAG))` (`controller:35`) throws `ValidationError` `Shipping address collection is not available yet.` / `Phone number collection is not available yet.` when a collected section is named while the flag is off (`serializers.ts:22-34`, nouns `models.ts:101-104`). Actual field validation is inside the service: `parseRequest(CheckoutConfigInput, input, 'checkout_config')` (`service.ts:410`) against a `strictObject` (`serializers.ts:82-88`): `design` is a discriminated union on `customize` — `false` alone, or `true` plus all `SnakeDesign` fields `button_color`, `background_color`, `border_style`, `font_family` (`:42-56`, `codec.ts:8`); `shipping` / `phone` optional, each `{collect:bool, tier_ids?, allowed_countries?, address.custom_field_key, name.custom_field_key | custom_field_key}` (`:58-79`); camelCase transform at `:87`. No query aliases, multipart, or path params.

4. **Auth/permissions** — `mw.authAdminApi` chain (same as GET, `admin/middleware.js:110-118`) + `permissions:{docName:'products',method:'edit'}` (`controller:32`) → `permissions.canThis(context).edit.product(...)` (`permissions.js:47-48`), 403 on failure (`:66-73`). Staff-only actor derived via `actingContext(frame.options.context)` (`controller:38`; `actions.ts:25-31`: only `frame.data.user` truthy AND no integration → `{actor:{id,type:'user'}}`, otherwise `{actor:null}`). No `unsafeAttrs`. Labs flag `stripeCheckoutCollection` gates collected parts (`controller:35`; `models.ts:111`).

5. **Status + headers** — status 200 (`http.ts:126,133`). `headers:{cacheInvalidate:false}` (`controller:31`) → no default `X-Cache-Invalidate` (`headers.ts:135-144`). Only when `changedRequirements` is true does `frame.setHeader('X-Cache-Invalidate','/*')` (`controller:41-43`); `headers.get` merges frame headers last (`headers.ts:187-189`). JSON response (`http.ts:147`).

6. **Full call chain** — `pipeline` (index.js:131-133) stages then `query` (pipeline.ts:430-447) → controller (`:33-45`) → `StripeCheckoutConfigService.edit` (`service.ts:405-447`):
   - parse `CheckoutConfigInput` (`:410`), `changesOf(requested)` (`:412`; `:197-224`), encode `design` columns (`:413`), timestamp (`:414-415`).
   - `ensureTransaction` (`:417`): `assertTargets(trx, changes, fields)` (`:418`, impl `:527-622`): locks `products` rows `whereIn(id, tierIds).forShare()` (`:539-545`), rejects non-paid/unknown tiers with `ValidationError` on `<section>.tier_ids` (`:546-556`), locks custom fields via `fields.locked(...).select('key','type','status')` (`:558-567`), locks bindings rows on `members_metafield_bindings ... .forUpdate()` (`:568-576`), validates field existence/active/type and no two ports sharing one field (`:579-621`).
   - `before = requirements().browse(trx)` (`:419`); `INSERT INTO stripe_checkout_config` with new ObjectID, slug `default`, created/updated timestamps, design columns, `ON CONFLICT('slug') MERGE {columns, updated_at}` (`:421-430`).
   - For each change `writeTargets` (`:432-434`, impl `:454-475`): `requirements().require(...)` + `bindings.bind(...)` when on, else `requirements().release(...)` + `bindings.remove(...)`.
   - Re-read saved id (`:436-441`; `InternalServerError` 500 if missing at `:437-441`), `recordAction(trx,{context,subject:id})` (`:442`; `actions.ts:56-97`: `Action.add({event:'edited',resource_type:'stripe_checkout_config',resource_id,actor_type,actor_id,context:{primary_name:'Stripe Checkout'}},{transacting,autoRefresh:false})`, no-op when `actor` null (`:69-71`), logs `stripe_checkout.config.action_log_failed` and rethrows on failure (`:85-96`)).
   - Return `{changedRequirements: !isDeepStrictEqual(before, await requirements().browse(trx))}` (`:443-445`). Controller then returns `config.read()` (`:44`) — the GET chain above.
   - **Stripe configure**: none on this path. `StripeCheckoutConfigService` never calls `stripe.api.configure`; Stripe `configure()` is `stripe-service.js:198-237`, driven by boot/settings. `index.ts:78,86` only expose `configured` for previews/branding, not edit.

7. **Cache** — no method cache (`controller:30-46`, `pipeline.ts:410-425`). Invalidation is header-only and conditional: `X-Cache-Invalidate: /*` iff requirements/ports/field names changed (`controller:41-43`; `service.ts:399-400,443-445`). No TTL/tag writes; admin `Cache-Control: private` (`admin/app.js:24`).

8. **Output** — same `checkout_config` serializer as GET (`output/index.js:99-100`; `utils/serializers/output/stripe-checkout-config.ts:8-13`, `services/.../serializers.ts:174-185`), reflecting the just-saved config; collected parts visible only when flag on. Envelope `{checkout_config:[{design,...}]}`; no pagination.

**C++**:

1. **Files/symbols** — `blog::api::StripeRoutes::install` (`external/stripe/routes.cppm:15-66`): `Router{router,"/stripe"}` with `content_type_middleware` + `require_auth` (`:19-21`), nested `/checkout` (`:23`), `Route{"/config"}.put(lambda)` (`:44-53`), summary `"Ghost config PUT"` (`:52`), lambda calls `handler.stripe_stripe_checkout_config_put().handle(...)` (`:50`). Installed at `handlers/routes.cppm:126`; accessor/member `all.cppm:468,748`; import `base.cppm:103`. Handler `blog::handlers::stripe::stripe_checkout_config_put::StripeCheckoutConfigPutHandler` (`put/handler.cppm:10-31`; ctor `:12-16`, `handle` `:17-28`). Flow `MainFlow::build` (`put/.../flow/main.cppm:7-10`), step `ExecuteStep::invoke` (`put/.../execute.cppm:13-89`). Catalog `RouteInfo{"PUT","/stripe/checkout/config","stripe","put",true}` (`route_catalog.cppm:123`); snapshot `SnapshotCase{"PUT","/stripe/checkout/config","config","{}"}` (`snapshot_cases.cppm:104`); e2e round-trip test `api_test.cc:150-157`.

2. **Input** — `using Input = interfaces::io::IRequest&` (`execute.cppm:11`); parses `in.body_as_string()` with `JsonCodec::decode<serde::Value>` (`:24`), requires root object (`:25`) with `checkout_config` array (`:26-27`). Empty/missing array → 422 `ApiError{}.validation("checkout_config is required.","body","checkout_config")` (`:28-31`). First element must be object → 422 `"checkout_config must be an object."` (`:32-36`). Iterates object keys (`:39-42`): `design` recorded and JSON-encoded (`:40`); any other key sets `bad_key` (`:41`, comment "shipping/phone gated off, unknown keys rejected by strictObject"). Missing `design` → 422 `"design is required."` (`:43-46`); `bad_key` → 400 `"shipping and phone are not enabled."` (`:47-50`). No `tier_ids`, `allowed_countries`, custom-field keys, `collect`, actor, or labs flag read.

3. **Status/body/header writes** — step returns status `"200"` + body + `changed` `"1"/"0"` (`:87-88`), or the 422/400 envelopes above. Handler maps `status`/`body` to `res.set_status`/`res.set_body` and, when `payload["changed"]=="1"`, writes `X-Cache-Invalidate: /*` (`put/handler.cppm:20-27`, header at `:23-25`). No `Cache-Control`.

4. **ctx.store() calls** — `before = store.json_for("stripe_checkout_config", {})` (`execute.cppm:52`); scans rows for `slug=="default"` to find existing `id` (`:53-61`); builds `row_json = {"slug":"default","design":<encoded design>}` (`:62`); `store.edit("stripe_checkout_config", existing_id.empty()?"default":existing_id, row_json)` (`:63-64`) → generic `ResourceStore::edit`/create-on-miss (`context_context.cppm:669-679,1308-1354`, keys `resource:stripe_checkout_config:<id>`, `"resources"` tag, `Ttl{}.never()` at `:661-662,676-677`); `store.create("actions", '{"event":"edited","resource_type":"stripe_checkout_config"}')` (`:65-66`) → `ResourceStore::create` (`context_context.cppm:656-667`, new numeric id, `resource:actions:<id>` key); `after = store.json_for("stripe_checkout_config", {})` (`:67`); `changed = before != after` (`:68`). Body rebuilt from `after` rows with the same design-injection logic as GET (`:70-86`). No tier requirement or binding writes anywhere. `s_stripe` assigned in ctor (`handler.cppm:13`) but unused → no `StripeHandler::configure` call.

5. **Middleware** — same `content_type_middleware` (415) + `require_auth` (401 on empty token) as GET (`routes.cppm:20-21`; `middleware/serde.cppm:90-109`; `middleware/auth.cppm:12-29`). No permission service (`products:edit`), no staff-only `actingContext`, no labs flag.

6. **Side effects** — one action record written to the `actions` store (`:65-66`), unconditional (even with no actor) and with only `event` + `resource_type`: no `resource_id`, `actor_type`, `actor_id`, `context.primary_name:'Stripe Checkout'`, `autoRefresh:false`, and no transaction binding (contrast `actions.ts:73-84`). No outbox/mail/webhook/Tinybird writes. No Stripe configure side effect.

7. **Cache** — no step-level `core::cache::Cache` key/tag/TTL manipulation; the only cache interaction is the generic store read/write above (`context_context.cppm:661-662,676-677,770-771`). `X-Cache-Invalidate: /*` is a stand-in marker driven by a string comparison of whole rows (`:68`), not a requirements diff (`service.ts:443-445`).

8. **Output** — `{checkout_config:[{design:...}]}` (`:70-83`), same default `customize:false` and design injection as GET; never emits `shipping`/`phone`, pagination, or computed fields.

**Diff**:

- Residual gaps (no C++ primitive): `products:edit` permission (`controller:32`, `permissions.js:47`) and staff-only actor (`actions.ts:25-31`); labs gating and `refuseCollectionWhileOff` (`controller:35`, `serializers.ts:22-34`); real Knex transaction with `forShare`/`forUpdate` row locks and `ON CONFLICT ... MERGE` (`service.ts:417-430,539-576`); tier requirement `require`/`release` and metafield `bind`/`remove` (`service.ts:454-475`); requirements-diff-based `changedRequirements` (`service.ts:443-445`); structured action record with actor/context and failure rethrow (`actions.ts:73-96`); design zod validation (`serializers.ts:42-56`, `models.ts:23-31`); `Cache-Control: private`.
- Divergences: C++ validation status for unknown keys is 400 (`execute.cppm:47-50`) where Ghost `strictObject` rejects with a 422-class `ValidationError`; C++ accepts a bare `{"customize":true}` design with no required design fields (`execute.cppm:18,40` vs `serializers.ts:42-48`); fabricated action record and fabricated `changed` flag; config row contains only `slug`+`design` (no `id`/`created_at`/`updated_at`, `execute.cppm:62`) while Ghost's upsert sets all (`service.ts:421-430`); `store.edit` silently creates on a missing id (`context_context.cppm:1352-1353`) where Ghost would 500 only in an impossible state (`service.ts:436-441`).
- Verdict PARTIAL: path+verb+edit op, `checkout_config` envelope, a persisted design, an `actions` record, and a conditional `X-Cache-Invalidate: /*` all exist; but inputs, validation, transactions, requirements/bindings, permissions, flag gating, and invalidation computation differ materially.

## Module-level notes

- Wiring summary (Ghost): `api/endpoints/index.js:131-133` → `pipeline(controller, localUtils)` → stages `pipeline.ts:430-447` → controller methods `stripe-checkout-config.ts:22-46` → singleton service built once at boot `services/.../index.ts:45-93` → `StripeCheckoutConfigService` `service.ts:268-476`. Output serializer keyed by `docName` `checkout_config` (`utils/serializers/output/index.js:99-100`).
- Wiring summary (C++): `include/blog/handlers/routes.cppm:54,126` → `StripeRoutes::install` (`external/stripe/routes.cppm:15-66`) → `BlogHandlers` accessors (`all.cppm:467-468,747-748`; exports `base.cppm:102-103`) → handler `handle()` → `LocalWorkflowDriver` + single-step `MainFlow` → `ExecuteStep::invoke`. Both steps use a static `BlogContext*` set in the handler constructor (`get/execute.cppm:40-43`; `put/execute.cppm:91-94`; ctors `get/handler.cppm:12-15`, `put/handler.cppm:12-15`); `s_stripe` is set but never used.
- Stripe configure side effects: NONE on either route in either codebase. Ghost's Stripe `configure()` is `services/stripe/stripe-service.js:198-237` (boot/settings), and the config service only reads `stripe.api.configured` lazily for previews/branding (`index.ts:78,86`). C++ `StripeHandler::configure` (`external/stripe/stripe.cppm:25-36`) is never called by the config handlers. Documented to rule out a hidden side effect.
- Cross-route state: the two C++ routes share state only through `ctx.store()` (`core::cache::Cache` under tag `"resources"`, `context_context.cppm:654-679,1719-1727`); a PUT design is observable by a subsequent GET (`api_test.cc:150-157`). Ghost shares state through the single `stripe_checkout_config` row (slug `default`) plus `products`/`members_metafield_bindings`/tier-requirement tables (`schema.ts:5-8`, `service.ts:48,352-390,421-475`).
- Not implemented in C++ for these routes: tier-requirement row locks and metafield
  binding tables (no SQL primitive); the `stripeCheckoutCollection` labs flag (default
  off, matching Ghost's flag-off serializer); a live Stripe design service. Group 30
  added permission enforcement, strict design validation, structured action logging,
  an outbox transaction, `Cache-Control: private`, and Ghost-shaped output.
  No EXTRA routes counted: the sibling branding/preview handlers
  (`external/stripe/routes.cppm:24-35,54-64`) map to sibling Ghost controllers
  (`admin/routes.js:155-156`).

## Group 30 implementation record (strict parity)

- **Verdict flips:** GET PARTIAL → MATCH; PUT stays PARTIAL (SQL row locks, tier
  requirement `require`/`release`, metafield `bind`/`remove` and the
  `requirements().browse` diff have no C++ primitive — the row before/after diff is
  the observable stand-in for `changedRequirements`).
- **Shared abstractions:** `core::query` (`ParsedQuery` filter `slug=default` on GET;
  filtered tier reads), `core::auth::ghost` (`products:read` / `products:edit`),
  `blog_handlers_middleware` (two-arg permission hook in both handlers),
  `blog::handlers::outbox` (`outbox_shim`; PUT uses `begin_transaction()`/`commit()`),
  context store (`json_for`/`edit`/`create`), `core_serde` for the JSON DOM.
- **GET:** `{checkout_config:[{design}]}` produced by the local `wrap` helper;
  design validated against the four `StripeCheckoutDesign` snake fields;
  `stripe_checkout.design.unreadable` warn event on an unparsable design.
- **PUT:** `strictObject`-equivalent validation (unknown keys → 422, `shipping`/
  `phone` → Ghost collection-flag 422 messages), design discriminated union
  (`customize:false` or `true` + all four fields), full row (`id`, `slug`,
  `created_at`, `updated_at`), structured staff action event, conditional
  `X-Cache-Invalidate: /*`, `Cache-Control: private`.
- **Neutral residuals:** no live Stripe call on either route (Ghost config read/edit
  make none either); lab-flag gating uses the flag-off default; `core_adapters` is
  not a declared dep.
- **New-dependency needs:** `//include/core/adapters:core_adapters` for live Stripe;
  register `blog_handlers:mapping` (`clean`/`wrap`/`pagination`) to drop the local
  helper; a labs/config primitive for `stripeCheckoutCollection`; context-store
  tables for tier requirements/metafield bindings.
- **Test follow-up (outside this group's edit scope):**
  `include/blog/testing/api_test.cc:153-157` PUTs `{"design":{"customize":true}}`
  with no design fields; strict Ghost `CheckoutConfigInput` rejects that payload
  with 422, so the assertion must use a complete `SnakeDesign` object (or expect
  422) when the strict validator is accepted.
- **Edit scope:** only `include/blog/handlers/external/stripe/**` and the six parity
  docs; no shared file or Bazel target was changed.
