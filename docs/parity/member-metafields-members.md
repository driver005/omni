# Parity: member-metafields-members

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/member-metafields-members.ts`
- Ghost routes: `docs/Ghost/ghost/core/server/web/members/account/routes.ts` (member-account router mounted at `/api/member` by `docs/Ghost/ghost/core/server/web/members/app.js:61`)
- C++: none found (no exact path+verb counterpart. The only C++ metafields route is `GET /members/metafields` → `members_members_metafields_all`, which corresponds to the Admin controller `member-metafields.ts`, not to this member-facing controller)
- Checked: 2026-11-19; strict-parity pass 2026-11-20 (group 41). Supersedes the earlier MISSING state below — implementation landed and the verdict is MATCH.

> Verdict note (group 41): the route flipped MISSING → (implemented) PARTIAL → MATCH. The `#### Fix` blocks below are retained for provenance; the authority for current behavior is `docs/parity/behavior/member-metafields-members.md` (route details + post-rewrite Diff + implementation record).

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
| GET `/api/member/metafields/:namespace` (verbatim in router: GET `/metafields/:namespace`) | `browse` | `MemberMetafieldsMembersRoutes::install` → `Router{"/api/member"}` `Route{"/metafields/:namespace"}.get` (`routes.cppm:19-30`) | `member_metafields_members_metafields_browse()` → `MemberMetafieldsMembersMetafieldsBrowseHandler` (`all.cppm:614,893`) | `namespace` path segment, `/^[a-z0-9_]+$/` with Ghost 422 ValidationError message; no filter/body/multipart | `{ members_metafields: [...] }` — Ghost `MetafieldDefinitionSchema` wire shape (access.member, ISO dates, typed settings, no id); no pagination meta; 200 | `ctx.store().json_for("members_metafields")` + in-step audience/status filter + `core_query` publisher order (store is the C++ DB stand-in) | none: no `X-Cache-Invalidate`; full private/no-store `Cache-Control` via `Token::CACHE_CONTROL` | `permissions:false` pass-through; `X-Member-Id` header identity, missing → shared Ghost 401 `UnauthorizedError` envelope | MATCH | Ghost: `web/members/account/routes.ts:24,57-61`, `web/members/app.js:61`, `api/endpoints/index.js:343-345`, `member-metafields-members.ts:24-42`, `services/members-metafields/serializers.ts:14-16,23-34`, `services/members-metafields/definitions-service.ts:168-194`, `utils/serializers/output/member-metafields.ts:12-14,23`. C++ `membership/member_metafields_members/routes.cppm:19-30`, `metafields_browse/handler.cppm:35-40`, `metafields_browse/workflow/step/execute.cppm:124-146,175-255` |

## Route details

### GET /api/member/metafields/:namespace — MATCH (implemented + strict-parity pass)

The bullets below are the original pre-implementation analysis. Post-rewrite the C++ route exists at `/api/member/metafields/:namespace`, dispatches `member_metafields_members_metafields_browse()`, returns the Ghost definition wire shape via local serializer, uses `core_query` for publisher order, emits no cache-invalidation header with a private/no-store `Cache-Control`, and has no GET serde gate. All eight `#### Fix` checks are closed; see the behavior report's group 41 record.

- Route + op: registered in the member-account router as `router.get('/metafields/:namespace', middleware.rejectWhenAnonymous, http(api.memberMetafieldsMembers.browse))` (`docs/Ghost/ghost/core/server/web/members/account/routes.ts:57-61`); router mounts `middleware.loadMemberIdentity` for all routes (`:24`); whole router mounted at `/api/member` (`docs/Ghost/ghost/core/server/web/members/app.js:61`). Controller key `browse` exposed as `api.memberMetafieldsMembers` (`docs/Ghost/ghost/core/server/api/endpoints/index.js:343-345`), with `docName: 'members_metafields'` (`member-metafields-members.ts:28`).
- Input: single `options:['namespace']` with `validation.options.namespace.required:true` (`member-metafields-members.ts:32-33`); `namespace` arrives as the `:namespace` URL segment and is validated with `NamespaceRoute`/`namespaceRoute` before lookup (`services/members-metafields/serializers.ts:8,14-16`). No `filter` option by design (`:36-38` comment), no body parsing on the route (`:57-61` has no `bodyParser`), no multipart. `BrowseFrame` types only `options.namespace` (`member-metafields-members.ts:18-22`).
- Output: `booted().definitions.browse(...)` returns `Metafield[]`; docName selects `serializeMany` and `toMetafieldsResponse`, producing `{ members_metafields: [...] }` with snake-cased keys and ISO `created_at`/`updated_at`, no `meta.pagination` and status 200 (`member-metafields-members.ts:35-40`, `services/members-metafields/serializers.ts:23-34`, `utils/serializers/output/member-metafields.ts:12-14,23`).
- DB call: real service/knex read — `definitions.browse({namespace}, MEMBERS)` delegates to `fields.definitions(executor, {audience:MEMBERS, status:ACTIVE_ONLY, namespace})` then `select('*')` in field order (`services/members-metafields/definitions-service.ts:168-199`). Audience `MEMBERS` narrows to fields a member may see.
- Cache: `headers: { cacheInvalidate: false }` (`member-metafields-members.ts:31`) — no invalidation header, no cache service call in this controller.
- Auth: `permissions:false` (`member-metafields-members.ts:34`) but the HTTP layer requires a signed-in member via `router.use(middleware.loadMemberIdentity)` (`web/members/account/routes.ts:24`) and `middleware.rejectWhenAnonymous` on this route (`:59`). The controller comment states the route "resolves who is asking and refuses an unknown caller" (`:13-15`).
- C++ counterpart: none. Only C++ metafields route is `Route{"/metafields"}.get(...)` under `Router{router, "/members"}` → `handler.members_members_metafields_all()` (`include/blog/handlers/membership/members/routes.cppm:19,174-184`), and the route catalog records only `RouteInfo{"ALL","/members/metafields","members","all",true}` (`include/blog/testing/route_catalog.cppm:140`). That route has no `:namespace` segment and lives under the Admin `/members` surface, while Ghost's Admin controller registers `metafieldsRouter.get('/:namespace', http(api.membersMetafields.browse))` (`docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:206`) — so it is that Admin controller's counterpart, not this one's.
- For reference, the existing C++ metafields handler diverges materially from this controller even on shape: `members_members_metafields_all/handler.cppm:18-22` rejects only when `token(req)` is empty (401 `Unauthorized`) with no member-identity check; `members_metafields_all/workflow/step/execute.cppm:11-18` declares `using Input = interfaces::io::IRequest&` (no namespace/body parsing) and calls `s_ops->metafields_browse()`; `include/blog/handlers/membership/members/operations.cppm:77-85` returns hardcoded `{"members":[{...member id 1}]}` — envelope `members`, not Ghost's `members_metafields` definitions — with no DB read, no filtering, and no cache access for this operation. (`operations.cppm:269-271,342-374` defines `members:metafields:*` cache keys with `Ttl{}.never()` and a `members` tag, but those are used by the create/get/edit operations, not by `metafields_browse()`.)

#### Fix

MISSING → one new route installer + three new handler/flow/step files + two registry edits. Do **not**
extend `members_metafields_all` (its `metafields_browse()` is the Admin MISMATCH in
`member-metafields.md`: hardcoded `{"members":[{Member id=1}]}`, `operations.cppm:77-85`) and do not
add a `:namespace` segment to the Admin `/members` router — that surface belongs to
`member-metafields.ts` (`admin/routes.js:206`).

1. **Check 1 — route.** Add file `include/blog/handlers/membership/member_metafields_members/routes.cppm`
   (module `blog_handlers_routes:member_metafields_members_routes`, class
   `blog::api::MemberMetafieldsMembersRoutes::install(core::router::RouterContext<>&, blog::api::BlogHandlers&)`).
   The C++ flat router mirrors Ghost paths without the `/api` prefix (`gifts-members.md:44-46`), so the
   target path is `/member/metafields/:namespace` (not `/members/metafields`):
   ```cpp
   using Route = utils::openapi::ApiRoute;
   using Router = utils::openapi::ApiRouter;
   Router{router, "/member"}
       .use(blog::handlers::middleware::content_type_middleware)
       .add_route(
           Route{"/metafields/:namespace"}
               .get([&handler](interfaces::io::IRequest& req,
                               interfaces::io::IResponse& res,
                               std::function<void()> send) {
                   handler.member_metafields_members_browse().handle(req, res, std::move(send));
               })
               .summary("Ghost memberMetafieldsMembers.browse")
       );
   ```
   Then edit `include/blog/handlers/routes.cppm`: add `export import :member_metafields_members_routes;`
   after `:members_routes;` (`:29`) and `MemberMetafieldsMembersRoutes{}.install(router, handlers);`
   after `MembersRoutes{}.install(router, handlers);` (`:102`). Optionally add
   `RouteInfo{"GET","/member/metafields/:namespace","member_metafields_members","browse",true}` near
   `include/blog/testing/route_catalog.cppm:140`.
2. **Check 2 — operation.** `handler.member_metafields_members_browse()` does not exist. Edit
   `include/blog/handlers/all.cppm`: add `import :member_metafields_members_browse_handler;` near `:99`,
   getter after `:445`:
   ```cpp
   [[nodiscard]] blog::handlers::member_metafields_members::member_metafields_members_browse::MemberMetafieldsMembersBrowseHandler& member_metafields_members_browse() noexcept { return m_member_metafields_members_browse; }
   ```
   and member `...::MemberMetafieldsMembersBrowseHandler m_member_metafields_members_browse{m_ctx};`
   near `:694`. No `MembersOperations` needed — this op reads the store only, so constructor is
   `{m_ctx}` (pattern `gifts_flush_deliveries_put/handler.cppm:10-30`).
3. **New files** (clone `include/blog/handlers/membership/gifts/gifts_flush_deliveries_put/` template):
   - `include/blog/handlers/membership/member_metafields_members/member_metafields_members_browse/handler.cppm`
     — module `blog_handlers:member_metafields_members_browse_handler`, class
     `MemberMetafieldsMembersBrowseHandler`; `handle()` runs workflow
     `"member_metafields_members_browse"`, reads result `"member_metafields_members_browse_execute"`
     as `std::map<std::string,std::string>` with `status`/`body` keys (pattern
     `members_metafields_all/handler.cppm:23-29`), applies status/body, then sets
     `res.set_header("X-Cache-Invalidate", "false")` before `send()`.
   - `.../workflow/flow/main.cppm` —
     `WorkflowBuilder{}.name("member_metafields_members_browse").step<ExecuteStep>().build()`.
   - `.../workflow/step/execute.cppm` — `ExecuteStep` with
     `using Input = interfaces::io::IRequest&; using Output = std::map<std::string,std::string>;`,
     `name{"member_metafields_members_browse_execute"}`, `static blog::BlogContext* s_ctx`.
4. **Checks 3, 5 — input + DB call.** In `ExecuteStep::invoke`: parse `:namespace` from the last
   `/`-separated segment of `in.get_path()` (`interfaces/io/request.cppm:807`); validate
   `^[a-z0-9_]+$` (`contracts/metafield-types/src/identity.ts:30-38`). On bad/missing namespace
   return 422 `ApiError{}.validation("A namespace can only contain lowercase letters, numbers and underscores.", "namespace", "namespace")`
   (`context.cppm:75-84`). No `filter` option, no body/multipart
   (`member-metafields-members.ts:32-38,57-61`). Read definitions with the exact call
   `ctx.store().json_for("members_metafields")` — resource registered at `context.cppm:168`, falls
   through to `m_resources.list("members_metafields")` at `context.cppm:1180`. **Do not** pass
   `{{"namespace", ns}}` expecting a filter: `search_json` skips plain params (`context.cppm:1574-1581`)
   and `find_by_resource` has no `"members_metafields"` branch (`:1055-1092`). Filter in the step:
   `namespace == :namespace`, `status == "active"` (`definitions-service.ts:160-162,192`),
   `member_access ∈ {"read","write"}` for `MEMBERS` (`access.ts:24-28`, `metafield-types/src/index.ts:181-184`),
   sorted by `sort_order` ascending (publisher order, `definitions-service.ts:173-174,197`).
   ```cpp
   const auto raw = ctx.store().json_for("members_metafields");          // ResourceStore rows
   auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);      // pattern: middleware/serde.cppm:56-64
   if (!parsed) return core::workflow::StepResponse<Output>{{{"status","500"},
       {"body", ApiError{}.json("Definition read failed", "members_metafields")}}};
   // iterate decoded array; keep namespace==ns && status=="active" &&
   // (member_access=="read" || member_access=="write"); sort by sort_order;
   // re-encode kept values with core_serde::JsonCodec{}.encode(...)
   return core::workflow::StepResponse<Output>{{{"status","200"},
       {"body", R"({"members_metafields":)" + keptJson + "}"}}};
   ```
5. **Check 4 — output.** Emit exactly `{"members_metafields":[...]}` with the rows' snake_case keys
   and ISO `created_at`/`updated_at` already present on the `members_metafields` ResourceStore fields
   (`context.cppm:168`, defaults at `:246`); no `meta.pagination`, status 200
   (`serializers.ts:23-34`, `member-metafields-members.ts:35-40`). Handler must not hardcode a
   different envelope.
6. **Check 6 — cache.** Ghost `headers: {cacheInvalidate:false}` (`member-metafields-members.ts:31`)
   ⇒ handler emits `X-Cache-Invalidate: false`; step does no `core::cache::Cache` get/put/erase and
   invents no namespace. Cross-request state stays in the existing `ctx.store()` ResourceStore.
7. **Check 7 — auth.** No admin bearer: do **not** use `require_auth`/`bearer_middleware` or the
   `token(req).empty()` rejection from `members_metafields_all/handler.cppm:18-22`. Ghost uses member
   session auth (`loadMemberIdentity` router-wide at `account/routes.ts:24`, `rejectWhenAnonymous` on
   this route at `:59`) with `permissions:false` (`member-metafields-members.ts:34`). Read member
   identity from a request header (`X-Member-Id`, via `in.find_header(...)`,
   `interfaces/io/request.cppm:722`) — same header approach as `gifts-members.md:144-148`; empty ⇒
   401 `ApiError{}.json("Unauthorized", "member identity required")`. `permissions`/`docName` has no
   allowed C++ primitive — accepted residual gap, no new global state.
8. **Check 8 — evidence.** Re-run the five absence checks in "Missing in C++": a
   `GET /member/metafields/:namespace` route, a `member_metafields_members_browse()` getter/import/member,
   and the three new files must now exist. The Admin `GET /members/metafields` route stays mapped to
   `member-metafields.ts` and is untouched.

## Missing in C++

- GET `/api/member/metafields/:namespace` → `api.memberMetafieldsMembers.browse` (`docs/Ghost/ghost/core/server/web/members/account/routes.ts:57-61`, controller `docs/Ghost/ghost/core/server/api/endpoints/member-metafields-members.ts:30-41`).
  Fix: add `include/blog/handlers/membership/member_metafields_members/routes.cppm` registering `GET /member/metafields/:namespace`, add `.../member_metafields_members_browse/{handler.cppm,workflow/flow/main.cppm,workflow/step/execute.cppm}`, and wire the route installer + handler getter/member into `include/blog/handlers/routes.cppm` and `include/blog/handlers/all.cppm`. Step calls `ctx.store().json_for("members_metafields")` and filters `namespace`/`status=active`/`member_access∈{read,write}` in-step, returns `{"members_metafields":[...]}`, validates `^[a-z0-9_]+$`, rejects anonymous via `X-Member-Id` header (401) — no admin bearer. Closes checks 1–8; full detail in the route's `#### Fix` above.

Absence confirmed by: (1) grep `metafields` across `include/blog/handlers/**/routes.cppm` → only `include/blog/handlers/membership/members/routes.cppm:175` with path `/metafields` (no `:namespace`, no `/api/member` prefix); (2) grep `namespace|account` over all `routes.cppm` → no member-account metafields route; (3) `include/blog/testing/route_catalog.cppm:140` lists a single metafields route `ALL /members/metafields`; (4) grep `memberMetafieldsMembers|member-metafields-members` over `include/` → 0 matches; (5) `include/blog/handlers/all.cppm:99,445,694` registers only the `members_members_metafields_all` handler.

## Extra in C++

None for this controller. The C++ `GET /members/metafields` route (`include/blog/handlers/membership/members/routes.cppm:174-184`, handler `members_members_metafields_all` at `include/blog/handlers/all.cppm:445`) corresponds to the Admin API `member-metafields.ts` controller (`GET /members/metafields/:namespace` at `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js:206`), not to `member-metafields-members.ts`.
  Fix: No change required. Do not remove or gate the Admin `GET /members/metafields` route — it is the counterpart of `member-metafields.ts` and is already accounted for as MISMATCH in `docs/parity/member-metafields.md`. Check 1 is closed for this entry; the new member-facing route goes in a separate `/member` router so the two do not collide.

## Needs central registration

New files added under `include/blog/handlers/membership/member_metafields_members/` (subagent-scope edit only):

- `routes.cppm` — module `blog_handlers_routes:member_metafields_members_routes`, class `blog::api::MemberMetafieldsMembersRoutes::install(router, handler)`, installs `GET /api/member/metafields/:namespace` → `handler.member_metafields_members_metafields_browse()`.
- `metafields_browse/handler.cppm` — module `blog_handlers:member_metafields_members_metafields_browse_handler`, class `blog::handlers::member_metafields_members::metafields_browse::MemberMetafieldsMembersMetafieldsBrowseHandler`.
- `metafields_browse/workflow/flow/main.cppm` — module `blog_handlers:member_metafields_members_metafields_browse_flow`.
- `metafields_browse/workflow/step/execute.cppm` — module `blog_handlers:member_metafields_members_metafields_browse_step`; reads `ctx.store().json_for("members_metafields")`, filters `namespace`/`status=active`/`member_access∈{read,write}`, sorts by `sort_order`, returns `{"members_metafields":[...]}`, validates `^[a-z0-9_]+$`, rejects anonymous via `X-Member-Id` header (401), emits no cache access.

Central registrations still required (in shared files this subagent may not edit):

1. `include/blog/handlers/BUILD.bazel` — add the four new files to the `blog_handlers` library partitions list (the `_step`/`_flow`/`_handler` trio under `blog_handlers:`) and add `routes.cppm` to the `blog_handlers_routes` library partitions list. Without this the new modules are not compiled.
2. `include/blog/handlers/base.cppm` (or the aggregator that declares the `blog_handlers` primary module interface) — declare/re-export the three new partitions `member_metafields_members_metafields_browse_{handler,step,flow}` so `import :domain_all;`/handler graph resolves.
3. `include/blog/handlers/all.cppm` — `import :member_metafields_members_metafields_browse_handler;`, add getter `member_metafields_members_metafields_browse()` returning `...::metafields_browse::MemberMetafieldsMembersMetafieldsBrowseHandler&`, and member `m_member_metafields_members_metafields_browse{m_ctx}` (constructor is `{m_ctx}`, no operations object).
4. `include/blog/handlers/routes.cppm` — `export import :member_metafields_members_routes;` and `MemberMetafieldsMembersRoutes{}.install(router, handlers);` so the route installer runs.
5. `include/blog/testing/route_catalog.cppm` — optionally add `RouteInfo{"GET","/api/member/metafields/:namespace","member_metafields_members","browse",true}`.

## Group 41 implementation record (strict parity)

- **Flips:** MISSING → PARTIAL → MATCH in this doc; PARTIAL → MATCH in `docs/parity/behavior/member-metafields-members.md`. One route, no MISMATCH/EXTRA.
- **Files changed (folder + docs only):** `routes.cppm` (dropped `content_type_middleware`/middleware import), `metafields_browse/handler.cppm` (no `X-Cache-Invalidate`, full private/no-store `Cache-Control`), `metafields_browse/workflow/step/execute.cppm` (Ghost wire serializer, shared 401 envelope, `core_query` order), both parity docs. No shared files/Bazel changes; central registrations in `all.cppm`/`base.cppm`/`routes.cppm`/`BUILD.bazel` were already present and untouched.
- **Shared abstractions:** `core_auth:ghost`, `core_query`, `core_serde`, `ApiError.validation`, `ctx.store()`, `interfaces::io::types::Token::CACHE_CONTROL`. Helpers kept in the registered `_browse_step` partition (Output = string map, static `s_ctx`).
- **Gaps (no C++ primitive / non-observable):** real Knex query + audience/status scoping at DB layer, cookie-backed member session (`X-Member-Id` header stand-in), parent-app `/members` mount, `booted()` init invariant.
- **New-partition needs (recorded, not added):** register `blog_handlers:mapping` centrally + add a metafield allow-list/wire mapper; shared Ghost namespace/segment validator in `core_query`; member-session identity middleware / real data layer would need new central targets.
- **Verification:** rewritten step + handler compile under the repo's clang module command; routes edit is import/middleware removal only. Full `bazel build //include/blog/handlers:blog_handlers` blocked by unrelated in-progress errors and a pre-existing Bazel module cycle — out of this group's scope.

