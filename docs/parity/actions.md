# Parity: actions

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/actions.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/actions/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 1 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /actions | browse | GET /actions | `actions_browse()` → `blog::handlers::actions::browse::BrowseHandler` | Ghost: `page,limit,fields,include,filter` via `restrictAdminApiQueryOptions`; C++: none parsed (`json_for("actions", {})`) | Ghost: model `{data,meta}` w/ real pagination, `cacheInvalidate:false`; C++: `{"actions":[...],"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}`, always 200 | Ghost: `models.Action.findPage(...)`; C++: `ctx.store().json_for("actions", {})` (no filter/paging) | none on both sides | Ghost: `mw.authAdminApi` + `permissions:true`/`docName:'actions'`; C++: `content_type_middleware` + token check (401 if empty) | PARTIAL | Ghost `admin/routes.js:544`, `actions.js:6,8-16`; C++ `external/actions/routes.cppm:19-28`, `all.cppm:553,802`, `browse/handler.cppm:17-28`, `browse/workflow/step/execute.cppm:11-17` |

## Route details

### GET /actions — PARTIAL

- **Route identity — MATCH**: Ghost `router.get('/actions', ..., http(api.actions.browse))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:544`; C++ `Router{router, "/actions"}` + `.get(...)` at `include/blog/handlers/external/actions/routes.cppm:19-28`. Verbatim path `/actions`, verb GET on both.
- **Operation mapping — MATCH**: Ghost controller key `browse` (`actions.js:8`); C++ binds `handler.actions_browse()` (`external/actions/routes.cppm:26`), accessor `actions_browse()` returning `blog::handlers::actions::browse::BrowseHandler` (`include/blog/handlers/all.cppm:553`, member at `:802`).
- **Input parity — PARTIAL (divergent)**: Ghost `options: ['page','limit','fields','include','filter']` (`actions.js:12`) passed through `restrictAdminApiQueryOptions(frame.options)` (`actions.js:2,15`). C++ declares `using Input = interfaces::io::IRequest&` (`browse/workflow/step/execute.cppm:11`) but never parses the request; the store call gets empty params `json_for("actions", {})` (`execute.cppm:16`). Params present on Ghost side only: `page`, `limit`, `fields`, `include`, `filter`.
- **Output parity — PARTIAL (divergent)**: Ghost `models.Action.findPage(...)` (`actions.js:15`) returns real `{data, meta}` with genuine pagination via the serializer; `headers: {cacheInvalidate: false}` (`actions.js:9-11`). C++ returns a hardcoded envelope `{"actions":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` (`execute.cppm:17`) — `limit/page/pages/prev/next/total` are constants and `total` is always `0`. Envelope key `actions` aligns with Ghost's docName, but pagination is not real. Status: C++ always sets `Status::OK` (`browse/handler.cppm:27`); Ghost has no custom `statusCode`.
- **DB call parity — PARTIAL (divergent)**: Ghost `models.Action.findPage(restrictAdminApiQueryOptions(...))` (`actions.js:15`) — real model with filtering/pagination. C++ `ctx.store().json_for("actions", {})` (`execute.cppm:16`) — same `actions` resource string, but no real filtering or pagination performed.
- **Cache parity — MATCH (none)**: Ghost explicitly disables cache invalidation via `headers.cacheInvalidate: false` (`actions.js:9-11`). C++ has no `core::cache::Cache` read, namespace (`members:*`/`emails:*`/`automations:*`), `ResourceStore` tag, TTL, or eviction in the handler/step (`browse/handler.cppm:17-28`, `execute.cppm:13-18`). Both sides: no cache behavior.
- **Auth / permissions — PARTIAL (mechanism differs)**: Ghost applies `mw.authAdminApi` on the route (`admin/routes.js:544`) plus API-framework permissions `permissions: true` with `docName: 'actions'` (`actions.js:6,13`). C++ applies `blog::handlers::middleware::content_type_middleware` (`external/actions/routes.cppm:20`) and a token-presence check inside `handle()` that returns `401 {"error":"Unauthorized"}` when `blog::handlers::middleware::token(req)` is empty (`browse/handler.cppm:18-22`). Both gate the route on auth, but C++ uses bearer-token presence only, with no permission/docName check.

Overall **PARTIAL**: route, verb, operation, and resource agree, but C++ parses none of the Ghost query options, returns hardcoded pagination (`total:0`), performs no filtering/pagination in the store call, and uses a weaker auth check.

## Missing in C++

None. The sole Ghost route `GET /actions` (`admin/routes.js:544`) has a C++ counterpart (`external/actions/routes.cppm:19-28`).

## Extra in C++

None. The matched folder `include/blog/handlers/external/actions/` contains only the `GET /actions` route. The other `/actions` string in `include/blog/handlers/membership/automations/routes.cppm:56` is a nested sub-path of `GET /automations/:automation_id/actions/:action_id/links` (`:56-66`), a different path owned by a different Ghost controller (`automationActionLinks.browse`, `admin/routes.js:303-307`), not an extra standalone `/actions` route.
