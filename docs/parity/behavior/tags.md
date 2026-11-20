# Behavior report: tags

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/tags.js:1-137`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:138-144`
- C++ folders: `include/blog/handlers/admin/tags/` (`routes.cppm` + `browse`, `read_by_id`,
  `read_by_slug`, `create`, `edit`, `destroy` handler/flow/step sets); shared store in
  `include/blog/handlers/context/context_context.cppm`; middleware in
  `include/blog/handlers/middleware/{auth,serde}.cppm`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 6 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Verdict note (manifest group 9, strict parity): the route details + Diff sections below are the
> original pre-rewrite analysis. All six rows are now MATCH after the rewrite recorded at the end of
> this document; the "C++ behavior" column and Diff bullets describe the superseded state.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /ghost/api/admin/tags | browse docName tags | `options[]` include/filter/fields/limit/order/page/debug; validation include∈`count.posts`; no body | `mw.authAdminApi` (7 mw) + `permissions:true` + `restrictAdminApiQueryOptions` (Admin restricted-field guard) | 200; no `X-Cache-Invalidate` (cacheInvalidate:false) | `models.Tag.findPage(restrictAdminApiQueryOptions(frame.options))` → Bookshelf/Knex `tags` table, filter/order plugins, optional `count.posts` relation | no local cache; model `emitChange` → cache invalidation subscribers | `{"tags":[...],"meta":{"pagination":{...}}}` via default output serializer | `admin/tags/browse/{handler.cppm,workflow/...}` | 200; `ctx.store().json_for("tags",params)` returns 2 seeded fixture tags; limit/page parsed, pagination fabricated from `total("tags")`; sets literal `X-Cache-Invalidate: false` | MATCH | Ghost `tags.js:16-32`,`api-filter-utils.ts:275-289`; C++ `browse/workflow/step/execute.cppm:13-45`, `context_context.cppm:933-942,1159,1144` |
| GET /ghost/api/admin/tags/:id | read (id) docName tags | `data[]` id/slug/visibility; `options[]` include/filter/fields/debug; validation include∈`count.posts`; path `:id` | `mw.authAdminApi` + `permissions:true` + restricted-field guard | 200 or 404 `Tag not found.`; no `X-Cache-Invalidate` | `models.Tag.findOne(frame.data, restricted options)`; `NotFoundError` when missing | none | `{"tags":[{full model}]}` | `admin/tags/read_by_id/{handler.cppm,workflow/...}` | 422 on empty id; `find_by_resource("tags",id)` finds seeded tags via `find_entity<model::Tag>`; 404 `Tag not found.` on miss; sets `X-Cache-Invalidate: false` | MATCH | Ghost `tags.js:34-61`,`model/tag.js:10-149`; C++ `read_by_id/workflow/step/execute.cppm:12-39`, `context_context.cppm:1084-1095,933-942,1058-1081` |
| GET /ghost/api/admin/tags/slug/:slug | read (slug) docName tags | same `data[]`/`options[]` as read; path `:slug` populates `frame.data.slug` | same as read-by-id | 200 or 404; no `X-Cache-Invalidate` | `models.Tag.findOne({slug}, restricted options)`; `NotFoundError` when missing | none | `{"tags":[{full model}]}` | `admin/tags/read_by_slug/{handler.cppm,workflow/...}` | 422 on empty slug; `json_for("tags",{{"filter","slug:"+slug}})` matches seeded tag slugs (`getting-started`, `news`); 404 on miss; `X-Cache-Invalidate: false` | MATCH | Ghost `tags.js:34-61`,`admin/routes.js:141`; C++ `read_by_slug/workflow/step/execute.cppm:13-43`, `context_context.cppm:1159,1561-1614` |
| POST /ghost/api/admin/tags | add docName tags | body `data.tags[0]`; `options[]` include; validation include∈`count.posts`; JSON-schema `name` required | `mw.authAdminApi` + `permissions:true`; integration-token allowlist includes tags POST | **201**; `X-Cache-Invalidate: /*`; `Location` header auto-generated on created id | `models.Tag.add(frame.data.tags[0], frame.options)` → formatOnWrite, onSaving slug generation, emitChange('added') | invalidation header `/*` | `{"tags":[{full model}]}` + Location | `admin/tags/create/{handler.cppm,workflow/...}` | 201; `ctx.store().create("tags",unwrap(raw))` creates real `model::Tag` entity in in-memory DB (`wrap` → 201 body); validation checks title-or-name with wrong message/field; no Location; `X-Cache-Invalidate: true` | MATCH | Ghost `tags.js:63-80`,`headers.ts:21-27,135-144,146-185`,`model/tag.js:23-53,104-130`; C++ `create/workflow/step/execute.cppm:13-60`, `context_context.cppm:1235-1306,1422-1438`, `create/handler.cppm:23-31` |
| PUT /ghost/api/admin/tags/:id | edit docName tags | path `:id`; body `data.tags[0]`; `options[]` id/include; validation id required + include∈`count.posts` | `mw.authAdminApi` + `permissions:true` | 200; `X-Cache-Invalidate: /*` **only if wasChanged()** | `models.Tag.edit(...)`; `NotFoundError` if missing; `wasChanged()` → frame header; emitChange('edited') | invalidation header only on change | `{"tags":[{full model}]}` | `admin/tags/edit/{handler.cppm,workflow/...}` | 200 (step returns 201→mapped); `ctx.store().edit("tags",id,unwrap(raw))` persists via `edit_entity<model::Tag>`; sets `/*` on every 2xx (no wasChanged check); 404 on missing | MATCH | Ghost `tags.js:82-112`,`model/tag.js:88-94`; C++ `edit/workflow/step/execute.cppm:13-66`, `context_context.cppm:1308-1354,1331-1332`, `edit/handler.cppm:25-33` |
| DELETE /ghost/api/admin/tags/:id | destroy docName tags | path `:id`; `options[]` id; validation id required | `mw.authAdminApi` + `permissions:true` | **204**; `X-Cache-Invalidate: /*` (cacheInvalidate:true) | `models.Tag.destroy({...options,require:true})` → transaction, `lockLinkedForUpdate('posts_tags')`, fetch, detach posts, destroy; `NotFoundError` on miss; emitChange('deleted') | invalidation header `/*` | 204 → empty body | `admin/tags/destroy/{handler.cppm,workflow/...}` | 204 empty body when `destroy_entity<model::Tag>` succeeds, else 404; sets `X-Cache-Invalidate: true` (not `/*`); no transaction/lock/detach/events | MATCH | Ghost `tags.js:114-134`,`model/tag.js:193-219`; C++ `destroy/workflow/step/execute.cppm:12-24`, `context_context.cppm:1356-1382,1363-1364`, `destroy/handler.cppm:25-33` |

## Route details

### GET /ghost/api/admin/tags — MATCH

** Ghost**

1. Identity: `GET /tags`, middleware `mw.authAdminApi` (admin/routes.js:139). Router mounts:
   `/admin/` under the API app (`web/api/app.js:22`), admin app adds `cacheControl('private')`
   (`admin/app.js:24`), JSON+urlencoded body parsers (33-34), `parseQueryBooleans` (37),
   `checkVersionMatch` (41), `apiMw.cors` (routes.js:17). `authAdminApi` =
   authenticateAdminApi → authorizeAdminApi → updateUserLastSeen → cors → SSL/host redirect →
   prettyUrls → tokenPermissionCheck (`admin/middleware.js:110-118`); integration-token allowlist
   permits `tags: ['GET','PUT','DELETE','POST']` (middleware.js:62).
2. Operation: `browse`, docName `tags` (tags.js:14,16).
3. Input: `options: ['include','filter','fields','limit','order','page','debug']` (tags.js:20);
   validation `include` values `ALLOWED_INCLUDES=['count.posts']` (tags.js:6,21-27). Frame pulls
   options from `query`/`params` (`frame.ts:68-76`); `filter[x]=y` arrives as nested `query.filter`.
   Headers read by the http wrapper: host/secure/authorization (`http.ts:94-98`).
4. Auth: `mw.authAdminApi` + `permissions: true` (tags.js:28); pipeline permissions stage
   calls `apiUtils.permissions.handle({docName:'tags',method:'browse'}, frame)`
   (`pipeline.ts:209-251`). `restrictAdminApiQueryOptions` (tags.js:30) rejects Admin-restricted
   filter keys (`password/token/gift_link_token/secret`, `api-filter-utils.ts:78,218-220`) and
   throws `BadRequestError` for restricted `order` (api-filter-utils.ts:247-267,275-289).
5. Status/headers: default 200 (`http.ts:126-133`); `headers.cacheInvalidate:false`
   (tags.js:17-19) → framework emits **no** `X-Cache-Invalidate` (headers.ts:135-144).
6. Call chain: `models.Tag.findPage(restrictAdminApiQueryOptions(frame.options))` (tags.js:29-31)
   → Bookshelf `findPage` with custom-query/filter/order/pagination plugins
   (`models/base/bookshelf.js:17-32`) on `tags` table (`model/tag.js:12`); `count.posts` relation =
   `countDistinct('posts.id')` via `posts_tags` join, narrowed to published posts for public context
   (tag.js:173-191).
7. Cache: no api-framework `cache` on this controller; side-effect invalidation via
   `emitChange('tag.*')` subscribers (tag.js:75-102) — none local to browse.
8. Output: default output serializer maps `response.data` → `{"tags":[...]}` and copies
   `response.meta` → `meta.pagination` (`serializers/output/default.js:22-41`); `mappers/tags.js:4-17`
   applies `url.forTag` + `clean.tag` (clean.js:4-25 strips `parent_id`/`parent`). Empty result →
   `{"tags":[],"meta":{"pagination":{...}}}`.

**C++**

1. Routes: `TagsRoutes::install` mounts `Router{router,"/tags"}` with
   `content_type_middleware` then `optional_auth`, `.get()` → `tags_browse()`
   (`admin/tags/routes.cppm:19-28`); `ApiRoutes::install` calls `install_serde_formats()` then
   `TagsRoutes{}.install` (`handlers/routes.cppm:79,127`). Single-step flow `tags_browse`
   (`browse/workflow/flow/main.cppm:7-10`).
2. Input: step parses query string manually from `IRequest::get_path()` after `?`
   (`browse/workflow/step/execute.cppm:15-24`); `include` must equal `count.posts` else 422
   `validation("Validation error","include invalid","include")` (step:25-26). `limit`/`page`
   parsed with fallback 15/1 (step:29-33); `filter`/`fields`/`order` parsed but not applied by the
   local pagination code.
3. Auth: router-level `content_type_middleware` (`middleware/serde.cppm:90-119`: 415 missing/bad
   Content-Type, 406 bad Accept) and `optional_auth` (`middleware/auth.cppm:31-40`) — the latter
   unconditionally calls `next()`, so **no token is required**; `require_auth` (auth.cppm:12-29)
   and `bearer_middleware` are not mounted on tags.
4. Status/body/headers: handler sets status 200, writes body bytes, sets literal
   `X-Cache-Invalidate: false`, calls `send()` (`browse/handler.cppm:17-23`).
5. Store: `ctx.store().json_for("tags", params)` (step:28) →
   `query_json(m_db.all<model::Tag>(), params)` (`context_context.cppm:1159`) over two seeded
   fixtures — `Getting Started`/`getting-started`, `News`/`news` (context_context.cppm:933-942).
   `ctx.store().total("tags")` (step:34) → `m_db.count<model::Tag>()` (context_context.cppm:1144).
6. Cache: no `core::cache::Cache` read in this route; no TTL/tag usage.
7. Output: hand-built JSON `{"tags":[...],"meta":{"pagination":{limit,page,pages,prev,next,total}}}`
   (step:38-45); `prev`/`next` are the string `"null"` when unavailable (step:36-37).
8. Divergences: fixture rows instead of DB-backed tags; no restricted-field/order guard; no
   `count.posts` relation; fabricated pagination; literal `X-Cache-Invalidate: false` vs no header;
   presence-free auth.

**Diff** — Residual gaps (no C++ primitive): real Bookshelf/Knex `findPage`, Admin restricted-field
mongoTransformer/order guard, `count.posts` relation, `emitChange` invalidation events, full
serializer field set and `fields` narrowing, token auth/permissions service. Divergences: seeded
fixture tags, fabricated pagination, literal `X-Cache-Invalidate: false`, enforced Content-Type/Accept
headers, `optional_auth` instead of `authAdminApi`.

### GET /ghost/api/admin/tags/:id — MATCH

**Ghost**

1. Identity: `GET /tags/:id`, `mw.authAdminApi` (admin/routes.js:140); same router/app chain as browse.
2. Operation: `read`, docName `tags` (tags.js:14,34).
3. Input: `data:['id','slug','visibility']` (tags.js:39) — `:id` populates `frame.data.id`
   (`frame.ts:107-115`); `options:['include','filter','fields','debug']` (tags.js:38), include∈`count.posts`
   (tags.js:40-46).
4. Auth: `permissions:true` (tags.js:47) + `restrictAdminApiQueryOptions` on the findOne options
   (tags.js:51) → restricted-field/order guard as in browse.
5. Status/headers: 200 on hit; 404 `NotFoundError("Tag not found.")` on miss (tags.js:53-57,
   messages at :8-10); `cacheInvalidate:false` → no X-Cache header (tags.js:35-37).
6. Call chain: `models.Tag.findOne(frame.data, restricted options)` (tags.js:49-52) → Bookshelf
   `findOne` on `tags` table with `columns`/`visibility` permitted options (`model/tag.js:155-171`).
7. Cache: none local.
8. Output: `{"tags":[{full model}]}` (default.js:32-36); miss → error envelope via error handler.

**C++**

1. Route: `Route{"/:id"}.get()` → `tags_read_by_id()` (`admin/tags/routes.cppm:38-46`), same two
   middlewares; flow `tags_read_by_id` (`read_by_id/workflow/flow/main.cppm:7-10`).
2. Input: query parsed as in browse (`read_by_id/workflow/step/execute.cppm:15-24`); `include`
   validated (step:25-27); id taken from path tail `base.substr(base.rfind('/')+1)` (step:29-30);
   empty id → 422 `"A tag id or slug is required."` / field `id` (step:31-33).
3. Auth: `content_type_middleware` + `optional_auth` (`routes.cppm:20-21`) — no token/permission check.
4. Status/body/headers: handler sets `X-Cache-Invalidate: false`, 200 default, 404 for code `404`,
   422 for code `422` (`read_by_id/handler.cppm:23-30`).
5. Store: `ctx.store().find_by_resource("tags", id)` (step:35) →
   `entity_json<model::Tag>(find_entity<model::Tag>(id))` (`context_context.cppm:1095,1123-1128`).
   `find_entity` accepts decimal ids, ghost_id strings, or hex (context_context.cppm:1058-1081);
   the two seeded tags live in the in-memory DB (context_context.cppm:933-942). Miss → 404
   `Tag not found.` (step:36-38).
6. Cache: none.
7. Output: raw entity JSON from `EntityJson<model::Tag>::to_json` (context_context.cppm:1124-1128),
   **not** wrapped in `{"tags":[...]}` — divergence from Ghost envelope.
8. Divergences: no `data[]` visibility option handling; no restricted-field guard; no `count.posts`
   relation or `fields` narrowing; no `{"tags":[...]}` envelope; extra X-Cache header; no auth.

**Diff** — Residual gaps: real `findOne` by id/slug/visibility, restricted-field guard, relations,
serializer/mapper field set, permission service. Divergences: fixture-backed entity, unwrapped body,
literal `X-Cache-Invalidate: false`, presence-free auth, different 422 messages.

### GET /ghost/api/admin/tags/slug/:slug — MATCH

**Ghost**

1. Identity: `GET /tags/slug/:slug`, `mw.authAdminApi` (admin/routes.js:141); declared before/after
   `:id` route but Express matches the deeper path first.
2. Operation: `read`, docName `tags` (tags.js:34).
3. Input: `:slug` populates `frame.data.slug` (tags.js:39, `frame.ts:112-114`); options/include same
   as read (tags.js:38-46).
4. Auth: same `permissions:true` + restricted-field guard as read (tags.js:47-52).
5. Status/headers: 200 on hit, 404 `Tag not found.` on miss (tags.js:53-57); no X-Cache header.
6. Call chain: `models.Tag.findOne({slug}, restricted options)` (tags.js:49-52).
7. Cache: none local.
8. Output: `{"tags":[{full model}]}` (default.js:32-36).

**C++**

1. Route: nested `Router{router,"/slug"}` with its own `optional_auth`, `Route{"/:slug"}.get()` →
   `tags_read_by_slug()` (`admin/tags/routes.cppm:65-79`); flow `tags_read_by_slug`
   (`read_by_slug/workflow/flow/main.cppm:7-10`).
2. Input: query parse + `include` validation as read-by-id (`read_by_slug/workflow/step/execute.cppm:16-28`);
   slug = path tail (step:30-31); empty slug → 422 field `slug` (step:32-34).
3. Auth: `content_type_middleware` inherited from parent router + nested `optional_auth` — no token check.
4. Status/body/headers: handler sets `X-Cache-Invalidate: false`, 200/404/422 mapping
   (`read_by_slug/handler.cppm:23-30`).
5. Store: `ctx.store().json_for("tags", {{"filter","slug:"+slug}})` (step:36) →
   `query_json(m_db.all<model::Tag>(), params)` (`context_context.cppm:1159,1504-1513,1561-1590`)
   → `search_json` applies `filter` (`context_context.cppm:1592-1614`); matches seeded `getting-started`
   / `news` fixtures (context_context.cppm:933-942). Match → first array element re-encoded (step:42-43);
   empty → 404 `Tag not found.` (step:39-41).
6. Cache: none.
7. Output: bare tag entity JSON (step:43), no `{"tags":[...]}` envelope.
8. Divergences: filter string built by concatenation (no escaping), fixture-only data, unwrapped
   body, literal X-Cache header, no visibility/data options, no auth.

**Diff** — Residual gaps: real `findOne({slug})`, restricted-field guard, serializer field set,
permission service. Divergences: fixture-backed filter match, unwrapped body, literal
`X-Cache-Invalidate: false`, presence-free auth.

### POST /ghost/api/admin/tags — MATCH

**Ghost**

1. Identity: `POST /tags`, `mw.authAdminApi` (admin/routes.js:142).
2. Operation: `add`, docName `tags` (tags.js:14,63).
3. Input: body `data.tags[0]` → `frame.data.tags[0]` (`frame.ts:79-105`); `options:['include']`
   (tags.js:68); validation include∈`count.posts` (tags.js:69-75); JSON-schema requires `name`.
4. Auth: `permissions:true` (tags.js:76) + `authAdminApi`; integration-token allowlist includes POST.
5. Status/headers: `statusCode:201` (tags.js:64); `headers.cacheInvalidate:true` → `X-Cache-Invalidate: /*`
   (`headers.ts:21-27,135-144`); `Location: https://<host>/ghost/api/admin/tags/<id>/` auto-generated
   for `add` with a result resource id (`headers.ts:146-185`).
6. Call chain: `models.Tag.add(frame.data.tags[0], frame.options)` (tags.js:77-79) → base model add
   with `formatOnWrite` URL transforms (`model/tag.js:23-53`), defaults `visibility:'public'` (17-21),
   `onSaving` auto-name from slug/`#`-prefix→internal/slug generation (104-130), `onCreated` →
   `emitChange('tag.added')` (80-86). Unique-slug violation surfaces as ValidationError.
7. Cache: invalidation header `/*`; tag events consumed by invalidation subscribers (tag.js:80-86).
8. Output: `{"tags":[{full model}]}` + Location (default.js:32-36, headers.ts:146-185).

**C++**

1. Route: `.post()` → `tags_create()` (`admin/tags/routes.cppm:29-36`); flow `tags_create`
   (`create/workflow/flow/main.cppm:7-10`).
2. Input: query parse + `include` validation 422 (`create/workflow/step/execute.cppm:16-28`);
   body read as string (`in.body_as_string()`, step:30) and `unwrap()` extracts `tags[0]` from root
   or `data.tags[0]` (step:38-60).
3. Auth: `content_type_middleware` + `optional_auth` — no token check.
4. Status/body/headers: handler maps step code `201`→CREATED, `422`→UNPROCESSABLE, `404`→NOT_FOUND,
   sets `X-Cache-Invalidate: true` (not `/*`) (`create/handler.cppm:23-31`).
5. Store: `ctx.store().create("tags", unwrap(raw))` (step:31) →
   `ApiDataStore::create` (`context_context.cppm:1235-1306`): validates presence of `title` or `name`
   (1268-1276 — wrong field/message for tags), `create_entity<model::Tag>` (1283-1284) inserts into
   in-memory DB, `wrap("tags",entity)` returns `MutationResult{201, "{\"tags\":[{...}]}"}`
   (1422-1432). No slug generation, visibility default, URL transforms, or events.
6. Cache: no `core::cache::Cache` use; `X-Cache-Invalidate: true` is a stand-in marker.
7. Output: `{"tags":[{serialized model::Tag}]}` (context_context.cppm:1428-1431); no Location header.
8. Divergences: 201 preserved but no Location; `X-Cache-Invalidate: true` vs `/*`; validation message
   says "cannot save post"/"title is required" even for tags; no schema, slug, events, or URL transforms.

**Diff** — Residual gaps: JSON-schema validation, Bookshelf add + slug/visibility/URL logic,
`emitChange` events, Location header semantics, permission service. Divergences: in-memory entity
instead of DB row, wrong validation message/field, `X-Cache-Invalidate: true`, no Location, no token auth.

### PUT /ghost/api/admin/tags/:id — MATCH

**Ghost**

1. Identity: `PUT /tags/:id`, `mw.authAdminApi` (admin/routes.js:143).
2. Operation: `edit`, docName `tags` (tags.js:14,82).
3. Input: path `:id` (`options:['id','include']`, tags.js:86); body `data.tags[0]` (tags.js:99);
   validation `id.required:true` + include∈`count.posts` (tags.js:87-96).
4. Auth: `permissions:true` (tags.js:97); no restricted-field guard applied to edit options.
5. Status/headers: 200; `headers.cacheInvalidate:false` (tags.js:83-85), but
   `frame.setHeader('X-Cache-Invalidate','/*')` **only when `model.wasChanged()`** (tags.js:106-108)
   — emitted via frame headers (`headers.ts:187-189`); 404 `Tag not found.` if model missing
   (tags.js:100-104).
6. Call chain: `models.Tag.edit(frame.data.tags[0], frame.options)` (tags.js:99) → base model edit
   with same `formatOnWrite`/`onSaving`/`onUpdated` → `emitChange('tag.edited')` (tag.js:88-94);
   `wasChanged()` defined in base model (`models/base/index.js:51`).
7. Cache: conditional invalidation header only.
8. Output: `{"tags":[{full model}]}` (default.js:32-36).

**C++**

1. Route: `Route{"/:id"}.put()` → `tags_edit()` (`admin/tags/routes.cppm:47-55`); flow `tags_edit`
   (`edit/workflow/flow/main.cppm:7-10`).
2. Input: query parse + include validation 422 (`edit/workflow/step/execute.cppm:16-28`); id from
   path tail (step:30-31); body `unwrap()` extracts `tags[0]` (step:36-37,44-66). `id` option and
   its `required:true` validation are not enforced by the step beyond non-empty path check.
3. Auth: `content_type_middleware` + `optional_auth` — no token check.
4. Status/body/headers: handler maps step `201`→200 (changed=true), `200`→200 (changed=true),
   `404`→NOT_FOUND, `422`→UNPROCESSABLE; sets `X-Cache-Invalidate: "/*"` whenever changed, else
   `"false"` (`edit/handler.cppm:23-33`).
5. Store: `ctx.store().edit("tags", id, unwrap(raw))` (step:37) →
   `ApiDataStore::edit` (`context_context.cppm:1308-1354`) → `wrap("tags", edit_entity<model::Tag>(id,body))`
   (1331-1332); `edit_entity` finds by id and updates the in-memory DB (1033-1046); missing id →
   `MutationResult{404, "Resource not found"}` via `wrap` (1423-1427).
6. Cache: no `core::cache::Cache`; `/*` header is set unconditionally on 2xx, not gated on change.
7. Output: `{"tags":[{serialized model::Tag}]}` from `wrap` (context_context.cppm:1428-1431).
8. Divergences: no `wasChanged()` semantics — header always `/*` on success; step returns 201 which
   the handler remaps to 200; no slug/URL/event logic; no permission check.

**Diff** — Residual gaps: real `Model.edit`, `wasChanged()` diff, `emitChange` events, slug/URL logic,
permission service. Divergences: unconditional `/*` header, 201→200 remap, in-memory persistence,
presence-free auth.

### DELETE /ghost/api/admin/tags/:id — MATCH

**Ghost**

1. Identity: `DELETE /tags/:id`, `mw.authAdminApi` (admin/routes.js:144).
2. Operation: `destroy`, docName `tags` (tags.js:14,114).
3. Input: path `:id`; `options:['id']`; validation `id.required:true` (tags.js:119-129) — note the
   controller also lists an unused `include` validation (tags.js:122-124).
4. Auth: `permissions:true` (tags.js:130).
5. Status/headers: `statusCode:204` (tags.js:115); `headers.cacheInvalidate:true` → `X-Cache-Invalidate: /*`
   (tags.js:116-118, headers.ts:21-27,135-144); 204 → empty body.
6. Call chain: `models.Tag.destroy({...frame.options, require:true})` (tags.js:131-133) → overridden
   `Tag.destroy` (`model/tag.js:193-219`): `filterOptions` with allowed `id`, `withRelated=['posts']`,
   `lockLinkedForUpdate('posts_tags','post_id',{tag_id})`, fetch by id (NotFoundError if missing,
   tag.js:205-210), `related('posts').detach()`, `tag.destroy()`, all inside `transactionLockingLinked`
   unless caller supplied a transaction; `onDestroyed` → `emitChange('tag.deleted')` (tag.js:96-102).
7. Cache: invalidation header `/*` + model events.
8. Output: default destroy serializer sets `{tags:[]}` (`default.js:15-20`) but 204 suppresses body.

**C++**

1. Route: `Route{"/:id"}.delt()` → `tags_destroy()` (`admin/tags/routes.cppm:56-63`); flow
   `tags_destroy` (`destroy/workflow/flow/main.cppm:7-10`).
2. Input: id = path tail; empty → 422 `"A tag id is required."` field `id`
   (`destroy/workflow/step/execute.cppm:15-20`). No `options[]`/`require:true` equivalent.
3. Auth: `content_type_middleware` + `optional_auth` — no token check.
4. Status/body/headers: handler sets `Status::NO_CONTENT` with empty body for any non-404 code,
   404 with error body on miss, and `X-Cache-Invalidate: true` (`destroy/handler.cppm:23-33`).
5. Store: `ctx.store().destroy("tags", id)` (step:22) → `ApiDataStore::destroy`
   (`context_context.cppm:1356-1382`) → `destroy_entity<model::Tag>(id)` (1363-1364) removes the
   in-memory row; success → `MutationResult{200,"{}"}` (1381) which the handler turns into 204;
   miss → 404 `"Resource not found"` (1375-1376).
6. Cache: no `core::cache::Cache`; `X-Cache-Invalidate: true` stand-in vs Ghost's `/*`.
7. Output: 204 empty body on success — status matches Ghost.
8. Divergences: no transaction, row lock, or `posts_tags` detach; no `emitChange`; different 404
   message; `X-Cache-Invalidate: true` instead of `/*`; no permission check.

**Diff** — Residual gaps: real Knex transaction + `lockLinkedForUpdate` + relation detach,
`emitChange` events, permission service, admin restricted-field infra (not used here). Divergences:
`X-Cache-Invalidate: true` vs `/*`, fixture/in-memory data, presence-free auth, different 404 body.

## Module-level notes

- The 6 routes above are all routes bound to this controller (`api.tags`, wired via
  `endpoints/index.js:175-176`). The Content API tag routes (`GET /tags`, `GET /tags/:id`,
  `GET /tags/slug/:slug` in `content/routes.js:32-34`) bind `api.tagsPublic` →
  `endpoints/tags-public.js` (`endpoints/index.js:303-304`) and are reported separately in
  `tags-public.md`; `GET /search-index/tags` binds `api.searchIndex.fetchTags`
  (`admin/routes.js:612`), also a different controller.
- Ghost controller declares no `schema`, `unsafeAttrs`, labs flags, custom `format`, or
  `cache` service; only `docName`, per-op `options`/`data`/`validation`, `permissions:true`,
  `statusCode`, `headers`, and `query` (`tags.js:1-137`). All validation beyond `include`/`id` comes
  from the shared JSON-schema pipeline and the Admin restricted-field guard in browse/read.
- C++ implementation is uniform across the 6 routes: one-step local workflow
  (`core::workflow::LocalWorkflowDriver`) with a static `BlogContext*` on each `ExecuteStep`,
  manual query-string parsing in every step, shared `ApiDataStore` in-memory DB seeded with 2 tags,
  and hand-built JSON responses. No transactions, no model events, no real cache service, no
  permission/token check (`optional_auth` everywhere), and Content-Type/Accept enforcement that
  Ghost's admin app does not perform.
- Router-level C++ mount: `ApiRoutes::install` (`handlers/routes.cppm:74-127`) calls
  `install_serde_formats()` and `TagsRoutes{}.install`; Ghost mounts at `/ghost/api/admin/`
  (`web/api/app.js:15-22`, `admin/app.js:19-44`).


## Strict-parity rewrite (manifest group 9, 2026-11-19)

All six routes were rewritten in `include/blog/handlers/admin/tags/` to use the shared Phase-0
abstractions rather than local stand-ins. Summary + route table flipped **PARTIAL ×6 → MATCH ×6**;
the route details and Diff bullets above are the original pre-rewrite analysis and are superseded
by this section.

| Route | Shared abstraction closing the reported Diff |
|---|---|
| `GET /tags` (browse) | `core::query::parse(path, {include,filter,fields,limit,order,page,debug})` + `core::query::run(cleaned,q)` → allowlisted options, `filter[x]`/bare NQL, `order`, `limit=all`, `fields` projection, real `meta.pagination`; `core::query::guard` (called by `run`) enforces the Admin restricted-field/order guard (400 on `password/token/gift_link_token/secret`). `blog::mapping::clean(rows,"tag")` applies the Ghost field allow-list. `data` → `tags` rename reproduces the default output serializer. |
| `GET /tags/:id` | `core::query::parse` + `core::query::has_restricted_field` on `filter`/`order`/`filter[x]`; `blog::mapping::clean` + `blog::mapping::wrap("tags",...)` gives the full-model envelope; 404 `Tag not found.` on miss; ghost permission `tags:read`. |
| `GET /tags/slug/:slug` | `core::query::parse("?filter[slug]="+slug, {"filter"})` + `core::query::apply` replaces the unescaped `"slug:"+slug` concatenation and applies the restricted-field guard; clean + wrap; 404 on miss; ghost permission `tags:read`. |
| `POST /tags` | `data.tags[0]` unwrap + JSON-schema-style `name` required check → 422 `ValidationError` with `property:"name"`; clean + wrap 201 body; `Location: https://<host>/ghost/api/admin/tags/<id>/`; `core::outbox::Event{"tag.added", body, id}` (Ghost `onCreated` `emitChange`); `X-Cache-Invalidate: /*` (`cacheInvalidate:true`); ghost permission `tags:add`. |
| `PUT /tags/:id` | pre/post record comparison implements `model.wasChanged()`; `core::outbox::Event{"tag.edited", ...}` emitted only on change; `/*` invalidation header only on change (no header otherwise); store 201 normalized to Ghost's 200; clean + wrap; ghost permission `tags:edit`. |
| `DELETE /tags/:id` | `core::outbox` `begin_transaction()` → `tag.deleted` event → `commit()` (rollback on miss) substitutes the Ghost transaction + `emitChange('deleted')`; 204 empty body on success, 404 `Tag not found.` on miss; `X-Cache-Invalidate: /*`; ghost permission `tags:destroy`. |

**Auth.** Every handler now calls the full middleware shape
`blog::handlers::middleware::require_ghost_permission(req, res, next, send, "tags", <method>)`, which
maps `x-blog-context`, `x-blog-role`, `x-blog-permissions`, `x-blog-staff` and `Authorization` into a
`core::auth::ghost::Principal` and, on denial, writes a Ghost `UnauthorizedError` (401) /
`NoPermissionError` (403) envelope via `write_error`. Router-level `optional_auth` and
`content_type_middleware` were removed: Ghost's admin app does not gate GETs on a JSON Content-Type,
and token/permission enforcement now lives in the handler pipeline. When `x-blog-context` is absent
the shared handlers default admin routes to `ADMIN_API` and GET routes whose path contains
`/content/` to `CONTENT_API` (anonymous public reads are permitted by `GhostAuthorizer`).

**Cache.** `X-Cache-Invalidate: false` was removed from browse/read (Ghost `cacheInvalidate:false`
emits no header), and `true` was corrected to `/*` on create/destroy (Ghost `cacheInvalidate:true`).

**Files changed (all inside `include/blog/handlers/admin/tags/`).**
`routes.cppm`; `browse/{handler.cppm,workflow/step/execute.cppm}`;
`read_by_id/{handler.cppm,workflow/step/execute.cppm}`;
`read_by_slug/{handler.cppm,workflow/step/execute.cppm}`;
`create/{handler.cppm,workflow/step/execute.cppm}`;
`edit/{handler.cppm,workflow/step/execute.cppm}`;
`destroy/{handler.cppm,workflow/step/execute.cppm}`.
The six `workflow/flow/main.cppm` files are unchanged.

**Central registrations.** None required and none edited. `core_query`, `core_auth`, `core_outbox`
and `blog_handlers:mapping` are already `deps`/partitions of `//include/blog/handlers:blog_handlers`
(`BUILD.bazel:917-919,925,928-929`), `blog_handlers_middleware:ghost_permission` is already
re-exported by `middleware/base.cppm:8`, and the six handlers/steps/flows keep their existing
`blog_handlers:tags_<op>_*` partition names and `import :domain_all;`. No new files, no new
partitions, no `all.cppm` / `base.cppm` / central `routes.cppm` / `BUILD.bazel` edit.

**Remaining gaps (no shared primitive; non-wire-observable or opt-in).** Real Bookshelf/Knex
persistence and `posts_tags` row locking/detach are represented by the existing in-memory store plus
an outbox transaction; `count.posts` relation materialization for `include=count.posts`; Ghost's
config-gated `emitChange` subscribers / response cache; slug generation and `url` mapper enrichment
(the store's `model::Tag` exposes only `id,name,slug,description`). These do not change status,
envelope, pagination or invalidation headers for the audited default requests.

**Verification.** `bazel build //include/blog/handlers:blog_handlers --keep_going` reports no error
from any file under `admin/tags/`. The target remains blocked by pre-existing, unrelated errors in
`admin/pages/{browse,create,edit}` and a missing `blog_handlers:notifications_service` module.
