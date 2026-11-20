# Route behavior + C++ comparison checklist

One agent per Ghost controller file under `docs/Ghost/ghost/core/server/api/endpoints/`.
Write one report per module: `docs/parity/behavior/<controller-basename>.md`.
Source is READ-ONLY. Only create/overwrite your own report file.

## Ghost side — per route, document every behavior

For each route in the controller:

1. **Identity**: verb + path from `core/server/web/api/endpoints/admin/routes.js`
   (public/content controllers: `content/routes.js`; members/other apps: their own
   `routes.js`). Include middleware chain on the route and router-level mounts.
2. **Operation**: exported controller key and op (`browse`, `read`, `add`, `edit`,
   `destroy`, custom action) and the `docName`.
3. **Input**: `options[]`, `data[]`, `validation{}` (field names, allowed values,
   required fields), body shape (`data.<resource>[0]`), multipart fields, path
   params, query aliases (including `filter[x]=y`), headers read from the request.
4. **Auth/permissions**: `mw.authAdminApi` / `authenticatePublic` / member
   middleware, `permissions` (`true`, `false`, `{docName, method}`), `unsafeAttrs`,
   labs flags, permission services called inside `query`.
5. **Status + headers**: `statusCode`, `headers.cacheInvalidate`, custom headers
   (`Content-Disposition`, `X-Cache-Invalidate`, `Cache-Control`, `Location`),
   response `format`/`stream`.
6. **Full call chain**: trace `query(frame)` into the service/repository/model.
   Name the service method, the model method (e.g. `findPage`, `add`, `edit`,
   `destroy`), filters/relations/transactions, and any transformer (e.g.
   `mongoTransformer`, `toMetafieldsResponse`). Include nested service calls,
   side effects (events emitted, jobs queued, mail/outbox writes, webhook calls,
   third-party API calls) and error branches (NotFoundError, ValidationError,
   BadRequestError, 401/403/404/422/500).
7. **Cache**: exact cache service/key/invalidation URLs, TTLs, cache headers,
   cache-tag or namespace behavior.
8. **Output**: serializer name, envelope, field set, pagination meta, computed
   fields, and what happens on empty/missing results.

## C++ side — per matched route

Locate the current handler by exact path + verb (folder names are not reliable).
For each matched handler document:

- file paths + symbols (`routes.cppm`, `handler.cppm`, step and flow),
- `Input` type and parsing code,
- status/body/header writes,
- exact `ctx.store()` calls (resource strings, params, mutations, resource-store
  access) and any `core::cache::Cache` usage (keys, tags, TTL),
- middleware chain (`require_auth`, `optional_auth`, `bearer_middleware`,
  content-type wrappers, local permission helpers),
- side effects implemented (outbox records, cache markers, fixture writes),
- what is NOT implemented.

## Comparison

Per route: `MATCH`, `PARTIAL`, `MISMATCH`, `MISSING` (no C++ route), or `EXTRA`
(C++ route with no Ghost counterpart). Note:
- Ghost behaviors with no C++ primitive (permissions services, Tinybird/SMTP,
  real Knex transactions, remote HTTP fetches) as **residual gaps**,
- C++ behaviors that differ from Ghost (hardcoded ids, fabricated pagination,
  fixture fallbacks, stand-in cache markers) as **divergences**,
- `file:line` evidence on both sides for every claim.

## Required report structure

```markdown
# Behavior report: <module>

- Ghost controller: `...`
- Ghost route table: `...`
- C++ folders: `...` (or none)
- Analyzed: 2026-11-19

## Summary

| Verdict | Count |
|---|---|
| MATCH | n |
| PARTIAL | n |
| MISMATCH | n |
| MISSING | n |
| EXTRA | n |

## Route behavior table

| Verb + path | Op | Input | Auth | Status/headers | Ghost call chain | Cache | Output | C++ files | C++ behavior | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|

## Route details

### <verb> <path> — <verdict>

**Ghost**: numbered facts 1–8 above with `file:line`.
**C++**: numbered facts with `file:line`.
**Diff**: residual gaps and divergences.

## Module-level notes

Shared services/helpers used by several routes, cross-route side effects, and
anything not captured per route.
```
