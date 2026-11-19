# Parity: labels

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/labels.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/labels/routes.cppm`
- Checked: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 4 |
| MISMATCH | 2 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/labels` | `browse` | GET `/labels` | `labels_browse()` | Ghost `options: include,filter,fields,limit,order,page`, `include` ∈ `['count.members']`; C++ reads none (`Input=IRequest&`, no query parse) | Ghost `Label.findPage` `{data,meta}`; C++ fixed `{"labels":rows,"meta":{"pagination":{limit:15,page:1,pages:1,prev:null,next:null,total:0}}}` + status 200 | Ghost `models.Label.findPage(restrictAdminApiQueryOptions(...))`; C++ `json_for("labels", {})` empty params, no filter/paginate | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | PARTIAL | G `routes.js:294`, `labels.js:32-48`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-27`, `browse/.../execute.cppm:13-17` |
| GET `/labels/:id` | `read` | GET `/labels/:id` | `labels_read_by_id()` | Ghost `data: id,slug`, `options: include,filter,fields`; C++ ignores `:id` (hardcoded `"1"`) | Ghost single label model, 404 if missing; C++ `{"labels":[one]}` or `{"labels":[]}`, status 200 (no 404) | Ghost `models.Label.findOne(frame.data, ...)`; C++ `find_by_resource("labels", "1")` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | MISMATCH | G `routes.js:295`, `labels.js:50-77`; C++ `routes.cppm:37-46`, `read_by_id/handler.cppm:17-27`, `read_by_id/.../execute.cppm:16-18` |
| GET `/labels/slug/:slug` | `read` | GET `/labels/slug/:id` | `labels_read_by_slug()` | Ghost `data: id,slug` (slug path); C++ ignores slug (`json_for("labels", {})`) | Ghost single label model, 404 if missing; C++ full list with fixed pagination meta (same as browse), status 200 | Ghost `models.Label.findOne(frame.data, ...)` by slug; C++ `json_for("labels", {})` returns all rows, no slug filter | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | MISMATCH | G `routes.js:296`, `labels.js:50-77`; C++ `routes.cppm:64-76`, `read_by_slug/handler.cppm:17-27`, `read_by_slug/.../execute.cppm:16-17` |
| POST `/labels` | `add` | POST `/labels` | `labels_create()` | Ghost `data.labels[0]`, `options: include`; C++ raw `in.body_as_string()` (`"{}"` when empty), no `data.labels[0]` unwrap/include parse | Ghost created label, status 201, duplicate→ValidationError; C++ raw mutation body, status 200, no duplicate handling | Ghost `models.Label.add(...).catch(handleDuplicateNameError)`; C++ `mutations.create("labels", body).body()` | Ghost `cacheInvalidate:false`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | PARTIAL | G `routes.js:297`, `labels.js:79-96`; C++ `routes.cppm:29-36`, `create/handler.cppm:17-27`, `create/.../execute.cppm:15` |
| PUT `/labels/:id` | `edit` | PUT `/labels/:id` | `labels_edit()` | Ghost `options: id,include`, `id` required, `data.labels[0]`; C++ empty id `""`, raw body, no id/include parse | Ghost edited label, 404 if missing, sets `X-Cache-Invalidate: /*` on change; C++ raw mutation body, status 200, no invalidate header | Ghost `models.Label.edit(...).catch(handleDuplicateNameError)`; C++ `mutations.edit("labels", "", body).body()` | Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate: /*`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | PARTIAL | G `routes.js:298`, `labels.js:98-131`; C++ `routes.cppm:47-54`, `edit/handler.cppm:17-27`, `edit/.../execute.cppm:16` |
| DELETE `/labels/:id` | `destroy` | DELETE `/labels/:id` (`.delt`) | `labels_destroy()` | Ghost `options: id`, `require:true`; C++ empty id `""`, no id option | Ghost empty body, status 204; C++ mutation body, status 200 | Ghost `models.Label.destroy({...frame.options, require:true})`; C++ `mutations.destroy("labels", "").body()` | Ghost `cacheInvalidate:true`; C++ none | Ghost `mw.authAdminApi`+`permissions:true`; C++ `content_type_middleware`+token-presence check | PARTIAL | G `routes.js:299`, `labels.js:133-153`; C++ `routes.cppm:55-62`, `destroy/handler.cppm:17-27`, `destroy/.../execute.cppm:16` |

## Route details

### GET `/labels` — PARTIAL

- Route identity: Ghost `router.get('/labels', ..., http(api.labels.browse))` (`routes.js:294`); C++ `Router{router, "/labels"}.get(...)` (`routes.cppm:19-28`). Match.
- Operation: Ghost `browse` (`labels.js:32`); C++ `handler.labels_browse()` (`routes.cppm:26`, registry `all.cppm:474`). Match.
- Input: Ghost options `include,filter,fields,limit,order,page` with `include` validated against `['count.members']` (`labels.js:11,36-43`); C++ `Input=IRequest&` but step parses no query params (`browse/.../execute.cppm:11-15`). Divergent: all six Ghost params absent in C++.
- Output: Ghost `models.Label.findPage` result with real pagination (`labels.js:46`); C++ hardcodes `limit:15,page:1,pages:1,total:0` and `prev/next:null` (`browse/.../execute.cppm:16`), status 200 (`browse/handler.cppm:27`). Divergent meta.
- DB call: Ghost `models.Label.findPage(restrictAdminApiQueryOptions(frame.options))` (`labels.js:46`); C++ `json_for("labels", {})` with empty params, no filtering/pagination (`browse/.../execute.cppm:15`). Resource string `"labels"` agrees; semantics differ.
- Cache: Ghost `cacheInvalidate:false` (`labels.js:33-35`); C++ none (no `core::cache::Cache` use in labels steps; cache only wired for members/emails/automations in `all.cppm:836-839`). Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:294`) + `permissions:true` (`labels.js:44`); C++ `.use(content_type_middleware)` (`routes.cppm:20`) + inline token-presence check returning 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`). No permissions/docName/unsafeAttrs model in C++.

### GET `/labels/:id` — MISMATCH

- Route identity: Ghost `router.get('/labels/:id', ..., http(api.labels.read))` (`routes.js:295`); C++ `Route{"/:id"}.get(...)` (`routes.cppm:37-46`). Match.
- Operation: Ghost `read` (`labels.js:50`); C++ `handler.labels_read_by_id()` (`routes.cppm:44`, registry `all.cppm:475`). Match.
- Input: Ghost `data:['id','slug']` + options `include,filter,fields` (`labels.js:54-61`); C++ step ignores the path id entirely and calls `find_by_resource("labels", std::string{}.empty()?"1":std::string{})`, i.e. hardcoded `"1"` (`read_by_id/.../execute.cppm:16`). Materially broken id lookup.
- Output: Ghost returns single label model, throws `NotFoundError` when absent (`labels.js:64-76`); C++ wraps result in `{"labels":[...]}` or returns `{"labels":[]}` with status 200 and no 404 (`read_by_id/.../execute.cppm:17-18`, `read_by_id/handler.cppm:27`). Material data-shape/status divergence.
- DB call: Ghost `models.Label.findOne(frame.data, ...)` (`labels.js:65-68`); C++ `find_by_resource("labels", "1")` (`read_by_id/.../execute.cppm:16`). Resource agrees; id source diverges.
- Cache: Ghost `cacheInvalidate:false` (`labels.js:51-53`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:295`) + `permissions:true` (`labels.js:63`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`read_by_id/handler.cppm:18-22`). Divergent.

### GET `/labels/slug/:slug` — MISMATCH

- Route identity: Ghost `router.get('/labels/slug/:slug', ..., http(api.labels.read))` (`routes.js:296`); C++ `Router{router, "/slug"}.add_route(Route{"/:id"}.get(...))` (`routes.cppm:64-76`). Path matches after normalizing `:slug` ↔ `:id` placeholder names.
- Operation: Ghost `read` by slug (`labels.js:50`); C++ `handler.labels_read_by_slug()` (`routes.cppm:73`, registry `all.cppm:476`). Match.
- Input: Ghost `data:['id','slug']` with slug supplied via path (`labels.js:55`); C++ step never reads the slug and calls `json_for("labels", {})` (`read_by_slug/.../execute.cppm:16`). No slug filter.
- Output: Ghost single label model + 404 when missing (`labels.js:64-76`); C++ returns the full label list with the same fixed pagination envelope as `browse` and status 200 (`read_by_slug/.../execute.cppm:17`, `read_by_slug/handler.cppm:27`). Material data-shape divergence.
- DB call: Ghost `models.Label.findOne(frame.data, ...)` (`labels.js:65-68`); C++ `json_for("labels", {})` returns all rows (`read_by_slug/.../execute.cppm:16`). Material operation divergence.
- Cache: Ghost `cacheInvalidate:false` (`labels.js:51-53`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:296`) + `permissions:true` (`labels.js:63`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`read_by_slug/handler.cppm:18-22`). Divergent.

### POST `/labels` — PARTIAL

- Route identity: Ghost `router.post('/labels', ..., http(api.labels.add))` (`routes.js:297`); C++ `.post(...)` on `/labels` (`routes.cppm:29-36`). Match.
- Operation: Ghost `add` (`labels.js:79`); C++ `handler.labels_create()` (`routes.cppm:34`, registry `all.cppm:477`). Match.
- Input: Ghost reads `frame.data.labels[0]` with `options:['include']` validated (`labels.js:84-95`); C++ sends raw `in.body_as_string()` (`"{}"` fallback) with no envelope unwrap and no include parse (`create/.../execute.cppm:15`). Divergent body shape.
- Output: Ghost returns created model with `statusCode:201` and converts unique-constraint violations to `ValidationError` (`labels.js:80,20-26,94`); C++ returns raw `mutations.create(...).body()` with status 200 and no duplicate handling (`create/.../execute.cppm:15`, `create/handler.cppm:27`). Divergent status and error shape.
- DB call: Ghost `models.Label.add(frame.data.labels[0], frame.options)` (`labels.js:94`); C++ `mutations.create("labels", body)` (`create/.../execute.cppm:15`). Resource `"labels"` agrees.
- Cache: Ghost `cacheInvalidate:false` (`labels.js:81-83`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:297`) + `permissions:true` (`labels.js:92`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`create/handler.cppm:18-22`). Divergent.

### PUT `/labels/:id` — PARTIAL

- Route identity: Ghost `router.put('/labels/:id', ..., http(api.labels.edit))` (`routes.js:298`); C++ `.put(...)` on `Route{"/:id"}` (`routes.cppm:47-54`). Match.
- Operation: Ghost `edit` (`labels.js:98`); C++ `handler.labels_edit()` (`routes.cppm:52`, registry `all.cppm:478`). Match.
- Input: Ghost `options:['id','include']`, `id` required, uses `frame.data.labels[0]` (`labels.js:102-115`); C++ passes empty id `std::string{}` and raw body, ignores `:id` and all options (`edit/.../execute.cppm:16`). Divergent.
- Output: Ghost returns edited model, throws `NotFoundError` if absent, and sets `X-Cache-Invalidate: /*` when `model.wasChanged()` (`labels.js:115-130`); C++ returns raw `mutations.edit(...).body()` with status 200 and no invalidate header (`edit/.../execute.cppm:16`, `edit/handler.cppm:27`). Divergent.
- DB call: Ghost `models.Label.edit(frame.data.labels[0], frame.options)` (`labels.js:115`); C++ `mutations.edit("labels", "", body)` (`edit/.../execute.cppm:16`). Resource agrees; id/body diverge.
- Cache: Ghost `cacheInvalidate:false` + dynamic `X-Cache-Invalidate: /*` on change (`labels.js:99-101,125-127`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:298`) + `permissions:true` (`labels.js:113`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`edit/handler.cppm:18-22`). Divergent.

### DELETE `/labels/:id` — PARTIAL

- Route identity: Ghost `router.delete('/labels/:id', ..., http(api.labels.destroy))` (`routes.js:299`); C++ `.delt(...)` on `Route{"/:id"}` (`routes.cppm:55-62`) — delete verb. Match.
- Operation: Ghost `destroy` (`labels.js:133`); C++ `handler.labels_destroy()` (`routes.cppm:60`, registry `all.cppm:479`). Match.
- Input: Ghost `options:['id']` with `id` required and `require:true` (`labels.js:138-151`); C++ passes empty id `std::string{}`, reads no id option (`destroy/.../execute.cppm:16`). Divergent.
- Output: Ghost empty body with `statusCode:204` (`labels.js:134`); C++ returns `mutations.destroy("labels", "").body()` with status 200 (`destroy/.../execute.cppm:16`, `destroy/handler.cppm:27`). Divergent status/body.
- DB call: Ghost `models.Label.destroy({...frame.options, require:true})` (`labels.js:151`); C++ `mutations.destroy("labels", "")` (`destroy/.../execute.cppm:16`). Resource agrees; id diverges.
- Cache: Ghost `cacheInvalidate:true` (`labels.js:135-137`); C++ none. Divergent.
- Auth: Ghost `mw.authAdminApi` (`routes.js:299`) + `permissions:true` (`labels.js:149`); C++ `content_type_middleware` (`routes.cppm:20`) + token-presence check (`destroy/handler.cppm:18-22`). Divergent.

## Missing in C++

None. All 6 Ghost label routes have a C++ route in `include/blog/handlers/admin/labels/routes.cppm`.

## Extra in C++

None. The C++ folder installs exactly 6 routes (`routes.cppm:19-77`), matching the 6 Ghost label routes; no additional C++ label routes exist. LabelsRoutes is installed once via `include/blog/handlers/routes.cppm:99`.
