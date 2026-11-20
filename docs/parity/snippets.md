# Parity: snippets

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/snippets.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/snippets/routes.cppm` (handlers + steps under `include/blog/handlers/admin/snippets/<op>/`)
- Checked: 2026-11-19
- Re-verified after strict-parity rewrite: 2026-11-19

Note: `.delt()` is the DELETE verb (`include/utils/openapi/route.cppm:96`, builder chain at `:370`). All 5 C++ operations are wired via `all.cppm:219-223` (`snippets_browse/_read_by_id/_create/_edit/_destroy`) and installed by `include/blog/handlers/routes.cppm:120`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /snippets | browse | GET /snippets (`:19-27`) | `snippets_browse().handle` (`:26`) | Ghost `options:['limit','order','page','formats','filter']` + `formats` validation (`snippets.js:19-27`); C++ `core::query::parse` over all five + framework `fields/include/offset`, `422` on bad `formats` | Ghost real pagination from `findPage`; C++ `core::query::run` data + `meta.pagination` through `mapping::wrap` | Ghost `Snippet.findPage(restrict...)` (`snippets.js:29`); C++ `store().resources().list("snippets")` with fixture seeded once, filtered/sorted/paged by `core::query` | Ghost `cacheInvalidate:false` (`snippets.js:17`); C++ emits no invalidation header — match | Ghost `mw.authAdminApi` + `permissions:true`; C++ `GhostAuthorizer.authorize(...,true,"snippets","browse")` + handler `require_ghost_permission` hook + router `require_auth` | MATCH | Ghost `routes.js:568`, `snippets.js:15-31`; C++ `routes.cppm:19-27`, `browse/.../execute.cppm` |
| GET /snippets/:id | read | GET /snippets/:id (`:38-45`) | `snippets_read_by_id().handle` (`:44`) | Ghost `data:['id']`, `options:['formats']`; C++ path `:id` + `core::query::parse` read options | Ghost serialized model or 404 `NotFoundError`; C++ 200 `{snippets:[…]}` or 404 `Snippet not found.` | Ghost `findOne(frame.data, frame.options)`; C++ `store().resources().get("snippets", id)` with `fields` projection | same | Ghost `mw.authAdminApi` + `permissions:true`; C++ `GhostAuthorizer` (`read`) + hook + `require_auth` | MATCH | Ghost `routes.js:569`, `snippets.js:33-50`; C++ `routes.cppm:38-45`, `read_by_id/.../execute.cppm` |
| POST /snippets | add | POST /snippets (`:29-35`) | `snippets_create().handle` (`:34`) | Ghost `options:['formats']`, body `data.snippets[0]`; C++ `support::unwrap_body` handles `data.snippets[0]` + legacy top-level `snippets` | Ghost `statusCode:201`; C++ 201 + full serialized row via `mapping::clean/wrap` | Ghost `Snippet.add(...)` + unique→ValidationError; C++ `resources().create` + unique-name 422 `"Snippet already exists."` + outbox `snippet.added` | same | Ghost `mw.authAdminApi` + `permissions:true`; C++ `GhostAuthorizer` (`add`) + hook + `require_auth` | MATCH | Ghost `routes.js:570`, `snippets.js:52-68`; C++ `routes.cppm:29-35`, `create/.../execute.cppm` |
| PUT /snippets/:id | edit | PUT /snippets/:id (`:47-53`) | `snippets_edit().handle` (`:52`) | Ghost `options:['id','formats']`, `id` required, body `data.snippets[0]`; C++ real `:id`, required-id 422, unwrapped body, `formats` validation | Ghost 200 model or 404; C++ 200 full row / 404 `Snippet not found.` / 422 | Ghost `Snippet.edit(...)`; C++ existence guard + `resources().edit` (no create-fallback) + outbox `snippet.edited` | same | Ghost `mw.authAdminApi` + `permissions:true`; C++ `GhostAuthorizer` (`edit`) + hook + `require_auth` | MATCH | Ghost `routes.js:571`, `snippets.js:70-93`; C++ `routes.cppm:47-53`, `edit/.../execute.cppm` |
| DELETE /snippets/:id | destroy | DELETE /snippets/:id (`:55-61`, `.delt`) | `snippets_destroy().handle` (`:60`) | Ghost `options:['id']`, `id` required; C++ real `:id` + required-id 422 | Ghost 204 no body, `require:true` 404; C++ 204 empty / 404 `Snippet not found.` | Ghost `Snippet.destroy({...opts,require:true})`; C++ `get` guard + `resources().destroy` + outbox `snippet.deleted` | same | Ghost `mw.authAdminApi` + `permissions:true`; C++ `GhostAuthorizer` (`destroy`) + hook + `require_auth` | MATCH | Ghost `routes.js:572`, `snippets.js:95-112`; C++ `routes.cppm:55-61`, `destroy/.../execute.cppm` |

## Route details

### GET /snippets — MATCH
- Route/verb/operation/resource agree: Ghost `routes.js:568` ↔ C++ `routes.cppm:19-27` (`snippets_browse`).
- Input: all five Ghost options parsed via `core::query::parse` (`include/core/query/base.cppm`);
  `filter[x]=y`, bare NQL (`+`=AND, `,`=OR, `[a,b]`, `field:null`), restricted-field guard,
  `order`, `fields`, `page`/`offset`/`limit=all` handled by `core::query::run`. Bad `formats`
  → 422 `ValidationError` (`support::formats_error`).
- DB: `core::query::run(store().resources().list("snippets"), q, 15)` replaces the former bare
  fixture return; the `SnippetWire` fixture is seeded once into the existing `ResourceStore`
  (cache keys `resource:snippets:*`, tag `resources`) so data and `meta.pagination.total` agree.
- Output: `support::with_formats` implements `Snippet.formatsToJSON` (default keeps `mobiledoc`,
  drops `lexical`); `blog::mapping::clean(rows,"snippet")` + `wrap("snippets",rows,meta)` yield
  `{snippets:[…],meta:{pagination:{…}}}` — same envelope as `output/mappers/snippets`.
- Cache: Ghost `headers.cacheInvalidate:false`; C++ emits no invalidation header — match.
- Auth: step `GhostAuthorizer.authorize(principal,true,"snippets","browse")`; handler
  `require_ghost_permission("snippets","browse")` when a Ghost identity context is published;
  router `content_type_middleware` + `require_auth`. All Ghost checks 1–8 closed.

### GET /snippets/:id — MATCH
- `:id` parsed from the last path segment (query + trailing `/` stripped); no hard-coded id.
- `core::query::parse(...,kReadOptions)` + `formats_ok`; `core::query::apply` projects `fields`.
- `store().resources().get("snippets",id)`; miss → 404 `Snippet not found.`, hit → 200 via
  `with_formats` + `mapping::clean`/`wrap`. The former always-404 branch is gone.
- Cache/auth: unchanged paragraphs from browse apply (`read` op for permission).

### POST /snippets — MATCH
- `support::unwrap_body` reads Ghost's `data.snippets[0]` (also tolerates a bare `snippets`
  array); `core::query` handles `formats`.
- Unique `name` over stored snippets → 422
  `validation("Snippet already exists.","Validation (Unique) failed for name","name")`.
- `resources().create` persists; stored row serialized with `with_formats` + mapping → 201
  `{snippets:[…]}`; `support::emit_action("added",…)` commits a `core::outbox` CRUD event
  matching `actionsCollectCRUD`/`actionsResourceType:'snippet'`.

### PUT /snippets/:id — MATCH
- Real `:id` + required-id 422; `formats` validated; body unwrapped from `data.snippets[0]`.
- Existence guard → 404 `Snippet not found.`; `resources().edit` (no create-fallback) → 200
  full serialized row; outbox `snippet.edited`.

### DELETE /snippets/:id — MATCH
- Real `:id` + required-id 422; `get` guard + `resources().destroy` failure → 404
  `Snippet not found.` (Ghost `require:true`); outbox `snippet.deleted`; 204 empty body.
- `.delt` is the DELETE verb alias, not a behavior difference.

Cross-cutting: every snippet step now uses `core::query`, `core::auth::ghost`, `blog::mapping`
and `core::outbox` via `blog::handlers::snippets::support` (defined in the already-registered
`:snippets_browse_step` partition, imported by the other four step partitions). Output is
`std::map<std::string,std::string>` status/body; `s_ctx` points at `blog::BlogContext`. No
`X-Cache-Invalidate` header is emitted on either side. No shared file was edited.

## Missing in C++

None. All 5 Ghost controller routes have a C++ counterpart (same path + verb):
- GET /snippets → `snippets_browse` (`routes.cppm:19-27`)
- GET /snippets/:id → `snippets_read_by_id` (`routes.cppm:38-45`)
- POST /snippets → `snippets_create` (`routes.cppm:29-35`)
- PUT /snippets/:id → `snippets_edit` (`routes.cppm:47-53`)
- DELETE /snippets/:id → `snippets_destroy` (`routes.cppm:55-61`)

## Extra in C++

None. `SnippetsRoutes` installs exactly the five Ghost routes (`routes.cppm:19-63`). No bulk,
export or public snippet route exists in this folder.

## Needs central registration

None. The former store-layer gaps were resolved locally using the existing `ResourceStore`
(`resources().list/get/create/edit/destroy`) and the `SnippetWire` fixture; no shared
`context_context.cppm` change is required. Shared-helper code lives in the already-registered
`:snippets_browse_step` partition, so no new BUILD partition, `all.cppm`, central
`routes.cppm` or middleware change is needed. Router-level `require_auth` remains in place
and the permission checks are performed per operation in each step (plus the two-arg
`require_ghost_permission` contract hook in each handler for centrally supplied Ghost
identities).

Notes:
- Ghost `cacheInvalidate:false` emits no header; C++ emits none — check 6 needs no change.
- The only remaining differences are non-observable implementation details (in-memory store
  instead of Knex/SQL, no transform-ready URL fields in the snippet fixture). They do not
  change the response for any Ghost-shaped request and are documented as neutral residuals.
