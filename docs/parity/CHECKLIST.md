# Route parity checklist

Use this checklist for every Ghost controller route. One subagent owns one Ghost
controller file under `docs/Ghost/ghost/core/server/api/endpoints/`; it writes one
`docs/parity/<controller-basename>.md`.

Sources:

- Ghost controller: `docs/Ghost/ghost/core/server/api/endpoints/<domain>.js|ts`
- Ghost URL table: `docs/Ghost/ghost/core/server/web/api/endpoints/admin/routes.js`
  (public/content variants: `.../content/routes.js`, members/account and other
  web apps have their own `routes.js`)
- C++ route installation: `include/blog/handlers/<area>/<domain>/routes.cppm`
- C++ central install list: `include/blog/handlers/routes.cppm`
- C++ handler registry: `include/blog/handlers/all.cppm`
- C++ step logic: `include/blog/handlers/<area>/<domain>/<operation>/workflow/step/execute.cppm`
- C++ data store: `include/blog/context/context.cppm` (`ApiDataStore`, `ResourceStore`)
- C++ cache: `core::cache::Cache` created in `include/blog/handlers/all.cppm`, plus
  `members/operations.cppm`, `emails/operations.cppm`, `automations/operations.cppm`

Do not assume folder names match. Find the C++ counterpart by exact route path and
HTTP verb, then by handler method name (`handler.<name>()`). A Ghost domain may map
to multiple C++ folders or to none.

## Checks (one row per route)

1. **Route identity**: Ghost `router.<verb>('<path>', ..., http(api.<domain>.<op>))`
   vs C++ `Router/Route` path + verb in `routes.cppm`. Normalize `:id` / `:slug` /
   `:uuid` placeholders. Record both paths verbatim.
2. **Operation mapping**: Ghost controller key (`browse`, `read`, `add`, `edit`,
   `destroy`, `exportCSV`, custom action, ...) vs exact C++ `handler.<method>()`
   bound to that route.
3. **Input parity**: Ghost `options[]`, `data[]`, `validation{}`, allowed values for
   `include` / `formats` / `source` / `order`, `statusCode`, multipart fields vs
   C++ query-string parsing in the step, body DTO / `IRequest::body_as_string()`,
   and declared `using Input = ...`. List param names present on one side only.
4. **Output parity**: Ghost serializer / `response.format` / `stream` / returned
   model or `{data, meta}` / `statusCode` / `headers` vs C++ `response.set_status`,
   `set_body`, JSON envelope (`{"<resource>":..., "meta":{"pagination":...}}`),
   serde DTO, any hardcoded `limit/page/pages/total`. Note concrete divergences.
5. **DB call parity**: Ghost `models.X.findPage / findOne / add / edit / destroy`
   or service call (`postsService.browsePosts`, `membersService.*`, ...) and model
   name vs C++ `ctx.store().json_for("<resource>", params)`, `find_by_resource`,
   `mutations.*`, or `core::cache::Cache` read. Record the exact resource string and
   whether the C++ step does any real filtering/pagination.
6. **Cache parity**: Ghost `headers.cacheInvalidate`, `X-Cache-Invalidate` values,
   cache service / invalidation URLs vs C++ `core::cache::Cache` key names,
   namespaces (`members:*`, `emails:*`, `automations:*`), `ResourceStore` tags,
   TTL, and eviction. Explicitly say "none" if a side has no cache behavior.
7. **Auth / permissions**: Ghost middleware (`mw.authAdminApi`, member auth),
   `permissions.method` / `docName`, `unsafeAttrs`, vs C++ middleware chain
   (`content_type_middleware`, auth/bearer middleware), serde validation, and where
   auth is absent.
8. **Verdict + evidence**: `MATCH`, `PARTIAL`, `MISMATCH`, `MISSING` (no C++
   counterpart), or `EXTRA` (C++ route with no Ghost counterpart). Every row needs
   `file:line` evidence for both sides; `MISSING`/`EXTRA` needs evidence for the
   existing side and the grep/search used to confirm absence.

## Verdict rules

- `MATCH`: path, verb, input names, output envelope, DB resource/call, cache and
  auth all agree (formatting/serializer differences that do not change data are OK).
- `PARTIAL`: route and core operation agree, but one or more of input names,
  output envelope/meta, DB call, cache, or auth differ.
- `MISMATCH`: route exists on both sides but operation, resource, or data shape is
  materially different.
- `MISSING`: Ghost route has no C++ route/handler (expected for `*-public` domains).
- `EXTRA`: C++ route has no Ghost route in the corresponding controller.

## Required parity.md structure

```markdown
# Parity: <domain>

- Ghost: `docs/Ghost/ghost/core/server/api/endpoints/<file>`
- Ghost routes: `.../routes.js`
- C++: `include/blog/handlers/<area>/<domain>/routes.cppm` (or "none found")
- Checked: <ISO date>

## Summary

| Verdict | Count |
|---|---|
| MATCH | n |
| PARTIAL | n |
| MISMATCH | n |
| MISSING | n |
| EXTRA | n |

## Routes

| Ghost route (verb + path) | Ghost op | C++ route | C++ handler | Input | Output | DB call | Cache | Auth | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|

## Route details

### <verb> <path> — <verdict>
Short bullets for each divergent check with `file:line` on both sides; include
"no differences" when MATCH.

## Missing in C++

List every Ghost route with no C++ counterpart. Public/content-API domains are
expected to be entirely MISSING — still list every route.

## Extra in C++

List C++ routes in the matched folder(s) with no Ghost controller route.
```

Read-only task: never modify source files. Only create/overwrite your own
`docs/parity/<domain>.md`.
