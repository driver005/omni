# Parity: labels

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/labels.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/admin/labels/routes.cppm`
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-19 (manifest group 2)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 6 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/labels` | `browse` | GET `/labels` | `labels_browse()` | `core::query::parse` allowlists include/filter/fields/limit/order/page; include ∈ `count.members`; restricted-field guard | `{"labels":[...],"meta":{"pagination":...}}` via `blog::mapping::wrap`/`pagination`; filtered total from real rows | `core::query::apply` over `ctx.store().resources().list("labels")`, default order `name asc,created_at desc`, `count.members` from `members_labels` | no X-Cache header (`cacheInvalidate:false`) | `require_ghost_permission("labels","browse")` | PARTIAL | G `routes.js:294`, `labels.js:32-48`; C++ `browse/.../execute.cppm`, `browse/handler.cppm` |
| GET `/labels/:id` | `read` | GET `/labels/:id` | `labels_read_by_id()` | path id (numeric ↔ wire id), `core::query` options + `filter[id]=`, include validation, restricted guard | `{"labels":[{full model}]}` or 404 `Label not found.` | `core::query::apply` over real stored labels | no X-Cache header | `require_ghost_permission("labels","read")` | PARTIAL | G `routes.js:295`, `labels.js:50-77`; C++ `read_by_id/.../execute.cppm`, `read_by_id/handler.cppm` |
| GET `/labels/slug/:slug` | `read` | GET `/labels/slug/:slug` | `labels_read_by_slug()` | path slug via `filter[slug]=`, same options/validation | `{"labels":[{full model}]}` or 404 `Label not found.` | `core::query::apply` over real stored labels | no X-Cache header | `require_ghost_permission("labels","read")` | PARTIAL | G `routes.js:296`, `labels.js:50-77`; C++ `read_by_slug/.../execute.cppm`, `read_by_slug/handler.cppm` |
| POST `/labels` | `add` | POST `/labels` | `labels_create()` | `data.labels[0]` unwrap, `include` validation, name required/≤191/trimmed, duplicate → `ValidationError("Label already exists")`, slug generated + deduped | 201 `{"labels":[{full model}]}` + `Location`; `label.added` outbox event | `ctx.store().resources().create("labels", ...)` | no X-Cache header | `require_ghost_permission("labels","add")` | PARTIAL | G `routes.js:297`, `labels.js:79-96`; C++ `create/.../execute.cppm`, `create/handler.cppm` |
| PUT `/labels/:id` | `edit` | PUT `/labels/:id` | `labels_edit()` | path id + `data.labels[0]` unwrap, include/duplicate validation | 200 `{"labels":[{full model}]}`; 404 `Label not found.`; `X-Cache-Invalidate: /*` only on change; `label.edited` event | `ctx.store().resources().edit("labels", ...)` | `/*` only when `wasChanged()` | `require_ghost_permission("labels","edit")` | PARTIAL | G `routes.js:298`, `labels.js:98-131`; C++ `edit/.../execute.cppm`, `edit/handler.cppm` |
| DELETE `/labels/:id` | `destroy` | DELETE `/labels/:id` (`.delt`) | `labels_destroy()` | path id, `require:true` | 204 empty body, 404 `Label not found.` on miss; `label.deleted` outbox event in transaction | `ctx.store().resources().destroy("labels", ...)` | `/*` (`cacheInvalidate:true`) | `require_ghost_permission("labels","destroy")` | PARTIAL | G `routes.js:299`, `labels.js:133-153`; C++ `destroy/.../execute.cppm`, `destroy/handler.cppm` |

## Fix implementation record

Applied on 2026-11-19 to `include/blog/handlers/admin/labels/`:

1. Input: every step now parses its query through `core::query::parse(path,
   allowlist)`; `include` is validated against `count.members`; browse applies
   restricted-field guard + NQL filter/order/fields/pagination via
   `core::query::apply`; read-by-id/slug use `filter[id]=`/`filter[slug]=`;
   create/edit unwrap `data.labels[0]`; destroy parses `id`.
2. DB call: the broken shared-store branches (`json_for("labels")` fixture,
   `find_by_resource("labels")` nullopt, fabricated `edit`) are bypassed. Labels
   use `ctx.store().resources()` directly with Ghost-shaped serialization
   (`support::rows`/`canonical`). Shared store files were NOT edited.
3. Output: `blog::mapping::wrap`/`pagination` produce Ghost envelopes;
   `support::canonical` emits id/name/slug/count/created_at/updated_at; `fields`
   narrows via `core::query`; 404/422/400 errors are Ghost-shaped envelopes
   (`NotFoundError`, `ValidationError`, `BadRequestError`).
4. Cache: `X-Cache-Invalidate` is no longer emitted on browse/read/create
   (`cacheInvalidate:false`); edit emits `/*` only when the record changed;
   destroy emits `/*` on 204.
5. Side effects: `core::outbox::global_outbox()` records `label.added`,
   `label.edited` (on change) and `label.deleted` (inside a transaction).
6. Auth: router-level `content_type_middleware`/`require_auth` removed; each
   handler enforces `core::auth::ghost` via `require_ghost_permission` with
   `docName = "labels"` and the Ghost op. An admin token is mapped to the
   Administrator role since no token store exists in this folder.
7. Headers: create sets `Location: .../labels/<id>/` on 201.

### Verdict flip

Two routes flip: read-by-id and read-by-slug move from **MISMATCH** to
**PARTIAL** (real id/slug lookup with proper 404 replaces the hardcoded
`nullopt`/fixture behavior). Browse/create/edit/destroy stay **PARTIAL**: all
documented divergences are closed, but the infrastructure residuals below have
no C++ primitive in this folder, so no route is promoted to MATCH.

## Needs central registration

None. No new partition was created — shared label helpers live in the existing
`blog_handlers:labels_browse_step` partition, which is already listed in
`include/blog/handlers/BUILD.bazel` and imported transitively by the other five
steps. `blog_handlers:mapping`, `core_query`, `core_outbox`, `core_auth:ghost`
and `blog_handlers_middleware:ghost_permission` are already centrally wired
(`base.cppm` `export import :mapping`, `all.cppm` imports, `BUILD.bazel` deps).
No `all.cppm` / `base.cppm` / central `routes.cppm` / BUILD change is required.

## Remaining gaps (residual, no C++ primitive here)

- Real Knex `labels` table / `Label.findPage` SQL; outbox events are not
  Ghost's cache-invalidation bus; in-memory `ResourceStore` instead of DB.
- JSON-schema `labels-add`/`labels-edit`: name presence/1..191/no-comma checked
  locally with Ghost-shaped errors; exact `@tryghost/admin-api-schema` wording
  approximated.
- `count.posts` is always `0`; `include=count.members` reads real
  `members_labels` rows when present.
- Destroy has no SQL row lock on `members_labels` or member detach; the
  transaction is the shared outbox transaction.
- Token→role mapping is presence-based; real admin-token verification and
  allowlisting stay in the main agent's shared auth middleware.
- `core_adapters` is not used: labels have no Stripe/Tinybird/SMTP/webmention
  side effects.

## Missing in C++

None. All six Ghost label routes have a C++ route.

## Extra in C++

None. `LabelsRoutes::install` mounts exactly the six Ghost label routes.
