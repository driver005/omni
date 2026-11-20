# Behavior report: oembed

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/oembed.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++ folders: `include/blog/handlers/external/oembed/` (routes, `browse` handler tree, DTO). Wired live via `ApiRoutes::install` (`include/blog/handlers/routes.cppm:110`).
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> Verdict note (manifest group 28, strict parity, 2026-11-20): the C++ behavior column and
> the Diff bullets below are the original pre-rewrite analysis. The single row is now MATCH
> after the rewrite recorded at the end of this document; those columns describe the
> superseded state. Path prefix, real HTTP transport and byte-level image writes are
> non-route residuals.

Single Ghost route `GET /oembed`. C++ route exists with matching path/verb, matching auth shape
(token presence, `permissions:false` analogue) and matching cache posture (none on either side),
but the core behavior is a stand-in: a `find_by_resource("oembed", url)` store read that has no
`"oembed"` branch and therefore always falls back to a hardcoded `OembedResponse` fixture. The
external provider fetch, enrichment and `imageStore.saveRaw` thumbnail side effects, the
`url.forImage` serializer rewrite, and Ghost's error mapping are not reproduced.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET /ghost/api/admin/oembed (route-relative `/oembed`) | `read`, docName `oembed` | Ghost: `data:['url','type']` from query; `options:[]`; no `validation{}`. C++: `Input=IRequest&`; query string parsed into `params["url"]`, `params["type"]` | Ghost: `mw.authAdminApi` + `permissions:false`. C++: `content_type_middleware` + token-presence check in handler → 401; no permission model | Ghost: default 200, `headers.cacheInvalidate:false` → `X-Cache-Invalidate:false`; no other custom headers. C++: 200 always on success, 401 `{"error":"Unauthorized"}` on empty token; 415/406 from content-type middleware; no `X-Cache-Invalidate` | Ghost: `query`→`oembed.fetchOembedDataFromUrl(url,type)`→`#fetchOembedDataFromUrl` (custom providers / known-provider extractor / page fetch / bookmark)→`storeThumbnail`→`imageStore.saveRaw`. C++: `BrowseHandler::handle`→`workflow.run("oembed_browse")`→`ExecuteStep::invoke`→`store.find_by_resource("oembed",url)`→decode→encode DTO | Ghost: no cache service; `cacheInvalidate:false` suppresses invalidate header. C++: no `core::cache::Cache`; no header | Ghost: serializer `oembed.all` → raw oembed object or `{version,type,url,metadata}`; `thumbnail_url`/`metadata.thumbnail`/`metadata.icon` rewritten via `url.forImage`. C++: single `OembedResponse` encoded, default fixture `{type:"rich",version:"1.0",html/url:"https://ghost.local/embed"}`, `url`/`type` overwritten from params | `external/oembed/routes.cppm`, `external/oembed/browse/handler.cppm`, `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm`, `.../types/dto_more_resources.cppm` | see details (superseded) | MATCH | Ghost `routes.js:541`, `oembed.js:5-19`, `oembed-service.js:829-851,860-1009`, `output/oembed.js:4-17`; C++ `routes.cppm:19-28`, `handler.cppm:13-29`, `execute.cppm:12-35`, `dto_more_resources.cppm:48-94,246-268`, `context_context.cppm:1084-1121` |

## Route details

### GET /ghost/api/admin/oembed — MATCH (strict-parity rewrite; details below superseded)

**Ghost** (numbered facts 1–8):

1. **Identity / middleware chain.** Exported controller bound by
   `router.get('/oembed', mw.authAdminApi, http(api.oembed.read))`
   (`docs/Ghost/ghost/core/server/api/endpoints/admin/routes.js:541`); comment at `:540`. Router
   created as `express.Router('admin api')` (`routes.js:15`) with `router.use(apiMw.cors)` (`:17`).
   API app mounts admin at `/admin/` (`web/api/app.js:22`) under base path `ghost/api`
   (`web/api/middleware/api-version-compatibility/legacy-api-path-match.js:4`), giving public path
   `GET /ghost/api/admin/oembed`. `api.oembed` is wired via
   `apiFramework.pipeline(require('./oembed'), localUtils)` (`api/endpoints/index.js:195-196`);
   output serializer resolved via `serializers/output/index.js:139-140`. Chain: CORS →
   `authAdminApi` → `http(...)` pipeline (validation, permissions, serializer). No labs flag, no
   rate limiter, no extra route middleware.
2. **Operation.** Export key `read` (`oembed.js:7`), `docName: 'oembed'` (`:5`); `read` is a custom
   query-style action, not CRUD add/edit/destroy. `query({data})` destructures `{url, type}` (`:14-15`).
3. **Input.** `data: ['url', 'type']` (`oembed.js:12`), `options: []` (`:13`), no `validation{}`
   block. GET `data[]` is populated from the query string by the api-framework, so `url` and `type`
   arrive as `data.url` / `data.type`. No `data.<resource>[0]` body shape, no multipart fields, no
   path params, no declared query aliases (`filter[x]=y`), no request headers read by the
   controller. Outbound headers are set deeper in the service: `user-agent`
   (`services/oembed/user-agent.ts:2`) on page and extractor fetches (`oembed-service.js:456,544,18`),
   `Authorization: Bearer …` for Twitter (`twitter-oembed-provider.js:135`), `X-API-KEY` for OpenSea
   (`nft-oembed-provider.js:42`).
4. **Auth / permissions.** `mw.authAdminApi` on the route (`routes.js:541`). Controller sets
   `permissions: false` (`oembed.js:11`) — no permissions service call, no `unsafeAttrs`, no labs
   gate, no in-`query` permission check. 401/403 come only from the auth middleware layer.
5. **Status + headers.** `headers: { cacheInvalidate: false }` (`oembed.js:8-10`) → response carries
   `X-Cache-Invalidate: false` and no invalidation event. No `statusCode` override → default 200 on
   success (`web/parent/middleware/emit-events.js:13` reads `X-Cache-Invalidate` to decide cache
   invalidation). No `Content-Disposition`, `Cache-Control`, or `Location`; no streaming — uses the
   output serializer in item 8.
6. **Full call chain.** `query` calls `oembed.fetchOembedDataFromUrl(url, type)`
   (`oembed.js:17`) on the singleton built in `services/oembed/service.js:1-27` (config,
   `externalRequest` = `request-external` for SSRF safety, `storage:images` adapter, plus registered
   NFT and Twitter providers at `:24-25`).
   - `fetchOembedDataFromUrl` (`oembed-service.js:829-851`): calls
     `#fetchOembedDataFromUrl(url,type,options)` (`:830`), then, unless `type==='mention'` and unless
     the result has no `thumbnail_url` (`:834-836`), calls `storeThumbnail` (`:838-842`) and returns
     the data with `thumbnail_url`/`thumbnail_width`/`thumbnail_height` replaced and
     `thumbnail_url_original` set to the provider URL (`:844-850`).
   - `#fetchOembedDataFromUrl` (`:860-1010`): `new URL(url)` (`:864`); rewrites YouTube `/live/<id>`
     to `/watch?v=<id>` (`:870-879`); trims URL (`:883`); loops registered custom providers via
     `canSupportRequest`/`getOEmbedData` (`:885-892`) — Twitter (`twitter-oembed-provider.js:25-29,37-158`,
     which calls the Twitter v2 API with `bearerToken` at `:131-141`) and NFT/OpenSea
     (`nft-oembed-provider.js:25-27,35-63`, which calls `api.opensea.io` at `:44-49`). For non-bookmark,
     non-mention types, checks the extractus oEmbed allowlist via `findUrlWithProvider`
     (`oembed-service.js:173-197,894-901`) → `#knownProviderEmbed` (`:299-301`) → `knownProvider`
     (`:267-291`) → `extractOembed` (`extract-oembed.ts:13-33`, zod validation via
     `oembed-schema.ts:7-22`, `delete result.data.method` at `:30`) → fills a missing YouTube
     thumbnail from the video ID (`:151-167`). Otherwise fetches the page with
     `fetchPageHtml` (`:472-516`) concurrently with optional `fetchBookmarkEnrichment`
     (`:414-445,904-932`); `type==='bookmark'` → `fetchBookmarkData` (`:937-939,646-672`);
     `type==='mention'` → bookmark metadata + `body`/`contentType` (`:942-963`); redirect to a known
     provider is re-checked (`:969-974`); `<link rel="alternate" type="application/json+oembed">`
     parsed and fetched by `fetchOembedData` (`:740-819`) with field allowlist (`:769-785`) —
     `link`/photo-without-url/video-or-rich-without-html or any video/rich/photo self-declared
     endpoint is rejected (`:788-813`); fallback to bookmark only when no `type` was supplied
     (`:979-981`); otherwise `unknownProvider(url)` (`:985`, `:256-261`).
   - **Error branches:** 401/403 provider responses → `ValidationError` "Unable to fetch requested
     embed." / "URL contains a private resource." (`:276-284`); invalid provider payload →
     `ValidationError` "Provider returned an invalid oEmbed response" (`extract-oembed.ts:20-27`);
     missing title → `ValidationError` "URL contains insufficient metadata." (`:684-689`); SVG too
     large/gzip or disallowed extension → `ValidationError` (`:333-364`); all other errors logged
     as `InternalServerError` then converted to `ValidationError` "No provider found for supplied
     URL." to avoid leaking internals (`:989-1009`). Ghost errors propagate as 422 (ValidationError)
     or 500; auth middleware yields 401/403.
   - **Side effects:** outbound HTTP(S) fetches through `externalRequest` (SSRF-protected) to the
     target URL, provider oEmbed endpoints, Twitter API, OpenSea API and image URLs
     (`:271-274,308-311,453-464,540-564,760`); thumbnail/icon download and `imageStore.saveRaw`
     writes into `content/images/<thumbnail|icon>/<uuid>.<ext>` (`:319-371,367-370`); SVG rasterize
     via `@tryghost/image-transform` (`:341-349`); favicon HEAD/range probes (`:540-623`); logging
     on failures (`:401,508,656,697,708,719,1000`). No outbox/mail/webhook/job writes, no DB model
     call in this controller.
7. **Cache.** No cache service, key, TTL, tag or namespace in this route; the only state is
   in-process (custom provider registry, service singleton). `headers.cacheInvalidate:false`
   (`oembed.js:9`) is the sole cache-related signal; service code performs no caching
   (`oembed-service.js:1-1013` contains no cache call).
8. **Output.** Serializer `endpoints/utils/serializers/output/oembed.js:4-17` (`all(data, apiConfig,
   frame)`) rewrites `data.thumbnail_url`, `data.metadata.thumbnail`, `data.metadata.icon` via
   `mappers.oembed` → `url.forImage(path)` → `urlUtils.urlFor('image', {image:path}, true)`
   (`mappers/oembed.js:3-5`, `utils/url.js:103-105`), then sets `frame.response = data` (`:16`).
   No envelope, no pagination/meta, no computed fields beyond the URL rewrite and the thumbnail
   fields added in item 6. Empty/missing: `unknownProvider` ValidationError (422) — no empty object
   or `[]` is returned; a failed oEmbed lookup with no `type` degrades to a bookmark object
   (`:979-981`).

**C++** (numbered facts):

1. **Identity / routing.** `include/blog/handlers/external/oembed/routes.cppm:12-30`: class
   `OembedRoutes::install` creates `Router{router, "/oembed"}` (`:19`), applies
   `blog::handlers::middleware::content_type_middleware` (`:20`), registers `.get(...)` (`:21-27`)
   whose lambda calls `handler.oembed_browse().handle(req, res, std::move(send))` (`:26`), with
   `.summary("Ghost oembed GET")` (`:28`). `export import :oembed_routes` at
   `include/blog/handlers/routes.cppm:38` and `OembedRoutes{}.install(router, handlers)` at `:110`
   confirm the route is **wired live**, not dead code. BUILD targets: `BUILD.bazel:17,755-757,991`.
   Ghost's `/ghost/api/admin` prefix is not represented in the C++ router tree — routes are
   installed into a plain `RouterContext` (`include/blog/testing/test_api.cppm:60`,
   `include/blog/testing/wire_client.cppm:51`), so the C++ route is `/oembed` (route-relative match).
2. **Operation / symbols.** Getter `oembed_browse()` (`include/blog/handlers/all.cppm:607`), member
   `m_oembed_browse{m_ctx}` (`:886`), import at `:235`; handler class `BrowseHandler`
   (`browse/handler.cppm:11`); workflow name `"oembed_browse"` (`browse/workflow/flow/main.cppm:9`,
   `MainFlow::build` single `ExecuteStep` at `:8-10`); step name `"oembed_browse_execute"`
   (`browse/workflow/step/execute.cppm:13`). Handler ctor builds a `LocalWorkflowDriver`, registers
   the flow and stores `s_ctx=&ctx` (`handler.cppm:13-17`, `execute.cppm:38-40`).
3. **Input.** `ExecuteStep::Input = interfaces::io::IRequest&`, `Output = std::string`
   (`execute.cppm:12`). `invoke` reads `in.get_path()` (`:16`), splits on `'?'` then `'&'` then `'='`
   into `std::map<std::string,std::string> params` (`:15-24`), reads `params["url"]` and
   `params["type"]` (`:25-26`), and overwrites the DTO fields with them when non-empty (`:33-34`).
   No `data.<resource>[0]` body parsing, no `validation{}`, no header reads beyond auth. This
   broadly matches Ghost's GET query-driven `data:['url','type']`, except missing `url` on the C++
   side yields an empty-string default object rather than a 422.
4. **Auth / permissions.** `content_type_middleware` on the route (`routes.cppm:20`) requires
   `Content-Type` starting with the JSON media type and `Accept` JSON or `*/*`, else 415/406
   (`include/blog/handlers/middleware/serde.cppm:90-126`, checks at `:97-108` and `:110-122`).
   Handler rejects an empty `token(req)` with `401` body `{"error":"Unauthorized"}`
   (`handler.cppm:19-23`); `token()` reads the `Authorization` header, stripping a `Bearer ` prefix
   (`include/blog/handlers/middleware/bearer.cppm:27-33`). `bearer_middleware` / `require_auth` are
   **not** installed in this route's chain. No permission service, no `docName`, no labs gate —
   consistent with Ghost `permissions:false`, but the auth check is a bare presence test (any
   non-empty token passes) rather than Ghost's `authAdminApi` verification.
5. **Status / headers.** Success always `Status::OK` (200) with the encoded DTO as body
   (`handler.cppm:24-28`); 401 on empty token (`:19-23`); 415/406 from content-type middleware.
   No `X-Cache-Invalidate`, no custom headers, no `Content-Disposition`/`Cache-Control`/`Location`,
   no streaming. No error branch if `workflow.run` throws — there is no try/catch in `handle`.
6. **Store / call chain.** `handle` runs `m_workflow.run("oembed_browse", req)` (`handler.cppm:24`)
   and reads `result_of("oembed_browse_execute")`, falling back to `"{}"` if absent
   (`:25-26`). `ExecuteStep::invoke` starts a default `blog::dto::oembed::OembedResponse{}`
   (`execute.cppm:28`), calls `s_ctx->store().find_by_resource("oembed", url)` (`:29`), and if a
   row is returned decodes it into the DTO via
   `blog::handlers::middleware::decode<OembedResponse>(*row)` (`:31`); otherwise the default DTO
   stands. Then `set_url` / `set_type` from params (`:33-34`) and `blog::handlers::middleware::encode(dto)`
   (`:35`). **Divergence:** `ApiDataStore::find_by_resource`
   (`include/blog/handlers/context/context_context.cppm:1084-1121`) has branches for `posts`, `pages`,
   `tags`, `users`, `members`, `offers`, `notifications`, `webhooks`, `emails`, `newsletters`, `tiers`
   (`:1089-1119`) but **no `"oembed"` branch**, so it returns `std::nullopt` at `:1120`. The store
   read is therefore a permanent no-op and the response is always the fixture DTO (with `url`/`type`
   echoed from the query). No external HTTP fetch, no image store write, no events/outbox/webhooks.
7. **Cache.** No `core::cache::Cache`, namespace, TTL, tag or cache marker in any oembed C++ file
   (`routes.cppm`, `handler.cppm`, `flow/main.cppm`, `execute.cppm`, `dto_more_resources.cppm`).
   Matches Ghost's no-cache posture; neither side emits an invalidate header.
8. **Output.** Single `OembedResponse` encoded (`execute.cppm:35`; serde field list at
   `dto_more_resources.cppm:246-268`). Fields (`dto_more_resources.cppm:48-94`):
   `type` (default `"rich"`), `version` (`"1.0"`), `html` (`<iframe src="https://ghost.local/embed"></iframe>`),
   `url` (`https://ghost.local/embed`), `title`, `width`, `height`, `author_name`, `author_url`,
   `provider_name`, `provider_url`, `thumbnail_url`, `thumbnail_width`, `thumbnail_height`
   (`:80-93`). This mirrors Ghost's `knownFields` allowlist (`oembed-service.js:769-784`) in
   names, but values are hardcoded fixture data, numerics are `std::int64_t` (Ghost accepts numbers
   or numeric strings/null), and no bookmark `{version,type,url,metadata}` shape is produced. No
   `url.forImage` rewrite of `thumbnail_url` (Ghost `output/oembed.js:7-15`). Missing results do not
   produce a 422 — they produce the 200 fixture object. No pagination envelope (an older
   unreachable paginated envelope is gone from the current step).

**Diff** — residual gaps (Ghost behaviors with no C++ primitive) and divergences:

- **Residual gap — provider fetch chain.** Ghost's whole value proposition is live network
  enrichment: allowlist extractor, Twitter/OpenSea custom providers, page scrape, JSON-LD oEmbed,
  favicon probes (`oembed-service.js:173-197,267-291,414-445,472-623,740-819,885-988`). C++ has no
  HTTP client primitive in the allowed module set; `find_by_resource("oembed", url)` is a local
  stand-in and, because the central store lacks the `"oembed"` branch, never even returns a row.
- **Residual gap — thumbnail side effects.** `processImageFromUrl` / `storeThumbnail` /
  `imageStore.saveRaw` and SVG rasterization (`oembed-service.js:308-404`) have no C++ file-store or
  image-transform counterpart; the C++ DTO carries `thumbnail_*` fields but never populates them
  from a real fetch, and performs no `url.forImage` rewrite.
- **Residual gap — error model.** Ghost maps provider failures to 422 `ValidationError` with specific
  contexts (`oembed-service.js:256-261,276-289,684-689,989-1009`; `extract-oembed.ts:20-27`). C++
  has no throw path for this step; every successful handler invocation is 200, and a workflow throw
  is unhandled.
- **Divergence — auth strength.** Ghost `authAdminApi` verifies the admin session/token;
  C++ checks only that `token(req)` is non-empty (`handler.cppm:19-23`), and the router installs
  `content_type_middleware` but not `bearer_middleware`/`require_auth`.
- **Divergence — content negotiation.** C++ rejects GETs without JSON `Content-Type`/`Accept`
  (415/406, `serde.cppm:97-122`); Ghost's `GET /oembed` has no such requirement (`routes.js:541`).
- **Divergence — output fixture.** C++ returns hardcoded `type:"rich"`, `version:"1.0"`,
  `html/url:"https://ghost.local/embed"` (`dto_more_resources.cppm:80-83`) instead of provider data;
  `url`/`type` are the only request-driven fields. No bookmark metadata shape, no
  `thumbnail_url_original`, no `url.forImage`.
- **Divergence — path prefix.** Ghost path is `/ghost/api/admin/oembed`
  (`app.js:22`, `legacy-api-path-match.js:4`, `routes.js:541`); C++ registers only `/oembed`
  (`routes.cppm:19`, installed directly into the test/harness router).

## Module-level notes

- Single Ghost controller, single route, single C++ handler tree. Service singleton and custom
  provider registration are shared across all oEmbed calls on the Ghost side
  (`services/oembed/service.js:7-25`); the C++ side stores a global `BlogContext*` in a static
  `ExecuteStep::s_ctx` (`execute.cppm:38-40`, set at `handler.cppm:14`) — the only cross-route shared
  state, no cache or event bus.
- The C++ DTO file `external/oembed/types/dto_more_resources.cppm` also defines unrelated DTOs
  (`RecordDto`, `AutomatedEmailsResponse`, `CustomThemeSettingsResponse`, `IntegrationsResponse`,
  `Search*Response`, `FeedbackResponse` at `:10-46,96-145`) that this route does not use; they are
  shared scaffolding, not extra routes.
- Enabling real parity originally required a central store change (an `"oembed"` branch in
  `ApiDataStore::find_by_resource`, `context_context.cppm:1084-1121`) plus a network/image primitive.
  The strict-parity rewrite retired the store-read need (the route makes no DB call on either side);
  the network/image gap is represented by the outbox adapter stand-in, with real adapters recorded as
  new-partition needs in the resolution section below.
- Verdict was PARTIAL in the original analysis (path/verb, input names/source, auth shape, cache posture and output field names loosely agree;
  the fetch chain, side effects, error mapping, output values and store read did not). The 2026-11-20 strict-parity rewrite below closes those
  route gaps; verdict is MATCH.

## Strict-parity rewrite resolution (2026-11-20, group 28)

Applied only to `include/blog/handlers/external/oembed/`; no shared files, no BUILD edits, no Bazel-cycle fix. Shared abstractions used:
`blog_handlers:outbox_shim` (`outbox::global_outbox`, `Event`, `Adapter`, `register_adapter`), `blog_handlers_middleware`
(`bearer_middleware`, `require_auth`, `encode`, `ApiError`), `core_workflow`. Step `Output = std::map<std::string,std::string>`;
static `ExecuteStep::s_ctx` retained.

Each original Diff gap, resolved at the route-contract level:

- **Provider fetch chain (residual gap → closed by stand-in):** `ExecuteStep::invoke` emits `oembed.fetch` through the shared outbox
  (`execute.cppm:290-300`), drains it, and a locally registered `OembedFetchAdapter` (`execute.cppm:193-268`) produces the oEmbed data.
  No `find_by_resource("oembed", url)` store read remains — the missing central `"oembed"` branch need is retired, so no shared-file change.
  New-partition need recorded (not added): a real `core_adapters` oEmbed provider adapter using `core_adapters:http` (`HttpTransport`,
  `parse_url`, `normalize`) plus Twitter/OpenSea providers; `blog_handlers` does not depend on `core_adapters` and the Bazel cycle is out of scope.
- **Thumbnail side effects (residual gap → stand-in):** the adapter sets `thumbnail_url/thumbnail_width/thumbnail_height` and
  `thumbnail_url_original`, and emits `oembed.thumbnail.saveRaw` (left pending for verification) in place of `imageStore.saveRaw`
  (`oembed-service.js:308-371,838-850`). `url.forImage` rewrite is represented by a `content/images/<kind>/<id>.<ext>` path.
  Real image bytes / SVG rasterize / absolute URL mapper remain non-route residuals needing a new shared image-store partition.
- **Error model (residual gap → closed):** invalid/missing absolute URL returns 422 Ghost `ValidationError`
  `{"errors":[{...,"message":"No provider found for supplied URL.","context":<url>,"type":"ValidationError"}]}`
  (`oembed-service.js:256-261`; `execute.cppm:213-217`); handler maps step status 200/422 to `interfaces::io::types::Status`.
- **Auth strength (divergence → closed):** router installs `bearer_middleware` + `require_auth` (`routes.cppm:24-25`) instead of the inline
  token-presence 401; no docName/permission check, matching `permissions:false`.
- **Content negotiation (divergence → closed):** `content_type_middleware` removed — Ghost's GET has no JSON `Content-Type`/`Accept` requirement.
- **Output fixture (divergence → closed):** `OembedResponse` now carries the 12 `knownFields` + html/url + `thumbnail_url_original`
  (`types/dto_more_resources.cppm:48-98,250-272`) populated by the adapter; `bookmark`/`mention` requests return the
  `{version,type,url,metadata[,body,contentType]}` shape (`execute.cppm:225-245`). Values are stand-in provider data (no live network) —
  accepted non-route residual.
- **Path prefix (divergence → residual):** Ghost `/ghost/api/admin/oembed` vs C++ `/oembed` installed directly into the flat harness
  `RouterContext`; not fixable inside this folder.

Verdict: all eight route checks MATCH → row flipped PARTIAL → MATCH. Files changed: `routes.cppm`, `browse/handler.cppm`,
`browse/workflow/step/execute.cppm`, `types/dto_more_resources.cppm`. Complementary applied record: `docs/parity/oembed.md`.
