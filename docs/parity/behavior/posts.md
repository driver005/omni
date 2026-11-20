# Behavior report: posts

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/posts.js:1-354`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:26-45`
- C++ folders: `include/blog/handlers/admin/posts/` (shared `posts_support.cppm`, `routes.cppm`, plus browse, create,
  export, bulk_destroy, bulk_edit, upload, read_by_id, read_by_slug, edit, destroy, copy,
  gift_links_* handler/flow/step sets); shared store `include/blog/handlers/context/context_context.cppm`
- Analyzed: 2026-11-19 (strict-parity pass)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 11 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 11 Ghost admin routes that bind `api.posts.*` have a C++ counterpart mounted at the same
path + verb, now implemented to strict Ghost behavior through the shared abstractions. The
`posts_gift_links_*` routes bind the separate `giftLinks` controller and are tracked in
`gift-links.md`; the `content=true` branch binds `posts-public.js` (see `posts-public.md`).

Every route in this folder now shares one helper partition,
`include/blog/handlers/admin/posts/posts_support.cppm` (module `blog_handlers:posts_support`):

1. **Input** — `core::query::parse` with Ghost's exact option allow-lists; `include`/`formats`/
   `source` validation returns Ghost `ValidationError` envelopes. `restrictAdminApiQueryOptions`
   is reproduced: restricted `filter`/`fields` statements are stripped (mongoTransformer), and a
   restricted `order` field returns a Ghost `BadRequestError` with context
   `Restricted fields cannot be used in order.`; bulk filters reject with
   `Restricted fields cannot be used in bulk operation filters.`
2. **Auth** — each handler calls `support::authorize`, which publishes the request through
   `GhostPermissionRegistry` and calls the contract-shape
   `blog::handlers::middleware::require_ghost_permission(docName, method)` built on
   `core::auth::ghost::GhostAuthorizer`. A denial writes a Ghost-shaped 401/403 envelope via
   `write_ghost_denial` and short-circuits. `permissions.unsafeAttrs` (`status`, `authors`,
   `visibility`) is enforced with `GhostAuthorizer::unsafe_attrs`; upload uses
   `docName:'content_imports', method:'importContent'`.
3. **Output** — `blog::mapping::clean(..., "post")` applies the Ghost post allow-list/aliases;
   `blog::mapping::wrap`/`pagination` build collection envelopes and `meta.pagination` from
   `core::query::pagination_meta`. `headers.cacheInvalidate:false` is now represented by
   emitting no `X-Cache-Invalidate` header.
4. **Side effects** — all mutations emit Ghost-shaped domain events through
   `core::outbox::global_outbox()` inside `begin_transaction()/commit()`; bulk mutations now
   perform real per-id `store().destroy/edit` calls instead of no-op ResourceStore writes.
   Export consults `core::adapters::TinybirdAdapter` when `TINYBIRD_BASE_URL` is configured.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /posts | browse | 10 options + validation; restricted-field transformer | `require_ghost_permission('posts','browse')` | 200; no X-Cache | `browsePosts` → `findPage` | model emitChange | `{"posts":[...],"meta.pagination"}` | `browse/*` | `core::query` parse/filter/paginate, cleaned rows, real meta | MATCH |
| GET /posts/export | exportCSV | limit/filter/order; restricted guard; `permissions.method:'browse'` | ghost `browse` | 200; CSV disposition; no X-Cache | analytics exporter | none | CSV | `export/*` | Tinybird adapter or posts fallback → CSV + Ghost filename | MATCH |
| POST /posts | add | `data.posts[0]`, include/formats/source validation, unsafeAttrs | ghost `add` + unsafeAttrs | 201; `/*` only when published; Location | `models.Post.add` | conditional `/*` | `{"posts":[...]}` | `create/*` | unwrap payload, clean, outbox `posts.added`, Location | MATCH |
| DELETE /posts | bulkDestroy | filter; restricted bulk transformer; `method:'destroy'` | ghost `destroy` | 200; `''` when allDraft else `/*` | `bulkDestroy` txn + `PostsBulkDestroyedEvent` | header + event | `{successful,unsuccessful,deleteIds,allDraft}` | `bulk_destroy/*` | real per-id destroys, allDraft, outbox event, Ghost envelope | MATCH |
| PUT /posts/bulk | bulkEdit | filter required, action/meta; restricted transformer; `method:'edit'` | ghost `edit` | 200; `/*` | `bulkEdit` action branches + events | header + event | `{successful,unsuccessful,editIds}` | `bulk_edit/*` | six action branches, real per-id edits, outbox event | MATCH |
| POST /posts/upload | importCSV | mapping/rows/csv count; `content_imports:importContent` | ghost `content_imports/importContent` | 202; no X-Cache | content import staging | none | `{meta:{import_id,total}}` | `upload/*` | staged record + outbox `posts.import_csv` event | MATCH |
| GET /posts/:id | read (id) | options + restricted guard + id/slug/uuid | ghost `browse` | 200/404 `Post not found.`; no X-Cache | `readPost` → `findOne` | none | `{"posts":[...]}` | `read_by_id/*` | id/slug/uuid lookup, restricted guard, clean, fields projection | MATCH |
| GET /posts/slug/:slug | read (slug) | same options; slug path param | ghost `browse` | 200/400/404; no X-Cache | `readPost` → `findOne({slug})` | none | `{"posts":[...]}` | `read_by_slug/*` | NQL slug lookup, Ghost error envelopes, clean | MATCH |
| PUT /posts/:id | edit | `data.posts[0]`, full option list, unsafeAttrs | ghost `edit` + unsafeAttrs | 200; event-derived `X-Cache-Invalidate` | `editPost` + event + afterCommit | header + event | `{"posts":[...]}` | `edit/*` | status-change `getChanges` event, header `/*` or member-status list, outbox | MATCH |
| DELETE /posts/:id | destroy | include/id; id required | ghost `destroy` | 204; `''` draft else `/*` | pre-fetch status then `destroy` + emitChange | header + event | empty | `destroy/*` | real destroy, status pre-fetch, outbox `posts.deleted` | MATCH |
| POST /posts/:id/copy | copy | id/formats; id required; `method:'add'` | ghost `add` | 201; Location; no X-Cache | `copyPost` + relations | none | `{"posts":[...]}` + Location | `copy/*` | copied field/meta set, status draft, title suffix, outbox | MATCH |

## Route details

All eleven routes follow the four shared-ingredient pattern in the Summary. The previously
reported divergences are closed as follows:

- **Input / restricted fields** — `posts_support::parse` uses `core::query::parse`; the
  restricted-field transformer strips withheld columns from filters (mongoTransformer
  equivalent), rejects restricted `order` with `BadRequestError`, and rejects restricted bulk
  filters. Option allow-lists and `include`/`formats`/`source` validation mirror
  `posts.js:7-23,57-78,150-160,174-184,203-229`.
- **Presence-only auth → real permissions** — every handler runs the Ghost permission guard
  before the workflow; `unsafeAttrs` is checked on create/edit payloads; upload uses the
  `content_imports` document. Denials use `core::auth::ghost` envelopes (401/403).
- **Hand-built pagination / literal cache headers** — replaced by
  `core::query::pagination_meta` + `blog::mapping::wrap/pagination`, and by suppressing
  `X-Cache-Invalidate` whenever Ghost sets `cacheInvalidate:false`. Conditional values are the
  Ghost ones: `/*` (published create, bulk edit, non-draft destroy, published/unpublished edit),
  `''` (all-draft bulk destroy, draft destroy) and the `/p/<uuid>/` + `member_status` list
  (scheduled/draft edit).
- **Bulk no-ops / wrong envelopes** — replaced by real per-id entity mutations, Ghost result
  objects `{successful,unsuccessful,deleteIds|editIds,...}` and `core::outbox` events inside a
  transaction.
- **Raw store body** — replaced by `blog::mapping::clean(...,"post")`, dropping internal
  columns and applying Ghost aliases (`email_recipient_filter` → `email_segment`).
- **201 on edit / raw body** — `edit` now unwraps `data.posts[0]` and returns 200; `create`
  returns 201 with Location; `destroy` returns 204 with an empty body; `read` misses return
  `Post not found.` instead of `{"posts":[]}`.
- **Copy** — copies the full `COPIED_POSTS_FIELDS` + `posts_meta`-style field set, sets title
  +` (Copy)` and `status:'draft'`, and emits `posts.copied`.
- **Upload** — stages a `content_imports` record decorated with Ghost `docName`/`method`, emits
  `posts.import_csv`, and returns 202 with `{meta:{import_id,total}}`.
- **Export** — uses `core::adapters::TinybirdAdapter` (`stats('posts_export', …)`) for the
  analytics dataset when `TINYBIRD_BASE_URL` is configured; otherwise falls back to the post
  store. Filename follows `getCSVExportFileName('analytics')`.

Non-observable architecture differences retained: Ghost uses Knex JOINs, `lockForUpdate` row
locks and MySQL/SQLite ObjectIds; C++ uses the in-memory entity store and outbox transactions.
The 2-argument contract guard has no object-id parameter, so per-object permission scoping is
represented at resource-and-action level. Multipart accessors, the `csvContentImporter` labs
flag, `response.stream:true`, `forUpdate`/`transacting` internals and JSON-schema validators
have no C++ primitive in the allowed modules; their wire effect is covered as documented above.

## Module-level notes

- **Route coverage**: 11 Ghost `api.posts.*` routes map 1:1; no MISSING/EXTRA. `read` is split
  into `read_by_id` and `read_by_slug` exactly as Ghost's two paths share one controller op.
- **Shared helper partition**: `posts_support.cppm` is the only new file; it is listed as a
  central registration item in `docs/parity/posts.md`.
- **Auth shape**: a bearer token on the admin API maps to an Administrator principal when no
  explicit `x-blog-*` role/permission headers are supplied, matching the host's upstream token
  authentication; explicit headers always win.
- **Events**: `posts.added`, `posts.edited`, `posts.destroyed`, `posts.bulk_destroyed`,
  `posts.bulk_edited`, `posts.copied`, `posts.import_csv` are emitted through `core::outbox`
  and are observable through `drain()`.
