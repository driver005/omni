# Behavior report: email-post

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/email-post.js` (server/API endpoint controller; `docName: 'email_post'`, exports `read`)
- Ghost route table: not `admin/routes.js` — the binding is the **frontend site router** `docs/Ghost/ghost/core/frontend/services/routing/email-router.js` (`GET /email/:uuid/:options?`), dispatched through the frontend proxy (`services/proxy.js:117`) into the server endpoint pipeline (`core/server/api/endpoints/index.js:191-193`). No Admin API URL-table route references `emailPost`.
- C++ folders: none (no `email_post`/`email-post` handler, route or accessor under `include/blog/handlers`)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 1 |
| EXTRA | 0 |

The controller is **not** orphaned. It is registered as the `emailPost` API endpoint (`index.js:191-193`) and its `read` operation is genuinely bound to an HTTP route: the hardcoded frontend route `GET /email/:uuid/:options?` (`email-router.js:15,29`). The frontend controller resolves `res.routerOptions.query.controller === 'emailPost'` and calls `api.emailPost.read(params)` (`controllers/email-post.js:17,27-28`), where `api` is the server endpoint collection (`proxy.js:117`). There is no C++ counterpart, so the one Ghost route is `MISSING`.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `GET /email/:uuid/:options?` (frontend site router, hardcoded `/email/`) | `read` under `docName:'email_post'` | path `:uuid` (required), optional `:options`; frontend builds `include:'authors,tags,tiers'`, `context.member`; API declares `options:['include']`, `data:['uuid']`, validation `uuid.required`, `include∈{authors,tags,tiers}` | `permissions:true` on `docName:'email_post'`; no explicit auth middleware on the route; `cacheInvalidate:false` | 302 → admin editor when `:options==='edit'` (if redirects enabled); 301 → canonical post URL when `status==='published'`; else render; `next()` on missing post/unknown option; `NotFoundError('Post not found.')` on API miss; `renderer.handleError` | `api.emailPost.read` → `forceUrlRelations(frame,'posts')` → `models.Post.findOne({...data,status:'sent'}, options)` → serializer `email_post` (`email-posts.js`): tiers `browse` → `mappers.posts` → `gating.forPost` → `{email_posts:[post]}` → frontend reads `result.email_posts[0]` | `headers.cacheInvalidate:false` → no invalidation; no cache key/TTL in controller; tiers `browse` may cache inside the tiers service | `{email_posts:[post]}` envelope; post fields from `mappers.posts` + computed tiers + gating; empty/miss → `next()` or 404 via renderer | none | not implemented (no `/email/:uuid` route, no `email_post` handler/store access) | MISSING | Ghost: `email-router.js:15,26,29`; `router-manager.js:109-113`; `controllers/email-post.js:17,27-30,36-63`; `config.ts:39-45`; `index.js:191-193`; `email-post.js:13-49`; `output/index.js:131-133`; `email-posts.js:6-35`. C++ absence: no `email_post` dir under `include/blog/handlers`; `routes.cppm`/`all.cppm` no `email_post`; `route_catalog.cppm`/`snapshot_cases.cppm` only `/email_previews/*`, `/emails/*`, `/users/email/:email`. |

## Route details

### GET /email/:uuid/:options? — MISSING

**Ghost**:

1. **Identity**: Hardcoded route value `/email/` (`email-router.js:15`); registered as `urlJoin(this.route.value, ':uuid', ':options?')` → `/email/:uuid/:options?` (`email-router.js:29`). Mounted only when `RESOURCE_CONFIG.QUERY.email` exists (`router-manager.js:109-113`) via `siteRouter.mountRouter(emailRouter.router())` (`router-manager.js:111`). Middleware: `router().use(this._prepareContext)` (`email-router.js:26`) sets `res.routerOptions = {type:'entry', query: RESOURCE_CONFIG.QUERY.email, context:['emailPost']}` (`email-router.js:39-44`). Router-level mount is the site router; no `mw.authAdminApi` on this frontend route. `robots.txt` disallows `/email/` (`core/frontend/public/robots.txt:4`).
2. **Operation**: `res.routerOptions.query` = `QUERY.email` with `controller:'emailPost'`, `resource:'email_posts'` (`config.ts:39-45`). `controllers.email` → `require('./email-post')` (`controllers/index.js:18-20`). It calls `api[res.routerOptions.query.controller].read(params)` = `api.emailPost.read(params)` (`controllers/email-post.js:17,27-28`). `api` = server endpoint collection (`proxy.js:117`), `emailPost` = `apiFramework.pipeline(require('./email-post'), localUtils)` (`index.js:191-193`). Controller exports `read` with `docName:'email_post'` (`email-post.js:13,15`).
3. **Input**: frontend params `{uuid: req.params.uuid, include:'authors,tags,tiers', context:{member:res.locals.member}}` (`controllers/email-post.js:19-25`). API declaration: `options:['include']`, `data:['uuid']` (`email-post.js:20-21`); validation `options.include.values=['authors','tags','tiers']`, `data.uuid.required=true` (`email-post.js:22-33`). `:options` path param optional; `:options==='edit'` triggers admin redirect when `config.get('admin:redirects')` true (`controllers/email-post.js:36-44`), other option values → `next()` (`controllers/email-post.js:45-48`). No request body; no direct header reads in the controller.
4. **Auth/permissions**: `permissions:true` on `docName:'email_post'` (`email-post.js:13,19`); permission enforcement happens inside the api-framework pipeline, not via a route middleware. Member context forwarded through `context.member` (`controllers/email-post.js:22-24`).
5. **Status + headers**: `headers.cacheInvalidate:false` (`email-post.js:16-18`) → no `X-Cache-Invalidate`. Frontend writes: 302 to `/#/editor/post/${post.id}` for `edit` option (`controllers/email-post.js:44`), 301 to canonical resource URL when `post.status==='published'` (`controllers/email-post.js:50-58`), otherwise `renderer.renderEntry(req,res)(post)` (`controllers/email-post.js:61`). API miss → `NotFoundError({message:tpl(messages.postNotFound)})` = `'Post not found.'` (`email-post.js:42-46`; `messages:8`) → `renderer.handleError(next)` (`controllers/email-post.js:63`). Missing `result.email_posts[0]` → `next()` (`controllers/email-post.js:30-34`).
6. **Full call chain**: `emailPost.read` pipeline → `read.query(frame)` (`email-post.js:34`): `urlSerializerUtils.forceUrlRelations(frame,'posts')` (`email-post.js:35`; `url.js:116-141` adds required URL relations/columns to `frame.options.withRelated`) → `models.Post.findOne(Object.assign(frame.data,{status:'sent'}), frame.options)` (`email-post.js:37-40`) — forces `status:'sent'` so only sent email posts resolve → NotFoundError on no model → returns model. Output serializer `email_post` (`output/index.js:131-133`): `tiersService.service.api.browse({})` side effect (`email-posts.js:7`), maps tiers to `{id,name,slug,active,welcome_page_url,visibility,trial_days,description,type,currency(lowercased for paid),monthly_price,yearly_price,created_at,updated_at,monthly_price_id:null,yearly_price_id:null}` (`email-posts.js:8-29`), `mappers.posts(model, frame, {tiers})` (`email-posts.js:30`), `gating.forPost(emailPost, frame)` (`email-posts.js:31`), `frame.response={email_posts:[emailPost]}` (`email-posts.js:33-35`). Frontend reads `result[res.routerOptions.query.resource][0]` = `result.email_posts[0]` (`controllers/email-post.js:30`).
7. **Cache**: `cacheInvalidate:false` (`email-post.js:16-18`) — no invalidation URL/header, no TTL/key declared in controller. Tiers `browse` (`email-posts.js:7`) may cache internally in the tiers service; outside this controller.
8. **Output**: serializer `email_post` → `email_posts` envelope with one post (`email-posts.js:33-35`); field set = `mappers.posts` output plus computed `tiers` and gating transforms (`email-posts.js:30-31`). Empty/missing: `undefined` post → `next()`; thrown NotFoundError → renderer error handling.

**C++**:

- No `email_post`/`email-post` handler directory under `include/blog/handlers` (glob `**/email_post*/**` and `**/*email*post*` → no files).
- `include/blog/handlers/routes.cppm` and `include/blog/handlers/all.cppm`: no `email_post`/`email-post`/`emailPost` import or accessor (only `emails_*`, `email_previews_*`, etc. — different controllers).
- `include/blog/testing/route_catalog.cppm`: only `GET /email_previews/posts/:id`, `/emails`, `/emails/:id`, `/emails/:id/status`, `/emails/:id/batches`, `/emails/:id/recipient-failures`, `/emails/:id/analytics`, and `/users/email/:email` — no `GET /email/:uuid`. `snapshot_cases.cppm` likewise has no `/email/` frontend route.
- No `/email/` frontend site router, no `res.routerOptions`-style query indirection, no `email_posts` resource store, no tiers-browse + post-gating serializer, no `status:'sent'` forced lookup, no 301/302 redirect semantics for this route.
- C++ implements the Admin API surface only; this Ghost route is a frontend/public rendered-pages route, so nothing is implemented.

**Diff**:

- Residual gap (Ghost-only): the entire frontend `/email/:uuid` read path — `forceUrlRelations`, `Post.findOne` with forced `status:'sent'`, the `email_post` serializer (tiers `browse`, `mappers.posts`, `gating.forPost`), and the api-framework `permissions:true` check — has no C++ primitive.
- Residual gap: 301/302 redirect behavior and `renderer.renderEntry` page rendering are frontend concerns absent from C++.
- No divergences: no C++ route exists to differ from Ghost. Verdict `MISSING` (Ghost frontend route, zero C++ counterpart). No `EXTRA` route to report.

## Module-level notes

- Binding nuance: the controller has **no Admin API URL-table route**. `grep` of `emailPost|email_post|email-post|uuid|sent` in `core/server/web/api/endpoints/admin/routes.js` → 0 matches; `grep` of `emailPost\.` across `core/server/web` → 0 matches. The only HTTP binding is the frontend site router `/email/:uuid/:options?`, which reaches the server controller indirectly via `config.ts:39-45` → `controllers/email-post.js:27-28` → `proxy.js:117` → `index.js:191-193`. A prior parity note (`docs/parity/email-post.md`) concluded the controller was orphaned; that conclusion is incomplete because it did not trace the frontend proxy/router path.
- Single binding: `controller:'emailPost'` appears once in `config.ts:40`; no other route sets `query.controller` to `emailPost`.
- Registrations: output serializer keyed `email_post` (`output/index.js:131-133`) matches `docName`/resource `email_posts`; frontend reads `result.email_posts[0]`, consistent with the serializer envelope.
- Shared helpers used by this route: `forceUrlRelations` (`utils/serializers/input/utils/url.js:116-141`), `mappers.posts`, `gating.forPost` (`email-posts.js:1-2,30-31`), `tiersService.service.api.browse` (`email-posts.js:3,7`), `urlUtils.redirectToAdmin`/`redirect301` and `routerManager.getUrlForResource` (`controllers/email-post.js:4,44,55-57`), `renderer.renderEntry`/`handleError` (`controllers/email-post.js:5,61,63`).
- Cross-route side effects: the serializer calls `tiersService.service.api.browse({})` on every read (`email-posts.js:7`) — a tiers-service fetch with possible internal caching; no mail/outbox writes, webhooks or third-party HTTP calls in this controller.
- No C++ handler, route, store access, cache marker or fixture write exists for this domain, so there is nothing to add or remove on the C++ side for parity of this module.
