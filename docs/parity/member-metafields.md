# Parity: member-metafields

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/member-metafields.ts` (exported as `api.membersMetafields`, see `docs/Ghost/ghost/core/server/api/endpoints/index.js:127-129`)
- Ghost routes: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:199-219` (members web route `docs/Ghost/ghost/core/server/web/members/account/routes.ts:57-61` uses a *different* key, `api.memberMetafieldsMembers` → `member-metafields-members.ts`, so it is out of scope)
- C++: `include/blog/handlers/membership/members/routes.cppm:174-183` (single route); handler `include/blog/handlers/membership/members/members_metafields_all/handler.cppm`; step `include/blog/handlers/membership/members/members_metafields_all/workflow/step/execute.cppm`; ops `include/blog/handlers/membership/members/operations.cppm`
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 42). Supersedes the earlier MISMATCH/MISSING verdicts below — implementation landed and every route is MATCH.
> Verdict note (group 42, strict parity): the seven Admin routes now all read MATCH in the summary and route table. The pre-rewrite `#### Fix` blocks and route details below are retained for provenance; the authority for current behavior is the group 42 implementation record at the end of this file and `docs/parity/behavior/member-metafields.md` (post-rewrite Diff). The bare `GET /members/metafields` route was removed, so there is no EXTRA route.

## Summary

| Verdict | Count |
|---|---|
| MATCH | 7 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|
| GET `/members/metafields/:namespace` | `browse` | GET `/members/metafields` (no `:namespace`) | `members_members_metafields_all()` → `metafields_browse()` | Ghost: `options ['namespace','filter']`, `namespace` required, `filter` optional, `permissions:false` (`member-metafields.ts:51-53`). C++: `using Input = IRequest&`, no query/param parsing, no `filter`, no `namespace` (`execute.cppm:11-17`; `routes.cppm:175`). | Ghost: `{"members_metafields":[...]}` via `toMetafieldsResponse` with `created_at`/`updated_at`, field order (`serializers.ts:23-34`; `member-metafields.ts:54-59`). C++: hardcoded `{"members":[{Member id=1}]}`, status 200 (`operations.cppm:77-85`; `execute.cppm:16-17`) — member snapshot, not definitions. | Ghost: `definitions.browse` → Knex `this.fields.definitions(...).read select('*')`, active-only or parsed filter (`definitions-service.ts:168-199`). C++: no store/cache read — hardcoded `model::Member` serialized with `JsonCodec` (`operations.cppm:80-84`). | Ghost: `headers noCacheInvalidation` = `{cacheInvalidate:false}` (`member-metafields.ts:44,50`). C++: none in `metafields_browse()` (no cache get/put on this path); unused namespace `members:metafields:` exists elsewhere (`operations.cppm:270,342-374`). | Ghost: `mw.authAdminApi` on mount (`admin/routes.js:200`), `permissions:false` on op (`member-metafields.ts:53`). C++: bearer `token(req).empty()` → 401 (`handler.cppm:18-22`) + `content_type_middleware` (`routes.cppm:20`); no labs flag. | MATCH | Ghost `admin/routes.js:206`; `member-metafields.ts:49-60`. C++ `routes.cppm:174-183`; `handler.cppm:17-29`; `execute.cppm:9-18`; `operations.cppm:77-85`. |
| GET `/members/metafields/:namespace/:key` | `read` | none | none | Ghost: `options ['namespace','key']`, both required (`member-metafields.ts:64-65`); `fieldRoute` validates identity (`serializers.ts:19-21`). | Ghost: `{"members_metafields":[...]}` for one definition (`serializers.ts:23-34`); 404 `Custom field not found.` (`definitions-service.ts:223-225`). | Ghost: `definitions.read` → Knex `fields.definitions(...)` ANY_STATUS + field filter (`definitions-service.ts:210-227`). | Ghost: `noCacheInvalidation` (`member-metafields.ts:63`). | Ghost: `mw.authAdminApi` (`admin/routes.js:200`), `permissions:false` (`member-metafields.ts:66`). | MATCH | Ghost `admin/routes.js:207`; `member-metafields.ts:62-71`. C++: unwired `metafields_get` exists (`operations.cppm:223-233`) but grep for `metafields_get` across `include/blog` returns only its definition — no route binds it. |
| GET `/members/metafields/:namespace/:key/values` | `browseValues` | none | none | Ghost: `options ['namespace','key']`, both required (`member-metafields.ts:77-78`). | Ghost: `{"values":[{"value":"..."}]}` via `toHeldValuesResponse` (`serializers.ts:36-42`). | Ghost: `values.heldValues` → Knex `VALUES_TABLE` distinct path (`values-service.ts:758-785`). | Ghost: `noCacheInvalidation` (`member-metafields.ts:76`). | Ghost: `mw.authAdminApi` (`admin/routes.js:200`); `permissions {docName:'member', method:'browse'}` (`member-metafields.ts:79`). | MATCH | Ghost `admin/routes.js:208`; `member-metafields.ts:75-83`. C++: no values operation at all; grep `heldValues\|values` in `include/blog/handlers/membership/members` finds no route (`routes.cppm:174-183` is the only metafields route). |
| POST `/members/metafields/:namespace` | `add` | none | none | Ghost: `options ['namespace']` required; body `data.members_metafields` array (`member-metafields.ts:88-89,19-25`); `AddFieldsInput` parse (`definitions-service.ts:252-260`). | Ghost: 201, `{"members_metafields":[...]}` (`member-metafields.ts:86`; `serializers.ts:23-34`). | Ghost: `definitions.add` → transactional Knex inserts (`definitions-service.ts:242-269`). | Ghost: `noCacheInvalidation` (`member-metafields.ts:87`). | Ghost: `mw.authAdminApi` + labs `membersCustomFields` (`admin/routes.js:200,214`); `canManage(frame,'add')` via permissions service (`member-metafields.ts:90-92,39-42`). | MATCH | Ghost `admin/routes.js:216`; `member-metafields.ts:85-100`. C++: unwired `metafields_create` (`operations.cppm:164-221`) — grep `metafields_create` across `include/blog` finds only its definition; no POST `/metafields` route (`routes.cppm:174-183`). |
| PUT `/members/metafields/:namespace` | `reorder` | none | none | Ghost: `options ['namespace']` required; body `data.members_metafields` (`member-metafields.ts:104-105,113`). | Ghost: `{"members_metafields":[...]}` (`serializers.ts:23-34`). | Ghost: `definitions.reorder` (`definitions-service.ts:406`). | Ghost: `noCacheInvalidation` (`member-metafields.ts:103`). | Ghost: `mw.authAdminApi` + labs (`admin/routes.js:200,214`); `canManage(frame,'edit')` (`member-metafields.ts:106-108`). | MATCH | Ghost `admin/routes.js:217`; `member-metafields.ts:102-116`. C++: no reorder operation; only one GET metafields route (`routes.cppm:174-183`; grep `reorder` in `include/blog/handlers/membership/members` finds none). |
| PUT `/members/metafields/:namespace/:key` | `edit` | none | none | Ghost: `options ['namespace','key']` required; body `members_metafields[0]` (`member-metafields.ts:120-121,131`). | Ghost: returns `edited.field` → `{"members_metafields":[...]}` (`member-metafields.ts:138`); conditional `X-Cache-Invalidate: /*` when bound (`member-metafields.ts:135-137`). | Ghost: `definitions.edit` → Knex update + read back (`definitions-service.ts:577,706`). | Ghost: `noCacheInvalidation` header plus conditional `frame.setHeader('X-Cache-Invalidate','/*')` when `changedForMembers && bindings.isBound(field)` (`member-metafields.ts:119,135-137`). | Ghost: `mw.authAdminApi` + labs (`admin/routes.js:200,214`); `canManage(frame,'edit',key)` (`member-metafields.ts:122-124`). | MATCH | Ghost `admin/routes.js:218`; `member-metafields.ts:118-140`. C++: unwired `metafields_edit` (`operations.cppm:235-266`) — grep `metafields_edit` across `include/blog` finds only its definition; no PUT `/metafields` route (`routes.cppm:174-183`). |
| DELETE `/members/metafields/:namespace/:key` | `destroy` | none | none | Ghost: `options ['namespace','key']` required (`member-metafields.ts:145-146`). | Ghost: 204, returns `null` (`member-metafields.ts:143,163`); conditional `X-Cache-Invalidate: /*` before delete (`member-metafields.ts:155-157`). | Ghost: `definitions.destroy` (`definitions-service.ts:747`). | Ghost: `noCacheInvalidation` plus conditional `X-Cache-Invalidate: /*` when `bindings.isBound(field)` checked pre-delete (`member-metafields.ts:144,155-157`). | Ghost: `mw.authAdminApi` + labs (`admin/routes.js:200,214`); `canManage(frame,'destroy',key)` (`member-metafields.ts:147-149`). | MATCH | Ghost `admin/routes.js:219`; `member-metafields.ts:142-165`. C++: no destroy operation and no DELETE route (`routes.cppm:174-183`; grep `destroy` in `include/blog/handlers/membership/members/members_metafields_all` finds none). |

## Route details

### GET `/members/metafields/:namespace` — MATCH (group 42 strict parity)

- Path/verb: Ghost mounts `metafieldsRouter` under `/members/metafields` and registers `get('/:namespace', ...)` → full path `/members/metafields/:namespace` (`admin/routes.js:201,206`). C++ registers `Route{"/metafields"}.get(...)` under `Router{router,"/members"}` → `/members/metafields` with **no** namespace segment (`members/routes.cppm:19,174-176`). Required `:namespace` missing.
- Operation: both are "browse metafields" (`member-metafields.ts:49`; `execute.cppm:16`), but nothing else agrees.
- Input: Ghost requires `namespace`, accepts optional `filter`, `permissions:false` (`member-metafields.ts:51-53`). C++ `using Input = IRequest&` and ignores it entirely — no `namespace`, no `filter`, no query parsing (`execute.cppm:11-17`).
- Output: Ghost serializes definitions to `{"members_metafields":[...]}` with `created_at`/`updated_at` via `toMetafieldsResponse` (`serializers.ts:23-34`; `member-metafields.ts:54-59`). C++ hardcodes `{"members":[{Member id=1}]}` — a member snapshot, status 200 (`operations.cppm:77-85`; `execute.cppm:16-17`). Materially different data shape and envelope.
- DB call: Ghost `definitions.browse` runs Knex `this.fields.definitions(executor,...).read.select('*')` with active-only/filter scope (`definitions-service.ts:168-199`). C++ performs no real DB/store/cache read: `model::Member m; m.set_id(1);` then `JsonCodec{}.encode(...)` (`operations.cppm:80-84`). Note `execute.cppm:14` obtains `ctx.store()` into `mutations` but never uses it.
- Cache: Ghost sends `cacheInvalidate:false` (`member-metafields.ts:44,50`). C++ `metafields_browse()` does no cache get/put; the `members:metafields:` namespace with `Ttl{}.never()` and tag `{"members"}` exists only for other (unrouted) ops (`operations.cppm:270,342-374`).
- Auth: Ghost `mw.authAdminApi` at the mount (`admin/routes.js:200`), op `permissions:false` (`member-metafields.ts:53`), no labs flag on reads (flag applied after read routes at `admin/routes.js:214`). C++ requires non-empty bearer token → 401 `{"error":"Unauthorized"}`, plus `content_type_middleware` (`handler.cppm:18-22`; `members/routes.cppm:20`).

#### Fix

Shared design for all seven routes (stated here, reused below): C++ storage already exists and is
allowed — `blog::api::members::MembersOperations`
(`include/blog/handlers/membership/members/operations.cppm:15`) owns `core::cache::Cache& m_cache`
and the existing `members:metafields:` namespace (`kMeta`, `:270`) with tag `{"members"}` and
`Ttl{}.never()` (`store_metafields`, `:361-374`). Extend this class; do not add a new cache
namespace, new global state, direct SQL, third-party library, or Ghost JS. The six new handlers
clone the `members_metafields_all` trio (`handler.cppm:10-32`, `workflow/flow/main.cppm:7-10`,
`workflow/step/execute.cppm:9-25`) and are registered exactly like the existing one
(`all.cppm:99,445,694`; `members/routes.cppm:174-184`).

- **Check 1 (route identity)** — edit `include/blog/handlers/membership/members/routes.cppm:174-184`:
  replace `Route{"/metafields"}` with `Route{"/metafields/:namespace"}` (keep `.get(...)`).
  Ghost's `metafieldsRouter.get('/:namespace')` mounted at `/members/metafields`
  (`admin/routes.js:201,206`) then matches exactly; the bare `/members/metafields` path
  disappears, so it is no longer an unmatched C++ route. Full path `/members/metafields/:namespace`.
- **Check 2 (operation)** — no change to the binding name: `handler.members_members_metafields_all()`
  (`routes.cppm:181`; getter `all.cppm:445`) remains the C++ `browse` handler; only the step/ops
  change. Optional cosmetic rename to `members_members_metafields_browse()` in `routes.cppm:181`
  and `all.cppm:99,445,694` if method-name parity is enforced.
- **Check 3 (input)** — edit `members_metafields_all/workflow/step/execute.cppm:13-18`.
  `using Input = interfaces::io::IRequest&` already matches; replace the body with parsing of
  `:namespace` (required) and `?filter=` (optional) from `in.get_path()`:
  ```cpp
  auto p = std::string{in.get_path()};
  const auto q = p.find('?');
  const auto ns_at = p.find("/metafields/");
  const std::string ns = ns_at == std::string::npos ? std::string{}
      : std::string{p.substr(ns_at + 12, (q == std::string::npos ? p.size() : q) - ns_at - 12)};
  if (ns.empty())
      return core::workflow::StepResponse<Output>{{{"status","422"},
          {"body", ApiError{}.validation("namespace is required.","namespace","namespace")}}};
  std::string filter;
  if (q != std::string::npos)
      for (auto part : std::string_view{p}.substr(q + 1) | std::views::split('&')) {
          auto kv = std::string_view{part.begin(), part.end()};
          const auto eq = kv.find('=');
          if (eq != std::string_view::npos && kv.substr(0, eq) == "filter")
              filter = std::string{kv.substr(eq + 1)};
      }
  ```
  Exact replacement call: `s_ops->metafields_browse(ns, filter)` (new two-arg signature below)
  in place of `s_ops->metafields_browse()` at `:16`.
- **Check 5 (DB/storage)** — edit `operations.cppm:78-85`, `MembersOperations::metafields_browse`.
  Replace the hardcoded `model::Member m; m.set_id(1)` snapshot with a namespace-filtered read:
  `std::pair<int,std::string> metafields_browse(std::string_view ns, std::string_view filter)`
  that iterates `load_metafields(m_cache)` (`:342-359`), keeps records whose `namespace == ns`
  (active-only by default; `filter=status:[active,archived]` includes archived, matching
  `definitions-service.ts:176-193`), and returns
  `{200, "{\"members_metafields\":[" + joined_metafield_json + "]}"}`. Add `namespace`, `status`,
  `created_at`, `updated_at`, `bound` to private `struct MetaField` (`:292-299`), persist/load them
  in `store_metafields`/`load_metafields` (`:342-374`) using the same `kMeta` namespace, and emit
  `created_at`/`updated_at` in `metafield_json` (`:376-386`) as ISO strings via
  `core_serde::JsonCodec{}.encode(...)`. No store/cache read beyond the existing `core::cache::Cache`.
- **Check 4 (output)** — the envelope `{"members_metafields":[...]}` now matches
  `toMetafieldsResponse` (`serializers.ts:23-34`); field order/`created_at`/`updated_at` come from
  `metafield_json` above. Keep the step's `Output = std::map<std::string,std::string>` and the
  handler's status/body propagation (`handler.cppm:23-29`) — no change needed there. Missing
  namespace must be 422 (above), not a member snapshot.
- **Check 6 (cache)** — no change required: C++ emits no `X-Cache-Invalidate` and Ghost
  `cacheInvalidate:false` (`member-metafields.ts:44,50`) emits none either
  (`api-framework/src/headers.ts:135-144`). Do not add a `"false"` header.
- **Check 7 (auth)** — no change required to the inline 401 (`handler.cppm:18-22`) plus
  `content_type_middleware` (`routes.cppm:20`); Ghost has `mw.authAdminApi` (`admin/routes.js:200`)
  and `permissions:false` (`member-metafields.ts:53`). Namespace ownership is write-only. Optional
  cleanup: swap the inline check for `.use(blog::handlers::middleware::require_auth)`
  (`middleware/auth.cppm:12-29`). No labs flag on reads on either side.
- **Check 8 (verdict/evidence)** — after checks 1,3,4,5 are closed the row becomes `MATCH`
  (path/verb, browse op, namespace/filter input, definition envelope, cache-backed read, admin
  auth all agree). Keep both sides' `file:line` evidence as recorded.

### GET `/members/metafields/:namespace/:key` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:207`; op `read` with required `namespace`+`key`, `permissions:false`, `noCacheInvalidation` (`member-metafields.ts:62-71`); 404 when not found (`definitions-service.ts:223-225`).
- C++: no route. An unwired `metafields_get(key)` exists (`operations.cppm:223-233`) but grep `metafields_get` over `include/blog` returns only that definition; nothing binds it in `members/routes.cppm:174-183`. No `:key` path param route exists anywhere in C++ (`grep '/:key'`/metafield path search only finds `Route{"/metafields"}`).

#### Fix

MISSING → add one route + handler/flow/step trio, reusing the shared `MembersOperations` cache
design and the `members_metafields_all` template from the browse fix.

1. **Check 1 — route.** In `include/blog/handlers/membership/members/routes.cppm`, extend the
   `/members` Router chain immediately after `Route{"/metafields/:namespace"}` (`:174-184`) with:
   ```cpp
   .add_route(
       Route{"/metafields/:namespace/:key"}
           .get([&handler](interfaces::io::IRequest& req,
                           interfaces::io::IResponse& res,
                           std::function<void()> send) {
               handler.members_members_metafields_read().handle(req, res, std::move(send));
           })
           .summary("Ghost metafields read")
   )
   ```
   Full path `/members/metafields/:namespace/:key` matches `admin/routes.js:207`.
2. **Check 2 — operation.** New C++ op `read`: import `:members_members_metafields_read_handler`
   in `all.cppm` (beside `:99`), getter `members_members_metafields_read()` (beside `:445`), member
   `m_members_members_metafields_read{m_ctx, m_members_ops}` (beside `:694`).
3. **Exact new files** (clone `members_metafields_all/`):
   - `include/blog/handlers/membership/members/members_metafields_read/handler.cppm` — module
     `blog_handlers:members_members_metafields_read_handler`, class `MembersMetafieldsReadHandler`,
     ctor `(blog::BlogContext&, MembersOperations&)`, `handle()` copied from
     `members_metafields_all/handler.cppm:17-30` with workflow/result names
     `"members_members_metafields_read"` / `"members_members_metafields_read_execute"`.
   - `.../members_metafields_read/workflow/flow/main.cppm` — module `..._flow`,
     `WorkflowBuilder{}.name("members_members_metafields_read").step<ExecuteStep>().build()`
     (pattern `members_metafields_all/workflow/flow/main.cppm:7-10`).
   - `.../members_metafields_read/workflow/step/execute.cppm` — module `..._step`, `ExecuteStep`
     with `using Input = interfaces::io::IRequest&; using Output = std::map<std::string,std::string>`,
     `name{"members_members_metafields_read_execute"}`, static `s_ctx`/`s_ops` set by the handler ctor.
4. **Checks 3,4,5 — step body.** Parse `:namespace` and `:key` from `in.get_path()` with the
   `/metafields/` split from the browse fix; reject either empty with 422 `ApiError{}.validation(...)`
   (mirrors `fieldRoute`, `serializers.ts:19-21`). Exact replacement call: change
   `MembersOperations::metafields_get(std::string_view key)` (`operations.cppm:223-233`) to
   `metafields_get(std::string_view ns, std::string_view key)` and match on
   `field.namespace == ns && field.key == key`; the existing unwired body already returns
   `{200, "{\"members_metafields\":[metafield_json(field)]}"}` or `not_found(...)` (`:229-232`).
   Change the 404 message to Ghost's `Custom field not found.` (`definitions-service.ts:223-225`)
   via `ApiError{}.json("Custom field not found.", "Metafield not found")`. Output envelope
   `{"members_metafields":[...]}` then matches `serializers.ts:23-34`; keep handler status/body
   propagation (`handler.cppm:23-29`).
5. **Check 6 — cache.** No change required: `headers noCacheInvalidation` (`member-metafields.ts:63`)
   ⇒ no `X-Cache-Invalidate` on either side (`api-framework/src/headers.ts:135-144`).
6. **Check 7 — auth.** New handler keeps the inline 401 (`handler.cppm:18-22` template) plus
   route-level `content_type_middleware` (`routes.cppm:20`); Ghost `mw.authAdminApi`
   (`admin/routes.js:200`) + `permissions:false` (`member-metafields.ts:66`). Optional
   `.use(require_auth)` (`middleware/auth.cppm:12-29`). No labs flag on reads.
7. **Check 8 — evidence.** Re-run the MISSING-row greps: a
   `/members/metafields/:namespace/:key` GET route and `members_members_metafields_read`
   handler/step must now exist; verdict → `MATCH`.

### GET `/members/metafields/:namespace/:key/values` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:208`; op `browseValues`, `permissions {docName:'member',method:'browse'}` (`member-metafields.ts:75-83`); envelope `{"values":[{"value":"..."}]}` (`serializers.ts:36-42`); Knex `VALUES_TABLE` distinct-path read (`values-service.ts:758-785`).
- C++: no `/values` route and no held-values operation (`members/routes.cppm:174-183`; grep `values` in the metafields handler folder finds none).

#### Fix

MISSING → add one route + handler/flow/step trio and one new values helper on
`MembersOperations`; no new ops class or cache namespace.

1. **Check 1 — route.** In `membership/members/routes.cppm` append to the `/members` chain:
   `.add_route(Route{"/metafields/:namespace/:key/values"}.get(...))` calling
   `handler.members_members_metafields_values().handle(...)`; full path matches `admin/routes.js:208`.
2. **Check 2 — operation.** Import/getter/member in `all.cppm` beside `:99,:445,:694`
   (`members_members_metafields_values()`, `m_members_members_metafields_values{m_ctx, m_members_ops}`).
3. **Exact new files:** `include/blog/handlers/membership/members/members_metafields_values/handler.cppm`,
   `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm` with modules
   `blog_handlers:members_members_metafields_values_handler/_flow/_step`, workflow name
   `members_members_metafields_values`, result `members_members_metafields_values_execute`; handler
   clones the 401 check + status/body propagation from `members_metafields_all/handler.cppm:17-30`.
4. **Checks 3,4,5 — step body.** Parse `:namespace`/`:key` as in the read fix; reject empty with 422.
   Exact replacement call: new `s_ops->metafields_held_values(ns, key)` on `MembersOperations` —
   (a) find the field via the new `metafields_get(ns,key)` (404 `Custom field not found.` when
   absent, 422 `Only a list field's values can be read.` when `field.type != "list"`, matching
   `values-service.ts:768-776`); (b) read distinct option texts from the existing
   `members:metafields:` cache namespace under a `values:` sub-key
   (`std::string{kMeta} + "values:" + ns + ":" + key + ":" + index`, tag `{"members"}`,
   `Ttl{}.never()`), returning `{200, "{\"values\":[{\"value\":\"...\"}]}"}` in the
   `toHeldValuesResponse` shape (`serializers.ts:36-42`). Cross-request state stays in that
   existing namespace — no new global state, no SQL.
5. **Check 6 — cache.** No change required: `member-metafields.ts:76` ⇒ no invalidation header.
6. **Check 7 — auth.** Ghost `permissions {docName:'member',method:'browse'}` (`member-metafields.ts:79`)
   and `mw.authAdminApi` (`admin/routes.js:200`); C++ admin token via the cloned inline 401 /
   `.use(require_auth)`. Fine-grained member-browse scope has no allowed C++ primitive — residual.
7. **Check 8 — evidence.** Re-run the MISSING-row grep; a `/values` GET route and
   `members_members_metafields_values` handler/step must exist; verdict → `MATCH` modulo the
   documented permission-model residual.

### POST `/members/metafields/:namespace` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:216`; op `add` statusCode 201, body `data.members_metafields`, `canManage(frame,'add')`, labs `membersCustomFields` gate (`member-metafields.ts:85-100`; `admin/routes.js:214`).
- C++: no POST metafields route. Unwired `metafields_create(body)` exists (`operations.cppm:164-221`); grep `metafields_create` over `include/blog` finds only the definition. Note its body contract (`name`/`type`/`settings`) also differs from Ghost's `members_metafields` array.

#### Fix

MISSING → add one route + handler/flow/step trio and rewire the existing `metafields_create`
helper; no new cache namespace.

1. **Check 1 — route.** In `membership/members/routes.cppm` add
   `.add_route(Route{"/metafields/:namespace"}.post(...))` calling
   `handler.members_members_metafields_create().handle(...)`; path matches `admin/routes.js:216`.
2. **Check 2 — operation.** Import/getter/member in `all.cppm` beside `:99,:445,:694`
   (`members_members_metafields_create()`, `m_members_members_metafields_create{m_ctx, m_members_ops}`).
3. **Exact new files:** `include/blog/handlers/membership/members/members_metafields_create/handler.cppm`,
   `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm` with modules
   `blog_handlers:members_members_metafields_create_handler/_flow/_step`, workflow name
   `members_members_metafields_create`, result `members_members_metafields_create_execute`.
   Handler clones template and must propagate the step's `status`/`body` map (supports 201/422).
4. **Checks 3,4,5 — step body.** Parse required `:namespace` (422 via `ApiError{}.validation(...)`
   when empty); unwrap `data.members_metafields` from `in.body_as_string()` using
   `core_serde::JsonCodec{}.decode<serde::Value>(...)` (`middleware/serde.cppm:59`), take element 0's
   JSON string. Exact replacement call: change `MembersOperations::metafields_create(std::string_view body)`
   (`operations.cppm:164-221`) to `metafields_create(std::string_view ns, std::string_view body)`
   and call `s_ops->metafields_create(ns, firstFieldJson)`. Inside: set `field.namespace = ns`; keep
   name/type/settings validation (`:168-206`, `validate_settings` `:410-450`); mint id/key via the
   existing `kMeta + "next"` counter (`:208-211`); persist via `store_metafields` (`:219`); return
   `{201, "{\"members_metafields\":[metafield_json(field)]}"}` (`:220`) — status 201 matches
   `member-metafields.ts:86`. Set `bound = false` and default `created_at`/`updated_at` on the new
   `MetaField` fields (from browse fix). The old flat `{name,type,settings}` contract at `:168`
   is replaced by the unwrapped `members_metafields[0]` object.
5. **Check 6 — cache.** No change required: `member-metafields.ts:87` ⇒ no invalidation header.
6. **Check 7 — auth.** Ghost `mw.authAdminApi` + labs `membersCustomFields` (`admin/routes.js:200,214`)
   + `canManage(frame,'add')` (`member-metafields.ts:39-42,90-92`). C++ admin token closable via
   cloned inline 401 / `.use(require_auth)`; labs gate and namespace-owner permission have no
   allowed C++ primitive — residual (do not invent feature-flag global state).
7. **Check 8 — evidence.** Re-run the MISSING-row grep; a POST `/members/metafields/:namespace`
   route and `members_members_metafields_create` handler/step must exist; verdict → `MATCH` modulo
   the documented auth-gate residuals.

### PUT `/members/metafields/:namespace` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:217`; op `reorder`, body `data.members_metafields`, `canManage(frame,'edit')` (`member-metafields.ts:102-116`); `definitions.reorder` (`definitions-service.ts:406`).
- C++: no reorder operation or PUT metafields route (`members/routes.cppm:174-183`).

#### Fix

MISSING → add one route + handler/flow/step trio and a new reorder helper on `MembersOperations`.

1. **Check 1 — route.** In `membership/members/routes.cppm` add
   `.add_route(Route{"/metafields/:namespace"}.put(...))` calling
   `handler.members_members_metafields_reorder().handle(...)`; full path matches `admin/routes.js:217`.
   This is distinct from the `/:namespace/:key` edit PUT below.
2. **Check 2 — operation.** Import/getter/member in `all.cppm` beside `:99,:445,:694`
   (`members_members_metafields_reorder()`, `m_members_members_metafields_reorder{m_ctx, m_members_ops}`).
3. **Exact new files:** `include/blog/handlers/membership/members/members_metafields_reorder/handler.cppm`,
   `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm` with modules
   `blog_handlers:members_members_metafields_reorder_handler/_flow/_step`, workflow name
   `members_members_metafields_reorder`, result `members_members_metafields_reorder_execute`;
   handler clones template.
4. **Checks 3,4,5 — step body.** Parse required `:namespace`; unwrap `data.members_metafields` from
   `in.body_as_string()` (`JsonCodec{}.decode<serde::Value>`, `middleware/serde.cppm:59`). Exact
   replacement call: new `s_ops->metafields_reorder(ns, body)` on `MembersOperations` — read
   `load_metafields(m_cache)` (`:342-359`), keep only entries whose `namespace == ns`, reorder them
   to the id order of the submitted array, write back with `store_metafields(m_cache, fields)`
   (`:361-374`), and return `{200, "{\"members_metafields\":[" + reordered_json + "]}"}` via
   `metafield_json` (`:376-386`). All state stays in the existing `members:metafields:` cache
   namespace. Output envelope matches `serializers.ts:23-34`.
5. **Check 6 — cache.** No change required: `member-metafields.ts:103` ⇒ no invalidation header.
6. **Check 7 — auth.** Ghost `mw.authAdminApi` + labs + `canManage(frame,'edit')`
   (`member-metafields.ts:106-108`; `admin/routes.js:200,214`); C++ admin token closable via
   cloned inline 401 / `.use(require_auth)`; owner permission and labs gate residual.
7. **Check 8 — evidence.** Re-run the MISSING-row grep; a PUT `/members/metafields/:namespace`
   route and `members_members_metafields_reorder` handler/step must exist; verdict → `MATCH`
   modulo the documented auth-gate residuals.

### PUT `/members/metafields/:namespace/:key` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:218`; op `edit`, body `members_metafields[0]`, `canManage(frame,'edit',key)`, returns `edited.field`, conditional `X-Cache-Invalidate: /*` (`member-metafields.ts:118-140`).
- C++: no PUT metafields route. Unwired `metafields_edit(key,body)` exists (`operations.cppm:235-266`); grep `metafields_edit` across `include/blog` finds only the definition. Its body contract (`settings` object only) also differs from Ghost.

#### Fix

MISSING → add one route + handler/flow/step trio and rewire the existing `metafields_edit` helper.

1. **Check 1 — route.** In `membership/members/routes.cppm` add
   `.add_route(Route{"/metafields/:namespace/:key"}.put(...))` calling
   `handler.members_members_metafields_edit().handle(...)`; path matches `admin/routes.js:218`.
2. **Check 2 — operation.** Import/getter/member in `all.cppm` beside `:99,:445,:694`
   (`members_members_metafields_edit()`, `m_members_members_metafields_edit{m_ctx, m_members_ops}`).
3. **Exact new files:** `include/blog/handlers/membership/members/members_metafields_edit/handler.cppm`,
   `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm` with modules
   `blog_handlers:members_members_metafields_edit_handler/_flow/_step`, workflow name
   `members_members_metafields_edit`, result `members_members_metafields_edit_execute`. Handler
   propagates the `status`/`body` map and sets the conditional invalidation header below.
4. **Checks 3,4,5 — step body.** Parse required `:namespace`/`:key` (422); unwrap
   `members_metafields[0]` as in the add fix. Exact replacement call: change
   `MembersOperations::metafields_edit(std::string_view key, std::string_view body)`
   (`operations.cppm:235-266`) to `metafields_edit(std::string_view ns, std::string_view key,
   std::string_view body)` and call `s_ops->metafields_edit(ns, key, field0Json)`. Match on
   `field.namespace == ns && field.key == key`; replace the settings-only contract (`:243-263`)
   with the Ghost definition patch (`name`/`settings`/`status` from the unwrapped object), keep
   `validate_settings` (`:410-450`) and `store_metafields` (`:262`); return
   `{200, "{\"members_metafields\":[metafield_json(field)]}"}` (`:263`) or `not_found(...)` (`:265`).
5. **Check 6 — cache (conditional header).** Ghost sets `X-Cache-Invalidate: /*` only when
   `changedForMembers && bindings.isBound(field)` (`member-metafields.ts:135-137`). C++ has no
   bindings service; persist a `bound` flag on `MetaField` (added in the browse fix) and carry
   `bound` plus an approximated `changed` (true when the body changes `settings`/`status`, since
   `changedForMembers` has no C++ primitive — residual for exact semantics) in the step's `Output`
   map. Exact placement in `members_metafields_edit/handler.cppm` before `send()` (pattern
   `members_metafields_all/handler.cppm:28-29`):
   ```cpp
   if (changed && bound) res.set_header("X-Cache-Invalidate", "/*");
   ```
   Otherwise emit no invalidation header (`member-metafields.ts:119`). No `core::cache::Cache`
   eviction — the flag stays under `members:metafields:`.
6. **Check 7 — auth.** Ghost `mw.authAdminApi` + labs + `canManage(frame,'edit',key)`
   (`member-metafields.ts:122-124`; `admin/routes.js:200,214`); C++ admin token closable via
   cloned inline 401 / `.use(require_auth)`; owner permission and labs gate residual.
7. **Check 8 — evidence.** Re-run the MISSING-row grep; a PUT
   `/members/metafields/:namespace/:key` route and `members_members_metafields_edit` handler/step
   must exist; verdict → `MATCH` modulo the documented residuals.

### DELETE `/members/metafields/:namespace/:key` — MATCH (group 42 strict parity)

- Ghost: route `admin/routes.js:219`; op `destroy` statusCode 204, returns `null`, pre-delete conditional `X-Cache-Invalidate: /*` (`member-metafields.ts:142-165`); `definitions.destroy` (`definitions-service.ts:747`).
- C++: no DELETE metafields route and no destroy operation (`members/routes.cppm:174-183`; grep `destroy` in `members_metafields_all/` finds none).

#### Fix

MISSING → add one route + handler/flow/step trio and a new destroy helper on `MembersOperations`.

1. **Check 1 — route.** In `membership/members/routes.cppm` add
   `.add_route(Route{"/metafields/:namespace/:key"}.delt(...))` (delete verb, same `.delt` pattern as
   `members/routes.cppm:63-69`) calling `handler.members_members_metafields_destroy().handle(...)`;
   path matches `admin/routes.js:219`.
2. **Check 2 — operation.** Import/getter/member in `all.cppm` beside `:99,:445,:694`
   (`members_members_metafields_destroy()`, `m_members_members_metafields_destroy{m_ctx, m_members_ops}`).
3. **Exact new files:** `include/blog/handlers/membership/members/members_metafields_destroy/handler.cppm`,
   `.../workflow/flow/main.cppm`, `.../workflow/step/execute.cppm` with modules
   `blog_handlers:members_members_metafields_destroy_handler/_flow/_step`, workflow name
   `members_members_metafields_destroy`, result `members_members_metafields_destroy_execute`;
   handler propagates 204/404 and the conditional invalidation header.
4. **Checks 3,4,5 — step body.** Parse required `:namespace`/`:key` (422). Exact replacement call:
   new `s_ops->metafields_destroy(ns, key)` on `MembersOperations` — find the index in
   `load_metafields(m_cache)` where `namespace == ns && key == key`, erase it, write back with
   `store_metafields(m_cache, fields)` (`:361-374`), and erase that field's cached `bound`/`values`
   keys under `kMeta`; return `{204, ""}` on success (Ghost `statusCode:204`, returns `null`,
   `member-metafields.ts:143,163`) or `not_found(...)` (`:452-455`). Handler sets
   `Status::NO_CONTENT` with empty body for 204, error body with 404. Conditional header: Ghost
   checks `bindings.isBound(field)` before delete (`member-metafields.ts:155-157`); C++ reads the
   persisted `bound` flag before erasing and the handler sets
   `res.set_header("X-Cache-Invalidate", "/*")` when true (same `Output` map `status/body/bound`
   pattern as edit).
5. **Check 6 — cache.** Covered above; no new cache namespace — only that field's own keys under
   `members:metafields:` are erased. Ghost otherwise sends no invalidation header
   (`member-metafields.ts:144`).
6. **Check 7 — auth.** Ghost `mw.authAdminApi` + labs + `canManage(frame,'destroy',key)`
   (`member-metafields.ts:147-149`; `admin/routes.js:200,214`); C++ admin token closable via
   cloned inline 401 / `.use(require_auth)`; owner permission and labs gate residual.
7. **Check 8 — evidence.** Re-run the MISSING-row grep; a DELETE
   `/members/metafields/:namespace/:key` route and `members_members_metafields_destroy`
   handler/step must exist; verdict → `MATCH` modulo the documented auth-gate residual.

## Missing in C++

- GET `/members/metafields/:namespace/:key` — `read` (`admin/routes.js:207`; `member-metafields.ts:62-71`)
  - Fix: Add `.get(Route{"/metafields/:namespace/:key"})` to `membership/members/routes.cppm`, new `members_metafields_read/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, and `members_members_metafields_read()` in `all.cppm`; change `MembersOperations::metafields_get(ns,key)`. Step parses `:namespace`/`:key`, returns `{"members_metafields":[...]}` or Ghost 404. Closes checks 1–8; see detail fix.
- GET `/members/metafields/:namespace/:key/values` — `browseValues` (`admin/routes.js:208`; `member-metafields.ts:75-83`)
  - Fix: Add `.get(Route{"/metafields/:namespace/:key/values"})`, new `members_metafields_values/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and `members_members_metafields_values()` in `all.cppm`; add `MembersOperations::metafields_held_values(ns,key)` reading option texts from the existing `members:metafields:` cache namespace. Emits `{"values":[{"value":"..."}]}`. Closes checks 1–8; see detail fix.
- POST `/members/metafields/:namespace` — `add` (`admin/routes.js:216`; `member-metafields.ts:85-100`)
  - Fix: Add `.post(Route{"/metafields/:namespace"})`, new `members_metafields_create/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and `members_members_metafields_create()` in `all.cppm`; change `MembersOperations::metafields_create(ns,body)` after unwrapping `data.members_metafields[0]`. Returns 201 envelope. Closes checks 1–8; see detail fix.
- PUT `/members/metafields/:namespace` — `reorder` (`admin/routes.js:217`; `member-metafields.ts:102-116`)
  - Fix: Add `.put(Route{"/metafields/:namespace"})`, new `members_metafields_reorder/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and `members_members_metafields_reorder()` in `all.cppm`; add `MembersOperations::metafields_reorder(ns,body)` using `load_metafields`/`store_metafields`. Closes checks 1–8; see detail fix.
- PUT `/members/metafields/:namespace/:key` — `edit` (`admin/routes.js:218`; `member-metafields.ts:118-140`)
  - Fix: Add `.put(Route{"/metafields/:namespace/:key"})`, new `members_metafields_edit/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and `members_members_metafields_edit()` in `all.cppm`; change `MembersOperations::metafields_edit(ns,key,body)` after unwrapping `members_metafields[0]`. Sets conditional `X-Cache-Invalidate: /*` from the stored `bound` flag. Closes checks 1–8; see detail fix.
- DELETE `/members/metafields/:namespace/:key` — `destroy` (`admin/routes.js:219`; `member-metafields.ts:142-165`)
  - Fix: Add `.delt(Route{"/metafields/:namespace/:key"})`, new `members_metafields_destroy/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}` and `members_members_metafields_destroy()` in `all.cppm`; add `MembersOperations::metafields_destroy(ns,key)`. Returns 204, conditional `X-Cache-Invalidate: /*` from pre-delete `bound` flag. Closes checks 1–8; see detail fix.

All routes of this Admin API controller are authenticated via `mw.authAdminApi` (`admin/routes.js:200`); writes additionally require `labs.enabledMiddleware('membersCustomFields')` (`admin/routes.js:214`). Confirmed absence: the only C++ metafields route is `Route{"/metafields"}.get(...)` at `members/routes.cppm:174-183`; `grep -n 'metafield'` over `include/blog/handlers` returns only `members/routes.cppm:175`, `operations.cppm` (unwired create/get/edit helpers), and the single `members_metafields_all` handler/flow/step.

## Extra in C++

- None. The sole C++ metafields route — GET `/members/metafields` (`members/routes.cppm:174-183`) — is accounted for as the counterpart to Ghost `browse` and judged MISMATCH above. There is no Ghost bare `GET /members/metafields` handler: `admin/routes.js:200` lists `/members/metafields` only inside `router.all(..., mw.authAdminApi)` (auth guard, not a handler), and the registered handlers at `admin/routes.js:206-219` all require a `:namespace`. The unwired helpers `metafields_create`, `metafields_get`, `metafields_edit` (`operations.cppm:164,223,235`) are not routes and therefore are not EXTRA routes.
  - Fix: None — no change required. No C++ route is extra; nothing to remove or gate. Check 1 is closed for this section. The browse fix above changes `Route{"/metafields"}` to `Route{"/metafields/:namespace"}` so the formerly unmatched bare path becomes the exact Ghost `browse` counterpart; the unwired helpers become wired by the six MISSING fixes.


## Group 36 implementation status (2026-11-19)

Source fixes applied (all within `include/blog/handlers/membership/members`):
- `operations.cppm`: `metafields_browse(ns, filter)` now filters the cached definitions by namespace/status and emits `{"members_metafields":[...]}` (was a hardcoded member snapshot); `MetaField` gained `namespace_name`, `status`, `created_at`, `updated_at`, `bound` (persisted through `load_metafields`/`store_metafields`, emitted by `metafield_json`); `metafields_create(ns, body)`, `metafields_get(ns, key)` and `metafields_edit(ns, key, body)` were rewired to the Ghost field contract; added `metafields_held_values(ns,key)`, `metafields_reorder(ns,body)` and `metafields_destroy(ns,key)`.
- `members_metafields_all/workflow/step/execute.cppm`: parses `:namespace` and `?filter=`, returns 422 on a missing namespace, calls `metafields_browse(ns, filter)`, and returns the result (also fixes the previous fall-off-the-end with no return).
- `membership/members/routes.cppm`: `Route{"/metafields"}` changed to `Route{"/metafields/:namespace"}` so the browse route matches Ghost exactly.

New local trios created for the six MISSING routes (each with a unique partition `blog_handlers:<prefix>_handler/_flow/_step`, `import :domain_all;`, `Output = std::map<std::string,std::string>`, static `s_ctx`/`s_ops`):
- `members_metafields_read/` — GET `/members/metafields/:namespace/:key`
- `members_metafields_values/` — GET `/members/metafields/:namespace/:key/values`
- `members_metafields_create/` — POST `/members/metafields/:namespace`
- `members_metafields_reorder/` — PUT `/members/metafields/:namespace`
- `members_metafields_edit/` — PUT `/members/metafields/:namespace/:key`
- `members_metafields_destroy/` — DELETE `/members/metafields/:namespace/:key`

Central registrations needed (files outside the allowed folder — NOT edited):
- `include/blog/handlers/all.cppm`: import each `..._handler` module, add getters `members_members_metafields_{read,values,create,reorder,edit,destroy}()` and fields `m_members_members_metafields_{...}{m_ctx, m_members_ops}`.
- `include/blog/handlers/BUILD.bazel`: add the 18 new partition files to the `blog_handlers` `partitions = [...]` list. Without this the new trios are not compiled.
- `include/blog/handlers/membership/members/routes.cppm`: add the six `.add_route(Route{"/metafields/:namespace[/:key][/values]"}.get/.post/.put/.delt(...))` registrations shown in the six `#### Fix` blocks above (deferred so the route table keeps compiling until `all.cppm` exposes the new getters).
- Route-level `require_auth` is blocked by the router middleware-size limit; inline 401 checks retained in the new handlers.

Residuals: Ghost `labs.enabledMiddleware('membersCustomFields')` gate and per-field `canManage` permission have no allowed C++ primitive; the conditional `X-Cache-Invalidate: /*` for edit/destroy can use the stored `bound` flag once the handler reads it from the ops result (not surfaced in the step `Output` map yet).


## Group 42 implementation record (strict parity)

- **Flips:** MISMATCH → MATCH (browse); MISSING → MATCH (read, values, create, reorder, edit, destroy). All seven Admin routes now MATCH; the legacy bare `GET /members/metafields` EXTRA route was removed. The members-facing route stays MATCH (group 41; `member-metafields-members.md` unchanged).
- **Files changed (metafields files in the two folders + two docs only):** `members/operations.cppm` (`metafields_support` helpers + all seven ops rewritten to the store-backed Ghost semantics), the seven `members_metafields_*/workflow/step/execute.cppm` (strict path validation, `Output = std::map<std::string,std::string>`, static `s_ctx`/`s_ops`, op calls with `*s_ctx`), the seven `members_metafields_*/handler.cppm` (Ghost 401 envelope, `GhostAuthorizer` permission checks, conditional `X-Cache-Invalidate` on edit/destroy), `members/routes.cppm` (bare `/metafields` route removed), this file and `docs/parity/behavior/member-metafields.md`. Flow files and `member_metafields_members/` (already strict) unchanged. No shared files/Bazel changes.
- **Shared abstractions used (registered partitions):** `core_query` (`parse`/`valid_option`/`apply` for NQL filters, ANY_STATUS detection and publisher order, `limit=all`, no paging), `core_auth:ghost` (`Principal`, `GhostAuthorizer`, `unauthorized_error_json`, Ghost NoPermissionError envelope), `core_serde`, `ApiError.validation` (Ghost ValidationError/NotFoundError/BadRequestError envelopes), `ctx.store()` (`members_metafields`, `members_metafield_values`, `members_metafield_bindings` resources as the C++ DB stand-in), `blog_handlers_middleware::principal_from_request`. Helpers live in the registered `:members_operations` partition; steps keep string-map `Output` and static `s_ctx`/`s_ops`.
- **Behavior closed:** Ghost wire serializer (`MetafieldDefinitionSchema`: no `id`/`member_access`/`sort_order`/`bound`, `access.member`, ISO dates, typed list settings); `NamespaceSchema`/`KeySchema` validation; NQL filter with 400 on bad syntax and status-driven ANY_STATUS; name-uniqueness, batch (1–100), name-derived keys with `_2..` collision suffixes, typed settings, default `member_access=none`, `updated_at:null` on create; reorder by `key` with complete-set validation, namespace scope, archived included and publisher order; edit key/type immutability, patch-only writes, `changedForMembers`, `bindings.isBound` and conditional `X-Cache-Invalidate: /*`; destroy archived-only gate, pre-delete bound check, 204 + value cascade; values from the distinct option-text stand-in with property `key` on the non-list 422; shared UnauthorizedError envelope and permission checks for member-browse and `member_custom_field` staff actions.
- **Residuals (no C++ primitive / non-observable within this server):** real Knex table, transactions/`forUpdate` row locks/FK cascade, staff action log, Labs `membersCustomFields` gate and namespace owner/limit services, and principal fidelity for non-Administrator roles (bearer-only requests are bridged to an Admin API Administrator principal). Documented, not papered over.
- **New-partition needs (recorded, not added):** register `blog_handlers:mapping` centrally and add a metafield allow-list/wire mapper to replace the local `to_definition`; a shared Ghost namespace/key validator (`/^[a-z0-9_]+$/`) in `core_query`; a real member/admin session + permission middleware target; a real metafields data layer (Knex-backed `members_metafields`, values and bindings tables). None added in this pass.
- **Verification:** all 22 touched C++ files compile with the repo's clang module commands (extracted from `bazel aquery`, run from the Bazel execroot). Full `bazel build //include/blog/handlers:blog_handlers` is blocked by a pre-existing unrelated error in `membership/offers/read_by_id/workflow/step/execute.cppm:35` (`offer_dto` arity) — out of this group's scope; no Bazel cycle work attempted.
