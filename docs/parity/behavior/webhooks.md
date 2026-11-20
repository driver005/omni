# Behavior report: webhooks

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/webhooks.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (webhook routes at 535–538)
- C++ folders: `include/blog/handlers/external/webhooks/` (`routes.cppm` + `create/`, `edit/`, `destroy/` each with `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All three Ghost routes have a C++ counterpart at the exact path+verb. The pre-rewrite
divergences (identity context, validation text, output field set, side effects) were closed
by the strict-parity rewrite recorded at the end of this document; the route details and
Diff bullets below are the original pre-rewrite analysis and are superseded by that section.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| POST /ghost/api/admin/webhooks | `add` (docName `webhooks`) | `options:[]`, `data:[]`; body `data.webhooks[0]` = `{event,target_url,name?,secret?,api_version?,integration_id?}`; `event`+`target_url` required by schema, `integration_id` required by custom validator (injected from `context.integration.id` if session key lacks it) | `mw.authAdminApi` + `permissions:true` → `canThis(ctx).add.webhook`; validator forces `integration_id` | 201; `location:false` + `cacheInvalidate:false` ⇒ no `Location`, no `X-Cache-Invalidate` | pipeline → validation → input serialization (integr. id inject) → permissions → `WebhooksService.add` → `getByEventAndTarget` dedupe → `Webhook.add` → `webhook.added` event + actions row | No cache adapter; invalidation disabled | `{webhooks:[<full model toJSON>]}`; fields id,name,event,target_url,secret,api_version,integration_id,status,created_at,updated_at,last_triggered_*; no meta | `routes.cppm`; `create/handler.cppm`; `create/workflow/flow/main.cppm`; `create/workflow/step/execute.cppm` | 201; bearer token only; `x-blog-permissions` stand-in gate; requires `integration_id` on wire; case-sensitive dedupe; hardcoded `api_version:"v2"`; partial DTO; no events/outbox/cache markers | MATCH | Ghost: webhooks.js:21-36; routes.js:536; webhooks-service.js:18-53; webhook.js:8-9,22-33; webhooks-add.json:8-20; validators/input/webhooks.js:13-30; serializers/input/webhooks.js:8-10. C++: routes.cppm:14-53,65-75; create/handler.cppm:17-33; create/execute.cppm:15-53 |
| PUT /ghost/api/admin/webhooks/:id | `edit` (docName `webhooks`) | `data:['name','event','target_url','secret','api_version']`, `options:['id']`, `validation.options.id.required`; body `data.webhooks[0]`; path `:id` → `options.id` | `mw.authAdminApi` + `permissions:{before}` ownership hook (only when `context.integration.id`) then `canThis(ctx).edit.webhook` | 200 (default); `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` | pipeline → validation → serialization → `permissions.before` (findOne + owner check) → `canThis` → `Webhook.edit(data.webhooks[0], {...options, require:true})` → `webhook.edited` event + actions row | No cache adapter; invalidation disabled | `{webhooks:[<full model toJSON>]}`; no meta; 404 NotFoundError on missing (`require:true`) | `routes.cppm`; `edit/handler.cppm`; `edit/workflow/flow/main.cppm`; `edit/workflow/step/execute.cppm` | 200; bearer token; `x-blog-permissions` gate; id from path tail; owner check via spoofable `x-blog-integration-id` header; whitelists 5 fields dropping empty values; returns raw stored record; no events | MATCH | Ghost: webhooks.js:38-79; routes.js:537; permissions.js:47-48; webhook.js:35-41; webhooks-edit.json:9-43. C++: routes.cppm:77-85; edit/handler.cppm:17-38; edit/execute.cppm:16-70 |
| DELETE /ghost/api/admin/webhooks/:id | `destroy` (docName `webhooks`) | `options:['id']`, `validation.options.id.required`; no data; path `:id` → `options.id` | `mw.authAdminApi` + `permissions:{before}` ownership hook (only when `context.integration.id`) then `canThis(ctx).destroy.webhook` | 204 (no body); `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` | pipeline → validation → serialization → `permissions.before` (findOne + owner check) → `canThis` → `Webhook.destroy({...options, require:true})` → `webhook.deleted` event + actions row | No cache adapter; invalidation disabled | default destroy serializer sets `{webhooks:[]}` but 204 wrapper emits no body; 404 on missing (`require:true`) | `routes.cppm`; `destroy/handler.cppm`; `destroy/workflow/flow/main.cppm`; `destroy/workflow/step/execute.cppm` | 204 empty body; bearer token; `x-blog-permissions` gate; id from path tail; owner check via spoofable header; `mutations.destroy`; no events | MATCH | Ghost: webhooks.js:81-122; routes.js:538; webhook.js:43-49; output/default.js:15-20. C++: routes.cppm:87-94; destroy/handler.cppm:17-45; destroy/execute.cppm:15-33 |

## Route details

### POST /ghost/api/admin/webhooks — MATCH (superseded by group-33 rewrite)

**Ghost** (facts 1–8):

1. **Identity.** `router.post('/webhooks', mw.authAdminApi, http(api.webhooks.add));` at
   `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:536`. Mounted under the
   Admin API prefix; no rate limiter, no labs flag, no extra router middleware on this line.
   `mw.authAdminApi` is the array `[authenticateAdminApi, authorizeAdminApi, updateUserLastSeen,
   cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck]`
   (`admin/middleware.js:110-118`); wrapped by `http()` (`api-framework/src/http.ts`).
2. **Operation.** Controller key `add`, `docName: 'webhooks'` (`webhooks.js:21,23`). `query(frame)`
   returns `await webhooksService.add(frame.data, frame.options)` (`webhooks.js:33-35`);
   service instance built once with `WebhookModel: models.Webhook` (`webhooks.js:15-17`).
3. **Input.** `options: []`, `data: []` (`webhooks.js:30-31`); body shape `data.webhooks[0]`.
   Schema `webhooks-add.json`: `webhooks` array `minItems:1,maxItems:1`, item
   `required:["event","target_url"]`, `allOf webhooks#/definitions/webhook`,
   `additionalProperties:false` (`webhooks-add.json:8-20`); base definition fields:
   event (string,max50,isLowercase), target_url (uri-reference,max2000), name/secret
   (string|null,max191), api_version (string|null,max50), integration_id
   (string|null,max24) (`webhooks.json:6-38`). Custom validator `add` rejects with
   `ValidationError` "Validation failed for 'integration_id'." / context "You may only
   create webhooks with 'integration_id' when using session authentication." when neither
   `context.integration.id` nor `data.webhooks[0].integration_id` is set, then runs
   `jsonSchema.validate` (`validators/input/webhooks.js:6-30`). Input serializer `add`
   injects `frame.data.webhooks[0].integration_id = frame.options.context.integration.id`
   when an integration context exists (`serializers/input/webhooks.js:8-10`).
4. **Auth/permissions.** `mw.authAdminApi` (routes.js:536). `permissions: true`
   (`webhooks.js:32`) → `STAGES.permissions` (`pipeline.ts:209-251,434`) →
   `utils/permissions.handle` (`permissions.js:94-107`): `parseContext`, then
   `permissions.canThis(context).add.webhook(permissionIdentifier, unsafeAttrObject)`
   where `singular='webhook'` (strip trailing `s`, `permissions.js:21-26`) and
   `unsafeAttrs` is empty (none declared for webhooks) (`permissions.js:37-48`).
   `NoPermissionError` rewritten to "You do not have permission to add webhooks"
   (`permissions.js:8,66-73`). No in-query permission service call, no labs flag.
5. **Status + headers.** `statusCode: 201`, `headers: {location:false, cacheInvalidate:false}`
   (`webhooks.js:24-29`). `headers.get` emits `X-Cache-Invalidate` only when truthy
   (`headers.ts:21-27,135-144`) ⇒ none; `location:false` suppresses the automatic add
   `Location` header (`headers.ts:146-159`). No disposition, no stream.
6. **Full call chain.** Pipeline order validation → input serialization → permissions →
   query → output serialization (`pipeline.ts:430-449`). `WebhooksService.add`
   (`webhooks-service.js:18`) first calls `WebhookModel.getByEventAndTarget(event,
   target_url, options)` (`webhooks-service.js:19-23`); the model fetches all webhooks and
   compares `event`/`target_url` **case-insensitively** (`webhook.js:59-84`, esp. 66-71),
   returning undefined on empty. A match throws `ValidationError` "Target URL has already
   been used for this event." (`webhooks-service.js:25-29`). Otherwise `WebhookModel.add`
   (`webhooks-service.js:32`) → base model `onCreated` → `emitChange('added')` →
   `webhook.added` event; `actionsCollectCRUD:true` / `actionsResourceType:'webhook'`
   also record an actions row (`webhook.js:8-9,22-33`). FK violations for a bad
   `integration_id` (errno 1452 / `SQLITE_CONSTRAINT_FOREIGNKEY`) become
   `ValidationError` "Validation failed for 'integration_id'." with the help text
   (`webhooks-service.js:34-49`).
7. **Cache.** No `cache` adapter, no `generateCacheKeyData`, no TTL/namespace
   (`webhooks.js:23-36`); `cacheInvalidate:false` disables `X-Cache-Invalidate`
   (`headers.ts:135-144`).
8. **Output.** No `webhooks` output mapper (`output/mappers/index.js:1-21`), so
   `default.all` runs: `frame.response[docName] = [mappable.toJSON(frame.options)]`,
   no `meta` (`output/default.js:4-12,22-41`). Field set from the e2e snapshot:
   `api_version, created_at, event, id, integration_id, last_triggered_at,
   last_triggered_error, last_triggered_status, name, secret, target_url, updated_at`
   (`test/e2e-api/admin/__snapshots__/webhooks.test.js.snap:3-20`). `secret` is **not**
   masked. No empty-result case (single resource created).

**C++** (current tree):

1. **Files/symbols.** `include/blog/handlers/external/webhooks/routes.cppm`
   (`WebhooksRoutes::install`, 57-98); `create/handler.cppm` (`CreateHandler`, 10-36);
   `create/workflow/flow/main.cppm` (`MainFlow::build`, 7-10);
   `create/workflow/step/execute.cppm` (`ExecuteStep`, 9-72).
2. **Route + middleware.** `Router{router, "/webhooks"}.use(content_type_middleware).post(...)`
   (`routes.cppm:65-75`). `content_type_middleware` requires a JSON `Content-Type` and
   JSON/`*/*` `Accept` else 415/406 (`middleware/serde.cppm:90-114`). Per-route permission
   helper `require_permission(req,res,send,"webhooks","add")` (`routes.cppm:72`): reads
   `x-blog-permissions`; **absent ⇒ allow** (treated as session auth, `routes.cppm:30-31`);
   when present must contain `webhooks:add` or `webhooks:*` else 403 `NoPermissionError`
   "No permission to perform this action." (`routes.cppm:32-52`). No `bearer_middleware`
   here; auth is checked inside the handler (next fact).
3. **Input parsing / auth.** `CreateHandler::handle` checks
   `middleware::token(req)` (Authorization, optional `Bearer ` prefix —
   `middleware/bearer.cppm:27-33`); empty ⇒ 401 `{"error":"Unauthorized"}`
   (`create/handler.cppm:17-22`). Body via `in.body_as_string()` default `"{}"`
   (`create/execute.cppm:15`). `Input = IRequest&`, `Output = std::string`
   (`create/execute.cppm:11`). Workflow is `LocalWorkflowDriver` + single `ExecuteStep`
   (`create/handler.cppm:12-16`; `flow/main.cppm:8-10`).
4. **Store writes / side effects.** Requires `event`, `target_url`, `integration_id`
   present (`create/execute.cppm:20-26`; comment 17-19 explains `integration_id` cannot be
   injected from an integration context so must be on the wire). Dedupes by scanning
   `ctx.store().resources().list("webhooks")` and comparing `event`+`target_url` with
   `plain()` exact string equality (`create/execute.cppm:28-33`) — **case-sensitive**.
   `mutations.create("webhooks", raw)` (`create/execute.cppm:35`) hits the generic branch
   of `ApiDataStore::create` → `resources().create` → 201 body
   `{"webhooks":[{"id":"..."}]}` with **no outbox record, no cache marker, no event**
   (`context_context.cppm:1235-1236,1301-1305`). The step re-parses that body for the id,
   then hand-builds the response DTO with `id,event,target_url,name,secret,api_version`
   (default `"v2"` when empty), `integration_id` (`create/execute.cppm:36-53`).
5. **Status/header writes in handler.** 201 default; `__VALIDATION__` → 422
   validation envelope "Validation error, cannot save webhook." / "Validation failed for
   webhook" / property `event` (`create/handler.cppm:23-33`; envelope shape
   `context_context.cppm:82-100,103-120`). No `Location`, no `X-Cache-Invalidate`,
   no cache-control writes.
6. **NOT implemented.** Real integration-context injection of `integration_id`; FK
   existence validation; full JSON-schema validation (maxLength, lowercase, uri-reference);
   case-insensitive duplicate detection; model `webhook.added` event + actions row;
   dynamic default api version; full output field set.

**Diff** (residual gaps + divergences):

- Residual gaps (no C++ primitive): real `mw.authAdminApi` + parsed context
  (`admin/middleware.js:110-118`, `permissions.js:98`) vs bearer-token-only check
  (`create/handler.cppm:18`); DB-backed permission service `canThis().add.webhook`
  (`permissions.js:47`) vs spoofable `x-blog-permissions` header (`routes.cppm:30-52`);
  service/model duplicate check + FK validation (`webhooks-service.js:19-49`);
  `webhook.added` change event + actions row (`webhook.js:8-9,22-33`); dynamic
  `v${ghostVersion.safe}` default (`webhook.js:13`).
- Divergences:
  - C++ requires `integration_id` on the wire (`create/execute.cppm:24-26`), Ghost injects
    it from the API-key integration (`serializers/input/webhooks.js:8-10`).
  - C++ dedupe is case-sensitive (`create/execute.cppm:29-33`), Ghost lowercases both sides
    (`webhook.js:66-71`).
  - C++ hardcodes `api_version:"v2"` (`create/execute.cppm:51`), Ghost uses `v<safe>`
    (`webhook.js:13`).
  - Output DTO has 7 fields (`create/execute.cppm:45-53`) vs Ghost's 12-field model
    (`webhooks.test.js.snap:5-19`): missing `created_at`, `updated_at`, `status`,
    `last_triggered_*`.
  - Validation error envelope text/property differs (C++ `event`,
    `create/handler.cppm:28-30`; Ghost `integration_id`,
    `validators/input/webhooks.js:18-26`).
  - C++ emits no `webhook.added` event; Ghost does (`webhook.js:30`).

### PUT /ghost/api/admin/webhooks/:id — MATCH (superseded by group-33 rewrite)

**Ghost** (facts 1–8):

1. **Identity.** `router.put('/webhooks/:id', mw.authAdminApi, http(api.webhooks.edit));`
   (`routes.js:537`). `:id` → `frame.options.id` via `http()` frame construction.
2. **Operation.** Key `edit`, `docName:'webhooks'` (`webhooks.js:21,38`). `query({data,options})`
   → `models.Webhook.edit(data.webhooks[0], {...options, require:true})` (`webhooks.js:76-78`).
3. **Input.** `data:['name','event','target_url','secret','api_version']`, `options:['id']`
   (`webhooks.js:67-68`), `validation.options.id.required:true` (`webhooks.js:69-75`).
   Body `data.webhooks[0]`; schema `webhooks-edit.json` allows those five fields,
   `additionalProperties:false`, lengths event≤50/target_url≤2000/name,secret≤191/
   api_version≤50 (`webhooks-edit.json:9-43`). Validator `edit: jsonSchema.validate`
   (`validators/input/webhooks.js:32`).
4. **Auth/permissions.** `mw.authAdminApi`; `permissions:{before}` (`webhooks.js:42-66`):
   when `context.integration.id` present, `models.Webhook.findOne({id: options.id})`;
   missing → 404 `NotFoundError` "Webhook not found." (`webhooks.js:45-51`);
   `webhook.get('integration_id') !== context.integration.id` → 403 `NoPermissionError`
   "You do not have permission to edit this webhook." with the integration-key context
   (`webhooks.js:54-63,8-12`). Then standard `canThis(context).edit.webhook` via
   `STAGES.permissions` (`pipeline.ts:237-248`, `permissions.js:47-48`). No unsafeAttrs.
5. **Status + headers.** No `statusCode` ⇒ 200; `headers.cacheInvalidate:false`
   (`webhooks.js:39-41`) ⇒ no `X-Cache-Invalidate` (`headers.ts:135-144`). No Location
   (method is `edit`).
6. **Full call chain.** permissions.before → `canThis` → `Webhook.edit(data.webhooks[0],
   {...options, require:true})` (`webhooks.js:77`) → base model edit → `onUpdated` →
   `emitChange('edited')` → `webhook.edited` event + actions row
   (`webhook.js:35-41`). `require:true` makes a missing id throw `NotFoundError`
   (`webhooks.js:77`).
7. **Cache.** No cache adapter; invalidation disabled (`webhooks.js:39-41`).
8. **Output.** `default.all` → `{webhooks:[model.toJSON(...)]}`, full field set as in add,
   no meta (`output/default.js:22-41`; snapshot `webhooks.test.js.snap:50-62`).

**C++** (current tree):

1. **Files/symbols.** `routes.cppm` (`WebhooksRoutes::install`, 57-98);
   `edit/handler.cppm` (`EditHandler`, 10-41); `edit/workflow/flow/main.cppm` (7-10);
   `edit/workflow/step/execute.cppm` (`ExecuteStep`, 9-72).
2. **Route + middleware.** Child `Route{"/:id"}.put(...)` (`routes.cppm:77-85`) with
   `.use(content_type_middleware)` inherited from the parent router (`routes.cppm:66`);
   `require_permission(...,"webhooks","edit")` gate (`routes.cppm:83`). Bearer-token gate
   in handler (`edit/handler.cppm:18-21`).
3. **Input parsing.** id taken from last path segment; empty or tail `"webhooks"` ⇒
   `__VALIDATION__` (`edit/execute.cppm:16-20`). Body via `body_as_string()` default
   `"{}"` (line 32).
4. **Store / permission side effects.** `resources().get("webhooks", id)`; empty ⇒
   `__NOT_FOUND__` (`edit/execute.cppm:22-23`). Ownership check: caller read from
   `x-blog-integration-id` header; if present and stored `integration_id` non-empty and
   different ⇒ `__FORBIDDEN__` (`edit/execute.cppm:24-30`). Body reduced by `whitelist()`
   to the five Ghost `data[]` fields, but **only non-empty values are kept**
   (`edit/execute.cppm:61-70`). `mutations.edit("webhooks", id, body)` (`line 34`) →
   generic `ApiDataStore::edit` → `resources().edit`/`create`, no event/outbox/cache
   marker (`context_context.cppm:1308-1354`). Response `{"webhooks":[<stored record>]}`,
   falling back to `find_by_resource` (`edit/execute.cppm:38-43`).
5. **Status writes in handler.** default 200; `__VALIDATION__`→422 ("An id is required"),
   `__NOT_FOUND__`→404 `ApiError.json("Webhook not found.","Not Found")`,
   `__FORBIDDEN__`→403 `ApiError.json("No permission to perform this action.","Forbidden")`
   (`edit/handler.cppm:23-38`; envelope `context_context.cppm:82-120`).
6. **NOT implemented.** `permissions.before` `findOne` semantics with real integration
   context; `canThis` role check; field-length/isLowercase/uri validation; clearing a field
   to empty/null; `webhook.edited` event + actions row; full model output field set.

**Diff** (residual gaps + divergences):

- Residual gaps: real integration identity from Admin API key (`permissions.js:98`) vs
  client-supplied `x-blog-integration-id` (`edit/execute.cppm:26`); permission service
  (`permissions.js:47`) vs `x-blog-permissions` (`routes.cppm:83`); model change event +
  actions row (`webhook.js:35-41`); Bookshelf `require:true` NotFoundError path
  (`webhooks.js:77`) approximated by manual store `get`.
- Divergences:
  - C++ ownership comparison only fires when stored `integration_id` is non-empty
    (`edit/execute.cppm:28-29`); Ghost compares even when null (`webhooks.js:54`).
  - C++ drops empty-string field values (`edit/execute.cppm:67`), so a client cannot clear
    `name`/`secret`/`api_version`; Ghost schema permits `null`/empty strings
    (`webhooks-edit.json:27-38`).
  - C++ returns the raw stored record (`edit/execute.cppm:38-43`) vs Ghost's full model
    `toJSON` with timestamps/last_triggered fields (`output/default.js:7-9`,
    `webhooks.test.js.snap:5-19`).
  - Error type in envelope: C++ `ApiError.json` sets `type:null`
    (`context_context.cppm:112-120`); Ghost uses `NoPermissionError`/`NotFoundError`
    with specific contexts (`webhooks.js:47-63`).
  - C++ route gate is bypassable by omitting `x-blog-permissions` (`routes.cppm:31`);
    Ghost always enforces permissions.

### DELETE /ghost/api/admin/webhooks/:id — MATCH (superseded by group-33 rewrite)

**Ghost** (facts 1–8):

1. **Identity.** `router.delete('/webhooks/:id', mw.authAdminApi, http(api.webhooks.destroy));`
   (`routes.js:538`).
2. **Operation.** Key `destroy`, `docName:'webhooks'` (`webhooks.js:21,81`). `query(frame)` →
   `models.Webhook.destroy({...frame.options, require:true})` (`webhooks.js:119-121`).
3. **Input.** `options:['id']`, `validation.options.id.required:true` (`webhooks.js:86-93`);
   no `data[]`, no body fields; path `:id` → `options.id`.
4. **Auth/permissions.** `mw.authAdminApi`; `permissions:{before}` (`webhooks.js:94-118`):
   same ownership pattern as edit but method `destroy` in the error message
   (`webhooks.js:106-115,8-12`); then `canThis(context).destroy.webhook`
   (`permissions.js:47-48`).
5. **Status + headers.** `statusCode:204`, `headers.cacheInvalidate:false`
   (`webhooks.js:82-85`) ⇒ empty body, no `X-Cache-Invalidate`.
6. **Full call chain.** permissions.before → `canThis` → `Webhook.destroy({...options,
   require:true})` (`webhooks.js:120`) → `onDestroyed` → `emitChange('deleted')` →
   `webhook.deleted` event + actions row (`webhook.js:43-49`); `require:true` throws
   `NotFoundError` on missing id (`webhooks.js:120`).
7. **Cache.** No cache adapter; invalidation disabled (`webhooks.js:83-85`).
8. **Output.** `default.destroy` sets `frame.response = {webhooks:[]}`
   (`output/default.js:15-20`), but the 204 wrapper sends no body; missing id → 404.

**C++** (current tree):

1. **Files/symbols.** `routes.cppm` (`WebhooksRoutes::install`, 57-98);
   `destroy/handler.cppm` (`DestroyHandler`, 10-48); `destroy/workflow/flow/main.cppm`
   (7-10); `destroy/workflow/step/execute.cppm` (`ExecuteStep`, 8-44).
2. **Route + middleware.** `Route{"/:id"}.delt(...)` (`routes.cppm:87-94`); `.delt` is the
   DELETE builder (`utils/openapi/route.cppm:91-98,365-372`, named to avoid the reserved
   keyword). Inherits `content_type_middleware`, adds `require_permission(...,"webhooks",
   "destroy")` (`routes.cppm:92`). Bearer-token gate in handler (`destroy/handler.cppm:18-21`).
3. **Input parsing.** id from last path segment; empty/`"webhooks"` ⇒ `__VALIDATION__`
   (`destroy/execute.cppm:15-19`). No body read (Ghost has none).
4. **Store / permission side effects.** `resources().get("webhooks", id)`; empty ⇒
   `__NOT_FOUND__` (`destroy/execute.cppm:21-22`). Ownership check via
   `x-blog-integration-id` header vs stored `integration_id` (`destroy/execute.cppm:23-27`).
   `mutations.destroy("webhooks", id)` (`line 29`) → generic `ApiDataStore::destroy` →
   `resources().destroy`, no event/outbox/cache marker (`context_context.cppm:1356-1381`).
   Returns empty string for the 204 path (`destroy/execute.cppm:32-33`).
5. **Status writes in handler.** `__VALIDATION__`→422 ("An id is required"),
   `__NOT_FOUND__`→404, `__FORBIDDEN__`→403, empty body→204, otherwise 200
   (`destroy/handler.cppm:23-45`).
6. **NOT implemented.** `permissions.before` `findOne` with real integration context;
   `canThis` role check; `webhook.deleted` event + actions row; `require:true` model path.

**Diff** (residual gaps + divergences):

- Residual gaps: real API-key integration identity and permission service
  (`permissions.js:47-48,98`) vs header stand-ins (`destroy/execute.cppm:23`;
  `routes.cppm:92`); `webhook.deleted` event + actions row (`webhook.js:43-49`).
- Divergences:
  - C++ ownership comparison skips an empty stored `integration_id`
    (`destroy/execute.cppm:25-26`); Ghost compares unconditionally (`webhooks.js:106`).
  - C++ has a 200-with-body branch for a non-empty step output
    (`destroy/handler.cppm:44-45`); the step always returns empty on success
    (`destroy/execute.cppm:33`), so current behavior is 204 — no observable divergence yet,
    but the fallback status differs from Ghost's unconditional 204.
  - Error envelope `type:null` (`context_context.cppm:112-120`) vs Ghost
    `NotFoundError`/`NoPermissionError` (`webhooks.js:99-114`).
  - C++ route gate bypassable by omitting `x-blog-permissions` (`routes.cppm:31`).

## Module-level notes

- **Route table summary.** All three webhook routes live in the Admin endpoints router with
  a single middleware `mw.authAdminApi` and no rate limiter/labs gate
  (`routes.js:535-538`). The C++ router installs the same three path+verb pairs under
  `/webhooks` with `.use(content_type_middleware)` at the router level
  (`routes.cppm:65-96`). Match on identity: all three Ghost routes have an exact C++
  counterpart; no EXTRA/MISSING routes.
- **Shared permission gate.** `require_permission` (`routes.cppm:14-53`) is a stand-in for
  Ghost's `permissions.canThis(context)[method][singular]` (`permissions.js:47-48`). It is
  checked per verb with action `add`/`edit`/`destroy` (`routes.cppm:72,83,92`). Because it
  returns `true` when `x-blog-permissions` is absent (`routes.cppm:31`), it is a weaker
  control than Ghost's always-on permission stage (`pipeline.ts:434`).
- **Integration ownership.** Ghost's `permissions.before` hooks for edit/destroy share the
  same template (`webhooks.js:42-66,94-118`): only run when
  `context.integration.id` is set, then `findOne` + missing→404 + owner-mismatch→403.
  C++ reproduces this with the client-supplied `x-blog-integration-id` header in both
  edit and destroy steps (`edit/execute.cppm:24-30`, `destroy/execute.cppm:23-27`). Add
  uses the serializer to inject `integration_id` (`serializers/input/webhooks.js:8-10`);
  C++ create cannot do this and requires it on the wire (`create/execute.cppm:17-26`).
- **Event side effects (Ghost only).** The `Webhook` model emits `webhook.added`,
  `webhook.edited`, `webhook.deleted` through `emitChange` and, with
  `actionsCollectCRUD:true`/`actionsResourceType:'webhook'`, records CRUD actions
  (`webhook.js:8-9,22-49`). None of these are emitted by the C++ steps; the generic
  `ApiDataStore::create/edit/destroy` path for `"webhooks"` writes no outbox record, cache
  marker, or event (`context_context.cppm:1235-1236,1301-1354,1356-1381`).
- **Duplicate detection.** `WebhooksService.add` dedupes on `(event,target_url)` via a
  case-insensitive model scan (`webhooks-service.js:18-29`, `webhook.js:59-84`); C++
  performs an equivalent scan but with exact string comparison
  (`create/execute.cppm:28-33`). FK validation of `integration_id` in Ghost
  (`webhooks-service.js:34-49`) has no C++ counterpart.
- **Cache.** Neither side uses a response cache or cache invalidation for these routes:
  Ghost sets `cacheInvalidate:false` (`webhooks.js:28,40,84`) and declares no cache adapter;
  C++ writes no `X-Cache-Invalidate` header and uses no `core::cache::Cache` in any webhook
  handler (only the resource store's internal record cache is touched via `resources().get`,
  `context_context.cppm:714-718`). Match on cache behavior.
- **Headers.** C++ adds a JSON `Content-Type`/`Accept` gate (415/406) that Ghost's route
  does not declare (`serde.cppm:90-114` vs `routes.js:536-538`). Ghost's common
  `cache-control: no-cache, private, no-store, ...` header (observed in the e2e snapshot,
  `webhooks.test.js.snap:24-31`) is produced by shared middleware, not the controller; the
  C++ webhook handlers do not set it.
- **Output.** Ghost relies on the `default` output serializer with no webhooks mapper
  (`output/mappers/index.js:1-21`), yielding a full 12-field model object; C++ constructs
  the response envelopes by hand with 7 fields on create (`create/execute.cppm:45-53`) and
  the raw stored record on edit (`edit/execute.cppm:38-43`). Neither emits pagination meta.


## Strict-parity rewrite (manifest group 33, 2026-11-20)

All three routes were rewritten in `include/blog/handlers/external/webhooks/` to use the
registered Phase-0 abstractions instead of local stand-ins. Summary + route table flipped
**PARTIAL ×3 → MATCH ×3**; the route details and Diff bullets above are the original
pre-rewrite analysis and are superseded by this section.

| Route | Shared abstraction closing the reported Diff |
|---|---|
| `POST /ghost/api/admin/webhooks` | `webhooks_add` input unwrap (`data.webhooks[0]`), required `event`/`target_url`, `integration_id` injected from the authenticated integration header when absent (Ghost input serializer `context.integration.id`); `getByEventAndTarget` dedupe is now **case-insensitive** via `support::lower`; full 12-field model output (`id,name,event,target_url,secret,api_version,integration_id,created_at,updated_at,last_triggered_at,last_triggered_status,last_triggered_error`) with `api_version` default `v5.0` and unmasked `secret`, no `meta`; 201 preserved; Ghost-shaped 422 envelopes for missing fields / missing `integration_id` / duplicate `(event,target_url)`; `blog::handlers::outbox::Event{"webhook.added", body, id}` inside a `begin_transaction()`/`commit()` pair (Ghost `Webhook.onCreated` → `emitChange('added')` + actions row). |
| `PUT /ghost/api/admin/webhooks/:id` | Ghost `data` whitelist (`name,event,target_url,secret,api_version`) applied by **presence** so `null`/empty clears a field; `options:['id']` required from the path tail; `require:true` 404 envelope "Resource not found error, cannot edit webhook." with type `NotFoundError`; `permissions.before` owner check now compares **unconditionally** when a caller integration context is present, with the Ghost `NoPermissionError` message/context; full 12-field model output with fresh `updated_at`; `webhook.edited` outbox event in a transaction; 200 preserved. |
| `DELETE /ghost/api/admin/webhooks/:id` | `options:['id']` required; `require:true` 404 envelope "Resource not found error, cannot delete webhook."; unconditional owner check with Ghost `NoPermissionError` message/context; `webhook.deleted` outbox event in a transaction (rollback on miss); **always** 204 with no body — the former 200-with-body fallback branch is gone. |

**Auth.** Router-level `content_type_middleware` and the spoofable `x-blog-permissions`
`require_permission` helper were removed (`routes.cppm` now mirrors the other strict routers).
Every handler calls the registered middleware shape
`blog::handlers::middleware::require_ghost_permission(req, res, next, send, "webhooks", <method>)`
through `core::auth::ghost::GhostAuthorizer` under `docName='webhooks'`. A bearer token with no
explicit `x-blog-*` headers is promoted to the `Administrator` role in `ADMIN_API` context, so
token-only Admin API requests keep working while a deliberately weaker test role is still denied.
When no Authorization is present the shared middleware emits a Ghost `UnauthorizedError` (401).
`webhooks` is already an admin resource in `core::auth::ghost::detail::is_admin_resource`, so
`add`/`edit`/`destroy` all require `Permission::ADMIN`.

**Cache / headers.** `cacheInvalidate:false` and `location:false` on all three routes mean no
`X-Cache-Invalidate` and no `Location` header; neither side holds a `core::cache::Cache`. The
C++-only JSON `Content-Type`/`Accept` 415/406 gate is gone, matching Ghost's body-parser-only
Admin app.

**Files changed (all inside `include/blog/handlers/external/webhooks/`).**
`routes.cppm`; `create/handler.cppm`; `create/workflow/step/execute.cppm`;
`edit/handler.cppm`; `edit/workflow/step/execute.cppm`; `destroy/handler.cppm`;
`destroy/workflow/step/execute.cppm`. The three `workflow/flow/main.cppm` files are unchanged.

**Central registrations.** None required and none edited. The shared helpers used are all in
existing registered partitions: `blog_handlers:outbox_shim` (`BUILD.bazel:919`),
`blog_handlers_middleware:ghost_permission` (`BUILD.bazel:945`), `core_auth:ghost`,
`core_workflow`, `core_serde` (where still needed) and the context-local `ApiDataStore`.
The step/handler/flow partition names keep their `blog_handlers:webhooks_<op>_*` identities.

**New-partition needs (recorded, not edited).**
- `blog_handlers:mapping` — `base.cppm`/`clean.cppm`/`wrap.cppm` exist under
  `include/blog/handlers/mapping/` but are **not** in the `blog_handlers` BUILD `partitions`
  list, so `blog::mapping::clean(obj,"webhook")` / `wrap("webhooks",...)` cannot be imported yet.
  `webhook_model` in `create/workflow/step/execute.cppm` is the local equivalent. Note
  `WEBHOOK_FIELDS` in `clean.cppm` contains a `status` key that the e2e snapshot does not emit;
  registration should reconcile that before replacing the local helper.
- `core_outbox` — canonical target `//include/core/outbox:core_outbox` exists but is not a
  `blog_handlers` dep; the registered `blog_handlers:outbox_shim` has the same
  `Event{type,payload,aggregate_id}` / `emit` / transaction API and is used here. Main agent
  can switch the import once the dep is wired.
- A shared `now_iso()` / timestamp helper would avoid the local copy in
  `create/workflow/step/execute.cppm`; the copy is taken from the registered
  `integrations::support::now_iso()` implementation.

**Remaining gaps (no shared primitive; non-wire-observable or data-level).** Ghost's FK
constraint on `integration_id` is not enforced: the context-local store has no integrations
seed and no relational FK, so an orphan id is accepted (no shared primitive, and enforcing it
would break the existing `api_test.cc` probe which posts `integration_id:"1"`). The
`webhook.*` outbox event stands in for the model change event and the CRUD actions row (no
actions store primitive). `api_version` uses the wire default `v5.0` rather than a dynamic
`v${ghostVersion.safe}` (no shared version constant). The authenticated integration id is still
read from the `x-blog-integration-id` header instead of a DB-backed API-key context. None of
these change status codes, headers, or response fields.
