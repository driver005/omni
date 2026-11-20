# Parity: oembed

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/oembed.js`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/oembed/routes.cppm` (handler tree `external/oembed/browse/`)
- Checked: 2026-11-19
- Fix applied: 2026-11-19 (checks 1–8). Strict-parity rewrite: 2026-11-20 (group 28),
  source reshaped inside the folder only; verdict flipped to MATCH with non-route residuals recorded below.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 1 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/oembed` | `read` (`oembed.js:7-19`) | GET `/oembed` (`external/oembed/routes.cppm:19-30`) | `oembed_browse()` → `BrowseHandler` (`all.cppm:552,801`; `routes.cppm:28`) | Ghost `data:['url','type']`, `options:[]` (`oembed.js:12-13`); C++ `Input=IRequest&`, query parsed into `std::map<std::string,std::string>` `url`/`type` (`execute.cppm:315-317`). MATCH. | Ghost raw oembed object (14 oembed fields incl. 12 `knownFields` + `thumbnail_url_original`) or bookmark `{version,type,url,metadata}` (`oembed-service.js:769-784,838-850,937-963`); C++ `OembedResponse` encode with all 14 + `thumbnail_url_original`, plus bookmark/mention shape, 422 `ApiError.validation` on bad URL (`execute.cppm:193-268`, `types/dto_more_resources.cppm:48-98,250-272`). MATCH (stand-in values). | Ghost none — external HTTP fetch + `imageStore.saveRaw` (`oembed-service.js:308-371,906-988`). C++ no store read; `oembed.fetch` emitted to shared `blog::handlers::outbox`, drained by local `OembedFetchAdapter` which emits `oembed.thumbnail.saveRaw` (`execute.cppm:290-300`); no DB call. MATCH (external fetch stand-in). | none both sides (Ghost `cacheInvalidate:false`, `oembed.js:8-10`; C++ no `core::cache::Cache`, no header). MATCH. | Ghost `mw.authAdminApi` (`routes.js:541`) + `permissions:false` (`oembed.js:11`); C++ `bearer_middleware` + `require_auth` at router (`routes.cppm:24-25`), no docName/permission check. MATCH. | MATCH (non-route residuals: real HTTP transport, image bytes, `/ghost/api/admin` prefix) | Ghost `routes.js:541`, `oembed.js:5-19`, `oembed-service.js:769-784,829-851,937-963`; C++ `routes.cppm:19-30`, `handler.cppm:18-36`, `execute.cppm:1-330`, `types/dto_more_resources.cppm:48-98,250-272`. |

## Route details

### GET `/oembed` — MATCH

- Route identity: MATCH. Ghost `router.get('/oembed', mw.authAdminApi, http(api.oembed.read))` (`admin/routes.js:541`) vs C++ `Router{router,"/oembed"}...get(...)` (`external/oembed/routes.cppm:19,28`). Same path, same verb.
- Operation mapping: MATCH. Ghost key `read` calls `oembed.fetchOembedDataFromUrl(url,type)` (`oembed.js:7,17`); C++ binds `handler.oembed_browse()` (`routes.cppm:29`; registry `all.cppm:552,801`) and the `oembed_browse` flow (`browse/workflow/flow/main.cppm:9`). The step now performs Ghost `read` semantics (single object) via the outbox fetch stand-in — no static placeholder.
- Input: MATCH. Ghost declares `data:['url','type']`, `options:[]` and destructures `{url,type}` (`oembed.js:12-15`); C++ `support::query_params` parses `in.get_path()` into `std::map<std::string,std::string>` and reads `url`/`type` (`execute.cppm:315-317`). Missing/invalid absolute URL → 422 `ValidationError` "No provider found for supplied URL." (`oembed-service.js:256-261,989-1009`; `execute.cppm:213-217`).
- Output: MATCH. Ghost returns the raw oEmbed object (`type,version,html,url,title,width,height,author_name,author_url,provider_name,provider_url,thumbnail_url,thumbnail_width,thumbnail_height` — 12 `knownFields` beyond html/url — plus `thumbnail_url_original` from `storeThumbnail`), or the bookmark `{version,type,url,metadata}` shape for `bookmark`/`mention` (`oembed-service.js:769-784,838-850,937-963`). C++ `OembedResponse` now encodes all 14 fields including `thumbnail_url_original` (`types/dto_more_resources.cppm:48-98,250-272`); the adapter populates them from the fetch stand-in and builds the bookmark/mention shape (`execute.cppm:225-245`). Successful status 200; 422 on bad input. Values are stand-in provider data (no live network); accepted non-route residual.
- DB call: MATCH. Ghost controller/service make no model call — they use `externalRequest` + `imageStore.saveRaw` (`oembed-service.js:308-371,906-988`). C++ no longer calls `find_by_resource("oembed", url)` (that central store has no `oembed` branch and always returned `nullopt`); instead it emits `oembed.fetch` through the registered shared `blog::handlers::outbox` shim (`blog_handlers:outbox_shim`, CONTRACTS.md §4), drains it, and a locally registered `OembedFetchAdapter` produces the fetched object and emits `oembed.thumbnail.saveRaw` (`execute.cppm:290-300`). No DB call on either side.
- Cache: MATCH. Ghost `headers.cacheInvalidate:false` and the service caches nothing; C++ sets no `X-Cache-Invalidate` header and uses no `core::cache::Cache` (`handler.cppm:34`).
- Auth: MATCH. Ghost `mw.authAdminApi` (`routes.js:541`) + `permissions:false` (`oembed.js:11`) — no docName check. C++ router installs `bearer_middleware` + `require_auth` (`routes.cppm:24-25`; `middleware/auth.cppm:12-29`), with the inline token-presence block and `content_type_middleware` removed; no permission model — Ghost's GET has no JSON content-type/Accept requirement.

#### Fix

Strict-parity rewrite applied to `include/blog/handlers/external/oembed/` only (no shared files, no BUILD edits, no Bazel-cycle change). Registered shared abstractions used: `blog_handlers:outbox_shim` (`outbox::global_outbox`, `outbox::Event`, `outbox::Adapter`, `register_adapter`), `blog_handlers_middleware` (`require_auth`, `bearer_middleware`, `encode`, `ApiError`), `core_workflow`. Output is `std::map<std::string,std::string>` (`{"status","body"}`); the step keeps static `s_ctx` (set by `BrowseHandler` ctor). No new global state outside the folder; the outbox adapter is registered once via a function-local static.

Checks 1–8 closed:
- **1, 2 (route/op):** kept `GET /oembed` and `handler.oembed_browse()` + `oembed_browse` flow. Removed `.use(content_type_middleware)` — Ghost's GET has no content-negotiation gate (closes the 415/406 divergence).
- **3 (input):** `query_params` parses `url`/`type` from `IRequest::get_path()`; invalid/missing absolute URL → Ghost ValidationError 422 (`unknownProvider`, `oembed-service.js:256-261`).
- **4 (output):** added `thumbnail_url_original` to `OembedResponse` and its `serde::FieldDesc`; adapter populates all 12 knownFields (+ html/url) from the fetch stand-in and adds the `thumbnail_url`/`thumbnail_width`/`thumbnail_height` rewrite; bookmark/mention requests return `{version,type,url,metadata[,body,contentType]}`. Handler maps step `status` (200/422) to `interfaces::io::types::Status`.
- **5 (DB call):** deleted `store.find_by_resource("oembed", url)` and the `decode` fallback fixture. Replaced with `oembed.fetch` outbox event → local `OembedFetchAdapter`, which emits `oembed.thumbnail.saveRaw` as the `imageStore.saveRaw` stand-in. No central store change was needed (the `"oembed"` branch need is retired; see new-partition note below).
- **6 (cache):** unchanged — none on either side.
- **7 (auth):** router-level `bearer_middleware` + `require_auth` replace the inline `token(req).empty()` → `{"error":"Unauthorized"}` block. `permissions:false` — no permission model.
- **8 (verdict):** all eight checks MATCH → row flipped PARTIAL/MISMATCH → MATCH. Non-route residuals: real outbound HTTP transport / provider registry, byte-level `imageStore.saveRaw` + SVG rasterize, and the `/ghost/api/admin` router prefix (harness installs `/oembed` directly).

New-partition needs recorded (not added — would require shared BUILD/main-agent wiring):
- A real `core_adapters` oEmbed provider adapter (HTTP fetch via `core_adapters:http`'s `HttpTransport`/`parse_url`/`normalize`, Twitter/OpenSea providers, thumbnail download) belongs in a new `include/core/adapters/oembed.cppm` partition. `blog_handlers` does not depend on `core_adapters` and the Bazel cycle is out of scope, so the adapter stays local to this folder using the `outbox` shim.
- A real image-store/SVG-rasterize primitive and the `url.forImage` absolute-URL mapper would need a new shared partition; the stand-in uses `content/images/<kind>/<id>.<ext>` paths.

## Missing in C++

None. The single Ghost route `GET /oembed` has a C++ counterpart at `external/oembed/routes.cppm:19-30`.

  Fix: No change required. Check 1 closed. The `#### Fix` reshaped the existing handler/step to parity; no route added.

## Extra in C++

None. `include/blog/handlers/external/oembed/` exposes exactly one route (GET `/oembed`), matching the Ghost controller route.

  Fix: No change required. Check 1 closed. The single GET registration is kept; middleware/input/output behavior changed only.

## Applied status (2026-11-20, strict parity)

Changed files (all inside the matched folder; no shared files, no BUILD edits):
- `routes.cppm` — removed `content_type_middleware`; added `bearer_middleware` + `require_auth` (Ghost `authAdminApi`, `permissions:false`).
- `browse/handler.cppm` — removed inline token-presence 401; maps step `{"status","body"}` map to response status (200/422); no cache header.
- `browse/workflow/step/execute.cppm` — `Output = std::map<std::string,std::string>`; parses `url`/`type`; 422 on invalid URL; emits `oembed.fetch` to shared outbox, drains via local `OembedFetchAdapter`, which populates the 12 knownFields DTO (+ `thumbnail_url_original`) or the bookmark/mention shape and emits `oembed.thumbnail.saveRaw`; static `s_ctx` retained.
- `types/dto_more_resources.cppm` — added `thumbnail_url_original` field + serde entry (12 knownFields + html/url already present from the 2026-11-19 pass).

Not applied (non-route residuals / new-partition needs):
- Real HTTP provider fetch, custom-provider registry, favicon probes, SVG rasterize, byte-level `imageStore.saveRaw`, and absolute `url.forImage` mapping — no allowed primitive; recorded as new `core_adapters` oEmbed / image-store partition needs above.
- `/ghost/api/admin` path prefix — the harness installs `/oembed` directly into a flat `RouterContext`.
- No Bazel-cycle fix, no shared/BUILD file edits.
