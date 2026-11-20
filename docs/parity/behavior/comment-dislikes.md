# Behavior report: comment-dislikes

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/comment-dislikes.js` (primary).
  Members dislike/undislike routes bind a **different** controller,
  `docs/Ghost/ghost/core/server/api/endpoints/comments-members.js`; they are included
  here because the task requires "members app routes too" and the C++ has matching handlers.
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
  (admin browse route), `docs/Ghost/ghost/core/server/web/comments/routes.js`
  (members dislike/undislike routes), mounted at `/api/comments` via
  `docs/Ghost/ghost/core/server/web/members/app.js:141`.
- C++ folders: `include/blog/handlers/discovery/comments/` — `comment_dislikes/`,
  `dislike/`, `undislike/` (each `handler.cppm` + `workflow/flow/main.cppm` +
  `workflow/step/execute.cppm`), routes in `discovery/comments/routes.cppm`.
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 3 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Scope note: `comment-dislikes.js:8` exports exactly one operation, `browse`, and the
**only** route binding it is `admin/routes.js:55`. No route in `web/comments/routes.js`
or any members web route references `api.commentDislikes`; the members dislike/undislike
routes bind `api.commentsMembers.dislike/undislike` (`comments-members.js:184,197`).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments/:id/dislikes` (admin) | `browse`, docName `comment_dislikes` | `options=['id','page','limit']`; `validation.options.id.required`; `id` from path param → `frame.options.id` | `mw.authAdminApi` + `permissions:{docName:'comments'}` | 200; no `X-Cache-Invalidate` (`cacheInvalidate:false`) | `query`→`controller.getCommentDislikes`→`#assertCommentExists`→`CommentLike.findPage({filter:"comment_id:'<id>'+score:-1",withRelated:['member'],order:'created_at desc',page,limit})` | none in controller/framework; no invalidation | `{comment_dislikes:[...],meta:{pagination}}` via default `all` serializer; real pagination from Bookshelf `findPage` | `discovery/comments/routes.cppm`, `comment_dislikes/handler.cppm`, `comment_dislikes/workflow/flow/main.cppm`, `comment_dislikes/workflow/step/execute.cppm` | 200/422/404 via status map; token check; parses `id`,query `page/limit/include/fields/debug`; existence check; `json_for("comment_likes",filter[comment_id]/filter[score]:-1/limit/offset)`; seeds fake dislike w/ `member_id:"1"`; pagination computed in step; no `member` embed, no `created_at desc` | PARTIAL | Ghost `comment-dislikes.js:7-26`, `admin/routes.js:55`, `endpoints/index.js:271-272`, `comments-controller.js:463-469`, `comments-service.js:22,331-341,765-778`, `frame.ts:66-76`, `output/default.js:22-41`; C++ `routes.cppm:19-21,75-85`, `handler.cppm:13-37`, `execute.cppm:12-93`, `context_context.cppm:1152-1210,1561-1663` |
| POST `/:id/dislike` (members, mount `/api/comments`) | `dislike`, docName `comments` | `options=['id']`; `validation:{}`; `id` from path; member from `frame.options.context.member` | `permissions:false`; router `loadMemberSession`, `checkMemberCommenting`, `maxLimitCap`; controller `#checkMember` | 204; `X-Cache-Invalidate` set explicitly by `setCacheInvalidationHeaders` (post/replies/comment paths); `cacheInvalidate:false` only disables auto header | `dislike`→`#checkMember`→`dislikeComment`→`checkEnabled`→`#setCommentVote`(Member.findOne→checkCommentAccess→txn→published comment `forUpdate`→Post.findOne tiers→checkPostAccess→dedupe/destroy votes→`CommentLike.add`)→`getCommentByID`→`setCacheInvalidationHeaders` | no cache service; explicit invalidation header only | empty 204 body; output envelope irrelevant | `discovery/comments/routes.cppm`, `dislike/handler.cppm`, `dislike/workflow/flow/main.cppm`, `dislike/workflow/step/execute.cppm` | 204; same token check; `X-Cache-Invalidate:/api/members/comments/<id>/` only; `member_id` from `X-Member-Id` header default `"1"`; creates `comment_likes` record; no member session, no access/gating checks, no txn, no duplicate-vote 400, no 404 on unpublished comment | PARTIAL | Ghost `comments-members.js:184-195`, `web/comments/routes.js:70,73,86`, `members/app.js:141`, `comments-controller.js:75-81,211-225,424-438`, `comments-service.js:174-180,183-193,201,343-362,364-428,499-509`, `headers.ts:121-192`; C++ `routes.cppm:19-21,115-123`, `dislike/handler.cppm:13-46`, `dislike/execute.cppm:12-49`, `middleware/auth.cppm:12-29`, `middleware/serde.cppm:90-126` |
| DELETE `/:id/dislike` (members, mount `/api/comments`) | `undislike`, docName `comments` | `options=['id']`; `validation:{}`; `id` from path; member from context | same as POST dislike | 204; explicit `X-Cache-Invalidate` (post/replies/comment paths) | `undislike`→`#checkMember`→`undislikeComment`→`checkEnabled`→`#clearCommentVote`(txn→published comment `forUpdate`→member votes by score -1→404 if none→destroy)→`getCommentByID`→`setCacheInvalidationHeaders` | no cache service; explicit invalidation header only | empty 204 body | `discovery/comments/routes.cppm`, `undislike/handler.cppm`, `undislike/workflow/flow/main.cppm`, `undislike/workflow/step/execute.cppm` | 204; same token check; `X-Cache-Invalidate:/api/members/comments/<id>/` only; destroys all `comment_likes` rows for comment with score -1 (not scoped to member); no member scoping, no 404 on missing vote, no txn/access checks | PARTIAL | Ghost `comments-members.js:197-208`, `web/comments/routes.js:87`, `comments-controller.js:443-457`, `comments-service.js:430-453,511-521`; C++ `routes.cppm:124-131`, `undislike/handler.cppm:13-46`, `undislike/execute.cppm:12-56` |

## Route details

### GET /comments/:id/dislikes — PARTIAL

**Ghost** (checklist facts 1–8):

1. **Identity**: `router.get('/comments/:id/dislikes', mw.authAdminApi, http(api.commentDislikes.browse))` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:55`; router named `'admin api'` at `routes.js:15`, router-level `router.use(apiMw.cors)` at `:17`. `api.commentDislikes` resolves via `apiFramework.pipeline(require('./comment-dislikes'), localUtils)` at `docs/Ghost/ghost/core/server/api/endpoints/index.js:271-272`. No members route binds this controller (`web/comments/routes.js:1-93`; `members/app.js:141`).
2. **Operation**: exported key `browse` (`comment-dislikes.js:8`), `docName:'comment_dislikes'` (`:7`); `query(frame)` → `commentsService.service.controller.getCommentDislikes(frame)` (`:24-26`). Not `browse` via `data[]`; no custom action.
3. **Input**: `options:['id','page','limit']` (`:12`); `validation.options.id.required:true` (`:13-19`); no `data[]`, no multipart fields, no direct header reads. `id` path param is copied into `frame.options.id` by `Frame.configure` — `Object.assign(this.options, _.pick(this.original.params, optionNames))` after the query pick (`docs/Ghost/packages/api-framework/src/frame.ts:66-76`). Query aliases `?page=&limit=` accepted; no `filter[x]=y` declared.
4. **Auth/permissions**: `mw.authAdminApi` (`admin/routes.js:55`) is the chain `authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck` (`admin/middleware.js:110-118`). Controller `permissions:{docName:'comments'}` (`comment-dislikes.js:21-23`) — framework permission stage checks the `comments` doc for the browse op; no `unsafeAttrs`, no labs flag, no permission service called inside `query`.
5. **Status + headers**: no `statusCode` → 200; `headers.cacheInvalidate:false` (`:9-11`) makes `headers.get` skip auto invalidation (`docs/Ghost/packages/api-framework/src/headers.ts:135-144`); no `Content-Disposition`, `Location`, `stream`, or other header writes. JSON response via `http` wrapper.
6. **Full call chain**: `query` → `comments-controller.js:463-469` `getCommentDislikes(frame)` reads `frame.options.id/page/limit` → `comments-service.js:765-778` `getCommentDislikes`: `#assertCommentExists(commentId)` (`:766` → `:331-341`, throws `NotFoundError` if missing) then `models.CommentLike.findPage({filter:"comment_id:'<id>'+score:-1",withRelated:['member'],order:'created_at desc',page,limit})` (`:769-775`; `COMMENT_DISLIKE_SCORE=-1` at `:22`). `findPage` is Bookshelf CRUD pagination; returns `{data, meta}`. No events/jobs/mail/webhooks/third-party calls. Error branches: 404 `NotFoundError` (`:336-340`), 400 on invalid filter/order via CRUD sanitize.
7. **Cache**: no cache service/key/TTL; `cacheInvalidate:false` deliberately omits `X-Cache-Invalidate` (`headers.ts:135-144`).
8. **Output**: no docName-specific output serializer for `comment_dislikes` → default `all` (`docs/Ghost/ghost/core/server/api/endpoints/utils/serializers/output/default.js:22-41`) → `frame.response[docName]=response.data.map(...)` and `frame.response.meta=response.meta`. Envelope `{comment_dislikes:[<CommentLike w/ member>],meta:{pagination:{limit,page,pages,total,prev,next}}}`. Empty result → `{comment_dislikes:[],meta:{pagination:{...}}}` (no 404 for empty page).

**C++** (numbered facts):

1. **Files/symbols**: `CommentsRoutes::install` at `include/blog/handlers/discovery/comments/routes.cppm:15-180`, imported/ installed via `include/blog/handlers/routes.cppm:14,86`. `Router{router,"/comments"}` with `.use(content_type_middleware).use(require_auth)` (`:19-21`); `Router{router,"/:id"}` (`:50`); `Route{"/dislikes"}.get(...)` (`:75-85`) → `handler.comments_comment_dislikes().handle(...)`. Handler accessor `BlogHandlers::comments_comment_dislikes()` at `include/blog/handlers/all.cppm:415`, member `:695`; module imports `base.cppm:50`, `all.cppm:44`. Handler class `blog::handlers::comments::comment_dislikes::CommentDislikesHandler` at `comment_dislikes/handler.cppm:13-37`. Flow `MainFlow::build` at `comment_dislikes/workflow/flow/main.cppm:7-10` registers `comments_comment_dislikes`; step `ExecuteStep` at `comment_dislikes/workflow/step/execute.cppm:12-93`, name `comments_comment_dislikes_execute` (`:16`).
2. **Input type/parsing**: `using Input=interfaces::io::IRequest&`, `Output=std::map<std::string,std::string>` (`execute.cppm:14-15`). `invoke` parses query keys `page,limit,include,fields,debug` from `in.get_path()` between `?` and `&` (`:19`, helper `:54-66`) — accepts `include/fields/debug` not declared by Ghost. `id` = segment after `/comments/` and before next `/` (`:20`, helper `:72-79`). Empty `id` → 422 `ApiError{}.validation("Validation error","Missing required option","id")` (`:21-24`). `page`/`limit` parsed with fallbacks 1/15 (`:33-34`). No `data[]`/multipart equivalent.
3. **Status/body/header writes**: `CommentDislikesHandler::handle` (`handler.cppm:20-35`): if `middleware::token(req).empty()` → 401 `{"error":"Unauthorized"}` (`:21-25`); runs workflow (`:26`), reads `result_of("comments_comment_dislikes_execute")` typed map (`:27-29`), default `{"status":"200","body":"{}"}` (`:30`), casts status (`:31`), sets status+body and `send()` (`:33-34`). No `X-Cache-Invalidate` write (matches Ghost's omission).
4. **`ctx.store()` calls / cache**: existence check `ctx.store().json_for("comments",{{"filter[id]",id}})`; `"[]"` → 404 `{"errors":[{"message":"Comment not found"}]}` (`execute.cppm:25-29`). Params `filter[comment_id]=id`, `filter[score]="-1"`, `include="member"` (`:30-32`); `offset=(page-1)*limit`, `page` erased (`:35-37`). If `json_for("comment_likes",params)=="[]"`, seeds a record via `ctx.store().resources().create("comment_likes",{"comment_id":id,"member_id":"1","score":-1})` (`:38-41`) — fabricated row/hardcoded member `"1"`. Reads rows again (`:42`), `total=array_count(rows)` (`:43`). `json_for("comment_likes",...)` has no typed-model branch → falls through to `parse_array(m_resources.list("comment_likes"))` + `search_json` (`context_context.cppm:1209-1210`), which applies `filter[*]`, `limit`, `offset`, `include` (`:1603-1619`) over the in-memory resource store. No `core::cache::Cache` usage.
5. **Middleware chain**: `content_type_middleware` then `require_auth` on the `/comments` router (`routes.cppm:20-21`). `content_type_middleware` rejects missing/non-JSON `Content-Type` with 415 and missing/non-JSON `Accept` (or `*/*`) with 406, normalizes media type (`middleware/serde.cppm:90-126`). `require_auth` rejects empty token with a structured 401 `UnauthorizedError` envelope (`middleware/auth.cppm:12-29`); `token()` reads normalized Authorization header (`middleware/bearer.cppm:27-33`). The handler repeats a token check (divergence: redundant).
6. **Side effects**: one in-memory `comment_likes` fixture write when no live rows exist (`execute.cppm:38-41`); in-process `LocalWorkflowDriver` bookkeeping (`handler.cppm:17-18`). No outbox, no event emission, no real DB transaction.
7. **Output**: `{comment_dislikes:<rows>,meta:{pagination:<pagination(limit,page,total)>}}` with status 200 (`execute.cppm:44-45`; `pagination()` at `:86-90` computes `pages=ceil(total/limit)`, `prev=page>1?page-1:null`, `next=(page<pages)?page+1:null`). Envelope key and pagination shape agree with Ghost's default serializer (`output/default.js:30-40`).
8. **NOT implemented**: `withRelated:['member']` — `include=member` is parsed (`:32`) but `embeddings()` only handles authors/tags/newsletters/tiers (`context_context.cppm:1646-1663`), so no member relation is embedded; `order:'created_at desc'` is never passed (no ordering — `query_json_impl`/`search_json` apply no order, `context_context.cppm:1561-1643`); the permission `docName:'comments'` stage; real Knex `findPage` with server-side WHERE/ORDER and real pagination count; `NotFoundError` message template parity.

**Diff**:

- **Divergences (C++ differs from Ghost)**:
  - `member` relation: Ghost eager-loads `withRelated:['member']` (`comments-service.js:771`); C++ sets `include=member` (`execute.cppm:32`) but `embeddings()` ignores it (`context_context.cppm:1646-1663`). C++ rows lack the nested member payload.
  - Ordering: Ghost `order:'created_at desc'` (`comments-service.js:772`); C++ applies no order (`context_context.cppm:1561-1643`).
  - Fixture fallback: Ghost returns `[]` on an empty page; C++ fabricates a dislike row with hardcoded `member_id:"1"` (`execute.cppm:38-41`), so total/pagination derive from seeded data.
  - Accepted options: C++ parses `include/fields/debug` (`execute.cppm:19`) though Ghost declares only `id/page/limit` (`comment-dislikes.js:12`).
  - Auth/permissions: Ghost `authAdminApi` chain + `permissions.docName:'comments'` (`admin/routes.js:55`, `comment-dislikes.js:21-23`); C++ `content_type_middleware` 415/406 + `require_auth` + a second inline token check (`routes.cppm:20-21`, `serde.cppm:90-126`, `auth.cppm:12-29`, `handler.cppm:21-25`), with no permission-service stage.
  - Error shape: C++ 404 body is `{"errors":[{"message":"Comment not found"}]}` (`execute.cppm:28`) vs Ghost's templated `NotFoundError` (`comments-service.js:336-340`); 422 uses `ApiError` validation, not Ghost's framework schema validator (`frame.ts:66-76`, `validators/input/all.ts`).
- **Residual gaps (no C++ primitive)**: Ghost's `permissions` stage, real Bookshelf `CommentLike.findPage` with `withRelated`/`order`, and real Knex pagination semantics have no C++ equivalent; C++ uses in-memory `search_json` over a fixture/seeded resource store (`context_context.cppm:1209-1210,1592-1643`).

### POST /comments/:id/dislike — PARTIAL

Ghost side is a **members-app** route, not `comment-dislikes.js`.

**Ghost** (checklist facts 1–8):

1. **Identity**: `router.post('/:id/dislike', checkMemberCommenting, http(api.commentsMembers.dislike))` at `docs/Ghost/ghost/core/server/web/comments/routes.js:86`; router `'comment api'` (`:53`), mounted at `/api/comments` (`docs/Ghost/ghost/core/server/web/members/app.js:141`). Router-level: `bodyParser.json({limit:'50mb'})` (`:54`), `privateSiteAccess.loadSession` (`:55`), `loadMemberSession` (`:70`), `shared.middleware.maxLimitCap` (`:73`). `api.commentsMembers` resolved in `endpoints/index.js` (same pipeline pattern as `:256`).
2. **Operation**: endpoint key `dislike` (`comments-members.js:184`), docName `comments`; `query(frame)` → `commentsService.service.controller.dislike(frame)` (`:192-194`).
3. **Input**: `options:['id']` (`comments-members.js:189`), `validation:{}` (`:190`); `id` from path `:id`; member from `frame.options.context.member` (controller `:429`); no body fields/multipart; no direct header reads.
4. **Auth/permissions**: `permissions:false` (`comments-members.js:191`) — no permission doc check. Member identity from session (`loadMemberSession`, `web/comments/routes.js:70`); `checkMemberCommenting` (`:22-32`) rejects members with `can_comment===false` (403-style `NoPermissionError`); controller `#checkMember` throws `UnauthorizedError` when `context.member.id` absent (`comments-controller.js:75-81`).
5. **Status + headers**: `statusCode:204` (`comments-members.js:185`); `headers.cacheInvalidate:false` (`:186-188`) disables the auto header, but the controller explicitly calls `frame.setHeader('X-Cache-Invalidate', ...)` (`comments-controller.js:435`), and `headers.get` merges frame headers last (`headers.ts:187-189`), so the header IS emitted with post/replies/comment paths (`comments-controller.js:211-225`). No other headers.
6. **Full call chain**: controller `dislike` (`comments-controller.js:424-438`): `#checkMember` → `service.dislikeComment(id, member, options)` → `checkEnabled` (`comments-service.js:174-180`, 405 when comments off) → `#setCommentVote` (`:364-428`): `Member.findOne({id:member.id},{require:true,withRelated:['products']})` → `checkCommentAccess` (`:183`, `:376`) → `#withTransaction` (`:201`, `:378`) → `#getPublishedCommentForAction(...forUpdate:true)` (`:379-386`) → `Post.findOne({id:comment.post_id},{withRelated:['tiers']})` (`:388-397`) → `checkPostAccess` (`:192-193`, `:399`) → `#getMemberCommentVotes` (`:343-358`, `:401-407`) → if exactly one vote with score -1 throw `BadRequestError(alreadyDisliked)` (`:408-415`, message `comments-service.js:11`) → `#destroyCommentVotes` (`:360-362`, `:417`) → `CommentLike.add({member_id,comment_id,score:-1})` (`:419-426`). Then `getCommentByID` (`:780-784`) → `setCacheInvalidationHeaders` (`:435`). Errors: 401 (no member), 403 (`can_comment:false`), 405 (comments off), 404 (comment/post not found), 400 (already disliked/access), 500.
7. **Cache**: no cache service/key/TTL; explicit `X-Cache-Invalidate` lists `/api/members/comments/post/<postId>/`, `/api/members/comments/<parentId>/replies/`, `/api/members/comments/<commentId>/` (`comments-controller.js:218-224`).
8. **Output**: 204, empty body; no serializer envelope.

**C++** (numbered facts):

1. **Files/symbols**: same `CommentsRoutes::install` router; `Route{"/dislike"}.post(...)` at `discovery/comments/routes.cppm:115-123` → `handler.comments_dislike().handle(...)`. Accessor `all.cppm:423`, member `:703`, imports `base.cppm:58`, `all.cppm:52`. Handler `dislike::DislikeHandler` at `dislike/handler.cppm:13-47`; flow `dislike/workflow/flow/main.cppm:7-11`; step `dislike/workflow/step/execute.cppm:12-49` (`comments_dislike_execute`, `:16`).
2. **Input**: `Input=IRequest&`, `Output=std::map<string,string>` (`execute.cppm:14-15`). `id` = last path segment (`:19`, helper `:30-35`); empty → 422 validation `invalid("id")` (`:20`, `:40-43`). Member resolved from header `X-Member-Id`, default `"1"` (`:36-39`) — no session.
3. **Status/body/header**: `DislikeHandler::handle` (`handler.cppm:20-37`): inline token check → 401 `{"error":"Unauthorized"}` (`:21-25`); workflow (`:26`), payload with default `{"status":"204","body":""}` (`:28-32`); sets `X-Cache-Invalidate:/api/members/comments/<id>/` from `path_id` (`:33-34`, helper `:40-45`); sets status/body, `send()` (`:35-36`).
4. **`ctx.store()` / cache**: comment existence via `ctx.store().json_for("comments",{{"filter[id]",id}})`; `"[]"` → 404 `ApiError{}.json("Resource not found","Not Found")` (`execute.cppm:21-22`, `:44-47`). On success, `ctx.store().resources().create("comment_likes",{"comment_id":id,"member_id":member,"score":-1})` (`:23-24`). `json_for`/`resources().create` path at `context_context.cppm:1209-1210`. No `core::cache::Cache`.
5. **Middleware**: inherited `content_type_middleware` + `require_auth` (`routes.cppm:20-21`), 415/406 + structured-401 as above (`serde.cppm:90-126`, `auth.cppm:12-29`); plus redundant inline token check (`handler.cppm:21-25`). No member-session or `checkMemberCommenting` equivalent.
6. **Side effects**: in-memory `comment_likes` record creation (`:23-24`); workflow bookkeeping. No outbox/event/txn.
7. **Output**: 204, empty body (`:25`, handler `:30-32`).
8. **NOT implemented**: member session/`can_comment` gating, `checkEnabled`, `checkCommentAccess`/`checkPostAccess` content gating, real transaction with `forUpdate` row locking, duplicate-vote `BadRequestError` (C++ blindly inserts), published-comment/post resolution, the three-path invalidation header, and member-from-session (uses `X-Member-Id` header instead).

**Diff**:

- **Auth divergence**: Ghost member session + `permissions:false` + `checkMemberCommenting` (`web/comments/routes.js:70,86`, `comments-members.js:191`); C++ applies admin-style `require_auth` + token and reads member from `X-Member-Id` header (`routes.cppm:21`, `dislike/execute.cppm:36-39`). No C++ primitive for member sessions or `can_comment`.
- **Vote semantics divergence**: Ghost dedupes (400 already-disliked), validates member/comment/post access, locks rows, and respects `checkEnabled` (`comments-service.js:364-428`); C++ unconditionally creates a `comment_likes` row (`dislike/execute.cppm:23-24`) — duplicate dislikes allowed, no gating, no txn.
- **Cache-header divergence**: Ghost invalidates post + replies + comment paths (`comments-controller.js:218-224`); C++ sets only `/api/members/comments/<id>/` (`dislike/handler.cppm:34`), and the path prefix differs from the actual Ghost mount `/api/comments` (`members/app.js:141`); this is a stand-in marker, not a real cache invalidation.
- **Residual gaps**: permission services, content gating (`contentGating.checkPostAccess`), real Knex transactions/row locks, and mail/outbox/webhook side effects (none for this op in Ghost) have no C++ counterpart.

### DELETE /comments/:id/dislike — PARTIAL

**Ghost** (checklist facts 1–8):

1. **Identity**: `router.delete('/:id/dislike', checkMemberCommenting, http(api.commentsMembers.undislike))` at `web/comments/routes.js:87`; same router-level chain and `/api/comments` mount as POST (`members/app.js:141`).
2. **Operation**: endpoint key `undislike` (`comments-members.js:197`), docName `comments`; `query` → `controller.undislike(frame)` (`:205-207`).
3. **Input**: `options:['id']` (`comments-members.js:202`), `validation:{}` (`:203`); `id` from path; member from `context.member` (`comments-controller.js:448`).
4. **Auth/permissions**: `permissions:false` (`comments-members.js:204`); member session + `checkMemberCommenting` (`web/comments/routes.js:70,87`); `#checkMember` (`comments-controller.js:444`, `:75-81`).
5. **Status + headers**: `statusCode:204` (`comments-members.js:198`); explicit `X-Cache-Invalidate` via `frame.setHeader` (`comments-controller.js:454`, `:211-225`), merged by `headers.get` (`headers.ts:187-189`).
6. **Full call chain**: `undislike` (`comments-controller.js:443-457`) → `#checkMember` → `service.undislikeComment(id, member, options)` (`comments-service.js:511-521`) → `checkEnabled` → `#clearCommentVote` (`:430-453`): `#withTransaction` → `#getPublishedCommentForAction(...forUpdate:true)` (`:432-434`) → `#getMemberCommentVotes({commentId,memberId,score:-1})` (`:436-443`) → if none, `NotFoundError(Unable to find dislike)` (`:445-449`, message `comments-service.js:10`) → `#destroyCommentVotes` (`:451`). Then `getCommentByID` + `setCacheInvalidationHeaders`. Errors: 401/403/404/405/500.
7. **Cache**: explicit `X-Cache-Invalidate` post/replies/comment paths (`comments-controller.js:218-224`); no cache service.
8. **Output**: 204, empty body.

**C++** (numbered facts):

1. **Files/symbols**: `Route{"/dislike"}.delt(...)` at `discovery/comments/routes.cppm:124-131` → `handler.comments_undislike().handle(...)`. Accessor `all.cppm:424`, member `:704`, imports `base.cppm:59`, `all.cppm:53`. Handler `undislike::UndislikeHandler` at `undislike/handler.cppm:13-47`; flow `undislike/workflow/flow/main.cppm:7-11`; step `undislike/workflow/step/execute.cppm:12-56` (`comments_undislike_execute`, `:16`).
2. **Input**: `id` = last path segment (`undislike/execute.cppm:19`, helper `:30-35`); empty → 422 (`:20`, `:51-54`). No member input read at all.
3. **Status/body/header**: `UndislikeHandler::handle` (`undislike/handler.cppm:20-37`) mirrors dislike: token check → 401 (`:21-25`), workflow (`:26`), default `{"status":"204","body":""}` (`:28-32`), `X-Cache-Invalidate:/api/members/comments/<id>/` (`:33-34`), status/body/send (`:35-36`).
4. **`ctx.store()` / cache**: `ctx.store().json_for("comment_likes",{{"filter[comment_id]",id},{"filter[score]","-1"}})` (`undislike/execute.cppm:21-23`), then `for` each `record_ids(rows)` → `ctx.store().resources().destroy("comment_likes", like_id)` (`:24`, ids parsed by raw string scan `:36-50`). Returns 204 unconditionally (no 404 when no votes). No `core::cache::Cache`.
5. **Middleware**: inherited `content_type_middleware` + `require_auth` (`routes.cppm:20-21`) plus inline token check (`undislike/handler.cppm:21-25`); no member session/`can_comment` check.
6. **Side effects**: in-memory `comment_likes` deletions (`:24`); workflow bookkeeping. No txn, no access checks.
7. **Output**: 204, empty body (`:25`, handler `:30-32`).
8. **NOT implemented**: member scoping (deletes **all** score -1 likes for the comment, not just the caller's — Ghost filters by `memberId` in `#getMemberCommentVotes`, `comments-service.js:436-441`), 404 `Unable to find dislike` when no vote exists (`:445-449`), `checkEnabled`, `checkCommentAccess`/`checkPostAccess`, transaction/`forUpdate`, published-comment existence check, and the three-path invalidation header.

**Diff**:

- **Member scoping divergence**: C++ removes every `score:-1` row for the comment (`undislike/execute.cppm:21-24`), ignoring member identity; Ghost scopes to the acting member and 404s when no matching vote (`comments-service.js:436-449`). One member can clear another's dislike in C++.
- **Auth/header/residual gaps**: same require_auth/token + single stand-in `X-Cache-Invalidate` divergence as POST (`undislike/handler.cppm:21-25,34`); no member session, gating, txn, or permission service (no C++ primitive).

## Module-level notes

- Both controllers live in the same `commentsService` (`docs/Ghost/ghost/core/server/services/comments/index` via `comments-service.js`); `getCommentDislikes` is admin-only (comment moderation), while `dislike`/`undislike` are members operations with `permissions:false`. `comment-dislikes.js` and `comment-likes.js` are structurally identical (compare `comment-likes.js:1-30`).
- The C++ combines all comment routes — admin and members — under one `/comments` router with `content_type_middleware` + `require_auth` (`discovery/comments/routes.cppm:19-21`). Ghost separates admin (`/ghost/api/admin`, `authAdminApi`) from members (`/api/comments`, member session) (`admin/routes.js:55`, `web/comments/routes.js:86-87`, `members/app.js:141`). This is the primary structural divergence across all three routes.
- All C++ handlers repeat an inline token-nonempty check (`handler.cppm:21-25` in each folder) on top of `require_auth` — redundant with the router middleware.
- C++ stores dislikes as `comment_likes` records with `score:-1` in the in-memory `ResourceStore` (`context_context.cppm:152,1209-1210`); there is no dedicated `CommentLike` C++ model/entity. Filtering/offset/include are applied by `search_json` (`context_context.cppm:1592-1643`), but `include=member` is never embedded (`embeddings()` at `:1646-1663` handles only authors/tags/newsletters/tiers).
- Cross-route side effects in Ghost: `dislike`/`undislike` set `X-Cache-Invalidate` for post/replies/comment via `setCacheInvalidationHeaders` (`comments-controller.js:211-225`); C++ approximates with a single `/api/members/comments/<id>/` marker (`dislike/handler.cppm:34`, `undislike/handler.cppm:34`). `getCommentDislikes` has no invalidation on either side.
- No `core::cache::Cache` keys/tags/TTLs are used by any of the three C++ handlers; Ghost uses no cache service either. Cache parity is effectively limited to the (divergent) invalidation header on the write routes.
- The prior doc `docs/parity/comment-dislikes.md` is stale: its header note claims a MATCH after implementation, but its body still describes the pre-implementation fixture/dead-code version. This behavior report reflects the current source.
