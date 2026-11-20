# Behavior report: settings-public

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/settings-public.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js` (content API router, `express.Router('content api')` at `:12`, router-level CORS at `:14`, settings binding at `:37`)
- C++ folders: `include/blog/handlers/system/settings/` (matched route `GET /settings` is the admin-settings browse handler; no dedicated public/content-settings handler exists)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 0 |
| MISMATCH | 1 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/ghost/api/content/settings` (route-relative `/settings`) | `browse`, docName `settings` | No `options[]`/`data[]`/`validation{}`; no body, multipart, path params, or query aliases; reads no headers in controller | `mw.authenticatePublic` (content API key + rate limit); `permissions:true` short-circuits on public context | 200; no `X-Cache-Invalidate` (`cacheInvalidate:false`); `Cache-Control: public, max-age=<caching:contentAPI:maxAge>`; CORS | `query` → `settingsCache.getPublic()` (public allowlist) + `urlUtils.urlFor('home',true)` + `ghostVersion.safe` + `labs.getAll()` → content serializer `serializeSettings` → `mappers.settings` | In-memory `settingsCache` source; no endpoint key/TTL; response marked public-cacheable | `{settings:{<allowlist>,url,version,labs}, meta:{}}`; object, not array; no pagination | `system/settings/routes.cppm`; `system/settings/browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`; `context/context_context.cppm` | Same path+verb but admin browse semantics: `require_auth` bearer (401), JSON `Content-Type`/`Accept` gates (415/406), hardcoded 21-pair `{key,value}` fixture array, no `url`/`version`/`labs`, no CORS/public cache header | MISMATCH | Ghost `settings-public.js:1-27`, `content/routes.js:37`, `content/middleware.js:18-25`, `output/settings.js:32-68`; C++ `settings/routes.cppm:19-48`, `browse/handler.cppm:17-23`, `browse/.../execute.cppm:12-25`, `context_context.cppm:1161,1682-1716` |

## Route details

### GET `/ghost/api/content/settings` (route-relative `/settings`) — MISMATCH

**Ghost** (facts 1–8):

1. **Identity.** `router.get('/settings', mw.authenticatePublic, http(api.publicSettings.browse));` — `docs/Ghost/ghost/core/server/web/api/endpoints/content/routes.js:37`. Router named `content api` at `:12`; router-level `router.use(cors({maxAge: config.get('caching:cors:maxAge')}))` at `:14`. Mount chain: `backendApp.lazyUse(BASE_API_PATH, require('../api'))` (`web/parent/backend.js:15`; `BASE_API_PATH='/ghost/api'` at `core/shared/url-utils.ts:4`) → `apiApp.lazyUse('/content/', require('./endpoints/content/app'))` (`web/api/app.js:21`) → content app bodyParser.json 50mb (`:21`), `parseQueryBooleans` (`:24`), `cacheControl('public',{maxAge:caching:contentAPI:maxAge})` (`:27-31`), then `routes()` (`:34`). Full path `GET /ghost/api/content/settings`. `api.publicSettings` = `apiFramework.pipeline(require('./settings-public'), localUtils, 'content')` (`api/endpoints/index.js:307-309`). Route-level `mw.authenticatePublic` = `[rateLimiting.contentApiKey, authenticateContentApi, authorizeContentApi, cors(), urlRedirects.adminSSLAndHostRedirect, prettyUrls]` (`content/middleware.js:18-25`). Wrapped by `http()` (`packages/api-framework/src/http.ts:51-63`).

2. **Operation.** Controller exports `docName:'settings'` (`settings-public.js:8`) and a single `browse` op implemented by `query()` (`:10`,`:15-23`). No add/edit/destroy/read, no custom action.

3. **Input.** No `options[]`, no `data[]`, no `validation{}`, no `data.<resource>[0]` body shape, no multipart fields, no path params, no query aliases (`filter[x]=y` not used), no headers read by the controller. Framework still populates `frame.body/file/files/query/params/url/context` from the request (`http.ts:86-105`).

4. **Auth/permissions.** Content API key auth via `authenticatePublic` (`content/middleware.js:18-25`; includes content API key rate limiting at `:19`). Controller declares `permissions:true` (`settings-public.js:14`); the public context short-circuits the permission check (`api/endpoints/utils/permissions.js:100-104`), so no `permissions.canThis(...)` service call runs. No `unsafeAttrs`, no labs gate. `query()` invokes no permission service — it is a pure synchronous read of `settingsCache`/`labs`/`urlUtils`/version.

5. **Status + headers.** No `statusCode` declared ⇒ default 200 (`http.ts:126-133`). `headers:{cacheInvalidate:false}` (`settings-public.js:11-13`) ⇒ no `X-Cache-Invalidate` header (framework only emits it when truthy: `packages/api-framework/src/headers.ts:135-141`, generator at `:21-27`). Content API app adds `Cache-Control: public, max-age=<caching:contentAPI:maxAge>` (`content/app.js:27-31`); router + auth each add CORS (`content/routes.js:14`, `content/middleware.js:22`). JSON via `res.json(result||{})` (`http.ts:139-147`); no stream, `Content-Disposition`, or `Location`.

6. **Full call chain.** `http()` builds `Frame` and runs pipeline (`http.ts:86-118`; validators → input serializers → permissions → query). `query(frame)` (`settings-public.js:15-23`) → `settingsCache.getPublic()` (`shared/settings-cache/index.js:1-6` → `shared/settings-cache/cache-manager.js:240-261`; allowlist `shared/settings-cache/public.js:1-68`, incl. computed `transistor_portal_enabled` at `cache-manager.js:252-258`) → `urlUtils.urlFor('home',true)` (`settings-public.js:19`; `core/shared/url-utils.ts:1-21`) → `ghostVersion.safe` (`settings-public.js:4,20`, `@tryghost/version`) → `labs.getAll()` (`settings-public.js:2,21`; `shared/labs.js:67-90` — GA flags forced true at `:70-72`, remote overrides `:77-80`, config overrides `:82-85`, computed `members` at `:87`) → `Object.assign({}, public, {url,version,labs})` (`:18-22`). Output serializer `serializeSettings` (`utils/serializers/output/settings.js:32-68`): `utils.isContentAPI(frame)` true (`:36`) so `filteredSettings=models`, `icon` URL rewritten `/content/images/` → `/content/images/size/w256h256/` (`:40-46`), then `frame.response={settings:mappers.settings(filteredSettings),meta:models.meta??{}}` (`:58-61`); `mappers.settings` returns the object unchanged when not an array (`output/mappers/settings.ts:3-11`). No DB/Knex transaction, no events, no jobs, no mail/outbox, no webhook, no third-party HTTP. Error branches: 401/403 from content API key auth and 500 via `next(err)` (`http.ts:169-176`); this controller raises no NotFoundError/ValidationError/BadRequestError itself.

7. **Cache.** Source is the in-memory `settingsCache` (CacheManager keyed by `setting.key`, `cache-manager.js:76-102`, populated on boot `init()` `:275-302`, kept fresh via `settings.edited`/`added`/`deleted` events `:289-291`); `labs.getAll()` also reads it (`labs.js:68`). No endpoint-specific cache key/tag/TTL and no invalidation header (`cacheInvalidate:false`, `settings-public.js:12`); response is explicitly public-cacheable via `Cache-Control` (`content/app.js:27-31`).

8. **Output.** Envelope `{settings:{title,description,logo,icon,accent_color,cover_image,facebook,twitter,threads,bluesky,mastodon,tiktok,youtube,instagram,linkedin,lang,locale,timezone,codeinjection_head,codeinjection_foot,navigation,secondary_navigation,meta_*,og_*,twitter_*,members_*,portal_*,comments_enabled,recommendations_enabled,outbound_link_tagging,default_email_address,support_email_address,labs,transistor_portal_*,site_uuid}, url, version, labs}` (`public.js:6-67` + `settings-public.js:18-22`) plus `meta:{}` (`output/settings.js:58-61`). `settings` is a **single object** (mapper passes non-array through, `mappers/settings.ts:4-10`), not an array. `url` = absolute home URL, `version` = safe Ghost version, `labs` = computed flag object. No `meta.pagination` (no browse paging on this controller). Missing values resolve to `null` via `?? null` (`cache-manager.js:248-250`); there is no empty-list case.

**C++** (facts 1–8):

1. **Identity.** `Router{router, "/settings"}.use(settings_content_type).use(blog::handlers::middleware::require_auth).get(lambda → handler.settings_browse().handle(...))` — `include/blog/handlers/system/settings/routes.cppm:39-48`, APIDoc summary "Ghost settings GET" at `:49`. `settings_content_type` (`routes.cppm:19-32`) bypasses the JSON gate only for paths starting `/settings/routes/yaml` (`:27`), so `GET /settings` passes through `content_type_middleware` (`:31`). Installed via `SettingsRoutes{}.install(router, handlers)` (`include/blog/handlers/routes.cppm:120`); handler accessor/member wired at `all.cppm:396,677`. Registered path is `/settings` with no `/ghost/api/content` prefix — C++ drops Ghost API mount prefixes (see sibling parity reports).

2. **Operation.** `BrowseHandler` (`system/settings/browse/handler.cppm:10-25`); workflow `settings_browse` (`browse/workflow/flow/main.cppm:7-10`); step `settings_browse_execute` (`browse/workflow/step/execute.cppm:11-12`). "settings" appears only as the store resource string (`execute.cppm:23`), not as a Ghost-style docName.

3. **Input.** `using Input = interfaces::io::IRequest&` (`execute.cppm:10`). Query string is parsed manually into `std::map<std::string,std::string> params` from `in.get_path()` (`:14-22`), but `params` is discarded: `json_for("settings", params)` dispatches to `settings_json()` which takes no params (`context/context_context.cppm:1152-1162,1682`). No body/data/validation/multipart/path-param handling.

4. **Auth/permissions.** `content_type_middleware` (`middleware/serde.cppm:90-119`) rejects missing/non-JSON `Content-Type` → 415 (`:97-108`) and missing/non-JSON/non-`*/*` `Accept` → 406 (`:110-119`). `require_auth` (`middleware/auth.cppm:12-29`) returns 401 `{"errors":[{"message":"Unauthorized","type":"UnauthorizedError"}]}` when `token(req)` is empty (`bearer.cppm:27-33`; token from `Authorization`/`x-blog-token`, `bearer.cppm:8,18-22`). No content API key path, no rate limiting, no public/member auth, no permission service.

5. **Status/headers.** `res.set_status(Status::OK); res.set_body(bytes); send();` (`browse/handler.cppm:22`). No `X-Cache-Invalidate` is set in the browse folder (contrast sibling handlers `settings_verifications_put/handler.cppm:25`, `settings_access_code_regenerate_post/handler.cppm:22`, `bulk_edit/handler.cppm:25`). No `Cache-Control`/CORS headers set here.

6. **Call chain.** `handle()` (`browse/handler.cppm:17-23`) → `m_workflow.run("settings_browse", req)` (`:18`) → `ex.result_of("settings_browse_execute")` (`:19`) → `std::any_cast<std::string>` of `typed_output` or `"{}"` fallback (`:20`) → bytes; step `invoke()` (`execute.cppm:12-25`) → `ctx.store().json_for("settings", params)` (`:23`) → `settings_json()` (`context_context.cppm:1682-1716`) → wraps result as `{"settings": <rows>}` (`execute.cppm:24`). No DB read, no events, no jobs, no outbox.

7. **Cache.** No `core::cache::Cache` usage in this handler (the store owns an unused `m_cache`, `context_context.cppm:1719-1726`). No key, tag, TTL, or cache header.

8. **Output.** `{"settings":[{key,value},…]}` — a **hardcoded array** of 21 fixture pairs (`context_context.cppm:1684-1715`: `members_track_sources=true`, `title=Ghost`, `icon=""`, `accent_color=#FF1A75`, `navigation="[]"`, etc.). No `url`, `version`, `labs`, no `meta`, no icon resize, no public allowlist. Error branches limited to 401/415/406; no 404/422/500 paths defined for this route.

**Diff** — residual gaps (Ghost behaviors with no C++ primitive):

- Live allowed settings cache, event-refreshed on boot and edits (`cache-manager.js:240-261,275-302`; `public.js:1-68`) vs a hardcoded fixture (`context_context.cppm:1685-1715`).
- `url = urlUtils.urlFor('home', true)` (`settings-public.js:19`) — not produced on any settings route.
- `version = ghostVersion.safe` (`settings-public.js:4,20`) — present in other C++ DTOs (`system/site/types/dto_site_response.cppm:73`, `system/config/browse/.../execute.cppm:62-63`) but not in settings output.
- `labs.getAll()` with GA/remote/config overlays and computed `members` (`labs.js:67-90`) — absent.
- Content API key auth + rate limiting + CORS + `prettyUrls` (`content/middleware.js:18-25`, `content/routes.js:14`) — no equivalent; bearer-only `require_auth`.
- `Cache-Control: public, max-age=…` (`content/app.js:27-31`) — absent.
- Content API output serializer and icon resize (`output/settings.js:32-46`, `mappers/settings.ts:3-11`) — absent.

Diff — divergences (C++ differs from Ghost):

- **Output shape**: array of `{key,value}` pairs (`context_context.cppm:1708-1715`) vs Ghost's single object (`mappers/settings.ts:4-10`).
- **Fixture content**: includes admin-only `members_track_sources` (`context_context.cppm:1686`) and omits most public allowlist keys (e.g. `site_uuid`, `portal_*`, `members_enabled`, `comments_enabled`; compare `public.js:6-67` vs `context_context.cppm:1685-1707`).
- **Auth posture**: bearer token mandatory (401 when absent, `auth.cppm:19-27`) while Ghost public settings is readable with a Content API key / public context (`permissions.js:100-104`).
- **Extra GET gates**: JSON `Content-Type` + `Accept` required (415/406, `serde.cppm:97-119`); Ghost imposes no such requirement on this GET.
- One `/settings` route serves admin-browse semantics; there is no dedicated public/content settings handler.

## Module-level notes

- One Ghost controller route (`settings-public.browse`) exists; exactly one C++ route matches by path+verb (`GET /settings`, `system/settings/routes.cppm:42-48`). It is the **Admin API** settings browse handler, so the match is structural only — hence MISMATCH, not MISSING.
- C++ combines Admin and Content API settings routes on one router with no mount-prefix distinction (`handlers/routes.cppm:120`; `system/settings/routes.cppm:39-118`). Sibling C++ routes under this router (`PUT /settings`, `/access_code/regenerate`, `/routes/yaml`, `/stripe/connect`, `/verifications`) correspond to the separate Admin `settings.js` controller and are out of scope for this report; they are not counted as EXTRA here.
- The C++ query string is parsed then ignored (`browse/.../execute.cppm:14-23`), and `json_for("settings", …)` deliberately discards params (`context_context.cppm:1161,1682`) — a fixture fallback pattern seen across the C++ handlers.
- No `X-Cache-Invalidate` on this C++ route matches Ghost's `cacheInvalidate:false` by coincidence of omission, not by modeled behavior; the public `Cache-Control` and CORS headers are still unimplemented.

## Group 56 — strict-parity implementation record (2026-11-20)

The browse step now emits the flat public-config object when the request carries
`x-blog-context: CONTENT_API` (`support::public_browse()` in the registered
`:settings_browse_step` partition): public allowlist keys + `url`, `version`,
`labs`, no `settings` array and no pagination meta. Admin requests keep the
`{"settings":[...]}` projection, so the single shared `GET /settings` route now
serves both controllers by content negotiation. `permissions:true` is enforced
through `core_auth:ghost` via the registered middleware, and the dead
`&& !true` guard is gone.

Verdict stays **MISMATCH**: the shared router has no `authenticatePublic` /
Content-API-key resolver, so a public key presented without the context header
still receives the admin projection. Full closure needs a central route split or
`public_settings_json()` in `context/context_context.cppm`, which is owned by the
main agent. New-partition needs (canonical public serializer in
`blog_handlers:mapping`, shared public allowlist and authenticatePublic
middleware) are recorded, not created; helpers remain in the existing registered
partition. No shared file, BUILD or cycle fix was attempted in this folder.
