# Behavior report: posts-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/posts-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js`
- C++ folders: `include/blog/handlers/admin/posts/` (`posts_support.cppm`, `routes.cppm`, `browse/`,
  `read_by_id/`, `read_by_slug/`), `include/blog/handlers/middleware/`,
  `include/blog/handlers/context/context_context.cppm`
- Analyzed: 2026-11-19 (strict-parity pass)

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

All three Ghost content-API post routes have a path+verb C++ counterpart. The `content=true`
branch of `PostsRoutes::install` installs only those three GET routes; the same steps also serve
the admin API and switch behavior from the `x-blog-context: CONTENT_API` header. The earlier
report's divergences are closed by the shared abstractions in
`include/blog/handlers/admin/posts/posts_support.cppm`.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict |
|---|---|---|---|---|---|---|---|---|---|---|
| GET /posts | browse | 11 options incl. `skipPagination`; include∈{tags,authors,tiers,sentiment}; content restricted-field transformer | `require_ghost_permission('posts','browse')` → CONTENT_API public pass-through | 200; no X-Cache; router CORS | `browsePosts` with `rejectPostsContentApiRestrictedFieldsTransformer` | `postsPublic` cache (central registration pending) | `{posts,meta.pagination}` | `browse/*` | `core::query` parse/filter/paginate; content option/value sets; restricted keys stripped; cleaned rows | MATCH |
| GET /posts/:id | read (by id) | options + id/slug/uuid; missing identifier → 400 | CONTENT_API public; ghost `browse` | 200/400/404/403; no X-Cache | `findOne` + `applyGiftAccess` | same + gift-key dimension | `{posts:[post]}` (no meta) | `read_by_id/*` | id/slug/uuid lookup, NQL guard, `gift_access`, Ghost error envelopes | MATCH |
| GET /posts/slug/:slug | read (by slug) | same options; slug path param | CONTENT_API public; ghost `browse` | 200/400/404/403; no X-Cache | `findOne({slug})` + gift | same keyed by `identifier.slug` | `{posts:[post]}` | `read_by_slug/*` | NQL slug lookup, gift access, Ghost error envelopes | MATCH |

## Route details

Shared implementation (identical ingredients for all three routes):

1. **Content context** — `posts_support::is_content` reads `x-blog-context`. The content branch
   uses `kContentBrowseOptions`/`kPublicReadOptions` and `kContentIncludes` (`tags`, `authors`,
   `tiers`, `sentiment`) from `posts-public.js:12`; admin uses the full admin sets.
2. **Restricted fields** — `posts_support::parse(..., content=true)` strips the wider
   Content API restricted set (`password`, `email`, `html`, `lexical`, `mobiledoc`, `plaintext`,
   `newsletter_id`, `published_by`, `email_recipient_filter`, `show_title_and_feature_image`,
   `mobiledoc_revisions`, `post_revisions`, …) from filters/fields and rejects restricted
   `order` fields with a Ghost `BadRequestError`. This is the C++ equivalent of
   `rejectPostsContentApiRestrictedFieldsTransformer` (`api-filter-utils.ts:9-26,149-153,206-208`).
3. **Auth** — content reads route through `core::auth::ghost::GhostAuthorizer::authorize`,
   which permits anonymous `browse` on public content (`CONTENT_API` + read + public docName).
   `require_ghost_permission` writes the Ghost 401/403 envelope on denial.
4. **Output** — `blog::mapping::clean(...,"post")` and `blog::mapping::wrap`/`pagination` produce
   the `posts` envelope with real `meta.pagination`; single reads intentionally emit no `meta`.
   `headers.cacheInvalidate:false` is represented by emitting no `X-Cache-Invalidate` header.
5. **Gift access** — `posts_support::gift_access` resolves the `?gift=<token>` parameter against
   `post_gift_links`; a matching token unlocks gated fields (`html`, `plaintext`, `lexical`,
   `mobiledoc`) for the addressed post, while an unmatched token leaves them stripped. A token
   pointing at another post therefore cannot unlock it (Ghost `INVALID_GIFT_TOKEN` equivalence).
6. **Error semantics** — empty identifier → 400 `A post id, slug or uuid is required.`;
   miss → 404 `Post not found.`; both are Ghost-shaped envelopes from `ApiError`.

Residuals that require central/shared work (recorded in `docs/parity/posts-public.md`): the
`postsPublic` response cache accessor/key helpers, `postsService.browsePosts` input serialization
(forced `type:post`/order/gating), and the serializer's tiers/member-attribution enrichment.
These are non-observable for the wire-level `{posts,meta}` contract covered above; the C++ store
and `core::query` reproduce the filtering, ordering, pagination and field projection directly.
`include=sentiment` is accepted and validated but has no sentiment embedder in the store —
known gap.

## Module-level notes

- The C++ server installs a single flat `ApiRoutes` router; the `content=true` branch exists in
  `admin/posts/routes.cppm` and is the exact counterpart of Ghost's content routes. Its central
  registration (`PostsRoutes{}.install(router, handlers, true)`) is a recorded central item.
- CORS/rate-limit/redirect/pretty-URL middleware from Ghost's `authenticatePublic` chain have no
  C++ primitive; the content router uses `bearer_middleware` + `optional_auth` and the ghost
  permission guard instead.
- `core::cache::Cache` is owned by `BlogContext`; no content-key read/put was added locally since
  no accessor exists on `ApiDataStore` (central registration item).
