# Behavior report: comments-members

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/comments-members.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/comments/routes.js`
- C++ folders: `include/blog/handlers/discovery/comments/` (mounted at `/comments` via `include/blog/handlers/routes.cppm:86`; Ghost mounts the same resource at `/members/api/comments` — `docs/Ghost/ghost/core/server/web/members/app.js:141`, `docs/Ghost/ghost/core/server/web/parent/frontend.js:20`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 8 |
| MISMATCH | 5 |
| MISSING | 0 |
| EXTRA | 3 |

Ghost has 13 verb+path routes; all 13 have a C++ counterpart (prefix differs: `/members/api/comments/*` vs `/comments/*`). C++ additionally exposes 3 routes that belong to other Ghost controllers (`comment-likes.js`, `comment-dislikes.js`, `comment-reports.js`), counted EXTRA here.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/api/comments/` | browse | options `post_id,include,page,limit,fields,filter,order,debug`; `include∈ALLOWED_INCLUDES` | `permissions:false`; route `checkCanReadComments`, member session, maxLimitCap | `cacheInvalidate:false` header | `commentsService.service.controller.browse` → `#withPostFilter` → `service.getComments` → `Comment.findPage` (parentId null, pinnedFirst) | no cache service; transformer `rejectContentApiRestrictedFieldsTransformer` | `{comments,meta.pagination}` via `commentMapper` (redaction) | `browse/handler.cppm`, `browse/.../execute.cppm` | token-gate; reads fixture store `json_for("comments")`; fabricated pagination; no status filter/redaction/checkEnabled | PARTIAL | Ghost `comments-members.js:36-51`, `comments-controller.js:139-141`, `comments-service.js:561-568`; C++ `browse/handler.cppm:20-35`, `browse/.../execute.cppm:17-30` |
| GET `/members/api/comments/counts` | counts | option `ids` (csv or array) | `permissions:false`; `checkCanReadComments` + `countsCache` (public/private cacheControl) | `cacheInvalidate:false`; `Cache-Control` maxAge from `caching:commentsCountAPI:maxAge` | `controller.count` → `stats.getAllCounts()` or `getCountsByPost(ids)` → `knex('comments')` grouped by post_id where status=published | `countsCache` sets Cache-Control; no X-Cache-Invalidate payload | raw `{postId:count}` map, `output/comments.js counts` passthrough | `counts/handler.cppm`, `counts/.../execute.cppm` | token-gate; counts fixture comments in memory; ignores `ids`; no Cache-Control/X-Cache-Invalidate | PARTIAL | Ghost `comments-members.js:147-156`, `routes.js:57-67`, `comments-controller.js:332-344`, `comments-stats-service.js:10-47`; C++ `counts/.../execute.cppm:17-40` |
| GET `/members/api/comments/post/:post_id` | browse (post_id) | path `post_id`; query options browse set | same as browse | `cacheInvalidate:false` | same as browse; `#withPostFilter` converts `post_id` into filter (AND with existing mongoTransformer) | same as browse | same as browse | `get_detail/handler.cppm`, `get_detail/.../execute.cppm` | token-gate; `filter=post_id:'id'`, but `total=store.total("comments")` (unfiltered) → fabricated pagination | PARTIAL | Ghost `routes.js:76`, `comments-controller.js:28-55`, `comments-service.js:561-568`; C++ `get_detail/.../execute.cppm:17-37` |
| GET `/members/api/comments/:id` | read | data `id,email`; options `include,fields` | `permissions:false`; `checkCanReadComments`, member session | `cacheInvalidate:false` | `controller.read` → `#withImpersonationContext` → `getCommentByID` → `#getReadableCommentByID` (`findOne`, status in [published,hidden], 404 otherwise) | none | `commentMapper`, html nulled for hidden/deleted | `read_by_id/handler.cppm`, `read_by_id/.../execute.cppm` | token-gate; `filter[id]=last_segment(path)`; 404 on `[]`; returns `{"comments":[...]}`; no status/redaction | PARTIAL | Ghost `comments-members.js:70-85`, `comments-controller.js:251-254`, `comments-service.js:273-293,780-784`; C++ `read_by_id/.../execute.cppm:17-31` |
| GET `/members/api/comments/:id/replies` | replies | options `include,page,limit,fields,filter,order,debug,id` | `permissions:false`; `checkCanReadComments`, member session | `cacheInvalidate:false` | `controller.replies` → `service.getReplies(id,options)` → `Comment.findPage` parentId=id, pinnedFirst; input serializer default order `created_at ASC,id ASC` | none | `{comments,meta.pagination}`, mapper redaction | `comment_replies/handler.cppm`, `comment_replies/.../execute.cppm` | token-gate; id via `segment_after("/comments/")`; `filter[parent_id]`; total from returned array (fabricated); no order/redaction/checkEnabled | PARTIAL | Ghost `comments-members.js:53-68`, `comments-controller.js:230-234`, `comments-service.js:700-707`, `input/comments.js:55-62`; C++ `comment_replies/.../execute.cppm:17-42` |
| POST `/members/api/comments/` | add (201) | body `data.comments[0]` with `post_id` (required), `parent_id`, `in_reply_to_id`, `html`; `X-Member-Id`/session member | `permissions:false`; `checkMemberCommenting` (can_comment false → 403), optional member session | 201, `cacheInvalidate:false`, `X-Cache-Invalidate` post/replies/self paths | `controller.add` → `#checkMember` → `commentOnPost`/`replyToComment` → member+post lookups, `checkCommentAccess`/`checkPostAccess`, `Comment.add`, emails (`sendNewCommentNotifications`), `DomainEvents.dispatch(MemberCommentEvent)`, re-fetch with relations | `setCacheInvalidationHeaders` builds `/api/members/comments/post/{id}/`, `/api/members/comments/{parent}/replies/`, `/api/members/comments/{id}/` | comment model via mapper | `create/handler.cppm`, `create/.../execute.cppm` | token-gate; unwraps `data.comments[0]`/`data.posts[0]`; requires `html` + (`post_id` or `parent_id`) → 422; member from `X-Member-Id` header (default none) or body; `mutations.create`; 201 from store; header `/comments/{id}` | PARTIAL | Ghost `comments-members.js:108-128`, `comments-controller.js:299-324,211-225`, `comments-service.js:793-929`, `comments-service-emails.js:61-120`; C++ `create/.../execute.cppm:17-56`, `create/handler.cppm:33-34` |
| PUT `/members/api/comments/:id` | edit | options `id,include`; body `data.comments[0]` (`html` or `status:deleted`) | `permissions:false`; `checkMemberCommenting`; `#checkMember` (401) | `cacheInvalidate:false`, `X-Cache-Invalidate` | `controller.edit` → `deleteComment` (ownership check, status=deleted, pinned_at null) or `editCommentContent` (ownership check, html+edited_at) → `Comment.edit` | `setCacheInvalidationHeaders` | edited comment | `edit/handler.cppm`, `edit/.../execute.cppm` | token-gate; body keys `status,pinned,html` passed opaquely to `mutations.edit`; ghost-id→numeric id coercion; no ownership/member/checkEnabled; header `/comments/{id}` | PARTIAL | Ghost `comments-members.js:87-106`, `comments-controller.js:272-294`, `comments-service.js:936-1003`; C++ `edit/.../execute.cppm:17-58`, `edit/handler.cppm:33-34` |
| DELETE `/members/api/comments/:id` | destroy | option `id,include` | `permissions:false`; `checkMemberCommenting` | declared 204 but always throws → 405; `cacheInvalidate:false` | `controller.destroy()` → `throw MethodNotAllowedError("You cannot destroy comments.")` | none | error envelope | `destroy/handler.cppm`, `destroy/.../execute.cppm` | token-gate (401 without token); returns 405 error `"Comment deletion is not allowed"`; no X-Cache-Invalidate:false header | PARTIAL | Ghost `comments-members.js:130-145`, `comments-controller.js:326-330`; C++ `destroy/.../execute.cppm:17-20`, `destroy/handler.cppm:20-34` |
| POST `/members/api/comments/:id/like` | like (204) | option `id` | `permissions:false`; `checkMemberCommenting`; `#checkMember` | 204, `cacheInvalidate:false`, `X-Cache-Invalidate` | `controller.like` → `checkEnabled` → `#setCommentVote(score=1)`: member lookup, access checks, transaction `forUpdate`, duplicate→400, destroy old votes, `CommentLike.add`, re-read comment for cache header | `setCacheInvalidationHeaders` | empty body | `like/handler.cppm`, `like/.../execute.cppm` | token-gate; `id=last_segment(path)` → "like" (wrong id); creates `comment_likes` score 1 with hardcoded member "1"; no gating/transaction/dup-check; header `/api/members/comments/like/` | MISMATCH | Ghost `comments-controller.js:349-363`, `comments-service.js:364-428,475-485`; C++ `like/.../execute.cppm:17-25,30-39`, `like/handler.cppm:33-34` |
| DELETE `/members/api/comments/:id/like` | unlike (204) | option `id` | same as like | 204, `cacheInvalidate:false`, `X-Cache-Invalidate` | `controller.unlike` → `#clearCommentVote(score=1)`: transaction `forUpdate`, 404 if no vote, destroy votes | `setCacheInvalidationHeaders` | empty | `unlike/handler.cppm`, `unlike/.../execute.cppm` | token-gate; `id=last_segment` → "like"; destroys `comment_likes` score 1 (no member filter); no 404 when missing; header wrong | MISMATCH | Ghost `comments-controller.js:368-382`, `comments-service.js:430-453,487-497`; C++ `unlike/.../execute.cppm:17-25` |
| POST `/members/api/comments/:id/dislike` | dislike (204) | option `id` | same as like | 204, `cacheInvalidate:false`, `X-Cache-Invalidate` | `controller.dislike` → `#setCommentVote(score=-1)` (same transaction/gating path) | `setCacheInvalidationHeaders` | empty | `dislike/handler.cppm`, `dislike/.../execute.cppm` | token-gate; `id=last_segment` → "dislike"; creates `comment_likes` score -1, member "1"; header wrong | MISMATCH | Ghost `comments-controller.js:424-438`, `comments-service.js:499-509`; C++ `dislike/.../execute.cppm:17-25` |
| DELETE `/members/api/comments/:id/dislike` | undislike (204) | option `id` | same as like | 204, `cacheInvalidate:false`, `X-Cache-Invalidate` | `controller.undislike` → `#clearCommentVote(score=-1)` | `setCacheInvalidationHeaders` | empty | `undislike/handler.cppm`, `undislike/.../execute.cppm` | token-gate; `id=last_segment` → "dislike"; destroys score -1 likes; no member filter/404; header wrong | MISMATCH | Ghost `comments-controller.js:443-457`, `comments-service.js:511-521`; C++ `undislike/.../execute.cppm:17-25` |
| POST `/members/api/comments/:id/report` | report (204) | option `id` | `permissions:false`; NO `checkMemberCommenting` (moderation allowed when commenting disabled); `#checkMember` | 204, `cacheInvalidate:false` | `controller.report` → `reportComment`: `checkEnabled`, published-comment lookup, duplicate `CommentReport.findOne` (silent no-op), `CommentReport.add`, `emails.notifyReport` (SMTP) | none | empty | `report/handler.cppm`, `report/.../execute.cppm` | token-gate; `id=last_segment` → "report"; creates `comment_reports` with member "1"; no duplicate check, no email, no member check | MISMATCH | Ghost `routes.js:89-90`, `comments-controller.js:387-395`, `comments-service.js:523-556`; C++ `report/.../execute.cppm:17-27` |
| GET `/comments/:id/likes` | (admin `getCommentLikes`) | page, limit, include, fields | C++ `require_auth` | 200 | Ghost counterpart lives in `comment-likes.js` (admin API), not in comments-members.js | — | `{comment_likes,meta.pagination}` | `comment_likes/.../execute.cppm` | EXTRA: reads comment, seeds a fake like for member "1", paginates in memory | EXTRA | C++ `routes.cppm:86-95`, `comment_likes/.../execute.cppm:17-45`; Ghost `endpoints/comment-likes.js` |
| GET `/comments/:id/dislikes` | (admin `getCommentDislikes`) | page, limit, include, fields | C++ `require_auth` | 200 | Ghost counterpart lives in `comment-dislikes.js` (admin API) | — | `{comment_dislikes,...}` | `comment_dislikes/.../execute.cppm` | EXTRA: seeds fake score -1 like, paginates | EXTRA | C++ `routes.cppm:75-85`, `comment_dislikes/.../execute.cppm:17-45`; Ghost `endpoints/comment-dislikes.js` |
| GET `/comments/:id/reports` | (admin `getCommentReporters`) | page, limit, include, fields | C++ `require_auth` | 200 | Ghost counterpart lives in `comment-reports.js` (admin API) | — | `{comment_reports,...}` | `comment_reports/.../execute.cppm` | EXTRA: seeds fake report, paginates | EXTRA | C++ `routes.cppm:144-155`, `comment_reports/.../execute.cppm:17-44`; Ghost `endpoints/comment-reports.js` |

## Route details

### GET /members/api/comments/ — PARTIAL

**Ghost**
1. Identity: `router.get('/', checkCanReadComments, http(api.commentsMembers.browse))` at `routes.js:75`; app-level `membersApp.use('/api/comments', commentRouter())` at `members/app.js:141`, mounted `/members` at `frontend.js:20`. Router-level: `bodyParser.json({limit:'50mb'})`, `privateSiteAccess.loadSession` (`routes.js:54-55`); members app adds `cacheControl('private')`, cors, magic-link session (`members/app.js:27-33`). After counts route: `loadMemberSession`, `maxLimitCap` (`routes.js:70-73`).
2. Operation key `browse`, docName `comments` (`comments-members.js:34,36`); op calls `commentsService.service.controller.browse`.
3. Input: `options: ['post_id','include','page','limit','fields','filter','order','debug']`; `validation.options.include = ALLOWED_INCLUDES` (14 tokens incl. `member`, `replies`, `count.likes`, `liked`, `disliked`, `post`, `parent` — `comments-members.js:16-30,40-45`). Input serializer `browse` restricts order via `rejectCommentsContentApiRestrictedOrderFields` for members API and defaults `created_at DESC, id DESC` (`input/comments.js:5-11,46-53`); `all` maps `liked`→`count.liked` etc. and appends post URL relations (`input/comments.js:13-44`).
4. Auth: `permissions:false` (`comments-members.js:46`); read access gated only by `checkCanReadComments` (private-site session) at `routes.js:38-47` — anonymous allowed on public site. No permission service in query.
5. Status/headers: default 200; `headers.cacheInvalidate:false` (`comments-members.js:37-39`).
6. Call chain: query → `restrictPublicQueryOptions(frame)` sets `frame.options.mongoTransformer = rejectContentApiRestrictedFieldsTransformer` (`comments-members.js:12-14,47-50`) → `controller.browse` → `#withPostFilter(frame.options)` (converts `post_id` to `filter: post_id:<id>` or ANDs transformer, `comments-controller.js:28-55`) → `service.getComments` → `checkEnabled()` (405 if `comments_enabled=off`, `comments-service.js:165-180,561-568`) → `models.Comment.findPage(withPinnedSelect({parentId:null, pinnedFirst:true}))` (only top-level; computed `pinned` column when requested, `comments-service.js:72-86`).
7. Cache: no cache service; `mongoTransformer` blocks restricted fields (`password,email,html,lexical,...,mobiledoc_revisions,post_revisions`, `api-filter-utils.ts:8-23,128-135,194-196`) from influencing `filter`/`order`; response header `X-Cache-Invalidate:false`.
8. Output: `commentMapper` envelope `{comments, meta.pagination}`; fields `id,parent_id,in_reply_to_id,in_reply_to_snippet,status,html,created_at,edited_at` + computed `pinned`, `member` (public fields `id,uuid,name,expertise,avatar_image`), `replies`, `post`, `liked`/`disliked`, `count{replies,direct_replies,likes}`; html nulled for non-published/deleted (`output/mappers/comments.js:6-33,51-155`). Empty → `comments:[]` with pagination meta.

**C++**
1. `Router{router,"/comments"}.use(content_type_middleware).use(require_auth).get(...)` → `comments_browse().handle` (`discovery/comments/routes.cppm:19-28`). No `/members/api` prefix; `content_type_middleware` requires JSON Content-Type and Accept on GET (`middleware/serde.cppm:90-117`), `require_auth` returns 401 envelope (`middleware/auth.cppm:12-29`); handler repeats a raw `{"error":"Unauthorized"}` 401 check via `token(req)` (`browse/handler.cppm:21-25`).
2. `ExecuteStep::invoke` parses query options from the raw path using a manual `?`/`&` split; allow-list includes an extra `include_nested` not in Ghost (`browse/.../execute.cppm:19-21,38-50`).
3. `limit` default 15, `page` default 1 (`execute.cppm:22-25`); `ctx.store().json_for("comments",params)` reads in-memory fixture model `m_db.all<model::Comment>()` (`context/context_context.cppm:1166`, `types/model_wire_fixtures.cppm`), not Knex.
4. `total=ctx.store().total("comments")` is the full store count, not the filtered count (`execute.cppm:27`), so pagination `pages/prev/next` are fabricated (`execute.cppm:56-61`).
5. Body `{"comments":<rows>,"meta":{"pagination":...}}`; `X-Cache-Invalidate:"false"` (`browse/handler.cppm:33`); status 200.
6. Not implemented: `checkEnabled`, status filter, restricted-field transformer, `post_id`→filter conversion (post_id passed as a raw param that `query_json` does not interpret as a filter), member session, private-site check, maxLimitCap, redaction/relation graph/`commentMapper`, real SQL, real pagination.

**Diff**: residual gaps — permissions/member middleware, Knex transactions, SMTP/email, `rejectContentApiRestrictedFieldsTransformer` security filter, default order. Divergences — auth required (Ghost public), fixture store + fabricated total, different route prefix, extra `include_nested` option.

### GET /members/api/comments/counts — PARTIAL

**Ghost**
1. `router.get('/counts', checkCanReadComments, countsCache, http(api.commentsMembers.counts))` (`routes.js:67`). `countsCache` = public `cacheControl('public',{maxAge:config.get('caching:commentsCountAPI:maxAge')})` or private `cacheControl('private')` when `res.isPrivateBlog` (`routes.js:57-66`).
2. Operation `counts`, docName `comments` (`comments-members.js:147`), `permissions:false`, `options:['ids']`.
3. Input: `ids` may be csv or array (`controller.js:337-341`). No body.
4. Auth: `checkCanReadComments` only; anonymous allowed on public site.
5. Status/headers: 200; `cacheInvalidate:false`; Cache-Control from countsCache.
6. Chain: `controller.count` → `stats.getAllCounts()` when no ids, else `stats.getCountsByPost(ids)` (`controller.js:332-344`) → `knex('comments').select(COUNT(*) AS count, post_id).where('status','published').groupBy('post_id')` (`comments-stats-service.js:10-47`).
7. Cache: public Cache-Control maxAge from config; private=private when site private.
8. Output: raw map `{post_id:count}`; `output/comments.js:1-5` counts serializer assigns frame.response. Missing ids → 0.

**C++**
1. `Route{"/counts"}.get(...)` → `comments_counts().handle` (`routes.cppm:38-48`); inherits `/comments` `content_type_middleware`+`require_auth` (token 401).
2. `invoke(const Input&,...)` ignores all input, including `ids` (`counts/.../execute.cppm:17`).
3. `ctx.store().json_for("comments",{{"filter[status]","published"}})` → in-memory fixture rows (`execute.cppm:19`); counts by `post_id` via manual JSON parse (`execute.cppm:20-31`).
4. Returns `{post_id:count}` body, status 200 (`execute.cppm:33-40`). Handler sets no cache headers (`counts/handler.cppm:20-35`).
5. Not implemented: `ids` filtering, Cache-Control/countsCache, private-site branch, X-Cache-Invalidate:false, Knex groupBy.

**Diff**: residual — cacheControl middleware, real SQL, private-site gating. Divergence — `ids` ignored (returns all counts), no cache headers, auth required.

### GET /members/api/comments/post/:post_id — PARTIAL

**Ghost** (same as browse, with post scoping)
1. `router.get('/post/:post_id', checkCanReadComments, http(api.commentsMembers.browse))` (`routes.js:76`).
2. Op `browse`, docName `comments` (`comments-members.js:36`).
3. Input: path `post_id` is an accepted option (`comments-members.js:40`).
4. Auth: `permissions:false`, `checkCanReadComments`.
5. Status/headers: 200, cacheInvalidate:false.
6. Chain: `controller.browse` → `#withPostFilter` — if `filter` present, wraps it under `$and:[{post_id}, transformedQuery]`; else sets `filter = post_id:<id>` (`comments-controller.js:28-55`) → `getComments` → `findPage` (`comments-service.js:561-568`).
7. Cache: transformer restriction same as browse.
8. Output: `{comments,meta.pagination}` with real filtered total.

**C++**
1. `Router{router,"/post"}.add_route(Route{"/:id"}.get(...))` → `comments_get_detail().handle` (`routes.cppm:167-179`).
2. `post_id=segment_after(path,"/post/")`; 422 validation if empty (`get_detail/.../execute.cppm:22-26`).
3. Builds `filter = "post_id:'<id>'"` (or `post_filter+"+"+user filter`) (`execute.cppm:27-29`); parses browse-style options (`execute.cppm:19-21`).
4. `total=ctx.store().total("comments")` — **unfiltered** total used for pagination → fabricated `pages/total` (`execute.cppm:35-37,72-77`).
5. `X-Cache-Invalidate:"false"` header, 200 (`get_detail/handler.cppm:33`).
6. Not implemented: checkEnabled, redaction/relations, restricted-field transformer, filtered total.

**Diff**: residual — real SQL/transformer/serializer. Divergence — route param named `:id` (Ghost `:post_id`), total unfiltered → incorrect pagination, auth required.

### GET /members/api/comments/:id — PARTIAL

**Ghost**
1. `router.get('/:id', checkCanReadComments, http(api.commentsMembers.read))` (`routes.js:77`).
2. Op `read`, docName `comments`; `options:['include','fields']`, `data:['id','email']` (`comments-members.js:70-85`).
3. Auth: `permissions:false`.
4. Status/headers: 200, cacheInvalidate:false.
5. Chain: `controller.read` → `#withImpersonationContext` (admin UUID→member id; not used by members route) → `getCommentByID(frame.data.id, options)` → `#getReadableCommentByID` uses `Comment.findOne({id}, filter status:[published,hidden])` → `NotFoundError` 404 when missing (`comments-controller.js:251-254`, `comments-service.js:273-293,780-784`).
6. Output: single comment via `commentMapper` (html nulled for hidden/deleted, `in_reply_to_snippet`), 404 on missing (`output/mappers/comments.js:51-155`).

**C++**
1. `Router{"/:id"}.get(...)` → `comments_read_by_id().handle` (`routes.cppm:49-58`); `content_type_middleware` + `require_auth`.
2. `id=last_segment(path)`; empty → 422 validation (`read_by_id/.../execute.cppm:19-24`); `params["filter[id]"]=id` (`execute.cppm:25`).
3. `json_for("comments",params)`; `[]` → 404 `{"comments":[]}`; else `{"comments":[...]}` (`execute.cppm:26-30`).
4. Not implemented: status restriction (hidden/deleted readable), member relations/redaction, `findOne` semantics, checkEnabled, `email` data option.

**Diff**: residual — serializer/relations/status filter. Divergence — auth required, different 404 body shape, no redaction.

### GET /members/api/comments/:id/replies — PARTIAL

**Ghost**
1. `router.get('/:id/replies', checkCanReadComments, http(api.commentsMembers.replies))` (`routes.js:78`).
2. Op `replies`; `options` include `id`; include allow-list (`comments-members.js:53-68`).
3. Auth: `permissions:false`.
4. Status/headers: 200, cacheInvalidate:false.
5. Chain: `controller.replies` strips `id` from options → `service.getReplies(id,options)` → `Comment.findPage({...options,parentId:id}, pinnedFirst)` (`comments-controller.js:230-234`, `comments-service.js:700-707`); input serializer `replies` sets default order `created_at ASC, id ASC` (`input/comments.js:55-62`).
6. Output: `{comments,meta.pagination}` with real total and mapper redaction.

**C++**
1. `Route{"/replies"}.get(...)` → `comments_comment_replies().handle` (`routes.cppm:133-143`).
2. `id=segment_after(path,"/comments/")` (correctly extracts the `:id` segment for this route); 422 if empty; 404 `{"errors":[{"message":"Comment not found"}]}` if comment missing (`comment_replies/.../execute.cppm:22-32`).
3. `params["filter[parent_id]"]=id`; limit/page defaults; `total=array_count(rows)` — total equals the current page length (fabricated pagination) (`execute.cppm:33-41,76-86`).
4. Not implemented: oldest-first order, checkEnabled, redaction/relations, real total.

**Diff**: residual — order/serializer/checkEnabled. Divergence — fabricated total from page length, auth required, different 404 shape.

### POST /members/api/comments/ — PARTIAL

**Ghost**
1. `router.post('/', checkMemberCommenting, http(api.commentsMembers.add))` (`routes.js:81`); `checkMemberCommenting` → 403 NoPermissionError when `req.member.can_comment===false` (`routes.js:22-32`).
2. Op `add`, docName `comments`; `statusCode:201`; `validation.data.post_id.required=true`; include allow-list (`comments-members.js:108-128`).
3. Input: `frame.data.comments[0]` with `post_id`/`parent_id`/`in_reply_to_id`/`html`; member id from member session context.
4. Auth: `permissions:false`; `#checkMember` throws `UnauthorizedError` 401 if no `context.member.id` (`comments-controller.js:75-81,299-300`).
5. Status/headers: 201, `cacheInvalidate:false`, `X-Cache-Invalidate` with post/replies/self paths (`comments-controller.js:211-225,321`).
6. Chain: `commentOnPost` (`comments-service.js:793-849`) or `replyToComment` (`:859-929`): `checkEnabled`, member lookup (`withRelated:['products']`), `checkCommentAccess` (paid gating), post lookup (`withRelated:['tiers']`), `checkPostAccess` (content gating), `Comment.add`, `sendNewCommentNotifications` → `emails.notifyPostAuthors` + `notifyParentCommentAuthor` (SMTP, `comments-service-emails.js:61-120`, `:456-473`), `DomainEvents.dispatch(MemberCommentEvent.create(...))`, re-fetch `Comment.findOne` with relations. Reply path rejects replying to a reply (400) and validates parent statuses.
7. Output: created comment via `commentMapper`.

**C++**
1. `Router.post` → `comments_create().handle` (`routes.cppm:30-37`); inherits content-type+auth middleware; `token` 401 check (`create/handler.cppm:21-25`).
2. `include` validated against only 4 values `post,member,replies,replies.member` (Ghost allow-list has 14) → 422 on other values (`create/.../execute.cppm:20-25`).
3. Unwraps `data.comments[0]` (or `data.posts[0]`) into entity (`execute.cppm:28-46`); requires `html` and (`post_id` or `parent_id`) → 422 validation (`execute.cppm:47-51,104-107`).
4. Member from `X-Member-Id` header merged into body (`execute.cppm:52-54`); no member session/gating.
5. `mutations.create("comments",body)` on store (`execute.cppm:55-56`); status from store; handler sets `X-Cache-Invalidate:"/comments/"+id` parsed from body (`create/handler.cppm:33-34,40-48`).
6. Not implemented: checkEnabled, paid/content gating, ownership, email notifications, DomainEvents, relation re-fetch, transaction, Ghost's multi-path cache header.

**Diff**: residual — permissions/member session, content gating, SMTP, domain events, Knex transaction. Divergence — header-based member id (default empty), reduced include set, different X-Cache-Invalidate value, fixture create.

### PUT /members/api/comments/:id — PARTIAL

**Ghost**
1. `router.put('/:id', checkMemberCommenting, http(api.commentsMembers.edit))` (`routes.js:82`).
2. Op `edit`; `options:['id','include']`; validation `id.required=true`, include allow-list (`comments-members.js:87-106`).
3. Input: `data.comments[0].html` or `status:'deleted'`.
4. Auth: `permissions:false`; `#checkMember` 401; route `checkMemberCommenting` 403.
5. Status/headers: 200, cacheInvalidate:false, X-Cache-Invalidate multi-path.
6. Chain: `controller.edit` → `deleteComment` (ownership check → 403, sets status=deleted+pinned_at null) or `editCommentContent` (ownership check → 403, sets html+edited_at) → `models.Comment.edit` (`comments-controller.js:272-294`, `comments-service.js:936-1003`).
7. Output: edited comment; `checkEnabled` 405 when disabled.

**C++**
1. `Router.put` → `comments_edit().handle` (`routes.cppm:59-66`); content-type+auth+token 401 (`edit/handler.cppm:21-25`).
2. `id=last_segment(path)`; empty→422; ghost-id→numeric coercion loop over 0..99999 (`edit/.../execute.cppm:20-24,51-56`).
3. Body unwrapped from `data.comments[0]`; keys `status,pinned,html` copied opaquely into update JSON (`execute.cppm:25-50`) — `pinned` is not accepted by the members controller.
4. `mutations.edit("comments",numeric_id,body)` (`execute.cppm:57`); header `/comments/{id}` (`edit/handler.cppm:33-34`).
5. Not implemented: ownership check, member check, checkEnabled, deleted-status special handling, edited_at, dropped `pinned` semantics, real multi-path cache header.

**Diff**: residual — permissions/member context. Divergence — accepts `pinned` (admin-only field), no ownership check, fabricated id coercion, different cache header.

### DELETE /members/api/comments/:id — PARTIAL

**Ghost**
1. `router.delete('/:id', checkMemberCommenting, http(api.commentsMembers.destroy))` (`routes.js:83`).
2. Op `destroy`; `statusCode:204` declared (`comments-members.js:131`) but `query()` unconditionally calls `controller.destroy()` which throws `MethodNotAllowedError("You cannot destroy comments.")` → 405 (`comments-controller.js:326-330`).
3. Auth: `permissions:false`; route `checkMemberCommenting`.
4. Status/headers: effective 405; cacheInvalidate:false.

**C++**
1. `.delt(...)` → `comments_destroy().handle` (`routes.cppm:67-74`); content-type+auth+token 401 (`destroy/handler.cppm:21-25`).
2. Step returns 405 with `{"errors":[{"message":"Comment deletion is not allowed","type":...}]}` via `ApiError{}.json` (`destroy/.../execute.cppm:17-20`); handler default status 405 (`destroy/handler.cppm:30-31`).
3. Not implemented: no `X-Cache-Invalidate:false` header.

**Diff**: residual — none material. Divergences — different error message, auth 401 before 405, missing cacheInvalidate:false header. Semantically both 405 → close but not exact.

### POST /members/api/comments/:id/like — MISMATCH

**Ghost**
1. `router.post('/:id/like', checkMemberCommenting, http(api.commentsMembers.like))` (`routes.js:84`).
2. Op `like`, `statusCode:204`, `options:['id']`, `permissions:false` (`comments-members.js:158-169`).
3. Auth: `checkMemberCommenting` + `#checkMember` 401.
4. Status/headers: 204, cacheInvalidate:false, X-Cache-Invalidate multi-path.
5. Chain: `controller.like` → `likeComment` → `checkEnabled` → `#setCommentVote(score=1)` → Member lookup with products, `checkCommentAccess`, transaction with `forUpdate` on published comment, Post lookup with tiers, `checkPostAccess`, duplicate single target vote → 400 `alreadyLiked`, destroy existing votes, `CommentLike.add(score=1)`, then re-read comment and `setCacheInvalidationHeaders` (`comments-controller.js:349-363`, `comments-service.js:364-428,475-485`).

**C++**
1. `Route{"/like"}.post(...)` → `comments_like().handle` (`routes.cppm:97-105`).
2. `id=last_segment(path)` — for `/comments/<id>/like` this yields the literal `"like"`, not the comment id (`like/.../execute.cppm:19,30-35`). Comment-existence check `filter[id]=like` → typically `[]` → 404.
3. If found, creates `comment_likes` `{"comment_id":"like","member_id":"1","score":1}` — hardcoded member `1`, no gating/transaction/duplicate check (`execute.cppm:21-25,36-39`).
4. 204 empty body; handler sets `X-Cache-Invalidate:"/api/members/comments/like/"` (`like/handler.cppm:33-34`).

**Diff**: residual — permissions, gating, transaction, real member context. Divergences — **wrong id extraction** (last path segment is the action verb), hardcoded member id, no duplicate/404 semantics matching Ghost, wrong cache paths.

### DELETE /members/api/comments/:id/like — MISMATCH

**Ghost**: `controller.unlike` → `#clearCommentVote(score=1)` → transaction `forUpdate`, 404 `likeNotFound` if none, destroy votes; 204 + X-Cache-Invalidate (`comments-controller.js:368-382`, `comments-service.js:430-453,487-497`).
**C++**: `Route{"/like"}.delt(...)` → `comments_unlike()` (`routes.cppm:106-112`); `id=last_segment` → `"like"`; destroys `comment_likes` with `filter[comment_id]=like,filter[score]=1` for **all members** (no member filter), returns 204 even when nothing removed; header `/api/members/comments/like/` (`unlike/.../execute.cppm:17-25`, `unlike/handler.cppm:33-34`).
**Diff**: wrong id, no member scoping, no 404 on missing vote, wrong cache path, no transaction/gating.

### POST /members/api/comments/:id/dislike — MISMATCH

**Ghost**: `controller.dislike` → `#setCommentVote(score=-1)` (same transaction/gating/dup-check as like, message `alreadyDisliked`) (`comments-controller.js:424-438`, `comments-service.js:499-509`).
**C++**: `Route{"/dislike"}.post` → `comments_dislike()` (`routes.cppm:115-123`); `id=last_segment` → `"dislike"`; creates score -1 like with member `"1"`, 204, header `/api/members/comments/dislike/` (`dislike/.../execute.cppm:17-25,36-39`, `dislike/handler.cppm:33-34`). Same divergence set as like.

### DELETE /members/api/comments/:id/dislike — MISMATCH

**Ghost**: `controller.undislike` → `#clearCommentVote(score=-1)` → 204 / 404 `dislikeNotFound` (`comments-controller.js:443-457`, `comments-service.js:511-521`).
**C++**: `Route{"/dislike"}.delt` → `comments_undislike()` (`routes.cppm:124-131`); `id=last_segment` → `"dislike"`; destroys score -1 likes, 204 always (`undislike/.../execute.cppm:17-25`). Same divergences as unlike.

### POST /members/api/comments/:id/report — MISMATCH

**Ghost**
1. `router.post('/:id/report', http(api.commentsMembers.report))` (`routes.js:90`) — deliberately no `checkMemberCommenting` (moderation allowed with commenting disabled).
2. Op `report`, `statusCode:204`, `options:['id']`, `permissions:false` (`comments-members.js:210-221`).
3. Auth: `#checkMember` 401 if no member context.
4. Chain: `reportComment` → `checkEnabled` (405 when off), published-comment lookup (`REPORT_REQUIRED_COLUMNS`), duplicate `CommentReport.findOne` → silent no-op if already reported, `CommentReport.add`, `emails.notifyReport(comment,reporter)` SMTP (`comments-controller.js:387-395`, `comments-service.js:523-556`).
5. Output: 204 empty.

**C++**
1. `Route{"/report"}.post` → `comments_report().handle` (`routes.cppm:155-165`); content-type+auth+token 401.
2. `id=last_segment` → `"report"`; creates `comment_reports` with member `"1"`, 204 (`report/.../execute.cppm:17-27`).
3. Not implemented: member context, duplicate check, SMTP email, checkEnabled.

**Diff**: wrong id extraction, hardcoded member, no email side effect, auth model differs (token vs member session).

### GET /comments/:id/likes — EXTRA

C++ route `routes.cppm:86-95` → `comment_likes` step: parses comment id via `segment_after("/comments/")`, 404 if comment missing, forces `filter[score]=1`, `include=member`, and **seeds a fake like for hardcoded member `"1"`** when none exists, then paginates in memory with `offset` (`comment_likes/.../execute.cppm:17-45`). No counterpart in `comments-members.js`; Ghost's equivalent admin endpoint lives in `endpoints/comment-likes.js` (`admin/routes.js:54`). Not part of this module's contract.

### GET /comments/:id/dislikes — EXTRA

C++ `routes.cppm:75-85` → `comment_dislikes` step seeds a fake score `-1` like for member `"1"` and paginates (`comment_dislikes/.../execute.cppm:17-45`). Ghost counterpart is admin `comment-dislikes.js` (`admin/routes.js:55`), not comments-members.

### GET /comments/:id/reports — EXTRA

C++ `routes.cppm:144-155` → `comment_reports` step seeds a fake report for member `"1"` and paginates (`comment_reports/.../execute.cppm:17-44`). Ghost counterpart is admin `comment-reports.js` (`admin/routes.js:53`), not comments-members.

## Module-level notes

- **Controller surface**: `comments-members.js` exports 12 keys — `browse, replies, read, edit, add, destroy, counts, like, unlike, dislike, undislike, report` — all `permissions:false`, all with `headers.cacheInvalidate:false` (`comments-members.js:33-222`). `docName:'comments'` (`:34`). Only `browse` and `replies` install the restricted-field `mongoTransformer` (`:48,65`); `read` does not (`:82-84`).
- **Ghost service graph**: `defineService('Comments',...)` wires `CommentsService` + `CommentsController` + `CommentsStats({db})` with `GhostMailer`, members `contentGating`, `urlService`, `DomainEvents`/`MemberCommentEvent`, `byNQL`, `createUnsubscribeUrl` (`comments/index.ts:25-69`).
- **C++ structure**: every handler is a thin wrapper that token-checks, runs a single-step `LocalWorkflowDriver` workflow, then maps a `map<string,string>{status,body}` payload onto the response (`<op>/handler.cppm` + `<op>/workflow/flow/main.cppm` + `<op>/workflow/step/execute.cppm`). No `Compensate` logic. `ctx.store()` is an in-memory resource store backed by seeded fixtures (`context/context_context.cppm:900,1152-1166`, `types/model_wire_fixtures.cppm`), with `create/edit/destroy` mutations (`context_context.cppm:1298-1299,1346-1347`). No `core::cache::Cache` is used by any comments handler; cache behavior is only stand-in `X-Cache-Invalidate` header strings.
- **Cross-cutting Ghost side effects absent in C++**: `comments_enabled` gating (`checkEnabled`, 405), paid-member + post content gating, `Comment.add/edit`, Knex transactions with `FOR UPDATE`, `CommentLike` vote reconciliation, `CommentReport` dedup, SMTP notification emails (`notifyPostAuthors`, `notifyParentCommentAuthor`, `notifyReport`), `MemberCommentEvent` domain events, private-site session check, member commenting-ability check, `maxLimitCap`, restricted-field transformer, real pagination counts, and `commentMapper` redaction of hidden/deleted content.
- **Cross-cutting C++ divergences**: all 13 matched routes are mounted at `/comments/*` not `/members/api/comments/*`; every matched route requires a bearer token (`require_auth` + per-handler `token()`), whereas Ghost's members comments API is public except member-context actions; content-type middleware enforces JSON headers on GETs; five action routes (like/unlike/dislike/undislike/report) derive the comment id as the last path segment (the action verb) → operate on the wrong id; member identity is a hardcoded `"1"` or `X-Member-Id` header rather than a session; pagination totals are fabricated (full store count or current-page length); three admin-only GET routes are exposed as EXTRA.
- **No MISSING routes**: every Ghost comments-members route has a C++ handler file.
