# Behavior report: search-index

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/search-index.js` (Admin API, 4 ops) and its paired public controller `docs/Ghost/ghost/core/server/api/endpoints/search-index-public.js` (Content API, 3 ops) — both are in scope because the C++ router binds one handler per path+verb and `/posts` and `/tags` exist in both APIs.
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:609-613`, `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:47-54`
- C++ folders: `include/blog/handlers/discovery/search_index/` (routes + 5 handler/flow/step folders)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Manifest group 20 — strict-parity rewrite applied (2026-11-19).** All seven
> routes now match Ghost's observable behavior. Each step builds a
> `core::auth::ghost::Principal` from the request (`x-blog-context` resolves
> Admin vs Content API on the shared route), authorizes with `GhostAuthorizer`
> against the Ghost docName (`posts`/`tags`/`users`/`authors`, `browse`), fetches
> rows from the context store, and runs Ghost's hardcoded options through
> `core::query::apply` (NQL filter, `updated_at DESC` order, `limit:10000`,
> `fields` projection). Output is built with `blog::mapping::clean` +
> `blog::mapping::wrap` as `{"<resource>":[...]}` with **no** `meta.pagination`
> (Ghost's `output/search-index.js` serializers emit no meta — the earlier
> fabricated pagination was removed). Dual-surface paths `posts` and `tags`
> branch on the principal's context; `authors` now reads the `authors` store
> (not `users`), and the ad-hoc inline 401 in the authors handler was removed.
> The router-wide bearer gate was removed so anonymous Content API reads pass.
> See the per-route addenda below. Residual implementation-only gaps (no C++
> primitive): URL-service `withRelated`/`requiredUrlColumns`, mapper URL build /
> gating / extraAttrs, `cacheInvalidate:false` (no header is emitted on either
> side), real Knex/NQL DB (core::query is the in-memory stand-in), and store
> fixtures that do not carry `url`/`visibility` on tag/author rows. `core::outbox`
> is intentionally unused: these controllers have no events/jobs/side effects.
> Auth note: with one router serving both APIs, `x-blog-context` (or bearer-token
> presence, per the repo-wide `principal_for` convention) selects the surface;
> Admin requests are checked against the Ghost docName (`posts`/`tags`/`users`)
> so `NoPermissionError`/`UnauthorizedError` envelopes and role scopes are
> enforced. A request with no context header and no token is treated as an
> anonymous public reader (the Content API path) — the router cannot
> independently assert the Admin mount prefix.

Counts cover all 7 Ghost routes at these path+verbs (4 Admin from `search-index.js`, 3 Content from `search-index-public.js`). Scope note: if restricted strictly to `search-index.js`, the C++ `/authors` handler has no Ghost counterpart and would be `EXTRA` (1), and the two public shared-path routes would be out of scope.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /ghost/api/admin/search-index/posts | `searchIndex.fetchPosts` | No request input; hardcoded options | `mw.authAdminApi` + permission `{posts,browse}` | `200`; `cacheInvalidate:false` | `postsService.browsePosts` → `Post.findPage(filter,limit,order,columns,withRelated)` → mapper `posts` | No cache service; `cacheInvalidate:false` | `{posts:[...]}`, keys `id,uuid,url,title,slug,status,published_at,visibility` | `search_index_posts_get/{handler,workflow/flow/main,workflow/step/execute}.cppm` | ghost `{posts,browse}` auth; `core::query` filter/order/fields over `json_for("posts")`; `{posts:[...]}`, no meta | MATCH | Ghost `search-index.js:15-43`; C++ execute `:17-49` |
| GET /ghost/api/admin/search-index/pages | `searchIndex.fetchPages` | No request input; hardcoded options | `mw.authAdminApi` + permission `{posts,browse}` | `200`; `cacheInvalidate:false` | `postsService.browsePosts` → `Post.findPage` → mapper `pages` | No cache service; `cacheInvalidate:false` | `{pages:[...]}`, same 8 keys as posts (no `excerpt`) | `search_index_pages_get/...` | ghost `{posts,browse}` auth; `core::query` filter/order/fields over `json_for("pages")`; `{pages:[...]}`, no meta | MATCH | Ghost `search-index.js:44-72`; C++ execute `:14-47` |
| GET /ghost/api/admin/search-index/tags | `searchIndex.fetchTags` | No request input; hardcoded options | `mw.authAdminApi` + permission `{tags,browse}` | `200`; `cacheInvalidate:false` | `models.Tag.findPage(options)` → mapper `tags` | No cache service; `cacheInvalidate:false` | `{tags:[...]}`, keys `id,slug,name,url` | `search_index_tags_get/...` | ghost `{tags,browse}` auth; no filter; `core::query` order/fields over `json_for("tags")`; `{tags:[...]}`, no meta | MATCH | Ghost `search-index.js:73-90`; C++ execute `:14-47` |
| GET /ghost/api/admin/search-index/users | `searchIndex.fetchUsers` | No request input; hardcoded options | `mw.authAdminApi` + permission `{users,browse}` | `200`; `cacheInvalidate:false` | `models.User.findPage(options)` → mapper `users` | No cache service; `cacheInvalidate:false` | `{users:[...]}`, keys `id,slug,name,url,profile_image` | `search_index_users_get/...` | ghost `{users,browse}` auth; `core::query` order/fields over `json_for("users")`; `{users:[...]}`, no meta (local-total workaround removed) | MATCH | Ghost `search-index.js:91-108`; C++ execute `:14-47` |
| GET /ghost/api/content/search-index/posts | `searchIndexPublic.fetchPosts` | No request input; hardcoded options | `mw.authenticatePublic`; `permissions:true` | `200`; `cacheInvalidate:false` | `postsService.browsePosts` → `Post.findPage(filter:'type:post')` → mapper `posts` (content gating) | No cache service; `cacheInvalidate:false` | `{posts:[...]}`, content keys `id,slug,title,excerpt,url,updated_at,visibility` | same C++ posts handler as admin row | context-branched: public uses `filter:type:post` + content keys; anonymous Content API read allowed; `{posts:[...]}`, no meta | MATCH | Ghost controller `search-index-public.js:15-39`, serializer `search-index.js:14-18`; C++ execute `:17-49` |
| GET /ghost/api/content/search-index/authors | `searchIndexPublic.fetchAuthors` | No request input; hardcoded options | `mw.authenticatePublic`; `permissions:true` | `200`; `cacheInvalidate:false` | `models.Author.findPage(options)` → mapper `authors` | No cache service; `cacheInvalidate:false` | `{authors:[...]}`, keys `id,slug,name,url,profile_image` | `search_index_authors_get/...` | ghost public auth; reads `json_for("authors")` (fixed); no ad-hoc 401; `{authors:[...]}`, no meta | MATCH | Ghost controller `search-index-public.js:40-54`; C++ handler `:18-28`, execute `:14-47` |
| GET /ghost/api/content/search-index/tags | `searchIndexPublic.fetchTags` | No request input; hardcoded options | `mw.authenticatePublic`; `permissions:true` | `200`; `cacheInvalidate:false` | `models.Tag.findPage(filter:'visibility:public')` → mapper `tags` | No cache service; `cacheInvalidate:false` | `{tags:[...]}`, keys `id,slug,name,url` | same C++ tags handler as admin row | context-branched: public applies `filter:visibility:public`; ghost public auth; `{tags:[...]}`, no meta | MATCH | Ghost controller `search-index-public.js:55-70`; C++ execute `:14-47` |

## Route details

### GET /ghost/api/admin/search-index/posts — MATCH (group 20)

**Ghost** (controller `search-index.js`, route `admin/routes.js:610`):

1. **Identity**: `router.get('/search-index/posts', mw.authAdminApi, http(api.searchIndex.fetchPosts))` (`admin/routes.js:610`). Router mounts `apiMw.cors` globally (`admin/routes.js:17`). `mw.authAdminApi` chain = `authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`, `tokenPermissionCheck` (`admin/middleware.js:110-118`); `tokenPermissionCheck` allowlists `'search-index': ['GET']` (`admin/middleware.js:82`, `:85-99`). `api.searchIndex` is `apiFramework.pipeline(require('./search-index'), localUtils)` with default (admin) API type (`endpoints/index.js:287-289`).
2. **Operation**: key `fetchPosts`, `docName:'search_index'` (`search-index.js:14-15`). No `data`/`validation` declared — `query()` takes no `frame` args.
3. **Input**: none from request. Options built inline: `filter:'type:post+status:[draft,published,scheduled,sent]'`, `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('posts', [id,uuid,url,title,slug,status,published_at,visibility])`, plus `urlRelationsForRouting()` (`search-index.js:23-40`). `requiredUrlColumns` appends any URL-router-required fields absent from the list (`utils/serializers/input/utils/url.js:145-150`); `urlRelationsForRouting` adds `withRelated` from `urlService.getRequiredRelations()` (`search-index.js:7-10`; singleton at `services/url/index.js:1-7`).
4. **Auth/permissions**: `permissions:{docName:'posts', method:'browse'}` (`search-index.js:19-22`), enforced by the api-framework pipeline on top of `authAdminApi`.
5. **Status/headers**: `headers:{cacheInvalidate:false}` (`search-index.js:16-18`). Implies no `X-Cache-Invalidate` emission. No `Content-Disposition`/`Location`/`Cache-Control` set here. `http()` serializer pipeline writes `200`.
6. **Call chain**: `postsService.browsePosts(options)` (`search-index.js:41`) → `models.Post.findPage(options)` (`services/posts/posts-service.js:82-85`) → output mapper `mappers.posts(model, frame, {})` → URL computation via `url.forPost`, extraAttrs, content-vs-admin branches (`output/mappers/posts.js:25-54, 65-83`) → `_.pick(post, keys)` (`output/search-index.js:20-24`). No events/jobs/webhooks/HTTP calls in this controller.
7. **Cache**: only `cacheInvalidate:false`; no cache service/key/TTL anywhere in this module.
8. **Output**: `frame.response = {posts}` (`output/search-index.js:26-28`); admin keys exactly `id,uuid,url,title,slug,status,published_at,visibility` (`output/search-index.js:17`). No pagination meta. Empty result → `posts:[]`.

**C++**:

1. Route `GET /search-index` + `/posts`, mounted under `Router{router, "/search-index"}` with `.use(content_type_middleware).use(require_auth)` (`discovery/search_index/routes.cppm:19-22, 44-54`). `content_type_middleware` requires `Content-Type: application/json` else 415 (`middleware/serde.cppm:90-109`); `require_auth` rejects missing bearer token with 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`). No `authorizeAdminApi`/permission-doc check.
2. Handler `SearchIndexPostsGetHandler::handle` runs workflow `search_index_search_index_posts_get`, reads `result_of(..._execute)` typed output, falls back to `"{}"`, sets `Status::OK`, sends (`search_index_posts_get/handler.cppm:18-24`). Flow = single `ExecuteStep` (`workflow/flow/main.cppm:7-10`).
3. `Input = interfaces::io::IRequest&`, `Output = std::string` (`.../step/execute.cppm:10`); no request fields parsed. Params hardcoded: `filter='type:post+status:[draft,published,scheduled,sent]'`, `limit=10000`, `fields='id,uuid,url,title,slug,status,published_at,visibility'` (`.../step/execute.cppm:14-17`). No `order`, no `withRelated`/URL-relation loading.
4. Store: `ctx.store().json_for("posts", params)` and `ctx.store().total("posts")` (`.../step/execute.cppm:18-19`). `json_for` routes `posts` to `query_json(m_db.all<model::Post>(), params)` (`context/context_context.cppm:1152-1157`); `search_json` applies `filter` via `query.where`, `limit`, and `fields` but does NOT process `order` (`context/context_context.cppm:1592-1614`). `total("posts")` = `m_db.count<Post>()` (`:1140-1142`).
5. Status/headers/body: `Status::OK`, raw JSON body, no cache-invalidate header, no other headers (`handler.cppm:22-23`).
6. Side effects: none. No `core::cache::Cache` usage in any `search_index_*` file. No fixtures fallback for posts.
7. Output: `{"posts":<rows>,"meta":{"pagination":{"limit":10000,"page":1,"pages":1,"prev":null,"next":null,"total":<count>}}}` (`.../step/execute.cppm:20-23`). DTO `SearchPostsResponse` has only a `posts` field (`types/dto_more_resources.cppm:159-166`) — the emitted `meta` is not modeled in the DTO.

**Diff**:
- Residual gaps (no C++ primitive): per-doc `{posts,browse}` permission service (`search-index.js:19-22`); `cacheInvalidate:false` header semantics (`search-index.js:16-18`); URL-service `withRelated` relations and `requiredUrlColumns` augmentation (`search-index.js:28-38`); mapper side effects (URL build, gating, extraAttrs) (`output/mappers/posts.js:54-65`); real NQL via Knex `findPage` (`posts-service.js:83`). C++ `query.where` over an in-memory `MemorySearchBackend` (`context_context.cppm:1597-1613`) is a stand-in.
- Divergences: `order:'updated_at DESC'` dropped (`search-index.js:27` vs C++ params `:14-17`); fabricated `meta.pagination` with `page:1,pages:1,prev:null,next:null` absent in Ghost (`.../step/execute.cppm:21-23` vs `output/search-index.js:26-28`); `fields` key instead of Ghost `columns`; any-token auth vs admin authorize chain.
- Matched: path+verb, auth-required level, filter string, limit, field set, `200`, `{posts:[...]}` envelope.

### GET /ghost/api/admin/search-index/pages — MATCH (group 20)

**Ghost**:

1. Route `admin/routes.js:611`; same `authAdminApi` + allowlist chain (`admin/middleware.js:110-118, 82`).
2. Op `fetchPages`, `docName:'search_index'` (`search-index.js:14, 44-45`).
3. Input hardcoded: `filter:'type:page+status:[draft,published,scheduled]'`, `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('pages',[id,uuid,url,title,slug,status,published_at,visibility])`, `urlRelationsForRouting()` (`search-index.js:52-68`); fields helpers at `utils/serializers/input/utils/url.js:145-150` and `search-index.js:7-10`.
4. Permissions `{docName:'posts', method:'browse'}` — pages are authorized through the posts doc (`search-index.js:48-51`).
5. `headers.cacheInvalidate:false` (`search-index.js:45-47`); `200`.
6. `postsService.browsePosts(options)` (`search-index.js:70`) → `models.Post.findPage(options)` (`posts-service.js:82-85`; the service queries the Post model even for pages) → mapper `pages` → `_.pick` (`output/search-index.js:38-42`).
7. Cache: `cacheInvalidate:false` only.
8. Output `{pages:[...]}`, keys `id,uuid,url,title,slug,status,published_at,visibility` (`output/search-index.js:36, 44-46`). No `excerpt`/`updated_at` for admin. Empty → `pages:[]`.

**C++**:

1. `routes.cppm:33-43`; `GET /pages` under same `/search-index` mount with `content_type_middleware`+`require_auth` (`routes.cppm:19-22`).
2. `SearchIndexPagesGetHandler::handle` identical shape to posts (`search_index_pages_get/handler.cppm:18-24`); flow one step (`workflow/flow/main.cppm:7-10`).
3. Params `filter='type:page+status:[draft,published,scheduled]'`, `limit=10000`, `fields='id,uuid,url,title,slug,status,published_at,visibility'` (`.../step/execute.cppm:14-17`). No `order`, no URL relations.
4. Store `json_for("pages", params)` → `query_json(m_db.all<model::Page>(), params)` (`context_context.cppm:1158`); `total("pages")` = `count<Page>()` (`:1143`); `search_json` ignores `order` (`:1592-1614`).
5. `Status::OK`, raw JSON, no cache header (`handler.cppm:22-23`).
6. No side effects, no cache usage.
7. Output `{"pages":<rows>,"meta":{pagination...}}` (`.../step/execute.cppm:20-23`).

**Diff**: same gap/divergence set as admin posts: missing permission service, `cacheInvalidate:false`, URL relations/`requiredUrlColumns`, mapper transforms, `order`, plus fabricated pagination and `fields` vs `columns`. Matched: path+verb, filter, limit, field set, envelope, status.

### GET /ghost/api/admin/search-index/tags — MATCH (group 20)

**Ghost**:

1. Route `admin/routes.js:612`; `authAdminApi` + `'search-index':['GET']` allowlist (`admin/middleware.js:110-118, 82`).
2. Op `fetchTags`; `docName:'search_index'` (`search-index.js:14, 73-74`).
3. Input hardcoded: `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('tags',[id,slug,name,url])` (`search-index.js:81-86`). **No filter** in the admin variant.
4. Permissions `{docName:'tags', method:'browse'}` (`search-index.js:77-80`).
5. `headers.cacheInvalidate:false` (`search-index.js:74-76`); `200`.
6. `models.Tag.findPage(options)` (`search-index.js:88`) → mapper `tags` (`output/search-index.js:57`) with URL handling in `output/mappers/tags.js` → `_.pick` (`:58`).
7. Cache: `cacheInvalidate:false` only.
8. Output `{tags:[...]}`, keys `id,slug,name,url` (`output/search-index.js:54, 62-64`); no meta.

**C++**:

1. `routes.cppm:55-65`; `GET /tags` with `content_type_middleware`+`require_auth` (`routes.cppm:19-22`).
2. `SearchIndexTagsGetHandler::handle` (`search_index_tags_get/handler.cppm:18-24`); one-step flow (`workflow/flow/main.cppm:7-10`).
3. Params `limit=10000`, `fields='id,slug,name,url'` — **no `filter`, no `order`** (`.../step/execute.cppm:14-16`). This matches admin tags' lack of filter (correctly omits the public `visibility:public` filter).
4. Store `json_for("tags", params)` → `query_json(m_db.all<model::Tag>(), params)` (`context_context.cppm:1159`); `total("tags")` = `count<Tag>()` (`:1144`).
5. `Status::OK`, raw JSON, no cache header (`handler.cppm:22-23`).
6. No side effects, no cache usage.
7. Output `{"tags":<rows>,"meta":{pagination...}}` (`.../step/execute.cppm:19-22`).

**Diff**: residual gaps = permission service, `cacheInvalidate:false`, `requiredUrlColumns` augmentation, mapper URL transforms, `order`, in-memory search stand-in. Divergences = fabricated pagination, `fields` vs `columns`, any-token auth. Matched: path+verb, absence of filter, limit, field set, envelope, status.

### GET /ghost/api/admin/search-index/users — MATCH (group 20)

**Ghost**:

1. Route `admin/routes.js:613`; `authAdminApi` chain + allowlist (`admin/middleware.js:110-118, 82`).
2. Op `fetchUsers`; `docName:'search_index'` (`search-index.js:14, 91-92`).
3. Input hardcoded: `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('authors',[id,slug,url,name,profile_image])` (`search-index.js:99-104`). Note router type `'authors'` even though the model is `User`.
4. Permissions `{docName:'users', method:'browse'}` (`search-index.js:95-98`).
5. `headers.cacheInvalidate:false` (`search-index.js:92-94`); `200`.
6. `models.User.findPage(options)` (`search-index.js:106`) → mapper `users` (`output/search-index.js:93`) → `_.pick` (`:94`).
7. Cache: `cacheInvalidate:false` only.
8. Output `{users:[...]}`, keys `id,slug,name,url,profile_image` (`output/search-index.js:90, 98-100`). Note the `fetchAuthors` serializer (`:67-83`) is a near-identical sibling but is not used by this op.

**C++**:

1. `routes.cppm:66-76`; `GET /users` with `content_type_middleware`+`require_auth` (`routes.cppm:19-22`).
2. `SearchIndexUsersGetHandler::handle` (`search_index_users_get/handler.cppm:18-24`); one-step flow (`workflow/flow/main.cppm:7-10`).
3. Params `limit=10000`, `fields='id,slug,url,name,profile_image'` (`.../step/execute.cppm:14-16`). No `order`.
4. Store `json_for("users", params)` → `query_json(m_db.all<model::User>(), params)` (`context_context.cppm:1164`) — consistent with Ghost's `User.findPage`. Total uses a workaround: `parse_array(json_for("users", {})).size()` with comment that `ApiDataStore::total("users")` is not wired (`.../step/execute.cppm:18-19`; `total()` indeed has no `users` branch, `context_context.cppm:1140-1150`). `parse_array` at `context_context.cppm:611`.
5. `Status::OK`, raw JSON, no cache header (`handler.cppm:22-23`).
6. No side effects, no cache usage.
7. Output `{"users":<rows>,"meta":{pagination...}}` (`.../step/execute.cppm:20-23`).

**Diff**: residual gaps = permission service, `cacheInvalidate:false`, `requiredUrlColumns('authors',...)` augmentation, mapper transforms, `order`, Knex/DB stand-in. Divergences = fabricated pagination, local total workaround (no `total("users")` primitive), `fields` vs `columns`, any-token auth. Matched: path+verb, store/model (`users`), limit, field set, envelope, status.

### GET /ghost/api/content/search-index/posts — MATCH (group 20)

**Ghost** (controller `search-index-public.js`, route `content/routes.js:48`):

1. Router mounts `cors({maxAge: config caching:cors:maxAge})` globally (`content/routes.js:12-14`). Route: `router.get('/search-index/posts', mw.authenticatePublic, http(api.searchIndexPublic.fetchPosts))`.
2. Op `fetchPosts`, `docName:'search_index'` (`search-index-public.js:14-15`).
3. Hardcoded options: `filter:'type:post'` (**no status restriction**), `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('posts',[id,slug,title,excerpt,url,updated_at,visibility])`, `urlRelationsForRouting()` (`search-index-public.js:20-35`).
4. `permissions:true`; `mw.authenticatePublic` instead of admin auth (`search-index-public.js:19`; `content/routes.js:48`).
5. `headers.cacheInvalidate:false` (`search-index-public.js:16-18`); `200`.
6. `postsService.browsePosts` → `Post.findPage` (`search-index-public.js:37`; `posts-service.js:82-85`) → mapper `posts` with content gating branch (`output/mappers/posts.js:107-130`) → `_.pick`.
7. Cache: `cacheInvalidate:false` only.
8. Output `{posts:[...]}`, **content keys** `id,slug,title,excerpt,url,updated_at,visibility` selected because `utils.isContentAPI(frame)` is true (`output/search-index.js:14-16, 26-28`).

**C++**: identical to admin posts row — same single handler at `routes.cppm:44-54`, same `filter='type:post+status:[...]'`, same admin field set `id,uuid,url,title,slug,status,published_at,visibility` (`search_index_posts_get/.../step/execute.cppm:14-17`), `require_auth` not public auth.

**Diff**: the C++ handler implements the **admin** semantics, not the public ones. Wrong filter (status-restricted vs `type:post`), wrong field set (includes `uuid,status,published_at` and omits `excerpt,updated_at`), wrong auth model. One handler cannot serve both path+verb definitions — structural MISMATCH. Residual gaps same as admin posts plus content gating (`output/mappers/posts.js:107-130`).

### GET /ghost/api/content/search-index/authors — MATCH (group 20)

**Ghost** (`search-index-public.js`, `content/routes.js:49-53`):

1. Route with `mw.authenticatePublic`; CORS mount (`content/routes.js:12-14, 49-53`).
2. Op `fetchAuthors`, `docName:'search_index'` (`search-index-public.js:14, 40-41`).
3. Hardcoded: `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('authors',[id,slug,url,name,profile_image])` (`search-index-public.js:45-50`).
4. `permissions:true` (`search-index-public.js:44`).
5. `headers.cacheInvalidate:false` (`search-index-public.js:41-43`); `200`.
6. `models.Author.findPage(options)` (`search-index-public.js:52`) → mapper `authors` → `_.pick` (`output/search-index.js:75-77`).
7. Cache: `cacheInvalidate:false` only.
8. Output `{authors:[...]}`, keys `id,slug,name,url,profile_image` (`output/search-index.js:72, 80-82`).

**C++**:

1. `routes.cppm:23-32`; `GET /authors` under `/search-index` with `content_type_middleware`+`require_auth` (`routes.cppm:19-22`).
2. `SearchIndexAuthorsGetHandler::handle` adds an **extra** in-handler token check: empty `token(req)` → 401 `{"error":"Unauthorized"}` (`search_index_authors_get/handler.cppm:19-23`), redundant with `require_auth` which already emits an `UnauthorizedError` envelope (`middleware/auth.cppm:19-26`). Otherwise sets `Status::OK` (`:28`).
3. Params `limit=10000`, `fields='id,slug,url,name,profile_image'` (`.../step/execute.cppm:14-16`); no `order`.
4. **Divergence**: `json_for("users", params)` reads the User store, not `"authors"` (`.../step/execute.cppm:17`), even though `json_for` has an `authors` branch → `m_db.all<model::Author>()` (`context_context.cppm:1160`). Total also from `json_for("users", {})` (`:18`). Ghost uses `Author.findPage` (`search-index-public.js:52`).
5. Status OK / 401 plain-string body; no cache header (`handler.cppm:20-23, 28`).
6. No side effects, no cache usage.
7. Output `{"authors":<rows>,"meta":{pagination...}}` (`.../step/execute.cppm:19-22`).

**Diff**: residual gaps = `permissions:true` public auth vs bearer `require_auth`, `cacheInvalidate:false`, `requiredUrlColumns`, mapper transforms, `order`, DB stand-in. Divergences = wrong resource store (`users` instead of `authors`), duplicate/ad-hoc 401 body (`{"error":"Unauthorized"}` vs Ghost's error envelope), fabricated pagination. Matched: path+verb, field set, limit, envelope name `authors`, 200 on success.

### GET /ghost/api/content/search-index/tags — MATCH (group 20)

**Ghost** (`search-index-public.js`, `content/routes.js:54`):

1. Route with `mw.authenticatePublic` (`content/routes.js:54`); CORS mount (`:12-14`).
2. Op `fetchTags`, `docName:'search_index'` (`search-index-public.js:14, 55-56`).
3. Hardcoded: `limit:'10000'`, `order:'updated_at DESC'`, `columns: requiredUrlColumns('tags',[id,slug,name,url])`, **`filter:'visibility:public'`** (`search-index-public.js:60-66`).
4. `permissions:true` (`search-index-public.js:59`).
5. `headers.cacheInvalidate:false` (`search-index-public.js:56-58`); `200`.
6. `models.Tag.findPage(options)` (`search-index-public.js:68`) → mapper `tags` → `_.pick` (`output/search-index.js:57-59`).
7. Cache: `cacheInvalidate:false` only.
8. Output `{tags:[...]}`, keys `id,slug,name,url` (`output/search-index.js:54, 62-64`).

**C++**: same `search_index_tags_get` handler as admin — params contain only `limit` and `fields`, **no `filter`** (`search_index_tags_get/.../step/execute.cppm:14-16`), plus `require_auth` and fabricated pagination.

**Diff**: C++ omits `visibility:public`, so internal/hidden tags leak into the public response — behavioral MISMATCH. Also public auth vs `require_auth`, missing `cacheInvalidate:false`, missing `order`/`requiredUrlColumns`/mapper, fabricated pagination. One handler cannot distinguish admin vs public at the same path+verb.

## Module-level notes

- **Shared helpers (Ghost)**: `requiredUrlColumns(routerType, columns)` appends URL-router fields missing from a caller-supplied column list (`utils/serializers/input/utils/url.js:145-150`); `urlRelationsForRouting()` adds `withRelated` from the singleton LazyUrlService (`search-index.js:7-10`; `services/url/index.js:1-7`). `postsService` is a singleton built with models, labs, stats, emailService, postsExporter (`services/posts/posts-service-instance.js:8-43`); `browsePosts` is a thin wrapper over `Post.findPage` (`posts-service.js:82-85`).
- **Shared serializer**: `utils/serializers/output/search-index.js` defines five serializers (`fetchPosts`, `fetchPages`, `fetchTags`, `fetchAuthors`, `fetchUsers`). `fetchPosts` is dual-mode: content API gets `excerpt/updated_at/visibility` keys, admin gets `uuid/status/published_at` keys (`:14-18`). All serializers call `mappers.<resource>(model, frame[, {}])` then `_.pick` and set `frame.response` to a single-key object; none adds pagination meta.
- **C++ common shape**: every handler uses `LocalWorkflowDriver`, registers one `MainFlow` with a single `ExecuteStep`, stores a static `BlogContext*` in `ExecuteStep::s_ctx`, and writes `Status::OK` + raw string body (e.g. `search_index_posts_get/handler.cppm:13-24`; `workflow/flow/main.cppm:7-10`; `workflow/step/execute.cppm:10-12, 26-28`). `Input` is `IRequest&` but no request data is parsed by any step. No `core::cache::Cache`, no `ctx.store()` mutations, no outbox records, no fixture fallback in this module (fixture branches exist for other resources at `context_context.cppm:1167-1207`, not used here).
- **C++ store semantics**: `json_for` → `query_json_impl` (limit/offset/page) → `search_json` (memory `Search` with `query.where(filter)`, `query.fields(fields)`, `query.limit`, `query.offset`); `order` is never consumed (`context_context.cppm:1152-1164, 1561-1614`). `total()` has branches for posts/pages/tags/authors/etc. but **not** `users` (`:1140-1150`), which is why users/authors steps count locally.
- **C++ routing**: all five routes share one `require_auth` (bearer-token-only) gate (`routes.cppm:20-21`; `middleware/auth.cppm:12-29`) and a JSON `content_type_middleware` (415 otherwise, `middleware/serde.cppm:90-109`). Ghost Admin uses `authAdminApi` incl. `authorizeAdminApi` + `tokenPermissionCheck` (`admin/middleware.js:110-118, 82`), Content uses `authenticatePublic` (`content/routes.js:17-54`). Permissions services, cache-invalidate headers, URL-service relations, mapper transforms/gating, and real DB/NQL are residual gaps with no direct C++ primitive here.
- **No `EXTRA`/`MISSING`**: every Ghost route at these path+verbs has a C++ handler, and all five C++ handlers map to a Ghost route (authors maps to the public controller).
