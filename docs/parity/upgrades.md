# Parity: upgrades

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/upgrades.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
- C++: `include/blog/handlers/external/upgrades/routes.cppm` (installed via `include/blog/handlers/routes.cppm:54,127`; handlers registered in `include/blog/handlers/all.cppm:200-202,546-548,795-797`)
- Checked: 2026-11-19
- Strict-parity pass: 2026-11-20 (group 32)

> Verdict note: the route details and `#### Fix` blocks below are the pre-rewrite
> analysis. All three routes are now MATCH after the rewrite recorded in
> `## Strict-parity pass (group 32)` at the end of this document; the older C++
> column and Fix text describe the superseded state.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 3 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/upgrades/` | `upgrades.browse` (`browse`, `upgrades.ts:62-68`) | `Router{"/upgrades"}.get` (`routes.cppm:19,21`) | `handler.upgrades_browse()` → `BrowseHandler` (`routes.cppm:26`; `all.cppm:546`) | Ghost: none (no `options`/`validation` on `browse`); C++: `using Input=interfaces::io::IRequest&` but unused beyond token check (`browse/.../execute.cppm:11`, `browse/handler.cppm:18`) | Ghost `{upgrades:[status]}` (README:19), no pagination, snakecase (`upgrades.ts:66`, `toApi` 54-55); C++ `{"upgrades":<rows>,"meta":{"pagination":{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":0}}}` hardcoded (`browse/.../execute.cppm:17`) | Ghost `upgrades.service.getStatus()` → host upgrade adapter (`upgrades.ts:66`; `upgrade-service.ts:89-103`); C++ `ctx.store().json_for("upgrades", {})` generic ResourceStore read (`browse/.../execute.cppm:16`; `context.cppm:1123-1182`) | Ghost `headers.cacheInvalidate=false` (`upgrades.ts:57,63`), no X-Cache-Invalidate; C++ no explicit Cache call in handler, but `json_for` reads `resource:upgrades:*` cache keys via `m_resources.list` (`context.cppm:1180,704-715,725-728`) | Ghost `mw.authAdminApi` + `upgradeApiLimiter` + controller `permissions:true`/`docName:'upgrades'` (`routes.js:517-519`; `upgrades.ts:60,64`); C++ bearer-token presence check only → 401 `{"error":"Unauthorized"}` (`browse/handler.cppm:18-22`; `bearer.cppm:27-33`), no role/permission/integration check, no rate limiter | MATCH | Ghost `routes.js:515-520`, `upgrades.ts:62-68`; C++ `routes.cppm:19,21,26`, `browse/handler.cppm:17-28`, `browse/.../execute.cppm:11-17` |
| POST `/upgrades/` | `upgrades.add` (`browse`-style body, `upgrades.ts:70-95`) | `Router{"/upgrades"}.post` (`routes.cppm:19,29`) | `handler.upgrades_create()` → `CreateHandler` (`routes.cppm:34`; `all.cppm:547`) | Ghost strict zod body `{upgrades:[{component,to_version,idempotency_key}]}`, unknown keys rejected, UUID-v4 lowercase key (`upgrades.ts:13-25,74-76`); must have `context.user` staff user (`upgrades.ts:80-83`). C++ raw `in.body_as_string()` straight to store, no validation, no user check (`create/.../execute.cppm:16`) | Ghost HTTP 202, `{upgrades:[job]}`, `location:false` (`upgrades.ts:71-72`; README:20); C++ status 200 OK (`create/handler.cppm:27`) and body `mutations.create(...).body()` = `{"upgrades":[{"id":"<n>"}]}`, id-only (`create/.../execute.cppm:16`; `context.cppm:1272-1276`) | Ghost `upgrades.service.createRequest({component,toVersion,idempotencyKey}, user)` with SHA-256 user-scoped key + host adapter (`upgrades.ts:85-92`; `upgrade-service.ts:105-130`); C++ `mutations.create("upgrades", body)` → `ResourceStore::create` generic JSON record (`create/.../execute.cppm:16`; `context.cppm:640-651,1272-1276`) | Ghost `headers.cacheInvalidate=false`, `location:false` (`upgrades.ts:71`), no cache service; C++ `ResourceStore::create` writes cache keys `resource:upgrades:<id>` and `resource:upgrades:ids`, TTL `never()`, tag `{"resources"}` (`context.cppm:645-649,725-728,747-756`) | Ghost `mw.authAdminApi` + `upgradeApiLimiter` + `upgradeLimiter`, `permissions:true`, staff-only user (`routes.js:522-526`; `upgrades.ts:77,80-83`); C++ token presence only → 401 (`create/handler.cppm:18-22`), no role/staff/rate-limit checks | MATCH | Ghost `routes.js:521-527`, `upgrades.ts:70-95`; C++ `routes.cppm:29,34`, `create/handler.cppm:17-28`, `create/.../execute.cppm:11-16` |
| GET `/upgrades/:id/` | `upgrades.read` (`upgrades.ts:97-108`) | `Route{"/:id"}.get` on `/upgrades` (`routes.cppm:37-39`) | `handler.upgrades_read_by_id()` → `ReadByIdHandler` (`routes.cppm:44`; `all.cppm:548`) | Ghost `options:['id']` + `parseJobId` UUID validation via `upgradeJobIdSchema` (`upgrades.ts:99-103,45-51`); C++ ignores `:id` entirely, no validation (`read_by_id/.../execute.cppm:11,16`) | Ghost `{upgrades:[job]}`, snake_case, adapter result incl. `unknown`/`expired` (README:21; `upgrades.ts:106`); C++ `{"upgrades":[]}` always — `find_by_resource("upgrades",...)` has no branch for `upgrades` and returns nullopt (`read_by_id/.../execute.cppm:16-18`; `context.cppm:1055-1092`) | Ghost `upgrades.service.getJob(id)` → host adapter, verifies returned id (`upgrades.ts:106`; `upgrade-service.ts:132-144`); C++ `ctx.store().find_by_resource("upgrades", "1")` hardcoded id `"1"` (`read_by_id/.../execute.cppm:16`) | Ghost `headers.cacheInvalidate=false` (`upgrades.ts:98`), no cache service; C++ `find_by_resource("upgrades",...)` returns nullopt before any cache access, so no cache interaction (`context.cppm:1055-1092`) | Ghost `mw.authAdminApi` + `upgradeApiLimiter`, `permissions:true` (`routes.js:528-533`; `upgrades.ts:104`); C++ token presence only → 401 (`read_by_id/handler.cppm:18-22`), no role/permission/rate-limit checks | MATCH | Ghost `routes.js:528-533`, `upgrades.ts:97-108`; C++ `routes.cppm:37-45`, `read_by_id/handler.cppm:17-28`, `read_by_id/.../execute.cppm:11-18` |

## Route details

### GET `/upgrades/` — MATCH (pre-rewrite analysis below)

- Route identity: Ghost `GET /upgrades/` (`routes.js:515-520`) vs C++ `Router{router,"/upgrades"}.get` (`routes.cppm:19,21`). Trailing slash normalized — agree.
- Operation: `browse` (`upgrades.ts:62`) → `handler.upgrades_browse()` (`routes.cppm:26`) — names agree.
- Output divergence: Ghost returns status via `getStatus()` → `toApi` → `{upgrades:[status]}` with no meta (`upgrades.ts:66`; README:19). C++ builds a fake pagination envelope with hardcoded `limit:15,page:1,pages:1,prev:null,next:null,total:0` (`browse/.../execute.cppm:17`). Materially different shape.
- DB divergence: Ghost calls upgrade adapter service (`upgrade-service.ts:89-103`); C++ reads generic `json_for("upgrades", {})` resource store (`browse/.../execute.cppm:16`; `context.cppm:1180-1181`). No real filtering/pagination — C++ writes constants.
- Cache: Ghost explicitly disables invalidation (`upgrades.ts:57,63`); C++ read path reads ResourceStore cache keys `resource:upgrades:*` (`context.cppm:704-715,725-728`). No Ghost-style `X-Cache-Invalidate` on either side.
- Auth: Ghost `authAdminApi` + `upgradeApiLimiter` + `permissions:true` (`routes.js:517-519`; `upgrades.ts:64`); C++ token-only check (`browse/handler.cppm:18-22`), no permission/role enforcement, no rate limiter.
- Verdict driver: resource + data shape materially different → MISMATCH.

#### Fix

- Checks 1, 2 (route identity, operation): no change — `GET /upgrades/` → `handler.upgrades_browse()` already matches (`routes.cppm:19-28`; registry `all.cppm:546`). No path parameter on this route.
- Check 3 (input): no change — Ghost `browse` declares no `options`/`validation` (`upgrades.ts:62-68`), and C++ `using Input=interfaces::io::IRequest&` (`browse/.../execute.cppm:11`) parses no query/body params. Both sides take no request-specific parameter.
- Check 4 (output): edit `include/blog/handlers/external/upgrades/browse/workflow/step/execute.cppm`, `upgrades::browse::workflow::ExecuteStep::invoke`, lines 16-17. Replace the fake `meta.pagination` envelope with the Ghost status-only envelope. Exact replacement (rows authored snake_case already match `toApi`, `upgrades.ts:54-55`):

  ```cpp
  const auto rows = ctx.store().json_for("upgrades", {});   // context.cppm:1123,1180-1181
  return core::workflow::StepResponse<Output>{"{\"upgrades\":" + rows + "}"};
  ```

  Seed one status row via `ctx.store().resources().create("upgrades", R"({"supported":true,"availability":"unavailable","one_click":[],"manual":[]})")` (`context.cppm:640-651`) so `rows` has the `upgradeStatusSchema` shape (`schemas.ts:43-66`) instead of `[]`. No `meta` key — Ghost `getStatus()` returns a bare status (`upgrades.ts:66`). `BrowseHandler::handle` already sends 200 (`browse/handler.cppm:27`): no change.
- Check 5 (DB call): same step. Keep resource string `"upgrades"` but make the call real — `json_for("upgrades", {})` reads `ResourceStore` rows via `m_resources.list("upgrades")` → `search_json` (`context.cppm:1180-1181`), not a dead fixture. Ghost's host adapter (`upgrade-service.ts:89-103`) has no allowed C++ primitive (no Ghost JS, third-party libs, or direct SQL); the context-local `ResourceStore` is the accepted stand-in. No new global state.
- Check 6 (cache): edit `browse/handler.cppm`, `BrowseHandler::handle`, before `send()` (`line 27`): `std::move(res).with_header("X-Cache-Invalidate", "false");` (`include/interfaces/io/response.cppm:133`), mirroring `noCacheInvalidation` (`upgrades.ts:57,63`). The read path already hits `resource:upgrades:*` keys via `m_resources.list` (`context.cppm:704-715,725-728`); no new cache namespace or TTL.
- Check 7 (auth): edit `routes.cppm:19-28`: add `.use(blog::handlers::middleware::require_auth)` beside `content_type_middleware` (`include/blog/handlers/middleware/auth.cppm:12-29`) and delete the inline `middleware::token(req).empty()` → `{"error":"Unauthorized"}` block in `browse/handler.cppm:18-22` so the Ghost error envelope is used. Ghost `upgradeApiLimiter` + `permissions:true` (`routes.js:517-519`; `upgrades.ts:64`) have no allowed C++ primitive — accepted residual; do not invent a global limiter.
- Check 8 (verdict/evidence): evidence already recorded on both sides (`routes.js:515-520`; `upgrades.ts:62-68`; `routes.cppm:19,21,26`; `browse/handler.cppm:17-28`; `browse/.../execute.cppm:11-17`). Superseded by the group 32 strict-parity pass at the end of this document: verdict flipped to MATCH after the rewrite. Checks 1–8 closed.

### POST `/upgrades/` — MATCH (pre-rewrite analysis below)

- Route identity: Ghost `POST /upgrades/` (`routes.js:521-527`) vs C++ `.post` on `/upgrades` (`routes.cppm:29`). Agree.
- Operation: `add` (`upgrades.ts:70`) → `handler.upgrades_create()` (`routes.cppm:34`) — names agree.
- Input divergence: Ghost strict zod schema rejects unknown keys and validates component/to_version/lowercase UUID-v4 idempotency_key (`upgrades.ts:13-25,74-76`) and requires a staff `context.user` (`upgrades.ts:80-83`). C++ forwards raw request body with no schema/UUID/user check (`create/.../execute.cppm:16`).
- Output divergence: Ghost 202 + `{upgrades:[job]}` (`upgrades.ts:71-72`; README:20); C++ 200 OK (`create/handler.cppm:27`) with generic id-only body `{"upgrades":[{"id":"<n>"}]}` (`create/.../execute.cppm:16`; `context.cppm:1272-1276`).
- DB divergence: Ghost `createRequest` hashes idempotency key with user id and calls host adapter, verifying accepted job matches (`upgrades.ts:85-92`; `upgrade-service.ts:105-130`). C++ `mutations.create("upgrades", body)` stores a generic JSON record (`context.cppm:640-651`).
- Cache: Ghost none (`upgrades.ts:71`); C++ writes cache keys `resource:upgrades:<id>` + `resource:upgrades:ids`, TTL never, tag `resources` (`context.cppm:645-649,747-756`).
- Auth: Ghost `authAdminApi` + both upgrade limiters + `permissions:true` + staff-only (`routes.js:522-526`; `upgrades.ts:77,80-83`); C++ token-only (`create/handler.cppm:18-22`).
- Verdict driver: operation semantics (host-adapter job creation with user-scoped idempotency) and data shape/status materially different → MISMATCH.

#### Fix

- Checks 1, 2: no change — `POST /upgrades/` → `handler.upgrades_create()` matches (`routes.cppm:19,29,34`; registry `all.cppm:547`).
- Check 3 (input): edit `include/blog/handlers/external/upgrades/create/workflow/step/execute.cppm`, `upgrades::create::workflow::ExecuteStep::invoke`, line 16. Replace raw `in.body_as_string()` with strict parsing: decode via `core_serde::JsonCodec{}.decode<serde::Value>(body)` (pattern in `membership/emails/dto/response.cppm:266`), require exactly one top-level key `upgrades`, an array of length 1, and an inner object with exactly `component`, `to_version`, `idempotency_key` (unknown keys rejected). Validate `component` against `^[a-z][a-z0-9-]{0,31}$` and `idempotency_key` as lowercase UUID-v4 (length 36, dashes at 8/13/18/23, `[14]=='4'`, `[19]∈{'8','9','a','b'}`, remaining chars lowercase hex) — `upgrades.ts:13-25`; manual char checks, no third-party regex/UUID lib. On failure return `MutationResult{422, ApiError{}.validation("Expected one update with component, to_version and idempotency_key.", "upgrades", "upgrades"), true}` body (`context.cppm:76-84`). Exact store call stays `mutations.create("upgrades", innerJson)` with `mutations` = `blog::context::ApiDataStore&` (`context.cppm:1206,1272`). Staff-user gate (`frame.options.context.user`, `upgrades.ts:80-83`) has no C++ request field — accepted residual; bearer presence is enforced in check 7. SHA-256 user-scoped idempotency key (`upgrade-service.ts:105-109`) needs `node:crypto` (Ghost JS), which has no allowed C++ primitive: store the client key verbatim in the context-local record and record as residual.
- Check 4 (output): edit `create/handler.cppm`, `CreateHandler::handle` (`line 27`): replace `Status::OK` with `Status::ACCEPTED` (`include/interfaces/io/types.cppm:21`) and pass the step body through. In the step, shape the created job as `{"upgrades":[{"id":"<id>","state":"pending","component":"<component>","to_version":"<to_version>","created_at":...,"updated_at":...}]}` from the validated input plus the `resources().create` id (`schemas.ts:94-103`, `toApi` at `upgrades.ts:54-55`); do not emit the generic id-only `{"upgrades":[{"id":"<n>"}]}` (`context.cppm:1272-1276`). `location:false` (`upgrades.ts:71`) means no `Location` header — emit none. Adapter job-field verification (`upgrade-service.ts:120-124`) has no C++ counterpart — residual.
- Check 5 (DB call): keep `mutations.create("upgrades", innerJson)` — resource string `"upgrades"` and `ApiDataStore::create` are correct (`context.cppm:1206,1272`); only the input/output shaping above changes. No direct SQL.
- Check 6 (cache): edit `create/handler.cppm`, before `send()`, add `std::move(res).with_header("X-Cache-Invalidate", "false");` (`response.cppm:133`; `upgrades.ts:57,71`). `ResourceStore::create` already writes `resource:upgrades:<id>` + `resource:upgrades:ids`, TTL `never()`, tag `{"resources"}` (`context.cppm:640-651,725-728,747-756`) — existing context-local cache, no new namespace or state.
- Check 7 (auth): edit `routes.cppm:29-36`: add `.use(blog::handlers::middleware::require_auth)` and delete the inline 401 block in `create/handler.cppm:18-22` (`auth.cppm:12-29`). `upgradeApiLimiter` + `upgradeLimiter` + `permissions:true` + staff-only (`routes.js:523-526`; `upgrades.ts:77,80-83`) have no allowed C++ primitive — accepted residual.
- Check 8 (verdict/evidence): evidence already recorded on both sides (`routes.js:521-527`; `upgrades.ts:70-95`; `routes.cppm:29,34`; `create/handler.cppm:17-28`; `create/.../execute.cppm:11-16`). Verdict flipped to MATCH by the group 32 strict-parity pass at the end of this document. Checks 1–8 closed.

### GET `/upgrades/:id/` — MATCH (pre-rewrite analysis below)

- Route identity: Ghost `GET /upgrades/:id/` (`routes.js:528-533`) vs C++ `Route{"/:id"}.get` (`routes.cppm:37-39`). Placeholder normalized — agree.
- Operation: `read` (`upgrades.ts:97`) → `handler.upgrades_read_by_id()` (`routes.cppm:44`) — names agree.
- Input divergence: Ghost validates `id` as an adapter job UUID (`upgrades.ts:99-103,45-51`); C++ never reads `:id` and calls `find_by_resource("upgrades", "1")` with a hardcoded `"1"` (`read_by_id/.../execute.cppm:16`).
- Output divergence: Ghost returns `{upgrades:[job]}` for real/unknown/expired lookups (README:21; `upgrades.ts:106`); C++ always returns `{"upgrades":[]}` because `find_by_resource` has no `upgrades` branch and returns `nullopt` (`read_by_id/.../execute.cppm:17-18`; `context.cppm:1091`).
- DB divergence: Ghost `getJob(id)` calls the host adapter and verifies id (`upgrade-service.ts:132-144`); C++ dead generic-store lookup.
- Cache: Ghost none (`upgrades.ts:98`); C++ no cache access for `upgrades` in `find_by_resource` (`context.cppm:1055-1092`).
- Auth: Ghost `authAdminApi` + `upgradeApiLimiter` + `permissions:true` (`routes.js:530-532`; `upgrades.ts:104`); C++ token-only (`read_by_id/handler.cppm:18-22`).
- Verdict driver: DB call and data shape materially different (id ignored, always empty) → MISMATCH.

#### Fix

- Checks 1, 2: no change — `GET /upgrades/:id/` → `Route{"/:id"}.get` → `handler.upgrades_read_by_id()` matches after placeholder normalization (`routes.cppm:37-45`; registry `all.cppm:548`).
- Check 3 (input): edit `include/blog/handlers/external/upgrades/read_by_id/workflow/step/execute.cppm`, `upgrades::read_by_id::workflow::ExecuteStep::invoke`, line 16. Replace the hardcoded `std::string{}.empty()?"1":std::string{}` with the `:id` segment parsed from `in.get_path()` (last `/`-separated segment; same pattern as `posts/browse/.../execute.cppm:22-28`). Validate against the `upgradeJobIdSchema` equivalent — `^[a-z0-9][a-z0-9-]{0,63}$` (`schemas.ts:6`): non-empty, first char `[a-z0-9]`, remaining chars `[a-z0-9-]`, length ≤ 64. On invalid id return `ApiError{}.validation("Expected a job id from this host.", "upgrades", "id")` with status 422 (`context.cppm:76-84`) without calling the store.
- Check 5 (DB call): add an `"upgrades"` branch to `blog::context::ApiDataStore::find_by_resource` (`context.cppm:1055-1092`, before the final `return std::nullopt;` at line 1091):

  ```cpp
  if (resource == "upgrades") {
      const auto stored = m_resources.get(resource, id);   // context.cppm:698-702
      if (stored.empty()) return std::nullopt;
      return stored;
  }
  ```

  Then the exact replacement call in the step is `ctx.store().find_by_resource("upgrades", id)` (`context.cppm:1055`), not the hardcoded `"1"`. Seed jobs via `ctx.store().resources().create("upgrades", jobJson)` (`context.cppm:640-651`). Ghost `getJob(id)` through the host adapter (`upgrade-service.ts:132-144`) has no allowed C++ primitive — context-local `ResourceStore` is the accepted stand-in; no SQL or third-party libs.
- Check 4 (output): in the same step, replace lines 17-18. On a hit emit `{"upgrades":[` + `*one` + `]}`. On `nullopt`, do not return `{"upgrades":[]}`: emit the adapter's unknown-result shape `{"upgrades":[{"id":"<id>","state":"unknown"}]}` (`schemas.ts:106-111`) so unknown/expired lookups still return one job (`upgrades.ts:106`). `ReadByIdHandler::handle` already sends 200 (`read_by_id/handler.cppm:27`): no change. The adapter id cross-check (`upgrade-service.ts:136-138`) is covered by reading the row keyed by the same parsed `id`.
- Check 6 (cache): edit `read_by_id/handler.cppm`, before `send()`, add `std::move(res).with_header("X-Cache-Invalidate", "false");` (`response.cppm:133`; `upgrades.ts:57,98`). The new `find_by_resource` branch reads the existing `resource:upgrades:<id>` key via `m_resources.get` (`context.cppm:698-702,725-728`) — context-local cache under the existing namespace, no new TTL or global state.
- Check 7 (auth): edit `routes.cppm:37-45`: add `.use(blog::handlers::middleware::require_auth)` and delete the inline 401 block in `read_by_id/handler.cppm:18-22` (`auth.cppm:12-29`). `upgradeApiLimiter` + `permissions:true` (`routes.js:530-532`; `upgrades.ts:104`) have no allowed C++ primitive — accepted residual.
- Check 8 (verdict/evidence): evidence already recorded on both sides (`routes.js:528-533`; `upgrades.ts:97-108`; `routes.cppm:37-45`; `read_by_id/handler.cppm:17-28`; `read_by_id/.../execute.cppm:11-18`). Verdict flipped to MATCH by the group 32 strict-parity pass at the end of this document. Checks 1–8 closed.

## Missing in C++

None. All three Ghost controller keys (`browse`, `add`, `read`) have C++ routes bound to a handler:
- `browse` → `routes.cppm:21,26`
- `add` → `routes.cppm:29,34`
- `read` → `routes.cppm:37-39,44`

Confirmed by grep `upgrade` (case-insensitive) over `include/blog/handlers/**` and by reading `include/blog/handlers/external/upgrades/routes.cppm` in full (only three routes installed).

  Fix: No change required. All three Ghost `api.upgrades.*` keys already have a C++ route plus handler (`browse` → `routes.cppm:21,26`; `add` → `routes.cppm:29,34`; `read` → `routes.cppm:37-39,44`), confirmed by the grep above. Check 1 is closed; the three route-level `#### Fix` blocks above reshape input/output/DB/cache/auth without adding or removing a route. No new handler, no `core::cache::Cache` instance beyond the context-local one, no new global state.

## Extra in C++

None. `include/blog/handlers/external/upgrades/routes.cppm` installs exactly three routes (`/upgrades` GET/POST, `/upgrades/:id` GET), each matching a Ghost controller key. No additional upgrades route exists in that folder; grep `upgrade` across `include/blog/handlers/**` found only this route file plus central registration (`routes.cppm:54,127`, `all.cppm:200-202,546-548,795-797`, `base.cppm:210-212`, `BUILD.bazel:584-592,834`).

  Fix: No change required. `blog::api::UpgradesRoutes::install` (`routes.cppm:15-48`) registers exactly the three counterparts of the Ghost controller keys; no C++ upgrades route is EXTRA. Do not delete or gate any route. Checks 1–8 for this section are closed by the three route-level `#### Fix` blocks above.

---

## Strict-parity pass (group 32, 2026-11-20)

The pre-rewrite Fix blocks above are historical. The implemented C++ state is
summarized here; full Ghost-side evidence and residual-gap detail live in
`docs/parity/behavior/upgrades.md`.

### Implemented changes

- `routes.cppm` — removed router-level `content_type_middleware` and `require_auth`;
  Ghost's admin app parses bodies globally and the shared permission gate is applied
  in each handler. Route paths/verbs unchanged (no MISSING/EXTRA).
- `browse/workflow/step/execute.cppm` — added the `blog::handlers::upgrades::support`
  namespace in this already-registered partition: `core_auth:ghost` principal and
  `permissions:true` check for docName `upgrades`, Ghost error envelopes, exact
  regex validators, std-only SHA-256, the host-adapter stand-in and a local `wrap`
  shim. Step `Output` is `std::map<std::string,std::string>` (`status`, `body`);
  `s_ctx` retained. The old fixture seed and injected `id`/`meta.pagination` are gone;
  default status is Noop `{supported:false,reason:'not-configured'}`.
- `create/workflow/step/execute.cppm` — strict body validation (exact keys, component
  regex, opaque 1–128 char version, lowercase UUID v4), `403` staff-user gate,
  SHA-256 user-scoped idempotency key, adapter error taxonomy and `202` job envelope.
  Accepted jobs are emitted as `upgrade.create_request` events on the registered
  `blog_handlers:outbox_shim`; no `mutations.create("upgrades", …)` record, no raw key
  or fabricated timestamps.
- `read_by_id/workflow/step/execute.cppm` — `:id` regex validation (`422`), adapter
  union (`full job | unknown | expired`), returned-id cross-check, `502` on adapter
  failure; no local `ResourceStore` read and no leaked `idempotency_key`.
- Three `handler.cppm` files — 401/403 through
  `blog::handlers::middleware::require_ghost_permission`, HTTP status read from the
  step's `Output` map (fixes the former 202-on-validation-error and 200-on-invalid-id
  divergences), app-level `Cache-Control`, and no `X-Cache-Invalidate` / `Location`.

### Verdicts

| Route | Old | New | Reason |
|---|---|---|---|
| GET `/upgrades/` | MISMATCH | MATCH | Noop status shape, `permissions:true`, 200, no invalidation header/meta, no fixture write |
| POST `/upgrades/` | MISMATCH | MATCH | 422/403/409/422/502/202 mapping, hashed scoped key, outbox acceptance, no leaked key/local record |
| GET `/upgrades/:id/` | MISMATCH | MATCH | 422 invalid id, adapter union incl. `expired`, id cross-check, 502 failures, no key leak |

### New-partition / new-dependency needs (central pass)

- `core_adapters:upgrades` — real host `UpgradeBase` adapter plus error taxonomy.
- Rate-limit primitive with `brute` table writes, IP/user keys, `Retry-After` and 429.
- Register `blog_handlers:mapping` (replace local `wrap`) and `utils:hash` (replace
  inlined SHA-256) as dependencies of `blog_handlers`.
- Integration-token flag on `core_auth:ghost::Principal` for `tokenPermissionCheck`.
- Existing `//include/blog/handlers:blog_handlers` dependency cycle and unrelated
  in-flight compile errors must be fixed centrally before full Bazel verification;
  this group did not edit shared BUILD files.

### Verification note

Clang compilation of the changed partitions produced no diagnostics for this folder.
Full `bazel build //include/blog/handlers:blog_handlers` remains blocked by the
pre-existing module dependency cycle and unrelated in-flight errors in other folders;
no shared file was edited to work around it.
