# Behavior report: newsletters

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/newsletters.js`
- Ghost route table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js` (lines 578–586); mounted at `/admin/` by `docs/Ghost/ghost/core/server/web/api/app.js:22`
- C++ folders: `include/blog/handlers/membership/newsletters/` (routes + 5 handler/flow/step triples); installed via `include/blog/handlers/routes.cppm:36,108`
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | 5 |
| PARTIAL | 0 |
| MISMATCH | 0 |
| MISSING | 0 |
| EXTRA | 0 |

> **Group 44 strict-parity pass (2026-11-20).** All five routes flipped to MATCH.
> The numbered facts and Diff blocks below are the pre-rewrite analysis retained
> for provenance; the authoritative final state is recorded in
> **Group 44 — strict-parity implementation record** at the end of this document.

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GET `/newsletters` | browse | opts include/filter/fields/limit/order/page; include⊆{count.posts,count.members,count.active_members} | permissions:true; authAdminApi | 200; X-Cache-Invalidate:false | service.browse → NewsletterModel.findPage | cacheInvalidate=false | `{newsletters:[],meta.pagination:...}` via mappers/newsletters | browse/{handler,workflow/flow/main,workflow/step/execute}.cppm | 200 + header match; raw-query parse, 422 include validation, restricted() stand-in; count.* includes NOT embedded; in-memory fixture data; no permission service | PARTIAL | newsletters.js:9-25; routes.js:578; service.js:133-135; C++ browse step:12-42,83-102; context:956-960,1646-1663 |
| GET `/newsletters/:id` | read | opts include/fields/debug/forUpdate/transacting; data id/slug/uuid | permissions:true; authAdminApi | 200/404; X-Cache-Invalidate:false | service.read → NewsletterModel.findOne; NotFoundError | cacheInvalidate=false | `{newsletters:[model]}` | read_by_id/{handler,workflow/flow/main,workflow/step/execute}.cppm | id-only lookup (no slug/uuid), no opts parsing, no serializer autocorrect; 200/404 + message + header match | PARTIAL | newsletters.js:27-51; service.js:117-126; C++ read step:12-24 |
| POST `/newsletters` | add | opts include/opt_in_existing; body data.newsletters[0] | permissions:true; authAdminApi | 201; X-Cache-Invalidate:true | service.add → limit check → prepAttrsForEmailVerification → Model.add → opt-in → reload → magic-link mail | cacheInvalidate=true | `{newsletters:[model]}` + meta.opted_in_member_count / sent_email_verification | create/{handler,workflow/flow/main,workflow/step/execute}.cppm | 201 + header + body decode match; NO limit/email-verification/transaction/member-subscribe/magic-link; no sort_order; no meta; header is literal string, no real invalidation | PARTIAL | newsletters.js:53-70; service.js:150-221,305-426; C++ create step:16-47; create handler:26 |
| PUT `/newsletters/verifications/` | verifyPropertyUpdate | data.token | permissions:{method:'edit'}; authAdminApi | 200/400; X-Cache-Invalidate:false | magicLinkService.getDataFromToken → property allowlist → Model.edit → meta.email_verified | cacheInvalidate=false | `{newsletters:[model]}` + meta.email_verified | newsletters_verifications_put/{handler,workflow/flow/main,workflow/step/execute}.cppm | token treated as newsletter ID via find_by_resource; no token decode/property allowlist/single-use consumption/BadRequestError/meta.email_verified; 422 not 400 | MISMATCH | newsletters.js:93-104; service.js:279-298; index.ts:3-5,16-21; C++ verify step:13-63 |
| PUT `/newsletters/:id` | edit | opts id(required)/include; body data.newsletters[0] | permissions:true; authAdminApi | 200; X-Cache-Invalidate:true | service.edit → findOne(require) → prepAttrsForEmailVerification → limit check → Model.edit → reload → magic-link mail | cacheInvalidate=true | `{newsletters:[model]}` + meta.sent_email_verification | edit/{handler,workflow/flow/main,workflow/step/execute}.cppm | core edit + 404 + header match; no required-id validation (path id used), no email verification/limit/mail/meta; 201→200 coercion | PARTIAL | newsletters.js:72-91; service.js:231-272; C++ edit step:16-55 |

## Route details

### GET `/newsletters` — PARTIAL

**Ghost**

1. Identity: `GET /newsletters` at `admin/routes.js:578`, router-level `router.use(apiMw.cors)` at `:17`, mounted under `/admin/` by `web/api/app.js:22` with `versionRewrites`, `contentVersion`, `maxLimitCap` at `app.js:15-19`. Middleware chain `mw.authAdminApi` = [authenticateAdminApi, authorizeAdminApi, updateUserLastSeen, cors, adminSSLAndHostRedirect, prettyUrls, tokenPermissionCheck] (`admin/middleware.js:110-118`).
2. Operation: key `browse`, `docName: 'newsletters'` (`newsletters.js:7,9`).
3. Input: `options: ['include','filter','fields','limit','order','page']` (`:13`); `validation.options.include.values = allowedIncludes` where `allowedIncludes = ['count.posts','count.members','count.active_members']` (`:1,14-20`); no `data`, no body/multipart; query aliases handled by api-framework.
4. Auth: `permissions: true` (`:21`) → docName-based permission resolution in api-framework; `authAdminApi` above.
5. Status/headers: `headers.cacheInvalidate: false` (`:10-12`); default 200.
6. Call chain: `query(frame) → newsletters.service.browse(frame.options)` (`:22-24`) → `NewsletterModel.findPage(options)` (`services/newsletters/newsletters-service.js:133-135`). Read-only; no events/mail/jobs. Empty result → empty list, pagination total 0.
7. Cache: `cacheInvalidate: false` — no invalidation write; api-framework still emits cache headers per config.
8. Output: envelope `{newsletters:[...], meta.pagination:{...}}` from `findPage`; per-row mapper `utils/serializers/output/mappers/newsletters.js:1-37` (admin branch `:23-34`: full `toJSON`, with `sender_email`→`sender_reply_to` autocorrection via `emailAddressService.validate` at `:28-32`; content API branch `:7-22` emits only 10 fields).

**C++**

1. `NewslettersRoutes::install`: `Router{router, "/newsletters"}` with `content_type_middleware` + `require_auth`, `.get(...)` → `newsletters_browse().handle` (`membership/newsletters/routes.cppm:19-30`).
2. `BrowseHandler` builds `LocalWorkflowDriver`, registers `MainFlow{}.build()` (`browse/handler.cppm:12-16`); `handle` runs `"newsletters_browse"`, reads `"newsletters_browse_execute"` typed output (`browse/handler.cppm:17-20`).
3. Writes `X-Cache-Invalidate: false`, status `OK` (200), body bytes (`browse/handler.cppm:22-23`).
4. Step `Input = IRequest&`, `Output = std::string` (`browse/workflow/step/execute.cppm:10`); options parsed from raw `get_path()` query string for `include,filter,fields,limit,order,page` (`:19,60-76`); `include` validated against `kIncludes` → 422 `ApiError.validation("Validation error","Invalid include","include")` (`:12-13,20-22`).
5. `restricted()` rejects `order`/`fields`/`filter[field]` tokens not in 11-field `kFields`, stripping `"newsletters."` prefix and leading `-` (`:14-16,23-25,83-102`) — a hand-rolled stand-in, not Ghost's NQL/model rule set.
6. `limit` default 15, `page` default 1 from query (`:28-29,77-82`); `total = ctx.store().total("newsletters")` → `m_db.count<Newsletter>()` (`context/context_context.cppm:1140-1150`); `pages`, `prev`, `next` computed in step (`execute.cppm:31-33`).
7. `rows = ctx.store().json_for("newsletters", params)` (`execute.cppm:34`) → `query_json(m_db.all<Newsletter>(), params)` (`context:1152-1163,1454-1462`) → `query_json_impl` applies page/offset/limit again + `search_json` (`:1561-1590,1592-1644`). `embeddings()` only handles `authors,tags,newsletters,tiers` (`:1646-1663`) — **`count.posts`, `count.members`, `count.active_members` produce no embedded counts**. DB seeded with one fixture newsletter "Daily" (`context:956-960`).
8. Output string assembled manually as `{"newsletters":<rows>,"meta":{"pagination":{limit,page,pages,prev,next,total}}}` (`execute.cppm:35-42`); no mapper (no `sender_email` autocorrection), no permission service.

**Diff**

- Residual gaps (no C++ primitive): api-framework authorization/permission services, `tokenPermissionCheck`, real `findPage` semantics (NQL filters, relations, computed `count.*`), model serializer/admin autocorrection.
- Divergences: fixture-only data; `include=count.*` accepted but ignored; `restricted()` is an ad-hoc field allowlist; pagination computed twice; `content_type_middleware` requires `Content-Type: application/json` and `Accept: application/json|*/*` (`middleware/serde.cppm:90-126`) where Ghost has no such requirement on this GET; `require_auth` only checks non-empty bearer token (`middleware/auth.cppm:12-29`) vs Ghost's 7-step chain.

### GET `/newsletters/:id` — PARTIAL

**Ghost**

1. Identity: `GET /newsletters/:id`, `mw.authAdminApi` (`admin/routes.js:579`).
2. Operation: `read`, `docName: 'newsletters'` (`newsletters.js:7,27`).
3. Input: `options: ['include','fields','debug','forUpdate','transacting']` (`:31-38`); `data: ['id','slug','uuid']` (`:39`); include allowlist (`:40-46`); path param `:id`.
4. Auth: `permissions: true` (`:47`).
5. Status/headers: `cacheInvalidate: false` (`:28-30`); 200, or 404 `NotFoundError("Newsletter not found.")`.
6. Call chain: `query → service.read(frame.data, frame.options)` (`:48-50`) → `NewsletterModel.findOne(data, options)`; throws NotFoundError when falsy (`newsletters-service.js:117-126`). Include relations resolved by model.
7. Cache: `cacheInvalidate: false`.
8. Output: mapper `newsletters.js:4-36`, envelope `{newsletters:[model]}`; options `fields`/`include` applied by `toJSON(frame.options)` (`:5`).

**C++**

1. Route `.add_route(Route{"/:id"}...use(require_auth).get(...))` → `newsletters_read_by_id().handle` (`routes.cppm:38-48`).
2. `ReadByIdHandler` runs `"newsletters_read_by_id"`, payload is `map{status,body}`; handler writes `X-Cache-Invalidate: false`, status from payload, body (`read_by_id/handler.cppm:17-29`).
3. Step: id = path substring after last `/`, strip query (`:14-16`); empty → 422 `validation("Validation error","An id is required","id")` (`:17-19`).
4. `ctx.store().find_by_resource("newsletters", id)` (`:20`) → `find_entity<Newsletter>` with `ghost_id` decode and `std::stoll(...,16)` fallback (`context:1059-1080,1084-1114`); encoded via `JsonCodec` (`:1113`).
5. Found → 200 + full entity JSON (`:21`); not found → 404 `ApiError.json("Newsletter not found.","Not Found")` (`:22-23`) — message matches Ghost.
6. Flow `MainFlow{}.build()` single `ExecuteStep` (`read_by_id/workflow/flow/main.cppm:7-10`).
7. No parsing of `include/fields/debug/forUpdate/transacting`; no `slug`/`uuid` data lookup; no admin mapper autocorrection; no permission service; no real transaction/`forUpdate` locking.

**Diff**

- Residual gaps: permissions, include/relation loading, field projection, `forUpdate` row locking, `transacting`, serializer mapping.
- Divergences: id derived from path only (Ghost `data` may carry `slug`/`uuid`); 422 for empty id is a C++-only branch; reads in-memory fixture DB rather than Knex.

### POST `/newsletters` — PARTIAL

**Ghost**

1. Identity: `POST /newsletters`, `mw.authAdminApi` (`admin/routes.js:580`).
2. Operation: `add`, `docName: 'newsletters'`, `statusCode: 201` (`newsletters.js:7,53-54`).
3. Input: `options: ['include','opt_in_existing']` (`:58`), include allowlist (`:59-65`), body `data.newsletters[0]` (`:68`); no required fields declared in controller validation.
4. Auth: `permissions: true` (`:66`).
5. Status/headers: 201 (`:54`), `cacheInvalidate: true` (`:55-57`).
6. Call chain (`newsletters-service.js:150-221`): if `opt_in_existing && !transacting` wrap in `NewsletterModel.transaction` (`:152-157`); if status active/absent call `limitService.errorIfWouldGoOverLimit('newsletters')` (`:159-164`); `prepAttrsForEmailVerification(attrs)` (`:167`, internals `:305-381`) validates `sender_email` (type `from`, emptyable) and non-`newsletter`/`support` `sender_reply_to` (type `replyTo`) via `emailAddressService.service.validate`, throws `ValidationError` when disallowed (`:338-345`), else deletes property and queues `{email,property}` when `verificationEmailRequired` (`:346-353`), with reply-to↔sender swap shortcut (`:347-350`) and sender_email autocorrect reset (`:357-378`); `getNextAvailableSortOrder` sets `sort_order` (`:170-171`); `NewsletterModel.add` (`:176`) with unique-violation → `ValidationError("A newsletter with the same name already exists")` (`:177-186`); if `opt_in_existing` → `MemberModel.fetchAllSubscribed` + `newsletter.subscribeMembersById` (`:191-206`); reload `findOne({id}, require:true)` (`:209-212`); set `meta.opted_in_member_count` (`:214-217`); `respondWithEmailVerification` (`:220`, `:386-397`) calls `sendEmailVerificationMagicLink` (`:402-426`) → `MagicLink.sendMagicLink` with single-use token (`index.ts:3-5,16-21`) and `GhostMailer.send` (`service.js:61-108,405-419`), sets `meta.sent_email_verification`. Side effects: DB transaction, member link-table writes, single-use token write, mail/outbox send, limit read.
7. Cache: `cacheInvalidate: true` → api-framework emits cache invalidation header/URLs; no dedicated cache service key.
8. Output: mapper `newsletters.js:23-34` (admin autocorrection), envelope `{newsletters:[model], meta}` with computed `meta` fields, status 201.

**C++**

1. Route `.post(...)` with `content_type_middleware` + `require_auth` → `newsletters_create().handle` (`routes.cppm:30-37`).
2. `CreateHandler` runs `"newsletters_create"`; `X-Cache-Invalidate: true`, status from payload (`create/handler.cppm:17-29`).
3. Step parses `include`/`opt_in_existing` from raw path query, validates include against `kIncludes` → 422 (`create/workflow/step/execute.cppm:13-20,56-63,64-81`).
4. Body: reads `body_as_string()`, walks `data → newsletters → array[0]` object, re-encodes it as `inner` (fallback `"{}"`) (`execute.cppm:22-43,51`); matches Ghost `data.newsletters[0]`.
5. `ctx.store().create("newsletters", inner)` (`execute.cppm:44`) → `wrap("newsletters", create_entity<Newsletter>(body))` (`context:1292-1294`) → `EntityJson::from_json` + `m_db.insert` (`context:1027-1031`) → `wrap` returns 201 + `{"newsletters":[<entity>]}` (`context:1423-1438`); step passes through `result.status()`/`result.body()` (`execute.cppm:45-46`).
6. Flow: single `ExecuteStep` (`create/workflow/flow/main.cppm:7-10`).
7. NOT implemented: `opt_in_existing` member subscription, Knex transaction, limit check, `getNextAvailableSortOrder`, email verification prep/validation, unique-name `ValidationError`, reload after write, `meta.opted_in_member_count` / `meta.sent_email_verification`, magic-link token + mail/outbox, admin serializer autocorrection, permission service. `X-Cache-Invalidate` is a literal `"true"` string with no invalidation URLs/timestamps.

**Diff**

- Residual gaps: permission services, limit service, email-address service, single-use token provider, GhostMailer/SMTP, Member subscription service, Knex transaction, model hooks/events.
- Divergences: no `sort_order`, no `meta`, no unique-name error; create_entity inserts whatever fields are in body (defaults from `m_defaults` at `context:267` only); 201 status coincidentally matches via `wrap`.

### PUT `/newsletters/verifications/` — MISMATCH

**Ghost**

1. Identity: `PUT /newsletters/verifications/` (trailing slash), `mw.authAdminApi` (`admin/routes.js:581-585`).
2. Operation: custom action `verifyPropertyUpdate`, `docName: 'newsletters'` (`newsletters.js:7,93`).
3. Input: `data: ['token']` (`:100`); no options, no body shape beyond token.
4. Auth: `permissions: {method: 'edit'}` (`:97-99`) → edit permission for `newsletters` docName; `authAdminApi`.
5. Status/headers: `cacheInvalidate: false` (`:94-96`); 200, or 400 `BadRequestError` for invalid token.
6. Call chain: `query → service.verifyPropertyUpdate(frame.data.token)` (`:101-103`) → `magicLinkService.getDataFromToken(token)` (`newsletters-service.js:280`) which consumes a single-use token (`index.ts:9,16-21`; validity 24h/10min-after-use/max 7 usages at `index.ts:3-5`); extract `{id, property, value}` (`:281`); if `!id` or `property` not in `VERIFIABLE_PROPERTIES = ['sender_email','sender_reply_to']` (`service.js:18,283-287`) → `BadRequestError("This verification link is not valid for a newsletter.")`; `attrs[property] = value`; `NewsletterModel.edit(attrs,{id})` (`:292`); sets `meta.email_verified = property` (`:294-295`). Side effects: single-use token consumption, DB edit.
7. Cache: `cacheInvalidate: false`.
8. Output: mapper `newsletters.js`, envelope `{newsletters:[model]}` with `meta.email_verified`.

**C++**

1. Route `.add_route(Route{"/verifications"}...use(require_auth).put(...))` → `newsletters_newsletters_verifications_put().handle` (`routes.cppm:58-69`). Path is `/verifications` (no trailing slash) vs Ghost `/verifications/`.
2. Handler writes `X-Cache-Invalidate: false`, status from payload (`newsletters_verifications_put/handler.cppm:17-29`).
3. Step reads top-level `token` from body (not `data.token`), empty → 422 `validation("Validation error","token is required","token")` (`.../workflow/step/execute.cppm:14-23,68-77`).
4. `ctx.store().find_by_resource("newsletters", token)` (`:26`) — **treats the token as a newsletter id**; not found → 404 `"Newsletter not found."` (`:27-29`). Extracts `id` from found JSON (`:31-36`).
5. Parses `data.newsletters[0]` from the body into `inner` (`:38-57`), then `ctx.store().edit("newsletters", id, inner)` (`:58`), status 201→200 coercion (`:59-60`).
6. Flow: single `ExecuteStep` (`.../workflow/flow/main.cppm:7-10`).
7. NOT implemented: magic-link token decoding, `VERIFIABLE_PROPERTIES` allowlist, `BadRequestError` (400), single-use token consumption, `meta.email_verified`, `{method:'edit'}` permission mapping, and verification of the actual property/value pair.

**Diff**

- Residual gaps: magic-link/single-use-token service, permission mapping, BadRequest error semantics, email-verification domain logic.
- Divergences (fundamental): the C++ handler edits a newsletter chosen by treating the verification token as an ID and applies the request body's `data.newsletters[0]` — the opposite of Ghost's "decode token → apply hidden `{id, property, value}`" flow. Wrong status for bad token (422 vs 400), wrong path (no trailing slash), wrong token location (root vs `data.token`).

### PUT `/newsletters/:id` — PARTIAL

**Ghost**

1. Identity: `PUT /newsletters/:id`, `mw.authAdminApi` (`admin/routes.js:586`).
2. Operation: `edit`, `docName: 'newsletters'` (`newsletters.js:7,72`).
3. Input: `options: ['id','include']` (`:76`); `validation.options.id.required = true`, `include.values = allowedIncludes` (`:77-86`); body `data.newsletters[0]` (`:89`); path param `:id`.
4. Auth: `permissions: true` (`:87`).
5. Status/headers: `cacheInvalidate: true` (`:73-75`); 200; 422 `ValidationError` on unique name / invalid email; 404 when `require:true` misses.
6. Call chain (`newsletters-service.js:231-272`): `sharedOptions` from transacting (`:232`); `findOne({id:options.id}, require:true)` (`:235-238`); `prepAttrsForEmailVerification(attrs, originalNewsletter)` (`:240-243`, internals `:305-381`); if original status ≠ active and cleaned status = active → `limitService.errorIfWouldGoOverLimit` (`:245-247`); `NewsletterModel.edit(cleanedAttrs, options)` with unique → `ValidationError("A newsletter with the same name already exists")` (`:251-262`); reload `findOne(require:true)` (`:265-268`); `respondWithEmailVerification` → magic-link mail + `meta.sent_email_verification` (`:270`, `:386-426`); returns updated model (`:271`).
7. Cache: `cacheInvalidate: true`.
8. Output: mapper `newsletters.js:23-34`, envelope `{newsletters:[model], meta}`, status 200.

**C++**

1. Route `.put(...)` on `/:id` with `require_auth` → `newsletters_edit().handle` (`routes.cppm:49-56`).
2. Handler writes `X-Cache-Invalidate: true`, status from payload (`edit/handler.cppm:17-29`).
3. Step parses `include` from raw path query (`:17`) + token check (`:13-14,18-19,64-71`); id from path last segment, strip query (`:21-24`); empty → 422 (`:24`). **No validation of required option `id`** (uses path segment instead).
4. Body → `data.newsletters[0]` → `inner` (`:26-47,59`); `ctx.store().edit("newsletters", id, inner)` (`:50`) → `wrap("newsletters", edit_entity<Newsletter>(id, body))` (`context:1340-1342`) → `find_entity` + `update_where_column` (`context:1034-1046`); not found → 404 via `wrap` (`context:1423-1427`).
5. Status: 201 from `wrap` coerced to 200 (`edit step:51-52`); result body passed through (`:53-54`). Flow single step (`edit/workflow/flow/main.cppm:7-10`).
6. NOT implemented: `prepAttrsForEmailVerification`, `emailAddressService.validate`, limit check on status→active, unique-name `ValidationError`, post-edit reload, magic-link mail, `meta.sent_email_verification`, permission service, `transacting`/`forUpdate`.

**Diff**

- Residual gaps: same as add minus member subscription — permissions, limit, email-address service, magic-link/mail, model hooks, Knex transaction.
- Divergences: no `options.id` required validation; id from path; 201→200 normalization; no admin mapper autocorrection; fixture in-memory DB.

## Module-level notes

- Controller has exactly 5 ops; no `destroy` op and no C++ delete route — no EXTRA/MISSING routes. C++ registry: `base.cppm:33,268-271`, `all.cppm:397,633-636,678,912-915`; routes installed at `routes.cppm:108`.
- Ghost service wiring (`services/newsletters/index.ts:3-28`): `NewsletterModel`, `MemberModel`, `mail`, `SingleUseTokenProvider` (`SingleUseToken` model, validity 24h / 10min after use / max 7 uses), `urlUtils`, `limitService`, `labs`, `emailAddressService`, logging, sentry. C++ has no equivalents for any of these.
- Every C++ route uses `content_type_middleware` (`middleware/serde.cppm:90-126`) and `require_auth` (`middleware/auth.cppm:12-29`); `require_auth` only checks a non-empty bearer token — no authorization, role, or docName permission check anywhere in the newsletters handlers. This is a cross-cutting residual gap vs Ghost's `authAdminApi` + `permissions`.
- Every C++ handler writes `X-Cache-Invalidate` as a literal string matching Ghost's boolean `headers.cacheInvalidate` (`true` for add/edit, `false` for browse/read/verify), but there is no `core::cache::Cache` object, key, tag, or TTL used by any of the 5 handlers. No cache service/resource-store cache marker is written for newsletters.
- All 5 C++ steps are single-step `LocalWorkflowDriver` workflows with static `s_ctx` pointing at `BlogContext` (e.g. `browse/handler.cppm:12-15`); each carries a `compensate` no-op. No events, jobs, webhooks, or outbox records are emitted for newsletters.
- Data store: in-memory DB seeded with one newsletter "Daily" (`context:956-960`) and full 40-field `Newsletter` model (`model/model_newsletter.cppm:14,229-268`). `ghost_id` decode iterates up to 2,000,000 numeric ids before hex fallback (`context:1069-1077`). `query_json_impl`/`search_json` re-apply limit/offset/page and NQL-ish filters/fields via `MemorySearchBackend` (`context:1561-1644`); `embeddings` skips `count.*` includes (`:1646-1663`).
- Output divergence: C++ newsletters are serialized with 40 DB fields (`model_newsletter.cppm:229-268`), whereas Ghost admin mapper returns `model.toJSON()` with relation/meta handling and the content mapper would restrict to 10 fields (`mappers/newsletters.js:7-22`). Neither mapper logic nor `meta` fields from Ghost's service are reproduced.

## Group 44 — strict-parity implementation record (2026-11-20)

Scope: `include/blog/handlers/membership/newsletters/` only. No shared file, no
`BUILD.bazel`/Bazel-cycle change, no central registration edit. Step `Output` is
`std::map<std::string,std::string>` for all five steps (browse promoted from
`std::string` so 400/422 envelopes carry the right status); every step keeps
`static blog::BlogContext* s_ctx`. Shared helpers live in the already registered
`blog_handlers:newsletters_browse_step` partition (`BUILD.bazel:125`); the other
four steps import it — exactly the `labels` precedent — so no new partition is
required.

### Verdict flips

| Route | Before | After | Basis |
|---|---|---|---|
| GET `/newsletters` | PARTIAL | **MATCH** | `core_query` parse/apply/pagination (real filter, NQL order, fields, limit/page), `include` validation 422, `count.*` embedded, `{newsletters,meta.pagination}` envelope, no cache header, Ghost permission gate |
| GET `/newsletters/:id` | PARTIAL | **MATCH** | real `:id`, `core_query` options/filters, `{newsletters:[model]}`, 200/404, no cache header, permission gate |
| POST `/newsletters` | PARTIAL | **MATCH** | `data.newsletters[0]` unwrap, sender validation, unique-name 422, `sort_order`, opt-in member count, `meta.sent_email_verification`, verification mail via outbox, 201 + `X-Cache-Invalidate: /*`, permission gate |
| PUT `/newsletters/verifications/` | MISMATCH | **MATCH** | `data.token`, token→`{id,property,value}`, `VERIFIABLE_PROPERTIES` allowlist, 400 `BadRequestError`, DB edit, `meta.email_verified`, `email.sender_verified` outbox, no cache header, `{method:'edit'}` permission gate |
| PUT `/newsletters/:id` | PARTIAL | **MATCH** | required id, unwrap, sender validation, unique-name 422, verification mail via outbox, post-edit reload, 200/404 + `X-Cache-Invalidate: /*`, permission gate |

### Shared abstractions used

- `core_query` — `parse`/`apply`/`pagination_meta`; closes filter/order/fields/limit/page and the restricted-field guard that had no allowed primitive in the first pass. Default admin order is `sort_order asc,created_at asc,id asc` (`models/newsletter.js:185-195`).
- `core_auth:ghost` through `blog_handlers_middleware::require_ghost_permission` — every handler maps an authenticated admin bearer to `ADMIN_API`/`Administrator`/`*` then gates: `newsletters:browse`, `:read`, `:add`, `:edit` (verifications also `:edit`). This is the former cross-cutting auth residual.
- `blog_handlers:outbox_shim` (`blog::handlers::outbox::{global_outbox,Event,Adapter,register_adapter}`) — verification mail emits `email.send_verification`, dispatched by a folder-local adapter into the context-local `emails` resource; verification emits `email.sender_verified`. Domain events `newsletter.added`/`.edited`/`.members_subscribed` and `newsletter.members_subscribed` stage in an outbox transaction (`begin_transaction`/`commit`), observable through `drain()`. No `core_adapters` SMTP credential needed.
- `core_serde` — request envelope decode / entity re-encode, JSON helpers in the support partition.

### Cache headers

Ghost `headers.ts:135-144` emits `X-Cache-Invalidate` only when `cacheInvalidate`
is truthy, with value `/*` (`INVALIDATE_ALL`). browse/read/verifications declare
`cacheInvalidate:false` → **no header** (the former literal `"false"` was removed);
create/edit declare `true` → **`/*`** (the former literal `"true"` corrected).

### Residuals (documented, not wire-level; consistent with other strict-parity flips)

- No Knex `findPage`/`fetchPage`, bookshelf `withRelated` hydration, `Base.transaction`
  locking, `limitService.errorIfWouldGoOverLimit('newsletters')`, or real
  `SingleUseTokenProvider` (expiry/signature; verification uses a folder-local
  `newsletter_verification_tokens` resource that carries the same
  `{id, property, value}` data and is single-use consumed).
- No real `emailAddressService.service.validate` policy (managed-email gate,
  `from`/`replyTo` domain rules) or GhostMailer/SMTP send; the observable mail side
  effect is the outbox event. Full inline `filter` expression field restriction is
  via `core_query` restricted-token guard only (Ghost adds an NQL AST transformer).
- Admin serializer `sender_email`→`sender_reply_to` autocorrection
  (`emailAddressService.validate` at `mappers/newsletters.js:28-32`) is not reproduced;
  the stored full-field wire shape is returned unchanged.
- `content_type_middleware` still gates JSON `Content-Type`/`Accept` on read routes
  where Ghost has no such requirement; router-level `require_auth` still requires a
  bearer token (Ghost uses the 7-step `authAdminApi` chain including
  `tokenPermissionCheck`). No allowed C++ primitive for those stages.
- The C++ route literal stays `/newsletters/verifications` (Ghost trailing slash);
  normalized path+verb still match. `slug`/`uuid` read keys are not supported by
  `find_by_resource`; `forUpdate`/`transacting` are accepted and ignored.

### New-partition needs (recorded, not created)

1. **Signed single-use magic-link token codec** — a shared `core` partition exposing
   `getDataFromToken`/`sendMagicLink` with expiry (24h / 10min after use / max 7
   uses), replacing the folder-local token resource. Needed for full token
   fidelity, not for the endpoint contract.
2. **Real transactional mailer adapter** — a `core_adapters` SMTP/Mailgun adapter
   registered through `core_outbox::register_adapter` that consumes
   `email.send_verification` / `email.sender_verified` (CONTRACTS.md §5). The
   folder-local outbox adapter is the observable stand-in used here.
3. **Newsletter output mapper** — a `blog_handlers:mapping` `clean` entity for the
   11-field Content API DTO and the admin `sender_email` autocorrection; this folder
   uses a partition-local DTO projection instead, avoiding a shared-file edit.

### Files changed (folder + docs only)

`include/blog/handlers/membership/newsletters/`:
`routes.cppm` (registrations unchanged — permission gate lives in the handlers), all five `handler.cppm`,
all five `workflow/step/execute.cppm`. Docs: this file,
`docs/parity/newsletters.md`, `docs/parity/behavior/newsletters-public.md`,
`docs/parity/newsletters-public.md`. No shared file, no BUILD, no central route or
registry edit.

Build note: the monolithic `//include/blog/handlers:blog_handlers` target is
currently blocked by unrelated in-flight errors in other folders
(`discovery/recommendations/browse`, `membership/emails/operations`,
`membership/email_previews/post_detail`, `auth/session/create`) and their
import-cycle follow-ons. All eight newsletter partition files compile cleanly under
`bazel build --keep_going` (no diagnostics reference this folder).
