# Parity: gift-links

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/gift-links.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 72-78)
- C++: `include/blog/handlers/admin/posts/routes.cppm`, `include/blog/handlers/admin/pages/routes.cppm`, `include/blog/handlers/membership/gift_links/routes.cppm`; shared ops `include/blog/handlers/membership/gift_links/operations.cppm`
- Checked: 2026-11-19 (strict-parity pass)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 7 Ghost routes have a bound C++ route with matching path + verb and strict Ghost behavior.
The earlier report's 7 MISMATCH verdicts were from stub handlers; all are now implemented via
the shared abstractions (`core::query`, `core::auth::ghost` + `require_ghost_permission`,
`blog::mapping`, `core::outbox`). The 3 pages routes previously reported MISSING are now
mounted in `admin/pages/routes.cppm`.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/posts/:id/gift_links` | `browse` | `admin/posts/routes.cppm` `/posts/:id/gift_links` GET | `posts_gift_links_browse()` | `id` parsed from path; 400 Ghost ValidationError if empty | `{gift_links:[{token,created_at}]}` via `core::query` projection + `blog::mapping::wrap`; 404 Ghost `Post <id> does not exist.` | `gift_links_ops::browse` → `find_by_resource` + association scan | none (Ghost `cacheInvalidate:false`) | `require_auth` + ghost gate `gift_links:manage` + `posts:edit` | MATCH | Ghost `routes.js:72`, `gift-links.ts:36-46`; C++ `posts/gift_links_browse/*`, `operations.cppm` |
| PUT `/posts/:id/gift_links` | `ensure` | `admin/posts/routes.cppm` `/posts/:id/gift_links` PUT | `posts_gift_links_edit()` | `id` path; actor from request; body ignored | same envelope; idempotent (existing link returned, no action) | `gift_links_ops::ensure` → mint-if-absent: real 24B base64url token, real ISO date, history insert + upsert, `actions` record + outbox event in transaction | none | `require_auth` + ghost gate (`manage` + `posts:edit`) | MATCH | Ghost `routes.js:73`, `gift-links.ts:48-59`, `service.ts:64-74`; C++ `posts/gift_links_edit/*`, `operations.cppm` |
| POST `/posts/:id/gift_links` | `create` | `admin/posts/routes.cppm` `/posts/:id/gift_links` POST | `posts_gift_links_create()` | `id` path; actor from request; body ignored | same envelope, fresh token | `gift_links_ops::reset` → always mint (upsert), `edited`/`reset` action + outbox event in transaction | none | same gate | MATCH | Ghost `routes.js:74`, `gift-links.ts:61-72`, `service.ts:80-87`; C++ `posts/gift_links_create/*`, `operations.cppm` |
| GET `/pages/:id/gift_links` | `browse` | `admin/pages/routes.cppm` `/pages/:id/gift_links` GET (now mounted) | `pages_gift_links_get()` | `id` path | same as posts browse | `gift_links_ops::browse(store,id,"pages")` | none | `require_auth` + ghost gate (`manage` + `pages:edit`) | MATCH | Ghost `routes.js:75`; C++ `pages/routes.cppm`, `pages/gift_links_get/*` |
| PUT `/pages/:id/gift_links` | `ensure` | `admin/pages/routes.cppm` `/pages/:id/gift_links` PUT (now mounted) | `pages_gift_links_put()` | `id` path; actor from request | same as posts ensure | `gift_links_ops::ensure(store,id,"pages",actor)` | none | same gate | MATCH | Ghost `routes.js:76`; C++ `pages/routes.cppm`, `pages/gift_links_put/*` |
| POST `/pages/:id/gift_links` | `create` | `admin/pages/routes.cppm` `/pages/:id/gift_links` POST (now mounted) | `pages_gift_links_post()` | `id` path; actor from request | same as posts create | `gift_links_ops::reset(store,id,"pages",actor)` | none | same gate | MATCH | Ghost `routes.js:77`; C++ `pages/routes.cppm`, `pages/gift_links_post/*` |
| PUT `/gift_links/remove_all` | `removeAll` | `membership/gift_links/routes.cppm` `/gift_links/remove_all` PUT | `gift_links_gift_links_remove_all_put()` | none | `{meta:{count:N}}` | `gift_links_ops::remove_all`: destroy all `post_gift_links`, keep history, `deleted`/`remove` action + outbox event if N>0, in transaction | none | `require_auth` + ghost gate `gift_links:removeAll` | MATCH | Ghost `routes.js:78`, `gift-links.ts:74-84`, `serializers.ts:19-22`; C++ `membership/gift_links/routes.cppm`, `gift_links_remove_all_put/*`, `operations.cppm` |

## Route details

Every route uses the same four shared ingredients from `docs/parity/behavior/CONTRACTS.md`:

1. **Auth/permissions** — the handler calls `gift_links_ops::permitted(req,res,send,docName,method)`
   before running the workflow. It publishes the request/response to `GhostPermissionRegistry`
   and calls the contract-shape `require_ghost_permission(docName, method)`. A denial writes a
   Ghost-shaped `UnauthorizedError` (401) or `NoPermissionError` (403) envelope via
   `write_ghost_denial`. A valid admin bearer token maps to an Administrator principal; explicit
   `x-blog-*` headers always win. Posts/pages routes additionally check `posts|pages:edit`,
   mirroring `assertCanEditAndGift`'s `edit.post(id)` clause.
2. **Input** — `parse_resource_id` extracts the `:id` path segment (Ghost injects it into
   `frame.options.id`); empty id returns a Ghost `ValidationError`. `actor_from_request` maps
   the request to the Ghost actor (`user`/`integration`).
3. **Output** — `core::query::apply` with a `fields = {token, created_at}` projection implements
   the Ghost `toGiftLinksResponse` field set; `blog::mapping::wrap("gift_links", ...)` builds the
   envelope. `remove_all` uses `encode_count` for `{meta:{count:N}}`. No DTO changes were needed.
4. **DB/side effects** — `ApiDataStore` holds state; `mint` generates a real 24-byte base64url
   token and UTC ISO timestamp, inserts a `gift_links` history row and upserts the single
   `post_gift_links` live association (`ResourceStore::edit`). `record_action` writes an
   `actions` record and emits a `gift_link.action` event through `core::outbox` inside a
   `begin_transaction()/commit()` block (Ghost `ensureTransaction` + `Action.add`).

Residuals from the earlier report are closed:
- fine-grained permissions → `core::auth::ghost` + `require_ghost_permission`;
- action history + transaction → `core::outbox` + `actions` store writes;
- fabricated token/date → `generate_token`/`now_iso`;
- double auth gate / non-Ghost 401 on the pages and remove_all handlers → removed, single Ghost-shaped gate;
- non-atomic erase+mint → single upsert inside an outbox transaction;
- `cacheInvalidate:false` → correct by emitting no `X-Cache-Invalidate` header.

Non-observable architecture differences retained: Ghost uses Knex JOINs and `forUpdate()` row
locks / rollback; C++ uses the in-memory store and outbox transaction. The shared
`require_ghost_permission(docName, method)` contract has no object-id parameter, so Ghost's
per-object `gift_link(id)` / `post(id)` scoping is represented at resource-and-action level.

## Missing in C++

None. All 7 Ghost routes have a C++ route with matching path + verb:
- `/posts/:id/gift_links` GET/PUT/POST → `admin/posts/routes.cppm`
- `/pages/:id/gift_links` GET/PUT/POST → `admin/pages/routes.cppm` (bindings added in this pass)
- `/gift_links/remove_all` PUT → `membership/gift_links/routes.cppm`

## Extra in C++

None. Every C++ gift-links route maps 1:1 to a Ghost route in `routes.js:72-78`.

## Needs central registration

No new central registrations are required. Recorded here for the central owner:
1. **No new partitions/files** — all 7 handlers, steps, flows and the `blog_handlers:gift_links_ops`
   partition were already listed in `include/blog/handlers/BUILD.bazel` and `all.cppm`/`base.cppm`.
   No `import`/`export import` additions to central module interfaces.
2. **Shared dependencies already present** — `include/blog/handlers/BUILD.bazel` already lists
   `//include/core/auth:core_auth`, `//include/core/outbox:core_outbox`,
   `//include/core/query:core_query`, `//include/core/adapters:core_adapters`,
   `//include/core/serde:core_serde`, `:blog_handlers_middleware` and the three
   `mapping/*.cppm` partitions. The `operations.cppm` partition's new imports
   (`:mapping`, `interfaces`, `core_query`, `core_outbox`, `core_auth`,
   `blog_handlers_middleware`) therefore need no BUILD change.
3. **Route bindings** — the 3 `Route{"/gift_links"}` bindings were added in-scope to
   `admin/pages/routes.cppm`; no change to central `routes.cppm` or `route_catalog.cppm`
   (which already lists all 7 routes).
4. **DTOs unchanged** — `types/dto_links.cppm` remains byte-identical; output is produced by
   `core::query` + `blog::mapping` instead.
