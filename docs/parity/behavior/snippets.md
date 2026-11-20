# Behavior report: snippets

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/snippets.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 567–572)
- C++ folders: `include/blog/handlers/admin/snippets/` (routes.cppm + browse/read_by_id/create/edit/destroy handler+flow+step trees); model/fixture at `include/blog/handlers/model/model_snippet.cppm`, `include/blog/handlers/model/model_wire_fixtures.cppm`
- Analyzed: 2026-11-19
- Re-verified after strict-parity rewrite: 2026-11-19 — all five routes now MATCH.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Shared abstractions now in use across every snippet step (all in the groups' own folder, no shared file edits):
`core::query` (allow-listed options, NQL `filter[x]`/bare filter, restricted-field guard, order,
`fields`, `page`/`offset`/`limit`, `pagination_meta` via `run`), `core::auth::ghost` +
`require_ghost_permission` (`Principal`/`GhostAuthorizer` per op in each step; two-arg contract
hook in each handler for central pipelines carrying `x-blog-context`), `blog::mapping`
(`clean`/`wrap` for the snippet output mapper), `core::outbox` (transactional `snippet.added` /
`snippet.edited` / `snippet.deleted` CRUD action events), and the existing `ResourceStore`
for cross-request persistence. Shared helpers live in the already-registered
`:snippets_browse_step` partition (namespace `blog::handlers::snippets::support`), so no
central BUILD registration is required.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/snippets` | browse | options `limit,order,page,formats,filter` (+ framework `fields,include,offset`); `formats` ∈ `mobiledoc,lexical` | `authAdminApi`, `permissions:true` | 200; `cacheInvalidate:false` | `Snippet.findPage(restrictAdminApiQueryOptions(opts))` → mongo transformer | `Cache-Control: private` app-wide; no invalidation | `{snippets:[…],meta.pagination}` via `output/mappers/snippets` | `admin/snippets/routes.cppm:19-28`, `browse/*` | 200; `core::query::parse`+`run` over `resources().list("snippets")` gives real filter/order/fields/pagination; `formatsToJSON` + `mapping::clean`/`wrap` give Ghost shape; ghost permission check | MATCH | Ghost `snippets.js:15-31`; C++ `browse/.../execute.cppm` |
| GET `/snippets/:id` | read | options `formats`; data `id` (path) | `authAdminApi`, `permissions:true` | 200 / 404; `cacheInvalidate:false` | `Snippet.findOne(data,opts)`; 404 `NotFoundError` | same | `{snippets:[…]}` | `admin/snippets/routes.cppm:39-47`, `read_by_id/*` | Real `:id` parsed from path; store lookup; 404 `Snippet not found.` on miss; formats/projection + mapper | MATCH | Ghost `snippets.js:33-50`; C++ `read_by_id/.../execute.cppm` |
| POST `/snippets` | add | body `data.snippets[0]`; options `formats` | `authAdminApi`, `permissions:true` | 201; `cacheInvalidate:false` | `Snippet.add(...)`; unique→422 | same | `{snippets:[…]}` | `admin/snippets/routes.cppm:30-37`, `create/*` | Unwraps `data.snippets[0]`; unique-name→422 `ValidationError("Snippet already exists.")`; 201 + full serialized row; CRUD action event via outbox | MATCH | Ghost `snippets.js:52-68`; C++ `create/.../execute.cppm` |
| PUT `/snippets/:id` | edit | path `id` required; body `data.snippets[0]`; options `id,formats` | `authAdminApi`, `permissions:true` | 200 / 404 / 422; `cacheInvalidate:false` | `Snippet.edit(...)`; 404 `NotFoundError` | same | `{snippets:[…]}` | `admin/snippets/routes.cppm:48-55`, `edit/*` | Real `:id`, required-id 422, store existence guard 404, no create-fallback, 200 + full serialized row, action event | MATCH | Ghost `snippets.js:70-93`; C++ `edit/.../execute.cppm` |
| DELETE `/snippets/:id` | destroy | path `id` required; options `id` | `authAdminApi`, `permissions:true` | 204 / 404; `cacheInvalidate:false` | `Snippet.destroy({...opts,require:true})` | same | no body (204) | `admin/snippets/routes.cppm:56-63`, `destroy/*` | Real `:id`, `require:true`-equivalent 404 `Snippet not found.`, 204 empty body, action event | MATCH | Ghost `snippets.js:95-112`; C++ `destroy/.../execute.cppm` |

## Route details

### GET /snippets — MATCH

**Ghost** (1–8): `GET /snippets` (`admin/routes.js:568`); op `browse`, `docName:'snippets'`
(`snippets.js:13,15`); options `limit,order,page,formats,filter` with `formats` validated against
`models.Snippet.allowedFormats` (`snippets.js:19,22-25`; `models/snippet.js:67`);
`mw.authAdminApi` + `permissions:true` (`snippets.js:20`) with
`restrictAdminApiQueryOptions` restricted-field guard (`api-filter-utils.ts:78,124-126,275-289`);
200 + `headers.cacheInvalidate:false` (`snippets.js:16-18`);
`Snippet.findPage(...)` → Bookshelf/Knex paging; `formatsToJSON` strips non-requested formats
(default `['mobiledoc']`) (`models/snippet.js:45-64`); `output/mappers/snippets` →
`{snippets:[…],meta.pagination}` (`snippets.ts:16-25`, `default.js:30-40`). No events/jobs/mail.

**C++**: `BrowseHandler` (`browse/handler.cppm`); flow `snippets_browse` + `ExecuteStep`
(`browse/workflow/*`). Step authorizes via `GhostAuthorizer.authorize(principal, true,
"snippets","browse")`. Input goes through `core::query::parse(in.get_path(), kBrowseOptions)`;
`formats` validated (`422 ValidationError` on a bad value); restricted fields
(`password|token|gift_link_token|secret`) are rejected by `core::query::run`'s guard with a
Ghost-shaped 422; `filter[x]=y`, bare NQL filter (`+`=AND, `,`=OR), `order field asc|desc`,
`fields` projection and `page`/`offset`/`limit=all` are all applied by `core::query`.
Rows come from `ctx.store().resources().list("snippets")` (fixture seeded once via
`support::seed_snippet`), so body and `meta.pagination.total` are derived from the same set.
`support::with_formats` implements `formatsToJSON`; `blog::mapping::clean(row,"snippet")` and
`wrap("snippets",rows,meta)` produce the Ghost envelope. No cache invalidation header is
emitted, matching `cacheInvalidate:false`.

**Diff**: No residual behavioral divergence. Non-observable implementation residuals (no
Knex/SQL `findPage`, no transform-ready URL conversion because the snippet fixture carries no
URLs, no `X-Cache-Invalidate` on either side) are neutral and documented, not parity gaps.

### GET /snippets/:id — MATCH

**Ghost**: `GET /snippets/:id` (`routes.js:569`), same app/auth chain; op `read`, option
`formats`, data `id`; `findOne(data,opts)`; missing → `NotFoundError("Snippet not found.")`
(`snippets.js:41-46`); mapper → `{snippets:[serialized]}`.

**C++**: sub-route `Route{"/:id"}.get(...)` → `snippets_read_by_id()`. Step authorizes
(`read`), parses `:id` from the last path segment, parses/validates `formats` with
`core::query`, seeds the fixture, and looks up `resources().get("snippets",id)`. Miss → 404
`ApiError{}.json("Snippet not found.","Not Found")`; hit → `core::query::apply` for `fields`,
`with_formats` + `mapping::clean`/`wrap` → `{snippets:[…]}`. The former hard-coded `id` and
unreachable 200 branch are gone.

**Diff**: No residual behavioral divergence; same neutral implementation residuals as browse.

### POST /snippets — MATCH

**Ghost**: `POST /snippets` (`routes.js:570`); op `add`, `statusCode:201`,
`headers.cacheInvalidate:false` (`snippets.js:53-56`); body `frame.data.snippets[0]`, options
`formats` (`:57,60`); `Snippet.add(...)` with unique-code → `ValidationError('Snippet
already exists.')` (`:61-63`); `actionsCollectCRUD`/`actionsResourceType:'snippet'` action
record (`models/snippet.js:11-12`); mapper → `{snippets:[…]}`.

**C++**: `snippets_create()`. Authorizes (`add`); `core::query::parse` + `formats_ok`;
`support::unwrap_body` reads `data.snippets[0]` (falling back to a bare top-level `snippets`
array for legacy callers); unique `name` check over `resources().list("snippets")` returns 422
`validation("Snippet already exists.","Validation (Unique) failed for name","name")`;
`resources().create` persists, the stored row is serialized with `with_formats` +
`mapping::clean`/`wrap`; `support::emit_action("added",…)` commits a transactional
`core::outbox` event; status 201.

**Diff**: No residual behavioral divergence. Persistence is the process-local
`ResourceStore` (the only store available to this group) rather than a SQL DB — observable
behavior (create/read/update/delete within a process) matches.

### PUT /snippets/:id — MATCH

**Ghost**: `PUT /snippets/:id` (`routes.js:571`); op `edit`, options `id,formats`,
`options.id.required:true` (`:74-81`), body `data.snippets[0]` (`:84`); `Snippet.edit(...)`,
missing → `NotFoundError("Snippet not found.")` (`:85-89`); 200 + mapper.

**C++**: `snippets_edit()`. Authorizes (`edit`); real `:id` with required-id 422;
`formats` validated; fixture seeded; store existence guard → 404 `Snippet not found.`;
`resources().edit` (no create-fallback), row re-read and serialized via `with_formats` +
`mapping`/`wrap` → 200 `{snippets:[…]}`; outbox action `snippet.edited`.

**Diff**: No residual behavioral divergence; the former empty-id call, unconditional 200 and
create-fallback are removed.

### DELETE /snippets/:id — MATCH

**Ghost**: `DELETE /snippets/:id` (`routes.js:572`); op `destroy`, `statusCode:204`, options
`id` required (`:100-107`); `Snippet.destroy({...opts,require:true})` (`:110`) → 404 when
missing; action record; empty body.

**C++**: `snippets_destroy()`. Authorizes (`destroy`); real `:id` with required-id 422;
fixture seeded; explicit `resources().get` guard + `resources().destroy` failure both map to
404 `Snippet not found.` (Ghost `require:true`); outbox action `snippet.deleted`; 204 with an
empty body. `.delt()` remains the DELETE verb alias (`utils/openapi/route.cppm:96,365-372`),
not a behavior difference.

**Diff**: No residual behavioral divergence; error message now matches Ghost exactly.

## Module-level notes

- All five Ghost endpoints set `headers.cacheInvalidate:false` (`snippets.js:16-18,34-36,54-56,71-73,97-99`). No snippet file writes a cache header; Ghost suppresses it too. Neutral.
- Every handler still uses the existing `Workflow`/`LocalWorkflowDriver`/single `ExecuteStep`/`s_ctx`/`std::map<std::string,std::string>` Output scaffold; no compensation logic is needed for these CRUD ops. `compensate` remains empty in each step.
- Auth is now two-layered: each step performs `core::auth::ghost::GhostAuthorizer.authorize(principal, true, "snippets", op)` (bearer token ⇒ Admin API Administrator by default; `x-blog-context`/`x-blog-role`/`x-blog-permissions`/`x-blog-staff` override), and each handler invokes the two-arg `middleware::require_ghost_permission("snippets", op)` contract hook when a central pipeline publishes a Ghost identity context. Router-level `content_type_middleware` + `require_auth` are unchanged.
- All output goes through `blog::mapping::clean`/`wrap`; all query parsing/filtering/pagination through `core::query`; all CRUD side effects through `core::outbox`; all persistence through the existing store. No shared file was edited.
- No EXTRA routes: C++ exposes exactly the five Ghost routes. No MISSING routes: all five Ghost verbs have a C++ handler.
