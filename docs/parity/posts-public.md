# Parity: posts-public

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/posts-public.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (content API)
- C++: `include/blog/handlers/admin/posts/` (`posts_support.cppm`, `routes.cppm`, `browse/`,
  `read_by_id/`, `read_by_slug/`)
- Checked: 2026-11-19 (strict-parity pass)

> Premise check: the audit brief anticipated "no C++ counterpart / all MISSING". Disproven — the
> `content=true` branch of `PostsRoutes::install` defines the three public GET routes at the same
> paths/verbs, and the steps now switch to content behavior via `x-blog-context: CONTENT_API`.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **BLOCKING CENTRAL STEPS**: (1) register `admin/posts/posts_support.cppm` in
> `include/blog/handlers/BUILD.bazel`; (2) call `PostsRoutes{}.install(router, handlers, true)`
> from `include/blog/handlers/routes.cppm`. Until then the content branch is unmounted.

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict |
|---|---|---|---|---|---|---|---|---|---|
| GET `/posts` | `postsPublic.browse` | GET `/posts` (`content=true`) | `posts_browse()` | 11 options; include∈{tags,authors,tiers,sentiment}; content restricted transformer | `{posts,meta.pagination}` | `core::query` over `json_for("posts")` | `postsPublic` key pending centrally | ghost CONTENT_API pass-through | MATCH |
| GET `/posts/:id` | `postsPublic.read` | GET `/posts/:id` (`content=true`) | `posts_read_by_id()` | options + id/slug/uuid; 400 guard | `{posts:[post]}` | NQL lookup + gift access | same + gift dimension centrally | ghost CONTENT_API pass-through | MATCH |
| GET `/posts/slug/:slug` | `postsPublic.read` | GET `/posts/slug/:slug` (`content=true`) | `posts_read_by_slug()` | same options; slug path | `{posts:[post]}` | NQL slug lookup + gift | same keyed by `identifier.slug` centrally | ghost CONTENT_API pass-through | MATCH |

## Route details

All three public routes share `include/blog/handlers/admin/posts/posts_support.cppm` with the
admin routes; behavior is selected by `x-blog-context: CONTENT_API`:

1. **Input** — content allow-lists (`kContentBrowseOptions`, `kContentIncludes`,
   `kPublicReadOptions`) mirror `posts-public.js:12,47-59,98`; `skipPagination` removes the
   pagination meta. `core::query` supplies NQL filtering/sorting/projection and real pagination.
2. **Restricted fields** — `parse(..., content=true)` strips the Content API restricted set
   (`api-filter-utils.ts:9-26,149-153,206-208`) and rejects restricted order fields with a Ghost
   `BadRequestError`.
3. **Auth** — `core::auth::ghost::GhostAuthorizer::authorize` permits anonymous `browse` on public
   content in `CONTENT_API` context; `require_ghost_permission` emits Ghost 401/403 on denial.
   The `content=true` route branch uses `bearer_middleware` + `optional_auth`.
4. **Output** — `blog::mapping::clean(...,"post")` + `wrap`/`pagination` yield
   `{posts:[...],meta.pagination}` for browse and `{posts:[post]}` (no meta) for reads.
   `cacheInvalidate:false` is represented by emitting no `X-Cache-Invalidate` header.
5. **Gift access** — `support::gift_access` resolves `?gift=<token>` through `post_gift_links`;
   a matching token unlocks gated fields for the addressed post, a mismatched token does not.
   Error envelopes: 400 `A post id, slug or uuid is required.`, 404 `Post not found.`.

Residuals requiring central/shared work (not observable in the `{posts,meta}` wire contract):
the `postsPublicService.service.cache` response cache and its key helpers
(`generateOptionsData`/`generateAuthData`/`generateGiftKeyData`), the input serializer's forced
`type:post`/order/gating columns, and serializer-level tiers/member-attribution enrichment.
`include=sentiment` is accepted and validated but has no embedder in the store — known gap.

## Missing in C++

None. All three Ghost content routes have path+verb counterparts in the `content=true` branch of
`admin/posts/routes.cppm`.

## Extra in C++

None relative to this controller. The admin write routes in the same folder belong to Ghost's
`posts.js` controller and are tracked in `docs/parity/posts.md`.

## Needs central registration

Local work done (files only in `include/blog/handlers/admin/posts/` and the four parity docs):

- The three GET steps/handlers now share the admin implementation: content option/value sets,
  restricted-field transformer, `core::query` filtering/pagination, Ghost error envelopes,
  `blog::mapping` output, `gift_access`, and no literal `X-Cache-Invalidate` header.
- `PostsRoutes::install(router, handler, bool content = false)` already defines the content-only
  route set (`/posts`, `/posts/:id`, `/posts/slug/:slug`) with `bearer_middleware`+`optional_auth`.

Central items reserved for the main agent:

1. **`include/blog/handlers/routes.cppm:108`** — call
   `PostsRoutes{}.install(router, handlers, true)` from `ApiRoutes::install` in addition to the
   existing two-argument admin install. Without it the content branch exists but is not mounted.
2. **`include/blog/handlers/BUILD.bazel`** — add `"admin/posts/posts_support.cppm"` to the
   `blog_handlers` partitions list after `mapping/{base,clean,wrap}.cppm`. No deps change needed.
3. **`include/blog/handlers/context/context_context.cppm`** — optional central follow-ups: add
   `core::cache::Cache& cache()` to `ApiDataStore`/`BlogContext` for the `postsPublic` response
   cache keys (`"posts:content:browse:"+canonical(params)`, `"posts:content:read:"+identifier`,
   `"posts:content:read:slug:"+slug`, tag `posts`), and `find_post_by_slug`/`uuid` helpers for the
   exact `models.Post.findOne({slug})` shape. Current NQL slug/uuid lookup already closes the
   observable read behavior locally.
4. **`include/blog/handlers/base.cppm`** — no change needed; `posts_support` is a local import.

Non-observable differences retained: Ghost's framework middleware chain (rate limiting, CORS,
SSL/host redirect, pretty URLs), response cache, forced input serialization and serializer-level
tiers/member attribution have no C++ primitive in the allowed modules.
