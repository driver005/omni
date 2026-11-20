# Behavior report: gift-links

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/gift-links.ts` (`module.exports`, docName `gift_links`)
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:72-78`
- Ghost service: `docs/Ghost/ghost/core/server/services/gift-links/{service,serializers,models,codec,actions,index,schema}.ts`
- Ghost output serializer: `docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/gift-links.ts`
- C++ folders: `include/blog/handlers/admin/posts/gift_links_{browse,edit,create}/`, `include/blog/handlers/admin/pages/gift_links_{get,put,post}/`, `include/blog/handlers/membership/gift_links/` (shared `operations.cppm`, remove_all handler)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All 7 Ghost routes now have a bound C++ route with matching path + verb and Ghost-equivalent
behavior. The 3 previously orphaned pages routes are wired into `admin/pages/routes.cppm`.
All four residual gap classes from the earlier report are closed using the shared
abstractions from `CONTRACTS.md`: fine-grained permissions via `core::auth::ghost` +
`require_ghost_permission`, action history / transactions via `core::outbox`, output field
projection via `core::query` + `blog::mapping`, and real token/timestamp generation in the
shared ops module. No EXTRA routes.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/posts/:id/gift_links` | browse | `:id`→`options.id`; `options:['id']`, `id.required` (`:38-39`); no body/query/headers | `mw.authAdminApi` (`routes.js:72`); `assertCanEditAndGift`: `manage.gift_link(id)` + `edit.post(id)` (`:14-18,40-42`) | 200; `cacheInvalidate:false` (`:31,37`) | `getPost(id)`→3-table JOIN→`postFrom` (404)→`z.decode(giftLinkCodec)` (`service.ts:24-47`) | no header; no cache service | `{gift_links:[{token,created_at}]}` | `posts/gift_links_browse/*`; `membership/gift_links/operations.cppm` | route GET; permission gate (`gift_links:manage` + `posts:edit`) via `require_ghost_permission`; `:id` path parse→`gift_links_ops::browse`; 404 Ghost NotFoundError `Post <id> does not exist.`; output via `core::query::apply` (project `token,created_at`) + `blog::mapping::wrap`; no header emitted (matches `cacheInvalidate:false`) | MATCH | Ghost `routes.js:72`, `gift-links.ts:36-46`, `service.ts:24-47`; C++ `posts/routes.cppm:135-143`, `gift_links_browse/{handler.cppm,workflow/step/execute.cppm}`, `operations.cppm` |
| PUT `/posts/:id/gift_links` | ensure | same `id` input (`:51-52`); no body used | router auth (`:73`); same `assertCanEditAndGift` (`:53-55`) | 200; `cacheInvalidate:false` (`:49-50`) | `ensure`→`ensureTransaction`→`lockedPost`→idempotent `mint` if no live link→`recordAction` verb `add` (`service.ts:64-74,113-134`; `actions.ts:23-27,47-82`) | no header | same `{gift_links:[{token,created_at}]}` | `posts/gift_links_edit/*`; `operations.cppm` | permission gate; `ensure`: 404, return existing link unchanged (no action) or `mint` inside `core::outbox` transaction: real 24-byte base64url token + UTC ISO date, `gift_links` history insert + `post_gift_links` upsert, `actions` record + outbox event `added` | MATCH | Ghost `routes.js:73`, `gift-links.ts:48-59`, `service.ts:64-74`; C++ `posts/routes.cppm:144-151`, `gift_links_edit/step/execute.cppm`, `operations.cppm` |
| POST `/posts/:id/gift_links` | create | same `id` input (`:64-65`); no body | router auth (`:74`); same `assertCanEditAndGift` (`:66-68`) | 200; `cacheInvalidate:false` (`:62-63`) | `create`→`ensureTransaction`→`lockedPost`→always `mint` (upsert)→`recordAction` `reset`→`edited` + `context.action_name:'reset'` (`service.ts:80-87,118-134`; `actions.ts:23-27,47-82,74`) | no header | same envelope | `posts/gift_links_create/*`; `operations.cppm` | permission gate; `reset`: 404, `mint` always (upsert replaces live association), transaction + `actions` record + outbox event `edited` with `action_name:'reset'` | MATCH | Ghost `routes.js:74`, `gift-links.ts:61-72`, `service.ts:80-87`; C++ `posts/routes.cppm:152-159`, `gift_links_create/step/execute.cppm`, `operations.cppm` |
| GET `/pages/:id/gift_links` | browse | same as posts browse (`gift-links.ts:36-46`) | `routes.js:75` auth + same permissions | 200; `cacheInvalidate:false` | identical `getPost` chain, no type branch (service uses `posts` table) | no header | same envelope | `pages/gift_links_get/*`; `admin/pages/routes.cppm` | **route now bound** under `/pages/:id/gift_links`; permission gate (`gift_links:manage` + `pages:edit`); `gift_links_ops::browse(store,id,"pages")`; Ghost-shaped 404/400 and output | MATCH | Ghost `routes.js:75`; C++ `pages/routes.cppm` `/gift_links` GET, `pages/gift_links_get/{handler.cppm,workflow/step/execute.cppm}` |
| PUT `/pages/:id/gift_links` | ensure | same as posts ensure (`gift-links.ts:48-59`) | `routes.js:76` auth + permissions | 200; `cacheInvalidate:false` | identical `ensure` chain | no header | same envelope | `pages/gift_links_put/*`; `admin/pages/routes.cppm` | route bound; permission gate; `gift_links_ops::ensure(store,id,"pages")` with real token/date, outbox action/transaction; no body read (Ghost ignores body) | MATCH | Ghost `routes.js:76`; C++ `pages/routes.cppm` `/gift_links` PUT, `pages/gift_links_put/{handler.cppm,workflow/step/execute.cppm}` |
| POST `/pages/:id/gift_links` | create | same as posts create (`gift-links.ts:61-72`) | `routes.js:77` auth + permissions | 200; `cacheInvalidate:false` | identical `create` chain | no header | same envelope | `pages/gift_links_post/*`; `admin/pages/routes.cppm` | route bound; permission gate; `gift_links_ops::reset(store,id,"pages")` with fresh token/upsert + `edited`/`reset` action | MATCH | Ghost `routes.js:77`; C++ `pages/routes.cppm` `/gift_links` POST, `pages/gift_links_post/{handler.cppm,workflow/step/execute.cppm}` |
| PUT `/gift_links/remove_all` | removeAll (custom) | no `options`/`validation`/body/no `id` (`gift-links.ts:74-84`) | router auth (`:78`); `canThis(context).removeAll.gift_link()` (`:77-79`) | 200; `cacheInvalidate:false` (`:75-76`) | `removeAll`→`ensureTransaction` `trx('post_gift_links').del()`, record action `remove`→`deleted` only if removed>0; keeps `gift_links` history (`service.ts:94-102`; `actions.ts:47-82`) | no header | `{meta:{count}}` (`serializers.ts:19-22`) | `membership/gift_links/routes.cppm`; `gift_links_remove_all_put/*`; `operations.cppm` | permission gate `gift_links:removeAll` via ghost authorizer (redundant ad-hoc `{"error":"Unauthorized"}` check removed); `remove_all` deletes all live associations, keeps history, returns `{meta:{count:N}}`; transaction + `deleted`/`remove` action + outbox event when removed>0 | MATCH | Ghost `routes.js:78`, `gift-links.ts:74-84`, `service.ts:94-102`, `serializers.ts:19-22`; C++ `membership/gift_links/routes.cppm`, `gift_links_remove_all_put/{handler.cppm,workflow/step/execute.cppm}`, `operations.cppm` |

## Route details

### GET /posts/:id/gift_links — MATCH

**Ghost** (controller `browse`, docName `gift_links`)
1. `router.get('/posts/:id/gift_links', mw.authAdminApi, http(api.giftLinks.browse))` (`admin/routes.js:72`).
2. Op `browse` (`gift-links.ts:36`); `options:['id']`, `options.id.required:true` (`:38-39`). No body/query/headers read.
3. `permissions()`→`assertCanEditAndGift` (`:40-42`→`:14-18`): `manage.gift_link(id)` + `edit.post(id)`.
4. No `statusCode` (framework 200); `headers: noCacheInvalidation` where `noCacheInvalidation={cacheInvalidate:false}` (`:31,37`). No response headers.
5. `query`→`giftLinks.service.getPost(id)` (`:44`)→`postWithLiveLink` JOIN (service.ts:30-36)→`postFrom` (0 rows→`NotFoundError Post <id> does not exist.`, service.ts:38-47)→`z.decode(giftLinkCodec)`.
6. Cache: only `cacheInvalidate:false`; no cache service/key/TTL.
7. Output: `serializeGiftLinks`→`toGiftLinksResponse.parse(post.giftLinks)`→`{gift_links:[{token,created_at}]}` (`output/gift-links.ts:8-10`; `serializers.ts:12-17`). Empty→`{gift_links:[]}`; missing post→404.

**C++**
1. `Router{router,"/posts"}` with `content_type_middleware` + `require_auth` → `/:id` → `Route{"/gift_links"}.get()` → `posts_gift_links_browse()` (posts/routes.cppm:135-143).
2. Handler runs the ghost permission gate first: `gift_links_ops::permitted(req,res,send,"gift_links","manage")` and `("posts","edit")`. `permitted` publishes the ambient request/response through `GhostPermissionRegistry`, calls the contract-shape `require_ghost_permission`, and writes a Ghost 401/403 envelope via `write_ghost_denial` on denial. A valid admin bearer token is mapped to an Administrator principal; explicit `x-blog-*` headers always win.
3. Step parses `:id` from `input.get_path()`; empty→400 Ghost `ValidationError` (`validation_error_body()`).
4. `gift_links_ops::browse(store,id,"posts")`: `find_by_resource`→404 `not_found_body(id)` (Ghost message `Post <id> does not exist.`) else live-link lookup.
5. Output: `core::query::apply` projects the link onto Ghost wire fields `token,created_at` (restricted-field guard active); `blog::mapping::wrap("gift_links",...)` builds the envelope. No `X-Cache-Invalidate` header emitted — identical observable behavior to Ghost's `cacheInvalidate:false`.

**Diff**: No residual gaps. Permission decision, 404 message shape, wire projection, and cache behavior now match. Non-observable architecture detail: Ghost uses a Knex 3-table JOIN and row locking; C++ uses the in-memory `ApiDataStore` through `find_by_resource` + association scan (same observable result).

### PUT /posts/:id/gift_links — MATCH

**Ghost** (`ensure`)
1. Router PUT `.../gift_links`, `mw.authAdminApi` (`routes.js:73`); `assertCanEditAndGift` (`:53-55`).
2. `options:['id']`, `id.required` (`:51-52`); no body used.
3. `statusCode:200` (`:50`), `cacheInvalidate:false` (`:49`).
4. `query`→`ensure(requestContextFromFrame(frame),id)` (`:57`)→`ensureTransaction`→`lockedPost` (`forUpdate`)→if live link exists return it, else `mint`: `generateGiftLinkToken()` = `crypto.randomBytes(24).toString('base64url')` (models.ts:8-10), `addToHistory` insert `gift_links`, `setLiveLink` upsert `post_gift_links` `onConflict('post_id').merge` (service.ts:64-74,113-134)→`recordAction` verb `add`→`added` (actions.ts:23-27,47-82).
5. Actor from `context.integration`→`{id,type:'integration'}`, else `context.user`→`{id,type:'user'}`, else `null` (no action) (`gift-links.ts:20-29`).

**C++**
1. Permission gate as browse (`gift_links:manage` + `posts:edit`).
2. Step parses `:id` (400 if empty), derives `Actor` from the request (`actor_from_request`), calls `gift_links_ops::ensure(store,id,"posts",actor)`.
3. `ensure`: 404 Ghost NotFoundError if post missing; returns existing live link unchanged (no action, idempotent). Otherwise opens a `core::outbox` transaction and `mint`s: `generate_token()` produces a real 32-character base64url token (24 random bytes), `now_iso()` produces a real UTC ISO-8601 timestamp, `gift_links` history insert and `post_gift_links` upsert via `ResourceStore::edit` when an association already exists (`find_association_id`). `record_action` writes an `actions` record and emits a `gift_link.action` outbox event with `event:'added'` and `context.action_name:'add'`, then the transaction commits.
4. Output via `core::query` projection + `blog::mapping::wrap`; 200; no cache header.

**Diff**: No residual gaps. Ghost's Knex `forUpdate` row lock and rollback-on-action-error have no in-memory equivalent; the `core::outbox` transaction plus upsert reproduces the observable single-live-link + action-history semantics.

### POST /posts/:id/gift_links — MATCH

**Ghost** (`create`)
1. Router POST `.../gift_links`, `mw.authAdminApi` (`routes.js:74`); `assertCanEditAndGift` (`:66-68`).
2. `options:['id']`, `id.required` (`:64-65`); `statusCode:200` (`:63`), `cacheInvalidate:false` (`:62`).
3. `create(context,id)`→`ensureTransaction`→`lockedPost`→always `mint` (fresh token, `onConflict('post_id').merge`)→`recordAction` verb `reset`→`edited` with `context.action_name:'reset'` (`service.ts:80-87,118-134`; `actions.ts:23-27,74`).

**C++**
1. Permission gate as browse. Step parses `:id`, derives actor, calls `gift_links_ops::reset(store,id,"posts",actor)`.
2. `reset`: 404 if missing; inside an outbox transaction always `mint`s (new token/date, upsert replaces the live association — no separate erase step), records an `actions` entry and a `gift_link.action` outbox event `edited` with `action_name:'reset'`.
3. Output/200/no-cache-header identical to ensure.

**Diff**: No residual gaps. C++ uses a single upsert rather than Ghost's erase-and-reinsert internal sequence; the observable result (one fresh live link, preserved history, one `reset` action) is identical.

### GET /pages/:id/gift_links — MATCH

**Ghost** (`browse`): `router.get('/pages/:id/gift_links', mw.authAdminApi, http(api.giftLinks.browse))` (`routes.js:75`). No post/page branch in the controller — the same `posts` table backs both (`service.ts:31`). Same input, permission, 200/`cacheInvalidate:false`, `getPost` chain and serializer as posts browse.

**C++**: The three pages gift-links handlers are now bound in `admin/pages/routes.cppm` under `/:id` → `Route{"/gift_links"}` with GET/PUT/POST. GET invokes `pages_gift_links_get()`; its step calls `gift_links_ops::browse(store,id,"pages")`. Permission gate uses `gift_links:manage` + `pages:edit`; output and error shapes are identical to posts browse (shared ops module).

**Diff**: No residual gaps. Previous MISSING verdict came from unmounted handlers; the route is now installed and uses the same shared ops as posts.

### PUT /pages/:id/gift_links — MATCH

**Ghost** (`ensure`): `router.put('/pages/:id/gift_links', ...)` (`routes.js:76`); identical `ensure` semantics, actor history verb `add`, 200, `cacheInvalidate:false`.

**C++**: Bound PUT `/gift_links` under `/pages/:id`; handler gate + step calling `gift_links_ops::ensure(store,id,"pages",actor)`; shared real token/date, upsert, outbox action/transaction; the old ad-hoc token check that returned `{"error":"Unauthorized"}` was removed in favor of the Ghost-shaped permission gate.

**Diff**: No residual gaps (same non-observable JOIN/lock note as posts ensure).

### POST /pages/:id/gift_links — MATCH

**Ghost** (`create`): `router.post('/pages/:id/gift_links', ...)` (`routes.js:77`); identical `create` semantics, verb `reset`, 200, `cacheInvalidate:false`.

**C++**: Bound POST `/gift_links` under `/pages/:id`; handler gate + step calling `gift_links_ops::reset(store,id,"pages",actor)`; fresh token/upsert + `edited`/`reset` action.

**Diff**: No residual gaps.

### PUT /gift_links/remove_all — MATCH

**Ghost** (`removeAll`, custom action)
1. `router.put('/gift_links/remove_all', mw.authAdminApi, http(api.giftLinks.removeAll))` (`routes.js:78`).
2. No `options[]`, `validation{}`, `data[]` or `id`; body unused (`gift-links.ts:74-84`).
3. `permissions()`→`canThis(context).removeAll.gift_link()` (`:77-79`); `statusCode:200` (`:76`), `cacheInvalidate:false` (`:75`).
4. `query`→`count = service.removeAll(requestContextFromFrame(frame))`→`ensureTransaction` `trx('post_gift_links').del()`, record action `remove`→`deleted` with `resource_id:null` only when `removed>0` (`service.ts:94-102`; `actions.ts:47-82`). `gift_links` history preserved.
5. `removeAll` serializer→`toRevokeAllResponse.parse({count})`→`{meta:{count}}` (`output/gift-links.ts:18-20`; `serializers.ts:19-22`); empty DB→`{meta:{count:0}}`.

**C++**
1. `GiftLinksRoutes::install` mounts `Router{router,"/gift_links"}` with `content_type_middleware` + `require_auth` → `Route{"/remove_all"}.put()` → `gift_links_gift_links_remove_all_put()`; catalog route `PUT /gift_links/remove_all`.
2. Handler gate: `gift_links_ops::permitted(req,res,send,"gift_links","removeAll")` via the ghost authorizer. The previous redundant handler token check returning a non-Ghost `{"error":"Unauthorized"}` was removed, so denial now uses the shared Ghost 401/403 envelope.
3. Step calls `gift_links_ops::remove_all(store,actor)`; deletes live `post_gift_links` records, preserves `gift_links` history, returns `encode_count(removed)` = `{meta:{count:N}}`.
4. Inside an outbox transaction; when `removed>0`, `record_action(store,"deleted",{},actor,"remove")` writes an `actions` record (null resource id) and emits a `gift_link.action` outbox event. No cache header.

**Diff**: No residual gaps. The previous double auth gate and non-Ghost 401 body are gone; count now drives a real `deleted`/`remove` action and outbox event. Non-observable detail: C++ destroys in-memory records instead of a Knex `del()` rowcount.

## Module-level notes

- **Shared ops module**: `include/blog/handlers/membership/gift_links/operations.cppm` (`blog_handlers:gift_links_ops`) implements `browse`, `ensure`, `reset`, `remove_all` plus `generate_token`, `now_iso`, `find_link`, `find_association_id`, `mint`, `record_action`, `encode_links`, `encode_count`, `not_found_body`, `validation_error_body`, `prepare_principal`, `permitted`, `actor_from_request`, `parse_resource_id`. It is imported by all 7 steps and all 7 handlers.
- **Shared abstractions used** (per CONTRACTS.md): `core::auth::ghost::GhostAuthorizer` + `blog::handlers::middleware::require_ghost_permission` (permission decisions and Ghost-shaped 401/403 envelopes); `core::outbox::global_outbox()` with `begin_transaction`/`commit` and `Event` (action-history side effects); `core::query::apply` with a `fields` projection (Ghost wire field set) and `blog::mapping::wrap` (response envelope); `blog::api::ApiDataStore` for all state changes.
- **Ghost action history**: `record_action` writes an `actions` store record with `resource_type:'gift_link'`, `event` `added`/`edited`/`deleted`, `actor_type`/`actor_id`, `context.action_name` for `add`/`reset`/`remove`, and emits one `gift_link.action` outbox event, all inside the ensure transaction. `browse` writes no action, and `remove_all` writes one only when `removed>0`.
- **Output**: `encode_links` projects `token,created_at` via `core::query` then wraps as `gift_links`; `encode_count` emits `{meta:{count:N}}`. No DTO changes were needed; `types/dto_links.cppm` remains byte-identical.
- **Cache**: Ghost `cacheInvalidate:false` (gift-links.ts:31) means no `X-Cache-Invalidate` response header; C++ emits none, so behavior agrees without a cache marker.
- **Permissions mapping**: Ghost `manage.gift_link(id)` + `edit.post(id)` map to `gift_links:manage` + `posts|pages:edit` (removeAll → `gift_links:removeAll`) through the shared ghost authorizer. The shared `require_ghost_permission(docName, method)` contract has no object-id parameter, so object-id scoping is not expressible; authorization is otherwise enforced by the same role/permission model as the rest of the API.
- **No EXTRA**: every C++ gift-links route maps 1:1 to a Ghost route (`routes.js:72-78`).
- **Central registrations required** (applied by the central owner; not in this group's edits): the new `import :mapping;` from `blog_handlers:gift_links_ops`, the `blog_handlers:mapping` / `core_query` / `core_outbox` / `core_auth` / `blog_handlers_middleware` dependencies are already present in `include/blog/handlers/BUILD.bazel`; no new partition, handler, step, flow or route registration is needed because all 7 handlers/steps/flows and the ops partition were already listed. The only shared wiring was the 3 `Route{"/gift_links"}` bindings added in `admin/pages/routes.cppm` (in-scope), with no change to central `all.cppm`, `base.cppm` or root `routes.cppm`.
