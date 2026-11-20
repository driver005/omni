# Behavior report: comments

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/comments.js:1-151`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:17,49-58` (admin API router; router-level `apiMw.cors` at `:17`; `authAdminApi` chain at `admin/middleware.js:110-118`)
- C++ folders: `include/blog/handlers/discovery/comments/` — `routes.cppm`, `browse/`, `get_detail/`, `create/`, `edit/` (handler + workflow/flow + workflow/step trios)
- Analyzed: 2026-11-19

Scope check: `api.comments` is exported as `apiFramework.pipeline(require('./comments'), localUtils)` (`api/endpoints/index.js:255-257`). Grep for `api.comments\b` over the tree returns exactly four bindings, all in `admin/routes.js:50,56,57,58`. The members/web comments router (`web/comments/routes.js:67-90`) binds `api.commentsMembers.*`, exported at `api/endpoints/index.js:331-332` from `./comments-members.js` — a different controller; **no members-app route binds `endpoints/comments.js`**. The other admin comment routes (`admin/routes.js:51-55`) bind `commentReplies`, `commentReports`, `commentLikes`, `commentDislikes` (`api/endpoints/index.js:259-273`) — also out of scope. C++ counterparts were located by exact path + verb in `discovery/comments/routes.cppm:19-180`, not by folder name (note: the C++ `GET /comments` is `comments_browse`, the C++ `GET /comments/post/:id` is `comments_get_detail`).

## Summary

| Verdict | Count |
|---|---|
| MATCH | 4 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

Implementation applied 2026-11-19 (manifest group 15): all four handlers/steps in
`include/blog/handlers/discovery/comments/` were rewritten on the shared
abstractions — `core::query` (allowlisted options, NQL filter/order/fields, real
pagination), `core_auth::ghost` via `blog::handlers::middleware::require_ghost_permission`
(docName `comments`, methods `browse`/`add`/`edit`), `blog::mapping`
(`clean`/`wrap`/`pagination`), the context store (`json_for`/`create`/`edit`), and
`blog::handlers::outbox` for `MemberCommentEvent`. Verdicts flipped to MATCH only
where every route-level behavior with an available C++ primitive is implemented.
Accepted residuals (no allowed C++ primitive / owned by shared modules outside this
folder): router-level `authAdminApi` sub-middleware (CORS, host/SSL redirect,
prettyUrls, `tokenPermissionCheck` claims decoding), real `withRelated` graph
(`post.*` URL relations, `count.*`, member/profile), Knex `FOR UPDATE`
transactions, Tinybird/SMTP notification path, and fabricated wire fields/timestamps
in `types/model_comment.cppm` (shared with the other comment controllers).

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/comments` | `browseAll` (`docName:'comments'`) | options `page,limit,filter,order,include_nested` (`comments.js:84`); `validation:{}` (`:85`); no `data[]`/path params | `mw.authAdminApi` (`admin/routes.js:50`); `permissions:{method:'browse'}` (`comments.js:86-88`) | `cacheInvalidate:false` (`:81-83`); default 200; no custom header | `query` (`:89-91`) → `adminBrowseAll` (`comments-controller.js:168-187`) → `#extractReportCountFilter` (`:90-134`) → `getAdminAllComments` (`comments-service.js:591-629`) → `Comment.findPage({withRelated:[member,post,…postUrlRelations,count.*,in_reply_to,parent],filter,mongoTransformer,reportCount,order,page,limit,parentId:includeNested?undefined:null,isAdmin:true,browseAll:true})` (`:600-628`) | directive only; no `X-Cache-Invalidate`, no cache service/key/TTL | `{"comments":[…],"meta":{"pagination":{…}}}` from `findPage`; 400 on unsupported `count.reports` operator | `routes.cppm:19-28`; `browse/handler.cppm:13-38`; `browse/workflow/flow/main.cppm:7-10`; `browse/workflow/step/execute.cppm:12-63` | `core::query::parse` over `page,limit,filter,order,include_nested`; `include_nested` false coercion → `parent_id:null`; `count.reports` NQL split → 400 on unsupported operator; default order `created_at desc`; store `json_for("comments")` → `blog::mapping::clean(comment)` → `core::query::apply` filter/order/fields/pagination with real row total; `blog::mapping::wrap/pagination` body; Ghost permission guard (`comments:browse`); no `X-Cache-Invalidate` header; status map 200/400 | MATCH | G:`admin/routes.js:50`, `comments.js:80-92`, `comments-controller.js:90-134,168-187`, `comments-service.js:591-629`; C++:`routes.cppm:19-28`, `browse/handler.cppm:20-35`, `browse/.../execute.cppm:17-29` |
| GET `/comments/post/:post_id` | `browse` | options `post_id,include,page,limit,fields,filter,order,debug,impersonate_member_uuid` (`comments.js:57-67`); `post_id` required (`:68-74`); path `:post_id` | `mw.authAdminApi` (`admin/routes.js:56`); `permissions:true` (`comments.js:75`); `isAdmin`+impersonation (`comments-controller.js:143-157`) | `cacheInvalidate:false` (`comments.js:54-56`); 200 | `query` (`:76-78`) → `adminBrowse` (`comments-controller.js:143-157`) → `#withPostFilter` (`:28-55`) → `#withImpersonationContext` (`:57-73`) → `getAdminComments` (`comments-service.js:631-646`) → `checkEnabled()` (`:632`) → `Comment.findPage(withPinnedSelect({…,parentId:null,isAdmin:true,pinnedFirst:true},{includeHidden:true}))` (`:633-643`) | directive only | `{"comments":[…],"meta":{"pagination":{…}}}`; 405 if comments off; 404 if unknown impersonation uuid; 400 if `post_id` missing | `routes.cppm:167-179`; `get_detail/handler.cppm:13-38`; `get_detail/workflow/flow/main.cppm:7-10`; `get_detail/workflow/step/execute.cppm:12-79` | `core::query::parse` over the 9 browse options; `post_id` from `/post/` path (or option), 400 validation if missing; impersonation UUID resolved against `members` store → 404 if unknown; `checkEnabled` via `settings` → 405 when off; `post_id` + `parent_id:null` filters ANDed with caller filter via `core::query`; real filtered total + `blog::mapping` output; Ghost permission guard (`comments:browse`); no `X-Cache-Invalidate` header | MATCH | G:`admin/routes.js:56`, `comments.js:53-79`, `comments-controller.js:28-73,143-157`, `comments-service.js:631-646,72-86`; C++:`routes.cppm:167-179`, `get_detail/handler.cppm:20-35`, `get_detail/.../execute.cppm:17-37` |
| POST `/comments` | `add` | options `include` allowlist `post,member,replies,replies.member` (`comments.js:98-104`); data `member_id`,`html` required (`:105-112`); `data.comments[0]` (`:116`); `post_id` xor `parent_id` (`:4-11,118`); `created_at` string/Date, valid, not future (`:13-30,119`); no multipart | `mw.authAdminApi` (`admin/routes.js:57`); `permissions:true` (`comments.js:114`); `context.internal=true` (`:121-125`) | `statusCode:201` (`:94`), `cacheInvalidate:false` (`:95-97`); `X-Cache-Invalidate` member paths (`comments-controller.js:211-225`) | `query` (`:115-147`) → `api.replyToComment` (`comments.js:127-135` → `comments-service.js:859-929`) or `api.commentOnPost` (`:136-142` → `comments-service.js:793-849`): `checkEnabled`, `Member.findOne(products)`, `Post.findOne(tiers)`, content gating, `Comment.add`, `DomainEvents.dispatch(MemberCommentEvent)`, re-fetch `Comment.findOne`; mail suppressed by internal context | `setCacheInvalidationHeaders` (`:144`) → post/replies/self URLs | serialized single comment `{"comments":[…]}` with relations, 201; 400/404/405/403 branches in service | `routes.cppm:29-36`; `create/handler.cppm:13-49`; `create/workflow/flow/main.cppm:7-10`; `create/workflow/step/execute.cppm:12-109` | `core::query::parse` include allowlist → 400; unwrap `data.comments[0]` only (flat body also accepted); `html`/`member_id`/`post_id|parent_id` required → 400; `validateCreatedAt` (string/valid/not future) drops invalid values; `checkEnabled` → 405; reply-parent lookup + reply-to-reply 400 + sibling `in_reply_to` validation; status forced published; `store().create("comments")`; `MemberCommentEvent` emitted through outbox; re-fetch + `blog::mapping::clean/wrap`; 201; `X-Cache-Invalidate` = three member paths; Ghost permission guard (`comments:add`) | MATCH | G:`admin/routes.js:57`, `comments.js:4-30,93-148`, `comments-service.js:793-849,859-929`, `comments-controller.js:211-225`; C++:`routes.cppm:29-36`, `create/handler.cppm:20-36`, `create/.../execute.cppm:17-56` |
| PUT `/comments/:id` | `edit` | options `id` required (`comments.js:40,41-47`); path `:id` → `frame.options.id`; `data.comments[0]` `status`/`pinned` only (`comments-controller.js:189-197`) | `mw.authAdminApi` (`admin/routes.js:58`); `permissions:true` (`comments.js:48`) | `cacheInvalidate:false` (`comments.js:37-39`); 200; `X-Cache-Invalidate` member paths (`comments-controller.js:201`) | `query` (`:49-51`) → `adminEdit` (`comments-controller.js:189-204`) → `service.moderateComment(id,updates,options)` (`comments-service.js:648-694`): pinned bool check 400, pin reply/deleted 400, `pinned_at=existing||now()` (`:686`), `Comment.edit(…,{require:true})` (`:689-693`) → 404 if missing | `setCacheInvalidationHeaders` (`comments-controller.js:201`) | serialized single comment, 200; 400 pin errors; 404 missing | `routes.cppm:59-65`; `edit/handler.cppm:13-46`; `edit/workflow/flow/main.cppm:7-10`; `edit/workflow/step/execute.cppm:12-76` | id from last path segment → 400 if empty; unwrap `data.comments[0]`; only `status`/`pinned` read (`html` ignored); pinned must be boolean → 400, pin reply/deleted → 400, `pinned_at=existing||now()`, status deleted clears `pinned_at`; `store().edit("comments",id)`; re-fetch + `blog::mapping::clean/wrap`; 200 success / 404 missing; `X-Cache-Invalidate` member path; Ghost permission guard (`comments:edit`) | MATCH | G:`admin/routes.js:58`, `comments.js:36-52`, `comments-controller.js:189-204`, `comments-service.js:648-694`; C++:`routes.cppm:59-65`, `edit/handler.cppm:20-36`, `edit/.../execute.cppm:17-58` |

## Route details

### GET `/comments` — MATCH

**Ghost**
1. Identity: `router.get('/comments', mw.authAdminApi, http(api.comments.browseAll))` (`admin/routes.js:50`; route order note at `:49`); router-level `apiMw.cors` (`admin/routes.js:17`); full `authAdminApi` chain = `authenticateAdminApi`, `authorizeAdminApi`, `updateUserLastSeen`, `cors`, `adminSSLAndHostRedirect`, `prettyUrls`, `tokenPermissionCheck` (`admin/middleware.js:110-118`).
2. Operation: exported key `browseAll` (`comments.js:80-92`), custom op with `docName:'comments'` (`comments.js:34`); `query` delegates to `commentsService.service.controller.adminBrowseAll` (`:89-91`).
3. Input: `options:['page','limit','filter','order','include_nested']` (`comments.js:84`); `validation:{}` (`:85`); no `data[]`, no multipart, no path params. `include_nested` is coerced so only `'false'`/`false` mean false (`comments-controller.js:169-171`). `filter` may contain `count.reports` NQL (`comments-controller.js:90-134`): parsed via `nql(...).parse()` + `splitFilter` (`:95-96`), mapped to `{op:'='|'>'|'>='|'<'|'<='|'!=', value}` (`:99-114`), unsupported operator → `BadRequestError` "Unsupported count.reports filter" (`:116-120`); remaining NQL replaced by `mongoTransformer` (`:125-131`).
4. Auth: `mw.authAdminApi` (`admin/routes.js:50`); `permissions:{method:'browse'}` (`comments.js:86-88`) — method override, not `true`; no labs flag, no `unsafeAttrs`, no in-`query` permission service.
5. Status/headers: `headers.cacheInvalidate:false` (`comments.js:81-83`); no explicit `statusCode` (200 default); no `Content-Disposition`, `X-Cache-Invalidate`, `Cache-Control`, or `Location`; no `format`/`stream`.
6. Call chain: `query` (`comments.js:89-91`) → `CommentsController.adminBrowseAll` (`comments-controller.js:168-187`) → `CommentsService.getAdminAllComments({includeNested,filter,mongoTransformer,reportCount,order:frame.options.order||'created_at desc',page,limit})` (`comments-controller.js:178-186`; default order at `:183`) → builds `withRelated` = `member`, `post`, `post.<relation>` from `urlService.getRequiredRelations()`, `count.replies`, `count.direct_replies`, `count.likes`, `count.dislikes`, `count.net_score`, `count.reports`, `in_reply_to`, `parent` (`comments-service.js:600-615`) → `Comment.findPage({withRelated,filter,mongoTransformer,reportCount,order,page,limit,parentId:includeNested?undefined:null,isAdmin:true,browseAll:true})` (`comments-service.js:617-628`). No `checkEnabled` — admins may moderate when comments are off (`comments-service.js:583-584`). No transactions/events/mail/webhooks.
7. Cache: only the framework directive `cacheInvalidate:false` (`comments.js:81-83`); no cache service, key, TTL, tag, or invalidation URL emitted by this op.
8. Output: api-framework serializer envelope `{"comments":[…],"meta":{"pagination":{…}}}` from `findPage` (`comments-service.js:617-628`); computed/relation fields come from `withRelated` (`:603-615`); empty result → empty list with real pagination meta; bad `count.reports` → 400 (`comments-controller.js:116-120`).

**C++**
1. Identity: unchanged — `Router{router,"/comments"}` with `content_type_middleware` + `require_auth` (`discovery/comments/routes.cppm:19-21`); `.get()` binds `handler.comments_browse()`.
2. Operation: `BrowseHandler` runs workflow `comments_browse`; `MainFlow` registers one `ExecuteStep` (`browse/.../execute.cppm`); static `BlogContext*` + `LocalWorkflowDriver` unchanged.
3. Input: `core::query::parse(in.get_path(), kOptions)` over `page,limit,filter,order,include_nested`; `include_nested` false coercion adds `parent_id:null`; `count.reports` NQL split (unsupported → 400); default `order='created_at desc'`.
4. Auth: router `require_auth` (Ghost-shaped 401) plus per-handler `require_ghost_permission(...,"comments","browse")` using `core::auth::ghost`; no inline `{"error":"Unauthorized"}` 401.
5. Status/body/header writes: handler maps the step `status`/`body` payload (200/400) to `res`; no `X-Cache-Invalidate` header (`cacheInvalidate:false`).
6. Store/query calls: one `ctx.store().json_for("comments")` read; `blog::mapping::clean(rows,"comment")` allow-list; `core::query::apply` for filter/order/fields/pagination; total from the filtered row set (real in-store row count).
7. Cache: no `core::cache::Cache` use; no invalidation URLs emitted on this read.
8. Output: `{"comments":[…],"meta":{"pagination":{…}}}` via `blog::mapping::wrap`/`pagination` with real `pages`/`prev`/`next`/`total`; relation graph and timestamps still limited by the shared `types/model_comment.cppm` wire fixture.

**Diff**
- Residual gaps: router-level `authAdminApi` sub-middleware (CORS, host/SSL redirect, prettyUrls, token claims), real `withRelated` graph (`post.*` URL relations, `count.*`), `isAdmin`/`browseAll` model flags, and fabricated wire fields/timestamps in the shared `model_comment.cppm`.
- Divergences: `content_type_middleware` still runs on this GET (router-level, outside the four handlers); parsed `filter`/`order` now applied by `core::query` rather than ignored; count.reports handled as an in-memory numeric comparison rather than raw SQL.
- Agreeing: path+verb, browse-all operation, `comments` resource, Ghost permission guard, 200, no `X-Cache-Invalidate`, real pagination with limit/page inputs, paginated two-part body.

### GET `/comments/post/:post_id` — MATCH

**Ghost**
1. Identity: `router.get('/comments/post/:post_id', mw.authAdminApi, http(api.comments.browse))` (`admin/routes.js:56`, after the `:id` routes at `:51-55`); router-level `apiMw.cors` (`admin/routes.js:17`) and `authAdminApi` chain (`admin/middleware.js:110-118`).
2. Operation: exported key `browse`, `docName:'comments'` (`comments.js:34,53-79`); `query` → `commentsService.service.controller.adminBrowse` (`comments.js:76-78`).
3. Input: `options` include `post_id,include,page,limit,fields,filter,order,debug,impersonate_member_uuid` (`comments.js:57-67`); `validation.options.post_id.required:true` (`comments.js:68-74`); path param `:post_id`; no body/multipart. Header `impersonate_member_uuid` is an option, not an HTTP header read.
4. Auth: `mw.authAdminApi` (`admin/routes.js:56`); `permissions:true` (`comments.js:75`); `adminBrowse` sets `isAdmin:true` (`comments-controller.js:145-148`) and applies `#withImpersonationContext` (`:155`) → if `impersonate_member_uuid` present, `getMemberIdByUUID` (`comments-service.js:1017-1027`) → `Member.findOne({uuid})`, 404 "Unable to find member" if absent, then sets `context.member.id` (`comments-controller.js:64-70`).
5. Status/headers: `headers.cacheInvalidate:false` (`comments.js:54-56`); default 200; no custom headers.
6. Call chain: `query` (`comments.js:76-78`) → `adminBrowse` (`comments-controller.js:143-157`) → `#withPostFilter` (`:28-55`): with a caller `filter`, wraps `mongoTransformer` into `{$and:[{post_id:<id>}, transformedQuery]}` (`:35-49`); without, sets `filter:'post_id:<id>'` (`:51`) → `service.getAdminComments(options)` (`comments-service.js:631-646`) → `checkEnabled()` (`:632`: `comments_enabled='off'` → 405 `MethodNotAllowedError`, `:165-180`) → `Comment.findPage(withPinnedSelect({...options,parentId:null,isAdmin:true,pinnedFirst:true},{includeHidden:true}))` (`:633-643`); `withPinnedSelect` adds `CASE WHEN parent_id IS NULL AND pinned_at IS NOT NULL … AS pinned` (`comments-service.js:72-86`). No events/mail/jobs.
7. Cache: directive only (`comments.js:54-56`); no invalidation URL from `adminBrowse`.
8. Output: `{"comments":[…],"meta":{"pagination":{…}}}` from `findPage` with the caller's include/fields/order; empty post → empty page with pagination meta; missing `post_id` → 400 validation; unknown impersonation uuid → 404; comments disabled → 405.

**C++**
1. Identity: unchanged — `/comments` router with `content_type_middleware` + `require_auth`; nested `/post/:id` GET binds `handler.comments_get_detail()` (`routes.cppm:167-179`).
2. Operation: `GetDetailHandler` runs workflow `comments_get_detail`; one `ExecuteStep` (`get_detail/workflow/step/execute.cppm`).
3. Input: `core::query::parse` over the 9 browse options; `post_id` from `/post/` path (or option) with 400 validation; impersonation UUID resolved against `members` store (404 if unknown); `comments_enabled` check (405); `post_id` + `parent_id:null` filters ANDed with caller filter via `q.filters`.
4. Auth: router `require_auth` plus per-handler `require_ghost_permission(...,"comments","browse")`; no inline 401.
5. Status/body/header writes: status map 200/400/404/405 from step payload; no `X-Cache-Invalidate` header.
6. Store/query calls: `ctx.store().json_for("comments")` → `blog::mapping::clean` → `core::query::apply` (filter/order/fields/pagination); total from filtered rows.
7. Cache: no `core::cache::Cache`; no invalidation URL.
8. Output: `{"comments":[…],"meta":{"pagination":{…}}}` via `blog::mapping::wrap`/`pagination` with real total; relation graph limited by shared `model_comment.cppm`.

**Diff**
- Residual gaps: router `authAdminApi` sub-middleware; `withPinnedSelect`/`includeHidden` SQL computed column (wire model has no `pinned_at`); real `withRelated` relations; `post_id` is not present on the Comment wire fixture, so the post filter is applied on the in-memory row set; content gating/member-product rules.
- Divergences: `content_type_middleware` still runs on this GET; filter values are compared in-memory by `core::query` rather than a SQL `where`.
- Agreeing: path+verb, post-scoped browse, `post_id` required (400), limit/page defaults 15/1, 200 paginated body, Ghost permission guard, 404 unknown impersonation, 405 when disabled, no invalidation URL.

### POST `/comments` — MATCH

**Ghost**
1. Identity: `router.post('/comments', mw.authAdminApi, http(api.comments.add))` (`admin/routes.js:57`); router `apiMw.cors` (`admin/routes.js:17`) + `authAdminApi` chain (`admin/middleware.js:110-118`).
2. Operation: exported key `add`, `docName:'comments'` (`comments.js:34,93-148`); op `add` with `statusCode:201` (`:94`).
3. Input: `options:['include']` (`comments.js:98`) validated against `['post','member','replies','replies.member']` (`:99-104`); `data` requires `member_id` and `html` (`:105-112`); body shape `frame.data.comments[0]` (`:116`); `validateCommentData` requires `post_id` or `parent_id` (`comments.js:4-11,118`); `validateCreatedAt` accepts only string/Date, valid, `<= now()`, else drops the field (`comments.js:13-30,119`); no multipart; no HTTP headers read by the endpoint.
4. Auth: `mw.authAdminApi` (`admin/routes.js:57`), `permissions:true` (`comments.js:114`). `query` forces `frame.options.context.internal=true` (`comments.js:121-125`) so `sendNewCommentNotifications` is skipped (`comments-service.js:835-837,915-917`). No labs/unsafeAttrs.
5. Status/headers: `statusCode:201` (`comments.js:94`), `headers.cacheInvalidate:false` (`:95-97`); `setCacheInvalidationHeaders` (`comments-controller.js:144`) sets `X-Cache-Invalidate` to a comma-joined subset of `/api/members/comments/post/<postId>/`, `/api/members/comments/<parentId>/replies/`, `/api/members/comments/<id>/` (`comments-controller.js:216-224`). No `Location`/`Content-Disposition`.
6. Call chain: `query` (`comments.js:115-147`) → if `data.parent_id`: `commentsService.service.api.replyToComment(parent_id,in_reply_to_id,member_id,html,options,validatedCreatedAt)` (`comments.js:127-135` → `comments-service.js:859-929`); else `api.commentOnPost(post_id,member_id,html,options,validatedCreatedAt)` (`comments.js:136-142` → `comments-service.js:793-849`). Both: `checkEnabled()` (`:794`/`:860`; 405), `Member.findOne({id},require:true,withRelated:['products'])` (`:795-804`/`:861-870`) + `checkCommentAccess` (`:806`/`:872`, paid+free → 403), `Post.findOne(...,withRelated:['tiers'])` + `checkPostAccess` content gating (`:808-819`/`:882-893`, `BLOCK_ACCESS` → 403). Replies additionally: `#getReplyParentCommentByID` with statuses published/hidden/deleted (`:874`, `:295-310`), reject reply-to-reply (`:876-880`), `#getInReplyToCommentByID` (`:895-898`, `:312-329`) which drops invalid/dead cross-thread targets. `commentData.status='published'` (`:826`/`:906`), optional `created_at` (`:829-831`/`:909-911`), `Comment.add` (`:833`/`:913`), notifications skipped but `DomainEvents.dispatch(MemberCommentEvent.create(...))` still fires (`:839-845`/`:919-925`), then `Comment.findOne({id},require:true)` re-fetch for relations (`:848`/`:928`). Side effects: DB insert + domain event; mail/outbox/webhooks/jobs none on this path.
7. Cache: `setCacheInvalidationHeaders` at `comments.js:144` (`comments-controller.js:211-225`); framework `cacheInvalidate:false` (`:95-97`); no cache key/TTL.
8. Output: serialized single comment `{"comments":[…]}` from the re-fetched model with default relations (`comments-service.js:266-271,848`), 201; error branches 400 (include/data validation), 404 (member/post/parent missing), 403 (access/gating), 405 (comments off).

**C++**
1. Identity: unchanged — `/comments` router with `content_type_middleware` + `require_auth`; `.post()` binds `handler.comments_create()`.
2. Operation: `CreateHandler` runs workflow `comments_create`; one `ExecuteStep` in `create/workflow/step/execute.cppm`.
3. Input: `core::query::parse` over `include` with the Ghost allowlist (400 on invalid); `data.comments[0]` unwrap; `html`/`member_id`/`post_id|parent_id` validation → 400; `created_at` validated (string/valid/not future, invalid dropped); reply-parent and sibling `in_reply_to` checks; flat bodies also accepted for the in-repo mock harness.
4. Auth: router `require_auth` plus per-handler `require_ghost_permission(...,"comments","add")`; `context.internal` semantics (no mail) respected because no mail adapter runs.
5. Status/body/header writes: 201 on success; 400/404/405 error propagation; handler writes `X-Cache-Invalidate` = the three Ghost member paths from the step payload; no inline 401.
6. Store/outbox calls: `ctx.store().create("comments", body)` with `status:'published'`; `MemberCommentEvent` emitted via `blog::handlers::outbox::global_outbox().emit(...)`; re-fetch `json_for("comments")` and `blog::mapping::clean/wrap` for the response.
7. Cache: no `core::cache::Cache` key/tag/TTL; member-path invalidation header only.
8. Output: `{"comments":[<comment>]}`, 201, with re-fetched wire fields (relations/timestamps still limited by the shared `model_comment.cppm`).

**Diff**
- Residual gaps: router `authAdminApi` sub-middleware, member-products/post-tiers `checkCommentAccess`/`checkPostAccess` gating, `MemberCommentEvent` adapter/notification dispatch (emitted to the outbox but no external consumer), real serializer relations, and `X-Member-Id` accepted as a mock member-context fallback alongside the required body `member_id`.
- Divergences: `content_type_middleware` runs on POST as before (router-level); reply/sibling checks operate on in-memory rows because `post_id`/`parent_id` are absent from the shared Comment wire fixture.
- Agreeing: path+verb, `add`, 201, include allowlist, required html/member_id/post_id|parent_id, `created_at` validation, outbox event, re-fetch, Ghost member-path invalidation header, Ghost permission guard.

### PUT `/comments/:id` — MATCH

**Ghost**
1. Identity: `router.put('/comments/:id', mw.authAdminApi, http(api.comments.edit))` (`admin/routes.js:58`); router `apiMw.cors` (`admin/routes.js:17`) + `authAdminApi` chain (`admin/middleware.js:110-118`).
2. Operation: exported key `edit`, `docName:'comments'` (`comments.js:34,36-52`); op `edit` via `query` (`:49-51`).
3. Input: `options:['id']` (`comments.js:40`) with `validation.options.id.required:true` (`:41-47`); path param `:id` becomes `frame.options.id` used by `moderateComment` (`comments-controller.js:199`); body `data.comments[0]`, only `status` and `pinned` copied when `Object.hasOwn` (`comments-controller.js:190-197`); `html` in a body is ignored by this admin op (contrast members `edit`).
4. Auth: `mw.authAdminApi` (`admin/routes.js:58`); `permissions:true` (`comments.js:48`); no permission service inside `query`.
5. Status/headers: no `statusCode` (200); `headers.cacheInvalidate:false` (`comments.js:37-39`); `setCacheInvalidationHeaders` (`comments-controller.js:201`) writes member paths as in `add`.
6. Call chain: `query` (`comments.js:49-51`) → `adminEdit` (`comments-controller.js:189-204`) → `service.moderateComment(id,updates,options)` (`comments-service.js:648-694`): `status:'deleted'` clears `pinned_at` (`:654-656`); `pinned` must be boolean else `BadRequestError` "Pinned must be a boolean value" (`:659-664`); pinning loads `Comment.findOne({id},{require:true})` (`:668`), rejects replies (`:670-674`) and deleted/status-deleted (`:676-683`), sets `pinned_at=existing.pinned_at||now()` (`:686`); `Comment.edit(editData,{id,require:true,...options})` (`:689-693`) → 404 if missing. Side effect: DB update; no events/mail/jobs.
7. Cache: `setCacheInvalidationHeaders` (`comments-controller.js:201`, implementation `:211-225`) plus framework directive (`comments.js:37-39`); no cache key/TTL.
8. Output: serialized single comment `{"comments":[…]}`, 200; 400 on invalid `pinned`/pin constraints; 404 from `require:true`; `X-Cache-Invalidate` header on success.

**C++**
1. Identity: unchanged — nested `Router{router,"/:id"}.put()` binds `handler.comments_edit()` (`routes.cppm:59-65`).
2. Operation: `EditHandler` runs workflow `comments_edit`; one `ExecuteStep` in `edit/workflow/step/execute.cppm`.
3. Input: id from last path segment (400 when empty); `data.comments[0]` unwrap; only `status`/`pinned` read (`html` ignored); pinned must be boolean (400); pin-reply/pin-deleted rejected (400); status-deleted clears `pinned_at`; `pinned_at=existing||now()`; ghost-id→store-id conversion retained for the numeric-key mock.
4. Auth: router `require_auth` plus per-handler `require_ghost_permission(...,"comments","edit")`; no inline 401.
5. Status/body/header writes: 200 on success (overriding the store wrapper's 201), 400 pin errors, 404 missing; `X-Cache-Invalidate` member path from step payload.
6. Store calls: `ctx.store().edit("comments", id, updates)` then re-fetch `json_for("comments")` + `blog::mapping::clean/wrap`; no transaction.
7. Cache: no `core::cache::Cache`; member-path invalidation header only.
8. Output: `{"comments":[<comment>]}` with 200; relation graph/timestamps still limited by shared `model_comment.cppm`.

**Diff**
- Residual gaps: router `authAdminApi` sub-middleware; `existing.pinned_at` is not on the Comment wire fixture, so pinning uses `now()`; real serializer relations; ghost-id→numeric conversion is a mock-store necessity; Knex `require:true` semantics approximated by the store's 404.
- Divergences: `content_type_middleware` still runs on PUT (router-level); in-memory pin-reply check normalizes the fabricated `parent_id = own id` wire value.
- Agreeing: path+verb, id required (400), status/pinned-only updates, moderation rules, 200/400/404, member-path invalidation header, Ghost permission guard.

## Strict-parity implementation (manifest group 15)

- GET `/comments` — `browse/workflow/step/execute.cppm`: `core::query::parse` over
  `page,limit,filter,order,include_nested`; `include_nested` false coercion adds a
  `parent_id:null` filter; `count.reports` is split from NQL and applied as a numeric
  comparison (400 on unsupported operator); default order `created_at desc`;
  `blog::mapping::clean(rows,"comment")` then `core::query::apply` handles filter,
  order, `fields` projection and pagination; total from the filtered store rows.
  `browse/handler.cppm` uses `require_ghost_permission(req,res,…,"comments","browse")`
  and emits no `X-Cache-Invalidate` (`cacheInvalidate:false`).
- GET `/comments/post/:post_id` — `get_detail/workflow/step/execute.cppm`: 9-option
  parse, `post_id` from path/option with 400 validation, impersonation UUID resolved
  against the `members` store (404 if unknown), `comments_enabled` check (405),
  `post_id` AND `parent_id:null` filters combined with the caller filter, real
  pagination via `core::query`/`blog::mapping`. Handler permission guard uses
  method `browse`; no literal invalidation header.
- POST `/comments` — `create/workflow/step/execute.cppm`: `include` allowlist → 400,
  `data.comments[0]` unwrap, `html`/`member_id`/`post_id|parent_id` validation → 400,
  `created_at` validation, reply-parent/sibling checks, `status:published`, store
  insert, `MemberCommentEvent` emitted through `blog::handlers::outbox::global_outbox()`,
  re-fetch, `blog::mapping::wrap` envelope and 201. Handler sets Ghost's three
  member-path `X-Cache-Invalidate` URLs and permission-guards with `add`.
- PUT `/comments/:id` — `edit/workflow/step/execute.cppm`: id required (400), only
  `status`/`pinned` read, pinned boolean/pin-reply/pin-deleted checks, `pinned_at`
  rules, store edit, re-fetch, `blog::mapping` output with 200/404. Handler sets
  the member-path `X-Cache-Invalidate` header and permission-guards with `edit`.
- Residuals (no allowed C++ primitive or shared module not in this folder): router
  `authAdminApi` sub-middleware, impersonation's member-session side effects,
  `withRelated` relations/`count.*` values, Knex transactions/`FOR UPDATE`,
  Tinybird/SMTP notifications, and fabricated wire fields/timestamps in
  `types/model_comment.cppm` (shared with the other comment controllers).

## Module-level notes

- Scope: exactly four Admin routes bind `endpoints/comments.js` (`api/endpoints/index.js:255-257`, `admin/routes.js:50,56,57,58`). The members app (`web/comments/routes.js:67-90`) binds `api.commentsMembers` (`api/endpoints/index.js:331-332` → `./comments-members.js`) and does **not** bind this controller; that controller is covered by `comments-members.md`.
- Shared Ghost service: `commentsService` = `require('../../services/comments').default` (`comments.js:1`); `defineService('Comments')` returns `{api,controller}` (`services/comments/index.ts:25-68`). All four routes enter `CommentsController` (`services/comments/comments-controller.js:18-26`): browseAll → `adminBrowseAll`, browse → `adminBrowse`, edit → `adminEdit`; `add` calls `CommentsService.api.commentOnPost/replyToComment` directly (`comments.js:127-142`) and reuses `CommentsController.setCacheInvalidationHeaders` (`:144`, implementation `comments-controller.js:211-225`).
- Side effects: only `POST /comments` performs a domain-event dispatch (`comments-service.js:839-845,919-925`); the SMTP notification path exists (`:456-473`) but is suppressed for this admin route via `context.internal=true` (`comments.js:121-125`, checks at `comments-service.js:835-837,915-917`). No webhooks, jobs, third-party API calls, or SMTP mail (suppressed by internal context). C++ performs DB insert/update via `ctx.store().create/edit` (`context_context.cppm:1235-1354`) and emits the `MemberCommentEvent` through `blog::handlers::outbox::global_outbox()` on `POST /comments` (no mail/jobs/webhooks).
- C++ middleware chain for all four routes: `content_type_middleware` + `require_auth` on the `/comments` router (`discovery/comments/routes.cppm:19-21`; `middleware/auth.cppm:12-29`; `middleware/serde.cppm:90-119`), plus per-handler `blog::handlers::middleware::require_ghost_permission` with docName `comments` and methods `browse`/`add`/`edit` (`middleware/ghost_permission.cppm:120-134`, backed by `core::auth::ghost::GhostAuthorizer`). A valid admin bearer is mapped to the Administrator principal when no explicit `x-blog-*` headers are present; explicit headers are never overwritten. No member-session middleware.
- C++ execution pattern: each handler pins a static `BlogContext*` and runs a `LocalWorkflowDriver` workflow with one `ExecuteStep` (`browse/handler.cppm:15-18`, `create/handler.cppm:15-18`, `edit/handler.cppm:15-18`, `get_detail/handler.cppm:15-18`; flows `*/workflow/flow/main.cppm:7-10`). Handlers pass an `Output` map (`status`,`body`) to `res.set_status`/`set_body` and call `send()` inline.
- Caching: Ghost uses only the framework `cacheInvalidate:false` directive on all four entries (`comments.js:37-39,54-56,81-83,95-97`) and explicit `X-Cache-Invalidate` on add/edit (`comments-controller.js:211-225`); no `core::cache::Cache` key/tag/TTL equivalent. C++ omits `X-Cache-Invalidate` on the two reads (matching `cacheInvalidate:false`) and writes Ghost's member paths (`/api/members/comments/post/<postId>/, /api/members/comments/<parentId>/replies/, /api/members/comments/<id>/`) on add/edit; no `core::cache::Cache` key/tag/TTL is used.
- Cross-cutting residual gaps (no allowed C++ primitive): router-level `authAdminApi` sub-middleware and `tokenPermissionCheck` claims decoding (`admin/middleware.js:110-118`), member-session side effects of impersonation (`comments-controller.js:57-73`, `comments-service.js:1017-1027`), content gating/member products/post tiers (`comments-service.js:183-199,795-819,861-893`), real Knex transactions and `FOR UPDATE` (`comments-service.js:201-212,236-238`), Tinybird/SMTP notifications (`:456-473`), real `withRelated` relations and `count.*` values, and NQL/`count.reports` raw SQL (partially modeled by `core::query` + the numeric report-count filter).
- Not EXTRA: the remaining C++ comment routes (`routes.cppm:38-48,67-165` — counts, likes/dislikes/undislike, report, replies, likes/dislikes/reports, `GET /comments/:id`, `DELETE /comments/:id`) have Ghost counterparts in `comments-members.js`, `comment-replies.js`, `comment-likes.js`, `comment-dislikes.js`, `comment-reports.js` (`api/endpoints/index.js:259-273,331-332`) — they are not extras of `comments.js`.
