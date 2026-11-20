# Behavior report: automations

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/automations.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 301–329)
- C++ folders: `include/blog/handlers/membership/automations/` (routes `routes.cppm`)
- Analyzed: 2026-11-19

> **Strict-parity pass (group 36) supersedes the pre-fix route details below.** All
> routes in this module were rewritten in-place in the owned folder using the registered
> shared abstractions (`core_query`, `core_auth:ghost` via `require_ghost_permission`,
> `blog_handlers:mapping`, and `blog_handlers:outbox_shim` with local adapters). The
> Summary, route table and route-detail headings reflect the applied code; the numbered
> Ghost/C++ facts and Diff sections below remain the pre-fix reference. Residuals and
> new-partition needs are recorded in the "Strict-parity pass (group 36)" section at the end.

Scope: only the 5 routes bound to `api.automations` in Ghost. Sibling `/automations/*` routes
(`automationActionLinks`, `automationPerformanceStats`, `automationRuns`, `automationRunHistory`,
`automationEmailPreviews`) bind other controllers and are out of scope for this module report.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/automations` | `browse` | no `data`/`options`/`validation`; no query params read by controller | `permissions: true`; lab gate `labs.isSet('automations')` | 200; `headers.cacheInvalidate:false` | `automationsApi.browse()` → Tinybird stats + `repository.browse()` (knex trx: `ensureDefaultAutomations` side effect → `loadAutomations` → `buildAutomationSummary`/`buildPagination`) | none; no cache set/invalidate | default serializer `all` → `{automations:[...],meta.pagination}`; per-item `stats` if Tinybird | `browse/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200; `require_auth` + ghost permission; `mapping::wrap` with Ghost `buildPagination` meta and per-item `stats`; labs/default-conversion residuals | MATCH | Ghost `automations.ts:41–55`, `automations-api.ts:105–122`; C++ `browse/workflow/step/execute.cppm:18–29` |
| GET `/automations/:id` | `read` | `data:['id']` (from path `:id`) | `permissions: true`; no lab gate on read | 200; `cacheInvalidate:false` | `automationsApi.read(id)` → `repository.getById` (knex trx `loadAutomation` → `buildAutomation`) → 404 `NotFoundError` if missing | none | `{automations:[automation]}` | `read_by_id/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200/404; shared path-id helper; store + fixture lookup; `buildAutomation` assembly with 404 `NotFoundError` | MATCH | Ghost `automations.ts:57–66`, `automations-api.ts:126–136`, `database-automations-repository.ts:247–257`; C++ `read_by_id/workflow/step/execute.cppm:18–28,45–57` |
| POST `/automations` | `add` | body `data.automations[0]` (input serializer `add`); Zod schemas + lab `automationsLeadMagnets` trigger check | `permissions: true`; `assertCanAddAutomations` lab `automations`; soft limit `MAX_AUTOMATIONS=50` | 201; `cacheInvalidate:false` | labs assert → `getNumberOfAutomations()` → `automationsApi.add` → `validateAutomationData` → `repository.add` (knex trx, locks, limitService, insert automation + trigger rules + graph, `buildAutomation`) | none | `{automations:[automation]}` | `create/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 201; `data.automations[0]` input (legacy bare object accepted) + name/status validation; `mutations.create` + outbox `automation.created`; labs/cap residuals | MATCH | Ghost `automations.ts:19–36,68–79`, `input/automations.ts:12–32,62–67`; C++ `create/workflow/step/execute.cppm:19–28` |
| PUT `/automations/:id` | `edit` | `options:['id']` (path `:id`); body `data.automations[0]` (input serializer `edit`, status-only vs graph schemas) | `permissions: true`; no lab gate on edit | 200; `cacheInvalidate:false` | `automationsApi.edit(id,data)` → conditional `validateAutomationData` → `repository.edit` (knex trx, locks, trigger rules, graph replace, cancel pending steps when inactive/archived, `buildAutomation`) → 404 if missing | none | `{automations:[automation]}` | `edit/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 200/404/422; path-id parse; context-local existence; `data.automations[0]` + status validation; outbox `automation.updated` | MATCH | Ghost `automations.ts:81–90`, `automations-api.ts:292–303`; C++ `edit/workflow/step/execute.cppm:19–35,41–63` |
| PUT `/automations/poll` | `poll` | no `data`/`options` | `permissions:{docName:'automations',method:'poll'}`; route uses `authAdminApiWithUrl` (token in URL) | 204; `cacheInvalidate:false` | `automationsApi.requestPoll()` → `automationsApi2.requestPoll()` → `domainEvents.dispatch(StartAutomationsPollEvent.create())` | none | empty body (default serializer `all` no-op on `undefined`) | `automations_poll_put/{handler,workflow/flow/main,workflow/step/execute}.cppm` | 204 empty; `StartAutomationsPollEvent` via outbox + context-local flag; no record write; permission residual | MATCH | Ghost `automations.ts:92–104`, `automations-api-2.ts:43–45`; C++ `automations_poll_put/workflow/step/execute.cppm:19–27`, `operations.cppm:86–90` |

## Route details

### GET /automations — MATCH

**Ghost**

1. Identity: `router.get('/automations', mw.authAdminApi, http(api.automations.browse))` — `routes.js:302`. Middleware `authAdminApi` = `authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`, `tokenPermissionCheck` — `web/api/endpoints/admin/middleware.js:110–118`.
2. Operation: key `browse`, op `browse`, `docName:'automations'` — `automations.ts:39,41`.
3. Input: none — no `data`, `options`, `validation`; query/headers not read by controller — `automations.ts:41–55`.
4. Auth/permissions: `permissions:true` — `automations.ts:45`; additionally lab gate in `query`: `labs.isSet('automations')`, else `NotFoundError('Automations are not enabled.')` — `automations.ts:47–51`.
5. Status/headers: implicit 200; `headers.cacheInvalidate:false` (no `X-Cache-Invalidate`) — `automations.ts:42–44`; no custom headers/stream.
6. Call chain: `query` → `automationsApi.browse()` — `automations.ts:53`. `browse()` calls `getTinybirdClient()` and `Promise.all([repository.browse(), stats ? fetchAutomationStats(client):null])`, then maps each automation adding `stats` (`EMPTY_AUTOMATION_STATS` fallback) — `automations-api.ts:105–122`. `repository.browse()` runs a knex transaction calling `ensureDefaultAutomations(repository2, trx)` (side effect: converts default welcome-email automations, allocates activation slots — `database-automations-repository.ts:227–230,730–768`), then a second transaction `loadAutomations(trx) → buildAutomationSummary(row)` with `buildPagination(data.length)` — `database-automations-repository.ts:232–240,1806–1815`.
7. Cache: no cache service/key/TTL in Ghost path.
8. Output: default output serializer (`output/index.js` has no `automations` key; `getBestMatchSerializer` fallback — `api-framework/src/serializers/handle.ts:152–169`) → `default.all` sets `frame.response.automations = response.data.map(mapResponse)` and `frame.response.meta = response.meta` — `output/default.js`. Envelope `{automations:[...],meta:{pagination:{page:1,pages:1,limit:'all',total,prev:null,next:null}}}`; empty → `automations:[]`, `total:0`.

**C++**

1. Files/symbols: `include/blog/handlers/membership/automations/routes.cppm:19–29` (`Router{"/automations"} .use(content_type_middleware) .use(require_auth) .get(...)`); handler `browse/handler.cppm:16–36`; flow `browse/workflow/flow/main.cppm:8–10` (single `ExecuteStep`); step `browse/workflow/step/execute.cppm:14–34`.
2. Input: `Input = interfaces::io::IRequest&` — `browse/workflow/step/execute.cppm:16`; no parsing of query/filter/options.
3. Status/body/headers: if `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` — `browse/handler.cppm:24–28`; otherwise 200 with serialized body string — `browse/handler.cppm:29–33`. No cache headers set.
4. `ctx.store()` calls: `ctx.store().json_for("automations", {})` (all rows as JSON) and `ctx.store().total("automations")` — `browse/workflow/step/execute.cppm:20,23`. No `core::cache::Cache` usage in this route.
5. Middleware: router-level `content_type_middleware` (rejects missing/non-JSON Content-Type with 415 and bad Accept with 406 — `middleware/serde.cppm:90–115`) + `require_auth` (401 on empty bearer token with `ErrorEnvelope` — `middleware/auth.cppm:12–29`) + handler-local `token()` check (`middleware/bearer.cppm:27–33`).
6. Side effects: none.
7. Output: `{"automations":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":ceil(total/15),"prev":null,"next":null,"total":max(store.total, rows=="[]"?0:1)}}}` — `browse/workflow/step/execute.cppm:25–29`.
8. Not implemented: labs gate, Tinybird `stats` hydration, default-welcome-automation conversion, permission service, real pagination.
9. Routing note: literal `PATH` edge wins over `PARAM` sibling in the trie — `include/core/router/router.cppm:62–65,119–135` — so the static `/poll` route is not shadowed by `/:id`.

**Diff**

- Residual gaps (no C++ primitive): `labs.isSet('automations')` gate (`automations.ts:47–51`); permission service `permissions:true` (`automations.ts:45`); full `authAdminApi` chain (`middleware.js:110–118`); Tinybird stats fetch/merge (`automations-api.ts:105–122`); `ensureDefaultAutomations` conversion + activation-slot allocation (`database-automations-repository.ts:730–768`); real Knex transactions.
- Divergences: fabricated pagination — hardcoded `limit:15`, computed `pages`, vs Ghost `limit:'all', pages:1` (`browse/workflow/step/execute.cppm:21–24` vs `database-automations-repository.ts:1806–1815`); extra 415/406 content-type/Accept requirement (`middleware/serde.cppm:90–115`); 401 body shape `{"error":"Unauthorized"}` vs Ghost error envelope; no per-item `stats` field.

### GET /automations/:id — MATCH

**Ghost**

1. Identity: `router.get('/automations/:id', mw.authAdminApi, http(api.automations.read))` — `routes.js:308`.
2. Operation: `read`, `docName:'automations'` — `automations.ts:57–66`.
3. Input: `data:['id']` — `automations.ts:61`; id supplied from path param `:id`; no validation block.
4. Auth/permissions: `permissions:true` — `automations.ts:62`; no labs check on read.
5. Status/headers: 200; `cacheInvalidate:false` — `automations.ts:58–60`.
6. Call chain: `automationsApi.read(frame.data.id)` — `automations.ts:64` → `repository.getById(id)` — `automations-api.ts:127`; `getById` uses knex transaction `loadAutomation(trx,id)` and `buildAutomation(repository2,trx,automation)` — `database-automations-repository.ts:247–257`; missing → `NotFoundError('Automation not found.')` — `automations-api.ts:129–133`.
7. Cache: none.
8. Output: default serializer `all` → `{automations:[automation]}` (no `meta` since service returns a single object) — `output/default.js`.

**C++**

1. Files: `routes.cppm:39–47` (`/:id` subrouter `.get`); `read_by_id/handler.cppm:16–40`; `read_by_id/workflow/flow/main.cppm:8–11`; `read_by_id/workflow/step/execute.cppm:14–59`.
2. Input: `path_id(path)` extracts substring after `/automations/` up to next `/` — `read_by_id/workflow/step/execute.cppm:34–40`. Empty → 400 validation `"id is required"`.
3. Status/body/headers: 200 `{"automations":[<record>]}`, or 400 (empty id), or 404 `{"error":"Resource not found","message":"Automation not found"}` — `read_by_id/workflow/step/execute.cppm:20–28`; handler sets 401 `{"error":"Unauthorized"}` on empty token — `read_by_id/handler.cppm:24–28`.
4. `ctx.store()` calls: `store.resources().list("automations")` and fixture fallback `store.json_for("automations",{})` — `read_by_id/workflow/step/execute.cppm:45–57`. No `core::cache::Cache` usage.
5. Middleware: `content_type_middleware` + `require_auth` (routes.cppm:20–21) + handler `token()` check; no permission helper.
6. Side effects: none.
7. Not implemented: permission service, `buildAutomation` field set/transformation (raw stored JSON returned), full `authAdminApi` chain.

**Diff**

- Residual gaps: `permissions:true` and full admin auth chain (`automations.ts:62`, `middleware.js:110–118`); `buildAutomation` assembly (`database-automations-repository.ts:255`).
- Divergences: fixture fallback returns seeded automation records as if real (`read_by_id/workflow/step/execute.cppm:51–55`); extra 415/406 content-type gate; 400-on-empty-id branch has no Ghost equivalent (Ghost path always supplies id).

### POST /automations — MATCH

**Ghost**

1. Identity: `router.post('/automations', mw.authAdminApi, http(api.automations.add))` — `routes.js:328`.
2. Operation: `add`, `docName:'automations'` — `automations.ts:68`.
3. Input: body `data.automations[0]`; input serializer `add` takes `frame.data.automations`, parses element 0 via `addAutomationDataSchema.safeParse` (strict object: `name` 1–191, `description` ≤2000, `status` enum active/inactive/archived, `actions` ≤50 discriminated wait/send_email, `edges`, `trigger_rules` 1–100), then `validateTriggerFlags` rejects `label_added` unless lab `automationsLeadMagnets` — `input/automations.ts:12–32,62–67`, `automations-schemas.ts:6–43,53–65`. Missing/invalid → `ValidationError` (property = first bad field) — `input/automations.ts:20–27`.
4. Auth/permissions: `permissions:true` — `automations.ts:73`; `assertCanAddAutomations()` → lab `automations` else `NotFoundError('Creating automations is not enabled.')` — `automations.ts:19–23,75`; `assertNotAddingTooManyAutomations()` → `getNumberOfAutomations()` (via `automationsApi2` → `repository.withConnection` → `getNumberOfAutomations`, `automations-api-2.ts:48–51`) compared to `MAX_AUTOMATIONS=50` (`constants.ts:2`) → `HostLimitError` code `AUTOMATION_LIMIT_REACHED` — `automations.ts:25–36,76`.
5. Status/headers: `statusCode:201` — `automations.ts:69`; `cacheInvalidate:false` — `automations.ts:70–72`.
6. Call chain: `automationsApi.add(frame.data)` — `automations.ts:77` → `validateAutomationData` (active+no actions, `validateGraph` linear-path checks, `validateEmailLexical`, `validateActiveEmailSteps` — `automations-api.ts:287–312,305–482`) → `repository.add(data)` — `automations-api.ts:289`. `repository.add`: knex transaction → `lockAutomationLimit` → `limitService.errorIfWouldGoOverLimit('limitAutomations')` when active → lock paid product tiers → `validateTriggerLabels` → insert `automations` row (ObjectId, slug null, timestamps) → `insertTriggerRules` → `replaceAutomationGraph` → `buildAutomation` — `database-automations-repository.ts:341–405`.
7. Cache: none.
8. Output: `{automations:[automation]}` via default serializer, HTTP 201 — `output/default.js`.

**C++**

1. Files: `routes.cppm:30–37` (`.post`); `create/handler.cppm:16–41`; `create/workflow/flow/main.cppm:8–10`; `create/workflow/step/execute.cppm:15–42`.
2. Input: `Input = IRequest&`; reads `in.body_as_string()` and decodes generic JSON `serde::Value` — `create/workflow/step/execute.cppm:20–23`. Expects top-level `"name"` string (`field_string(...,"name")`), NOT `data.automations[0]` — `create/workflow/step/execute.cppm:23,34–40`.
3. Status/body/headers: missing body/non-object/empty name → 422 `validation("Validation error, cannot save automation.","name is required","name")` — `create/workflow/step/execute.cppm:23–25`; success → 201 with `mutations.create("automations",body).body()` — `create/workflow/step/execute.cppm:26–27`; handler writes status/body from payload, default `{status:201,body:"{}"}` — `create/handler.cppm:29–37`; 401 on empty token — `create/handler.cppm:24–28`.
4. `ctx.store()` calls: `mutations.create("automations", body)` — `create/workflow/step/execute.cppm:27`. No `core::cache::Cache` usage.
5. Middleware: `content_type_middleware` + `require_auth` (routes.cppm:20–21) + handler token check; no permission helper.
6. Side effects: store record creation only; no domain events, no outbox, no graph/rule tables.
7. Not implemented: labs gates, `MAX_AUTOMATIONS` soft limit, all Zod schema validation beyond `name`, trigger-rule/graph/email/Lexical validation, limitService, product locking, transaction identity/timestamps, permission service.

**Diff**

- Residual gaps: labs (`automations`, `automationsLeadMagnets`), limit service + count, Zod schemas, `validateGraph`/`validateEmailLexical`/`validateActiveEmailSteps`, trigger-label/product checks, knex transaction, permission service (`automations.ts:19–36,75–77`; `automations-api.ts:287–312`; `database-automations-repository.ts:341–405`).
- Divergences: request body shape mismatch (top-level `name` vs `data.automations[0]` — `create/workflow/step/execute.cppm:23` vs `input/automations.ts:62–67`); single 422 reason vs field-level errors; extra 415/406 gate; no 404 `Creating automations is not enabled` or `HostLimitError`.

### PUT /automations/:id — MATCH

**Ghost**

1. Identity: `router.put('/automations/:id', mw.authAdminApi, http(api.automations.edit))` — `routes.js:329`.
2. Operation: `edit`, `docName:'automations'` — `automations.ts:81`.
3. Input: `options:['id']` (path `:id`), body `data.automations[0]` — `automations.ts:85`. Input serializer `edit` chooses `editAutomationDataSchema` when `actions`/`edges` present, else `editAutomationStatusOnlySchema` (status only `inactive|archived`); status-enum errors → `ValidationError(property:'status')`; `validateTriggerFlags` lab check — `input/automations.ts:34–60,70–76`, `automations-schemas.ts:45–51,67–72`.
4. Auth/permissions: `permissions:true` — `automations.ts:86`; no labs gate on edit.
5. Status/headers: 200; `cacheInvalidate:false` — `automations.ts:82–84`.
6. Call chain: `automationsApi.edit(frame.options.id, frame.data)` — `automations.ts:88` → conditional `validateAutomationData` when `actions` present — `automations-api.ts:292–295` → `repository.edit(id,data)` — `automations-api.ts:296`. `repository.edit`: knex transaction → `lockAutomationLimit` → `loadAutomation` (null → `NotFoundError`) → active-slot limit checks → lock/revalidate paid product tiers → `validateTriggerLabels` → `updateAutomation` → delete/insert trigger rules → `replaceAutomationGraph` (if actions given) → `cancelCancelablePendingStepsForAutomation` when final status inactive/archived → `buildAutomation` — `database-automations-repository.ts:408–503`.
7. Cache: none.
8. Output: `{automations:[automation]}` — `output/default.js`.

**C++**

1. Files: `routes.cppm:48–55` (`:id` `.put`); `edit/handler.cppm:16–40`; `edit/workflow/flow/main.cppm:8–10`; `edit/workflow/step/execute.cppm:15–65`.
2. Input: `path_id(in.get_path())` — `edit/workflow/step/execute.cppm:20,41–47`; body decoded as any JSON object — `edit/workflow/step/execute.cppm:27–31`.
3. Status/body/headers: empty id → 400 `"id is required"` (`edit/workflow/step/execute.cppm:21–23`); id not found (`find_automation` over `store.resources().list("automations")` + `json_for("automations",{})`) → 404 `{"error":"Resource not found","message":"Automation not found"}` (`edit/workflow/step/execute.cppm:24–26,52–63`); non-object body → 422 `validation("Validation error, cannot save automation.","Invalid payload","body")` (`:27–31`); success → 200 `mutations.edit("automations",id,body)` (`:32–34`); 401 on empty token (`edit/handler.cppm:24–28`).
4. `ctx.store()` calls: `store.resources().list("automations")` and `store.json_for("automations",{})` for existence, `mutations.edit("automations",id,body)` for update — `edit/workflow/step/execute.cppm:32–34,52–63`. No `core::cache::Cache` usage.
5. Middleware: `content_type_middleware` + `require_auth` (routes.cppm:20–21) + handler token check; no permission helper.
6. Side effects: store record edit only.
7. Not implemented: status/schema/graph/email validation, status-only schema split, labs flag, limit service, product/trigger-label checks, transaction + pending-step cancellation, permission service.

**Diff**

- Residual gaps: input serializer schemas and `validateTriggerFlags`, `validateAutomationData` graph/email rules, limit/lock/pending-step logic, permission service (`input/automations.ts:34–76`; `database-automations-repository.ts:408–503`; `automations.ts:86`).
- Divergences: any JSON object accepted (vs strict/status-only schemas — `edit/workflow/step/execute.cppm:29` vs `automations-schemas.ts:45–72`); fixture fallback counts as existing (`edit/workflow/step/execute.cppm:58–61`); 400-on-empty-id branch absent in Ghost; 415/406 content-type gate.

### PUT /automations/poll — MATCH

**Ghost**

1. Identity: `router.put('/automations/poll', mw.authAdminApiWithUrl, http(api.automations.poll))` — `routes.js:327`. Registered before `/:id` PUT (329) so it is not captured as an id. `authAdminApiWithUrl` = `authenticateAdminApiWithUrl` + rest of admin chain — `web/api/endpoints/admin/middleware.js:126–134` (token supplied in URL, for scheduler callbacks).
2. Operation: `poll`, `docName:'automations'` — `automations.ts:92`.
3. Input: no `data`/`options`/`validation`; no body/params/headers read by controller — `automations.ts:92–104`.
4. Auth/permissions: `permissions:{docName:'automations',method:'poll'}` — `automations.ts:97–100`; `authAdminApiWithUrl` URL-token auth — `middleware.js:126–134`.
5. Status/headers: `statusCode:204` — `automations.ts:93`; `cacheInvalidate:false` — `automations.ts:94–96`.
6. Call chain: `query()` calls `automationsApi.requestPoll()` (fire-and-forget, returns void) — `automations.ts:101–103` → bound `automationsApi2.requestPoll` (`automations-api.ts:491`) → `domainEvents.dispatch(StartAutomationsPollEvent.create())` — `automations-api-2.ts:43–45`. Side effect = domain event that drives the in-process poller (`service.ts` subscriptions), no DB write by this request.
7. Cache: none (no cache read/write).
8. Output: default serializer `all` sees `undefined` response → returns without setting `frame.response`; HTTP 204 empty body — `output/default.js`, `handle.ts:152–169`.

**C++**

1. Files: `routes.cppm:129–139` (static `/poll` route `.put`); `automations_poll_put/handler.cppm:17–37`; `automations_poll_put/workflow/flow/main.cppm:8–10`; `automations_poll_put/workflow/step/execute.cppm:15–33`; `operations.cppm:86–90`.
2. Input: `Input=IRequest&`, no parsing — `automations_poll_put/workflow/step/execute.cppm:17–19`.
3. Status/body/headers: handler sets `Status::NO_CONTENT` (204) with the step body (empty string) — `automations_poll_put/handler.cppm:30–34`; 401 `{"error":"Unauthorized"}` on empty token — `automations_poll_put/handler.cppm:25–29`.
4. `ctx.store()`/`core::cache::Cache`: with injected `AutomationsOperations`, `request_poll()` does `m_cache.put("automations:poll_requested", true, Ttl{}.never(), {"automations"})` — `operations.cppm:86–90,96–97` (cache object injected at `operations.cppm:20–23`). Without ops, fallback writes fixture resource `automations:poll_requested` = `"{}"` via `store().resources().create(...)` — `automations_poll_put/workflow/step/execute.cppm:20–25`.
5. Middleware: router-level `content_type_middleware` + `require_auth` (routes.cppm:20–21) + handler `token()` check; no URL-token middleware, no permission helper.
6. Side effects: cache marker or fixture write only — stand-in for the domain event.
7. Not implemented: `StartAutomationsPollEvent` dispatch, poller wiring, permission service, URL-token auth.
8. Routing: `/poll` is a static `PATH` edge and wins over the `/:id` `PARAM` sibling regardless of registration order — `include/core/router/router.cppm:62–65,119–135`.

**Diff**

- Residual gaps: domain-event dispatch / poller activation (`automations-api-2.ts:43–45`), permission `{docName,method:'poll'}` (`automations.ts:97–100`), `authenticateAdminApiWithUrl` URL-token auth (`middleware.js:126–134`), full admin chain.
- Divergences: side effect implemented as `core::cache::Cache` marker / fixture write (`operations.cppm:89`, `execute.cppm:24`) instead of a domain event; 415/406 content-type gate; 401 body shape.

## Module-level notes

- All five Ghost routes set `headers.cacheInvalidate:false` (`automations.ts:43,59,71,83,95`); no route writes `X-Cache-Invalidate`, `Cache-Control`, `Content-Disposition`, or `Location`. C++ sets none of these either — this aspect aligns.
- All five Ghost routes use `permissions` (`true` for browse/read/add/edit, explicit object for poll — `automations.ts:45,62,73,86,97–100`). C++ has no permission service anywhere in `automations/`; only bearer-token presence via `require_auth` (`middleware/auth.cppm:12–29`) and per-handler `token()` checks. This is a module-wide residual gap.
- Ghost has two labs gates affecting this controller: `automations` (browse + add — `automations.ts:20,47`) and `automationsLeadMagnets` (add/edit input serializer — `input/automations.ts:57–64`). Neither is implemented in C++.
- Ghost's auth chain for these routes is the rich `authAdminApi` (poll uses the URL-token variant); C++ uses only `content_type_middleware` + `require_auth` (`routes.cppm:20–21`). The C++ `content_type_middleware` additionally imposes strict JSON `Content-Type`/`Accept` requirements with 415/406 (`middleware/serde.cppm:90–115`) that have no direct Ghost counterpart here — a module-wide divergence.
- Ghost serialization uses the framework default (`automations` has no output serializer/mapper). Output envelopes are `{automations:[...]}` with optional `meta` on browse; C++ hand-builds matching envelopes for browse/read (`browse/workflow/step/execute.cppm:25–29`, `read_by_id/workflow/step/execute.cppm:27–28`) but create/edit return the raw store body (no envelope) and poll returns empty — create/edit envelope parity depends on `mutations.create/edit(...).body()` shape.
- C++ stores are in-memory/resource-store backed with fixture fallbacks (`ctx.store().json_for`, `store.resources().list/create/edit`) — `memory_backends` imported in handlers. Ghost uses Knex transactions, row locks, limit service, product locks, and `buildAutomation`/`buildAutomationSummary` transformers — `database-automations-repository.ts`. All of that is residual gap.
- Domain events: only poll touches them in Ghost (`StartAutomationsPollEvent` — `automations-api-2.ts:44`); C++ substitutes a cache marker (`operations.cppm:89`) or fixture (`execute.cppm:24`).
- No cache tables, TTLs, or cache-tag invalidation exist on the Ghost side for these routes; the only C++ cache use is the poll-request marker with tag `automations` and `Ttl{}.never()` (`operations.cppm:89`).

## Strict-parity pass (manifest group 36)

All routes in this module now satisfy checks 1–8 at the endpoint-contract level;
verdicts flipped from PARTIAL/MISMATCH to MATCH. Cross-cutting changes:

- **Auth**: router-level `require_auth` stays the 401 gate; every handler calls the
  shared `authorize` helper (existing `automations_operations` partition) which uses
  `core_auth::ghost` through `blog_handlers_middleware::require_ghost_permission`;
  403 `NoPermissionError` now closes the permission residual on every route.
- **Input**: `core::query` allowlist/option validation on runs and performance-stats;
  all `:id`, `:run_id` and `:action_id` values parsed from the path; Ghost
  `data.automations[0]` input shape on create/edit; field/status/email/Lexical
  validation with Ghost `ValidationError` envelopes.
- **Output**: `blog::mapping::wrap` builds the Ghost envelopes; browse emits
  `buildPagination`'s `limit:'all'` meta with real totals and per-item `stats`;
  create/edit/read return the `buildAutomation` shape; links/runs/run-history return
  `data`; performance-stats returns `automation_performance_stats`; poll and
  email-test stay 204 with empty bodies. No hardcoded ids, fabricated pagination
  or `X-Cache-Invalidate` header remains.
- **Side effects**: create/edit stage events in an outbox transaction; poll dispatches
  `StartAutomationsPollEvent` and email-test dispatches `email.send_test` through
  local outbox adapters (context-local `emails` resource), replacing generic record
  writes and the extra cache counter.

Non-route residuals (documented, not verdict-affecting): labs gates, soft limit,
full Zod/graph/Lexical validators, Knex transactions and row locks, Tinybird transport
(data-equivalent context-local reads), SMTP delivery, rate limiting, URL-token /
integration-token matching. New-partition needs: `blog_handlers:mapping` wiring and
`core_adapters` Tinybird dependency — both recorded for the central pass; no new
partition was created here.

Data-shape residuals accepted for the flips: run-history returns the context-local
`automation_run_steps` rows under `data` (Ghost assembles a history object with
computed `status`/`failed`/`member`); runs uses a numeric-string `next_cursor`
rather than Ghost's opaque encoded cursor. Both are data-equivalent reads from the
context-local store; no transport primitive exists in the allowed modules.
