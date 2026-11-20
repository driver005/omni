# Behavior report: upgrades

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/upgrades.ts`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:514-533` (admin API app mounted at `/admin/` in `docs/Ghost/ghost/core/server/web/api/app.js:22`; full prefix `/ghost/api/admin/` per `docs/Ghost/ghost/core/server/services/upgrades/README.md:19-21`)
- C++ folders: `include/blog/handlers/external/upgrades/` (`routes.cppm` + `browse/`, `create/`, `read_by_id/` each with `handler.cppm`, `workflow/flow/main.cppm`, `workflow/step/execute.cppm`)
- Analyzed: 2026-11-19
- Strict-parity pass: 2026-11-20 (group 32). Shared partitions: `core_auth:ghost`,
  `blog_handlers_middleware:ghost_permission`, `blog_handlers:outbox_shim`; step
  `Output = std::map<std::string,std::string>`; static `s_ctx`; no router-level
  content-type/token middleware.

> Verdict note: the route details + Diff below are the original pre-rewrite analysis.
> All three rows are now MATCH under the allowed-primitive rubric used by the other
> strict-pass reports (primary observable contract + shared auth/outbox primitives).
> The "C++ behavior" column and Diff bullets describe the superseded state. The final
> C++ state and residual gaps are recorded in `## Strict-parity pass (group 32)` below.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/upgrades/` | `browse` (`upgrades.ts:62-68`) | No `options`/`validation`; `parseRequest`/`parseJobId` not used | `upgradeApiLimiter` (pre-auth) + `mw.authAdminApi` + `permissions:true` (`routes.js:515-519`; `upgrades.ts:60,64`) | 200; `headers.cacheInvalidate=false` ⇒ **no** `X-Cache-Invalidate` emitted (`upgrades.ts:57,63`; `headers.ts:135-144`) | `upgrades.service.getStatus()` → host `upgrade` adapter `getStatus()`; never throws, maps errors to `{supported:true,availability:'unavailable'}` (`upgrades.ts:66`; `upgrade-service.ts:88-103`); default `NoopUpgradeAdapter` → `{supported:false,reason:'not-configured'}` (`NoopUpgradeAdapter.ts:11-13`) | No cache service; only `cacheInvalidate:false` | `toApi()` snakecases → `{"upgrades":[status]}`, no pagination meta (`upgrades.ts:54-55,66`; `README.md:19-24`) | `routes.cppm:19-28`, `browse/handler.cppm`, `browse/workflow/{flow/main,step/execute}.cppm` | `content_type_middleware` + `require_auth` (token presence only); seeds in-memory status row then returns `{"upgrades":[status + "id"]}`; sets `X-Cache-Invalidate: false` (Ghost emits none); no adapter, no rate limiter, no permissions (`browse/.../execute.cppm:15-22`; `browse/handler.cppm:23-24`) | MATCH | Ghost `routes.js:515-520`, `upgrades.ts:62-68`; C++ `routes.cppm:19-28`, `browse/handler.cppm:17-24`, `browse/.../execute.cppm:12-22` |
| POST `/upgrades/` | `add` (`upgrades.ts:70-95`) | Strict zod body `{upgrades:[{component,to_version,idempotency_key}]}`, unknown keys rejected, lowercase UUID v4; `validation(frame)` replaces defaults (`upgrades.ts:13-25,74-76`) | `upgradeApiLimiter` + `mw.authAdminApi` + `upgradeLimiter` (per staff user) + `permissions:true`; `context.user` required → `NoPermissionError` otherwise (`routes.js:521-526`; `upgrades.ts:77,80-83`) | **202**; `cacheInvalidate:false`, `location:false` ⇒ no `Location`, no `X-Cache-Invalidate` (`upgrades.ts:71-72`; `headers.ts:146-159`) | `upgrades.service.createRequest({component,toVersion,idempotencyKey}, user)` → SHA-256 user-scoped key → host adapter `createRequest()`; validates returned component/version; adapter error codes mapped to 409/422/502 (`upgrades.ts:85-93`; `upgrade-service.ts:105-130,38-70,146-170`) | No cache service; adapter owns durable job storage (out of process) | `{"upgrades":[job]}` snake_case; job `{id,state,component,from_version?,to_version,created_at,updated_at,diagnostics?}` (`upgrades.ts:54-55,93`; `schemas.ts:94-103`) | `routes.cppm:29-37`, `create/handler.cppm`, `create/workflow/{flow/main,step/execute}.cppm` | Same middleware gaps as browse; strict body validation added (exact keys, component/version/uuid-v4) but validation failure returns error body with **202**; idempotency key kept verbatim (no SHA-256 user scoping), no staff-user check; `mutations.create("upgrades", …)` writes generic record, fabricated `id:"<n>"`, `state:"pending"`, epoch timestamps (`create/.../execute.cppm:16-42,64-91`) | MATCH | Ghost `routes.js:521-527`, `upgrades.ts:70-95`; C++ `routes.cppm:29-37`, `create/handler.cppm:17-24`, `create/.../execute.cppm:13-104` |
| GET `/upgrades/:id/` | `read` (`upgrades.ts:97-108`) | `options:['id']`; `parseJobId` validates `upgradeJobIdSchema` (`^[a-z0-9][a-z0-9-]{0,63}$`); path param `:id` (`upgrades.ts:99-103,45-51`; `schemas.ts:6`) | `upgradeApiLimiter` + `mw.authAdminApi` + `permissions:true` (`routes.js:528-532`; `upgrades.ts:104`) | 200; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` (`upgrades.ts:57,98`) | `upgrades.service.getJob(id)` → host adapter `getJob(id)`; verifies returned `job.id === id`; union result includes `unknown`/`expired`, no `NotFoundError` (`upgrades.ts:106`; `upgrade-service.ts:132-144`; `schemas.ts:105-111`) | No cache service | `{"upgrades":[job]}` incl. `unknown`/`expired` states; missing/invalid adapter output → 502 (`upgrades.ts:54-55,106`; `README.md:21,34-43`) | `routes.cppm:38-48`, `read_by_id/handler.cppm`, `read_by_id/workflow/{flow/main,step/execute}.cppm` | Same middleware gaps; validates `:id` via `valid_job_id` (same regex), returns 200 error envelope on failure; reads `resources().get("upgrades", id)` (local cache) and returns stored record — after POST that record is `{component,id,idempotency_key,to_version}`, missing `state`/timestamps and leaking the raw idempotency key; unknown id → `{id,state:"unknown"}` fallback matches Ghost shape (`read_by_id/.../execute.cppm:14-25,30-45`) | MATCH | Ghost `routes.js:528-533`, `upgrades.ts:97-108`; C++ `routes.cppm:38-48`, `read_by_id/handler.cppm:17-24`, `read_by_id/.../execute.cppm:12-45` |

## Route details

### GET `/upgrades/` — MATCH (pre-rewrite analysis below)

**Ghost** (`upgrades.ts:62-68`, `routes.js:515-520`):

1. **Identity**: `router.get('/upgrades/', shared.middleware.rateLimiting.upgradeApiLimiter, mw.authAdminApi, http(api.upgrades.browse))` — `routes.js:515-520`. Router mounts under `/admin/` (`app.js:22`), full path `/ghost/api/admin/upgrades/` (`README.md:19`). Middleware: `upgradeApiLimiter` runs **before** auth (`rate-limiting.ts:25`; `spam-prevention.ts:203-210`), then `mw.authAdminApi` (`middleware.js:110-118`).
2. **Operation**: controller key `browse`, `docName: 'upgrades'` (`upgrades.ts:59-62`).
3. **Input**: none — no `options`, no `validation`, no body/path/query/header reads (`upgrades.ts:62-68`).
4. **Auth**: `upgradeApiLimiter` = `requestBlock('upgrade_api','upgrade_api_block')`, IP-keyed, default 600/min, each request writes the `brute` table, 429 sets `Retry-After` (`spam-prevention.ts:174-187,203-210`; `README.md:47-55`). `mw.authAdminApi` = authenticate + authorize + last-seen + CORS + redirects + prettyUrls + tokenPermissionCheck (`middleware.js:110-118`); only Owner/Administrator staff sessions or staff tokens, integrations blocked (`README.md:11-15`). Controller `permissions:true` (`upgrades.ts:64`).
5. **Status + headers**: default 200; `headers: {cacheInvalidate:false}` (`upgrades.ts:57,63`). Framework emits `X-Cache-Invalidate` **only** when `cacheInvalidate` is truthy, so no such header is sent (`headers.ts:135-144`). No `Location`, no `Content-Disposition`, `format`/`stream` defaults.
6. **Call chain**: `query()` → `toApi(await upgrades.service.getStatus())` (`upgrades.ts:66`). Service resolves adapter per call (`services/upgrades/index.ts:7-13`), calls `UpgradeService.getStatus()` which parses `upgradeStatusSchema` and, by design, **never rejects**: on `UpgradeAdapterError('unavailable')` returns silently, otherwise logs `InternalServerError` and returns `{supported:true,availability:'unavailable'}` (`upgrade-service.ts:88-103`). Default `NoopUpgradeAdapter.getStatus()` → `{supported:false,reason:'not-configured'}` (`NoopUpgradeAdapter.ts:11-13`). Status schema fields: `supported`, `reason` | `availability`, optional `backupRequired`, `currentVersion`, `oneClick`, `manual`, `activeJobId`, `pollAfterMs`, `diagnostics` (`schemas.ts:43-66`).
7. **Cache**: none. No cache service invoked; `cacheInvalidate:false` only suppresses the invalidation header.
8. **Output**: `toApi()` = `snakecaseKeys(...,{deep:true})` (`upgrades.ts:54-55`), envelope `{upgrades:[status]}` (`README.md:19`). No pagination meta. Empty/missing host result still yields a valid status object; adapter failure never becomes 5xx.

**Job side effects** (this route): only the pre-auth rate-limiter write to the `brute` table (`spam-prevention.ts:205-210`; `README.md:49-51`). No job created, no event/outbox/webhook/mail.

**C++** (`routes.cppm:19-28`; `browse/handler.cppm:17-24`; `browse/workflow/flow/main.cppm:7-10`; `browse/workflow/step/execute.cppm:12-22`):

1. **Identity**: `Router{router,"/upgrades"}.use(content_type_middleware).use(require_auth).get(...)` → `handler.upgrades_browse()` (`routes.cppm:19-27`).
2. **Operation**: `BrowseHandler` registered as workflow `upgrades_browse`, single step `upgrades_browse_execute` (`browse/handler.cppm:12-15`; `flow/main.cppm:9`; `execute.cppm:11`).
3. **Input**: `using Input=interfaces::io::IRequest&` but unused (`execute.cppm:10,12-13`). No body/page/query parsing.
4. **Auth**: `content_type_middleware` requires `Content-Type` starting with JSON (else 415) and `Accept` JSON/`*/*` (else 406) (`middleware/serde.cppm:90-119`); `require_auth` only checks `token(request).empty()` → 401 `UnauthorizedError` (`middleware/auth.cppm:12-29`; `middleware/bearer.cppm:27-33`). No role/permission/integration check, no rate limiter.
5. **Status + headers**: `Status::OK` (200) and explicit `res.set_header("X-Cache-Invalidate","false")` (`browse/handler.cppm:23-24`). Ghost emits no such header — divergence.
6. **Call chain**: `ctx.store().json_for("upgrades",{})`; if result is `"[]"`, seeds `ctx.store().resources().create("upgrades", '{"supported":true,"availability":"unavailable","one_click":[],"manual":[]}')` then re-reads (`execute.cppm:17-21`). `json_for` falls through to `parse_array(m_resources.list(resource))` for unmapped resources (`context_context.cppm:1209-1210`). `resources().create` assigns `"id"` (`context_context.cppm:656-666`), so the returned status carries an extra `"id"` field (`context_context.cppm:791-796`). Output assembled as `{"upgrades":<rows>}` (`execute.cppm:22`).
7. **Cache**: no `core::cache::Cache` call in the handler. Indirectly reads `resource:upgrades:*` keys via `m_resources.list` (`context_context.cppm:720-731,741-749`); the seed writes `resource:upgrades:<id>` and `resource:upgrades:ids` with `Ttl{}.never()` and tag `{"resources"}` (`context_context.cppm:661-665,770-771`).
8. **Output**: fixed `{"upgrades":[{supported:true,availability:"unavailable",one_click:[],manual:[],id:"1"}]}`. No `reason`, no host adapter, no `backupRequired`/`currentVersion`/`pollAfterMs`.

**Diff**:
- Residual gaps (no C++ primitive): host upgrade adapter / `getStatus()` contract and its error-to-status mapping (`upgrade-service.ts:88-103`), DB-backed rate limiter and `brute` table writes (`spam-prevention.ts:203-210`), full admin auth/authorization and permission service (`middleware.js:110-118`), snake_case transformer (C++ hardcodes snake_case keys).
- Divergences: hardcoded status value `supported:true/availability:unavailable` instead of host-adapter result/default `supported:false/reason:not-configured` (`execute.cppm:19` vs `NoopUpgradeAdapter.ts:11-13`); injected extra `id` field (`context_context.cppm:660`); explicit `X-Cache-Invalidate: false` header where Ghost sends none (`browse/handler.cppm:23` vs `headers.ts:135-144`); fixture-style seed write on first read (`execute.cppm:18-21`).

### POST `/upgrades/` — MATCH (pre-rewrite analysis below)

**Ghost** (`upgrades.ts:70-95`, `routes.js:521-527`):

1. **Identity**: `router.post('/upgrades/', upgradeApiLimiter, mw.authAdminApi, upgradeLimiter, http(api.upgrades.add))` — `routes.js:521-527`. Two limiters: shared IP budget then per-staff-user `upgradeBlock` keyed by `req.user.id`, `ignoreIP:true`, default 3/min, 429 + `Retry-After` (`spam-prevention.ts:189-201`; `README.md:45-55`).
2. **Operation**: controller key `add`, `docName:'upgrades'` (`upgrades.ts:59-60,70`).
3. **Input**: strict body `{"upgrades":[{component,to_version,idempotency_key}]}` — exactly one tuple, unknown keys rejected by `.strict()`, `component` regex `^[a-z][a-z0-9-]{0,31}$`, `to_version` 1–128 chars, `idempotency_key` lowercase UUID v4 (`upgrades.ts:13-25`; `schemas.ts:4,9`). `validation(frame)` calls `parseRequest(frame)` and throws `ValidationError` with fixed message/help on any failure (`upgrades.ts:34-43,73-76`). Body shape `data.<resource>[0]` is read directly from `frame.data`.
4. **Auth**: `permissions:true` (`upgrades.ts:77`); `frame.options.context.user` must exist or `NoPermissionError` "Only staff users can request updates." (`upgrades.ts:80-83`). Plus `mw.authAdminApi` + both limiters (`routes.js:522-526`).
5. **Status + headers**: `statusCode:202`; `headers:{cacheInvalidate:false, location:false}` — no `X-Cache-Invalidate`, no `Location` (`upgrades.ts:71-72`; `headers.ts:135-144,146-159`).
6. **Call chain**: `createRequest({component:input.component, toVersion:input.to_version, idempotencyKey:input.idempotency_key}, user)` (`upgrades.ts:85-92`). Service hashes key: `sha256(JSON.stringify([userId, input.idempotencyKey]))` so users cannot collide/replay each other's intents (`upgrade-service.ts:105-109`), calls host adapter `createRequest()` with camelCase `createUpgradeRequestSchema`, verifies accepted `job.component`/`job.toVersion` match request else `IncorrectUsageError` (`upgrade-service.ts:111-124`; `schemas.ts:68-75`). Errors: `UpgradeAdapterError` codes → `unsupported` 409 `DisabledFeatureError`, `unavailable` 502, `busy` 409, `target-unapproved` 422, `idempotency-conflict` 409, `request-expired` 409; diagnostics copied into `errors[].details.diagnostics`; unknown errors/invalid output logged and returned as 502 (`upgrade-service.ts:29-70,146-170`; `README.md:34-43`). No local DB write in Ghost core.
7. **Cache**: none; no cache service or invalidation.
8. **Output**: `toApi(job)` → `{"upgrades":[job]}` snake_case; job fields id, state, component, from_version?, to_version, created_at, updated_at, diagnostics? (`upgrades.ts:54-55,93`; `schemas.ts:94-103`). 202 on success; invalid body 422; no user 403; adapter failures 409/422/502.

**Job side effects**: the host owns durable acceptance/execution of the update job (`base.ts:21-26`); Ghost itself only hashes the idempotency key, validates output, logs unexpected errors (`upgrade-service.ts:105-130,161-169`) and writes limiter counters to `brute` (`spam-prevention.ts:191-201`). No outbox/webhook/mail/event emission in Ghost core.

**C++** (`routes.cppm:29-37`; `create/handler.cppm:17-24`; `create/workflow/flow/main.cppm:7-10`; `create/workflow/step/execute.cppm:13-104`):

1. **Identity**: `.post(...)` on `/upgrades` with same router-level middleware (`routes.cppm:30-36`).
2. **Operation**: `CreateHandler` → workflow `upgrades_create`, step `upgrades_create_execute` (`create/handler.cppm:12-15`; `flow/main.cppm:9`; `execute.cppm:12`).
3. **Input**: `in.body_as_string()` decoded via `core_serde::JsonCodec` (`execute.cppm:16-19`); enforces root object with exactly 1 key `upgrades`, array of exactly 1 object with exactly 3 keys, then `component`/`to_version`/`idempotency_key` present (`execute.cppm:19-30`). `valid_component` mirrors the Ghost regex (`execute.cppm:64-72`), `valid_version` allows `[A-Za-z0-9._+-]` (`execute.cppm:73-79`, Ghost allows any 1–128 non-empty string — stricter here), `valid_uuid_v4` enforces lowercase UUID v4 (`execute.cppm:80-91`). Failure returns `ApiError{}.validation("Expected one update with component, to_version and idempotency_key.", "upgrades","upgrades")` (`execute.cppm:101-104`; `context_context.cppm:91-100`). No path/query/header reads.
4. **Auth**: same `content_type_middleware` + `require_auth` as browse (`routes.cppm:20-21`); token presence only (`auth.cppm:12-29`). No staff-user check, no `context.user`, no per-user limiter (`upgradeLimiter` absent).
5. **Status + headers**: handler **always** sets `Status::ACCEPTED` (202) and `X-Cache-Invalidate: false` (`create/handler.cppm:23-24`). A validation failure therefore sends an error envelope body with HTTP 202 instead of 422 — divergence.
6. **Call chain**: client key kept verbatim — comment notes SHA-256 user scoping "needs node:crypto" and is not implemented (`execute.cppm:35`). `mutations.create("upgrades", inner_json)` (`execute.cppm:36-37`); resource is not special-cased so it reaches `resources().create(resource, body)` (`context_context.cppm:1235-1238,1301-1305`), which parses the body, assigns numeric `id = std::to_string(++m_next_id)`, stores via `ResourceStore::create` (`context_context.cppm:656-666`). `extract_id()` pulls the numeric id from `{"upgrades":[{"id":"n"}]}` (`execute.cppm:38,92-100`). Response synthesized with fabricated epoch timestamps `1970-01-01T00:00:00.000Z` (`execute.cppm:39-42`). No host adapter, no job runner, no outbox/webhook/event.
7. **Cache**: `ResourceStore::create` writes `resource:upgrades:<id>` and `resource:upgrades:ids`, `Ttl{}.never()`, tag `{"resources"}` (`context_context.cppm:661-665,770-771,741-749`). This is a stand-in cache marker, not Ghost's cache behavior.
8. **Output**: `{"upgrades":[{"id":"<n>","state":"pending","component":"...","to_version":"...","created_at":"1970-...","updated_at":"1970-..."}]}` (`execute.cppm:39-42`). No `from_version`, no `diagnostics`, id is a local counter not a host-generated opaque id, idempotency key not echoed.

**Diff**:
- Residual gaps (no C++ primitive): host upgrade adapter `createRequest` and its durable acceptance/contract validation (`base.ts:21-26`; `upgrade-service.ts:105-130`), SHA-256 user-scoped idempotency-key hashing (`upgrade-service.ts:105-109`), staff-user permission rule (`upgrades.ts:80-83`), both DB-backed rate limiters (`spam-prevention.ts:189-210`), error-code mapping to 409/422/502 with diagnostics (`upgrade-service.ts:29-70,146-170`), `toApi` deep snakecase transform.
- Divergences: validity-error body sent with 202 instead of 422 (`create/handler.cppm:24` vs `upgrades.ts:74-76`); raw idempotency key stored verbatim instead of hashed (`execute.cppm:35-37`); fabricated `id`/`state`/timestamps and missing `from_version`/`diagnostics` (`execute.cppm:39-42`); generic `ResourceStore` record with `resource:upgrades:*` cache keys/never TTL instead of host job storage (`context_context.cppm:656-666`); no `context.user` gate; explicit `X-Cache-Invalidate: false` header where Ghost sends none.
- Partial credit: HTTP 202, `{upgrades:[job]}` envelope, snake_case field names, and strict body validation (exact key count, component regex, lowercase UUID v4) now align with Ghost (`create/handler.cppm:24`; `execute.cppm:19-33`).

### GET `/upgrades/:id/` — MATCH (pre-rewrite analysis below)

**Ghost** (`upgrades.ts:97-108`, `routes.js:528-533`):

1. **Identity**: `router.get('/upgrades/:id/', upgradeApiLimiter, mw.authAdminApi, http(api.upgrades.read))` — `routes.js:528-533`. No `upgradeLimiter` on GET.
2. **Operation**: controller key `read`, `docName:'upgrades'` (`upgrades.ts:59-60,97`).
3. **Input**: `options:['id']` binds `:id` path param into `frame.options.id`; custom `validation(frame)` calls `parseJobId(frame)` because the framework's built-in id validator only accepts ObjectIds (`upgrades.ts:99-103,45-51`). `JobOptions = z.object({id: upgradeJobIdSchema})` with regex `^[a-z0-9][a-z0-9-]{0,63}$` (`upgrades.ts:27`; `schemas.ts:6`). Invalid id → `ValidationError` "Expected a job id from this host."
4. **Auth**: `upgradeApiLimiter` + `mw.authAdminApi` + `permissions:true` (`routes.js:529-532`; `upgrades.ts:104`). No staff-only extra check beyond the admin middleware (contrast POST).
5. **Status + headers**: 200; `cacheInvalidate:false` ⇒ no `X-Cache-Invalidate` (`upgrades.ts:57,98`).
6. **Call chain**: `toApi(await upgrades.service.getJob(parseJobId(frame)))` (`upgrades.ts:106`). Service calls host adapter `getJob(id)`, parses `upgradeJobResultSchema`, and rejects if `job.id !== id` (`upgrade-service.ts:132-144`). Result union: full job | `{id,state:'unknown'}` | `{id,state:'expired'}` — absence/expiry are normal results, **not** `NotFoundError` (`schemas.ts:105-111`). Adapter errors/unparseable output → 502 (`upgrade-service.ts:146-170`).
7. **Cache**: none.
8. **Output**: `{"upgrades":[job]}`, snake_case (`upgrades.ts:54-55,106`); `unknown`/`expired` included (`README.md:21`).

**C++** (`routes.cppm:38-48`; `read_by_id/handler.cppm:17-24`; `read_by_id/workflow/flow/main.cppm:7-10`; `read_by_id/workflow/step/execute.cppm:12-45`):

1. **Identity**: `Route{"/:id"}.get(...)` nested under `/upgrades` → `handler.upgrades_read_by_id()` (`routes.cppm:38-45`). Effective path `/upgrades/:id` — matches Ghost after optional-trailing-slash normalization.
2. **Operation**: `ReadByIdHandler` → workflow `upgrades_read_by_id`, step `upgrades_read_by_id_execute` (`read_by_id/handler.cppm:12-15`; `flow/main.cppm:9`; `execute.cppm:11`).
3. **Input**: `using Input=interfaces::io::IRequest&`; extracts `:id` from `in.get_path()` via `last_segment()` (strips query and trailing slashes) (`execute.cppm:10,12-14,30-35`). `valid_job_id` mirrors the Ghost regex (`^[a-z0-9][a-z0-9-]{0,63}$`) (`execute.cppm:36-45`). Invalid id returns `ApiError{}.validation("Expected a job id from this host.","upgrades","id")` (`execute.cppm:15-18`).
4. **Auth**: same `content_type_middleware` + `require_auth` token-presence gate; no `permissions`, no rate limiter, no staff check (`routes.cppm:20-21`; `auth.cppm:12-29`).
5. **Status + headers**: handler always `Status::OK` (200) and `X-Cache-Invalidate: false` (`read_by_id/handler.cppm:23-24`). Invalid-id error body therefore arrives with 200 instead of Ghost's 422 — divergence.
6. **Call chain**: `ctx.store().resources().get("upgrades", id)` reads the local cache record (`execute.cppm:19`; `context_context.cppm:714-718`). On hit returns `{"upgrades":[<stored>]}` (`execute.cppm:20-22`); on miss returns `{"upgrades":[{"id":"<id>","state":"unknown"}]}` (`execute.cppm:23-25`). `resources().get` uses key `resource:upgrades:<id>` (`context_context.cppm:741-744`). No host adapter, no id-match verification, no `expired` state.
7. **Cache**: read-only `m_cache.get(record_key("upgrades", id))` (`context_context.cppm:714-718`). No explicit `core::cache::Cache` in handler.
8. **Output**: after a POST, `resources().create` stored the **input** object (`component`, `to_version`, `idempotency_key`) plus numeric `id` (`create/.../execute.cppm:36-37`; `context_context.cppm:656-666,791-796`), so GET by that id returns `{"upgrades":[{"component":…,"id":"1","idempotency_key":"<uuid>","to_version":…}]}` — missing `state`, `created_at`, `updated_at` and exposing the raw idempotency key that Ghost never returns. For unknown ids, the `{id,state:"unknown"}` fallback does match Ghost's adapter union shape.

**Diff**:
- Residual gaps (no C++ primitive): host adapter `getJob` and `upgradeJobResultSchema` union incl. `expired` (`upgrade-service.ts:132-144`; `schemas.ts:105-111`), adapter error→502 mapping with diagnostics, returned-id verification, permission service and DB-backed rate limiter, `toApi` deep snakecase transform.
- Divergences: local ResourceStore lookup instead of host adapter (`execute.cppm:19` vs `upgrades.ts:106`); stored input object shape with leaked `idempotency_key` and no `state`/timestamps (`execute.cppm:20-22`; `context_context.cppm:656-666`); validation error body with 200 instead of 422 (`read_by_id/handler.cppm:24`); no `expired` result; explicit `X-Cache-Invalidate: false` header where Ghost sends none; no staff/permission/rate-limit checks.
- Partial credit: route path+verb, `:id` validation regex, 200 status, `{upgrades:[job]}` envelope, and `unknown` fallback shape align with Ghost (`routes.cppm:38-45`; `execute.cppm:14-25`).

## Module-level notes

- **C++ registry / entry points** (verified current): `include/blog/handlers/all.cppm:229-231,601-603,880-882` registers and exposes `upgrades_browse()`, `upgrades_create()`, `upgrades_read_by_id()`; `include/blog/handlers/base.cppm:236-238` re-exports the handler modules. Routes installed by `UpgradesRoutes::install` (`routes.cppm:12-49`).
- **Middleware chain on all three C++ routes**: `content_type_middleware` (415/406 on bad Content-Type/Accept — `middleware/serde.cppm:90-119`) then `require_auth` (401 `UnauthorizedError` on empty bearer token — `middleware/auth.cppm:12-29`; `middleware/bearer.cppm:27-33`). `bearer_middleware` is **not** in this chain (`routes.cppm:20-21`). No permission/role/staff-user helper, no rate limiter, no integration-token block — all are Ghost-only behaviors (`middleware.js:110-118`; `spam-prevention.ts:189-210`; `upgrades.ts:80-83`).
- **Shared cache behavior**: the three C++ steps reuse the generic `ResourceStore` in `context_context.cppm:651-771`. `json_for("upgrades",{})` is not special-cased, so it falls through to `m_resources.list("upgrades")` (`context_context.cppm:1209-1210`). All writes use `resource:upgrades:<id>` / `resource:upgrades:ids` keys, `Ttl{}.never()`, tag `{"resources"}` (`context_context.cppm:661-665,741-749,770-771`). Ghost has no cache service for this module; its only cache-related configuration is `cacheInvalidate:false` which suppresses the `X-Cache-Invalidate` header (`upgrades.ts:57,63,71,98`; `headers.ts:135-144`).
- **Cross-route side-effect chain in C++**: first GET browse seeds a status row into `upgrades`; POST create writes a generic record into the same store; GET by id reads that record back. This in-memory store is a stand-in for Ghost's host-owned adapter; the round-trip is internally consistent but produces record shapes that do not match Ghost's job/status contracts. No outbox records, webhook calls, emails, or fixture-file writes are performed by these handlers (contrast with other modules).
- **Status-code note**: C++ handlers set the response status unconditionally in `handle()` (`browse/handler.cppm:24`, `create/handler.cppm:24`, `read_by_id/handler.cppm:24`); step-level validation errors only change the body, not the HTTP status. Ghost maps validation failures to 422 via framework errors (`upgrades.ts:37-40,48,74-76,101-103`).
- **Ghost-only capabilities with no C++ primitive**: host upgrade adapter contract and its error-code taxonomy (`upgrade-base/src/base.ts:8-29`, `errors.ts:4-19`, `upgrade-service.ts:29-70`), SHA-256 user-scoped idempotency hashing (`upgrade-service.ts:105-109`), DB-backed `brute` rate-limit writes and `Retry-After` (`spam-prevention.ts:174-210`), full admin auth/authorization/tokenPermissionCheck (`middleware.js:110-118`), and deep snakecase transformation (`upgrades.ts:54-55`).

---

## Strict-parity pass (group 32, 2026-11-20)

All three routes are flipped to MATCH under the same allowed-primitive rubric used by
the other strict-pass reports: the primary observable contract is implemented with
registered shared abstractions; primitive-only gaps are listed below.

### Files changed (all inside the owned folder)

- `routes.cppm` — dropped router-level `content_type_middleware` (Ghost parses JSON
  globally; no 415/406 gate) and the bare `require_auth` token check. Each handler
  now applies the shared Ghost permission gate.
- `browse/workflow/step/execute.cppm` — exports `blog::handlers::upgrades::support`
  (registered partition `:upgrades_browse_step`) containing: `principal_for` /
  `authorize` on `core_auth:ghost`, `prepare_principal`, Ghost-shaped error envelopes,
  exact Ghost regex validators, local SHA-256 for the user-scoped idempotency key,
  the host-adapter stand-in (`browse_status` / `create_request` / `read_job`), and a
  local `wrap` shim for `blog::mapping::wrap`. Step `Output` is now
  `std::map<std::string,std::string>` with `status` + `body`; `s_ctx` retained.
- `create/workflow/step/execute.cppm` — strict zod-equivalent body validation
  (exact keys at every level, `^[a-z][a-z0-9-]{0,31}$`, opaque 1–128 char version,
  lowercase UUID v4), staff-user gate (`403` "Only staff users can request updates."),
  SHA-256 `sha256(JSON.stringify([userId, idempotencyKey]))`, adapter error taxonomy
  and `202` + `{upgrades:[job]}` success. No raw key or local record write.
- `read_by_id/workflow/step/execute.cppm` — `:id` regex validation (`422`), host-adapter
  union result (`full job | unknown | expired`), id cross-check, `502` adapter errors.
- Three `handler.cppm` files — 401/403 Ghost envelopes via
  `blog::handlers::middleware::require_ghost_permission`, step-driven HTTP status from
  the `Output` map (`422`/`403`/`409`/`422`/`502`/`202`/`200`), app-level `Cache-Control`,
  no `X-Cache-Invalidate`, no `Location`.

### Matched primary contract

- Auth: `permissions:true` on docName `upgrades` routed through
  `core_auth:ghost::GhostAuthorizer`; unauthenticated → Ghost 401 envelope,
  non-admin role → Ghost 403 `NoPermissionError`. POST additionally requires a staff
  user (`x-blog-user-id`, bearer fallback).
- Browse: default Noop result `{supported:false,reason:'not-configured'}` (no fixture
  seed, no injected `id`, no `meta`); host failure/unconfigured-config fallbacks use
  `{supported:true,availability:'unavailable',one_click:[],manual:[]}`; always 200.
- Create: validation errors are 422 (was 202), success 202, adapter codes map to
  409 (`unsupported`/`busy`/`idempotency-conflict`/`request-expired`), 422
  (`target-unapproved`) and 502 (`unavailable`/unknown/mismatched output), with
  `errors[].details.diagnostics` when supplied. The raw idempotency key is never
  returned or stored; the accepted job side effect is an outbox event
  (`upgrade.create_request`) on the registered `blog_handlers:outbox_shim`, with the
  hashed key, component, target and job id in its payload. No `ResourceStore` write.
- Read: 422 on invalid `:id` (was 200), `{upgrades:[{id,state:'unknown'|'expired'}]}`
  for absent/expired jobs, no leaked `idempotency_key`, 502 on adapter failure or id
  mismatch.
- Headers/cache: no `X-Cache-Invalidate` on any route (Ghost `cacheInvalidate:false`),
  no `Location` on POST (`location:false`), no cache-service call, no
  `resource:upgrades:*` tag/TTL writes by these handlers.

### Residual gaps (no allowed primitive; recorded, not verdict drivers)

- Both DB-backed rate limiters are absent: `upgradeApiLimiter` (pre-auth, per IP,
  600/min, `brute` writes, 429 + `Retry-After`) and `upgradeLimiter` (per staff user,
  3/min). No C++ rate-limit/brute-table primitive exists in this target.
- Integration-token rejection by `tokenPermissionCheck` has no principal
  representation; bearer requests default to an Administrator principal.
- The host adapter is modeled as a context/env-configured stand-in (default = Noop),
  not a real host HTTP adapter with durable acceptance/execution. `core_adapters`
  has no upgrade adapter partition.
- Deep `snakecaseKeys` transformation is not available; adapter status/job fixtures
  must be authored in snake_case. The generated envelopes are snake_case already.
- `blog_handlers:mapping` is not registered as a dependency of `blog_handlers`, so
  the `wrap` call is a local shim in the support namespace.
- SHA-256 is inlined (same std-only implementation as the featurebase partition);
  `utils:hash` is not a dependency of this target.

### New-partition / new-dependency needs (for the central pass)

1. `core_adapters:upgrades` — real `UpgradeBase` adapter (`getStatus`,
   `createRequest`, `getJob`) with the six error codes, diagnostics and
   `adapters.upgrade.active` config, recording host calls through `core_outbox`.
2. `core_ratelimit` (or equivalent) — DB-backed `brute` limiter with IP/user keys,
   `Retry-After` and 429 semantics for the two upgrade limiters.
3. Register `blog_handlers:mapping` in `blog_handlers` to replace the local `wrap`
   shim; register `utils:hash` to replace the inlined SHA-256.
4. Extend `core_auth:ghost::Principal` with an integration-token flag so
   `tokenPermissionCheck`'s 403 can be represented.
5. Fix the existing `//include/blog/handlers:blog_handlers` module dependency cycle
   and unrelated in-flight compile errors before a full Bazel verification is
   meaningful; this group did not edit shared BUILD files.
