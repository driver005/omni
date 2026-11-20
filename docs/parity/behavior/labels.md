# Behavior report: labels

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/labels.js:1-156`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:294-299`
- C++ folders: `include/blog/handlers/admin/labels/` (routes + browse, read_by_id,
  read_by_slug, create, edit, destroy handler/flow/step sets); shared store in
  `include/blog/handlers/context/context_context.cppm`
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19 (manifest group 2)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 6 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All six routes now implement Ghost's observable HTTP contract: verb/path, input
parsing + validation, `permissions` check through `core::auth::ghost`, status,
headers (including conditional `X-Cache-Invalidate`, `Location`), `{"labels":...}`
serialization, pagination meta, duplicate/not-found error envelopes, and
mutation persistence / side effects. Verdicts stay `PARTIAL`: the residual gaps
below (no real Knex/DB, no JSON-schema validator package, no linked-member row
lock or detach, presence-based token→role mapping) have no C++ primitive inside
this folder and are recorded, not hidden.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /labels | browse, docName labels | `options[]` include/filter/fields/limit/order/page; include∈`count.members`; no body | `mw.authAdminApi` + `permissions:true` + restricted-field guard | 200; no X-Cache header | `Label.findPage(restrictAdminApiQueryOptions(options))`, default order `name ASC, created_at DESC`, optional `count.members` | no route cache; `emitChange` subscribers | `{"labels":[...],"meta":{"pagination":{...}}}` | `admin/labels/browse/{handler.cppm,workflow/...}` | `core::query::parse` allowlisted options; include validation; restricted guard → 400; `core::query::apply` filter/order/fields/pagination over real ResourceStore labels; default order `name asc,created_at desc`; total from filtered rows; `blog::mapping::pagination`+`wrap`; no X-Cache header | PARTIAL | Ghost `labels.js:32-48`,`api-filter-utils.ts:246-289`; C++ `browse/.../execute.cppm:222-283`,`browse/handler.cppm:40-79` |
| GET /labels/:id | read (id), docName labels | `data[]` id/slug; `options[]` include/filter/fields; include∈`count.members`; path `:id` | same + restricted guard | 200 or 404 `Label not found.`; no X-Cache | `Label.findOne({id}, restricted options)`, `NotFoundError` on miss | none | `{"labels":[{full model}]}` | `admin/labels/read_by_id/{handler.cppm,workflow/...}` | path id resolved (numeric ↔ Ghost wire id); `core::query` `filter[id]=` lookup over real labels; 404 `NotFoundError` envelope on miss; `fields` projection; ghost auth + include/restricted validation; no X-Cache header | PARTIAL | Ghost `labels.js:50-77`,`model/label.js:12`; C++ `read_by_id/.../execute.cppm:16-72`,`read_by_id/handler.cppm:40-79` |
| GET /labels/slug/:slug | read (slug), docName labels | same `data`/`options`, path `:slug` | same + restricted guard | 200 or 404 `Label not found.`; no X-Cache | `Label.findOne({slug}, restricted options)` | none | `{"labels":[{full model}]}` | `admin/labels/read_by_slug/{handler.cppm,workflow/...}` | slug parsed from path; `core::query` `filter[slug]=` lookup; 404 on miss; `fields` projection; ghost auth; no X-Cache header | PARTIAL | Ghost `labels.js:50-77`,`admin/routes.js:296`; C++ `read_by_slug/.../execute.cppm:16-72`,`read_by_slug/handler.cppm:40-79` |
| POST /labels | add, docName labels | body `data.labels[0]`; `options[]` include; include∈`count.members`; JSON-schema `name` required, 1..191 | `mw.authAdminApi` + `permissions:true` | **201**; no X-Cache; `Location: .../labels/<id>/` | `Label.add(...).catch(handleDuplicateNameError)`; trim + slug generate; `emitChange('added')` | invalidation via events | `{"labels":[{full model}]}` + Location | `admin/labels/create/{handler.cppm,workflow/...}` | `data.labels[0]` unwrapped; name required/length trimmed; duplicate → `ValidationError("Label already exists")`; slug generated + deduped; full model (id/name/slug/count/timestamps); `core::outbox` `label.added`; 201 + Location; no X-Cache | PARTIAL | Ghost `labels.js:79-96`,`headers.ts:153-185`,`model/label.js:46-61`; C++ `create/.../execute.cppm:16-83`,`create/handler.cppm:40-85` |
| PUT /labels/:id | edit, docName labels | path `:id`; body `data.labels[0]`; `options[]` id/include; id required; include∈`count.members`; JSON-schema `labels-edit` | same | 200; `X-Cache-Invalidate: /*` only if `wasChanged()` | `Label.edit(...).catch(handleDuplicateNameError)`; `NotFoundError`; `emitChange('edited')` | invalidation header only on change | `{"labels":[{full model}]}` | `admin/labels/edit/{handler.cppm,workflow/...}` | path id resolved; resource-store edit persists name/slug/updated_at; duplicate → `ValidationError("Label already exists")`; 404 `Label not found.`; `core::outbox` `label.edited` only when changed; handler emits `/*` only when changed | PARTIAL | Ghost `labels.js:98-131`,`model/label.js:30-36`; C++ `edit/.../execute.cppm:16-103`,`edit/handler.cppm:40-83` |
| DELETE /labels/:id | destroy, docName labels | path `:id`; `options[]` id; id required; `require:true` | same | **204**; `X-Cache-Invalidate: /*`; 404 on miss | `Label.destroy({...options,require:true})` in transaction with linked-member lock/detach; `emitChange('deleted')` | invalidation header | 204 → empty body | `admin/labels/destroy/{handler.cppm,workflow/...}` | id resolved; 404 `Label not found.` on miss; `core::outbox` transaction + `label.deleted` event around ResourceStore destroy; 204 empty body + `/*` header | PARTIAL | Ghost `labels.js:133-153`,`model/label.js:144-175`; C++ `destroy/.../execute.cppm:16-58`,`destroy/handler.cppm:40-85` |

## Strict-parity implementation notes

### Shared abstractions used

- `core::query` — `parse()` with per-route option allowlists (`include`, `filter`,
  `fields`, `limit`, `order`, `page`, `id`), NQL `filter[id]=`/`filter[slug]=`
  lookups, restricted-field guard (`has_restricted_field` → Ghost 400
  `BadRequestError`), `apply()` for filter/order/field projection/pagination,
  and the `name asc,created_at desc` Ghost default order on browse.
- `core::auth::ghost` + `blog::handlers::middleware::require_ghost_permission`
  — every handler builds a principal from `x-blog-*` headers (an presented
  admin token is mapped to `ADMIN_API`/Administrator/`*`, the C++ stand-in for
  Ghost's `authAdminApi` token→user resolution), then enforces
  `permissions:true` for `docName = "labels"` and Ghost op (`browse`/`read`/
  `add`/`edit`/`destroy`). Denials write a Ghost `NoPermissionError`/
  `UnauthorizedError` envelope — no inline presence-only token check remains.
  Router-level `content_type_middleware`/`require_auth` were removed (Ghost
  uses body parsers, not a Content-Type gate).
- `blog::mapping` — `clean(rows,"label")` strips restricted fields, `wrap()`
  builds the `{"labels":[...]}`/`{"labels":[...],"meta":...}` envelopes,
  `pagination()` builds `meta.pagination`.
- `core::outbox` — `label.added` / `label.edited` / `label.deleted` events are
  emitted through `global_outbox()`; destroy wraps the mutation in
  `begin_transaction()`/`commit()`/`rollback()` to mirror Ghost's transaction.
- `core_adapters` — not relevant to labels (no Stripe/Tinybird/SMTP/webmention
  side effects in `labels.js`); documented, not wired.
- `blog_context` store — labels live in `ResourceStore` under `resource:labels:*`
  (the `json_for("labels")` fixture / `find_by_resource("labels")` nullopt
  branches in the shared context are deliberately bypassed; no shared file was
  edited). Shared label helpers live in the already-registered
  `blog_handlers:labels_browse_step` partition and are imported by the other
  five steps.

### Central registrations

None required. All touched files were already registered partitions in
`include/blog/handlers/BUILD.bazel`; `blog_handlers:mapping`, `core_query`,
`core_outbox`, `core_auth:ghost` and `blog_handlers_middleware:ghost_permission`
are already wired centrally by the main agent. No new partition, import in
`all.cppm`/`base.cppm`, or BUILD entry was added by this group.

### Residual gaps (no C++ primitive in this folder)

- Real Bookshelf/Knex `labels` table and `findPage` SQL (in-memory
  `ResourceStore` instead); `emitChange` reaches the outbox, not Ghost's
  cache-invalidation subscriber bus.
- JSON-schema `labels-add`/`labels-edit` validators: name presence, 1..191
  length and no-comma pattern are checked locally with Ghost-shaped
  `ValidationError` envelopes; exact `@tryghost/admin-api-schema` wording is
  approximated.
- `count.posts` is `0` (no `posts_labels` relation in the store);
  `include=count.members` counts real `members_labels` rows when present.
- Destroy: no SQL row lock on `members_labels` or member detach (in-memory
  store has no linked rows); the transaction is the shared outbox transaction.
- Admin token → role mapping is presence-based (no token store in this folder);
  real token verification and admin-token allowlisting remain in the shared
  auth middleware owned by the main agent.

## Route details

### GET /labels — PARTIAL

Ghost validates `include ∈ count.members`, applies `restrictAdminApiQueryOptions`
(restricted fields/order → `BadRequestError`), calls `Label.findPage` with default
order `name ASC, created_at DESC`, and emits `{"labels":[...],"meta":{pagination}}`
with `cacheInvalidate:false` (`labels.js:32-48`). C++ parses the allowlisted
options with `core::query::parse`, rejects invalid `include` with 422, converts
restricted-field `runtime_error` to a Ghost 400 envelope, runs `core::query::apply`
twice (once with `limit=all` for the filtered total, once paged) over
`ResourceStore` labels rebuilt into Ghost's wire shape by `support::rows`, and
builds the envelope with `blog::mapping::pagination`/`wrap`. No `X-Cache-Invalidate`
header is written.

### GET /labels/:id and GET /labels/slug/:slug — PARTIAL

Both read routes parse `include/filter/fields` with `core::query`, validate
`include`, apply the restricted-field guard, take the last path segment, and
use `core::query::apply` with `filter[id]=`/`filter[slug]=` against real label
rows. Missing rows return `NotFoundError("Label not found.")` (404); hits return
`{"labels":[{full model}]}`. The shared-store `find_by_resource("labels")`
nullopt branch and the hardcoded `LabelWire` fixture are no longer called.

### POST /labels — PARTIAL

Unwraps `data.labels[0]`, enforces name presence/length, trims the name,
generates and de-duplicates the slug, rejects duplicate names with
`ValidationError("Label already exists")`, persists a full label record in
`ResourceStore`, emits `label.added` on `core::outbox`, and returns 201 with a
`Location` header and no `X-Cache-Invalidate`.

### PUT /labels/:id — PARTIAL

Resolves the path id, returns 404 `Label not found.` for unknown ids, applies
partial label updates to `ResourceStore` (including a regenerated slug when the
name changes), rejects duplicate names, emits `label.edited` only when the
record changed, and sets `X-Cache-Invalidate: /*` only in that case.

### DELETE /labels/:id — PARTIAL

Resolves the path id, returns 404 `Label not found.` for unknown ids, runs the
destroy inside a `core::outbox` transaction emitting `label.deleted`, and returns
204 with an empty body and `X-Cache-Invalidate: /*`.

## Module-level notes

- Route coverage: six Ghost label routes ↔ six C++ routes; no EXTRA/MISSING.
- Output model: `support::canonical()` emits Ghost's serialized label field set
  (`id`, `name`, `slug`, `count`, `created_at`, `updated_at`) and maps numeric
  resource ids to Ghost ObjectID-shaped wire ids; `fields` narrows it.
- Error envelopes use `blog::api::ApiError` with Ghost types
  (`ValidationError`, `NotFoundError`, `BadRequestError`).
- Files changed: `routes.cppm`, six `handler.cppm`, six
  `workflow/step/execute.cppm` under `include/blog/handlers/admin/labels/`;
  both parity docs. No shared file modified.
