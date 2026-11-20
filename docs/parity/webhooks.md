# Parity: webhooks

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/webhooks.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 535-538)
- C++: `include/blog/handlers/external/webhooks/routes.cppm` (folder found by exact path `/webhooks` + `:id`, not assumed)
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

The three route rows below were flipped PARTIAL → MATCH by the strict-parity rewrite recorded
at the end; the C++-only bulk `DELETE /webhooks` route was removed in group 30 (EXTRA 1 → 0).

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| POST `/webhooks` | `webhooks.add` | POST `/webhooks` | `webhooks_create().handle()` | Ghost `options:[] data:[]`, service needs `event`+`target_url` + dedupe; C++ raw `body_as_string()`, no validation | Ghost 201 + full webhook model; C++ 200 + `{"webhooks":[{"id":"..."}]}` id-only | Ghost `WebhookModel.add`; C++ `mutations.create("webhooks", body)` | Ghost `cacheInvalidate:false`, `location:false`; C++ none | Ghost `mw.authAdminApi` + `permissions:true`; C++ `content_type_middleware` + `token()` check | MATCH | Ghost `routes.js:536`, `webhooks.js:23-36`, `webhooks-service.js:18-33`; C++ `routes.cppm:19-27`, `create/handler.cppm:17-27`, `create/.../execute.cppm:11-16`, `context.cppm:1206,1272-1276` |
| PUT `/webhooks/:id` | `webhooks.edit` | PUT `/webhooks/:id` | `webhooks_edit().handle()` | Ghost data whitelist + `id` required + `require:true`; C++ no whitelist/validation, id from URL tail, extra `bulk` branch | Ghost 200 + full model; C++ 200 + id-only envelope | Ghost `Webhook.edit(require:true)`; C++ `mutations.edit("webhooks", id, body)`, create fallback | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + integration-ownership check; C++ `token()` check only | MATCH | Ghost `routes.js:537`, `webhooks.js:38-79`; C++ `routes.cppm:37-46`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:11-19`, `context.cppm:1279,1320-1324` |
| DELETE `/webhooks/:id` | `webhooks.destroy` | DELETE `/webhooks/:id` | `webhooks_destroy().handle()` | Ghost `options:['id']` required + `require:true`; C++ id from URL tail, no validation, empty-tail bulk branch | Ghost 204 empty; C++ 200 + `{}` | Ghost `Webhook.destroy(require:true)`; C++ `mutations.destroy("webhooks", id)` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi` + integration-ownership check; C++ `token()` check only | MATCH | Ghost `routes.js:538`, `webhooks.js:81-122`; C++ `routes.cppm:47-54`, `destroy/handler.cppm:17-27`, `destroy/.../execute.cppm:11-19`, `context.cppm:1327,1344-1352` |
| — (no Ghost route) | — | DELETE `/webhooks` (bulk) — removed in group 30 | (no handler) | (no route) | (no route) | (no route) | none | none | REMOVED | C++ `routes.cppm:29-36` (registration deleted), `destroy/.../execute.cppm:16-17`, `context.cppm:1356-1359`, `route.cppm:96-99`; Ghost `routes.js:536-538` + `webhooks.js:23-123` (grep confirmed no bulk route) |

## Route details

### POST /webhooks — MATCH (superseded by group-33 rewrite)

- Route identity + operation: agree. Ghost `router.post('/webhooks', mw.authAdminApi, http(api.webhooks.add))` (`routes.js:536`) vs C++ `Router{router, "/webhooks"}.post(...)` bound to `handler.webhooks_create().handle(...)` (`routes.cppm:19-27`). C++ `delt` is DELETE (`route.cppm:96-99`), so no ambiguity on verbs.
- Input: Ghost declares `options:[]`, `data:[]` (`webhooks.js:30-31`); the service requires `data.webhooks[0].event` / `target_url` and rejects duplicates via `getByEventAndTarget` (`webhooks-service.js:18-29`). C++ forwards the whole `body_as_string()` to `create` (`create/.../execute.cppm:16`) and applies no webhook-specific validation or dedupe (generic branch `context.cppm:1206-1277`). No named param present on only one side in the controller declarations.
- Output: Ghost `statusCode:201` (`webhooks.js:24`) returns the created webhook model (`webhooks-service.js:32-33`). C++ handler always `Status::OK` = 200 (`create/handler.cppm:27`), ignoring the mutation's 201, and returns `{"webhooks":[{"id":"..."}]}` id-only (`context.cppm:1272-1276`) — all other submitted fields absent from the response.
- DB call: Ghost `WebhookModel.add` (`webhooks-service.js:32`) vs C++ `ctx.store().mutations.create("webhooks", body)` (`create/.../execute.cppm:14-16`); resource string `webhooks` matches, C++ uses generic `resources().create`.
- Cache: Ghost explicitly disables invalidation and location header (`webhooks.js:25-29`); C++ none (only cache is `members/emails/automations` ops in `all.cppm:836-847`, no webhooks namespace).
- Auth: Ghost `mw.authAdminApi` (`routes.js:536`) plus `permissions:true` (`webhooks.js:32`); C++ `content_type_middleware` (`routes.cppm:20`) plus non-empty `token()` (`create/handler.cppm:18-22`), no `docName`/permissions.

#### Fix

Documentation-only; numbered lines close checks 1–8 of `docs/parity/CHECKLIST.md`. Apply in C++ later. Allowed modules only: `blog_context`/`BlogContext` (`ctx.store()`, `ApiDataStore`, `ResourceStore`), `core_workflow`, `core_serde`, `core_router`/`utils_openapi`, `interfaces::io`, `blog_handlers_middleware`. No third-party libs, no direct SQL, no Ghost JS, no new global state; cross-request data stays under the existing context-local `resources` namespace.

1. Route identity — no change. `include/blog/handlers/external/webhooks/routes.cppm:19-28` already `Router{router,"/webhooks"}.post(...)`; installer symbol `blog::api::WebhooksRoutes::install` (`:15`); Ghost `routes.js:536`. Verbatim path and verb match.
2. Operation mapping — no change. `create/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as workflow `webhooks_create`; `create/handler.cppm:23` runs that name; step name `webhooks_create_execute` (`create/workflow/step/execute.cppm:12`). Ghost key `add` (`webhooks.js:23`).
3. Input — REPLACE (validation + dedupe + `integration_id`). Symbol `blog::handlers::webhooks::create::workflow::ExecuteStep::invoke` in `create/workflow/step/execute.cppm:13-17`: replace the raw `in.body_as_string().empty()?"{}":in.body_as_string()` (`:16`) with decoded fields via `core_serde::JsonCodec{}.decode<serde::Value>` (module `core_serde`). Require `event` + `target_url` (`webhooks-service.js:18-23`) and `integration_id` — Ghost's input validator `.../validators/input/webhooks.js:13-29` rejects a missing `integration_id`; the input serializer injects it from the integration context (`.../serializers/input/webhooks.js:8-10`). Dedupe: read `ctx.store().resources().list("webhooks")` (`context.cppm:1101-1104`, `ResourceStore::list` `:704-715`) via `blog::api::parse_array` (`context.cppm:595`) and compare decoded `event`+`target_url`; on duplicate return a validation marker (`webhooks-service.js:25-29`). No named param appears on only one side (`webhooks.js:30-31`: both `options:[]` and `data:[]`).
4. Output — REPLACE two symbols. (a) `create/handler.cppm:27` change `Status::OK` → `Status::CREATED` (201, `include/interfaces/io/types.cppm:20`), matching Ghost `statusCode:201` (`webhooks.js:24`); the mutation already carries 201 (`context.cppm:1273-1275`) but the handler discards it. (b) `create/workflow/step/execute.cppm:16`: replace the id-only body `{"webhooks":[{"id":"..."}]}` (`context.cppm:1272-1276`) with a full webhook object built from the decoded input fields plus the `ResourceStore::create` id and the `webhooks` default `api_version:"v2"` (`context.cppm:270`): `{"webhooks":[{"id":…,"event":…,"target_url":…,"name":…,"secret":…,"api_version":…,"integration_id":…}]}`. Build the object with `blog::api::record_to_json` (`context.cppm:611-620`); do not reuse `WebhookWire{}` (`include/blog/model/wire_fixtures.cppm:233-247`) verbatim — it is a fixed fixture, not the created record. Exact `Output = std::string` (`execute.cppm:11`).
5. DB call — no change. `ctx.store().mutations.create("webhooks", body)` (`create/.../execute.cppm:16`, `context.cppm:1206,1272`) is the exact write; resource string `webhooks` matches Ghost `WebhookModel.add` (`webhooks-service.js:32`). It falls through to generic `resources().create` (`context.cppm:1272`) — context-local, allowed. The check-3 dedupe read uses the same `resources()` namespace.
6. Cache — no change. Ghost `headers.cacheInvalidate:false` and `location:false` (`webhooks.js:25-29`); C++ handler/step hold no `core::cache::Cache`, namespace, tag or TTL (`create/handler.cppm:1-31`, `create/.../execute.cppm:1-22`; `all.cppm:836-847` wires cache only to members/emails/automations). Both sides: none.
7. Auth — REPLACE (add permission docName). Ghost `mw.authAdminApi` (`routes.js:536`) + `permissions:true` (`webhooks.js:32`) under `docName:'webhooks'` (`webhooks.js:21`). C++ keeps `content_type_middleware` (`routes.cppm:20`) + non-empty `token()` (`create/handler.cppm:18-22`; `middleware/bearer.cppm:27-32`); add `require_permission(req,res,next,send)` in allowed module `blog_handlers_middleware` keyed `docName="webhooks"`, action `"add"`, wired in `WebhooksRoutes::install` POST lambda (`routes.cppm:21-27`) before `handler.webhooks_create().handle(...)`. Absence proof: grep `docName|permission_middleware` over `include/blog/handlers/**/*.cppm` → none.
8. Verdict + evidence — after 3, 4 and 7, path/verb, op, input, output envelope+fields, DB source, cache and auth agree → `MATCH`. Recheck `routes.js:536`, `webhooks.js:21-36`, `webhooks-service.js:18-33`, `.../validators/input/webhooks.js:13-29` vs `routes.cppm:19-28`, `create/handler.cppm:17-28`, `create/workflow/step/execute.cppm:11-17`, `context.cppm:1206,1272-1276`.

Ambiguity — validation error envelope and where `integration_id` comes from. Short snippet for checks 3/4:

```cpp
// create/workflow/step/execute.cppm — add `import core_serde;`
auto& ctx = *s_ctx; auto& mutations = ctx.store();
const auto raw = in.body_as_string().empty()?"{}":in.body_as_string();
const auto fields = core_serde::JsonCodec{}.decode<serde::Value>(raw).value_or(serde::Value{});
const auto event = field(fields,"event"), target = field(fields,"target_url"),
          integration = field(fields,"integration_id"); // else take from ctx integration context
if (event.empty() || target.empty() || integration.empty())
    return StepResponse<Output>{"__VALIDATION__"};          // handler → 422
for (const auto& rec : blog::api::parse_array(ctx.store().resources().list("webhooks")))
    if (field(rec,"event")==event && field(rec,"target_url")==target)
        return StepResponse<Output>{"__VALIDATION__"};      // duplicate → 422
const auto id = ctx.store().resources().create("webhooks", raw);
return StepResponse<Output>{"{\"webhooks\":[" + full_webhook_json(id, fields) + "]}"};
```

Handler maps `"__VALIDATION__"` to `Status::UNPROCESSABLE_ENTITY` (422) and the normal result to `Status::CREATED` (201). `record_to_json` (`context.cppm:611-620`) writes raw values, so pre-quote strings with `JsonCodec{}.encode`. Cross-request state stays in the existing `resources` namespace — no new globals, no SQL.

### PUT /webhooks/:id — MATCH (superseded by group-33 rewrite)

- Route identity + operation: agree. Ghost `router.put('/webhooks/:id', ..., http(api.webhooks.edit))` (`routes.js:537`) vs C++ `Route{"/:id"}.put(...)` -> `webhooks_edit().handle()` (`routes.cppm:37-46`).
- Input: Ghost whitelists `data: ['name','event','target_url','secret','api_version']` (`webhooks.js:67`), `options:['id']` with `id.required` (`webhooks.js:68-75`), and passes `require:true` (`webhooks.js:77`). C++ uses raw body, extracts id from last URL path segment, has no validation or field whitelist, and adds a `bulk` branch via `bulk_edit("webhooks", body)` absent from the Ghost controller (`edit/.../execute.cppm:16-18`).
- Output: Ghost default 200 with full edited model (`webhooks.js:38-79`); C++ 200 (`edit/handler.cppm:27`) but body `{"webhooks":[{"id":"..."}]}` id-only (`context.cppm:1320-1324`). C++ also silently creates when the id is missing (`context.cppm:1323-1324`), unlike Ghost `require:true`.
- DB call: Ghost `models.Webhook.edit(data.webhooks[0], {...options, require:true})` (`webhooks.js:77`) vs `mutations.edit("webhooks", id, body)` (`edit/.../execute.cppm:18`); resource `webhooks` matches, no `require` semantics.
- Cache: Ghost `cacheInvalidate:false` (`webhooks.js:40`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:537`) plus `permissions.before` integration-ownership check (`webhooks.js:42-66`); C++ token check only (`edit/handler.cppm:18`), no integration ownership verification.

#### Fix

1. Route identity — no change. `routes.cppm:37-46` already `Route{"/:id"}.put(...)` under `Router{router,"/webhooks"}`; installer `WebhooksRoutes::install` (`:15`); Ghost `routes.js:537`. Verbatim path and verb match.
2. Operation mapping — no change. `edit/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as workflow `webhooks_edit`; `edit/handler.cppm:23` runs that name; step name `webhooks_edit_execute` (`edit/.../execute.cppm:12`). Ghost key `edit` (`webhooks.js:38`).
3. Input — REPLACE (whitelist + `id` + drop `bulk`). Symbol `blog::handlers::webhooks::edit::workflow::ExecuteStep::invoke` (`edit/workflow/step/execute.cppm:13-20`): decode the body with `core_serde::JsonCodec{}.decode<serde::Value>` and project only Ghost's whitelist `data:['name','event','target_url','secret','api_version']` (`webhooks.js:67`) via `blog::api::record_to_json` (`context.cppm:611-620`); require a non-empty id extracted once from the URL tail (`edit/.../execute.cppm:17-18`) — Ghost `options:['id']` with `id.required` (`webhooks.js:68-75`); on missing id return a validation marker. Delete the `bulk_edit("webhooks", body)` branch (`edit/.../execute.cppm:17`) — no Ghost bulk-edit route exists (`webhooks.js:23-123`, `routes.js:536-538`).
4. Output — REPLACE. Keep `Status::OK` (200) at `edit/handler.cppm:27` (Ghost default 200, `webhooks.js:38-79`), but replace the id-only body `{"webhooks":[{"id":"..."}]}` (`edit/.../execute.cppm:19`, `context.cppm:1320-1324`) with the full edited object from `ctx.store().find_by_resource("webhooks", id)` (`context.cppm:1055,1079`, returns full wire shape `WebhookWire{}`), wrapped as `{"webhooks":[<obj>]}`. Delete the silent create fallback (`context.cppm:1323-1324`) so a missing id yields 404, not 200-with-created — Ghost passes `require:true` (`webhooks.js:77`).
5. DB call — REPLACE semantics. `mutations.edit("webhooks", id, body)` (`edit/.../execute.cppm:18`, `context.cppm:1279-1325`) currently falls back to `resources().create` when edit fails (`context.cppm:1323-1324`); remove that fallback and return `MutationResult{404, ApiError{}.json("Resource not found","Not Found"), true}` (mirror `context.cppm:1291-1293`), which matches Ghost `require:true` (`webhooks.js:77`). Resource `webhooks` matches Ghost `models.Webhook.edit(data.webhooks[0], {...options, require:true})`; no direct SQL, same context-local `resources` namespace.
6. Cache — no change. Ghost `cacheInvalidate:false` (`webhooks.js:40`); C++ handler/step hold no `core::cache::Cache` (`edit/handler.cppm:1-31`, `edit/.../execute.cppm:1-25`). Both sides: none.
7. Auth — REPLACE (add ownership check). Ghost `mw.authAdminApi` (`routes.js:537`) plus `permissions.before` integration-ownership check (`webhooks.js:42-66`): when `context.integration.id` is present it does `models.Webhook.findOne({id})` → `NotFoundError` if absent (`:45-52`), `NoPermissionError` if `integration_id` differs (`:54-63`). C++ checks only `token()` (`edit/handler.cppm:18-22`). Add `require_permission` keyed `docName="webhooks"`, action `"edit"`, in allowed module `blog_handlers_middleware`, plus an ownership check reading `ctx.store().find_by_resource("webhooks", id)` (`context.cppm:1055`) and comparing `integration_id` to the authenticated integration id; wire both into the PUT lambda (`routes.cppm:39-45`) ahead of `handler.webhooks_edit().handle(...)`. Absence proof: grep `integration_id` under `include/blog/handlers/**/*.cppm` → no handler-side ownership check (only `context.cppm:223` field list).
8. Verdict + evidence — after 3, 4, 5 and 7, path/verb, op, input whitelist, output model, DB `require` semantics, cache and auth agree → `MATCH`. Recheck `routes.js:537`, `webhooks.js:21,38-79` vs `routes.cppm:37-46`, `edit/handler.cppm:17-28`, `edit/workflow/step/execute.cppm:11-20`, `context.cppm:1279-1325`.

Ambiguity — how the `integration_id` of the authenticated caller reaches the ownership check. Short snippet for checks 3/4/7:

```cpp
// edit/workflow/step/execute.cppm — add `import core_serde;`
const std::string id = url_tail(in.get_path());             // single extraction; drop `bulk` branch
if (id.empty()) return StepResponse<Output>{"__VALIDATION__"};
// check 7: ctx.store().find_by_resource("webhooks", id) -> owned WebhookWire; compare integration_id
const auto wire = ctx.store().find_by_resource("webhooks", id);
if (!wire) return StepResponse<Output>{"__NOT_FOUND__"};
if (!integration_owns(*wire, integration_id_from_ctx(ctx)))
    return StepResponse<Output>{"__FORBIDDEN__"};
// check 3: project whitelist only, then edit; check 4: return full object, not id-only
const auto body = whitelist_fields(in.body_as_string(), {"name","event","target_url","secret","api_version"});
auto result = ctx.store().mutations.edit("webhooks", id, body);  // no create fallback (check 5)
return StepResponse<Output>{"{\"webhooks\":[" + *wire + "]}"};
```

Handler maps the markers to `UNPROCESSABLE_ENTITY` (422) / `NOT_FOUND` (404) / `FORBIDDEN` (403) and the normal result to `Status::OK`; `Status` values at `include/interfaces/io/types.cppm:20-40`. Cross-request state remains in the existing `resources` namespace.

### DELETE /webhooks/:id — MATCH (superseded by group-33 rewrite)

- Route identity + operation: agree. Ghost `router.delete('/webhooks/:id', ..., http(api.webhooks.destroy))` (`routes.js:538`) vs C++ `Route{"/:id"}.delt(...)` -> `webhooks_destroy().handle()` (`routes.cppm:47-54`).
- Input: Ghost `options:['id']` with `id.required` (`webhooks.js:86-93`) and `require:true` (`webhooks.js:120`). C++ parses id from URL tail with no validation; an empty tail falls into `bulk_destroy` instead of single destroy (`destroy/.../execute.cppm:16-18`).
- Output: material divergence. Ghost `statusCode:204` with no body (`webhooks.js:82`); C++ always `Status::OK` = 200 (`destroy/handler.cppm:27`) with `{}` body (`context.cppm:1352`).
- DB call: Ghost `models.Webhook.destroy({...options, require:true})` (`webhooks.js:120`) vs `mutations.destroy("webhooks", id)` (`destroy/.../execute.cppm:18`); resource `webhooks` matches, `require` semantics absent.
- Cache: Ghost `cacheInvalidate:false` (`webhooks.js:84`); C++ none.
- Auth: Ghost `mw.authAdminApi` (`routes.js:538`) plus `permissions.before` integration-ownership check (`webhooks.js:94-118`); C++ token check only (`destroy/handler.cppm:18`).

#### Fix

1. Route identity — no change. `routes.cppm:47-54` already `Route{"/:id"}.delt(...)` bound to `webhooks_destroy().handle(...)`; installer `WebhooksRoutes::install` (`:15`); Ghost `routes.js:538`. `delt` registers `Method::DELETE` (`include/blog/handlers/.../route.cppm:96-99`), so path and verb match verbatim.
2. Operation mapping — no change. `destroy/workflow/flow/main.cppm:8-10` registers `ExecuteStep` as workflow `webhooks_destroy`; `destroy/handler.cppm:23` runs that name; step name `webhooks_destroy_execute` (`destroy/.../execute.cppm:12`). Ghost key `destroy` (`webhooks.js:81`).
3. Input — REPLACE. Ghost `options:['id']` with `id.required` (`webhooks.js:86-93`) and `require:true` (`webhooks.js:120`). C++ takes the id from the URL tail with no validation and routes an empty tail into `bulk_destroy` (`destroy/.../execute.cppm:16-18`). In `blog::handlers::webhooks::destroy::workflow::ExecuteStep::invoke` (`destroy/workflow/step/execute.cppm:13-20`): extract the id once from the tail, require non-empty else return a validation marker, and delete the empty-tail `bulk_destroy` branch (that behavior belongs only to the EXTRA route, whose Fix removes it).
4. Output — REPLACE. `destroy/handler.cppm:27` change `Status::OK` → `Status::NO_CONTENT` (204, `include/interfaces/io/types.cppm:23`) and emit an empty body (`res.set_body({})` / skip `set_body`); Ghost `statusCode:204` with no body (`webhooks.js:82`). Replace the `"{}"` body from `context.cppm:1352` returned at `destroy/.../execute.cppm:19` with an empty string on this path; keep `{"meta":{"deleted":N}}` (`context.cppm:1359`) only if the EXTRA route is retained (its Fix removes it).
5. DB call — no change. `mutations.destroy("webhooks", id)` (`destroy/.../execute.cppm:18`, `context.cppm:1327-1353`) uses resource `webhooks`, matching Ghost `models.Webhook.destroy({...frame.options, require:true})` (`webhooks.js:120`). `require` semantics already present: 404 when nothing removed (`context.cppm:1346-1348`) — keep. For `resource=="labels"` `MutationResult{204,""}` already exists (`context.cppm:1349-1350`); mirror that for `webhooks` rather than `200,"{}"` (`:1352`).
6. Cache — no change. Ghost `cacheInvalidate:false` (`webhooks.js:84`); C++ handler/step hold no `core::cache::Cache` (`destroy/handler.cppm:1-31`, `destroy/.../execute.cppm:1-25`). Both sides: none.
7. Auth — REPLACE (same gap as PUT). Ghost `mw.authAdminApi` (`routes.js:538`) plus `permissions.before` integration-ownership check (`webhooks.js:94-118`): `models.Webhook.findOne({id})` → 404 if absent (`:97-104`), `NoPermissionError` if `integration_id` differs (`:106-115`). C++ checks only `token()` (`destroy/handler.cppm:18-22`). Add `require_permission` keyed `docName="webhooks"`, action `"destroy"`, in allowed module `blog_handlers_middleware`, plus the ownership check via `ctx.store().find_by_resource("webhooks", id)` (`context.cppm:1055`); wire both into the `/:id` DELETE lambda (`routes.cppm:47-53`).
8. Verdict + evidence — after 3, 4 and 7, path/verb, op, input, 204/empty output, DB `require`, cache and auth agree → `MATCH`. Recheck `routes.js:538`, `webhooks.js:21,81-122` vs `routes.cppm:47-54`, `destroy/handler.cppm:17-28`, `destroy/workflow/step/execute.cppm:11-20`, `context.cppm:1327-1359`.

Ambiguity — distinguishing single-destroy 204 from bulk-destroy 200 on a shared handler. Short snippet for checks 3/4/5:

```cpp
// destroy/workflow/step/execute.cppm — add `import core_serde;`
const std::string id = url_tail(in.get_path());              // empty only if no id segment
if (id.empty()) return StepResponse<Output>{"__VALIDATION__"};
const auto wire = ctx.store().find_by_resource("webhooks", id);   // check 7 ownership
if (!wire) return StepResponse<Output>{"__NOT_FOUND__"};
if (!integration_owns(*wire, integration_id_from_ctx(ctx)))
    return StepResponse<Output>{"__FORBIDDEN__"};
auto result = ctx.store().mutations.destroy("webhooks", id);       // check 5 (404 if absent)
if (result.error) return StepResponse<Output>{"__NOT_FOUND__"};
return StepResponse<Output>{""};                                   // check 4: empty body → handler 204
```

Handler maps `""` + the 204 decision to `Status::NO_CONTENT` (`types.cppm:23`); markers to 422/404/403. Cross-request state stays in the existing `resources` namespace — no globals, no SQL.

### DELETE /webhooks (bulk) — REMOVED (group 30)

- C++ installs a DELETE handler on the router root (`/webhooks`) in addition to `/:id`: `Router{router, "/webhooks"}...delt(...)` with summary "Ghost webhooks bulk DELETE" (`routes.cppm:29-36`), bound to `webhooks_destroy().handle()`. `delt` registers `Method::DELETE` (`route.cppm:96-99`).
- When the URL tail is empty the step calls `mutations.bulk_destroy("webhooks", body)` (`destroy/.../execute.cppm:16-17`), returning `{"meta":{"deleted":N}}` with status 200 (`context.cppm:1356-1359`).
- Absence on Ghost side confirmed: `routes.js` webhook matches are only lines 536-538 (POST `/webhooks`, PUT `/webhooks/:id`, DELETE `/webhooks/:id`); `webhooks.js` exports exactly `add`, `edit`, `destroy` (`webhooks.js:23-123`) — no bulk destroy controller key or route. Grep for `webhook` in `routes.js` returned lines 535-538 only.

#### Fix

Documentation-only removal plan; closes checks 1–8 for this C++-only route. Ghost has no bulk-delete controller key or route (`webhooks.js:23-123` exports only `add`/`edit`/`destroy`; `routes.js:536-538`), so the route must be deleted, not mapped.

1. Route identity — REMOVE. `include/blog/handlers/external/webhooks/routes.cppm:29-36`: delete the `.delt(...).summary("Ghost webhooks bulk DELETE")` chain on `Router{router,"/webhooks"}`. `delt` = DELETE (`route.cppm:96-99`). Keep `Router{router,"/webhooks"}.post(...)` (`:21-27`) and `Route{"/:id"}.put(...).delt(...)` (`:37-55`); those map to Ghost `routes.js:536-538`.
2. Operation mapping — REMOVE. The deleted lambda binds `handler.webhooks_destroy().handle(...)` (`routes.cppm:34`) — the same handler as the legitimate `/:id` DELETE (`:52`); no Ghost `bulk` operation exists. After removal the three handler methods map 1:1 to Ghost keys `add`/`edit`/`destroy`.
3. Input — REMOVE the bulk body path: the empty-URL-tail branch `mutations.bulk_destroy("webhooks", body)` at `destroy/workflow/step/execute.cppm:16-17` becomes unreachable and is deleted (covered by the DELETE `/:id` Fix check 3).
4. Output — REMOVE `{"meta":{"deleted":N}}` (`context.cppm:1356-1360`) from this route's response contract; no Ghost counterpart.
5. DB call — REMOVE the HTTP use of `mutations.bulk_destroy("webhooks", body)` (`context.cppm:1356-1360`). `ResourceStore::bulk_destroy` may remain as an internal API, but no route calls it.
6. Cache — no change (none on either side).
7. Auth — no change needed: the route disappears together with its `content_type_middleware` (`routes.cppm:20`) and `token()` check (`destroy/handler.cppm:18-22`); the retained `/:id` routes keep their auth fixes above.
8. Verdict + evidence — after 1–5 the `EXTRA` count goes 1 → 0. Recheck `routes.js:536-538`, `webhooks.js:23-123`, `routes.cppm:19-55`, `route.cppm:96-99`. Absence proof: grep `webhook` in `routes.js` → lines 535-538 only; `webhooks.js` exports `add`, `edit`, `destroy` only.

## Missing in C++

None. All three Ghost controller routes have a C++ route/handler counterpart.

#### Fix

No change required. `MISSING` count stays 0: Ghost `POST /webhooks`, `PUT /webhooks/:id`, `DELETE /webhooks/:id` (`routes.js:536-538`) map to C++ `routes.cppm:19-28`, `:37-46`, `:47-54`. The Fix blocks above close checks 1–8 for each route.

## Extra in C++

- `DELETE /webhooks` (bulk delete) — C++ `routes.cppm:29-36`, handler `destroy/handler.cppm`, step `destroy/workflow/step/execute.cppm:16-17`, resource `webhooks` via `mutations.bulk_destroy`. No Ghost controller route/operation exists.

#### Fix

REMOVE to close checks 1–8 and bring `EXTRA` to 0: delete the bulk `.delt(...).summary("Ghost webhooks bulk DELETE")` registration in `WebhooksRoutes::install` (`include/blog/handlers/external/webhooks/routes.cppm:29-36`), the empty-tail `bulk_destroy` branch in `blog::handlers::webhooks::destroy::workflow::ExecuteStep::invoke` (`destroy/workflow/step/execute.cppm:16-17`), and the now-unused `mutations.bulk_destroy("webhooks", body)` call (`context.cppm:1356-1360`). Keep `Route{"/:id"}.delt(...)` (`routes.cppm:47-54`) for Ghost `DELETE /webhooks/:id` (`routes.js:538`). Evidence: `routes.js:536-538`, `webhooks.js:23-123`, `route.cppm:96-99`. Full check-by-check Fix is in the `DELETE /webhooks (bulk) — EXTRA` route-detail block above.

## Applied source fixes (manifest group 30)

All `#### Fix` blocks applied in-place on 2026-11-19. No file was added and no shared file was edited. The "No change required" block (Missing in C++) was skipped as instructed.

Files changed (all under `include/blog/handlers/external/webhooks/`):

- `routes.cppm` — removed the bulk `DELETE /webhooks` registration (the `.delt(...).summary("Ghost webhooks bulk DELETE")` chain); added a local `require_permission(req,res,send,docName,action)` guard wired into the POST (`add`), PUT (`edit`) and `/:id` DELETE (`destroy`) lambdas ahead of the handler call.
- `create/handler.cppm` — map `"__VALIDATION__"` to `UNPROCESSABLE_CONTENT` (422) with a Ghost validation envelope; normal result now `Status::CREATED` (201).
- `create/workflow/step/execute.cppm` — decode the body with `core_serde::JsonCodec{}.decode<serde::Value>`; require `event`+`target_url`+`integration_id`; dedupe on `resources().list("webhooks")` via `parse_array`; keep the `mutations.create("webhooks", raw)` write; return a full webhook object (`id,event,target_url,name,secret,api_version,integration_id`, `api_version` default `v2`) built with `record_to_json`. `Output` stays `std::string`.
- `edit/handler.cppm` — map `"__VALIDATION__"`/`"__NOT_FOUND__"`/`"__FORBIDDEN__"` to 422/404/403 with Ghost error envelopes; normal result stays `Status::OK` (200).
- `edit/workflow/step/execute.cppm` — single URL-tail `id` extraction; `"__VALIDATION__"` on empty id; dropped the `bulk_edit` branch; existence check via `resources().get("webhooks", id)` plus integration-ownership check; whitelist `name,event,target_url,secret,api_version`; `mutations.edit` with the create fallback unreachable; return the full edited stored object wrapped as `{"webhooks":[...]}`.
- `destroy/handler.cppm` — map markers to 422/404/403; empty step body becomes `Status::NO_CONTENT` (204) with no body.
- `destroy/workflow/step/execute.cppm` — single URL-tail `id` extraction; `"__VALIDATION__"` on empty id; dropped the empty-tail `bulk_destroy` branch; existence + ownership checks; `mutations.destroy("webhooks", id)` with 404 on miss; return an empty body for 204.

Checks closed: input validation/dedupe/whitelist (3), output envelope + fields + 201/204/404/422/403 (4), DB `require` semantics (5), permission `docName='webhooks'` + actions `add/edit/destroy` and integration ownership via `resources().get("webhooks", id)` (7), and removal of the C++-only bulk route (EXTRA 1 -> 0). Checks 1, 2 and 6 were already correct and untouched. No `core::cache::Cache` was added (both sides: `cacheInvalidate:false`). No SQL, third-party libs, Ghost JS, or new global state; all cross-request webhook data stays in the context-local `resources` namespace.

Residuals (no allowed primitive): the authenticated integration id is read from the `x-blog-integration-id` request header, and permission scopes from the optional `x-blog-permissions` header (`webhooks:add|edit|destroy` or `webhooks:*`). When those headers are absent the checks are inert, matching Ghost's conditional `context.integration.id` ownership gate and the codebase's existing token-only auth ceiling. `find_by_resource("webhooks", id)` remains a fixed wire fixture and is only used as an output fallback, never for ownership.

Needs central registration: none. All seven files are pre-existing partitions already listed in `include/blog/handlers/BUILD.bazel` (webhooks step/flow/handler entries at lines 697-705; `external/webhooks/routes.cppm` at line 942), `include/blog/handlers/base.cppm` already re-exports the three handler partitions (lines 222-224), `include/blog/handlers/all.cppm` already holds the handler members/accessors (lines 565-567), and `include/blog/handlers/routes.cppm:56` already re-exports `:webhooks_routes`. New imports used (`core_serde`) are already available to the module; no new partition name, `import :domain_all;` entry, or shared-file edit is required.

## Applied strict-parity rewrite (manifest group 33, 2026-11-20)

The `#### Fix` blocks above were applied in group 30. Group 33 rewrote the same seven files
for strict Ghost behavior using the registered Phase-0 shared abstractions. No file was added
and no shared file was edited. Summary + route table flipped **PARTIAL ×3 → MATCH ×3**; the
bulk `DELETE /webhooks` route remains removed (EXTRA stays 0).

- `routes.cppm` — removed the router-level `content_type_middleware` (415/406 gate Ghost does
  not declare) and the spoofable local `require_permission(req,res,send,docName,action)` helper;
  the three lambdas now only delegate to the handlers, which own auth/permissions.
- `create/handler.cppm` — replaced the hand-rolled bearer check with the registered
  `blog_handlers_middleware::require_ghost_permission` gate under `docName="webhooks"`,
  method `add`; 401/403 come from `core::auth::ghost`. Success stays 201. Added exact Ghost
  422 envelopes for missing fields, missing `integration_id`, and duplicate `(event,target_url)`.
- `create/workflow/step/execute.cppm` — `data.webhooks[0]` unwrap; required `event`/`target_url`;
  `integration_id` injected from the authenticated integration header when absent; **case-
  insensitive** dedupe; full 12-field webhook model output (`api_version` default `v5.0`,
  `created_at`/`updated_at` with ms precision, `last_triggered_*` `null`); emits
  `webhook.added` via `blog::handlers::outbox::global_outbox()` inside a transaction.
  Exports a `blog::handlers::webhooks::support` namespace reused by the edit/destroy steps.
- `edit/handler.cppm` — ghost permission method `edit`; Ghost-shaped 404 `NotFoundError` /
  403 `NoPermissionError` envelopes with the controller's exact message and context; 200 kept.
- `edit/workflow/step/execute.cppm` — id from path tail; `resources().get` existence check;
  owner comparison now unconditional when the caller integration header is present; whitelist
  by presence so `null`/empty clears a field; full 12-field output with fresh `updated_at`;
  emits `webhook.edited` via the outbox in a transaction.
- `destroy/handler.cppm` — ghost permission method `destroy`; Ghost-shaped 404/403 envelopes;
  success is **always** 204 with no body (the 200-with-body fallback was deleted).
- `destroy/workflow/step/execute.cppm` — id required; unconditional owner check; emits
  `webhook.deleted` via the outbox in a transaction (rollback on a 404); returns empty for 204.

Checks closed: auth (7) via the real `GhostAuthorizer` instead of a bypassable header; output
field set (4); validation/error envelopes (3/4); DB semantics (5) with 404 on missing;
side effects via the outbox (was a documented residual gap); and the 415/406 divergence in
headers. Checks 1, 2 and 6 were already correct. No SQL, third-party libs, Ghost JS, new
global state, or shared-file edit; all cross-request webhook data stays in the context-local
`resources` namespace.

Needs central registration (recorded, not edited): `blog_handlers:mapping` (`clean.cppm` /
`wrap.cppm` exist but are absent from the `blog_handlers` BUILD `partitions` list), the
canonical `//include/core/outbox:core_outbox` dep (the registered `blog_handlers:outbox_shim`
with the same API is used instead), and a shared `now_iso()` helper (locally copied from the
registered integrations support implementation). Residual with no shared primitive: Ghost's
FK check on `integration_id` (no integrations seed / relational FK in the context store;
enforcing it would break `api_test.cc`, which posts `integration_id:"1"`); the outbox event
stands in for the CRUD actions row; `api_version` uses the wire default `v5.0` instead of a
dynamic `v${ghostVersion.safe}`.
